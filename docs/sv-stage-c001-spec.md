# C001 — запуск установленного SV-клиента

Статус: accepted, 2026-11-04, по явному решению пользователя.
[Реализация и Linux evidence](sv-c001-evidence.md),
[решение и техническая приёмка](sv-stage-c001-acceptance.md).
План от 2026-10-31 реализован; canonical JSON не изменялся.

## Пользовательский результат

Игрок распаковывает Linux amd64 дистрибутив и запускает `tomenet-sv` без
служебных аргументов: открывается принятый production startup UI B2.
Запуск не зависит от checkout, sibling web prototype или текущего каталога.
Поставляемые ресурсы находятся относительно binary; пользовательские ресурсы
и записи используют существующий client profile contract.

Единственный owned outcome — `capability.platform.package-launch`.
[Каталог](capabilities/stages.json), [manifest](capabilities/manifest.json) и
[ledger](capabilities/native-coverage.json) остаются каноническими; у outcome
`prerequisites=[]`, implementation/evidence `pending`. B и B2 приняты;
их accepted behavior используется без повторной приёмки. План не завершает
общие SV-REALLOC-005/006/007.

## Локальный semantic sweep и решения

Каталог/manifest/ledger согласованы. Узкий sweep выявил пять чуждых связей
inventory → package-launch в reconciliation: rows 281, 681, 976, 1252, 1805
(admin, audio/Windows overrides, item/spell names). Полный разбор, включая
остальные четыре связи, — [исследование 01](../.scratch/sv-c001-planning/issues/01-scope-and-ownership.md).
Эти связи не добавляют outcomes C001. Source-backed applicability disposition всех девяти связей, включая эти пять,
[выполнен в 001](tasks/c001/reconciliation-disposition.md), без изменения
canonical JSON. Sweep не объявляется
полностью clean; весь REALLOC-005 остаётся незавершённым.

[Shipping decision 08](../.scratch/single-window-sdl3-client/issues/08-name-and-ship-the-new-client.md)
задаёт Fedora41 baseline и имя `tomenet-sv-<version>-linux-amd64.tar.bz2`.
Они сохраняются. Плановая интерпретация canonical scope C001 и
[decision 38](../.scratch/single-window-sdl3-client/issues/38-reallocate-post-b-by-functional-block.md):
package-launch проверяется полностью для текущего функционального содержимого.
Пометка pregame-only ограничивает содержимое архива, а не объём принимаемого
outcome: частичная acceptance под старым ID недопустима. Требования final release
к audio, archive extraction, screenshots и Guide сохраняются в поздних
этапах/итоговом verification. C001 не объявляет текущий архив полноценным
финальным клиентом.

## Production граница

- SV-local Linux packaging entrypoint собирает архив из актуального production
  binary, необходимых `lib/` ресурсов, runtime libraries и лицензий. Состав
  фиксируется явным inventory; секреты, реальные профили, build caches и внешние
  checkout в пакет не попадают. Legacy release workflow не требуется менять.
- Предпочтительное решение обычного запуска — default endpoint dispatch в
  `src/client/sv/main.c`. Явные `--endpoint` и `--synthetic` сохраняются;
  несовместимые аргументы по-прежнему отклоняются. Старое ожидание ошибки bare
  launch в existing smoke актуализируется точечно, без новой suite.
- Переиспользуются существующие profile/library/font/B2 resource paths.
  Исправлять их только при подтверждённой package-facing проблеме. Технический
  lookup для startup не означает приёмку общего overlay/persistence outcome C003.
- До сборки архива зафиксировать конкретную Linux reference environment,
  на утверждённой Fedora41 builder baseline, требуемую libc/loader и
  системные prerequisites, а также что bundled.
  Evidence относится только к указанной среде: amd64 не означает все Linux.
  Включить все необходимые ELF-зависимости либо явно перечислить допустимые
  системные prerequisites; проверка только на build host недостаточна.

C002 settings bootstrap, полные C003 file operations, C006/C007 fonts/rendering,
C057 optional integrations и финальный пакет полного gameplay клиента остаются
в своих этапах. Не менять protocol/gameplay, legacy/common и инфраструктуру
релизов ради устранения дублирования. Если необходим отсутствующий outcome
соседнего этапа, сначала оформить конфликт prerequisites/order, не расширять
C001 молча. Применяются [CONTEXT](../CONTEXT.md),
[архитектура](sv-architecture.md), [ADR-0003](adr/0003-explicit-module-connections.md),
[ADR-0006](adr/0006-errors-and-overload.md) и [AGENTS](../AGENTS.md).

## Приёмка и evidence

Только Linux согласно [правилу платформ](agents/platform-validation.md).
Windows/MinGW/Wine deferred, без фиктивного PASS. Проверяется извлечённый
архив, а не executable из checkout. Свежие временные profile/HOME/XDG paths,
никаких реальных аккаунтов и секретов; server login не нужен для package-launch.
Для успешного explicit `--profile-root` заранее создать временный каталог:
сегодня endpoint выбирает этот путь, но не создаёт его. Synthetic marker/bootstrap
для production профиля не использовать.

Минимум: build/package command и fingerprint; состав и лицензии; фактический
ELF closure в заявленной runtime-среде; обычный запуск из постороннего cwd;
видимый production endpoint с bundled B2 assets/fonts и корректный выход;
проверка существующего user-resource precedence на одном startup consumer;
применимые ошибки обязательного ресурса, недоступного профиля и loader dependency.
Для каждого результата указать actual paths, exit/result и диагностический вывод.
Пакетный старт подтверждается на software и доступном accelerated backend
короткими запусками; B2 visual/input matrix заново не выполняется.

Обе obligations `.result` и `.failure-lifecycle` получают явное disposition.
Cancel — обычный выход startup UI. Server cancellation, concurrent edit,
replace/write failures проверяются лишь для операций, которыми реально владеет
package launch; неприменимость обосновывается, generic текст не создаёт новую
matrix. Нельзя маскировать отсутствие ресурса неявным чтением checkout или
заявлять успех только по SDL initialization/списку файлов.

Переиспользовать существующие production checks только по diff; новый core test
допустим исключительно для необходимого изменения production behavior. Новый
harness, расширение suites и stress/matrix требуют отдельного предварительного
согласования согласно AGENTS.md. Во время планирования тесты не запускаются.

Evidence собирается при реализации в `docs/sv-c001-evidence.md`: команды,
revision/build/archive hashes, OS/loader/library versions, inventory, actual
resource/profile paths, запуск/выход/ошибки и явно pending/deferred результаты.
Изменение canonical evidence/implementation статуса допустимо только после
фактических результатов через существующий registry workflow; сам план их
не меняет. Итоговый acceptance gate сверяет evidence и представляет результат
пользователю без автоматической отметки accepted.

## План

[Два implementation tickets и ownership snapshot](tasks/c001/README.md).
[Исследования трёх агентов](../.scratch/sv-c001-planning/map.md).
