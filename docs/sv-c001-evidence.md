# C001 — Linux package-launch evidence

Дата: 2026-11-04. Статус: implementation и технические Linux observations
выполнены; C001 принят пользователем 2026-11-04.
[Решение](sv-stage-c001-acceptance.md). Это пакет текущего
pregame-only SV, не final gameplay release. Windows/MinGW/Wine deferred.

## Реализация

- Обычный запуск вызывает существующий `sv_endpoint_run`; explicit `--endpoint`
  и `--synthetic` сохранены. После login architecture refactor parser находится
  в `tests/sv/diagnostics/launcher.c`, а `src/client/sv/main.c` делегирует ему.
  Изменён actual production dispatch, отдельной модели поведения для tests нет.
- [SV-only packaging script](../tools/package_sv_linux.sh) собирает архив с
  tracked stock `lib/`, production binary, recursive ELF dependencies,
  лицензиями и inventory; unresolved dependencies прерывают упаковку.
  Legacy build/release scripts и CI не менялись.
- [Package contract](tasks/c001/package-contract.md) фиксирует Fedora41/glibc,
  пользовательский запуск, bundled/system dependency boundary и ограничения.
- [Disposition reconciliation](tasks/c001/reconciliation-disposition.md)
  разбирает все 9 связей, включая 5 семантически чуждых package-launch mappings.
  Они не создают скрытых C001 prerequisites; canonical JSON не переписывался.

## Artifact и provenance

Архив: `/tmp/sv-c001-packages/tomenet-sv-4.9.4-linux-amd64.tar.bz2`.
Размер: 18 635 335 bytes. SHA256:
`5c03d91101978ae0b1e3cbe3ea4bfb614c9d8a1a90971e98b9aa76ba1e730d5c`.
Sibling `.sha256` содержит basename; `sha256sum -c` на host — PASS.
Пакет и checksum остаются локальными artifacts, релиз не публиковался.

Исходный HEAD: `4ce96497068c93fe6a34818c356a86c285e18f0b` плюс текущие
незакоммиченные CLI changes. Fedora41 build ID `linux-e8f97e06ea0cd4309818`.
[PACKAGE.txt](evidence/sv-c001/PACKAGE.txt) сохраняет hashes production diff и
самого packager. [Build log](evidence/sv-c001/build.log) подтверждает сборку
через `make -C /workspace/src -f makefile.sv tomenet-sv`.

Builder — minimal Fedora41 Linux-only environment, SDL3 3.2.16, SDL_ttf 3.2.2,
FreeType 2.13.3, libcurl 8.9.1, libsecret 0.21.7, glibc 2.40.
Он удовлетворяет production minimums `makefile.sv`; pins большого legacy/Windows
Containerfile не заявляются. Runtime построен отдельно из Fedora41 base с
Xvfb/Mesa/X11; исходники, compiler и SDL development install в него не переданы.

| Image | Immutable identity |
| --- | --- |
| Fedora41 base | `sha256:893f7eeffce8e3e2bb2bc62786ba5603fb1e0bd6ba07091acb892266b95bafdf` |
| Builder | `sha256:514ee49568cc0e6199fe167a8195e7fac4b91d2f4e1ff46597c6c82344d4cf62` |
| Runtime | `sha256:634e58f8fb8e3fdc8b1483b5c6cf24aa6ee3954fad8c4d679fffaace8bef763f` |

[Containerfiles и actual package commands](../.scratch/sv-c001-runtime/commands.md)
сохранены рядом с одноразовой runtime orchestration. Новый reusable harness,
framework или tests suite не добавлялись. Initial project GHCR `latest` был
недоступен; использован локально собранный Linux-only builder.

## Состав и зависимости

856 archive members; 47 bundled ELF libraries. Включены stock fonts/B2 assets,
их существующие лицензии, `COPYING`, runtime license files/versions,
[dependency inventory](evidence/sv-c001/runtime-dependencies.tsv) и
[system dependency inventory](evidence/sv-c001/system-dependencies.tsv).
Private profiles, `.git`, `.sv-build` в архиве отсутствуют.
[Полный shipped-file SHA256 inventory](evidence/sv-c001/SHA256SUMS) проверен
после извлечения: [все файлы OK](evidence/sv-c001/inventory-check.log).
Cascadia SHA256 соответствует approved
`06520d032ec274fa5040b22c6f4a1d829081b24ba40b2da56dae89bf10c7b481`.

[ELF header](evidence/sv-c001/elf-header.log),
[RUNPATH/NEEDED](evidence/sv-c001/elf-dynamic.log),
[symbol versions](evidence/sv-c001/elf-versions.log),
[actual runtime ldd](evidence/sv-c001/ldd.log) сохранены.
Bundled closure разрешается из relocated package; системными остаются
совместимые glibc/loader и runtime providers дисплея/графики. X11 libraries,
Mesa и Xvfb присутствуют в runtime image; это не headless bare-glibc promise.
Нет `LD_LIBRARY_PATH` или checkout resource fallback.

