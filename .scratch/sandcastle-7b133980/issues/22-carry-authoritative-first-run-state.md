# 22: Передать authoritative first-run state в birth

**Type:** implementation

**What to build:** Production character-management/session owner передаёт first-run eligibility независимо от roster count, поэтому returning player с пустым roster получает legal Fruit Bat, Hellish и PvP choices, а реальный first startup остаётся ограничен даже при непустом server overview.

**Blocked by:** 18: Соблюсти first-run и Archer body legality (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-15-2

Labels: bug, ready-for-agent

**Finding ID:** `D-2a20d07678497370` (`birth.first-run-body-legality`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` вызывается тем же production startup/character-management path, который владеет first-run state; server-confirmed name creation переносит этот state через pregame в production birth interaction, а native input и outcome наблюдают legal body/mode identities и draft. Seam принят P1-AC01, P1-AC02, P1-AC07, P1-AC09 и P1-AC28; проверки — `sv-core` и `controller-review`. Отдельный test-only first-run option не является допустимым seam.

- [x] First-run eligibility имеет authoritative production owner и передаётся через character-management/session handoff отдельно от количества server-provided characters; birth больше не выводит её из empty/non-empty roster. [Contract: P1-AC01, P1-AC02, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Returning-empty-roster production flow разрешает Fruit Bat, Hellish и PvP согласно baseline first-run gate, сохраняя server/RPG compatibility и stable highlighted identity. [Contract: P1-AC02, P1-AC07, P1-AC09, P1-AC10, P2-AC04; checks: `sv-core`]
- [x] First-startup-with-existing-roster production flow запрещает те же first-run-ineligible body/mode confirmations независимо от direct, mouse, random или DNA input; illegal choice не меняет owner, highlight или committed draft. [Contract: P1-AC02, P1-AC07, P1-AC09, P1-AC10, P1-AC11, P1-AC26; checks: `sv-core`]
- [x] Production-path cases покрывают обе независимые комбинации first-run/roster в обеих compile configurations и сохраняют уже проверенные Archer random и ordinary legal-choice semantics без test-only behavior implementation. [Contract: P1-AC07, P1-AC09, P1-AC19, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [x] Передача eligibility и все legal/illegal choices не отправляют partial/completed character result и не принимают DNA persistence или final-send ownership. [Contract: P1-AC02, P1-AC26, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Изменения ограничены необходимым production handoff и SV birth behavior; legacy/shared refactor не выполняется только ради deduplication. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

Согласованный public production seam: `sv_endpoint_run(SvEndpointOptions)`.
Endpoint определяет authoritative first-run eligibility по production profile
state: отсутствие собственного `U/sv/tomenet.cfg` означает first launch, наличие
сохранённого CFG — returning launch. Это соответствует settings policy и не
выполняет implicit write. State передаётся в session-owned `SvPregame`, затем
через server-confirmed name handoff в `SvBirth` отдельно от authoritative
`character_count`; test-only first-run option не добавлен.

Body и ordinary mode legality теперь используют только переданный `first_run`.
Returning-empty roster получает Fruit Bat, Hellish и PvP; RPG сохраняет запрет
PvP/Normal/Everlasting, но разрешает Hellish. First-start с непустым overview
оставляет Fruit Bat, Hellish и PvP вне legal identities. Direct, out-of-list mouse
и DNA attempts сохраняют owner, highlight и draft; random выбирает только из
legal list. Existing Archer random и ordinary legal paths остались зелёными.

TDD red → green, 2026-10-07:

- Body handoff slice, seam `sv_endpoint_run`: `SV_BIRTH_CASE=returning-empty-body-manual
  python3 -B tests/sv_name_editor_submit_checks.py` — **RED**, assertion
  `birth_legal_body_count == 2` получил first-run-only list из одного Normal.
  После минимального profile → pregame → birth handoff тот же command — **GREEN**,
  exit 0 для `SV_CLASS_BEFORE_RACE=1` и `=0`.
- Mode gate slice, тот же seam: `SV_BIRTH_CASE=returning-empty-mode-pvp python3
  -B tests/sv_name_editor_submit_checks.py` — **RED**, assertion
  `birth_legal_mode_count == 6` получил roster-derived список из четырёх mode.
  После перевода Hellish/PvP gate на authoritative `first_run` тот же command —
  **GREEN**, exit 0 для обеих compile configurations.

Production-path coverage в `tests/sv/name-editor-submit.c` создаёт настоящий
saved/missing SV CFG fixture и проводит server-confirmed name, native input и
birth interaction через endpoint. Покрыты returning-empty body/PvP/RPG и
first-start-existing direct/mouse/DNA rejects и random для body/mode в обоих
compile orders. Controlled peer принимает ровно name request и затем требует EOF,
поэтому partial/completed-character send провалил бы каждый scenario.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0; полный
  focused production birth/name suite в обеих compile configurations с
  `-Wall -Wextra -Werror` и ASan/UBSan.
- После удаления больше не нужного roster argument:
  `SV_BIRTH_CASE=first-start-existing-mode-reject python3 -B
  tests/sv_name_editor_submit_checks.py` — **PASS**, exit 0, обе configurations.
- `python3 -B tests/sv_login_checks.py` — **PASS**, exit 0.
- `git diff --check` — **PASS**, exit 0.

Полный build/test/review workflow не запускался: assembled gates выполняет
orchestrator. Наблюдавшиеся diagnostics о fallback font и `Leaked thread` уже
известны; новых improvement opportunities не найдено. Legacy/shared files,
DNA persistence и final completed-character send не изменялись.

`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** по исходной
acceptance-phase причине и не объявляется выполненным этим implementation ticket.
