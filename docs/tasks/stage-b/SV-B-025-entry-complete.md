# SV-B-025 — M2: account/character creation screen flow

Проверки платформ: [текущее правило](../../agents/platform-validation.md); Windows/MinGW/Wine отложены и не блокируют текущую приёмку.

Статус: accepted — принято пользователем 2026-10-28 вместе с этапом B.
Оставшиеся проверки сняты с текущего gate прямым решением пользователя;
они не объявлены выполненными. [Решение о приёмке](../../sv-stage-b-acceptance.md#решение-пользователя--2026-10-28).

Исторический статус до решения пользователя: production integration implemented; Linux controlled-peer и native X11
проверки пройдены (2026-10-28); полная Linux acceptance pending.
Windows/MinGW/Wine отложены пользователем и не блокируют текущую приёмку.

## Пользовательский результат

Один production executable проходит оба обязательных пути Stage B:

- existing account → existing character selection;
- new account или existing account → new slot/name → birth choices → final
  acknowledgement.

## Зависимости и граница

Зависит от [SV-B-020](SV-B-020-first-session.md),
[SV-B-021](SV-B-021-account-manage.md) и
[SV-B-024](SV-B-024-birth-dna.md). Milestone не владеет capability IDs.

Успех означает полностью рабочие native startup screens через production
UI/model/input/protocol path и controlled peer. Он не означает real-server
compatibility, startup FILE/Lua/profile, `session.enter-game` или первый gameplay
screen; эти результаты принимаются в C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
<!-- Нет: это integration milestone без собственного capability ownership. -->
<!-- owned-capabilities:end -->

## Production SV проверки

1. Existing и creation routes с exact wire bytes, все legal version/build
   branches и ни одного terminal fallback.
2. Cancel/back/retry/rejection/disconnect на каждом surface восстанавливают
   правильный parent, focus, draft/selection и session generation.
3. Physical keyboard/input method и мышь, resize/minimize/restore, split/chained
   packets, keepalive/ping и provider completion не меняют семантику.
4. Final acknowledgement и peer-driven MOTD оставляют клиент в live-session
   handoff с disabled gameplay input и не требуют map/HUD.

Evidence возвращается каждому implementation owner; milestone не принимает
capability по одному happy-path screenshot.

## Comments

2026-10-13: по решению пользователя запуск Sandcastle `3fd6b311` отменён и
удалён вместе с использованными рабочими деревьями. Задача считается не начатой;
результаты отменённого запуска не засчитываются как выполнение или приёмка.


## Реализация и проверки — 2026-10-28

Оркестратор выполнил реализацию, отдельные Standards/Spec review, исправления
и повторные проверки. Исходный HEAD `d9862f29e43905a3dc87ce9b08c0e1970c25094c`;
проверялось рабочее дерево с сохранёнными пользовательскими изменениями B-024.
Изменения и удаление прежнего Sandcastle не откатывались.

Изменения текущего milestone изолированы в SV:

- Клик по отображённой строке overview выбирает server-owned character slot
  через существующий production login interaction.
- SDL text input включается только для credentials и активного редактора имени,
  включая восстановление draft. Командный Shift+N/E больше не добавляет
  открывающую букву в имя. Cancel/submit/failure/teardown выключают text input;
  retry снова включает его при входе в credentials.
- Имя и feedback размещены над списком персонажей; общие SV-local координаты
  используются renderer и hit testing. Encoding/clipboard error заменяет
  подсказку, не перекрывая имя.
- Минимальные существующие core-проверки дополнены mouse row mapping и
  наблюдением реального SDL text-input lifecycle в production endpoint.
  Устаревший lowercase-q жест в recovery fixture заменён на baseline Shift+Q.

Уже находившиеся в рабочем дереве B-024 fixes (MOTD только после play ACK,
обработка END, закрытие disconnected Guide) сохранены и проверены вместе с M2.

### Результаты

| Проверка | Результат |
|---|---|
| Linux production build, `make -s -C src -f makefile.sv tomenet-sv -j4` | PASS |
| `tests/sv_login_checks.py` | PASS: protocol/interaction/native core |
| `tests/sv_login_live_checks.py` | PASS: standalone existing route / rejection |
| `tests/sv_first_session_checks.py` | PASS: success/disconnect/quit/retry |
| `tests/sv_account_create_checks.py`, `tests/sv_account_failure_checks.py` | PASS: provider completion и отказы |
| `tests/sv_ordinary_create_cancel_checks.py` | PASS: ordinary/dedicated availability и cancel |
| `tests/sv_name_rejection_recovery_checks.py` | PASS: 6 recovery/generation сценариев |
| `tests/sv_name_editor_submit_checks.py` с `SV_BIRTH_CASE=mouse,session-interleave,session-stale-input,session-disconnect,session-retry-generation,body-back,stats-back,mode-back` | PASS: focused input/back/generation через production endpoint, оба build orders где применимо |
| `tests/sv_birth_complete_checks.py` | PASS: waiting/partial/ack, exact bytes |
| `tests/sv_birth_version_checks.py` | PASS: 40 version-boundary rows |
| `tests/sv_birth_reply_checks.py` | PASS: 17 authoritative reply outcomes |
| `tests/sv_birth_dedicated_checks.py` | PASS: 5 dedicated outcomes × waiting/ack × 2 build orders |
| `tests/sv_birth_single_submission_checks.py` | PASS: повторные gestures, backtracking, quit/cancel без повторного handshake |
| `tests/sv_guide_motd_checks.py` | PASS: Guide, peer MOTD, ack gating, skip, point-buy; 2 build orders |
| Standards review | PASS: 0 hard violations; feedback overlap устранён |
| Spec review | Исправлен mouse selection; минимальная высота уже ограничена production; блокирующих замечаний после исправлений нет в проверенном Linux scope |
| MinGW production build | NOT PASSED: `check-deps`, отсутствуют i686 SDL3/SDL_ttf/FreeType development packages |

Нативный дополнительный прогон использовал **один и тот же** `src/tomenet-sv`
с SHA-256 `a8221eafafac5e7c02c584d74d08bf6258e2b07eb19bff530c0e6e864c75d358`:
existing account → выбор Hero мышью; existing account → Carol → birth;
ввод нового account/password физическими X11/XTest событиями → пустой overview
→ Carol → birth. Controlled peer подтверждал настоящие request bytes;
creation/play handshake обоих creation routes совпал с literal fixture
`tests/sv/fixtures/birth-current.hex` (16266 bytes).

Визуально просмотрены overview, редактор с `Carol` без лишней N, birth choices,
waiting/partial ACK, peer MOTD и handoff. Выполнены resize/minimize/restore;
запрос размера ниже установленного production minimum ограничивается окном.
В handoff клавиши Up/period/Enter не отправляют gameplay bytes (допускается
только startup keepalive). Map/HUD/gameplay отсутствуют по границе Stage B.
Настоящие OS IME и Windows этим прогоном не подтверждаются.

Локальные артефакты: `/tmp/sv-b025-evidence/` — команды/логи существующих
проверок, `native.py` (одноразовый driver фактического executable),
`native/summary.json`, `.wire` и PNG; `environment.json`,
`source-sha256.json` фиксируют executable/environment и production inputs.
Первые sandbox-denied попытки и устаревшие recovery/native-driver результаты
сохранены; final-* логи и последний native summary содержат итоговые результаты.
Это локальные временные артефакты, не долговременный acceptance archive.

### Handoff исходным owners и оставшиеся ограничения

Evidence возвращено по ссылке на этот раздел: B-020/B-006 — existing mouse
selection и native MOTD/handoff; B-021 — account/provider checks и native новый
аккаунт; B-022 — editor/recovery/text lifecycle; B-023 — choices/back/generation,
Guide и compile orders; B-024 — exact completion/version/reply matrix и native ACK.
Это не меняет ownership capability IDs и не закрывает pending acceptance owners.

Полная Linux-приёмка M2 остаётся pending: нужны OS IME и mouse/render
correspondence при display-scale/pixel-density ratio != 1. Windows native
build/validation отложены пользователем и не блокируют текущую приёмку.
Наблюдение о масштабировании вынесено отдельно в `docs/sv-improvements.md`.
Stage C live packet transfer/real-server compatibility не принимаются M2.


## Cumulative Linux review — 2026-10-28

[Сводный B-075 отчёт](../../sv-stage-b-acceptance.md) учитывает свежие
production-path проверки owners, actual Secret Service и cumulative A
регрессии. Milestone остаётся pending до закрытия актуальных Linux
input/provider и per-obligation evidence требований; Windows deferred.


Уточнение текущего review: формулировка выше «нужны OS IME» не задаёт
отдельную обязательную установку ibus/fcitx или IME matrix. Исходное требование
— physical keyboard/input method в применимом Linux окружении. Отсутствие
IME service само по себе не блокирует B; human native review и текущие
production input obligations остаются применимыми.
