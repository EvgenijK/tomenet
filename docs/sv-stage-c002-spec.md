# C002 — bootstrap настроек SV

Статус: accepted, 2026-11-07, по явному решению пользователя.
Реализация от 2026-11-05 и birth test follow-up от 2026-11-07 приняты в Linux scope.
[Evidence](sv-c002-evidence.md), [техническая сверка](sv-stage-c002-acceptance.md).
Ниже сохранён исходный план; initial audio и
[CLI boundary](tasks/c002/cli-disposition.md) разрешены при реализации.
[Задачи и ownership](tasks/c002/README.md).

## Результат и нормативная база

Обычный production запуск загружает собственные настройки SV, применяет defaults
и overrides до их первого использования. Настройки персонажа попадают в первый
пакет опций; выход не сохраняет CFG/OPT автоматически. UI настроек не добавляется.

Этап владеет шестью IDs `capability.settings.*`: `defaults`, `parse`, `load`,
`aliases`, `cli-overrides`, `exit-unsaved`. В [каталоге](capabilities/stages.json)
это Settings bootstrap; у всех шести строк [ledger](capabilities/native-coverage.json)
prerequisites пусты, implementation/evidence pending. Полные conditions и obligations
сохраняются в [локальном snapshot](tasks/c002/coverage.json); распределение задач
не сужает их. B/B2 и C001 используются как принятые предыдущие этапы.

Приоритет имеют [AGENTS](../AGENTS.md), [CONTEXT](../CONTEXT.md),
[settings policy](capabilities/settings-policy.md),
[архитектура](sv-architecture.md), [ADR-0002](adr/0002-main-thread-state-ownership.md),
[ADR-0003](adr/0003-explicit-module-connections.md) и
[ADR-0006](adr/0006-errors-and-overload.md).
Архивные SV-B-003/004 — навигация к источникам, а не актуальные требования процесса.

## Точка старта

На 2026-11-05 `src/client/sv/main.c` делегирует запуск
`tests/sv/diagnostics/launcher.c`; production login находится в
`src/client/sv/login/native-run.c` и `login/flow.c`.
`profile_first_run()` проверяет наличие CFG, но это не загрузка настроек.
`src/client/sv/protocol/login.c` копирует `sv_stub_play_options` в login packet.
Прежних `src/client/sv/profile.c` и `settings.c` в текущем дереве нет:
исторические заявления о готовом parser/load нельзя переносить на текущий код.

Перед реализацией 001 выполнить узкую сверку шести rows ledger с manifest,
reconciliation и cited baseline owners: `client.c`, `c-files.c`, `c-util.c`,
`nclient.c`, SDL3 CFG reader/defaults и client stock CFG. Зафиксировать конкретные
ключи, defaults, aliases, слои, CLI precedence и version gates, необходимые C002.
Не запускать общий semantic sweep всех этапов.

## Границы реализации

- `U` — существующий SDL3 user root; `S=U/sv`; `B` — выбранный game library.
  Собственные CFG/OPT читаются из S. Shared resources и fingerprint остаются
  привязаны к U. Bootstrap includes не подмешивают legacy user CFG/OPT.
- Defaults берутся из client stock/compiled baseline с утверждёнными SV
  overrides: fullscreen/wide/UI100%, CascadiaMono-Regular.ttf, 16x24x.pcf,
  graphics1/16x24sv, Linear tiles/Nearest PCF, FPS100, audio44100/32/1024,
  volumes70. Отсутствие CFG не создаёт и не переписывает CFG/OPT.
- CFG остаётся line-oriented: LF/CRLF, полные tokens, comments/flags,
  обработка duplicates/invalid values по policy. Supported unknown records
  сохраняются в памяти для будущего Save; incompatible future schema включает
  defaults, диагностику и запрет перезаписи. `pass` не загружается и не считается
  сохраняемым unknown. Никакого guessed encoding или plaintext password fallback.
