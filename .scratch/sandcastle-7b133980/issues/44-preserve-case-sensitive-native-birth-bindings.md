# 44: Сохранить регистр native birth bindings

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Native birth keyboard bindings определяют регистр буквы по SDL keycode вместе с Shift/Caps modifier state. Только uppercase Q или Ctrl-Q завершают creation, только uppercase H выбирает legal Hellish mode, а direct sex/body/stats/mode и server-list letters принимаются только в baseline lowercase форме; поэтому lowercase q остаётся доступной семнадцатой list binding и никогда не подменяется quit.

**Blocked by:** 09: Завершить или откатить creation без send (completed); 14: Изолировать production birth input context (completed); 29: Сделать длинные birth choice lists полностью достижимыми (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-29-2

Labels: bug, ready-for-agent

**Finding ID:** `D-9ff71cd954b73dcd` (`birth.case-sensitive-key-binding-semantics`), one coherent repair of native case decoding and every affected birth binding.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает server-provided birth lists and mode flags; настоящие SDL keyboard events с keycode и modifier state проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают quit state, owner, highlighted server identity, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC09, P1-AC10, P1-AC11, P1-AC12 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production SDL-event regressions доказывают текущую коллизию; green regressions подтверждают, что unshifted lowercase q не завершает creation ни у одного birth owner, тогда как uppercase Q и Ctrl-Q проходят baseline quit/retry handling без partial или completed send. Shift/Caps case resolution и Ctrl modifier обрабатываются явно, а не выводятся из SDL letter keycode. [Contract: P1-AC02, P1-AC11, P1-AC12, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] В legal long server-provided class, race и trait lists lowercase q выбирает семнадцатую identity со stable highlight и legal confirm, а shifted/caps-uppercase list letters не выбирают lowercase bindings и не меняют owner, highlight или draft. Проверка проходит в обеих compile-time class/race order configurations. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC11, P1-AC19, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Lowercase h не выбирает Hellish, uppercase H выбирает Hellish только когда этот ordinary или dedicated mode legal; остальные direct sex/body/stats/mode letters принимаются только в lowercase baseline form, а их uppercase variants безопасно отклоняются с неизменным owner, highlight и draft. [Contract: P1-AC03, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P2-AC04; checks: `sv-core`]
- [x] Keyboard case repair не меняет mouse navigation/confirmation, random/DNA semantics, Backspace graph, exclusive input ownership, disconnect/stale-generation suppression или запрет partial/completed-character send; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC10, P1-AC11, P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC21, P1-AC22, P1-AC26, P1-AC29; checks: `sv-core`]
- [x] Repair остаётся по возможности SV-local и проверяется через production native adapter и interaction/input router без fixed-response или test-only key decoder; обнаруженные unrelated improvements только записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-29-2`. Agreed TDD seam is the ticket-defined public `sv_endpoint_run(SvEndpointOptions)` boundary with native SDL keyboard events, the production birth interaction/input router, controlled peer observations, and `SvEndpointOutcome`.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative server setup и длинные class/race/trait lists; настоящие
`SDL_EVENT_KEY_DOWN` с SDL keycode и Shift/Caps/Ctrl modifier проходят
`sv_native_birth_event()` и production birth interaction/input router.
`SvEndpointOutcome` наблюдает owner, stable highlighted server identity и draft,
а peer после name packet требует EOF, поэтому любой partial/completed-character
или иной запрещённый outbound send проваливает scenario.

Native adapter теперь явно разрешает регистр как Shift XOR Caps, не выводя его
из SDL letter keycode. Uppercase Q и Ctrl-Q обрабатываются до owner-local letter
bindings; lowercase q остаётся обычной 17-й list binding. Direct sex/body/stats
и ordinary/dedicated mode letters принимаются только в lowercase baseline form,
кроме legal uppercase H для Hellish/IDDC Hellish. Изменения production code
остались в `src/client/sv/input/native-birth.c`; legacy/shared code не менялся.

TDD evidence (каждая команда компилирует и запускает default и
`SV_CLASS_BEFORE_RACE=0` production paths):

- RED: `SV_BIRTH_CASE=case-sensitive-class-binding python3 -B tests/sv_name_editor_submit_checks.py`
  — exit 1, `birth_class_confirmed` false из-за прежнего lowercase-q quit.
  GREEN: та же команда — exit 0 после первого case decoder/class slice.
- RED: focused race и trait команды с `SV_BIRTH_CASE=case-sensitive-race-binding`
  и `SV_BIRTH_CASE=case-sensitive-trait-binding` — exit 1 на неверных stable
  identities; GREEN: обе команды — exit 0 после owner-local lowercase list slices.
- RED: focused `case-sensitive-sex-direct`, `case-sensitive-body-direct` и
  `case-sensitive-stats-direct` — exit 1, потому что uppercase direct letters
  меняли highlight. GREEN: те же команды — exit 0 после соответствующих
  lowercase-only slices; body refinement также выявил и исправил приоритет Ctrl-Q.
- RED: `SV_BIRTH_CASE=case-sensitive-mode-lower-h python3 -B tests/sv_name_editor_submit_checks.py`
  — exit 1, lowercase h выбирал Hellish. GREEN: та же команда — exit 0 после
  разделения lowercase baseline modes и uppercase Hellish.

Focused verification, 2026-10-08:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS, exit 0 для полного
  профильного SV-B-022/023 production suite в обеих compile configurations с
  `-Wall -Wextra -Werror`, ASan и UBSan. Матрица включает lowercase q на всех
  семи owners, Shift-Q/Caps-Q/Ctrl-Q, Shift+Caps parity, long class/race/trait
  q selection, ordinary/dedicated Hellish, uppercase direct rejects и прежние
  mouse/random/DNA/Backspace/Guide/interleaving/disconnect scenarios.
- `git diff --check` — PASS.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review workflow
не запускались по указанию orchestrator. Deferred
`P1-birth-runtime-evidence` / P1-AC30 остаётся за full acceptance owner и здесь
не заявлен пройденным. Новых unrelated improvement opportunities не обнаружено;
`docs/sv-improvements.md` не менялся.
