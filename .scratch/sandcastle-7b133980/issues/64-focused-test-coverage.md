# 64: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-43-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-43-1` before
  coverage inspection. The accepted production seam is public
  `sv_endpoint_run(SvEndpointOptions)`: authoritative setup and native SDL
  events pass through the production birth interaction/input router, while
  `SvEndpointOutcome` and the controlled peer observe interaction results and
  prohibited outbound sends. This focused seam does not replace deferred native
  runtime evidence P1-AC30.
- 2026-10-08: Active-record inspection found only record 63 before this ticket.
  It added a documentation-only future improvement record and explicitly did
  not change production or test behavior. Existing scenarios already exercise
  both affected hidden-class identities, PvP Maia semantic transitions, a
  non-Maia control and exclusive production entry in both compile orders. No
  focused coverage gap or behavior defect was found, so no test or production
  change and no red → green loop are claimed.

## Answer

Проверен active record 63: он только записал отдельно предлагаемую будущую
SV-local semantic-identity mapping и не изменил production/test code. Поэтому
добавлять тест для документационной записи или создавать test-only mapping seam
не требовалось. Legacy/shared файлы не изменялись; новых improvement
opportunities при этой проверке не обнаружено.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт authoritative server setup; реальные SDL events проходят native
adapter и публичную границу birth interaction/input router. Production outcome
наблюдает owner, selection и draft, а peer допускает только name handoff и затем
требует EOF, поэтому partial/completed birth send провалил бы сценарий.

Focused verification, 2026-10-08:

`for birth_case in trait-hidden-dna-hell-knight
trait-hidden-dna-corrupted-priest mode-pvp-maia mode-pvp-race16-control
exclusive-pvp-maia-production; do SV_BIRTH_CASE="$birth_case" python3 -B
tests/sv_name_editor_submit_checks.py; done`

Результат: **PASS**, exit 0 для всех пяти cases в default и
`SV_CLASS_BEFORE_RACE=0` compile configurations. Проверены обе trait-derived
hidden DNA classes, post-mode PvP Maia selection, non-Maia race identity control
и exclusive PvP Maia production entry. Runner собрал production seam с
`-Wall -Wextra -Werror`, ASan и UBSan; compiler, sanitizer и assertion failures
не было. Повторяющиеся declared-font fallback и SDL `Leaked thread` diagnostics
совпадают с известным focused-runner output.

Это coverage-only ticket для уже реализованного поведения. Первый focused
production-seam запуск был green; дефект не обнаружен, поэтому red → green
цикл и новые tests не заявляются. Полные `sv-build`, `sv-core`, controller review
и общий workflow не запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.
