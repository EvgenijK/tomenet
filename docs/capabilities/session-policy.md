# Session allocation: resolved policy sources

Source snapshots for ticket 08, observed at `0ef6b362cc7676805fdd0becff680dac50fe83c1`. These are planning requirements, not runtime evidence. Original local decision paths are retained below; only the resolved answers are authoritative.


## Original: `.scratch/single-window-sdl3-client/issues/24-define-credential-storage-policy.md`

Original file SHA-256: `849829742ffbfd8fc1b3adadd9cfd30c53ef25c85d61d06de93adfb2576ff5f4`. Relative links in the quoted snapshot are relative to that original file.

## Answer

Решение согласовано последовательными ответами пользователя на Q1–Q10, включая отклонение «Забыть пароль» и немедленное сохранение при смене пароля. Тикет разрешён 2026-09-18. Это planning policy; код клиента, реальное чтение секретов и runtime acceptance не выполнялись.

### Storage и ownership

SV использует защищённое системное хранилище текущего OS user: Linux amd64 — Secret Service через libsecret с binary secret values; Windows10/11 i686 MinGW32 — generic Credential Manager (`CRED_TYPE_GENERIC`, `CRED_PERSIST_LOCAL_MACHINE`, `CredReadW`/`CredWriteW`). Windows persistence означает того же пользователя на той же машине, без roaming. Собственного master password SV нет; разблокировку системного хранилища обслуживает его provider.

Это явное исключение из shared-settings policy. Старый SDL3 не читает SV secrets; SV не записывает свой пароль в `tomenet.cfg`, `.tomenetrc`, INI, PRF или UI configuration. Существующий открытый пароль legacy config может оставаться: его cleanup вне SV. При записи общих настроек legacy password record сохраняется без замены значением SV; все startup/login/conversion/manual-save/shutdown writers должны соблюдать это исключение. Иные common settings сохраняют согласованное ownership.

Системное хранилище сохраняет точные credential bytes, без Unicode normalization и преобразования по font. Runtime/protocol `my_memfrob` не является шифрованием at rest и не определяет storage format; переходы между raw secret и core representation должны быть явными. Binary libsecret/Windows blob не навязывает UTF-8/UTF-16 secret payload. Metadata, labels, names и lookup keys не содержат секрет или его обратимое представление.

API evidence и ограничения: [Secure credential storage APIs](../research/secure-credential-storage.md). Требуется usable persistent Secret Service provider; одно наличие libsecret не доказывает защищённую конфигурацию provider. Нельзя обещать изоляцию от произвольного кода с доступом того же OS user. Существующая legacy plaintext copy этим решением не защищается.

### Identity, login и import

Record identity — отдельный SV namespace плюс server address, effective port и account. Разные aliases/DNS names/IP автоматически не объединяются; серверное разрешение адреса не является основанием отправлять пароль другой identity. Account соответствует baseline login name semantics, без blanket lowercase. Windows case-insensitive target names не должны сливать разные case-sensitive account identities; конкретная collision-free serialization/reference schema принадлежит [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md).

При обычном входе пароль загружается только для выбранной identity и доступен форме ввода. Успешный login автоматически создаёт/обновляет соответствующую защищённую запись. Автовход управляется отдельно от запоминания и по умолчанию выключен; имеющаяся запись сама по себе не включает автовход. Ошибка аутентификации возвращает ручной ввод и не удаляет сохранённую запись. Успешный вход с исправленным паролем обновляет её по тому же правилу.

Legacy password читается только при явном импорте с preview/conflict policy из [Define settings and migration boundary](05-define-settings-and-migration-boundary.md). Существующий SV secret имеет приоритет, заменить его импортом можно только явно. Импорт привязывает пароль к выбранным server/port/account и сохраняет bytes точно; preview не раскрывает содержимое секрета. Невозможность записать secret не выдаётся за успешный импорт этой группы. Внешний источник неизменен; уже общий legacy password также не удаляется.

Отсутствие SV record, её внешнее удаление, ошибка чтения или отказ разблокировки не запускают автоматический fallback к открытому паролю старого конфига. Legacy loader не должен предварительно подставлять его в login memory, обходя это правило.

### Failure и session lifecycle

При недоступном хранилище, отсутствии службы или отмене разблокировки доступен ручной вход на текущую сессию. Открытого disk fallback нет. Отсутствующая запись отличается от ошибки доступа; UI объясняет невозможность загрузки/сохранения, не объявляя ошибку «пароля нет». Ошибка запоминания не превращает успешный игровой login в ошибку: текущая сессия продолжается, пользователь видит «Пароль не сохранён». Автоматических повторяющихся unlock prompts нет.

Операции хранилища не блокируют SDL event/render loop. Их завершение связано с исходной identity и текущей login operation; поздний результат после отмены/смены аккаунта не выполняет неожиданный автовход и не попадает в другой record. Промежуточные secret buffers освобождаются/очищаются после использования, runtime copy живёт лишь столько, сколько нужно действующему login/relogin contract; дисковая diagnostic copy не создаётся.

Отдельную опцию «Забыть пароль», UI для удаления records и remember-disable switch не добавляем. Это не требует запрещать пользователю управление записями средствами ОС; отсутствие записи после такого действия обрабатывается обычным ручным входом. Автоматическое запоминание после успешного входа сохраняется.

### Немедленное сохранение при смене пароля

После локального подтверждения нового пароля и успешной постановки запроса смены на отправку (`Send_change_password == 1` в baseline) runtime password обновляется и сразу запускается запись нового пароля в защищённое хранилище соответствующего аккаунта. Сохранение не откладывается до следующего login, не ждёт server acknowledgement и не разбирает текст игрового сообщения как подтверждение. Отмена формы, несовпадение повторного ввода либо локальная ошибка отправки не являются завершённой сменой и не запускают эту замену.

