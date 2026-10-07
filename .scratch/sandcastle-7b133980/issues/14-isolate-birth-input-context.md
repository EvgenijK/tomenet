# 14: Изолировать production birth input context

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Пока birth interaction активна, каждый keyboard/mouse gesture обрабатывается только её input context: известные gestures становятся semantic birth commands, остальные безопасно потребляются, а Escape в point-buy выполняет baseline confirmation. Никакой birth input не подтверждает login/MOTD state и не завершает endpoint в обход birth owner.

**Blocked by:** 13: Verify focused production coverage (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-12-1

Labels: bug, ready-for-agent

**Finding ID:** `D-a8ac860d1445a95f` (`birth.production-input-routing-boundary`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` с активным production `SvBirth`; реальные SDL keyboard/mouse events проходят через native birth adapter и semantic input router, а outcome и controlled outbound connection наблюдают owner/draft/lifecycle. Seam принят P1-AC08, P1-AC11 и P1-AC28; проверки — `sv-core` и `controller-review`.

- [x] При активном birth каждый gesture либо преобразуется в semantic birth command, либо потребляется birth context; illegal/unknown input и Escape не попадают в login/name/MOTD routing и не меняют чужое interaction state. [Contract: P1-AC11, P1-AC28; checks: `sv-core`]
- [x] Escape в point-buy проходит baseline confirmation flow: учитывает pending confirmation и оставшиеся points, сохраняет корректные owner/highlight/draft и не завершает endpoint как глобальный Escape. [Contract: P1-AC08, P1-AC11, P2-AC04; checks: `sv-core`]
- [x] Native adapter не меняет interaction state, включая ignored/error counters, до проверки active generation; stale или closed-generation input не имеет state/network effects. [Contract: P1-AC22, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Незавершённый birth interaction принадлежит documented input-router/interaction boundary либо доступен через неё без test-only interface; production tests вызывают тот же route для default и alternate compile orders. [Contract: P1-AC19, P1-AC28, P2-AC01, P2-AC05, P2-AC12; checks: `sv-build`, `sv-core`, `controller-review`]
- [x] Ни consumed input, ни point-buy confirmation, ни stale input не отправляют partial/completed character result; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC29; checks: `sv-core`, `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Managed peer
передаёт authoritative setup, endpoint создаёт production `SvBirth`, реальные
SDL keyboard/mouse events проходят через `sv_native_birth_event()` и
`sv_birth_command()`, а `SvEndpointOutcome`, lifecycle и обязательный EOF после
единственного name packet наблюдают owner/draft и отсутствие partial/completed
character send. Отдельного test-only router или choice implementation нет.

Birth теперь является исключительным input context: после начала interaction
endpoint передаёт event только native birth adapter и потребляет даже
`SV_BIRTH_INPUT_IGNORED`, не позволяя ему попасть в login/name/MOTD или global
Escape routing. Window close остаётся lifecycle event и обрабатывается до birth
route. В point-buy Escape переводится в существующую semantic command
`SV_BIRTH_CONFIRM`; при оставшихся points первый Escape устанавливает pending
confirmation с неизменным owner/highlight/draft, повторный подтверждает, а при
нулевом остатке подтверждение сразу передаёт owner в mode.

Native adapter сначала проверяет совпадение generation, `active` и `quit`, и
лишь затем рассматривает timestamp, gesture и меняет navigation/illegal
counters. Поэтому stale/closed-generation event не может открыть placeholder,
поменять counter/draft или произвести network effect. Публичная граница
незавершённого interaction закреплена в `input/birth-interaction.h`; endpoint,
UI и SDL adapter больше не включают backing `session/birth.h` напрямую, а
владение описано в `docs/sv-architecture.md`. Возможность позднее переместить
сам backing source без расширения текущей задачи записана отдельно в
`docs/sv-improvements.md` со статусом и required checks.

TDD red → green через согласованный production seam:

- Slice input-context isolation: `SV_BIRTH_CASE=birth-context-escape python3 -B tests/sv_name_editor_submit_checks.py` — **RED**, exit 1: unknown birth key подтвердил чужое MOTD state, после чего Escape завершил endpoint до birth Q (`result` assertion). После exclusive birth consumption та же команда — **GREEN**, exit 0 для default и `SV_CLASS_BEFORE_RACE=0` binaries.
- Slice point-buy Escape: `SV_BIRTH_CASE=stats-point-escape-warning python3 -B tests/sv_name_editor_submit_checks.py` — **RED**, exit 1: `birth_stats_confirmation_pending` не был установлен. После mapping Escape в `SV_BIRTH_CONFIRM` та же команда — **GREEN**, exit 0 для обеих compile configurations.
- Дополнительные production cases `stats-point-escape-confirm` и
  `stats-point-escape-no-warning` прошли с первого запуска после второго slice:
  они проверяют повторное подтверждение с 30 remaining points и немедленное
  подтверждение при нулевом остатке, включая точный stats draft.

Focused result, 2026-10-07:

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0; полная
  production endpoint matrix для default и `SV_CLASS_BEFORE_RACE=0`, сборка с
  ASan/UBSan, включая новые context/Escape cases и существующие
  stale/disconnect/retry-generation checks.
- `git diff --check` — **PASS**, exit 0.

Managed peer во всех новых cases после допустимого name packet требует EOF;
лишний outbound byte роняет test, поэтому consumed input, point-buy confirmation
и lifecycle filtering не отправляют partial/completed result. Финальный send
остаётся у SV-B-024. Повторяющиеся diagnostics `Leaked thread` и declared font
fallback уже записаны отдельно и не расширяют ticket. Полные `sv-build`,
`sv-core` и controller review не запускались: assembled gates выполняет
orchestrator.

`P1-birth-runtime-evidence` остаётся **deferred; not run** по исходной причине:
completion scope контракта — implementation, а evidence собирается только на
full acceptance для exact accepted contract digest
`690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82` и exact
тогдашнего HEAD. Producer — SV-B-023 implementation assignee; verifier —
Sandcastle controller full-acceptance verifier.
