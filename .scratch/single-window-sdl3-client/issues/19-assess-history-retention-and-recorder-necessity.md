# Assess history retention and recorder necessity

Type: research
Status: resolved
Assignee: codex
Blocked by: 02, 11, 12, 14

## Question

Для disk persistence использовать [Persisted settings and files: source inventory](../research/persisted-settings-and-files.md): различать chat input history, message exports, private notes и guide bookmarks; учитывать текущую SDL3 асимметрию load/save paths. Результат исследования должен давать baseline/outcome facts для import/privacy policy в [Define settings and migration boundary](05-define-settings-and-migration-boundary.md), не утверждать её самостоятельно.

Какая история действительно необходима завершённому single-window client, а какая не должна храниться? По behavior baseline и согласованному Session presentation model классифицировать историю сообщений/chat, ordered events и consumer cursors, raw packet envelopes, formatted document pages, special-store draw/clear/animation operations и inputs/results для replay. Для каждого вида определить потребителя и наблюдаемый outcome, обязательность, минимальный lifetime, условия освобождения/compaction и возможность заменить историю текущим snapshot без потери поведения, repaint, неизвестного formatted content или ordering. Отдельно оценить необходимость полного recorder: нужен ли он для acceptance/debugging, достаточно ли тестовых fixtures/ограниченной записи, что происходит при выключенной записи, какие чувствительные данные нельзя сохранять без отдельной policy. Не предполагать бесконечный operation log или обязательный recorder; результат — обоснованная retention matrix и требования к последующему acceptance-решению, не реализация.


## Answer

Исследование завершено 2026-09-14. Каноническая retention matrix и source evidence: [History retention and recorder necessity: source-backed assessment](../research/history-retention-and-recorder-necessity.md). Это AFK research resolution, не утверждение новой runtime/disk/privacy policy.

Baseline требует bounded message/input recall, сохранения полученных сообщений через relog и независимой session identity для новых events. Message storage dedup сохраняет отдельные occurrences; input recall намеренно deduplicates. Активные requests, lossless envelopes/documents и logical canvas state должны переживать repaint/UI recreation. Старые pages/known operations/events могут освобождаться после ordered apply/consumer completion только при доказуемой сохранности всех surviving outcomes и lossless content; unknown operations требуют явного owner/reset/compaction contract.

Полный session recorder не установлен behavior baseline и не следует из Session presentation model. User macro recorder — отдельная обязательная capability. Fixtures и ограниченный capture являются возможными источниками acceptance evidence; выбор shipped capture, consumers/overflow, compaction и recorder-off/failure contract вынесен в [Define bounded working retention and optional capture](26-define-bounded-working-retention-and-optional-capture.md). Тикет создан и связан с acceptance/persistence; решение за пользователя не принято.

Подтверждённая общая SDL3 storage/import policy сохраняется. Точные histories/bookmarks paths, file owners/defaults и deletion/retention принадлежат [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md); credentials — [Define credential storage policy](24-define-credential-storage-policy.md). [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md) дополнен evidence требованиями для relog, duplicates, cursor delivery, checkpoints и recorder-off.

Source citations и существенные relog/history gates проверены статически. Runtime acceptance, реализация и численные resource budgets не выполнялись; существующий fog не изменён.
