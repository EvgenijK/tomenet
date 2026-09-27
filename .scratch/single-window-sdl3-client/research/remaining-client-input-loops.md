# Remaining client input loops

Статус: source audit, 2026-09-14. Это дополнение к [TomeNET single-window input loops](../../../docs/research/single-window-input-loops.md), а не runtime evidence или решение о новом UX. HEAD: `4211671279ff820575c32239272bca68cf8762f7`; исследован working tree. Общий dirty checkout не переключался; отдельная research branch не создавалась. SHA-256 источников и воспроизводимый site inventory ниже фиксируют фактически прочитанное состояние.

## Результат и граница атомарности

За пределами предыдущего file scope имеются самостоятельные selectors магии, двухшаговый account reorder, account character overview, runecraft composition, server-file perusal и first-run choices. Packet-driven prompts, Lua callbacks, shutdown prompts и platform save-chat handlers в основном делегируют уже перечисленным primitives. Их нужно добавить как **owners/entry points и intent contracts**, не копировать текстовый editor, direction selector или confirmation loop.

Далее `same` означает отсутствие `request_command()`/N-R translation; `inherited` — действующий `inkey()` macro pipeline с флагами вызывающего owner. Для прямого `get_com()` его primitive contract включает Esc=false и queue flush; отдельные owners ниже задают retry поверх него. `list` означает условные `Term_save`/`Term_load`, показ/скрытие списка и возврат в вызывающий surface; legacy clipping/icky flags не являются новой window topology.

## Самостоятельные contexts

