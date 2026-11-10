# C009 — baseline, текущий SV и решения

Дата сверки: 2026-11-10. Выполнены три read-only агентских анализа и root review.
Graphify query использован до поиска исходников; точные факты сверены в файлах.
Граф дал общую навигацию, но не точный C009 contract. Координаты ниже относятся
к workspace исходной сверки до реализации. Изначальный audit код не менял;
actual реализация и startup closure описаны в конце этого документа,
[результаты проверок](../../sv-c009-evidence.md) — отдельно.

## Canonical contract

`docs/capabilities/stages.json`: C009, lua, Startup Lua reload, один outcome.
`manifest.json:20990` и `native-coverage.json:48628`: capability.lua.reload,
prerequisite files.resource-overlay, три exact obligations в coverage.json.
Реальный reload, Guide metadata, visible failure, audio restart warning only.
Canonical conditions/Windows rows сохраняются; текущие проверки только Linux.

## Проверенные источники исходной сверки до реализации

| Источник | Контракт |
|---|---|
| `src/client/c-script.c:32; src/server/lua/lbaselib.c:266–269,613–614` | NO_CLIENT_IOLIB исключает I/O library, native base dofile/dostring сохраняются без overlay resolver |
| `src/client/c-script.c:348–401` | init: existing Lua engine, libraries/bindings, xml/meta/audio/guide/activations |
| `c-script.c:282–344,416–447` | server flags до c-init.lua; finish_school/finish_spell и C-owned данные |
| `c-script.c:449–471` | reopen освобождает schools/spells names/arrays, пересоздаёт runtime и повторяет open |
| `c-script.c:495–546` | pern_dofile, overlay/fallback, errors; init/open игнорируют return codes, done не доказывает успех |
| `src/client/nclient.c:489–589` | static updated_audio/guide, flags устанавливаются при успешном INIT по substring; END close → last reload → audio warning → Guide metadata |
| `nclient.c:597+` | version branch checksum принадлежит C010, не Lua reload |
| `nclient.c:865` | open_lua после получения server features |
| `src/common/files.c:476–519` | close пишет destination и удаляет transfer record даже при failure |
| `lib/scpt/c-init.lua:53–85` | реальная цепочка вложенных scripts: player/s_aux/spells/runecraft и data tables |
| `lib/scpt/guide.lua:56` | вычисляемые Lua expressions; literal parser не покрывает metadata |
| `src/makefile.sv:47–55` | Lua runtime/bindings в SV пока не подключены |
| `src/client/sv/settings/audio.c:105` | literal reader audio registry, не Lua engine и не reload |
| `src/client/sv/files/files.h:15` | существующий resource resolver C003 |
| `src/client/sv/settings/startup.c:133` | -u хранится; INIT consumer — C010, не запрет local startup read |

Один SV-local adapter над тем же vendored engine допустим и сохраняет изоляцию.
Нельзя просто линковать legacy c-script и все его globals; нельзя заменить
семантику новым мини-интерпретатором. Required startup binding closure определить
в 001; реальные вызовы не подменять успешными заглушками. Runtime data школ/
заклинаний входит, их игровые действия/UI принимаются соответствующими этапами.

## Ошибка последнего END: решение пользователя

Baseline `nclient.c:547–560` вызывает reopen при нулевом числе transfers вне
условия success от local_file_close. Последний failed close тоже убирает record;
чтение Lua возможно со старого или частично записанного destination.
В manifest transfer.end и native-coverage.json:48926 написано «failed transfer
retains required error semantics and does not falsely reload». Это не буквальный
запрет любой попытки перечитывания.

После объяснения вариантов пользователь 2026-11-10 решил:
«сохраняем поведение старого клиента». Сохраняем reload attempt текущего
содержимого после последнего END даже при ошибке сохранения; отдельно возвращаем
ошибку передачи и результат reload. Успешный reload не превращает failed transfer
в успешный и не даёт ложной startup readiness. C010 сохраняет ERR,
C011 принимает решение продолжения по явным результатам. Reload может также
завершиться ошибкой; сохранность старого файла не обещается.
Вариант подавления reload при failed last close не выбран.
Вопрос закрыт; blocker needs-info задачи 002 снят. Canonical тексты сохранены.

## Sticky flags и failure visibility

updated_audio/guide — function-static и не сбрасываются в Receive_file. После
первого подходящего INIT warning/rebuild повторяются на следующих last END,
даже если следующий batch не содержал этих файлов. По правилу behavior baseline
сохраняем этот lifetime в application owner; session generation проверяется до
принятия события. Нужен accepted-INIT seam, сохраняющий flags немедленно:
INIT → disconnect до END не снимает flag, и last END новой session его видит.
Rejected INIT/`-u` не меняют flags; DATA/END failure их не снимает. END completion
не должен восстанавливать flags только из текущего session batch.
Batch reset не внедрять скрыто. Возможное улучшение отдельно
записано в sv-improvements.md.

