# 29: Сделать длинные birth choice lists полностью достижимыми

**Type:** repair

**What to build:** Игрок может увидеть и выбрать keyboard и mouse любой legal server-provided class, race или trait даже когда список длиннее доступной высоты surface. Bounded viewport прокручивается или перелистывается вместе со stable highlighted identity, а rendering и hit testing используют одну и ту же видимую область, включая disabled race entries.

**Blocked by:** 02: Выбрать class в default order (completed); 03: Выбрать совместимую race (completed); 04: Выбрать или пропустить trait (completed).

**Status:** resolved

**Assignee:** codex/sandcastle-7b133980-wave-18-3

Labels: bug, ready-for-agent

**Finding ID:** `D-1032de8ff6bb0776` (`birth.choice-list-viewport-reachability`).

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает protocol-permitted long class/race/trait lists; реальные keyboard, mouse and resize SDL events проходят native birth adapter и production input router, а production renderer и interaction outcome показывают одну stable server identity. Seam принят P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC11, P1-AC21 и P1-AC28; проверки — `sv-core` и `sv-build`.

- [x] Red production-path scenarios с class, race и trait lists длиннее видимого row count воспроизводят недостижимость поздней legal identity; green scenarios достигают и подтверждают её keyboard и mouse без test-only choice engine. [Contract: P1-AC02, P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC11, P1-AC28, P2-AC05; checks: `sv-core`]
- [x] Bounded viewport показывает только rows внутри текущей logical surface; navigation автоматически сохраняет highlighted identity видимой, а explicit scrolling/paging позволяет mouse дойти до каждой protocol-permitted legal identity. [Contract: P1-AC04, P1-AC05, P1-AC06, P1-AC10, P1-AC11, P2-AC04; checks: `sv-core`, `controller-review`]
- [x] Renderer и native hit testing используют одинаковый viewport offset и row geometry: click выбирает identity именно отображённой строки, double-click подтверждает её, а disabled/illegal race row не меняет прежний highlight или draft. [Contract: P1-AC02, P1-AC04, P1-AC10, P1-AC11; checks: `sv-core`]
- [x] Focus, network и resize interleaving не меняют owner или highlighted identity; resize пересчитывает viewport bounds и удерживает текущую identity видимой, если она остаётся legal. [Contract: P1-AC10, P1-AC21; checks: `sv-core`]
- [x] Short lists, random/DNA continuation, both compile orders, Backspace, quit and no-send behavior остаются green; final completed-character send остаётся у SV-B-024. [Contract: P1-AC02, P1-AC10, P1-AC12, P1-AC13, P1-AC14, P1-AC15, P1-AC19, P1-AC26, P1-AC29; checks: `sv-build`, `sv-core`]
- [x] Viewport implementation остаётся внутри SV presentation/input boundaries и не переносит interaction ownership в renderer; unrelated legacy/shared improvements только записываются отдельно. [Contract: P2-AC01, P2-AC02, P2-AC03, P2-AC04, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 остаётся deferred и не считается пройденным этим implementation repair; native full-acceptance evidence для длинных списков собирается только на exact accepted contract digest и HEAD.

## Answer

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Protocol fixture
передаёт длинные class/race/trait lists, SDL keyboard, wheel, mouse, focus и resize
events проходят native birth adapter и production input router, а результат
проверяется через `SvEndpointOutcome`. Отдельного test-only choice engine нет.

Добавлен SV-local bounded viewport, которым совместно пользуются renderer и native
hit testing. PageUp/PageDown и обычная keyboard navigation удерживают stable
highlighted server identity видимой; wheel меняет visible offset. Race viewport
сохраняет все server rows, включая disabled, а illegal click не меняет highlight
или draft. Resize пересчитывает row capacity и повторно включает текущую legal
identity в visible range. Interaction ownership остаётся в birth interaction/input
router; renderer только читает viewport.

TDD evidence:

- Red: `SV_BIRTH_CASE=long-class-viewport python3 -B tests/sv_name_editor_submit_checks.py`
  завершился с exit 1 на assertion ожидаемого позднего class 15: прежний path
  не достиг требуемой server identity через page/wheel/click.
- Green: та же команда завершилась с exit 0 для обоих compile orders после
  минимального viewport slice.
- Green: `SV_BIRTH_CASE=long-race-viewport python3 -B tests/sv_name_editor_submit_checks.py`
  и `SV_BIRTH_CASE=long-trait-viewport python3 -B tests/sv_name_editor_submit_checks.py`
  завершились с exit 0 для обоих compile orders. Сценарии подтверждают exact late
  identities 15 и 16; race дополнительно проверяет disabled visible row и
  focus/resize interleaving.
- Focused regression: `python3 -B tests/sv_name_editor_submit_checks.py`
  завершился с exit 0 и проверил полный профильный SV-B-022/023 production suite
  в обоих compile orders.

Полные `sv-build` и `sv-core` не запускались по указанию orchestrator. Deferred
`P1-birth-runtime-evidence` / P1-AC30 остаётся за full acceptance owner и здесь
не заявлен пройденным. Возможностей unrelated legacy/shared improvement не
обнаружено; `docs/sv-improvements.md` не менялся.
