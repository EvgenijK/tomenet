# Inventory formatted and server-driven surfaces

Type: research
Status: resolved
Assignee: codex
Blocked by: 01

## Question

Какие `SPECIAL_FILE_*`, normal/wide/special store rows и actions, guide/help/lore экраны, NPC/server prompts и прочие форматированные поверхности существуют, какие из них допускают semantic parsing и где завершённый клиент обязан сохранять permanent lossless raw representation?

## Answer

Инвентаризация разделяет поверхности по устойчивости их wire/source contract. `PKT_STORE`/`PKT_STORE_WIDE`, `PKT_STORE_INFO`, `PKT_BACT` и `PKT_REQUEST_*` уже передают типизированные поля: они становятся semantic state и typed intents, сохраняя все decoded fields, sentinels, неизвестные IDs/flag bits и исходные formatted strings. Все 18 значений `SPECIAL_FILE_*` (`NONE`–`EXTRAINFO`) перечислены; `EXTRAINFO` остаётся dormant, а ни одна активная категория при текущем протоколе не допускает замены raw-представления semantic parsing — только дополнительную versioned projection.

`PKT_SPECIAL_LINE` является formatted-document protocol, а `PKT_STORE_SPECIAL_{STR,CHAR,CLR,ANIM}` — координатным presentation protocol. Поэтому finished client постоянно, в смысле архитектуры и lifetime активной поверхности, хранит lossless document/canvas state: исходные bytes, attrs/color codes, logical/physical coordinates, page markers, arrival order, position changes и ordered draw/clear/animation operations вместе с восстанавливаемым attributed canvas. Неизвестные title/markup/opcode остаются отображаемыми; terminal cells не являются источником истины.

Последующее нормативное уточнение retention, checkpoint/unknown-operation semantics и восстановления transient store animation: [Define bounded working retention and optional capture](26-define-bounded-working-retention-and-optional-capture.md#answer). Его animation exception имеет приоритет над любым прочтением этого inventory как требования сохранять/replay весь ход анимации; общий operation archive этим inventory не установлен. Неизвестный animation opcode не получает выдуманного визуального эффекта.

Локальные Guide, spoilers и notes сохраняют точные source lines и строят поверх них spans/indexes. Artifact/Monster Lore могут быть полностью semantic, поскольку собираются из typed setup/data, но их formatted/chat-paste projections остаются compatibility-tested. Остальные formatted streams (`PKT_MESSAGE`, `PKT_TARGET_INFO`, decorated floor names, setup MOTD) подчиняются тому же правилу: typed classification добавляется, но точный текст и ordering не теряются.

Полный отчёт с category/store/request/surface tables, lifecycle и acceptance matrix: [Formatted and server-driven surfaces inventory](../research/formatted-and-server-driven-surfaces.md).

Нового тикета или graduation fog не требуется: последствия уже принадлежат Choose the presentation-state boundary, Define the single-window interaction model, Preserve input and macro semantics, Set the compatible protocol-extension policy, Specify renderer parity и Design parity evidence and acceptance.
