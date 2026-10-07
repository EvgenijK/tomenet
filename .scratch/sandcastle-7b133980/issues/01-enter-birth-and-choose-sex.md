# 01: Войти в birth и выбрать пол

**Type:** implementation

**What to build:** После подтверждённого сервером создания имени игрок остаётся в production pregame session, видит native sex choice из authoritative birth setup, меняет устойчиво подсвеченный вариант клавиатурой или мышью, использует допустимый random/DNA choice и подтверждает legal выбор без отправки partial или completed character result.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Production seam for TDD:** полный пользовательский путь вызывается через публичный `sv_endpoint_run(SvEndpointOptions)` и существующий server-confirmed handoff в `SV_PREGAME_BIRTH`. Seam принят критериями P1-AC01, P1-AC02, P1-AC03 и P1-AC28; проверки идут через `sv-core`, а отсутствие этого production handoff блокирует ticket вместо добавления test-only entry point.

- [ ] Existing SV-B-022 ordinary-name flow действительно достигает birth только после server acceptance, сохраняет generation и authoritative setup; ticket не реализует заново character management и останавливается как blocked, если этот prerequisite production handoff отсутствует. [Contract: P1-AC01, P1-AC28; checks: `sv-core`, `controller-review`]
- [ ] Native birth interaction показывает legal server-provided sex values с устойчивыми identities; illegal/stale value не меняет draft и owner, а random и доступный DNA choice сохраняют baseline semantics без чтения или записи DNA persistence. [Contract: P1-AC03, P1-AC10, P1-AC26, P1-AC27; checks: `sv-core`, `controller-review`]
- [ ] Keyboard и mouse navigation меняют одну production interaction selection, confirmation переводит owner к следующему шагу, а Q/Ctrl-Q на sex использует baseline quit/retry outcome. [Contract: P1-AC03, P1-AC11, P1-AC12, P1-AC28; checks: `sv-core`]
- [ ] Вход, navigation, random/DNA, confirm и quit не создают partial result и не отправляют completed play packet; final completed-character send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC12, P1-AC29; checks: `sv-core`, `controller-review`]
- [ ] Реализация и production-path tests остаются по возможности внутри SV, сохраняют behavior baseline и не выделяют shared код только ради устранения допустимого изолирующего дублирования. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC05, P2-AC09; checks: `controller-review`]
