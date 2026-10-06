# SV-B-001 — Endpoint surface и native input primitives

Статус: частично реализован; полная acceptance по суженной границе B pending.
[Существующее evidence](../../sv-b001-evidence.md) сохраняется только там, где
оно подтверждает перечисленные ниже IDs и остаётся актуальным.

## Пользовательский результат

Игрок выбирает сервер или вводит host/port в native whole-window surface,
редактирует поле физической клавиатурой/input method и может отменить startup
без подключения к угаданному endpoint.

## Зависимости и граница

Предыдущих B owners нет; используется принятый foundation A. Реализуются общие
input primitives только через первый production caller. Gameplay raw keys,
макросы, clipboard outcome и последующие игровые contexts относятся к C.

Поздние actual-caller checks: [SV-B-006](SV-B-006-login.md),
[SV-B-022](SV-B-022-character-manage.md) и
[SV-B-023](SV-B-023-birth-choices.md).

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.connection.select-server` | Select a metaserver entry and preserve its endpoint; ping results may update without changing the selected server. | `source.baseline.session-server`, `source.policy.session` |
| `capability.connection.enter-host` | Enter host and effective port manually or from startup arguments; no hostname-to-IP canonicalization of credential identity. | `source.baseline.session-server`, `source.baseline.session-connect`, `source.policy.session` |
| `capability.connection.cancel-host` | Escape cancels manual entry and follows the startup caller exit path without contacting a guessed/default host. | `source.baseline.session-server`, `source.policy.session` |
| `capability.input.physical-keys` | Preserve printable, function, keypad, navigation, modifier and lock key byte sequences on Linux/Windows; X11-compatible macro triggers remain usable. | `source.reconciliation.input.physical-keys` |
| `capability.input.prompt-navigation` | Decode navigation and caller-specific macro bypass without changing parent input context. | `source.reconciliation.input.prompt-navigation` |
| `capability.input.text-edit` | Edit bounded byte fields with cursor, history/search, clipboard and private masking; preserve caller-specific accept/cancel/empty behavior and approved field safety policy. | `source.reconciliation.input.text-edit`, `source.behavior.information-askfor-aux` |
| `capability.input.confirm` | Preserve distinct binary/default/retry/ternary confirmation contracts and queue flush; caller decides whether cancel aborts an action. | `source.reconciliation.input.confirm` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Metaserver refresh не меняет выбранный endpoint; manual host/port и startup
   arguments дают одинаковую identity, Escape не создаёт connect attempt.
2. Production SDL adapter и input router проверяются на Linux и Windows:
   printable/function/keypad/navigation/modifier/lock keys, input method,
   cursor/selection/history, boundary−1/boundary/boundary+1, accept/cancel.
3. Focus/resize и interleaved peer event не теряют draft, caret, selection или
   parent. Encoding error оставляет исправляемый draft и не отправляет bytes.
4. Повторить primitives в login, character-name и birth callers. Наличие API
   или unit-only editor не принимает capability.

## Definition of Done

Все canonical obligations, prerequisites, source IDs и allocation hashes
зафиксированы в [coverage.json](coverage.json). Тесты вызывают production
SDL→router→model→surface path; нет terminal fallback или test-only editor.
Секретные ограничения дополнительно принимает SV-B-005. Реальный TomeNET
server не требуется; сетевую границу начинает SV-B-002.
