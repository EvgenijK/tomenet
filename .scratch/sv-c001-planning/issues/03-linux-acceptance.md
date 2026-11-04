# C001 — минимальная Linux acceptance и evidence

Type: research
Status: resolved
Assignee: c001_acceptance
Labels: enhancement, ready-for-agent

## Question

Какая минимальная проверка production-пути позволяет принять установленный SV,
не возрождая прежний Sandcastle и не расширяя тестовую инфраструктуру?

## Answer

C001 владеет одним outcome `capability.platform.package-launch` и двумя
obligations: `result` и `failure-lifecycle`. Реализация и evidence в canonical
ledger пока `pending`. Новые команды ниже — **предложение для implementation**,
а не выполненная проверка. В этой planning-задаче сборки и тесты не запускались.

Текущая приёмка — Linux amd64. Windows/MinGW/Wine deferred решением
2026-10-28, без PASS. Fedora41 builder baseline остаётся согласованной Linux
ABI-границей; запуск только на имеющемся dev host не подтверждает её.

### Уже есть, но не закрывает C001

- `src/makefile.sv:17-35,50-79`: production Linux executable `tomenet-sv`,
  отдельные outputs, `$ORIGIN` rpath; SDL3/SDL3_ttf/FreeType/libpng/libcurl и
  Linux libsecret. Это compile/link contract, не recipe готового архива.
- `src/client/sv/endpoint-run.c:1142-1193`: обычный startup root через CLI,
  `TOMENET_SDL3_USER_PATH` или `SDL_GetPrefPath("TomenetGame", "tomenet")`;
  library через CLI, `TOMENET_PATH` или adjacent `lib/`. Использовать этот
  production-путь для C001. Вызов только synthetic shell недостаточен.
- `src/client/sv/ui/font.c:40-68`: shared user overlay прежде bundled font;
  damaged/missing TTF и declared PCF fallback; fatal при отказе обоих.
- `tests/sv_shell_smoke.py:29-81` уже проверяет настоящий shell executable,
  изоляцию личного профиля, user overlay/fallback и renderer/resource failures.
  Но использует repository `lib`, `--synthetic`, explicit `--library`; запуск
  этого suite даже с extracted binary не доказывает package relocation.
- `docs/sv-stage-a-evidence.md:13-20` прямо оставляет shipping ABI, final packages
  и closure дальнейшим этапам. B2 native evidence относится к UI и конкретному
  dev runtime; package evidence в нём нет.
- `.github/docker/fedora41/release-client-sdl3-linux.sh` — baseline recipe для
  **legacy SDL3**. Его запуск не создаёт SV-package. Нельзя переименовать
  legacy artifact и засчитать C001.

### Рекомендуемая минимальная декомпозиция

| Предлагаемый ticket | Работа | Done |
| --- | --- | --- |
| SV-C001-001 | Зафиксировать scoped package contract и stock asset inventory | Exact archive/layout, Linux ABI, применимые runtime dependencies и явный список недоступных пока функций; решено противоречие final release requirements и текущего pregame-only SV |
| SV-C001-002 | Создать изолированный SV packaging recipe и устранить найденные startup/path gaps | `tomenet-sv-<version>-linux-amd64.tar.bz2`, stock resources/docs/licenses, Linux closure; legacy recipe/binary не перезаписаны, личные данные не попали в архив |
| SV-C001-003 | Проверить распакованный artifact, записать evidence и принять outcome | Нижеописанные production-наблюдения со свежими artifact identities; обе obligations явно рассмотрены, `pending` не превращён в PASS без наблюдений |

Разделять каждую проверку на отдельный ticket или заводить matrix/framework не
нужно. Если выявится core-дефект, добавить только необходимую проверку его
production-функции в существующем core suite. Новые native harness, broad
regression suite, stress/soak или полная compile-feature matrix требуют
предварительного согласования с пользователем.

### Минимальные наблюдения production-пути

1. Распаковать реальный SV archive вне checkout, переместить каталог, запустить
   binary из другого cwd **без** `--library` и `TOMENET_PATH`. Временные
   HOME/XDG/profile должны исключать доступ к реальному профилю. Подтвердить
   startup pregame UI и штатное закрытие; никакой live login для этого не нужен.
   После реализации обычного default dispatch основной запуск — bare binary
   или поставляемый SV launcher; `--endpoint` ниже описывает существующий
   production consumer и не заменяет проверку пользовательского entrypoint.
2. Записать фактический effective bundled resource path. В отдельном запуске
   проверить `TOMENET_SDL3_USER_PATH` и user-overlay-first на изолированном root;
   explicit `--library`/`TOMENET_PATH` проверить только как override, не как
   единственный способ найти resources. Снимки sentinel-файлов показывают
   неизменность unrelated owners. Отказ всех применимых font fallback должен
   давать явную ошибку и отказ startup без terminal fallback.
3. Из того же extracted directory проверить фактические ELF/runtime dependencies:
   architecture, ABI requirements и разрешённые system prerequisites, recursive
   bundled dependency identities. `readelf`/`ldd` — дополнение к реальному запуску
   на согласованной Linux baseline; динамически загружаемые providers/data нельзя
   считать покрытыми только `DT_NEEDED`.
