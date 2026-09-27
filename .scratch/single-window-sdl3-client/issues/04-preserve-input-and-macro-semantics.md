# Preserve input and macro semantics

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 01, 02, 10, 17, 18

## Question

Применить [Slash-command grammar and dispatch](../research/slash-command-grammar-and-dispatch.md): согласовать сохранение смешанных case/exact/prefix recognizers, whitespace и accepted suffixes; порядок editor history → channel decoration → screenshot pre-pass → colour/item substitutions → local dispatch/server fallback. Определить text-command и direct typed-action contracts, включая различия direct automation toggle и generic option hooks/packets, newest-item selection, wager default, mass autoinscriptions, Lua, screenshot capture и open/convert-last. Явно решить disposition `/wager` flag leak, пустого accepted message, expansion bounds и substitutions внутри Lua/option arguments; не переносить unsafe memory operations ради parity. Opaque server text остаётся server-owned, а локальные aliases не становятся физическими bindings автоматически.

Применить [Remaining client input loops](../research/remaining-client-input-loops.md): дополнительные spell/account/perusal/startup contexts, delegated packet/Lua/shutdown owners и SOUND_SDL alternatives. Явно согласовать intent при каждом cancel/default/abort, включая различающиеся wire replies, Lua extra/item hooks, save-chat filename cancellation и child screen/flag restoration. Различать сохранение игрового поведения и disposition обнаруженных cleanup defects; не считать всякий Esc универсальным abort всей цепочки.

Учитывать согласованное различие в [Define settings and migration boundary](05-define-settings-and-migration-boundary.md): разовый legacy import не исполняет load-time PRF actions, но штатная preference loading сохраняет baseline execution, input queue ordering и gates. Не распространять data-only migration policy на пользовательскую загрузку preference-файлов внутри клиента.

Как центральный input router нового UI сохраняет точную клавиатурную и пользовательскую macro-семантику legacy-клиентов во всех контекстах, одновременно поддерживая text input, focus layers и mouse-first маршруты к тем же игровым действиям?

## Answer

Пользователь подтвердил итоговое общее понимание 2026-09-14; решения Q1–Q14 и уточнения составляют каноническую резолюцию ниже. Реализация клиента не выполнялась; runtime parity не заявлена.

### Input router interface и ownership

Новый input router — единственный module/interface между UI gestures/text/semantic actions и переиспользуемой command/macro semantics. Он владеет logical input context, unfinished interaction input и смысловым focus. Widgets/renderer не интерпретируют macro queue и не отправляют packets. Session-derived pending requests и target остаются session state согласно presentation-model решению; router обеспечивает их input lifecycle, а view state содержит размещение/hover, не авторитетный routing state. Независимые копии одного pending interaction не создаются.

Interface предоставляет active context/prompt и его type, allowed responses, default, bounds, macro policy и outcome; принимаемый ввод различает physical gesture, text input, semantic action и prompt result. Context сохраняет parent/return contract, cancellation/retry/default, eligibility/gates и cleanup; exact C signatures и механизм преобразования legacy blocking reads не выбираются в planning-карте. Command logic переиспользуется с заменой Term reads/rendering на semantic prompts/results. Term/screen stack/globals не становятся interface законченного UI; это не эмуляция legacy Terms.

### Keyboard, macros и routing

Нормативны [Every input loop](../../../docs/research/single-window-input-loops.md), [Remaining client input loops](../research/remaining-client-input-loops.md) и [Slash-command grammar and dispatch](../research/slash-command-grammar-and-dispatch.md), с поправкой empty message в slash inventory. Все normal/roguelike mappings, raw command contexts, bypass/control conversion, normal/command/hybrid macro policies, load/override order, longest match, queue delimiters, navigation bypass, safe_macros/abort_prompt/missing-item и flush rules сохраняются. Произвольные user definitions разрешаются runtime; shipped defaults не заменяют macro engine.

Macro \w и \W сохраняют разные duration/confirmation/semaphore и fresh-key cancel/resume/queue contracts. Ожидания не блокируют network/timers/rendering; принятие отмены не расширяется на waits, где её нет в baseline. Keyboard command gestures и editor text проходят разные routes с context-specific macro policy; один physical event не вставляет текст и не исполняет gameplay command одновременно. Chat доступен только в штатных contexts. Encoding/IME/glyph conversion остаются вопросом [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md).

Raw key отправляется только в штатных fallback owners после предусмотренного macro/keymap/dispatch; это не общий fallback любого unknown UI gesture. UI shortcuts не отнимают baseline context bindings или пользовательские macro triggers. Неверный input сохраняет owner-specific default/retry/bell/exit.

### Mouse и command checks

Mouse actions идут прямо через semantic command interface, сохраняя checks, gates, confirmations, effects и packets соответствующего действия. Искусственный chat history и text substitutions для таких actions не создаются; реально введённые slash commands и macros сохраняют свой text route. Automation trio direct toggles не подменяется generic options hooks/synchronization. Mouse prompt response следует штатному выбору/default/cancel данного context. Во время macro-driven цепочки конкурирующий action отклоняется с feedback, не сохраняется для позднего исполнения; разрешённый ответ на текущий prompt допустим.

