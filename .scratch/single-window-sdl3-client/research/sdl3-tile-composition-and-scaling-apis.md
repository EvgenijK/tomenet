# SDL3 tile composition and scaling APIs

## Scope and result

This note answers the factual part of [Research SDL3 tile composition and scaling APIs](../issues/33-research-sdl3-tile-composition-and-scaling-apis.md). It audits the current TomeNET X11/SDL3 paths and the official SDL3 3.4 API/source, then presents implementation alternatives for [Choose raster references and defect compatibility](../issues/23-choose-raster-references-and-defect-compatibility.md). It does **not** choose the product filter, renderer backend, atlas layout, outline appearance, or fallback policy.

The central correction is terminological:

- **single-scaling** means every original asset layer is sampled directly into its final output-cell rectangle;
- it does **not** mean one draw call or a naive `terrain tile -> foreground tile -> masks on top` stack.

The current client first interprets mask colors as operations/layers, applies live palette attributes, and composes background and foreground visuals in a defined order. A new renderer can preserve that semantic pipeline with several SDL draw calls per cell while eliminating the old `source sheet -> font-sized sheet -> window-sized result` resampling chain.

The repository's release build currently pins SDL **3.4.10**, so `SDL_SCALEMODE_PIXELART`, added in SDL 3.4.0, is available to the planned product baseline ([release pin](../../../.github/docker/fedora43/install-build-requirements.sh#L30), [SDL 3.4.0 release notes](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.0#user-content-general)).

## 1. What the logical map cell contains

### 1.1 The protocol/Term model carries identities, not a flattened bitmap

In two-mask mode a cell has two independent attr/character pairs:

```text
background: a_back + c_back
foreground: a      + c
```

