# 35: Ограничить Guide point-buy stats owner

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** `?` на stats owner открывает временный native Guide placeholder только для point-buy stats, где canonical baseline вызывает Guide. Во время ordered-stat selection тот же input безопасно потребляется как no-op без placeholder, state mutation или network operation; остальные уже проверенные canonical birth Guide entries не меняются.

**Blocked by:** 25: Ограничить Guide canonical birth owners (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-23-2

Labels: bug, ready-for-agent

**Finding ID:** `D-0710fc5e46fbdf22` (`birth.noncanonical-sex-guide-entry`), residual repair of the still-open ledger finding.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative ordered или point-buy creation setup; реальные SDL `?` и close events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, selection/draft, exact placeholder evidence и отсутствие Guide/network operations. Seam принят P1-AC23, P1-AC24, P1-AC25 и P1-AC28; проверка — `sv-core`.

- [x] Red production-path regression достигает ordered stats через production birth flow и показывает текущий лишний placeholder; после repair `?` потребляется active birth input context как canonical no-op, не меняя owner, highlighted stat, partial/committed order, automatic state или illegal-input evidence. [Contract: P1-AC23, P1-AC25, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
- [x] Для point-buy stats `?` по-прежнему открывает ровно временный placeholder с точным текстом `The guide is in development`; закрытие возвращает тому же stats caller с byte-for-byte unchanged selection/draft и без Guide state/subsystem или network operation. [Contract: P1-AC23, P1-AC24, P1-AC25; checks: `sv-core`]
- [x] Раздельные ordered-stats и point-buy production scenarios проходят в default и `SV_CLASS_BEFORE_RACE=0` configurations; owner-mask/count evidence отличает no-op от canonical placeholder и сохраняет coverage остальных reachable canonical entries. [Contract: P1-AC19, P1-AC23, P1-AC24, P1-AC25, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`, `controller-review`]
- [x] Stage G сохраняет ownership Guide capability; repair ограничен временной SV-local birth placeholder routing boundary, не расширяет Guide coverage и не меняет legacy/shared runtime. [Contract: P1-AC23, P2-AC01, P2-AC03, P2-AC04, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled peer
передаёт authoritative ordered или point-buy creation setup, реальные SDL key и
close events проходят через production native birth adapter и публичную
birth-interaction/input-router boundary. `SvEndpointOutcome` наблюдает owner,
highlight, полный stats draft, committed state и placeholder evidence; peer
требует EOF после единственного допустимого name packet, поэтому любое Guide- или
birth-связанное network output провалило бы сценарий. Test-only реализации
поведения нет.

SV-local native birth adapter теперь возвращает consumed
`SV_BIRTH_INPUT_HANDLED` для `?` у ordered stats, как уже делал у sex. Point-buy
stats по-прежнему возвращает `SV_BIRTH_INPUT_PLACEHOLDER`. Остальные canonical
owners class/race/trait/body/mode не менялись; их aggregate evidence теперь даёт
пять open/close и owner mask `0xbc`. Point-buy stats отдельно даёт ровно один
open/close, owner mask `1 << SV_BIRTH_STATS`, точный текст
`The guide is in development` и byte-for-byte preserved `SvBirth` draft при
закрытии. Stage G сохраняет Guide ownership; legacy и shared runtime не менялись.

TDD red → green через согласованный production seam:

- `SV_BIRTH_CASE=guide-ordered-stats-no-op python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1: production outcome нарушил assertion
  `birth_placeholder_open_count == 0`, потому что ordered stats открыл лишний
  placeholder.
- После минимальной adapter repair та же команда — **GREEN**, exit 0 в default и
  `SV_CLASS_BEFORE_RACE=0`. Partial order `{1, -1, -1, -1, -1, -1}`, committed
  state, legal list, highlighted stat, owner, confirmation, automatic и
  illegal-input evidence остались неизменными; placeholder count/mask/text равны
  нулю.

Focused production verification:

- `SV_BIRTH_CASE=stats-point-guide python3 -B tests/sv_name_editor_submit_checks.py`
  — **PASS**, exit 0 для обеих compile configurations; изменённый point-buy draft
  `{10, 12, 10, 10, 10, 10}` и remaining budget 28 сохраняются через точный
  placeholder round trip.
- `SV_BIRTH_CASE=guide-all-owners python3 -B tests/sv_name_editor_submit_checks.py`
  — **PASS**, exit 0 для обеих compile configurations; пять остальных canonical
  owners сохраняют placeholder coverage, а ordered stats исключён из mask/count.
- `SV_BIRTH_CASE=guide-sex-no-op python3 -B tests/sv_name_editor_submit_checks.py`
  — **PASS**, exit 0 для обеих compile configurations; ранее принятый sex no-op
  не изменён.
- `git diff --check` — **PASS**, exit 0.

Полный build/test/review workflow не запускался: его выполняет orchestrator.
Возможностей улучшения вне текущего scope не обнаружено. Проверка
`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** до full
acceptance на exact accepted contract digest и HEAD.