Проверки предметов выполняются на существующих шагах каждой command. Выбор остаётся slot-based; universal pre-send revalidation/stable item identity не добавляются. Mouse не обходит эти проверки и не заменяет недоступный выбор автоматически.

### Cancellation, server prompts и focus return

Cancel/default/abort не унифицируются: key request Esc отвечает0, string request — byte27, amount/number — штатное значение, item/spell cancellation может не иметь reply; Lua extra callback false может быть aux0, canceled item callback — item=-1 с последующим cast; direction cancellation отменяет соответствующий send. Встроенная UI-навигация без baseline аналога следует ранее согласованным parent/focus rules.

Receive-item/spell/direction сохраняют штатные busy/eligibility/qbuf rules и исключения (subinventory, shopping); REQUEST_KEY/AMT/NUM/STR/CFR остаются немедленными, без новой общей очереди или универсального busy gate. Semantic contexts заменяют терминальные busy flags по их значению, не по числу виджетов. REQUEST_ABORT, queue processing и restore следуют owner contracts. Response привязан к актуальному session/request/context; UI не создаёт двойной send через два пути обработки одного принятого ответа и не возобновляет завершённые interactions. Новые wire IDs/общая server transaction semantics не предполагаются.

Resize/recreation UI сохраняют logical context/text/caret/selected step и действительного parent. Focus loss не очищает принятый input/macro queue, не приостанавливает сеть/timers/macro chain; prompt с отсутствующим ответом ждёт. Modifiers читаются из SDL; focus gain сам не генерирует command. Только собственные held-key/button/repeat признаки нового UI сбрасываются при focus loss; intentional optional sticky modifiers сохраняются как baseline state. Session end/relogin очищает старые inputs/macros/requests, без replay.

### Slash compatibility и явные исправления

Сохраняются exact/prefix/case recognizers, whitespace/suffix behavior и порядок editor history → channel decoration → screenshot pre-pass → colour/item substitutions → self/local dispatch → opaque server forwarding. Substitutions внутри Lua/option arguments сохраняются для text route. Screenshot capture/configured inversion и cvpng open/convert-last сохраняют разные baseline outcomes/platform availability. Local Lua не превращается в finite server command whitelist; wager изменяет default, не делает bet; массовые autoinscriptions сохраняют local/server packet effects. Новые confirmations не добавляются автоматически.

Accepted empty message и channel prefixes остаются. Expansion проверяется относительно реальной вместимости output buffer с безопасными writes; сохраняется legacy удаление неуместившегося shortcut/ограниченная обрезка хвоста, без новой ошибки/reopen всего submit. Исправляются memory bounds и input cleanup defects: /wager не оставляет chat macro mode, child cleanup восстанавливает действительный parent/focus/policy, отмена save-chat filename не пишет file, но не отменяет выбранный выход из client. Эти изменения имеют отдельное acceptance evidence; остальные определённые unusual behavior не исправляются по догадке.

Preference loading сохраняет предусмотренные actions/queue ordering/gates; data-only policy относится только к разовому external legacy import.

### Handoff и доказательство

Implementation/acceptance обязаны дать per-context binding/transition coverage по всем трём inventories: input route/source, macro/keyset policy, context gates, accepted/default/cancel/retry result, local/wire effects, queue ordering и restoration. User macro/load/wait/missing-item scenarios и intentional fixes проверяются отдельно; mouse/keyboard routes сопоставляются по semantic outcome, а не искусственным одинаковым byte stream. Runtime parity и конкретный механизм тестов здесь не заявляются; evidence policy принадлежит [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md). Форматы UI-config/persisted bindings принадлежат persistence-schema тикету, encoding — соответствующему отдельному решению. Новых самостоятельных in-scope вопросов этот тикет не выявил.

## Comments

### Согласованные положения — 2026-09-14, первый раунд

- Q1: пользователь принял переиспользование command/macro semantics. Новый input router владеет logical input context, focus и незавершённым вводом, UI использует единый interface; widgets и renderer не интерпретируют macro bytes и не отправляют игровые packets самостоятельно.
- Q2: пользователь принял сохранение определённого поведения команд и пользовательских macros, включая необычные recognizers/substitutions и разные ответы при отмене. Ошибки памяти и утечки состояния ввода исправляются явно с отдельным evidence; конкретные спорные случаи ещё обсуждаются.
- Q3: пользователь принял отклонение конкурирующего mouse action во время macro-driven цепочки с понятным feedback, без отложенной очереди таких кликов. Mouse может отвечать на текущий prompt, если он допускает ответ; отмена следует owning context.

### Согласованные положения — второй раунд

