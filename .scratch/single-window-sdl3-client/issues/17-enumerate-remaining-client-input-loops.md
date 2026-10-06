# Enumerate the remaining client input loops

Type: research
Status: resolved
Assignee: codex
Blocked by: 10

## Question

Какие blocking reads, prompt primitives, modal transitions и platform/build-gated input loops находятся за пределами уже исследованных `c-birth.c`, `c-cmd.c`, `c-inven.c`, `c-store.c`, `c-util.c`, `c-xtra2.c`, `skills.c` и SDL3 audio menus — включая spell browsing/casting, targeting helpers, network initialization, Lua UI и platform setup — и как они дополняют атомарную таблицу input contexts без дублирования общих primitives?

## Answer

Полный отчёт: [Remaining client input loops](../research/remaining-client-input-loops.md). Это source audit текущего working tree с owner/site ledger, воспроизводимыми поисками, учётом ignored generated bindings и SHA-256 C/Lua sources; runtime parity не заявлена. Ссылки и captured source hashes проверены.

Предыдущую scoped inventory дополняют 14 самостоятельных contexts: legacy spell selection, mimic powers/immunity, school selection/browse, stances, melee/ranged technique selection, runecraft composition, breath preference, account character overview/reorder, server-file perusal и first-run bigmap/graphics choices. Общие editor/item/direction/confirmation/acknowledgement primitives не дублируются: packet-driven, Lua, shutdown и platform save-chat paths добавлены как delegated owners с собственными gates, state, intent и exit contracts. Target/look остаются у ранее исследованных owners; Lua target wrapper в `#if0` не создаёт активного loop.

Сохраняются точные различия отмены: request-key отвечает 0 на Esc, request-string — строкой с byte27, canceled item/spell request не отправляет обычный reply; request-abort действует на pending prompt. Lua extra wrapper возвращает TRUE при наличии callback и передаёт его результат в aux; false не отменяет cast. False item hook оставляет item_obj=-1, после чего cast send всё равно следует. Отмена direction прекращает cast. Filename cancellation при save-chat не проверяется caller, а некоторые name-child exits имеют отличающийся screen/flag cleanup. Эти факты требуют явного решения в [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md), не автоматического исправления или переноса дефекта.

Альтернативные SOUND_SDL audio owners сверены с прежними SDL3 contexts; supported font/resource/file outcomes не исключены вместе с OS window topology. Unsupported Amiga readers, file/socket reads, inactive Lua blocks и API exposure без самостоятельного shipped state machine классифицированы отдельно. Новых обязательных research тикетов не выявлено; slash grammar остаётся отдельным [Enumerate slash-command grammar and dispatch](18-enumerate-slash-command-grammar-and-dispatch.md).
