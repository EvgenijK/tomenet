# SV-B-021 — Создание аккаунта

Статус: implementation-ready for review; build/core прошли 2026-10-06,
controlled-peer, Linux/Windows native и SV-B-025 downstream acceptance deferred.

## Пользовательский результат

Игрок создаёт новый аккаунт через те же native private fields и production
protocol path; только server response подтверждает создание и открывает overview.

## Зависимости и граница

Зависит от [SV-B-002](SV-B-002-contact.md),
[SV-B-005](SV-B-005-vault.md) и [SV-B-006](SV-B-006-login.md).
Смена пароля и account information перенесены в C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.account.create` | A valid unused account name and password follow server new-account handling; account flags/validation and resulting overview remain authoritative. | `source.baseline.session-credentials`, `source.protocol.session-login`, `source.baseline.session-login`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. New-account request exact bytes/version branches через production serializer;
   поля переиспользуют accepted input/private semantics без второй реализации.
2. Used/invalid name, bad password, account flag rejection, disconnect и retry
   сохраняют правильный parent и не показывают overview до server confirmation.
3. Successful authentication выполняет approved credential save policy; provider
   failure остаётся явным session-only состоянием.

## Definition of Done

Controlled peer покрывает success/rejection/cancel/error и fragmentation;
Linux/Windows native input и provider paths проверены. Результат включается в
[SV-B-025](SV-B-025-entry-complete.md).

## Implementation evidence 2026-10-06

Evidence относится к итоговой реализации tickets 01–03, объединённой в
`f0dab44955ed91c46797912c497780224b91f6ff`, и к packaging diff этого ticket.
Перед review прочитаны корневой `CONTEXT.md`, `docs/sv-architecture.md`,
ADR-0001–0006, `docs/agents/domain.md`, правила issue tracker и triage labels.
Password change, credential change-write и account information остались в
Stage C; этот handoff их не реализует и не принимает.

### Воспроизводимые build/core результаты

Среда: Linux 6.18.49-1-MANJARO x86_64, Clang 19.1.7, SDL 3.2.10,
SDL_ttf 3.2.2, FreeType 26.2.20, libsecret 0.21.7. Core использует
`SDL_VIDEODRIVER=dummy`, `SDL_RENDER_DRIVER=software` и Python environment
`/opt/sv-venv` с `jsonschema` 4.26.0.

| Check | Команда | Результат |
|---|---|---|
| `sv-build` | `make -s -C src -f makefile.sv tomenet-sv` | **pass**, exit 0; build key `.sv-build/linux/8b3130721e8ad8146e3e/build.txt`. Existing common `atol` warnings записаны отдельно, а не исправлены в SV-B-021. |
| `sv-core` | `PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin bash .sandcastle/checks.sh core` | **pass**, exit 0. Runner является repository source для contract invocation `bash -s -- core`; он больше не ссылается на удалённые при Stage B narrowing checks и включает production M1 и обе SV-B-021 suites. |

Core прошёл `sv_contact_checks.py`, `sv_endpoint_checks.py`,
`sv_login_checks.py`, `sv_login_live_checks.py`,
`sv_first_session_checks.py`, `sv_account_create_checks.py`,
`sv_account_failure_checks.py`, `sv_vault_checks.py` и остальные перечисленные
runner checks. Managed test peers и dummy/software rendering являются
автоматизированным branch evidence; они не заменяют external review,
controlled-peer checkpoint или native platform acceptance.

### Карта production seams

| Responsibility | Production seam | Evidence path |
|---|---|---|
| Native accepted/private input | `sv_endpoint_run(SvEndpointOptions)` вызывает существующие `input/credentials.c`, `input/native-endpoint.c` и `input/text-field.c`; отдельного account editor нет. | `sv_endpoint_checks.py`, `sv_account_create_checks.py`, cancel cases в `sv_account_failure_checks.py`. |
| Contact request and validation | `sv_contact_create`, `sv_contact_take_output`, `sv_contact_receive` через `protocol/contact.c` и production socket adapter. | `sv_contact_checks.py` и полный `sv_endpoint_run` loopback path. |
| Versioned login request/response | `sv_login_create`, `sv_login_take_output`, `sv_login_receive` в `protocol/login.c`. | `sv_login_checks.py`, `sv_login_live_checks.py`, `sv_account_create_checks.py`. |
| Server-authoritative presentation | `sv_pregame_sync_login`, `sv_pregame_fail`, `sv_pregame_disconnect`; generation принадлежит production pregame model. | `sv_login_checks.py`, success/failure account suites и M1 retry path. |
| Approved credential persistence | Production caller запускает `sv_vault_store` только после `pregame.authenticated`, затем использует `sv_vault_poll`/`sv_vault_cancel`. | Save/degradation/stale-generation cases в `sv_account_create_checks.py`; provider identity seam в `sv_vault_checks.py`. |

Tests компилируют и вызывают эти production SV sources. Provider doubles
находятся только на внешней OS-provider boundary и возвращают результаты
production caller; отдельной test-only реализации account state, serializer,
save policy или private-field semantics нет.

### Review packet: protocol, state и credential policy

Этот раздел является материалом для external checks, а не утверждением, что
`P1-protocol-review`, `P1-state-review` или
`P1-credential-policy-review` уже выполнены.

- Exact request bytes: `tests/sv/contact-negotiation.c` сравнивает независимые
  literal packets production contact/verify serializer для поддерживаемых
  protocol 1 и 2, включая plain/XOR42 password branch и atomic undersized
  output. `tests/sv/login.c` сравнивает двухбайтовый pre-4.9.2.1.0.2 initial
  login и восьмибайтовую branch с шестью iaddr bytes; full endpoint peer также
  проверяет modern contact/verify/login bytes. Никакой partial packet не
  считается request success.
- Authoritative overview: `SvPregame.authenticated`, creation/server flags,
  character count и `SV_PREGAME_OVERVIEW` публикуются только после полного
  server details + terminated character list. Success peer удерживает последний
  byte terminator и подтверждает отсутствие overview-dependent credential
  store до его получения.
- Rejection/parent matrix: contact invalid-name, bad-password и game-full/
  account-flag rejection завершают generation с server-shaped reason без
  overview; login `PKT_QUIT` покрывает used-account rejection; malformed input
  и disconnect дают terminal failure/disconnect; password cancel возвращает к
  account owner без send, account cancel выходит из startup; retry создаёт
  отличающийся generation и не принимает старый provider completion.
- Credential policy: store получает exact endpoint/account identity и exact
  synthetic password bytes только после server confirmation. Saved публикует
  `SAVED`; unavailable, locked, refused, invalid, error и teardown pending
  публикуют явный `SESSION_ONLY`, сохраняя confirmed overview. Plaintext/legacy
  fallback не добавлен, stale completion освобождается и не меняет новый result.

### Isolation и policy review handoff

Diff от planning base `232de737df` до implementation integration commit
`f0dab44955` меняет production code только в `src/client/sv`. Legacy и common
production sources не изменены. `.sandcastle/checks.sh` содержит минимальную
регистрацию текущих production-path checks и удаление ссылок на scripts,
удалённые прежним Stage B narrowing; tests остаются в `tests/`.

Поведение сохраняет contact/login wire baseline, server authority, private
input, parent/cancel semantics и session generation из ADR. Изоляция следует
актуальному `AGENTS.md`: старое требование общей SV/legacy реализации не
переопределяет эту policy, а intentional SV-local duplication само по себе не
считается defect. Во время slices обнаруженный resolver teardown follow-up и
наблюдавшийся common `atol` warning записаны отдельно в
`docs/sv-improvements.md` с problem, affected code, proposal, status и required
checks; эти записи не расширяют scope SV-B-021.

External review checks `P1-implementation-review`,
`P1-production-path-review`, `P1-protocol-review`, `P1-state-review`,
`P1-credential-policy-review`, `P2-isolation-review`, `P2-behavior-review`,
`P2-production-seam-review`, `P2-improvements-review`,
`P2-policy-precedence-review`, `P2-workflow-review` и
`P2-domain-doc-review` имеют приведённый выше review packet, но в этом ticket
не запускались и не обозначаются как passed.

### Deferred checks — не выполнены

| Check | Status | Причина | Owner |
|---|---|---|---|
| `P1-controlled-peer-runtime` | **deferred; not run** | `Requires a controlled-peer runtime environment; no allowed executable controlled-peer runner was supplied for this checkpoint.` Иными словами, нет разрешённого executable controlled-peer runner для checkpoint. Требуются success, rejection, cancel, error и fragmentation через production path; local managed test peers не являются этим acceptance. | SV-B-021 controlled-peer verification owner |
| `P1-linux-native-runtime` | **deferred; not run** | `Requires the Linux native input and credential-provider environment.` Dummy/software, branch coverage и provider doubles не являются native acceptance. | SV-B-021 Linux native-path verification owner |
| `P1-windows-native-runtime` | **deferred; not run** | `Requires the Windows native input and credential-provider environment.` Wine, cross-build или branch coverage не являются Windows native acceptance. | SV-B-021 Windows native-path verification owner |
| `P1-entry-complete-integration` | **deferred; not run** | `Downstream integration evidence belongs to SV-B-025 after SV-B-021 acceptance.` | SV-B-025 owner |

[SV-B-025](SV-B-025-entry-complete.md) этим handoff не изменён и не закрыт.
