# SV-LOGIN-ARCH-005 — Проверить интеграцию и зафиксировать архитектуру входа

**Статус:** implemented; адресные Linux checks passed; принято пользователем 2026-11-04.
**Дата:** 2026-11-04.
Labels: enhancement

**Зависимости:** [004](SV-LOGIN-ARCH-004-view-input-birth.md); использует результаты 001…003.

## Результат и scope

Проверить связанный production-путь нового клиента после переноса, устранить
только интеграционные дефекты этого изменения. Сверить конечный interface,
ownership и физическое размещение с 001; актуализировать `docs/sv-architecture.md`,
индекс задач и ссылку на результат предложения в `docs/sv-improvements.md` при
реализации. Не изменять историческое evidence B/B2 и другие accepted outcomes.

Отразить фактические seam, lifetime views и будущий handoff contract, не обещать
поддержку реального live runtime. Проверить согласование callers с C001 без
присвоения его dispatch/packaging deliverables этому плану.

## Критерии завершения

- [x] Сценарий входа локализован; runner не владеет смысловыми переходами flow,
      renderer не использует input state как общую модель.
- [x] Минимальный согласованный production-path Linux набор подтверждает выбор/
      создание персонажа, back/cancel, name recovery/retry и generation.
- [x] Проверки выполняют новую реализацию; старых реализаций/forwarding headers и
      копий поведения для tests нет.
- [x] Зафиксированы конкретные команды, результаты, ограничения и фактический scope;
      legacy/server/common архитектура не затронута.
- [x] CLI и реальный live handoff перечислены как отдельные работы, не blockers и
      не реализованные возможности; pending состояния не названы PASS.
- [x] Документация и статус 001…005 отражают реальный результат; пользовательская
      приёмка зафиксирована после явного решения пользователя.

## Минимальная проверка

Linux SV build и только согласованный набор затронутых production-path checks.
Результаты 002…004 переиспользовать; повторять при новых изменениях/ошибках,
а не для механического дублирования прогонов. Не переоткрывать полные B/B2,
actual-server/provider, Windows или capability matrix. Новые checks и расширение
проверок требуют предварительного согласования по AGENTS.md.

## Вне scope

Новые функции, отдельный acceptance stage/ledger, реальный startup-to-live handoff,
CLI/packaging, организация legacy/server/common и общая перестройка tests.


## Результат реализации — 2026-11-04

- Feature-specific исходники находятся в `src/client/sv/login/`; прежних
  forwarding headers и второй реализации flow нет. Общие protocol, vault,
  input/UI utilities и `session/character-setup.h` сохранены отдельно.
- `login/flow.[ch]` владеет переходами и attempt lifecycle, retry/name recovery,
  credential buffers и semantic projections. `selected`, `credentials_result`
  и `next` возвращают требуемый следующий шаг; native runner его исполняет.
- Renderer читает `login/ui/view.h`, native input хранит mutable focus/scroll.
  Render metrics возвращают пределы прокрутки, не изменяя interaction.
- Birth public interface скрывает backing state; native creation adapter
  использует private state внутри модуля. Диагностика читает value snapshot.
- Изменены только SV, ссылки его сборки/проверок и документация этой работы.
  Legacy/server/common source и CLI dispatch не менялись; чужая работа сохранена.

## Адресные Linux checks

