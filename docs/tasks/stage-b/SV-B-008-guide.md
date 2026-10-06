# SV-B-008 — Guide placeholder

Статус: superseded-to-G 2026-10-05; активной ответственности Stage B нет.

> Guide capabilities перенесены в G. Точная временная placeholder, если её
> вызывает canonical birth obligation, проверяется actual caller в SV-B-023/024;
> этот исторический тикет не входит в активный B graph и не владеет IDs.

## Пользовательский результат

Любая Guide entry point, доступная в B, показывает ровно
`The guide is in development` и возвращает игрока к тому же caller.

## Зависимости и граница

Заглушка — native unavailable-state, не Guide implementation и не fallback.
Она не читает Guide и bookmarks, не запускает search/checksum/update,
не отправляет `PKT_GUIDE` и не входит в terminal adapter.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
<!-- Заглушка не принимает Guide capability IDs. -->
<!-- owned-capabilities:end -->

## Production SV проверки

1. Birth entry показывает точный текст один раз на одно invocation; live/final
   character sheet и server-directed entry проверяются на назначенных этапах.
2. После close/acknowledge сохранены parent, pending selection, draft, focus,
   macro queue и session generation; никакая игровая команда не повторяется.
3. Keyboard/macro/mouse bindings, resize, focus/minimize/restore, interleaved network
   и relog/teardown не открывают content и не создают Guide state.
4. B actual callers проверяются в [SV-B-023](SV-B-023-birth-choices.md) и
   [SV-B-024](SV-B-024-birth-dna.md); остальные — на назначенных этапах.
Полная реализация Guide принадлежит [этапу G](../stage-g/README.md).
