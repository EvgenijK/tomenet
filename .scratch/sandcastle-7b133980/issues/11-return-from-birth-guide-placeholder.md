# 11: Вернуться из birth Guide placeholder

**Type:** implementation

**What to build:** `?` на каждом birth owner открывает native временную поверхность с точным текстом `The guide is in development`; закрытие возвращает того же caller с неизменными selection и draft, не создавая Guide state и не выполняя network operation.

**Blocked by:** 08: Выбрать mode и завершить draft.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** все reachable Guide entries вызываются через публичный `sv_endpoint_run(SvEndpointOptions)` и production birth input routing. Seam принят P1-AC23, P1-AC24, P1-AC25 и P1-AC28; проверки используют `sv-core`.

- [ ] `?` на каждом достижимом sex/race/trait/class/body/stats/mode owner показывает ровно `The guide is in development` на native temporary surface. [Contract: P1-AC23, P1-AC24; checks: `sv-core`, `controller-review`]
- [ ] Закрытие placeholder возвращает тот же owner и сохраняет highlighted identity, selection и полный draft побайтно/по identity. [Contract: P1-AC25; checks: `sv-core`]
- [ ] Открытие и закрытие placeholder не создаёт Guide state, не вызывает Guide subsystem и не производит network/output operation; Stage G ownership не переносится в B. [Contract: P1-AC23, P1-AC25; checks: `sv-core`, `controller-review`]
- [ ] Placeholder доступен через production input router и native scene, а не через fixed-response или test-only surface. [Contract: P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