Успех записи означает только сохранение нового значения локально. Если сервер отклонит запрос или соединение оборвётся до обработки, защищённая запись всё равно может содержать новый пароль; автоматического rollback по текстовому сообщению нет. При следующей ошибке входа действует ручной ввод, после успешной аутентификации запись обновляется. Если сама запись в vault не удалась, текущая сессия продолжается с явным сообщением о несохранённости; нельзя объявлять новый пароль сохранённым. Перед заменой запись намеренно не удаляется.

### Outputs и границы

Секрет, его runtime/protocol representation и содержимое auth/password-change payload не попадают в presentation events, логи, diagnostic capture, обычные exports и конфигурационные dumps. Password fields не раскрывают значение в штатном UI; import preview сообщает о наличии/конфликте, а не печатает пароль. Ограничение относится к самим секретным полям, не отменяя обычные игровые сообщения и существующие message/character/screenshot exports.

Q3 не вводит новый exporter: отдельная функция выгрузки профиля с паролями и формат зашифрованного переноса не входят в этот контракт. Исходно согласованный legacy import сохраняется. Если отдельный перенос секретов когда-либо появится в новом scope, прежнее условие явного выбора не означает разрешения включать их автоматически.

### Follow-through и evidence

- [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md): metadata/identity serialization и references, per-writer исключение SV password из shared files, import group commit/error semantics. Storage policy не выбирается заново.
- [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md): synthetic-secret проверки обеих платформ, successful login/change-password writes, server reject/disconnect после отправки, locked/unavailable/cancel/save-failure cases, responsive UI и late completion, independent identities/case-sensitive accounts, legacy fallback prohibition и отсутствие секретов в outputs. Особо проверить сохранность legacy password при любом common-config save.
- Platform packaging должно включать клиентские зависимости libsecret/GLib для выбранного backend; отсутствие доступного persistent provider обрабатывается по session-only policy. Windows headers/import library не заменяют реального runtime evidence; actual blob limits и exact credential bytes входят в проверки. Конкретные dependency closure и OS scenarios принадлежат acceptance, а не новому product decision.

Новых decision tickets не требуется: точные schema и acceptance вопросы уже имеют владельцев. Resource-budget fog не меняется.


## Original: `.scratch/single-window-sdl3-client/issues/35-decide-text-field-boundaries-and-legacy-defects.md`

Original file SHA-256: `cf70f044b506c6313bea547fc1a078b1ed7735dc6226587165d0771a59ee4301`. Relative links in the quoted snapshot are relative to that original file.

## Answer

Решение согласовано в живом обсуждении 2026-09-18–19. Нормативный source/field inventory — [Source text and server-field byte contracts](../research/source-text-and-server-field-byte-contracts.md); таблица ниже задаёт dispositions, а не утверждает уже выполненные исправления или runtime proof.

| Случай | Принятое поведение | Проверяемый результат |
|---|---|---|
| Request-string editor 159 / safe wire payload 79; remote-script editor 80 / safe wire payload 79 | Ограничить поле 79 payload bytes с учётом фактического field contract, prefixes и escaping. Более строгий конкретный предел имеет приоритет. | Лишний ввод не принимается; обычная отправка содержит только допустимое значение и завершающий NUL. |
| Paste, текст макроса, уже загруженный default/imported value в ограниченном поле | Принять только помещающийся префикс, остаток обрезать без уведомления. Замена выделения учитывает освободившееся место. Принимается, что последующий Enter макроса отправит именно сокращённое значение серверу. | Обрезка относится к данным поля, не только к отображению. Не оставляет повреждённой последовательности представления или частичного escaping. Открытие поля само по себе не переписывает исходный файл. |
| Длинное имя локально выбранного font/tileset и служебные сведения о ресурсах | Полное значение сохраняется для локальной загрузки и настроек; только reporting copy для сервера сокращается до safe payload своего slot. | Ресурс продолжает работать по полному имени; сервер получает сокращённое название. Не переносить эту политику на file-transfer filenames или исполняемый текст. |
| Skill search / autoinscription search E80 при destination[80] | Обеспечить хранение минимум 81 byte, сохранив предел 80 payload bytes. | Допустимый поиск не сокращается до 79 из-за ошибки внутреннего массива. |
| Macro-set name / stage comment E20 при destination[20] | Обеспечить минимум 21 byte во всей цепочке копирования при прежнем пределе 20 payload bytes. | Сохраняются допустимые пользовательские значения и совместимые форматы файлов. |
| Outgoing packet framing и socket-room exhaustion | Проверить fields и сформировать/поставить в очередь целый пакет; нехватка места ждёт возможности постановки, не оставляет частичный пакет. Неучтённое переполнение вне согласованной editor/reporting truncation — локальный отказ отправки, не неявная обрезка в codec. | Все строковые поля имеют NUL в пределах slot, следующие поля не повреждаются. Baseline cancel/retry/reply и success semantics действуют только для действительного результата операции. |
| Incomplete / malformed incoming packet | Неполный пакет ждёт продолжения без частичного apply. Нарушение границ формата, включая отсутствие NUL в допустимом slot, завершает соединение с ошибкой протокола; границы не восстанавливаются догадкой. | Корректная фрагментация не вызывает disconnect. При malformed packet нет частично применённого состояния или выдуманного ответа незавершённому request; действует согласованный disconnect lifecycle. |
| Receive_playerlist, decoder до 80 bytes при NAME_LEN20 storage | Принять корректный wire field в достаточно большое хранение и сохранить полное значение для отображения. | Никакой обрезки имени до 19 байт ради старого массива; допустимость последующих действий с именем проверяется их собственными contracts. |
| RID_ITEM_ORDER на текущем сервере: 40-byte input / str2[40] | Конкретное поле ответа ограничить 39 payload bytes. Это клиентский обход server-side storage defect, не изменение сервера. | Длинный ответ сокращается по принятой editor policy; parse/count/item semantics, cancel и существующие допустимые короткие ответы сохраняются. Доступная длина заказа сознательно уменьшается. |
| Clipboard bounds, signed-char и locale filtering | Проверять фактическую вместимость после преобразований; устранить случайное отбрасывание bytes из-за signed char/locale, сохраняя явные правила конкретных полей и server-owned transforms. | Нет нового общего alphabet restriction, guessed charset или Unicode repertoire extension. Неизвестное Unicode↔byte соответствие по-прежнему даёт ранее согласованную draft-preserving encoding error. |
| Account credentials с `*`, server_protocol >= 2 | До отправки проверить возможность полного protocol round-trip. `*` превращается XOR42 в NUL, поэтому такой login/change-password не отправлять; показать объяснение несовместимости протокола, не посылать усечённый префикс и не терять введённый draft. Подтверждение нового пароля сравнивать по исходным bytes до transform. | Разные suffix после `*` не считаются совпавшим подтверждением. Локальный отказ смены не обновляет runtime/vault password. На protocol < 2 этот конкретный запрет не переносится. |
| Credential import / secure storage | Сохранить ранее согласованное точное хранение исходных bytes. Успешный импорт не объявляет значение пригодным для login: несовместимый protocol transform блокирует именно отправку. | Нет автоматического усечения/замены секрета. Моменты записи, приоритеты, ошибки и отсутствие plaintext fallback следуют credential policy. |
| Пароли архивов sound/music packs в меню установки | Скрытый ввод, исключение из общей input history и diagnostics, временное хранение для установки/повтора. Account-vault policy на эти пароли не распространяется. | Пароль распаковки не появляется в обычном recall; отмена/пустой ввод и retry сохраняют workflow установки. |

