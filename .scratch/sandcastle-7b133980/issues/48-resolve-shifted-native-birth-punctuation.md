# 48: Разрешить shifted punctuation в native birth bindings

**Type:** repair

**Tracker role:** Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.

**What to build:** Реальные SDL keyboard events для Shift+/, Shift+=, Shift+8, Shift+3 и Shift+5 вызывают соответственно Guide, point-buy increase, random, staged DNA и automatic DNA через production birth input context. Разрешение printable punctuation по физическому scancode и modifier state сохраняет уже проверенную Shift/Caps семантику букв, owner, stable highlight, draft и запрет partial/completed-character send.

**Blocked by:** 44: Сохранить регистр native birth bindings (completed).

**Status:** ready-for-agent

Labels: bug, ready-for-agent

**Finding ID:** `D-9ff71cd954b73dcd` (`birth.case-sensitive-key-binding-semantics`), one coherent repair of the reopened shifted-printable facet; the completed letter-case behavior from ticket 44 remains preserved and is not reimplemented as a separate defect.

**Production seam for TDD:** публичный `sv_endpoint_run(SvEndpointOptions)` получает authoritative server-provided birth setup and DNA draft; real `SDL_EVENT_KEY_DOWN` events with physical scancode, SDL base keycode and Shift/Caps/Ctrl modifiers pass through the native SDL adapter into the public `input/birth-interaction.h` production interaction/input-router boundary. `SvEndpointOutcome` and the controlled peer observe owner, stable highlighted identity, draft, Guide placeholder lifecycle and absence of prohibited outbound bytes. This seam is accepted by P1-AC02, P1-AC10, P1-AC11, P1-AC12, P1-AC24, P1-AC26 and P1-AC28; checks are `sv-core` and `sv-build`.

- [ ] Red production-path cases inject realistic scancode/base-key/modifier triples rather than precomposed symbol keycodes and show that Shift+/ opens the Guide placeholder, Shift+= increases the selected point-buy stat, Shift+8 performs random, Shift+3 performs staged DNA and Shift+5 starts automatic DNA; the green implementation resolves printable punctuation from the SDL event without changing the accepted Shift XOR Caps letter rules. [Contract: P1-AC02, P1-AC08, P1-AC10, P1-AC11, P1-AC24, P1-AC26, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] Shift+/ is exercised at every canonically reachable Guide entry: it displays exactly `The guide is in development`, then closes to the same birth caller with unchanged stable highlight and draft and no Guide-state or network operation. Unshifted `/` and unavailable Guide owners remain safely consumed by the birth context without opening the placeholder. [Contract: P1-AC10, P1-AC11, P1-AC23, P1-AC24, P1-AC25, P1-AC28, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
- [ ] Shift+= changes only the currently selected point-buy stat under its existing legality and remaining-point rules; Shift+8 random and Shift+3 staged DNA traverse the legal server-provided sex/class/race/trait/body/stats/mode owners in both compile-time class/race orders, retaining compatibility checks, stable identities, unavailable-step skipping and existing ordered-stats behavior. [Contract: P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC11, P1-AC19, P1-AC26, P2-AC04, P2-AC05; checks: `sv-build`, `sv-core`]
- [ ] Shift+5 starts automatic DNA only where the existing owner policy permits it, preserves the accepted suspension/resumption and illegal-rejection semantics around ordered stats, and does not add DNA persistence, `birth.restore-dna` or `birth.save-dna`. [Contract: P1-AC02, P1-AC03, P1-AC04, P1-AC05, P1-AC06, P1-AC07, P1-AC08, P1-AC09, P1-AC10, P1-AC26, P1-AC27, P2-AC04, P2-AC05; checks: `sv-core`, `controller-review`]
- [ ] Unshifted base keys, unsupported modifier combinations and illegal punctuation inputs do not invoke shifted-symbol commands or change owner/highlight/draft; keypad multiply and the completed lowercase/uppercase Q, Ctrl-Q, Shift+Caps and uppercase-H bindings remain green. Q/Ctrl-Q, Backspace, disconnect and stale-generation handling still prohibit partial/completed-character sends, with final send owned by SV-B-024. [Contract: P1-AC02, P1-AC10, P1-AC11, P1-AC12, P1-AC13, P1-AC14, P1-AC15, P1-AC16, P1-AC17, P1-AC18, P1-AC21, P1-AC22, P1-AC29, P2-AC04, P2-AC05; checks: `sv-core`]
- [ ] The repair stays SV-local where possible, uses only the production native adapter and interaction/input-router path, and introduces neither a fixed-response wizard nor a test-only symbol decoder. Any unrelated legacy/shared/SV improvement discovered during implementation is recorded separately rather than added to this repair. [Contract: P1-AC28, P2-AC01, P2-AC02, P2-AC03, P2-AC05, P2-AC06, P2-AC07, P2-AC08, P2-AC09; checks: `controller-review`]

**Deferred check:** `P1-birth-runtime-evidence` / P1-AC30 remains deferred to the named producer and verifier at full acceptance on the exact accepted contract digest and HEAD. This implementation repair does not claim that native runtime evidence passed.
