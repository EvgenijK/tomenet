# SV-C006-001 — active map font, overlay и reporting C003

Статус: planned, 2026-11-09.
Flow evidence: pending; реализация и native evidence отсутствуют.
Labels: enhancement
Связка: FI-01.
Stage outcomes: `capability.fonts.map-select`, `capability.fonts.fallback`,
`capability.fonts.live-failure` в рамках их полного C006 контракта.
C003 obligations: `capability.files.resource-overlay`,
`capability.files.report-resource-identity`; владельцы —
[SV-C003-001](../c003/SV-C003-001-file-owners.md) и
[SV-C003-005](../c003/SV-C003-005-resource-reporting.md).

## Решение и production consumer

Решение пользователя 2026-11-09 сохраняет границы этапов: C003 предоставляет
production file/reporting core и source-backed missing/disabled map identity,
пока настоящий consumer отсутствует. C006 реализует map-font consumer и завершает
его flow evidence. Missing-only branch и injected snapshot не доказывают active
font lifecycle. Canonical ownership, prerequisites и statuses не меняются;
C003 принят пользователем 2026-11-09 без ожидания этой интеграции.

Нормативные источники: [CONTEXT](../../../CONTEXT.md),
[settings policy](../../capabilities/settings-policy.md),
[архитектура](../../sv-architecture.md),
[ADR-0002](../../adr/0002-main-thread-state-ownership.md),
[ADR-0003](../../adr/0003-explicit-module-connections.md),
[ADR-0005](../../adr/0005-session-scoped-work.md),
[ADR-0006](../../adr/0006-errors-and-overload.md).

Production consumer — владелец реально используемого map font/profile, а не UI
text font и не значение `svMapFont` само по себе. Исходный seam для сверки:
`src/client/sv/login/flow.c` → `protocol/login.c:sv_login_start_play`; C003 обязан
устранить прежнюю передачу UI text font как map font. При реализации сверить
актуальные C003 public interfaces вместо предположения об уже готовых API.

## Работа

1. Загружать map font и связанные profiles/mappings через C003 resolved file owner:
   общий SDL3 user root U имеет приоритет перед bundled B. Read/stat и применимые
   resource mutations сохраняют общий owner; не дублировать поиск или файловые
   алгоритмы. Ошибочный overlay отличать от отсутствующего; fallback выбирает
   consumer по settings policy, а не универсальное игнорирование ошибок.
2. Сохранять полную requested reference в settings, отдельно публиковать actual
   effective map font после успешной загрузки/подготовки. Выполнить нормативный
   порядок map-font → mapping reload → graphics overrides → server visual update.
   Font должен реально обслуживать font-based map rendering; создание объекта
   loader без применения к карте не закрывает задачу. Не перетаскивать весь C007.
3. Подключить effective identity к одному production reporting owner C003-005:
   первоначальный PLAY и последующий font/resource exchange согласованы с actual
   consumer; version report сохраняет применимые metadata. Не добавлять поля
   протокола для requested/effective: использовать baseline смысл существующих
   полей, оставляя полные requested данные локально.
4. Text-only font change не меняет map identity/mappings и не выдаётся за map
   resource change. Startup failure применяет согласованный fallback с ошибкой;
   failed live change сохраняет прежний рабочий набор и effective identity.
   Reinit/recreation не публикует старое поколение и не создаёт повторных effects.
5. Активные tiles и их lifecycle передать C007 через actual resource state;
   отсутствие graphics consumer не выдавать за enabled evidence. App-owned
   resources могут переживать session; отправка всегда относится к текущему
   generation. Не добавлять настройки C056 или audio flows C055.

## Завершение и минимальные проверки

Через existing Linux core/peer production interfaces проверить U-before-B,
missing/invalid overlay и declared fallback; реальную активацию map font;
успешную смену, failed change с сохранением прежнего stack, reinit/relog;
независимость text font. Captured PLAY/font/version bytes должны соответствовать
фактическому consumer и source-defined version gates C003 serializer.
Для применимых `%s` полей reporting предел — 79 bytes + NUL (`MAX_CHARS=80`),
даже если server font buffer больше: проверить 78/79/80 bytes без изменения full
local reference. Использовать C003 serializer checks повторно, расширяя только
необходимым consumer evidence, а не новым общим harness.

Вернуть в `docs/sv-c003-evidence.md` и [SV-C003-006](../c003/SV-C003-006-acceptance.md)
FI-01: concrete production caller/revision, Linux команды, expected/actual
resolved paths, requested/effective identities и wire bytes; сопоставить
применимые `resource-overlay` obligations и все четыре
`report-resource-identity.{result,failure-lifecycle,wire,allocation-checks}`
obligations. Не заявлять выполненными ветви, для которых consumer/evidence ещё
отсутствует. FI-02 C007 остаётся отдельным обязательством. C006 этим не принимается; пользовательская приёмка C003 уже дана.
Возврат evidence не prerequisite C003, ошибки исправляются в C006.

Проверки — только минимальный core production path; расширение согласуется
заранее по AGENTS.md. [Linux обязателен](../../agents/platform-validation.md),
Windows/MinGW/Wine deferred. Этот план не содержит новых runtime PASS.

## Приёмка C003 — уточнение 2026-11-09

Пользователь явно принял C003 и задачи 001–006 без ожидания поздних проверок.
Требования этого документа остаются обязательствами своего этапа; его реализация
и приёмка не заявлены. Непроверенные flows остаются непроверенными. Проблемы
функционала C003, найденные здесь, исправляются в рамках этого этапа; они не
блокируют и не переоткрывают C003 автоматически. Evidence можно добавить в
сводку C003 для трассировки, но это не prerequisite уже состоявшейся приёмки.
Canonical runtime registration C003 остаётся отдельным workflow.
