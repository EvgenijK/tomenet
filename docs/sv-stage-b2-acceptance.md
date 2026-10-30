# B2 — интеграция и visual/input review

Статус: accepted, 2026-10-30. Пользователь явно принял этап B2 после запуска
клиента и исправления буквенных хоткеев выбора режима. Приёмка включает
SV-B2-001…006 и представленные корректировки. Текущая платформа — Linux;
Windows/MinGW/Wine остаются отложены по правилу платформ.

Последняя корректировка: буквенный хоткей режима сразу подтверждает выбор и
переводит к следующему шагу без Enter. Минимальная core-проверка production-пути
в `tests/sv/birth-mode-arrows.c`: RED до правки, GREEN после (шесть режимов,
включая Shift+H); Linux-сборка `src/tomenet-sv` выполнена успешно.

Ниже сохранена история реализации и проверки. Формулировки об ожидаемой
приёмке относятся к датам соответствующих записей; итоговое решение выше
заменяет их. Исторические fingerprints и ограничения evidence сохраняются.

## История до приёмки

[Show/Hide и настройка соединения](sv-b2-connect-followup.md): исправлено и проверено, приёмка пользователем ожидается.
[Клавиатура и возврат по Esc](sv-b2-keyboard-followup.md): исправления подготовлены и проверены; причина прерывистости пользовательских одиночных нажатий требует подтверждения.
[Character Overview](sv-b2-overview-followup.md): оформление, положение колонок и ограничение контента исправлены; пользовательская приёмка ожидается.

[Account screen follow-up](sv-b2-account-followup.md): исправлено по следующему сравнению скриншотов; приёмка пользователем ожидается.

Дополнительные замечания по сравнению двух скриншотов: [исправления и проверки](sv-b2-design-followup.md), 2026-10-29; пользовательская приёмка ожидается.

2026-10-29. Статус: corrections implemented, повторный user review ожидается.
[Исправления пользовательских замечаний](sv-b2-006-corrections.md) реализованы
и проверены; последующие [blur/double click/Back defects](sv-b2-interactive-bugs.md)
также исправлены. Приёмка B2 не объявлена. B остаётся accepted с прежними границами. Linux — обязательная
платформа; Windows/MinGW/Wine deferred по [правилу платформ](agents/platform-validation.md).
Новые capability IDs, protocol/gameplay scope и allocation не добавлялись.

## Production executable и среда

Все новые native captures выполнены одним production `src/tomenet-sv`, скопированным
без изменения bytes в `/tmp/sv-b2-006-capture` для независимости от сборки.
Final SHA-256 `65b6798b679a48c92f9da05195a7d8d3e34485d4145aa58b12b7ea8bad58450a`;
build ID `linux-bd7f9b5e26c5789bb7e9`. `--fixture-window` — существующий production
CLI выбор windowed geometry, не preview executor и не patched binary.

Linux X11/Xwayland, `DISPLAY=:1`, SDL3 3.4.16, SDL3_ttf 3.2.2, FreeType 26.6.20,
libpng 1.6.58, libsecret 0.21.7; clang 22.1.8. Native B2 resources — bundled
`lib/xtra/sv/login/`, Liberation Mono и Liberation Serif, background 0
(`cirith_ungol_ascii_seed06.png`), reduced motion. Profile временный; ожидаемый
warning об отсутствующем profile CascadiaMono ведёт к declared fallback.
Credential provider для replay отключён недоступным D-Bus address, статус session-only
честный. Password captures masked; secret fields в report/transcript отсутствуют.

Frozen D reference — bundled `lib/xtra/sv/login/reference/`, upstream HEAD
`1bb2d5d45b9001bf2a6f2084f78184c29a73fca7`. Exact HTML/CSS/data/assets hashes —
[resource manifest](../lib/xtra/sv/login/SHA256.json), provenance и финальный cascade —
[reference](tasks/stage-b2/reference.md). MOTD C — approved prototype commit
`e271c582cbf89d404deda6189881e531d3775770`; [capture description](sv-b2-005-evidence.md).
Browser reference использует Liberation Mono, DPR1 и тот же logical viewport;
prototype demo data не становится native authoritative data.

## Coverage по reference mapping

Таблица даёт owner и наблюдаемый результат для каждой строки frozen mapping.
Ссылки reference ведут на уже сохранённые exact frozen-reference captures предыдущих
B2 tickets; новые native кадры ниже сняты final executable. Во всех строках 1440×900
и 1024×768 сохраняют одну logical geometry; поддержка minimum просмотрена отдельно.

