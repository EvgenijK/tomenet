# Slash-command grammar and dispatch

Статус: source audit, 2026-09-14; исследование для [Enumerate slash-command grammar and dispatch](../issues/18-enumerate-slash-command-grammar-and-dispatch.md). HEAD: `4211671279ff820575c32239272bca68cf8762f7`; исследован working tree, без runtime evidence. Общий dirty checkout не переключался, research branch не создавалась: контекстом служат этот файл, ticket и SHA-256 прочитанных источников ниже. Это инвентарь фактов и предложения для решения, не утверждённый action catalog или реализация нового клиента.

## Результат

`cmd_message()` содержит весь локальный slash-dispatch в одном owner: screenshot pre-pass и последовательную цепочку после chat transformations. Общего tokenizer, нормализации регистра, whitespace trimming, argument quoting или таблицы verbs нет. Любая строка, которую цепочка не поглотила, отправляется как один `PKT_MESSAGE`; неизвестные команды и неверно оформленные варианты части известных команд остаются server-owned. Локальная обработка не обязательно означает отсутствие сетевых эффектов: options, auto-inscriptions и audio reload используют другие packets. [c-cmd.c:8100](../../../src/client/c-cmd.c#L8100), [fallback:8588](../../../src/client/c-cmd.c#L8588), [Send_msg:7826](../../../src/client/nclient.c#L7826).

## Entry, editor и порядок dispatch

1. Основной logical binding `:` вызывает `cmd_message`; многочисленные меню предоставляют тот же entry. Буфер имеет размер `MSG_LEN - strlen(cname) - 5 - 3 - 8`, editor limit ещё на единицу меньше; `MSG_LEN=256`. Буфер зануляется, `inkey_msg=TRUE`, затем `get_string("Message: ", …)`. [binding:412](../../../src/client/c-cmd.c#L412), [owner:8100](../../../src/client/c-cmd.c#L8100), [constant:275](../../../src/common/defines.h#L275).
2. `get_string` включает ASKFOR_CHATTING; prompt меняется на Party/Floor/Guild. `askfor_aux` обеспечивает text editing/history и macro semantics: hybrid/command macros подавляются через `inkey_msg`, обычные macros не исключены этим флагом. Esc возвращает FALSE и не отправляет сообщение; accepted text записывается в chat history до decoration. [get_string:3464](../../../src/client/c-util.c#L3464), [macro filter:251](../../../src/client/c-util.c#L251), [history:3350](../../../src/client/c-util.c#L3350).
3. До возврата editor добавляет `!:` / `#:` / `$:` в соответствующем chat mode, кроме строк с первым `/` и explicit one-character addressed prefixes `%:`, `!:`, `$:`, `#:`, `+:`. Здесь нет общего распознавания `name:`. `%%:` не попадает в это исключение: в nonnormal mode может получить channel prefix и уже не быть локальным self chat. [decoration:3421](../../../src/client/c-util.c#L3421).
4. Screenshot pre-pass с case-sensitive prefix обрабатывается раньше любых substitutions. Затем каждый `{` заменяется на byte `\377` во всём буфере; проходят item shortcuts; затем self-chat; затем локальные slash branches по порядку таблицы ниже; затем fallback `Send_msg`. Поэтому даже Lua source, option names и server arguments получают colour/item transformations, а screenshot filename — нет. [pre-pass:8121](../../../src/client/c-cmd.c#L8121), [colour:8139](../../../src/client/c-cmd.c#L8139), [paste:8150](../../../src/client/c-cmd.c#L8150), [self:8286](../../../src/client/c-cmd.c#L8286).
5. Большинство exits очищает `inkey_msg=FALSE`, без восстановления старого значения; `/new` очищает перед nested selector. Исключение: exact no-argument `/wager` возвращает до очистки, оставляя TRUE; это source-visible compatibility question, не исправленный здесь bug. [new:8332](../../../src/client/c-cmd.c#L8332), [wager:8565](../../../src/client/c-cmd.c#L8565), [cleanup:8591](../../../src/client/c-cmd.c#L8591).

**Поправка к предыдущему inventory:** Enter с пустым текстом не отсеивается в `cmd_message`. `askfor_aux` возвращает TRUE после non-Esc completion; в normal mode пустая строка попадает в `Send_msg`, в channel mode декорируется prefix. Формулировка «empty ignored» в прежнем input-loop inventory не доказывается клиентским owner. Это не означает, что сервер покажет пустое сообщение. [askfor result:3349](../../../src/client/c-util.c#L3349), [decoration:3421](../../../src/client/c-util.c#L3421), [fallback:8588](../../../src/client/c-cmd.c#L8588).

## Полный локальный verb inventory

Обозначения: **CI exact** — `!strcasecmp`, только полное совпадение без trailing spaces; **CS prefix** — `prefix`, чувствительный к регистру, без delimiter check сверх указанного литерала; **CI prefix N** — `!strncasecmp` первых N bytes. `prefix` не проверяет границу слова. Порядок строк соответствует исполнению; aliases в строке имеют один outcome. [prefix:165](../../../src/common/z-util.c#L165).

| Verb / aliases | Recognizer и аргументы | Outcome, state / packets / prompts | Gates и source |
|---|---|---|---|
| `/shot`, `/screenshot`; variants `/shotpng`, `/screenshotpng` | CS prefix `/shot` OR `/screenshot`; первый ASCII space и непустой следующий byte дают filename tail, иначе `screenshot????`. PNG variants — тоже unrestricted CS prefix | `xhtml_screenshot(name,FALSE)`; variants устанавливают `inkey_shift_special=3`; нет slash packet и confirmation | capture outcome зависит от screenshot_keys, ENABLE_SHIFT_SPECIALKEYS и backend; [8121](../../../src/client/c-cmd.c#L8121) |
| `/ctime` | CI exact; без args | local wall-clock time через time/localtime, английский weekday; formatted chat/important message | нет server gate; [8298](../../../src/client/c-cmd.c#L8298) |
| `/cver`, `/cversion` | CI exact | local compile-time VERSION_MAJOR..BUILD/TAG/OS/SUB | [8306](../../../src/client/c-cmd.c#L8306) |
| `/apickup` | CI exact | toggle c_cfg.auto_pickup; feedback; не обновляет Client_setup.options и не вызывает Send_options | [8310](../../../src/client/c-cmd.c#L8310) |
| `/adestroy` | CI exact | toggle c_cfg.auto_destroy, тот же local-only update contract | [8315](../../../src/client/c-cmd.c#L8315) |
| `/daunmatched` | CI exact | toggle c_cfg.destroy_all_unmatched; feedback сообщает dependency auto_destroy, но команда не включает её и не блокируется при FALSE | [8320](../../../src/client/c-cmd.c#L8320) |
| `/new` | CI exact; без args | показывает previous newest; child item-select `Which item?`, extra name lookup `Item name?`; USE_INVEN/EQUIP/EXTRA/SUBINVEN/UNPREFER_SUBINVEN; cancel/no valid i exits; только 0<=i<INVEN_TOTAL принимается; задаёт item_newest, optional redraw_newest | ENABLE_SUBINVEN только previous diagnostic; child selector допускает subinventory, owner отклоняет его encoded result; [8325](../../../src/client/c-cmd.c#L8325) |
| `/new ` | CS prefix с одним space; пустой tail вызывает тот же selector | substring `help` anywhere (CS) OR first argument byte `?` → usage. Иначе используется только buf[5], tail игнорируется; lower maps inventory, upper maps equipment; byte >= 'a'+INVEN_WIELD-1 заменяется на Z; bounds и nonempty tval проверяются; item_newest+redraw | нет support двухбуквенного subinventory arg (TODO). `/NEW a` не этот branch; [8344](../../../src/client/c-cmd.c#L8344) |
| `/opty`, `/optvy` | CI prefix 5/6; обязательно space сразу за verb и хотя бы один следующий byte; offset6/7; option tail exact CS | enable; redundant quiet unless verbose; validates o_desc/o_text; updates o_var и Client_setup.options; options_immediate(TRUE/FALSE), check_immediate_options, Send_options при изменении | специальный exact tail big_map → set_bigmap(1,verbose) без Send_options; [8383](../../../src/client/c-cmd.c#L8383) |
| `/optn`, `/optvn` | такой же parser | disable; redundant quiet unless verbose; hooks+Send_options при изменении | big_map → set_bigmap(0,verbose); [8435](../../../src/client/c-cmd.c#L8435) |
| `/optt`, `/optvt` | такой же parser | toggle; verbose feedback; hooks+Send_options для найденного option | big_map → set_bigmap(-1,verbose). Ошибка syntax для verbose варианта печатает `/optt`, не `/optvt`; [8488](../../../src/client/c-cmd.c#L8488) |
| `/know` | CI exact | cmd_check_misc(): existing knowledge/menu surface, дальнейшие inputs и packets принадлежат меню | synonym logical `~`; [8533](../../../src/client/c-cmd.c#L8533), [binding:383](../../../src/client/c-cmd.c#L383) |
| `/cvpng` | CI exact | png_screenshot(); result игнорируется; это conversion/open-last outcome, не cmd_message filename capture | platform/helper gates ниже; [8537](../../../src/client/c-cmd.c#L8537) |
| `/? a` … `/? t` | CS prefix `/? `, total length4, exactly lower a..t | cmd_the_guide(3,0,one-letter bookmark); нет Send_msg | GUIDE_BOOKMARKS; иначе весь текст fallback; [8541](../../../src/client/c-cmd.c#L8541) |
| `/reinit_guide` | CI exact | init_guide(), diagnostics, forced checksum check and outdated feedback; нет update/download guide content action | checksum backend gates ниже; [8547](../../../src/client/c-cmd.c#L8547) |
| `/reinit_audio` | CI exact | re_init_sound(); success message only result0; audio/resource state reload, downstream Send_audio | helper USE_SOUND_2010, SOUND_SDL/SOUND_SDL3, runtime use_sound; recognizer itself unguarded; [8555](../../../src/client/c-cmd.c#L8555) |
| `// ` | CI prefix3 (punctuation); всё после третьего byte Lua source, может быть пустым | string_exec_lua(0,buf+3), local formatted result; arbitrary exposed Lua state/I/O/callback effects; нет встроенной confirmation | не admin/server gate, не restricted command whitelist; [8559](../../../src/client/c-cmd.c#L8559), [executor:533](../../../src/client/c-script.c#L533) |
| `/wager` | CI prefix6; если buf[6]==0 report; иначе atol(buf+6), delimiter не обязателен | std_wager query/set; clamps >PY_MAX_GOLD to max, <1 to1 after assignment to s32b; no packet. `/wager10` →10, `/wager abc` →1, `/wager 10junk` →10; no-arg branch leaves inkey_msg TRUE | PY_MAX_GOLD=2000000000L; atol overflow/platform long width не safe parsing contract; [8565](../../../src/client/c-cmd.c#L8565), [constant:2434](../../../src/common/defines.h#L2434) |
| `/apply_auto_inscriptions`, `/aai` | CI exact | apply_all_auto_inscriptions(), all inventory/equipment and conditional subinventory; downstream inscription/uninscription/server-autoinscription packets; нет confirmation | ENABLE_SUBINVEN, REGEX_SEARCH, autoinscription flags/rules; server autoinscribe version gate ниже; [8582](../../../src/client/c-cmd.c#L8582) |

Ни один recognizer в этом owner не вызывает `get_check*`: file capture, toggles, wager, Lua и массовые autoinscriptions не получают отдельного подтверждения. `/new` добавляет selection, `/know` и guide открывают уже существующие interactive surfaces. Новые UI confirmations были бы решением, а не source parity. [полный owner:8100](../../../src/client/c-cmd.c#L8100).

## Chat substitutions и non-slash local routes

| Syntax после editor | Transformation / routing | Source |
|---|---|---|
| `{` | byte FF colour prefix; replacement не ограничивается visible chat text | [8139](../../../src/client/c-cmd.c#L8139) |
| два backslash + a..inventory limit, A..equipment limit, `_`, `+` | inventory/equipment name, floor description, newest name; newest=-1 даёт empty item; subinventory newest gated ENABLE_SUBINVEN. Colour `FFs`, restore `FF-`, spacing, colon escaping и size check; overflow branch discards shortcut | [8150](../../../src/client/c-cmd.c#L8150) |
| три backslash + uppercase pack slot + lowercase subslot | subinventory name; эта substitution branch **не окружена ENABLE_SUBINVEN** в cmd_message; exact available-build declarations отдельно от этой lexical observation. Overflow cleanup использует i+3, хотя consumed shortcut длиной5 | [8211](../../../src/client/c-cmd.c#L8211) |
| три backslash + lower store slot | store_top+slot; a..l normally, a..z screen_hgt==MAX_SCREEN_HGT. store_paste_item/where; location only once/message, colon escaping. store stock/size failure discards shortcut | [8249](../../../src/client/c-cmd.c#L8249), [big_shop:8112](../../../src/client/c-cmd.c#L8112) |
| `%:` кроме `%::` | local c_msg_format to messages; consumes, no network | [8286](../../../src/client/c-cmd.c#L8286) |
| `%%:` кроме `%%::` | local format with chat marker FC; consumes, no network, subject to earlier chat-mode decoration | [8292](../../../src/client/c-cmd.c#L8292) |
| всё остальное | Send_msg; includes explicit channels/private addresses, escaped self routes, unknown slash, bare `/`, malformed case/spacing variants | [8588](../../../src/client/c-cmd.c#L8588) |

Size checks в item paste сравниваются с MSG_LEN, хотя actual local buf меньше MSG_LEN из-за cname tax; tails ограничиваются sizeof(buf) только в отдельных paths. Это source-visible mismatch, не доказательство безопасной максимальной expansion. Short screenshot name также проходит downstream `x=strlen(name)-4` до suffix check. Исследование фиксирует эти места для defect-compatibility discussion, не утверждает runtime crash или exploit. [paste bounds:8176](../../../src/client/c-cmd.c#L8176), [screenshot suffix:2626](../../../src/client/c-files.c#L2626).

## Downstream effects, persistence и availability

- Toggle trio напрямую меняет c_cfg, не записывает файлы. Эти fields — реальные option_info entries auto_pickup/auto_destroy/destroy_all_unmatched; generic `/opt*` использует другой synchronization/hook path. Команда сама не выполняет pickup/destroy action. Сохранение options отдельным owner описано в [Persisted settings and files: source inventory](persisted-settings-and-files.md). [option table:535](../../../src/client/c-tables.c#L535), [toggles:8310](../../../src/client/c-cmd.c#L8310).
- Generic option operand — **весь suffix**; `option_info[i].o_desc` должен существовать и o_text совпасть. Space/tab/trailing whitespace не нормализуются. Compile/platform/version/runtime restrictions применяются через hooks: GCU palette_animation/disable_lightning/font_map_solid_walls и Windows USE_LOGFONT corrections; legacy !GLOBAL_BIG_MAP branch и server flags. options_immediate может redraw bars/status, load form macros, apply autoinscriptions, Send_redraw. Не следует считать accepted toggle boolean окончательным до hooks. [options_immediate:12771](../../../src/client/c-util.c#L12771), [check_immediate_options:19879](../../../src/client/c-util.c#L19879).
- `big_map` special path меняет global_c_cfg_big_map; GCU rejected. Actual screen resize/clear/Send_screen_dimensions требуют server **newer than 4.4.9.1.0.1** и SFLG1_BIG_MAP. Flag может поменяться раньше capability condition; map dimension change — отдельный intent, не оконная топология новой UI. [set_bigmap:20655](../../../src/client/c-util.c#L20655).
- Wager initial default10, process-local s32b; используется как default quantity store casino action. Slash set не делает bet и не сохраняет файл. [variable.c:63](../../../src/client/variable.c#L63), [store:780](../../../src/client/c-store.c#L780).
- Autoinscriptions helper вызывает local matching с force=TRUE, но apply_auto_inscriptions_aux всё равно early-rejects c_cfg.auto_inscr_off; top-level `if(auto_inscr_off)` disabled через #if0. При отсутствии local handled inscription и enabled server flag проверяется существующая inscription и вызывается Send_autoinscribe; это возможно даже при local-off. force=TRUE разрешает overwrite existing inscription; matching зависит от REGEX_SEARCH, disabled/bag-only rules, rule/tag и existing inscription; sends PKT_INSCRIBE или PKT_UNINSCRIBE; server fallback PKT_AUTOINSCRIBE только server newer than4.5.5.0.0.0. Все эти effects обходят Send_msg и не меняют файл rules. [mass owner:1729](../../../src/client/c-cmd.c#L1729), [matching:6818](../../../src/client/nclient.c#L6818), [sends:7007](../../../src/client/nclient.c#L7007), [wire/gate:7699](../../../src/client/nclient.c#L7699).
- Screenshot filename tail не shell/token parsed: first space делит head и filename, весь tail передаётся helper. XHTML default suffix ???? генерирует serial или timestamp по screenshot_format; explicit name получает .xhtml, существующий file переименовывается в .bak **до raster choice**. SDL3 writes user user-dir, legacy ANGBAND_DIR_USER. screenshot_keys инвертирует regular-vs-shift choice; PNG suffix лишь synthetic shift=3, не unconditional PNG contract. ENABLE_SHIFT_SPECIALKEYS selects X11 ImageMagick import / SDL3 screenshot ext / Windows async batch capture. XHTML path ниже пишет file. [naming:2626](../../../src/client/c-files.c#L2626), [path/backup:2706](../../../src/client/c-files.c#L2706), [raster gate:2730](../../../src/client/c-files.c#L2730).
- `/cvpng`: USE_SDL3 opens last screenshot with FILEMAN if filename/file exists; legacy Windows/X11 converts last XHTML using external browser/helper paths; other backend diagnostic rejects. FILEMAN macro присваивает res; SDL3 helper вызывает SDL_OpenURL(file://…), returns0/-1. Combined backend builds используют порядок USE_X11 → WINDOWS → USE_SDL3 macro выбора. Этот audit не проверяет фактический launch или converter availability. [FILEMAN/backend helper:6955](../../../src/client/c-cmd.c#L6955). [png_screenshot:7016](../../../src/client/c-cmd.c#L7016).
- Guide reload reads TomeNET-Guide.txt and recreates buffered guide metadata, then forced checksum helper. USE_SDL3+SDL3_CURL_SSL hashes local file and fetches remote checksum; USE_SDL3 without feature prints manual-check diagnostic and returns0; legacy implementation external tools. cmd_message blanket nonzero diagnostic claims missing sha256sum even for other helper errors: retain helper/result ownership for later decision. No requested guide-content update is executed by this slash verb. [init:2985](../../../src/client/c-init.c#L2985), [checks:3676](../../../src/client/c-files.c#L3676), [fallbacks:3833](../../../src/client/c-files.c#L3833), [caller:8547](../../../src/client/c-cmd.c#L8547).
- Audio reload helper defined under USE_SOUND_2010; runtime !use_sound returns0 after thunder lookup, so success can mean no module initialization. SDL module reload SOUND_SDL/SOUND_SDL3, set_mixing and Lua sound-index cache follow; successful full path sends Send_audio. Recognizer has no local compile guard: unsupported configuration viability must be assessed at build layer, not inferred as an absent verb. [helper:3553](../../../src/client/c-init.c#L3553), [notification:3620](../../../src/client/c-init.c#L3620).
- Lua executor lua_dostring; returns first result converted to string or null for no returned value, empty string on error, then restores Lua stack. Arbitrary script can call shipped exposed APIs/I/O; finite slash inventory cannot enumerate arbitrary programs' effects. Input-loop Lua API boundaries documented in [Remaining client input loops](remaining-client-input-loops.md). [executor:533](../../../src/client/c-script.c#L533).

## Wire boundary и server-owned grammar

`Send_msg` сериализует `PKT_MESSAGE` через `Packet_printf(&wbuf,"%c%S",…,message)`; не интерпретирует slash, не добавляет channel prefix и не проверяет server version. cmd_message не проверяет результат send; network batching/flush принадлежат network loop. Server Receive_message читает `%c%S`, обрезает MSG_LEN-1, вызывает player_talk; тот split по TAB и делегирует player_talk_aux. Slash path дальше вызывает do_slash_cmd. Поэтому client fallback должен сохранять возможность server commands, в том числе tab-separated server language; локальный parser не превращает все компоненты такого message в local actions. [Send_msg:7826](../../../src/client/nclient.c#L7826), [receive:14031](../../../src/server/nserver.c#L14031), [tab split:6493](../../../src/server/util.c#L6493), [server slash boundary:5733](../../../src/server/util.c#L5733).

Полный каталог do_slash_cmd, permissions и server-side confirmations находится за границей этого ticket: generic forwarding не означает, что every server verb должен стать hardcoded client action. Build/server rollout может менять opaque grammar без изменения cmd_message. Direct Send_msg callers (например menu-generated /ex и /xguild_adders) обходят local command parser; их owners остаются в surface inventory. [direct caller:7901](../../../src/client/c-cmd.c#L7901), [guild:8702](../../../src/client/c-cmd.c#L8702).

## Предложения для последующего решения

Это candidates для [Preserve input and macro semantics](../issues/04-preserve-input-and-macro-semantics.md), не самостоятельные bindings, утверждённые исследованием:

| Proposed intent family | Typed parameters / outcome | Source basis |
|---|---|---|
| message.compose / submit | text, current chat mode; raw history separate from decorated/transformed wire payload; unknown server text fallback | editor+cmd_message |
| screenshot.capture; screenshot.open-or-convert-last | filename/default pattern; explicit raster intent resolved against screenshot_keys/backend; last capture state | screenshot pre-pass + cvpng |
| client.show-local-time / show-version | read-only local result | ctime/cver |
| automation.toggle | pickup/destroy/destroy-unmatched target; preserve direct-toggle-vs-generic-option synchronization distinction until decided | toggle trio |
| newest-item.choose / assign-slot | item selector versus slot result, cancel outcome and invalid item feedback | new branches |
| option.set / toggle | canonical exact option name, desired boolean or toggle, verbose; hooks/capability result; big-map separate dimension action | opt families |
| knowledge.open; guide.open-bookmark / reload; audio.reload | existing modal intents, bookmark a..t, helper outcomes | know/bookmarks/reinit |
| lua.execute-local | raw program text, explicit legacy transformation decision; no inferred server permission | // branch |
| wager.query / set-default; inscriptions.apply-all | clamped amount/default quantity and local-vs-wire effects | wager/aai |

Logical `:` и menu entries should bind compose; Ctrl-T уже capture, `~` knowledge, `H` single-item autoinscription (не mass apply). Comment «CTRL+G» у mass verb не доказательство main logical binding; process_command main map содержит H. Slash aliases — текстовые spellings этих outcomes, не физические gestures. Macros могут вызывать text-entry через свой existing byte stream; предложение прямого typed binding требует сохранения history, suppression и cancel contract. [bindings:347](../../../src/client/c-cmd.c#L347), [383](../../../src/client/c-cmd.c#L383), [412](../../../src/client/c-cmd.c#L412), [461](../../../src/client/c-cmd.c#L461).

Решения, которые исследование делает конкретными: оставить ли смешанную legacy case/prefix grammar и pathological accepted spellings; как трактовать /wager flag leak, empty message и unsafe expansion bounds; сохранять ли colour/item substitutions внутри option/Lua arguments; как представить screenshot capture format versus configured inversion и cvpng platform outcomes; предоставлять ли direct mass-autoinscription/Lua typed actions и какие history semantics они имеют. Это вопросы parity/defect compatibility и action contracts; не implementation slices.

## Полнота, воспроизведение и ограничения

Прочитан непрерывный owner cmd_message8100–8592 и весь local recognizer chain, не только grep hits. Lexical proof ledger для inventory: screenshot8121/8125; ctime8298; cver8306; apickup8310; adestroy8315; daunmatched8320; new8325/8344; opty8383; optn8435; optt8488; know8533; cvpng8537; bookmark8542; reinit_guide8547; reinit_audio8555; Lua8559; wager8565; aai8582; fallback8588. Self routes8286/8292 отдельно. Downstream helpers и server transport проверены по указанным источникам. Не проводились login, actual Lua execution, destructive automation, capture/helper launch, network checksum request или builds unsupported variants. Чтение локального первичного source не требует внешних docs; никаких фактов о текущем внешнем API report не утверждает.

```bash
rg -n 'cmd_message\(|Send_msg\(' src/client/c-cmd.c src/client/nclient.c
sed -n '8100,8600p' src/client/c-cmd.c
rg -n 'prefix\(|strcasecmp|strncasecmp|Send_|get_check|#if|#else|#endif' src/client/c-cmd.c
rg -n 'apply_all_auto_inscriptions|apply_auto_inscriptions_aux|Send_autoinscribe|Send_inscribe|Send_uninscribe' src/client/c-cmd.c src/client/nclient.c
rg -n 'options_immediate|check_immediate_options|set_bigmap' src/client/c-util.c
rg -n 'xhtml_screenshot|png_screenshot|screenshot_keys|inkey_shift_special|SDL3_CURL_SSL|check_guide_checksums' src/client/c-cmd.c src/client/c-files.c
rg -n 'std_wager' src/client
rg -n 'Receive_message|player_talk|do_slash_cmd' src/server/nserver.c src/server/util.c
sha256sum src/client/c-cmd.c src/client/nclient.c src/client/c-files.c src/client/c-util.c src/client/c-script.c src/client/c-init.c src/client/c-tables.c src/client/variable.c src/client/c-store.c src/common/z-util.c src/common/defines.h src/server/nserver.c src/server/util.c
```

SHA-256 исследованного source state:

| Source | SHA-256 |
|---|---|
| src/client/c-cmd.c | fe190ffdb11ae636ec7cf96964a0e4e3cc895fd74e9bbdcd23dac5b51a9e9a8a |
| src/client/nclient.c | a0c6e85ec05944a1bc0e3115012a2a57b69f16f9c14e8a59bc5baabc0029ce60 |
| src/client/c-files.c | ccbd1dcfbc6d743419719264991dd62960670a2eb66e08ca6177dd1a03887968 |
| src/client/c-util.c | a948ad57c78fbb22d04adddabd149cec85f311ead463ac17c30ee7b7c566f05c |
| src/client/c-script.c | 83740e2fe5a36e340fbacabe258e5f9180035e84b4749f793044e5f12db831f7 |
| src/client/c-init.c | 51eded7b8d2ed221f9995bd777aa4f7a23af9e876296f28ee2db119d82a28672 |
| src/client/c-tables.c | 94cc27da7b272aa7d6967a39b1731cd94fe22547088c1f455c1f1a69cb4957f4 |
| src/client/variable.c | 0da4064e96f1871859e05790c6f0b7aba1727266db901db48dfb9f12b2668e37 |
| src/client/c-store.c | 69062361291e1b366351e27779bdfb1b148572cf07fe8ddf1ac179b1391d65a7 |
| src/common/z-util.c | a6d027d8844f7dde9cffc04b4fcedeee42828c9b37b30cd657908ee7eb075fe9 |
| src/common/defines.h | fed6de422b6fa1b945989507425f4ac513895a7a44a493a7bf94ab0928052c4e |
| src/server/nserver.c | 2bd3b7c1f4c7cf5a27c144425d39e49c967fbafbf5ec44f199e8f67c745206e9 |
| src/server/util.c | 58fb9ce7fc2cd39f4a457cfbb9b4579a2a40b85da44e037ca61545512af0d5dc |