4. Коротко проверить применимый failure/lifecycle: missing/invalid required
   resource, unavailable renderer/provider, недоступный profile path и закрытие
   приложения. Записать actual exit/status, отсутствие ложного успеха и порчи
   соседнего профиля. Provider должен сохранять существующий session-only
   fallback там, где он предусмотрен. Для write/replace/concurrent-edit частей
   obligation перечислить реально затронутые startup writers; не объявлять их
   автоматически N/A и не разворачивать аудит всех будущих settings.

Для `failure-lifecycle` cancellation в package context — штатное закрытие
startup UI, без повторных побочных эффектов; исторические игровые cancel flows
не становятся дополнительной задачей C001. Сетевые timeout/gameplay/guide/audio
сценарии добавлять только при наличии связанного shipping production consumer.

### Команды и остающиеся детали

Подтверждённая существующая команда сборки (ещё не выполнялась здесь):

```sh
make -C src -f makefile.sv tomenet-sv
```

SV packaging command — **pending**: recipe для SV пока нужно создать, точное
имя и аргументы определяет SV-C001-002. Не использовать существующий SDL3
release script для этой проверки.

Read-only диагностика конкретного будущего extracted executable:

```sh
readelf -h /ABS/EXTRACTED/tomenet-sv
readelf -d /ABS/EXTRACTED/tomenet-sv
readelf --version-info /ABS/EXTRACTED/tomenet-sv
ldd /ABS/EXTRACTED/tomenet-sv
```

Production launch для ручного закрытия pregame (после создания пустых временных
каталогов; `DISPLAY`/`XDG_RUNTIME_DIR` сохраняют доступ к выбранному display):

```sh
env -u TOMENET_PATH -u LD_LIBRARY_PATH \
  HOME=/ABS/TEMP/home XDG_CONFIG_HOME=/ABS/TEMP/config \
  XDG_DATA_HOME=/ABS/TEMP/data TOMENET_SDL3_USER_PATH=/ABS/TEMP/profile \
  /ABS/EXTRACTED/tomenet-sv --endpoint
```

После добавления обычного default dispatch повторить этот запуск без
`--endpoint` через поставляемый entrypoint; именно этот результат закрывает
installed-launch flow. Приведённый вариант фиксирует только уже существующий
production path, а не рекомендует заставлять игрока знать служебный режим.

Запускать из cwd вне checkout и extracted tree. Эта форма проверяет override
root; отдельный invocation с unset `TOMENET_SDL3_USER_PATH` проверяет production
`SDL_GetPrefPath` в изолированных HOME/XDG. Не использовать `env -i` вслепую:
это может отключить display/provider и изменить проверяемый результат. Точный
метод доказать отсутствие чтения checkout и runtime-loaded closure — pending
после выбора доступной Fedora41 baseline среды; существующие observations и
диагностика предпочтительнее нового harness.

### Evidence и границы утверждения

Достаточен компактный отчёт рядом с implementation tickets либо в
`docs/evidence/sv-c001/`: commit и dirty state, build ID/configuration,
archive SHA-256, extracted binary/resource/library identities, distro/architecture
и versions, cwd и очищенные path overrides, команды/exit codes, короткие
startup/failure наблюдения и screenshot успешного pregame. Для каждой obligation
указать observation либо точное неприменимое условие с причиной. Не писать
секреты/private profile в логи. Canonical ledger/evidence обновлять после
реальных результатов по существующему формату; этот документ их не меняет.

**Открытая граница:** подтверждённый shipping contract ticket 08 требует PNG
screenshots, audio pack extraction и automated Guide update вместе с dependency
closure, тогда как текущий `makefile.sv` декларирует `gameplay=off;audio=off`, а
функциональное распределение выносит эти flows в дальнейшие C-этапы. C001 scope
в `stages.json` — installed launch, optional platform integrations deferred.
Нужно явно решить в SV-C001-001, считается ли C001 проверкой текущего этапного
пакета с сохранением final shipping obligations на соответствующих поздних
этапах. Нельзя без этого решения назвать pregame-only архив полноценным final
release или, напротив, включить реализацию всего audio/guide в C001.

### Источники

- `docs/capabilities/manifest.json:21647` — outcome description.
- `docs/capabilities/native-coverage.json:51735-51778` — C001, conditions и обе obligations.
- `docs/capabilities/stages.json:6`, `docs/sv-functional-stage-plan.md:34` — stage scope.
- `AGENTS.md`, `docs/sandcastle.md`, `docs/agents/platform-validation.md` — текущие приоритеты процесса и Linux.
- `CONTEXT.md`, `docs/adr/0003-explicit-module-connections.md`, `docs/adr/0006-errors-and-overload.md` — owners и failure semantics.
- `.scratch/single-window-sdl3-client/issues/08-name-and-ship-the-new-client.md`, Answer — names, Fedora41 baseline, stock contents, shared user root и adjacent `lib/`.
- `.scratch/single-window-sdl3-client/issues/09-design-parity-evidence-and-acceptance.md` — extraction/runtime evidence отличается от compile/link.
- `.scratch/single-window-sdl3-client/research/platform-deltas-and-packaging.md:5,62-75` — статический аудит closure limitations.
- `docs/capabilities/settings-policy.md:17-41,99-111` — ownership и font failure policy.

Graph query `package-launch evidence packaging` выполнен первым; ответ был
truncated и в основном исторический. Уточнения выше проверены чтением текущих
исходников и документов. Код и canonical JSON не изменялись.
