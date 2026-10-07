# 21: Сохранить hidden DNA class до подтверждения race

**Type:** implementation

**What to build:** В class-before-race flow выбранная DNA hidden class остаётся обязательной identity до подтверждения race: совместимая race завершает исходный hidden choice, а race, разрешающая другую derived class от того же visible base, останавливает staged или automatic DNA без подмены requested class и оставляет согласованный ручной выбор.

**Blocked by:** 16: Разрешить hidden derived class при выборе race (completed); 19: Продолжить automatic DNA choices через birth flow (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-15-1

Labels: bug, ready-for-agent

**Finding ID:** `D-6d2b2e9f103a1224` (`birth.hidden-derived-class-resolution`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative decoded class/race setup и DNA identities; реальные `#`/`%` SDL events проходят native birth adapter и production interaction/input router, а outcome наблюдает owner, visible highlight и committed draft. Seam принят P1-AC04, P1-AC06, P1-AC10, P1-AC19 и P1-AC28; проверка — `sv-core`.

- [x] При DNA hidden-class choice до race production interaction хранит requested server identity отдельно от visible base highlight и при подтверждении требует, чтобы `resolved_class(visible, selected_race)` совпала именно с requested identity. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Если последующая legal DNA race разрешает другую hidden class от того же visible base, staged `#` и automatic `%` не подменяют class: automatic continuation выключается, owner остаётся на race, а текущие legal highlight и draft сохраняются согласованными для ручного продолжения. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P1-AC26; checks: `sv-core`]
- [x] Production-path regression использует setup, где requested hidden class legal для race A, а DNA выбирает race B с другой derived identity; он проверяет reject mismatch и compatible control case в class-before-race build, а alternate build остаётся green. [Contract: P1-AC04, P1-AC06, P1-AC19, P1-AC26, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Reject и successful continuation не выполняют DNA persistence и не отправляют partial/completed character result; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Исправление и regression остаются по возможности SV-local и не меняют legacy/shared gameplay rules. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` получает
authoritative decoded class/race setup и DNA identities; реальные SDL `#`/`%`
events проходят `sv_native_birth_event()` и production
`sv_birth_command()`. Outcome наблюдает owner, visible highlights и committed
draft, а managed peer требует EOF после единственного name packet и тем самым
отклоняет любой partial/completed character send.

В class-before-race interaction добавлена отдельная `requested_class`: hidden
DNA identity сохраняется независимо от visible base highlight. Race confirmation
сначала вычисляет `resolved_class(visible, highlighted_race)` и выполняет commit
только при точном совпадении с requested identity. Mismatch оставляет owner на
race, legal race highlight и draft без изменений, выключает automatic
continuation и не помечает race DNA успешно применённой. Requested identity
сохраняется, поэтому ручной выбор совместимой race завершает исходную hidden
class. Обычный ручной или random class choice очищает pending identity.

Production regression использует две hidden identities с одним visible base:
class 1 разрешена race 0, class 3 — race 1. Staged `#` mismatch остаётся на race;
compatible `#` control коммитит class 1; automatic `%` mismatch останавливается,
после чего ручной выбор race 0 коммитит ту же class 1. Новые scenarios относятся
к class-before-race build. Существующие hidden-derived и full automatic cases
остаются green в default и `SV_CLASS_BEFORE_RACE=0` configurations. Изменены
только SV birth interaction и его production-path tests; legacy/shared gameplay,
DNA persistence и final-send ownership не менялись.

TDD red → green:

- `SV_BIRTH_CASE=hidden-derived-dna-race-mismatch python3 -B tests/sv_name_editor_submit_checks.py`
  — RED, exit 1: production flow покинул race и assertion owner упал; после
  сохранения requested identity и exact race-time check — GREEN, exit 0.
- `SV_BIRTH_CASE=hidden-derived-dna-race-compatible python3 -B tests/sv_name_editor_submit_checks.py`
  — GREEN, exit 0: compatible race завершает requested hidden class.
- `SV_BIRTH_CASE=auto-dna-hidden-race-mismatch python3 -B tests/sv_name_editor_submit_checks.py`
  — GREEN, exit 0: automatic continuation останавливается, затем manual race
  continuation сохраняет requested hidden identity.

Focused verification, 2026-10-07:

- Все три команды выше — PASS, exit 0.
- `SV_BIRTH_CASE=hidden-derived-dna python3 -B tests/sv_name_editor_submit_checks.py`
  — PASS, exit 0 в обеих compile configurations.
- `SV_BIRTH_CASE=auto-dna-full python3 -B tests/sv_name_editor_submit_checks.py`
  — PASS, exit 0 в обеих compile configurations.
- `git diff --check` — PASS.

Полный orchestrator build/test/review workflow и `sv-build` отдельно не
запускались. `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не
объявляется выполненным. Повторяющиеся diagnostics font fallback и SDL leaked
thread уже учтены прежними задачами; новых improvement opportunities не
обнаружено.
