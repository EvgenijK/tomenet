# 11: Вернуться из birth Guide placeholder

**Type:** implementation

**What to build:** `?` на каждом birth owner открывает native временную поверхность с точным текстом `The guide is in development`; закрытие возвращает того же caller с неизменными selection и draft, не создавая Guide state и не выполняя network operation.

**Blocked by:** 08: Выбрать mode и завершить draft.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-9-3

Labels: enhancement, ready-for-agent

**Production seam for TDD:** все reachable Guide entries вызываются через публичный `sv_endpoint_run(SvEndpointOptions)` и production birth input routing. Seam принят P1-AC23, P1-AC24, P1-AC25 и P1-AC28; проверки используют `sv-core`.

- [x] `?` на каждом достижимом sex/race/trait/class/body/stats/mode owner показывает ровно `The guide is in development` на native temporary surface. [Contract: P1-AC23, P1-AC24; checks: `sv-core`, `controller-review`]
- [x] Закрытие placeholder возвращает тот же owner и сохраняет highlighted identity, selection и полный draft побайтно/по identity. [Contract: P1-AC25; checks: `sv-core`]
- [x] Открытие и закрытие placeholder не создаёт Guide state, не вызывает Guide subsystem и не производит network/output operation; Stage G ownership не переносится в B. [Contract: P1-AC23, P1-AC25; checks: `sv-core`, `controller-review`]
- [x] Placeholder доступен через production input router и native scene, а не через fixed-response или test-only surface. [Contract: P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]

## Answer

Birth Guide entry реализован как SV-local temporary presentation surface. `?`
проходит через production `sv_native_birth_event()` для каждого доступного owner
sex/class/race/trait/body/stats/mode и возвращает только результат открытия
placeholder. Endpoint loop показывает отдельный native frame с точным текстом
`The guide is in development`; следующий keyboard или mouse input закрывает его и
возвращает существующую birth surface.

`SvBirth` остаётся единственным birth draft: placeholder не добавляет Guide state,
не вызывает Guide subsystem и не создаёт protocol command. Production endpoint
делает byte-for-byte snapshot полного `SvBirth` при открытии и подтверждает его
неизменность при закрытии. Сценарный peer после name request наблюдает закрытие
socket без дополнительных bytes, поэтому открытие/закрытие не производит network
output. Stage G ownership не переносился; legacy/shared gameplay code не менялся.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → production
birth interaction → `sv_native_birth_event()` → native endpoint scene. Default и
`SV_CLASS_BEFORE_RACE=0` binaries используют один production path; отдельной
test-only поверхности или fixed-response wizard нет.

TDD (`SV_BIRTH_CASE=guide-all-owners python3 -B tests/sv_name_editor_submit_checks.py`):

- RED: exit 1 на compile — public endpoint outcome не содержал evidence открытия,
  закрытия, caller-owner и текста temporary surface.
- GREEN: exit 0 в default и alternate compile configurations. Один production
  scenario открыл и закрыл surface на всех семи owners, проверил точный текст,
  owner mask, семь round trips, byte-for-byte сохранность draft, stable highlighted
  identities и отсутствие post-name network bytes.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS (весь birth regression
  set, обе compile configurations).
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Возможностей улучшения вне
текущего scope не обнаружено.
