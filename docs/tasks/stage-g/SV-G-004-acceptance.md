# SV-G-004 — Guide и final acceptance

> Политика проверок от 2026-10-28: [текущая платформа — Linux](../../agents/platform-validation.md). Windows/MinGW/Wine build, runtime, visual/input и acceptance временно отложены пользователем для всех этапов; они не являются обязательным gate или блокером. Windows-only проверки остаются deferred, без заявления pass; исторические результаты сохранены.

Статус: specified; runtime evidence не выполнено.

Зависит от [SV-G-001](SV-G-001-viewer.md),
[SV-G-002](SV-G-002-bookmarks-sharing.md) и [SV-G-003](SV-G-003-update.md).

Проверить все 21 Stage G outcomes и cumulative A–F regression на Linux,
software и accelerated rendering, enabled/disabled optional builds,
изолированных профилях и packaged runtime. Source/build проверка
не находит заглушку `The guide is in development` и Guide fallback routes;
runtime проходит каждый caller, error и lifecycle path. Windows 10/11 build/runtime/acceptance отложены пользователем. Только этот gate
может заявить полную финальную приёмку всех active IDs.
