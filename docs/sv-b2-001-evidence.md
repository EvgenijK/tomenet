# SV-B2-001 — production shell D evidence

2026-10-28. Статус: implemented; Linux проверки и исправления после двух
независимых code review завершены. Это результат B2-001, не приёмка всего B2.

## Реализация и границы

Production `src/tomenet-sv --endpoint` отрисовывает D shell на server-list surface.
`ui/login-shell.[ch]` владеет ресурсами, bounded text cache и elapsed animation;
`ui/login-layout.h` вычисляет rectangles для draw и production mouse adapter.
SV-local ресурсы находятся в `lib/xtra/sv/login/`: 19 PNG, Liberation Mono с
лицензией, logo/torch data, six source-derived realm icons, frozen HTML/CSS/data,
provenance и actual SHA-256. Внешний web checkout не менялся и при запуске не нужен.
A font API, legacy/common, protocol и gameplay не менялись этой задачей.

Широкая композиция содержит logo, central panel, Chronicle и realm links;
≤1240 скрывает Chronicle/labels и переносит icons под panel. Short height
уменьшает hero/интервалы и row height. Размеры пересчитываются из текущих output,
display scale и model; mouse использует согласованный window pixel-density mapping.
Main loop обслуживает ввод/provider без дополнительной фиксированной задержки
после уже затраченного времени кадра. Stale mouse events отсеиваются перед dispatch.

Анимация: elapsed cell-logo/torch primitive, background rotation 30 s, fade 2 s,
gentle movement; reduced motion фиксирует кадр, hidden/minimized приостанавливает
декоративное время. Reset hook пересоздаёт renderer-owned textures; teardown явный.
Torch primitive подготовлен для account consumer B2-002; на server screen эталон
его скрывает. Остальные account/character/birth/MOTD visuals остаются своим tickets.

## Reference и визуальный результат

Source HEAD `1bb2d5d45b9001bf2a6f2084f78184c29a73fca7`; exact bytes зафиксированы
в `lib/xtra/sv/login/SHA256.json`. Итоговый cascade D, actual browser font Liberation
Mono, background `cirith_ungol_ascii_seed06.png`, reduced motion, DPR/display scale 1.
Runtime build `linux-bd7f9b5e26c5789bb7e9`, clang 22.1.8; версии/flags —
[build.txt](evidence/sv-b2-001/build.txt). Binary SHA-256 указан в evidence manifest.

| Logical viewport | Browser reference | Native software | Native OpenGL |
| --- | --- | --- | --- |
| 1440×900 | [reference](evidence/sv-b2-001/reference-1440x900.png) | [native](evidence/sv-b2-001/software-1440x900.png) | [native](evidence/sv-b2-001/opengl-1440x900.png) |
| 1024×768 | [reference](evidence/sv-b2-001/reference-1024x768.png) | [native](evidence/sv-b2-001/software-1024x768.png) | [native](evidence/sv-b2-001/opengl-1024x768.png) |

Просмотрены композиция, logo, readable panel text, side areas, icons и доступность
Direct connection при wide/minimum. Software использует SDL dummy output;
OpenGL — локальное X11 окно на DISPLAY=:1. Captures читаются из production renderer,
не из отдельной preview scene. Physical display timing/IME этим не подтверждаются.

Native adaptations: Chronicle честно сообщает News unavailable; realm launchers
не реализованы и помечены Links unavailable, включая compact. Server rows читают
настоящий endpoint model; синтетические host labels из сохранённого servers.txt
используются только как вход capture, mock ping/population не подставляются.
Detailed server-card styling принадлежит B2-002. Фон использует load-time linear
resampling 640×320 и shading вместо точного CSS blur/filter/backdrop; shadows и
raster edges отличаются. Эти отклонения просмотрены, pixel-perfect gate не вводился.

Frozen reference воспроизводится без sibling checkout:
`node docs/evidence/sv-b2-001/capture-reference.cjs /tmp/sv-b2-reference`.
Скрипт создаёт временные symlink paths к сохранённым ресурсам; использует уже
установленные Chromium/Playwright, пути версий записаны в нём. Повторный capture
из frozen HTML/CSS/data/assets побитово совпал по pixels с исходным browser capture
в обоих размерах. Computed final styles и reproduction log лежат рядом с PNG.

## Standards

Независимый агент проверил scoped diff относительно сохранённых до работы файлов,
а не общий HEAD diff с уже существовавшими изменениями пользователя. Два P2
замечания исправлены: renderer reset и повторная растеризация static text.
Повторное ревью: открытых hard violations и heuristic findings нет.

## Spec

Второй независимый агент сверил ticket/reference/spec и actual native captures.
Исправлены placeholder icons, compact unavailable feedback, torch primitive,
renderer reset, слишком резкий фон, неверный fill book/eye icons.
Повторное ревью: открытых замечаний нет. Полная приёмка B2 не заявляется.

## Проверки и воспроизведение

- `make -C src -f makefile.sv tomenet-sv -j4` — PASS, warnings-as-errors.
- `python3 tests/sv_endpoint_checks.py` — PASS: existing endpoint/editor,
  credentials/physical/metaserver, normal+sticky native input и production scene.
  Добавлены только минимальные mouse assertions в существующий core test:
  stale event, measured row selection и Direct connection.
- `SDL_VIDEODRIVER=dummy python3 tests/sv_shell_smoke.py --backend software` — PASS:
  A font/resource callers, fallback/error paths, profile isolation.
- Production endpoint software/OpenGL при 1440×900 и 1024×768 — PASS.
- Production software endpoint, `SV_B2_REDUCED_MOTION=0 --frames 2100` — PASS:
  проход более 30 s через rotation/fade без ошибки; это bounded runtime run,
  не stress suite и не отдельное измерение каждого animation frame.
- Все 39 записей resource SHA256 manifest проверены — PASS.
- `git diff --check` для изменённых task files — PASS.

Новый test harness или расширенный набор suites не создавался. В двенадцати
существующих consumer scripts добавлены только новый production source и libpng
к link inputs. Проверка настоящего device loss отдельно не выполнялась: reset
wiring проверен ревью. Windows/MinGW/Wine deferred по текущему правилу платформ.

Пример native capture из корня проекта:

```sh
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software \
SV_B2_BACKGROUND=0 SV_B2_REDUCED_MOTION=1 SV_B2_CAPTURE=/tmp/sv-b2.bmp \
src/tomenet-sv --endpoint --server-list docs/evidence/sv-b2-001/servers.txt \
  --library "$PWD/lib" --profile-root /tmp/sv-b2-profile \
  --fixture-window 1440x900 --frames 2
```

Для OpenGL: `SDL_VIDEODRIVER=x11 DISPLAY=:1 SDL_RENDER_DRIVER=opengl`;
X11-доступ в sandbox потребовал auto-approved escalation. Captures BMP конвертированы
в PNG без изменения pixels. Логи, styles, входные данные и hashes сохранены в
`docs/evidence/sv-b2-001/`. Existing working-tree изменения сохранены; коммитов нет.

## Обновление графа

`.graphify-venv/bin/graphify update .` — завершено: 17744 nodes, 46225 edges.
CLI обновляет code graph; semantic re-extraction docs/images этим не заявляется.
Extractor сообщил partial syntax extraction в 77 файлах и отсутствие symbols
в 154 файлах (включая legacy C/Lua), поэтому граф не считается полным доказательством
связей. HTML автоматически агрегирован до community view из-за размера >5000 nodes;
имена части communities автоматически заменены hub names. Source и build checks
использовались независимо от этих ограничений.
