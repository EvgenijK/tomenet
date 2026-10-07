# 27: Получить human disposition для stable hidden DNA race highlight

**Type:** repair

**What to build:** После явного human disposition class-before-race flow либо получает разрешённое узкое исправление, при котором отклонённая DNA race не меняет прежнюю highlighted race, owner или draft, либо сохраняет обязательный дефект открытым без четвёртого автоматического repair. Совместимая DNA race и сохранённые random/DNA semantics продолжают работать как раньше.

**Blocked by:** 21: Сохранить hidden DNA class до подтверждения race (completed).

**Status:** resolved

**Assignee:** Codex (`codex/sandcastle-7b133980-wave-18-1`)

Labels: bug, ready-for-agent

**Finding ID:** `D-6d2b2e9f103a1224` (`birth.hidden-derived-class-resolution`).

**Production seam for TDD if correction is authorized:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative decoded class/race setup и DNA identities; реальные `#`/`%` SDL events проходят native birth adapter и production interaction/input router, а outcome наблюдает owner, highlighted identity и committed draft. Seam принят P1-AC04, P1-AC06, P1-AC10, P1-AC19 и P1-AC28; проверки — `sv-core` и `controller-review`.

- [x] Human disposition явно записан до любых production/test изменений; без явной авторизации ticket остаётся `ready-for-human`, а четвёртый автоматический repair не запускается. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P2-AC04; checks: `controller-review`]
- [x] Если correction авторизован, DNA race candidate проверяется на exact hidden-derived-class compatibility до изменения highlighted race; mismatch оставляет прежние owner, highlight и draft, останавливает automatic continuation и не помечает DNA race применённой. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P1-AC26; checks: `sv-core`, `controller-review`]
- [x] Авторизованный production-path regression сначала воспроизводит mutation, затем требует сохранения pre-attempt highlight для staged `#` и automatic `%`; compatible control case и обе compile-time class/race order configurations остаются green. [Contract: P1-AC04, P1-AC06, P1-AC19, P1-AC26, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Reject и compatible continuation не выполняют DNA persistence и не отправляют partial/completed character result; final send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Любое авторизованное исправление остаётся по возможности SV-local; найденные побочные улучшения только записываются отдельно и не расширяют repair. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Comments

- 2026-10-07 — Human disposition: пользователь явно авторизовал correction командой
  `Implement exactly .../27-disposition-stable-hidden-dna-race-highlight.md`.
  Разрешено узкое SV-local исправление из этого ticket с TDD на принятом публичном
  seam `sv_endpoint_run(SvEndpointOptions)`; четвёртый автоматический repair не
  выполняется. Ticket claimed до любых production/test изменений.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` получает
authoritative decoded class/race setup и DNA identities. Реальные SDL `#`/`%`
events проходят `sv_native_birth_event()` и production `sv_birth_command()`;
outcome наблюдает owner, highlighted identity и committed draft. Managed peer
после name packet требует EOF, поэтому partial/completed character send также
отклоняется тестом.

В class-before-race `SV_BIRTH_DNA` теперь проверяет DNA race candidate на exact
совпадение `resolved_class(chosen_visible_class, dna_race)` с сохранённой
`requested_class` до присваивания `highlighted_race`. Mismatch оставляет прежние
owner, race highlight и draft, выставляет illegal outcome, останавливает automatic
continuation и не выставляет `race_dna_used`. Compatible DNA race проходит прежний
production path без изменения random/DNA semantics. Изменены только SV birth
interaction и его production-path regression; DNA persistence и final send не
добавлялись.

TDD red → green:

- `SV_BIRTH_CASE=hidden-derived-dna-race-mismatch python3 -B tests/sv_name_editor_submit_checks.py`
  — RED, exit 1: staged `#` сохранил owner/draft, но mutation оставила highlight
  race 1 вместо pre-attempt race 0; после pre-mutation exact check — GREEN, exit 0.
- `SV_BIRTH_CASE=auto-dna-hidden-race-reject-stable python3 -B tests/sv_name_editor_submit_checks.py`
  — RED, exit 1: automatic `%` оставил отклонённую race 1 highlighted; после
  того же production correction — GREEN, exit 0.

Focused verification, 2026-10-07:

- `SV_BIRTH_CASE=hidden-derived-dna-race-compatible python3 -B tests/sv_name_editor_submit_checks.py`
  — PASS, exit 0: compatible DNA race по-прежнему подтверждается.
- `SV_BIRTH_CASE=auto-dna-hidden-race-mismatch python3 -B tests/sv_name_editor_submit_checks.py`
  — PASS, exit 0: после reject сохраняется прежнее manual continuation.
- `SV_BIRTH_CASE=hidden-derived-dna python3 -B tests/sv_name_editor_submit_checks.py`
  — PASS, exit 0 в обеих compile-time class/race order configurations.
- `SV_BIRTH_CASE=auto-dna-full python3 -B tests/sv_name_editor_submit_checks.py`
  — PASS, exit 0 в обеих compile-time class/race order configurations.

Полный orchestrator build/test/review workflow и `sv-build` отдельно не
запускались. `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не
объявляется выполненным. Новых improvement opportunities не обнаружено.