| Mapping row / owner | Reference | Final native / результат |
| --- | --- | --- |
| Shell/branding/background/Chronicle/links, 001 | [D 1440](evidence/sv-b2-001/reference-1440x900.png), [D minimum](evidence/sv-b2-001/reference-1024x768.png) | [wide shell](evidence/sv-b2-006/software-existing-overview-1440x900.png), [compact shell](evidence/sv-b2-006/software-existing-overview-1024x768.png). Logo, artwork, side areas и compact links присутствуют; Chronicle/links unavailable явно показаны. Animation/lifecycle — bounded B2-001 evidence; новые кадры static reduced-motion. |
| Server list selected/empty/pending/unavailable, 002 | [server](evidence/sv-b2-002/reference-server-1440x900.png) | Final geometry frames — [1920 software](evidence/sv-b2-006/software-1920x1080-scale1-server.png); server rows следуют input list, telemetry unavailable честно. Selected/empty/pending core coverage сохранена в existing endpoint suite; не все эти transient states пересняты. |
| Direct editor/error/cancel, 002 | [manual](evidence/sv-b2-002/reference-manual-1024x768.png) | Fractional actual hit evidence ниже; existing editor core сохраняет validation/cancel. Native адреса реальные input, browser demo endpoint не используется. |
| Account/password/auth/create/validation/provider, 002 | [account wide](evidence/sv-b2-002/reference-account-1440x900.png), [minimum](evidence/sv-b2-002/reference-account-1024x768.png) | [wide masked](evidence/sv-b2-006/software-new-account-masked-1440x900.png), [minimum masked](evidence/sv-b2-006/opengl-new-account-masked-1024x768.png). Actual account/edit/focus/submit; unavailable provider и session-only policy. Reveal остаётся private render-only, его capture не выполняется. Account creation server-confirmation — historical B2-002 account-create suite; в final B2-006 этот suite не повторялся. |
| Connecting/auth wait/failure/retry, 002/005 | D loading-mask family; native adaptation | Final scaled failure/Retry evidence ниже. Factual native reason и caller actions; transient connecting frame отдельно не удерживался, core ownership проверен existing suite. |
| Character overview selected/empty/full/disabled, 003 | [overview wide](evidence/sv-b2-003/reference-overview-1440x900.png), [minimum](evidence/sv-b2-003/reference-overview-1024x768.png) | [12-row overview](evidence/sv-b2-006/software-existing-overview-1440x900.png), [last row minimum](evidence/sv-b2-006/opengl-existing-scroll-last-1024x768.png), [empty account](evidence/sv-b2-006/software-new-overview-1440x900.png). Actual roster/slots и disabled availability; no mock roster. |
| Ordinary/exclusive slot/name/rejection, 003 | [name](evidence/sv-b2-003/reference-name-1440x900.png), [error](evidence/sv-b2-003/reference-error-1024x768.png) | [name/edit](evidence/sv-b2-006/software-new-name-edit-1440x900.png), [minimum](evidence/sv-b2-006/opengl-new-name-edit-1024x768.png). Actual mouse Create, text correction и submit Carol. Exclusive/rejection production core — historical B2-003 suites, не новый final-run; historical [visual error](evidence/sv-b2-003/software-error-1024x768.png), без изменения этой family в B2-006. |
| Birth choices/stats/mode/summary/legal/back/confirm, 004 | [class](evidence/sv-b2-004/reference-class-1440x900.png), [stats](evidence/sv-b2-004/reference-stats-1024x768.png) | Final [class](evidence/sv-b2-006/software-new-class-1440x900.png), [race minimum](evidence/sv-b2-006/opengl-new-race-1024x768.png), [stats](evidence/sv-b2-006/software-new-stats-1024x768.png), [mode](evidence/sv-b2-006/opengl-new-mode-1440x900.png), [Back](evidence/sv-b2-006/software-new-back-stats-1024x768.png). Sex/class/race/trait/body/stats/mode кадры обоих backends и размеров рядом; actual keyboard legal IDs, confirm и Back. Point-buy/other ordering — B2-004 evidence + existing core. |
| Birth child confirmation/Guide/quit, 004/005 | [Guide](evidence/sv-b2-004/reference-guide-1440x900.png), [unused warning](evidence/sv-b2-004/reference-warning-1440x900.png) | Final [Guide](evidence/sv-b2-006/opengl-new-guide-1440x900.png), [caller return](evidence/sv-b2-006/software-new-guide-return-1024x768.png). Exact placeholder text. Unused-point child historical [native warning](evidence/sv-b2-004/software-warning-1440x900.png), production core final checks. Quit сохраняет direct caller exit; web quit confirmation не изобретён. |
| Final ACK/partial/error/MOTD/handoff, 005 | [MOTD C](evidence/sv-b2-005/reference-notice-1440x900.png) | Final [waiting](evidence/sv-b2-006/software-new-waiting-1440x900.png), [partial](evidence/sv-b2-006/opengl-new-partial-1024x768.png), [MOTD](evidence/sv-b2-006/software-new-motd-1440x900.png), [handoff](evidence/sv-b2-006/opengl-new-handoff-1024x768.png). Partial reply не открывает MOTD; full ACK разрешает, any-key закрывает, subsequent input не запускает gameplay. Short/wide/unknown/empty MOTD — prior B2-005 frames, decoder unchanged. |

