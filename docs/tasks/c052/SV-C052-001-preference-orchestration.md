# SV-C052-001 — macro layers, reloads и server context

Статус: planned, 2026-11-09.
Labels: enhancement
Owned IDs: `capability.preferences.macro-precedence` (existing C052 allocation).
Зависимости реализации: принятый production C004 и существующие C011/C014
session transitions. C053 принимает actual macro execution позднее; он не
является prerequisite или readiness gate C052.

## Проблема и исходные callers

`src/client/sv/settings/preferences.[ch]` уже хранит A/P/H/C/D, включает файлы,
обрабатывает gates/effects и bootstrap. `login/flow.c` применяет fresh character
OPT до первого options packet. Global/race/trait/class/character/form macro
orchestration по реальным server transitions пока отсутствует.

C004 завершён и принят решением пользователя 2026-11-09. Эта работа и её проверки
обязательны в C052, не являются условием повторной приёмки C004. Дефекты,
найденные при интеграции, исправляются в текущем этапе; evidence backlink в C004
разрешён как трассировка. Canonical allocation/conditions/obligations не меняются;
исторические три `preferences.*` ID C004 не переезжают в C052.

Baseline: `src/client/c-init.c:308 initialize_player_pref_files`;
`src/client/nclient.c:Receive_char_info` (вызов 2867), `Receive_various` (2905);
`src/client/c-files.c:load_charspec_macros` (1870 onward),
`process_pref_file`/`process_pref_file_manual` (1840/1859).
Перед реализацией пересверить действительные version branches и reload callers.

## Реализация

1. Продолжить один session-owned `SvPreferences` state после согласованного
   C011/C014 handoff. При готовности actual character identity выполнить
   baseline global.prf → race → trait → class → character PRF, с исходными
   race/trait/class availability gates. Сохранить ранний OPT pre-send point
   и исходный поздний character OPT/keymap-init порядок; class OPT autoload
   не добавлять. Shared PRFs читаются через U overlay/B fallback, own OPT — S.
2. Связать once-per-character load с реальным decoded `Receive_char_info`
   equivalent, а не redraw/login-view read. Смена identity/relog получает
   свежую state по lifecycle C011/C014 без прошлого overlay. Полные и split/chained
   packet branches сохраняют signedness, absent defaults, byte fields и порядок;
   partial frame не применяет layer changes.
3. Подключить later character/form reloads к тем же semantic transitions,
   что baseline. Учитывать `load_form_macros`, фактическое body name и возвращение
   в `Player`; не загружать все PRFs на каждый form packet или display refresh.
   Body reload context устанавливает `bodymonster`: `?` пропускается, `!` остаётся
   разрешённым. Macro-only reload допускает A/P/H/C/D, обходит `%` и отсекает
   options/mappings/!/?/#. Exclusive-trigger reload сохраняет точный trigger gate.
4. Определить arcade selection из actual decoded server mode. Передавать в
   `SvPreferenceContext.arcade_file` source-backed system filename, не выводить
   server applicability из наличия файла или frontend name. Сохранить automatic
   tail после loads/includes и отсутствие прямого tail у manual selected source;
   вложенные includes manual source идут обычным путём. Не подменять глобальный
   `ANGBAND_SYS`; проверить SV-specific naming против применимого baseline.
5. Использовать существующие load/clone/options/macros/effects API. A state
   сохраняется через includes и последовательные loads одного owner; definitions
   заменяются/удаляются по exact decoded trigger. Никаких second parser или
   conversion через Unicode. Retained raw record не исполняется повторно;
   encoded effective prefix1023 и LIMIT/partial сохраняются.
6. Публиковать действительную semantic macro state и разрешённые effects через
   production interface, согласованный с C014/C016 и будущим C053 runtime.
   Сохранить WAIT96, XWAIT30 и pending prepend order с marker29; action1 затем
   action2 означает action2,29,action1,29. C052 доказывает real decoded update →
   layer orchestration → итоговые definitions/effects; actual execution и action
   caller regression обязательны в SV-C053-001 и своём gameplay stage. Их
   отсутствие не блокирует C052; успешный enqueue не называется исполнением.
   Missing/invalid include не откатывает уже применённый normal-load state;
   partial/diagnostics и source immutability остаются видимыми.

## Минимальная Linux проверка и готовность

В существующем production session/core entrypoint использовать различающиеся
U/S/B files с конфликтующим trigger во всех применимых layers; наблюдать конечные
kind/action bytes в настоящем production SvPreferences state после orchestration,
без отдельной тестовой реализации. Runtime C053 для этой проверки не требуется.
Через real decoded
character/form updates подтвердить initial order, later reload, `Player`,
`load_form_macros` off, body `?`/`!`, macro-only и trigger-exclusive branches.
Для actual server mode проверить arcade enabled/disabled и manual/include
различие; отсутствие серверного flow не закрывать выставленным тестом flag.

Сохранить обязательную wire проверку version/build branches `Receive_char_info`
из canonical `.wire`: split/chained frames, complete-only publication и точные
bytes. Отдельный actual action caller regression из `.allocation-checks`
обязательно выполняется в
[SV-C053-001](../c053/SV-C053-001-preference-runtime.md) и конкретном gameplay stage;
precedence evidence C052 его не подменяет, но его отсутствие не создаёт
C052→C053 prerequisite. Проверить missing/cyclic include с usable continuation,
no source writes и once-only публикацию effects при redraw/relog/teardown
через существующие C011/C014 owners.

Готовность: production decoded-update callers выполняют нужную macro
orchestration, точные semantic definitions/order/gates и server/form/relog
flows доказаны. Четыре canonical obligations получают фактическую disposition
в C052, включая явного обязательного владельца позднего execution/action
regression C053. C052 может быть принят до C053 по собственным результатам;
фактическое исполнение позднее не требует переоткрытия C052. Generic
write/replace failures проверяет настоящий writer; read-only loader не требует
нового Save framework. Только Linux; расширение core checks согласовать,
Windows/MinGW/Wine deferred. Реализация и приёмка C052 этим документом не объявлены.
