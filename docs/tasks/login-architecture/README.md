# Архитектура входа нового клиента SV

**Статус:** implemented; адресные Linux checks passed, 2026-11-04.
План 001…005 реализован; принято пользователем 2026-11-04.
Фактические проверки, review и ограничения — в [005](SV-LOGIN-ARCH-005-integration.md).
Labels: enhancement

Основание: [предложение Codebase Design](../../sv-improvements.md#архитектура-нового-клиента-модуль-входа-и-локальность-сценариев).
В scope только новый клиент SV и необходимые ссылки его сборки и проверок.
Legacy client, сервер и общие модули не реорганизуются и не получают новые
обязанности. Изолированное дублирование ради SV не считается дефектом.

## Порядок и результаты

| Тикет | Результат | Зависит от |
|---|---|---|
| [001 — interface и ownership](SV-LOGIN-ARCH-001-contract.md) | Карта состояния, lifetime и seam модуля входа | — |
| [002 — локальность файлов](SV-LOGIN-ARCH-002-locality.md) | Связанные части входа рядом в `src/client/sv/login/` | 001 |
| [003 — flow](SV-LOGIN-ARCH-003-flow.md) | Существующее управление входом внутри глубокого модуля | 002 |
| [004 — view, input и birth](SV-LOGIN-ARCH-004-view-input-birth.md) | Read-only view отдельно от ввода; backing birth state скрыт | 003 |
| [005 — интеграция](SV-LOGIN-ARCH-005-integration.md) | Проверенная связка и актуальная архитектурная документация | 004 |

Проработка interface, перечня переносов и существующих checks может идти
параллельно. Production-изменения последовательны: один integration owner
редактирует runner, связанные интерфейсы и `src/makefile.sv`; иначе параллельные
правки ownership и путей создадут конфликт. Названия публичных C-типов,
функций описаны в 001; карта текущего размещения — в 002. Private native
adapters синхронно работают с принадлежащими flow interactions.

## Ограничения и соседние задачи

- Сохранить behavior baseline, существующие визуальные решения B2, порядок
  протокола, back/cancel/retry, отклонение имени и generation semantics.
- Flow переносит существующее orchestration; вторая state machine поверх
  прежней, event bus, plugin framework и общий I/O framework не нужны.
- Wire protocol, общие text editing/font/render utilities и credential vault
  сохраняют самостоятельность; физическое соседство не объединяет model,
  interaction, native input и UI из ADR-0001.
- [C001](../../sv-stage-c001-spec.md) владеет default endpoint dispatch,
  packaging и сохранением текущих режимов запуска. Согласовать пути includes,
  build callers и entry point при переносе; C001 не блокирует этот план.
- [Разделение CLI и диагностического launcher](../../sv-improvements.md#разделить-production-cli-и-диагностический-launcher-sv)
  не реализуется здесь; диагностика продолжает использовать текущий путь.
- [Реальный startup-to-live handoff](../../sv-improvements.md#preserve-live-protocol-bytes-at-the-startup-handoff)
  остаётся отдельной возможностью. Здесь описывается контракт будущего владельца
  socket/generation/setup/персонажа/leftover bytes, заглушка не объявляется live runtime.
- Accepted B/B2 и capability allocation не переоткрываются.

## Проверки и принятие

Текущая обязательная платформа — [Linux](../../agents/platform-validation.md).
При подготовке первоначального плана сборки и tests не запускались. После
реализации выполнены Linux build и адресные production-path проверки;
подробности и ограничения — в 005. Для дальнейших изменений использовать
Linux SV build и только адресные существующие production-path checks затронутого
core. Перед изменением зафиксировать минимальный набор и команды в 001;
не запускать автоматически полные исторические matrix. Новые suites и расширение
набора согласуются заранее по AGENTS.md. Для чистого переноса не создавать
тесты расположения файлов или копию поведения в tests.

После 005 каждый тикет содержит фактический diff scope, команды и результаты,
ограничения и ссылки на выполненные зависимости; draft не даёт фиктивный PASS.

Приёмка: пользователь принял правки 2026-11-04 и поручил commit/push.
