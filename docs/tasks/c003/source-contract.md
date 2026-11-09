# C003 — исходники, готовые части и пробелы

Дата исходной сверки: 2026-11-07; актуализация 2026-11-09. Это source-backed planning, не runtime evidence.
Graphify query `C003 files resources persistence` выполнен первым: граф 21163
nodes дал 110 результатов, вывод усечён до 41. Он показал persistence contract
и allocation docs, но не доказал актуальные callers; ниже проверены исходники.
Новый граф не генерировался. Код не менялся, update не требуется.

## База C002 и ownership

- `src/client/sv/settings/config.c`, `options.c`, `startup.c`: собственные настройки
  и option layers уже существуют. Clone flags 145/146 по умолчанию 0; это данные,
  не реализация sinks. При C003 не добавлять CFG/OPT autosave.
- `ui/font.c:sv_font_open_selected`: U/xtra/font → B/xtra/font, затем объявленные
  bundled Cascadia/PCF fallback. Requested имя хранится settings owner отдельно.
- `settings/audio.c:roots/pack`: user/bundled lookup и initial pack metadata;
  `.default` читается без создания копии. Сохранить эту семантику при подключении
  file owner. Это не общий owner raw stat/temp/rename/remove.
- `login/native-run.c`: соединяет profile, settings, font и audio. Не читать файлы
  из renderer/protocol; связывать consumers явно здесь/в app assembly.
- `credential/vault.c`: Linux libsecret, thread request и generation, без собственного
  file vault. Не изобретать credential temp или plaintext fallback ради C003.

## History

Baseline `src/client/c-init.c:do_save_chatinput` пишет непустую историю oldest→newest
с ring wrap; load перед `Input_loop` ограничен MSG_HISTORY_MAX, full ring меняет
index/looped state. `c-util.c:askfor_aux` переносит повторённую строку на конец.
SDL3 load использует os_temp_path, save идёт через ANGBAND_DIR_USER/my_fopen;
принятая policy устраняет зависимость от этих legacy path различий:
обе операции SV используют S/chathist-account.tmp.

`src/client/sv/input/text-field.*` уже имеет SvTextHistory, используемый endpoint
editor. Это не дисковая история account chat и не основание сохранять endpoint,
login/password или всякий text field. Production account history load/save и
публичный chat commit необходимо связать отдельно; существующая login UI не
становится chat UI. Актуальная сверка подтверждает отсутствие submitted-chat
caller: core реализуется сейчас, связывание принятого ввода — C028. Baseline
MSG_HISTORY_MAX=2000 (`src/config.h`), endpoint SvTextHistory=16. History update в
askfor_aux идёт после проверки Escape/nohist и до chat-mode преобразований:
фиксировать принятый несекретный input до Send_msg, а не только успешно отправленное.

## DNA

Baseline `c-files.c:save_birth_file/load_birth_file`:
нормализация CHARNAME_ROMAN suffix, `.dna`, previous-name reincarnation, shared
U/user с bundled fallback для чтения; header + mode/class/race/trait + 6 stats.
Формат до 4.7.1b переводит MODE_MALE_OLD/MODE_FRUIT_BAT_OLD и выполняет touch save.
Hidden class mapping учитывает server gate 4.7.0.2 и текущий class setup.
`c-birth.c` вызывает save_birth_file после birth; момент сравнить с существующим
SV committed play, не переносить его на произвольный redraw или полный login ACK.

`login/creation/birth.c` уже поддерживает DNA choices, validity и committed draft;
`login/flow.c` получает birth_dna_* через options. Реального file loader/save в
этих callers не найдено. Подключить production file→semantic DNA и обратно,
сохраняя validation и cancel; не считать fixture injection доказательством файлов.
C040 остаётся владельцем отдельных birth capability IDs, не дубликатом C003 ticket.

## Notes и clone

Baseline `nclient.c:Receive_message`: HCMSG_NOTE в buf+6, продолжение при got_note
и пробеле в buf[2], non-note сбрасывает цепочку; запись с timestamp, удалением
253/254/252 markers и 255 colour pairs, включая broken final marker.
Фраза «исходный текст» означает baseline текст после этих преобразований,
а не дословную запись wire controls. Повтор одинакового события не подавляется.

Baseline `c-util.c:c_msg_print`: console/file sinks после baseline routing,
colour stripping, newline, console fflush; file append/close. SDL3 путь stdout
через os_temp_path не переносится буквально: policy требует shared U/user/stdout.txt.

