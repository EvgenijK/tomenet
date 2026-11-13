# Планирование B2 — native startup UI по веб-макету

Дата: 2026-10-28. Запрос пользователя: «запусти агентов спланировать этап B2
с реализацией интерфейса по макету».

Статус: planning complete. Этот effort планирует, но не реализует B2.
Этап B принят пользователем; его снятые проверки не возобновляются.

## Цель

Подготовить спецификацию B2, implementation tickets и критерии визуальной
приёмки native pregame screens по существующему HTML UX prototype.
Canonical B54 и C001–C064 allocation сохраняются.

## Исследования

| Ticket | Ответственный | Вопрос |
|---|---|---|
| [01](issues/01-web-reference-and-screen-scope.md) | b2_ux_plan | Web reference, scope screens/states, утверждённый UX |
| [02](issues/02-native-implementation-seams.md) | b2_native_plan | SV implementation seams и порядок работ |
| [03](issues/03-acceptance-and-stage-integration.md) | b2_acceptance_plan | Visual acceptance и интеграция этапа |

## Decisions so far

- [01: reference и screen scope](issues/01-web-reference-and-screen-scope.md): final D cascade, hashes и assets; MOTD использует отдельный approved C; mock content не создаёт новые outcomes.
- [02: production seams](issues/02-native-implementation-seams.md): SV-local layout/resources/scenes с общей geometry rendering/hit testing и существующими semantic interactions.
- [03: acceptance](issues/03-acceptance-and-stage-integration.md): current native/reference comparison на Linux, reuse affected production checks, без восстановления waived B gates.

## Итоговый план

- [B2 spec](../../docs/sv-stage-b2-spec.md).
- [Шесть implementation tickets и DAG](../../docs/tasks/stage-b2/README.md).
- [Versioned reference и screen/state mapping](../../docs/tasks/stage-b2/reference.md).

План подготовлен; production implementation не начиналась. Canonical JSON,
B accepted status и C001–C064 allocation не изменялись.

## Финальная сверка плана

Acceptance-agent не обнаружил scope/waiver противоречий. UX-agent дополнительно
уточнил final cascade: overview detail aside скрыт, обязательны inline rows;
birth detail остаётся, но отдельного progress/step-list widget нет. Эти правки
внесены в reference и tasks до завершения планирования. Локальные targets
59 ссылок проверены; production code/tests не менялись.
