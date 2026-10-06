# 04: Подготовить implementation evidence и deferred runtime handoff

**What to build:** Реализация SV-B-021 передаётся на review с воспроизводимыми build/core результатами, явной картой production seams и честным перечнем ещё не выполненных controlled-peer, platform и downstream checks. Ни один deferred check не обозначается как пройденный.

**Blocked by:** 02: Сохранить parent state при отказах создания аккаунта; 03: Явно обработать отказ сохранения credentials.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

- [ ] `sv-build` и `sv-core` выполнены на итоговой реализации; evidence связывает результат с использованием production contact/vault/login seams и подтверждает, что password change/account information остались в Stage C. [Contract: P1-1, P1-2, P1-3, P1-7; checks: `sv-build`, `sv-core`, `P1-implementation-review`, `P1-production-path-review`]
- [ ] Review evidence отдельно покрывает exact supported-version request bytes, server-authoritative overview transition, rejection/parent-state matrix и credential save/session-only failure policy, не подменяя external review автоматическими тестами. [Contract: P1-4, P1-5, P1-6, P1-8, P1-9; checks: `P1-protocol-review`, `P1-state-review`, `P1-credential-policy-review`]
- [ ] Diff review подтверждает SV-local placement, минимальные legacy/shared изменения, baseline semantics и отсутствие test-only behavior implementation; прежнее требование общей SV/legacy реализации не переопределяет актуальную isolation policy. [Contract: P2-1, P2-2, P2-3, P2-4, P2-8; checks: `P2-isolation-review`, `P2-behavior-review`, `P2-production-seam-review`, `P2-policy-precedence-review`]
- [ ] Все обнаруженные, но не входящие в SV-B-021 улучшения legacy/shared/SV либо отсутствуют, либо записаны отдельно с problem, affected code, proposal, status и required checks; запись не расширяет текущий implementation scope и intentional isolation duplication сама по себе не считается defect. [Contract: P2-5, P2-6, P2-7; checks: `P2-improvements-review`, `P2-isolation-review`]
- [ ] Task artifacts остаются в разрешённом local Markdown location, сохраняют отдельные Status/Labels fields и standard triage role; evidence фиксирует прочтение root context и относящихся ADR до архитектурной работы. [Contract: P2-9, P2-10, P2-11; checks: `P2-workflow-review`, `P2-domain-doc-review`]
- [ ] Controlled-peer success/rejection/cancel/error/fragmentation runtime check записан как deferred с причиной «нет разрешённого executable controlled-peer runner для checkpoint», владельцем SV-B-021 controlled-peer verification owner и без claim о прохождении. [Contract: P1-10; check: `P1-controlled-peer-runtime`]
- [ ] Linux native private-input/provider и Windows native private-input/provider checks записаны как deferred с их contract reasons/owners; Wine или branch coverage не выдаются за platform acceptance. [Contract: P1-11, P1-12; checks: `P1-linux-native-runtime`, `P1-windows-native-runtime`]
- [ ] Включение принятого результата в SV-B-025 записано как downstream deferred check владельца SV-B-025; parent task не закрывается и не изменяется этой декомпозицией. [Contract: P1-13; check: `P1-entry-complete-integration`]
