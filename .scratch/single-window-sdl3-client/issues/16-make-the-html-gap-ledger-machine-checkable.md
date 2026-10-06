# Make the HTML gap ledger machine-checkable

Type: research
Status: resolved
Assignee: codex
Blocked by: 01, 07

## Question

Как атомарно сопоставить native capabilities с `MENUS.md`, `HOTKEYS.md` и фактическими prototype modules так, чтобы машинная проверка отличала approved UX и prototype-complete flows от protocol/native parity, обнаруживала drift и не превращала HTML fixtures в источник игрового поведения?

## Answer

Исследование: [HTML gap ledger: атомарное покрытие и проверяемые claims](../research/html-gap-ledger.md). Проверены локальные первичные документы и реальные entrypoints/modules HTML-прототипа; ключевые источники закреплены SHA-256, поскольку Git HEAD не фиксирует dirty worktree.

Consumer ledger явно учитывает каждую active canonical capability и нормализует mappings к UX assertions, surfaces/actions/states/bindings/contexts, actual routes, scenarios и acceptance evidence. Implementation coverage `missing|planned|prototype-complete` независимо от человеческого `ux-approved`; checkbox `[x]`, наличие модуля, synthetic success и screenshot-message не доказывают полный flow и не дают approval или protocol/native parity. Например, MENUS Ghost powers `[x]` покрывает только unavailable-строку, тогда как HOTKEYS `U` и execution flow отсутствуют.

Отчёт задаёт предлагаемые schema/semantic gates, полный source inventory с обнаружением добавления/удаления/изменения файлов, fingerprints manifest/evidence/approval, консервативную invalidation и staged local sync с recovery. Проверка различает честный `ledger-valid` с missing, `prototype-ready` и freshness `current|unavailable`; недоступный canonical source не даёт подтверждения актуальности. Более точная dependency invalidation требует доказанного closure.

Решение закрывает исследовательский вопрос о проверяемом контракте, а не заявляет готовый checker или завершённое атомарное покрытие. Canonical manifest/schema и HTML ledger сейчас отсутствуют; наполнение зависит от завершения baseline inventories. Реализация и выполнение предложенных checker acceptance cases относятся к следующему этапу; native evidence policy остаётся в [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md). Нового обязательного вопроса для карты исследование не выявило.
