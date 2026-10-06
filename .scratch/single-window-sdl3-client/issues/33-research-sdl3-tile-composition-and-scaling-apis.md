# Research SDL3 tile composition and scaling APIs

Type: research
Status: resolved
Assignee: codex-research
Blocked by: 13, 22, 30

## Question

Как точно устроены current X11/SDL3 preparation и composition для single-mask/two-mask tiles, terrain/foreground attrs, mask colors, outline, subtilesets, palette, filters, caches и mixed glyph paths; какие semantic stages новый renderer обязан сохранить, даже если implementation меняется?

Какие готовые актуальные SDL3 APIs и algorithms позволяют масштабировать и компоновать исходные textures непосредственно в final cell/output target без двойного scaling: SDL_Renderer scale modes, pixel-art sampling, render geometry, blend modes/targets, SDL_GPU samplers либо другие first-party SDL3 facilities? Какие режимы реально предоставляют nearest, linear, pixel-art, mipmaps или higher-quality downsampling; чего SDL3 не предоставляет (например, Lanczos), какие version/backend gates и fallback contracts следуют из официальной документации/source?

Исследование должно опираться на current repo source и первичные SDL3 sources/docs, разделить обязательную legacy semantics и сменяемую implementation technique, сравнить viable single-pass pipelines и сформулировать decision-ready вопросы для [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md). Не выбирать policy за пользователя, не вводить pixel-perfect acceptance и не переносить custom legacy algorithm только ради parity.

## Answer

[SDL3 tile composition and scaling APIs](../research/sdl3-tile-composition-and-scaling-apis.md) records the exact current single-mask/two-mask pipeline and the ready SDL3 alternatives without selecting product policy.

The compatibility unit is a logical cell with independent background and foreground attr/visual identities, not a flattened bitmap. Encoded mask colors are first decoded into authored-color, background-hole, foreground-coverage and optional outline roles; current attr colors and per-ID subtilesets are then resolved; background/terrain stages are composed before foreground/object stages. Mixed glyph/tile dispatch, retained background semantics, raw pictures, generated/authored outlines and palette/cache invalidation are included. X11 bitwise operations and the current SDL3 CPU resize/cache representation are implementation techniques rather than required raster behavior.

Double scaling can be removed while retaining that order: original decoded layers can be sampled directly into the final output-cell rectangle, using several ordered draws where necessary. `SDL_Renderer` supplies direct float source-to-destination rendering, geometry, tint, alpha blending, final-size render targets and `Nearest / Linear / Pixel art`; raw `SDL_GPU` adds explicit nearest/linear min/mag filters and mipmaps. SDL3 supplies no Lanczos mode. The report compares decoded-layer Renderer, isolated final-size cache, raw-GPU/mipmap and encoded-mask shader pipelines, identifies atlas-boundary and backend-fallback obligations, and ends with ten policy questions plus structural acceptance invariants without pixel equality.