| Context | Owner / entry | Keys / prompt | N/R; macro | Exit / retry | State | Intent | Focus / restoration | Gates | Source |
|---|---|---|---|---|---|---|---|---|---|
| `input.spell.legacy-select` | `get_spell`, вызывается ghost/legacy spell activation | `a..a+num-1` select; `A..` select **index+64**, не verification; Space/`*`/`?` toggle list; `(Spells/Powers…, *=List, ESC=exit)` | same; inherited | Esc=false; нет usable spells: sn=-2; canceled=-1; invalid bell/retry | realm/book/sval; sn; redraw | local result; `do_ghost` → `Send_ghost`; caller skill → `Send_activate_skill` | list restore + flush | ghost → REALM_GHOST; server spell_info availability | [c-spell.c:242](../../../src/client/c-spell.c#L242), [read:309](../../../src/client/c-spell.c#L309), [uppercase:352](../../../src/client/c-spell.c#L352), [ghost:857](../../../src/client/c-spell.c#L857) |
| `input.spell.mimic-select` | `get_mimic_spell` → `do_mimic` | lower letters index accessible powers; Space/`*`/`?` list; `@` → text primitive `Power?` exact case-insensitive available innate name | same; inherited, safe_macros checks captured parse_macro | Esc=false; name Esc exits whole selector; invalid bell/retry; safe_input+abort_prompt flush/break; final abort_prompt reset | corresp maps four fixed powers + RF4/5/6/0 available bits; sn; redraw | local result; skill caller below | list; clipping flags cleared | MIMIC_LUA only changes metadata, not selector vocabulary; always_show_lists; safe_macros | [c-spell.c:412](../../../src/client/c-spell.c#L412), [read:490](../../../src/client/c-spell.c#L490), [name:523](../../../src/client/c-spell.c#L523), [cleanup:582](../../../src/client/c-spell.c#L582) |
| `input.spell.mimic-immunity` | `do_mimic`, selected power=3 | `a..h` check/none/electricity/cold/fire/acid/poison/water; Space/`*`/`?` list; `@` → `Immunity?`, case-insensitive substring Ch/No/El/Co/Fi/Ac/Po/Wa, priority in source order | same; inherited | Esc/no c → return without packet; name Esc returns entire flow; unmatched bell/retry | c=1..8; redraw | `Send_activate_skill(MKEY_MIMICRY,0,25000,c,0,0)` | list + clipping reset; name-cancel early return skips later clipping reset | server newer than4.4.9.1.0.0 | [c-spell.c:722](../../../src/client/c-spell.c#L722), [read:747](../../../src/client/c-spell.c#L747), [name:780](../../../src/client/c-spell.c#L780), [intent:819](../../../src/client/c-spell.c#L819) |
| `input.spell.school-select` | `get_school_spell`; optional book item primitive first | lower letter selects; upper letter describes and remains; Space/`*`/`?` list; `…Descs A-…, …which spell?` | same; inherited | Esc=-1; invalid or !is_ok_spell bell/retry; forced spell bypasses read but still availability check | item; spell; where/redraw; hack_force_spell/level | result `spell+1000*level`, item_book; caller sends | list restore; no owner Flush_queue at final restore | SFLG1_LIMIT_SPELLS; DISCRETE_SPELL_SYSTEM name lookup; book payload/Lua | [c-spell.c:1058](../../../src/client/c-spell.c#L1058), [book:1087](../../../src/client/c-spell.c#L1087), [read:1152](../../../src/client/c-spell.c#L1152), [desc:1207](../../../src/client/c-spell.c#L1207), [exit:1254](../../../src/client/c-spell.c#L1254) |
| `input.spell.school-browse` | `browse_school_spell` | lower letter description; upper letter chat entry+description via Lua; Esc exit; no Space/`*`/`?` toggle (invalid) | same; inherited | invalid bell/retry; successful desc/paste remains | where/clipping; selected spell | local descriptions; uppercase Lua emits chat | unconditional save, Term_restore per key, load on exit; clipping reset | book data; USE_SOUND_2010 browse sound | [c-spell.c:1268](../../../src/client/c-spell.c#L1268), [read:1289](../../../src/client/c-spell.c#L1289), [paste:1321](../../../src/client/c-spell.c#L1321) |
| `input.spell.stance-select` | `get_combatstance` → `do_stance` | `a/b/c` balanced/defensive/offensive; `-` previous; Space/`*`/`?` list | same; inherited | Esc=false; invalid bell/retry | corresp=0/1/2/-1; redraw | `Send_activate_skill(MKEY_STANCE,stance,0,0,0,0)` | list; clipping reset | always_show_lists | [c-spell.c:1360](../../../src/client/c-spell.c#L1360), [read:1396](../../../src/client/c-spell.c#L1396), [intent:1471](../../../src/client/c-spell.c#L1471) |
| `input.spell.technique-select` | separate `get_melee_technique` and `get_ranged_technique`, equivalent context over different bitmask/name table | lower letters available bitmask; Space/`*`/`?` list; `@` text `Technique?`, default `Sprint` / `Flare Missile`, exact case-insensitive available name | same; inherited | Esc=false; name Esc exits entire flow; invalid bell/retry | corresp available bits; sn=-2 initial/-1 cancel; redraw/topline_icky | `Send_activate_skill(MKEY_MELEE/MKEY_RANGED,0,technique,0,0,0)` | list + final forced topline clear/icky reset; name Esc early returns before normal flag cleanup | available melee/ranged_techniques; always_show_lists | [melee:1499](../../../src/client/c-spell.c#L1499), [read:1549](../../../src/client/c-spell.c#L1549), [name:1582](../../../src/client/c-spell.c#L1582), [ranged:1679](../../../src/client/c-spell.c#L1679), [read:1726](../../../src/client/c-spell.c#L1726), [name:1760](../../../src/client/c-spell.c#L1760) |
| `input.spell.runecraft-compose` | `do_runecraft` | `a..a+rcraft_max(u)` choose available rune bit; Space/`*`/`?` list; Backspace undo; dynamic `rcraft_com(u)` prompt | same; inherited; get_com_bk | Esc return; Backspace at step0 return; invalid bell/retry; complete rcraft_end then optional direction primitive | u bitfield; u_prev[4]; step; list | `Send_activate_skill(MKEY_RCRAFT,low16(u),high16(u),dir,0,0)` | save/restore list per stage; final unconditional Term_load in source | Lua runecraft grammar; always_show_lists | [c-spell.c:1861](../../../src/client/c-spell.c#L1861), [read:1879](../../../src/client/c-spell.c#L1879), [back:1886](../../../src/client/c-spell.c#L1886), [direction/intent:1929](../../../src/client/c-spell.c#L1929), [runecraft.lua](../../../lib/scpt/runecraft.lua) |
| `input.spell.breath-preference` | `get_breath` → `do_pick_breath` | `a..g` check/none/lightning/frost/fire/acid/poison; TRAIT_POWER additionally h..m confusion/inertia/sound/shards/chaos/disenchantment; Space/`*`/`?` list; `@` → `Element?` substrings Ch-except-Cha/No/Li/Fr/Fi/Ac/Po/Co/In/So/Sh/Cha/Di | same; inherited | outer Esc=false; invalid bell/retry; **name Esc restores if shown then continues**, does not exit; name-match path accepts i7..12 without num check | num7 or13; corresp; redraw; br | `Send_activate_skill(MKEY_PICK_BREATH,br,0,0,0,0)` | list; clipping reset; child cancel leaves redraw true after load (source imbalance possibility) | caller only TRAIT_MULTI/POWER; power lineage described hypothetical by source | [c-spell.c:1987](../../../src/client/c-spell.c#L1987), [read:2023](../../../src/client/c-spell.c#L2023), [name:2053](../../../src/client/c-spell.c#L2053), [caller:2116](../../../src/client/c-spell.c#L2116) |
| `input.account.character-overview` | `Receive_login` | runtime `a..` existing; `N` new; `E` slot-exclusive new; `S/I/A` swap/insert-before/append-after; `Q`/Ctrl-Q quit; Ctrl-T screenshot | same; inherited | invalid silent retry; name Esc → overview; reorder Esc → overview; successful reorder → re-read server characters | ch; new_ok/exclusive_ok/firstrun; names/modes/counts; reincarnate_previous | existing/new character selection consumed by login; reorder packet below | replace screen; hide cursor; clears before return | server max_cpa/dedicated slots; allow_reordering; RETRY_LOGIN; E forbidden firstrun | [nclient.c:762](../../../src/client/nclient.c#L762), [read:1134](../../../src/client/nclient.c#L1134), [reorder:1151](../../../src/client/nclient.c#L1151), [name:1189](../../../src/client/nclient.c#L1189) |
| `input.account.character-reorder` | `reorder_characters`, two separately owned stages in one two-step context | first slot a..charcount; second slot a..charcount; mode-specific prompts swap/before/after | same; inherited | Esc cancels either stage; invalid silent retry; second valid sends and returns | sortA then sortB; mode1/2/3 | `PKT_LOGIN`, string `***<sortA><sortB><mode>`; Net_flush; caller waits refreshed overview | inherits overview; prompt and first selection erased on cancel | allow_reordering in caller | [nclient.c:701](../../../src/client/nclient.c#L701), [first:715](../../../src/client/nclient.c#L715), [second:731](../../../src/client/nclient.c#L731), [packet:745](../../../src/client/nclient.c#L745) |
| `input.server-file.peruse` | `peruse_file` | exact transitions next section | same; inherited, inkey_interact_macros=false; nested search sets inkey_msg=true | Esc/Ctrl-Q exit; `?` HELP exits then guide; player-list inkey -1 redraw/re-request; invalid key no-op/re-request | cur_line/col; srcstr; searching/reverse/regexp; max_line/page_size; perusing | `Send_special_line(type,line,search)`; exit `SPECIAL_FILE_NONE`; chat/inscription delegates | save/load; clipping managed received lines; restore macro flag+Flush_queue on exit | REGEX_SEARCH and server>=4.9.0; SPECIAL_FILE_PLAYER auto-refresh; USE_SOUND_2010 | [c-files.c:1953](../../../src/client/c-files.c#L1953), [requests:1993](../../../src/client/c-files.c#L1993), [reads:2042](../../../src/client/c-files.c#L2042), [exit:2244](../../../src/client/c-files.c#L2244) |
| `input.startup.bigmap-choice` | `ask_for_bigmap_generic` | `y/Y` enable; `n/N` decline; explicit first-run double-window-size prompt | same; inherited | every other key including Esc silently retries | ok; decline clears global_c_cfg_big_map | local decision; persists hint-cleared config/INI | replace; clear screen before/after | GLOBAL_BIG_MAP; WINDOWS vs POSIX persistence | [c-init.c:4608](../../../src/client/c-init.c#L4608), [read:4620](../../../src/client/c-init.c#L4620), [persist:4638](../../../src/client/c-init.c#L4638) |
| `input.startup.graphics-choice` | `ask_for_graphics_generic`, supported server and UG_NONE | `y/Y` start graphics now; `n/N` continue ASCII | same; inherited | other incl Esc retry; y execv replaces process; execv failure displays error and exits loop | ask_for_graphics=false on return; password temporarily unfrobnicated | execv argv0 `-G -l<account> <pass> <server>` | replace; clear at end | USE_GRAPHICS; server >=4.8.1; WINDOWS executable name | [c-init.c:4645](../../../src/client/c-init.c#L4645), [read:4671](../../../src/client/c-init.c#L4671), [restart:4679](../../../src/client/c-init.c#L4679) |

## Точные transitions server-file perusal

В [c-files.c:2052–2243](../../../src/client/c-files.c#L2052): `#` → text primitive `Goto Line(max…)`, length10, default0; `s` → `Search for text`, length60, previous srcstr; `r` → `Search for regexp`, length60, server-gated; cancel returns browser. `d` next match, `D` previous (prefix byte251 sent once). `-` back10; `b/p/Ctrl-U` back page; LF/CR/`j/2` +1 line; `k/8/Backspace` -1; Space/Ctrl-D next page; `g` start; `G` last page; `4/</h` horizontal -40 bounded0; `6/>/l` +40 bounded ONAME_LEN. Beginning/end pause and wrap rules use max_line/special_page_size. Ctrl-T screenshot (item details first page uses short capture); `:` chat; Ctrl-C/Ctrl-X toggle music, Ctrl-N/Ctrl-V master under sound; `{`/`}` inscription delegates. `!` unique-monster title searches `strongest unique monster`; `?` only SPECIAL_FILE_HELP opens guide after exiting browser. The `#if0` item-details `?` guide lookup is inactive. REGEX `r` writes regexp=true even after canceled text primitive; no replacement behavior is inferred.


Runecraft shipped grammar is exact: first and second rune stages `a..f` = Light/Darkness/Nexus/Nether/Chaos/Mana, choosing bit `1<<i` then `1<<(i+8)`; mode stage `a..h` = minimized/lengthened/compressed/moderate/enhanced/expanded/brief/maximized (`1<<(i+16)`); type `a..g` = bolt/cloud/ball/storm/cone/surge/flare (`1<<(i+24)`). Stage is determined by R1/R2/MODE/TYPE masks. Both rune stages use `Which rune?`; mode/type use their own prompt above. Type bit completes composition; direction is required except STRM or SURG (also their enhanced forms). No native availability gate rejects a letter based on player skill here; data computes display/cost/failure. [runecraft.lua:224](../../../lib/scpt/runecraft.lua#L224), [tables:49](../../../lib/scpt/runecraft.lua#L49), [bit/dir:419](../../../lib/scpt/runecraft.lua#L419).

## Delegated owners: новые entry points, существующие primitives

| Owner / sites | Delegate context; keys / prompt | Exit / retry | State / intent | Focus | Gates / source |
|---|---|---|---|---|---|
| `show_browse`:403; `show_motd`:1937; `display_message`:3337; `Receive_pause`:6208 | one-key acknowledgement via core read; any logical key | browse/display/pause one read; motd loops while key0; display skipped quit_no_prompt | browse local; motd timed delay; pause flushes physical queue **before** read and Flush_queue after | browse/motd/display save/load; pause existing surface | [c-spell.c:386](../../../src/client/c-spell.c#L386), [c-files.c:1910](../../../src/client/c-files.c#L1910), [c-init.c:3268](../../../src/client/c-init.c#L3268), [nclient.c:6193](../../../src/client/nclient.c#L6193) |
| `ask_for_graphics_generic`:4661/4693 | same acknowledgement primitive; unsupported old server or no graphics | any key → clear/ask flag reset | local | startup replace | USE_GRAPHICS else / server<4.8.1; [c-init.c:4658](../../../src/client/c-init.c#L4658) |
| `do_mimic`:677 | `input.prompt.text`, length40, empty default; `Which form (name or number; 0 for player; -1 for previous) ?` | ignores get_string boolean; resulting empty returns; invalid name/number returns, no retry | strip leading@ for name; exact monster name; number0..2767 →20000+n; previous -1→32767; `Send_activate_skill(MKEY_MIMICRY,0,spell,0,0,0)` | topline | monster_list file for name; [c-spell.c:670](../../../src/client/c-spell.c#L670) |
| `get_mimic_spell`:609 | `input.prompt.confirm` default-no `Really change the form?` | no→false | only selected corresp index0; local | topline/list already restored | other_query_flag; [c-spell.c:607](../../../src/client/c-spell.c#L607) |
| `get_item_hook_find_spell`:891/936; `get_school_spell`:1087/1094 | text `Spell name?`, length79; existing item-select context with USE_EXTRA/USE_LIMIT, inventory+equip | cancel false/-1; old exact lookup; discrete supports partial multi-tier and strongest available duplicate by level | hack_force_spell and item; no packet itself | caller item/list | DISCRETE_SPELL_SYSTEM, ALLOW_DUPLICATE_NAMES, LIMIT; [c-spell.c:883](../../../src/client/c-spell.c#L883), [new:924](../../../src/client/c-spell.c#L924), [book:1085](../../../src/client/c-spell.c#L1085) |
| `do_mimic`:845; `do_runecraft`:1930; `do_breath`:2132 | `input.command.direction` | cancel returns no skill packet | uses_dir metadata / rcraft_dir gate / breath always; selected dir in typed skill intent | topline then parent | mimic server>4.4.5.10 and MIMIC_LUA metadata; [c-spell.c:825](../../../src/client/c-spell.c#L825), [breath:2129](../../../src/client/c-spell.c#L2129) |
| `Receive_login`:1189 | `input.prompt.text`, CNAME_LEN-1, ASKFOR_LIVETRIM/PLUSPREFIX; New name, optional reincarnation `+` | Esc overview; empty generates random name then remains name editor; nonempty accepts | + prefix reincarnate; trim/titlecase; creation mode E dedicated flags | overview row, no extra top-level modal | previous/commandline/account defaults; [nclient.c:1168](../../../src/client/nclient.c#L1168) |
| `Receive_item`:3881/3925/3928 | `input.command.item-select`; Which item; different item_tester hooks/name prompts | cancel returns1 **without Send_item**; busy queues packet | immediate `Send_item(item)`; equip-only RUNE_ENCHANT; other USE_EXTRA/inven/equip + optional subinven | existing topline/item list | (!screen_icky&&!topline_icky) OR (ENABLE_SUBINVEN and using_subinven_item!=-1); ENABLE_SUBINVEN/version packet shape; [nclient.c:3831](../../../src/client/nclient.c#L3831), [prompt:3881](../../../src/client/nclient.c#L3881), [send:3929](../../../src/client/nclient.c#L3929) |
| `Receive_spell_request` | school-select context | canceled return1 no Send_spell; busy queues | `Send_spell(item,spell)` | child list then original surface | !screen_icky&&!topline_icky; [nclient.c:3944](../../../src/client/nclient.c#L3944) |
| `Receive_direction`:4020 | direction primitive | cancel returns0 no reply; busy qbuf | PKT_DIRECTION byte dir | topline | !screen_icky&&!topline_icky&&!shopping; [nclient.c:4012](../../../src/client/nclient.c#L4012) |
| `Receive_sell`:5508; `Receive_pickup_check`:5963 | confirm default-no; donation/Accept price; runtime pickup prompt | declined no reply/command | sale →Send_store_confirm; no_verify_sell auto confirms; pickup →Send_stay | existing parent topline | store_num MATHOM; [nclient.c:5495](../../../src/client/nclient.c#L5495), [pickup:5955](../../../src/client/nclient.c#L5955) |
| `Receive_request_key`:7184 | one-key primitive runtime prompt/id | Esc → reply0, not byte27 | request_pending true during primitive; Send_request_key(id,key/0) | topline | [nclient.c:7176](../../../src/client/nclient.c#L7176) |
| `Receive_request_amt`:7197; `Receive_request_num`:7209 | quantity / bounded number primitives runtime prompt/max or predef/min/max | canceled value0 still replied | Send_request_amt/num(id,value), pending scoped around prompt | topline | [nclient.c:7190](../../../src/client/nclient.c#L7190), [number:7202](../../../src/client/nclient.c#L7202) |
| `Receive_request_str`:7221 | text primitive runtime prefilled buf, MAX_CHARS_WIDE-1 | Esc → string byte27; accepted empty distinct | Send_request_str(id,text), pending scoped | topline | [nclient.c:7214](../../../src/client/nclient.c#L7214) |
| `Receive_request_cfr`:7242 | get_check3 runtime prompt/default | strict/default semantics from primitive; result replied | Send_request_cfr(id,bool); request_abort sets flag only if pending | topline | server>4.5.6.0.0.1 includes default byte; [nclient.c:7227](../../../src/client/nclient.c#L7227), [abort:7248](../../../src/client/nclient.c#L7248); abort consumption remains c-util primitive owner |
| `quit_hook`:3446; X11 CheckEvent close:1571; Windows client-save path:5706 | ternary primitive `Save chat log/all messages?`, default-no → do_save_chat; filename text primitive3371 length79 | prompt gated history/save_chat/quit_no_prompt; filename **return boolean ignored**, therefore Esc does not veto file write | res1 chat/res2 all; writes user-dir file; no network command | shutdown/current surface | WINDOWS/X11; [c-init.c:3435](../../../src/client/c-init.c#L3435), [filename:3371](../../../src/client/c-init.c#L3371), [X11:1571](../../../src/client/main-x11.c#L1571), [Win:5706](../../../src/client/main-win.c#L5706) |

## Lua UI: reachability и API exposure

[c-script.c:348](../../../src/client/c-script.c#L348) открывает util/player/spells bindings и (если !NO_CLIENT_IOLIB) стандартный Lua I/O; [open_lua:428](../../../src/client/c-script.c#L428) загружает c-init.lua. [c-init.lua:52](../../../lib/scpt/c-init.lua#L52) загружает player/s_aux/spells/runecraft и таблицы. Это runtime templates/data, а не дополнительный `inkey` reader. Generated bindings могут быть ignored git files: scan обязан использовать `rg --files -uuu`.

| Owner | Primitive / keys | Exit/result/state/intent | Focus | Gate / source |
|---|---|---|---|---|
| `toluaI_util_get_check200/300` | get_check2/get_check3 confirmation | wrapper pushes bool to Lua; no separate retry owner or packet | caller | [w_util.c:403](../../../src/client/w_util.c#L403), [427](../../../src/client/w_util.c#L427), declarations [util.pre:60](../../../src/client/util.pre#L60) |
| `get_item_aux` and generated player wrappers | builds USE_EQUIP/INVEN/FLOOR mode; delegates c_get_item; other generated wrapper exposes c_get_item directly | bool plus cp returned; API exposure alone not a new user-visible state machine | caller item primitive | [lua_bind.c:357](../../../src/client/lua_bind.c#L357), [w_play.c:8708](../../../src/client/w_play.c#L8708), [8735](../../../src/client/w_play.c#L8735) |
| `pre_exec_spell_extra` → spell extra callback | confirmation primitive, concrete DIVINE extra asks `Cast on yourself?`, default-no | s_aux returns TRUE whenever extra callback exists, after assigning __pre_exec_extra=extra(); skills then assigns aux from that value (false numeric confirmation result becomes0). Only absent extra callback preserves prior aux (initialized0 or LIMIT_SPELLS-derived). Send_activate_skill still follows; callback false is not cast cancel | nested spell owner | [skills.c:1035](../../../src/client/skills.c#L1035), [s_aux.lua:1104](../../../lib/scpt/s_aux.lua#L1104), [s_divin.lua:12](../../../lib/scpt/s_divin.lua#L12) |
| `pre_exec_spell_item` | runtime get_item.prompt/mode/hook, get_item_aux | skills updates item_obj only true; canceled keeps -1; Send_activate_skill still follows | nested item primitive | [skills.c:1040](../../../src/client/skills.c#L1040), [s_aux.lua:1111](../../../lib/scpt/s_aux.lua#L1111) |
| `pre_exec_spell_dir`, `rcraft_*` | metadata controls native direction/composition contexts already above | direction false means no prompt; cancel real get_dir aborts native cast | native caller | [skills.c:1030](../../../src/client/skills.c#L1030), [s_aux.lua:1094](../../../lib/scpt/s_aux.lua#L1094) |
| `s_aux` compatibility fallback; m_aux power_get_check2 | older binding fallback calls get_check/power_get_check if absent; current generated get_check2 exists | current fallback not activated; m_aux not loaded by c-init path; commented mana warning not active | n/a | [s_aux.lua:16](../../../lib/scpt/s_aux.lua#L16), [m_aux.lua:8](../../../lib/scpt/m_aux.lua#L8) |

Lua `module.lua` numeric `read` calls belong module file parsing, not a keyboard UI. Arbitrary local exec Lua can invoke exposed prompts or I/O; static exhaustiveness enumerates API boundaries and shipped callbacks, not unknowable user script content. [lua_bind.c:194](../../../src/client/lua_bind.c#L194) `set_target` is inside `#if0`; it creates no active target input loop. Active target/look/friendly state machines remain predecessor `input.command.target`, owned c-cmd.c2040/2101/2109 and get_dir in c-util3672. Network Receive_target_info supplies display data and does not read a key.

## SDL2 audio alternate and platform input topology

`SOUND_SDL` [snd-sdl.c:4137](../../../src/client/snd-sdl.c#L4137)/[4770](../../../src/client/snd-sdl.c#L4770) are concrete alternate owners of predecessor SFX/music contexts, not excluded merely because SDL3 audio exists. Reads4317/5022; text volume4346/5184 and search4465/4488/5782/5805 delegate text editor. Fixed SFX keys: Esc restore mixer+stop+autosave; v volume; Ctrl-T screenshot; : chat; t/y/n toggle/set; CR preview (LF not accepted by SFX switch), shifted preview conditional; #/s/S// search/index; PgUp/9/p and PgDn/3/Space page; End/1/G, Home/7/g; Up/8, Down/2; Backspace previous entry; Ctrl-U/F mixer. Music adds +/- selected-event volume by10, c current, e/E event play, a/A/u/U play-all/shuffle, w/W/q/Q next/previous song/event, P pause, Left/4/Right/6 seek backward/forward MUSIC_SKIP seconds. -1 is synthetic redraw, not gesture. SDL2 path uses real mixer timing, JUKEBOX_INSTANT_PLAY stop/return decisions, event/sample arrays and SDL2 callbacks; retain owner-specific gates alongside shared key vocabulary. [snd-sdl.c:4320](../../../src/client/snd-sdl.c#L4320), [music switch:5045](../../../src/client/snd-sdl.c#L5045), [seek/navigation:5894](../../../src/client/snd-sdl.c#L5894). GCU cannot advertise Shift-Enter boost (ENABLE_SHIFT_SPECIALKEYS check explicitly excludes gcu).

`z-term.c:4069` is underlying queue/event-pump owner for predecessor core context. wait=true invokes TERM_XTRA_EVENT until queue nonempty; wait=false polls once; take=false peeks; optional Term_inkey_hook replaces queue path; recording appends consumed bytes. No additional modal vocabulary. [z-term.c:4069](../../../src/client/z-term.c#L4069).

Platform main-* event pumps translate physical events to Term_keypress/Term_xtra. Their multiple windows, per-window resize/decorations and OS focus raising are **excluded topology**, not new single-window modal contexts. Font/resource selection, file outcomes, platform menus and physical-to-logical/device semantics remain applicable behaviors recorded by input/macro and platform-delta tickets; OS GUI APIs are alternate presentation owners, not justification for dropping their outcomes. X11 and Windows save-chat close prompts above remain applicable behavior. Unsupported legacy AMI map display direct Term_inkey2694 is an any-key view acknowledgement with map cursor/save/restore; unsupported AMY graphics first-run direct Term_inkey909 takes y/Y graphics, every other key decline and saves environment var. These are explicitly excluded from supported-platform acceptance, not silently missed. [main-ami.c:2694](../../../src/client/main-ami.c#L2694), [main-amy.c:905](../../../src/client/main-amy.c#L905). VME fgets1107 and X11 font fgets5672/5738/5755, Win fgets5866 are file reads; no prompt contexts. `src/console` is a separate console binary, `src/server` and updater/world are outside client UI destination.

## Completeness method and limits

No runtime sessions, login, private credentials or packet replay were used. Source-visible availability is preserved as gates, not asserted as reachable on every build/server. Inactive `#if0` code is not a shipped binding. First-run bigmap/graphics, ignored cancellation, mismatched screen cleanup and Lua cancellation need interpretation in **Preserve input and macro semantics** / **Choose raster references and defect compatibility**; no automatic fixes or new behavior approvals are implied.

Reproduce inventory from repo root (ignore filtering must be disabled for generated wrappers):

```bash
rg --files -uuu src/client lib/scpt
rg -n -uuu '\b(inkey|inkey_combo|get_com|get_com_bk|get_check[23]?|get_3way|get_string|askfor_aux|c_get_item|get_item_aux|c_get_quantity|c_get_number|get_dir|Term_inkey|request_command)\s*\(' src/client lib/scpt
rg -n -uuu '\b(fgets|getchar|scanf|read|select|poll)\s*\(' src/client lib/scpt
rg -n -uuu 'Term_keypress|TERM_XTRA_EVENT|CheckEvent|GetMessage|PeekMessage|WaitNextEvent|XNextEvent|wgetch|SDL_.*Event|request_abort|pre_exec_spell_|rcraft_|pern_dofile' src/client lib/scpt
```

The lexical site ledger below includes comments/definitions/inactive alternatives rather than pretending grep is a parser. Every interactive consumer is accounted above or delegated predecessor; false positives include source comments, function definition/prototype, generated bindings and legacy topology. The second/third searches audit socket/file/event owners separately; non-keyboard fgets/select never create input contexts. Zero-hit remaining C files are listed to make missing-file coverage visible.

### Remaining C owner/site ledger

| Source | Lexical primitive sites (`line:callee`) | Classification |
|---|---|---|
| [a-mac-h.c](../../../src/client/a-mac-h.c) | — | no direct primitive read; data/render/adapter/file work |
| [c-files.c](../../../src/client/c-files.c) | 1937:inkey, 1968:inkey, 2042:inkey, 2048:inkey, 2057:askfor_aux, 2067:askfor_aux, 2084:askfor_aux | owners/delegates above |
| [c-init.c](../../../src/client/c-init.c) | 3225:request_command, 3239:request_command, 3337:inkey, 3371:get_string, 3446:get_3way, 4620:inkey, 4661:inkey, 4671:inkey, 4693:inkey | owners/delegates above |
| [c-script.c](../../../src/client/c-script.c) | — | no direct primitive read; data/render/adapter/file work |
| [c-spell.c](../../../src/client/c-spell.c) | 309:get_com, 403:inkey, 490:get_com, 523:get_string, 609:get_check2, 677:get_string, 747:get_com, 780:get_string, 845:get_dir, 891:get_string, 936:get_string, 1087:c_get_item, 1094:c_get_item, 1152:get_com, 1289:get_com, 1396:get_com, 1549:get_com, 1582:get_string, 1726:get_com, 1760:get_string, 1879:get_com_bk, 1930:get_dir, 2023:get_com, 2053:get_string, 2132:get_dir | owners/delegates above |
| [c-tables.c](../../../src/client/c-tables.c) | — | no direct primitive read; data/render/adapter/file work |
| [c-xtra1.c](../../../src/client/c-xtra1.c) | — | no direct primitive read; data/render/adapter/file work |
| [client.c](../../../src/client/client.c) | — | no direct primitive read; data/render/adapter/file work |
| [graphics_common.c](../../../src/client/graphics_common.c) | — | no direct primitive read; data/render/adapter/file work |
| [lua_bind.c](../../../src/client/lua_bind.c) | 357:get_item_aux, 364:c_get_item | owners/delegates above |
| [main-acn.c](../../../src/client/main-acn.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-ami.c](../../../src/client/main-ami.c) | 2694:Term_inkey | excluded topology / shared core / file reads |
| [main-amy.c](../../../src/client/main-amy.c) | 909:Term_inkey | excluded topology / shared core / file reads |
| [main-cap.c](../../../src/client/main-cap.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-emx.c](../../../src/client/main-emx.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-gcu.c](../../../src/client/main-gcu.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-ibm.c](../../../src/client/main-ibm.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-lsl.c](../../../src/client/main-lsl.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-mac.c](../../../src/client/main-mac.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-sla.c](../../../src/client/main-sla.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-vme.c](../../../src/client/main-vme.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-win.c](../../../src/client/main-win.c) | 5706:get_3way | applicable save-chat delegate above; remaining event/window topology or file reads |
| [main-x11.c](../../../src/client/main-x11.c) | 1571:get_3way | applicable save-chat delegate above; remaining event/window topology or file reads |
| [main-xaw.c](../../../src/client/main-xaw.c) | — | no direct primitive read; data/render/adapter/file work |
| [main-xxx.c](../../../src/client/main-xxx.c) | — | no direct primitive read; data/render/adapter/file work |
| [nclient.c](../../../src/client/nclient.c) | 715:inkey, 731:inkey, 1134:inkey, 1189:askfor_aux, 1584:inkey, 3881:c_get_item, 3925:c_get_item, 3928:c_get_item, 4020:get_dir, 5508:get_check2, 5882:inkey, 5963:get_check2, 6208:inkey, 7184:get_com, 7185:get_com, 7197:c_get_quantity, 7209:c_get_number, 7221:get_string, 7242:get_check3, 8592:Term_inkey | owners/delegates above |
| [snd-sdl.c](../../../src/client/snd-sdl.c) | 4317:inkey, 4338:c_get_quantity, 4346:askfor_aux, 4456:c_get_quantity, 4465:askfor_aux, 4488:askfor_aux, 5022:inkey, 5175:c_get_quantity, 5184:askfor_aux, 5775:c_get_quantity, 5782:askfor_aux, 5805:askfor_aux | owners/delegates above |
| [variable.c](../../../src/client/variable.c) | — | no direct primitive read; data/render/adapter/file work |
| [w_play.c](../../../src/client/w_play.c) | 8708:get_item_aux, 8735:c_get_item | owners/delegates above |
| [w_spells.c](../../../src/client/w_spells.c) | — | no direct primitive read; data/render/adapter/file work |
| [w_util.c](../../../src/client/w_util.c) | 403:get_check2, 427:get_check3 | owners/delegates above |
| [z-term.c](../../../src/client/z-term.c) | 3731:Term_inkey, 4069:Term_inkey | excluded topology / shared core / file reads |

### Source SHA-256

```text
f973c8019d179222a420adc83c3cef930a21c7790666fb7ebe93c860deec0f7e  src/client/a-mac-h.c
ccbd1dcfbc6d743419719264991dd62960670a2eb66e08ca6177dd1a03887968  src/client/c-files.c
51eded7b8d2ed221f9995bd777aa4f7a23af9e876296f28ee2db119d82a28672  src/client/c-init.c
83740e2fe5a36e340fbacabe258e5f9180035e84b4749f793044e5f12db831f7  src/client/c-script.c
35aff58dab60cdecc39002cee153fbf0e2b2829f0d226b1589a9094e7e0fab36  src/client/c-spell.c
94cc27da7b272aa7d6967a39b1731cd94fe22547088c1f455c1f1a69cb4957f4  src/client/c-tables.c
b954e894ed9dd87cff4b78ac5798347c27ca16e0c54595efb363d664787bfa68  src/client/c-xtra1.c
dcf87b7b2cfd6812a30e58cebd55527048da8e37eedfaa5637398c140dfb5891  src/client/client.c
a5220525239279e5e62cae1655270c2c03b8d6026c4a3a28d7c4eb4c274b20cf  src/client/graphics_common.c
05446ee0d663a5a60a53642c6528fc9b3967352717d19ca8bc21bf22bc62fb83  src/client/lua_bind.c
4408fd84a3f05d9e8c3047c8a6607276c21edc5a306006ec8e9e1f3c083dfe6a  src/client/main-acn.c
75a1d753ca455f880d375bdc7c2acea5e8b36ffabff3b20bbcaa287644be7a17  src/client/main-ami.c
831c1f6564606ac893eaeefddb53788c8028ead594f6cff53a390aab93cdbc34  src/client/main-amy.c
4492405c32db2295966250261ef0d373a444063c411d971f7dc992ba28018cc5  src/client/main-cap.c
5f4afdc536f206d0a391569f39425822ea4256a8414bf159ff529a26a9268d05  src/client/main-emx.c
444ea8214feba8273e1640eb55352977196fe0b89b06ed6d8ab55d28a2e3a97d  src/client/main-gcu.c
f219080cd599305ede00353b2b417117707d22a5a5bc36ff2cfc5ef00225e977  src/client/main-ibm.c
8ffa71cfae8eaad677daa72ef318ea3b0f48025e0e35d63333d845c6ac84d068  src/client/main-lsl.c
eca3609e411fe4883fb04245d6658d88ed63fae9b799ab4cb162a14aa91805e9  src/client/main-mac.c
3958cd135d1ec73eb53f174b471388bbea4ebea909c29fab3e47248e8df740ba  src/client/main-sla.c
e7c747ba5af9109bed9836029e099acfdcf0a08d8f277ddbfeb10300345f3634  src/client/main-vme.c
259b005a5f4409664885b5c1e44df9c83fc9ca5d2d18d6bff1b533bbb32d44cd  src/client/main-win.c
c1e7fbac381e506807ff6bf44bc8ee885dcd2eb1d529b811983880a6075fbe57  src/client/main-x11.c
cd4015c7730887d1cbeba0c9a5d9f85f3afcb8f48818fe505f6475929f9a5430  src/client/main-xaw.c
03fad0db675924c52a980952a72ad64c52cdc46d8ab7dd4fb64bf10eeb1a5e56  src/client/main-xxx.c
a0c6e85ec05944a1bc0e3115012a2a57b69f16f9c14e8a59bc5baabc0029ce60  src/client/nclient.c
3c1b4747b2e7843ed63cdb3ee790d694970fc88f08891d256a37056e670972be  src/client/snd-sdl.c
0da4064e96f1871859e05790c6f0b7aba1727266db901db48dfb9f12b2668e37  src/client/variable.c
95e9c348f8fbd763a76debfad1245ea0bbeb899f46e529d5295d0dfd2cfdedb2  src/client/w_play.c
e730021910ef33e415f7755313321343eb78b9d386b9fc7c3193f50910e68625  src/client/w_spells.c
f2ed9e1dc05806e4602e8682ab5b5984e62e4f33a79b5eb8b43f5ce2bb997bb0  src/client/w_util.c
3718be9fe2d59acc701b1c786bcff48d05b664436943b482033a37aa4f930b76  src/client/z-term.c
b5e50465e8694f1c691937af4bbae85b4c8136d6a69a1064ca5aa0fa764409cd  src/client/skills.c
a948ad57c78fbb22d04adddabd149cec85f311ead463ac17c30ee7b7c566f05c  src/client/c-util.c
927b26ac77a3e5dd683f9f77357b6443b0a888b11f3fa6cf7bf83f96065b7c4b  lib/scpt/c-init.lua
e3f0ec42a376653d551bdb5d82939e46cf57afabcbb29604a226f7abe7d7a333  lib/scpt/s_aux.lua
701aa99a5e266d2a5c378348436984d3dff0a08af281d9d00c027a492a129ebb  lib/scpt/s_divin.lua
799571aa32317d756a18e9c3d5574ae167148513b414d910e8b9747806d5b6d6  lib/scpt/runecraft.lua
7fcd573fb157553a4dfce7c9b6b4cd009cef19be02b466a08bf041c3d261172d  lib/scpt/m_aux.lua
```

### Lua lexical input-call ledger

Expanded alias/file scan adds `get_item`, `power_get_check` and `read` to the primitive regex; lexical matching deliberately includes comments.

| Source | Sites | Classification |
|---|---|---|
| [m_aux.lua](../../../lib/scpt/m_aux.lua) | 7:get_check2, 10:power_get_check, 321:get_check2 | compatibility fallback or commented warning; not client-init loaded |
| [module.lua](../../../lib/scpt/module.lua) | 94:read, 95:read, 109:read, 115:read, 116:read, 127:read, 128:read, 139:read, 140:read, 145:read, 146:read, 151:read, 152:read | module file numeric parsing, not keyboard |
| [s_aux.lua](../../../lib/scpt/s_aux.lua) | 15:get_check2, 17:get_check2, 18:get_check, 919:get_check2, 1125:get_item_aux | callback/delegate or inactive/fallback; see reachability section |
| [s_divin.lua](../../../lib/scpt/s_divin.lua) | 12:get_check2 | callback/delegate or inactive/fallback; see reachability section |
| [s_udun.lua](../../../lib/scpt/s_udun.lua) | 20:get_item | inactive --[[ block; Drain spell not registered, no client prompt |

Zero expanded lexical primitive/alias/file-read calls in other shipped scripts: `activations.lua`, `adventures.lua`, `attributes.lua`, `audio.lua`, `bpr.lua`, `c-init.lua`, `cblue.lua`, `classes.lua`, `custom.lua`, `d_astral.lua`, `dg.lua`, `dr_arcane.lua`, `dr_physical.lua`, `evil.lua`, `guide.lua`, `init.lua`, `it.lua`, `jir.lua`, `m_attunement.lua`, `m_mintrusion.lua`, `m_ppower.lua`, `meta.lua`, `mikaelh.lua`, `mimicry.lua`, `moltor.lua`, `o_hereticism.lua`, `o_shadow.lua`, `o_spirit.lua`, `o_unlife.lua`, `p_curing.lua`, `p_defense.lua`, `p_offense.lua`, `p_support.lua`, `player-info.lua`, `player.lua`, `powers.lua`, `quests.lua`, `races.lua`, `runecraft.lua`, `s_air.lua`, `s_convey.lua`, `s_earth.lua`, `s_fire.lua`, `s_mana.lua`, `s_meta.lua`, `s_mind.lua`, `s_nature.lua`, `s_tempo.lua`, `s_water.lua`, `spells.lua`, `test.lua`, `traits.lua`, `update.lua`, `xml.lua`.

Lua scan digests:

```text
4bc09e56219fbe40ecf28b9583f37bcb1ca64c5c56424cdd4c145f818839f6aa  lib/scpt/activations.lua
e69a806ea64190d560cb6ffc0a7be6f41694d48b57d5af8b68d98de4d45327fe  lib/scpt/adventures.lua
0b8e985bcd9030eaf982c83d9408e921160002ead7173bfcc37363fdc8131c1e  lib/scpt/attributes.lua
479a61f347783f715a6814133e3a1a7cef56a0593add340751cbb463a52206b3  lib/scpt/audio.lua
e7b40700022557c6a41ccd22a5202c07f69b9d36a7784ffee16a7b5d3119a6cb  lib/scpt/bpr.lua
927b26ac77a3e5dd683f9f77357b6443b0a888b11f3fa6cf7bf83f96065b7c4b  lib/scpt/c-init.lua
56bff263e6748bd32bd2969d2fb9a513011fb4f2c392e14d37aaeabcd62cc448  lib/scpt/cblue.lua
e63ca28838e7cae94db8c1cf540c185e67c6349fa5af155cc46aa4cebf73959d  lib/scpt/classes.lua
9e7dc8ad7a491e71e21c7f637575390c08d364d2740b6c13e5a38615800e4d9f  lib/scpt/custom.lua
2d446ed0a8502b58dc4a2a5d03a231463a3d647d944b762b28a4014b81101fc0  lib/scpt/d_astral.lua
2de93f62560be9ba60c7484c87445b26d3b9949ae011f68eb89886fa0066aefd  lib/scpt/dg.lua
f76c6e6d2ed698c653af8321d68a93ba1aa4b9c78680403b028208134b810d6e  lib/scpt/dr_arcane.lua
7481609bcf1d8d1e061c3af778e24cda2f37fbeca8747eabeb3ae36207cab2d0  lib/scpt/dr_physical.lua
80a411fb1f443c4ac71062af54797c22b7e95e25fec7d195f5bb45a3272d7df4  lib/scpt/evil.lua
1e93692b77d675deb7378a5357cecca2d1cd58a5b1e6301fe57d70090c5a72fb  lib/scpt/guide.lua
8ac1058ac763f2586486c127e08f46d0fce8930e30f2bab5c9cb3246d38e126e  lib/scpt/init.lua
7ab7d786a504304be283e3602271c6a4c718c8fe54dfad8fcb41b2070e051097  lib/scpt/it.lua
53df30933228dd53635e9275caf4adf924c40c2d18f02e1acf5ecd0e93423c6a  lib/scpt/jir.lua
d0a89172e8ac392ad3eb9da80af5c71da7e97c399e8d4b6ff0f117d790fc1605  lib/scpt/m_attunement.lua
7fcd573fb157553a4dfce7c9b6b4cd009cef19be02b466a08bf041c3d261172d  lib/scpt/m_aux.lua
3be91ab397a1116ff3b65124c34ac74bda6d374d402d351b734aca4fae84afb1  lib/scpt/m_mintrusion.lua
86d885bcdf5c57cd64f45454cbf73b6b72ef09d15618b124089a7c2db1caa279  lib/scpt/m_ppower.lua
c239bce478870f06338e5f7ce36ca0e4b74b50550351d9079cfe59e52ebc6e19  lib/scpt/meta.lua
9819c87d1dd12d7f837456c9e49932cd02f1bd7069a2ce1a687f2f7703ae8dcd  lib/scpt/mikaelh.lua
9113d230c2be9a6494f2cf13f1716c52818ed0e3609e556a4f1f4986043a8c16  lib/scpt/mimicry.lua
70c6fe1f1476ed4e01f3b0b77e27e4a78be44282816f769ee5ffee2733e8e43d  lib/scpt/module.lua
9bc6bf85b60cae9ac882af9a7a562462876730528b3061d609cf9c4ad9dd1e48  lib/scpt/moltor.lua
7d421a1e74d6752abdc816d67e0f357c09063affbbfb9c5c44824542b657da5e  lib/scpt/o_hereticism.lua
f56a24883ef1d5fadc20635b4c5f24b85013c4d60898b4a48aedebac32033da2  lib/scpt/o_shadow.lua
ec65152e2a03a2f6424d6a111602fd8583166dd31a82837915a6c3651d1b8c4f  lib/scpt/o_spirit.lua
c96dc5c0c53a2933553b03cb235161e3a0edfac075e101d64e4597ed73706f7f  lib/scpt/o_unlife.lua
00528f564b560a8af0938290e9e8e0964c9aac7f54e14aa998c510eb09af76b8  lib/scpt/p_curing.lua
bebeb14536c804f27ff67f5136c6e4632dd942381d941eaf84d951612cd769e3  lib/scpt/p_defense.lua
90c29156871b55582d45297ce7df741aa576cee32f181dd2943699e9473406a1  lib/scpt/p_offense.lua
050499a86b3ef0525c1c265e317af7c6c4d4d1deb7a14c4002c25fc79a55f4e3  lib/scpt/p_support.lua
0c6f8d8da937ec2def95acba3876a2c537620e6744b14396020ee5f44f60dc31  lib/scpt/player-info.lua
de73478f6609e97ae6b00bb94d2c16b74e47452da8acc93b18fb3edd684561c9  lib/scpt/player.lua
57742ece3ac9eb8b230949a7ab2dc2bd7c4f2bfaee703ea31b236e7fc23fa4b4  lib/scpt/powers.lua
9b8bfc2038b41e40c61574abbac411fc6bb53cfbecd3849473ccb8f461989776  lib/scpt/quests.lua
46f2517d8ce54e5027bea82150e69b01a75cb52f3ae78b63fe5908316b163a7d  lib/scpt/races.lua
799571aa32317d756a18e9c3d5574ae167148513b414d910e8b9747806d5b6d6  lib/scpt/runecraft.lua
30eeb5ba8f7d7ab3e560c1bb7df25d1d41b472be5237a925b42c254f4714ba6e  lib/scpt/s_air.lua
e3f0ec42a376653d551bdb5d82939e46cf57afabcbb29604a226f7abe7d7a333  lib/scpt/s_aux.lua
ab7674ae02bab0e6018aeacb3f2a1110bc194e4d2356ab1c9ba576194a51926c  lib/scpt/s_convey.lua
701aa99a5e266d2a5c378348436984d3dff0a08af281d9d00c027a492a129ebb  lib/scpt/s_divin.lua
174ada69b7d80398441b2a5bd83b95054490d394bd936c03fb0d69a1c158d184  lib/scpt/s_earth.lua
23de0510d389706ecf99bcf366ffa7c30c02ad8fa618989f6c5630fba3bd8101  lib/scpt/s_fire.lua
eeb5c98829dc1ab9c8c5292908a4c38fb2d7180ec471fd7eb091673ffd49b4f9  lib/scpt/s_mana.lua
c1a017beb99348d7f423eab7116ea6389c7f03c1981f95f629af55c9c8151741  lib/scpt/s_meta.lua
67c55bd93ebaa17e388bca18c97e6fd8a7047c49fc4011cec9bc7cf2b42dc69d  lib/scpt/s_mind.lua
fbbafdd4dd064aa3945767c27b0d6bf52feec16c50565d16413b27e127f391de  lib/scpt/s_nature.lua
5c251b3ed9feb2af87314aedeb2bc65dc2b582d808aa5c9d961b0389f62ad240  lib/scpt/s_tempo.lua
bf90bbc70f6fb1c8c7773ea2425c703c7fc05e519971cbd5fc75957c11705fec  lib/scpt/s_udun.lua
7479099660f7ab4d8790393a56bbe7a44728bf751e2b818f9b2b0c9b6f0f040d  lib/scpt/s_water.lua
6b19e104850b6db9e26a34060d48557d00ffdbc730280834b797649f0f030cd1  lib/scpt/spells.lua
a9993f43c1a2ff0475f7c2dc1aa3ac266cdc057070f7738844c2c89e1eaf7af3  lib/scpt/test.lua
da7f36a6d6a1bfbdabf128808c2553b75fe719dce33aba8e97352d1743b6a1ad  lib/scpt/traits.lua
eacc43ceba3542c7a2ca92c6effe0db333016dc86eee88068bc7ed3cf06d438c  lib/scpt/update.lua
047b03cc81f2cb117ca3e22b487fdba270ccc0fa701c9163d2b2e6554442406d  lib/scpt/xml.lua
```
