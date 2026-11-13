# SV-C054-001 — Macro load/save UI поверх stateful preferences

Статус: planned, 2026-11-09.
Labels: enhancement
Тип: обязательная поздняя интеграция принятого C004; не новые capability IDs.

C054 связывает actual macro management с stateful C004 production loader.
[C052](../c052/SV-C052-001-preference-orchestration.md) владеет layers/reload/body
contexts, [C053](../c053/SV-C053-001-preference-runtime.md) — recognition/execution.
Это дополнение к 35 canonical outcomes C054, не их полный план. Здесь завершаются
explicit macro-load caller evidence и сохранность mixed preference records.

## Production seam и работа

1. Macro load/load-class/reinitialize/stage-switch UI передаёт намерение живому
   `SvPreferences`, вызывает `sv_preferences_load` с explicit purpose, role и
   C052 context. Options-only wrappers создают временный owner и не подходят:
   production A state, P/H/C/D definitions, trigger/action opaque bytes, source
   provenance и includes должны сохраняться между последовательными loads.
2. UI inspection/edit/list читает `sv_preferences_macro/record/diagnostic`,
   не реконструирует definitions из input table. Однобайтовый `SvInputBindings`
   не представляет arbitrary PRF macros, waits или multi-byte triggers. Kind,
   delete identity, exact escapes, effective encoded prefix и source-defined
   macro-only/body/trigger-exclusive gates сохраняются.
3. Ordinary manual load допускает разрешённые X/Y, `#:/!/?` effects; actual
   message/action recipients подключаются через C016/C053, без повторов на
   refresh/list/redraw. Сам selected source manual caller загружается без
   собственного automatic arcade tail-load; его nested includes сохраняют
   normal wrapper semantics и могут вызвать arcade load по исходному gate.
   Missing/invalid/partial result не выдаётся за clean success; valid tail и
   уже применённые effects не откатываются как transactional import.
4. Explicit macro Save направляется в shared U/user destination и сохраняет
   unrelated recognized mixed records/includes/actions; option edits сами по
   себе не переписывают этот source и не превращают файл в OPT snapshot.
   Preserve whole linked stage group при publication/conflict failure по policy,
   не заявлять complete при частичной записи. Никакого blanket macro autoresave
   при normal load, UI close, quit или alias conversion.

## Минимальные Linux проверки и завершение

Настоящий UI caller загружает mixed PRF, затем второй файл с trigger без новой A:
проверить production definitions/kind/delete и сохранённый action, actual
execution через C053 и messages через C016. UI inspect/redraw не повторяет effects.
Один explicit shared Save→reload сохраняет unrelated X/Y/%/#/! records и source
ownership; missing/partial load, cancel и applicable failed publication/conflict
дают честный результат без false complete. Использовать минимум относящихся
production core/UI checks; не запускать новую broad macro matrix.

Связка завершена при actual macro UI→stateful loader/store→C053 execution и
безопасном mixed-record Save. Parser/queue и single-byte fake binding не
закрывают macro flow; integration gaps устраняются здесь и у C052/C053.

## Статус C004 и ограничения

C004 и задачи 001–004 приняты и завершены решением пользователя 2026-11-09;
эта поздняя интеграция и её необходимые исправления обязательны в текущем этапе,
но не блокируют и не переоткрывают C004 автоматически. [C004 evidence](../../sv-c004-evidence.md)
сохраняет границу проверенного core и отсутствовавших consumers. Три canonical
C004 IDs, conditions и девять obligations остаются в [snapshot](../c004/coverage.json);
эта задача не переносит allocation и не повышает canonical runtime statuses.
Результаты нужны приёмке своего этапа; обратная ссылка в C004 evidence допустима
для трассировки, не является gate его состоявшейся приёмки.

Только минимально необходимые Linux core checks через production путь; расширение
проверок согласовать по [AGENTS](../../../AGENTS.md). Windows/MinGW/Wine deferred.
Не создавать новый harness, general fault framework или полную visual matrix.
Изменения SV-local; соблюдать main-thread ownership, явные связи и generation
lifetime из ADR-0002/0003/0005/0006. Это план: реализация и PASS этой связки не заявлены.