## Два production controlled-peer routes

[Transcript](evidence/sv-b2-006/routes.json) и [capture log](evidence/sv-b2-006/routes-capture.log)
содержат четыре завершённых runs: existing и empty-account/new-character на software
и OpenGL. Local peer посылает real contact/setup/login packets с явными synthetic
display data. Это не real-server/gameplay acceptance.

Existing route: actual keyboard account/password → 12 Hero0…Hero11 → keyboard scroll
до last row → resize minimum/wide → Home/Enter выбирает Hero0 → server acceptance →
23-row colored MOTD → actual A → live handoff; B/Enter затем остаются в handoff.
New-character route: account Unusedaccount с пустым roster (peer подтверждает auth/login, но не выдаёт отдельного account-created результата; это empty-account/new-character route) → actual mouse Create →
редактирование ошибочного имени backspace до Carol → sex/class/race/trait/body →
stat-order → mode → Back к stats → повторный confirm → Guide → Escape возвращает
caller → final submit → waiting → partial ACK → full ACK `02 03 7a` → MOTD →
A → handoff; дальнейшие B/Enter не открывают gameplay.

Peer проверяет reached phases, но не является независимым packet oracle. `.request`
сохраняет первые 16266 bytes потока, включая elapsed-dependent keepalive `0d`;
это не заявление о длине PLAY packet. Остаток потока сохранён в transcript. Exact
final-request bytes и version gates проверяются существующим core oracle, отдельно
от visual replay. В raw stream keepalive не считается пользовательским gameplay.
Deliberate peer teardown после captures может печатать disconnect/error reading play
reply; это не failure завершённого visual route.

## Bounded geometry, input и hover

`SDL_VIDEO_X11_SCALING_FACTOR` задаёт виртуальную display geometry: это не
physical display timing или реальный OS IME. Requested logical и actual capture
различаются, поэтому clamped/stale initial output не считается требуемым viewport.

| Requested geometry | Actual evidence | Result |
| --- | --- | --- |
| Main 1440×900 at100%, minimum1024×768 | Four final controlled-peer routes, обоих размеров | PASS: families и actual route inputs; gross clipping/overlap на просмотренных frames не найден. |
| Software1920×1080 at100% | [SDL readpixels1920×1080](evidence/sv-b2-006/software-1920x1080-scale1-server.png) | Geometry capture PASS. |
| OpenGL1920×1080 logical,3840×2160 output at200% | [SDL readpixels3840×2160](evidence/sv-b2-006/opengl-1920x1080-scale2-server-virtual.png), [metadata](evidence/sv-b2-006/large-virtual.json) | Virtual geometry PASS. Initial managed window был clamped; final attempt unmap/override_redirect/reparent-to-root/map/resize только task-owned window обошёл WM frame. Desktop/global display settings не менялись. |
| Software1024×768 logical at125% | Final [SDL readpixels1280×960](evidence/sv-b2-006/software-1024x768-scale1.25-server.png) и [modal window1280×960](evidence/sv-b2-006/software-1024x768-scale1.25-manual.png) | PASS after correction: actual Direct opens modal; Cancel exits endpoint with result0 (baseline). Actual Retry returns retained account, cleared password; hover distinct. |
| OpenGL1024×768 logical at150% | [modal1536×1152](evidence/sv-b2-006/opengl-1024x768-scale1.5-manual.png) | Actual Direct/Cancel exit0 PASS; [failure](evidence/sv-b2-006/opengl-error-scale1.5-failure.png) → actual Retry → [retained Test, blank password](evidence/sv-b2-006/opengl-error-scale1.5-retry-account.png). |

