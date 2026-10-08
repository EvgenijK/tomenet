# 59: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-39-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-39-1` before
  coverage inspection. The accepted production seam is public
  `sv_endpoint_run(SvEndpointOptions)` through authoritative decoded setup,
  native SDL birth events and the public birth interaction/input-router
  boundary. It observes owner, server-provided lists, stable selection, draft
  and outbound-send suppression; it does not replace deferred native runtime
  evidence P1-AC30.

## Answer

Проверены собранные active records 57 и 58. Они изменили SV-local production
birth interaction для сохранения обычных compatible traits в post-mode PvP
Maia и полного server-visible class list с disabled incompatible identities в
alternate race-before-class flow. Legacy и shared production client files в
этих repairs не менялись.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт authoritative race/trait/class setup, PvP availability и DNA
draft; настоящие SDL keyboard, mouse, wheel, PageUp/PageDown, focus, resize,
random, DNA, confirm, Backspace, quit, stale и disconnect inputs проходят
native adapter и public birth interaction/input router. `SvEndpointOutcome`
наблюдает owner, visible/legal identities, stable highlight и draft. Peer после
разрешённого name packet требует EOF, поэтому partial/completed birth send
провалил бы scenario.

Coverage inspection не обнаружил missing test. Committed cases record 57 уже
различают ordinary/dedicated PvP, compatible identities 1/2, optional special
identities 14/15, действительно пустой post-mode list, staged/automatic DNA и
Backspace/Ctrl-Q. Committed cases record 58 проверяют stable disabled letter и
mouse identities, legal subset, двунаправленный PageUp/PageDown через disabled
rows, wheel, interleaved network/focus/resize, random, incompatible DNA и long
visible list. Все проверки вызывают production endpoint/native-input seam;
отдельной test-only реализации поведения нет. Это coverage-only ticket, поэтому
новый red → green цикл и изменение production behavior не требовались и не
заявляются; добавлять дублирующий тест не потребовалось.

Focused verification, 2026-10-08: следующий matrix command завершился **PASS**,
exit 0; runner компилировал production seam в default и
`SV_CLASS_BEFORE_RACE=0` configurations с `-Wall -Wextra -Werror`, ASan и UBSan
и выполнял каждый case в применимых configurations:

`for birth_case in mode-pvp-maia-no-post-traits
exclusive-pvp-maia-no-post-traits unknown-setup-pvp-maia-no-post-traits-back
auto-dna-dedicated-pvp-maia-no-post-traits mode-pvp-maia-empty-post-traits
mode-pvp-maia-trait-reenter alternate-visible-class-letter
alternate-visible-class-mouse alternate-visible-class-viewport alternate-random
alternate-class-dna-incompatible long-class-viewport; do
SV_BIRTH_CASE="$birth_case" python3 -B
tests/sv_name_editor_submit_checks.py; done`

Первые шесть cases покрыли ordinary/dedicated post-mode Maia, keyboard/mouse,
random, staged/automatic DNA, Backspace/Ctrl-Q, special-trait re-entry и empty
list control. Следующие пять покрыли stable visible/legal class identities,
disabled letter/mouse rejection, legal confirm, paging/wheel/interleaving,
random и incompatible DNA в alternate order. `long-class-viewport` сохранил
long-list regression в обеих configurations. `git diff --check` — **PASS**.

Повторяющиеся declared-font fallback и SDL `Leaked thread` diagnostics совпали
с известным focused-runner output; assertion, compiler и sanitizer failures не
было. Новых unrelated improvement opportunities не обнаружено. Полные
`sv-build`, `sv-core`, controller review и общий workflow не запускались: их
выполняет orchestrator после assembly. `P1-birth-runtime-evidence` / P1-AC30 для
accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.
