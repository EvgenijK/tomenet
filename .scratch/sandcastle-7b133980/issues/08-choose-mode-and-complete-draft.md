# 08: Выбрать mode и завершить draft

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Игрок выбирает legal mode для server flags и текущего draft, включая dedicated IDDC/PvP paths; после mode обычный draft становится готовым для будущего SV-B-024 send, а PvP Maia при обязательном post-mode trait возвращается к trait owner и затем снова к mode без преждевременной отправки.

**Blocked by:** 07: Выбрать stats.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-8-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** mode и dedicated branches проходят через публичный `sv_endpoint_run(SvEndpointOptions)` с production server setup/flags и input router. Seam принят P1-AC09, P1-AC18, P1-AC20, P1-AC28 и P1-AC29; проверки используют `sv-core`.

- [x] Mode surface предлагает только legal server-provided/flag-authorized identities, сохраняет stable highlight и корректно обрабатывает illegal, keyboard, mouse, random и доступный DNA choice. [Contract: P1-AC09, P1-AC10, P1-AC11, P1-AC26; checks: `sv-core`]
- [x] Dedicated IDDC и PvP birth contexts сохраняют baseline mode semantics; недоступные dedicated choices нельзя подтвердить. [Contract: P1-AC09, P1-AC20; checks: `sv-core`]
- [x] Backspace из mode возвращает stats owner без отправки; PvP Maia, требующая trait после mode, получает trait owner и после legal trait снова возвращается к mode-ready outcome с сохранённым draft. [Contract: P1-AC18, P1-AC20; checks: `sv-core`]
- [x] Обычный legal mode confirm формирует только session-scoped ready draft: partial result и PKT_PLAY/completed-character send отсутствуют, поскольку send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC20, P1-AC29; checks: `sv-core`, `controller-review`]
- [x] Q/Ctrl-Q и все mode branches используют production interaction/input router; implementation и tests остаются SV-local. [Contract: P1-AC11, P1-AC12, P1-AC28, P2-AC01, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]

## Answer

Mode завершён внутри production SV birth interaction. Legal identities строятся
из authoritative server flags, ordinary/dedicated birth context и наличия уже
существующего персонажа. Ordinary surface сохраняет baseline Normal, No Ghost,
Soloist, Everlasting, Hellish и PvP gates; RPG исключает Normal, Everlasting и
PvP. Dedicated surface предлагает только разрешённые IDDC, IDDC Soloist,
Hellish IDDC и dedicated PvP identities. Keyboard, mouse, random и legal DNA
используют общий `sv_native_birth_event()` → `sv_birth_command()` path; illegal
choice не меняет stable highlight.

Legal confirm создаёт `SV_BIRTH_READY` только в session-scoped draft и не
сериализует partial result или completed character. Backspace из mode возвращает
`SV_BIRTH_STATS`. Для server `PVP_MAIA` draft с Maia и ordinary/dedicated PvP
после mode получает `SV_BIRTH_TRAIT`; только Enlightened/Corrupted post-mode
traits legal, а confirm возвращает сохранённые body/stats/mode в ready outcome.
Q/Ctrl-Q завершают interaction через тот же native router. Controlled peer после
единственного name request во всех сценариях наблюдает только закрытие socket,
поэтому PKT_PLAY и другие post-name bytes отсутствуют.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → decoded
`SvContactSetup`/server flags → `SV_PREGAME_BIRTH` → `SvBirth` →
`sv_native_birth_event()`. Default и `SV_CLASS_BEFORE_RACE=0` binaries используют
один production interaction/input path; отдельной test-only mode реализации нет.

TDD (`python3 -B tests/sv_name_editor_submit_checks.py`):

- RED ordinary slice: exit 1 на compile — public endpoint outcome не содержал
  mode list/highlight/ready state. GREEN: exit 0 для legal list, stable illegal,
  keyboard/mouse confirmation и ready draft.
- RED DNA slice: exit 1 на compile — `SvEndpointOptions` не принимал mode DNA.
  GREEN: `SV_BIRTH_CASE=mode-dna ...` и `mode-dna-missing` — exit 0.
- RED dedicated slice: exit 1 на compile — production endpoint не передавал
  dedicated birth context. GREEN: dedicated/unauthorized focused cases — exit 0.
- RED Backspace slice: exit 1 на runtime assertion — mode оставался owner.
  GREEN: `SV_BIRTH_CASE=mode-back ...` — exit 0; random и Ctrl-Q focused cases
  также exit 0.
- RED PvP Maia slice: exit 1 на runtime assertion — PvP преждевременно становился
  ready без post-mode trait owner. GREEN: `SV_BIRTH_CASE=mode-pvp-maia ...` —
  exit 0 в обеих compile configurations с сохранёнными stats/mode и без send.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS (default и alternate
  compile configurations, весь birth regression set).
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — PASS.
- `python3 -B tests/sv_ordinary_create_cancel_checks.py` — PASS.
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Legacy и shared gameplay code не
менялись; возможностей улучшения вне текущего scope не обнаружено.
