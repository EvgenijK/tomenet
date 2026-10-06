# 04: Подготовить implementation evidence и deferred runtime handoff

**Type:** implementation

**What to build:** Реализация SV-B-021 передаётся на review с воспроизводимыми build/core результатами, явной картой production seams и честным перечнем ещё не выполненных controlled-peer, platform и downstream checks. Ни один deferred check не обозначается как пройденный.

**Blocked by:** 02: Сохранить parent state при отказах создания аккаунта; 03: Явно обработать отказ сохранения credentials.

**Status:** resolved

**Assignee:** codex/sandcastle-f5fbf91a-wave-3-1

Labels: enhancement, ready-for-agent

- [x] `sv-build` и `sv-core` выполнены на итоговой реализации; evidence связывает результат с использованием production contact/vault/login seams и подтверждает, что password change/account information остались в Stage C. [Contract: P1-1, P1-2, P1-3, P1-7; checks: `sv-build`, `sv-core`, `P1-implementation-review`, `P1-production-path-review`]
- [x] Review evidence отдельно покрывает exact supported-version request bytes, server-authoritative overview transition, rejection/parent-state matrix и credential save/session-only failure policy, не подменяя external review автоматическими тестами. [Contract: P1-4, P1-5, P1-6, P1-8, P1-9; checks: `P1-protocol-review`, `P1-state-review`, `P1-credential-policy-review`]
- [x] Diff review подтверждает SV-local placement, минимальные legacy/shared изменения, baseline semantics и отсутствие test-only behavior implementation; прежнее требование общей SV/legacy реализации не переопределяет актуальную isolation policy. [Contract: P2-1, P2-2, P2-3, P2-4, P2-8; checks: `P2-isolation-review`, `P2-behavior-review`, `P2-production-seam-review`, `P2-policy-precedence-review`]
- [x] Все обнаруженные, но не входящие в SV-B-021 улучшения legacy/shared/SV либо отсутствуют, либо записаны отдельно с problem, affected code, proposal, status и required checks; запись не расширяет текущий implementation scope и intentional isolation duplication сама по себе не считается defect. [Contract: P2-5, P2-6, P2-7; checks: `P2-improvements-review`, `P2-isolation-review`]
- [x] Task artifacts остаются в разрешённом local Markdown location, сохраняют отдельные Status/Labels fields и standard triage role; evidence фиксирует прочтение root context и относящихся ADR до архитектурной работы. [Contract: P2-9, P2-10, P2-11; checks: `P2-workflow-review`, `P2-domain-doc-review`]
- [x] Controlled-peer success/rejection/cancel/error/fragmentation runtime check записан как deferred с причиной «нет разрешённого executable controlled-peer runner для checkpoint», владельцем SV-B-021 controlled-peer verification owner и без claim о прохождении. [Contract: P1-10; check: `P1-controlled-peer-runtime`]
- [x] Linux native private-input/provider и Windows native private-input/provider checks записаны как deferred с их contract reasons/owners; Wine или branch coverage не выдаются за platform acceptance. [Contract: P1-11, P1-12; checks: `P1-linux-native-runtime`, `P1-windows-native-runtime`]
- [x] Включение принятого результата в SV-B-025 записано как downstream deferred check владельца SV-B-025; parent task не закрывается и не изменяется этой декомпозицией. [Contract: P1-13; check: `P1-entry-complete-integration`]

## Answer

Implementation/review handoff записан непосредственно в owning ticket
[SV-B-021](../../../docs/tasks/stage-b/SV-B-021-account-manage.md). Он содержит
воспроизводимую среду и команды, карту production seams, отдельные protocol,
state, credential-policy и isolation review packets, а также таблицу deferred
checks с contract reasons и owners. External review checks не запускались и не
обозначены passed.

Итоговые current checks:

- `make -s -C src -f makefile.sv tomenet-sv` — pass, exit 0; build key
  `.sv-build/linux/8b3130721e8ad8146e3e/build.txt`.
- `PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin bash
  .sandcastle/checks.sh core` — pass, exit 0. Для воспроизводимости runner
  очищен только от ссылок на четыре scripts, удалённые прежним Stage B
  narrowing, и включает production `sv_first_session_checks.py` плюс обе
  SV-B-021 suites.

Согласованные production seams не менялись: публичный
`sv_endpoint_run(SvEndpointOptions)`, serializers/decoders
`sv_contact_*` и `sv_login_*`, authoritative state seams
`sv_pregame_sync_login` / `sv_pregame_fail` /
`sv_pregame_disconnect`, provider lifecycle `sv_vault_store` /
`sv_vault_poll` / `sv_vault_cancel`. Ticket 04 не менял production behavior и
не добавлял coverage нового поведения, поэтому новый red→green цикл по skill
`tdd` не заявляется. Red/green slices реализации находятся в сохранённых
Answers tickets 01–03; здесь выполнены final build/core checks через те же
production seams без test-only state или policy implementation.

`P1-controlled-peer-runtime` остаётся **deferred; not run**: нет разрешённого
executable controlled-peer runner для checkpoint; owner — SV-B-021
controlled-peer verification owner. `P1-linux-native-runtime` и
`P1-windows-native-runtime` остаются **deferred; not run** с contract native
environment reasons и соответствующими SV-B-021 native-path owners; managed
peers, dummy/software, provider doubles, Wine или branch coverage их не
принимают. `P1-entry-complete-integration` остаётся **deferred; not run** у
SV-B-025 owner; сам SV-B-025 не изменён и не закрыт.

Diff review подтверждает SV-local production implementation и отсутствие
legacy/common production edits. Минимальная shared workflow правка ограничена
актуализацией core runner. Resolver teardown и обнаруженное build warning для
obsolete common `atol` declaration записаны отдельно в
`docs/sv-improvements.md` со всеми обязательными полями; ни одна запись не
расширяет scope SV-B-021. Root context, SV architecture, ADR-0001–0006 и local
Markdown workflow rules были прочитаны до packaging review; отдельные
`Status`/`Labels`/`Assignee` fields и standard `ready-for-agent` triage label
сохранены.
