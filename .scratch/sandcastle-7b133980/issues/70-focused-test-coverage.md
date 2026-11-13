# 70: Verify focused production coverage

Type: implementation
Tracker role: Wayfinder planning/execution history for canonical [SV-B-023](../../../docs/tasks/stage-b/SV-B-023-birth-choices.md); this child does not own Stage B capability IDs.
Status: resolved
Assignee: codex/sandcastle-7b133980-wave-47-1
Labels: enhancement, ready-for-agent

Test the completed SV change through production paths. Inspect the active tickets and add focused missing tests if needed. Run the relevant focused checks. Keep all explicit deferred acceptance checks pending. The orchestrator runs the full headless gate after assembly.

- [x] Relevant production paths are exercised and results recorded under Answer.
- [x] Any new tests are committed, with no test-only behavior or claims of unrun acceptance.

## Comments

- 2026-10-08: Claimed on `codex/sandcastle-7b133980-wave-47-1` before
  coverage inspection. The accepted public production seam is
  `sv_endpoint_run(SvEndpointOptions)`: the controlled peer negotiates the old
  server version and sends authoritative character setup, while real SDL
  events pass through contact decoding, the native birth adapter and the
  production interaction/input router. `SvEndpointOutcome` and the peer
  observe interaction state and prohibited outbound sends. This focused seam
  does not replace deferred native runtime evidence P1-AC30.
- 2026-10-08: Active-record inspection found ticket 69 as the only production
  delta since the preceding focused coverage review. Its eight endpoint cases
  already cover the legacy hidden-class decoder's direct-choice exclusion,
  navigation and stable identity, random/confirm, Backspace, quit, stale and
  disconnect paths, plus all three derived hidden identities. Modern and
  ordinary controls also remain covered. No missing focused test or behavior
  defect was found, so no test or production change and no red -> green loop
  are claimed.

## Answer

Проверен active record 69 и его SV-local version-aware decoder для legacy
server class layout. Существующие восемь `legacy-hidden-*` production endpoint
scenarios покрывают все изменённые ветви: identities 13–15 исключены из прямых
visible/legal choices с сохранением server order и stable public highlight;
random подтверждает только public identity; Backspace, uppercase-Q/Ctrl-Q,
stale generation и disconnect не отправляют partial/completed birth result;
Death Knight, Hell Knight и Corrupted Priest остаются достижимыми как derived
identities через authoritative race/trait/DNA flow. Отдельной test-only
реализации decoder или birth behavior нет. Legacy/shared файлы не изменялись;
новых improvement opportunities при проверке не обнаружено.

Production seam: публичный `sv_endpoint_run(SvEndpointOptions)`. Controlled
peer версии `4.7.0.2.0.0` передаёт authoritative setup с unprefixed legacy
classes; реальные keyboard, mouse, random, DNA, Backspace, quit, stale и
disconnect events проходят production contact decoder, native birth adapter и
interaction/input-router boundary. `SvEndpointOutcome` наблюдает owner,
visible/legal identities, selection и draft. Peer после разрешённого name
handoff требует EOF, поэтому любой partial/completed birth send провалил бы
scenario.

Focused verification, 2026-10-08:

```sh
for birth_case in \
  legacy-hidden-class-direct legacy-hidden-class-random \
  legacy-hidden-class-back legacy-hidden-class-stale \
  legacy-hidden-class-disconnect legacy-hidden-dna-death-knight \
  legacy-hidden-dna-hell-knight auto-dna-legacy-hidden-corrupted-priest \
  trait-hidden-dna-hell-knight hidden-derived-dna class-navigate
do
  SV_BIRTH_CASE="$birth_case" \
    python3 -B tests/sv_name_editor_submit_checks.py
done
```

Результат: **PASS**, общий exit 0. Все одиннадцать cases прошли в default и
`SV_CLASS_BEFORE_RACE=0` compile configurations. Проверены восемь legacy
decoder scenarios, modern encoded hidden-class control и два ordinary birth
controls. Runner при каждом запуске собирал production endpoint с
`-Wall -Wextra -Werror`, ASan и UBSan; compiler, sanitizer и assertion failures
не было. Ожидаемый controlled disconnect diagnostic, повторяющиеся
declared-font fallback и SDL `Leaked thread` diagnostics не сопровождались
failure и совпали с уже известным focused-runner output.

Это coverage-only ticket для уже реализованного поведения. Первый focused
production-seam запуск был green; дефект не обнаружен, поэтому навык TDD не
порождает red -> green цикл и новые tests не добавлены. Полные `sv-build`,
`sv-core`, controller review и общий workflow не запускались: их выполняет
orchestrator после assembly. `P1-birth-runtime-evidence` / P1-AC30 для accepted
contract digest `690a04cefb7fd0e1615dce46d1a538e1dcbd856232af1a1373be93f987095c82`
остаётся **deferred; not run** у named full-acceptance producer/verifier и этим
ticket не объявляется выполненным.
