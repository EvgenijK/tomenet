# 40: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-26-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed by `codex/sandcastle-7b133980-wave-26-1` before
  coverage inspection. The contract-approved production seam is public
  `sv_endpoint_run(SvEndpointOptions)` through authoritative decoded setup,
  native SDL birth input and the production interaction/input router. Review
  of resolved records 38–39 found one missing focused case: rejecting a new
  owner-local automatic-DNA command must preserve an existing partial ordered-
  stats draft and a non-default highlighted stat identity.

## Answer

Проверены resolved implementation record 38 и tracker-only record 39. Existing
coverage record 38 проверял owner-local `%` только на пустом ordered-stats draft,
хотя его contract требовал сохранить partial/committed draft и highlighted
identity. Добавлен focused scenario
`auto-dna-ordered-local-partial-reject` для этого отсутствовавшего состояния.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative ordered-stat setup; реальные SDL события доводят flow до
stats, подтверждают первый stat, выбирают другой highlighted stat, вводят `%` и
завершают через quit. События проходят native birth adapter и production
interaction/input router. `SvEndpointOutcome` подтверждает прежнего owner,
non-default highlight, partial и committed draft, legal list, выключенный
`auto_reincarnation` и отсутствие stats/mode completion. Peer после разрешённого
name packet требует EOF, поэтому partial/completed-character send провалил бы
scenario. Test-only behavior path и fixed-response wizard не добавлены;
production, legacy и shared runtime files не изменялись.

Это coverage-only test уже реализованного поведения. Новый scenario прошёл с
первого запуска, production defect не обнаружен, поэтому TDD red → green claim
не делается.

Focused verification, 2026-10-08:

- `SV_BIRTH_CASE=auto-dna-ordered-local-partial-reject python3 -B
  tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0 в default и
  `SV_CLASS_BEFORE_RACE=0` configurations с `-Wall -Wextra -Werror` и
  ASan/UBSan.
- Тем же production runner отдельно проверены
  `auto-dna-ordered-local-reject`, `auto-dna-ordered-local-manual` и
  `auto-dna-ordered-keyboard` — **PASS**, exit 0 каждый в обеих compile
  configurations.
- `git diff --check` — **PASS**, exit 0.

Повторяющиеся diagnostics declared font fallback и SDL `Leaked thread`
совпадают с существующим focused-runner output; assertion, compiler и sanitizer
failures не было. Новых unrelated improvement opportunities не обнаружено.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review
workflow не запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** до full acceptance и не объявляется выполненным.
