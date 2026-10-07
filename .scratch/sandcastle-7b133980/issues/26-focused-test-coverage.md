# 26: Verify focused production coverage

Type: implementation
**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-17-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-07: Claimed by `codex/sandcastle-7b133980-wave-17-1`. The
  contract-approved public production seam is `sv_endpoint_run(SvEndpointOptions)`:
  decoded server setup and authoritative profile/roster state enter the
  session-owned birth interaction, while real SDL events pass through the native
  adapters and production input router.

## Answer

Проверены resolved implementation tickets 21–25: сохранение hidden DNA class до
race, authoritative first-run handoff, production `E` entry в dedicated birth,
per-stat point-buy DNA normalization и canonical Guide-owner boundary. Их
сценарии уже входят в focused production suites. Сопоставление production code,
сценариев и принятого контракта не выявило отсутствующего meaningful coverage,
поэтому новые tests и изменения production behavior не потребовались.

Проверенный public seam — `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт decoded server-provided setup и наблюдает protocol output; production
endpoint переносит profile-owned first-run и login-owned dedicated intent через
`SvPregame` в session-owned birth interaction. Реальные SDL keyboard/mouse
events проходят native adapters и общий semantic input router. Outcome наблюдает
owner, legal identities, highlight, draft, Guide placeholder и lifecycle, а peer
после разрешённого name packet требует EOF, поэтому partial/completed-character
send провалил бы сценарий. Test-only choice engine или fixed-response wizard нет.

Это focused verification уже реализованного поведения. Новый test не добавлялся
и TDD red → green claim не делается: существующая coverage прошла с первого
запуска, дефекта поведения не обнаружено.

Focused checks, 2026-10-07:

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0. Полная
  production endpoint birth matrix прошла в обеих class/race compile
  configurations с `-Wall -Wextra -Werror` и ASan/UBSan; в том числе scenarios
  tickets 21–25, interleaving/disconnect/stale suppression и отсутствие лишних
  outbound bytes.
- `python3 -B tests/sv_ordinary_create_cancel_checks.py` — **PASS**, exit 0;
  authorized exclusive cancel, unavailable slots и first-run rejection не
  начинают character request.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — **PASS**, exit 0;
  exclusive rejection/retry сохраняет production intent.
- `python3 -B tests/sv_login_checks.py` — **PASS**, exit 0; login parser/contact
  handoff остаётся green.

Диагностика fallback font и `Leaked thread` совпадает с уже известным output
focused environment; assertion, compiler и sanitizer failures отсутствовали.
Новых improvement opportunities не обнаружено. Legacy/shared code не менялся.

Полный build/test/review workflow и `sv-build` отдельно не запускались: эти gates
выполняет orchestrator после assembly. `P1-birth-runtime-evidence` / P1-AC30 для
digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** до full acceptance и не объявляется выполненным.
