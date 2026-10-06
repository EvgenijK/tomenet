# Reallocate post-B work by functional block

Type: decision
Status: resolved
Assignee: user

## Question

Как заменить широкие этапы C–G последовательностью пригодных к реализации
этапов, если каждый новый этап должен владеть только одним функциональным блоком,
получать лишь минимально необходимые зависимости и появляться в порядке,
позволяющем как можно раньше пользоваться клиентом?

## Comments

### Confirmed by the user — 2026-10-06

Пользователь потребовал полностью перераспределить нынешний объём C–G по
функциональным блокам. Этапы A и B, их документы, implementation tickets и
canonical allocation не затрагиваются.

Ранние post-B этапы должны сначала обеспечить загрузку необходимых данных, затем
минимальную живую игровую сессию: карту, управление персонажем, предметы и другие
основные игровые действия. Guide, macros, audio, alerts, screenshots, OS
integration, party/social и interface settings относятся к поздней части плана.

## Answer

Этапы A и B сохраняются как grandfathered checkpoints с текущим составом
**A8/B54**. Новое правило применяется только к 863 active outcomes, сейчас
распределённым между C–G. Никакой A/B outcome, prerequisite, owner или acceptance
boundary не меняется в рамках этой reallocation.

Каждый новый post-B этап владеет outcomes ровно одного функционального блока.
Один post-B этап владеет не более чем 50 outcomes; замороженный B54 не входит в
это ограничение и остаётся неизменным.
Функциональный блок определяется канонической stage-классификацией outcome;
исходной классификацией служит namespace `capability.<block>.*`. Этап может
содержать вспомогательную production-инфраструктуру и regression checks других
уже завершённых блоков, но не получает ownership их outcomes и не принимает их
заново.

Зависимость другого блока реализуется в более раннем этапе. Её нельзя скрыто
подтянуть в текущий этап под видом минимального scope. Если capability DAG не
позволяет завершить весь блок одной фазой, один блок можно разделить на несколько
этапов, не объединяя его с другим блоком. Составной outcome при необходимости
остаётся в поздней фазе либо разделяется на самостоятельно проверяемые outcomes;
частичная acceptance под старым ID запрещена.

Порядок post-B этапов определяется минимальной полезностью работающего клиента и
реальными prerequisites. Сначала идут загрузка данных и необходимые session
primitives, затем карта и управление персонажем, основные игровые блоки и лишь
после них дополнительные и интеграционные возможности. Указанные пользователем
Guide, macros, audio, alerts, screenshots, OS integration, party/social и
interface settings не должны становиться ранними этапами только из-за общего
parser, surface primitive или historical allocation.

Cross-block milestone, cumulative regression, package review и финальная
приёмка оформляются как verification gates без capability ownership. Они не
являются исключением из правила одного блока на этап.

Machine-readable stage catalog задаёт стабильный ID, явный порядок и один block
каждого post-B этапа. Canonical validator использует этот порядок, а не
лексикографическое сравнение stage IDs. A/B защищаются отдельным regression lock;
изменение post-B allocation само по себе не изменяет их semantic projection.

Подробные migration tickets находятся в
[stage reallocation plan](../../../docs/tasks/stage-reallocation/README.md).
Старые C–G строки [implementation sequence](../migration-sequence.md), broad
[Stage C spec](../../../docs/sv-stage-c-spec.md) и Stage G как смешанный
Guide/files/platform gate superseded этим решением. Смысл прежнего Guide
решения — реализовывать Guide поздно — сохраняется, но буква G и смешанное
ownership больше не нормативны. Граница B из decision 37 остаётся без изменений;
его ссылка на C означает начало новой post-B последовательности, а не один
широкий этап.

Архитектурные решения ADR-0001…0006 не меняются: это новая acceptance allocation,
а не новая граница runtime-модулей.
