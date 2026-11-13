# C002: CLI/environment branch disposition

Статус: source-backed boundary, 2026-11-05. Не самостоятельная acceptance.
Canonical `settings.cli-overrides` не сужается; parser success не доказывает
эффект отсутствующего consumer. Implementation/evidence сводятся отдельно в
[acceptance](../../sv-stage-c002-acceptance.md).

## Основание границы

[Decision38](../../../.scratch/single-window-sdl3-client/issues/38-reallocate-post-b-by-functional-block.md)
требует сначала загрузки данных, затем минимальной живой сессии; audio, macros,
OS integration и дополнительные возможности идут поздно. Он одновременно
запрещает скрыто вводить prerequisite другого блока и частично принимать
составной outcome под прежним ID. Поэтому необходима проверка **каждого первого
использования данных**: если оно уже существует в production startup/login,
нужен работающий override сейчас; если существует только у ещё не реализованного
позднего outcome, C002 сохраняет invocation data и явно обозначает отсутствие
этого consumer. Его поздняя семантика остаётся обязательной в своём этапе.

Это та же граница, по которой загрузка OPT/defaults не принимает gameplay
эффекты всех options, а initial audio adaptation не принимает полные C055/C056.
Нельзя заменить её общим утверждением «все flags поздние»: endpoint protocol,
credentials, reincarnation request, actual initial audio и текущий DPI startup
уже имеют ранних consumers.

[Settings policy](../../capabilities/settings-policy.md) сохраняет compatible
CLI/environment/root overrides на один запуск, исключает legacy terminal
topology/GCU fallback и запрещает implicit CFG/OPT writes. Неизвестные flags
не становятся молча работающими. Обычный CLI не разрешает создание plaintext
credential CFG; пароль `-l` остаётся private session input.

## Каждая ветка client.c

