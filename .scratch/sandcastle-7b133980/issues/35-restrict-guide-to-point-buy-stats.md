# 35: Ограничить Guide point-buy stats owner

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** `?` на stats owner открывает временный native Guide placeholder только для point-buy stats, где canonical baseline вызывает Guide. Во время ordered-stat selection тот же input безопасно потребляется как no-op без placeholder, state mutation или network operation; остальные уже проверенные canonical birth Guide entries не меняются.

**Blocked by:** 25: Ограничить Guide canonical birth owners (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-agent

**Finding ID:** `D-0710fc5e46fbdf22` (`birth.noncanonical-sex-guide-entry`), residual repair of the still-open ledger finding.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative ordered или point-buy creation setup; реальные SDL `?` и close events проходят native birth adapter и публичную production birth-interaction/input-router boundary. `SvEndpointOutcome` и controlled peer наблюдают owner, selection/draft, exact placeholder evidence и отсутствие Guide/network operations. Seam принят P1-AC23, P1-AC24, P1-AC25 и P1-AC28; проверка — `sv-core`.

- [ ] Red production-path regression достигает ordered stats через production birth flow и показывает текущий лишний placeholder; после repair `?` потребляется active birth input context как canonical no-op, не меняя owner, highlighted stat, partial/committed order, automatic state или illegal-input evidence. [Contract: P1-AC23, P1-AC25, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
- [ ] Для point-buy stats `?` по-прежнему открывает ровно временный placeholder с точным текстом `The guide is in development`; закрытие возвращает тому же stats caller с byte-for-byte unchanged selection/draft и без Guide state/subsystem или network operation. [Contract: P1-AC23, P1-AC24, P1-AC25; checks: `sv-core`]
- [ ] Раздельные ordered-stats и point-buy production scenarios проходят в default и `SV_CLASS_BEFORE_RACE=0` configurations; owner-mask/count evidence отличает no-op от canonical placeholder и сохраняет coverage остальных reachable canonical entries. [Contract: P1-AC19, P1-AC23, P1-AC24, P1-AC25, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`, `controller-review`]
- [ ] Stage G сохраняет ownership Guide capability; repair ограничен временной SV-local birth placeholder routing boundary, не расширяет Guide coverage и не меняет legacy/shared runtime. [Contract: P1-AC23, P2-AC01, P2-AC03, P2-AC04, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; полная native evidence matrix собирается только на exact accepted contract digest и HEAD во время full acceptance.
