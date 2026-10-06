# 13: Восстановить индекс и metadata Wayfinder effort

Type: repair
Status: open
Assignee: unassigned
Labels: bug, ready-for-agent
Finding IDs: D-d6785374d0ab1f4c

**What to build:** Wayfinder effort SV-B-021 получает обязательную effort map с
индексом принятых решений, а каждый существующий child ticket имеет отдельные
Type, Status и Assignee metadata. Исправление структуры сохраняет прежние
решения, статусы, исполнителей и историю tickets.

**Blocked by:** 10: Verify focused production coverage.

- [ ] Effort map создана в установленном Wayfinder location, содержит
  `Decisions so far` и ссылки на resolved tickets 01–10 без копирования или
  переопределения их ответов. [Contract: P2-AC12; check: `sv-core`]
- [ ] Tickets 01, 02, 03, 04, 06, 08 и 09 получают явный `Type` near the top;
  после repair все child tickets 01–13 имеют отдельные Type, Status и Assignee
  fields согласно local Markdown tracker rules. [Contract: P2-AC12;
  check: `sv-core`]
- [ ] Существующие resolved/claimed history, assignees, Labels, blocking edges,
  acceptance criteria и Answers не переписаны; metadata repair не возвращает
  завершённые tickets во frontier и не выполняет triage заново. [Contract:
  P2-AC12, P2-AC13; check: `sv-core`]
- [ ] Repair не меняет account production code, tests, acceptance evidence или
  explicit deferrals и не заявляет Linux native, Windows native либо SV-B-025
  integration выполненными. [Contract: P1-11, P1-12, P1-13, P2-AC1,
  P2-AC2; checks: `sv-build`, `sv-core`]
