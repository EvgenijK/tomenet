# 25: Ограничить Guide canonical birth owners

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** `?` открывает временный native Guide placeholder только у birth owners, чьи canonical baseline handlers вызывают Guide; на sex owner этот input безопасно потребляется как no-op без placeholder, state mutation или network operation.

**Blocked by:** 11: Вернуться из birth Guide placeholder (completed); 14: Изолировать production birth input context (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-16-2

Labels: bug, ready-for-agent

**Finding ID:** `D-0710fc5e46fbdf22` (`birth.noncanonical-sex-guide-entry`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` принимает реальные SDL `?` and close events through production birth input routing and native temporary surface; outcome/socket evidence observes owner, selection/draft, placeholder counts/text and absence of network work. Seam принят P1-AC23, P1-AC24, P1-AC25 and P1-AC28; проверка — `sv-core`.

- [x] `?` на sex owner потребляется активным birth input context как canonical no-op: placeholder не открывается, owner/highlight/draft не меняются и input не попадает в login/MOTD routing. [Contract: P1-AC23, P1-AC28, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Остальные canonical reachable Guide entries по-прежнему показывают ровно `The guide is in development`, а закрытие возвращает тот же caller с неизменными selection и draft. [Contract: P1-AC23, P1-AC24, P1-AC25; checks: `sv-core`]
- [x] Owner-mask/count production regression исключает sex, покрывает canonical owners в обеих compile configurations и подтверждает отсутствие Guide state, Guide subsystem call и network/output operation для open, close and sex no-op. [Contract: P1-AC19, P1-AC23, P1-AC24, P1-AC25, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`, `controller-review`]
- [x] Stage G сохраняет ownership Guide capability; ticket меняет только временную birth placeholder routing boundary и не расширяет Guide coverage. [Contract: P1-AC23, P2-AC01, P2-AC03, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Managed peer
передаёт authoritative setup, реальные SDL key events проходят через production
birth input routing и `sv_native_birth_event()`, а `SvEndpointOutcome` и
контролируемый socket наблюдают owner, highlight/draft, placeholder evidence и
отсутствие outbound work. Отдельной test-only реализации поведения нет.

Граница временного placeholder теперь совпадает с canonical baseline handlers:
class, race, trait, body, stats и mode возвращают
`SV_BIRTH_INPUT_PLACEHOLDER`, а `?` у sex явно возвращает
`SV_BIRTH_INPUT_HANDLED` до generic illegal-input ветки. Поэтому sex input
потребляется активным birth context без placeholder, Guide state/subsystem call,
изменения draft/counters и fallthrough в login/MOTD routing. Изменён только
SV-local native birth adapter; legacy и shared files не менялись, ownership
полноценной Guide capability остаётся у Stage G.

TDD red → green через согласованный production seam:

- `SV_BIRTH_CASE=guide-sex-no-op python3 -B tests/sv_name_editor_submit_checks.py`
  — **RED**, exit 1: production outcome имел
  `birth_placeholder_open_count == 1`, поэтому assertion ожидаемого нуля упал.
- После исключения sex из placeholder range промежуточный прогон всё ещё был
  **RED**, exit 1: generic unknown-input path установил
  `birth_illegal_ignored`; это выявило запрещённую state mutation.
- После явного consumed no-op та же команда — **GREEN**, exit 0 для default и
  `SV_CLASS_BEFORE_RACE=0`. Outcome сохраняет sex owner, male highlight и
  unconfirmed/empty draft, оставляет illegal flag сброшенным и сообщает нулевые
  placeholder count/mask/text. Peer после единственного допустимого name packet
  получает EOF без дополнительных bytes.

Focused verification:

- `SV_BIRTH_CASE=guide-all-owners python3 -B tests/sv_name_editor_submit_checks.py`
  — **PASS**, exit 0 для обеих compile configurations. Шесть canonical owners
  дают ровно шесть open/close, mask `0xfc`, точный текст
  `The guide is in development`, byte-for-byte preserved draft и неизменные
  highlighted/selected values; controlled socket не наблюдает outbound bytes.
- `git diff --check` — **PASS**, exit 0.

Полный build/test/review workflow не запускался: его выполняет orchestrator.
Возможностей улучшения вне текущего scope не обнаружено. Проверка
`P1-birth-runtime-evidence` / P1-AC30 остаётся **deferred; not run** до full
acceptance для exact contract digest и HEAD.