Production SV уже декодирует полный PKT_MESSAGE в `protocol/message-update.h`,
`protocol/protocol.c`; `session/session.c:apply_message` выдаёт sequence и pending
occurrences; `app.c` имеет take-message entrypoint. Sinks/notes здесь ещё не
обнаружены. Подключить эффекты ровно один раз на occurrence, не в renderer или
чтении recall; partial packet не меняет notes chain. Chain живёт в поколении
сессии, destination содержит account. После reconnect/account change reset.

## Identity и wire

`protocol/login.c:sv_login_set_audio/sv_login_send_audio` уже передаёт snapshot,
копирует reporting names в 80-byte slots, сохраняет local names и шлёт после
PLAY_ACKNOWLEDGED. Source gates Send_audio: 4.7.3 и identity 4.9.2.1.0.1.
Полный Send_version и отдельный Send_font здесь не обнаружены; font также входит
в start-play layout. Нельзя подменять map font текущим text-font path.

`nclient.c:Send_version`: <=4.8.0 no-op; далее version/OS; >4.9.1 добавляется avg;
>4.9.2 добавляются guide/branch/build/tag/language; с 4.9.2.1.0.1 — packs;
с 4.9.2.1.0.2 branch/build удалены. Сохранить strict/inclusive сравнения.
`Send_font`: gate 4.8.1.2, short graphics mode + tiles name + font name.
Сверять также server `nserver.c:Receive_version/Receive_audio/Receive_font`:
все поля %s ограничены MAX_CHARS=80 в `src/common/sockbuf.c`, несмотря на
font/tiles receive buffers 1024. Reporting copy: до 79 bytes+NUL, в том числе
font/tiles; полный local reference не меняется. Тесты limit−1/limit/limit+1 обязательны
для reporting copy; полные local selections остаются неизменными.

Guide placeholder не читает Guide; значение отсутствующей возможности берётся
из baseline missing-feature branch. Requested/effective map identities должны
поступать от фактического consumer; отсутствие полного consumer фиксируется,
не прикрывается успешной сериализацией выдуманного effective значения.

## Publication и границы незавершённых consumers

Baseline `c-files.c:my_fopen`, `c-util.c:my_fexists`, SDL3 resource stat/remove/cache
операции показывают, почему одного fopen wrapper недостаточно. Реестр C003
требует generated suffix, explicit replace/cancel, failed write сохраняет старое
назначение. Экспорт tomb/messages/screenshots должен пользоваться одним policy
owner без реализации всех этих surfaces в C003. Production exporters в SV отсутствуют, что подтверждает deferred export
message в native-run. По решению 2026-11-09 publication core реализуется в 001,
реальные export consumers и их evidence — C029/C032/C061. Аналогично history и late resource
identity. Универсальные mocks и тестовый producer не закрывают production outcome.

Transient files принадлежат операции/owner, очищаются на cancel/relog; history
исключение является persistent personal data независимо от `.tmp`. Существующие
credential provider и diagnostic -e не надо переносить в новый storage framework.
Legacy writer locking не добавляется; ограничение concurrent edits сохраняется
из settings policy. Полные transfer CHECK/INIT/DATA/END и import относятся к своим
этапам; C003 не меняет их destination mapping или allow-list.

## Разрешённые исследованием швы и предлагаемые entrypoints

Дополнительная сверка `input/native-input.c`, `app.h`, `app.c`,
`login/native-run.c`, `protocol/login.c`, `protocol/protocol.c` выполнена в плане:

1. **Chat caller отсутствует.** Native input принимает raw gameplay bytes и
   односимвольный server request; публичного submitted-chat action/serializer нет.
   Предлагаемый минимум C003: account-scoped history owner с begin-account,
   accept-public-chat и finish-account интерфейсами, load при установке account,
   save на exit/relog; accept-public-chat принимает только явно типизированный
   подтверждённый публичный текст, никогда произвольный text-field buffer.
   Подключение будущего chat submit остаётся явным незавершённым consumer.
   По решению 2026-11-09 core/lifecycle делается сейчас, подключение accepted
   chat input и полное flow evidence — C028, без новой chat UI внутри C003.
2. **Export producer отсутствует.** Native-run явно сохраняет export requests для
   позднего consumer; tomb/message/screenshot exporters в production SV не найдены.
   Предлагаемый минимум C003 — public file publication operation с generated/explicit
   режимом, результатом needs-replace и отдельными confirm/cancel continuation,
   реальными owned bytes и безопасным commit. Его core проверяется прямо, но
   UI/user export flow остаётся pending до подключения настоящего producer.
   Не добавлять screenshot/tomb/message exporter лишь ради вызова тестового helper.
