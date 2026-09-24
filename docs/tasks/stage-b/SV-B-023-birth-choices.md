# SV-B-023 — Рождение: выборы и backtracking

Статус: specified; реализация и runtime evidence не выполнены.

## Пользовательский результат

Игрок проходит baseline birth choices и возвращается на предусмотренный предыдущий шаг.

## Зависимости и граница

Завершить необходимые production части [SV-B-008](SV-B-008-guide.md), [SV-B-011](SV-B-011-transfer.md), [SV-B-022](SV-B-022-character-manage.md).

Граф задаёт порядок готовности production implementation для следующих задач; это не автоматическое закрытие полной acceptance. Runtime branches и fixtures не обязаны исполняться последовательно. Полный primary owner сохраняет acceptance pending до всех своих obligations и перечисленных поздних integration checks; readiness prerequisites canonical ledger при этом не меняются. Точный полный список capability prerequisites, sources и obligation IDs для каждого owner находится в [coverage.json](coverage.json); hashes связывают его с неизменённым canonical registry. Инженерные зависимости выше добавляют конкретных потребителей, не меняя ledger.

## Production subsets и поздние integration checks


Этот тикет выполняет повторные/недостающие actual-caller проверки для [SV-B-008](SV-B-008-guide.md), [SV-B-011](SV-B-011-transfer.md). Использовать их production code, сохранить каждый исходный obligation и вернуть evidence первоначальному owner.

