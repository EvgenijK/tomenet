# Bundled TTF candidate inventory

## Scope and result

This note answers [Inventory bundled TTF candidates](../issues/32-inventory-bundled-ttf-candidates.md). It inventories concrete, versioned assets that can be shipped with the single-window client; it does **not** choose the product default, define a network charset, or introduce pixel-perfect acceptance.

Three official release binaries are decision-ready candidates:

- **Cascadia Mono Regular 2407.24** and **JetBrains Mono NL Regular 2.304** are the lower-risk grid candidates: both open successfully and report fixed-width through SDL_ttf.
- **Noto Sans Mono Regular 2.014** has the broadest Latin/Greek/Cyrillic Unicode coverage of the three, but SDL_ttf 3.2.2 reports it as not fixed-width. It remains a valid text/UI candidate, or a map candidate only if the new renderer validates and owns cell advances instead of relying on the font's fixed-width flag.

No candidate's Unicode `cmap` proves a universal correspondence for TomeNET bytes `0x00…0xFF`. Every bundled TTF still needs an explicit legacy-ID profile. The profile can map only established IDs to Unicode scalars present in that exact binary; solid cells and unavailable symbols keep procedural/visible-fallback roles.

## Existing project and SDL_ttf constraints

The current repository contains no `.ttf` or `.otf` below `lib`, although it already discovers TTF files in user storage before game storage ([font discovery](../../../src/client/c-util.c#L13544)) and loads them by filename plus point size ([loader](../../../src/client/main-sdl3.c#L505)). The default remains `9x15` PCF ([configuration](../../../src/config.h#L869)). Shipping one TTF therefore adds a deterministic built-in asset without removing custom-font selection.

The existing TTF path:

- calls `TTF_OpenFont` and queries `TTF_FontIsFixedWidth` ([loader and metrics](../../../src/client/main-sdl3.c#L528));
- falls back to per-cell monospace emulation when the font does not report fixed-width ([fallback](../../../src/client/main-sdl3.c#L583));
- passes text to `TTF_RenderText_LCD`, whose official contract is UTF-8 ([current call](../../../src/client/main-sdl3.c#L852), [SDL documentation](https://wiki.libsdl.org/SDL3_ttf/TTF_RenderText_LCD)).

SDL_ttf officially opens a font file at a floating-point point size ([`TTF_OpenFont`](https://wiki.libsdl.org/SDL3_ttf/TTF_OpenFont)), can query fixed-width status ([`TTF_FontIsFixedWidth`](https://wiki.libsdl.org/SDL3_ttf/TTF_FontIsFixedWidth)), and can render one Unicode codepoint separately from a UTF-8 text run ([SDL_ttf quick reference](https://wiki.libsdl.org/SDL3_ttf/QuickReference)). SDL_ttf is a FreeType/HarfBuzz wrapper ([official repository](https://github.com/libsdl-org/SDL_ttf)). These APIs support the already-decided split: Unicode UI text may use UTF-8 runs, while legacy glyph IDs first pass through an explicit ID→Unicode/profile lookup and can be rendered per cell.

The finished client's metric contract should not repeat one current weakness: a false `TTF_FontIsFixedWidth` result turns emulation on, but the current branch does not establish an independent cell width before drawing. A bundled candidate must therefore either report fixed-width or pass a new explicit advance/cell-width validation.

## Method

Only first-party project pages, repositories, release archives, licenses, SDL documentation, and local source were used. The three exact static Regular TTF files were extracted from official releases and exercised on Linux amd64 with the repository host's SDL_ttf 3.2.2:

1. initialize SDL_ttf;
2. open the exact binary at 16 pt;
3. query family/style, height, ascent, descent, line skip and fixed-width status;
4. call `TTF_FontHasGlyph` over Unicode scalar values and selected structural blocks;
5. record size and SHA-256.

This is a technical inventory, not final acceptance. Windows MinGW32 must repeat the same open/metrics/glyph smoke test with the packaged SDL_ttf/FreeType build; no Windows runtime was available in this research. Full profile coverage also waits for the approved legacy-ID correspondence table. Raster pixels were not compared.

## Candidate matrix

| Candidate asset | Stable first-party binary | SDL_ttf 3.2.2 metrics at 16 pt | Verified Unicode coverage in exact binary | Distribution conditions | Decision disposition |
|---|---|---|---|---|---|
| **`CascadiaMono-Regular.ttf`**, release **v2407.24** | Official [release](https://github.com/microsoft/cascadia-code/releases/tag/v2407.24), path `ttf/static/CascadiaMono-Regular.ttf`; 575,912 bytes; SHA-256 `06520d032ec274fa5040b22c6f4a1d829081b24ba40b2da56dae89bf10c7b481`. Microsoft's README defines Cascadia Mono as the no-ligature variant and supplies static TTFs ([variants and formats](https://github.com/microsoft/cascadia-code/tree/v2407.24#font-variants)). | Opens; family `Cascadia Mono`, style `Regular`; **fixed-width true**; height 19, ascent 15, descent −3, line skip 19. | 2,426 scalars; ASCII 95/95, Latin-1 graphics 96/96, Greek block 77/144, Cyrillic block 126/256, box drawing 128/128, block elements 32/32, Unicode control pictures 39/39. It also contains all 31 tested classic ROM/CP437 control-symbol shapes. Its `cmap` includes Symbols for Legacy Computing, consistent with the project's [FONTLOG](https://github.com/microsoft/cascadia-code/blob/v2407.24/FONTLOG.txt). | [SIL OFL 1.1](https://github.com/microsoft/cascadia-code/blob/v2407.24/LICENSE), with reserved font name `Cascadia Code`; unmodified font may be bundled with software if copyright and license accompany it. | **Eligible for text or map-profile evaluation.** Broadest tested legacy-symbol coverage and correct fixed-width signal; largest release archive, but only the one static TTF and license need enter the client package. Not selected as default here. |
| **`JetBrainsMonoNL-Regular.ttf`**, release **v2.304** | Official [release](https://github.com/JetBrains/JetBrainsMono/releases/tag/v2.304), path `fonts/ttf/JetBrainsMonoNL-Regular.ttf`; 208,576 bytes; SHA-256 `fb3b2575d7b0657359707993288f12a7360344d39387bb26050e276d61f6bd2a`. The official README identifies `NL` as the variant without ligatures ([repository README](https://github.com/JetBrains/JetBrainsMono/tree/v2.304)). | Opens; family `JetBrains Mono NL`, style `Regular`; **fixed-width true**; height 22, ascent 17, descent −4, line skip 22. | 1,363 scalars; ASCII 95/95, Latin-1 graphics 96/96, Greek block 79/144, Cyrillic block 122/256, box drawing 128/128, block elements 32/32, Unicode control pictures 36/39; 15/31 tested classic ROM/CP437 control-symbol shapes. The project's official [supported-symbol list](https://github.com/JetBrains/JetBrainsMono/wiki/List-of-supported-symbols) separately documents box, block and control symbols. | [SIL OFL 1.1](https://github.com/JetBrains/JetBrainsMono/blob/v2.304/OFL.txt); unmodified font may be bundled with software if copyright and license accompany it. | **Eligible for text or a narrower map profile.** Smallest binary, explicit no-ligature face, correct fixed-width signal. Missing profile symbols must use declared replacements/procedural roles, never guessed remapping. Not selected as default here. |
| **`NotoSansMono-Regular.ttf`**, release **NotoSansMono-v2.014**, hinted static TTF | Official [release](https://github.com/notofonts/latin-greek-cyrillic/releases/tag/NotoSansMono-v2.014), path `NotoSansMono/hinted/ttf/NotoSansMono-Regular.ttf`; 596,428 bytes; SHA-256 `65b5e2b2c4a1fba9ae8be1f026cb35b03dcb8886d9b2a4147054fde12f7e767d`. Google Fonts metadata points to that same repository, commit and archive ([metadata](https://github.com/google/fonts/blob/main/ofl/notosansmono/METADATA.pb#L1915-L1943)). | Opens; family `Noto Sans Mono`, style `Regular`; **fixed-width false** as reported by SDL_ttf; height 22, ascent 18, descent −4, line skip 22. The false flag does not contradict the design name, but it triggers the current emulation branch and is a concrete integration caveat. | 3,489 scalars observed; ASCII 95/95, Latin-1 graphics 95/96 (soft hyphen absent), Greek block 121/144, Cyrillic block 256/256, box drawing 128/128, block elements 32/32, Unicode control pictures 0/39; 18/31 tested classic ROM/CP437 control-symbol shapes. Official Noto documentation describes the family as monospaced and reports broad Latin/Greek/Cyrillic and symbol coverage ([specimen](https://github.com/notofonts/noto-docs/blob/main/docs/specimen/NotoSansMono.md)). | [SIL OFL 1.1](https://github.com/notofonts/latin-greek-cyrillic/blob/NotoSansMono-v2.014/OFL.txt); unmodified font may be bundled with software if copyright and license accompany it. | **Eligible for text/UI; conditional for map cells.** Best tested Cyrillic/Greek breadth, but requires renderer-owned cell metrics and full profile validation. It should not be the sole low-risk proof of the fixed-grid TTF path. Not selected as default here. |

The numeric coverage counts above are observations of the exact hashed binaries through `TTF_FontHasGlyph`, not claims that every codepoint is suitable for a TomeNET role. Combining marks, wide-script behavior, shaping, glyph aesthetics and legibility are separate concerns.

## Legacy-ID correspondence contract

The bundled TTF does not define the meaning of network bytes or game glyph IDs. Candidate evaluation must apply these rules:

1. **Proven correspondence only.** Printable ASCII `0x20…0x7E` can map to the identical Unicode scalars in a shipped visual profile. Any high/control ID requires an explicit profile entry backed by the game's approved source/field or visual mapping; neither Latin-1 coverage nor CP437-like art authorizes a universal byte charset.
2. **Identity remains numeric.** Store the original legacy ID and attribute independently of its selected Unicode scalar. Font switching may change the visual profile, not the received ID or outgoing bytes.
3. **Missing mapping differs from missing glyph.** An unknown ID→Unicode correspondence is not guessed. A known Unicode scalar absent from the chosen font uses the agreed visible one-cell replacement while retaining the original ID for diagnostics/acceptance.
4. **Procedural and non-font roles stay separate.** Solid-wall/fill cells can be procedural; tile IDs continue through tileset lookup; formatting introducers and state markers are parsed semantics, not Unicode glyph requests. These paths do not need counterfeit Unicode assignments merely to make a TTF appear complete.
5. **Coverage is profile-specific.** Before packaging, generate a report for all entries in the chosen TTF profile: present Unicode scalar, procedural role, tile role, or explicit visible fallback. A count of 256 outputs is insufficient unless every disposition is named.
6. **Custom fonts remain supported.** The bundled file provides a deterministic tested path. User TTFs and PCFs retain discovery/selection/load-failure behavior; a custom TTF uses an explicit compatible profile or reports missing mappings instead of silently changing the game's charset.

The tested classic ROM/CP437 symbols are therefore only a useful coverage probe. They are **not** a proposal to interpret TomeNET data as CP437.

## Selection gates for the eventual packaging decision

Any of the candidates may be selected without making it the product default. The selection decision should require:

- one exact static TTF binary, filename, version, byte size and SHA-256 pinned in packaging;
- its upstream OFL/copyright file included in Linux and Windows archives;
- successful `TTF_OpenFont`, metrics and glyph-presence smoke tests in packaged Linux amd64 and Windows MinGW32 builds;
- fixed-width true **or** an explicit, tested renderer-owned cell-advance policy;
- full disposition of the approved legacy-ID profile, including missing mappings and procedural roles;
- text-role and map-role tests kept independent, because shipping a TTF does not require selecting it for both;
- structural acceptance (identity, one-cell geometry, clipping, readable fallback and lifecycle), with no pixel-perfect promise.

On current evidence, **Cascadia Mono Regular** and **JetBrains Mono NL Regular** can proceed directly to profile/Windows acceptance; **Noto Sans Mono Regular** proceeds directly for Unicode text/UI but needs the additional cell-metrics gate for map use. This is a technical shortlist, not a choice of product default.

## Evidence limitations

- Runtime probing was Linux amd64 only, using SDL_ttf 3.2.2; Windows MinGW32 compatibility remains an explicit packaging test.
- Only the named static Regular binary from each official archive was tested. Variable, italic, bold, Powerline and Nerd Font variants are not implied substitutes.
- No visual-quality ranking, minimum readable point size, scaling policy or raster comparison was performed.
- No universal mapping for high legacy bytes was inferred. The source-backed profile is still the authoritative acceptance input.
