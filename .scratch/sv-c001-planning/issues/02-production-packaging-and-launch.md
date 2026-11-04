# C001: production Linux packaging and launch

Type: research
Status: resolved
Assignee: c001_packaging
Labels: enhancement, ready-for-agent

## Question

Какие production-пути уже существуют и какой минимальный план нужен для запуска установленного SV из Linux-архива?

## Answer

C001 пока не имеет собственного production archive: существующий SV binary уже открывает production endpoint/B2 UI, разрешает профиль и ресурсы, но обычный запуск без аргументов отвергается. Нужны небольшой SV-local startup change, отдельная Linux упаковка и evidence запуска именно распакованного результата. Windows/MinGW/Wine отложены по `docs/agents/platform-validation.md:6`; старый Stage A synthetic результат не заменяет package acceptance (`docs/sv-stage-a-evidence.md:16`).

Прочитаны AGENTS.md, CONTEXT.md, platform-validation.md, issue-tracker.md, triage-labels.md, архитектурное размещение `docs/sv-architecture.md:41`, ADR 0003 (явные связи), ADR 0006 (ошибки/lifecycle). Перед исходниками выполнен `.graphify-venv/bin/graphify query "package launch install SV resources"`; граф дал в основном Stage A runners и legacy release infrastructure, детали уточнены исходниками. Новые сборки, runtime проверки и production правки не выполнялись.

### Уже существующий production-путь

| Owner | Фактический контракт |
|---|---|
| `src/makefile.sv:16`, `:30`, `:51`, `:77` | Linux amd64; clang/pkg-config; SDL3, SDL3_ttf, FreeType, PNG, curl, libsecret; `$ORIGIN` RUNPATH; копирование готового binary в `src/tomenet-sv`. Нет install/package target. |
| `src/client/sv/main.c:133`, `:145`, `:157` | `--endpoint` вызывает `sv_endpoint_run`; без него и без `--synthetic` выход 2 с `live client is not implemented`. Это устаревшая default dispatch для уже реализованного endpoint. |
| `src/client/sv/endpoint-run.c:1152` | Profile: explicit `--profile-root`, затем `TOMENET_SDL3_USER_PATH`, затем `SDL_GetPrefPath("TomenetGame", "tomenet")`. |
| `src/client/sv/endpoint-run.c:1170` | Library: `--library`, затем `TOMENET_PATH`, затем `SDL_GetBasePath()` + `lib`. CWD не определяет adjacent library. |
| `src/client/sv/ui/font.c:40` | Cascadia TTF: `U/xtra/font` перед `B/xtra/font`; повреждённый user font уступает bundled; PCF fallback только `B/xtra/font/16x24x.pcf`; полный отказ возвращает ошибку. |
| `src/client/sv/endpoint-run.c:1193`, `src/client/sv/ui/login-shell.c:186` | Реальная B2 shell обязательна для endpoint. Она использует `B/xtra/sv/login`, LiberationMono/Serif TTF, PNG backgrounds/icons/doors (`login-shell.c:160`). Лого встроено в binary (`login-shell.c:12`). Synthetic font smoke не покрывает эти assets. |
| `src/client/sv/endpoint-run.c:1310` | Startup error даёт ненулевой результат с диагностикой и cleanup SDL/resources/providers. |
| `src/temporary/sv/synthetic.c:16` | Маркер и обязательное новое/помеченное абсолютное directory относятся к synthetic bootstrap. Не использовать этот путь для production package acceptance. |

Нормативные root owners: `docs/capabilities/settings-policy.md:18` задаёт U, S=U/sv, B и сохранение fingerprint U; `:43` запрещает legacy CFG write при штатном SV startup. C001 не реализует полную CFG/persistence C002/C003. `:230` явно требует зависимости libsecret/GLib для выбранного backend; отдельный persistent Secret Service provider не появляется от копирования libsecret.

### Существующая упаковка и команды

Текущий release pipeline `.github/workflows/build-release-assets.yml:62` вызывает `.github/docker/fedora41/release-all.sh`, который собирает пять legacy packages, включая Windows. SV отсутствует. Fedora43 copy имеется, но не является текущим workflow owner.

`.github/docker/fedora41/release-client-sdl3-linux.sh:17` собирает `makefile.sdl3`, кладёт legacy `tomenet`, `tomenet.cfg`, launcher scripts и полный `lib/`, затем вызывает `copy_linux_deps_recursive` и создаёт tar.bz2 (`:23–33`). Это образец layout, не готовая SV упаковка. Нельзя принимать старый архив за SV archive.

