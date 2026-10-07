# SV-B-023 birth choices Wayfinder effort

Label: wayfinder:map
Status: open

## Destination and canonical ownership

Этот effort хранит Wayfinder planning/execution history decomposition для
canonical Stage B implementation ticket
[SV-B-023](../../docs/tasks/stage-b/SV-B-023-birth-choices.md), который остаётся
единственным владельцем перечисленных там `capability.birth.*` IDs. Canonical
allocation остаётся в [Stage B index](../../docs/tasks/stage-b/README.md); child
records этого effort не являются вторым набором SV implementation tickets.

Immutable acceptance contract:
[acceptance-contract.json](acceptance-contract.json), digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`.
Batch manifests: [batch 0](batch-0.json), [batch 1](batch-1.json),
[batch 2](batch-2.json), [batch 3](batch-3.json), [batch 4](batch-4.json).

`P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у указанного в
immutable contract producer/verifier ownership. Wayfinder reconciliation не
объявляет этот check выполненным и не меняет production behavior или evidence.

## Frontier

- [Восстановить обычный owner graph после post-mode PvP Maia](issues/32-unwind-post-mode-pvp-maia-context.md) — unclaimed; blockers 06, 15 и 23 resolved, поэтому ticket готов к работе.

## Decisions so far

- [Войти в birth и выбрать пол](issues/01-enter-birth-and-choose-sex.md)
- [Выбрать класс в default compile order](issues/02-choose-class-in-default-order.md)
- [Выбрать race с class compatibility](issues/03-choose-race-with-class-compatibility.md)
- [Выбрать или пропустить trait](issues/04-choose-or-skip-trait.md)
- [Выбрать или пропустить body](issues/05-choose-or-skip-body.md)
- [Сохранить alternate class/race compile order](issues/06-preserve-alternate-class-race-order.md)
- [Выбрать stats](issues/07-choose-stats.md)
- [Выбрать mode и завершить draft](issues/08-choose-mode-and-complete-draft.md)
- [Завершить или вернуться без send](issues/09-terminate-or-backtrack-without-send.md)
- [Сохранить owner при session events](issues/10-preserve-owner-across-session-events.md)
- [Вернуться из birth Guide placeholder](issues/11-return-from-birth-guide-placeholder.md)
- [Проверить границу birth implementation](issues/12-verify-birth-implementation-boundary.md)
- [Verify focused production coverage](issues/13-focused-test-coverage.md)
- [Изолировать production birth input context](issues/14-isolate-birth-input-context.md)
- [Восстановить stats при backtrack и повторном входе](issues/15-restore-stats-on-backtrack-reentry.md)
- [Разрешить hidden derived class при выборе race](issues/16-resolve-hidden-derived-class.md)
- [Безопасно обработать random при пустом class list](issues/17-handle-empty-class-random-safely.md)
- [Соблюсти first-run и Archer body legality](issues/18-enforce-first-run-body-legality.md)
- [Продолжить automatic DNA choices через birth flow](issues/19-continue-automatic-dna-choices.md)
- [Verify focused production coverage](issues/20-focused-test-coverage.md)
- [Сохранить hidden DNA class до подтверждения race](issues/21-preserve-hidden-dna-class-until-race.md)
- [Передать authoritative first-run state в birth](issues/22-carry-authoritative-first-run-state.md)
- [Войти в dedicated birth через production exclusive selection](issues/23-enter-dedicated-birth-from-exclusive-selection.md)
- [Нормализовать point-buy DNA отдельно для каждого stat](issues/24-normalize-point-buy-dna-per-stat.md)
- [Ограничить Guide canonical birth owners](issues/25-restrict-guide-to-canonical-birth-owners.md)
- [Verify focused production coverage](issues/26-focused-test-coverage.md)
- [Получить human disposition для stable hidden DNA race highlight](issues/27-disposition-stable-hidden-dna-race-highlight.md)
- [Пропустить unusable stats для unknown creation setup](issues/28-skip-unusable-stats-for-unknown-setup.md)
- [Сделать длинные birth choice lists полностью достижимыми](issues/29-reach-long-birth-choice-lists.md)
- [Привести SV birth decomposition к canonical tracker layout](issues/30-reconcile-sv-birth-tracker-layout.md)
- [Verify focused production coverage](issues/31-focused-test-coverage.md)
