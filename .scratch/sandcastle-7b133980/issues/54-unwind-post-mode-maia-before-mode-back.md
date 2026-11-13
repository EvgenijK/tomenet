# 54: Снять post-mode Maia context перед глубоким Backspace

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Для PvP Maia с authoritative setup, где ordinary stats и body недоступны, первый Backspace из обязательного post-mode trait возвращает к mode, а следующий Backspace сначала восстанавливает pre-mode ordinary trait state и только затем выбирает предыдущего доступного owner. Synthetic post-mode trait больше не появляется как ordinary predecessor ни в одном compile order; skipped owners остаются skipped, draft и highlight согласованы, prohibited sends отсутствуют.

**Blocked by:** 32: Восстановить обычный owner graph после post-mode PvP Maia (completed); 43: Пропустить пустые stats после post-mode PvP Maia Backspace (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-35-3

Labels: bug, ready-for-agent

**Finding ID:** `D-11ec7c1d1ff9b668` (`birth-post-mode-maia-unavailable-step-unwind`), one coherent repair of the second Backspace and deeper compile-order owner graph.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative `creation_flags > 1`, unavailable ordinary stats/body, server-provided Maia traits и ordinary/dedicated PvP intent; реальные mode, trait и Backspace SDL events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, ordinary/post-mode legal traits, stable highlight, draft и отсутствие запрещённых outbound bytes. Seam принят P1-AC14, P1-AC15, P1-AC18, P1-AC19, P1-AC20 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path cases проходят PvP mode → обязательный Maia trait → Backspace к mode → Backspace глубже при unavailable stats/body; green outcome до вычисления predecessor снимает post-mode requirement, восстанавливает сохранённые ordinary trait availability/highlight/draft и не назначает synthetic Enlightened/Corrupted-only trait list как ordinary owner. [Contract: P1-AC02, P1-AC14, P1-AC18, P1-AC19, P1-AC20, P1-AC28, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] В class-before-race order второй Backspace пропускает недоступные ordinary trait/body и возвращает к race; если ordinary trait доступен, он получает восстановленный pre-mode list. В race-before-class order второй Backspace возвращает к class, а следующий Backspace использует восстановленный ordinary skip state и не входит в synthetic post-mode trait. [Contract: P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC18, P1-AC19, P1-AC20, P2-AC04; checks: `sv-build`, `sv-core`]
- [x] Ordinary и dedicated PvP cases сохраняют подтверждённые sex/race/class values, baseline representation unavailable body/stats и stable ordinary trait highlight; повторное продвижение и PvP mode confirm заново создаёт обязательный post-mode trait только пока mode остаётся confirmed. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC20, P2-AC04; checks: `sv-core`]
- [x] Backspace, reconfirm, illegal input, Q/Ctrl-Q, disconnect и stale-generation handling не отправляют partial/completed character result; final send остаётся у SV-B-024, а existing known-setup post-mode flow остаётся green. [Contract: P1-AC02, P1-AC12, P1-AC14, P1-AC15, P1-AC18, P1-AC20, P1-AC22, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Repair и regression остаются по возможности SV-local, используют production interaction/input-router path и не добавляют test-only owner graph; обнаруженные unrelated improvements записываются отдельно. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим implementation repair; evidence собирается только на exact accepted contract digest и HEAD.

## Answer

Repair выполнен в production SV birth interaction. При Backspace из mode после
возврата из обязательного post-mode Maia trait теперь сначала вызывается
существующий `leave_post_mode_trait()`: он снимает post-mode requirement,
восстанавливает сохранённые ordinary trait highlight/chosen/confirmed state и
перестраивает ordinary legal list, а уже затем mode выбирает predecessor. Поэтому
synthetic traits 14/15 не участвуют в ordinary owner graph; unavailable body,
stats и ordinary trait остаются skipped в обоих compile orders.

Согласованный public seam: `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative `creation_flags = 0x10`, Maia race/traits и ordinary либо
dedicated PvP intent. Настоящие SDL mode, trait, Backspace, reconfirm, illegal,
stale, quit и disconnect events проходят native birth adapter и production
birth-interaction/input-router boundary. `SvEndpointOutcome` наблюдает owner,
legal lists, stable highlight и draft; peer принимает только предусмотренный name
packet, поэтому partial/completed birth send провалил бы scenarios.

TDD red → green, 2026-10-08:

- Red: `SV_BIRTH_CASE=unknown-setup-pvp-maia-deep-back python3 -B
  tests/sv_name_editor_submit_checks.py` — **FAIL**, exit 1: default
  class-before-race binary оставил synthetic post-mode trait predecessor и не
  вернулся к race.
- Green: тот же command после минимального SV-local unwind — **PASS**, exit 0
  для default и `SV_CLASS_BEFORE_RACE=0` binaries.
- Focused green slices: `unknown-setup-pvp-maia-trait-reenter` и
  `unknown-setup-exclusive-pvp-maia-trait-reenter` тем же command с
  соответствующим `SV_BIRTH_CASE` — **PASS**, exit 0 в обеих compile
  configurations. Они покрывают ordinary traits 1/2, восстановленный highlight
  2 и confirmed draft, alternate class → trait unwind, повторный mode confirm и
  новый post-mode list 14/15 для ordinary/dedicated PvP.
- Focused regressions: cases `mode-pvp-maia-deep-back`,
  `exclusive-pvp-maia-deep-back`, `unknown-setup-pvp-maia-back-confirm` и
  `unknown-setup-pvp-maia-back-disconnect` — **PASS**, exit 0 каждый в обеих
  compile configurations с `-Wall -Wextra -Werror`, ASan и UBSan. Они сохраняют
  known-setup flow, first Backspace, illegal/stale handling, reconfirm и
  disconnect send suppression. `git diff --check` — **PASS**.

Legacy/shared gameplay code и test-only behavior не менялись; новых unrelated
improvements не обнаружено. Полные `sv-build`, `sv-core` и controller review не
запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** до full
acceptance на exact accepted contract digest и HEAD.
