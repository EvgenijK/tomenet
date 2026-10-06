# 17: Instrument complete native fallback runtime checks

**What to build:** Production-owned runtime evidence, подтверждающее завершение конкретного native-сценария и фактическое отсутствие входов в terminal fallback во всём его ходе.

**Blocked by:** none; контракт checker из 13 уже реализован.

**Status:** implemented; see `docs/sv-runtime-checks.md` for verification and retained acceptance blockers.

**Closes blocker:** `fallback_routes=0` сейчас печатается как константа, а candidate records имеют `runtimeCheck.completed=false`. Разблокирует [18](18-complete-scoped-stage-a-evidence.md) и [15](15-run-and-report-stage-a-acceptance.md).

## Scope

- [x] Определить ограниченный lifecycle runtime-check в SV: начало сценария, накопление фактических входов, завершение только после обработки required событий, отмен, child interactions и сериализации/проверки ответов.
- [x] Генерировать completion/counts из production-пути, а не из ожидаемого тестом результата, строк логов или отсутствия ошибки запуска. Ранний выход/ошибка оставляет check незавершённым.
- [x] Сохранить изоляцию session generation: результаты старой сессии не могут завершить check новой. Если сценарий намеренно включает reset, его проверка охватывает весь сценарий и не теряет сведения о предыдущей части.
- [x] Не создавать terminal adapter ради измерения. Текущий native-only путь должен иметь проверяемый нулевой результат; любой существующий или будущий разрешённый переход обязан регистрироваться до передачи управления. Зафиксировать эту границу в SV.
- [x] Выдавать только scenario ID, completion, bounded route ID/reason/count и total. Не записывать prompts, credentials, произвольные payloads или архив пользовательского ввода.
- [x] Обновить production scenarios и producer `tools/run_stage_a.py`, чтобы candidate `runtimeCheck` отражал измеренный результат. Не подставлять безусловное `completed=true` и не принимать literal `fallback_routes=0` за evidence.

## Acceptance and checks

- [x] Проверки через production SV application/scenario seam: полностью завершённый native flow, незавершённый/ошибочный flow, отмена, child flow и session reset.
- [x] Проверить реальный production collector на ненулевом входе; отдельная test-only реализация счётчика запрещена. Полноценный legacy fallback для negative case не требуется.
- [x] Production evidence checker отвергает incomplete check и любой fallback entry, даже с зарегистрированным route ID; корректный check устраняет именно `evidence-runtime`, не маскируя остальные blockers.
- [x] Linux software/OpenGL regression, exact replies, macro ordering и timing остаются корректными. Изменившийся executable инвалидирует прежнее runtime/human evidence; связать повторный MinGW/Wine запуск с 16.

## Sources

`docs/capabilities/native-evidence.md`, `tools/native_evidence.py`, `src/client/sv/main.c`, `src/client/sv/app.[ch]`, `tests/sv/scenarios/`, предложение “Record runtime fallback absence before promoting Stage A observations” в `docs/sv-improvements.md`.
