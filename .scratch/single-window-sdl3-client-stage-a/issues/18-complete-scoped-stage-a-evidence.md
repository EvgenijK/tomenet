# 18: Complete dependency provenance and the scoped Stage A evidence gate

**What to build:** Проверяемые свежие evidence-записи и результат именно Stage A, без заведомо неполных dependency inventories и без подмены будущей полной Windows/gameplay приёмки.

**Blocked by:** [17](17-instrument-native-fallback-runtime-checks.md) для полного runtime-check; [16](16-restore-mingw-and-run-wine-acceptance.md) для итоговых платформенных наблюдений. Инвентаризацию зависимостей можно готовить параллельно.

**Status:** implemented; scoped Stage A evidence gate passed (8/8, 21 records). Full runner 60/61; existing additional Wine Direct3D timing failure retained, human review remains ticket 19. See `docs/sv-stage-a-evidence.md`.

**Closes blocker:** Восемь текущих candidate records отвергаются по runtime и dependency errors; scoped inventories имеют `impact=unknown`, полное замыкание зависимостей не доказано. Разблокирует [19](19-review-and-close-stage-a-acceptance.md) и [15](15-run-and-report-stage-a-acceptance.md).

## Scope

- [ ] Установить и документировать transitive dependency scope сценариев: production SV и используемый common code/headers, build scripts/flags, fixture producers, config/resource discovery, compiler/SDK/runtime libraries и фактически используемые resources. `.d`/link metadata — исходные данные для проверки, не автоматическое доказательство полноты.
- [ ] Формировать content-addressed inventories из проверенных directory scopes, обнаруживающих добавление/удаление файлов, включая untracked/ignored. Не объявлять `impact=complete` только ради прохождения validator.
- [ ] При неизвестном влиянии использовать допустимый широкий immutable snapshot; artifacts/evidence хранить отдельно, чтобы не возникала самоссылка digest. Зафиксировать доступность и привязку external SDK/source roots.
- [ ] Связать actual results, executable/configuration/reports, obligations и manifest/allocation hashes с окончательной сборкой. Использовать завершённый runtime-check из 17. Wine сохраняет собственную идентичность и не превращается в Windows 10/11 record.
- [ ] Проверить весь scope A: четыре HP/message/key-request outcomes и четыре platform outcomes (`linux-build`, `windows-build`, `software-renderer`, `one-window`). Указать, какие обязательства подтверждены и какие относятся к более поздней полной приёмке.
- [ ] Развести scoped Stage A gate и полную cross-platform capability acceptance. Текущий `required_environments()` требует actual Windows для полного accepted claim; это не должно ни превращать Wine в Windows, ни искусственно переносить Windows 10/11 checkpoint из B в A. При необходимости ввести явный ограниченный checkpoint result, сохранив строгий контракт полного acceptance и совместимость существующих ledger/evidence.
- [ ] Обновить `run_stage_a.py`: убрать заведомо неполные candidates и безусловные pending/blocked placeholders; вычислять результат по фактическим build/scenario/evidence checks. До human review из 19 Stage A остаётся pending. Полноценные B–F capabilities остаются pending независимо от завершения A.

## Acceptance and checks

- [ ] Production checker принимает достаточные, корректно scoped evidence-записи; `evidence-dependencies`/`evidence-runtime` не игнорируются и не объявляются успешными negative tests.
- [ ] Через production CLI проверить изменение executable/config/report, изменение/добавление/удаление зависимости при неизменном HEAD, недоступный SDK/root, новый include/resource путь, неполный runtime-check и неверное platform/checkpoint заявление. Невалидные данные не могут дать passed Stage A.
- [ ] Scope A можно подтвердить Linux + требуемым MinGW/Wine checkpoint, сохраняя actual Windows obligations B/E/F и отсутствие claim о полном клиенте. Ни пустая evidence collection, ни schema-valid allocation сами по себе не завершают gate.
- [ ] Полный regression и повторная проверка registry/reconciliation/HTML tooling проходят; опубликовать воспроизводимые команды и новые artifacts для 15. Human approval не генерировать автоматически.

## Sources

`docs/capabilities/native-evidence.md`, `docs/sv-stage-a-acceptance.md`, `tools/native_evidence.py`, `tools/run_stage_a.py`, `src/makefile.sv`; предложение “Capture reviewed native dependency inventories from build outputs” в `docs/sv-improvements.md`; parent `spec.md` и утверждённый migration sequence.
