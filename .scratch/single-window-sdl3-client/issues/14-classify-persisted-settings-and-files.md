# Classify persisted settings and files

Type: research
Status: resolved
Assignee: codex
Blocked by: 01

## Question

Как классифицируются все persisted keys и files между safe read-only import, отдельной modern UI configuration и неизменяемыми legacy artifacts, включая gameplay options, macros, auto-inscriptions, credentials, audio/graphics packs, bookmarks, histories, screenshots/dumps и server file updates?

## Answer

Source-backed inventory охватывает 200 уникальных lexical `option_info` names, распознаваемые CFG/Windows INI keys и PRF opcodes, файловые семейства и форматы, Linux/Windows roots, load/save triggers и resource-update paths. Класс назначается отдельной записи и операции владельца, а не расширению файла: gameplay/macro/inscription/birth/audio/resource data — кандидаты selective read-only import; single-window presentation preferences требуют отдельной UI schema; legacy Term configuration, неизвестные records, backups/caches и exports не становятся автоматически импортируемыми настройками. Credentials и personal documents/history выделены отдельно. Это классификационная основа для человеческого решения, не утверждение import/storage policy.

Stock loaders не дают read-only гарантии: preference actions могут исполняться, options применяются немедленно, старые `.opt`, `.ins` и `.dna` могут автоматически перезаписываться. SDL3 `my_fopen` не покрывает прямые file operations; chat/bookmarks имеют разные load/save roots. CFG writer-only subset keys и неактивные/дефектные PRF paths отмечены отдельно. Server updates остаются core resource flows и требуют согласованного allow-listed interface, не превращаются в UI preference import.

Полный отчёт: [Persisted settings and files: source inventory](../research/persisted-settings-and-files.md). Проверены совпадение 200 имён с lexical source registry и существование/границы source citations; число enabled options, runtime parity, disk failures и все compile-gate combinations не проверены.

Уточнены уже существующие [Define settings and migration boundary](05-define-settings-and-migration-boundary.md), [Assess history retention and recorder necessity](19-assess-history-retention-and-recorder-necessity.md) и [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md). Новых неохваченных точных решений и прояснившихся resource-budget/fallback fog patches не обнаружено; их scope остаётся прежним.
