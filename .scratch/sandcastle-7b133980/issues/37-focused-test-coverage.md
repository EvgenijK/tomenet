# 37: Verify focused production coverage

Type: implementation
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-24-1
Labels: enhancement

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-24-1` before
  coverage inspection. The contract-approved public production seam is
  `sv_endpoint_run(SvEndpointOptions)` through authoritative decoded setup,
  native SDL birth input and the production interaction/input router. Focused
  inspection identified the missing cross-ticket check: pending automatic DNA
  and a partial ordered-stats draft must survive the canonical `?` no-op with
  no placeholder or outbound send.

## Answer

Проверены resolved implementation records 34–36 и production delta после
focused record 33. Ticket 34 уже покрывал suspension/continuation automatic DNA
на ordered stats, а ticket 35 — canonical `?` no-op на ordered stats без
automatic DNA. Между ними отсутствовал meaningful cross-ticket scenario:
pending automatic reincarnation и частичный ordered-stat draft при `?`.

Добавлен focused scenario `auto-dna-ordered-guide-no-op`. Он использует
публичный production seam `sv_endpoint_run(SvEndpointOptions)`: controlled peer
передаёт authoritative ordered-stat setup и DNA identities, а настоящие SDL
`%`, stat selection, `?` и quit events проходят native birth adapter и
production interaction/input router. `SvEndpointOutcome` подтверждает прежний
stats owner и highlight, неизменные partial/committed draft и legal list,
сохранённый `auto_reincarnation`, отсутствие placeholder open/close и Guide
text. Peer разрешает только name packet и затем требует EOF, поэтому любой
partial/completed-character или Guide-related network send провалил бы
scenario. Test-only choice implementation и fixed-response wizard не добавлены;
production, legacy и shared runtime files не изменялись.

Это coverage-only test уже реализованного поведения. Новый scenario прошёл с
первого запуска, production defect не обнаружен, поэтому TDD red → green claim
не делается.

Focused verification, 2026-10-07:

- `SV_BIRTH_CASE=auto-dna-ordered-guide-no-op python3 -B tests/sv_name_editor_submit_checks.py`
  — **PASS**, exit 0 в default и `SV_CLASS_BEFORE_RACE=0` configurations с
  `-Wall -Wextra -Werror` и ASan/UBSan.
- Тем же production runner отдельно проверены
  `auto-dna-ordered-keyboard`, `auto-dna-ordered-mouse-dedicated`,
  `auto-dna-ordered-mode-missing`, `auto-dna-ordered-mode-illegal`,
  `guide-ordered-stats-no-op`, `stats-point-guide` и `guide-all-owners` —
  **PASS**, exit 0 каждый в обеих compile configurations.
- `git diff --check` — **PASS**, exit 0.

Повторяющиеся diagnostics declared font fallback и SDL `Leaked thread`
совпадают с уже известным focused-runner output; assertion, compiler и
sanitizer failures не было. Новых unrelated improvement opportunities не
обнаружено.

Полные `sv-build`, `sv-core`, controller review и общий build/test/review
workflow не запускались: их выполняет orchestrator после assembly.
`P1-birth-runtime-evidence` / P1-AC30 для accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** до full acceptance и не объявляется выполненным.
