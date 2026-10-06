# 13: Восстановить индекс и metadata Wayfinder effort

Type: repair
Status: resolved
Assignee: codex
Labels: bug, ready-for-agent
Finding IDs: D-d6785374d0ab1f4c

**What to build:** Wayfinder effort SV-B-021 получает обязательную effort map с
индексом принятых решений, а каждый существующий child ticket имеет отдельные
Type, Status и Assignee metadata. Исправление структуры сохраняет прежние
решения, статусы, исполнителей и историю tickets.

**Blocked by:** 10: Verify focused production coverage.

- [x] Effort map создана в установленном Wayfinder location, содержит
  `Decisions so far` и ссылки на resolved tickets 01–10 без копирования или
  переопределения их ответов. [Contract: P2-AC12; check: `sv-core`]
- [x] Tickets 01, 02, 03, 04, 06, 08 и 09 получают явный `Type` near the top;
  после repair все child tickets 01–13 имеют отдельные Type, Status и Assignee
  fields согласно local Markdown tracker rules. [Contract: P2-AC12;
  check: `sv-core`]
- [x] Существующие resolved/claimed history, assignees, Labels, blocking edges,
  acceptance criteria и Answers не переписаны; metadata repair не возвращает
  завершённые tickets во frontier и не выполняет triage заново. [Contract:
  P2-AC12, P2-AC13; check: `sv-core`]
- [x] Repair не меняет account production code, tests, acceptance evidence или
  explicit deferrals и не заявляет Linux native, Windows native либо SV-B-025
  integration выполненными. [Contract: P1-11, P1-12, P1-13, P2-AC1,
  P2-AC2; checks: `sv-build`, `sv-core`]

## Answer

Создан обязательный effort index
`.scratch/sandcastle-f5fbf91a/map.md`. Раздел `Decisions so far` содержит
короткие ссылки на resolved tickets 01–10 без пересказа или переопределения их
`## Answer`; при resolution этого repair добавлена также ссылка на ticket 13 по
правилу Wayfinder. Tickets 11 и 12 остаются open и в индекс принятых решений не
добавлены.

Metadata дополнена только отсутствующими типами: tickets 01–04 и 09 отмечены
как `implementation`, tickets 06 и 08 — как `repair`. Существующие Status,
Assignee, Labels, blocking edges, acceptance criteria и Answers tickets 01–12
не менялись. Ticket 13 был сначала переведён из `open` в `claimed` с assignee
`codex`, затем после focused verification — в `resolved`; triage заново не
выполнялся.

Согласованный production seam из resolved ticket 10 остаётся неизменным:
account flow проходит через `sv_endpoint_run(SvEndpointOptions)`, versioned
protocol — через production `sv_contact_*` / `sv_login_*`, authoritative state
— через `sv_pregame_sync_login` / `sv_pregame_fail` /
`sv_pregame_disconnect`, credential lifecycle — через `sv_vault_store` /
`sv_vault_poll` / `sv_vault_cancel`. Этот ticket меняет только Wayfinder
Markdown metadata, поэтому behavior-changing seam отсутствует и red → green
TDD loop не применялся; production или test-only behavior не добавлялся.

Focused verification:

- read-only `python3 -B` structural check — **pass**: все 13 child tickets
  содержат ровно по одному Type, Status и Assignee field; map содержит
  `Decisions so far`; links 01–10 ведут к существующим tickets со status
  `resolved`; open tickets 11–12 отсутствуют в decision index;
- `git diff --check` — **pass**;
- scope inspection — изменены только effort map и metadata tickets 01, 02, 03,
  04, 06, 08, 09 и 13. Account production code, tests, acceptance evidence и
  explicit deferrals не изменялись.

Полные `sv-build`, `sv-core` и review workflow не запускались: согласно handoff
их выполняет orchestrator. `P1-linux-native`, `P1-windows-native` и
`P1-entry-integration` остаются **deferred; not run** у прежних owners; никаких
Linux native, Windows native или SV-B-025 integration claims не сделано. Новых
opportunities для `docs/sv-improvements.md` в scope metadata repair не
обнаружено.
