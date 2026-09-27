# Research PCF loading and scaling options

Type: research
Status: resolved
Assignee: codex-research
Blocked by: 22, 30, 33

## Question

Какие готовые возможности SDL3, SDL_ttf 3 и FreeType позволяют загружать и отображать PCF fonts, включая custom byte-glyph fonts этого репозитория? Отделить font decoding, glyph identity/charmap/default/range semantics, fixed bitmap strikes, rasterization и масштабирование готового bitmap. Проверить SDL_ttf поддержку bitmap fonts по официальным docs/source, actual size-selection semantics и доступность raw encoded IDs без guessed Unicode conversion.

Какие общепринятые варианты bitmap-font scaling подтверждаются первичными источниками: native/integer sampling, nearest при дробном масштабе, linear/coverage filtering, SDL PixelArt, выбор prepared bitmap sizes, outline/SDF alternatives? Указать ограничения сохранения мелких деталей при downscale, разницу native bitmap и vector glyphs, cost/lifecycle в принятом final-size prepared-cache pipeline. Не объявлять SDF или font conversion универсальным улучшением и не выбирать PCF policy за пользователя. Сохранить custom fonts, known origin/default fixes и отсутствие pixel-perfect требований.

Результат нужен для Q8.5 в [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md). Использовать первичные источники; runtime smoke при наличии локальных библиотек отделять от Windows/release coverage.

## Answer

[PCF loading and scaling options](../research/pcf-loading-and-scaling-options.md) separates PCF decoding, encoded IDs, font strikes and final bitmap sampling. SDL_ttf 3.2.2 actually opens representative PCF files through FreeType, including custom fonts without Unicode metadata; changing requested size selects a fixed strike rather than generating a resized glyph. Numeric per-glyph lookup works for the inspected implementation and six-font Linux smoke, but public Unicode API semantics and custom charmap/default handling require an explicit adapter; raw glyph index is not the encoded ID.

Nearest, Linear and supported texture PixelArt are viable PCF appearance options; decoded font coverage is distinct from categorical tile masks. CPU `SDL_ScaleSurface` silently maps PixelArt to Nearest in the inspected SDL 3.4 implementation. Native/whole-number bitmap sampling, fractional sampling, authored alternatives and SDF limitations are compared. The prepared final-size cache can support an independent PCF filter without adding repeated frame scaling.

Local SDL 3.4.16 / SDL_ttf 3.2.2 / FreeType 2.14.3 smoke covered `9x15`, `9x15tg`, `12x24`, `8x16`, `16x22`, `16x24tg`, including origin/default-relevant presence and non-Unicode maps; no Windows, release, arbitrary-font or visual-quality guarantee is inferred. Product PCF sampling/default remains a live decision in the parent raster ticket.