Источник всех branches: [client.c:1397](../../../src/client/client.c#L1397)–1545;
ниже disposition по consumer, а не только по букве switch.

| Флаг / форма | Baseline effect и source consumer | C002 disposition и production owner |
|---|---|---|
| positional server | Вне login mode присваивает `svname`; endpoint selection | **applicable**: `settings/startup.c` → `config.server/port` → `login/native-run.c` → `sv_endpoint_argument` |
| `-i` | Reset уже прочитанных connection/audio/FPS/palette/graphics/hints; сохраняет real_name/path; reset quiet; не сбрасывает все прочие invocation globals | **applicable**: `sv_settings_startup_arguments`; reset только requested settings и соответствующий invocation subset, без root relocation или file write. Defaults SV имеют приоритет над старым graphics0/Term reset |
| `-fFILE` | `read_mangrc` в текущей позиции CLI; поздний файл может перебить ранний `-p/-F` | **applicable**: `sv_settings_config_overlay_load` → startup reducer; no write/copy в отличие от legacy reader |
| `-pPORT` | game port | **applicable**: startup reducer → actual endpoint; порт участвует в credential identity |
| `-FFPS` | frame scheduler preference | **applicable**: reducer → `SvEndpointOptions.fps` → native frame pacing, `0` unlimited |
| `-PPATH` | library selection | **applicable**: отдельный `invocation.library_override`; не config path migration и не U/S relocation |
| `-NCHAR` | cname, сам по себе не auto reincarnation | **applicable**: config.name → `SvEndpointOptions.character` → existing login interaction initial character/name |
| `-RCHAR` | `auto_reincarnation=true`, fallthrough в `-N`; [nclient.c:871](../../../src/client/nclient.c#L871) пробует DNA при заданном имени | **applicable** для имени/request: invocation.auto_reincarnation → `flow_poll` → `sv_login_interaction_request_reincarnation`; существующее имя выбирается точно, отсутствующее переносится в обычный New/Enter request с reincarnation intent; **deferred C040** только lookup/load DNA. Missing-name auto-submit без DNA не добавлен |
| `-lACCOUNT` / `-l ACCOUNT` | `modus=2/1`: account, следующий positional password, затем positional server | **applicable**: reducer сохраняет positional state machine; account в config.nick, password отдельно в private invocation buffer; `native-run` передаёт его в `sv_login_flow_create`, который копирует secret в credential owner, затем invocation немедленно wiped. Общий done path также очищает secret. Account-only сохраняется как initial_account для существующей credential формы. PASSWORD_LEN16 ограничивает actual contact payload, secret не попадает в CFG records |
| `-nNAME` | real_name | **applicable**: reducer → `SvEndpointOptions.real_name` → contact identity |
| bare `-n` | fallthrough в `force_cui` | **excluded**: ровно legacy CUI branch, не undocumented synonym другого SV setting; явно сообщить unsupported |
| `-c` | `force_cui=true`; [client.c:1609](../../../src/client/client.c#L1609) пропускает visual modules; также legacy Term presentation branches | **excluded**: terminal topology/GCU fallback прямо исключены settings-policy. Не добавлять второй frontend или притворный CUI mode |
| `-C` | `server_protocol=1`; [c-init.c:4123](../../../src/client/c-init.c#L4123) меняет contact version framing, [nclient.c auth](../../../src/client/nclient.c) — password transformation | **applicable**: существующие `sv_contact_create(server_protocol,...)`, endpoint.protocol и protocol-aware credential validation уже поддерживают1/2. `sv_login_flow_selected` применяет invocation override после любого endpoint selection, включая metaserver; это ранний consumer, не late network feature |
| `-q` | quiet_mode, в конце parse `use_sound=false` | **applicable**: `invocation.quiet` → temporary audio_config.sound0 → actual audio bootstrap disabled; requested CFG sound не переписывается |
| `-w` | temporary `noweather_mode`, отдельный от OPT `no_weather`; [nclient.c:5591/5613/8442](../../../src/client/nclient.c#L5591) suppress weather sound/visual update | **data-only сейчас; consumer deferred C007/C055**: хранить отдельный invocation value, не менять persistent OPT и не переименовывать в cache/audio enable. В early startup weather playback/render consumer отсутствует; pack/device open не является weather event |
| `-u` | `no_lua_updates`; [nclient.c:510](../../../src/client/nclient.c#L510) отвергает PKT_FILE_INIT, фактически не только `.lua` filenames | **data-only сейчас; consumer deferred C010**: отдельный invocation flag; не блокировать обычное чтение local startup data/registry. C009 startup Lua reload не заменяет transfer INIT semantics |
| `-m` | skip MOTD | **applicable**: reducer → options.skip_motd → existing pregame MOTD gate |
| `-v` | `save_chat=1`: chat only при exit | **data-only сейчас; consumer deferred C029/C032**: mode1. [c-init.c:3437–3447](../../../src/client/c-init.c#L3437) принадлежит самостоятельному export/exit lifecycle, не CFG Save |
| `-V` | `save_chat=2`: chat+messages при exit | **data-only сейчас; consumer deferred C029/C032**: mode2, не recorder и не автоматическая settings запись |
| `-x` | `save_chat=3`: без chat save prompt/export | **data-only сейчас; consumer deferred C029/C032**: mode3; current SV не имеет того export prompt. Не создавать ранний message export только для CLI |
| `-X` | `quit_no_prompt=true`; [c-init.c:3332](../../../src/client/c-init.c#L3332) убирает error-screen key wait и :3446 chat-save prompt | **data-only/currently satisfied absence of prompts; consumer deferred C032**: текущий SV не показывает эти legacy prompts. Не трактовать как разрешение удалить unrelated confirm dialogs или autosave CFG |
| `-e` | `freopen("tomenet.log","w",stderr)`; immediate process diagnostic output redirection | **applicable**: это существующий stderr output consumer, а не gameplay export; `sv_endpoint_run` делает actual freopen после успешного parse, до дальнейшего startup, error передаётся общему done path. No secret-bearing CLI dump |
| `-a` | override graphics0 после всего parse, no graphics offer | **applicable requested data**: reducer delayed graphics override0; renderer effect/chooser deferred C007/C056; нельзя включать fake graphics packet report при реально отсутствующем renderer |
| `-g` | override graphics1 после всего parse | **applicable requested data**: delayed override1; тот же consumer boundary |
| `-G` | override graphics2, исходные dual-mask/build/server gates | **applicable requested data**: delayed override2; не сводить mode2 к bool. Actual dual-mask effect deferred C007 |
| `-t` (`USE_GRAPHICS`) | disable_tileset_caching; [main-sdl3.c:2662/2750/3079](../../../src/client/main-sdl3.c#L2662) отключает legacy file/preparation cache | **data-only/inert legacy mechanism сейчас**: сохранить flag и diagnostic; startup SV graphics cache такого типа не использует. Future C007/C003 должен явно решить compatible cache consumer; не отключать unrelated text-font cache |
| `-T` (`TILE_CACHE_SIZE`) | disable_tile_cache; old tile runtime-cache mechanism | **data-only/inert unavailable build mechanism**: соответствует policy для recognizable cache workaround settings; не создавать legacy cache и не объявлять эффект другого cache эквивалентным |
| `-s` (`USE_SDL3`) | ignore_scaling; initial [main-sdl3.c:3724](../../../src/client/main-sdl3.c#L3724) scale1 и :1446 пропуск display-scale changes | **applicable**: текущий SV уже создаёт DPI-aware window и считает `window_scale`, `sv_login_ui_scale`, text/hit-testing projection. `SV_LOGIN_IGNORE_DPI_PROPERTY` хранится у window; `sv_login_display_scale` и `sv_login_ui_scale` используются initial minimum/window geometry, render и pointer routing, включая subsequent display changes. Default обязательный OS DPI handling сохраняется; temporary explicit CLI override не становится saved UI scale |
| unknown `-?` | default branch печатает usage/terminates argument handling | **applicable failure**: reducer rejects, launcher usage/error2; не принимать неизвестный switch как no-op |

## Environment и modern entrypoints

| Источник | Gate и production result |
|---|---|
| `TOMENET_SDL3_USER_PATH` | **applicable**: U до загрузки S/tomenet.cfg, fallback SDL_GetPrefPath; fingerprint всегда U. `--profile-root` имеет explicit invocation priority |
| `TOMENET_PATH` | **applicable**: library fallback перед adjacent lib; `--library`/`-P` override; не активировать imported config.path как relocation |
| `TOMENET_PLAYER`, `TOMENET_USER` | **build-gated**: [client.c:1309](../../../src/client/client.c#L1309) только SET_UID. SV Linux make не включает SET_UID; не добавлять unconditional username/environment leakage. Если SET_UID build появится, нужны те же pre-CFG defaults account/real_name, не late override |
| AMIGA `tomenet_name` | **excluded platform**: не текущий Linux/SDL3 target; не объявлять Linux branch evidence за AMIGA |
| HOME | Legacy X11 rc lookup; **excluded owner для SV CFG**: U/S policy заменяет его. SDL_GetPrefPath может использовать OS environment внутри SDL; не читать ~/.tomenetrc автоматически |
| SDL audio provider/hints | **applicable**: actual audio bootstrap сохраняет higher-priority SDL hint/environment choice; requested/effective snapshot отдельно, buffer hint scoped/restored |
| `--server/--port/--metaserver/--account/--real-name/--character` | **applicable**: existing SV launcher → production ordered reducer → реальные startup/login owners; поздний `-f` может менять ранние assignments согласно baseline order |
| `--password-stdin` | **applicable private input**: separate existing session credential producer; CFG password никогда не fallback |
| `--profile-root/--library/--server-list/--endpoint` | Existing SV startup paths/providers, не imported settings. Root/provider ownership остаётся явным |
| diagnostic flags `--frames/--fixture-window/...` | Existing diagnostic entrypoints, отдельны от перечня baseline CLI; не capability evidence сами по себе |

## Что требуется для закрытия C002

Полный baseline flag union должен быть распознан с exact source meaning и
correct unavailable/excluded diagnostic. Applicable branches выше должны
достигать existing production consumer сейчас; data-only значения принадлежат
invocation snapshot и не пишутся в CFG/OPT. Evidence фиксирует их как retained
для названного late owner, без заявления late effect PASS.

При read-only review реализации проверены `settings/startup.[ch]`,
`login/native-run.c`, `login/flow.c`, `login/characters/login-interaction.[ch]`
и `login/ui/scale.h`. Applicable branches `-l/-R/-C/-s/-e` имеют явные текущие
production paths выше. `flow->options.invocation` удерживает deferred values
до закрытия flow; secret внутри него уже очищен. Единственный call
`SDL_GetWindowDisplayScale` в SV находится внутри общего display-scale getter.

`-e` выполняется после успешного reduce всего argv: diagnostic failure при
невалидном последующем аргументе остаётся в прежнем stderr, а не в `tomenet.log`.
Это ограничение порядка diagnostic output, не изменение connection/resource
settings precedence и не скрытая запись CFG. Повторный `-e` в baseline мог ещё
раз обнулить тот же log; SV открывает его один раз. Secret values не печатаются.

Source review не заменяет runtime evidence: фактические результаты previous
settings core/native startup/login smoke фиксирует основной evidence document.
Дополнительные tests или suite в рамках этой сверки не запускались. Technical
completion по этой границе не означает автоматическую пользовательскую приёмку.

CLI `-R` intent хранится отдельно от видимого имени editor:15-byte имя не
теряет символ из-за служебного `+`. Такой же production submit выставляет
reincarnation flag; manual leading `+` по-прежнему поддержан. Некорректный
request явно отклоняется, а не превращается в другое имя.
