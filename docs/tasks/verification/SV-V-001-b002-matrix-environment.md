# SV-V-001 — Восстановить среду проверки SV-B-002

Статус: открыта, 2026-09-24. Инженерная задача по воспроизводимости evidence;
не владеет capability ID и не принимает SV-B-002 или Stage B.

## Причина и граница

[Текущий cumulative run](../../sv-b002-evidence.md) заблокирован окружением:
не найдены MinGW development dependencies, Python runner запущен без
`jsonschema`, dummy SDL не поддерживает minimize и OpenGL. Кроме того, нужны
отдельные Wine intermediate и реальные Windows 10/11 observations.
[Stage A ранее был принят](../../sv-stage-a-acceptance.md) в подготовленной
среде; этот run не доказывает новую ошибку production code.

Задача готовит и документирует **новые актуальные запуски**. Право закрыть
protocol/caller obligations остаётся у [SV-B-002](../stage-b/SV-B-002-contact.md),
итоговый platform/evidence matrix — у [SV-B-075](../stage-b/SV-B-075-acceptance.md).
Не ослаблять checks, не переписывать pass/stale result вручную и не выдавать
Wine за Windows evidence.

## Зависимости и работа

1. Воспроизвести Python environment по
   [Stage A recipe](../../sv-stage-a-acceptance.md#reproduce) и
   `tools/requirements-capabilities.txt`; подтвердить `jsonschema` в **том же
   interpreter**, которым запускается `tools/run_stage_a.py`.
2. Подготовить pinned i686 MinGW SDL3/SDL3_ttf/FreeType SDK по
   [sv-mingw.md](../../sv-mingw.md), сохранить версии/хэши, выполнить независимый
   SV PE build и Wine software smoke. Не подменять i686 сборку Linux binary.
3. Запустить Linux software и accelerated native checks в desktop session с
   window manager, поддерживающим minimize/restore, и доступным OpenGL
   renderer. Сохранить actual `SDL_GetRendererName`, geometry, timing и
   presenter observations; software pass не заменяет accelerated.
4. После устранения первых блокеров выполнить `tools/run_stage_a.py` в новом
   output directory с нужным SDK и Python interpreter. Разобрать source-digest
   fixture drift от изменений B: обновлять только проверенные fingerprints и
   dependencies, не принимать устаревшие records. Приложить `report.json`,
   logs, build/SDK/source/fixture hashes и все оставшиеся failures к
   [SV-B-002 evidence](../../sv-b002-evidence.md).
5. Обеспечить проверяющему реальные Windows 10 и 11 среды (software и
   accelerated renderer) для native contact/failure сценариев. Выполнить
   platform-specific socket/error и UI observations на собранном SV PE;
   записать OS/build/renderer, exact проверенные сценарии и безопасные
   fingerprints. Передать records в [SV-B-075](../stage-b/SV-B-075-acceptance.md).

## Acceptance и проверки

- Fresh report достигает всех registry/data, Linux software/accelerated и
  MinGW/Wine checks; каждый failed/blocked check отдельно объяснён и остаётся
  pending до исправления. Наличие `jsonschema` подтверждено логом runner.
- Native minimize/restore и OpenGL проходят в реальной оконной сессии;
  зафиксированы фактические video/renderer names. i686 PE построен из pinned
  SDK и Wine запускает именно этот binary с необходимыми DLL.
- Actual Windows 10/11 records сделаны на соответствующих ОС, а не Wine;
  они содержат version/config/executable/SDK/dependency fingerprints и
  результаты SV-B-002 сценариев без секретов или private payload.
- У [SV-B-002](../stage-b/SV-B-002-contact.md) и
  [SV-B-075](../stage-b/SV-B-075-acceptance.md) остаётся статус pending, пока
  их собственные product и platform obligations не доказаны полностью.