`.github/docker/fedora41/release-common.sh:39` делает рекурсивный `ldd` сбор зависимостей, исключает host system libraries (`:60`), переписывает RUNPATH copied libraries (`:68`). `ldd` не доказывает dynamic provider/runtime closure; строки `not found` текущий parser не считает ошибкой. Для SV нужен явный fail на unresolved dependencies и декларация base system ABI/provider requirements. Не исправлять legacy helper ради C001; локальное SV решение допустимо.

Существующая команда сборки, без запуска в этом исследовании:

```bash
make -C src -f makefile.sv tomenet-sv
```

Существующий production runtime (путь к заранее существующему изолированному U):

```bash
/path/to/extracted/tomenet-sv --endpoint --profile-root /tmp/c001-profile --frames 3
```

Это пример реального production интерфейса; ему нужны adjacent `lib/`, display/renderer и доступный профиль. Для воспроизводимого acceptance следует дать локальный server-list через `--server-list`/`--server`, чтобы достижение первой shell не зависело от внешнего metaserver. Network/provider runtime принадлежит уже реализованным owners; C001 проверяет доставку требуемых libraries и корректное отсутствие внешнего optional provider, а не переоткрывает их полную приёмку.

### Минимальные implementation tickets

1. **SV-C001-001 — Обычный production startup SV.** Выбрать default endpoint в `src/client/sv/main.c`: при отсутствии `--synthetic` использовать тот же `sv_endpoint_run`, сохранив явный `--endpoint`, CLI/env overrides и synthetic gates. Это предпочтительнее второго launcher с собственной dispatch логикой: игрок может запустить сам поставляемый binary. Локальный change условного dispatch и usage; не менять session/protocol/legacy. Существующий `tests/sv_shell_smoke.py:51` прямо требует прежний отказ bare binary — точечно удалить/актуализировать это устаревшее ожидание, не добавляя отдельный набор тестов. Не запускать существующий no-args call с личным профилем: он начнёт production startup.
2. **SV-C001-002 — Изолированная Linux упаковка.** Отдельный SV packaging entry point в SV-owned path (например `tools/sv/package-linux.sh`), собирающий `makefile.sv` и создающий перемещаемый archive: binary, adjacent `lib/` с актуальными font/B2 assets и лицензиями, runtime closure с fail на unresolved deps, небольшая инструкция запуска. Проще использовать существующий полный `lib/`, чем вводить новый asset registry; состав можно сократить только при доказанном текущем runtime использовании. Не поставлять legacy CFG/региональные legacy launchers как SV настройки; не запускать `release-all.sh`. CI публикация релиза не обязательна для локального C001 acceptance, не менять все legacy workflows.
3. **SV-C001-003 — Linux archive acceptance и coverage.** Создать archive, распаковать вне исходников, запустить production binary из другого CWD без `TOMENET_PATH`/`LD_LIBRARY_PATH` подсказок build machine. Отдельно проверить применимый user font overlay и корректную ошибку отсутствующего обязательного bundled B2 asset. Зафиксировать archive/binary/resource hashes, OS/base ABI, dependency resolution и реальные startup/exit observations. Использовать isolated U и decoy legacy CFG/resource snapshots, различать штатный quit от startup failure; обе obligations C001 должны иметь конкретные dispositions. Минимум core tests только при изменении core; runtime acceptance не превращать в новую общую matrix/harness. Перед расширением набора автоматизированных проверок требуется отдельное согласование пользователя.

Порядок: 001 → 002 → 003. Snapshot/ownership C001 должны сохранить canonical `capability.platform.package-launch`, `obligation.platform.package-launch.result` и `.failure-lifecycle` (`docs/capabilities/native-coverage.json:51735`). Приёмка Linux не означает подтверждения Windows. Audio/mixer/codecs, полноценная настройка ресурсов и persistence не подтягиваются только из старых документов Stage A; включаются лишь зависимости реально поставляемого SV binary.

### Ограничения и точечные улучшения вне C001

Фактически обнаруженная возможность улучшения legacy packaging: `.github/docker/fedora41/release-common.sh:52` и `:67` отбрасывают `ldd` unresolved diagnostics; proposal — после отдельного решения сделать fail-fast в legacy helper и подтвердить negative missing-library packaging case. Аналогичный helper существует в fedora43. Статус: обнаружено, вне текущего implementation scope. Для C001 решить в локальном SV packaging code; legacy cleanup не нужен.
