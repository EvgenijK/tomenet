# 16: Разрешить hidden derived class при выборе race

**Type:** implementation

**What to build:** Когда legal server setup связывает выбранный visible base class и race с hidden derived class, подтверждение race сохраняет в draft derived server identity, при этом backtracking продолжает показывать и подсвечивать выбранный игроком visible base class.

**Blocked by:** 06: Сохранить alternate class/race compile order (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-6d2b2e9f103a1224` (`birth.hidden-derived-class-resolution`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative decoded class/race setup, а keyboard/mouse/DNA input проходит production birth interaction и native input router; outcome наблюдает visible class highlight и committed draft identities. Seam принят P1-AC04, P1-AC06 и P1-AC28; проверка — `sv-core`.

- [ ] При race confirmation production interaction находит только server-provided hidden class, чей `base_class` и compatibility соответствуют выбранным visible class/race; draft получает derived identity в baseline boundary, а неподходящая hidden class не выбирается. [Contract: P1-AC02, P1-AC04, P1-AC06, P1-AC10, P2-AC04; checks: `sv-core`, `controller-review`]
- [ ] Backspace и повторный вход сохраняют visible base-class identity как пользовательский highlight, не раскрывают hidden class как обычный selectable item и повторно вычисляют derived draft только после legal race confirmation. [Contract: P1-AC06, P1-AC13, P1-AC15, P1-AC19; checks: `sv-core`]
- [ ] Production-path coverage проверяет representative visible-base → hidden-derived outcome и DNA mapping, включая reject для missing/incompatible/stale derived relationships и обе compile configurations. [Contract: P1-AC04, P1-AC06, P1-AC10, P1-AC19, P1-AC26, P1-AC28; checks: `sv-build`, `sv-core`]
- [ ] Resolve, backtrack и DNA paths не отправляют partial/completed character result и не добавляют DNA persistence или final-send ownership. [Contract: P1-AC02, P1-AC27, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Реализация и tests остаются SV-local и вызывают production interaction вместо отдельной test-only compatibility model. [Contract: P2-AC01, P2-AC04, P2-AC05; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