- Последовательность слоёв и pre-send point берётся из policy и baseline.
  C002 владеет settings data и option entrypoints S/options.prf,
  S/global.opt, S/global-sv.opt, S/<character>.opt. Не вводить class-OPT autoload.
  Полная загрузка shared preferences, mapping и macro execution остаётся C004/C052/C053.
  Includes требуют явного resolved owner/provenance; одного расширения PRF недостаточно.
- Создать минимальный SV-local владелец settings data, например `settings/`.
  Имена и число файлов уточняются по существующим callers. Связать его с startup,
  shell и login через явные интерфейсы; renderer не читает файлы, protocol не
  выбирает paths. Менять legacy/common только при доказанной необходимости.
- Из launcher перенести лишь необходимую production обработку настроек/overrides,
  сохранив совместимость существующих аргументов. Полный refactor diagnostics не
  входит в этап. Ошибки сообщаются без содержимого секретных полей.
- Нормальный quit/retry/relog не пишет pending CFG/OPT. Это не отменяет
  самостоятельные lifecycles vault/history/DNA и других существующих owners.

Полные resource operations/persistence — C003; preference behavior — C004;
font/render outcomes — C006/C007; settings UI/Save/conflicts — C044;
import — C051; полный audio — C055; editable configuration — C056.
Данные для этих consumers сохраняются без объявления их outcomes готовыми.

## Граница initial audio и других поздних consumers

У `settings.load` сохранены obligations `startup-audio-packs` и
`startup-audio-device`: actual initial open, requested/effective identity,
disabled/missing sound, no-device responsiveness и Send_audio. Parser-only
результат их не закрывает. До реализации 002 проверить текущего production
consumer и нормативную границу с C055/C056, включая старые ссылки B/E в ledger.
Аналогично проверить, какие font/mapping consumers нужны для точного load order.

Если требуется ещё отсутствующий полный outcome позднего этапа, оформить
source-backed решение о prerequisites/order или metadata correction прежде,
чем реализовывать этот участок. Не удалять obligations и не подтягивать полный
audio/rendering/preferences блок молча. Пока решение или обязательное evidence
отсутствует, соответствующая acceptance остаётся pending; независимые работы
001/003 могут продолжаться. Это открытый вопрос плана, а не готовое disposition.

## Минимальная проверка и приёмка

Только [Linux](agents/platform-validation.md). Проверки ограничены необходимым
core, вызывают тот же settings/startup/serializer production путь. Использовать
existing test entrypoints; при необходимости добавить один компактный core test
settings, без отдельной реализации поведения или нового harness.
Расширение набора проверок согласовать заранее по AGENTS.md.

Минимальные группы: defaults/invalid CFG; слои и exact aliases; CLI precedence;
первый option packet с character values и сохранёнными version gates; отсутствие
CFG/OPT writes на startup/exit. Временные roots содержат различающиеся SV/legacy
значения; фиксируются фактические paths и bytes до/после. Для aliases нужны все
различающиеся source branches, а не случайный один rename. Проверки unavailable
read и future schema входят в core cases. Остальные generic failure obligations
получают обоснованное applicable/not-applicable disposition по реальному caller.
Нет записи — не строить новую Save fault matrix для будущего C044.

Короткий production startup/quit подтверждает, что конфигурация реально достигает
существующего UI. Проверки затронутого login serializer используют существующий
peer, без реального аккаунта. Повторять B2 visual matrix или packaging C001 целиком
не требуется. Initial audio evidence определяется сохранёнными obligations и
решением о границе выше, а не объявляется parser PASS.

При реализации вести `docs/sv-c002-evidence.md`: revision/build, команды,
результаты expected/actual, production callers, paths, wire bytes и disposition
каждой obligation. Плановые проверки сейчас не запускаются. Acceptance ticket
сводит результаты и оставшиеся pending/deferred пункты для решения пользователя;
план сам по себе не меняет canonical JSON и не ставит accepted.