- Q4: пользователь принял context-specific cancellation из inventories, включая различные server replies/no-reply и Lua extra/item hook outcomes; mouse Cancel эквивалентен штатной отмене данного шага, универсальный abort всей цепочки не вводится.
- Q5: пользователь принял исправление cleanup defects: отмена save-chat filename прекращает запись файла, не отменяя выбранный выход из клиента; восстанавливаются действительный родитель, focus и macro mode без возобновления завершённых действий. `/wager` не оставляет input в chat mode.
- Q6: пользователь принял прямые semantic mouse actions через command interface с теми же checks/effects/packets, без искусственного chat history и text substitutions. Реально введённые slash-команды и macros сохраняют text route; direct automation toggles и generic option paths не смешиваются.
- Q7 пока не согласован: пользователь запросил уточнение фактического legacy behavior для empty input и overflow.

### Уточнение Q7

Пользователь выбрал сохранение accepted empty message и штатного channel decoration, исправление size checks относительно действительной вместимости output buffer и сохранение legacy truncation behavior. Исходная рекомендация отклонять весь submit и повторно открывать editor не принята. В overflow paths сохраняются baseline удаление неуместившегося item shortcut и ограниченная обрезка хвоста, но только через безопасные size checks/writes; unsafe memory operations не переносятся. Проверка message expansion не устанавливает новую общую policy обрезки файлов/Lua.

### Согласованные положения — третий раунд

- Q8: пользователь подтвердил полный legacy macro pipeline: P/C/H policy, load/override order, longest match, normal/roguelike keymap/bypass, delimiters, safe_macros, missing-item, context-specific suppression/flush и различные wait/cancel contracts `\w`/`\W`. Network/rendering не блокируются ожиданиями.
- Q9: пользователь подтвердил раздельные command-gesture и text-editor routes с context-specific macro policy; один physical event не выполняет одновременно text insertion и игровую команду. Chat entry остаётся baseline-specific; encoding/glyph conversion решается в отдельном тикете.
- Q10 и Q11 пока не согласованы: пользователь запросил фактическую обработку server prompts в legacy и подробное объяснение focus/resize behavior.

### Уточнение Q10–Q11

- Q10: пользователь подтвердил сохранение текущей client policy, включая qbuf/eligibility для item/spell/direction и немедленные REQUEST_KEY/AMT/NUM/STR/CFR без общего busy gate. Нельзя подменять это единой очередью всех prompts.
- Q11.1: пользователь подтвердил сохранение logical context, unfinished text/caret и шага при resize/focus loss.
- Q11.3: пользователь подтвердил продолжение network/timers и macro chain при focus loss; требуется пользовательский ответ — prompt ждёт. Session end очищает старое взаимодействие без replay после relogin.
- Q11.2 пока не согласован: пользователь запросил фактическое key/modifier reset behavior текущего клиента.

### Финальное уточнение Q11.2

Пользователь принял фактический SDL-based подход: обычные modifiers берутся из SDL, focus gain не генерирует command; принятый ввод и macro queue не очищаются из-за focus loss. Если новый UI ведёт собственные held-key/button/repeat признаки, focus loss очищает только их. Intentional optional sticky modifiers остаются отдельным состоянием и сохраняются согласно baseline, без общего reset. В текущем main-sdl3 CheckEvent нет собственного KEY_UP/focus lost/gained handler; react_keypress использует SDL_GetModState; SDL3_STICKY_KEYS выключен по умолчанию. Evidence: src/client/main-sdl3.c:1188–1218,1396–1459; src/makefile.sdl3:40.

### Четвёртый раунд

- Q13: пользователь подтвердил переиспользование command/macro logic с заменой Term-based reads/rendering на semantic prompts/results через input router; prompt interface несёт type/allowed responses/default/bounds/result. Legacy Term/screen stack/globals не становятся interface нового UI; конкретная реорганизация C остаётся реализации.
- Q12 и Q14 пока не согласованы: пользователь запросил уточнение fallback/shortcuts и фактических inventory selection/send checks legacy.

### Согласование Q12 и Q14

- Q12: пользователь принял baseline raw-key fallback только в существующих dispatch owners, с preceding macro/keymap processing; остальные contexts сохраняют свои invalid/default/retry semantics. Новые UI-navigation shortcuts не перехватывают bindings действующего context или user macro triggers. Slash server forwarding отдельный.
- Q14: пользователь принял уточнённую baseline policy: проверки выполняются на штатных шагах конкретной команды; item selection сохраняет slot index semantics, не вводится универсальная повторная проверка перед send или стабильная item identity. Mouse выполняет те же проверки на соответствующих шагах; gates/confirmations не обходятся. Первоначальная рекомендация универсально revalidate current selection перед выполнением заменена этим уточнением. Evidence: src/client/c-inven.c:81–112; src/client/c-cmd.c:1815–1833,1859–1876; src/client/nclient.c:7728–7747.

### Итоговое подтверждение

Пользователь подтвердил общее понимание и закрытие тикета; проект перенесён в Answer без изменения согласованной семантики.
