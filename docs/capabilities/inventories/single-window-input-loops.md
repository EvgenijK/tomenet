# TomeNET single-window input loops

Status: source audit, 2026-09-11. Scope is the current checkout at `4211671279ff820575c32239272bca68cf8762f7` plus its uncommitted SDL3 work. This is a planning input, not a new behavior specification.

## Result

The legacy client does not have one input loop. It has a four-stage input pipeline feeding 52 reusable input contexts in the inventory below, implemented by a larger number of direct C owners. Two C owners count as one context only when they consume the same key vocabulary, mutate the same-shaped local state, and have the same exit/return contract; the per-function ownership table below preserves the implementation boundaries instead of hiding them. The context count is therefore intentionally smaller than the function-level owner count: wrapper/delegate prompts and compile-time duplicate implementations do not create a new user-visible input state machine.

1. SDL3 turns physical keys and modifiers into bytes or navigation-key sequences and enqueues them with `Term_keypress()` ([`src/client/main-sdl3.c:1091`](../../src/client/main-sdl3.c#L1091), [`src/client/main-sdl3.c:1215`](../../src/client/main-sdl3.c#L1215)).
2. `inkey_aux()` recognizes normal, command, and hybrid macros and pushes the selected action back onto the terminal queue; `inkey()` strips protocol delimiters, restores the active term/cursor, and returns one logical key ([`src/client/c-util.c:1160`](../../src/client/c-util.c#L1160), [`src/client/c-util.c:1262`](../../src/client/c-util.c#L1262), [`src/client/c-util.c:1323`](../../src/client/c-util.c#L1323), [`src/client/c-util.c:1705`](../../src/client/c-util.c#L1705)).
3. Only `request_command()` maps that logical key through the normal/roguelike keymap. Most modal loops consume `inkey()` directly and therefore implement their own key aliases ([`src/client/c-util.c:3583`](../../src/client/c-util.c#L3583), [`src/client/c-util.c:3647`](../../src/client/c-util.c#L3647)).
4. `process_command()` interprets the canonical command locally, opens another loop, sends a typed `Send_*` request, or falls through to `Send_raw_key()` ([`src/client/c-cmd.c:274`](../../src/client/c-cmd.c#L274), [`src/client/c-cmd.c:10585`](../../src/client/c-cmd.c#L10585)).

For a single-window client, therefore, input context must be explicit. A binding cannot be modeled as `key -> action` alone; its minimum identity is `(physical gesture, macro policy, keyset policy, owning loop) -> local transition or emitted intent`.

## Reading the tables

- `N/R` means normal/roguelike keyset. `same` means the modal loop reads raw logical keys and does not call the keyset translator.
- `M` records macro acceptance: `all`, `command-only`, `no command`, or `suppressed`. Hybrid macros are also suppressed wherever `inkey_msg` is true. `safe` means a failed macro is flushed when `safe_macros` is enabled.
- `Exit` states cancel, confirm, and retry behavior. Invalid input generally rings `bell()` and remains in the same loop.
- `Focus` describes legacy terminal ownership. `save/load` means the previous screen is restored; `topline` means only row zero is restored/cleared; `replace` means the flow owns the whole screen until its parent redraws.
- `Intent` is `local` unless a concrete network command is named. A slash/chat message ultimately uses `Send_msg()` through `cmd_message()`.

## Atomic loop inventory

Each row is one independently owned blocking input context or one reusable prompt primitive. Comma-separated keys are aliases within that context; semicolon-separated clauses are distinct transitions.

| ID | Owner / entry | Keys and prompts | N/R | M | Exit / retry | Local state transition | Intent | Focus | Gates | Source |
|---|---|---|---|---|---|---|---|---|---|---|
| `input.core.event` | SDL3 `key_press()` | printable text; F1-F12; arrows/Home/End/PgUp/PgDn/Insert; keypad; Ctrl/Shift/Alt | physical | before macros | key-up is irrelevant to logical loop | modifier bits become `inkey_shift_special`; special keys become `NAVI_KEY_*` sequences when enabled | enqueue bytes | current `term` | `USE_SDL3`; `ALLOW_NAVI_KEYS_IN_PROMPT`; `ENABLE_SHIFT_SPECIALKEYS`; `SDL3_STICKY_KEYS` | [`main-sdl3.c:1091`](../../src/client/main-sdl3.c#L1091), [`main-sdl3.c:1162`](../../src/client/main-sdl3.c#L1162), [`main-sdl3.c:1215`](../../src/client/main-sdl3.c#L1215) |
| `input.core.macro` | `inkey_aux()` / `inkey()` | longest matching trigger; backquote becomes Esc after matching; bytes 28/29/30/31 delimit fallback/action/special sequences | before keymap | policy from flags | unmatched trigger bytes are pushed back; multi-key wait tops out after increasing 10-unit delays; macro completion byte 29 ends `parse_macro` | `parse_macro`, `after_macro`, `parse_under`, `parse_slash`, `strip_chars`, `macro_missing_item` | none | activates main term, then restores old term and cursor | `ALLOW_NAVI_KEYS_IN_PROMPT`; `SOME_NAVI_KEYS_DISABLE_MACROS_IN_PROMPTS` | [`c-util.c:1160`](../../src/client/c-util.c#L1160), [`c-util.c:1262`](../../src/client/c-util.c#L1262), [`c-util.c:1376`](../../src/client/c-util.c#L1376), [`c-util.c:1618`](../../src/client/c-util.c#L1618) |
| `input.core.macro-wait` | macro `\\wDD` -> `sync_sleep()` | byte 96 plus exactly two decimal digits; waits `DD * 100ms` while pumping network/timers/redraw | before keymap | only inside expanded macro | completes on duration, nested input semaphore, or server `PKT_CONFIRM`; interactive cancellation is compiled out | `command_confirmed=-1`, `inkey_sleep=true`; reset sleep/semaphore at completion | keepalive/ping/network processing | spinner in last topline cell, then erase | `ACCEPT_KEYS` disabled by source; Windows timer vs `gettimeofday` | [`c-util.c:156`](../../src/client/c-util.c#L156), [`c-util.c:417`](../../src/client/c-util.c#L417), [`c-util.c:494`](../../src/client/c-util.c#L494), [`c-util.c:939`](../../src/client/c-util.c#L939) |
| `input.core.macro-xwait` | macro `\\WDDDD` encoding -> byte 30 -> `sync_xsleep()` | byte 30 plus exactly four decimal digits; waits `DDDD * 100ms`; fresh Esc aborts, Space resumes early, other fresh keys are preserved in old queue | before keymap | only inside expanded macro | duration/semaphore/confirm completes; Esc discards temporary queue; Space restores old queue | swaps `Term->keys` with temporary queue; same sleep flags | keepalive/ping/network processing | spinner in last topline cell, then erase | Windows timer vs `gettimeofday`; byte 30 legacy control-caret collision noted in source | [`c-util.c:157`](../../src/client/c-util.c#L157), [`c-util.c:568`](../../src/client/c-util.c#L568), [`c-util.c:651`](../../src/client/c-util.c#L651), [`c-util.c:1036`](../../src/client/c-util.c#L1036) |
| `input.core.missing-item` | `inkey_aux()` + `c_get_item()` + `bell*()` | state 1 drops an `@` and enters state 2, or drops one numeric tag; state 2 drops name bytes through CR; state 3 drops one numeric inscription byte | before keymap | expanded macro only | successful/consumed branch returns state 0; with `safe_macros`, a failed item loop sees `abort_prompt`, sets `command_gap=50`, flushes and exits | `macro_missing_item: 0 -> 1/3 -> 2/0`; `abort_prompt` is set only when a bell occurs during `parse_macro` | none | failed topline prompt is force-cleared; prompt can be restored at macro end when configured | `safe_macros`, `keep_topline` | [`c-inven.c:1325`](../../src/client/c-inven.c#L1325), [`c-inven.c:1420`](../../src/client/c-inven.c#L1420), [`c-util.c:1154`](../../src/client/c-util.c#L1154), [`c-util.c:1951`](../../src/client/c-util.c#L1951) |
| `input.core.prompt-navigation` | `inkey_combo()` / `scan_navi_key()` | arrow/Home/End/PgUp/PgDn/Delete become `NAVI_KEY_*`; optional edit modifier preserves legacy default text behavior | same | navigation may bypass macro matching | helper toggles `inkey_location_keys` only for the read, then clears it | cursor/key result | none | caller prompt | `ALLOW_NAVI_KEYS_IN_PROMPT`; optional `SOME_NAVI_KEYS_DISABLE_MACROS_IN_PROMPTS` makes `inkey_aux()` return decoded navigation before macro lookup | [`c-util.c:732`](../../src/client/c-util.c#L732), [`c-util.c:855`](../../src/client/c-util.c#L855), [`c-util.c:1241`](../../src/client/c-util.c#L1241) |
| `input.prompt.text` | `askfor_aux()` | Enter accept; Esc cancel; Backspace/Delete; arrows/Home/End; Ctrl-A/E/B/F; history up/down; Ctrl-R search, Ctrl-G cancel search; Ctrl-K copy; Ctrl-L paste; printable insertion | same | hybrid normally caller-suppressed | Esc false; Enter true, including empty; invalid/full input bells and retries | editable buffer, cursor, history cursor, search result; private mode masks output and disables initial edit | none | topline/caller cursor | `ALLOW_NAVI_KEYS_IN_PROMPT`; clipboard hooks; `ASKFOR_PRIVATE`, `ASKFOR_CHATTING`, `ASKFOR_LIVETRIM` | [`c-util.c:2613`](../../src/client/c-util.c#L2613), [`c-util.c:2699`](../../src/client/c-util.c#L2699), [`c-util.c:2720`](../../src/client/c-util.c#L2720), [`c-util.c:3385`](../../src/client/c-util.c#L3385) |
| `input.prompt.command` | `get_com()` / `get_com_bk()` | one logical key | same | inherited | Esc false; `_bk`: Backspace returns -2; every other key true | none | none | topline; flush queue after | none | [`c-util.c:3516`](../../src/client/c-util.c#L3516), [`c-util.c:3542`](../../src/client/c-util.c#L3542) |
| `input.prompt.confirm` | `get_check2/3()`, `get_3way()` | y/n; `get_3way`: y/a/n | same | inherited | `get_check2`: any key takes displayed default except explicit opposite; `get_check3(0)` retries until y/n; `get_3way` retries until y/a/n and, with default-no, Esc/Enter/Ctrl-Q mean no | none | none | topline; flush queue | none | [`c-util.c:3765`](../../src/client/c-util.c#L3765), [`c-util.c:3816`](../../src/client/c-util.c#L3816), [`c-util.c:3858`](../../src/client/c-util.c#L3858) |
| `input.prompt.quantity` | `c_get_quantity()`, `c_get_number()` | text number; `a/A/Space` can mean all through `inkey_letter_all` | same | caller policy | Esc/empty -> 0; clamps or rejects to caller range | numeric value | none | topline | none | [`c-util.c:5058`](../../src/client/c-util.c#L5058), [`c-util.c:5159`](../../src/client/c-util.c#L5159) |
| `input.command.gameplay` | `request_command()` -> `process_command()` | next available key; `\\` bypasses keymap; `^` reads a second key and makes Control | translated | command-only (`inkey_flag`) | scan mode returns immediately if empty; Esc/no mapping no-op | sets `command_cmd`, `command_dir`; clears topline | see command table | current game surface | none | [`c-util.c:3583`](../../src/client/c-util.c#L3583), [`c-cmd.c:274`](../../src/client/c-cmd.c#L274) |
| `input.command.direction` | `get_dir()` | direction; `*` open targeter; `-` acquired target; `+` acquired/manual | keymap direction | inherited | no valid direction false; target cancel false; otherwise confirm | returns 1..11 | nested target may emit target state; caller emits action | topline | none | [`c-util.c:3672`](../../src/client/c-util.c#L3672) |
| `input.command.minimap` | `cmd_mini_map()` | direction pan; Space/Esc/normal `L` or rogue `W` exit; `:` chat; Ctrl-T screenshot | directions translated locally | hybrid suppressed around chat | exit returns; invalid retries | viewport offsets | `Send_special_line()` supplies/refreshes map | save/load | map/server feature branches | [`c-cmd.c:555`](../../src/client/c-cmd.c#L555), [`c-cmd.c:638`](../../src/client/c-cmd.c#L638), [`c-cmd.c:788`](../../src/client/c-cmd.c#L788) |
| `input.command.inventory-view` | `cmd_inven()`, `cmd_subinven()`, `cmd_equip()` | item letter examine; x examine; d drop; normal k / rogue Ctrl-D destroy; `{`/`}` inscription; `:` chat; inventory additionally b/s; subinventory a/S; equipment t take off | explicit N/R aliases | inherited | Esc leaves child selectors; loop itself is one-key action then restore | selected item/subinventory, optional list switch | typed item commands | save/load | `ENABLE_SUBINVEN` | [`c-cmd.c:965`](../../src/client/c-cmd.c#L965), [`c-cmd.c:1119`](../../src/client/c-cmd.c#L1119), [`c-cmd.c:1297`](../../src/client/c-cmd.c#L1297) |
| `input.command.item-select` | `c_get_item()` | letter select; uppercase verify/alternate; digits inscription tag; Enter sole item; `@` name; `* ? Space` list; `/` inven/equip; `!` bag; `#` spell limit; `-` special return; `+` newest | same | command exception; safe failure flush | Esc false; bad key/item retry; uppercase rejected confirmation ends false; safe failed macro aborts | `command_wrk`, `command_see`, `using_subinven`; returns item, negative alternate, -3 special | none; caller emits | save/load if list shown, always redraw inventory/equipment | `ENABLE_SUBINVEN`; `ITEM_PROMPT_ALLOWS_SWITCHING_TO_SUBINVEN`; `CAPS_ALT`/mode flags | [`c-inven.c:1092`](../../src/client/c-inven.c#L1092), [`c-inven.c:1420`](../../src/client/c-inven.c#L1420), [`c-inven.c:1629`](../../src/client/c-inven.c#L1629), [`c-inven.c:2032`](../../src/client/c-inven.c#L2032) |
| `input.command.target` | `cmd_target()`, `cmd_target_friendly()`, `cmd_look()` | directions move; t/5 choose; p player; q/Esc cancel; look adds x/l modes, `:` chat, Ctrl-T | directions via keymap | hybrid suppressed in look | cancel returns 0; invalid retries | cursor, target mode, selected grid/entity | target packets through target helpers | screen redraw/return | version/entity availability | [`c-cmd.c:2040`](../../src/client/c-cmd.c#L2040), [`c-cmd.c:2101`](../../src/client/c-cmd.c#L2101), [`c-cmd.c:2109`](../../src/client/c-cmd.c#L2109) |
| `input.command.character` | `cmd_character()` | 2/8 page; 4/6 horizontal; ? guide; h/f export; v mode; `:` chat; q/Q/Esc/C exit; Ctrl-T | same | inherited | explicit exits; failed filename retry in menu | sheet page/column | optional file dump only | save/load | sheet/version branches | [`c-cmd.c:2199`](../../src/client/c-cmd.c#L2199), [`c-cmd.c:2262`](../../src/client/c-cmd.c#L2262), [`c-cmd.c:2351`](../../src/client/c-cmd.c#L2351) |
| `input.command.guide` | `cmd_the_guide()` | arrows/2/8 line; PgUp/P/N/PgDn page; Home/g, End/G; C/c chapter; `/ s r R S` searches; d/a/A/D bookmarks; f file; # chapter; B/b history; Esc exit; Ctrl-K copy; Ctrl-L paste; `:` chat; `{}` | same | normal disabled selectively; hybrid suppressed in prompts | Esc exit; canceled prompt returns to guide; search failure remains | line/chapter, search direction, bookmarks, marking | guide requests/file reads | save/load | `REGEX_SEARCH`, clipboard, guide protocol versions | [`c-cmd.c:2462`](../../src/client/c-cmd.c#L2462), [`c-cmd.c:3444`](../../src/client/c-cmd.c#L3444), [`c-cmd.c:4395`](../../src/client/c-cmd.c#L4395), [`c-cmd.c:4809`](../../src/client/c-cmd.c#L4809) |
| `input.command.local-file` | `browse_local_file()` | guide navigation subset plus Left/4 and Right/6 horizontal; `/ s r R S` search; d/a/A/D marks; f dump; # line; Esc; Ctrl-K/L; `:` chat | same | modal suppression | cancel prompt returns; invalid retries | file position/offset/search/marks | local file only | save/load | `REGEX_SEARCH`, clipboard | [`c-cmd.c:4818`](../../src/client/c-cmd.c#L4818), [`c-cmd.c:5548`](../../src/client/c-cmd.c#L5548), [`c-cmd.c:5934`](../../src/client/c-cmd.c#L5934) |
| `input.command.lore` | `artifact_lore()`, `monster_lore()` | letter or `@` name; navigation/scroll in received perusal; Esc cancel | same | hybrid suppressed | missing selection / Esc returns; safe macro failures flush | selected lore index, browser position | `Send_special_line()` then perusal | save/load | server data/version | [`c-cmd.c:5947`](../../src/client/c-cmd.c#L5947), [`c-cmd.c:6112`](../../src/client/c-cmd.c#L6112), [`c-cmd.c:6355`](../../src/client/c-cmd.c#L6355), [`c-cmd.c:6711`](../../src/client/c-cmd.c#L6711) |
| `input.command.spoilers` | `cmd_spoilers()` | k/e/r/E/a/v/d/f/t/s select report; `:`; Ctrl-T; Esc/Ctrl-Q | same | normal suppressed for choice | selection opens received/local browser; exit restores | report type | special-file request | save/load | server features | [`c-cmd.c:7345`](../../src/client/c-cmd.c#L7345), [`c-cmd.c:7368`](../../src/client/c-cmd.c#L7368) |
| `input.command.notes` | `cmd_notes()` | Enter open; `/ s` search; PgUp/9/p, PgDn/3/n/Space, Home/7, End/1, arrows/2/8, Backspace; `:`; Ctrl-T; Esc/Ctrl-Q | same | hybrid suppressed | Esc exits; canceled search returns | selected note, scroll/search | local note files | save/load | filesystem | [`c-cmd.c:7424`](../../src/client/c-cmd.c#L7424), [`c-cmd.c:7535`](../../src/client/c-cmd.c#L7535), [`c-cmd.c:7611`](../../src/client/c-cmd.c#L7611) |
| `input.command.misc-index` | `cmd_check_misc()` | exact knowledge/report/platform keys and second-level filters in subordinate map below | same | normal suppressed only while reading menu key | child cancel returns to menu | chosen report/filter | `Send_special_line()` / child browser / platform opener | save/load | many version, platform, SDL3 branches | [`c-cmd.c:7615`](../../src/client/c-cmd.c#L7615), [`c-cmd.c:7779`](../../src/client/c-cmd.c#L7779), [`c-cmd.c:8097`](../../src/client/c-cmd.c#L8097) |
| `input.command.message` | `cmd_message()` | text editor under `Message:`; slash commands parsed locally; ordinary chat sent | same | hybrid suppressed; text may contain macro-like bytes | Esc cancels; Enter submits; empty ignored | chat history/mode; numerous slash shortcuts may alter local options | `Send_msg()` or local command | topline | command-specific/platform screenshot branches | [`c-cmd.c:8100`](../../src/client/c-cmd.c#L8100), [`c-cmd.c:8117`](../../src/client/c-cmd.c#L8117), [`c-cmd.c:8591`](../../src/client/c-cmd.c#L8591) |
| `input.command.guild` | `cmd_guild_options()` | `1` allow-adders; `2` auto-readd; `3` prompt minimum level; `a` request adder list; `b` prompt player to toggle as adder; `:` chat; Ctrl-T; Esc/Ctrl-Q | same | hybrid suppressed | Esc/Ctrl-Q exits; canceled `3`/`b` stays; unavailable/non-master action bells | guild flags/minimum/adder target | `Send_guild_config`; `a` sends `/xguild_adders` | save/load | guild membership/master | [`c-cmd.c:8595`](../../src/client/c-cmd.c#L8595), [`c-cmd.c:8670`](../../src/client/c-cmd.c#L8670) |
| `input.command.party` | `cmd_party()` | `1` create/rename party; `2` iron team; `3` add self/player or join named party; `4` remove player; `5` leave; `0` close iron team + confirm; `a` create guild; `b` add/join guild; `C` remove from guild; `D` leave + confirm; `e` guild options; `A` hostility; `P` peace; `:`; Ctrl-T; Esc/Ctrl-Q | same | hybrid suppressed | Esc/Ctrl-Q exits; canceled name/confirm stays | selected social operation | `Send_party()` / `Send_guild()` | save/load | party/guild state; `s_NO_PK`; guild-options requires server >4.5.2 | [`c-cmd.c:8731`](../../src/client/c-cmd.c#L8731), [`c-cmd.c:8802`](../../src/client/c-cmd.c#L8802), [`c-cmd.c:8921`](../../src/client/c-cmd.c#L8921) |
| `input.command.store` | `display_store()` / special -> `store_process_command()` | store-defined action letters first; Space/Backspace pages; 1-0 page; c paste; `:` chat; I inspect; i/e/b; `{}`; physical keyset-specific wield/takeoff/destroy aliases; `$` gold; Esc/Ctrl-Q | explicit N/R collisions | depends `macros_in_stores`; otherwise command macros blocked | Esc leaves and sends leave; child cancel returns; invalid/default is raw key | `shopping`, `store_top`, stock state | `Send_store_command`, typed item operations, or `Send_raw_key` | save/load | `USE_SOUND_2010`; subinventory; store flags | [`c-store.c:800`](../../src/client/c-store.c#L800), [`c-store.c:1127`](../../src/client/c-store.c#L1127), [`c-store.c:1249`](../../src/client/c-store.c#L1249) |
| `input.command.stock-select` | `get_stock()` | displayed stock letter through `get_com()` | same | store policy | Esc cancels; invalid stock retries | chosen stock index | caller purchase/examine | topline | stock bounds | [`c-store.c:187`](../../src/client/c-store.c#L187) |
| `input.command.skill-tree` | `do_cmd_skill()` | 2/j, 8/k move; 6/l raise; Enter expand; c collapse all; o expand all; g/7/Home, G/1/End; n/Space/3/PgDn; p/b/9/PgUp; # line; s// search; ? guide; `:`; Ctrl-T; Esc/Ctrl-Q | same aliases | inherited; hybrid suppressed for search | exit restores; cancel search stays | selection/start, local `dev` tree | `Send_skill_dev`, `Send_skill_mod` | save/load | available skills | [`skills.c:288`](../../src/client/skills.c#L288), [`skills.c:316`](../../src/client/skills.c#L316), [`skills.c:388`](../../src/client/skills.c#L388) |
| `input.command.ability-select` | `do_cmd_activate_skill_aux()` | a..t; `@` name/number; `* ? Space` show; +/- page; Ctrl-T; Esc | same | safe | Esc/missing safe macro -> -1; bad key retries (or safe flush) | list visibility/start, chosen skill | caller activates skill, may open item/direction prompt | conditional save/load | learned active skills | [`skills.c:535`](../../src/client/skills.c#L535), [`skills.c:586`](../../src/client/skills.c#L586), [`skills.c:624`](../../src/client/skills.c#L624) |
| `input.command.messages` | `do_cmd_messages()` / `_important()` | 8/k/Backspace one older; 2/j/Enter one newer; +/- ten; p/Ctrl-P/b/Ctrl-U older page; n/Ctrl-N/Space newer; g/G ends; 4/h/<, 6/l/> horizontal; # line; = mark; / search; r regex; f/F dump; Ctrl-K copy; `:`; Ctrl-T; Esc/Ctrl-Q | same aliases | hybrid suppressed only in child prompts | exit restores; canceled child returns; no match/invalid bells | message offset/search/mark | optional local dump | save/load | `REGEX_SEARCH`; `COPY_MULTILINE`; important adds Ctrl-O older alias | [`c-xtra2.c:144`](../../src/client/c-xtra2.c#L144), [`c-xtra2.c:329`](../../src/client/c-xtra2.c#L329), [`c-xtra2.c:594`](../../src/client/c-xtra2.c#L594), [`c-xtra2.c:747`](../../src/client/c-xtra2.c#L747) |
| `input.options.root` | `do_cmd_options()` | `1..0` ten option pages; `/` search; `s/S/a` save character/global/class options; `l` load; `T` save window config; `A` account; `C/U` check/update guide; `w` windows; `b/M/m` big/force-big/normal map; `E` redraw layout; `f/g/F` fonts/tilesets/cycle font; `x/X` mixer/pack selector; `n/N` SFX/music event menus; `I` install pack; `c` colour; Windows `Y` copy user dirs; `:`; Ctrl-T; Esc/Ctrl-Q | same | hybrid suppressed | Esc exits; canceled filename/search stays | option values and selected child | option/local-file/platform-specific effects | save/load | platform/audio/graphics/window gates | [`c-util.c:16874`](../../src/client/c-util.c#L16874), [`c-util.c:16983`](../../src/client/c-util.c#L16983), [`c-util.c:17448`](../../src/client/c-util.c#L17448) |
| `input.options.page` | `do_cmd_options_aux()` | 2/8 selection; y/n/t/Enter toggle; Esc; Ctrl-T; `:` | same | hybrid suppressed | Esc closes page | selected option and boolean/value | option-dependent | parent screen | option page contents | [`c-util.c:12849`](../../src/client/c-util.c#L12849), [`c-util.c:12919`](../../src/client/c-util.c#L12919) |
| `input.options.account-window` | `do_cmd_options_acc()`, `_win()` | exact account/window keys in subordinate map below | same | hybrid suppressed | Esc exits; prompt cancel returns | account/window flags | account/server option messages where applicable | save/load | account privileges, `ANGBAND_TERM_MAX` | [`c-util.c:13070`](../../src/client/c-util.c#L13070), [`c-util.c:13106`](../../src/client/c-util.c#L13106), [`c-util.c:13199`](../../src/client/c-util.c#L13199) |
| `input.options.fonts` | `do_cmd_options_fonts()` | exact font keys/name prompts in subordinate map below | directions translated | hybrid suppressed | Esc exits; canceled rename stays | active font/subfont | local renderer reconfigure | save/load/redraw | `USE_X11`, Windows, SDL variants | [`c-util.c:13483`](../../src/client/c-util.c#L13483), [`c-util.c:13765`](../../src/client/c-util.c#L13765), [`c-util.c:14082`](../../src/client/c-util.c#L14082) |
| `input.options.tilesets` | `do_cmd_options_tilesets()` | exact preview/apply/discard keys in subordinate map below | same | hybrid suppressed | Esc with pending changes asks apply then discard; refusal keeps menu | pending vs active tileset, subtiles, resize, outline | renderer/server graphics mode request | save/load/redraw | SDL3/graphics compile options | [`c-util.c:14537`](../../src/client/c-util.c#L14537), [`c-util.c:14971`](../../src/client/c-util.c#L14971), [`c-util.c:15082`](../../src/client/c-util.c#L15082) |
| `input.options.audio-install` | `do_cmd_options_install_audio_packs()` | exact accept/reject/password/acknowledgement transitions in subordinate map below | same | inherited | Esc/password failure aborts current stage; verification errors acknowledge then retry/return | selected archive, detected sound/music pack | filesystem/archive extraction | replace/redraw | `USE_SDL3`, libarchive/platform shell branches | [`c-util.c:15866`](../../src/client/c-util.c#L15866), [`c-util.c:16051`](../../src/client/c-util.c#L16051), [`c-util.c:16547`](../../src/client/c-util.c#L16547) |
| `input.options.colourblind` | `do_cmd_options_colourblindness()` | exact palette/component keys in subordinate map below | same | hybrid suppressed | Esc exits; canceled value returns | palette transform/options | local renderer | save/load | color support | [`c-util.c:16554`](../../src/client/c-util.c#L16554), [`c-util.c:16647`](../../src/client/c-util.c#L16647) |
| `input.options.audio-mixer` | `interact_audio()` | n/6 and p/4 move; 8/+ and 2/- volume; g/G/h set 0/100/50; Enter/Space toggle; a/c/m/s/w and uppercase direct toggles; Ctrl-A/S/W/E presets; Ctrl-G/H/L/B/I/O volume presets; r reset; `:`; Ctrl-T; Esc/Ctrl-U or rogue Ctrl-F | explicit R alias | normal and hybrid suppressed | Esc/audio shortcut exits; values apply live; invalid bells | cursor, four booleans, four volumes | mixer API only | save/load | `USE_SOUND_2010`; weather available | [`c-util.c:18032`](../../src/client/c-util.c#L18032), [`c-util.c:18152`](../../src/client/c-util.c#L18152), [`c-util.c:18395`](../../src/client/c-util.c#L18395) |
| `input.options.audio-pack` | `audio_pack_selector()` | `a/q` next/previous sound pack; `s/w` next/previous music pack; `+/-` next/previous music subset; Ctrl-T; Esc | same | hybrid suppressed | Esc commits current selections if changed, otherwise just restores; invalid retries | `cur_sp`, `cur_mp`, selected music subset | update folders then reinitialize audio/config | save/load | `USE_SOUND_2010`, pack availability, quiet-mode local parsing | [`c-util.c:18693`](../../src/client/c-util.c#L18693), [`c-util.c:19658`](../../src/client/c-util.c#L19658), [`c-util.c:19813`](../../src/client/c-util.c#L19813) |
| `input.audio.sfx` | `do_cmd_options_sfx_sdl()` | 8/2/arrows one; 9/p/PgUp and 3/Space/PgDn page; 7/g/Home, 1/G/End; Backspace up; # index; s event search; S filename; / next; t/y/n enable; v volume; Enter play; Shift-Enter 200%; Ctrl-U or rogue Ctrl-F mixer; `:`; Ctrl-T; Esc | explicit R mixer alias; directions use keymap only for 8/2 | inherited; prompts suppress hybrid | Esc saves overrides and restores entry mixer state; canceled volume resets event volume to 100-equivalent; canceled search leaves term | selected event, disable flags, volume, current sample | local audio/config files | replaces screen; caller redraws | `USE_SOUND_2010`, SDL3; `ENABLE_SHIFT_SPECIALKEYS`; GCU lacks shifted Enter distinction | [`snd-sdl3.c:4268`](../../src/client/snd-sdl3.c#L4268), [`snd-sdl3.c:4444`](../../src/client/snd-sdl3.c#L4444), [`snd-sdl3.c:4700`](../../src/client/snd-sdl3.c#L4700) |
| `input.audio.music` | `do_cmd_options_mus_sdl()` | SFX navigation/search/toggle/volume set; c current; Enter play; a/A all, u/U shuffle all, e/E event only (uppercase boost); P pause; q/w subsong and Q/W event previous/next; 4/6 or Left/Right seek 10s; Ctrl-U/F mixer; `:`; Ctrl-T; Esc | explicit R mixer alias | inherited; prompts suppress hybrid | Esc optionally restores prior game music or permits play-all continuation, then saves; invalid bells | jukebox selection/play-all/shuffle/pause/time/volume/disable state | local audio/config files | replaces screen; caller redraws | `JUKEBOX_INSTANT_PLAY`; `ENABLE_SHIFT_SPECIALKEYS`; SDL3 mixer seek | [`snd-sdl3.c:4784`](../../src/client/snd-sdl3.c#L4784), [`snd-sdl3.c:5018`](../../src/client/snd-sdl3.c#L5018), [`snd-sdl3.c:5360`](../../src/client/snd-sdl3.c#L5360), [`snd-sdl3.c:5885`](../../src/client/snd-sdl3.c#L5885) |
| `input.macro.editor` | `interact_macros()` | `l/L` load named/class pref; `s/S/F/A` save named/global/form/class; `a` action; `c/h/n` command/hybrid/normal macro; `d` identity/delete (compile branch); `o/O` on-load comment/action; `e` empty; `t` test; `w` swap; `i` list; `q/Q` quick macro/config; `r/R` record/config; `G/C/B/U/X/I` forget global/character/both/autoloaded/all/reinitialize; `p` paste; `z` wizard; `Z` macro-set shortcut; `:`; Ctrl-T; Esc | same | explicitly suppresses macros while choosing triggers/menu; raw trigger uses `inkey_base` | Esc leaves current level; child text cancellation returns/goto parent; unsafe collisions warn and retry | `macro__buf`, macro arrays/type flags, macro-set/stage selection | local `.prf`/macro-set files; generated actions later emit commands | save/load | macro recording/set compile branches | [`c-util.c:7296`](../../src/client/c-util.c#L7296), [`c-util.c:7385`](../../src/client/c-util.c#L7385), [`c-util.c:8049`](../../src/client/c-util.c#L8049), [`c-util.c:12031`](../../src/client/c-util.c#L12031) |
| `input.autoinscription.editor` | `auto_inscriptions()` | exact editor/navigation/file keys in subordinate map below | same | normal/hybrid suppressed | Esc exits; canceled child stays | ordered rules and tags | local `.ins` files; apply can affect item inscriptions | save/load | auto-inscription features | [`c-util.c:12043`](../../src/client/c-util.c#L12043), [`c-util.c:12175`](../../src/client/c-util.c#L12175), [`c-util.c:12741`](../../src/client/c-util.c#L12741) |
| `input.birth.credentials` | `choose_name()`, `enter_password()` | live-trim account text; private password | same | startup macro set not semantically applicable | name Esc exits process; password Esc goes back to name; empty retries | `nick`, `pass` | later login handshake | replace | `RETRY_LOGIN`, `SIMPLE_LOGIN` | [`c-birth.c:191`](../../src/client/c-birth.c#L191), [`c-birth.c:274`](../../src/client/c-birth.c#L274) |
| `input.birth.identity` | sex/race/trait/class | letter; 2/4/6/8 and +/-/<> navigate where listed; Enter highlighted; `*` random; `#` saved DNA; `%` auto-reincarnate; Backspace previous; ? guide; Q/Ctrl-Q quit; Ctrl-T | same | startup | invalid retries; Backspace returns false; random/DNA loops internally until legal | `sex`, `race`, `trait`, `class`, selection/highlight, `auto_reincarnation` | none until completed birth | replace | class/race compatibility, `CLASS_BEFORE_RACE`, `RETRY_LOGIN`, server setup | [`c-birth.c:323`](../../src/client/c-birth.c#L323), [`c-birth.c:438`](../../src/client/c-birth.c#L438), [`c-birth.c:626`](../../src/client/c-birth.c#L626), [`c-birth.c:887`](../../src/client/c-birth.c#L887) |
| `input.birth.stats-mode-body` | `choose_stat_order()`, `choose_mode()`, `choose_body_modification()` | selection letters/directions; random/DNA/reincarnate; Backspace; Q; ?; stat distribution +/- and confirmation if points remain | same | startup | invalid retry; Backspace parent; remaining-stat no returns to editor | stat order/points, mode bits, fruit-bat bit | none until birth submit | replace | server mode flags, arcade/RPG, auto-reincarnation | [`c-birth.c:1122`](../../src/client/c-birth.c#L1122), [`c-birth.c:1422`](../../src/client/c-birth.c#L1422), [`c-birth.c:1552`](../../src/client/c-birth.c#L1552), [`c-birth.c:1896`](../../src/client/c-birth.c#L1896) |
| `input.birth.server` | `get_server_name()` / manual | metaserver entry letter; Q/Ctrl-Q manual; manual hostname text/Esc | same | startup | invalid list key retries; manual Esc cancels/quit at caller; selection confirms | chosen host/port; ping subprocess bookkeeping | connect target | replace; first `inkey()` may raise SDL/X11 window | `EXPERIMENTAL_META`, `META_PINGS`, Windows/POSIX, `USE_SDL3` | [`c-birth.c:2380`](../../src/client/c-birth.c#L2380), [`c-birth.c:2470`](../../src/client/c-birth.c#L2470), [`c-birth.c:2742`](../../src/client/c-birth.c#L2742) |
| `input.shutdown` | `c_close_game()` | acknowledgements; tomb/reincarnation menu; Ctrl-T screenshot; filename prompt | same | macros suppressed in menus | key depends death/alive branch; exit eventually closes | death/reincarnation/screenshot state | disconnect/quit handled by caller | replace | death state, screenshot support | [`c-util.c:17705`](../../src/client/c-util.c#L17705), [`c-util.c:17796`](../../src/client/c-util.c#L17796), [`c-util.c:17907`](../../src/client/c-util.c#L17907) |
| `input.command.house` | `cmd_purchase_house()` and house helpers | exact house keys/prompts in subordinate map below | same except initial direction | hybrid suppressed in menu | Esc/Ctrl-Q cancels; canceled child prompt still closes menu; rejected purchase/delete confirmation returns to menu | selected adjacent-house operation | `Send_purchase_house` or `Send_admin_house` | save/load; child owner screen replaces temporarily | house/admin privilege and item availability | [`c-cmd.c:9091`](../../src/client/c-cmd.c#L9091), [`c-cmd.c:9206`](../../src/client/c-cmd.c#L9206) |
| `input.admin.master` | `cmd_master*` and script helpers | exact root/level/generation/build/summon/player/system keys in subordinate map below | same except nested directions | hybrid suppressed in menu loops | Esc/Ctrl-Q returns; canceled child aborts that action and returns to owner | editor/generation/admin parameters | admin `Send_*` messages | save/load or topline | admin DM/wizard, numerous server compile/version gates | [`c-cmd.c:9314`](../../src/client/c-cmd.c#L9314), [`c-cmd.c:10463`](../../src/client/c-cmd.c#L10463) |
| `input.command.lagometer` | `cmd_lagometer()` | `1` enable; `2` disable full and mini meter; `c` clear 60 ping samples; `:` chat; Ctrl-T screenshot; Esc/Ctrl-Q/Ctrl-I exit | same | inherited | explicit exit restores; all other keys are ignored and retry | `lagometer_enabled`, `ping_times[]`, `lagometer_open` | none | save/load, then flush queue | none | [`c-cmd.c:10606`](../../src/client/c-cmd.c#L10606) |

## Canonical gameplay command table

This table is exhaustive for `process_command()`. `Normal` and `Rogue` are physical keys after `keymap_init()`; blank differences inherit the canonical key. The command layer consumes macros first. Any canonical key not listed below is sent as `Send_raw_key(key)`; `]` is raw unless the player is an admin ([`src/client/c-cmd.c:426`](../../src/client/c-cmd.c#L426), [`src/client/c-cmd.c:506`](../../src/client/c-cmd.c#L506)).

| Canonical | Normal | Rogue | Action / emitted intent |
|---|---|---|---|
| Esc, `-`, CR | same | same | no-op (`-` is intentionally ignored after target macros) |
| Space | same | same | clear repeated actions |
| `)` | same | same | `Send_clear_buffer` |
| `+` | `+` or `T`; Ctrl-direction tunnels | `+` or Ctrl-b/j/n/h/l/y/k/u | direction -> tunnel |
| `;` | `;` or numpad/digits | b/j/n/h/l/y/k/u or digits | direction -> walk |
| `.` | `.` | shifted rogue direction or `,` | run |
| `,`, `g`, Ctrl-G | same | `.`/5, `g`, Ctrl-Z | stay/pick up; one-item variant |
| `M`, `L`, `s`, `S`, `R` | same | `M`, Ctrl-W, `s`, `#`, `R` | minimap, locate, search, search toggle, rest |
| `<`, `>` | same | same | stairs |
| `o`, `c`, `B`, `D` | same | `o`, `c`, `f`, `D` | open/close/bash/disarm with direction |
| `i`, `e`, `d`, `$`, `w`, `t`, `x`, `k`, `K` | same | `i`, `e`, `d`, `$`, `w`, `T`, `S`, Ctrl-D, Ctrl-C | inventory/equipment/drop gold/wield/takeoff/swap/destroy/force-stack |
| `{`, `}`, `H`, `j` | same | `{`, `}`, Ctrl-G, Ctrl-A | inscribe/uninscribe/apply auto-inscriptions/steal |
| `q`, `r`, `a`, `u`, `z`, `F`, `E`, `A` | same | `q`, `r`, `z`, `Z`, `a`, `F`, `E`, `A` | quaff/read/aim/staff/rod/refill/eat/activate |
| `f`, `v` | same | `t`, `v` | fire/throw |
| `b`, `G`, `m`, `U` | same | `P`, `G`, `m`, Ctrl-X | browse book/skill tree/activate skill/ghost power |
| `*`, `(`, `l`, `I` | same | `*`, `(`, `x`, `I` | target/friendly target/look/inspect item |
| `C`, `~`, `\|`, `'`, `@`, `#`, `?` | same | same | character/misc/uniques/player equipment/players/scores/help |
| `:`, `P` | same | `:`, `O` | chat/social menu |
| `_`, `p`, `W`, `V` | same | same | sip/telekinesis/secondary wield/cloak |
| Ctrl-O, Ctrl-P | same | same | important/all message recall |
| Ctrl-Q, Ctrl-R, `Q` | same | same | quit connection/redraw/suicide confirmation |
| `=`, `"`, `%`, `&`, `h`, `/` | same | `=`, `"`, `%`, `&`, Ctrl-E, `/` | options/load pref/macro editor/autoinscriptions/house/all-in-one |
| Ctrl-S, Ctrl-T, Ctrl-I | same | same | spike/screenshot/lagometer |
| Ctrl-U, Ctrl-C, Ctrl-N | same | Ctrl-F, unavailable, Ctrl-V | SFX menu/music/master toggles (`USE_SOUND_2010`) |
| `!` | same | same | BBS |
| `]` | same | same | admin menu, else raw key |
| other | same | same | `Send_raw_key` |

The translator itself is authoritative for every N/R difference and direction code ([`src/client/c-util.c:1729`](../../src/client/c-util.c#L1729), [`src/client/c-util.c:1848`](../../src/client/c-util.c#L1848), [`src/client/c-util.c:1890`](../../src/client/c-util.c#L1890)). The canonical dispatch and typed call sites are in [`src/client/c-cmd.c:274`](../../src/client/c-cmd.c#L274) through [`src/client/c-cmd.c:506`](../../src/client/c-cmd.c#L506).

## Shipped and user macro layers

Macro records are action/trigger pairs. `A:` stores the next action; `P:` creates a normal macro, `H:` a hybrid macro, and `C:` a command macro. Redefinition replaces action and flags for the same trigger ([`src/client/c-files.c:1487`](../../src/client/c-files.c#L1487), [`src/client/c-util.c:303`](../../src/client/c-util.c#L303)). Runtime acceptance is:

| Type | Gameplay command wait | Ordinary modal loop | Text prompt (`inkey_msg`) | Store with `macros_in_stores=false` |
|---|---|---|---|---|
| normal `P:` | yes | yes | yes unless menu sets `inkey_interact_macros` | yes |
| command `C:` | yes | no | no | no |
| hybrid `H:` | yes | yes | no | no |

These rules come directly from the macro filter ([`src/client/c-util.c:1173`](../../src/client/c-util.c#L1173)). `safe_macros` adds prompt-specific abort/flush behavior rather than a fourth macro type, notably for missing items and invalid abilities ([`src/client/c-inven.c:1420`](../../src/client/c-inven.c#L1420), [`src/client/skills.c:645`](../../src/client/skills.c#L645)).

Queue control is part of the observable macro contract. A matched action is pushed in front of byte 29; byte 29 clears `parse_macro` and may restore a stored topline prompt. Byte 28 brackets special fallback text, byte 31 begins a platform key sequence, and `inkey()` strips all three control forms rather than delivering them as commands ([`src/client/c-util.c:1276`](../../src/client/c-util.c#L1276), [`src/client/c-util.c:1569`](../../src/client/c-util.c#L1569), [`src/client/c-util.c:1609`](../../src/client/c-util.c#L1609), [`src/client/c-util.c:20048`](../../src/client/c-util.c#L20048)). The two wait directives are not interchangeable: `\\wDD` preserves the active queue and ordinarily cannot be canceled by the user; `\\WDDDD` temporarily separates newly pressed keys and gives Esc/Space explicit abort/resume semantics.

Shipped general macros are:

| Trigger | Type | Action |
|---|---|---|
| `X` | command | swap weapons tagged `@0` (`x0`) |
| `,` | hybrid | reply to whisper (`:+:`) |
| Ctrl-Q | hybrid | escape pending prompts then quit |
| `;` | hybrid | combined guide search |
| `<`, `>` | hybrid | escape pending prompts then stairs |
| `[` | command | repeat last chat/slash command |

Source: [`lib/user/pref.prf:47`](../../lib/user/pref.prf#L47) through [`lib/user/pref.prf:76`](../../lib/user/pref.prf#L76).

The shipped SDL3 file defines four equivalent families of normal keypad/navigation triggers: keypad digits, legacy R7-R15, navigation keys, and keypad-navigation keysyms. Unmodified keys produce `0..9`; Shift produces `Esc Esc \\.<digit>` (run); Control produces `Esc Esc \\+<digit>` (tunnel). It then defines hybrid F-key actions: F1 target+fire; F2 and Shift-F2 first/second birth spell; F3-F8 quaff/read slots 1..3; F11 targeted throw by `@bad`; F12 universal `/1` action ([`lib/user/pref-sdl3.prf:18`](../../lib/user/pref-sdl3.prf#L18), [`lib/user/pref-sdl3.prf:108`](../../lib/user/pref-sdl3.prf#L108), [`lib/user/pref-sdl3.prf:197`](../../lib/user/pref-sdl3.prf#L197), [`lib/user/pref-sdl3.prf:284`](../../lib/user/pref-sdl3.prf#L284)). Arcade builds additionally auto-load `arcade-sdl3.prf`, which chains to shipped Tron/Smash/Other macro sets ([`src/client/c-files.c:1829`](../../src/client/c-files.c#L1829), [`lib/user/arcade-sdl3-tron.prf:15`](../../lib/user/arcade-sdl3-tron.prf#L15), [`lib/user/arcade-sdl3-smash.prf:6`](../../lib/user/arcade-sdl3-smash.prf#L6)).

User macros are intentionally not enumerable from the repository. The load order is normative and machine-transcribable: `pref.prf` -> `global.opt` -> `pref-$SYS.prf` -> graphics/font file -> `global-$SYS.opt`, then per character `$cname.opt`, `global.prf`, `$Race.prf`, `$Trait.prf`, `$Class.prf`, `$cname.prf`, and optionally `$cname^$Form.prf` ([`src/client/c-init.c:232`](../../src/client/c-init.c#L232), [`src/client/c-init.c:302`](../../src/client/c-init.c#L302), [`src/client/c-files.c:1855`](../../src/client/c-files.c#L1855)). Later definitions for an identical trigger replace earlier ones, so a consumer must resolve this ordered overlay at runtime rather than snapshot only shipped defaults.

## Completeness ledger by requested file

This ledger lists every function in the requested files that directly calls `inkey`, `inkey_combo`, `Term_inkey`, `askfor_aux`, `get_string`, `get_com`, `get_check*`, or `get_dir`. A function may delegate to an atomic context already represented above.

| File | Direct owners covered |
|---|---|
| `c-birth.c` | `choose_name`, `enter_password`, `choose_sex`, `choose_race`, `choose_trait`, `choose_class`, `choose_stat_order`, `choose_mode`, `choose_body_modification`, `get_char_info`, `enter_server_name`, `get_server_name` |
| `c-cmd.c` | `cmd_all_in_one`, `process_command`, movement direction wrappers, `cmd_mini_map`, `cmd_locate`, inventory/subinventory/equipment, destroy/inscribe, steal/device/activate direction prompts, target/look, character, guide/local-file, artifact/monster lore, spoilers, notes, misc index, message, guild, party, fire/throw, load-pref, house commands, suicide, all `cmd_master*`/script helpers, lagometer |
| `c-inven.c` | `verify`, `get_item_hook_find_obj`, `c_get_item` |
| `c-store.c` | `get_stock`, purchase/sell quantity and confirmation prompts, `display_store`, `display_store_special` |
| `c-util.c` | `sync_sleep`, `sync_xsleep`, `inkey_combo`, `inkey_aux`, `inkey`, `askfor_aux`, all prompt helpers, `request_command`, `get_dir`, quantity/number, macro trigger/file-set/editor, auto-inscriptions, all options submenus, audio install/mixer/pack selector, shutdown |
| `c-xtra2.c` | `do_cmd_messages`, `do_cmd_messages_important` |
| `skills.c` | `do_cmd_skill`, `do_cmd_activate_skill_aux`, activation direction prompts |
| `snd-sdl3.c` | `do_cmd_options_sfx_sdl`, `do_cmd_options_mus_sdl` |

The following matrix cross-checks every requested source against each input-call family. Counts are lexical occurrences in the working tree, so they deliberately include helper definitions, disabled compatibility branches, and comments naming the primitive; the owner ledger above is the semantic accounting. A nonzero cell is mapped to the listed report contexts.

| Required source | raw `inkey*` / `Term_inkey` | text `askfor/get_string` | confirm `get_check*` | direction `get_dir` | command `request/get_com` | Accounted by |
|---|---:|---:|---:|---:|---:|---|
| `c-birth.c` | 13 | 3 | 1 | 0 | 0 | `input.birth.*`, prompt primitives |
| `c-cmd.c` | 39 | 56 | 28 | 24 | 6 | `input.command.*`, prompt primitives, `input.admin` |
| `c-inven.c` | 1 | 1 | 1 | 0 | 0 | `input.command.item-select`, `input.core.missing-item` |
| `c-store.c` | 2 | 0 | 1 | 0 | 1 | `input.command.store`, `input.command.stock-select`, quantity/confirm primitives |
| `c-util.c` | 128 | 69 | 5 | 1 | 9 | `input.core.*`, `input.prompt.*`, macro/options/audio/shutdown contexts |
| `c-xtra2.c` | 2 | 10 | 0 | 0 | 0 | `input.command.messages`, text primitive |
| `skills.c` | 2 | 3 | 0 | 2 | 0 | skill-tree/ability/item/direction contexts |
| `snd-sdl3.c` | 2 | 6 | 0 | 0 | 0 | `input.audio.sfx`, `input.audio.music`, text/quantity primitives |

Low-level `sync_sleep()` and `sync_xsleep()` are network-aware waits, not user decision contexts: they poll `Term_inkey`, network input, timers and redraws, and may consume an interrupting key ([`src/client/c-util.c:417`](../../src/client/c-util.c#L417), [`src/client/c-util.c:568`](../../src/client/c-util.c#L568)). `get_char_info()`'s final “hit any key” and several installer/shutdown acknowledgements are represented as acknowledgement transitions rather than separate command contexts.

### Per-function ownership cross-check

The larger rows above describe a shared interaction contract, not shared C ownership. The following subordinate table is the atomic function-level accounting: every function in the required sources that directly performs a blocking/key/prompt read appears exactly once. “Delegate” means it owns one prompt transition but not a retry loop; “loop” means it owns repeated input. Multiple names in one cell are compile-time duplicate definitions or two entry points sharing the same state machine.

| Required source / direct owner | Kind | Context / transition |
|---|---|---|
| `c-birth.c: choose_name` | loop | credentials: edit account; Esc terminates client |
| `c-birth.c: enter_password` | loop | credentials: private edit; Esc returns to name; empty retries |
| `c-birth.c: choose_sex` | loop | birth identity: select/random/DNA/quit |
| `c-birth.c: choose_race` | loop | birth identity: navigate/select/back/guide |
| `c-birth.c: choose_trait` | loop | birth identity: navigate/select/back/guide |
| `c-birth.c: choose_class` | loop | birth identity: navigate/select/back/guide |
| `c-birth.c: choose_stat_order` | loop | birth stats: order/distribute/confirm remainder/back |
| `c-birth.c: choose_mode` | loop | birth mode: select/random/DNA/back |
| `c-birth.c: choose_body_modification` | loop | birth body: normal/bat/random/DNA/back |
| `c-birth.c: get_char_info` | loop coordinator | sequences the birth loops; final any-key acknowledgement |
| `c-birth.c: enter_server_name` | delegate | manual hostname text prompt |
| `c-birth.c: get_server_name` | loop | metaserver choice/manual fallback |
| `c-cmd.c: cmd_all_in_one` | delegate | item/action chooser followed by typed direction prompts |
| `c-cmd.c: process_command` | delegate | optional uniques filter prompt; canonical dispatch owner |
| `c-cmd.c: cmd_tunnel, cmd_walk, cmd_run, cmd_open, cmd_close, cmd_bash, cmd_disarm` | delegates | one direction prompt each; caller returns on cancel |
| `c-cmd.c: cmd_mini_map` | loop | minimap pan/exit/chat |
| `c-cmd.c: cmd_locate` | loop | map locate pan/exit |
| `c-cmd.c: cmd_inven` | loop | one-key inventory viewer |
| `c-cmd.c: cmd_subinven` | loop | one-key subinventory viewer |
| `c-cmd.c: cmd_equip` | loop | one-key equipment viewer |
| `c-cmd.c: cmd_destroy` | delegate | item selector plus conditional confirmation |
| `c-cmd.c: cmd_inscribe` | delegate | item selector plus inscription text |
| `c-cmd.c: cmd_steal` | delegate | direction prompt |
| `c-cmd.c: cmd_aim_wand` | delegate | item then direction |
| `c-cmd.c: cmd_zap_rod` | delegate | item then conditional direction |
| `c-cmd.c: cmd_activate` | delegate | item then conditional direction |
| `c-cmd.c: cmd_target, cmd_target_friendly` | loop | hostile/friendly target cursor |
| `c-cmd.c: cmd_look` | loop | look cursor |
| `c-cmd.c: cmd_character` | loop | character sheet browser/export |
| `c-cmd.c: cmd_the_guide` | loop | guide browser/search/bookmarks |
| `c-cmd.c: browse_local_file` | loop | local file browser/search/marks |
| `c-cmd.c: artifact_lore` | loop | artifact selection/perusal |
| `c-cmd.c: monster_lore` | loop | monster selection/perusal |
| `c-cmd.c: cmd_spoilers` | loop | spoiler report chooser |
| `c-cmd.c: cmd_notes` | loop | notes chooser/search |
| `c-cmd.c: cmd_check_misc` | loop | misc-report chooser and filter prompts |
| `c-cmd.c: cmd_message` | delegate | one text editor; slash/chat dispatch |
| `c-cmd.c: cmd_guild_options` | loop | guild options and nested text prompts |
| `c-cmd.c: cmd_party` | loop | party/guild action menu and prompts |
| `c-cmd.c: cmd_fire, cmd_throw` | delegates | direction prompt after item/ammo resolution |
| `c-cmd.c: cmd_load_pref` | delegate | pref filename prompt |
| `c-cmd.c: cmd_house_chown` | loop | owner/access choice plus name prompt |
| `c-cmd.c: cmd_house_chmod, cmd_house_kill, cmd_house_tag` | delegates | confirmation series / tag text |
| `c-cmd.c: cmd_purchase_house` | loop | direction, operation choice, confirmation |
| `c-cmd.c: cmd_suicide` | loop | yes/no then verification key |
| `c-cmd.c: cmd_master_aux_level` | loop | level-builder menu and nested fields/confirmations |
| `c-cmd.c: cmd_master_aux_generate_vault` | loop | vault generator menu/name |
| `c-cmd.c: cmd_master_aux_generate` | loop | generator menu |
| `c-cmd.c: cmd_master_aux_build` | loop | builder menu/sign text |
| `c-cmd.c: cmd_master_aux_summon_orcs` | loop | summon preset chooser |
| `c-cmd.c: cmd_master_aux_summon_undead_low` | loop | summon preset chooser |
| `c-cmd.c: cmd_master_aux_summon_undead_high` | loop | summon preset chooser |
| `c-cmd.c: cmd_master_aux_summon` | loop | summon menu/custom target text |
| `c-cmd.c: cmd_master_aux_player` | loop | player-admin menu and name/message fields |
| `c-cmd.c: cmd_script_upload, cmd_script_exec, cmd_script_exec_local` | delegates | script name/source text prompts |
| `c-cmd.c: cmd_master_aux_system, cmd_master` | loops | system/root admin menus |
| `c-cmd.c: cmd_king` | delegate | ownership confirmation |
| `c-cmd.c: cmd_spike` | delegate | direction prompt |
| `c-cmd.c: cmd_lagometer` | loop | lagometer enable/style/exit |
| `c-inven.c: verify` | delegate | uppercase item confirmation |
| `c-inven.c: get_item_hook_find_obj` | delegate | item-name text search |
| `c-inven.c: c_get_item` | loop | full item selector, bags and safe-macro states |
| `c-store.c: get_stock` | loop | stock letter selector |
| `c-store.c: store_purchase, store_sell, store_do_command` | delegates | quantities and conditional confirmation |
| `c-store.c: display_store, display_store_special` | loops | same `store_process_command` state machine over normal/special rendering |
| `c-util.c: sync_sleep, sync_xsleep` | loops | macro wait and cancelable extended wait |
| `c-util.c: inkey_combo` | delegate | one decoded navigation-capable logical key |
| `c-util.c: inkey_aux, inkey` | loops | event/network/macro pipeline |
| `c-util.c: askfor_aux` | loop | text editor |
| `c-util.c: get_string, get_com, get_com_bk` | delegates | text/one-key prompt wrappers |
| `c-util.c: request_command` | delegate | scanned gameplay command plus escape/control prefixes |
| `c-util.c: get_dir` | delegate | direction/target prompt |
| `c-util.c: get_check2, get_check3, get_3way` | loops | binary/ternary confirmations |
| `c-util.c: c_get_quantity, c_get_number` | delegates | bounded numeric text prompts |
| `c-util.c: get_macro_trigger` | loop | raw trigger capture (`inkey_base`) |
| `c-util.c: macrofileset_get, macrofileset_stage_get` | loops | numeric set/stage selection |
| `c-util.c: interact_macros` | loop | sole owner of all goto-labelled macro/wizard substates |
| `c-util.c: auto_inscriptions` | loop | rule editor and nested text/file prompts |
| `c-util.c: do_cmd_options_aux` | loop | generic option page |
| `c-util.c: do_cmd_options_acc` | loop | account/password options |
| `c-util.c: do_cmd_options_win` | loop | terminal-window flag options |
| `c-util.c: do_cmd_options_fonts` | loop | font/subfont chooser |
| `c-util.c: do_cmd_options_tilesets` | loop | staged graphics chooser/apply/discard |
| `c-util.c: do_cmd_options_install_audio_packs` | loop | archive/install/password workflow |
| `c-util.c: do_cmd_options_colourblindness` | loop | palette editor |
| `c-util.c: do_cmd_options` | loop | root options dispatcher; child loops above return here |
| `c-util.c: c_close_game` | loops | death/tomb/quit acknowledgement branches |
| `c-util.c: interact_audio` | loop | audio mixer |
| `c-util.c: audio_pack_selector` | loop | audio pack/subset chooser |
| `c-xtra2.c: do_cmd_messages, do_cmd_messages_important` | loops | same recall state machine over different backing histories |
| `skills.c: do_cmd_skill` | loop | skill tree |
| `skills.c: do_cmd_activate_skill_aux` | loop | ability selector |
| `skills.c: do_activate_skill` | delegates | skill-specific item/direction follow-ups |
| `snd-sdl3.c: do_cmd_options_sfx_sdl` | loop | SFX event browser/editor |
| `snd-sdl3.c: do_cmd_options_mus_sdl` | loop | music jukebox/editor |

### Raw-key fallthrough, exhaustively

In these sources “raw key” has a precise implementation meaning: `cmd_raw_key(k)` calls `Send_raw_key(k)` without interpreting it locally ([`src/client/c-cmd.c:10585`](../../src/client/c-cmd.c#L10585)). Every call site in the required files is:

| Site | Condition | Result |
|---|---|---|
| `process_command()` `]` | not admin DM/wizard | `Send_raw_key(']')` |
| `process_command()` default | canonical command has no local case | `Send_raw_key(command_cmd)` |
| `store_process_command()` `w` | store claims `w`/rogue collision disallows local wield | raw `w` |
| `store_process_command()` `W` | rogue keyset or store collision | raw `W` |
| `store_process_command()` Ctrl-W | normal keyset or store collision | raw Ctrl-W |
| `store_process_command()` `t` | rogue keyset or store collision | raw `t` |
| `store_process_command()` `T` | normal keyset or store collision | raw `T` |
| `store_process_command()` `k` | rogue keyset or store collision | raw `k` |
| `store_process_command()` Ctrl-D | normal keyset or store collision | raw Ctrl-D |
| `store_process_command()` default (including `$` outside home) | no store/local case consumed it | raw original store key |

Evidence: [`src/client/c-cmd.c:426`](../../src/client/c-cmd.c#L426), [`src/client/c-cmd.c:506`](../../src/client/c-cmd.c#L506), [`src/client/c-store.c:1017`](../../src/client/c-store.c#L1017) through [`src/client/c-store.c:1054`](../../src/client/c-store.c#L1054). This is distinct from slash commands: `cmd_message()` recognizes `/...` strings in the chat editor, but the audited sources never call those “raw commands” and they do not use `Send_raw_key`. Accordingly, this ticket inventories the slash-command **input loop** and its local-vs-`Send_msg` dispatch boundary, not every slash verb grammar. That grammar is explicitly listed as a follow-up gap below.

### Exact nested-loop key maps

This is the subordinate transition table for compact inventory rows that share one owner/state machine. Together with the main inventory it is the atomic key-to-transition manifest; phrases such as “exact ... below” are cross-references, not unenumerated behavior. Unless a row says otherwise, Esc returns to the parent, a canceled child prompt returns to its owning menu, and an unrecognized key rings `bell()` or performs no transition before the next read.

| Owner/substate | Exact accepted keys -> transition, prompt, or intent |
|---|---|
| Macro wizard step 1 | `a/b` quaff/read; `c/C` fire/throw; `d/D` untargeted school/mimic spell; `e/E` targeted school/mimic spell; `f` mimic spell number; `g/G` polymorph/immunity; `h/H` rune/trap; `i/I` fighting technique/combat mode; `j/J` shooting technique/device; `k/K` any item without/with target; `l/L` basic ability without/with target; `m/M` common action/element preference; `n/N` slash/custom; `o/O` load macro file/option; `p` equipment; `q/r/s/t/u` run/tunnel/disarm/bash/close direction; `Z` chain; `S` macro-file set; Backspace/Esc exit; `:` chat; Ctrl-T screenshot ([`src/client/c-util.c:8532`](../../src/client/c-util.c#L8532), [`src/client/c-util.c:8660`](../../src/client/c-util.c#L8660)). |
| Macro wizard choice leaves | Potion/scroll/any item/throw/trap/device prompts ask distinctive item text; spells/abilities/techniques ask name or number; option asks option name then `y/n/t/Y/N/T`; equipment normal asks `w/W/t/x/d/b`, rogue asks `w/W/T/S/d/b`; direction leaves take raw direction; each leaf then captures a trigger and adds command/hybrid/normal according to selected chain type. Esc/`p`/Backspace returns/restarts; invalid choices retry ([`src/client/c-util.c:8699`](../../src/client/c-util.c#L8699), [`src/client/c-util.c:9055`](../../src/client/c-util.c#L9055), [`src/client/c-util.c:9206`](../../src/client/c-util.c#L9206), [`src/client/c-util.c:9984`](../../src/client/c-util.c#L9984), [`src/client/c-util.c:11717`](../../src/client/c-util.c#L11717)). |
| Macro-file-set wizard | `W` select set; `D` delete set; `C` create set; `I` insert stage; `S` activate stage; `A/F` add/forget reference keys in loaded macros; `k` set cyclic key; `m` set free-switch key; `s` save stage; `t` rename; `i` insert; `d` delete; `a` activate/load; `c` comment; `w` write/update; Esc/`p`/Backspace leave; `:` chat; Ctrl-T screenshot ([`src/client/c-util.c:10469`](../../src/client/c-util.c#L10469), [`src/client/c-util.c:10728`](../../src/client/c-util.c#L10728), [`src/client/c-util.c:11210`](../../src/client/c-util.c#L11210)). |
| Auto-inscription editor | Ctrl-T screenshot; `:` chat; `{`/`}` inscribe/uninscribe; `?/h` help; `/` search; `#` index; `P` paste; Esc exit; `3/n/Space/PgDn`, `9/p/Backspace/PgUp`, `G/1/End`, `g/7/Home`, `2/Down`, `8/Up` navigate; `x/w` move rule down/up; `l/L/s/S/A` load/save file variants; `e/6/Enter/Right` edit; `d` clear; `X` delete; `I` insert; `c` clear all; `a` cycle pickup/destroy/ignore; `f` force; `b` bags-only; `t` disabled ([`src/client/c-util.c:12175`](../../src/client/c-util.c#L12175), [`src/client/c-util.c:12340`](../../src/client/c-util.c#L12340), [`src/client/c-util.c:12718`](../../src/client/c-util.c#L12718)). |
| Generic option page | Esc/Ctrl-Q return; `/` next search match; Ctrl-T; `:`; `-/8/k/Backspace` previous; `+/2/j/Enter/Space` next; `y/6/l/s` set; `n/4/h/u` clear; `t/5/w` toggle ([`src/client/c-util.c:12919`](../../src/client/c-util.c#L12919)). |
| Account options | `C` runs three private password prompts: current, new, repeat; Esc returns; mismatch/error returns to page ([`src/client/c-util.c:13070`](../../src/client/c-util.c#L13070), [`src/client/c-util.c:13106`](../../src/client/c-util.c#L13106)). |
| Window options | `d` SDL3 decorations; `T/t` choose terminal by following key then flag; `y/Y`, `n/N`, Enter set/clear/toggle selected window flag; `:`; Ctrl-T; Esc ([`src/client/c-util.c:13199`](../../src/client/c-util.c#L13199), [`src/client/c-util.c:13282`](../../src/client/c-util.c#L13282)). |
| Font options | `v` visibility; Space window focus; `r` rename; `R` reset names; `./=/+` grow/cycle forward; `,/-` shrink/back; Enter choose name; `l/L` logfont now/next start; `a` antialias; `:`; Ctrl-T; Esc ([`src/client/c-util.c:13765`](../../src/client/c-util.c#L13765), [`src/client/c-util.c:13991`](../../src/client/c-util.c#L13991)). |
| Tileset options | `i/o` outline down/up; `q/Q`, `w/W` previous/next terrain preview; `a/A`, `s/S` item preview; `z/Z`, `x/X` monster preview; `p/P` apply/revert pending; `v` graphics mode; `r` resize interpolation; `=/+`, `-`, Enter select tileset; `l` list; `f` fonts; `:`; Ctrl-T; Esc apply/discard decision ([`src/client/c-util.c:14971`](../../src/client/c-util.c#L14971), [`src/client/c-util.c:15079`](../../src/client/c-util.c#L15079), [`src/client/c-util.c:15217`](../../src/client/c-util.c#L15217)). |
| Colourblind options | `c` palette-animation mode; `n/d/p/t/s/r` edit named transform components through 2- or 3-digit prompts; `:`; Esc ([`src/client/c-util.c:16554`](../../src/client/c-util.c#L16554), [`src/client/c-util.c:16647`](../../src/client/c-util.c#L16647)). |
| Audio installer | candidate confirmation accepts `y/Y/Space/Enter`, rejects `n/N`, all other keys retry; encrypted archives prompt password until valid or Esc/empty skips; target-folder confirmation accepts only `y/Y` or `n/N`; error/status screens accept any key ([`src/client/c-util.c:16145`](../../src/client/c-util.c#L16145), [`src/client/c-util.c:16221`](../../src/client/c-util.c#L16221), [`src/client/c-util.c:16445`](../../src/client/c-util.c#L16445)). |
| Knowledge/misc index | `1` artifacts; `2` monsters then filter one key (Esc all, Space uniques, otherwise the byte is the monster glyph) and optional minimum-level quantity; `3` uniques then Esc/other all, Space/`a` alive, `!/b/e` bosses; `4` objects then one type byte or Esc all; `5` traps; `6` artifact lore; `7` monster lore; `8` recalls; `9` houses; `0` minimap; `a` players; `b` player equipment; `c` scores; `d` deaths; `e` lagometer; `f` server news; `g` guide; `h` MOTD; `i` server settings; `j/k` all/important message recall; `l` spoilers; `n` notes; SDL3 `o` or non-SDL3 `x` sends `/ex`; `?` help; `:` chat; Ctrl-T screenshot; Esc/Ctrl-Q exit. Platform-open keys are `T/U/S/M/X` for game/user/sound/music/xtra paths and SDL3 adds lowercase `t/u/s/m/x` variants; `G/W/P/R/L` open project/server/player-store/monster/ladder URLs; `#` opens the last screenshot; `C` opens the config file where supported. Unsupported terminal-platform opener keys display an error instead ([`src/client/c-cmd.c:7779`](../../src/client/c-cmd.c#L7779), [`src/client/c-cmd.c:7887`](../../src/client/c-cmd.c#L7887), [`src/client/c-cmd.c:7988`](../../src/client/c-cmd.c#L7988)). |
| House menu and leaves | After a required direction, `1` confirms buy/sell then `Send_purchase_house`; `2` chooses owner type, where `1` prompts player name and sends `O1<name>`, while `2` sends guild ownership `O2`; `3` asks party/class/race/winner/fallen-winner/no-ghost permission confirmations plus minimum level and sends `M<flags><level>`; `4` selects a potion and sends `P<item>`; `5` prompts a tag and sends `T<tag>`; `s` sends `S` to enter player store; `k` sends `H` to knock; admin-only `D` confirms then sends `K`; `:` chat; Ctrl-T screenshot; Esc/Ctrl-Q cancel. The menu closes after every selected child, even if that child's text/item prompt is canceled ([`src/client/c-cmd.c:9091`](../../src/client/c-cmd.c#L9091), [`src/client/c-cmd.c:9144`](../../src/client/c-cmd.c#L9144), [`src/client/c-cmd.c:9206`](../../src/client/c-cmd.c#L9206)). |
| Admin root/system | root `1` level, `2` build, `3` summon, `4` generation, `5` player, `6` system, debug `a/b`, `c`/Esc/Ctrl-Q exit; system `1` log, `e` execute script, `u` upload, `c` local script, Esc/Ctrl-Q; both accept `:` and Ctrl-T ([`src/client/c-cmd.c:10409`](../../src/client/c-cmd.c#L10409), [`src/client/c-cmd.c:10499`](../../src/client/c-cmd.c#L10499)). |
| Admin level/generation | level `1/2` static/unstatic, `3/4` add/remove dungeon, `5` town, `6/7` perma-static/unstatic, `a/b` save/load module, `c` blank, `d` entry; generation `1` vault then vault `1` numeric or `2` name; all Esc, `:`, Ctrl-T ([`src/client/c-cmd.c:9314`](../../src/client/c-cmd.c#L9314), [`src/client/c-cmd.c:9522`](../../src/client/c-cmd.c#L9522), [`src/client/c-cmd.c:9594`](../../src/client/c-cmd.c#L9594)). |
| Admin build | `1` wall, `2` permanent wall, `3` tree, `4` dead tree, `5` grass, `6` dirt, `7` floor, `8` house door, `9` sign + text, `0` arbitrary feature, `a/A/B/C/D` floor/info variants; `:`; Ctrl-T; Esc ([`src/client/c-cmd.c:9644`](../../src/client/c-cmd.c#L9644), [`src/client/c-cmd.c:9692`](../../src/client/c-cmd.c#L9692)). |
| Admin summon | root `1` orcs, `2` low undead, `3` high undead, `4` depth, `5` name, `6` obliterate, `7` disable; placement `1` one here, `2` one random, `3` group here, `4` group random, `5` mode. Orc leaves are `1..7`; low undead `1..9,a..c`; high undead `1..9,a..e`; `:`/Ctrl-T/Esc at each menu ([`src/client/c-cmd.c:9807`](../../src/client/c-cmd.c#L9807), [`src/client/c-cmd.c:9882`](../../src/client/c-cmd.c#L9882), [`src/client/c-cmd.c:9969`](../../src/client/c-cmd.c#L9969), [`src/client/c-cmd.c:10059`](../../src/client/c-cmd.c#L10059)). |
| Admin player | `1` edit, `2` account, `3` kill (optional `!` no-ghost), `4` summon, `5` unsummon, `6` recall, `7` telekinesis, `8` message; each applicable action prompts player/message text; Esc/default leaves; `:` and Ctrl-T ([`src/client/c-cmd.c:10267`](../../src/client/c-cmd.c#L10267)). |

One key family cannot be spelled as fixed literals because it is runtime protocol data: before built-ins, the store compares the key against every active server-supplied `c_store.letter[i]` and maps an exact match to `store_do_command(i, FALSE)`. Those letters also suppress colliding local wield/takeoff/destroy aliases; unmatched input then reaches the fixed built-ins or raw fallthrough enumerated above ([`src/client/c-store.c:800`](../../src/client/c-store.c#L800), [`src/client/c-store.c:815`](../../src/client/c-store.c#L815)). Race, class, skill, and similar letter lists are likewise indexed runtime data, as recorded under uncertainties.

## Focus restoration contract

Legacy focus has three distinct meanings that must not be collapsed:

- OS input focus: the first blocking SDL/X11 `Term_inkey` may raise/grab the main window; the metaserver code explicitly relies on this side effect ([`src/client/c-birth.c:2750`](../../src/client/c-birth.c#L2750)).
- terminal activation: every `inkey()` activates `term_term_main`, then reactivates the prior `Term` and restores cursor visibility before returning ([`src/client/c-util.c:1408`](../../src/client/c-util.c#L1408), [`src/client/c-util.c:1705`](../../src/client/c-util.c#L1705)).
- visual/modal restoration: full-screen modal owners use `Term_save()`/`Term_load()`; topline prompts clear row zero and flush queued events; replace-screen birth/shutdown flows rely on the parent state machine to draw the next screen.

The new client should restore the previously focused surface and its selection/caret, not emulate OS window raising. A nested prompt must return to its owning modal loop; leaving the outer loop returns to the gameplay surface.

## Gaps and uncertainties

1. The ticket names “every input loop” but constrains a file list. Other client files contain loops (spell browsing/casting, targeting helpers, network initialization, Lua UI, platform setup). They were followed only when directly called by an audited loop; they are not exhaustively enumerated here.
2. `cmd_message()` contains many locally recognized slash commands. They are text commands, not distinct blocking loops, and are represented as one text-input context. A separate raw/slash-command protocol inventory is needed if the destination requires every slash verb and argument grammar.
3. Admin/master branches are source-visible but depend on server privileges and version-specific raw protocols. The table preserves their ownership and prompt shape but does not claim player-facing availability.
4. Macro-set wizard internals span thousands of lines and generate actions whose semantics depend on server command strings. The atomic context is the macro editor plus its nested wizard; a future manifest should derive individual generated bindings from the wizard templates, not hand-copy them.
5. `snd-sdl3.c` is uncommitted relative to the cited HEAD in this checkout. Line citations are therefore exact for the working tree but not durable until that work is committed.
6. Several compile-time alternatives are dead `#if 0` branches. They were not treated as active bindings. GCU explicitly cannot distinguish Shift-Enter in the audio menus; that platform must not advertise boosted preview.
7. User macro contents, user option files, server-supplied store action letters, and server-supplied race/class/skill lists are runtime data. Exhaustiveness means the loaders and overlay/dispatch rules are covered, not that those unknowable values are pre-enumerated.

## Recommended transcription boundary

Transcribe the rows above as `inputContexts` first. Bindings should point to either a local action (`navigate`, `toggle`, `open-child`, `cancel`, `confirm`) or a typed/raw network intent. Preserve these fields per binding: `physicalGesture`, `logicalKey`, `keyset`, `macroPolicy`, `compileGate`, `ownerContext`, `transition`, `intent`, and `returnContext`. Macro definitions should remain a separate ordered overlay feeding the same logical-key pipeline.
