# Inventory bundled TTF candidates

Type: research
Status: resolved
Assignee: codex-research
Blocked by: 22, 30

## Question

Какие конкретные monospaced TTF assets являются технически пригодными кандидатами для поставки хотя бы одного штатного TTF с single-window client на Linux amd64 и Windows MinGW32?

Для каждого кандидата установить по первичным источникам происхождение/version, доступность стабильного font binary, условия распространения для package inclusion, glyph coverage, fixed-width/metrics behavior, SDL_ttf compatibility и возможность построить explicit legacy-ID correspondence profile без объявления universal network charset. Разделить доказанное Unicode correspondence, отсутствующие legacy symbols и допустимые procedural/fallback roles; не выбирать product default за пользователя и не вводить pixel-perfect contract.

## Answer

Исследование и decision-ready матрица сохранены в [Bundled TTF candidate inventory](../research/bundled-ttf-candidates.md). Product default не выбран и universal network charset не введён.

Проверены точные static Regular binaries из официальных releases, с зафиксированными путями, размерами и SHA-256:

- Cascadia Mono Regular v2407.24 и JetBrains Mono NL Regular v2.304 открываются SDL_ttf 3.2.2 и сообщают fixed-width; оба пригодны для следующего profile/Windows acceptance, причём Cascadia покрывает больше проверенных legacy-style symbols, а JetBrains NL имеет меньший binary и не содержит ligatures.
- Noto Sans Mono Regular v2.014 имеет наиболее широкое проверенное Latin/Greek/Cyrillic coverage, но SDL_ttf сообщает `fixed-width=false`; он пригоден для text/UI, а для map cells требует явной renderer-owned cell-metrics проверки.

Все три распространяются под SIL OFL 1.1 и допускают включение неизменённого font binary в software package при включённом copyright/license. Для каждого release в отчёте даны первичные upstream sources и точный binary asset.

Ни один Unicode `cmap` не определяет значение TomeNET bytes/IDs `0x00…0xFF`. Shipped visual profile перечисляет только доказанные ID→Unicode correspondences; unknown mapping не угадывается, known missing glyph получает видимый one-cell fallback с сохранённым original ID, а solid fill, tiles и formatting markers сохраняют procedural/non-font roles. Bundled TTF не отменяет custom PCF/TTF discovery и не обязан одновременно быть text-font, map-font или product default.

Runtime smoke выполнен на Linux amd64; packaged Windows MinGW32 build обязан повторить open/metrics/glyph checks. Pixel equality не требуется.
