# C009 — Linux production core evidence

Дата: 2026-11-10. Реализован production core одного outcome
`capability.lua.reload`. Пользователь поручил закрыть C009 сейчас:
[решение о приёмке](sv-stage-c009-acceptance.md). Этот технический отчёт не
заявляет полный native transfer/startup PASS. Canonical allocation, prerequisite
`capability.files.resource-overlay`, три obligations и pending registry statuses
сохранены. [Спецификация](sv-stage-c009-spec.md),
[задачи и границы](tasks/c009/README.md), [canonical snapshot](tasks/c009/coverage.json).

## Проверенный production путь

Одна компактная проверка [sv_lua_core.py](../tests/sv_lua_core.py) собирает
[Lua core executable](../tests/sv/lua-core.c) с настоящими
`sv/lua/runtime.c`, `constants.c`, `reload.c`, resource resolver C003 и теми же
vendored Lua 4 engine sources, что production `makefile.sv`. Тест не реализует
интерпретатор, callbacks школ/заклинаний, transfer coordinator или Guide getters.
Его callback только наблюдает diagnostics и порядок сообщений.

Runtime исполняет реальные bundled `xml.lua`, `meta.lua`, `audio.lua`, `guide.lua`,
`activations.lua`, затем `c-init.lua` и вложенную цепочку player/s_aux/spells/
runecraft/data scripts. Зафиксировано 41 успешная загрузка скриптов, 25 школ,
310 заклинаний; вычисляемое bundled значение `guide_skills` равно 40.
Schools/spells подготовлены настоящими startup bindings и Lua finalization.

Проверка изменяет только временный shared overlay. Полный bundled `guide.lua`
дополняется вычисляемыми expressions; новый typed getter возвращает `Overlay 42`.
Настоящие Lua `dostring` и `dofile` сохраняют несколько возвращаемых значений
(42/строка и 48/строка). Нативные base functions не заменены overlay loader;
только `pern_dofile` разрешает resource roots. Baseline `NO_CLIENT_IOLIB`
сохраняется: base/string/debug доступны, I/O library не открывается. Отдельная
новая конфигурация с включённой I/O library не заявлена.

К реальному `c-init.lua` добавлен вложенный `pern_dofile`: отсутствие и
синтаксическая ошибка дочернего файла дают failure, concrete child path и
непустой diagnostic, не маскируются bundled fallback или успешным done flag.
После исправления reload снова успешен, server flags меняются, clear удаляет
runtime и C-owned schools/spells. Старые borrowed pointers через reload не
используются.

Completion проверяется через production API, без wire decoder C010:

- При remaining transfers reload не происходит. Чтение/take completion не
  повторяет эффект; новый настоящий END occurrence сохраняет baseline повтор.
- Rejected INIT не выставляет flags. Accepted audio/Guide INIT → disconnect до
  END → новое поколение → последний END сохраняет process-lifetime sticky flags.
  Stale INIT/END не меняет state и не выдаёт diagnostics.
- При failed last close выполняется согласованный reload текущих файлов;
  `transfer_success=false` остаётся отдельно от успешного runtime result.
  Успешное чтение не объявляет успешную передачу или startup readiness.
- Guide metadata пересобираются через реальные getters (`Transferred 21`).
  Invalid count очищает старые metadata, даёт failure; audio restart warning
  наблюдается раньше metadata error. Warning не вызывает audio backend/remap.
- Close очищает runtime/metadata и pending completion; shared files не удаляются.

## Команды и результаты

Базовая revision: `ec6c77568bf1f3178b46a66ab64c339208b0fabf`; workspace dirty:
C009 production sources/build/startup integration, tests и документы добавляются
в текущей работе. Команды относятся к этому рабочему дереву, а не чистой revision.

```sh
SV_LUA_COMMAND_LOG=.scratch/sv-c009/lua-core-command.json python3 tests/sv_lua_core.py
```