| Проверка | Результат и evidence |
|---|---|
| Production build `make -C src -f makefile.sv tomenet-sv` | PASS; [build.log](../../../.scratch/login-architecture/build.log) |
| Existing `tests/sv/login-interaction.c` + production dependencies, строгий clang и UBSan | PASS; проверены существующие assertions interaction/pregame |
| Existing `birth-mode-arrows.c`, production birth/native adapter, UBSan | PASS; один выбранный native input check |
| Existing `birth-body-hotkeys.c`, production birth/native adapter, UBSan | PASS; [log](../../../.scratch/login-architecture/birth-body-hotkeys.log). Запущен root до получения сообщения агента о mode-arrows; дополнительных native checks после этого нет |
| Existing paced endpoint replay через реальный production runner | PASS; [paced.log](../../../.scratch/login-architecture/paced.log), [play.bin](../../../.scratch/login-architecture/play.bin): literal 16266 bytes, authoritative ACK и END/live tail, milestone `live-session-handoff` |
| Existing `tests/sv/name-rejection-recovery.c`, только `forbidden`, ASan/UBSan | PASS; [forbidden.log](../../../.scratch/login-architecture/forbidden.log): два соединения, новое поколение, сохранённый draft, исправленный запрос имени и stale input assertions |

Paced replay уже использовался в `.scratch/pregame-clock` и
`.scratch/b2-stats-mode/decode`; для текущих путей адаптированы только source lists,
includes и draw signature в [build.py](../../../.scratch/login-architecture/build.py)
и [endpoint.c](../../../.scratch/login-architecture/endpoint.c). Это producer/observer
вокруг production-пути, а не отдельная реализация игровых правил.
Forbidden собирается и запускается через
[name-forbidden.py](../../../.scratch/login-architecture/name-forbidden.py), без
остальных режимов существующего runner.

Команды воспроизведения из корня, с установленными Linux build dependencies:

```sh
make -C src -f makefile.sv tomenet-sv
python3 .scratch/login-architecture/build.py
env SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software DBUS_SESSION_BUS_ADDRESS=unix:path=/tmp/sv-login-no-vault .scratch/login-architecture/endpoint .scratch/login-architecture/profile "$PWD/lib" ack tests/sv/fixtures/birth-current.hex .scratch/login-architecture/play.bin 4.9.4.0.0.0
python3 .scratch/login-architecture/name-forbidden.py
```

Peer локальный, аккаунт и данные синтетические. Публичная Chronicle работает
как существующий фоновый provider; реальный игровой аккаунт не использовался.

## Ограничение исторического fixture

Выбранный сначала batch `birth-single-submission` в режиме `ack` не доходит до
финального запроса: producer заранее ставит ввод разных owners в одну очередь,
которую production barrier обязан очищать. Одинаковое падение подтверждено на
исходном HEAD `85592381b61ba163ccf06edaf55c5f7c969ff9ee` и текущем коде;
лог сравнения исходной ревизии сохранён в [baseline-ack.log](../../../.scratch/login-architecture/baseline-ack.log).
Это не PASS. Вместо него выполнен существующий paced golden replay. Полная
матрица ack/retry/cancel-mode не объявляется пройденной и массово не исправлялась.
Name retry проверен отдельным рабочим `forbidden` case. Полный visual/actual-server
и Windows/MinGW/Wine suite не запускался.

## Code review

Фиксированная точка — стартовый HEAD
`85592381b61ba163ccf06edaf55c5f7c969ff9ee`; сравнение с working tree, новых
коммитов в этой задаче нет. Review учитывал untracked новые `login/*`,
сопоставляя moved implementations с `git show` исходной ревизии.

| Ось | Итог |
|---|---|
| Standards | 0 незакрытых hard/эвристических замечаний |
| Spec | 0 незакрытых замечаний |

В ходе реализации устранены: несброшенный attempt при новом входе после Back;
оставшееся управление retry/следующим экраном в runner; очистка queued input
на каждом failed frame, поглощавшая R; устаревший diagnostic birth owner при Guide.
Последняя name retry ошибка подтверждена рабочим forbidden case после исправления.

Реальный перенос владения соединением игровому runtime не добавлен: flow destroy
по-прежнему освобождает startup connection. Принятые ADR, B/B2 evidence и ownership
соседних этапов не переоткрываются. Принято пользователем 2026-11-04.


## Граф кода

