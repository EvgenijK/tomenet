# Enumerate source text and server-field byte contracts

Type: research
Status: resolved
Assignee: codex
Blocked by: 04, 11, 22

## Question

Какой атомарный source/field inventory позволяет реализовать согласованное разделение Unicode UI, lossless legacy content, visual glyph mappings и исходящего byte contract без введения неявной общей кодировки?

Использовать [Encoding and the server contract](../research/encoding-server-contract.md), [Every input loop](../../../docs/research/single-window-input-loops.md), remaining/slash inventories и [Map every packet field to semantic state](11-map-every-packet-field-to-semantic-state.md). Для каждого text source и server-bound/local editor field перечислить producer/consumer, исходный byte/явно Unicode representation, markers/escaping, editor/wire/output capacities включая NUL, validation/allowed-byte rules и locale/signedness/build/version dependencies, case/normalization и transforms, persistence owner, observable reply/cancel/default/truncation/error outcomes. Показать delegated Lua/request-specific callbacks, file/resource sources и credential paths без секретных данных.

Отделить доказанную Unicode↔byte correspondence от неопределённой: отсутствие charset declaration не заполняется догадкой или metadata случайного font. Source facts не вводят repertoire extension или новые alphabet restrictions. Explicit visual-profile glyph correspondence — presentation contract, не network charset. Найти любые editor/wire capacity mismatches, signed-char filtering и byte slicing; определить baseline outputs и передать необходимые dispositions в input/encoding и acceptance decisions, не исправлять код.

Результат — source-backed registry с completeness evidence и конкретными остаточными вопросами. Не нужна runtime Unicode-support claim или реализация encoder. Storage/profile fields/defaults принадлежат persistence ticket; raster glyph profiles/oracles — raster ticket; fixtures/platform/runtime proof — acceptance.

## Answer

Исследование завершено 2026-09-18. [Source text and server-field byte contracts](../research/source-text-and-server-field-byte-contracts.md) содержит source/field registry, 176 lexical editor/parser entries, 114 полных client text packet calls, независимый cross-check server receivers и отдельные request/Lua/file/resource/credential boundaries. Counts включают явно отмеченные comments, declarations и build alternatives; это не число пользовательских capabilities. Exact source lines, byte capacities вместе с NUL, transforms, cancellation, persistence owners, ссылки на предшествующие inventories и source fingerprints задают проверяемую область охвата.

Подтверждены разные editor/storage/wire/server limits и отсутствие общей declared network charset. Account/character/group transforms не переносятся на opaque chat, passwords, inscriptions или Lua. Font/glyph correspondence остаётся presentation contract. Отчёт уточняет прежний bounded audit: packet slot capacity не гарантирует NUL при переполнении, а character-name path включает server Trim_name. Локальные поля, clipboard expansion, короткие receive buffers и credential protocol transforms имеют самостоятельные boundary cases.

Выявленные случаи требуют пользовательского решения в [Decide text-field boundaries and legacy defects](35-decide-text-field-boundaries-and-legacy-defects.md); исследование не выбирает truncate/reject policy и не меняет принятые input/encoding/storage решения. Это новый child ticket, блокирующий persistence schema и acceptance. Source-specific profiles/defaults остаются в [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md), а fixtures, platform/locale/build evidence и проверка полного flow — в [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md).

Полнота заявлена для перечисленных lexical boundaries с делегированием grammar/state/persistence прежним inventories. Не заявлены известная кодировка любых high bytes, конечный перечень динамических user/server scripts или доказанная безопасность всех промежуточных formatting buffers. Runtime tests и encoder/client implementation не выполнялись; проверены source findings, ссылки и fingerprints. Секретные пользовательские данные не читались.
