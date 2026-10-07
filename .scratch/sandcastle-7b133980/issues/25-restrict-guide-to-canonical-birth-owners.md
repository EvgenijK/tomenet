# 25: Ограничить Guide canonical birth owners

**Type:** implementation

**What to build:** `?` открывает временный native Guide placeholder только у birth owners, чьи canonical baseline handlers вызывают Guide; на sex owner этот input безопасно потребляется как no-op без placeholder, state mutation или network operation.

**Blocked by:** 11: Вернуться из birth Guide placeholder (completed); 14: Изолировать production birth input context (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-0710fc5e46fbdf22` (`birth.noncanonical-sex-guide-entry`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` принимает реальные SDL `?` and close events through production birth input routing and native temporary surface; outcome/socket evidence observes owner, selection/draft, placeholder counts/text and absence of network work. Seam принят P1-AC23, P1-AC24, P1-AC25 and P1-AC28; проверка — `sv-core`.

- [ ] `?` на sex owner потребляется активным birth input context как canonical no-op: placeholder не открывается, owner/highlight/draft не меняются и input не попадает в login/MOTD routing. [Contract: P1-AC23, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Остальные canonical reachable Guide entries по-прежнему показывают ровно `The guide is in development`, а закрытие возвращает тот же caller с неизменными selection и draft. [Contract: P1-AC23, P1-AC24, P1-AC25; checks: `sv-core`]
- [ ] Owner-mask/count production regression исключает sex, покрывает canonical owners в обеих compile configurations и подтверждает отсутствие Guide state, Guide subsystem call и network/output operation для open, close and sex no-op. [Contract: P1-AC19, P1-AC23, P1-AC24, P1-AC25, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`, `controller-review`]
- [ ] Stage G сохраняет ownership Guide capability; ticket меняет только временную birth placeholder routing boundary и не расширяет Guide coverage. [Contract: P1-AC23, P2-AC01, P2-AC03, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
