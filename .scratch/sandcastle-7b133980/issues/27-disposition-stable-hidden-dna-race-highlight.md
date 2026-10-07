# 27: Получить human disposition для stable hidden DNA race highlight

**Type:** repair

**What to build:** После явного human disposition class-before-race flow либо получает разрешённое узкое исправление, при котором отклонённая DNA race не меняет прежнюю highlighted race, owner или draft, либо сохраняет обязательный дефект открытым без четвёртого автоматического repair. Совместимая DNA race и сохранённые random/DNA semantics продолжают работать как раньше.

**Blocked by:** 21: Сохранить hidden DNA class до подтверждения race (completed).

**Status:** unclaimed

**Assignee:** —

Labels: bug, ready-for-human

**Finding ID:** `D-6d2b2e9f103a1224` (`birth.hidden-derived-class-resolution`).

**Production seam for TDD if correction is authorized:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative decoded class/race setup и DNA identities; реальные `#`/`%` SDL events проходят native birth adapter и production interaction/input router, а outcome наблюдает owner, highlighted identity и committed draft. Seam принят P1-AC04, P1-AC06, P1-AC10, P1-AC19 и P1-AC28; проверки — `sv-core` и `controller-review`.

- [ ] Human disposition явно записан до любых production/test изменений; без явной авторизации ticket остаётся `ready-for-human`, а четвёртый автоматический repair не запускается. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P2-AC04; checks: `controller-review`]
- [ ] Если correction авторизован, DNA race candidate проверяется на exact hidden-derived-class compatibility до изменения highlighted race; mismatch оставляет прежние owner, highlight и draft, останавливает automatic continuation и не помечает DNA race применённой. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P1-AC26; checks: `sv-core`, `controller-review`]
- [ ] Авторизованный production-path regression сначала воспроизводит mutation, затем требует сохранения pre-attempt highlight для staged `#` и automatic `%`; compatible control case и обе compile-time class/race order configurations остаются green. [Contract: P1-AC04, P1-AC06, P1-AC19, P1-AC26, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Reject и compatible continuation не выполняют DNA persistence и не отправляют partial/completed character result; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Любое авторизованное исправление остаётся по возможности SV-local; найденные побочные улучшения только записываются отдельно и не расширяют repair. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
