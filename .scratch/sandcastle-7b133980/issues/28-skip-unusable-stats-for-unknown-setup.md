# 28: Пропустить unusable stats для unknown creation setup

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Игрок с authoritative `creation_flags > 1` не попадает на пустую stats surface: после последнего доступного выбора production birth flow безопасно сохраняет baseline draft и переходит к mode. Backspace из mode возвращает к фактически предшествующему доступному owner в обеих compile-time class/race order configurations, а повторный проход снова пропускает недоступные body/stats без partial или completed send.

**Blocked by:** 08: Выбрать mode и завершить draft (completed); 09: Завершить или вернуться без send (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-18-2

Labels: bug, ready-for-agent

**Finding ID:** `D-498640166ca1cc34` (`birth.unknown-creation-flags-stats-deadlock`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative decoded `creation_flags` и server-provided choices; реальные keyboard/mouse SDL events проходят native birth adapter и production interaction/input router, а controlled peer наблюдает owner/draft transitions и отсутствие запрещённых outbound bytes. Seam принят P1-AC02, P1-AC08, P1-AC19 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red regression через production seam воспроизводит `creation_flags > 1` с нулём legal stats и доказывает, что прежний flow застревает на stats; green outcome никогда не назначает owner недоступной stats surface и достигает mode с неизменённым baseline draft. [Contract: P1-AC02, P1-AC08, P1-AC19, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [x] Обычный и alternate compile-time class/race orders проходят unknown setup до mode: недоступные body и stats пропускаются, а доступные sex/race/trait/class choices сохраняют server-provided legality, highlighted identity и committed selections. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC08, P1-AC10, P1-AC19; checks: `sv-build`, `sv-core`]
- [x] Backspace из mode возвращает к последнему доступному owner (trait в class-before-race order, class в race-before-class order); повторное продвижение снова достигает mode без пустой stats owner и без потери draft. [Contract: P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC19; checks: `sv-core`]
- [x] Confirm, random, Backspace, Q/Ctrl-Q, disconnect и stale generation на этом setup не отправляют partial/completed character result; accepted mode только завершает draft для будущего SV-B-024 send. [Contract: P1-AC02, P1-AC09, P1-AC12, P1-AC18, P1-AC22, P1-AC29; checks: `sv-core`]
- [x] Repair остаётся по возможности SV-local и не меняет legacy/shared gameplay semantics; discovered out-of-scope improvements записываются отдельно. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

Исправление выполнено локально в SV birth interaction. Новый переход
`enter_stats_or_mode` подготавливает production baseline stats draft, но при
нуле legal stats атомарно передаёт ownership mode. Для authoritative unknown
setup Backspace из mode минует недоступные stats/body и возвращает trait в
class-before-race либо class в race-before-class order. Повторное подтверждение
снова проходит тем же production путём до mode; обычные setup `0/1` сохраняют
прежний stats re-entry.

Согласованный public seam: `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative `creation_flags = 0x10` и server-provided choices; native
SDL keyboard events проходят production birth adapter и input router. Outcome
проверяет owners, legal counts, committed selections и baseline `-1` stats
draft, а peer требует EOF после единственного разрешённого name packet, поэтому
partial/completed character bytes провалили бы сценарий.

TDD, 2026-10-07:

- Red: `SV_BIRTH_CASE=unknown-setup-mode python3 tests/sv_name_editor_submit_checks.py`
  — **FAIL**, exit 1: assertion показал, что production owner остался на пустом
  `SV_BIRTH_STATS`.
- Green: тот же command после минимальной реализации — **PASS**, exit 0 в обеих
  compile-time class/race configurations.
- Green slices: focused cases `unknown-setup-mode-back`,
  `unknown-setup-mode-reenter`, `unknown-setup-ready` и
  `unknown-setup-disconnect` — **PASS** в обеих configurations. Они покрывают
  Backspace/re-entry, random mode acceptance, Q/Ctrl-Q, stale SDL generation и
  disconnect без запрещённых outbound bytes.
- Focused regression: `python3 tests/sv_name_editor_submit_checks.py` — **PASS**,
  exit 0; полная production endpoint birth matrix прошла с `-Wall -Wextra
  -Werror`, ASan/UBSan и обеими compile-time order configurations.

Legacy/shared gameplay code не менялся; новых out-of-scope improvements не
обнаружено. Полный build/test/review workflow и `sv-build` отдельно не
запускались — их выполняет orchestrator. `P1-birth-runtime-evidence` / P1-AC30
остаётся **deferred; not run** до full acceptance.
