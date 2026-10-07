# 17: Безопасно обработать random при пустом class set

**Type:** implementation

**What to build:** Если authoritative birth setup не даёт ни одного viable visible class/race path, random class input завершается предсказуемым production error/ignored outcome без arithmetic fault, изменения draft или сетевой отправки; lifecycle получает смысловой результат для безопасного продолжения или закрытия.

**Blocked by:** 02: Выбрать класс в default compile order (completed).

**Status:** ready-for-agent

**Assignee:** unassigned

Labels: bug, ready-for-agent

**Finding ID:** `D-c69d3e18378a47ab` (`birth.empty-class-random-divide-by-zero`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает empty/wholly-incompatible authoritative setup и подаёт `*` через native SDL birth adapter и production interaction; endpoint outcome и process/sanitizer result наблюдают semantic error, lifecycle и отсутствие send. Seam принят P1-AC06, P1-AC10 и P1-AC28; проверки — `sv-core` и `controller-review`.

- [ ] Empty или wholly incompatible server class/race setup не приводит к modulo-by-zero, crash или undefined behavior при random class command. [Contract: P1-AC02, P1-AC06, P1-AC10, P2-AC04; checks: `sv-core`]
- [ ] Production interaction отклоняет `*` без изменения highlight/draft либо сообщает lifecycle смысловую ошибку и безопасно закрывает interaction; выбранный outcome согласован с ADR error boundary и проверяется явно. [Contract: P1-AC06, P1-AC10, P1-AC28, P2-AC12; checks: `sv-core`, `controller-review`]
- [ ] Malformed/empty-list regression проходит через настоящий decoded setup, native router и `sv_endpoint_run`, а не через test-only random helper; default и alternate builds сохраняют эквивалентную безопасную семантику. [Contract: P1-AC19, P1-AC28, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Reject/error outcome не отправляет partial/completed character data и не меняет ownership final send. [Contract: P1-AC02, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Изменение остаётся локальным для SV; возможное более широкое hardening legacy/shared code записывается отдельно и не расширяет repair. [Contract: P2-AC01, P2-AC03, P2-AC06, P2-AC07, P2-AC08; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation ticket; evidence собирается только на full acceptance для exact accepted contract digest и HEAD.
