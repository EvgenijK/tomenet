# 23: Войти в dedicated birth через production E selection

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Игрок с разрешённым exclusive slot нажимает `E` в production character overview, проходит тот же native name/server-confirmation flow и входит в dedicated IDDC или PvP birth с сохранённым intent; недоступный или first-run exclusive action не начинает creation и не отправляет request.

**Blocked by:** 08: Выбрать mode и завершить draft (completed); 22: Передать authoritative first-run state в birth.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-16-1

Labels: bug, ready-for-agent

**Finding ID:** `D-fa58f4def44cd460` (`birth.dedicated-entry-production-handoff`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative overview/server flags и реальные SDL `E`, name-edit и confirm events; production login interaction, pregame handoff and birth interaction carry dedicated intent, while controlled peer and outcome observe requests, owner, legal modes and draft. Seam принят P1-AC01, P1-AC02, P1-AC20 и P1-AC28; проверки — `sv-core` и `controller-review`. Test-only `birth_dedicated` injection не может быть единственным входом coverage.

- [x] Production overview распознаёт `E` как отдельный exclusive creation action только когда authoritative limits/flags and first-run eligibility разрешают dedicated slot; unavailable action сохраняет overview и не отправляет name/character request. [Contract: P1-AC01, P1-AC02, P1-AC20, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Authorized `E` проходит native name editing, server acceptance/rejection and cancel semantics, а dedicated intent принадлежит interaction/session state и переживает handoff до начала birth без dependency на test-only options. [Contract: P1-AC01, P1-AC02, P1-AC20, P1-AC28; checks: `sv-core`, `controller-review`]
- [x] После server-confirmed name production birth предлагает соответствующие authoritative dedicated IDDC или dedicated PvP modes и сохраняет post-mode PvP Maia trait-return semantics; ordinary `N` path остаётся ordinary. [Contract: P1-AC02, P1-AC09, P1-AC20, P2-AC04; checks: `sv-core`]
- [x] Complete caller-to-birth scenarios проходят для IDDC и PvP server variants в обеих compile configurations через реальные `E` events; они не используют direct dedicated injection как sole entry и не добавляют fixed-response/test-only wizard. [Contract: P1-AC19, P1-AC20, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Exclusive selection, rejection, cancel and birth choices не отправляют partial/completed character result; final completed-character send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC20, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Реализация минимизирует изменения общих и legacy files и сохраняет protocol/gameplay semantics production B-022→B-023 handoff. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

Согласованный public production seam: `sv_endpoint_run(SvEndpointOptions)` с
authoritative overview/server flags, profile-owned first-run state и реальными SDL
`E`, name-edit и confirm events. Native login router теперь различает uppercase
`E` и `N`. Login interaction вычисляет свободные IDDC/PvP exclusive slots из
server version, limits, flags и roster modes, запрещает `E` на first start и
владеет выбранным dedicated intent на всём name flow.

При submit endpoint явно передаёт intent и разрешённые dedicated mode flags в
session-owned `SvPregame`; после server acceptance birth получает только этот
production handoff. При server rejection retry повторно открывает тот же native
editor как exclusive и пересчитывает доступные flags по новому authoritative
overview. Cancel очищает intent. Test-only `SvEndpointOptions.birth_dedicated`
удалён, а существующие dedicated scenarios переведены на реальные `E` events.

Dedicated IDDC-only birth публикует только IDDC, IDDC Soloist и IDDC Hellish;
PvP-only birth публикует только dedicated PvP. PvP Maia проходит post-mode возврат
к trait owner, повторный stats/mode flow и завершает draft с dedicated PvP.
Обычные `N` scenarios продолжают входить в ordinary birth.

TDD red → green, 2026-10-07:

- Caller-to-IDDC slice, seam `sv_endpoint_run`:
  `SV_BIRTH_CASE=exclusive-iddc-production python3 -B
  tests/sv_name_editor_submit_checks.py` — **RED**, exit 1: controlled peer не
  получил name request и endpoint истёк по timeout, потому что production `E`
  маршрутизировался как выбор slot `e`, а dedicated intent существовал только в
  test option. После минимального native action → login interaction → pregame →
  birth handoff тот же command — **GREEN**, exit 0 для
  `SV_CLASS_BEFORE_RACE=1` и `=0`.

Focused production verification:

- `SV_BIRTH_CASE=exclusive-pvp-maia-production python3 -B
  tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0, обе compile
  configurations; PvP-only legal mode и post-mode Maia trait return.
- `SV_BIRTH_CASE=stats-order-reenter-pvp-maia python3 -B
  tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0, обе configurations;
  production `E`, stats re-entry и final dedicated PvP draft.
- `python3 -B tests/sv_ordinary_create_cancel_checks.py` — **PASS**, exit 0;
  authorized cancel, unavailable full dedicated slots и authoritative first-run
  с существующим roster остаются в overview без character request.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — **PASS**, exit 0;
  exclusive server rejection/retry сохраняет intent и draft.
- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0; полный
  focused birth/name suite в обеих compile configurations с
  `-Wall -Wextra -Werror` и ASan/UBSan. Controlled peers после допустимых name
  requests требуют EOF, поэтому partial/completed-character send провалил бы
  scenarios selection, rejection, cancel или birth choices.
- `python3 -B tests/sv_login_checks.py` — **PASS**, exit 0.
- `git diff --check` — **PASS**, exit 0.

Полный build/test/review workflow не запускался: assembled gates выполняет
orchestrator. Изменены только SV production files, тесты и этот ticket; legacy и
shared files, DNA persistence и final completed-character send не затронуты.
Новых improvement opportunities не обнаружено. Diagnostics о fallback font и
`Leaked thread` совпадают с уже известным focused test environment output.

`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** по исходной
acceptance-phase причине и не объявляется выполненным этим implementation ticket.