`Term_draw_2mask` stores both pairs. A zero `c_back` means “keep the current background”; the queue also repairs legacy ASCII/weather transitions by substituting the graphical solid feature where a composable graphical background is required ([two-mask queue](../../../src/client/z-term.c#L475), [network background retention](../../../src/client/nclient.c#L3280), [line-packet retention](../../../src/client/nclient.c#L4350)). This state behavior must occur before rasterization. A renderer that receives only an already-flattened tile cannot reproduce it reliably.

The Term redraw path is mixed-mode: with `higher_pict`, characters above `MAX_FONT_CHAR` use the picture hook, while ordinary IDs use the text hook; two-mask picture calls receive both pairs ([redraw dispatch](../../../src/client/z-term.c#L2639)). Therefore “map rendering” is not synonymous with “draw every cell from the tileset.” Font glyphs, procedural solid cells and graphical tiles can coexist.

### 1.2 A tilesheet pixel has one of several meanings

The legacy sheets encode categorical mask roles as exact colors. Defaults include foreground `0xFC00FB`, background `0x3E3D00`, and dual-background/transparent `0x1D211C`; explicit `m:<index>:<hexRGB>` entries can override these for the main sheet or an individual subtileset ([mask definitions](../../../src/common/defines-features.h#L627), [preference parser](../../../src/client/c-files.c#L1653)). For three-mask sheets, the SDL3 compatibility defaults intentionally swap the historical BG/BG2 positions when preferences do not define the proper order; explicit preference order is background, foreground, outline ([default compatibility mapping](../../../src/client/main-sdl3.c#L3114), [main/subtileset inheritance](../../../src/client/main-sdl3.c#L3135)).

At native asset resolution, each pixel therefore resolves to one of:

1. ordinary authored color;
2. background/transparent hole;
3. foreground-color coverage, tinted from the cell's current attr;
4. optional outline coverage.

Those are semantic channels. Interpolating the encoded RGB keys before decoding them destroys the categories. The current SDL3 client consequently nearest-scales the mask-bearing source, separates its layers, and only then substitutes filtered ordinary-color pixels when linear or Lanczos is selected ([preparation contract](../../../src/client/main-sdl3.c#L3503), [layer split](../../../src/client/main-sdl3.c#L3200)). A new pipeline may decode at original resolution instead, but it must preserve the same distinction.

### 1.3 Current single-mask composition

For ordinary graphics (`masks-per-tile = 2`, `tiles-per-coordinate = 1`) the current SDL3 path:

1. resolves the palette entry through `term2attr`;
2. preserves the text convention that equal foreground/background colors yield a solid cell;
3. fills the destination cell with the attr's background color;
4. selects the main sheet or `c_subtileset[c]`;
5. draws authored tile pixels while replacing foreground-mask coverage with the attr's foreground color ([single-mask entry](../../../src/client/main-sdl3.c#L2092), [common cell composer](../../../src/client/main-sdl3.c#L2070)).

The X11 path performs the same role substitution by building a prepared tile from ordinary pixels plus the scaled foreground mask; its pixel arithmetic is an implementation technique, not an additional semantic layer ([X11 preparation](../../../src/client/main-x11.c#L2306), [single-mask draw](../../../src/client/main-x11.c#L2380)).

### 1.4 Current two-mask composition

For two-mask graphics (`masks-per-tile = 3`, `tiles-per-coordinate = 2`) the current SDL3 path builds two visual records and draws them bottom-to-top ([two-mask entry](../../../src/client/main-sdl3.c#L2147), [ordered tile loop](../../../src/client/main-sdl3.c#L2079)):

```text
fill cell with background attr's background color

background/terrain visual:
    authored base pixels
    foreground-mask coverage tinted with a_back's foreground color
    authored/generated outline tinted with a_back's background color

foreground/object visual:
    authored base pixels, with background-mask holes preserving terrain
    foreground-mask coverage tinted with a's foreground color
    authored/generated outline tinted semi-transparent black
```

For the upper visual, its own background color is explicitly transparent/ignored so it cannot overwrite the terrain fill. `c_back == 32` or `0` suppresses the background tile; the cell keeps the selected background-color fill, with `32` forcing the legacy dark fill. Each of `c_back` and `c` independently resolves a subtileset.

X11 arrives at the comparable result by preparing the background visual, preparing the foreground visual, replacing the foreground visual's dual-background-mask pixels with pixels from the prepared background, then copying the combined tile ([X11 two-mask composition](../../../src/client/main-x11.c#L2635)). The new renderer does not need the X11 reverse-AND/OR-era technique, but it cannot replace this process with “draw complete foreground bitmap, then draw masks over it”: the background mask is a hole/select operation, and the foreground mask is a palette-color substitution.

### 1.5 Outline is a separate semantic stage

The outline can be authored in the third mask channel or forced/generated. Current SDL3 generation dilates non-background content within each tile boundary and subtracts the source silhouette; it scales the configured source-radius to the active cell dimensions and deliberately prevents bleed into adjacent atlas tiles ([outline generation](../../../src/client/main-sdl3.c#L3290)). In one-layer mode, an authored outline key is cleared rather than exposed as an ordinary color ([single-layer cleanup](../../../src/client/main-sdl3.c#L3470)).

The planned renderer must preserve whether outline coverage exists, its per-tile bounds, its place after the tile's base/foreground-color stage, and its selected runtime color/alpha. It need not preserve this exact CPU dilation implementation or pixel footprint; the new cursor/indicator design is a separate decision.

### 1.6 Subtilesets, raw pictures, palette animation and caches

Subtilesets are not just alternate source images. Visual mappings (`R/K/F/U/...`) record a per-character `c_subtileset` at preference-load time; background and foreground IDs can therefore come from different sheets ([mapping examples](../../../src/client/c-files.c#L1201)). A subtileset may override mask colors or inherit the primary set, and failures disable that subtileset rather than changing the logical ID ([subtileset preparation](../../../src/client/main-sdl3.c#L3617)).

Raw pictures are a separate path. `I:` preferences name arbitrary source rectangles which may exceed one cell; the current renderer selects the owning subtileset and draws that rectangle rather than interpreting it as a masked cell tile ([raw-picture definitions](../../../src/client/c-files.c#L1315), [SDL3 raw-picture draw](../../../src/client/main-sdl3.c#L2249)). They need source-to-final-canvas scaling and correct rectangle bounds, but not the normal terrain/foreground mask stack.

Current prepared-tile cache identity includes tile index, subtileset and runtime mask colors; the sheet cache additionally includes source hash, destination tile size, filter and mask/raw-picture context ([tile cache](../../../src/client/main-sdl3.c#L1981), [sheet-cache keys](../../../src/client/main-sdl3.c#L2655)). This reveals the invalidation dependencies, not a required cache representation. A new renderer that keeps coverage separate and applies attr colors during drawing can avoid rebuilding colored tile pixels when the live palette changes.

## 2. What the current scaler does—and what can be removed

The current SDL3 backend is surface-based; no `SDL_Renderer` or `SDL_GPU` calls occur in `src/client`. At graphics initialization it resizes the complete main sheet and each enabled subtileset to the active legacy Term font cell, splits layers, retains a scaled raw-picture sheet, and allocates font-cell-sized preparation/cache surfaces ([asset state](../../../src/client/main-sdl3.c#L1863), [graphics initialization](../../../src/client/main-sdl3.c#L3586)).

Its selectable filters are:

- project nearest-neighbor sheet scaling;
- project bilinear scaling with per-tile/raw-picture bounds and mask-color omission;
- project Lanczos scaling with the same boundary handling ([linear implementation](../../../src/client/main-sdl3.c#L2839), [Lanczos implementation](../../../src/client/main-sdl3.c#L2943), [dispatch](../../../src/client/main-sdl3.c#L3057)).

That architecture creates a font-sized raster asset before the result is copied to the window. In the accepted fit-plus-zoom model, stretching a finished map canvas again would add a second resampling stage. Neither the old CPU scaler nor the intermediate font-sized sheet is part of the compatibility contract. What remains contractual is:

- categorical mask decoding before non-nearest color sampling;
- no sampling across logical tile/raw-picture boundaries;
- consistent final geometry for base, coverage, outline, terrain and foreground;
- per-ID subtileset selection;
- runtime attr/palette color application;
- raw-picture source bounds;
- safe invalidation after asset/filter/palette/size changes.

## 3. Ready-made scaling and composition facilities in SDL3

### 3.1 `SDL_Renderer`: direct source-region to final-cell drawing

`SDL_RenderTexture` copies an `SDL_FRect` source region to an `SDL_FRect` destination at subpixel precision. This is the direct primitive for `original asset region -> final cell`; no intermediate raster is required ([official API](https://wiki.libsdl.org/SDL3/SDL_RenderTexture)). `SDL_RenderGeometry` supplies textured triangles/quads with UVs and per-vertex color/alpha, which can batch many cell/layer quads while preserving the same sampling model ([official API](https://wiki.libsdl.org/SDL3/SDL_RenderGeometry)). Geometry does not itself understand TomeNET mask semantics; those semantics still have to be decoded into textures/coverage or expressed by a shader.

Per texture, `SDL_SetTextureScaleMode` provides:

| SDL mode | Available | Stated behavior | Important qualification |
|---|---:|---|---|
| `SDL_SCALEMODE_NEAREST` | 3.2+ | nearest pixel sampling | Explicitly set it; SDL3 textures otherwise default to linear. |
| `SDL_SCALEMODE_LINEAR` | 3.2+ | linear filtering | Built-in, but not high-quality multi-tap downsampling. |
| `SDL_SCALEMODE_PIXELART` | 3.4+ | improved nearest-style pixel-art scaling without blur | Backend may substitute the closest supported mode; verify the returned status and visual result on release backends. |

Sources: [`SDL_ScaleMode`](https://wiki.libsdl.org/SDL3/SDL_ScaleMode), [`SDL_SetTextureScaleMode`](https://wiki.libsdl.org/SDL3/SDL_SetTextureScaleMode), [SDL 3.4.0 release notes](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.0#user-content-general).

`PIXELART` is a real SDL algorithm, not a TomeNET custom resampler. In SDL's GPU renderer it uses a dedicated shader path: UVs are adjusted from pixel derivatives/texel size, then linearly sampled ([SDL 3.4 shader](https://github.com/libsdl-org/SDL/blob/main/src/render/gpu/shaders/texture_advanced.frag.hlsl#L1034), [SDL 3.4.10 GPU renderer selection](https://raw.githubusercontent.com/libsdl-org/SDL/release-3.4.10/src/render/gpu/SDL_render_gpu.c)). SDL's public contract still allows backend fallback, so `PIXELART` cannot be treated as an identical cross-backend raster guarantee.

`SDL_ScaleSurface` accepts the same `SDL_ScaleMode` enum for a CPU-created scaled copy, but this does **not** mean it implements all the same algorithms ([official API](https://wiki.libsdl.org/SDL3/SDL_ScaleSurface)). Follow-up [PCF research](pcf-loading-and-scaling-options.md) verified that SDL 3.4.0's surface blitter substitutes Nearest for PixelArt; indexed surfaces are also forced to Nearest ([surface implementation](https://github.com/libsdl-org/SDL/blob/release-3.4.0/src/video/SDL_surface.c#L1066)). Genuine PixelArt preparation requires a supported renderer texture path; a successful surface call alone does not establish that the requested algorithm ran. Surface scaling is appropriate for a cache produced at the **exact final size** using its effective supported filter. Scaling that cached copy again during presentation recreates double scaling.

### 3.2 Tint and alpha composition

Decoded foreground/outline coverage can be stored as white RGBA coverage textures and recolored at draw time. `SDL_SetTextureColorModFloat` multiplies source RGB by the requested color ([official API](https://wiki.libsdl.org/SDL3/SDL_SetTextureColorModFloat)); `SDL_RenderGeometry` can instead carry color/alpha per vertex. Normal alpha/premultiplied-alpha modes are predefined and portable; SDL documents its predefined blend modes as supported everywhere ([blend definitions](https://wiki.libsdl.org/SDL3/SDL_BlendMode)).

Custom blend modes exist, but support differs by renderer and the call may fail or be approximated. They are not a portable substitute for decoding categorical mask operations ([custom blend support](https://wiki.libsdl.org/SDL3/SDL_ComposeCustomBlendMode), [`SDL_SetTextureBlendMode`](https://wiki.libsdl.org/SDL3/SDL_SetTextureBlendMode)). The portable renderer formulation is therefore multiple ordinary alpha draws in the legacy semantic order, not a recreation of X11 bitwise composition.

### 3.3 Render targets do not inherently cause double scaling

`SDL_SetRenderTarget` can select a texture created with `SDL_TEXTUREACCESS_TARGET`; viewport, clipping, scale and logical-presentation state are stored per target ([official API](https://wiki.libsdl.org/SDL3/SDL_SetRenderTarget)). A map target sized to the final output viewport can receive all cell-layer draws and then be copied to the window **1:1**. This is still single-scaling and is useful for damage caching.

A low-resolution map target later stretched to the final viewport is a second scaling stage. Likewise, applying `SDL_SetRenderLogicalPresentation` to both an intermediate target and the final renderer can apply presentation twice; SDL explicitly warns about this ([logical-presentation API](https://wiki.libsdl.org/SDL3/SDL_SetRenderLogicalPresentation)).

### 3.4 Atlas boundary safety is still the application's responsibility

Nearest, linear and pixel-art sampling operate on a texture, not on TomeNET's logical tile boundaries. Clamp addressing clamps at the whole texture edge, not at every `srcrect`. Filter taps near a tile edge can therefore see the neighboring atlas tile. SDL's pixel-art shader likewise operates from texture-wide texel size and UVs.

Viable representations are:

- extracted texture per tile/coverage layer;
- a padded atlas with replicated gutters around every tile and UVs targeting the padded region;
- a size/filter cache that materializes isolated final-size tiles;
- a custom shader/texture-array representation with explicit per-tile bounds.

This is especially important for custom sheets and for fractional/down-scaling. An inset UV alone is not a complete general solution for arbitrary minification. The chosen representation must also isolate generated outlines per tile, as current code already does.

## 4. Raw `SDL_GPU`: more control, not automatically a better filter

SDL's lower-level GPU API separates minification, magnification and mipmap sampling. `SDL_GPUSamplerCreateInfo` carries min/mag filters, mipmap mode, LOD limits/bias, address modes and optional anisotropy ([official struct](https://wiki.libsdl.org/SDL3/SDL_GPUSamplerCreateInfo)). However:

- `SDL_GPUFilter` contains only `NEAREST` and `LINEAR` ([official enum](https://wiki.libsdl.org/SDL3/SDL_GPUFilter));
- mipmap selection is only nearest or linear between levels ([official enum](https://wiki.libsdl.org/SDL3/SDL_GPUSamplerMipmapMode));
- textures must be created with multiple levels and mipmaps explicitly generated outside a pass ([texture levels](https://wiki.libsdl.org/SDL3/SDL_GPUTextureCreateInfo), [`SDL_GenerateMipmapsForGPUTexture`](https://wiki.libsdl.org/SDL3/SDL_GenerateMipmapsForGPUTexture));
- `SDL_BlitGPUTexture` is a nearest/linear copy operation outside a pass, not a layer-composition API ([official API](https://wiki.libsdl.org/SDL3/SDL_BlitGPUTexture)).

The high-level SDL 3.4.10 GPU renderer creates ordinary `SDL_Texture` backing images with one mip level, so selecting `LINEAR` through `SDL_Renderer` does not secretly provide a mip chain ([pinned renderer source](https://raw.githubusercontent.com/libsdl-org/SDL/release-3.4.10/src/render/gpu/SDL_render_gpu.c)). Raw `SDL_GPU` can add mipmaps for strong minification, but doing so is an explicit architecture choice with atlas/mask generation consequences.

`SDL_SCALEMODE_PIXELART` is not a raw `SDL_GPUFilter`; reproducing it in raw GPU code means using an equivalent shader. SDL 3.4 can also attach a custom fragment shader to its GPU-backed 2D renderer through `SDL_CreateGPURenderState`, but that is a GPU-renderer-only extension and the shader becomes project code ([official API](https://wiki.libsdl.org/SDL3/SDL_CreateGPURenderState)). A shader can decode/combine several channels in one draw, but it does not remove the need to specify the legacy layer semantics and fallbacks.

### 4.1 Facilities that SDL3 does not provide

There is no ready-made SDL3 Lanczos mode:

- it is absent from `SDL_ScaleMode`;
- it is absent from `SDL_GPUFilter` and GPU blit filters;
- mipmaps are not Lanczos;
- SDL3 removed SDL2's ambiguous `ScaleModeBest` and directs users to linear instead ([SDL3 migration guide](https://wiki.libsdl.org/SDL3/README-migration#renderer)).

Keeping Lanczos would require the existing CPU implementation, a new custom shader/compute path, or an external library. None qualifies as “use a ready SDL3 algorithm.” Anisotropy is a directional sampler feature, not a general Lanczos replacement, and its GPU-device feature is optional.

## 5. Viable single-scaling pipelines

All alternatives below preserve the same logical cell state and composition order. They differ only in where mask decoding, filtering and caching occur.

### Alternative A — decoded native-resolution layers with `SDL_Renderer`

At asset load, decode each primary/subtileset sheet at original resolution into:

- RGBA authored-color/base texture with mask pixels transparent;
- foreground coverage texture;
- outline coverage texture where present/generated;
- unchanged logical source rectangles and raw-picture definitions.

At frame time, calculate one final `SDL_FRect` per grid cell and submit the ordered base/coverage/outline draws for terrain and then foreground. Each texture is sampled directly to that final rectangle with `NEAREST`, `LINEAR` or `PIXELART`; palette attrs tint coverage at draw time. A final-size render target is optional and copied 1:1.

Properties:

- uses ready high-level SDL algorithms and portable predefined alpha blending;
- removes both custom scaling and double scaling;
- needs atlas isolation/padding or per-tile textures;
- linear/pixel-art filtering of decoded coverage produces partial alpha at boundaries, which is a visual policy to evaluate, not corrupted mask decoding;
- high-level textures have no application-controlled mip chain.

### Alternative B — isolated final-size tile cache using SDL scaling

Keep decoded native-resolution channels, but on demand build an isolated cache entry for `(asset generation, subset, tile ID, final cell pixel size, filter, outline settings)`. Use `SDL_ScaleSurface` or render directly into a target of exactly that cell size. Runtime palette colors should remain separate if palette animation must avoid cache churn.

Properties:

- uses SDL's scaler and guarantees one scaling step if copied 1:1;
- naturally prevents cross-atlas bleeding;
- cache rebuilds on output/DPI/zoom size changes and can have many fractional-size variants;
- precomposing palette colors increases invalidation and memory;
- not equivalent to a low-resolution map canvas subsequently stretched.

### Alternative C — raw `SDL_GPU` with decoded channels and mipmaps

Upload isolated/padded decoded channel atlases with explicit mip levels, generate mipmaps, and use textured quads with nearest/linear min/mag/mipmap samplers. A custom shader can apply attr colors and combine base/coverage/outline channels; without a custom shader, issue ordered draws as in Alternative A.

Properties:

- offers explicit minification behavior, texture arrays and custom shaders;
- can reduce aliasing for strong downscaling through mipmaps;
- raw samplers do not provide SDL's ready `PIXELART` mode or Lanczos;
- mip generation for coverage and padded atlases needs a defined semantic/edge policy;
- substantially larger backend, shader-packaging and cross-platform validation surface.

### Alternative D — encoded-mask shader

Upload the original encoded sheet, sample/decode mask keys in a custom shader, and combine channels there. This avoids CPU-created coverage textures only if categorical matching happens before filtered color interpolation or uses separate nearest samples. A single ordinary filtered sample of the encoded sheet is invalid.

Properties:

- can reduce textures/draw calls;
- preserves source assets without CPU layer expansion;
- necessarily uses project shader logic, not only ready SDL scaling/composition;
- exact-key comparisons, filtered authored color, outline generation, subtileset overrides, atlas bounds and fallbacks all become shader/backend obligations.

## 6. Decision-ready questions

This research does not answer these product/architecture questions; it makes their consequences explicit.

1. **Primary rendering level:** use decoded-layer `SDL_Renderer` (Alternative A), its final-size isolated cache variant (B), or accept raw-GPU/shader complexity for explicit mipmaps/multi-channel composition (C/D)?
2. **Exposed filters:** replace the current project `Nearest / Linear / Lanczos` choice with the ready SDL3 set `Nearest / Linear / Pixel art`, remove Lanczos, or retain a custom non-SDL path as a separately named optional mode?
3. **Filter scope:** one user choice for authored-color and decoded coverage, or separate profile-owned choices? Mask-key decoding itself is always unfiltered/source-space.
4. **Pixel-art fallback:** if `SDL_SetTextureScaleMode(PIXELART)` resolves to a closest backend mode, accept that backend result, fall back explicitly to nearest, or mark the mode unavailable? No pixel-equality contract is implied.
5. **Strong minification:** are high-level linear/pixel-art results sufficient for the minimum supported fitted map, or is raw-GPU mipmap support required? This should be judged on the representative coverage corpus, not by algorithm name alone.
6. **Atlas isolation:** padded/extruded decoded atlases, per-tile textures, or exact-final-size isolated cache?
7. **Coverage scaling appearance:** allow linear/pixel-art coverage alpha for smoother edges, or force nearest sampling for PCF/mask-derived coverage while authored color uses the selected filter?
8. **Outline source:** retain authored outline plus optional generated outline, and if generated, produce it once in source space or per final size? This is separate from the new cursor design.
9. **Map target/cache:** draw directly to the window every frame or compose into a final-output-sized damage-cache target copied 1:1? A smaller stretched map target is excluded by the accepted single-scaling premise.
10. **Raw pictures:** apply the same ready SDL filter choice directly from their original isolated rectangles to their final special-canvas rectangles, or give raw pictures a separate profile policy?

## 7. Acceptance consequences independent of the selected alternative

No pixel-perfect or golden-image gate is required. The structural acceptance contract can require:

- a cell retains independent `(a_back, c_back)` and `(a, c)` identities through rendering;
- single-mask and two-mask cells follow the ordered stages in sections 1.3/1.4;
- encoded mask colors are decoded before non-nearest filtering;
- background-mask holes reveal the current terrain, not a stale captured bitmap;
- main/subtileset selection is independent for terrain and foreground;
- attr/palette changes recolor coverage without changing the visual ID;
- authored/generated outlines stay within their cell/tile bounds;
- mixed glyph/tile cells dispatch by semantic visual type and preserve background state across ASCII weather/effects;
- all cell layers, cursor/effects and hit testing use exactly the same final grid edges;
- source sampling cannot bleed from adjacent atlas tiles;
- each asset layer is scaled at most once on its route to final output pixels;
- any map render target has final output dimensions and is presented 1:1;
- `Nearest`, `Linear` and, if exposed, `Pixel art` are verified by returned status and structural output on Linux and Windows release renderers;
- fallback/substitution is observable in diagnostics rather than silent;
- raw-picture bounds and custom asset/subtileset mappings remain intact;
- resize, zoom, DPI, filter and asset switches invalidate only the affected raster resources and never the logical map state.

## 8. Concise factual answer

Yes, the map can remain a logical stack and be rendered directly from original assets into final cells without double scaling. Preserving the original client does **not** mean drawing a whole tile and then laying colored masks over it. It means decoding mask keys into base/foreground-coverage/outline roles, applying current attrs, and executing the terrain-then-foreground stage order above. That composition may take several SDL draws while every source layer is sampled only once.

SDL3 3.4.10 already supplies nearest, linear and pixel-art texture scaling, float destination rectangles, textured geometry, alpha blending and final-size render targets. Raw SDL_GPU additionally supplies nearest/linear min/mag sampling and mipmaps. SDL3 does not supply Lanczos. The remaining choice is therefore between the high-level decoded-layer renderer and more complex cache/GPU variants, plus the visual policy for SDL's three high-level scale modes.
