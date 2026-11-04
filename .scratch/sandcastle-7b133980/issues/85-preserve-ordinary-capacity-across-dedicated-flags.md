# 85: Сохранить ordinary capacity при optional dedicated flags

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Игрок с доступным ordinary slot может начать обычное создание персонажа при любом сочетании advertised dedicated IDDC/PvP feature flags. Encoded dedicated limits резервируют место только для действительно поддерживаемых dedicated modes: ordinary capacity остаётся равной decoded ordinary limit с учётом общего client cap, а effective total capacity увеличивается только на supported dedicated capacity.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-64-1

Labels: bug, ready-for-agent

**Finding ID:** `D-d037649996d38bb2` (`sv.creation-availability.reserves-unsupported-dedicated-slots`), one coherent repair of SV character-creation availability accounting and its production-boundary regressions.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative contact limits, dedicated feature flags и roster от controlled peer; реальные keyboard/mouse create actions проходят production login interaction/input router, а `SvEndpointOutcome` и peer наблюдают character-selection state, вход в name/birth flow и outbound packets. Этот seam принят для production-path проверки prerequisite integration и interaction routing по P1-AC01, P1-AC02, P1-AC28 и P2-AC05; проверки — `sv-core`, `sv-build` и `controller-review`.

- [x] RED production-endpoint matrix с encoded limits 12 ordinary / 2 IDDC / 1 PvP воспроизводит потерю ordinary slots для четырёх сочетаний feature flags: neither, IDDC-only, PvP-only и both; GREEN подтверждает ordinary create на последнем свободном ordinary slot и отклоняет следующий slot на точной ordinary boundary в каждом сочетании. [Contract: P1-AC01, P1-AC02, P2-AC04, P2-AC05; check: `sv-core`]
- [x] Effective total capacity равна ordinary capacity плюс только advertised dedicated capacity (12, 14, 13 и 15 для указанной matrix, пока не срабатывает общий cap); existing или absent dedicated slots не вычитаются из ordinary limit, а IDDC/PvP creation отдельно соблюдает собственную supported-mode boundary. [Contract: P1-AC01, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Keyboard и mouse ordinary create actions на разрешённой boundary достигают существующего production name/birth handoff без fixed-response или test-only availability implementation; запрещённая boundary оставляет character-selection state неизменным и не отправляет name, birth, partial либо completed-character packet. [Contract: P1-AC01, P1-AC02, P1-AC11, P1-AC28, P2-AC05; check: `sv-core`]
- [x] Regression coverage сохраняет normalized `creation_flags` 0/1 и fail-fast policy для значений больше 1, оба compile orders и существующие dedicated IDDC/PvP entry paths; final completed-character send остаётся вне SV-B-023. [Contract: P1-AC19, P1-AC20, P1-AC29, P1-AC31; checks: `sv-build`, `sv-core`]
- [x] Repair остаётся SV-local по возможности, не меняет legacy gameplay implementation и не расширяет canonical SV-B-023 scope; обнаруженные unrelated improvement opportunities записываются отдельно с требуемыми полями. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC06, P2-AC07, P2-AC08, P2-AC09; check: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred у named full-acceptance producer/verifier и не считается пройденным этим repair; evidence собирается только на exact accepted contract digest `15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6` и implementation HEAD.

## Comments

- 2026-10-09: Claimed on `codex/sandcastle-7b133980-wave-64-1` before
  implementation. Confirmed the ticket-approved public production seam
  `sv_endpoint_run(SvEndpointOptions)` after reading `CONTEXT.md`, the SV
  architecture and relevant interaction/session ADRs.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer передаёт encoded limits, advertised dedicated flags и roster через реальный
contact/login protocol; SDL keyboard и mouse events проходят native login
adapter и production interaction/router. `SvEndpointOutcome` наблюдает
character-selection phase, roster и name/birth entry, а peer запрещает любой
неожиданный outbound character request.

Причиной дефекта было повторное вычитание обоих encoded dedicated limits из уже
вычисленного effective total. Теперь ordinary boundary равна decoded ordinary
limit с ограничением `SV_LOGIN_MAX_CHARACTERS`; effective total по-прежнему
добавляет только advertised IDDC/PvP capacity и ограничивается общим client cap.
IDDC/PvP counts используются только для собственных exclusive boundaries.

Production matrix с limits `12/2/1` проверяет effective totals `12`, `14`, `13`
и `15`: при 11 ordinary characters keyboard либо mouse action открывает и
отменяет production name interaction, при 12 ordinary characters та же action
оставляет overview неизменным. Fixtures включают и занятые, и отсутствующие
dedicated slots. Отдельные IDDC `1/2` и PvP `0/1` controls подтверждают последнюю
доступную и полностью занятую supported-mode boundary. Mouse action имеет
видимую SV overview row и переводится в тот же semantic
`SV_LOGIN_COMMAND_CREATE_ORDINARY`, что и клавиша N.

TDD evidence:

```sh
PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin python3 -B tests/sv_ordinary_create_cancel_checks.py
```

- RED до capacity fix: exit 1 на `capacity-neither-available`; production name
  interaction не открывался при 11 из 12 ordinary slots.
- RED mouse slice: exit 1 на `capacity-iddc-available`; overview mouse action
  ещё не маршрутизировалась в semantic create command.
- GREEN на implementation commit `b29ea925d`: PASS всех 18 production endpoint
  scenarios, включая четыре flag combinations, обе ordinary boundaries,
  keyboard/mouse routing и отдельные IDDC/PvP boundaries.

Focused regression verification на `b29ea925d`: для cases
`stats-order-navigate`, `stats-point-navigate`, `mode-dedicated`,
`stats-point-dedicated`, `unsupported-creation-ordinary` и
`unsupported-creation-exclusive` выполнена команда
`SV_BIRTH_CASE=<case> PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin python3 -B tests/sv_name_editor_submit_checks.py`.
Все шесть cases прошли в обеих compile configurations. Дополнительно
`PATH=/opt/sv-venv/bin:/usr/local/bin:/usr/bin:/bin python3 -B tests/sv_login_checks.py`
— PASS. Normalized `creation_flags` 0/1, ordinary/dedicated paths и fail-fast
`> 1` сохранены. Unsupported ordinary control использует production mouse
create action; exact message и отсутствие name/birth/partial/completed send
остались green. Focused runners собирали production sources с
`-Wall -Wextra -Werror`, ASan и UBSan.

Implementation commit: `b29ea925d` (`fix(sv): preserve ordinary creation
capacity`). Изменения ограничены SV и production endpoint tests; legacy/shared
gameplay code не менялся. Новых unrelated improvement opportunities не
обнаружено. Full `sv-build`, unfiltered `sv-core`, controller review и full
acceptance не запускались: их выполняет orchestrator. `P1-birth-runtime-evidence`
/ P1-AC30 для digest
`15de367676b8c4c23c1ba787bb60b4e976455f27926f4b7688e17fc75204dbe6`
остаётся deferred и не объявляется пройденным.
