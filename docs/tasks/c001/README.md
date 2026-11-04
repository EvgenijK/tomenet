# C001 — запуск установленного SV-клиента

Статус: accepted, 2026-11-04. Пользователь явно принял C001 и оба тикета. [Спецификация](../../sv-stage-c001-spec.md).
[Планирование агентов](../../../.scratch/sv-c001-planning/map.md).

Этап владеет одним outcome `capability.platform.package-launch`.
Реализация и техническое Linux evidence выполнены; B/B2 accepted.
Canonical registry пока сохраняет исходные `pending`: machine-readable native
runtime record не создан, локальный отчёт не выдаётся за registry PASS.
[Evidence](../../sv-c001-evidence.md) · [Technical acceptance review](../../sv-stage-c001-acceptance.md).
Только Linux, Windows/MinGW/Wine deferred. Изоляция SV и минимальные проверки
по AGENTS.md обязательны; расширение проверок предварительно согласовывается.

| Ticket | Результат | Зависит от | Ownership | Статус |
|---|---|---|---|---|
| [SV-C001-001](SV-C001-001-package-launch.md) | Linux архив, обычный production запуск, ресурсы/dependency closure и evidence | — | `capability.platform.package-launch` | accepted |
| [SV-C001-002](SV-C001-002-acceptance.md) | Сверка evidence и приёмка C001 | 001 | 0 IDs | accepted |

Последовательность: 001 → 002. Делить один outcome на несколько owners не нужно.
[Coverage snapshot](coverage.json) — локальный planning snapshot с исходными
hashes и exact-once ownership, не runtime evidence и не новая registry schema.

C001 — локальный результат SV-REALLOC-005/006. Общий semantic sweep, specs других
этапов и structural/semantic checker migration остаются незавершёнными в
[общем плане](../stage-reallocation/README.md). Минимальная проверка ссылок и
одного ID этого плана не заменяет будущий общий stage/task checker.


## Реализация 2026-11-04

[Packaging script](../../../tools/package_sv_linux.sh),
[package contract](package-contract.md),
[reconciliation disposition](reconciliation-disposition.md).
Linux Fedora41 архив собран, 8 observations в отдельной runtime environment
дали ожидаемые результаты. OpenGL проверен на Xvfb/Mesa software GL; physical GPU
coverage не заявляется. Пользовательская приёмка зафиксирована
2026-11-04 в [решении](../../sv-stage-c001-acceptance.md). Общая reallocation и чужие outcomes не закрываются.