Пределы считаются в bytes конкретного поля, отдельно от Unicode UI, C storage и server-side semantic truncation. Для `%s/%S/%I` intended safe payload составляет 79/255/159 bytes плюс NUL в slot 80/256/160. Это не общий editor limit: purely local fields сохраняют собственные пределы; ранее принятый chat expansion с удалением неуместившегося shortcut/ограниченной обрезкой хвоста остаётся по [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md#answer). Серверные преобразования имён, quest clipping, callback-specific cancel/default/retry и семантика ответов остаются по atomic registry, кроме явно согласованного RID_ITEM_ORDER guard. Макрос теряет только избыточный текст ограниченного поля; последующие команды и control boundaries остаются по input-router contract.

### Follow-through

- [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md): полные локальные resource names и отдельная reporting copy, сохранность внешнего import source, credential raw-byte references и исключение archive passwords из обычной history. Конкретные profile/default/serialization решения остаются там.
- [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md): lengths 0/limit−1/limit/limit+1 до и после transforms, paste/selection/default/import/macro→Enter, source-file preservation, локальные 80/20-byte values, длинный playerlist field, RID_ITEM_ORDER39/40, split/oversized/chained packets с sentinel и queue exhaustion, locale/signedness, protocol branches и synthetic credential suffix collisions, archive history/diagnostics. Проверять сокращённое реальное server-bound значение, не только видимую строку.

Новых decision tickets не требуется: schema и acceptance имеют владельцев; resource-budget fog не изменился. Реализация, серверные исправления и runtime доказательства не выполнялись в этой planning-сессии.


## Original: `.scratch/single-window-sdl3-client/migration-sequence.md`

Original file SHA-256: `6727c5d0fb7b3dd8d7da8daa61cb48f36eb1afacfeaac50eba62e6ee23b40331`. Relative links in the quoted snapshot are relative to that original file.

## Approved framework

A–F checkpoints are cumulative. Dependencies needed by a capability (input, errors, persistence, resources, platform behavior) arrive with it, even if their broader feature family completes later. HTML completeness does not block native work. Previously accepted capabilities receive regression checks; future capabilities remain explicitly pending.

Linux-first; MinGW32 build from A, Wine smoke each checkpoint, Windows 10/11 VM checks at B/E/F and earlier for platform-specific changes. Linux software rendering starts with the first rendered slice; Windows software rendering with the first Windows runtime. Claims identify actually checked environments; Wine is not Windows acceptance.

## Stage detail

| Stage | Capabilities and required evidence | Known limits after this stage |
|---|---|---|
| A — runnable foundation | Separate executable/object paths, one SDL window, production model/command interfaces and renderer exercised by a synthetic decode→state→surface→input scenario. Prove complete/incomplete packet handling for this slice, ordered events, cancellation, resize/focus and software rendering. Create/validate full source-backed atomic manifest, native ledger and allocation to A–F; HTML snapshot/ledger may honestly report missing. | No claim of real login or gameplay; synthetic scope only. Later capabilities have named owners/stages and pending evidence. |
| B — session and game screen | Real server selection/connect/login/account and character selection/creation, MOTD, map/HUD/basic movement/chat/messages, disconnect/reconnect/exit and death/session transitions. Include credentials, byte limits, clipboard/text input, histories and minimal config/resource prerequisites; startup server file transfer/Lua reload and map weather/palette/resize negotiation arrive here when required by these flows. Verify real server round trip, normal/big map geometry, keyboard/macro routes of these flows, pending-request/session cleanup and Secret Service/Credential Manager behavior. | Item/combat selection and other C–E flows may use registered development fallback. Merely surviving a death transition does not claim ghost powers. |
| C — game actions | Items/inventory/equipment, targeting/look/directions, spells/skills/ghost/mimic/runes/stances/techniques, ordinary stores and associated confirmations/requests. Preserve keysets, user macro load/play/waits, mouse intents, cancellation/retry and return to parent. Verify relevant Lua and wire gates, store transactions, slot identity and multi-step macro chains. | Information/social/document/special-store and remaining integration flows stay pending unless required by an accepted C outcome. |
| D — information and server surfaces | Remaining character/knowledge/social flows, housing/utilities and applicable admin/DM flows, local and arbitrary server documents, search/page navigation, special-store canvases/animations and relevant server-driven controls. Verify lossless content, ordered replies, partial updates, fit/scroll rules, close/reset/recreation and no repeated side effects. | Remaining E settings/resource/file/audio/platform outcomes are pending; generic source-preserving documents/canvases are native surfaces, not terminal fallback. |
| E — integration completeness | Finish all settings/import/save/cancel, visual preferences/fonts/tiles/filters/effects, audio/packs, Guide, macros editing/recording, files/exports/screenshots and clipboard/OS associations. Cover every remaining in-scope atomic outcome, optional-feature configurations and provider/disk/resource errors. Prove independent CFG/OPT/history and shared-file ownership. All stage acceptance scenarios run with fallback disabled. | No missing native capability remains; final archive closure and final complete evidence review still await F. |
| F — final acceptance | Build without fallback linkage; verify all active in-scope IDs have current evidence, all input/packet/version/build inventories are mapped, no unclassified legacy route remains. Run required target archives in isolated profiles, Linux ABI/dependency checks, Windows 10/11 runtime, optional configurations, software rendering, complete regression and human review. | Only approved exclusions and explicitly documented environment limits; no pending required capability or runtime check can pass final acceptance. |

Every stage applies the existing acceptance layers to its scope, including short concurrency/lifecycle cases and submission gates 20/50/200 ms where applicable. No stress/soak, XHTML or global memory ceiling is added. Rendering/fonts and OS dependencies needed earlier cannot be postponed to E. Slash verbs, bindings, packet variants and local/Lua flows are allocated by their outcomes, not treated as a separate deferred umbrella feature.

## Registry and stage gates

The planned canonical files do not yet exist. A creates the schema, source-backed atomic manifest, native coverage ledger and full stage allocation from the completed inventories. Never fabricate IDs in this planning document or infer coverage from a broad family label. Each active in-scope outcome has one acceptance stage, its prerequisites, evidence requirements and a native/fallback/pending disposition. Cross-cutting scenarios reference all involved IDs. Shared primitives do not establish coverage of all callers.

A changed source or newly discovered outcome updates the registry and stage allocation before acceptance of the affected scope. If needed by an earlier claimed flow, it is not deferred merely to preserve a passed status. XHTML is explicitly excluded under the approved policy. HTML coverage and UX approval remain separate and truthful; missing HTML functionality cannot fail native acceptance.

Each checkpoint records current revision/configuration/environment, new accepted outcomes, regression results, remaining limitations and fallback inventory. Earlier accepted flows receive regression checks; changed dependencies invalidate affected evidence. Missing/failed/stale checks fail the corresponding claim. Exact test code and harness design belong to implementation.

## Native/fallback transition and retirement

A single input router owns logical input context, macro queue and pending requests. Development fallback is an allow-listed adapter for specific future capabilities inside the same SDL window; there are no permanent virtual Terms and no native reads from terminal buffers.

Routing is selected at a named flow/context entry, never as catch-all recovery from an unknown key or native error. A child flow may use fallback only when explicitly declared as pending; the parent is not claimed to cover that complete child outcome. Passing control restores the true parent and preserves the baseline queue/reply/cancel rules; no duplicated input, command or response. Networking, timers, model updates and required urgent rendering continue while fallback is active. Relog/teardown invalidates both native and fallback session context.

Each entry records its owning flow/module, replacement stage and removal check. Diagnostics contain only safe IDs/reasons/counts, never user content or secrets. Accepted flows entering fallback fail regression. Disable a route as its native replacement is accepted; remove unused adapter code incrementally. E demonstrates all scenarios with fallback disabled. F excludes adapter linkage and combines build/source route checks with runtime evidence; zero runtime hits alone cannot prove zero dependency.


## Original: `.scratch/single-window-sdl3-client/persistence-contract.md`

Original file SHA-256: `5fcdcdf8814dfd64ee580834277228a0330a93e7a7943fb35488f3cc1c5a155b`. Relative links in the quoted snapshot are relative to that original file.

# Single-window client: persistence contract

Status: approved planning contract, 2026-09-20. The user confirmed the complete contract in [Specify persistence ownership and UI configuration schema](issues/25-specify-persistence-ownership-and-ui-configuration-schema.md#answer). This document consolidates the final decisions and exact paths/serialization mechanics; it supersedes conflicting historical proposals in the linked source-audit drafts. It is not an implementation or runtime parity claim.

## Ownership and paths

`U` is the existing SDL3 user root (`TOMENET_SDL3_USER_PATH`, otherwise `SDL_GetPrefPath("TomenetGame", "tomenet")`); `S = U/sv`; `B` is the selected game library, normally `lib/` beside the executable. Do not globally change `ANGBAND_DIR_USER` or `SDL3_USER_PATH` to S: that would relocate deliberately shared data and change the SDL3 handshake fingerprint. Root selection and compatible CLI/environment overrides retain baseline behavior; importing a config does not implicitly relocate U/B. The SDL3 fingerprint continues using U, not S.

| Data | Concrete destination | Load/save contract |
|---|---|---|
| Main connection/audio/resource-selection/UI CFG | `S/tomenet.cfg` | Independent of legacy CFG; explicit Save only |
| Global option snapshot | `S/global.opt` | Existing OPT syntax and global scope; explicit Save |
| System-specific option layer | `S/global-sv.opt` | Existing system-layer position; no inheritance from user `global-sdl3.opt` |
| Character option snapshot | `S/<character>.opt` | Existing naming/lifecycle, including pre-login application before option packets; explicit Save |
| Explicit class/named option snapshots | `S/<class>.opt` or user-selected SV option destination | Keep baseline named/global/class commands; class save does not introduce automatic class OPT loading |
| Dedicated option entrypoint | `S/options.prf` when present | Independent option owner even though suffix is PRF; conversion is in memory until Save |
| Macro PRFs and stage metadata | Existing shared `U/user` overlay, bundled B fallback | Existing global/race/trait/class/character/form precedence, normal permitted load-time effects; explicit macro saves update shared files |
| Autoinscriptions INS | Existing shared `U/user` | Existing most-specific-file selection, named/global/class saves and automatic format conversion |
| Birth templates DNA | Existing shared `U/user` | Existing name/reincarnation lookup, automatic birth save and format conversion |
| Input history | `S/chathist-<account>.tmp` | Existing account naming, bounded recall/dedup/relog rules; symmetric load/save path and baseline exit lifecycle |
| Guide bookmarks | `S/bookmarks.tmp` | Existing format/lifecycle; symmetric load/save, explicit old-file import |
| Received private notes | `U/user/notes-<account>.txt` | Existing automatic append/browser behavior; not a settings save |
| Message/character/screenshot exports | Existing shared `U/user` | Existing content/format semantics; generated names get a free suffix on collision, explicitly selected existing filename requires replace/cancel choice |
| Message cloning | Console and shared `U/user/stdout.txt` | Existing `clone_to_stdout`/`clone_to_file`, initially off; colour stripping, line ordering, append/flush behavior from SDL3; independent of bounded recall and new-recorder prohibition |
| Fonts/tilesets/mappings | Existing shared user resource overlay and bundled resources | Independent requested selections in S CFG; asset/mapping data remain shared |
| Audio packs, event mappings, disabled events, per-event volumes | Existing shared pack directories | Shared resource config edits; selection/master/category controls belong to S CFG |
| Derived graphics caches | Shared resource root with a distinct SV cache version/namespace | Do not import; keys cover source/profile/filter/final geometry. Never reuse legacy cache identity for incompatible output |
| Updated guide | `U/TomeNET-Guide.txt` | Viewer/checksum/update use the same user override; bundled B/G guide is fallback. Legacy may still use its installation copy |
| Credentials | Previously selected OS vault | No secret/reference-to-secret-content in ordinary CFG/OPT; precise policy remains in credential ticket |
| Server file transfers | Original client's destination mapping | Q40: preserve original validation, mapping, CHECK/INIT/DATA/END and reload behavior; no new resource-only allow-list/deny-list. Do not rebase generic transfer destinations onto S |

Absolute/custom filenames and explicit file operations retain their existing meaning. Automatic defaults and internal Save routing use the owned locations above. External import is a different operation: it never modifies its source. No feature is permitted to invoke a legacy writer against `U/tomenet.cfg` merely because it previously did so at startup, audio selection, palette change or shutdown.

The full operation inventory is [file ownership evidence](research/sv-file-ownership-draft.md). Its old proposals for a new transfer allow-list, guide placement or unresolved group rules are superseded by this contract and the live answers.

## Settings editing, publication and conflicts

1. Opening settings records the current values. Changes preview immediately. Resource changes must first prepare a usable replacement; a failed live load retains the previous working selection and reports the failure.
2. **Сохранить** persists the edited settings to their owned CFG/OPT destinations. An ordinary in-game option saves to the current character scope by default; the existing explicit global/class/named save actions retain their meanings. New UI settings are user-root-wide, not character/server-specific.
3. Closing a dirty form offers **Сохранить / Отменить изменения / Вернуться**. Cancel changes restores settings values from opening the form; it cannot undo gameplay actions already performed while previewing an option. Gameplay state, incoming messages and already-delivered replies are not rolled back.
4. Normal game exit does not save pending CFG/OPT changes and does not add another settings prompt. Baseline history/bookmark/notes/DNA/resource writes and approved vault writes have independent lifecycles.
5. Own CFG/OPT missing-file defaults and obsolete-key conversion remain in memory until Save. A successful explicit import has its own reviewed commit action; it is an authorized write, not an automatic startup import. Shared INS/DNA conversions remain automatic. Normal macro files do not acquire blanket auto-resave.
6. Resolve the concrete owner/path once for every read/stat/temporary/backup/replace/delete. Avoid legacy `my_fopen` redirection combined with raw rename against a different logical path. Parse and validate before publishing, use unique sibling temporaries, and retain old destination data if writing fails. A failed Save leaves the edited values active and dirty and reports that they were not saved.
7. Save rereads its destination and merges only changed records. For concurrent edits to the same value, retain the external value and report the conflict; do not silently claim the attempted value was persisted. Unrelated unknown compatible fields remain intact. This applies to multiple SV instances and still-shared writers; it is no longer a reason to share main CFG/OPT with legacy.
8. Shared legacy writers do not participate in a new locking protocol. SV can coordinate its own writers and detect changes before replacement, but cannot guarantee that a running old client will not overwrite a shared file later. This limitation was accepted. Macro-stage/import/resource linked groups must not be reported as complete if only part was published; rollback/error results identify the affected group.

Existing named save/load operations do not silently change scope. A settings Save does not rewrite a common macro PRF just because that file supplied an option value. Explicit macro saves retain unrelated recognized records in a mixed file; conversion does not replace the file with an option snapshot. Includes and execution records are preserved as records, not executed during migration.

## Main CFG schema and scalar defaults

The format is SDL3's line-oriented CFG, **not JSON**: `key` followed by spaces/tabs and a value, `#` comments, and existing standalone flags. Writer emits a tab separator and LF. Reader accepts CRLF. Existing compatible string/flag semantics remain; do not introduce a guessed encoding or apply UI Unicode conventions to opaque field bytes.

Technical version rule: `svSchemaVersion\t1`; absent version is treated as version1 for an SV file using only recognized compatible records. Unknown fields in a supported version survive Save. A newer incompatible version uses defaults with a message and is not overwritten. Duplicate recognized values are evaluated in file order, last well-formed assignment wins; an invalid occurrence is reported and uses that field's default. Match complete key tokens: `soundpackSubset` must not accidentally toggle `sound`. Reject embedded NUL/newline in a scalar and unrepresentable serialization before file publication; preserve the draft on an encoding error.

Use the exact same keys for existing compatible settings, with complete Save coverage even where the legacy writer only wrote a key on file creation. `pass` is a recognized exception: it is not read in normal startup and not emitted or preserved as an unknown field. `nick`, server/port and fullauto remain nonsecret metadata. Explicit legacy import alone can read a legacy password and send it to the vault. No secret-bearing temporary CFG or backup is created.

The [complete CFG source registry](research/sv-config-key-draft.md) enumerates all original read/write keys and the cross-product of ten old/new Term prefixes and suffixes. Apply these dispositions to every row:

- Connection/audio/palette/resource selections with the same meaning are retained in S CFG and save independently. Existing compiled/default initialization and the **client** stock `tomenet.cfg` provide baseline defaults; the server `lib/config/tomenet.cfg` is not a source of client defaults. Explicit decisions below override them.
- Existing values include FPS100; audio44100Hz/32 channels/buffer1024; master/music/effects/weather volumes70 and switches enabled; selected pack names `sound`/`music`, subset1. SDL3 audio bounds are rate4000…48000, buffer128…8192, effect-track count4…32 (not physical speaker channels); mixer output remains stereo. Out-of-range serialized values follow the approved invalid-value default/message rule, not unsafe indexing. Validate other values against the reused backend's supported bounds; unavailable device/capability follows baseline failure behavior rather than a false successful save/apply claim.
- Existing palette entries, `lighterDarkBlue`, outline setting, standalone flags and subtileset selections retain stock SDL3 meaning/defaults. Previously write-only subset preferences get a real read/write round-trip. Baseline frontend/build gates remain explicit; unsupported keys are retained inert with a diagnostic, not converted to an unrelated feature.
- Legacy Term geometry, titles, visibility and W assignments never instantiate SV Terms. For explicit font import, the **main** Term font maps to both text and map roles. Other Term-font assignments are not guessed into modern panels. Render-cache/workaround keys remain recognizable compatibility data where their old mechanism does not exist; do not turn them into different controls.
- CLI values override loaded preferences for the running invocation according to existing option semantics; the CFG is not rewritten just because CLI overrides were supplied. Import does not silently activate bootstrap path changes. Roots/resources are resolved by their established startup owner.

| Key / value | Contract |
|---|---|
| `svSchemaVersion 1` | Format version, no secret fields |
| `svLayout wide` | `wide` / `small`; saved SV value governs actual normal/big map geometry; startup default wide, existing once-per-launch small offer retained |
| `svWindowMode fullscreen` | `fullscreen` / `window`; fullscreen default. This means the display's desktop mode, no new resolution-switch UI. Remember saved windowed geometry separately from fullscreen output size |
| `svWindowWidth`, `svWindowHeight` | Optional last explicitly saved windowed logical client size. Never save fullscreen dimensions over it. With no saved windowed geometry, use available desktop work area subject to existing minimum1024×768; WM may constrain placement |
| `svUiScalePercent 100` | Range50…200, step5; invalid/off-step values use100 with a message; OS DPI applies separately |
| `svTextFont CascadiaMono-Regular.ttf` | Bundled exact v2407.24 asset identified by the existing candidate report; OFL/copyright included |
| `svMapFont 16x24x.pcf` | Default map font, independently changeable |
| `graphics` / `graphic_tiles 16x24sv` | Default `graphics 1` (the first standard enabled mode of the existing SDL3 toggle), selected16x24sv; retain explicit dual-mask mode2 and its existing server gate, masks/composition and resource definitions |
| `svGraphicsFilter linear` | `nearest` / `linear` / `pixelart`; default Linear. Imported legacy0/1/2 mean Nearest/Linear/Lanczos, with Lanczos→Linear reported in import result; never map numeric2 to PixelArt accidentally |
| `svPcfFilter nearest` | Independent PCF sampling selector, same modes, default Nearest; applies to PCF rendering in either role, independently of tiles |
| `svTextProfile`, `svMapProfile` | Optional explicit resource-profile references; absent means the registered profile for the selected resource/source role. No network charset inferred from font metadata |
| `svPanelLeftWidth`, `svPanelRightWidth` | Saved logical widths; exact defaults/bounds are supplied by approved HTML UX metadata, not earlier throwaway numbers |
| `svBlockVisible_<id>` | Boolean visibility preference indexed by stable semantic block ID from approved HTML/manifest |
| `svWidgetOrder_<container-id>` | Ordered stable widget IDs; duplicates invalid, unknown IDs retained inert for forward compatibility; default order/placement follows HTML UX |

There is **no map zoom key**: always100% maximum full-grid fit with accepted integer final-cell geometry. Cell raster size, effective viewport, fallback asset and cache generation are derived, not saved choices. No persistent small-offer dismissal, new recorder setting, plaintext password, or per-Term layout records are introduced.

Exact text metrics at100%, panel/block/widget catalogue and constraints, cursor appearance and control layout remain with the main HTML UX prototype by explicit user delegation. Native schema binds to the same stable IDs and approved layout version. Missing approval in the HTML ledger is not a claim of native UX acceptance; this map does not invent those visual defaults.

## Resource identity, loading and fallbacks

Keep requested resource name/reference bytes and associated profile references in S CFG; lookup uses the existing user-overlay-before-bundled hierarchy. Full local names are not shortened to fit server reporting slots; only the reporting copy follows the accepted field-boundary policy. Resource identity for caches also includes source/profile generation, not just a mutable filename. Profile data preserve proven legacy-ID correspondence, control markers and source-specific byte contracts; no universal outgoing charset is introduced. Existing PCF numeric glyph semantics and font/graphics PRF mappings remain normative.

Text font changes do not reload map definitions. Map font changes retain the established map-font→mapping reload→graphics overrides lifecycle. Explicit import of a legacy main font assigns both roles once; subsequent role changes remain independent. A missing or unsupported profile is handled as a resource failure, not a guessed mapping or an unnoticed replacement of all legacy glyphs.

| Startup failure | Effective fallback |
|---|---|
| Requested text font/profile unusable | Bundled Cascadia Mono/profile, then bundled16x24x.pcf/profile; skip duplicate steps |
| Requested map font/profile unusable | Bundled16x24x.pcf/profile |
| Selected tileset unusable | Font-only map, message; preserve requested tileset and enabled preference for next startup |
| All applicable font fallbacks fail | System error identifying the failed resource; stop startup |
| PixelArt unsupported by actual preparation/backend path | Effective Nearest, show requested/effective distinction; preserve requested PixelArt |

Live resource-change failure retains the previous working stack rather than committing fallback as the requested value. Successful current PCF is never replaced solely for aesthetics or scaling. Updated assets invalidate only affected role caches/generations. Pack config/event volumes are shared, but selecting a pack or moving a category volume is a private CFG edit with explicit Save.

## OPT registry and semantic preservation

[All 200 lexical option keys](research/sv-option-ownership-draft.md) are the exhaustive source index: 201 initializer rows because `big_map` has two build variants; 12 keys are inside `#if0`. Preserve active-build defaults, boolean meanings, network slot identities and gates. Hidden is not inactive: server-used `view_reduce_light`/`view_reduce_view` and client-used `other_query_flag` still have consumers.

The earlier draft's L/U classifications are **not authorizations to remove display behavior**. Q37 explicitly requires meaningful old display preferences to survive and affect SV. The following disposition overlay covers those formerly disputed groups:

| Keys / family | SV effect and ownership |
|---|---|
| `exp_bar`, `hp_bar`, `mp_bar`, `st_bar` | Select number/bar presentation of the corresponding semantic status indicator; HTML defines its modern appearance |
| `mp_huge_bar`, `sn_huge_bar`, `hp_huge_bar`, `stun_huge_bar`, `st_huge_bar`, `solid_bars`, `huge_bars_gfx` | Preserve expanded-indicator visibility and presentation preferences in the corresponding HUD elements; layout eligibility remains meaningful. Do not lose the preference merely because symbols/Term coordinates are gone |
| `keep_topline`, `topline_no_msg`, `topline_first` | Control retention/visibility/first-line selection of the corresponding immediate game-message presentation. They do not erase occurrences, full recall or mandatory delivery; they are not blanket suppression of every message surface |
| `alert_offpanel_dam` | Keep alerts for the same originating interaction contexts that formerly suspended the ordinary game view; input-context ownership replaces screen_icky, not a guess from pixel occlusion |
| `highlight_player` | Toggle self-highlight semantic indicator; confirmed HTML cursor design supplies the shape |
| `overview_startup` | Select resistance/bonuses character-page preference as baseline; not a new instruction to open an overlay at login |
| `always_show_lists` | Preserve when selectable-item/spell lists are presented during prompts; new picker layout does not discard the option |
| `player_list`, `player_list2` | Preserve normal/compact/more-compact player-list presentation; HTML expresses the levels without a legacy Term |
| `time_stamp_chat` | Preserve baseline periodic timestamp marker behavior; do not reinterpret as a timestamp attached to every message |
| `target_history`, `targetinfo_msg` | Keep existing message-production/routing conditions and source occurrence semantics; no duplicate messages just to emulate a second terminal |
| `no_flicker`, `subterm_flicker`, `misc_no_flicker` | Preserve global animation switch and semantic scopes: ordinary main game content versus auxiliary content, with messages excluded from the misc-only suppression. Content roles supply scope; don't recreate windows |
| `thin_down_flush`, `disable_flush` | Preserve existing pacing choices and counter/skip semantics in presentation scheduling. No sleep may block packet apply/urgent render deadlines; use nonblocking due times rather than redefining these options as FPS or network batching |
| `use_color` | Retain existing supported coloured/monochrome presentation behavior in applicable contexts; hiddenness is not an automatic no-op |
| `gfx_autooff_fmsw`, `no2mask_weather` | Preserve the existing solid-wall adjustment and weather-composition branch preference, subject to corresponding source/build gates |
| `clone_to_stdout`, `clone_to_file` | Original SDL3 message clone semantics, off by default; destinations specified above; not a new session recorder |
| `screenshot_keys`, `screenshot_format` | [XHTML removed and bindings unified](issues/09-design-parity-evidence-and-acceptance.md#screenshot-bindings-unified--confirmed-2026-09-20): Ctrl+T/Ctrl+Shift+T and /shot / /screenshot invoke native PNG/BMP capture. Retire screenshot_keys swap; importing its old value does not change the action or restore XHTML. Preserve screenshot_format filename mode and no-silent-overwrite rule |
| `big_map` | Compatibility record does not override saved SV layout. Actual normal/big geometry and any required wire projection derive from layout; preserve protocol slots and avoid a competing source of truth |
| `gfx_palanim_repaint`, `gfx_hack_repaint` | Legacy repaint-technique controls remain recognized/inert when that technique is absent. This exception does not disable palette animation or meaningful display preferences |
| `#if0` and unavailable build records | Preserve imported records inert with an explicit unsupported disposition; do not reactivate absent capabilities or shift protocol slots |

Every other active option retains its source semantics through the reused core/presentation consumer and owns its value in SV global/character OPT. Display options are not all moved to global scope: only the new UI settings have explicitly global scope. HTML determines appearance, not whether an active native option works.

Deprecated aliases follow the exact branches in `process_pref_file_aux_aux` (c-files.c:1092 onward), including inversions/one-to-many conversion and deliberately discarded aliases. No guessed rename algorithm. Alias conversion does not modify a legacy source. Unknown OPT/PRF records keep baseline warning/skip-at-load behavior, but compatible source records are retained for non-destructive save/import reporting; recognized executable records are never executed by migration.

## Preference load order and boundaries

Defaults → shipped preference/mapping bootstrap → SV global options → applicable system preferences → selected font/graphics preferences → SV system options → SV character options at the original early pre-send point → baseline global/race/trait/class/character macro sequence. Preserve the existing later character/form reload points and local command effects; no silent addition of class-OPT autoload. Settings save targets independent snapshots, not whichever shared PRF last supplied the value.

`pref.prf` bootstrap must not implicitly reintroduce old user `global.opt` or `options.prf` as a settings base. Resolve option entrypoints to the S owner, use shipped defaults as baseline data, and route explicit macro/resource sources to the shared owner. An explicitly selected legacy options file is a load/import action, not automatic synchronization. Includes carry provenance and resolved ownership; a `.prf` suffix alone does not imply macro ownership.

Shared macro option directives deliberately affect the current session. Legacy Term-window W assignments do not. Macro loading is not migration: normal permitted execution gates remain as originally agreed. A settings Save does not flatten all loaded includes/actions into a shared file. Explicit macro modifications preserve unrelated records in mixed sources.

## Import and failure granularity

Offer import on first launch only if known SDL3 or standard X11 settings are found; user may select another source manually. No whole-disk scan, no silent mixture of SDL3/X11 profiles, no automatic resource-installation discovery. Imports and reimports are explicit. If setup was skipped without saving a CFG, the next absence-of-CFG launch remains a first-run situation; detecting settings still does not authorize applying them.

Preview groups: main CFG, global/character OPT, linked macro/include/stage sets, INS, DNA, resource preferences, input history, bookmarks, notes and credentials. Compatible groups remain selected by the earlier import policy; user controls conflicts. Already shared files are not redundantly copied onto themselves. Existing destination values win by default; replacements require explicit choice. Unsupported/inert settings and Lanczos→Linear substitutions are listed.

Identity of conflicts: CFG/OPT scope+key; macro set/file+trigger/stage identity without reordering precedence; INS whole ordered file; DNA normalized baseline filename; history whole account history; bookmarks whole bookmark file; notes whole source document. Treating a document/file as a group avoids guessing how to deduplicate identical legitimate events. A linked macro/include/stage group commits completely or is skipped; missing/cyclic/out-of-source includes prevent that group, not unrelated valid groups. Import does not fetch missing resources: retain requested references and apply the approved resource-failure policy.

Parse/stage without execution or source writes. Save failure retains prior destination group; independent successful groups are reported separately. Existing vault secret wins by default and is replaced only explicitly; vault failure cannot be reported as successful credential import. Imported source remains untouched. Saving settings never rewrites a credential merely because an account/server metadata field was saved.

## Credential identity and private inputs

Reuse [credential policy](issues/24-define-credential-storage-policy.md#answer) and [text-field dispositions](issues/35-decide-text-field-boundaries-and-legacy-defects.md#answer), including session-only fallback, exact raw bytes, protocol-star rejection, no hidden legacy-password fallback and immediate vault write after accepted password-change send.

Technical serialization: namespace `tomenet-sv/v1`; collision-free length-prefixed byte tuple of baseline server-address spelling, effective port and account bytes. Do not case-fold account or resolve a hostname to an IP merely to merge records. Encode the tuple as lowercase hex for a Windows case-insensitive TargetName; preserve the same identity tuple in Linux attributes/lookup. Provider length limits are checked before storage; failure follows approved session-only behavior, never silent key truncation. Identity/reference metadata does not contain secret bytes. Credential import destination identity is shown without revealing the password.

Archive/resource passwords remain temporary private input excluded from shared history/diagnostics. Existing independent message/character/screenshot/clone outputs continue; they do not gain access to account credential buffers or reversible representations.

## Completion and evidence boundary

This contract is a planning decision, not proof of native behavior. Acceptance must cover all keys/aliases/files and both target platforms: load→apply→Save→reload, unknown/incompatible/corrupt CFG, numeric boundaries, selected/effective fallbacks, resource disappearance/reappearance, role-specific font changes, options from common macros, namespace routing including raw rename/remove, private histories, shared INS/DNA/resource updates, concurrent external edits, group import failure, original server transfers, guide override/read/update and vault identities/exclusions.

No final claim is made from source counts alone: 200 lexical names include inactive/build-specific records. The HTML layout catalogue and explicit font/profile corpus are versioned evidence inputs, not invented values. Required performance/resource budgets remain with acceptance; user UI-scale/settings decisions do not invent those budgets. The next map ticket after this one resolves acceptance, followed by migration/fallback-retirement sequencing.
