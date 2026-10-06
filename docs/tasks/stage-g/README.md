# Этап G: Guide

Статус: specified; реализация и runtime evidence не выполнены.

План покрывает 21 outcome, перенесённый из B/D/E/F. Общие
границы и acceptance gate задаёт [спецификация](../../sv-stage-g-spec.md).

| Ticket | Пользовательский результат | IDs | Зависит от |
|---|---|---:|---|
| [SV-G-001](SV-G-001-viewer.md) | Native Guide viewer и все callers | 9 | F |
| [SV-G-002](SV-G-002-bookmarks-sharing.md) | Bookmarks, copy и chat integration | 7 | SV-G-001 |
| [SV-G-003](SV-G-003-update.md) | Checksum, reload, update и Guide build/provider gates | 5 | SV-G-001 |
| [SV-G-004](SV-G-004-acceptance.md) | Удаление placeholder и final acceptance | 0 | SV-G-001…003 |
