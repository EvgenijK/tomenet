# 06: Сохранить alternate class/race compile order

**Type:** implementation

**What to build:** Второй поддержанный compile order ведёт игрока по sex → race → optional trait → optional body → class, сохраняя те же legal choices и compatibility, а каждый Backspace возвращает реального предыдущего owner; default order продолжает работать без расхождения semantics.

**Blocked by:** 05: Выбрать или пропустить body.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** обе compile configurations запускают публичный `sv_endpoint_run(SvEndpointOptions)` через один production interaction/input route; seam и обязательность обеих сборок заданы P1-AC13–P1-AC19 и P1-AC28. Проверки используют `sv-build` для обеих конфигураций и `sv-core`.

- [ ] Default `CLASS_BEFORE_RACE` order остаётся sex → class → race → optional trait → optional body с корректными confirm/backtrack owners. [Contract: P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC19; checks: `sv-build`, `sv-core`]
- [ ] Alternate order собирается и проходит sex → race → optional trait → optional body → class; Backspace из race возвращает sex, из trait — race, из body — trait/race, из class — body/trait/race согласно реально доступным steps. [Contract: P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC19; checks: `sv-build`, `sv-core`]
- [ ] В обеих сборках server-provided identity, compatibility, stable highlight, random/DNA semantics и keyboard/mouse/quit routing совпадают, а transitions не отправляют partial/completed result. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC10, P1-AC11, P1-AC12, P1-AC26, P1-AC28, P1-AC29; checks: `sv-core`]
- [ ] Compile-order adaptation остаётся в SV и не требует изменения legacy birth implementation. [Contract: P2-AC01, P2-AC02, P2-AC04, P2-AC09; checks: `controller-review`]
