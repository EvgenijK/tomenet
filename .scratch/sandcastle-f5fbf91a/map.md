# SV-B-021 account creation implementation effort

Label: wayfinder:map
Status: open

## Destination

Реализовать и проверить production account-creation flow SV-B-021, сохраняя
server-authoritative protocol и gameplay semantics, approved credential policy,
изоляцию SV и явные external acceptance deferrals.

## Decisions so far

- [Создать неиспользованный аккаунт через production flow](issues/01-create-unused-account-through-production-flow.md)
- [Сохранить parent state при отказах создания аккаунта](issues/02-preserve-account-creation-failure-parents.md)
- [Явно обработать отказ сохранения credentials](issues/03-handle-credential-save-degradation.md)
- [Подготовить implementation evidence и deferred runtime handoff](issues/04-package-implementation-evidence-and-deferred-runtime.md)
- [Verify focused production coverage](issues/05-focused-test-coverage.md)
- [Исправить core registry после сужения Stage B](issues/06-repair-sv-core-test-registry-after-stage-b-narrowing.md)
- [Verify focused production coverage](issues/07-focused-test-coverage.md)
- [Согласовать account-create acceptance check с production suites](issues/08-reconcile-account-create-acceptance-check.md)
- [Проверить восстановленные account-create gates на assembled HEAD](issues/09-verify-recovered-account-create-gates.md)
- [Verify focused production coverage](issues/10-focused-test-coverage.md)
- [Вернуть change set SV-B-021 в account scope](issues/11-return-sv-b021-to-account-scope.md)
- [Зафиксировать current controlled-peer acceptance](issues/12-record-controlled-peer-acceptance.md)
- [Восстановить индекс и metadata Wayfinder effort](issues/13-restore-wayfinder-effort-index.md)
- [Verify focused production coverage](issues/14-focused-test-coverage.md)
- [Индексировать последний resolved Wayfinder child](issues/15-index-latest-resolved-wayfinder-child.md)
- [Канонизировать причины contact rejection](issues/16-canonicalize-contact-rejection-reasons.md)
