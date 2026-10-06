# 15: Индексировать последний resolved Wayfinder child

Type: repair
Status: open
Assignee: unassigned
Labels: bug, ready-for-agent
Finding IDs: D-d6785374d0ab1f4c

**What to build:** Effort map снова является полным индексом принятых решений:
завершённый focused-coverage ticket 14 и все последующие resolved children
перечисляются в `Decisions so far`, при этом существующие статусы, metadata,
Answers и история children остаются без изменений.

**Blocked by:** 14: Verify focused production coverage.

**Production seam:** отсутствует — это repair только Wayfinder ledger. Он не
меняет account behavior, production tests или acceptance evidence.

- [ ] В `Decisions so far` добавлена рабочая ссылка на resolved ticket 14, а
  structural check подтверждает, что каждый resolved child текущего effort
  индексирован ровно один раз. [Contract: P2-AC12; check: `sv-core`]
- [ ] Ticket 14 сохраняет существующие `Type`, `Status`, `Assignee`, `Labels`,
  acceptance/result text и историю; остальные children также не переводятся в
  другой status и не проходят повторный triage. [Contract: P2-AC12, P2-AC13;
  check: `sv-core`]
- [ ] Repair не меняет production code, tests, task acceptance record или
  immutable contract и не заявляет Linux native, Windows native либо SV-B-025
  integration выполненными. [Contract: P1-11, P1-12, P1-13, P2-AC1,
  P2-AC2; checks: `sv-build`, `sv-core`, `P1-linux-native`,
  `P1-windows-native`, `P1-entry-integration`]
