# 15: Индексировать последний resolved Wayfinder child

Type: repair
Status: resolved
Assignee: codex/sandcastle-f5fbf91a-wave-12-1
Labels: bug, ready-for-agent
Finding IDs: D-d6785374d0ab1f4c

**What to build:** Effort map снова является полным индексом принятых решений:
завершённый focused-coverage ticket 14 и все последующие resolved children
перечисляются в `Decisions so far`, при этом существующие статусы, metadata,
Answers и история children остаются без изменений.

**Blocked by:** 14: Verify focused production coverage.

**Production seam:** отсутствует — это repair только Wayfinder ledger. Он не
меняет account behavior, production tests или acceptance evidence.

- [x] В `Decisions so far` добавлена рабочая ссылка на resolved ticket 14, а
  structural check подтверждает, что каждый resolved child текущего effort
  индексирован ровно один раз. [Contract: P2-AC12; check: `sv-core`]
- [x] Ticket 14 сохраняет существующие `Type`, `Status`, `Assignee`, `Labels`,
  acceptance/result text и историю; остальные children также не переводятся в
  другой status и не проходят повторный triage. [Contract: P2-AC12, P2-AC13;
  check: `sv-core`]
- [x] Repair не меняет production code, tests, task acceptance record или
  immutable contract и не заявляет Linux native, Windows native либо SV-B-025
  integration выполненными. [Contract: P1-11, P1-12, P1-13, P2-AC1,
  P2-AC2; checks: `sv-build`, `sv-core`, `P1-linux-native`,
  `P1-windows-native`, `P1-entry-integration`]

## Answer

В `map.md` раздел `Decisions so far` дополнен рабочей относительной ссылкой на
resolved ticket 14. Существующие записи 01–13 сохранены; open tickets 16 и 17
в индекс принятых решений не добавлялись.

Production seam отсутствует по контракту этого ticket: repair меняет только
Wayfinder ledger и не затрагивает account behavior, production code или tests.
Поэтому TDD red → green для behavior не применялся и test-only seam не
создавался. Вместо этого выполнена сфокусированная read-only structural
verification состояния ledger:

- до изменения `python3 -B` structural check — **fail**, exit 1:
  `resolved=14`, `indexed=13`, единственная missing link —
  `14-focused-test-coverage.md`, duplicates и non-resolved links отсутствуют;
- после resolution тот же structural check с проверкой существования targets —
  **pass**, exit 0: `resolved=15`, `indexed=15`, missing, duplicates,
  non-resolved и broken links отсутствуют;
- `git diff --check` — **pass**;
- scope inspection — изменены только effort map и этот ticket 15.

Ticket 14 не редактировался: его SHA-256 до и после repair —
`67f893f0f2312ec11c9b3e8ca2257b333434e45b6d4246476b9fe10c62152a18`.
Его Type, Status, Assignee, Labels, acceptance/result text и история, как и
metadata/история остальных children, сохранены. Immutable acceptance contract
также не менялся (SHA-256
`d9e2226352ab7fcd75d57eb9830660bb9caa1a7f3a7f51baac20335cc7e4c5db`).

Production tests, task acceptance record и полный build/test/review workflow не
запускались и не менялись: post-assembly gates остаются за orchestrator.
`P1-linux-native`, `P1-windows-native` и `P1-entry-integration` остаются
**deferred; not run** у прежних owners; claims о Linux native, Windows native
или SV-B-025 integration не сделаны. Новых improvement opportunities в scope
этого ledger repair не обнаружено.