Результат: PASS. [Вывод](../.scratch/sv-c009/lua-core.log),
[полные compile/link/run argv, SHA-256 compiled sources и версия compiler](../.scratch/sv-c009/lua-core-command.json).
SV sources/test компилируются с `-Wall -Wextra -Werror`; production constants
сохраняют исключение `-Wno-deprecated-non-prototype` для legacy headers.
Vendored engine собирается отдельно без `-Werror`. Включены ASan и UBSan,
`UBSAN_OPTIONS=halt_on_error=1`; sanitizer diagnostics отсутствуют.
`ASAN_OPTIONS=detect_leaks=0` соответствует существующей core-check policy;
проверка утечек этим запуском не заявлена.

Финальные runtime sources проверены повторно после исправления typed spell data,
проверки настоящей finalization names/refs и source-local include `sys/time.h`
для production constants. Source hashes последнего focused run находятся в
`lua-core-command.json` (`sourcesSha256`).

```sh
make -C src -f makefile.sv -j4 tomenet-sv
```

Результат: PASS, [build log](../.scratch/sv-c009/build.log),
[compiler/flags/dependency metadata](../.scratch/sv-c009/build-metadata.txt).
Первая сборка обнаружила incomplete `struct timeval` через legacy header в
новом `constants.c`; добавлен локальный include, общие headers не менялись.
[Первый failed log](../.scratch/sv-c009/build-first-failed.log) сохранён;
финальный build выше прошёл после исправления.

Выполнен только существующий bounded default-startup case из
`tests/sv_shell_smoke.py:55`, без запуска полного smoke suite:

```sh
TOMENET_SDL3_USER_PATH=<empty temporary user root> TOMENET_PATH="$PWD/lib" SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy src/tomenet-sv --frames 3 --fixture-window 1024x768
```

Результат: PASS, exit 0; early Lua `status=0 scripts=5`; endpoint selection
не завершена, game contact не выполнялся; временный user root после завершения
не содержит файлов. [Native log](../.scratch/sv-c009/native-startup.log),
[точные argv/environment, exit, file inventory и binary SHA-256](../.scratch/sv-c009/native-startup-command.json).
Фикстура использует реальные bundled fonts/resources и SDL software renderer.
Existing diagnostics `options.prf:36/37`, in-memory conversions и недоступные
Chronicle sources сохранены в log; их успешное исправление не заявлено.
Этот case подтверждает ранние пять scripts в actual startup consumer, но не
server-dependent open/transfer/handoff C011.

## Disposition трёх obligations

| Exact obligation | Проверено и что остаётся |
|---|---|
| `obligation.lua.reload.result` | CORE CHECKED: настоящие bundled startup scripts, reload overlay expressions, обновлённые metadata, visible nested failure. Actual last FILE_END caller/publication — C010; реальная session preparation — C011. Guide content/bookmark/update не реализованы; текущая entry сохраняет development placeholder. |
| `obligation.lua.reload.failure-lifecycle` | CORE CHECKED: production public API, missing/invalid resources, reload/cleanup, stale generations, disconnect между INIT/END, отсутствие replay от observation/take. Wire read/write/replace failures и concurrent transfer publication принадлежат C010; реальные session cancellation/handoff/teardown — C011/C014. Эти flows не получают PASS от core test. |
| `obligation.lua.reload.allocation-checks` | CORE CHECKED: remaining/last completion, настоящий повторный END, Guide getters, failed/missing nested scripts, audio warning перед metadata без live remap, sticky lifetime и согласованный reload после failed close. Actual packet ACK/ERR/order — C010, readiness — C011; они остаются pending. |

Только Linux. Windows/MinGW/Wine deferred по текущему правилу платформ.
Полная live-runtime certification, расширенные visual/input/protocol matrix,
стресс/soak, реальные аккаунты и автоматическое повышение canonical статусов
не выполнялись. Пользователь принял текущую core-границу 2026-11-10.
Поздние integrations и проверки закреплены в SV-C010-001, SV-C011-003 и
SV-C014-004; они обязательны своим этапам, не блокируют и не переоткрывают
C009 автоматически. Backlink evidence служит трассировке; canonical statuses
сохранены, непроверенные flows не получают PASS.
