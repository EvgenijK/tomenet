# SV-G-001 — Native Guide viewer и все callers

> Политика проверок от 2026-10-28: [текущая платформа — Linux](../../agents/platform-validation.md). Windows/MinGW/Wine build, runtime, visual/input и acceptance временно отложены пользователем для всех этапов; они не являются обязательным gate или блокером. Windows-only проверки остаются deferred, без заявления pass; исторические результаты сохранены.

Статус: specified; реализация и runtime evidence не выполнены.

Заменить `The guide is in development` полным native viewer во всех
entry points. Сохранить source bytes, formatting, line identity, search,
resize/reopen state, server packet semantics и точный caller/pending interaction.

## Владение

- `capability.guide.read`
- `capability.guide.navigate`
- `capability.guide.search`
- `capability.guide.close`
- `capability.guide.server-open`
- `capability.guide.mark-results`
- `capability.guide.restore-search`
- `capability.guide.help`
- `capability.guide.context-help`

## Проверки

Все Guide callers из birth, character sheet, skills, lore/options и server-directed
requests открывают нужную тему/строку и возвращают тот же parent.
Missing/corrupt content, failed search, split/chained packet, resize, relog и stale
generation не публикуют partial state и не возвращают placeholder.

