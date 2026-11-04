# 16: Разрешить hidden derived class при выборе race

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Когда legal server setup связывает выбранный visible base class и race с hidden derived class, подтверждение race сохраняет в draft derived server identity, при этом backtracking продолжает показывать и подсвечивать выбранный игроком visible base class.

**Blocked by:** 06: Сохранить alternate class/race compile order (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-12-3

Labels: bug, ready-for-agent

**Finding ID:** `D-6d2b2e9f103a1224` (`birth.hidden-derived-class-resolution`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative decoded class/race setup, а keyboard/mouse/DNA input проходит production birth interaction и native input router; outcome наблюдает visible class highlight и committed draft identities. Seam принят P1-AC04, P1-AC06 и P1-AC28; проверка — `sv-core`.

- [x] При race confirmation production interaction находит только server-provided hidden class, чей `base_class` и compatibility соответствуют выбранным visible class/race; draft получает derived identity в baseline boundary, а неподходящая hidden class не выбирается. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Backspace и повторный вход сохраняют visible base-class identity как пользовательский highlight, не раскрывают hidden class как обычный selectable item и повторно вычисляют derived draft только после legal race confirmation. [Contract: P1-AC06, P1-AC13, P1-AC15, P1-AC19; checks: `sv-core`]
- [x] Production-path coverage проверяет representative visible-base → hidden-derived outcome и DNA mapping, включая reject для missing/incompatible/stale derived relationships и обе compile configurations. [Contract: P1-AC04, P1-AC06, P1-AC10, P1-AC19, P1-AC26, P1-AC28; checks: `sv-build`, `sv-core`]
- [x] Resolve, backtrack и DNA paths не отправляют partial/completed character result и не добавляют DNA persistence или final-send ownership. [Contract: P1-AC02, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Реализация и tests остаются SV-local и вызывают production interaction вместо отдельной test-only compatibility model. [Contract: P2-AC01, P2-AC04, P2-AC05; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

SV birth draft теперь раздельно хранит выбранную игроком visible class identity и
разрешённую server class identity. Hidden class остаётся исключённой из legal
class list. После legal пары visible class/race production interaction выбирает
первую authoritative hidden identity только при одновременном совпадении
`base_class` и race compatibility; иначе draft сохраняет visible base identity.
Обе compile orders используют тот же resolver: default order применяет его при
race confirmation, alternate order — при class confirmation после уже
подтверждённой race.

При возврате в race derived draft сбрасывается к visible base, а class highlight
не меняется. Повторное legal race confirmation снова вычисляет derived identity.
Hidden DNA identity маппится на visible base только если текущий decoded setup
содержит непросроченную совместимую relationship; missing, incompatible и stale
relationships отклоняются. Persistence DNA и final character send не добавлялись.
Каждый managed production peer проверяет закрытие socket без дополнительных bytes,
поэтому resolve, backtrack, mouse и DNA scenarios не отправляют partial/completed
character result.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → authoritative
decoded `SvContactSetup` → `SV_PREGAME_BIRTH` → `SvBirth` → native birth input
router. Outcome наблюдает committed `birth_class`, visible
`birth_highlighted_class`, legal list и owner; отдельной test-only compatibility
model нет.

TDD red → green:

- `SV_BIRTH_CASE=hidden-derived python3 -B tests/sv_name_editor_submit_checks.py`:
  RED exit 1 на assertion derived draft identity; GREEN exit 0 после разделения
  visible/draft identity и race-time resolve.
- `SV_BIRTH_CASE=hidden-derived-back python3 -B tests/sv_name_editor_submit_checks.py`:
  RED exit 1, потому что backtracking оставлял derived draft; GREEN exit 0 после
  восстановления visible base при возврате в race.
- `SV_BIRTH_CASE=hidden-derived-dna python3 -B tests/sv_name_editor_submit_checks.py`:
  RED exit 1 на неразрешённой hidden DNA identity; GREEN exit 0 в default и
  alternate binaries после authoritative DNA mapping.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS, включая keyboard,
  mouse, repeated confirmation, mismatched base, DNA missing/incompatible/stale,
  отсутствие hidden item в legal list и обе compile configurations.
- `SV_BIRTH_CASE=hidden-derived-base-mismatch python3 -B tests/sv_name_editor_submit_checks.py`
  — PASS в обеих compile configurations.
- `make -s -C src -f makefile.sv tomenet-sv` — PASS.
- `make -s -C src -f makefile.sv SV_CLASS_BEFORE_RACE=0 tomenet-sv` — PASS.
- `git diff --check` — PASS.

Полный orchestrator build/test/review workflow и deferred
`P1-birth-runtime-evidence` не запускались. Изменены только SV birth и его
production-path tests; legacy/shared gameplay code не менялся. Наблюдавшиеся
diagnostics SDL leaked-thread и common `atol` уже записаны в
`docs/sv-improvements.md`; новых improvement opportunities не обнаружено.