Полная таблица ответственности и связей — [coverage.json](coverage.json); [две границы готовности](../../sv-stage-b-spec.md#readiness-and-integration) различают implementation DAG и acceptance closure.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Первичные источники |
|---|---|---|
| `capability.birth.sex` | Complete the sex choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | [c-birth.c:318](../../../src/client/c-birth.c#L318)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.race` | Complete the race choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | [c-birth.c:423](../../../src/client/c-birth.c#L423)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.trait` | Complete the trait choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | [c-birth.c:645](../../../src/client/c-birth.c#L645)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.class` | Complete the class choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | [c-birth.c:887](../../../src/client/c-birth.c#L887)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.body` | Complete the body choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | [c-birth.c:1899](../../../src/client/c-birth.c#L1899)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.stats` | Complete the stats choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | [c-birth.c:1129](../../../src/client/c-birth.c#L1129)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.mode` | Complete the mode choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | [c-birth.c:671](../../../src/client/c-birth.c#L671)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.quit` | Q or Ctrl-Q terminates creation through baseline quit/retry handling without sending a completed character. | [c-birth.c:318](../../../src/client/c-birth.c#L318)<br>[c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.backtrack-race` | Backspace from race returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | [c-birth.c:423](../../../src/client/c-birth.c#L423)<br>[c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.backtrack-trait` | Backspace from trait returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | [c-birth.c:645](../../../src/client/c-birth.c#L645)<br>[c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.backtrack-class` | Backspace from class returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | [c-birth.c:887](../../../src/client/c-birth.c#L887)<br>[c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.backtrack-body` | Backspace from body returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | [c-birth.c:1899](../../../src/client/c-birth.c#L1899)<br>[c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.backtrack-stats` | Backspace from stats returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | [c-birth.c:1129](../../../src/client/c-birth.c#L1129)<br>[c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
| `capability.birth.backtrack-mode` | Backspace from mode returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | [c-birth.c:671](../../../src/client/c-birth.c#L671)<br>[c-birth.c:2344](../../../src/client/c-birth.c#L2344)<br>[session-policy.md:10](../../capabilities/session-policy.md#L10) |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Все race/trait/class/body/stats/mode branches/build availability, server/Lua descriptions; keyboard/macros и backtracking сохраняют допустимые values.
2. Guide topics для каждого birth step идут через native read/search/close и восстанавливают тот же pending selection; missing Guide/failed search не теряет форму.
3. Resize/focus/network update под choice, quit/teardown закрывают только baseline continuation; ничего не отправляется повторно.

Для каждого собственного ID дополнительно обязательны следующие условия; это требования будущей реализации, а не результаты выполненных тестов.

| ID | Конкретные проверки и ранние handoffs |
|---|---|
| `capability.birth.sex` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Complete the sex choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. |
| `capability.birth.race` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Complete the race choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. |
| `capability.birth.trait` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Complete the trait choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. |
| `capability.birth.class` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Complete the class choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. |
| `capability.birth.body` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Complete the body choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. |
| `capability.birth.stats` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Complete the stats choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. |
| `capability.birth.mode` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Complete the mode choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. |
| `capability.birth.quit` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Q or Ctrl-Q terminates creation through baseline quit/retry handling without sending a completed character. |
| `capability.birth.backtrack-race` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Backspace from race returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. |
| `capability.birth.backtrack-trait` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Backspace from trait returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. |
| `capability.birth.backtrack-class` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Backspace from class returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. |
| `capability.birth.backtrack-body` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Backspace from body returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. |
| `capability.birth.backtrack-stats` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Backspace from stats returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. |
| `capability.birth.backtrack-mode` | Этот шаг: legal/random/DNA выбор, Backspace/Escape/quit, CLASS_BEFORE_RACE, skipped traits/body, dedicated IDDC/PvP; Guide return и один финальный send. Точный проверяемый результат: Backspace from mode returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. |

[Общий обязательный recipe](../../sv-stage-b-spec.md#verification) применяется к каждому пути success/cancel/error: production decoder/router/model/renderer/serializer, bytes и split/chained input, актуальный parent, macro/physical routes, interleaved network, focus/resize и stale generation. Fixture подменяет peer/clock/filesystem/provider inputs, но не реализацию поведения.

## Версии, build gates и источники

- `capability.birth.sex`, `capability.birth.race`, `capability.birth.trait`, `capability.birth.class`, `capability.birth.body`, `capability.birth.stats`, `capability.birth.mode`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — CLASS_BEFORE_RACE; RETRY_LOGIN; server race/class/trait tables; dedicated/PvP/RPG/arcade mode flags; auto_reincarnation..
- `capability.birth.quit`, `capability.birth.backtrack-race`, `capability.birth.backtrack-trait`, `capability.birth.backtrack-class`, `capability.birth.backtrack-body`, `capability.birth.backtrack-stats`, `capability.birth.backtrack-mode`: versions — All supported baseline servers; server-owned availability and rejection remain authoritative.; builds — Supported SDL3/X11 behavior baseline; normal and roguelike keysets and ordered user macro overrides..

Версионные границы читаются в перечисленных primary sources соответствующей manifest revision; номер строки — навигация в текущем checkout, literal anchor и full-file SHA берутся из [manifest](../../capabilities/manifest.json). Не считать одну текущую server version проверкой всех ветвей. [Session byte policy](../../capabilities/session-policy.md), [persistence/resource policy](../../capabilities/settings-policy.md), [layout/stage policy](../../capabilities/item-policy.md) имеют приоритет над историческими дефектами и Terminal topology.

## Evidence и Definition of Done

- Production code расположен преимущественно в SV по [правилу изоляции](../../../AGENTS.md) и [архитектуре](../../sv-architecture.md). Нет test-only decoder/behavior, нового virtual Term или незапрошенного legacy refactor. Обнаруженные отдельные улучшения записаны отдельно.
- Готовность implementation позволяет продолжать зависимые задачи; закрытие полного acceptance требует также перечисленных поздних integration checks. Каждый принадлежащий тикету ID сохраняет весь исходный outcome и ВСЕ его existing obligations; таблицы не сужают `.result`, `.lifecycle`, `.wire` или прочие условия canonical ledger. Реализация caller не принимается по успеху общего primitive.
- Автоматизированные тесты вызывают production seam и фиксируют exact expected/actual values, safe command/reply bytes и generation/fallback observations; native visual/input review использует тот же executable. Evidence содержит revision/config/server/build/platform/renderer, проверенные source/fixture/resource/SDK fingerprints и complete dependency scope. Секреты и пользовательский private content в отчёты не попадают.
- Linux software и accelerated, отдельный MinGW i686 build/Wine intermediate smoke; actual Windows10/11 software/accelerated обязательны для B, platform-specific behavior проверяется при появлении. Частичные наблюдения сохраняются pending; missing/failed/stale evidence не проходит gate.
- Выполнены относящиеся к изменению cumulative A regressions и consumer scenarios; все нарушения20/50/200ms submission deadlines записаны. Нет принятого B flow с fallback entry. Полный matrix, human review и актуальность evidence сводятся в [SV-B-075](SV-B-075-acceptance.md).

## Ограничения после тикета

Тикет не заявляет полноту B в одиночку. Quantity/item selection/transactions C, полные lore/document/context-help/chat-cancel caller unions D, macro editing/recording/wizard, INS management, reimport и audio pack/device editors E сохраняют свои этапы. Ранние branches/handoffs проверяются у существующих B owners без сужения поздних IDs. Успешный death transition не принимает ghost powers; parse/Save значения не принимает поздний consumer.
