# 06: Сохранить alternate class/race compile order

**Type:** implementation

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Второй поддержанный compile order ведёт игрока по sex → race → optional trait → optional body → class, сохраняя те же legal choices и compatibility, а каждый Backspace возвращает реального предыдущего owner; default order продолжает работать без расхождения semantics.

**Blocked by:** 05: Выбрать или пропустить body.

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-6-1

Labels: enhancement, ready-for-agent

**Production seam for TDD:** обе compile configurations запускают публичный `sv_endpoint_run(SvEndpointOptions)` через один production interaction/input route; seam и обязательность обеих сборок заданы P1-AC13–P1-AC19 и P1-AC28. Проверки используют `sv-build` для обеих конфигураций и `sv-core`.

- [x] Default `CLASS_BEFORE_RACE` order остаётся sex → class → race → optional trait → optional body с корректными confirm/backtrack owners. [Contract: P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC19; checks: `sv-build`, `sv-core`]
- [x] Alternate order собирается и проходит sex → race → optional trait → optional body → class; Backspace из race возвращает sex, из trait — race, из body — trait/race, из class — body/trait/race согласно реально доступным steps. [Contract: P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC19; checks: `sv-build`, `sv-core`]
- [x] В обеих сборках server-provided identity, compatibility, stable highlight, random/DNA semantics и keyboard/mouse/quit routing совпадают, а transitions не отправляют partial/completed result. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC10, P1-AC11, P1-AC12, P1-AC26, P1-AC28, P1-AC29; checks: `sv-core`]
- [x] Compile-order adaptation остаётся в SV и не требует изменения legacy birth implementation. [Contract: P2-AC01, P2-AC02, P2-AC04, P2-AC09; checks: `controller-review`]

## Answer

Добавлена локальная SV compile configuration `SV_CLASS_BEFORE_RACE` с default
значением `1`; `makefile.sv` принимает `SV_CLASS_BEFORE_RACE=0` для alternate
production build. Default flow не изменён: sex → class → race → optional trait →
optional body → stats. Alternate flow проходит sex → race → optional trait →
optional body → class → stats.

В alternate build race получает все server-provided identity, для которых есть
хотя бы один visible compatible class. После выбора race список class строится из
той же authoritative compatibility mask. Trait и body до class проверяют выбранную
race и свои server-provided masks; class повторно проверяет совместимость перед
select/confirm. Highlight сохраняется по identity, пока identity остаётся legal.

Backspace owners проверены через production input route: race → sex, trait → race,
body → trait либо race при skipped trait, class → body, trait либо race в зависимости
от реально доступных steps. Недоступные trait/body не становятся owner. Default
owners продолжают проверяться существующими сценариями. Каждый managed peer после
name handoff требует закрытия socket без дополнительных bytes, поэтому ни confirm,
ни Backspace, ни skip, ни quit не отправляют partial/completed character result.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)` → decoded
`SvContactSetup` → `SV_PREGAME_BIRTH` → `SvBirth` →
`sv_native_birth_event()`. Default и alternate binaries используют один и тот же
production interaction/input path; отдельного test-only choice engine нет.

TDD (`python3 -B tests/sv_name_editor_submit_checks.py`):

- RED full-order slice: exit 1 на `birth_race_confirmed` — alternate binary всё
  ещё назначал class сразу после sex.
- GREEN full-order slice: exit 0 — sex → race → trait → body → class → stats,
  authoritative race/class compatibility и default regression scenarios прошли.
- RED unavailable-body slice: exit 1 на ожидаемом owner `SV_BIRTH_TRAIT` после
  Backspace — skipped body ошибочно переводил alternate flow прямо в stats.
- GREEN unavailable-body slice: exit 0 — skip передаёт owner class, а Backspace
  возвращает trait; вариант с одновременно skipped trait/body возвращает race.

Итоговое production coverage тем же seam: default scenarios плюс alternate legal,
illegal, keyboard/mouse, random, DNA, incompatible DNA, Q/Ctrl-Q, все требуемые
Backspace owners и trait/body skip combinations — PASS.

Focused verification:

- `python3 -B tests/sv_name_editor_submit_checks.py` — PASS.
- `make -s -C src -f makefile.sv tomenet-sv` — PASS.
- `make -s -C src -f makefile.sv SV_CLASS_BEFORE_RACE=0 tomenet-sv` — PASS.
- `python3 -B tests/sv_name_rejection_recovery_checks.py` — PASS.
- `python3 -B tests/sv_endpoint_checks.py` — PASS.
- `python3 -B tests/sv_ordinary_create_cancel_checks.py` — PASS.
- `git diff --check` — PASS.

Полный build/test/review workflow не запускался. Legacy birth и shared gameplay
code не менялись. Наблюдавшиеся существующие SDL leaked-thread diagnostics и
common `atol` warning уже записаны в `docs/sv-improvements.md`; новых improvement
opportunities вне текущего scope не обнаружено.
