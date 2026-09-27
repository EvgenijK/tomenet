# UI and asset scaling standards for the single-window SDL3 client

## Scope and result

This note answers the factual part of [Research UI and asset scaling standards](../issues/30-research-ui-and-asset-scaling-standards.md). It separates platform/API facts from candidate product policies. It does **not** choose a scaling policy, renderer implementation, filter, minimum legibility threshold, or performance budget, and it does not introduce pixel-perfect acceptance.

The central finding is that one scalar called “DPI” is not a sufficient model. The client needs at least four distinct quantities:

1. SDL window coordinates, whose physical meaning is platform-dependent;
2. render-output/framebuffer pixels;
3. OS-selected content/display scale;
4. independent user choices for UI/text scale and map/cell scale.

The existing product decision that the supported client area is **1024×768 logical units**, while UI text and map scales remain independent, can be implemented consistently on Windows, X11 and Wayland only after those quantities are kept separate. The formulas and alternatives below make the remaining decisions explicit.

## 1. Authoritative platform and library facts

### 1.1 SDL3 exposes three different sizes/scales

SDL3 explicitly distinguishes:

- window size in platform-native window coordinates (`SDL_GetWindowSize`);
- window/backbuffer size in pixels (`SDL_GetWindowSizeInPixels`, or renderer output size);
- window pixel density, defined as pixel size divided by window size (`SDL_GetWindowPixelDensity`);
- display content scale, folded with pixel density into `SDL_GetWindowDisplayScale`.

