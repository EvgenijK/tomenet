# SV-G-003 — Guide checksum, reload и update

> Политика проверок от 2026-10-28: [текущая платформа — Linux](../../agents/platform-validation.md). Windows/MinGW/Wine build, runtime, visual/input и acceptance временно отложены пользователем для всех этапов; они не являются обязательным gate или блокером. Windows-only проверки остаются deferred, без заявления pass; исторические результаты сохранены.

Статус: specified; реализация и runtime evidence не выполнены.

Зависит от [SV-G-001](SV-G-001-viewer.md).

## Владение

- `capability.guide.checksum`
- `capability.guide.reload`
- `capability.guide.update`
- `capability.platform.optional-download`
- `capability.platform.optional-gameplay`

## Проверки

Reader/checksum/updater используют один resolved owner. Download проходит
validate→atomic replace; любой provider, TLS, checksum, disk и replace failure
сохраняет прежний рабочий Guide. Disabled optional build показывает
manual instructions, но не false success. Reload обновляет indices/cache
без повтора побочных эффектов. Здесь же принимается полный optional-build
matrix, включая `GUIDE_BOOKMARKS`; его non-Guide ветви можно реализовать
раньше, но атомарный outcome до G не принимается.
