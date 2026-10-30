# SV-G-002 — Guide bookmarks, copy и chat integration

> Политика проверок от 2026-10-28: [текущая платформа — Linux](../../agents/platform-validation.md). Windows/MinGW/Wine build, runtime, visual/input и acceptance временно отложены пользователем для всех этапов; они не являются обязательным gate или блокером. Windows-only проверки остаются deferred, без заявления pass; исторические результаты сохранены.

Статус: specified; реализация и runtime evidence не выполнены.

Зависит от [SV-G-001](SV-G-001-viewer.md).

## Владение

- `capability.files.bookmarks-load`
- `capability.files.bookmarks-save`
- `capability.guide.bookmark-set`
- `capability.guide.bookmark-open`
- `capability.guide.bookmark-delete`
- `capability.guide.copy-lines`
- `capability.guide.paste-line`

## Проверки

Создание, rename, open и delete сохраняют unrelated bookmarks и source
Guide. Load/save используют private SV ownership и truthfully report
provider/read/write/replace/conflict failures. Copy не включает secrets;
paste-line отправляет ровно одну baseline-formatted chat reference и
возвращает Guide state.

