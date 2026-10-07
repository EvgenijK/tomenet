# 21: Сохранить hidden DNA class до подтверждения race

**Type:** implementation

**What to build:** В class-before-race flow выбранная DNA hidden class остаётся обязательной identity до подтверждения race: совместимая race завершает исходный hidden choice, а race, разрешающая другую derived class от того же visible base, останавливает staged или automatic DNA без подмены requested class и оставляет согласованный ручной выбор.

**Blocked by:** 16: Разрешить hidden derived class при выборе race (completed); 19: Продолжить automatic DNA choices через birth flow (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-6d2b2e9f103a1224` (`birth.hidden-derived-class-resolution`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative decoded class/race setup и DNA identities; реальные `#`/`%` SDL events проходят native birth adapter и production interaction/input router, а outcome наблюдает owner, visible highlight и committed draft. Seam принят P1-AC04, P1-AC06, P1-AC10, P1-AC19 и P1-AC28; проверка — `sv-core`.

- [ ] При DNA hidden-class choice до race production interaction хранит requested server identity отдельно от visible base highlight и при подтверждении требует, чтобы `resolved_class(visible, selected_race)` совпала именно с requested identity. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Если последующая legal DNA race разрешает другую hidden class от того же visible base, staged `#` и automatic `%` не подменяют class: automatic continuation выключается, owner остаётся на race, а текущие legal highlight и draft сохраняются согласованными для ручного продолжения. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P1-AC26; checks: `sv-core`]
- [ ] Production-path regression использует setup, где requested hidden class legal для race A, а DNA выбирает race B с другой derived identity; он проверяет reject mismatch и compatible control case в class-before-race build, а alternate build остаётся green. [Contract: P1-AC04, P1-AC06, P1-AC19, P1-AC26, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Reject и successful continuation не выполняют DNA persistence и не отправляют partial/completed character result; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Исправление и regression остаются по возможности SV-local и не меняют legacy/shared gameplay rules. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
