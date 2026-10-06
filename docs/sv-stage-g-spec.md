# Этап G — Guide и финальная приёмка TomeNET SV

> **Статус: superseded 2026-10-06.** Смешанный Guide/files/platform
> scope G заменён [post-B этапами по одному функциональному блоку](../.scratch/single-window-sdl3-client/issues/38-reallocate-post-b-by-functional-block.md#answer).
> Guide остаётся late block, но буква G и cross-block final gate больше
> не нормативны. План миграции —
> [stage reallocation](tasks/stage-reallocation/README.md). Текст ниже сохранён как история.

Статус: specified; реализация и runtime evidence не выполнены.

Этап G идёт после A–F и владеет всеми постоянными Guide outcomes. До G
любая Guide entry point показывает ровно `The guide is in development`
и возвращает focus/pending interaction исходному caller. Заглушка не
читает Guide или bookmarks, не отправляет Guide-specific packets, не
запускает update/checksum и не входит в terminal fallback.

## Состав

1. Нативный source-preserving Guide viewer: local override/bundled source,
   navigation, search, help, highlights, server-directed opening и exact caller return.
2. Context help для всех ранних и поздних callers из B–F.
3. Guide bookmarks, их private persistence, copy-lines и paste-line/chat integration.
4. Checksum, reload, update, optional provider/platform configurations и все
   failure/rollback paths.
5. Удаление заглушки из всех entry points и полная cumulative
   Linux/Windows, software/accelerated, lifecycle, optional-build и human приёмка.

## Граница

G не меняет server help/documents: `SPECIAL_FILE_HELP` остаётся
самостоятельной server-document capability D. Обычные local-file marks
также не являются Guide bookmarks.

## Acceptance gate

G не завершён, если хотя один Guide ID остался pending, хотя один
entry point показывает заглушку или fallback, либо нет актуального
evidence для обязательной platform/build/error ветви. После G нет
terminal fallback linkage, Guide placeholder и неклассифицированных active IDs.