[Fractional transcript](evidence/sv-b2-006/scales-fractional-replay.json) records actual
Direct/Cancel results; Cancel закрывает endpoint, не возвращает server list.
[Error/hover transcript](evidence/sv-b2-006/error-hover.json) различает результаты:
OpenGL150% paired [Back without hover](evidence/sv-b2-006/opengl-error-scale1.5-account-unhovered.png)
и [with hover](evidence/sv-b2-006/opengl-error-scale1.5-account-hovered.png) отличаются
в Back rectangle. После startup correction Software125% также имеет distinct
[hover](evidence/sv-b2-006/software-error-scale1.25-account-hovered.png) относительно
[unhovered](evidence/sv-b2-006/software-error-scale1.25-account-unhovered.png),
изменён только Back rectangle `(283,837)–(408,875)`; actual nominal Retry click
[возвращает retained Test с пустым password](evidence/sv-b2-006/software-error-scale1.25-retry-account.png).
До correction те же coordinates не срабатывали; это historical failure ниже.

Orchestration scripts сохранены в `.scratch/sv-b2-006/runtime-*.py`; они используют
production executable, existing capture hook, localhost peer и XTest, без второго
client behavior implementation. При display scale SDL X11 заменяет initial native
window ID; capture/input обязаны находить актуальный window принадлежащего PID.
Первая версия replay держала старый XID и получала capture timeout; эти попытки
не считаются product failures. Новых permanent suites/harness нет.

## Review, отклонения и решение

B2-006 выявил отсутствие hover feedback account/manual/character controls.
Минимальная SV-local правка `ui/endpoint-scene.c` добавляет draw-time pointer hover
с production density/display-scale mapping, без изменения selection/focus/actions.
Source review сохраняет отдельные keyboard focus, pointer hover, selected и disabled; paired captures Software125% и OpenGL150% подтверждают hover; palette и control
geometry следуют existing families. Дополнительных suites/harness не добавлено.

Сохраняются явные native adaptations: Liberation Serif вместо browser Georgia;
TTF raster/целочисленные metrics, квадратные borders и arrow-enter; linear-resampled,
shaded background без CSS blur/filter/backdrop; более простой native summary;
`--`/unknown/pending вместо mock numeric values и descriptions без server authority;
masked `*` вместо browser bullets; Chronicle/realm launchers unavailable. MOTD имеет
canonical server RGB вместо demo palette, uniform integer metrics могут оставлять
разные поля; no wrap/crop/client headings. Эти отличия представлены для решения
пользователя, pixel-perfect acceptance не заявляется.

Visual review рассматривает композицию/пропорции/palette/type/spacing/art,
readable text, overflow/scroll, focus/selected/disabled/hover и minimum layout.
Финальное пользовательское решение B2: **ожидается**. Исторический waiver B не
изменён; B2 не расширяет real server/provider/IME/gameplay scope.


## Software fractional correction

Первый scaled replay подтвердил defect production startup: X11 resize выполнялся
асинхронно, а software renderer создавался до его завершения. Window1280×960
сохранял software output1024×768: painting и hit testing использовали разную
геометрию. Nominal125% Retry не срабатывал, hover отсутствовал. Эти initial failure
frames сохранены в [before-scale-fix](evidence/sv-b2-006/before-scale-fix/README.txt),
с exact прежним SHA; это не final PASS.

Single-variable environment probe с SDL synchronous window operations устранил
ошибку; затем production `SDL_SyncWindow` добавлен после startup sizing перед
`SDL_CreateRenderer` в SV endpoint и matching synthetic startup. Это минимальная
SV-local lifecycle correction, без новых input/protocol semantics. Final executable
не содержит probe/instrumentation и не требует environment hint. Диагностика и
результат original deterministic replay — в [validation report](evidence/sv-b2-006/validation.md).
Final visual/input refresh всех четырёх routes, software125%/OpenGL150% input и bounded geometry использует только обычный final executable с одним SHA в metadata. Повторный actual replay на обоих backends PASS; software window/output1280×960 согласованы.

## Existing Linux checks

