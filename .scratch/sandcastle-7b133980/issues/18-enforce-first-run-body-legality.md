# 18: Соблюсти first-run и Archer body legality

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Body choice учитывает production first-run eligibility и текущий class: первый персонаж не может напрямую выбрать Fruit Bat, а random для Archer всегда подтверждает Normal; legal non-first-run manual choices продолжают работать по behavior baseline.

**Blocked by:** 05: Выбрать или пропустить body (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-13-2

Labels: bug, ready-for-agent

**Finding ID:** `D-2a20d07678497370` (`birth.first-run-body-legality`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает first-run eligibility из production character/session handoff вместе с authoritative setup, затем keyboard/mouse/random input проходит native birth adapter и interaction; outcome наблюдает legal body list, highlight и draft. Seam принят P1-AC01, P1-AC07 и P1-AC28; проверка — `sv-core`.

- [x] First-run eligibility приходит из production SV-B-022/session state, а не из test-only option; при first run Fruit Bat отсутствует среди manual legal choices и не может быть подтверждён direct, mouse, random или DNA input. [Contract: P1-AC01, P1-AC07, P1-AC10, P1-AC28; checks: `sv-core`, `controller-review`]
- [x] Для Archer random body подтверждает Normal даже когда non-first-run manual Fruit Bat остаётся legal; manual compatible choice сохраняет существующую baseline semantics. [Contract: P1-AC02, P1-AC07, P1-AC10, P2-AC04; checks: `sv-core`]
- [x] Production-path scenarios покрывают first-run manual/random/DNA reject, non-first-run manual Fruit Bat и Archer random в обеих compile configurations с stable highlight при reject. [Contract: P1-AC07, P1-AC10, P1-AC11, P1-AC19, P1-AC26, P1-AC28; checks: `sv-build`, `sv-core`]
- [x] Body legality transitions и rejects не отправляют partial/completed character result; DNA persistence и final send остаются вне ticket. [Contract: P1-AC02, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Реализация и production-path tests остаются по возможности внутри SV и не меняют legacy/shared birth rules. [Contract: P2-AC01, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

Согласованный public production seam: `sv_endpoint_run(SvEndpointOptions)`.
Controlled peer публикует настоящий decoded `SvContactSetup` и authoritative
character overview; `SvPregame.character_count` передаётся в session-owned
`SvBirth`, где из него выводится first-run eligibility. Keyboard и mouse events
проходят через production `sv_native_birth_event()` и `sv_birth_command()`, а
legal body list, stable highlight и draft наблюдаются через `SvEndpointOutcome`.
После единственного допустимого name request peer требует EOF, поэтому любой
partial или completed-character send провалил бы scenario.

Реализация остаётся в `src/client/sv/session/birth.{c,h}`. На first run Fruit Bat
не входит в legal list и отклоняется той же production legality проверкой для
direct и DNA input; mouse не может адресовать отсутствующую строку, random
подтверждает единственный legal Normal. Для non-first-run сохраняются обе manual
body choices. Archer random подтверждает Normal без изменения manual legality.
В alternate class/race order body-policy помнит identity последнего
подтверждённого class при возврате к body, но существующая draft semantics
по-прежнему очищает выбранный class на Backspace.

TDD red → green, 2026-10-07:

- First-run slice, seam `sv_endpoint_run`: `SV_BIRTH_CASE=first-run-body-direct
  python3 -B tests/sv_name_editor_submit_checks.py` — **RED**, default binary
  остановился на `birth_legal_body_count == 1` (фактически список содержал
  Normal и Fruit Bat). После минимального first-run filtering тот же command —
  **GREEN**, exit 0 для default и `SV_CLASS_BEFORE_RACE=0`.
- Archer slice, тот же seam: `SV_BIRTH_CASE=archer-body-random python3 -B
  tests/sv_name_editor_submit_checks.py` — **RED**, deterministic second random
  выбрал Fruit Bat и assertion `birth_body == SV_BIRTH_BODY_NORMAL` упал. После
  минимального Archer random rule тот же command — **GREEN**, exit 0 для обеих
  compile configurations.

Production-path coverage добавлена в `tests/sv/name-editor-submit.c` и
`tests/sv_name_editor_submit_checks.py`: first-run direct, mouse и DNA rejects со
stable Normal highlight; first-run random Normal; non-first-run manual Fruit Bat;
Archer random Normal в обоих orders. Fixtures различают empty и non-empty
authoritative overview, не добавляя test-only eligibility option или отдельный
choice engine.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0; обе
  compile configurations собраны с `-Wall -Wextra -Werror` и ASan/UBSan, вся
  production endpoint birth matrix прошла.
- `git diff --check` — **PASS**, exit 0.

Полный build/test/review workflow не запускался: assembled gates выполняет
orchestrator. Повторяющиеся diagnostics о fallback font и `Leaked thread` уже
отражены в `docs/sv-improvements.md`; новых improvement opportunities не найдено.
Legacy/shared birth rules не менялись, DNA persistence и final send не
добавлялись.

`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** по исходной
acceptance-phase причине. Этот ticket не объявляет full-acceptance evidence
выполненным; оно должно быть собрано и проверено на точных accepted contract
digest и HEAD full acceptance.