3. **Reporting частично существует.** Audio consumer реально работает в login.
   Start-play font/graphics содержит `sv_stub_play_graphics`; выбранный UI text font
   не является effective map resource. PKT_VERSION dispatcher/Send_version и
   отдельного Send_font пока нет. Предлагаемый минимум: SV-local snapshot фактических
   ресурсов (audio из existing owner; map/graphics — честные baseline missing/disabled
   values, пока consumer отсутствует), version serializers и вызов на server version
   request; resource-change notification вызывает font serializer только после
   подтверждённой смены effective map identity. Никакого нового renderer. Byte
   evidence возможно через production protocol API; полнота active map branches
   остаётся pending до появления consumer в C006/C007 по утверждённому решению 2026-11-09.
4. **Message effects имеют подходящий seam.** Подключать в `sv_app_step` после
   успешного apply occurrence (через явный file/effect interface), либо выделить
   production delivery owner, вызываемый там же. Не захватывать очередь из renderer:
   `sv_app_take_message` уже существует, diagnostic loop лишь опустошает её.
   Сохранить единственного потребителя и не воспроизводить эффекты на observer.

## Решение пользователя и завершение связок — 2026-11-09

Утверждены файловый core и текущие production consumers; поздние consumers и
flow evidence остаются в своих этапах. C003 принят пользователем 2026-11-09. Прежнее
предложение создавать узкие chat/export consumers внутри C003 заменено этим
решением; повторное согласование развилки не требуется. [Документы владельцев и условия завершения](README.md)
содержат обязательства непосредственно; общий verification список исторический
и не является prerequisite чтения.

Дополнительная сверка исходников через существующий Graphify query `chat history
export exit log resource reporting C028 C029`: 175 nodes, вывод усечён до 83;
граф не давал точных callers, утверждения проверены по текущим исходникам.

- `session/session.h`: SV_MESSAGE_LINES=6; это live feed, не полный recall.
  C016 обеспечивает full historical data для C029; clone/notes не зависят от
  удаления строк из feed и не повторяются при recall/redraw.
- `login/flow.c` передаёт basename flow->font_resource в start_play; native-run
  устанавливает этот resource из UI text font. Это false map identity, подлежит
  исправлению C003-005; текущий map/graphics stub не является effective consumer.
- `Send_version`: strict >4.8.0, >4.9.1, >4.9.2; inclusive pack layout с
  4.9.2.1.0.1, удаление branch/build с 4.9.2.1.0.2. `Send_audio`: >=4.7.3,
  metadata >=4.9.2.1.0.1. `Send_font`: >=4.8.1.2. Все %s copies <=79+NUL.
  Version/font handlers реализуются в C003 production protocol; active map
  identity и resource reinitialization evidence — C006/C007, не snapshot-only PASS.
- `c-init.c:quit_hook`: Net_cleanup → quit reason → optional do_save_chat →
  do_save_chatinput → bookmarks. `do_save_chat` использует полный message_num и
  dump_messages_aux, при chat-only проверяет important scrollback, а не input
  history. Export полученного chat/message log и сохранение введённого текста —
  разные операции. В SV нет настоящего exit-log producer; invocation.chat_log_mode
  только сохраняется C002 и отмечается deferred. C014 владеет quit/reconnect,
  C016 — full recall data, C029 — formatter/publication. Доступное сейчас account
  history save не откладывается из-за будущего export.
- CLI `-v/-V/-x` задают chat-only/all/skip; обычный prompt учитывает quit_no_prompt
  и наличие сообщений. C014 сохраняет конечный parent, C029 завершает child,
  C032 даёт final-review/tomb callers. Поздний порядок evidence не создаёт новый
  canonical prerequisite или перенос ID.

Ни история на 16 строк, ни шестистрочный feed, ни искусственный exporter, ни
serializer из выдуманной active identity не закрывают зависимые flow obligations.
Core evidence учитывается отдельно, missing/disabled branches не принимают
будущие enabled branches. Эта актуализация — только документы, без runtime tests.

## Реализация — 2026-11-09

Исходные наблюдения выше описывают стартовую базу. Current SV-local file/history/
DNA core и app effects/reporting реализованы: [evidence](../../sv-c003-evidence.md).
`flow.c` больше не отправляет UI text font как map font, version/font handlers
добавлены. Ordinary native gameplay adoption по-прежнему отсутствует; app core
и native startup доказательства различаются. Public chat submit, full recall и
export producers не добавлены. C003 принят пользователем 2026-11-09; canonical JSON неизменен.
