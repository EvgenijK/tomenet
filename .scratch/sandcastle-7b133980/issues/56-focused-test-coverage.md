# 56: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-37-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-37-1` before
  coverage inspection. The accepted production seam is public
  `sv_endpoint_run(SvEndpointOptions)` through authoritative decoded setup,
  native SDL birth events and the public birth interaction/input-router
  boundary. It observes owner, server-provided lists, stable selection, draft
  and outbound-send suppression; it does not replace deferred native runtime
  evidence P1-AC30.

## Answer

Проверено собранное после предыдущего focused record изменение `0a52014870`
(`fix(sv): derive Maia from server setup`). Production birth теперь находит Maia
по title в authoritative server-provided race setup. Коммит уже содержит
production-path cases с Maia на race 17 и отдельные ordinary/dedicated PvP
controls, где race 16 остаётся non-Maia. Legacy и shared production client files
не менялись.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative setup, PvP flags и DNA draft; настоящие SDL keyboard,
DNA, confirm, Backspace и quit events проходят native adapter и public birth
interaction/input router. `SvEndpointOutcome` наблюдает owner, authoritative
race/trait identities, draft и готовность mode. Peer после разрешённого name
packet требует EOF, поэтому partial/completed birth send провалил бы scenario.
Этот focused seam не заменяет controller-observed native runtime evidence.

Coverage inspection не обнаружил missing test. Уже committed positive cases
проверяют server-provided Maia race 17 в обычном PvP, dedicated automatic DNA,
trait-derived hidden DNA и post-mode race re-entry. Новые committed negative
controls `mode-pvp-race16-control` и `exclusive-pvp-race16-control` проходят
тот же production path с race 16, подтверждают ready mode без ложного post-mode
trait owner и тем самым защищают regression во всех изменённых Maia branches.
Отдельная test-only реализация поведения отсутствует. Это coverage-only ticket;
новый red → green цикл и изменение production behavior не требовались и не
заявляются.

Focused verification, 2026-10-08 — все commands **PASS**, exit 0; каждый runner
компилировал production endpoint seam в default и `SV_CLASS_BEFORE_RACE=0`
configurations с `-Wall -Wextra -Werror`, ASan и UBSan:

- `SV_BIRTH_CASE=mode-pvp-maia python3 -B tests/sv_name_editor_submit_checks.py`;
- те же command/runner для `mode-pvp-race16-control` и
  `exclusive-pvp-race16-control`;
- те же command/runner для `auto-dna-dedicated-pvp-maia`;
- те же command/runner для `trait-hidden-dna-hell-knight` и
  `trait-hidden-dna-corrupted-priest`;
- те же command/runner для `mode-pvp-maia-race-reenter` и
  `exclusive-pvp-maia-race-reenter`;
- `git diff --check` — **PASS**.

Повторяющиеся declared-font fallback и SDL `Leaked thread` diagnostics совпали
с известным focused-runner output; assertion, compiler и sanitizer failures не
было. Полные `sv-build`, `sv-core`, controller review и общий workflow не
запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.