Visible missing/invalid failure требуется canonical contract. SV должен вернуть
явный semantic result, а не повторить игнорирование ошибок init/open. Это
адаптация error reporting по ADR-0006, не разрешение ввести restricted Lua subset.
Actual session owner решает продолжение обязательного startup; необязательная
ошибка не должна автоматически завершать всё приложение.

## Preference, Guide и downstream границы

Строка manifest про preference lifecycle не означает новый Reload action.
PRF !/? и local source execution не дают права реализовать C058 заранее.
C009 готовит runtime и данные; actual transfer вызывается C010, подготовка сессии
принимается C011, session teardown — C014. Исполнение Lua и metadata не доказывает
готовый Guide; текущая Guide entry сохраняет точный development placeholder.
Приёмка B/C002–C008 этим планом не переоткрывается; отсутствующие consumers
остаются явно непроверенными. При исходном планировании core-only приёмка не
предполагалась заранее; затем пользователь поручил закрыть C009 сейчас
2026-11-10: [решение](../../sv-stage-c009-acceptance.md). Поздние integrations
обязательны своим этапам и не блокируют/не переоткрывают C009 автоматически.


## Actual startup closure SV-C009-001

Реализация 2026-11-10: `src/client/sv/lua/runtime.[ch]`, `constants.c` и
изолированные `lua-engine/*.o` в `makefile.sv`. Используется существующий Lua 4.0.1
из `src/server/lua`; legacy c-script/lua_bind/generated UI bindings не линкуются.
Base/string/debug libraries открываются; I/O/system и math library не добавляются
(как `init_lua` baseline). Native base `dofile`/`dostring` сохраняются. Только
startup loader и `pern_dofile(Ind, name)` применяют C003 resolver `scpt/name`:
shared user → bundled, только при отсутствии; invalid overlay не маскируется.

Реальный bundled путь исполняет 41 загрузку (40 разных файлов, `activations.lua`
загружается дважды): early `xml.lua`, `meta.lua`, `audio.lua`, `guide.lua`,
`activations.lua`; после реальных server flags — `c-init.lua` и его `player.lua`,
`s_aux.lua`, `spells.lua`, `runecraft.lua`, `races.lua`, `classes.lua`, `traits.lua`,
`attributes.lua`, `bpr.lua`, `mimicry.lua`, повторный `activations.lua`.
`spells.lua` исполняет `s_mana`, `s_fire`, `s_air`, `s_water`, `s_earth`, `s_convey`,
`s_divin`, `s_tempo`, `s_nature`, `s_udun`, `p_offense`, `p_defense`, `p_curing`,
`p_support`, `dr_arcane`, `dr_physical`, `m_ppower`, `m_attunement`, `m_mintrusion`,
`d_astral`, `o_shadow`, `o_spirit`, `o_hereticism`, `o_unlife` (все `.lua`).
Неактивные закомментированные script calls не превращаются в обязательный load.

`constants.c` регистрирует 587 имён из actual exported constants `w_util.c`,
`w_play.c`, `w_spells.c`; значения берутся из production C macros, а не из
копии script data. Required callbacks: настоящий `pern_dofile`; u32 util
`mod/band/bor/bxor/bnot/bshl/bshr` (они в baseline перекрывают bitlib names),
long `imod/lshift/rshift/arshift`; `new_school/new_spell/spell` и diagnostics.
`Ind=1`; server-specific RPG/ARCADE/FUN/PARTY/TEST и TEMP0–7 выставляются перед
`c-init.lua`. Доступ к player/session globals и gameplay/UI/network callbacks
не подменяется заглушками: deferred function bodies остаются настоящим Lua,
их исполнение поздними consumers не принимается C009. Если overlay реально
вызывает отсутствующий gameplay callback при startup, загрузка явно ошибочна.

Actual `finish_school` и `finish_spell` вызываются из production Lua. Bundled
результат — 25 C-owned школ и 310 spells; userdata `spell(i)` читает/меняет
реальные C fields с type validation и byte/s16 casts generated binding baseline.
Не-C поля (включая extrareq/extralev) сохраняются как script-owned mate fields.
Строки и arrays принадлежат runtime; clear/reload/destroy освобождают state и
данные. Typed Guide getters читают реальные globals/tables и копируют строки
потребителю, не передавая borrowed Lua pointers. Cleanup не удаляет shared files.

Диагностика `_ALERT` информационная; `_ERRORMESSAGE` и protected call failures
дают semantic failure. Первый nested `pern_dofile` failure сохраняет path/status,
даже если script игнорирует boolean return. Missing finalizer проверяется raw
lookup без unprotected undefined-global hook. Failed open retry пересоздаёт state,
успешный open идемпотентен; actual new-session caller обязан clear/reload.
Успешный protected вызов finish_* также проверяет фактически созданный C-owned
name/record: no-op finalizer не выдаёт пустые arrays за ready state.

Owner main-thread synchronous; bundled scratch probe на Linux исполнил полный
путь примерно за 10 ms CPU без оптимизации. Это измерение shipped scripts,
не доказательство latency bounds произвольного overlay. Background framework,
новые ограничения Lua языка и instruction quotas в C009 не добавлялись.
Actual C010 delivery/C011 readiness/C014 teardown остаются поздним evidence.
