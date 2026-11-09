# C013 — дополнение: controls на live карте

Статус: planned, 2026-11-09. Это target addendum, не полный план C013 и не
заявление реализации или приёмки live карты.

| Задача | Результат |
|---|---|
| [SV-C013-001](SV-C013-001-network-control-live-map.md) | Actual map ingress → presentation → network controls, ordering и минимальное Linux evidence |

[C008 принят пользователем](../../sv-stage-c008-acceptance.md) в текущих границах
core. Оставшиеся live map integrations и проверки обязательны C013; они не
блокируют и не переоткрывают C008 автоматически. Core/synthetic evidence не
принимает actual live flow. Canonical IDs/obligations/statuses не меняются.
Полный scope `capability.world.read-map` сохраняется за C013; эта задача не
подменяет его будущий полный план. Linux обязателен, Windows/MinGW/Wine deferred.
