# 02: Сохранить parent state при отказах создания аккаунта

**Type:** implementation

**What to build:** При used/invalid name, bad password, server/account-flag rejection, cancel, disconnect или retry игрок остаётся в правильном pregame owner, видит причину ровно в разрешённом состоянии и никогда не получает overview без подтверждения сервера. Retry начинает свежую session generation.

**Blocked by:** 01: Создать неиспользованный аккаунт через production flow.

**Status:** resolved

**Assignee:** codex/sandcastle-f5fbf91a-wave-2-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** controlled peer и native events входят через публичный `sv_endpoint_run(SvEndpointOptions)`; fragment/error assertions используют production receive/state seams `sv_contact_receive`, `sv_login_receive`, `sv_pregame_sync_login` и `sv_pregame_disconnect`. Эти seams проверяют P1-4/P1-8 через `P1-state-review`, не создавая отдельного тестового автомата.

- [x] Used or invalid account name, bad password и server/account-flag rejection показывают server-shaped failure, сохраняют предусмотренного retry/exit owner и не публикуют overview ни до, ни после отказа. [Contract: P1-4, P1-5, P1-8, P2-3; checks: `sv-build`, `sv-core`, `P1-state-review`, `P2-behavior-review`]
- [x] Cancel из account-name и password steps использует существующие native private-field semantics: password cancel возвращает к account owner без отправки, account cancel выходит из startup; секрет очищается в соответствии с владельцем. [Contract: P1-3, P1-7, P1-8; checks: `sv-build`, `sv-core`, `P1-production-path-review`, `P1-state-review`]
- [x] Disconnect и malformed/error outcomes на каждом ожидании завершают текущую generation с одной понятной причиной; incomplete или fragmented delivery не создаёт partial overview, ложного success или повторного side effect. [Contract: P1-4, P1-8, P2-3; checks: `sv-build`, `sv-core`, `P1-state-review`, `P2-behavior-review`]
- [x] Retry создаёт свежую generation, повторно проходит production contact/login path и не принимает старые bytes, callbacks, provider completion или pending interaction; разрешённый parent после нового server response остаётся тем же, что в baseline. [Contract: P1-8, P2-3, P2-4; checks: `sv-build`, `sv-core`, `P1-state-review`, `P2-production-seam-review`]
- [x] Автоматизированные проверки управляют только peer bytes/events/faults и вызывают production decoder, model, interaction, serializer и UI seams; отдельной реализации account-creation behavior для тестов нет. [Contract: P2-4; checks: `sv-core`, `P2-production-seam-review`]
- [x] Изменения ограничены account-creation failures в SV и не добавляют password change, account information либо общий legacy refactor. [Contract: P1-2, P2-1, P2-2, P2-8; checks: `P1-implementation-review`, `P2-isolation-review`, `P2-policy-precedence-review`]

## Answer

Использованы согласованные production seams: native events и controlled peer
входят через `sv_endpoint_run(SvEndpointOptions)`, а terminal/fragment/stale
переходы проверяются через `sv_contact_receive`, `sv_login_receive`,
`sv_pregame_sync_login` и `sv_pregame_disconnect`. До изменений прочитаны
корневой `CONTEXT.md`, `docs/sv-architecture.md`, ADR-0001–0006 и правила local
Markdown; dependency 01 была resolved в исходной ветке.

Pregame failure/disconnect теперь terminal для своей generation: сохраняется
первая понятная причина, поздние bytes/callbacks не могут сменить owner или
опубликовать overview. Contact rejections публикуются как server failure, а
transport/decode outcomes — как disconnect; endpoint outcome содержит причину
только соответствующей завершённой попытки. Retry повторяет production
contact/login path с гарантированно отличающейся generation; `prior_generation`
делает эту границу проверяемой, а новый успешный ответ не наследует старую
причину, flags или interaction. Existing private-field flow сохранён: password
cancel очищает secret и возвращает к account step, account cancel выходит без
contact.

TDD seams и red → green:

- `sv_pregame_disconnect` / `sv_pregame_sync_login`:
  `python3 -B tests/sv_login_checks.py` — red на повторном disconnect, который
  менял terminal reason; после terminal generation guard — green.
- `sv_endpoint_run`: `python3 -B tests/sv_account_failure_checks.py` — red на
  отсутствующем production failure reason; после server-failure publication —
  green для fragmented invalid-name rejection.
- `sv_endpoint_run` retry: тот же command — red compile на отсутствующем
  `prior_generation`; после явного fresh-generation handoff — green для полного
  набора controlled-peer/native-event сценариев.

Focused checks:

- `python3 -B tests/sv_account_failure_checks.py` — passed: invalid/used name,
  bad password, server/account-flag rejection, fragmented reason, malformed
  login, partial disconnect, retry и оба native cancel parent.
- `python3 -B tests/sv_login_checks.py` — passed: atomic overview, terminal and
  stale-generation state seams.
- `python3 -B tests/sv_contact_checks.py` — passed: fragmented contact/verify/
  setup, rejection and malformed production decoder paths.
- `python3 -B tests/sv_account_create_checks.py` — passed: accepted account
  creation and server-authoritative overview remain intact.
- `python3 -B tests/sv_first_session_checks.py` — passed: existing
  success/disconnect/quit/retry startup flow remains intact.
- `python3 -B tests/sv_endpoint_checks.py` — passed: native endpoint and private
  credential interactions remain intact.

Полный build/test/review workflow не запускался согласно handoff; его выполняет
orchestrator. Наблюдавшийся вне scope diagnostic detached resolver записан
отдельно в `docs/sv-improvements.md`; legacy/shared implementation, password
change и account information не изменялись.
