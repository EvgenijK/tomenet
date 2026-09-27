# SV option ownership — factual draft

> Review precedence, 2026-09-20: this source-audit draft retains historical proposals. The current consolidated contract is [persistence contract](../persistence-contract.md), based on ticket answers through Q40. In particular, display options DO affect SV, fonts import into BOTH roles, startup is fullscreen, Lanczos maps to Linear, own CFG/OPT conversion is Save-only, and server file transfer retains original behavior WITHOUT the proposed new allow-list. Do not implement superseded proposals below.

Статус: **черновик для обсуждения**, 2026-09-20. Это полная сверка option-key inventory и предложения disposition; ни таблица, ни слово «предлагается» не закрывают решения по каждому ключу. Производственный код и пользовательские настройки не менялись.

## Утверждённая граница

Применены актуальные Comments в [Specify persistence ownership and UI configuration schema](../issues/25-specify-persistence-ownership-and-ui-configuration-schema.md#comments), включая Independent OPT files, Q21/Q22/Q23/Q24/Q25–Q28:

- Собственные OPT в `sv/` внутри SDL3 user root; прежний `X:<name>`/`Y:<name>` формат, explicit import legacy values, без дальнейшего автоматического наследования или синхронизации legacy OPT. Global/character scope сохраняется. Точные nested paths здесь не выбираются.
- Общие macro/resource PRF остаются на прежних местах. Допустимые option directives применяются к текущей SV-сессии; legacy Term windows к SV не применяются. Save settings записывает значения только в собственные SV OPT; исходный PRF не переписывается этим действием. Явный macro save — отдельная общая операция.
- Settings preview применяется сразу, «Сохранить» пишет на диск, отмена возвращает pre-edit snapshot; обычный выход не сохраняет несохранённые settings. Сама read-time конверсия старого имени не разрешает незаметно обойти Save-only либо переписать legacy OPT.
- SV layout — самостоятельный owner normal/big effective geometry. Старый `big_map` не задаёт SV layout. Предложенное ниже распознавание `big_map` как derived record не вводит новую пользовательскую настройку.
- Q1 задаёт global scope новым SV fonts/scales/layout/panels. Это само по себе не переводит все старые presentation options из character scope в global.

## Метод и пределы фактов

Из `option_info` в [c-tables.c:186](../../../src/client/c-tables.c#L186) извлечены все **200 уникальных имён / 201 initializer row**. Повторяется только `big_map`, с различным `o_enabled` по `GLOBAL_BIG_MAP`. Множество имён автоматически сверено с полным registry [Persisted settings and files](persisted-settings-and-files.md), без пропусков и добавлений.

Из 200 имён **12 находятся внутри `#if 0`**, а 188 представлены вне этого блока. Это lexical/source count, не число видимых переключателей, не результат сборки и не доказательство поддержки всех effects в SV. Поле `o_enabled` определяет показ в старом options menu, а не приемлемость `X/Y`: parser ищет `o_text`/`o_var`, не проверяя `o_enabled` ([c-files.c:1555](../../../src/client/c-files.c#L1555)). Hidden option может иметь активные клиентские/серверные consumers. Неактивные `#if 0` rows не являются recognized keys текущей таблицы.

Колонка default — **буквальное исходное `o_norm` expression** до pref loading, а не итоговый профиль пользователя и не утверждённое новое значение SV. Default initialization: [c-init.c:221](../../../src/client/c-init.c#L221). Колонка gate описывает компиляцию initializer row; feature guards внутри consumers ею не исчерпываются. Например sound effects зависят от audio build/backend, dual-mask weather — от graphics capability. Runtime/consumer gates необходимо сохранить при реализации; они не создают разрешение включить несуществующую возможность по одному наличию ключа.

Source descriptions в таблице приведены дословно как provenance существующего намерения. Они не являются обещанием новой UI-компоновки или доказательством, что все consumers description реализуют. Для спорных случаев ниже даны отдельные consumer references.

## Scope, precedence и writer facts для каждой строки

`G→C; PRF` в таблице означает один и тот же существующий file-level scope **каждого** имени: defaults → `pref.prf`/includes → `global.opt` → `pref-<sys>.prf` → font/graphics preferences → `global-<sys>.opt`, затем `<character>.opt` и загружаемые macro PRF. В старом loader нет отдельного per-key global-only ограничения. Последующие `X/Y` переопределяют текущую переменную и `Client_setup.options[i]` сразу. Источники: [c-init.c:234](../../../src/client/c-init.c#L234), [c-init.c:252](../../../src/client/c-init.c#L252), [c-init.c:309](../../../src/client/c-init.c#L309), [c-files.c:1555](../../../src/client/c-files.c#L1555). Для I этот scope лишь lexical compatibility context: такого recognized key в активной таблице нет.

В SV сохранение global/character scopes уже принято; точный порядок собственных layers и file names задаёт основной persistence registry. Предложения G/P не меняют scope: используют собственный global/character option owner, тот же key и boolean meaning, исходный default при отсутствии применимого override; explicit legacy import и допустимые PRF directives меняют SV effective value. Перенос кода loader не должен вновь подмешать legacy `global.opt`/`options.prf` как пользовательскую базу через include. Bundled defaults/resource mappings и user migration source надо различать по owner, не по одному расширению.

`options_dump` пишет все известные rows и flags существующих Terms, создавая полный snapshot вместо round-trip исходного файла ([c-util.c:15297](../../../src/client/c-util.c#L15297)). Unknown records/comments/includes не сохраняются. Conversion obsolete names вызывает dump при загрузке `.opt`/`options.prf` ([c-files.c:1801](../../../src/client/c-files.c#L1801)). Этот старый writer нельзя просто направить в общий source macro/OPT для SV. X/Y loaded from shared macro не делает shared macro owner-ом их дальнейшего settings save.

Числа option slots являются частью `Client_setup.options[]` и version-dependent `PKT_OPTIONS` ([nclient.c:8051](../../../src/client/nclient.c#L8051)); UI/renderer no-op не разрешает удалять/reindex старые сетевые slots. Политика inert/imported value и effective wire value требует отдельной записи там, где они различаются.

## Обозначения предложенного disposition

| Код | Предложение для обсуждения |
|---|---|
| G | Переиспользовать существующую gameplay/input/selection/server semantic option с собственным SV OPT owner. Это не доказывает, что key только серверный. |
| P | Переиспользовать semantic presentation/audio/message preference; применять смысл в надлежащем SV consumer, не координаты/Term. Исходные wording/default/scope — отправная точка, не утверждение конкретного UX. |
| L | Распознать legacy terminal presentation/render-workaround option, **предлагаемый SV no-op**; не создавать ради неё Term/panel. Сохранность inert imported representation и wire value ещё требуют спецификации. |
| D | Derived `big_map`: actual normal/big задаёт утверждённый SV layout, legacy X/Y не переключает его. Не сохранять второе конкурирующее requested preference. |
| I | Row внутри `#if 0`: не активировать; import должен отличать известное историческое имя от действующей опции и сообщать disposition. Не считать его рабочим default. |
| U | Нужна отдельная semantic/ownership проверка; номер U ссылается на вопросы после registry. Никакой automatic conversion в новое UI поведение не предлагается. |

Для G/P слово «переиспользовать» не обходит допустимость источника: migration preview не выполняет effects, ordinary permitted PRF loading применяет session effects, settings save имеет SV destination. Для L/I/U draft не выбирает молчаливое удаление imported source records.

## Все option keys

`T`/`F` в колонках означают исходные C expressions `TRUE`/`FALSE`. `—` gate означает unconditional initializer row в таблице. `E` — `o_enabled` (видимость legacy меню).

| Key → c_cfg member | Default; E; row gate | Старый scope | Existing source description | Предложение | Source |
|---|---|---|---|---|---|
| `rogue_like_commands` | F; E=T; — | G→C; PRF | Rogue-like keyset (for covering lack of a numpad) | G | [L189](../../../src/client/c-tables.c#L189) |
| `newbie_hints` | T; E=T; — | G→C; PRF | Display tips/warnings for new players | P | [L191](../../../src/client/c-tables.c#L191) |
| `censor_swearing` | T; E=T; — | G→C; PRF | Censor certain swear words in public messages | G | [L193](../../../src/client/c-tables.c#L193) |
| `highlight_chat` → `hilite_chat` | T; E=T; — | G→C; PRF | Highlight chat messages containing your name | P | [L196](../../../src/client/c-tables.c#L196) |
| `highbeep_chat` → `hibeep_chat` | T; E=T; — | G→C; PRF | Beep on chat messages containing your name | P | [L198](../../../src/client/c-tables.c#L198) |
| `page_on_privmsg` | F; E=T; — | G→C; PRF | Beep when receiving a private message | P | [L200](../../../src/client/c-tables.c#L200) |
| `page_on_afk_privmsg` | T; E=T; — | G→C; PRF | Beep when receiving a private message while AFK | P | [L202](../../../src/client/c-tables.c#L202) |
| `big_map` | F; E=T при `!GLOBAL_BIG_MAP`, E=F при `GLOBAL_BIG_MAP` | G→C; PRF | Double height of the map shown in the main window | D | [L207](../../../src/client/c-tables.c#L207), [L210](../../../src/client/c-tables.c#L210) |
| `font_map_solid_walls` | T; E=T; — | G→C; PRF | Certain fonts only: Walls look like solid blocks | P | [L214](../../../src/client/c-tables.c#L214) |
| `view_animated_light` → `view_animated_lite` | T; E=T; — | G→C; PRF | Animate lantern light, flickering in colour | P | [L216](../../../src/client/c-tables.c#L216) |
| `wall_lighting` | T; E=T; — | G→C; PRF | Generally enable lighting/shading for wall grids | P | [L218](../../../src/client/c-tables.c#L218) |
| `view_lamp_walls` | T; E=T; — | G→C; PRF | Use special colors for lamp-lit wall grids | P | [L220](../../../src/client/c-tables.c#L220) |
| `view_shade_walls` | T; E=T; — | G→C; PRF | Use special colors to shade wall grids | P | [L222](../../../src/client/c-tables.c#L222) |
| `floor_lighting` | T; E=T; — | G→C; PRF | Generally enable lighting/shading for floor grids | P | [L224](../../../src/client/c-tables.c#L224) |
| `view_lamp_floor` | T; E=T; — | G→C; PRF | Use special colors for lamp-lit floor grids | P | [L226](../../../src/client/c-tables.c#L226) |
| `view_shade_floor` | T; E=T; — | G→C; PRF | Use special colors to shade floor grids | P | [L228](../../../src/client/c-tables.c#L228) |
| `view_light_extra` → `view_lite_extra` | T; E=T; — | G→C; PRF | Lamp light affects more floor/wall types | P | [L230](../../../src/client/c-tables.c#L230) |
| `alert_hitpoint` | F; E=T; — | G→C; PRF | Beep/message about critical hitpoints/sanity | P | [L233](../../../src/client/c-tables.c#L233) |
| `alert_mana` | F; E=T; — | G→C; PRF | Beep/message about critically low mana pool | P | [L235](../../../src/client/c-tables.c#L235) |
| `alert_afk_dam` | F; E=T; — | G→C; PRF | Beep when taking damage while AFK | P | [L237](../../../src/client/c-tables.c#L237) |
| `alert_offpanel_dam` | T; E=T; — | G→C; PRF | Beep when taking damage while looking elsewhere | U — U1: старый trigger — screen_icky, не факт скрытия карты в SV | [L239](../../../src/client/c-tables.c#L239) |
| `exp_bar` | T; E=T; — | G→C; PRF | Show experience bar instead of a number | L | [L242](../../../src/client/c-tables.c#L242) |
| `uniques_alive` | F; E=T; — | G→C; PRF | List only unslain uniques for your local party | P | [L245](../../../src/client/c-tables.c#L245) |
| `warn_unique_credit` | F; E=T; — | G→C; PRF | Beep on attacking a unique you already killed | P | [L247](../../../src/client/c-tables.c#L247) |
| `limit_chat` | F; E=T; — | G→C; PRF | Chat only with players on the same floor | G | [L249](../../../src/client/c-tables.c#L249) |
| `no_afk_msg` | F; E=T; — | G→C; PRF | Don't show AFK toggle messages of other players | P | [L251](../../../src/client/c-tables.c#L251) |
| `overview_startup` | F; E=T; — | G→C; PRF | Display overview resistance/boni page at startup | U — U2: выбирает legacy страницу character sheet при входе; SV destination не задан | [L253](../../../src/client/c-tables.c#L253) |
| `allow_paging` | T; E=T; — | G→C; PRF | Allow users to page you (recommended!) | G | [L256](../../../src/client/c-tables.c#L256) |
| `ring_bell` | T; E=T; — | G→C; PRF | Beep on misc warnings and errors | P | [L258](../../../src/client/c-tables.c#L258) |
| `linear_stats` | F; E=T; — | G→C; PRF | Stats are represented in a linear way | P | [L262](../../../src/client/c-tables.c#L262) |
| `exp_need` | F; E=T; — | G→C; PRF | Show the experience needed for next level | P | [L264](../../../src/client/c-tables.c#L264) |
| `depth_in_feet` | T; E=T; — | G→C; PRF | Show dungeon level in feet | P | [L266](../../../src/client/c-tables.c#L266) |
| `newb_suicide` | T; E=T; — | G→C; PRF | Display newbie suicides | P | [L268](../../../src/client/c-tables.c#L268) |
| `show_weights` | T; E=T; — | G→C; PRF | Show weights in object listings | P | [L270](../../../src/client/c-tables.c#L270) |
| `time_stamp_chat` | F; E=T; — | G→C; PRF | Add half-hourly time stamps to chat window | U — U3: вставляет периодические marker messages; не эквивалент timestamp каждого сообщения | [L274](../../../src/client/c-tables.c#L274) |
| `hide_unusable_skills` | T; E=T; — | G→C; PRF | Hide unusable skills | P | [L276](../../../src/client/c-tables.c#L276) |
| `short_item_names` | F; E=T; — | G→C; PRF | Don't display known 'flavours' in item names | P | [L278](../../../src/client/c-tables.c#L278) |
| `keep_topline` | F; E=T; — | G→C; PRF | Don't clear messages in the top line if avoidable | L | [L280](../../../src/client/c-tables.c#L280) |
| `target_history` | F; E=T; — | G→C; PRF | Add target informations to the message history | U — U3: маршрутизация target info/recall требует проверки против принятой истории | [L282](../../../src/client/c-tables.c#L282) |
| `taciturn_messages` | F; E=T; — | G→C; PRF | Suppress server messages as far as possible | P | [L284](../../../src/client/c-tables.c#L284) |
| `always_show_lists` | T; E=T; — | G→C; PRF | Always show lists in item/skill selection | U — U2: implicit list visibility у legacy item/skill selectors; SV picker mapping не задан | [L286](../../../src/client/c-tables.c#L286) |
| `no_weather` | F; E=T; — | G→C; PRF | Disable weather visuals and sounds completely | P | [L289](../../../src/client/c-tables.c#L289) |
| `player_list` | F; E=T; — | G→C; PRF | Show a more compact player list in @ screen | U — U2: компактность server/legacy @ screen, не автоматически modern social panel | [L292](../../../src/client/c-tables.c#L292) |
| `player_list2` | F; E=T; — | G→C; PRF | Compacts the player list in @ screen even more | U — U2: второй уровень компактности @ screen; не новый layout panel | [L294](../../../src/client/c-tables.c#L294) |
| `flash_player` | T; E=T; — | G→C; PRF | Flash own character icon after far relocation | P | [L297](../../../src/client/c-tables.c#L297) |
| `highlight_player` → `hilite_player` | F; E=T; — | G→C; PRF | Highlight own character icon with the cursor | U — U1: semantic self-highlight возможен, Term cursor side effect не переносится | [L300](../../../src/client/c-tables.c#L300) |
| `basic_players_symb` | F; E=T; — | G→C; PRF | Use very basic static symbols for players | P | [L302](../../../src/client/c-tables.c#L302) |
| `subterm_flicker` | T; E=T; — | G→C; PRF | Show animated text colours in sub-windows | U — U4: scope определяется legacy Term; нужен semantic animation scope | [L305](../../../src/client/c-tables.c#L305) |
| `no_verify_sell` | F; E=T; — | G→C; PRF | Skip safety question when selling items | G | [L307](../../../src/client/c-tables.c#L307) |
| `no_verify_destroy` | F; E=T; — | G→C; PRF | Skip safety question when destroying items | G | [L309](../../../src/client/c-tables.c#L309) |
| `auto_afk` | T; E=T; — | G→C; PRF | Set 'AFK mode' automatically | G | [L312](../../../src/client/c-tables.c#L312) |
| `idle_starve_kick` | T; E=T; — | G→C; PRF | Disconnect when idle for 30s while starving | G | [L314](../../../src/client/c-tables.c#L314) |
| `safe_float` | F; E=T; — | G→C; PRF | Prevent floating for a short while after death | G | [L316](../../../src/client/c-tables.c#L316) |
| `safe_macros` | T; E=T; — | G→C; PRF | Abort macro if item is missing or an action fails | G | [L318](../../../src/client/c-tables.c#L318) |
| `auto_untag` | F; E=T; — | G→C; PRF | Remove unique monster inscription on pick-up | G | [L321](../../../src/client/c-tables.c#L321) |
| `clear_inscr` | F; E=T; — | G→C; PRF | Clear @-inscriptions on taking item ownership | G | [L323](../../../src/client/c-tables.c#L323) |
| `auto_inscr_server` | F; E=T; — | G→C; PRF | Also use predefined server-side auto-inscriptions | G | [L325](../../../src/client/c-tables.c#L325) |
| `stack_force_notes` | T; E=T; — | G→C; PRF | Merge inscriptions when stacking | G | [L327](../../../src/client/c-tables.c#L327) |
| `stack_force_costs` | T; E=T; — | G→C; PRF | Merge discounts when stacking | G | [L329](../../../src/client/c-tables.c#L329) |
| `stack_allow_items` | T; E=T; — | G→C; PRF | Allow weapons and armor to stack | G | [L331](../../../src/client/c-tables.c#L331) |
| `stack_allow_devices` | T; E=T; — | G→C; PRF | Allow wands/staffs/rods to stack | G | [L333](../../../src/client/c-tables.c#L333) |
| `whole_ammo_stack` | F; E=T; — | G→C; PRF | For ammo/misc items always operate on whole stack | G | [L335](../../../src/client/c-tables.c#L335) |
| `always_repeat` | T; E=T; — | G→C; PRF | Repeat obvious commands (eg search/tunnel) | G | [L337](../../../src/client/c-tables.c#L337) |
| `always_pickup` | F; E=T; — | G→C; PRF | Pick things up by default | G | [L339](../../../src/client/c-tables.c#L339) |
| `use_old_target` | T; E=T; — | G→C; PRF | Use old target by default | G | [L341](../../../src/client/c-tables.c#L341) |
| `autooff_retaliator` | F; E=T; — | G→C; PRF | Stop the retaliator when protected by GoI etc | G | [L343](../../../src/client/c-tables.c#L343) |
| `fail_no_melee` | F; E=T; — | G→C; PRF | Don't melee if other auto-retaliation ways fail | G | [L345](../../../src/client/c-tables.c#L345) |
| `basic_players_col` | F; E=T; — | G→C; PRF | Use very basic static colours for players | P | [L347](../../../src/client/c-tables.c#L347) |
| `auto_target` | F; E=T; — | G→C; PRF | Automatically set target to the nearest enemy | G | [L349](../../../src/client/c-tables.c#L349) |
| `thin_down_flush` | T; E=T; — | G→C; PRF | Thin down screen flush signals to avoid freezing | U — U5: снижает частоту 1ms delays после protocol flush; не просто GUI toggle | [L351](../../../src/client/c-tables.c#L351) |
| `disable_flush` | F; E=T; — | G→C; PRF | Disable delays from flush signals | U — U5: отключает delays после protocol flush; event-loop mapping не задан | [L353](../../../src/client/c-tables.c#L353) |
| `find_ignore_stairs` | F; E=T; — | G→C; PRF | Run past stairs | G | [L356](../../../src/client/c-tables.c#L356) |
| `find_ignore_doors` | T; E=T; — | G→C; PRF | Run through open doors | G | [L358](../../../src/client/c-tables.c#L358) |
| `find_cut` | T; E=T; — | G→C; PRF | Run past known corners | G | [L360](../../../src/client/c-tables.c#L360) |
| `find_examine` | T; E=T; — | G→C; PRF | Run into potential corners | G | [L362](../../../src/client/c-tables.c#L362) |
| `disturb_move` | F; E=T; — | G→C; PRF | Disturb whenever any monster moves | G | [L364](../../../src/client/c-tables.c#L364) |
| `disturb_near` | F; E=T; — | G→C; PRF | Disturb whenever viewable monster moves | G | [L366](../../../src/client/c-tables.c#L366) |
| `disturb_panel` | F; E=T; — | G→C; PRF | Disturb whenever map panel changes | G | [L368](../../../src/client/c-tables.c#L368) |
| `disturb_state` | F; E=T; — | G→C; PRF | Disturb whenever player state changes | G | [L370](../../../src/client/c-tables.c#L370) |
| `disturb_minor` | F; E=T; — | G→C; PRF | Disturb whenever boring things happen | G | [L372](../../../src/client/c-tables.c#L372) |
| `disturb_other` | F; E=T; — | G→C; PRF | Disturb whenever various things happen | G | [L374](../../../src/client/c-tables.c#L374) |
| `view_perma_grids` | T; E=T; — | G→C; PRF | Map remembers all perma-lit grids | G | [L376](../../../src/client/c-tables.c#L376) |
| `view_torch_grids` | F; E=T; — | G→C; PRF | Map remembers all torch-lit grids | G | [L378](../../../src/client/c-tables.c#L378) |
| `view_reduce_light` → `view_reduce_lite` | F; E=F; — | G→C; PRF | Reduce light radius when running | G | [L382](../../../src/client/c-tables.c#L382) |
| `view_reduce_view` | F; E=F; — | G→C; PRF | Reduce view radius in town | G | [L384](../../../src/client/c-tables.c#L384) |
| `easy_open` | T; E=T; — | G→C; PRF | Automatically open doors | G | [L387](../../../src/client/c-tables.c#L387) |
| `easy_disarm` | F; E=T; — | G→C; PRF | Automatically disarm traps (except under items) | G | [L389](../../../src/client/c-tables.c#L389) |
| `easy_tunnel` | F; E=T; — | G→C; PRF | Automatically tunnel walls | G | [L391](../../../src/client/c-tables.c#L391) |
| `audio_paging` | T; E=T; — | G→C; PRF | Use audio system for page/alert, if available | P | [L394](../../../src/client/c-tables.c#L394) |
| `paging_master_vol` → `paging_master_volume` | F; E=T; — | G→C; PRF | Play page/alert sounds at master volume | P | [L396](../../../src/client/c-tables.c#L396) |
| `paging_max_vol` → `paging_max_volume` | F; E=T; — | G→C; PRF | Play page/alert sounds at maximum volume | P | [L398](../../../src/client/c-tables.c#L398) |
| `no_ovl_close_sfx` | T; E=T; — | G→C; PRF | Prevent re-playing sfx received after <100ms gap | P | [L400](../../../src/client/c-tables.c#L400) |
| `ovl_sfx_attack` | T; E=T; — | G→C; PRF | Allow overlapping combat sounds of same type | P | [L402](../../../src/client/c-tables.c#L402) |
| `no_combat_sfx` | F; E=T; — | G→C; PRF | Don't play melee/launcher attack/miss sound fx | P | [L404](../../../src/client/c-tables.c#L404) |
| `no_magicattack_sfx` | F; E=T; — | G→C; PRF | Don't play basic spell/device attack sound fx | P | [L406](../../../src/client/c-tables.c#L406) |
| `no_defense_sfx` | F; E=T; — | G→C; PRF | Don't play attack-avoiding/neutralizing sound fx | P | [L408](../../../src/client/c-tables.c#L408) |
| `half_sfx_attack` | F; E=T; — | G→C; PRF | Skip every second attack sound | P | [L410](../../../src/client/c-tables.c#L410) |
| `cut_sfx_attack` | T; E=T; — | G→C; PRF | Skip attack sounds based on speed and bpr | P | [L412](../../../src/client/c-tables.c#L412) |
| `ovl_sfx_command` | T; E=T; — | G→C; PRF | Allow overlapping command sounds of same type | P | [L414](../../../src/client/c-tables.c#L414) |
| `ovl_sfx_misc` | T; E=T; — | G→C; PRF | Allow overlapping misc sounds of same type | P | [L416](../../../src/client/c-tables.c#L416) |
| `ovl_sfx_mon_attack` | T; E=T; — | G→C; PRF | Allow overlapping monster attack sfx of same type | P | [L418](../../../src/client/c-tables.c#L418) |
| `ovl_sfx_mon_spell` | T; E=T; — | G→C; PRF | Allow ovl. monster spell/breath sfx of same type | P | [L420](../../../src/client/c-tables.c#L420) |
| `ovl_sfx_mon_misc` | T; E=T; — | G→C; PRF | Allow overlapping misc monster sfx of same type | P | [L422](../../../src/client/c-tables.c#L422) |
| `no_monsterattack_sfx` | F; E=T; — | G→C; PRF | Don't play basic monster attack sound fx | P | [L424](../../../src/client/c-tables.c#L424) |
| `positional_audio` | T; E=T; — | G→C; PRF | Play '3d' positional sound fx, via normal stereo | P | [L426](../../../src/client/c-tables.c#L426) |
| `no_house_sfx` | F; E=T; — | G→C; PRF | Don't play ambient/weather sound in buildings | P | [L428](../../../src/client/c-tables.c#L428) |
| `quiet_house_sfx` | T; E=T; — | G→C; PRF | Play quieter ambient/weather sound in buildings | P | [L430](../../../src/client/c-tables.c#L430) |
| `mute_when_idle` | F; E=T; — | G→C; PRF | Mute music+ambient sfx while AFK/idle in town | P | [L432](../../../src/client/c-tables.c#L432) |
| `alert_starvation` | T; E=T; — | G→C; PRF | Beep when taking damage from starvation | P | [L434](../../../src/client/c-tables.c#L434) |
| `use_color` | T; E=F; — | G→C; PRF | (deprecated) Use color if possible | U — U6: hidden; реальные menu consumers есть, нельзя считать неизвестным ключом | [L439](../../../src/client/c-tables.c#L439) |
| `other_query_flag` | F; E=F; — | G→C; PRF | Prompt for various information (mimic polymorph) | G | [L441](../../../src/client/c-tables.c#L441) |
| `quick_messages` | F; E=T; `#if 0` | G→C; PRF | Activate quick messages (skill etc) | I | [L447](../../../src/client/c-tables.c#L447) |
| `carry_query_flag` | F; E=F; `#if 0` | G→C; PRF | (broken) Prompt before picking things up | I | [L449](../../../src/client/c-tables.c#L449) |
| `show_labels` | T; E=F; `#if 0` | G→C; PRF | (broken) Show labels in object listings | I | [L451](../../../src/client/c-tables.c#L451) |
| `show_choices` | F; E=F; `#if 0` | G→C; PRF | (broken) Show choices in certain sub-windows | I | [L453](../../../src/client/c-tables.c#L453) |
| `show_details` | T; E=F; `#if 0` | G→C; PRF | (broken) Show details in certain sub-windows | I | [L455](../../../src/client/c-tables.c#L455) |
| `expand_look` | F; E=F; `#if 0` | G→C; PRF | (broken) Expand the power of the look command | I | [L457](../../../src/client/c-tables.c#L457) |
| `expand_list` | F; E=F; `#if 0` | G→C; PRF | (broken) Expand the power of the list commands | I | [L459](../../../src/client/c-tables.c#L459) |
| `avoid_other` | F; E=F; `#if 0` | G→C; PRF | (broken) Avoid processing special colors | I | [L461](../../../src/client/c-tables.c#L461) |
| `flush_failure` | T; E=F; `#if 0` | G→C; PRF | (broken) Flush input on various failures | I | [L463](../../../src/client/c-tables.c#L463) |
| `flush_disturb` | F; E=F; `#if 0` | G→C; PRF | (broken) Flush input whenever disturbed | I | [L465](../../../src/client/c-tables.c#L465) |
| `fresh_after` | F; E=F; `#if 0` | G→C; PRF | (obsolete) Flush output after every command | I | [L467](../../../src/client/c-tables.c#L467) |
| `speak_unique` | T; E=T; `#if 0` | G→C; PRF | Allow shopkeepers and uniques to speak | I | [L470](../../../src/client/c-tables.c#L470) |
| `shuffle_music` | F; E=T; — | G→C; PRF | Don't loop song files but shuffle through them | P | [L474](../../../src/client/c-tables.c#L474) |
| `permawalls_shade` | F; E=T; — | G→C; PRF | Display permanent vault walls in a special colour | P | [L476](../../../src/client/c-tables.c#L476) |
| `topline_no_msg` | F; E=T; — | G→C; PRF | Don't display messages in main window top line | L | [L478](../../../src/client/c-tables.c#L478) |
| `targetinfo_msg` | F; E=T; — | G→C; PRF | Display look/target info in message window too | U — U3: дополнительная доставка target info не должна дублировать SV occurrences | [L480](../../../src/client/c-tables.c#L480) |
| `live_timeouts` | T; E=T; — | G→C; PRF | Always update item timeout numbers on every tick | P | [L482](../../../src/client/c-tables.c#L482) |
| `flash_insane` | F; E=T; — | G→C; PRF | Flash own character icon when going badly insane | P | [L484](../../../src/client/c-tables.c#L484) |
| `last_words` | T; E=T; — | G→C; PRF | Get last words when the character dies | P | [L487](../../../src/client/c-tables.c#L487) |
| `disturb_see` | F; E=T; — | G→C; PRF | Disturb whenever seeing any monster | G | [L489](../../../src/client/c-tables.c#L489) |
| `diz_unique` | T; E=T; — | G→C; PRF | Displays lore when killing a unique monster | P | [L492](../../../src/client/c-tables.c#L492) |
| `diz_death` | T; E=T; — | G→C; PRF | Displays lore on monster that killed you | P | [L494](../../../src/client/c-tables.c#L494) |
| `diz_death_any` | T; E=T; — | G→C; PRF | Displays lore on monster that kills anyone | P | [L496](../../../src/client/c-tables.c#L496) |
| `diz_first` | T; E=T; — | G→C; PRF | Displays lore on first-time monster kill | P | [L498](../../../src/client/c-tables.c#L498) |
| `screenshot_format` | T; E=T; — | G→C; PRF | Screenshots are timestamped instead of numbered | P | [L500](../../../src/client/c-tables.c#L500) |
| `palette_animation` | T; E=T; — | G→C; PRF | Shade world surface colours depending on daytime | P | [L502](../../../src/client/c-tables.c#L502) |
| `play_all` | T; E=T; — | G→C; PRF | Loop over all available songs instead of just one | P | [L504](../../../src/client/c-tables.c#L504) |
| `id_selection` | T; E=T; — | G→C; PRF | Show/accept only eligible items for ID/*ID* | G | [L506](../../../src/client/c-tables.c#L506) |
| `hp_bar` | F; E=T; — | G→C; PRF | Display hit points as bar instead of numbers | L | [L508](../../../src/client/c-tables.c#L508) |
| `mp_bar` | F; E=T; — | G→C; PRF | Display mana pool as bar instead of numbers | L | [L510](../../../src/client/c-tables.c#L510) |
| `st_bar` | F; E=T; — | G→C; PRF | Display stamina as bar instead of numbers | L | [L512](../../../src/client/c-tables.c#L512) |
| `find_ignore_montraps` | T; E=T; — | G→C; PRF | Run through monster traps | G | [L515](../../../src/client/c-tables.c#L515) |
| `quiet_os` | F; E=T; — | G→C; PRF | Don't play beep/alert/page beeps through OS | P | [L518](../../../src/client/c-tables.c#L518) |
| `disable_lightning` | F; E=T; — | G→C; PRF | Disable visual screen flash effect for lightning | P | [L520](../../../src/client/c-tables.c#L520) |
| `macros_in_stores` | F; E=T; — | G→C; PRF | Eable macros while inside a store | G | [L522](../../../src/client/c-tables.c#L522) |
| `item_error_beep` | T; E=T; — | G→C; PRF | Beep when an item selection fails | P | [L524](../../../src/client/c-tables.c#L524) |
| `keep_bottle` | F; E=T; — | G→C; PRF | Keep the empty bottle when you quaff a potion | G | [L526](../../../src/client/c-tables.c#L526) |
| `easy_disarm_montraps` | F; E=T; — | G→C; PRF | Automatically disarm monster traps ('/edmt') | G | [L529](../../../src/client/c-tables.c#L529) |
| `no_house_magic` | F; E=T; — | G→C; PRF | Prevent using magic inside houses | G | [L531](../../../src/client/c-tables.c#L531) |
| `no_light_fainting` → `no_lite_fainting` | F; E=T; — | G→C; PRF | Disable shading effect for fainting light source | P | [L533](../../../src/client/c-tables.c#L533) |
| `auto_pickup` | F; E=T; — | G→C; PRF | Automatically pickup items (see '/apickup') | G | [L536](../../../src/client/c-tables.c#L536) |
| `auto_destroy` | F; E=T; — | G→C; PRF | Automatically destroy items (see '/adestroy') | G | [L538](../../../src/client/c-tables.c#L538) |
| `destroy_all_unmatched` | F; E=T; — | G→C; PRF | Destroys ALL unmatched items. (Like A'#' in &.) | G | [L540](../../../src/client/c-tables.c#L540) |
| `mp_huge_bar` | T; E=T; — | G→C; PRF | Also show mana pool as huge bar (big_map only) | L | [L543](../../../src/client/c-tables.c#L543) |
| `sn_huge_bar` | T; E=T; — | G→C; PRF | Also show sanity as huge bar (big_map only) | L | [L545](../../../src/client/c-tables.c#L545) |
| `hp_huge_bar` | T; E=T; — | G→C; PRF | Also show HP pool as huge bar (big_map only) | L | [L547](../../../src/client/c-tables.c#L547) |
| `clone_to_stdout` | F; E=T; — | G→C; PRF | Clone client chat and messages to stdout | U — U7: существующий непрерывный text clone; не автоматически новый recorder | [L550](../../../src/client/c-tables.c#L550) |
| `clone_to_file` | F; E=T; — | G→C; PRF | Clone client chat and messages to 'stdout.txt' | U — U7: существующий text append stdout.txt; owner/privacy/disposition не выбран | [L552](../../../src/client/c-tables.c#L552) |
| `first_song` | T; E=T; — | G→C; PRF | Start with first eligible song of a music event | P | [L554](../../../src/client/c-tables.c#L554) |
| `equip_text_colour` | F; E=T; — | G→C; PRF | Display equipment indices/weight in yellow | P | [L556](../../../src/client/c-tables.c#L556) |
| `equip_set_colour` | T; E=T; — | G→C; PRF | Colourize indices of items giving set bonus | P | [L558](../../../src/client/c-tables.c#L558) |
| `colourize_bignum` | F; E=T; — | G→C; PRF | Colourize prices, AU and XP in 3-digit columns | P | [L560](../../../src/client/c-tables.c#L560) |
| `flash_player2` | F; E=T; — | G→C; PRF | Flash own character icon after close relocation | P | [L563](../../../src/client/c-tables.c#L563) |
| `load_form_macros` | T; E=T; — | G→C; PRF | Automatically load form-specific macros on change | G | [L565](../../../src/client/c-tables.c#L565) |
| `auto_inscr_off` | F; E=T; — | G→C; PRF | Disable all client-side auto-inscriptions | G | [L567](../../../src/client/c-tables.c#L567) |
| `ascii_feats` | F; E=T; — | G→C; PRF | Disable font-specific mapping for all floor feats | P | [L570](../../../src/client/c-tables.c#L570) |
| `ascii_items` | F; E=T; — | G→C; PRF | Disable font-specific mapping for all items | P | [L572](../../../src/client/c-tables.c#L572) |
| `ascii_monsters` | F; E=T; — | G→C; PRF | Disable font-specific mapping for all monsters | P | [L574](../../../src/client/c-tables.c#L574) |
| `ascii_uniques` | F; E=T; — | G→C; PRF | Disable font-specific mapping for unique monsters | P | [L576](../../../src/client/c-tables.c#L576) |
| `no_flicker` | F; E=T; — | G→C; PRF | Disable all fast colour animations | P | [L579](../../../src/client/c-tables.c#L579) |
| `stun_huge_bar` | T; E=T; — | G→C; PRF | Show stun status as huge bar (big_map only) | L | [L581](../../../src/client/c-tables.c#L581) |
| `gfx_autooff_fmsw` | F; E=F; — | G→C; PRF | Auto-disable font_map_solid_walls for graphics | U — U6: hidden, но меняет font_map_solid_walls при graphics/login | [L585](../../../src/client/c-tables.c#L585) |
| `solid_bars` | T; E=T; — | G→C; PRF | Display status bars in solid style, not as '#' | L | [L588](../../../src/client/c-tables.c#L588) |
| `autopickup_chemicals` | T; E=T; — | G→C; PRF | Automatically pick up freshly dropped chemicals | G | [L590](../../../src/client/c-tables.c#L590) |
| `add_kind_diz` | T; E=T; — | G→C; PRF | Inv/eq window only: Item-to-chat-paste extra info | P | [L592](../../../src/client/c-tables.c#L592) |
| `hide_lore_paste` | F; E=T; — | G→C; PRF | Hide artifact/monster lore pastes in public chat | P | [L594](../../../src/client/c-tables.c#L594) |
| `new_retaliator` | T; E=T; — | G→C; PRF | Grant extra energy for an escape action | G | [L597](../../../src/client/c-tables.c#L597) |
| `sunburn_msg` | T; E=T; — | G→C; PRF | Show message if you start/stop burning in the sun | P | [L599](../../../src/client/c-tables.c#L599) |
| `wide_scroll_margin` | T; E=T; — | G→C; PRF | Scroll the screen further away from the edges | G | [L601](../../../src/client/c-tables.c#L601) |
| `gfx_palanim_repaint` | T; E=F; — | G→C; PRF | Flicker-free repaint, glitches on some systems | L | [L603](../../../src/client/c-tables.c#L603) |
| `gfx_hack_repaint` | T; E=F; — | G→C; PRF | May reduce flicker if gfx_palanim_repaint is off | L | [L605](../../../src/client/c-tables.c#L605) |
| `topline_first` | T; E=T; — | G→C; PRF | Top line shows first line of multi-line messages | L | [L607](../../../src/client/c-tables.c#L607) |
| `ascii_weather` | F; E=T; — | G→C; PRF | Always use plain ASCII symbols for weather | P | [L609](../../../src/client/c-tables.c#L609) |
| `no2mask_weather` | F; E=F; — | G→C; PRF | Never use dual-mask grapics for weather | U — U6: hidden, но влияет на dual-mask weather composition | [L611](../../../src/client/c-tables.c#L611) |
| `auto_inscr_server_ch` | T; E=T; — | G→C; PRF | Server-defined auto-inscriptions for chemicals | G | [L613](../../../src/client/c-tables.c#L613) |
| `screenshot_keys` | F; E=T; — | G→C; PRF | Swap CTRL+T and SHIFT+CTRL+T key (XHTML vs image) | U — U8: legacy CTRL+T/SHIFT+CTRL+T image/XHTML routing, mapping SV shortcuts не задан | [L616](../../../src/client/c-tables.c#L616) |
| `wild_resume_from_any` | T; E=T; — | G→C; PRF | Resume wilderness music even after non-wild music | P | [L618](../../../src/client/c-tables.c#L618) |
| `tavern_town_resume` | T; E=T; — | G→C; PRF | If wild_resume_from_any, also resume tavern/town | P | [L620](../../../src/client/c-tables.c#L620) |
| `st_huge_bar` | F; E=T; — | G→C; PRF | Also show stamina as huge bar (big_map only) | L | [L622](../../../src/client/c-tables.c#L622) |
| `huge_bars_gfx` | T; E=T; — | G→C; PRF | Display huge bars graphically if graphics are on | L | [L624](../../../src/client/c-tables.c#L624) |
| `autoinsc_debug` | F; E=T; — | G→C; PRF | Displays extra info on auto-inscriptions matching | G | [L627](../../../src/client/c-tables.c#L627) |
| `autoloot_dunonly` | F; E=T; — | G→C; PRF | Suppress auto-pickup/destroy while not in dungeon | G | [L629](../../../src/client/c-tables.c#L629) |
| `autoloot_dununown` | F; E=T; — | G→C; PRF | Suppress auto-pck/des if owned and not in dungeon | G | [L631](../../../src/client/c-tables.c#L631) |
| `flash_starvation` | T; E=T; — | G→C; PRF | Flash 'Starved' food status by colour animation | P | [L633](../../../src/client/c-tables.c#L633) |
| `autoswitch_inven` | T; E=T; — | G→C; PRF | Empty inven prompts auto-switch to equip or bag | G | [L635](../../../src/client/c-tables.c#L635) |
| `show_newest` | F; E=T; — | G→C; PRF | Marks the inventory slot of the 'newest' item | P | [L637](../../../src/client/c-tables.c#L637) |
| `log_music` | F; E=T; — | G→C; PRF | Log game-initiated music to 'Messages' window | P | [L639](../../../src/client/c-tables.c#L639) |
| `misc_no_flicker` | F; E=T; — | G→C; PRF | Only animate colours for main screen and messages | U — U4: scope определяется main/message Term flags | [L641](../../../src/client/c-tables.c#L641) |
| `prefer_subinven` | T; E=T; — | G→C; PRF | Prefer using items in bags over inventory items | G | [L643](../../../src/client/c-tables.c#L643) |

## Спорные semantics и точечные source evidence

1. **U1: alert/cursor contexts.** `alert_offpanel_dam` проверяет `screen_icky` при damage, а не произвольную видимость карты ([nclient.c:2096](../../../src/client/nclient.c#L2096), [nclient.c:2211](../../../src/client/nclient.c#L2211)). Для SV нужно назвать equivalent gameplay interaction state; термин «offpanel» не даёт этого автоматически. `highlight_player` управляет Term cursor ([nclient.c:3304](../../../src/client/nclient.c#L3304), [c-util.c:20000](../../../src/client/c-util.c#L20000)); semantic self-highlight может сохраниться, но cursor style остаётся у HTML prototype/принятых cursor rules.
2. **U2: бывшие document/picker layouts.** `overview_startup` лишь выбирает `csheet_page = 2` на входе ([nclient.c:2878](../../../src/client/nclient.c#L2878)); это не основание автоматически открыть SV окно поверх игры. `always_show_lists` запускает старые selection lists ([c-inven.c:1311](../../../src/client/c-inven.c#L1311), [c-spell.c:297](../../../src/client/c-spell.c#L297)). `player_list`/`player_list2` по source descriptions относятся к compact @ screen; не объявлять настройками modern social panel без отдельного mapping.
3. **U3: message routing.** `time_stamp_chat` — periodic marker lines, не формат timestamp каждого message ([c-tables.c:271](../../../src/client/c-tables.c#L271)); `target_history`/`targetinfo_msg` надо сверить с уже принятыми occurrence/recall/delivery semantics. Не добавлять duplicate messages ради старого второго Term. L `keep_topline`/`topline_no_msg`/`topline_first` касаются legacy top line ([c-util.c:4796](../../../src/client/c-util.c#L4796), [c-util.c:4932](../../../src/client/c-util.c#L4932)); proposed no-op не означает удаления самого сообщения.
4. **U4: animation scope.** `subterm_flicker` и `misc_no_flicker` сравнивают `Term`/`window_flag` в [z-term.c:1133](../../../src/client/z-term.c#L1133). `no_flicker` — отдельное глобальное animation preference; source не задаёт corresponding SV semantic regions для первых двух. Решить reuse semantic scope либо inert legacy setting, не создавать невидимые Terms для совместимости.
5. **U5: protocol flush timing.** `Receive_flush` делает `Term_fresh`, затем условный 1ms delay; `disable_flush` выключает delays, `thin_down_flush` сокращает их число ([nclient.c:4030](../../../src/client/nclient.c#L4030)). Решить, имеют ли эти legacy pacing settings допустимый смысл в выбранном SV event loop. Их нельзя переименовать в FPS/vsync или объявить сетевым batching по одному названию.
6. **U6: hidden не равно inactive.** `use_color` ещё используется в old options/window menus ([c-util.c:13239](../../../src/client/c-util.c#L13239)); основной monochrome text path частично закомментирован ([c-util.c:2022](../../../src/client/c-util.c#L2022)). `gfx_autooff_fmsw` изменяет `font_map_solid_walls` при graphics/login ([nclient.c:2888](../../../src/client/nclient.c#L2888)); `no2mask_weather` влияет на ветку compositing weather ([c-xtra1.c:5543](../../../src/client/c-xtra1.c#L5543)). Их скрытость не разрешает автоматический no-op. G `view_reduce_light`/`view_reduce_view` hidden, но сервер принимает slots 83/84 ([nserver.c:2848](../../../src/server/nserver.c#L2848)); effects видны в [xtra1.c:2045](../../../src/server/xtra1.c#L2045) и [cave.c:6895](../../../src/server/cave.c#L6895). G `other_query_flag` hidden, но используется в [c-spell.c:607](../../../src/client/c-spell.c#L607).
7. **U7: существующий text cloning.** `clone_to_stdout`/`clone_to_file` выводят сообщения в stdout/`stdout.txt` ([c-util.c:4862](../../../src/client/c-util.c#L4862), [c-util.c:4892](../../../src/client/c-util.c#L4892)). Отсутствие нового session recorder уже принято, но это не само по себе решение отключить/сохранить baseline continuous clone. Требуются explicit capability/disposition, file owner и согласование с privacy/working-history policy; draft не включает их по умолчанию (source default F) и не создаёт новый archive.
8. **U8: screenshot shortcut.** `screenshot_keys` описывает swap CTRL+T/SHIFT+CTRL+T между XHTML/image. Сохранить сырой boolean без mapping shortcut/action owners недостаточно. `screenshot_format` отдельно предлагает сохранить timestamped-vs-numbered filenames; collision policy новых exports уже принята в Q27.

L **bars** (`exp_bar`, `hp_bar`, `mp_bar`, `st_bar`, `*_huge_bar`, `solid_bars`, `huge_bars_gfx`) меняют legacy rasterized status/bar presentation ([c-xtra1.c:227](../../../src/client/c-xtra1.c#L227), [c-util.c:20017](../../../src/client/c-util.c#L20017)). Proposed no-op относится к старому render control, не к потере HP/MP/ST/XP/sanity/stun model values или удалению SV status indicators. Если отдельная visual preference всё-таки нужна, её следует сопоставить с HTML UX, а не автоматически портировать terminal shape.

L **repaint hacks** `gfx_palanim_repaint`/`gfx_hack_repaint` стоят в SDL3/X11 legacy paint paths ([main-sdl3.c:5186](../../../src/client/main-sdl3.c#L5186), [main-x11.c:5531](../../../src/client/main-x11.c#L5531)); proposed SV no-op не обещает воспроизведение старых repaint defects и не отключает palette animation preference.

D **big_map** имеет default FALSE в обеих source rows; `GLOBAL_BIG_MAP` меняет menu-enabled flag, не имя. Current features включают macro ([defines-features.h:893](../../../src/common/defines-features.h#L893)); непосредственная legacy geometry reaction расположена под `#ifndef GLOBAL_BIG_MAP` ([c-util.c:19927](../../../src/client/c-util.c#L19927)). Следовательно shared OPT не обязательно менял фактическую geometry этой SDL3 сборки; независимость SV выбирается по утверждённой ownership policy, не по ошибочному предположению о каждой old build. При импорте `big_map` не имеет приоритета над wide/small SV layout. Exact normalization/inert preservation/wire synchronization должен указать финальный registry.

## Соседние записи, которые не входят в 200 имён

- `W:<window>:<use>` — отдельный opcode, не option key. Принято не применять legacy Term windows к SV. Старый parser заменяет `window_flag[i]` ([c-files.c:1583](../../../src/client/c-files.c#L1583)); не превращать W в SV panel position/visibility. Original shared macro сохраняется settings save-ом; policy explicit mixed macro save должна обеспечить сохранность unrelated records.
- `%:` включает файл в текущий preference context ([c-files.c:1075](../../../src/client/c-files.c#L1075)). Раздельный OPT namespace должен действовать и на record provenance при include; current loader по расширению не различает source owners.
- `X/Y` deprecated aliases в [c-files.c:1092](../../../src/client/c-files.c#L1092) не являются дополнительными `option_info` names. Например `recall_flicker`→`subterm_flicker`, `instant_retaliator`→inverted `new_retaliator`, `basic_players`/`consistent_players`→`basic_players_symb` плюс `basic_players_col=FALSE`. Важно: alias `view_reduce_lite` explicitly discards line ([c-files.c:1184](../../../src/client/c-files.c#L1184)), тогда как canonical `view_reduce_light` остаётся действующим hidden key. Полный alias transform registry нужен отдельно; не использовать общее snake_case угадывание.
- Unknown X/Y/opcode вызывает parse warning с продолжением загрузки, не transactional rejection ([c-files.c:1735](../../../src/client/c-files.c#L1735)); explicit migration должен report unknown disposition без исполнения shared source. Решение о совместимых unknown fields main CFG не автоматически задаёт политику неизвестных PRF opcodes.

## Что этот draft ещё не решает

- Принять/изменить предложенные L/no-op и U1–U8 mappings, включая распознавание/round-trip inert values и их отдельный wire disposition. G/P — candidates reuse, не blanket approval нового UX по description.
- Точные SV global/character filenames, applicable layer precedence и scope выбора «Сохранить» для изменённой character option; формальная conflict policy нескольких SV instances.
- Полный decoder/default/feature matrix после preprocessing целевых Windows/Linux builds; source rows и o_enabled не заменяют её. Не удалять network slots ради уменьшения меню.
- Full alias/directive registry и conversion owner: migration/ordinary PRF load/settings save различаются; сохранение источника и Save-only должны выдерживать прочтение obsolete names.
- Для L/I/U точная схема inert import representation, diagnostics и повторного импорта; этот документ не даёт разрешения ни тихо отбросить такие записи, ни применить их как новые SV controls.

## Проверка полноты и provenance

- Git HEAD на момент сверки: `4211671279ff820575c32239272bca68cf8762f7`; читается рабочий source, не обещается clean worktree.
- SHA-256 `src/client/c-tables.c`: `94cc27da7b272aa7d6967a39b1731cd94fe22547088c1f455c1f1a69cb4957f4`.
- Source initializer rows: 201; unique keys: 200; inventory missing: 0; inventory extra: 0; `#if 0` keys: 12.
- Disposition counts: G=63, P=89, L=16, D=1, I=12, U=19; sum=200.

Сверка выполнялась статическим извлечением initializer rows с сохранением line offsets и preprocessor branch context, dedup по `o_text`; множество имён независимо сравнено с published source inventory. Все 200 keys получили ровно один proposed disposition. Runtime/GUI tests и сборка не выполнялись: это planning draft, без реализации.
