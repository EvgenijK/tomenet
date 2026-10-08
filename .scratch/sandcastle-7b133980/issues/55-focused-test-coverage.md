# 55: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-36-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-36-1` before
  coverage inspection. The accepted production seam is public
  `sv_endpoint_run(SvEndpointOptions)` through authoritative decoded setup,
  native SDL birth events and the public birth interaction/input-router
  boundary. It observes owner, server-provided lists, stable selection, draft
  and outbound-send suppression; it does not replace deferred native runtime
  evidence P1-AC30.

## Answer

Проверены resolved active records 52–54. Они изменили SV-local production birth
interaction для trait-derived hidden DNA, invalidation race list после hidden
DNA confirm и снятия post-mode PvP Maia context перед глубоким Backspace.
Legacy и shared production client files не менялись.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт authoritative setup и DNA draft; настоящие SDL DNA, keyboard,
mouse, random, confirm, Backspace, quit, stale и disconnect events проходят
native adapter и public birth interaction/input router. `SvEndpointOutcome`
наблюдает owner, legal lists, stable highlight и draft. Peer после разрешённого
name packet требует EOF, поэтому partial/completed birth send провалил бы
scenario.

Обнаружен и закрыт один focused coverage gap: после staged trait-derived Hell
Knight DNA production `confirm_trait` снимает временное Maia constraint и
перестраивает race list, но существующие tests не наблюдали этот результат при
Backspace. Новый `trait-hidden-dna-race-refresh` scenario даёт visible Paladin
ещё одну legal server-provided race, проходит sex → class DNA → Maia DNA →
Corrupted trait DNA → body Backspace → trait Backspace и проверяет полный list
`[0, Maia]`, сохранённый Maia highlight, class/race draft, keyboard navigation
и отсутствие birth send. Он вызывает только production seam; test-only
реализации поведения нет. Это coverage-only test уже реализованного behavior,
поэтому TDD red → green не заявляется и production code не менялся.

Focused verification, 2026-10-08 — все commands **PASS**, exit 0; runner
компилировал production seam с `-Wall -Wextra -Werror`, ASan и UBSan:

- `SV_BIRTH_CASE=trait-hidden-dna-race-refresh python3 -B
  tests/sv_name_editor_submit_checks.py` — новый class-before-race refresh path;
- те же command/runner для `trait-hidden-dna-{hell-knight,corrupted-priest}` и
  `auto-dna-trait-hidden-{hell-knight,corrupted-priest,wrong-race,wrong-trait}`
  — staged/automatic accept и safe reject в обеих compile configurations;
- те же command/runner для всех шести
  `hidden-derived-dna-race-refresh-*` cases — trait/body/stats и skipped-owner
  refresh, keyboard/mouse/random, illegal/stale input и quit suppression;
- те же command/runner для `unknown-setup-pvp-maia-deep-back`,
  `unknown-setup-pvp-maia-trait-reenter` и
  `unknown-setup-exclusive-pvp-maia-trait-reenter` — ordinary/dedicated deep
  Backspace и restored trait state в обеих compile configurations;
- controls `hidden-derived-dna`, `auto-dna-full`, `mode-pvp-maia-deep-back` и
  `unknown-setup-pvp-maia-back-disconnect` — **PASS**;
- `git diff --check` — **PASS**.

Повторяющиеся declared-font fallback и SDL `Leaked thread` diagnostics совпали
с известным focused-runner output; assertion, compiler и sanitizer failures не
было. Полные `sv-build`, `sv-core`, controller review и общий workflow не
запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.