[Validation report](evidence/sv-b2-006/validation.md) содержит выбранные команды и
результаты: existing endpoint/login/first-session/Guide-MOTD/shell-software — PASS
на initial code; после hover correction и затем SDL_SyncWindow correction affected endpoint и shell-software
повторены PASS. [Final build fingerprint](evidence/sv-b2-006/build-fingerprint-sync.txt),
[build](evidence/sv-b2-006/build-sync.log), [endpoint](evidence/sv-b2-006/endpoint-sync.log),
[shell](evidence/sv-b2-006/shell-software-sync.log). Новые tests/assertions не добавлены.
Exact handshake/versioned assertions относятся к существующему Guide core oracle;
visual peer подтверждает phases и внешний вид. Дополнительные historical проверки
в coverage table явно названы historical, не final-run.


Final scoped whitespace check — PASS; все local report links разрешаются. Evidence
file fingerprints, включая historical before-scale-fix captures, — в
[SHA256 manifest](evidence/sv-b2-006/SHA256.json). Independent Spec/report review
проверяет final refresh отдельно от initial failure material. Graph source update
выполнен orchestrator после final code; semantic refresh документов не заявляется.


## Предварительные замечания пользователя — 2026-10-29

Пользователь уточнил: [приложенный screenshot](evidence/sv-b2-user-review/2026-10-29-web-reference.png)
показывает **веб-макет в браузере**, а не native executable. Это дополнительный
визуальный reference для сравнения. Ниже — пользовательские наблюдения о native
экране выбора сервера, не результаты новой диагностики или исправлений.

| № | Замечание / требуемый результат | Состояние |
|---|---|---|
| 1 | Клиент должен стандартно открываться на весь экран. | При запуске из этого чата агент явно передал `--fixture-window 1440x900`; оконный запуск вызван этой командой. Дефект default fullscreen этим не доказан. Для штатного запуска override убрать. |
| 2 | Анимация над logo использует только `^`; в макете есть в том числе `.`, `)`, `(`. Перенести соответствующий набор символов и анимацию макета. | Исправление не выполнено. |
| 3 | Фоны выглядят некачественно; перенести исходные изображения 1 в 1 без ухудшения качества. | Исправление не выполнено. |
| 4 | World Chronicle должна получать данные из сетевых источников и показывать события в оформлении reference. | Исправление не выполнено. Уточнение пользователя: получение данных реализуется сразу в текущей корректировке B2; offline snapshot не является итоговой реализацией. |
| 5 | Визуальные стили элементов не соответствуют макету. | Требуется сверка styles по приложенному reference; список предварительный. |
| 6 | Для входа на сервер должна использоваться иконка двери из макета. | Исправление не выполнено. |
| 7 | Иконки ссылок должны быть строго круглыми, без приплюснутой нижней части. | Исправление не выполнено. Уточнение пользователя: круглая форма реализуется сразу; асимметричная форма frozen CSS не переносится. |

B2-006 требует устранения и повторного предъявления этих расхождений; прежние
implementation/check/review результаты сохраняются как история и не означают
пользовательскую visual acceptance. Код и tests при фиксации feedback не менялись.

## Корректировки по замечаниям пользователя — 2026-10-29

После фиксации приведённого выше feedback пользователь поручил агентам внести
исправления. [Итоговый correction report](sv-b2-006-corrections.md) описывает
сетевую Chronicle, исходное качество backgrounds, анимацию, server cards/door,
круглые icons и перенос Direct в header с сохранением пяти видимых карточек.
Независимое review и применимые Linux production/input проверки выполнены;
[финальные результаты и fingerprints](evidence/sv-b2-006-corrections/review-validation.md).
Предыдущая таблица отражает состояние на момент сообщения пользователя;
она не является списком оставшихся дефектов после корректировок.
Пользовательское решение о visual acceptance ожидается.

Follow-up 2026-10-30: [экран выбора имени и штатный E_NEED_INFO при создании](sv-b2-creation-followup.md). Linux проверки/ревью завершены; пользовательская приёмка остаётся открытой.

Follow-up 2026-10-30: [единая сводка, initial preview10, рамки и birth hotkeys](sv-b2-summary-followup.md). Linux проверки/ревью завершены; пользовательская приёмка открыта.

Follow-up 2026-10-30: [credentials editing, sex Back, class grid и оформление](sv-b2-editing-followup.md). Linux checks/review PASS, пользовательская приёмка открыта; physical cause первоначального Backspace не доказана.

Follow-up 2026-10-30: [race/trait hotkeys/grid и подтверждённый native keyboard clock guard](sv-b2-race-trait-followup.md). Физический credentials Enter-сценарий пользователь подтвердил исправленным; общая B2 visual acceptance остаётся открытой.