## Production observations

[Полные команды/env/results](evidence/sv-c001/observations.json).
Контейнер выполнялся с `--network none --user 1000:1000`; mounts — только
архив readonly, results и одноразовый orchestration script, без checkout.
Архив извлечён, каталог перемещён в
`/tmp/sv-c001-observation/relocated/installed`; cwd —
`/tmp/sv-c001-observation/unrelated`. HOME/XDG всегда временные.
Основной путь не задаёт `--endpoint`, `--library`, `TOMENET_PATH` или profile
root override: используется default SDL profile внутри временного XDG root.

| Observation | Result |
| --- | --- |
| Default startup, software, existing `--frames 2` | exit 0; [capture](evidence/sv-c001/default-software.png) |
| Default startup, OpenGL, existing `--frames 2` | exit 0; [capture](evidence/sv-c001/default-opengl.png) |
| Bare binary без аргументов, обычный `WM_DELETE_WINDOW` своему PID/window | exit 0; [capture](evidence/sv-c001/bare-close.png) |
| Isolated user font overlay | exit 0; actual user font open, legacy sentinel unchanged |
| Explicit profile root — файл, overlay недоступен | exit 0 через bundled fallback; sentinel unchanged |
| Все обязательные bundled font fallbacks отсутствуют в повреждённой копии | exit 1; fatal resource diagnostic; capture отсутствует |
| Несуществующий requested renderer | exit 1; startup failure; capture отсутствует |
| Bundled libSDL3.so.0 удалена из повреждённой копии | exit 127; loader сообщает missing dependency, UI не запущен |

[Software trace](evidence/sv-c001/default-software.strace) показывает actual open
packaged `libSDL3.so.0`, bundled Cascadia и `lib/xtra/sv/login` assets.
[Overlay trace](evidence/sv-c001/user-overlay.strace) подтверждает user-first
font lookup. Source checkout недоступен контейнеру. Capture получен существующим
`SV_B2_CAPTURE` observer из real production renderer; software кадр просмотрен.

Xvfb — виртуальный X11 display. `LIBGL_ALWAYS_SOFTWARE=1`: OpenGL использует
Mesa software rasterization (llvmpipe), physical GPU performance/driver coverage
не заявляется. Сеть намеренно недоступна, поэтому metaserver/Chronicle unavailable
ожидаемы; это не проверка live login и не повторная приёмка B/B2.

Initial bare-close orchestration выбрала временное SDL window и получила
X11 BadWindow. Выбор уточнён до stable named PID-owned window; повтор ordinary
close прошёл. Это не product bug и не скрытый PASS первой попытки; история
сохранена в [первом observation JSON](evidence/sv-c001/first-attempt/observations.json)
и runtime notes. Первый sandbox desktop smoke не получил video device;
повтор существующей проверки с доступом к дисплею прошёл.

## Проверки CLI и applicability obligations

Существующий `python3 tests/sv_shell_smoke.py --backend software` — PASS на
native Wayland dev host; [фактические вызовы и versions](../.scratch/sv-c001-runtime/cli-checks.md).
Это regression CLI changes, отдельно от Fedora41 archive observations.
Точечно заменено устаревшее требование bare-launch refusal; synthetic paths,
ошибки renderer/resources и decoy-profile isolation сохранены.
`bash -n tools/package_sv_linux.sh`, `--help`, штатный отказ Fedora41-only mode
на Manjaro и `git diff --check` — PASS. Новая automated suite не вводилась.

- `obligation.platform.package-launch.result`: выполнены extracted/relocated
  production launch, isolated profile, real bundled/user resources и dependency
  closure на объявленной Linux runtime environment.
- `obligation.platform.package-launch.failure-lifecycle`: выполнены ordinary close,
  resource/renderer/loader failures и unavailable overlay fallback без изменения
  соседнего owner. До server selection/login startup не пишет private profile;
  write/replace/concurrent-edit/secret/account lifecycle поэтому неприменимы
  данному package-launch consumer. Это не подтверждает C002/C003 persistence
  или credential-provider behavior. Default SDL_GetPrefPath может создать
  directory — штатный SDL bootstrap, не сохранение SV CFG.

Canonical registry implementation/evidence остаются исходными `pending`:
machine-readable native-runtime record не создавался и PASS ему не приписан.
Локальный planning snapshot сохраняет исходные hashes; он не превращён в runtime
schema. [Техническая приёмка](sv-stage-c001-acceptance.md) и user decision отдельно
от этих observations. Общие REALLOC-005/006/007 и другие этапы не закрывались.