`.graphify-venv/bin/graphify update .` завершился с exit 0: code graph обновлён
до 20898 nodes / 55152 edges. Ограничения extractor (частично разбираемые старые
C-файлы и semantic extraction документов) не являются результатами компилятора
или повторной приёмкой legacy. Лог команды — `/tmp/sv-login-graph-update.log`.


## Команды выполненных core checks

Обе команды ниже выполнены integration owner с exit 0. Interaction stdout пустой,
mode stdout — `mode four-direction grid navigation, wrap and keypad PASS`;
отдельные logs этих двух запусков не сохранялись.

```sh
clang -std=c99 -Wall -Wextra -Werror -fsanitize=undefined -Isrc/client/sv tests/sv/login-interaction.c src/client/sv/protocol/login.c src/client/sv/login/characters/login-interaction.c src/client/sv/input/text-field.c src/client/sv/login/pregame.c src/client/sv/login/login-view.c -o /tmp/login-architecture-interaction
/tmp/login-architecture-interaction
clang -std=c99 -Wall -Wextra -Werror -fsanitize=undefined -Isrc/client/sv tests/sv/birth-mode-arrows.c src/client/sv/login/creation/native-birth.c src/client/sv/login/creation/birth.c src/client/sv/input/key-encoding.c $(pkg-config --cflags --libs sdl3) -o /tmp/login-architecture-mode
/tmp/login-architecture-mode
```


## Дополнительный smoke с пятью кадрами — 2026-11-04

По отдельному запросу пользователя выполнен один базовый визуальный smoke
через production endpoint с локальным peer. Scratch harness переиспользует
предыдущий paced replay; добавлены только переход через реальный server picker
и credentials editor, автоматическая подача синтетического ввода и захват
готового SDL framebuffer перед present. Клиентские исходники не менялись.

Результат PASS, процесс exit 0: выбор локального сервера → форма учётных данных
→ обзор персонажей → создание Carol → подтверждённый startup milestone.
Exact 16266-byte request и authoritative ACK/END подтверждены прежними
assertions; пять кадров проверены визуально. Скриншоты — реальные кадры
production renderer 1024×768, SDL dummy video + software rendering, без
запуска desktop compositor. BMP сохранены через SDL_RenderReadPixels/SDL_SaveBMP;
PNG — lossless conversion без изменения размеров/содержимого. Это базовый
smoke, не полная visual matrix и не игра на реальном сервере. Последний кадр
честно показывает существующую заглушку live-session handoff.

- [Выбор сервера](../../../.scratch/login-smoke-2026-11-04/01-server-selection.png)
- [Учётные данные](../../../.scratch/login-smoke-2026-11-04/02-account-login.png)
- [Обзор персонажей](../../../.scratch/login-smoke-2026-11-04/03-characters.png)
- [Создание персонажа: режим](../../../.scratch/login-smoke-2026-11-04/04-creation-mode.png)
- [Вход завершён до live handoff](../../../.scratch/login-smoke-2026-11-04/05-login-complete.png)

[Лог](../../../.scratch/login-smoke-2026-11-04/smoke.log),
[manifest](../../../.scratch/login-smoke-2026-11-04/manifest.json),
[build.py](../../../.scratch/login-smoke-2026-11-04/build.py),
[producer/capture harness](../../../.scratch/login-smoke-2026-11-04/endpoint.c).

Команда запуска после `python3 .scratch/login-smoke-2026-11-04/build.py`:

```sh
env SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software DBUS_SESSION_BUS_ADDRESS=unix:path=/tmp/sv-smoke-no-vault timeout 25s .scratch/login-smoke-2026-11-04/endpoint .scratch/login-smoke-2026-11-04/profile "$PWD/lib" ack tests/sv/fixtures/birth-current.hex .scratch/login-smoke-2026-11-04/play.bin 4.9.4.0.0.0
```

Для loopback socket потребовалось разрешённое sandbox escalation; первая
попытка внутри sandbox завершилась до запуска peer, поэтому в PASS не включена.

Приёмка: пользователь принял правки 2026-11-04 и поручил commit/push.
