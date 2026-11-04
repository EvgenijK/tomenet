# 10: Сохранить owner при session events

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Во время любого birth choice сеть продолжает обслуживаться, а focus и resize обновляют только presentation; interleaving не меняет owner, selection или draft. Disconnect и stale generation немедленно закрывают interaction и запрещают любые последующие sends.

**Blocked by:** 08: Выбрать mode и завершить draft.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-9-2

Labels: enhancement, ready-for-agent

**Production seam for TDD:** event sequences подаются через публичный `sv_endpoint_run(SvEndpointOptions)` и его production network/SDL loop, с outbound connection как наблюдаемой границей. Seam принят P1-AC21, P1-AC22 и P1-AC28; проверки используют `sv-core`.

- [x] Interleaved legal network traffic на каждом birth owner обслуживается без блокировки UI и не меняет owner, highlighted identity или draft. [Contract: P1-AC21, P1-AC28; checks: `sv-core`]
- [x] Focus lost/gained и resize меняют только native presentation/layout; после события тот же production owner и selection продолжают принимать keyboard/mouse input. [Contract: P1-AC11, P1-AC21; checks: `sv-core`]
- [x] Disconnect завершает текущую birth interaction, а input и queued result после закрытия не могут отправить bytes или изменить новую generation. [Contract: P1-AC22; checks: `sv-core`]
- [x] Любое действие со stale generation отклоняется до сериализации; outbound trace остаётся пустым для stale confirm, Backspace, quit и Guide-adjacent input. [Contract: P1-AC22, P1-AC29; checks: `sv-core`]
- [x] Lifecycle behavior соответствует main-thread/session-generation ADR и проверяется через production SV path без legacy/shared expansion. [Contract: P2-AC01, P2-AC04, P2-AC05, P2-AC12; checks: `controller-review`]

## Answer

Production birth interaction теперь имеет явный active/closed lifecycle. При
disconnect `run_contact()` закрывает interaction до маршрутизации следующих SDL
events, сбрасывает owner в `SV_BIRTH_NONE` и отделяет birth state от
transport-owned `SvContactSetup`. Нужные для outcome counts копируются в
session-owned state, поэтому публикация закрытого interaction не читает уже
освобождённое connection state. `sv_birth_command()` fail-closed отклоняет
команды закрытого или другого generation.

Native birth adapter получает timestamp epoch момента входа в birth. SDL events
старше epoch отбрасываются до построения semantic command, поэтому stale confirm,
Backspace, Q и Guide-adjacent `?` не меняют owner/highlight/draft и не достигают
какой-либо сериализации. При retry старый interaction закрывается, новая
generation получает новый epoch и сохраняет собственный draft.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → настоящий
contact/login network loop → `SV_PREGAME_BIRTH` → production SDL input adapter;
наблюдаемая граница — controlled outbound socket. Сценарий interleave получает
и подтверждает семь legal `PKT_PING` replies по одному на sex/class/race/trait/
body/stats/mode, между selection и последующим keyboard/mouse input подаёт focus
lost/gained и resize, после чего сверяет полный готовый draft. Disconnect и retry
сценарии проверяют EOF без post-close bytes, terminal owner и разные generation.

TDD (`SV_BIRTH_CASE=<case> python3 -B tests/sv_name_editor_submit_checks.py`):

- RED `session-stale-input`: exit 1, runtime assertion — stale confirm/Q были
  приняты, female highlight не сохранился. GREEN после birth input epoch: exit 0.
- RED `session-disconnect`: exit 1 на compile — public outcome не имел
  active/closed lifecycle. Первый минимальный close переход затем выявил ASan
  heap-use-after-free на borrowed setup после освобождения connection. GREEN
  после session-owned counts и detach setup: exit 0, без sanitizer errors.
- `session-interleave` и `session-retry-generation` добавлены как production-path
  coverage уже сохранявшегося network/presentation behavior вокруг исправленных
  lifecycle границ; оба exit 0.

Focused verification:

- `SV_BIRTH_CASE=session-interleave python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `SV_BIRTH_CASE=session-stale-input python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `SV_BIRTH_CASE=session-disconnect python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `SV_BIRTH_CASE=session-retry-generation python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `SV_BIRTH_CASE=mode-ordinary-ready python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Изменения ограничены SV и его
production-path tests; legacy/shared gameplay code не менялся. Возможностей
улучшения вне scope текущей задачи не обнаружено.