`SDL_WINDOW_HIGH_PIXEL_DENSITY` asks for a high-density backbuffer. `SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED` is guaranteed on creation and resize and is the event SDL designates for rebuilding/resizing graphics contexts; `SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED` reports a content-scale change. SDL also says its APIs use platform-native coordinates unless stated otherwise. On Windows and X11 these coordinates follow the physical-pixel-oriented model; Wayland follows the logical-window-coordinate model. These are SDL contracts, not implementation suggestions: [SDL3 high-DPI README](https://wiki.libsdl.org/SDL3/README-highdpi), [`SDL_GetWindowDisplayScale`](https://wiki.libsdl.org/SDL3/SDL_GetWindowDisplayScale), [`SDL_CreateWindow`](https://wiki.libsdl.org/SDL3/SDL_CreateWindow).

Let:

- `W` = a size in SDL window coordinates;
- `P` = the corresponding render-output size in pixels;
- `D = P / W` = window pixel density;
- `S` = `SDL_GetWindowDisplayScale(window)`.

SDL does not return “TomeNET logical units”. A cross-platform project coordinate can, however, be **derived** as `L = P / S`, with window-event conversion `L = W × D / S`. This is an inference from SDL's definitions, not another SDL API. It yields the same content extent in SDL's published examples: a Windows 3840-pixel window at 200% and a high-density logical 1920-point window both become 1920 content units. Consequently the already accepted 1024×768 logical minimum would be tested against `P/S`, not blindly against `SDL_GetWindowSize`.

Fractional values are normal. SDL's render API uses floating-point destination rectangles and offers window↔render coordinate conversion; rounding should therefore occur at raster allocation/boundaries, not by converting the whole layout to integers early: [`SDL_RenderTexture`](https://wiki.libsdl.org/SDL3/SDL_RenderTexture), [`SDL_RenderCoordinatesFromWindow`](https://wiki.libsdl.org/SDL3/SDL_RenderCoordinatesFromWindow). GTK's first-party desktop guidance independently uses the same separation: application/widget units are not device pixels, and a surface scale may be 1.5 or 1.75, not only an integer: [GTK coordinate systems and scaling](https://docs.gtk.org/gtk4/coordinates.html).

### 1.2 SDL logical presentation is a tool, not the required architecture

`SDL_SetRenderLogicalPresentation` can make a render target act like a fixed resolution and provides stretch, letterbox, overscan and integer-scale modes. SDL warns against applying logical presentation twice and explicitly permits disabling it while drawing high-resolution text. `SDL_SetRenderScale` instead scales drawing coordinates for the current target and says integer factors give best results. These APIs do not choose whether a resizable application should use one fixed low-resolution composition texture, rasterize every element at output density, or mix both approaches: [`SDL_SetRenderLogicalPresentation`](https://wiki.libsdl.org/SDL3/SDL_SetRenderLogicalPresentation), [`SDL_RendererLogicalPresentation`](https://wiki.libsdl.org/SDL3/SDL_RendererLogicalPresentation), [`SDL_SetRenderScale`](https://wiki.libsdl.org/SDL3/SDL_SetRenderScale).

For this project that distinction matters: stretching one complete 1024×768 texture is simple, but would also stretch text, PCF glyphs, tiles and UI chrome together and would not preserve the already-decided independent UI/map scales. Rendering directly to the pixel-sized output from a logical layout, or using separate density-aware targets by role, avoids that coupling. This is an engineering implication, not a selected policy.

### 1.3 Windows requires real per-monitor DPI awareness, not OS bitmap stretching

For supported Windows 10/11 targets, Microsoft recommends declaring the process default DPI awareness in the application manifest and documents `PerMonitorV2`. A per-monitor-aware window is responsible for rerendering/re-layout when DPI changes; DPI-unaware and system-aware modes can be bitmap-scaled by Windows and become blurry on another monitor. Microsoft also documents `WM_DPICHANGED` and its suggested window rectangle. SDL surfaces this cross-platform as its display-scale and pixel-size events, but packaging still needs a verified DPI-aware manifest/context: [setting process DPI awareness](https://learn.microsoft.com/en-us/windows/win32/hidpi/setting-the-default-dpi-awareness-for-a-process), [DPI awareness contexts](https://learn.microsoft.com/en-us/windows/win32/hidpi/dpi-awareness-context), [high-DPI API reference](https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-reference).

The platform obligation is therefore behavioral: avoid OS bitmap virtualization, track the window's current monitor scale, relayout/rerasterize at that scale, and keep pointer hit-testing in the same transformed coordinate system as drawing. Whether SDL alone supplies the process declaration in a particular packaged build or the executable manifest supplies it must be verified in Windows packaging; it should not be assumed from successful compilation.

### 1.4 Linux has both X11-like and Wayland-like coordinate models

SDL3 prefers Wayland where available. Its Wayland documentation says high-density custom SDL surfaces use logical sizes for `SDL_SetWindowSize` and pixel sizes for the backbuffer; SDL manages the Wayland viewport/scale when the window is created high-density. SDL warns that forcing non-DPI-aware 1:1 output can introduce rounding, blur, imprecise positions, monitor-transition jumps and cursor precision loss, and tells new applications to implement DPI awareness instead: [SDL3 Wayland README](https://wiki.libsdl.org/SDL3/README-wayland), [`SDL_HINT_VIDEO_WAYLAND_SCALE_TO_DISPLAY`](https://wiki.libsdl.org/SDL3/SDL_HINT_VIDEO_WAYLAND_SCALE_TO_DISPLAY).

The Wayland fractional-scale protocol reports a preferred surface scale in 1/120 increments; this confirms that fractional scale is a first-class compositor input rather than an error to quantize away: [official `fractional-scale-v1` protocol](https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/staging/fractional-scale/fractional-scale-v1.xml). SDL owns that protocol integration for a normal `SDL_Window`; the application should consume SDL's window scale/pixel-size contract rather than attach its own Wayland viewport.

XSETTINGS is a generic X11 desktop mechanism for propagating settings and runtime changes, not an application-specific configuration store: [freedesktop.org XSETTINGS specification](https://specifications.freedesktop.org/xsettings/0.5/). SDL already supplies a portable display-scale abstraction across X11 and Wayland. Directly reading desktop-specific DPI/text keys would therefore be an additional platform integration with its own fallback and testing burden, not a prerequisite for basic SDL DPI correctness.

### 1.5 Display scale and accessibility text scale are different inputs

Windows guidance asks applications to validate both global display scale and the separate accessibility text-size setting. WinUI exposes text scale independently, with a documented 1.0–2.25 range and change notification; smaller and larger font sizes need not scale uniformly. An SDL-drawn custom UI does not inherit WinUI control behavior automatically: [Microsoft accessible text requirements](https://learn.microsoft.com/en-us/windows/apps/design/accessibility/accessible-text-requirements).

WCAG2ICT is an **informative W3C Group Note**, not a mandatory native-game standard. It is nevertheless a useful acceptance benchmark: non-web software should work with platform text-resizing capabilities, or provide enlargement up to 200%, without loss of content or functionality; its reflow guidance explicitly allows exceptions for maps, games, tables and other inherently two-dimensional regions: [WCAG2ICT 2.2, Resize Text and Reflow](https://www.w3.org/TR/wcag2ict-22/#resize-text).

Facts established by those sources:

- honoring OS display scale does not by itself honor a separate accessibility text-size preference;
- an application-level UI/text scale is a defensible cross-platform fallback where SDL exposes no portable text-scale API;
- map cells and spatial canvases need not grow with text at the same rate, but controls/labels must not clip or disappear when text grows.

Whether the client reads a Windows-only system text factor, offers only its own cross-platform UI/text control, or combines both is a product decision.

### 1.6 SDL_ttf can rerasterize vector fonts at a target size/DPI

SDL_ttf treats font size as a floating-point point size and supports explicit horizontal/vertical target DPI; its default target is 72 DPI. It also lets a font be resized dynamically. Hinting controls pixel-grid fitting, while SDF rendering is available for sharp shader-assisted scaling but requires a compatible shader path: [`TTF_OpenFontWithProperties`](https://wiki.libsdl.org/SDL3_ttf/TTF_OpenFontWithProperties), [`TTF_SetFontSizeDPI`](https://wiki.libsdl.org/SDL3_ttf/TTF_SetFontSizeDPI), [`TTF_HintingFlags`](https://wiki.libsdl.org/SDL3_ttf/TTF_HintingFlags), [`TTF_SetFontSDF`](https://wiki.libsdl.org/SDL3_ttf/TTF_SetFontSDF).

Therefore a TTF can be rerasterized for `OS scale × user text/map scale` instead of rendering once and stretching the result. Multiplying point size by scale and also setting proportional target DPI would double-apply scaling; a future implementation must choose one sizing convention and test it. This is independent from which TTF file supplies the required TTF path.

### 1.7 Filtering is per texture/asset operation, not a global DPI rule

SDL texture scaling supports nearest and linear filtering; current SDL 3.4 also names a pixel-art mode. The default texture scale mode is linear, and unsupported modes may be replaced by the closest supported mode: [`SDL_SetTextureScaleMode`](https://wiki.libsdl.org/SDL3/SDL_SetTextureScaleMode), [`SDL_ScaleMode`](https://wiki.libsdl.org/SDL3/SDL_ScaleMode). Thus a client that needs deterministic nearest sampling for a specific bitmap must set it on that texture rather than depend on the renderer default.

No cited platform standard says “all pixel art must use nearest” or “all raster assets must use linear”. Filter choice is visual policy. Categorical data such as legacy chroma masks is different: interpolating mask colors before decoding them creates values that no longer equal the mask keys. Mask extraction must happen in source space, or masks must be represented as semantic/coverage channels before filtered color scaling. The current client independently demonstrates this constraint in its own pipeline (section 2.3).

## 2. Existing TomeNET contracts and current implementation

### 2.1 Already-decided single-window constraints

[Specify surface layouts and responsive rules](../issues/21-specify-surface-layouts-and-responsive-rules.md#answer) already fixes these product constraints:

- one `SDL_Window` and a supported minimum client area of 1024×768 logical units;
- OS DPI maps logical units to surface pixels;
- UI/text and map/cell scales are independent;
- a DPI/monitor change rerasterizes and relayouts without losing interaction state;
- the selected map is the complete 66×22 or 66×44 server viewport, uniformly fitted without crop or map scroll;
- opening a primary surface does not change the server viewport;
- special-store text/grid/raw-picture/animation geometry uses one uniform fit transform without scroll.

The research does not reopen those decisions. It exposes the remaining raster, rounding, asset and accessibility choices needed to implement them.

### 2.2 Current SDL3 DPI behavior is per legacy `Term`, not a reusable single-window model

The current SDL3 backend creates each legacy window with `SDL_WINDOW_HIGH_PIXEL_DENSITY` and draws through `SDL_GetWindowSurface` ([window creation](../../../src/client/main-sdl3.c#L321)). At initialization it derives a display content scale from the saved position, stores it per `term_data`, and passes it into the font loader ([scale selection](../../../src/client/main-sdl3.c#L3695), [term initialization](../../../src/client/main-sdl3.c#L3724)). On `SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED` it reloads the font; on ordinary resize it refreshes the window surface ([events](../../../src/client/main-sdl3.c#L1407)). It does not use `SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED` as the primary allocation event in this path.

TTF loading multiplies the configured size by the display scale before `TTF_OpenFont` ([TTF loader](../../../src/client/main-sdl3.c#L505)). PCF loading rounds `glyphWidth × scale` and `glyphHeight × scale` separately, then nearest-scales the bitmap ([PCF loader](../../../src/client/main-sdl3.c#L5428), [PCF scaling](../../../src/client/main-sdl3.c#L5817)). Cell metrics then drive legacy window size, text placement and tile size. This works as one legacy policy, but it couples OS DPI, font metrics, cell size, window extent and tile scaling. It does not express the accepted independent single-window UI/map scale contract.

The `-s` option can force scale 1.0 ([CLI parse and log](../../../src/client/client.c#L1519)); preserving that exact switch as a finished-client user policy has not been decided. An OS-DPI bypass is not an accessibility-safe default on high-density desktops.

### 2.3 Current tiles already separate mask sampling from color filtering

Current graphics preparation scales a tilesheet to the active font cell. The selected color filter can be nearest, linear or the project's own Lanczos implementation, but mask source data is always nearest-scaled before layer separation because exact mask colors would be destroyed by filtering. For a filtered mode, non-mask color pixels and raw pictures are produced separately and copied into the decoded base layer ([graphics preparation](../../../src/client/main-sdl3.c#L3508), [filter implementations/cache dispatch](../../../src/client/main-sdl3.c#L3059)). Raw-picture rectangles are transformed from original-sheet to scaled-sheet coordinates and may exceed one cell ([raw-picture geometry](../../../src/client/main-sdl3.c#L3440), [drawing](../../../src/client/main-sdl3.c#L2249)).

This establishes useful compatibility facts, not the new policy:

- a legacy tilesheet carries both appearance and categorical mask information;
- foreground/background layers and subtilesets must share the same final cell geometry;
- raw pictures need the same sheet transform as their coordinate metadata;
- changing filter currently rebuilds graphics and is a persisted user option ([live apply](../../../src/client/main-sdl3.c#L1921), [settings UI](../../../src/client/c-util.c#L14847)).

The new renderer may preserve that decoded structure without preserving the current CPU resampler or its pixels.

### 2.4 Custom fonts and tilesets are an existing capability

The current SDL3 font loader searches user storage before bundled storage for both PCF and TTF ([TTF path precedence](../../../src/client/main-sdl3.c#L516), [PCF path precedence](../../../src/client/main-sdl3.c#L637)). The font selector enumerates `.pcf` and `.ttf` assets, and a failed live font load retains the old font before any replacement ([font selection](../../../src/client/c-util.c#L13544), [transactional replacement](../../../src/client/main-sdl3.c#L4701)).

The tileset selector similarly scans user graphics before bundled graphics for size-prefixed `.bmp` names, supports subtilesets, and applies a selected tileset/filter at runtime ([discovery](../../../src/client/c-util.c#L14521), [load precedence](../../../src/client/main-sdl3.c#L4144)). Tile dimensions are currently parsed from the asset name, and associated `graphics-<name>.prf` mappings are part of the asset contract.

The user's confirmed requirement is to preserve the ability to use custom fonts and tilesets. “Preserve” therefore includes discover/select/load/fail-without-destroying-the-active-resource and the mapping/subtileset associations, not only accepting arbitrary bitmap bytes. Requiring density variants would narrow the current capability; assets with one source density need a defined scaling path and diagnostics instead.

No `.ttf` or `.otf` file is present under the current `lib` tree, although the loader supports TTF. The user has required at least one TTF path in the finished client but has explicitly not requested a special “licensable TTF” policy. Selecting or sourcing that file is outside this scaling research.

### 2.5 Server viewport dimensions are cells, not pixels

The legacy client sends `PKT_SCREEN_DIM` with `screen_wid` and `screen_hgt`, not physical sizes ([wire function](../../../src/client/nclient.c#L8072)). Current resize derives terminal columns/rows from pixel extent divided by font cell metrics and may consequently send new dimensions ([legacy resize](../../../src/client/main-sdl3.c#L1311), [main-Term update](../../../src/client/main-sdl3.c#L4860)). The single-window layout decision supersedes that coupling: selected 66×22/66×44 is semantic/session state, while the renderer fits those cells to available map space.

Therefore OS DPI changes, user UI scale, user map zoom, panel widths and ordinary window resize must not themselves invent a third server viewport size. They change the cell transform. A packet change belongs only to the already-defined normal/big layout transition and server capability handling.

## 3. Candidate scaling model — alternatives, not decisions

### 3.1 Coordinate stack

A coherent implementation can name the transforms rather than store an ambiguous global scale:

| Layer | Quantity | Candidate transform |
|---|---|---|
| OS/window | SDL event/window coordinates `W` | Platform-native; never assumed to be pixels or project logical units |
| Output | framebuffer/render pixels `P` | Query after pixel-size change; `D=P/W` |
| Project layout | logical units `L` | Candidate normalization `L=P/S`, where `S=SDL_GetWindowDisplayScale` |
| UI preference | UI/text scale `U` | Multiplies UI metrics in `L`; may include a separate accessibility input if adopted |
| Map preference | desired cell scale `M` | Sets desired map glyph/tile cell in `L`, independent from `U` |
| Fit | map/canvas fit `F≤1` | Uniform downscale needed to show the complete selected grid/canvas |
| Raster | final pixels | UI metric: `base×U×S`; map metric: `baseCell×M×F×S`, with one defined rounding strategy |

This model is compatible with the accepted layout, but the formula is still a candidate until issue 23 adopts it. In particular, the project must decide whether user “UI scale” changes all UI geometry or only text plus text-dependent controls, and how an OS accessibility text factor composes with it.

### 3.2 Three viable composition strategies

| Model | Strength | Cost/risk in this project | Decisions it does not remove |
|---|---|---|---|
| **A. Output-pixel rendering from logical layout** | Maximum sharpness; UI and map can use independent scales; direct correspondence between final raster and hit-testing transform | Requires disciplined float→pixel conversion, shared grid edges and density-aware cache invalidation | PCF/tile filters, rounding, accessibility composition |
| **B. Role-specific render targets** | UI, map and special canvas can each have an appropriate raster density and uniform internal transform | Target sizing and composition are more complex; accidental double scaling is possible | Target density, final sampling, cross-target alignment |
| **C. One fixed logical framebuffer stretched to the window** | Simplest resize/fullscreen path; SDL logical presentation supplies fitting | Scales already-rasterized text and couples UI/map density; independent scales need extra passes; a fixed low-resolution target can blur | Full-frame filter, high-resolution text path, independent map scale |

In model A, layout and hit testing remain floating-point `L`; each frame converts bounds to `P`, TTF is rerasterized for effective pixel size, and bitmap textures are sampled per asset. In model B, role targets compose into the final pixel output. In model C, SDL logical presentation provides the fixed framebuffer transform.

SDL supports all three building blocks. No source standard selects one.

### 3.3 Fractional cell geometry and rounding alternatives

A 66-column grid fitted into an arbitrary pixel width usually has a non-integer cell width. Three policies are materially different:

1. **Continuous grid transform:** retain floating cell edges and render every glyph/tile/effect through the same transform. Outer extent and alignment are exact, but individual raster columns may cover unequal pixel counts.
2. **Shared snapped edges:** compute all `N+1` grid edges from one origin and scale, snap edges, and derive each cell from adjacent edges. Cells differ by at most rounding distribution; no gaps/overlap accumulate. Glyph/tile content needs a rule for slightly differing destinations.
3. **Quantized cell size:** choose one integer pixel cell size and center/letterbox the whole map. Every cell is identical and bitmap assets can remain crisper, but the complete map uses less available space and changes physical size in steps.

Independently rounding every cell origin and width is not viable because it can create gaps, overlap or cumulative drift. Whatever policy is selected must be shared by terrain, actor, cursor, weather, targeting, hit testing and raw-picture anchors.

### 3.4 Bitmap PCF policy alternatives

PCF has a natural integer bitmap grid. Plausible policies are:

- nearest-scale it to an integer effective cell raster and accept stepwise sizes;
- nearest-scale into the snapped cell rectangles, accepting uneven duplicated rows/columns at fractional scale;
- allow a filtered PCF mode for smoother fractional fitting while retaining nearest as a user/profile choice;
- keep PCF map cells quantized while vector UI text remains continuous.

None preserves both perfect source pixels and arbitrary physical size. Since pixel-perfect acceptance has been removed, the actual decision should be driven by legibility, full-grid geometry, absence of clipping/bleed, and user control.

### 3.5 Tile, mask, subtile and raw-picture alternatives

For color tile content, nearest, linear and project-provided higher-quality resampling remain legitimate choices. A single global filter is not required; filter can be per asset class or a user setting. Atlas rendering must prevent sampling from adjacent tiles (decoded per-tile textures, padding/clamp, or equivalent source-bound handling), especially under linear filtering.

For legacy chroma masks, two safe families exist:

- decode categorical masks/layers at source resolution, then scale RGBA/coverage layers;
- scale the categorical mask with nearest and filtered color separately, as current SDL3 does.

Filtering the encoded mask sheet first is unsafe. Foreground, terrain, outline/coverage and effects must use the exact same destination cell transform even if their sampling filters differ.

Subtilesets can have different source density, but their declared logical tile size must normalize to the base grid. Raw pictures retain their declared multi-cell/source-sheet bounds and follow the one uniform special-canvas transform; they are not coerced to one cell. Assets without density variants remain usable by resampling the available source and reporting quality limitations rather than silently selecting another resource.

### 3.6 Cursor and effects

The user has already rejected both legacy cursor rasters and requested a new cursor design. Scaling policy can be decided before visual design only at the structural level:

- cursor/target/effect geometry is expressed in map logical/cell coordinates;
- it uses the same cell edges and clipping as the map;
- monitor/UI changes cannot leave stale pixels or move the semantic target;
- any fixed-width stroke needs a declared unit (logical UI unit, fraction of cell, or physical pixel) and minimum visibility rule.

Whether the new cursor is vector/procedural, nine-sliced or bitmap-backed remains a prototype/design decision.

### 3.7 Monitor-change and resize transaction

A candidate lifecycle consistent with SDL and the accepted interaction contract is:

1. receive window resize, pixel-size, display-scale or user-scale change;
2. query current `W`, `P`, `D`, `S` rather than trusting stale cached relationships;
3. recompute logical client extent and layout without changing semantic session state;
4. rebuild/rerasterize only density-dependent resources (TTF glyph caches, PCF/tile variants, render targets, cursor/effect resources);
5. atomically swap valid resources, retain the old active custom asset on load failure;
6. redraw the final composed frame and remap hit testing through the same transform.

Drafts, selection, focus, scroll position, macro/request state and selected normal/big viewport are model state and survive this transaction. Cache keys need source identity, effective raster size/density, filter, mask/profile metadata and renderer/dependency version sufficient to avoid reusing an incompatible variant; the exact cache design and budget remain outside this ticket.

## 4. Decisions now sharp enough for the raster ticket

The research does not answer these for the user. It makes them precise enough for [Choose raster references and defect compatibility](../issues/23-choose-raster-references-and-defect-compatibility.md):

1. **Logical coordinate definition:** adopt `logical extent = output pixels / SDL window display scale`, with input converted through pixel density, or use another explicitly cross-platform formula?
2. **Composition strategy:** output-pixel rendering, role-specific targets, or a fixed logical framebuffer with separate sharp-text/map passes?
3. **UI/accessibility composition:** is user UI scale whole-UI geometry or text-led geometry; what range is required; does Windows system text scale feed it, and what is the Linux fallback?
4. **Map size meaning:** is the user map setting a desired logical cell size that may be reduced by uniform fit, as the layout ticket implies; can it ever enlarge beyond native asset size?
5. **Fractional grid policy:** continuous transform, shared snapped edges, or quantized integer cells/letterboxing?
6. **PCF sampling:** always nearest/quantized, selectable filtering, or a split policy between map and UI roles?
7. **Tile sampling:** preserve the current user nearest/linear/Lanczos choice globally, scope it per asset class/profile, or simplify it; what fallback applies when a renderer lacks a requested mode?
8. **Mask representation:** source-space decoded layers/coverage or current-style nearest categorical masks plus separately filtered color?
9. **Density variants:** optional preference order when variants exist, while guaranteeing a defined resampling path for single-density custom fonts/tilesets?
10. **Custom asset failure contract:** exact validation/warnings and transactional fallback, without silent remapping, rejection merely for absent density variants, or loss of current user selection?
11. **TTF sizing:** scaled point size versus explicit target DPI; hinting/SDF policy; which one TTF exercises the supported path (asset selection is separate from licensing language)?
12. **Effects/strokes:** which measurements track UI scale, map cell scale, or physical pixels, including the new cursor design?
13. **Accessibility target:** adopt WCAG2ICT's informative 200% text-resize benchmark, only platform-provided range, or a project-specific range, and which inherently spatial surfaces are excepted?
14. **Lifecycle acceptance:** exact observable invariants on fractional scale, monitor move, resize, fullscreen transition and failed custom-resource reload, without any pixel-equality gate.

### Ownership guardrail

| Owner | What it owns after this research | What it must not silently decide |
|---|---|---|
| [Surface layouts](../issues/21-specify-surface-layouts-and-responsive-rules.md#answer) — resolved | 1024×768 logical minimum, wide/small composition, independent UI/map scale, complete normal/big map fitting, canvas/document layout rules | Sampling filters, pixel rounding, font raster/profile validation |
| [Raster compatibility](../issues/23-choose-raster-references-and-defect-compatibility.md) — active | Coordinate normalization adoption, composition model, fractional-grid/raster rules, filters/masks, custom-asset validation/fallback, cursor/effect units and structural raster acceptance | Which settings are defaults or where they are stored |
| [Persistence/UI schema](../issues/25-specify-persistence-ownership-and-ui-configuration-schema.md) | Setting names, defaults, bounds/steps once selected, startup/import precedence and storage ownership | Visual behavior of a scale/filter value or renderer transform |
| Acceptance owners | Scenario matrix and evidence for adopted behavior on Linux/Windows, monitor changes, fractional scale and custom assets | New product behavior not decided by the owning ticket |

Issue 30 itself owns only the source-backed comparison. Its resolution does not select among the alternatives.

## 5. Non-pixel-perfect acceptance consequences

Whichever alternatives are chosen, source facts support strict acceptance for:

- identical semantic glyph/tile IDs, palette roles and layer order;
- exactly 66×22 or 66×44 visible map cells for the selected mode;
- one common terrain/actor/cursor/effect grid with no gaps, overlap, drift or mismatched hit target;
- complete special-canvas fit and preserved raw-picture anchors/extents;
- no clipping, atlas bleed, mask-color leakage, stale output or lost interaction state;
- current-monitor scale applied after a monitor transition, with output resources matching the current pixel size;
- independent UI/text and map settings with no unintended server-dimension packet;
- custom PCF/TTF/tileset discovery, transactional activation and actionable failure diagnostics;
- structural legibility at all adopted UI/text/map scale limits.

Screenshots remain review/diagnostic evidence. No full-frame, primitive or pinned-environment pixel equality is required by this research.
