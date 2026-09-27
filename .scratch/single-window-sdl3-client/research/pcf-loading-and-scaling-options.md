# PCF loading and scaling options

Research for [Research PCF loading and scaling options](../issues/34-research-pcf-loading-and-scaling-options.md), 2026-09-18. This resolves factual questions, not the user-owned PCF sampling policy in [Choose raster references and defect compatibility](../issues/23-choose-raster-references-and-defect-compatibility.md).

## Findings

PCF is a font containing already-drawn bitmap glyphs. SDL3 can scale the decoded images; SDL_ttf 3 can load PCF through FreeType despite its name. Changing the requested point size does not turn a PCF into a vector font. The local smoke below demonstrates actual loading, byte-ID lookup and unchanged native dimensions for six representative repository fonts. It does not establish Windows packaging or arbitrary custom-font compatibility.

### Loading, sizes and IDs are separate concerns

- SDL_ttf 3.2.2 opens the file through FreeType, selects a charmap and uses `FT_Get_Char_Index` for individual character lookup. Its non-scalable branch treats requested size as a clamped bitmap-strike index, then calls `FT_Select_Size`. `TTF_GetGlyphImage` rejects absent characters; `TTF_GetGlyphImageForIndex` addresses glyph slots directly. [SDL_ttf release source](https://github.com/libsdl-org/SDL_ttf/blob/release-3.2.2/src/SDL_ttf.c#L1936), [size selection](https://github.com/libsdl-org/SDL_ttf/blob/release-3.2.2/src/SDL_ttf.c#L5174), [glyph lookup](https://github.com/libsdl-org/SDL_ttf/blob/release-3.2.2/src/SDL_ttf.c#L2880).
- FreeType explicitly distinguishes selecting an existing bitmap strike from requesting outline scaling. `FT_Select_Size(face, 0)` selects the native PCF strike; requesting a different arbitrary pixel height is not bitmap resampling. [FreeType sizing](https://freetype.org/freetype2/docs/reference/ft2-sizing_and_scaling.html#ft_select_size).
- PCF charmap keys are encoded character IDs. FreeType's PCF driver checks row/column bounds and subtracts their origins. It marks suitable ISO encodings as Unicode but preserves other maps as `FT_ENCODING_NONE`. Its glyph bitmap is monochrome, with bearings and advance independent of the ink rectangle. [FreeType PCF driver](https://github.com/freetype/freetype/blob/VER-2-13-3/src/pcf/pcfdrivr.c#L93), [character mapping API](https://freetype.org/freetype2/docs/reference/ft2-character_mapping.html).
- FreeType inserts glyph slot zero for the PCF default character, so its glyph indices differ from raw file bitmap indices. The encoded default is resolved through the encoding table, with driver fallback for malformed defaults. A missing encoded ID returns index zero; presence and fallback must remain distinguishable. [PCF encoding/default reader](https://github.com/freetype/freetype/blob/VER-2-13-3/src/pcf/pcfread.c#L963).

For this project, **encoded ID 65, Unicode U+0041 and internal glyph index 65 are different concepts**. Do not convert a map byte through a guessed charset or pass it directly as a glyph index. `TTF_GetGlyphImage(font, id, ...)` reaches raw PCF encoding lookup in the inspected implementation, including custom non-Unicode PCF maps tested below. However, its public API is documented as Unicode. A PCF adapter using that path needs explicit regression coverage and version/dependency validation; direct FreeType gives explicit charmap selection and font metadata access. `TTF_GetGlyphImageForIndex` is an alternative only after resolving the real index; it does not perform PCF encoding lookup itself. [Glyph image API](https://wiki.libsdl.org/SDL3_ttf/TTF_GetGlyphImage), [glyph-index API](https://wiki.libsdl.org/SDL3_ttf/TTF_GetGlyphImageForIndex), [BDF/PCF metadata API](https://freetype.org/freetype2/docs/reference/ft2-bdf_fonts.html).

A UTF-8 text-layout call is not the map-byte path: NUL/control IDs, shaping and arbitrary custom glyph meanings require explicit individual glyph handling. Existing accepted rules still apply: correct PCF origin/range/default, preserved original ID, procedural IDs handled by their own dispatch, and visible one-cell fallback after the valid font default. Loader choice must not silently alter those contracts.

### Current repository path

The current SDL3 client has its own `PCF_OpenFont`/`PCF_RenderText` implementation rather than routing PCF through SDL_ttf. It decodes a horizontal white/transparent atlas, scales that atlas once with `SDL_ScaleSurface(..., SDL_SCALEMODE_NEAREST)` when display scale differs, and draws character cells from it. The code itself contains a TODO about allowing the graphics resize filter. [Loader](../../../src/client/main-sdl3.c:5428), [scaling](../../../src/client/main-sdl3.c:5824), [lookup](../../../src/client/main-sdl3.c:5905).

The same source reads but does not preserve origin for lookup, substitutes `default_char` as a bitmap index, and indexes the table directly by byte. Reusing that parser unchanged would carry known defects already rejected by [Define encoding and glyph identity](../issues/22-define-encoding-and-glyph-identity.md). Replacing the decoder with FreeType is a viable way to avoid maintaining PCF format mechanics; it does not by itself select the visual filter or prove the complete compatibility contract.

### Bitmap scaling choices

SDL's own migration guidance describes nearest as sharp/blocky and linear as smoother/blurrier, with the choice depending on the content. Its scale-mode enum also supplies PixelArt from SDL 3.4.0, intended to retain pixel-art edges. [SDL migration guidance](https://wiki.libsdl.org/SDL3/SDL12MigrationGuide), [SDL scale modes](https://wiki.libsdl.org/SDL3/SDL_ScaleMode).

The following quality consequences are engineering inferences from those algorithms and the finite PCF source bitmap, not guarantees of readability for every glyph:

| Choice | Expected result and limitation |
| --- | --- |
| Native bitmap size | Shows the authored grid without resampling; may not fit the chosen map area. |
| Whole-number enlargement with Nearest | Replicates each source pixel uniformly; crisp, visibly block-shaped. |
| Fractional Nearest | Fits the final cell, remains sharp; different source strokes can occupy different numbers of output pixels. Downscaling can remove thin strokes. |
| Linear on decoded glyph coverage | Softens pixel boundaries; can look less jagged, but small glyphs can blur and thin strokes can weaken. |
| PixelArt texture sampling | A ready SDL option worth comparing for enlargement; it does not invent font outlines or guarantee small-size legibility. Backend fallback applies. |
| Authored font at a nearer native size | Avoids scaling a very small bitmap too far. Requires a genuinely equivalent authored glyph family/profile, not guessing equivalence from filenames. |

The previously accepted rule about nearest for tile mask categories does not settle PCF sampling. An encoded tile mask contains role labels that must be decoded before filtering. A decoded PCF glyph is already foreground **coverage**: smooth alpha values are a meaningful representation if the user chooses a softer appearance. White RGB with varying alpha can be tinted at draw time without rebuilding for palette changes. It must be filtered per isolated glyph/cell to avoid neighbouring glyph bleed.

### CPU surface scaling and renderer sampling differ

`SDL_ScaleSurface` takes a scale-mode parameter, but SDL 3.4.0's surface blitter explicitly replaces `SDL_SCALEMODE_PIXELART` with Nearest. Indexed surfaces likewise get forced to Nearest by `SDL_ScaleSurface`; convert coverage to an RGBA representation before requesting linear filtering. Therefore success from a surface-scale call does **not** prove the PixelArt algorithm ran. [Surface scaling implementation](https://github.com/libsdl-org/SDL/blob/release-3.4.0/src/video/SDL_surface.c#L1066), [indexed conversion/scaling](https://github.com/libsdl-org/SDL/blob/release-3.4.0/src/video/SDL_surface.c#L2072).

Use a supported renderer texture path for actual PixelArt preparation: source texture → exact final-size target → 1:1 draws. SDL explicitly allows backends to substitute an available scale mode. The existing visible-fallback policy must cover preparation as well as display; enum availability alone is insufficient. [Texture scale-mode API](https://wiki.libsdl.org/SDL3/SDL_SetTextureScaleMode).

This fits the accepted prepared-cache model: decode native glyphs once, prepare each required glyph for the final cell size/filter when dimensions change, then draw 1:1. Cache identity includes source generation, glyph/strike, final size and effective sampling mode; ordinary colour changes can use tint. This is an architectural inference from the accepted plan, not a benchmark claim.

### Vector conversion and SDF

A vector font supplies outlines for rasterization at the target size; PCF contains an authored pixel grid. Automatic conversion cannot recover information absent from that grid. It also risks changing the visual meaning of custom game glyphs. An equivalent vector family is therefore an optional asset choice, not a transparent PCF replacement. [FreeType font tutorial](https://freetype.org/freetype2/docs/tutorial/step1.html).

FreeType has both outline SDF and bitmap SDF (`bsdf`) generators. Its documentation warns about poor results with very small glyphs because the distance-field grid is coarse. Thus SDF is technically possible for bitmap input but is not a universal quality upgrade for 9×15 game glyphs. SDL_ttf has an SDF option; consuming distance data correctly is a further rendering concern, not simply changing a scale-mode enum. No SDF runtime claim is made here. [FreeType SDF properties](https://freetype.org/freetype2/docs/reference/ft2-properties.html#spread), [SDL_ttf SDF](https://wiki.libsdl.org/SDL3_ttf/TTF_SetFontSDF).

## Local runtime evidence

Linux smoke used installed SDL **3.4.16**, SDL_ttf **3.2.2**, FreeType **2.14.3**. This differs from the older SDL 3.4.10 recorded in previous research; it does not change the release pin automatically. No window or renderer was created. Temporary harness: `/tmp/pcf-options-smoke.c`, built with `cc -Wall -Wextra` and `pkg-config --cflags --libs sdl3 sdl3-ttf freetype2`.

For each font, it opened the same file using FreeType and SDL_ttf at requested sizes 15 and 30, enumerated numeric IDs 0…255, compared presence, and checked decoded native glyph coverage. Both APIs use FreeType underneath: this tests wrapper lookup/decoding consistency, not two independent font engines. The bitmap comparison is bounded research evidence, **not a new pixel-perfect product or acceptance requirement**.

| PCF | Native cell | Present IDs in 0…255 | Char map | Height at request 15 / 30 |
| --- | --- | --- | --- | --- |
| `9x15.pcf` | 9×15 | 223 | ISO8859/1, Unicode-tagged | 15 / 15 |
| `9x15tg.pcf` | 9×15 | 256 | No declared charset, `FT_ENCODING_NONE` | 15 / 15 |
| `12x24.pcf` | 12×24 | 221 | ISO8859/1, origin 1 | 24 / 24 |
| `8x16.pcf` | 8×16 | 221 | ISO8859/1, origin 1 | 16 / 16 |
| `16x22.pcf` | 16×22 | 128 | No declared charset, partial range | 22 / 22 |
| `16x24tg.pcf` | 16×24 | 256 | No declared charset | 24 / 24 |

All six loaded successfully with one non-scalable strike. Presence and native coverage mismatches were zero; all decoded alpha values were 0 or 255. `9x15tg` IDs 0/1/32/127/255 mapped to glyph indices 1/2/33/128/256, demonstrating why ID must not be confused with index. Origin-1 fonts correctly reported ID 0 absent. `16x22` correctly reported ID 255 absent. Requests 15 and 30 did not resize any font.

`SDL_ScaleSurface` returned success for Nearest, Linear and PixelArt on a 2×2 RGBA source enlarged to 5×5. Given the inspected source fallback, that last success cannot establish genuine PixelArt. GPU quality, performance during resize, complete bundled corpus, malformed assets and Windows dependency closure remain future acceptance work.

## Decision-ready options

Keeping Nearest as the PCF default is conservative for the authored grid, but there is no SDL constraint requiring it exclusively. A separate PCF choice among Nearest / Linear / available PixelArt is technically feasible within the already accepted prepared-cache pipeline. PCF and tile filters can remain independent. Native-size alternatives and vector fonts can be explicit user choices; they should not replace a selected custom font silently.

The unresolved product question is whether PCF remains Nearest-only or exposes a selectable filter (and which default). This report does not choose that answer or close the parent raster decision.
