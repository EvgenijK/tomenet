# Ticket 10: information, social and server-driven outcomes

The local task is `.scratch/single-window-sdl3-client-stage-a/issues/10-register-information-social-and-server-driven-outcomes.md`. Baseline: `399fdd38211035b0810cf721ba8e0f1efecf5629`. This allocation adds 193 outcomes (462 total), all implementation/evidence **pending**, zero accepted. No game runtime, shared client code, schema or validator changes.

## Authority and stages

Current C sources define behavior/protocol. [item-policy.md](item-policy.md) retains the approved A–F allocation and native layout contract; [session-policy.md](session-policy.md) retains the field-byte/defect policy. Referenced source files are fingerprinted in the manifest. Inventory snapshots below are discovery aids, never execution evidence; source corrections take precedence over old summaries.

B includes real chat/messages and the read-only character/history children of session final review. C includes AMT/NUM/STR/CFR requests and their concrete ordinary-store consumers (order, mail, donation, repair and memory services). D includes remaining information/social/house/admin, local/server documents and special stores. The existing A message/key-request rows are unchanged and prove no new caller. Each consumer must exercise its real ID/type, parent, success, rejection and cancellation, even when a shared prompt is an earlier prerequisite.

Generic documents and special canvases are native surface obligations. They do not authorize terminal fallback. All new ledger rows carry source-backed result/lifecycle obligations; network rows additionally require exact bytes, complete/incomplete/chained decode and version gates. Every result requires the full caller scenario, not a screenshot or a synthetic isolated request.

## Shared references and handoffs

| Owner | Mapping / requirement |
| --- | --- |
| 08 | Reuse session MOTD, map/minimap, targeting/directions, lifecycle and final review IDs. Setup MOTD is not news/MOTD2. Final review explicitly depends on the B sheet/history children. |
| 09 | Reuse `items.details-read/navigate/search/close` for item OTHER perusal at C, and inventory/inscription/paste/skills/store actions. Ordinary store shell IDs remain shared; special canvas body is D. `store.service` now depends on concrete C server consumers. A generic D document does not delay an item C child. |
| 11 | Own Guide load/update/search/bookmark, clipboard, screenshots/export/chardump, file persistence/import, local Lua integration, resource and OS openers. Their concrete production paths must arrive with any earlier caller needing them (sheet help at B, skills help at C, lore exports or admin upload at D); family E is not permission to postpone prerequisites. This ticket registers local spoiler/note content/navigation, not shared file ownership. |
| 12 | Reconcile whole-client denominator and all server-owned slash verbs/raw keys. `chat.forward-slash` registers forwarding, not acceptance of every server command. Preserve optional builds and current unknown payload representation. |
| 13 | Scope real native evidence and reject unsupported/stale claims. This data validation and foundation regressions do not accept future gameplay. |

Known cross-domain entry points must be exercised as caller scenarios: chat from stores/skills/books/lore/house/admin/documents restores the exact parent; `{`/`}` calls use existing inscription IDs; document Help `?` invokes local Guide; screenshot/export/copy retains its actual owner and error behavior. Until ticket 11 creates its IDs those named child obligations remain pending and block the corresponding complete caller acceptance. No placeholder capability duplicates those owners.

## Source corrections and exact contracts

- Character `h/H` cycles pages; `2/8` selects help topics, not pages. `4/6` moves selection, `v` only changes equipment matrix orientation. Only `f/F` dumps a file; `h` is not export. Preserve existing reopen state; resizing cannot reset it.
- Local artifact/monster lore is generated from local typed arrays and data, not `Send_special_line`. Local notes/spoilers use files. Preserve entity/file identity, filters, detail mode, title/current/all chat-paste bytes, missing-file behavior and per-file positions. Unknown source text remains accessible.
- Documents retain category, original title/line/attribute bytes, page markers/total, logical position, search and arrival order. `line=-1` is title, geometry sentinels are not ordinary rows. SPECIAL_LINE_POS and player refresh update the active owner. No wrap; horizontal scroll for wide source, server paging for long content. Close sends NONE, clears local type/count and restores macro/queue state. OTHER can be packet-driven and must preserve arbitrary content.
- SPECIAL_FILE_NONE is close, not a document; EXTRAINFO=17 is compiled out, not an active category. `/ex` remains direct slash/message output. All active 1–16 categories have explicit open outcomes.
- Special string/char/clear/animation packets apply only while shopping; raw operations retain original coordinates and order. Start+100 force-clear includes graphical images. Uniform complete fit covers text, glyphs, raw multi-cell pictures and clear rectangles; controls remain outside canvas. Opcode 0 wheel, 1 slots, 2 in-between, 3 dice settle, 4 cards; unknown opcode emits the baseline error and remains representable. Resize/recreation reconstructs state without replaying sound, betting or replies. Optional ASCII/graphics/raw-picture/audio/animation branches remain evidence obligations.
- Request KEY Escape returns zero (existing IDs); AMT/NUM cancel still replies zero; STR Escape sends byte27, accepted empty differs; CFR uses get_check3 defaults, strictness and abort handling. CFR default byte is present only for server >4.5.6.0.0.1. ABORT has no ID and only affects request_pending, not item/spell/direction. Consumers validate matching ID/type, consume once and reject stale replies; test replacement, disconnect/kick and no pending request explicitly.
- Chat editor capacity is `MSG_LEN - strlen(cname) - 5 - 3 - 8`, with edit limit one less. Test typing/paste/history/macros at 0/limit−1/limit/limit+1 and after transformations. Preserve colour conversion, colon escaping, channel prefixes, local `%:`/`%%:` versus doubled-colon forwarding and recognizer ordering. Recall may filter/bundle its display; underlying occurrence order is unchanged.
- Social names use 79-byte editors; guild minimum-level editor uses 4 bytes and `atoi`, adder uses NAME_LEN. House tag/owner and admin operands retain their own byte capacities, binary operation prefixes and serializer limits. Do not normalize all fields into a universal Unicode character count. Incoming source bytes and FF colour/reset controls remain lossless.
- Party/guild mutations return to their menu after canceled name/confirmation; house selected children normally close their menu even after cancellation, including rejected deletion; tag ignores the editor result and still sends T after Escape. House root/owner-menu Ctrl-Q only breaks switch and does not exit, unlike Escape. Guild creation is not complete before the server cost confirmation. Privilege/membership/s_NO_PK/version gates remain supported conditional outcomes.
- Admin root Ctrl-Q only breaks its switch while the loop is `i != ESCAPE`; it does not exit like Escape. MASTER_PLAYER keys 1–8 mean offline editor, acquirement, wrath, static, unstatic, delete, telekinesis and broadcast. Their editors ignore cancellation and still dispatch a prefix-only command; dispatch leaves the whole menu. /xguild_adders returns messages (GUILD_ADDERS_LIST), not a document. Local DM Script> accepts Lua text, not a file path. TEST_CLIENT a/b are diagnostic outputs and c is a no-op, not ordinary DM game effects. This source discrepancy is recorded separately in the improvements log.

## Outcome index

Each row maps a concrete source owner to one ledger allocation. Full expected results, dependencies and evidence are in the JSON, including cancellation per caller.

| ID | Stage | Source owner | Expected result |
| --- | --- | --- | --- |
| `capability.information.read-sheet` | B | `src/client/c-cmd.c:cmd_character` | Read current character abilities, identity and stat sheet from decoded state, preserving glyphs, colours and missing/versioned values. |
| `capability.information.read-history` | B | `src/client/c-cmd.c:cmd_character` | Read server history lines at their original indices; updates replace only the addressed line and session reset clears old character content. |
| `capability.information.read-equipment-flags` | B | `src/client/c-cmd.c:cmd_character` | Read equipment resistance/ability flag matrix and its horizontal/vertical view without changing equipment. |
| `capability.information.navigate-sheet` | B | `src/client/c-cmd.c:cmd_character` | h/H cycles the three character pages; 2/8 selects help topics (not pages), 4/6 changes selection, v toggles equipment orientation only on page 2. |
| `capability.information.close-sheet` | B | `src/client/c-cmd.c:cmd_character` | q/Q/Escape/C closes the sheet and restores its exact live or final-review parent without issuing a gameplay command. |
| `capability.messages.recall-read` | B | `src/client/c-xtra2.c:do_cmd_messages` | Read recall message history with original ordering, channel/control markers and owner-specific filtering/bundling; do not replace stored occurrences with the display projection. |
| `capability.messages.recall-navigate` | B | `src/client/c-xtra2.c:do_cmd_messages` | Navigate recall recall by line/page/top/bottom and horizontal offset while retaining the current history position and updates. |
| `capability.messages.recall-search` | B | `src/client/c-xtra2.c:do_cmd_messages` | Search/highlight recall recall with the exact case/direction and optional REGEX_SEARCH rules; failed or canceled search preserves the recall owner. |
| `capability.messages.recall-close` | B | `src/client/c-xtra2.c:do_cmd_messages` | Close recall recall and restore gameplay or final-review caller and its queue policy. |
| `capability.messages.important-read` | B | `src/client/c-xtra2.c:do_cmd_messages_important` | Read important message history with original ordering, channel/control markers and owner-specific filtering/bundling; do not replace stored occurrences with the display projection. |
| `capability.messages.important-navigate` | B | `src/client/c-xtra2.c:do_cmd_messages_important` | Navigate important recall by line/page/top/bottom and horizontal offset while retaining the current history position and updates. |
| `capability.messages.important-search` | B | `src/client/c-xtra2.c:do_cmd_messages_important` | Search/highlight important recall with the exact case/direction and optional REGEX_SEARCH rules; failed or canceled search preserves the recall owner. |
| `capability.messages.important-close` | B | `src/client/c-xtra2.c:do_cmd_messages_important` | Close important recall and restore gameplay or final-review caller and its queue policy. |
| `capability.chat.send` | B | `src/client/c-cmd.c:cmd_message` | Submit ordinary, private and explicit-channel chat through cmd_message and Send_msg, preserving channel/address decoration and transformed bytes; editor limit is MSG_LEN minus cname length minus 17 including the final terminator allowance. |
| `capability.chat.cancel` | D | `src/client/c-cmd.c:cmd_message` | Escape the chat editor sends no message and restores the actual store, lore, document, skill, map or gameplay caller; a parent permitting chat does not enable chat inside every request. |
| `capability.chat.history` | B | `src/client/c-cmd.c:cmd_message` | Recall/edit chat text using the production text editor and its chat mode/history rules; draft edits and input method/paste respect byte limits. |
| `capability.chat.substitute-items` | B | `src/client/c-cmd.c:cmd_message` | Expand inventory/equipment/floor/newest/bag/store shortcuts, colours, spacing and colon escaping in source order; preserve unavailable-slot and expansion-limit behavior, testing final transformed bytes. |
| `capability.chat.local-self` | B | `src/client/c-cmd.c:cmd_message` | Route %: to local messages and %%: to local chat, except doubled-colon escape forms; no Send_msg for consumed local forms. |
| `capability.chat.forward-slash` | B | `src/client/c-cmd.c:cmd_message` | Forward unmatched slash text, bare slash and malformed local recognizers through Send_msg once; server grammar and permission outcomes remain server-owned, with no invented local whitelist. |
| `capability.documents.read` | D | `src/client/c-files.c:peruse_file` | Retain document category, title, original attributed line bytes and logical indices, total lines, page-size markers and arrival order. Unknown markup and arbitrary OTHER content remain readable without guessed semantic filtering. No automatic wrap; wide rows use horizontal scroll. |
| `capability.documents.navigate` | D | `src/client/c-files.c:peruse_file` | Navigate server pages/lines/top/bottom/absolute line and horizontal scroll; preserve logical position, negotiated geometry and SPECIAL_LINE_POS asynchronous repositioning. |
| `capability.documents.search` | D | `src/client/c-files.c:peruse_file` | Send forward/reverse text and optional regexp searches with the original category and logical line; preserve failed-search position and old-server search omission. |
| `capability.documents.refresh-players` | D | `src/client/c-files.c:peruse_file` | While PLAYER perusal is open, membership change interrupts input and requests refresh without losing owner state or repeating an unrelated command. |
| `capability.documents.close` | D | `src/client/c-files.c:peruse_file` | Send SPECIAL_FILE_NONE, reset local type/line count, restore the actual parent and macro state and flush according to peruse_file. Reopen initializes the correct content identity. |
| `capability.documents.open-unique` | D | `src/client/c-cmd.c:cmd_check_misc` | Open unique knowledge with all/alive/boss filters and strongest-slain shortcut via SPECIAL_FILE_UNIQUE and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-artifact` | D | `src/client/c-cmd.c:cmd_check_misc` | Open found artifacts (distinct from local artifact lore) via SPECIAL_FILE_ARTIFACT and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-player` | D | `src/client/c-cmd.c:cmd_check_misc` | Open players online with live membership refresh via SPECIAL_FILE_PLAYER and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-other` | D | `src/client/c-cmd.c:cmd_check_misc` | Open arbitrary server content and unknown titles via SPECIAL_FILE_OTHER and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-scores` | D | `src/client/c-cmd.c:cmd_check_misc` | Open high scores via SPECIAL_FILE_SCORES and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-help` | D | `src/client/c-cmd.c:cmd_check_misc` | Open server help, distinct from local Guide via SPECIAL_FILE_HELP and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-player-equip` | D | `src/client/c-cmd.c:cmd_check_misc` | Open other-player equipment via SPECIAL_FILE_PLAYER_EQUIP and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-log` | D | `src/client/c-cmd.c:cmd_check_misc` | Open privileged server log via SPECIAL_FILE_LOG and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-deaths` | D | `src/client/c-cmd.c:cmd_check_misc` | Open recent deaths via SPECIAL_FILE_DEATHS and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-server-setting` | D | `src/client/c-cmd.c:cmd_check_misc` | Open server settings via SPECIAL_FILE_SERVER_SETTING and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-monster` | D | `src/client/c-cmd.c:cmd_check_misc` | Open monster knowledge with glyph/minimum-level/unique filter encoding via SPECIAL_FILE_MONSTER and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-object` | D | `src/client/c-cmd.c:cmd_check_misc` | Open known objects filtered by type byte via SPECIAL_FILE_OBJECT and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-house` | D | `src/client/c-cmd.c:cmd_check_misc` | Open house locations and ownership via SPECIAL_FILE_HOUSE and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-trap` | D | `src/client/c-cmd.c:cmd_check_misc` | Open known traps via SPECIAL_FILE_TRAP and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-recall` | D | `src/client/c-cmd.c:cmd_check_misc` | Open recall depths and towns via SPECIAL_FILE_RECALL and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.documents.open-motd2` | D | `src/client/c-cmd.c:cmd_check_misc` | Open news (distinct from setup MOTD) via SPECIAL_FILE_MOTD2 and complete read/navigation/search/close through the native raw document surface; preserve category-specific input filters, permission failures and source identity. |
| `capability.information.open-index` | D | `src/client/c-cmd.c:cmd_check_misc` | Open knowledge index through ~ or case-insensitive exact /know; each selected report returns to its invoking menu. |
| `capability.information.extended-report` | D | `src/client/c-cmd.c:cmd_check_misc` | Send /ex from knowledge index as a direct server message; SPECIAL_FILE_EXTRAINFO is compiled out and does not identify this result. |
| `capability.information.local-time` | D | `src/client/c-cmd.c:cmd_message` | Case-insensitive exact /ctime reports local time without a network message. |
| `capability.information.client-version` | D | `src/client/c-cmd.c:cmd_message` | /cver and /cversion report client version and OS locally without Send_msg. |
| `capability.information.lag-read` | D | `src/client/c-cmd.c:cmd_lagometer` | Read ping samples and enabled state. |
| `capability.information.lag-enable` | D | `src/client/c-cmd.c:cmd_lagometer` | 1 enables lagometer. |
| `capability.information.lag-disable` | D | `src/client/c-cmd.c:cmd_lagometer` | 2 disables full and mini lagometer. |
| `capability.information.lag-clear` | D | `src/client/c-cmd.c:cmd_lagometer` | c clears the 60 ping samples without reconnecting. |
| `capability.information.lag-close` | D | `src/client/c-cmd.c:cmd_lagometer` | Escape/Ctrl-Q/Ctrl-I exits, restores parent and flushes its queue. |
| `capability.information.local-file-read` | D | `src/client/c-cmd.c:browse_local_file` | Read local-file from local source records/files, retaining original text/colour/glyph identity. Missing data/file errors stay visible; do not fabricate server document packets. |
| `capability.information.local-file-navigate` | D | `src/client/c-cmd.c:browse_local_file` | Select and page local-file entries with source-specific indices, offsets and remembered per-file state; resize preserves selection and scroll. |
| `capability.information.local-file-search` | D | `src/client/c-cmd.c:browse_local_file` | Search/filter local-file using its own input rules; canceled/failed search returns to this owner without selecting an unrelated entry. |
| `capability.information.local-file-close` | D | `src/client/c-cmd.c:browse_local_file` | Close local-file and restore the actual parent, selection and baseline macro/flush rules; teardown clears session-owned data. |
| `capability.information.notes-read` | D | `src/client/c-cmd.c:cmd_notes` | Read notes from local source records/files, retaining original text/colour/glyph identity. Missing data/file errors stay visible; do not fabricate server document packets. |
| `capability.information.notes-navigate` | D | `src/client/c-cmd.c:cmd_notes` | Select and page notes entries with source-specific indices, offsets and remembered per-file state; resize preserves selection and scroll. |
| `capability.information.notes-search` | D | `src/client/c-cmd.c:cmd_notes` | Search/filter notes using its own input rules; canceled/failed search returns to this owner without selecting an unrelated entry. |
| `capability.information.notes-close` | D | `src/client/c-cmd.c:cmd_notes` | Close notes and restore the actual parent, selection and baseline macro/flush rules; teardown clears session-owned data. |
| `capability.information.artifact-lore-read` | D | `src/client/c-cmd.c:artifact_lore` | Read artifact-lore from local source records/files, retaining original text/colour/glyph identity. Missing data/file errors stay visible; do not fabricate server document packets. |
| `capability.information.artifact-lore-navigate` | D | `src/client/c-cmd.c:artifact_lore` | Select and page artifact-lore entries with source-specific indices, offsets and remembered per-file state; resize preserves selection and scroll. |
| `capability.information.artifact-lore-search` | D | `src/client/c-cmd.c:artifact_lore` | Search/filter artifact-lore using its own input rules; canceled/failed search returns to this owner without selecting an unrelated entry. |
| `capability.information.artifact-lore-close` | D | `src/client/c-cmd.c:artifact_lore` | Close artifact-lore and restore the actual parent, selection and baseline macro/flush rules; teardown clears session-owned data. |
| `capability.information.artifact-lore-details` | D | `src/client/c-cmd.c:artifact_lore` | Toggle lore versus stats for the selected stable entity; preserve source-generated formatted details. |
| `capability.information.artifact-lore-paste-title` | D | `src/client/c-cmd.c:artifact_lore` | Paste selected lore title to chat once. |
| `capability.information.artifact-lore-paste-current` | D | `src/client/c-cmd.c:artifact_lore` | Paste current lore detail to chat once with source colour/continuation bytes. |
| `capability.information.artifact-lore-paste-all` | D | `src/client/c-cmd.c:artifact_lore` | Paste complete lore detail to chat in source order, preserving line limits and continuation formatting. |
| `capability.information.monster-lore-read` | D | `src/client/c-cmd.c:monster_lore` | Read monster-lore from local source records/files, retaining original text/colour/glyph identity. Missing data/file errors stay visible; do not fabricate server document packets. |
| `capability.information.monster-lore-navigate` | D | `src/client/c-cmd.c:monster_lore` | Select and page monster-lore entries with source-specific indices, offsets and remembered per-file state; resize preserves selection and scroll. |
| `capability.information.monster-lore-search` | D | `src/client/c-cmd.c:monster_lore` | Search/filter monster-lore using its own input rules; canceled/failed search returns to this owner without selecting an unrelated entry. |
| `capability.information.monster-lore-close` | D | `src/client/c-cmd.c:monster_lore` | Close monster-lore and restore the actual parent, selection and baseline macro/flush rules; teardown clears session-owned data. |
| `capability.information.monster-lore-details` | D | `src/client/c-cmd.c:monster_lore` | Toggle lore versus stats for the selected stable entity; preserve source-generated formatted details. |
| `capability.information.monster-lore-paste-title` | D | `src/client/c-cmd.c:monster_lore` | Paste selected lore title to chat once. |
| `capability.information.monster-lore-paste-current` | D | `src/client/c-cmd.c:monster_lore` | Paste current lore detail to chat once with source colour/continuation bytes. |
| `capability.information.monster-lore-paste-all` | D | `src/client/c-cmd.c:monster_lore` | Paste complete lore detail to chat in source order, preserving line limits and continuation formatting. |
| `capability.information.spoilers` | D | `src/client/c-cmd.c:cmd_spoilers` | Choose each supported k/e/r/E/a/v/d/f/t/s spoiler report and open the corresponding local file with original W/W2/G/S parsing and per-file state; missing files and canceled chooser restore parent. |
| `capability.request.answer-amt` | C | `src/client/nclient.c:Receive_request_amt` | Answer AMT request with original ID and exactly one typed reply. Quantity max and amount syntax; cancel returns integer zero, still replied. |
| `capability.request.cancel-amt` | C | `src/client/nclient.c:Receive_request_amt` | Cancel AMT request with original ID and exactly one typed reply. Quantity max and amount syntax; cancel returns integer zero, still replied. |
| `capability.request.answer-num` | C | `src/client/nclient.c:Receive_request_num` | Answer NUM request with original ID and exactly one typed reply. Predefined/min/max bounded number; cancel returns zero, still replied, even when zero is outside the supplied bounds. |
| `capability.request.cancel-num` | C | `src/client/nclient.c:Receive_request_num` | Cancel NUM request with original ID and exactly one typed reply. Predefined/min/max bounded number; cancel returns zero, still replied, even when zero is outside the supplied bounds. |
| `capability.request.answer-str` | C | `src/client/nclient.c:Receive_request_str` | Answer STR request with original ID and exactly one typed reply. Prefilled text editor MAX_CHARS_WIDE-1; accepted empty string differs from cancellation byte 27. |
| `capability.request.cancel-str` | C | `src/client/nclient.c:Receive_request_str` | Cancel STR request with original ID and exactly one typed reply. Prefilled text editor MAX_CHARS_WIDE-1; accepted empty string differs from cancellation byte 27. |
| `capability.request.answer-cfr` | C | `src/client/nclient.c:Receive_request_cfr` | Answer CFR request with original ID and exactly one typed reply. Default-aware get_check3; preserve strict/default yes/no behavior. Server >4.5.6.0.0.1 supplies default byte, older servers use FALSE. |
| `capability.request.cancel-cfr` | C | `src/client/nclient.c:Receive_request_cfr` | Cancel CFR request with original ID and exactly one typed reply. Default-aware get_check3; preserve strict/default yes/no behavior. Server >4.5.6.0.0.1 supplies default byte, older servers use FALSE. |
| `capability.request.abort-pending` | C | `src/client/nclient.c:Receive_request_abort` | REQUEST_ABORT carries no ID and affects only a currently pending generic request. Preserve primitive-specific interruption/reply and clear pending/abort on completion; do not cancel item/spell/direction or replay a stale reply after reconnect. |
| `capability.social.create-party` | D | `src/client/c-cmd.c:cmd_party` | Create or rename ordinary party with PARTY_CREATE and 79-byte name. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.create-iron-team` | D | `src/client/c-cmd.c:cmd_party` | Create or rename iron team with PARTY_CREATE_IRONTEAM. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.add-party-member` | D | `src/client/c-cmd.c:cmd_party` | Send PARTY_ADD for named player. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.join-party` | D | `src/client/c-cmd.c:cmd_party` | Send PARTY_ADD for named party; retain >4.4.7.0.0.0 presentation branch. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.remove-party-member` | D | `src/client/c-cmd.c:cmd_party` | Send PARTY_DELETE for named member. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.leave-party` | D | `src/client/c-cmd.c:cmd_party` | Send PARTY_REMOVE_ME with empty operand without invented confirmation. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.close-iron-team` | D | `src/client/c-cmd.c:cmd_party` | Confirm default-no closure only for an open iron team, then PARTY_CLOSE. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.hostile` | D | `src/client/c-cmd.c:cmd_party` | When !s_NO_PK, send PARTY_HOSTILE for player/party; no invented confirmation. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.peace` | D | `src/client/c-cmd.c:cmd_party` | When !s_NO_PK, send PARTY_PEACE for named target. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.create-guild` | D | `src/client/c-cmd.c:cmd_party` | Send GUILD_CREATE with name and complete server cost confirmation before claiming creation. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.add-guild-member` | D | `src/client/c-cmd.c:cmd_party` | Send GUILD_ADD for named player. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.join-guild` | D | `src/client/c-cmd.c:cmd_party` | Send GUILD_ADD for named guild. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.remove-guild-member` | D | `src/client/c-cmd.c:cmd_party` | Send GUILD_DELETE for named player. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.leave-guild` | D | `src/client/c-cmd.c:cmd_party` | Default-no confirmation then GUILD_REMOVE_ME with empty operand. Canceled text/confirmation sends no corresponding mutation; return to social menu and wait for authoritative server result. |
| `capability.social.read-party` | D | `src/client/c-cmd.c:cmd_party` | Read party name/member/owner/mode and party stats using versioned received fields. |
| `capability.social.read-guild` | D | `src/client/c-cmd.c:cmd_party` | Read guild name/member/owner and configuration updates, preserving mode and privilege visibility. |
| `capability.social.close` | D | `src/client/c-cmd.c:cmd_party` | Escape/Ctrl-Q closes social menu, restores inkey_msg and flushes queue. |
| `capability.social.guild-allow-adders` | D | `src/client/c-cmd.c:cmd_guild_options` | Toggle allow-adders flag. Preserve guild membership/master checks, flags and >4.5.2.0.0.0 entry gate; canceled child remains in options. |
| `capability.social.guild-auto-readd` | D | `src/client/c-cmd.c:cmd_guild_options` | Toggle auto-readd flag. Preserve guild membership/master checks, flags and >4.5.2.0.0.0 entry gate; canceled child remains in options. |
| `capability.social.guild-minimum-level` | D | `src/client/c-cmd.c:cmd_guild_options` | Edit minimum level with exact numeric conversion. Preserve guild membership/master checks, flags and >4.5.2.0.0.0 entry gate; canceled child remains in options. |
| `capability.social.guild-list-adders` | D | `src/client/c-cmd.c:cmd_guild_options` | Guild master key a sends direct /xguild_adders. GUILD_ADDERS_LIST server branch emits messages listing adders or no-adders; disabled build emits unavailable message. No SPECIAL_FILE document is requested. |
| `capability.social.guild-toggle-adder` | D | `src/client/c-cmd.c:cmd_guild_options` | Prompt named player to toggle as guild adder. Preserve guild membership/master checks, flags and >4.5.2.0.0.0 entry gate; canceled child remains in options. |
| `capability.housing.trade` | D | `src/client/c-cmd.c:cmd_purchase_house` | Confirm purchase/sale then Send_purchase_house; rejected confirmation returns to menu. Preserve selected adjacent direction and restore parent exactly once. |
| `capability.housing.owner-player` | D | `src/client/c-cmd.c:cmd_house_chown` | Prompt player name and send O1<name>. Preserve selected adjacent direction and restore parent exactly once. |
| `capability.housing.owner-guild` | D | `src/client/c-cmd.c:cmd_house_chown` | Send O2 for guild ownership. Preserve selected adjacent direction and restore parent exactly once. |
| `capability.housing.access` | D | `src/client/c-cmd.c:cmd_house_chmod` | Collect party/class/race/winner/fallen-winner/no-ghost flags and minimum level, send M<flags><level>. Preserve selected adjacent direction and restore parent exactly once. |
| `capability.housing.paint` | D | `src/client/c-cmd.c:cmd_house_paint` | Select eligible potion and send P<item>. Preserve selected adjacent direction and restore parent exactly once. |
| `capability.housing.tag` | D | `src/client/c-cmd.c:cmd_house_tag` | House key 5 edits at most 19 tag bytes, ignores the get_string boolean and always sends T plus resulting text, including prefix-only T after Escape; the house menu then closes. |
| `capability.housing.enter-store` | D | `src/client/c-cmd.c:cmd_house_store` | Send S to enter player store. Preserve selected adjacent direction and restore parent exactly once. |
| `capability.housing.knock` | D | `src/client/c-cmd.c:cmd_house_knock` | Send H to knock. Preserve selected adjacent direction and restore parent exactly once. |
| `capability.housing.delete` | D | `src/client/c-cmd.c:cmd_house_kill` | Admin-only D asks default-no destruction confirmation; yes sends K, no/Escape sends no K. Both outcomes close the house menu because the caller assigns Escape after cmd_house_kill. |
| `capability.housing.cancel` | D | `src/client/c-cmd.c:cmd_purchase_house` | Escape cancels the house direction/menu without mutation. Ctrl-Q only breaks the house/menu switch and does not exit the while loop; owner-selection submenu has the same distinction. Selected child text/item cancellation normally closes the outer menu; tag still sends its T prefix. |
| `capability.special-store.text` | D | `src/client/nclient.c:Receive_store_special_str` | Apply attributed byte strings at original cell coordinates, including erasure of underlying raw pictures. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.glyph` | D | `src/client/nclient.c:Receive_store_special_char` | Apply one attributed glyph at its original cell coordinate. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.clear` | D | `src/client/nclient.c:Receive_store_special_clr` | Clear inclusive normal line region, retaining untouched content. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.force-clear` | D | `src/client/nclient.c:Receive_store_special_clr` | Decode start+100 forced clear and erase raw graphical visuals as well as text. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.wheel` | D | `src/client/nclient.c:Receive_store_special_anim` | Animate opcode 0 to server-selected wheel result. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.slots` | D | `src/client/nclient.c:Receive_store_special_anim` | Animate opcode 1 three slots to server-selected results; retain ANIM_SLOT_SPINALL variants. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.in-between` | D | `src/client/nclient.c:Receive_store_special_anim` | Animate opcode 2 dice result with ASCII/graphics alternatives. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.dice` | D | `src/client/nclient.c:Receive_store_special_anim` | Apply opcode 3 dice settle delay without replaying a wager. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.cards` | D | `src/client/nclient.c:Receive_store_special_anim` | Apply opcode 4 card value/colour/position including hidden stacks and raw-picture extents. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.unknown-animation` | D | `src/client/nclient.c:Receive_store_special_anim` | Retain unknown animation opcode and all four u16 operands; baseline default reports an unknown-animation error and draws nothing, with no invented action or dropped protocol identity. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.special-store.recreate` | D | `src/client/nclient.c:Receive_store_info` | Close/kick/reopen resets canvas identity. Resize/DPI/device recreation reconstructs current visible canvas without replaying sound, animation completion, replies or transactions. Retain ordered partial operations and shopping gate; one uniform complete-fit canvas without scroll, with controls outside it and raw multi-cell pictures in the same transform. |
| `capability.server-flow.go` | D | `src/server/xtra1.c:RID_GO_MOVE` | Go challenge/start/move/decline via RID_GO, RID_GO_START and RID_GO_MOVE; ENABLE_GO_GAME and active-store checks; preserve engine text and canvas. |
| `capability.server-flow.wheel` | D | `src/server/xtra1.c:RID_SPIN_WHEEL` | Wheel number request with AMT backward compatibility for 4.9.2; one wager, server-selected result. |
| `capability.server-flow.craps` | D | `src/server/xtra1.c:RID_CRAPS` | Craps key replies and repeated round state, escape/no-bet and settlement exactly once. |
| `capability.server-flow.blackjack` | D | `src/server/xtra1.c:RID_BLACKJACK1` | Blackjack RID_BLACKJACK1/2/3 ordered choice stages, stake and final settlement exactly once. |
| `capability.server-flow.guild-rename` | D | `src/server/xtra1.c:RID_GUILD_RENAME` | Confirm then enter guild name; server truncates to 40 bytes and ignores Escape/empty. |
| `capability.server-flow.item-order` | C | `src/server/xtra1.c:RID_ITEM_ORDER` | ENABLE_ITEM_ORDER name/specification then confirmation; quote, rejection, cancellation and final order remain server-authoritative. |
| `capability.server-flow.mail-item` | C | `src/server/xtra1.c:RID_SEND_ITEM` | Recipient/item and RID_SEND_ITEM/2 plus payment PAY/PAY2 chain; preserve request_extra, changed/disappeared item, confirmation and delivery result. |
| `capability.server-flow.mail-gold` | C | `src/server/xtra1.c:RID_SEND_GOLD` | Recipient and RID_SEND_GOLD/2 confirmation chain; exactly one gold delivery and fee outcome. |
| `capability.server-flow.mail-fee` | C | `src/server/xtra1.c:RID_SEND_FEE` | RID_SEND_FEE and RID_SEND_FEE_PAY acceptance/refusal; preserve payment and inventory-capacity failures. |
| `capability.server-flow.repair-armour` | C | `src/server/xtra1.c:RID_REPAIR_ARMOUR` | Confirm selected armour repair once; refusal/cancel does not repair. |
| `capability.server-flow.repair-weapon` | C | `src/server/xtra1.c:RID_REPAIR_WEAPON` | Confirm selected weapon repair once; refusal/cancel does not repair. |
| `capability.server-flow.lose-memories` | C | `src/server/xtra1.c:RID_LOSE_MEMORIES_I` | I/II confirmation and skill-name child preserve request identity and exact skill-loss result. |
| `capability.server-flow.donate` | C | `src/server/xtra1.c:RID_SR_DONATE` | SOLO_REKING donation amount with bounds, zero cancel and authoritative donation result. |
| `capability.server-flow.contact-owner` | D | `src/server/xtra1.c:RID_CONTACT_OWNER` | Contact owner text through its server request consumer and exact message result. |
| `capability.server-flow.quest-reply` | D | `src/server/xtra1.c:RID_QUEST` | Quest string/confirmation and acquire confirmation ranges, 30-byte reply truncation and ID-offset routing; wrong ID/type and stale duplicate produce no quest effect. |
| `capability.special-store.wager` | D | `src/client/c-cmd.c:cmd_message` | /wager case-insensitive prefix queries or sets process-local default wager; atol suffix accepts no delimiter and trailing junk, clamp to 1..PY_MAX_GOLD. Query is not a bet; retain source inkey_msg early-return behavior. |
| `capability.admin.level-static` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level static through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-unstatic` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level unstatic through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-add-dungeon` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level add-dungeon through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-remove-dungeon` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level remove-dungeon through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-create-town` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level create-town through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-perma-static` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level perma-static through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-unperma-static` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level unperma-static through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-save-module` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level save-module through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-load-module` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level load-module through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-blank-module` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level blank-module through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.level-module-entry` | D | `src/client/c-cmd.c:cmd_master_aux_level` | Perform privileged level module-entry through cmd_master_aux_level with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-wall` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build wall through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-permanent-wall` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build permanent-wall through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-tree` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build tree through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-dead-tree` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build dead-tree through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-grass` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build grass through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-dirt` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build dirt through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-floor` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build floor through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-house-door` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build house-door through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-sign` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build sign through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-feature` | D | `src/client/c-cmd.c:cmd_master_aux_build` | Perform privileged build feature through cmd_master_aux_build with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.build-disable` | D | `src/client/c-cmd.c:cmd_master_aux_build` | a sets FEAT_FLOOR with F to disable build mode. Preserve MASTER_BUILD C-string operand semantics including embedded zero termination, numeric cancellation/defaults and server privilege checks. TEST_CLIENT only controls menu labels for A–D; their switch branches remain compiled. |
| `capability.admin.build-set-info` | D | `src/client/c-cmd.c:cmd_master_aux_build` | A sends i plus four little-endian info bytes. Preserve MASTER_BUILD C-string operand semantics including embedded zero termination, numeric cancellation/defaults and server privilege checks. TEST_CLIENT only controls menu labels for A–D; their switch branches remain compiled. |
| `capability.admin.build-set-info2` | D | `src/client/c-cmd.c:cmd_master_aux_build` | B sends j plus four little-endian info2 bytes. Preserve MASTER_BUILD C-string operand semantics including embedded zero termination, numeric cancellation/defaults and server privilege checks. TEST_CLIENT only controls menu labels for A–D; their switch branches remain compiled. |
| `capability.admin.build-info-mode` | D | `src/client/c-cmd.c:cmd_master_aux_build` | C sends I plus four little-endian info bytes. Preserve MASTER_BUILD C-string operand semantics including embedded zero termination, numeric cancellation/defaults and server privilege checks. TEST_CLIENT only controls menu labels for A–D; their switch branches remain compiled. |
| `capability.admin.build-info2-mode` | D | `src/client/c-cmd.c:cmd_master_aux_build` | D sends J plus four little-endian info2 bytes. Preserve MASTER_BUILD C-string operand semantics including embedded zero termination, numeric cancellation/defaults and server privilege checks. TEST_CLIENT only controls menu labels for A–D; their switch branches remain compiled. |
| `capability.admin.summon-orcs` | D | `src/client/c-cmd.c:cmd_master_aux_summon` | Perform privileged summon orcs through cmd_master_aux_summon with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.summon-low-undead` | D | `src/client/c-cmd.c:cmd_master_aux_summon` | Perform privileged summon low-undead through cmd_master_aux_summon with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.summon-high-undead` | D | `src/client/c-cmd.c:cmd_master_aux_summon` | Perform privileged summon high-undead through cmd_master_aux_summon with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.summon-depth` | D | `src/client/c-cmd.c:cmd_master_aux_summon` | Perform privileged summon depth through cmd_master_aux_summon with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.summon-name` | D | `src/client/c-cmd.c:cmd_master_aux_summon` | Perform privileged summon name through cmd_master_aux_summon with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.summon-obliterate` | D | `src/client/c-cmd.c:cmd_master_aux_summon` | Perform privileged summon obliterate through cmd_master_aux_summon with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.summon-disable` | D | `src/client/c-cmd.c:cmd_master_aux_summon` | Perform privileged summon disable through cmd_master_aux_summon with exact MASTER selector/operand bytes and all nested prompts. Preserve caller-specific Escape/default return, bounds, permission rejection and optional gates. Summon selection/placement (one/group, here/random/mode) are separate required scenarios where applicable; no repeated mutation on redraw. |
| `capability.admin.player-edit` | D | `src/client/c-cmd.c:cmd_master_aux_player` | Request offline editor through MASTER_PLAYER with prefix E. Target editor is 15 bytes (broadcast 69); get_string boolean is ignored, so Escape still sends the prefix with empty operand. Broadcast converts { to FF only within the first 60 buffer bytes. Any sent command closes the whole DM menu; server permission/target rejection is not success. |
| `capability.admin.player-acquirement` | D | `src/client/c-cmd.c:cmd_master_aux_player` | Request acquirement through MASTER_PLAYER with prefix A. Target editor is 15 bytes (broadcast 69); get_string boolean is ignored, so Escape still sends the prefix with empty operand. Broadcast converts { to FF only within the first 60 buffer bytes. Any sent command closes the whole DM menu; server permission/target rejection is not success. |
| `capability.admin.player-kill` | D | `src/client/c-cmd.c:cmd_master_aux_player` | Request wrath/kill (optional ! no-ghost target) through MASTER_PLAYER with prefix k. Target editor is 15 bytes (broadcast 69); get_string boolean is ignored, so Escape still sends the prefix with empty operand. Broadcast converts { to FF only within the first 60 buffer bytes. Any sent command closes the whole DM menu; server permission/target rejection is not success. |
| `capability.admin.player-static` | D | `src/client/c-cmd.c:cmd_master_aux_player` | Request make player static through MASTER_PLAYER with prefix S. Target editor is 15 bytes (broadcast 69); get_string boolean is ignored, so Escape still sends the prefix with empty operand. Broadcast converts { to FF only within the first 60 buffer bytes. Any sent command closes the whole DM menu; server permission/target rejection is not success. |
| `capability.admin.player-unstatic` | D | `src/client/c-cmd.c:cmd_master_aux_player` | Request make player unstatic through MASTER_PLAYER with prefix U. Target editor is 15 bytes (broadcast 69); get_string boolean is ignored, so Escape still sends the prefix with empty operand. Broadcast converts { to FF only within the first 60 buffer bytes. Any sent command closes the whole DM menu; server permission/target rejection is not success. |
| `capability.admin.player-delete` | D | `src/client/c-cmd.c:cmd_master_aux_player` | Request delete player through MASTER_PLAYER with prefix r. Target editor is 15 bytes (broadcast 69); get_string boolean is ignored, so Escape still sends the prefix with empty operand. Broadcast converts { to FF only within the first 60 buffer bytes. Any sent command closes the whole DM menu; server permission/target rejection is not success. |
| `capability.admin.player-telekinesis` | D | `src/client/c-cmd.c:cmd_master_aux_player` | Request telekinesis through MASTER_PLAYER with prefix t. Target editor is 15 bytes (broadcast 69); get_string boolean is ignored, so Escape still sends the prefix with empty operand. Broadcast converts { to FF only within the first 60 buffer bytes. Any sent command closes the whole DM menu; server permission/target rejection is not success. |
| `capability.admin.player-broadcast` | D | `src/client/c-cmd.c:cmd_master_aux_player` | Request broadcast through MASTER_PLAYER with prefix B. Target editor is 15 bytes (broadcast 69); get_string boolean is ignored, so Escape still sends the prefix with empty operand. Broadcast converts { to FF only within the first 60 buffer bytes. Any sent command closes the whole DM menu; server permission/target rejection is not success. |
| `capability.admin.generate-vault` | D | `src/client/c-cmd.c:cmd_master_aux_generate_vault` | Choose vault by numeric ID or name and send exact MASTER_GENERATE operand. |
| `capability.admin.server-script` | D | `src/client/c-cmd.c:cmd_script_exec` | Prompt server script and dispatch MASTER_SCRIPTS once. |
| `capability.admin.upload-script` | D | `src/client/c-cmd.c:cmd_script_upload` | Select local script for server upload, preserving file/transfer errors and cancellation. |
| `capability.admin.local-script` | D | `src/client/c-cmd.c:cmd_script_exec_local` | Enter up to 80 bytes of Lua source at Script> and execute string_exec_lua locally once, displaying its result; Escape sends/executes nothing. This is a text prompt, not a file selector. |
| `capability.admin.close` | D | `src/client/c-cmd.c:cmd_master` | Escape closes the current DM owner; root Ctrl-Q only breaks the switch and stays in the loop. Successful MASTER_PLAYER dispatch closes the entire DM menu, including prefix-only dispatch after canceled text. Preserve each nested owner and hybrid macro restoration. |
| `capability.messages.read-live` | B | `src/client/nclient.c:Receive_message` | Read real-session messages with original channel/colour/control markers, clear-topline, live feed and full-history routing; identical incoming occurrences remain distinct even if recall bundles display. This B outcome needs live decode-to-view evidence beyond the synthetic A occurrence slice. |
| `capability.housing.claim-land` | D | `src/client/c-cmd.c:cmd_king` | Confirm default-no land ownership and send KING_OWN once; decline sends nothing and server rejection does not imply ownership. |
| `capability.information.bbs` | D | `src/client/c-cmd.c:cmd_BBS` | Send BBS request through the gameplay binding and preserve server-provided response; do not substitute a local document. |
| `capability.information.local-file-bookmarks` | D | `src/client/c-cmd.c:browse_local_file` | Add, replace, delete and revisit local-file marks with file-specific positions and exact d/a/A/D semantics; preserve canceled editors and repeated-open state. |
| `capability.social.guild-options-close` | D | `src/client/c-cmd.c:cmd_guild_options` | Escape/Ctrl-Q closes guild configuration, restores social parent and inkey_msg, clears guildcfg_mode and flushes its queue. |

## Inventory reconciliation

Quoted rows retain their original line numbers and source links (resolve relative links from the original path). Mappings are row-level discovery coverage, not proof that a shared primitive accepts every caller.

### Formatted/server-driven inventory

Original `.scratch/single-window-sdl3-client/research/formatted-and-server-driven-surfaces.md`, SHA-256 `123d9fe4aa501466cf8cd11c5eef285cfb215165aeb298b2d4bd1a40f70f4d72`.

**L5: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

This inventory answers which legacy surfaces deliver presentation-shaped data rather than ordinary game state, which can safely become semantic UI, and where the finished single-window client must keep a permanent lossless representation. The sources are the native client/server protocol and implementation in this repository; no secondary sources or HTML-prototype assumptions are used.

**L7: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

The governing decision is:

**L9: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

1. **Normal and wide stores, store metadata/actions, and typed request packets are semantic protocol.** Decode them directly into typed state and intents. Preserve all decoded fields, including currently presentation-oriented strings/attributes, but a terminal-cell copy is not their authoritative representation.

**L10: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

2. **`PKT_SPECIAL_LINE` documents and `PKT_STORE_SPECIAL_*` drawing operations are presentation protocols.** Their authoritative client state must permanently retain a lossless raw document/canvas model for the lifetime of the surface. Known categories may additionally expose versioned semantic projections, but those projections must never replace or filter the raw representation.

**L11: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

3. **Local Guide, spoiler/help-like files, and lore screens are distinct local-content surfaces.** The Guide and spoiler browsers must retain the original source lines plus parsed spans/navigation metadata. Artifact and monster lore are eligible for fully semantic models because their selector records and detail content are computed locally from typed setup/data, but chat-paste output remains a separately testable formatted projection.

**L12: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

4. **Unknown, future, and `SPECIAL_FILE_OTHER` content always renders through the lossless generic document path.** Unknown special-store animation opcodes likewise remain represented even if the client cannot interpret them semantically.

**L14: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

“Permanent” here means permanent in the product architecture and retained for the active surface/session—not persisted to disk forever. Raw protocol evidence must also be capturable by replay/acceptance tooling.

**L20: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

All implemented categories share one client perusal loop. Entering sets `special_line_type`, saves the prior screen, clears it, and requests the current page. Each navigation/search action sends `(type, logical start line, optional search string)` back to the server. Exit sends `SPECIAL_FILE_NONE`, clears the local type and line count, restores the prior screen, flushes queued events, and restores macro-interaction state. The player list alone can interrupt `inkey()` and request a refresh when membership changes. Navigation includes page/line movement, absolute line, top/bottom, forward/reverse text search, optional regexp search, horizontal scrolling, chat, inscriptions, screenshot, and a unique-list “strongest slain” shortcut [src/client/c-files.c:1951](../../../src/client/c-files.c#L1951) [src/client/c-files.c:1963](../../../src/client/c-files.c#L1963) [src/client/c-files.c:1985](../../../src/client/c-files.c#L1985) [src/client/c-files.c:2033](../../../src/client/c-files.c#L2033) [src/client/c-files.c:2051](../../../src/client/c-files.c#L2051) [src/client/c-files.c:2093](../../../src/client/c-files.c#L2093) [src/client/c-files.c:2141](../../../src/client/c-files.c#L2141) [src/client/c-files.c:2158](../../../src/client/c-files.c#L2158) [src/client/c-files.c:2171](../../../src/client/c-files.c#L2171) [src/client/c-files.c:2183](../../../src/client/c-files.c#L2183) [src/client/c-files.c:2205](../../../src/client/c-files.c#L2205) [src/client/c-files.c:2244](../../../src/client/c-files.c#L2244).

**L22: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

`PKT_SPECIAL_LINE` is not a record-oriented domain packet. Each response carries total logical line count, a logical/physical line discriminator, a base attribute, and an `ONAME_LEN` formatted byte string. `line == -1` is a stationary title; high synthetic line values negotiate page geometry; ordinary lines may contain inline `\377` color changes and are horizontally sliced only at render time. `PKT_SPECIAL_LINE_POS` can asynchronously reset the current logical position [src/client/nclient.c:5801](../../../src/client/nclient.c#L5801) [src/client/nclient.c:5825](../../../src/client/nclient.c#L5825) [src/client/nclient.c:5828](../../../src/client/nclient.c#L5828) [src/client/nclient.c:5885](../../../src/client/nclient.c#L5885) [src/client/nclient.c:5890](../../../src/client/nclient.c#L5890) [src/client/nclient.c:5910](../../../src/client/nclient.c#L5910) [src/client/nclient.c:5934](../../../src/client/nclient.c#L5934). The wire request itself is versioned: modern peers send type, 32-bit line and search string; intermediate and old peers omit search and/or use a 16-bit line [src/client/nclient.c:7979](../../../src/client/nclient.c#L7979).

**L24: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

The server turns file lines into presentation before sending: it removes `***** ` control records and blank lines, injects title/page-size markers, performs server-side searches, and colorizes log/death/account lines by inserting color codes [src/server/files.c:548](../../../src/server/files.c#L548) [src/server/files.c:599](../../../src/server/files.c#L599) [src/server/files.c:632](../../../src/server/files.c#L632) [src/server/files.c:683](../../../src/server/files.c#L683) [src/server/files.c:785](../../../src/server/files.c#L785) [src/server/files.c:828](../../../src/server/files.c#L828) [src/server/files.c:838](../../../src/server/files.c#L838) [src/server/files.c:876](../../../src/server/files.c#L876) [src/server/files.c:916](../../../src/server/files.c#L916). `Send_special_line` further prefixes a base color, maps attributes to legacy color characters, truncates to the protocol limit, and expands `\{` into the inline color marker [src/server/nserver.c:10537](../../../src/server/nserver.c#L10537) [src/server/nserver.c:10548](../../../src/server/nserver.c#L10548) [src/server/nserver.c:10582](../../../src/server/nserver.c#L10582) [src/server/nserver.c:10611](../../../src/server/nserver.c#L10611). Therefore the client cannot reconstruct the server's original file or typed records; its only complete truth is the exact post-wire title/line/page-marker stream.

**L26: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

The lossless active-document model must retain, without stripping or normalizing:

**L28: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

- category/type and request/search parameters;

**L29: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

- negotiated page-size markers and server-reported total line count;

**L30: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

- title and every received logical line, including line index, base attribute, original formatted bytes, inline color/reset codes, empty/erased state, and arrival order;

**L31: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

- server-driven position updates;

**L32: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

- horizontal offset and current logical position as interaction state.

**L34: documents.read/navigate/search/refresh-players/close and category open outcomes; item OTHER reuse items.details-*.**

A parsed span tree (plain text plus color runs, links if a known syntax supports them) is a derivative cache. Accessibility/search/export may consume it, but raw bytes and coordinates remain available to generic rendering and replay.

**L38: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

The enum is exhaustive from `NONE=0` through `EXTRAINFO=17` [src/common/defines.h:2047](../../../src/common/defines.h#L2047). The server dispatch is the authoritative mapping [src/server/nserver.c:14744](../../../src/server/nserver.c#L14744).

**L40: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

| Category | Actual source/meaning | Parsing eligibility and raw requirement |

**L42: documents.close; NONE is a lifecycle intent.**

| `SPECIAL_FILE_NONE` (0) | Lifecycle close. Clears the server's `special_file_type` and deletes its temporary info file [src/server/nserver.c:14745](../../../src/server/nserver.c#L14745). | Semantic close intent. No document of its own; retain the close event in replay evidence. |

**L43: documents.open-unique.**

| `SPECIAL_FILE_UNIQUE` (1) | Server-generated unique-monster knowledge. `line` also encodes mode in units of 100000 [src/server/nserver.c:14751](../../../src/server/nserver.c#L14751); the UI offers all/alive/boss choices [src/client/c-cmd.c:7787](../../../src/client/c-cmd.c#L7787). | Candidate for a versioned semantic projection because rows describe known uniques, but layout/text are the only payload. Raw document is mandatory and authoritative. |

**L44: documents.open-artifact.**

| `SPECIAL_FILE_ARTIFACT` (2) | Server-generated found-artifact list [src/server/nserver.c:14755](../../../src/server/nserver.c#L14755); opened by `cmd_artifacts` [src/client/c-cmd.c:2391](../../../src/client/c-cmd.c#L2391). | Same: semantic projection allowed, raw mandatory. Do not confuse with local Artifact Lore. |

**L45: documents.open-player.**

| `SPECIAL_FILE_PLAYER` (3) | Live server-generated players-online list [src/server/nserver.c:14758](../../../src/server/nserver.c#L14758), with client auto-refresh interruption semantics [src/client/c-files.c:2037](../../../src/client/c-files.c#L2037). | Strong projection candidate, but rows are still formatted-only protocol and may change while open. Raw mandatory. |

**L46: documents.open-other.**

| `SPECIAL_FILE_OTHER` (4) | Universal escape hatch (“can handle everything”) [src/common/defines.h:2054](../../../src/common/defines.h#L2054). Server code binds an arbitrary existing or generated file/title, signals `PKT_SPECIAL_OTHER`, then serves it through the shared file viewer [src/server/cmd4.c:3935](../../../src/server/cmd4.c#L3935) [src/server/cmd4.c:3942](../../../src/server/cmd4.c#L3942) [src/server/cmd4.c:3961](../../../src/server/cmd4.c#L3961); client entry is packet-driven [src/client/nclient.c:4495](../../../src/client/nclient.c#L4495). Known uses include item inspection/self-knowledge, exploration reports/history, guild roster, gambling rules, news/log/help files and arbitrary slash-command outputs. | Never generically semantic-parse. A title-specific adapter may add affordances but must be optional and versioned. Raw mandatory; unknown titles/content must remain fully usable. |

**L47: documents.open-scores.**

| `SPECIAL_FILE_SCORES` (5) | Server high-score display [src/server/nserver.c:14767](../../../src/server/nserver.c#L14767); opened by `cmd_high_scores` [src/client/c-cmd.c:2415](../../../src/client/c-cmd.c#L2415). | Projection candidate; raw mandatory because no typed score rows reach the client. |

**L48: documents.open-help.**

| `SPECIAL_FILE_HELP` (6) | Server help root (`tomenet.hlp` or rogue-like variant) [src/server/nserver.c:14770](../../../src/server/nserver.c#L14770) [src/server/files.c:951](../../../src/server/files.c#L951). Pressing `?` inside it exits and opens the local Guide [src/client/c-files.c:2205](../../../src/client/c-files.c#L2205) [src/client/c-files.c:2261](../../../src/client/c-files.c#L2261). | Treat as formatted hypertext/document, not domain rows. Parse recognized links/menu markers only as affordances; raw mandatory. |

**L49: documents.open-player-equip.**

| `SPECIAL_FILE_PLAYER_EQUIP` (7) | Other-player equipment display [src/server/nserver.c:14761](../../../src/server/nserver.c#L14761), entered with a preselected type before perusal [src/client/c-cmd.c:22](../../../src/client/c-cmd.c#L22). | Projection candidate; raw mandatory. |

**L50: documents.open-log.**

| `SPECIAL_FILE_LOG` (8) | Admin-only server log; still reachable from admin command menu [src/server/nserver.c:14773](../../../src/server/nserver.c#L14773). | Free-form operational text. No semantic assumption; raw mandatory. |

**L51: documents.open-deaths.**

| `SPECIAL_FILE_DEATHS` (9) | Recent-deaths log, with low-level omissions [src/server/nserver.c:14780](../../../src/server/nserver.c#L14780). | A best-effort death-entry projection is possible, but wording/colorization is not a stable schema. Raw mandatory. |

**L52: documents.open-server-setting.**

| `SPECIAL_FILE_SERVER_SETTING` (10) | Generated server-settings report [src/server/nserver.c:14797](../../../src/server/nserver.c#L14797). | Key/value projection is plausible only under a tested format version. Raw mandatory. |

**L53: documents.open-monster.**

| `SPECIAL_FILE_MONSTER` (11) | Killed/learnt monster knowledge. `line` multiplexes glyph/type, minimum level and uniques-only mode [src/server/nserver.c:14800](../../../src/server/nserver.c#L14800); client collects those filters before sending [src/client/c-cmd.c:7808](../../../src/client/c-cmd.c#L7808). | Projection candidate; raw mandatory. This is server knowledge, distinct from local Monster Lore. |

**L54: documents.open-object.**

| `SPECIAL_FILE_OBJECT` (12) | Known-object knowledge, filtered by item-type character encoded in `line` [src/server/nserver.c:14812](../../../src/server/nserver.c#L14812) [src/client/c-cmd.c:7829](../../../src/client/c-cmd.c#L7829). | Projection candidate; raw mandatory. |

**L55: documents.open-house.**

| `SPECIAL_FILE_HOUSE` (13) | Generated house list [src/server/nserver.c:14819](../../../src/server/nserver.c#L14819). | Projection candidate, especially for coordinates/ownership, but raw mandatory. |

**L56: documents.open-trap.**

| `SPECIAL_FILE_TRAP` (14) | Known-traps report [src/server/nserver.c:14822](../../../src/server/nserver.c#L14822). | Projection candidate; raw mandatory. |

**L57: documents.open-recall.**

| `SPECIAL_FILE_RECALL` (15) | Recall depths/towns report [src/server/nserver.c:14825](../../../src/server/nserver.c#L14825); generated formatting includes locations, dungeon/tower names, levels and recall depths [src/server/cmd4.c:2351](../../../src/server/cmd4.c#L2351) [src/server/cmd4.c:2386](../../../src/server/cmd4.c#L2386). | Strong projection candidate, but raw mandatory until the protocol supplies typed locations. |

**L58: documents.open-motd2.**

| `SPECIAL_FILE_MOTD2` (16) | Server news/MOTD document [src/server/nserver.c:14790](../../../src/server/nserver.c#L14790); exposed as “News (Message of the day)” [src/client/c-cmd.c:7660](../../../src/client/c-cmd.c#L7660). | Formatted prose only. Raw mandatory. |

**L59: information.extended-report; reserved EXTRAINFO has no active surface.**

| `SPECIAL_FILE_EXTRAINFO` (17) | Reserved equivalent of `/ex`, explicitly not implemented [src/common/defines.h:2067](../../../src/common/defines.h#L2067); dispatch and client call are compiled out [src/server/nserver.c:14828](../../../src/server/nserver.c#L14828) [src/client/c-cmd.c:7895](../../../src/client/c-cmd.c#L7895). Current UI sends `/ex` instead. | Reserve the typed category and generic raw behavior for forward compatibility; do not claim a current surface. `/ex` currently belongs to normal message/slash-command output, not special-file parsing. |

**L61: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

No category is eligible for **replacement** by semantic parsing under the current protocol. Eligibility above means additive projection only.

**L67: ticket 09 store.enter/read-stock/read-actions/service/leave/kicked; special body maps special-store.*.**

`PKT_STORE` is already a typed row update keyed by `pos`: attribute, weight, count, price, display name, `tval`, `sval`, `pval`, and—on supported versions—a powers string. The client stores every field and schedules redraw [src/client/nclient.c:4548](../../../src/client/nclient.c#L4548) [src/client/nclient.c:4555](../../../src/client/nclient.c#L4555) [src/client/nclient.c:4572](../../../src/client/nclient.c#L4572) [src/client/nclient.c:4577](../../../src/client/nclient.c#L4577) [src/client/nclient.c:4589](../../../src/client/nclient.c#L4589). `PKT_STORE_WIDE`, intended for custom spellbooks, replaces the powers string with nine typed `xtra` spell slots while retaining the rest [src/server/nserver.c:9680](../../../src/server/nserver.c#L9680) [src/client/nclient.c:4595](../../../src/client/nclient.c#L4595) [src/client/nclient.c:4636](../../../src/client/nclient.c#L4636).

**L69: ticket 09 store.enter/read-stock/read-actions/service/leave/kicked; special body maps special-store.*.**

These rows should be fully semantic. The canonical row must preserve all wire-decoded fields and protocol provenance. The display name and powers string remain opaque formatted text spans because they may contain color codes; do not re-derive identity from them. Wide rows expose `xtra1..xtra9` as ordered spell IDs, not as a reparsed display string. Negative weight and price values are meaningful legacy sentinels (player-store sign, museum/sold-out behavior), evidenced by renderer branches [src/client/c-store.c:54](../../../src/client/c-store.c#L54) [src/client/c-store.c:81](../../../src/client/c-store.c#L81) [src/client/c-store.c:110](../../../src/client/c-store.c#L110); preserve them exactly rather than normalizing to missing values.

**L73: ticket 09 store.enter/read-stock/read-actions/service/leave/kicked; special body maps special-store.*.**

`PKT_STORE_INFO` defines store number, store/owner names, item count, purse/capacity, glyph/attribute and price multiplier. A negative item count is the protocol's special-store flag; nonnegative means normal NPC/player/home store [src/client/nclient.c:5445](../../../src/client/nclient.c#L5445) [src/client/nclient.c:5460](../../../src/client/nclient.c#L5460) [src/client/nclient.c:5467](../../../src/client/nclient.c#L5467). Model the special flag explicitly while retaining the signed wire value and version defaults.

**L75: ticket 09 store.enter/read-stock/read-actions/service/leave/kicked; special body maps special-store.*.**

Each `PKT_BACT` action is a typed indexed record: building action ID, server action, label, attribute, key letter, signed cost and 32-bit flags [src/client/nclient.c:4510](../../../src/client/nclient.c#L4510) [src/client/nclient.c:4519](../../../src/client/nclient.c#L4519) [src/client/nclient.c:4531](../../../src/client/nclient.c#L4531). The schema has nine slots [src/common/defines.h:2096](../../../src/common/defines.h#L2096); flags request store-item selection, inventory selection, gold/amount input, identification and item-class restrictions, hardcoded behavior, and admin gating [src/common/defines.h:9741](../../../src/common/defines.h#L9741). The client resolves those flags into typed item/gold inputs then sends `(action,item,item2,amount,gold)` [src/client/c-store.c:644](../../../src/client/c-store.c#L644) [src/client/c-store.c:669](../../../src/client/c-store.c#L669) [src/client/c-store.c:695](../../../src/client/c-store.c#L695) [src/client/c-store.c:777](../../../src/client/c-store.c#L777) [src/client/c-store.c:797](../../../src/client/c-store.c#L797) [src/client/nclient.c:7906](../../../src/client/nclient.c#L7906). Actions are therefore semantic commands, not labels to parse. Preserve unknown flag bits and unknown action IDs for forward compatibility; disable only the unsupported action, not the whole store.

**L77: ticket 09 store.enter/read-stock/read-actions/service/leave/kicked; special body maps special-store.*.**

Normal-store lifecycle is server-entered: `PKT_STORE_INFO` opens the surface, row/action packets update it, and either local Escape or `PKT_STORE_LEAVE` sets the exit condition. On local exit the client sends leave, clears transient stock/last-item state, flushes events and restores the previous surface; a server kick also stops safe macros and clears stale player-store visuals [src/client/c-store.c:1127](../../../src/client/c-store.c#L1127) [src/client/c-store.c:1133](../../../src/client/c-store.c#L1133) [src/client/c-store.c:1201](../../../src/client/c-store.c#L1201) [src/client/c-store.c:1230](../../../src/client/c-store.c#L1230) [src/client/c-store.c:1236](../../../src/client/c-store.c#L1236) [src/client/nclient.c:5479](../../../src/client/nclient.c#L5479). Paging, purchase/take, sell/drop, examine/book browse, paste-to-chat, inventory/equipment, inscription and server-defined hotkeys are all in-surface actions [src/client/c-store.c:133](../../../src/client/c-store.c#L133) [src/client/c-store.c:254](../../../src/client/c-store.c#L254) [src/client/c-store.c:305](../../../src/client/c-store.c#L305) [src/client/c-store.c:800](../../../src/client/c-store.c#L800) [src/client/c-store.c:890](../../../src/client/c-store.c#L890) [src/client/c-store.c:961](../../../src/client/c-store.c#L961). Sell confirmation is a server response followed by a client yes/no and `PKT_STORE_CONFIRM` [src/client/nclient.c:5496](../../../src/client/nclient.c#L5496).

**L79: ticket 09 store.enter/read-stock/read-actions/service/leave/kicked; special body maps special-store.*.**

The state model must permit a **staging** phase before visible open: server store display code sends row/action material before the final `STORE_INFO` packet that causes the legacy client to enter shopping mode [src/server/store.c:3367](../../../src/server/store.c#L3367) [src/server/store.c:3390](../../../src/server/store.c#L3390) [src/server/store.c:3490](../../../src/server/store.c#L3490). `STORE_INFO` also defines the active row extent; there is no independent row-delete packet, so shrinking stock invalidates keyed rows beyond `stock_num` even if their old bytes remain cached.

**L83: special-store.text/glyph/clear/force-clear/wheel/slots/in-between/dice/cards/unknown-animation/recreate; server-flow.go/wheel/craps/blackjack.**

Special stores use the same metadata/action/lifecycle shell but no item list. Instead the server sends a coordinate-addressed drawing protocol:

**L85: special-store.text/glyph/clear/force-clear/wheel/slots/in-between/dice/cards/unknown-animation/recreate; server-flow.go/wheel/craps/blackjack.**

- string write `(line,col,attr,string)`;

**L86: special-store.text/glyph/clear/force-clear/wheel/slots/in-between/dice/cards/unknown-animation/recreate; server-flow.go/wheel/craps/blackjack.**

- character write `(line,col,attr,char)`;

**L87: special-store.text/glyph/clear/force-clear/wheel/slots/in-between/dice/cards/unknown-animation/recreate; server-flow.go/wheel/craps/blackjack.**

- clear line range, with `line_start+100` meaning force-clear for graphical visuals;

**L88: special-store.text/glyph/clear/force-clear/wheel/slots/in-between/dice/cards/unknown-animation/recreate; server-flow.go/wheel/craps/blackjack.**

- animation `(opcode,arg2,arg3,arg4)` [src/server/nserver.c:9727](../../../src/server/nserver.c#L9727) [src/server/nserver.c:9755](../../../src/server/nserver.c#L9755) [src/server/nserver.c:9783](../../../src/server/nserver.c#L9783) [src/server/nserver.c:9815](../../../src/server/nserver.c#L9815) [src/server/nserver.c:9820](../../../src/server/nserver.c#L9820).

**L90: special-store.text/glyph/clear/force-clear/wheel/slots/in-between/dice/cards/unknown-animation/recreate; server-flow.go/wheel/craps/blackjack.**

The current client applies strings/chars/clears directly to terminal cells and runs casino-specific animations immediately [src/client/nclient.c:4662](../../../src/client/nclient.c#L4662) [src/client/nclient.c:4672](../../../src/client/nclient.c#L4672) [src/client/nclient.c:4682](../../../src/client/nclient.c#L4682) [src/client/nclient.c:4689](../../../src/client/nclient.c#L4689) [src/client/nclient.c:4711](../../../src/client/nclient.c#L4711) [src/client/nclient.c:4719](../../../src/client/nclient.c#L4719) [src/client/nclient.c:5045](../../../src/client/nclient.c#L5045) [src/client/nclient.c:5064](../../../src/client/nclient.c#L5064). Go proves this is not merely prose: the server sends board coordinates, stones/status/clocks, free-form dialogue, and an instruction line through the same channel [src/server/go.c:1292](../../../src/server/go.c#L1292) [src/server/go.c:1324](../../../src/server/go.c#L1324) [src/server/go.c:1376](../../../src/server/go.c#L1376) [src/server/go.c:1496](../../../src/server/go.c#L1496) [src/server/go.c:1628](../../../src/server/go.c#L1628).

**L92: special-store.text/glyph/clear/force-clear/wheel/slots/in-between/dice/cards/unknown-animation/recreate; server-flow.go/wheel/craps/blackjack.**

The permanent lossless model must therefore include both (a) an ordered operation/event log with original opcode/arguments/bytes and (b) the resulting attributed cell/canvas state. The operation log is needed for replay, timing/animation parity, unknown opcodes and force-clear semantics; the canvas is needed for repaint after resize/occlusion without a terminal fallback. Semantic adapters may recognize known Go/casino states, but are additive. Parsing visible board text back into game state is forbidden.

**L94: special-store.text/glyph/clear/force-clear/wheel/slots/in-between/dice/cards/unknown-animation/recreate; server-flow.go/wheel/craps/blackjack.**

Leaving a special store must abort any pending typed request and cancel ongoing store-specific interaction; the server explicitly sends `PKT_REQUEST_ABORT`, resets action state, and performs game cleanup [src/server/store.c:9124](../../../src/server/store.c#L9124) [src/server/store.c:9181](../../../src/server/store.c#L9181) [src/server/store.c:9184](../../../src/server/store.c#L9184) [src/server/store.c:9189](../../../src/server/store.c#L9189).

**L98: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

The `PKT_REQUEST_*` family is typed interaction protocol suitable for a single reusable modal-request model:

**L100: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

| Packet | Request payload | Response/cancel semantics |

**L102: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

| KEY | request ID, prompt | one key; Escape becomes zero |

**L103: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

| AMT | ID, prompt, maximum | amount from quantity input |

**L104: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

| NUM | ID, prompt, predefined/min/max | bounded integer |

**L105: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

| STR | ID, prompt, default string | string; Escape becomes literal `\e` sentinel |

**L106: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

| CFR | ID, prompt, optional default choice | tri-state/default-aware confirmation returned as integer/bool |

**L107: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

| ABORT | no ID | interrupts the pending client input |

**L109: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

Client handling and exact cancel values are explicit [src/client/nclient.c:7176](../../../src/client/nclient.c#L7176) [src/client/nclient.c:7190](../../../src/client/nclient.c#L7190) [src/client/nclient.c:7202](../../../src/client/nclient.c#L7202) [src/client/nclient.c:7213](../../../src/client/nclient.c#L7213) [src/client/nclient.c:7226](../../../src/client/nclient.c#L7226) [src/client/nclient.c:7246](../../../src/client/nclient.c#L7246). The server records request ID/type/extra for validation and routes typed return packets by ID [src/server/nserver.c:11242](../../../src/server/nserver.c#L11242) [src/server/nserver.c:11258](../../../src/server/nserver.c#L11258) [src/server/nserver.c:11274](../../../src/server/nserver.c#L11274) [src/server/nserver.c:11299](../../../src/server/nserver.c#L11299) [src/server/nserver.c:11324](../../../src/server/nserver.c#L11324) [src/server/nserver.c:16149](../../../src/server/nserver.c#L16149) [src/server/nserver.c:16170](../../../src/server/nserver.c#L16170) [src/server/nserver.c:16191](../../../src/server/nserver.c#L16191) [src/server/nserver.c:16212](../../../src/server/nserver.c#L16212) [src/server/nserver.c:16233](../../../src/server/nserver.c#L16233).

**L111: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

These prompts need no terminal-cell raw model. Preserve exact prompt/default/bounds/ID/type and cancellation sentinel in semantic state, plus unknown inline formatting spans in the prompt. Only one request is represented as pending by current client globals [src/client/variable.c:481](../../../src/client/variable.c#L481); the new model should explicitly enforce replacement/abort ordering rather than silently stacking dialogs. The server comments state that this mechanism can serve special stores, quests, and neutral monsters [src/common/types.h:4587](../../../src/common/types.h#L4587), so the UI must not brand it as store-only.

**L113: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

Other server-driven confirmation surfaces (`PKT_SELL`, `PKT_PICKUP_CHECK`) carry a typed price or prompt and synchronously invoke a yes/no before sending confirm/stay [src/client/nclient.c:5496](../../../src/client/nclient.c#L5496) [src/client/nclient.c:5956](../../../src/client/nclient.c#L5956). They should reuse the same modal infrastructure while preserving their distinct response intents.

**L119: ticket 11 Guide; content identity separate from documents.open-help; earlier callers require its child path.**

The Guide is a local `TomeNET-Guide.txt` browser, not `SPECIAL_FILE_HELP`. It can be buffered or read from disk, remembers position/search/chapter, accepts externally supplied search types/line/topic, supports bookmarks and substantial topic-normalization shortcuts, and reports missing/outdated file states [src/client/c-cmd.c:2462](../../../src/client/c-cmd.c#L2462) [src/client/c-cmd.c:2496](../../../src/client/c-cmd.c#L2496) [src/client/c-cmd.c:2510](../../../src/client/c-cmd.c#L2510) [src/client/c-cmd.c:2528](../../../src/client/c-cmd.c#L2528) [src/client/c-cmd.c:2534](../../../src/client/c-cmd.c#L2534) [src/client/c-cmd.c:2557](../../../src/client/c-cmd.c#L2557). Its correct new representation is lossless original lines plus parsed color/style/link/chapter markers and a semantic navigation/search index. The source text remains authoritative because the client ships/updates it independently and unknown future markup must remain visible.

**L121: ticket 11 Guide; content identity separate from documents.open-help; earlier callers require its child path.**

Server Help is the category-6 generic document described above; `?`-then-`?` bridges from it to the Guide. They must remain separate content sources even if presented in one workspace.

**L125: information.local-file-* and information.spoilers; export/clipboard ticket 11.**

`browse_local_file` is a second local browser for fixed spoiler/data files. It retains per-file line/search positions, supports wide lines, and recognizes file-dependent `W/W2/G/S` syntaxes [src/client/c-cmd.c:4812](../../../src/client/c-cmd.c#L4812) [src/client/c-cmd.c:4818](../../../src/client/c-cmd.c#L4818) [src/client/c-cmd.c:4822](../../../src/client/c-cmd.c#L4822) [src/client/c-cmd.c:4847](../../../src/client/c-cmd.c#L4847). As with the Guide, retain lossless source lines and build semantic indices/spans additively. Do not fold it into server special-file state.

**L129: information.artifact-lore-* and information.monster-lore-*; shared chat and item inscription IDs.**

Artifact Lore and Monster Lore are local interactive selectors built from setup/data arrays, not server-formatted special files. Both support incremental name/index (and monster symbol/level) filtering, list navigation/paging, lore-versus-stats detail mode, screenshots, inscription actions, chat during the surface, and title/current/all chat-paste variants. Artifact selector inputs are structured arrays (`code`, rarity, base-kind index, activation, preformatted selector name); monster selector inputs include code, name, glyph, level and flags [src/client/variable.c:485](../../../src/client/variable.c#L485) [src/client/variable.c:490](../../../src/client/variable.c#L490) [src/client/c-cmd.c:5946](../../../src/client/c-cmd.c#L5946) [src/client/c-cmd.c:5982](../../../src/client/c-cmd.c#L5982) [src/client/c-cmd.c:6211](../../../src/client/c-cmd.c#L6211) [src/client/c-cmd.c:6269](../../../src/client/c-cmd.c#L6269) [src/client/c-cmd.c:6301](../../../src/client/c-cmd.c#L6301) [src/client/c-cmd.c:6355](../../../src/client/c-cmd.c#L6355) [src/client/c-cmd.c:6551](../../../src/client/c-cmd.c#L6551) [src/client/c-cmd.c:6700](../../../src/client/c-cmd.c#L6700) [src/client/c-cmd.c:6811](../../../src/client/c-cmd.c#L6811) [src/client/c-cmd.c:6867](../../../src/client/c-cmd.c#L6867) [src/client/c-cmd.c:6872](../../../src/client/c-cmd.c#L6872) [src/client/c-cmd.c:6899](../../../src/client/c-cmd.c#L6899) [src/client/c-cmd.c:6926](../../../src/client/c-cmd.c#L6926).

**L131: information.artifact-lore-* and information.monster-lore-*; shared chat and item inscription IDs.**

These are fully eligible for semantic replacement: selected entity ID, filters, result list, selection, detail mode, lore/stat sections and paste intents. Nevertheless, keep the source-format strings/color/glyph attributes used in content generation and acceptance fixtures; chat paste is gameplay-visible output and must be byte/format compatible where the existing server/chat contract depends on color and continuation markers.

**L135: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

The existing “Display current knowledge” menu is a useful surface census: besides the categories above it exposes wilderness map, lag meter, setup intro MOTD, message history, chat history, `/ex`, received notes and local spoiler files [src/client/c-cmd.c:7643](../../../src/client/c-cmd.c#L7643) [src/client/c-cmd.c:7650](../../../src/client/c-cmd.c#L7650) [src/client/c-cmd.c:7655](../../../src/client/c-cmd.c#L7655) [src/client/c-cmd.c:7660](../../../src/client/c-cmd.c#L7660) [src/client/c-cmd.c:7667](../../../src/client/c-cmd.c#L7667) [src/client/c-cmd.c:7674](../../../src/client/c-cmd.c#L7674) [src/client/c-cmd.c:7846](../../../src/client/c-cmd.c#L7846) [src/client/c-cmd.c:7873](../../../src/client/c-cmd.c#L7873) [src/client/c-cmd.c:7879](../../../src/client/c-cmd.c#L7879) [src/client/c-cmd.c:7900](../../../src/client/c-cmd.c#L7900).

**L137: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

- The setup intro MOTD is a fixed 23-row, 120-byte-stride server setup buffer displayed locally until keypress [src/client/c-files.c:1907](../../../src/client/c-files.c#L1907) [src/client/c-files.c:1915](../../../src/client/c-files.c#L1915) [src/client/c-files.c:1924](../../../src/client/c-files.c#L1924) [src/client/c-files.c:1933](../../../src/client/c-files.c#L1933). Preserve the exact setup payload/rows; semantic prose is unnecessary.

**L138: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

- Message/chat history must preserve ordered formatted message records and channel metadata rather than only final painted cells; its deeper packet/state inventory belongs to the existing packet and input-loop work.

**L139: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

- Other presentation-bearing packets follow the same rule. `PKT_MESSAGE` is ordered formatted text with color/control markers and client behaviors including a top-line-clear sentinel; `PKT_TARGET_INFO` carries exact server-formatted target text; `PKT_WHATS_UNDER_YOUR_FEET` combines typed flags with an exact decorated object name used by display and auto-pickup [src/client/nclient.c:3339](../../../src/client/nclient.c#L3339) [src/client/nclient.c:3387](../../../src/client/nclient.c#L3387) [src/client/nclient.c:5515](../../../src/client/nclient.c#L5515) [src/client/nclient.c:7477](../../../src/client/nclient.c#L7477). Semantic classifiers are additive; retain exact formatted text and event order.

**L140: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

- Wilderness/minimaps and lag meter are not text-document parsing problems: retain their typed grid/sample state as covered by renderer/state-boundary tickets.

**L141: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

- `/ex` is slash-command/message output today, not implemented `SPECIAL_FILE_EXTRAINFO`; it must remain accessible through the command/message model.

**L142: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

- Received notes and local spoiler files are local-file content: preserve source text and parse additively.

**L144: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

The boundary test is simple: if the protocol gives stable typed fields that drive rendering/actions, those fields are semantic state; if it gives already-laid-out text/cells/animation commands, the lossless presentation representation is semantic state too and cannot be discarded after extracting guessed entities.

**L148: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

The eventual specification should require these state types and evidence, without implementing them during wayfinding:

**L150: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

1. `FormattedDocumentSession`: raw category, request/search/page lifecycle, raw title/line packets, parsed spans, view position and close semantics.

**L151: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

2. `StoreSession`: typed metadata, keyed normal/wide rows, server-defined typed actions, pending transaction/confirm state and lifecycle.

**L152: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

3. `SpecialStoreSurface`: ordered raw draw/clear/animation operations plus reconstructible attributed canvas, layered inside `StoreSession`.

**L153: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

4. `ServerRequest`: discriminated KEY/AMT/NUM/STR/CFR request with exact ID, prompt/default/bounds, response and abort semantics.

**L154: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

5. `LocalDocument`: lossless Guide/spoiler/note source plus indexes/spans.

**L155: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

6. `LoreWorkspace`: typed artifact/monster search, selection, detail mode and paste intents.

**L157: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

Acceptance fixtures must include at least one instance of every implemented `SPECIAL_FILE_*` category, arbitrary `OTHER`, unknown title/markup, long horizontally scrolled/color-coded lines, server search and position change, live player-list refresh, normal/home/player store sentinels, both normal and wide row packets, all nine action slots and unknown bits, special-store string/char/normal-clear/force-clear/each known animation/unknown animation, every typed request and cancellation path, local Guide missing/loaded/search/bookmark flows, and lore search/detail/paste flows.

**L161: Cross-cutting rule for documents.*, special-store.*, request.*, information.*; setup MOTD/map/target reuse 08, normal stores/underfoot reuse 09, Guide/files integration 11, whole-client closure 12.**

This resolution does **not** sharpen fog into a new decision ticket. Its consequences are already owned by the open decisions **Choose the presentation-state boundary**, **Define the single-window interaction model**, **Preserve input and macro semantics**, **Set compatible protocol extension policy**, **Design parity evidence and acceptance**, and **Specify renderer parity**. The inventory supplies constraints and acceptance cases to those tickets; it does not introduce a new unresolved product or architecture question.


### `docs/research/single-window-input-loops.md`

SHA-256 `40a47b0ae685a8a1ee2f1309c76e60722783b05aa494504f47b7a5ed8b88fef7`.

**L46 → `capability.information.close-sheet`, `capability.information.navigate-sheet`, `capability.information.read-equipment-flags`, `capability.information.read-history`, `capability.information.read-sheet`.**

| `input.command.character` | `cmd_character()` | 2/8 page; 4/6 horizontal; ? guide; h/f export; v mode; `:` chat; q/Q/Esc/C exit; Ctrl-T | same | inherited | explicit exits; failed filename retry in menu | sheet page/column | optional file dump only | save/load | sheet/version branches | [`c-cmd.c:2199`](../../src/client/c-cmd.c#L2199), [`c-cmd.c:2262`](../../src/client/c-cmd.c#L2262), [`c-cmd.c:2351`](../../src/client/c-cmd.c#L2351) |

**L48 → `capability.information.local-file-bookmarks`, `capability.information.local-file-close`, `capability.information.local-file-navigate`, `capability.information.local-file-read`, `capability.information.local-file-search`.**

| `input.command.local-file` | `browse_local_file()` | guide navigation subset plus Left/4 and Right/6 horizontal; `/ s r R S` search; d/a/A/D marks; f dump; # line; Esc; Ctrl-K/L; `:` chat | same | modal suppression | cancel prompt returns; invalid retries | file position/offset/search/marks | local file only | save/load | `REGEX_SEARCH`, clipboard | [`c-cmd.c:4818`](../../src/client/c-cmd.c#L4818), [`c-cmd.c:5548`](../../src/client/c-cmd.c#L5548), [`c-cmd.c:5934`](../../src/client/c-cmd.c#L5934) |

**L49 → `capability.information.artifact-lore-close`, `capability.information.artifact-lore-details`, `capability.information.artifact-lore-navigate`, `capability.information.artifact-lore-paste-all`, `capability.information.artifact-lore-paste-current`, `capability.information.artifact-lore-paste-title`, `capability.information.artifact-lore-read`, `capability.information.artifact-lore-search`, `capability.information.monster-lore-close`, `capability.information.monster-lore-details`, `capability.information.monster-lore-navigate`, `capability.information.monster-lore-paste-all`, `capability.information.monster-lore-paste-current`, `capability.information.monster-lore-paste-title`, `capability.information.monster-lore-read`, `capability.information.monster-lore-search`.**

| `input.command.lore` | `artifact_lore()`, `monster_lore()` | letter or `@` name; navigation/scroll in received perusal; Esc cancel | same | hybrid suppressed | missing selection / Esc returns; safe macro failures flush | selected lore index, browser position | `Send_special_line()` then perusal | save/load | server data/version | [`c-cmd.c:5947`](../../src/client/c-cmd.c#L5947), [`c-cmd.c:6112`](../../src/client/c-cmd.c#L6112), [`c-cmd.c:6355`](../../src/client/c-cmd.c#L6355), [`c-cmd.c:6711`](../../src/client/c-cmd.c#L6711) |

**L50 → `capability.information.spoilers`.**

| `input.command.spoilers` | `cmd_spoilers()` | k/e/r/E/a/v/d/f/t/s select report; `:`; Ctrl-T; Esc/Ctrl-Q | same | normal suppressed for choice | selection opens received/local browser; exit restores | report type | special-file request | save/load | server features | [`c-cmd.c:7345`](../../src/client/c-cmd.c#L7345), [`c-cmd.c:7368`](../../src/client/c-cmd.c#L7368) |

**L51 → `capability.information.notes-close`, `capability.information.notes-navigate`, `capability.information.notes-read`, `capability.information.notes-search`.**

| `input.command.notes` | `cmd_notes()` | Enter open; `/ s` search; PgUp/9/p, PgDn/3/n/Space, Home/7, End/1, arrows/2/8, Backspace; `:`; Ctrl-T; Esc/Ctrl-Q | same | hybrid suppressed | Esc exits; canceled search returns | selected note, scroll/search | local note files | save/load | filesystem | [`c-cmd.c:7424`](../../src/client/c-cmd.c#L7424), [`c-cmd.c:7535`](../../src/client/c-cmd.c#L7535), [`c-cmd.c:7611`](../../src/client/c-cmd.c#L7611) |

**L52 → `capability.documents.open-artifact`, `capability.documents.open-deaths`, `capability.documents.open-help`, `capability.documents.open-house`, `capability.documents.open-log`, `capability.documents.open-monster`, `capability.documents.open-motd2`, `capability.documents.open-object`, `capability.documents.open-other`, `capability.documents.open-player`, `capability.documents.open-player-equip`, `capability.documents.open-recall`, `capability.documents.open-scores`, `capability.documents.open-server-setting`, `capability.documents.open-trap`, `capability.documents.open-unique`, `capability.information.extended-report`, `capability.information.open-index`.**

| `input.command.misc-index` | `cmd_check_misc()` | exact knowledge/report/platform keys and second-level filters in subordinate map below | same | normal suppressed only while reading menu key | child cancel returns to menu | chosen report/filter | `Send_special_line()` / child browser / platform opener | save/load | many version, platform, SDL3 branches | [`c-cmd.c:7615`](../../src/client/c-cmd.c#L7615), [`c-cmd.c:7779`](../../src/client/c-cmd.c#L7779), [`c-cmd.c:8097`](../../src/client/c-cmd.c#L8097) |

**L53 → `capability.chat.cancel`, `capability.chat.forward-slash`, `capability.chat.history`, `capability.chat.local-self`, `capability.chat.send`, `capability.chat.substitute-items`, `capability.information.client-version`, `capability.information.local-time`, `capability.special-store.wager`.**

| `input.command.message` | `cmd_message()` | text editor under `Message:`; slash commands parsed locally; ordinary chat sent | same | hybrid suppressed; text may contain macro-like bytes | Esc cancels; Enter submits; empty ignored | chat history/mode; numerous slash shortcuts may alter local options | `Send_msg()` or local command | topline | command-specific/platform screenshot branches | [`c-cmd.c:8100`](../../src/client/c-cmd.c#L8100), [`c-cmd.c:8117`](../../src/client/c-cmd.c#L8117), [`c-cmd.c:8591`](../../src/client/c-cmd.c#L8591) |

**L54 → `capability.social.guild-allow-adders`, `capability.social.guild-auto-readd`, `capability.social.guild-list-adders`, `capability.social.guild-minimum-level`, `capability.social.guild-options-close`, `capability.social.guild-toggle-adder`.**

| `input.command.guild` | `cmd_guild_options()` | `1` allow-adders; `2` auto-readd; `3` prompt minimum level; `a` request adder list; `b` prompt player to toggle as adder; `:` chat; Ctrl-T; Esc/Ctrl-Q | same | hybrid suppressed | Esc/Ctrl-Q exits; canceled `3`/`b` stays; unavailable/non-master action bells | guild flags/minimum/adder target | `Send_guild_config`; `a` sends `/xguild_adders` | save/load | guild membership/master | [`c-cmd.c:8595`](../../src/client/c-cmd.c#L8595), [`c-cmd.c:8670`](../../src/client/c-cmd.c#L8670) |

**L55 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| `input.command.party` | `cmd_party()` | `1` create/rename party; `2` iron team; `3` add self/player or join named party; `4` remove player; `5` leave; `0` close iron team + confirm; `a` create guild; `b` add/join guild; `C` remove from guild; `D` leave + confirm; `e` guild options; `A` hostility; `P` peace; `:`; Ctrl-T; Esc/Ctrl-Q | same | hybrid suppressed | Esc/Ctrl-Q exits; canceled name/confirm stays | selected social operation | `Send_party()` / `Send_guild()` | save/load | party/guild state; `s_NO_PK`; guild-options requires server >4.5.2 | [`c-cmd.c:8731`](../../src/client/c-cmd.c#L8731), [`c-cmd.c:8802`](../../src/client/c-cmd.c#L8802), [`c-cmd.c:8921`](../../src/client/c-cmd.c#L8921) |

**L60 → `capability.messages.recall-close`, `capability.messages.recall-navigate`, `capability.messages.recall-read`, `capability.messages.recall-search`.**

| `input.command.messages` | `do_cmd_messages()` / `_important()` | 8/k/Backspace one older; 2/j/Enter one newer; +/- ten; p/Ctrl-P/b/Ctrl-U older page; n/Ctrl-N/Space newer; g/G ends; 4/h/<, 6/l/> horizontal; # line; = mark; / search; r regex; f/F dump; Ctrl-K copy; `:`; Ctrl-T; Esc/Ctrl-Q | same aliases | hybrid suppressed only in child prompts | exit restores; canceled child returns; no match/invalid bells | message offset/search/mark | optional local dump | save/load | `REGEX_SEARCH`; `COPY_MULTILINE`; important adds Ctrl-O older alias | [`c-xtra2.c:144`](../../src/client/c-xtra2.c#L144), [`c-xtra2.c:329`](../../src/client/c-xtra2.c#L329), [`c-xtra2.c:594`](../../src/client/c-xtra2.c#L594), [`c-xtra2.c:747`](../../src/client/c-xtra2.c#L747) |

**L79 → `capability.housing.cancel`, `capability.housing.trade`.**

| `input.command.house` | `cmd_purchase_house()` and house helpers | exact house keys/prompts in subordinate map below | same except initial direction | hybrid suppressed in menu | Esc/Ctrl-Q cancels; canceled child prompt still closes menu; rejected purchase/delete confirmation returns to menu | selected adjacent-house operation | `Send_purchase_house` or `Send_admin_house` | save/load; child owner screen replaces temporarily | house/admin privilege and item availability | [`c-cmd.c:9091`](../../src/client/c-cmd.c#L9091), [`c-cmd.c:9206`](../../src/client/c-cmd.c#L9206) |

**L80 → `capability.admin.close`.**

| `input.admin.master` | `cmd_master*` and script helpers | exact root/level/generation/build/summon/player/system keys in subordinate map below | same except nested directions | hybrid suppressed in menu loops | Esc/Ctrl-Q returns; canceled child aborts that action and returns to owner | editor/generation/admin parameters | admin `Send_*` messages | save/load or topline | admin DM/wizard, numerous server compile/version gates | [`c-cmd.c:9314`](../../src/client/c-cmd.c#L9314), [`c-cmd.c:10463`](../../src/client/c-cmd.c#L10463) |

**L81 → `capability.information.lag-clear`, `capability.information.lag-close`, `capability.information.lag-disable`, `capability.information.lag-enable`, `capability.information.lag-read`.**

| `input.command.lagometer` | `cmd_lagometer()` | `1` enable; `2` disable full and mini meter; `c` clear 60 ping samples; `:` chat; Ctrl-T screenshot; Esc/Ctrl-Q/Ctrl-I exit | same | inherited | explicit exit restores; all other keys are ignored and retry | `lagometer_enabled`, `ping_times[]`, `lagometer_open` | none | save/load, then flush queue | none | [`c-cmd.c:10606`](../../src/client/c-cmd.c#L10606) |

**L157 → `capability.admin.close`.**

| `c-cmd.c` | `cmd_all_in_one`, `process_command`, movement direction wrappers, `cmd_mini_map`, `cmd_locate`, inventory/subinventory/equipment, destroy/inscribe, steal/device/activate direction prompts, target/look, character, guide/local-file, artifact/monster lore, spoilers, notes, misc index, message, guild, party, fire/throw, load-pref, house commands, suicide, all `cmd_master*`/script helpers, lagometer |

**L161 → `capability.messages.important-close`, `capability.messages.important-navigate`, `capability.messages.important-read`, `capability.messages.important-search`, `capability.messages.recall-close`, `capability.messages.recall-navigate`, `capability.messages.recall-read`, `capability.messages.recall-search`.**

| `c-xtra2.c` | `do_cmd_messages`, `do_cmd_messages_important` |

**L214 → `capability.information.close-sheet`, `capability.information.navigate-sheet`, `capability.information.read-equipment-flags`, `capability.information.read-history`, `capability.information.read-sheet`.**

| `c-cmd.c: cmd_character` | loop | character sheet browser/export |

**L216 → `capability.information.local-file-bookmarks`, `capability.information.local-file-close`, `capability.information.local-file-navigate`, `capability.information.local-file-read`, `capability.information.local-file-search`.**

| `c-cmd.c: browse_local_file` | loop | local file browser/search/marks |

**L217 → `capability.information.artifact-lore-close`, `capability.information.artifact-lore-details`, `capability.information.artifact-lore-navigate`, `capability.information.artifact-lore-paste-all`, `capability.information.artifact-lore-paste-current`, `capability.information.artifact-lore-paste-title`, `capability.information.artifact-lore-read`, `capability.information.artifact-lore-search`.**

| `c-cmd.c: artifact_lore` | loop | artifact selection/perusal |

**L218 → `capability.information.monster-lore-close`, `capability.information.monster-lore-details`, `capability.information.monster-lore-navigate`, `capability.information.monster-lore-paste-all`, `capability.information.monster-lore-paste-current`, `capability.information.monster-lore-paste-title`, `capability.information.monster-lore-read`, `capability.information.monster-lore-search`.**

| `c-cmd.c: monster_lore` | loop | monster selection/perusal |

**L219 → `capability.information.spoilers`.**

| `c-cmd.c: cmd_spoilers` | loop | spoiler report chooser |

**L220 → `capability.information.notes-close`, `capability.information.notes-navigate`, `capability.information.notes-read`, `capability.information.notes-search`.**

| `c-cmd.c: cmd_notes` | loop | notes chooser/search |

**L221 → `capability.documents.open-artifact`, `capability.documents.open-deaths`, `capability.documents.open-help`, `capability.documents.open-house`, `capability.documents.open-log`, `capability.documents.open-monster`, `capability.documents.open-motd2`, `capability.documents.open-object`, `capability.documents.open-other`, `capability.documents.open-player`, `capability.documents.open-player-equip`, `capability.documents.open-recall`, `capability.documents.open-scores`, `capability.documents.open-server-setting`, `capability.documents.open-trap`, `capability.documents.open-unique`, `capability.information.extended-report`, `capability.information.open-index`.**

| `c-cmd.c: cmd_check_misc` | loop | misc-report chooser and filter prompts |

**L222 → `capability.chat.cancel`, `capability.chat.forward-slash`, `capability.chat.history`, `capability.chat.local-self`, `capability.chat.send`, `capability.chat.substitute-items`, `capability.information.client-version`, `capability.information.local-time`, `capability.special-store.wager`.**

| `c-cmd.c: cmd_message` | delegate | one text editor; slash/chat dispatch |

**L223 → `capability.social.guild-allow-adders`, `capability.social.guild-auto-readd`, `capability.social.guild-list-adders`, `capability.social.guild-minimum-level`, `capability.social.guild-options-close`, `capability.social.guild-toggle-adder`.**

| `c-cmd.c: cmd_guild_options` | loop | guild options and nested text prompts |

**L224 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| `c-cmd.c: cmd_party` | loop | party/guild action menu and prompts |

**L227 → `capability.housing.owner-guild`, `capability.housing.owner-player`.**

| `c-cmd.c: cmd_house_chown` | loop | owner/access choice plus name prompt |

**L228 → `capability.housing.access`, `capability.housing.delete`, `capability.housing.tag`.**

| `c-cmd.c: cmd_house_chmod, cmd_house_kill, cmd_house_tag` | delegates | confirmation series / tag text |

**L229 → `capability.housing.cancel`, `capability.housing.trade`.**

| `c-cmd.c: cmd_purchase_house` | loop | direction, operation choice, confirmation |

**L231 → `capability.admin.level-add-dungeon`, `capability.admin.level-blank-module`, `capability.admin.level-create-town`, `capability.admin.level-load-module`, `capability.admin.level-module-entry`, `capability.admin.level-perma-static`, `capability.admin.level-remove-dungeon`, `capability.admin.level-save-module`, `capability.admin.level-static`, `capability.admin.level-unperma-static`, `capability.admin.level-unstatic`.**

| `c-cmd.c: cmd_master_aux_level` | loop | level-builder menu and nested fields/confirmations |

**L232 → `capability.admin.generate-vault`.**

| `c-cmd.c: cmd_master_aux_generate_vault` | loop | vault generator menu/name |

**L234 → `capability.admin.build-dead-tree`, `capability.admin.build-dirt`, `capability.admin.build-disable`, `capability.admin.build-feature`, `capability.admin.build-floor`, `capability.admin.build-grass`, `capability.admin.build-house-door`, `capability.admin.build-info-mode`, `capability.admin.build-info2-mode`, `capability.admin.build-permanent-wall`, `capability.admin.build-set-info`, `capability.admin.build-set-info2`, `capability.admin.build-sign`, `capability.admin.build-tree`, `capability.admin.build-wall`.**

| `c-cmd.c: cmd_master_aux_build` | loop | builder menu/sign text |

**L238 → `capability.admin.summon-depth`, `capability.admin.summon-disable`, `capability.admin.summon-high-undead`, `capability.admin.summon-low-undead`, `capability.admin.summon-name`, `capability.admin.summon-obliterate`, `capability.admin.summon-orcs`.**

| `c-cmd.c: cmd_master_aux_summon` | loop | summon menu/custom target text |

**L239 → `capability.admin.player-acquirement`, `capability.admin.player-broadcast`, `capability.admin.player-delete`, `capability.admin.player-edit`, `capability.admin.player-kill`, `capability.admin.player-static`, `capability.admin.player-telekinesis`, `capability.admin.player-unstatic`.**

| `c-cmd.c: cmd_master_aux_player` | loop | player-admin menu and name/message fields |

**L240 → `capability.admin.local-script`, `capability.admin.server-script`, `capability.admin.upload-script`.**

| `c-cmd.c: cmd_script_upload, cmd_script_exec, cmd_script_exec_local` | delegates | script name/source text prompts |

**L241 → `capability.admin.close`.**

| `c-cmd.c: cmd_master_aux_system, cmd_master` | loops | system/root admin menus |

**L242 → `capability.housing.claim-land`.**

| `c-cmd.c: cmd_king` | delegate | ownership confirmation |

**L244 → `capability.information.lag-clear`, `capability.information.lag-close`, `capability.information.lag-disable`, `capability.information.lag-enable`, `capability.information.lag-read`.**

| `c-cmd.c: cmd_lagometer` | loop | lagometer enable/style/exit |

**L275 → `capability.messages.important-close`, `capability.messages.important-navigate`, `capability.messages.important-read`, `capability.messages.important-search`, `capability.messages.recall-close`, `capability.messages.recall-navigate`, `capability.messages.recall-read`, `capability.messages.recall-search`.**

| `c-xtra2.c: do_cmd_messages, do_cmd_messages_important` | loops | same recall state machine over different backing histories |


### `.scratch/single-window-sdl3-client/research/remaining-client-input-loops.md`

SHA-256 `76f85717b58f8a026a43dab05ee322150abea260f07f38316886186b71d47caf`.

**L26 → `capability.documents.close`, `capability.documents.navigate`, `capability.documents.read`, `capability.documents.refresh-players`, `capability.documents.search`.**

| `input.server-file.peruse` | `peruse_file` | exact transitions next section | same; inherited, inkey_interact_macros=false; nested search sets inkey_msg=true | Esc/Ctrl-Q exit; `?` HELP exits then guide; player-list inkey -1 redraw/re-request; invalid key no-op/re-request | cur_line/col; srcstr; searching/reverse/regexp; max_line/page_size; perusing | `Send_special_line(type,line,search)`; exit `SPECIAL_FILE_NONE`; chat/inscription delegates | save/load; clipping managed received lines; restore macro flag+Flush_queue on exit | REGEX_SEARCH and server>=4.9.0; SPECIAL_FILE_PLAYER auto-refresh; USE_SOUND_2010 | [c-files.c:1953](../../../src/client/c-files.c#L1953), [requests:1993](../../../src/client/c-files.c#L1993), [reads:2042](../../../src/client/c-files.c#L2042), [exit:2244](../../../src/client/c-files.c#L2244) |

**L52 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| `Receive_request_key`:7184 | one-key primitive runtime prompt/id | Esc → reply0, not byte27 | request_pending true during primitive; Send_request_key(id,key/0) | topline | [nclient.c:7176](../../../src/client/nclient.c#L7176) |

**L53 → `capability.request.answer-amt`, `capability.request.answer-num`, `capability.request.cancel-amt`, `capability.request.cancel-num`.**

| `Receive_request_amt`:7197; `Receive_request_num`:7209 | quantity / bounded number primitives runtime prompt/max or predef/min/max | canceled value0 still replied | Send_request_amt/num(id,value), pending scoped around prompt | topline | [nclient.c:7190](../../../src/client/nclient.c#L7190), [number:7202](../../../src/client/nclient.c#L7202) |

**L54 → `capability.request.answer-str`, `capability.request.cancel-str`.**

| `Receive_request_str`:7221 | text primitive runtime prefilled buf, MAX_CHARS_WIDE-1 | Esc → string byte27; accepted empty distinct | Send_request_str(id,text), pending scoped | topline | [nclient.c:7214](../../../src/client/nclient.c#L7214) |

**L55 → `capability.request.answer-cfr`, `capability.request.cancel-cfr`.**

| `Receive_request_cfr`:7242 | get_check3 runtime prompt/default | strict/default semantics from primitive; result replied | Send_request_cfr(id,bool); request_abort sets flag only if pending | topline | server>4.5.6.0.0.1 includes default byte; [nclient.c:7227](../../../src/client/nclient.c#L7227), [abort:7248](../../../src/client/nclient.c#L7248); abort consumption remains c-util primitive owner |


### `.scratch/single-window-sdl3-client/research/slash-command-grammar-and-dispatch.md`

SHA-256 `61041690124c847a56b5c5fb14023aa0d8699d595a071d0e93219484fbccf580`.

**L23 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| Verb / aliases | Recognizer и аргументы | Outcome, state / packets / prompts | Gates и source |

**L25 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/shot`, `/screenshot`; variants `/shotpng`, `/screenshotpng` | CS prefix `/shot` OR `/screenshot`; первый ASCII space и непустой следующий byte дают filename tail, иначе `screenshot????`. PNG variants — тоже unrestricted CS prefix | `xhtml_screenshot(name,FALSE)`; variants устанавливают `inkey_shift_special=3`; нет slash packet и confirmation | capture outcome зависит от screenshot_keys, ENABLE_SHIFT_SPECIALKEYS и backend; [8121](../../../src/client/c-cmd.c#L8121) |

**L26 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| `/ctime` | CI exact; без args | local wall-clock time через time/localtime, английский weekday; formatted chat/important message | нет server gate; [8298](../../../src/client/c-cmd.c#L8298) |

**L27 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| `/cver`, `/cversion` | CI exact | local compile-time VERSION_MAJOR..BUILD/TAG/OS/SUB | [8306](../../../src/client/c-cmd.c#L8306) |

**L28 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/apickup` | CI exact | toggle c_cfg.auto_pickup; feedback; не обновляет Client_setup.options и не вызывает Send_options | [8310](../../../src/client/c-cmd.c#L8310) |

**L29 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/adestroy` | CI exact | toggle c_cfg.auto_destroy, тот же local-only update contract | [8315](../../../src/client/c-cmd.c#L8315) |

**L30 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/daunmatched` | CI exact | toggle c_cfg.destroy_all_unmatched; feedback сообщает dependency auto_destroy, но команда не включает её и не блокируется при FALSE | [8320](../../../src/client/c-cmd.c#L8320) |

**L31 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/new` | CI exact; без args | показывает previous newest; child item-select `Which item?`, extra name lookup `Item name?`; USE_INVEN/EQUIP/EXTRA/SUBINVEN/UNPREFER_SUBINVEN; cancel/no valid i exits; только 0<=i<INVEN_TOTAL принимается; задаёт item_newest, optional redraw_newest | ENABLE_SUBINVEN только previous diagnostic; child selector допускает subinventory, owner отклоняет его encoded result; [8325](../../../src/client/c-cmd.c#L8325) |

**L32 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/new ` | CS prefix с одним space; пустой tail вызывает тот же selector | substring `help` anywhere (CS) OR first argument byte `?` → usage. Иначе используется только buf[5], tail игнорируется; lower maps inventory, upper maps equipment; byte >= 'a'+INVEN_WIELD-1 заменяется на Z; bounds и nonempty tval проверяются; item_newest+redraw | нет support двухбуквенного subinventory arg (TODO). `/NEW a` не этот branch; [8344](../../../src/client/c-cmd.c#L8344) |

**L33 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/opty`, `/optvy` | CI prefix 5/6; обязательно space сразу за verb и хотя бы один следующий byte; offset6/7; option tail exact CS | enable; redundant quiet unless verbose; validates o_desc/o_text; updates o_var и Client_setup.options; options_immediate(TRUE/FALSE), check_immediate_options, Send_options при изменении | специальный exact tail big_map → set_bigmap(1,verbose) без Send_options; [8383](../../../src/client/c-cmd.c#L8383) |

**L34 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/optn`, `/optvn` | такой же parser | disable; redundant quiet unless verbose; hooks+Send_options при изменении | big_map → set_bigmap(0,verbose); [8435](../../../src/client/c-cmd.c#L8435) |

**L35 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/optt`, `/optvt` | такой же parser | toggle; verbose feedback; hooks+Send_options для найденного option | big_map → set_bigmap(-1,verbose). Ошибка syntax для verbose варианта печатает `/optt`, не `/optvt`; [8488](../../../src/client/c-cmd.c#L8488) |

**L36 → `capability.documents.open-artifact`, `capability.documents.open-deaths`, `capability.documents.open-help`, `capability.documents.open-house`, `capability.documents.open-log`, `capability.documents.open-monster`, `capability.documents.open-motd2`, `capability.documents.open-object`, `capability.documents.open-other`, `capability.documents.open-player`, `capability.documents.open-player-equip`, `capability.documents.open-recall`, `capability.documents.open-scores`, `capability.documents.open-server-setting`, `capability.documents.open-trap`, `capability.documents.open-unique`, `capability.information.extended-report`, `capability.information.open-index`.**

| `/know` | CI exact | cmd_check_misc(): existing knowledge/menu surface, дальнейшие inputs и packets принадлежат меню | synonym logical `~`; [8533](../../../src/client/c-cmd.c#L8533), [binding:383](../../../src/client/c-cmd.c#L383) |

**L37 → `capability.chat.cancel`, `capability.chat.forward-slash`, `capability.chat.history`, `capability.chat.local-self`, `capability.chat.send`, `capability.chat.substitute-items`, `capability.information.client-version`, `capability.information.local-time`, `capability.special-store.wager`.**

| `/cvpng` | CI exact | png_screenshot(); result игнорируется; это conversion/open-last outcome, не cmd_message filename capture | platform/helper gates ниже; [8537](../../../src/client/c-cmd.c#L8537) |

**L38 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/? a` … `/? t` | CS prefix `/? `, total length4, exactly lower a..t | cmd_the_guide(3,0,one-letter bookmark); нет Send_msg | GUIDE_BOOKMARKS; иначе весь текст fallback; [8541](../../../src/client/c-cmd.c#L8541) |

**L39 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/reinit_guide` | CI exact | init_guide(), diagnostics, forced checksum check and outdated feedback; нет update/download guide content action | checksum backend gates ниже; [8547](../../../src/client/c-cmd.c#L8547) |

**L40 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/reinit_audio` | CI exact | re_init_sound(); success message only result0; audio/resource state reload, downstream Send_audio | helper USE_SOUND_2010, SOUND_SDL/SOUND_SDL3, runtime use_sound; recognizer itself unguarded; [8555](../../../src/client/c-cmd.c#L8555) |

**L41 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `// ` | CI prefix3 (punctuation); всё после третьего byte Lua source, может быть пустым | string_exec_lua(0,buf+3), local formatted result; arbitrary exposed Lua state/I/O/callback effects; нет встроенной confirmation | не admin/server gate, не restricted command whitelist; [8559](../../../src/client/c-cmd.c#L8559), [executor:533](../../../src/client/c-script.c#L533) |

**L42 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| `/wager` | CI prefix6; если buf[6]==0 report; иначе atol(buf+6), delimiter не обязателен | std_wager query/set; clamps >PY_MAX_GOLD to max, <1 to1 after assignment to s32b; no packet. `/wager10` →10, `/wager abc` →1, `/wager 10junk` →10; no-arg branch leaves inkey_msg TRUE | PY_MAX_GOLD=2000000000L; atol overflow/platform long width не safe parsing contract; [8565](../../../src/client/c-cmd.c#L8565), [constant:2434](../../../src/common/defines.h#L2434) |

**L43 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `/apply_auto_inscriptions`, `/aai` | CI exact | apply_all_auto_inscriptions(), all inventory/equipment and conditional subinventory; downstream inscription/uninscription/server-autoinscription packets; нет confirmation | ENABLE_SUBINVEN, REGEX_SEARCH, autoinscription flags/rules; server autoinscribe version gate ниже; [8582](../../../src/client/c-cmd.c#L8582) |

**L49 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| Syntax после editor | Transformation / routing | Source |

**L51 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| `{` | byte FF colour prefix; replacement не ограничивается visible chat text | [8139](../../../src/client/c-cmd.c#L8139) |

**L52 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| два backslash + a..inventory limit, A..equipment limit, `_`, `+` | inventory/equipment name, floor description, newest name; newest=-1 даёт empty item; subinventory newest gated ENABLE_SUBINVEN. Colour `FFs`, restore `FF-`, spacing, colon escaping и size check; overflow branch discards shortcut | [8150](../../../src/client/c-cmd.c#L8150) |

**L53 → `capability.chat.cancel`, `capability.chat.forward-slash`, `capability.chat.history`, `capability.chat.local-self`, `capability.chat.send`, `capability.chat.substitute-items`, `capability.information.client-version`, `capability.information.local-time`, `capability.special-store.wager`.**

| три backslash + uppercase pack slot + lowercase subslot | subinventory name; эта substitution branch **не окружена ENABLE_SUBINVEN** в cmd_message; exact available-build declarations отдельно от этой lexical observation. Overflow cleanup использует i+3, хотя consumed shortcut длиной5 | [8211](../../../src/client/c-cmd.c#L8211) |

**L54 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| три backslash + lower store slot | store_top+slot; a..l normally, a..z screen_hgt==MAX_SCREEN_HGT. store_paste_item/where; location only once/message, colon escaping. store stock/size failure discards shortcut | [8249](../../../src/client/c-cmd.c#L8249), [big_shop:8112](../../../src/client/c-cmd.c#L8112) |

**L55 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| `%:` кроме `%::` | local c_msg_format to messages; consumes, no network | [8286](../../../src/client/c-cmd.c#L8286) |

**L56 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| `%%:` кроме `%%::` | local format with chat marker FC; consumes, no network, subject to earlier chat-mode decoration | [8292](../../../src/client/c-cmd.c#L8292) |

**L57 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| всё остальное | Send_msg; includes explicit channels/private addresses, escaped self routes, unknown slash, bare `/`, malformed case/spacing variants | [8588](../../../src/client/c-cmd.c#L8588) |

**L84 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| Proposed intent family | Typed parameters / outcome | Source basis |

**L86 → `capability.chat.cancel`, `capability.chat.forward-slash`, `capability.chat.history`, `capability.chat.local-self`, `capability.chat.send`, `capability.chat.substitute-items`, `capability.information.client-version`, `capability.information.local-time`, `capability.special-store.wager`.**

| message.compose / submit | text, current chat mode; raw history separate from decorated/transformed wire payload; unknown server text fallback | editor+cmd_message |

**L87 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| screenshot.capture; screenshot.open-or-convert-last | filename/default pattern; explicit raster intent resolved against screenshot_keys/backend; last capture state | screenshot pre-pass + cvpng |

**L88 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| client.show-local-time / show-version | read-only local result | ctime/cver |

**L89 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| automation.toggle | pickup/destroy/destroy-unmatched target; preserve direct-toggle-vs-generic-option synchronization distinction until decided | toggle trio |

**L90 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| newest-item.choose / assign-slot | item selector versus slot result, cancel outcome and invalid item feedback | new branches |

**L91 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| option.set / toggle | canonical exact option name, desired boolean or toggle, verbose; hooks/capability result; big-map separate dimension action | opt families |

**L92 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| knowledge.open; guide.open-bookmark / reload; audio.reload | existing modal intents, bookmark a..t, helper outcomes | know/bookmarks/reinit |

**L93 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| lua.execute-local | raw program text, explicit legacy transformation decision; no inferred server permission | // branch |

**L94 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| wager.query / set-default; inscriptions.apply-all | clamped amount/default quantity and local-vs-wire effects | wager/aai |

**L118 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| Source | SHA-256 |

**L120 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/client/c-cmd.c | fe190ffdb11ae636ec7cf96964a0e4e3cc895fd74e9bbdcd23dac5b51a9e9a8a |

**L121 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/client/nclient.c | a0c6e85ec05944a1bc0e3115012a2a57b69f16f9c14e8a59bc5baabc0029ce60 |

**L122 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/client/c-files.c | ccbd1dcfbc6d743419719264991dd62960670a2eb66e08ca6177dd1a03887968 |

**L123 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/client/c-util.c | a948ad57c78fbb22d04adddabd149cec85f311ead463ac17c30ee7b7c566f05c |

**L124 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/client/c-script.c | 83740e2fe5a36e340fbacabe258e5f9180035e84b4749f793044e5f12db831f7 |

**L125 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/client/c-init.c | 51eded7b8d2ed221f9995bd777aa4f7a23af9e876296f28ee2db119d82a28672 |

**L126 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/client/c-tables.c | 94cc27da7b272aa7d6967a39b1731cd94fe22547088c1f455c1f1a69cb4957f4 |

**L127 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/client/variable.c | 0da4064e96f1871859e05790c6f0b7aba1727266db901db48dfb9f12b2668e37 |

**L128 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/client/c-store.c | 69062361291e1b366351e27779bdfb1b148572cf07fe8ddf1ac179b1391d65a7 |

**L129 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/common/z-util.c | a6d027d8844f7dde9cffc04b4fcedeee42828c9b37b30cd657908ee7eb075fe9 |

**L130 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/common/defines.h | fed6de422b6fa1b945989507425f4ac513895a7a44a493a7bf94ab0928052c4e |

**L131 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/server/nserver.c | 2bd3b7c1f4c7cf5a27c144425d39e49c967fbafbf5ec44f199e8f67c745206e9 |

**L132 disposition → local option/macro/audio/file/Guide/Lua integration is ticket 11; newest/autoinscription/item paste reuse 09; unknown server grammar is chat.forward-slash with ticket 12 full server reconciliation. Keep the original gate below.**

| src/server/util.c | 58fb9ce7fc2cd39f4a457cfbb9b4579a2a40b85da44e037ca61545512af0d5dc |


### `.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md`

SHA-256 `71585ae2b09367c05ef9ba4b25d24db218fd6f5f430f19231e7efae1d6da0fc3`.

**L52 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| Party/iron-team create/name, add/join target, remove target, hostility target, peace target | Each menu E=79, wire `%s`80. Receive_party clips40 and replaces plain-char `<32` with `_`; signed-char high bytes therefore replaced. Create/rename legal function further rejects >=20 bytes and imposes group alphabet/minimum/censor/collision checks | Each command keeps distinct semantics; no generic party name for target fields. Esc no send; server party/player state owns success; [c-cmd.c:8820](../../../src/client/c-cmd.c#L8820), [nserver.c:15051](../../../src/server/nserver.c#L15051), [party.c:1407](../../../src/server/party.c#L1407) |

**L68 → `capability.server-flow.quest-reply`.**

| `id >= RID_QUEST` | `str[30]=0`, then `quest_reply(Ind,id-RID_QUEST,str)` including ESC; quest-specific keywords/stages are data. No universal accepted alphabet. [10739](../../../src/server/xtra1.c#L10739) |

**L69 → `capability.server-flow.go`.**

| `RID_GO_MOVE` / ENABLE_GO_GAME | Discard if left building; `str[160]=0` (not a promise 160 bytes can arrive); go_engine_move_human owns coordinate/command parse and invalid-move retry. [10753](../../../src/server/xtra1.c#L10753), [go.c:1575](../../../src/server/go.c#L1575) |

**L70 → `capability.server-flow.guild-rename`.**

| `RID_GUILD_RENAME` | Clip40; ESC or empty means cancel; guild_rename invokes stricter name<20 legal checks. [10760](../../../src/server/xtra1.c#L10760) |

**L71 → `capability.server-flow.item-order`.**

| `RID_ITEM_ORDER` / ENABLE_ITEM_ORDER | Case-insensitive `cancel` has explicit active-order behavior; clip40, trim spaces, collapse repeated spaces into local `str2[40]`, parse article/count and case-insensitive item/spell names. **40-byte source plus NUL into 40-byte str2 boundary requires review.** Store/order state owns persistence. [10767](../../../src/server/xtra1.c#L10767) |

**L72 → `capability.server-flow.mail-item`.**

| `RID_SEND_ITEM` | Uppercase first byte, lookup player ID/account, validate item/fee/mode; unknown ESC suppresses unknown-addressee message. Mail state owns send. [11205](../../../src/server/xtra1.c#L11205) |

**L73 → `capability.server-flow.mail-gold`.**

| `RID_SEND_GOLD` | Same byte first-letter/lookup semantics, independent funds/fee/mode flow. [11427](../../../src/server/xtra1.c#L11427) |

**L76 → `capability.server-flow.contact-owner`.**

| `RID_CONTACT_OWNER` / PLAYER_STORES | Censor, quota/store/account validation; clip at MSG_LEN-CNAME_LEN-1=239 even though incoming wire delivers<=79; may notify online recipient and persist note. No explicit generic ESC suppression in this case. [11563](../../../src/server/xtra1.c#L11563) |

**L132 → `capability.information.close-sheet`, `capability.information.navigate-sheet`, `capability.information.read-equipment-flags`, `capability.information.read-history`, `capability.information.read-sheet`.**

| [c-cmd.c:2351](../../../src/client/c-cmd.c#L2351) | `cmd_character`; delegate | `if (get_string("Filename (you can post it to https://angband.live): ", tmp, MAX_CHARS - 1)) {` |

**L142 → `capability.information.local-file-bookmarks`, `capability.information.local-file-close`, `capability.information.local-file-navigate`, `capability.information.local-file-read`, `capability.information.local-file-search`.**

| [c-cmd.c:5647](../../../src/client/c-cmd.c#L5647) | `browse_local_file`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |

**L143 → `capability.information.local-file-bookmarks`, `capability.information.local-file-close`, `capability.information.local-file-navigate`, `capability.information.local-file-read`, `capability.information.local-file-search`.**

| [c-cmd.c:5686](../../../src/client/c-cmd.c#L5686) | `browse_local_file`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |

**L144 → `capability.information.local-file-bookmarks`, `capability.information.local-file-close`, `capability.information.local-file-navigate`, `capability.information.local-file-read`, `capability.information.local-file-search`.**

| [c-cmd.c:5780](../../../src/client/c-cmd.c#L5780) | `browse_local_file`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |

**L145 → `capability.information.local-file-bookmarks`, `capability.information.local-file-close`, `capability.information.local-file-navigate`, `capability.information.local-file-read`, `capability.information.local-file-search`.**

| [c-cmd.c:5826](../../../src/client/c-cmd.c#L5826) | `browse_local_file`; delegate | `if (!askfor_aux(buf, 7, 0)) {` |

**L146 → `capability.information.notes-close`, `capability.information.notes-navigate`, `capability.information.notes-read`, `capability.information.notes-search`.**

| [c-cmd.c:7567](../../../src/client/c-cmd.c#L7567) | `cmd_notes`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |

**L147 → `capability.chat.cancel`, `capability.chat.forward-slash`, `capability.chat.history`, `capability.chat.local-self`, `capability.chat.send`, `capability.chat.substitute-items`, `capability.information.client-version`, `capability.information.local-time`, `capability.special-store.wager`.**

| [c-cmd.c:8119](../../../src/client/c-cmd.c#L8119) | `cmd_message`; delegate | `if (get_string("Message: ", buf, sizeof(buf) - 1)) {` |

**L148 → `capability.social.guild-allow-adders`, `capability.social.guild-auto-readd`, `capability.social.guild-list-adders`, `capability.social.guild-minimum-level`, `capability.social.guild-options-close`, `capability.social.guild-toggle-adder`.**

| [c-cmd.c:8698](../../../src/client/c-cmd.c#L8698) | `cmd_guild_options`; delegate | `if (!get_string("Specify new minimum level: ", buf0, 4)) continue;` |

**L149 → `capability.social.guild-allow-adders`, `capability.social.guild-auto-readd`, `capability.social.guild-list-adders`, `capability.social.guild-minimum-level`, `capability.social.guild-options-close`, `capability.social.guild-toggle-adder`.**

| [c-cmd.c:8705](../../../src/client/c-cmd.c#L8705) | `cmd_guild_options`; delegate | `if (!get_string("Specify player name: ", buf0, NAME_LEN)) continue;` |

**L150 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8820](../../../src/client/c-cmd.c#L8820) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_CREATE, buf);` |

**L151 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8827](../../../src/client/c-cmd.c#L8827) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_CREATE_IRONTEAM, buf);` |

**L152 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8835](../../../src/client/c-cmd.c#L8835) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_ADD, buf);` |

**L153 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8839](../../../src/client/c-cmd.c#L8839) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_ADD, buf);` |

**L154 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8847](../../../src/client/c-cmd.c#L8847) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_DELETE, buf);` |

**L155 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8865](../../../src/client/c-cmd.c#L8865) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_HOSTILE, buf);` |

**L156 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8871](../../../src/client/c-cmd.c#L8871) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_PEACE, buf);` |

**L157 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8878](../../../src/client/c-cmd.c#L8878) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_guild(GUILD_CREATE, buf);` |

**L158 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8883](../../../src/client/c-cmd.c#L8883) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_guild(GUILD_ADD, buf);` |

**L159 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8887](../../../src/client/c-cmd.c#L8887) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_guild(GUILD_ADD, buf);` |

**L160 → `capability.social.add-guild-member`, `capability.social.add-party-member`, `capability.social.close`, `capability.social.close-iron-team`, `capability.social.create-guild`, `capability.social.create-iron-team`, `capability.social.create-party`, `capability.social.hostile`, `capability.social.join-guild`, `capability.social.join-party`, `capability.social.leave-guild`, `capability.social.leave-party`, `capability.social.peace`, `capability.social.read-guild`, `capability.social.read-party`, `capability.social.remove-guild-member`, `capability.social.remove-party-member`.**

| [c-cmd.c:8892](../../../src/client/c-cmd.c#L8892) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_guild(GUILD_DELETE, buf);` |

**L162 → `capability.housing.owner-guild`, `capability.housing.owner-player`.**

| [c-cmd.c:9111](../../../src/client/c-cmd.c#L9111) | `cmd_house_chown`; delegate | `if (get_string("Enter new name: ", &buf[2], 60))` |

**L163 → `capability.housing.tag`.**

| [c-cmd.c:9193](../../../src/client/c-cmd.c#L9193) | `cmd_house_tag`; delegate | `get_string("Enter a tag for the houses list (max 19 characters): ", &buf[1], 20 - 1);` |

**L164 → `capability.admin.level-add-dungeon`, `capability.admin.level-blank-module`, `capability.admin.level-create-town`, `capability.admin.level-load-module`, `capability.admin.level-module-entry`, `capability.admin.level-perma-static`, `capability.admin.level-remove-dungeon`, `capability.admin.level-save-module`, `capability.admin.level-static`, `capability.admin.level-unperma-static`, `capability.admin.level-unstatic`.**

| [c-cmd.c:9380](../../../src/client/c-cmd.c#L9380) | `cmd_master_aux_level`; delegate | `if (!get_string("Theme (ESC/0 = default vanilla, +100 to set type instead): ", ts, 3)) t = 0;` |

**L165 → `capability.admin.level-add-dungeon`, `capability.admin.level-blank-module`, `capability.admin.level-create-town`, `capability.admin.level-load-module`, `capability.admin.level-module-entry`, `capability.admin.level-perma-static`, `capability.admin.level-remove-dungeon`, `capability.admin.level-save-module`, `capability.admin.level-static`, `capability.admin.level-unperma-static`, `capability.admin.level-unstatic`.**

| [c-cmd.c:9449](../../../src/client/c-cmd.c#L9449) | `cmd_master_aux_level`; delegate | `(void)get_string("Custom DF1 flags (string of 8 hex chars, logical OR): ", fshextmp, 8);` |

**L166 → `capability.admin.level-add-dungeon`, `capability.admin.level-blank-module`, `capability.admin.level-create-town`, `capability.admin.level-load-module`, `capability.admin.level-module-entry`, `capability.admin.level-perma-static`, `capability.admin.level-remove-dungeon`, `capability.admin.level-save-module`, `capability.admin.level-static`, `capability.admin.level-unperma-static`, `capability.admin.level-unstatic`.**

| [c-cmd.c:9455](../../../src/client/c-cmd.c#L9455) | `cmd_master_aux_level`; delegate | `(void)get_string("Custom DF2 flags (string of 8 hex chars, logical OR): ", fshextmp, 8);` |

**L167 → `capability.admin.level-add-dungeon`, `capability.admin.level-blank-module`, `capability.admin.level-create-town`, `capability.admin.level-load-module`, `capability.admin.level-module-entry`, `capability.admin.level-perma-static`, `capability.admin.level-remove-dungeon`, `capability.admin.level-save-module`, `capability.admin.level-static`, `capability.admin.level-unperma-static`, `capability.admin.level-unstatic`.**

| [c-cmd.c:9461](../../../src/client/c-cmd.c#L9461) | `cmd_master_aux_level`; delegate | `(void)get_string("Custom DF3 flags (string of 8 hex chars, logical OR): ", fshextmp, 8);` |

**L168 → `capability.admin.level-add-dungeon`, `capability.admin.level-blank-module`, `capability.admin.level-create-town`, `capability.admin.level-load-module`, `capability.admin.level-module-entry`, `capability.admin.level-perma-static`, `capability.admin.level-remove-dungeon`, `capability.admin.level-save-module`, `capability.admin.level-static`, `capability.admin.level-unperma-static`, `capability.admin.level-unstatic`.**

| [c-cmd.c:9489](../../../src/client/c-cmd.c#L9489) | `cmd_master_aux_level`; delegate | `get_string("Save module name (max 19 char): ", &buf[1], 19);` |

**L169 → `capability.admin.level-add-dungeon`, `capability.admin.level-blank-module`, `capability.admin.level-create-town`, `capability.admin.level-load-module`, `capability.admin.level-module-entry`, `capability.admin.level-perma-static`, `capability.admin.level-remove-dungeon`, `capability.admin.level-save-module`, `capability.admin.level-static`, `capability.admin.level-unperma-static`, `capability.admin.level-unstatic`.**

| [c-cmd.c:9494](../../../src/client/c-cmd.c#L9494) | `cmd_master_aux_level`; delegate | `get_string("Load module name (max 19 char): ", &buf[1], 19);` |

**L170 → `capability.admin.level-add-dungeon`, `capability.admin.level-blank-module`, `capability.admin.level-create-town`, `capability.admin.level-load-module`, `capability.admin.level-module-entry`, `capability.admin.level-perma-static`, `capability.admin.level-remove-dungeon`, `capability.admin.level-save-module`, `capability.admin.level-static`, `capability.admin.level-unperma-static`, `capability.admin.level-unstatic`.**

| [c-cmd.c:9499](../../../src/client/c-cmd.c#L9499) | `cmd_master_aux_level`; delegate | `get_string("WxH string (eg. 1x1-5x5): ", &buf[1], 19);` |

**L171 → `capability.admin.level-add-dungeon`, `capability.admin.level-blank-module`, `capability.admin.level-create-town`, `capability.admin.level-load-module`, `capability.admin.level-module-entry`, `capability.admin.level-perma-static`, `capability.admin.level-remove-dungeon`, `capability.admin.level-save-module`, `capability.admin.level-static`, `capability.admin.level-unperma-static`, `capability.admin.level-unstatic`.**

| [c-cmd.c:9503](../../../src/client/c-cmd.c#L9503) | `cmd_master_aux_level`; delegate | `get_string("Set level entry (> < or +): ", &buf[0], 1);` |

**L172 → `capability.admin.generate-vault`.**

| [c-cmd.c:9571](../../../src/client/c-cmd.c#L9571) | `cmd_master_aux_generate_vault`; delegate | `get_string("Enter vault name: ", &buf[2], 77);` |

**L173 → `capability.admin.build-dead-tree`, `capability.admin.build-dirt`, `capability.admin.build-disable`, `capability.admin.build-feature`, `capability.admin.build-floor`, `capability.admin.build-grass`, `capability.admin.build-house-door`, `capability.admin.build-info-mode`, `capability.admin.build-info2-mode`, `capability.admin.build-permanent-wall`, `capability.admin.build-set-info`, `capability.admin.build-set-info2`, `capability.admin.build-sign`, `capability.admin.build-tree`, `capability.admin.build-wall`.**

| [c-cmd.c:9736](../../../src/client/c-cmd.c#L9736) | `cmd_master_aux_build`; delegate | `get_string("Sign: ", &buf[2], 77);` |

**L174 → `capability.admin.summon-depth`, `capability.admin.summon-disable`, `capability.admin.summon-high-undead`, `capability.admin.summon-low-undead`, `capability.admin.summon-name`, `capability.admin.summon-obliterate`, `capability.admin.summon-orcs`.**

| [c-cmd.c:10154](../../../src/client/c-cmd.c#L10154) | `cmd_master_aux_summon`; delegate | `get_string("Summon which monster or character? ", &buf[3], 79 - 3);` |

**L175 → `capability.admin.player-acquirement`, `capability.admin.player-broadcast`, `capability.admin.player-delete`, `capability.admin.player-edit`, `capability.admin.player-kill`, `capability.admin.player-static`, `capability.admin.player-telekinesis`, `capability.admin.player-unstatic`.**

| [c-cmd.c:10304](../../../src/client/c-cmd.c#L10304) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |

**L176 → `capability.admin.player-acquirement`, `capability.admin.player-broadcast`, `capability.admin.player-delete`, `capability.admin.player-edit`, `capability.admin.player-kill`, `capability.admin.player-static`, `capability.admin.player-telekinesis`, `capability.admin.player-unstatic`.**

| [c-cmd.c:10308](../../../src/client/c-cmd.c#L10308) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |

**L177 → `capability.admin.player-acquirement`, `capability.admin.player-broadcast`, `capability.admin.player-delete`, `capability.admin.player-edit`, `capability.admin.player-kill`, `capability.admin.player-static`, `capability.admin.player-telekinesis`, `capability.admin.player-unstatic`.**

| [c-cmd.c:10312](../../../src/client/c-cmd.c#L10312) | `cmd_master_aux_player`; delegate | `get_string("Enter player name (prefix with '!' for no-ghost kill): ", &buf[1], 15);` |

**L178 → `capability.admin.player-acquirement`, `capability.admin.player-broadcast`, `capability.admin.player-delete`, `capability.admin.player-edit`, `capability.admin.player-kill`, `capability.admin.player-static`, `capability.admin.player-telekinesis`, `capability.admin.player-unstatic`.**

| [c-cmd.c:10316](../../../src/client/c-cmd.c#L10316) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |

**L179 → `capability.admin.player-acquirement`, `capability.admin.player-broadcast`, `capability.admin.player-delete`, `capability.admin.player-edit`, `capability.admin.player-kill`, `capability.admin.player-static`, `capability.admin.player-telekinesis`, `capability.admin.player-unstatic`.**

| [c-cmd.c:10320](../../../src/client/c-cmd.c#L10320) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |

**L180 → `capability.admin.player-acquirement`, `capability.admin.player-broadcast`, `capability.admin.player-delete`, `capability.admin.player-edit`, `capability.admin.player-kill`, `capability.admin.player-static`, `capability.admin.player-telekinesis`, `capability.admin.player-unstatic`.**

| [c-cmd.c:10324](../../../src/client/c-cmd.c#L10324) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |

**L181 → `capability.admin.player-acquirement`, `capability.admin.player-broadcast`, `capability.admin.player-delete`, `capability.admin.player-edit`, `capability.admin.player-kill`, `capability.admin.player-static`, `capability.admin.player-telekinesis`, `capability.admin.player-unstatic`.**

| [c-cmd.c:10329](../../../src/client/c-cmd.c#L10329) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |

**L182 → `capability.admin.player-acquirement`, `capability.admin.player-broadcast`, `capability.admin.player-delete`, `capability.admin.player-edit`, `capability.admin.player-kill`, `capability.admin.player-static`, `capability.admin.player-telekinesis`, `capability.admin.player-unstatic`.**

| [c-cmd.c:10335](../../../src/client/c-cmd.c#L10335) | `cmd_master_aux_player`; delegate | `get_string("Message: ", &buf[1], 69);` |

**L183 → `capability.admin.upload-script`.**

| [c-cmd.c:10375](../../../src/client/c-cmd.c#L10375) | `cmd_script_upload`; delegate | `if (!get_string("Script name: ", name, 30)) return;` |

**L184 → `capability.admin.server-script`.**

| [c-cmd.c:10393](../../../src/client/c-cmd.c#L10393) | `cmd_script_exec`; delegate | `if (!get_string("Script> ", buf, 80)) return;` |

**L185 → `capability.admin.local-script`.**

| [c-cmd.c:10402](../../../src/client/c-cmd.c#L10402) | `cmd_script_exec_local`; delegate | `if (!get_string("Script> ", buf, 80)) return;` |

**L186 → `capability.documents.close`, `capability.documents.navigate`, `capability.documents.read`, `capability.documents.refresh-players`, `capability.documents.search`.**

| [c-files.c:2057](../../../src/client/c-files.c#L2057) | `peruse_file`; delegate | `if (askfor_aux(tmp, 10, 0)) cur_line = atoi(tmp);` |

**L187 → `capability.documents.close`, `capability.documents.navigate`, `capability.documents.read`, `capability.documents.refresh-players`, `capability.documents.search`.**

| [c-files.c:2067](../../../src/client/c-files.c#L2067) | `peruse_file`; delegate | `if (askfor_aux(tmp, 60, 0)) {` |

**L188 → `capability.documents.close`, `capability.documents.navigate`, `capability.documents.read`, `capability.documents.refresh-players`, `capability.documents.search`.**

| [c-files.c:2084](../../../src/client/c-files.c#L2084) | `peruse_file`; delegate | `if (askfor_aux(tmp, 60, 0)) {` |

**L268 → `capability.messages.recall-close`, `capability.messages.recall-navigate`, `capability.messages.recall-read`, `capability.messages.recall-search`.**

| [c-xtra2.c:346](../../../src/client/c-xtra2.c#L346) | `do_cmd_messages`; delegate | `if (askfor_aux(tmp, 79, 0)) {` |

**L269 → `capability.messages.recall-close`, `capability.messages.recall-navigate`, `capability.messages.recall-read`, `capability.messages.recall-search`.**

| [c-xtra2.c:385](../../../src/client/c-xtra2.c#L385) | `do_cmd_messages`; delegate | `if (!askfor_aux(shower, 79, 0)) {` |

**L270 → `capability.messages.recall-close`, `capability.messages.recall-navigate`, `capability.messages.recall-read`, `capability.messages.recall-search`.**

| [c-xtra2.c:408](../../../src/client/c-xtra2.c#L408) | `do_cmd_messages`; delegate | `if (!askfor_aux(finder, 79, 0)) {` |

**L271 → `capability.messages.recall-close`, `capability.messages.recall-navigate`, `capability.messages.recall-read`, `capability.messages.recall-search`.**

| [c-xtra2.c:453](../../../src/client/c-xtra2.c#L453) | `do_cmd_messages`; delegate | `if (!askfor_aux(finder, 79, 0)) {` |

**L272 → `capability.messages.recall-close`, `capability.messages.recall-navigate`, `capability.messages.recall-read`, `capability.messages.recall-search`.**

| [c-xtra2.c:550](../../../src/client/c-xtra2.c#L550) | `do_cmd_messages`; delegate | `if (get_string("Filename: ", tmp, 79)) {` |

**L273 → `capability.messages.important-close`, `capability.messages.important-navigate`, `capability.messages.important-read`, `capability.messages.important-search`.**

| [c-xtra2.c:764](../../../src/client/c-xtra2.c#L764) | `do_cmd_messages_important`; delegate | `if (askfor_aux(tmp, 79, 0)) {` |

**L274 → `capability.messages.important-close`, `capability.messages.important-navigate`, `capability.messages.important-read`, `capability.messages.important-search`.**

| [c-xtra2.c:804](../../../src/client/c-xtra2.c#L804) | `do_cmd_messages_important`; delegate | `if (!askfor_aux(shower, 79, 0)) {` |

**L275 → `capability.messages.important-close`, `capability.messages.important-navigate`, `capability.messages.important-read`, `capability.messages.important-search`.**

| [c-xtra2.c:826](../../../src/client/c-xtra2.c#L826) | `do_cmd_messages_important`; delegate | `if (!askfor_aux(finder, 79, 0)) {` |

**L276 → `capability.messages.important-close`, `capability.messages.important-navigate`, `capability.messages.important-read`, `capability.messages.important-search`.**

| [c-xtra2.c:871](../../../src/client/c-xtra2.c#L871) | `do_cmd_messages_important`; delegate | `if (!askfor_aux(finder, 79, 0)) {` |

**L277 → `capability.messages.important-close`, `capability.messages.important-navigate`, `capability.messages.important-read`, `capability.messages.important-search`.**

| [c-xtra2.c:969](../../../src/client/c-xtra2.c#L969) | `do_cmd_messages_important`; delegate | `if (get_string("Filename: ", tmp, 79)) {` |

**L285 → `capability.request.answer-str`, `capability.request.cancel-str`.**

| [nclient.c:7221](../../../src/client/nclient.c#L7221) | `Receive_request_str`; delegate | `if (get_string(prompt, buf, MAX_CHARS_WIDE - 1)) Send_request_str(id, buf);` |

**L360 → `capability.messages.read-live`.**

| [nclient.c:3346](../../../src/client/nclient.c#L3346) | `Receive_message`; inbound | `Packet_scanf(&rbuf, "%c%S", &ch, buf)` |

**L379 → `capability.special-store.text`.**

| [nclient.c:4669](../../../src/client/nclient.c#L4669) | `Receive_store_special_str`; inbound | `Packet_scanf(&rbuf, "%c%c%c%c%s", &ch, &line, &col, &attr, str)` |

**L380 → `capability.special-store.recreate`.**

| [nclient.c:5453](../../../src/client/nclient.c#L5453) | `Receive_store_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%d%c%c%c", &ch, &store_num, store_name, owner_name, &num_items, &max_cost, &store_attr, &store_char, &store_price_mul)` |

**L381 → `capability.special-store.recreate`.**

| [nclient.c:5455](../../../src/client/nclient.c#L5455) | `Receive_store_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%d%c%c", &ch, &store_num, store_name, owner_name, &num_items, &max_cost, &store_attr, &store_char)` |

**L382 → `capability.special-store.recreate`.**

| [nclient.c:5457](../../../src/client/nclient.c#L5457) | `Receive_store_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%d", &ch, &store_num, store_name, owner_name, &num_items, &max_cost)` |

**L385 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:5813](../../../src/client/nclient.c#L5813) | `Receive_special_line`; inbound | `Packet_scanf(&rbuf, "%c%d%d%c%I", &ch, &max, &line, &attr, buf)` |

**L386 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:5817](../../../src/client/nclient.c#L5817) | `Receive_special_line`; inbound | `Packet_scanf(&rbuf, "%c%hd%hd%c%I", &ch, &old_max, &old_line, &attr, buf)` |

**L388 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:5976](../../../src/client/nclient.c#L5976) | `Receive_party_stats`; inbound | `Packet_scanf(&rbuf, "%c%d%d%s%d%d%d%d%d", &ch, &j, &color, &partymembername, &k, &chp, &mhp, &cmp, &mmp)` |

**L389 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:5989](../../../src/client/nclient.c#L5989) | `Receive_party`; inbound | `Packet_scanf(&rbuf, "%c%s%s%s", &ch, pname, pmembers, powner)` |

**L390 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:6040](../../../src/client/nclient.c#L6040) | `Receive_guild`; inbound | `Packet_scanf(&rbuf, "%c%s%s%s", &ch, gname, gmembers, gowner)` |

**L391 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:6092](../../../src/client/nclient.c#L6092) | `Receive_guild_config`; inbound | `Packet_scanf(&rbuf, "%s", dummy)` |

**L392 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:6096](../../../src/client/nclient.c#L6096) | `Receive_guild_config`; inbound | `Packet_scanf(&rbuf, "%s", guild.adder[i])` |

**L397 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:7181](../../../src/client/nclient.c#L7181) | `Receive_request_key`; inbound | `Packet_scanf(&rbuf, "%c%d%s", &ch, &id, prompt)` |

**L398 → `capability.request.answer-amt`, `capability.request.cancel-amt`.**

| [nclient.c:7194](../../../src/client/nclient.c#L7194) | `Receive_request_amt`; inbound | `Packet_scanf(&rbuf, "%c%d%s%d", &ch, &id, prompt, &max)` |

**L399 → `capability.request.answer-num`, `capability.request.cancel-num`.**

| [nclient.c:7206](../../../src/client/nclient.c#L7206) | `Receive_request_num`; inbound | `Packet_scanf(&rbuf, "%c%d%s%d%d%d", &ch, &id, prompt, &predef, &min, &max)` |

**L400 → `capability.request.answer-str`, `capability.request.cancel-str`.**

| [nclient.c:7218](../../../src/client/nclient.c#L7218) | `Receive_request_str`; inbound | `Packet_scanf(&rbuf, "%c%d%s%s", &ch, &id, prompt, buf)` |

**L401 → `capability.request.answer-cfr`, `capability.request.cancel-cfr`.**

| [nclient.c:7235](../../../src/client/nclient.c#L7235) | `Receive_request_cfr`; inbound | `Packet_scanf(&rbuf, "%c%d%s%c", &ch, &id, prompt, &dy)` |

**L402 → `capability.request.answer-cfr`, `capability.request.cancel-cfr`.**

| [nclient.c:7238](../../../src/client/nclient.c#L7238) | `Receive_request_cfr`; inbound | `Packet_scanf(&rbuf, "%c%d%s", &ch, &id, prompt)` |

**L411 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:7983](../../../src/client/nclient.c#L7983) | `Send_special_line`; outbound | `Packet_printf(&wbuf, "%c%c%d%s", PKT_SPECIAL_LINE, type, line, srcstr ? srcstr : "")` |

**L424 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nclient.c:9059](../../../src/client/nclient.c#L9059) | `Send_request_str`; outbound | `Packet_printf(&wbuf, "%c%d%s", PKT_REQUEST_STR, id, str)` |

**L449 → `capability.messages.read-live`.**

| [nserver.c:14038](../../../src/server/nserver.c#L14038) | `Receive_message` | `Packet_scanf(&connp->r, "%c%S", &ch, buf)` |

**L450 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nserver.c:14063](../../../src/server/nserver.c#L14063) | `Receive_admin_house` | `Packet_scanf(&connp->r, "%c%hd%s", &ch, &dir, buf)` |

**L451 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nserver.c:14703](../../../src/server/nserver.c#L14703) | `Receive_special_line` | `Packet_scanf(&connp->r, "%c%c%d%s", &ch, &type, &line, srcstr)` |

**L452 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nserver.c:15068](../../../src/server/nserver.c#L15068) | `Receive_party` | `Packet_scanf(&connp->r, "%c%hd%s", &ch, &command, buf)` |

**L453 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nserver.c:15122](../../../src/server/nserver.c#L15122) | `Receive_guild` | `Packet_scanf(&connp->r, "%c%hd%s", &ch, &command, buf)` |

**L454 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nserver.c:15157](../../../src/server/nserver.c#L15157) | `Receive_guild_config` | `Packet_scanf(&connp->r, "%c%d%d%s", &ch, &command, &flags, adder)` |

**L455 → corresponding documents / special-store / request / chat / social / admin / housing outcomes above; exact protocol source below.**

| [nserver.c:15423](../../../src/server/nserver.c#L15423) | `Receive_master` | `Packet_scanf(&connp->r, "%c%hd%s", &ch, &command, buf)` |

**L458 → `capability.request.answer-str`, `capability.request.cancel-str`.**

| [nserver.c:16224](../../../src/server/nserver.c#L16224) | `Receive_request_str` | `Packet_scanf(&connp->r, "%c%d%s", &ch, &id, str)` |


## Versioned wire owners

These pinned source excerpts are normative scenarios for the associated row(s), not additional capability IDs. Receivers publish only complete packets. Source-root validation fingerprints the complete files.

### Receive_guild

```c
int Receive_guild(void) {
	int n;
	char ch, gname[MAX_CHARS], gmembers[MAX_CHARS], gowner[MAX_CHARS];

	if ((n = Packet_scanf(&rbuf, "%c%s%s%s", &ch, gname, gmembers, gowner)) <= 0) return(n);

	/* Copy info */
	strcpy(guild_info_name, gname);
	strcpy(guild_info_members, gmembers);
	strcpy(guild_info_owner, gowner);

	if (chat_mode == CHAT_MODE_GUILD && !guild_info_name[0]) chat_mode = CHAT_MODE_NORMAL;

	/* Re-show party info */
	if (party_mode) {
		Term_erase(0, 18, 70);
		Term_erase(0, 22, 90);
		Term_putstr(0, 18, -1, TERM_WHITE, "Command: ");
		if (strlen(gname)) Term_putstr(0, 22, -1, TERM_WHITE, format("%s (%s, %s)", gname, gmembers, gowner));
		else Term_putstr(0, 22, -1, TERM_SLATE, "(You are not in a guild.)");

		if (guild_info_name[0]) Term_putstr(5, 9, -1, TERM_WHITE, "(\377Ub\377w) Add a player to guild");
		else Term_putstr(5, 9, -1, TERM_WHITE, "(\377Ub\377w) Add yourself to guild");
	}

	return(1);
}
```

### Receive_guild_config

```c
int Receive_guild_config(void) {
	int i, n, master, guild_adders, ghp;
	char ch, dummy[MAX_CHARS];
	int x, y;
	int minlev_32b; /* 32 bits transmitted over the network gets converted to 16 bits */
	char *stored_sbuf_ptr = rbuf.ptr;
	Term_locate(&x, &y);

	if ((n = Packet_scanf(&rbuf, "%c%d%d%d%d%d%d%d", &ch, &master, &guild.flags, &minlev_32b, &guild_adders, &guildhall_wx, &guildhall_wy, &ghp)) <= 0) return(n);
	guild.minlev = minlev_32b;
	switch (ghp) {
	case 0: strcpy(guildhall_pos, "north-western"); break;
	case 4: strcpy(guildhall_pos, "northern"); break;
	case 8: strcpy(guildhall_pos, "north-eastern"); break;
	case 1: strcpy(guildhall_pos, "western"); break;
	case 5: strcpy(guildhall_pos, "central"); break;
	case 9: strcpy(guildhall_pos, "eastern"); break;
	case 2: strcpy(guildhall_pos, "south-western"); break;
	case 6: strcpy(guildhall_pos, "southern"); break;
	case 10: strcpy(guildhall_pos, "south-eastern"); break;
	default: strcpy(guildhall_pos, "unknown");
	}
	if (master) guild_master = TRUE;
	else guild_master = FALSE;

	for (i = 0; i < 5; i++) guild.adder[i][0] = 0;
	for (i = 0; i < guild_adders; i++) {
		if (i >= 5) {
			n = Packet_scanf(&rbuf, "%s", dummy);
			if (n == 0) goto rollback;
			else if (n < 0) return(n);
		} else {
			n = Packet_scanf(&rbuf, "%s", guild.adder[i]);
			if (n == 0) goto rollback;
			else if (n < 0) return(n);
		}
	}

	/* Re-show guild config info -- in theory this can't happen if there is only 1 guild master, but w/e */
	if (guildcfg_mode) {
		int acnt = 0;
		char buf[(NAME_LEN + 1) * 5 + 1];

		if (guildhall_wx == -1) Term_putstr(5, 2, -1, TERM_SLATE, "The guild does not own a guild hall.");
		else if (guildhall_wx >= 0) Term_putstr(5, 2, -1, TERM_L_UMBER, format("The guild hall is located in the %s area of (%d,%d).",
		    guildhall_pos, guildhall_wx, guildhall_wy));
		Term_putstr(5, 4, -1, TERM_WHITE,  format("adders     : %s", guild.flags & GFLG_ALLOW_ADDERS ? "\377GYES" : "\377rno "));
		Term_putstr(5, 5, -1, TERM_L_WHITE,       "    Allows players designated via /guild_adder command to add others.");
		Term_putstr(5, 6, -1, TERM_WHITE,  format("autoreadd  : %s", guild.flags & GFLG_AUTO_READD ? "\377GYES" : "\377rno "));
		Term_putstr(5, 7, -1, TERM_L_WHITE,      "    If a guild mate ghost-dies then the next character he logs on with");
		Term_putstr(5, 8, -1, TERM_L_WHITE,      "    - if it is newly created - is automatically added to the guild again.");
		Term_putstr(5, 9, -1, TERM_WHITE, format("minlev     : \377%c%d   ", guild.minlev <= 1 ? 'w' : (guild.minlev <= 10 ? 'G' : (guild.minlev < 20 ? 'g' :
		    (guild.minlev < 30 ? 'y' : (guild.minlev < 40 ? 'o' : (guild.minlev <= 50 ? 'r' : 'v'))))), guild.minlev));
		Term_putstr(5, 10, -1, TERM_L_WHITE,      "    Minimum character level required to get added to the guild.");

		Term_erase(5, 11, 69);
		Term_erase(5, 12, 69);

		buf[0] = 0;
		for (i = 0; i < 5; i++) if (guild.adder[i][0] != '\0') {
			sprintf(buf, "Adders are: ");
			strcat(buf, guild.adder[i]);
			acnt++;
			for (i++; i < 5; i++) {
				if (guild.adder[i][0] == '\0') continue;
				if (acnt != 3) strcat(buf, ", ");
				strcat(buf, guild.adder[i]);
				acnt++;
				if (acnt == 3) {
					Term_putstr(5, 11, -1, TERM_SLATE, buf);
					buf[0] = 0;
				}
			}
		}
		Term_putstr(5 + (acnt <= 3 ? 0 : 12), acnt <= 3 ? 11 : 12, -1, TERM_SLATE, buf);
	}

	Term_gotoxy(x, y);
	return(1);

	/* Rollback the socket buffer in case the packet isn't complete */
	rollback:
	rbuf.ptr = stored_sbuf_ptr;
	return(0);
}
```

### Receive_history

```c
int Receive_history(void) {
	int n;
	char ch;
	s16b line;
	char buf[MAX_CHARS];

	if ((n = Packet_scanf(&rbuf, "%c%hu%s", &ch, &line, buf)) <= 0) return(n);

	strcpy(p_ptr->history[line], buf);

	/*printf("Received history line %d: %s\n", line, buf);*/

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return(1);
}
```

### Receive_message

```c
int Receive_message(void) {
	int n, c;
	char ch;
	char buf[MSG_LEN] = { 0 }, *bptr, *sptr, *bnptr;
	char l_buf[MSG_LEN], l_cname[NAME_LEN], *ptr, l_nick[NAME_LEN], called_name[NAME_LEN];
	static bool got_note = FALSE;

	if ((n = Packet_scanf(&rbuf, "%c%S", &ch, buf)) <= 0) return(n);

	/* Ultra-hack for light-source fainting. (First two bytes are "\377w".) */
	if (!c_cfg.no_lite_fainting && !strcmp(buf + 2, HCMSG_LIGHT_FAINT)) lamp_fainting = 30; //deciseconds

	/* Hack for tombstone music from insanity-deaths. (First four bytes are "\377w\377v".) */
	if (!strcmp(buf + 4, HCMSG_VEGETABLE)) insanity_death = TRUE;

	/* Hack for storing private messages to disk, in case we miss them; handle multi-line messages (subsequent lines start on a space) */
	if (!strncmp(buf + 6, HCMSG_NOTE, 10) || (got_note && buf[2] == ' ')) {
		FILE *fp;
		char path[1024];
		time_t ct = time(NULL);
		struct tm* ctl = localtime(&ct);

		path_build(path, 1024, ANGBAND_DIR_USER, format("notes-%s.txt", nick));
		fp = my_fopen(path, "a");
		if (fp) {
			char buf2[MSG_LEN] = { 0 }, *c = buf + (got_note ? 2 : 6 + 10), *c2 = buf2;

			while (*c) {
				switch (*c) {
				/* skip special markers */
				case '\376':
				case '\375':
				case '\374':
					c++;
					continue;
				/* strip colour codes */
				case '\377':
					switch (*(c + 1)) {
					case 0: /* broken colour code (paranoia) */
						c++;
						continue;
					default: /* assume colour code and discard */
						c += 2;
						continue;
					}
					break;
				}
				*c2 = *c;
				c++;
				c2++;
			}

			fprintf(fp, "[%04d/%02d/%02d - %02d:%02d] %s\n", 1900 + ctl->tm_year, ctl->tm_mon + 1, ctl->tm_mday, ctl->tm_hour, ctl->tm_min, buf2);
			fclose(fp);
		}
		got_note = TRUE;
	} else got_note = FALSE;

	/* Hack to clear topline: It's a translation of the former msg_print(Ind, NULL) hack, as we cannot transmit the NULL. */
	if (buf[0] == '\377' && !buf[1]) {
		if (screen_icky && (!shopping || perusing)) Term_switch(0);
		c_msg_print(NULL);
		if (screen_icky && (!shopping || perusing)) Term_switch(0);
		return(1);
	}

	/* XXX Mega-hack -- because we are not using checksums, sometimes under
	 * heavy traffic Receive_line_input receives corrupted data, which will cause
	 * the run-length encoded algorithm to exit prematurely.  Since there is no
	 * end of line_info tag, the packet interpretor assumes the line_input data
	 * is finished, and attempts to parse the next byte as a new packet type.
	 * Since the ascii value of '.' (a frequently updated character) is 46,
	 * which equals PKT_MESSAGE, if corrupted line_info data is received this function
	 * may get called wihtout a valid string.  To try to prevent the client from
	 * displaying a garbled string and messing up our screen, we will do a quick
	 * sanity check on the string.  Note that this might screw up people using
	 * international character sets.  (Such as the Japanese players)
	 *
	 * A better solution for this would be to impliment some kind of packet checksum.
	 * -APD
	 */

	/* perform a sanity check on our string */
	/* Hack -- ' ' is numericall the lowest charcter we will probably be trying to
	 * display.  This might screw up international character sets.
	 */
	for (c = 0; c < n; c++)
		if (buf[c] < ' ' && /* exempt control codes */
		    buf[c] != -1 && /* \377 colour code */
		    buf[c] != -2 && /* \376 important-scrollback code */
		    buf[c] != -3 && /* \375 chat-only code */
		    buf[c] != -4) /* \374 chat+no-chat code */
			return(1);

	if (screen_icky && (!shopping || perusing)) Term_switch(0);


	/* Highlight or beep on incoming chat messages containing our name?
	   Current weakness: Char 'Mim' (acc 'Test) won't get highlights from
	   another char 'Test' (acc 'Test') writing 'Mim' in chat. - C. Blue */
	if (c_cfg.hilite_chat || c_cfg.hibeep_chat) { /* enabled? */
		/* Test sender's name, if it is us */
		int we_sent_offset;
		char *we_sent_p = strchr(buf, '[');

		if (we_sent_p) {
			char we_sent_buf[NAME_LEN + 1 + 10], *we_sent_p_end;

			strncpy(we_sent_buf, we_sent_p + 1, NAME_LEN + 1 + 10);
			we_sent_buf[NAME_LEN + 10] = '\0';
			if ((we_sent_p_end = strchr(we_sent_buf, ']'))
			    && we_sent_p_end - we_sent_p <= NAME_LEN /* Prevent buffer overflow if the [...] wasn't a name but some longer text that was just within brackets for some reason */
			    ) {
				char exact_name[NAME_LEN + 1], *en_p = exact_name;

				/* we found SOME name, so don't test it again */
				we_sent_offset = strchr(buf, ']') - buf;
				/* and also check if name = us, strictly */
				we_sent_p = we_sent_buf - 1;
				while (*(++we_sent_p)) {
					/* skip colour codes (occur in the beginning and end of name, possibly) */
					if (*we_sent_p == '\\') {
						we_sent_p++;
						continue;
					}
					if (*we_sent_p == ']') break;
					*en_p++ = *we_sent_p;
				}
				*en_p = '\0';
				/* so, if it's not someone else talking, whose name _contains_ our name (eg 'Bat' vs 'FruitBat'),
				   then we can allow highlighting the (our) name again: */
				if (!strcmp(exact_name, cname)) we_sent_offset = 0;
			} else we_sent_offset = 0;
		} else we_sent_offset = 0; /* just paranoid initialization */

		/* Is it non-private chat? */
		if (strlen(buf) > 2 && //paranoia, not needed
		    (sptr = strchr(buf, '[')) /* a '[' occurs somewhere at the start? */
		    && sptr <= buf + 7 + ((strstr(buf, "(IRC)") <= buf + 7) ? 9 : 0)
		    //&& (*(sptr - 1) != 'y')
		    //&& (*(sptr - 1) != 'G')
		    && (*(sptr - 1) != 'g') /* and it's not coloured as a private message? */
		    && tolower(buf[2]) == toupper(buf[2]) /* even safer check, that it isn't a generic server message */
		    ) {
			/* my_strcasestr() */
			strcpy(l_buf, buf);
			ptr = l_buf;
			while (*ptr) { *ptr = tolower(*ptr); ptr++; }
			strcpy(l_cname, cname);
#ifdef CHARNAME_ROMAN /* Convenience feature: Ignore roman numbers at the end of our character name? (Must be separated by a space.) */
			if ((ptr = roman_suffix(l_cname, NULL))) *(ptr - 1) = 0;
#endif
			ptr = l_cname;
			while (*ptr) { *ptr = tolower(*ptr); ptr++; }
			strcpy(l_nick, nick);
			ptr = l_nick;
			while (*ptr) { *ptr = tolower(*ptr); ptr++; }
			/* map location found onto original string -_- */
			if ((bptr = strstr(l_buf + we_sent_offset, l_cname))) bptr = buf + (bptr - l_buf);
			if ((bnptr = strstr(l_buf, l_nick))) bnptr = buf + (bnptr - l_buf);

			/* check that our 'name' wasn't just part of a different string,
			   eg 'Heya' if our name is 'Eya' (important for very short names): */
			if (bptr && bptr > buf && *(bptr - 1) >= 'a' && *(bptr - 1) <= 'z') bptr = NULL;
			if (bptr && *(bptr + strlen(l_cname)) && *(bptr + strlen(l_cname)) >= 'a' && *(bptr + strlen(l_cname)) <= 'z') bptr = NULL;
			if (bnptr && bnptr > buf && *(bnptr - 1) >= 'a' && *(bnptr - 1) <= 'z') bnptr = NULL;
			if (bnptr && *(bnptr + strlen(l_nick)) && *(bnptr + strlen(l_nick)) >= 'a' && *(bnptr + strlen(l_nick)) <= 'z') bnptr = NULL;

			/* Check which one it is (first), character name or account name */
			if (bptr) {
				if (!bnptr || bptr <= bnptr) {
					/* keep the way our name was actually written (lower/upper-case) in the original chat message */
#ifndef CHARNAME_ROMAN
					strncpy(called_name, bptr, strlen(cname));
					called_name[strlen(cname)] = 0;
#else
					strncpy(called_name, bptr, strlen(l_cname));
					called_name[strlen(l_cname)] = 0;
#endif
				} else {
					bptr = bnptr;
					/* keep the way our name was actually written (lower/upper-case) in the original chat message */
					strncpy(called_name, bptr, strlen(nick));
					called_name[strlen(nick)] = 0;
				}
			} else if (bnptr) {
				bptr = bnptr;
				/* keep the way our name was actually written (lower/upper-case) in the original chat message */
				strncpy(called_name, bptr, strlen(nick));
				called_name[strlen(nick)] = 0;
			}

			/* our name occurs in the message? */
			if (bptr &&
			    bptr > sptr + 2 && /* and isn't the sender of this message? */
			    (!(ptr = strchr(sptr + 1, '(')) || ptr > sptr + 3 || bptr > ptr + 6)) { /* mind '(G)' and '(P)' */
				/* enough space to add colour codes for highlighting? */
				if (c_cfg.hilite_chat
				    && strlen(buf) < MSG_LEN - 4) {
					char buf2[MSG_LEN], *col_ptr = buf;
					int prev_colour = 'w';

					/* remember last colour used before our name occurred, so we can restore it */
					while (col_ptr < bptr) {
						if (*col_ptr == '\377') {
							col_ptr++;
							prev_colour = *col_ptr;
						} else col_ptr++;
					}

					strcpy(buf2, buf);
					strcpy(bptr, "\377R");
					strcpy(bptr + 2, called_name);
					strcpy(bptr + 2 + strlen(called_name), format("\377%c", prev_colour));
					strcpy(bptr + 4 + strlen(called_name), buf2 + (bptr - buf) + strlen(called_name));
				}

				/* also give audial feedback if enabled */
				if (c_cfg.hibeep_chat) page();
			}
		}
	}

	c_msg_print(buf);

	if (screen_icky && (!shopping || perusing)) Term_switch(0);

	/* For the casino: Cursor was visible after game result message (won/loss) */
	if (shopping) {
		/* hack: hide cursor */
		Term->scr->cx = Term->wid;
		Term->scr->cu = 1;
	}

	return(1);
}
```

### Receive_party

```c
int Receive_party(void) {
	int n;
	char ch, pname[MAX_CHARS], pmembers[MAX_CHARS], powner[MAX_CHARS];

	if ((n = Packet_scanf(&rbuf, "%c%s%s%s", &ch, pname, pmembers, powner)) <= 0) return(n);

	/* Copy info */
	strcpy(party_info_name, pname);
	strcpy(party_info_members, pmembers);
	strcpy(party_info_owner, powner);

	if (chat_mode == CHAT_MODE_PARTY && !party_info_name[0]) chat_mode = CHAT_MODE_NORMAL;

	/* Check for iron team state */
	if (!strncmp(party_info_name, "Iron Team", 9)) party_info_mode = PA_IRONTEAM; /* Normal (open) iron team */
	else if (!strncmp(party_info_name + 2, "Iron Team", 9)) party_info_mode = (PA_IRONTEAM | PA_IRONTEAM_CLOSED); /* Prefixed colour code indicates 'closed' iron team */
	else party_info_mode = PA_NORMAL; /* Normal party (or no party!) */

	/* Re-show party info */
	if (party_mode) {
		if (is_newer_than(&server_version, 4, 4, 7, 0, 0, 0)) {
			Term_erase(0, 18, 70);
			Term_erase(0, 21, 90);
			Term_putstr(0, 18, -1, TERM_WHITE, "Command: ");
			if (strlen(pname)) Term_putstr(0, 21, -1, TERM_WHITE, format("%s (%s, %s)", pname, pmembers, powner));
			else Term_putstr(0, 21, -1, TERM_SLATE, "(You are not in a party.)");

			if (party_info_name[0]) Term_putstr(5, 4, -1, TERM_WHITE, "(\377G3\377w) Add a player to party");
			else Term_putstr(5, 4, -1, TERM_WHITE, "(\377G3\377w) Add yourself to party");

			if (is_newer_than(&server_version, 4, 7, 1, 1, 0, 0)) {
				if ((party_info_mode & PA_IRONTEAM) && !(party_info_mode & PA_IRONTEAM_CLOSED))
					Term_putstr(40, 6, -1, TERM_WHITE, "(\377G0\377w) Close your iron team");
				else
					Term_putstr(40, 6, -1, TERM_WHITE, "                              ");
			}
		} else {
			Term_erase(0, 18, 70);
			Term_erase(0, 20, 90);
			Term_erase(0, 21, 20);
			Term_erase(0, 22, 50);
			Term_putstr(0, 18, -1, TERM_WHITE, "Command: ");
			Term_putstr(0, 20, -1, TERM_WHITE, pname);
			Term_putstr(0, 21, -1, TERM_WHITE, pmembers);
			Term_putstr(0, 22, -1, TERM_WHITE, powner);
		}
	}

	return(1);
}
```

### Receive_party_stats

```c
int Receive_party_stats(void) {
	int n, j, k, chp, mhp, cmp, mmp, color;
	char ch, partymembername[MAX_CHARS];

	if ((n = Packet_scanf(&rbuf, "%c%d%d%s%d%d%d%d%d", &ch, &j, &color, &partymembername, &k, &chp, &mhp, &cmp, &mmp)) <= 0) return(n);

	if (screen_icky) Term_switch(0);
	prt_party_stats(j, color, partymembername, k, chp, mhp, cmp, mmp);
	if (screen_icky) Term_switch(0);
	return(1);
}
```

### Receive_request_abort

```c
int Receive_request_abort(void) {
	int n;
	char ch;

	if ((n = Packet_scanf(&rbuf, "%c", &ch)) <= 0) return(n);
	if (request_pending) request_abort = TRUE;
	return(1);
}
```

### Receive_request_amt

```c
int Receive_request_amt(void) {
	int n, id, max;
	char ch, prompt[MAX_CHARS];

	if ((n = Packet_scanf(&rbuf, "%c%d%s%d", &ch, &id, prompt, &max)) <= 0) return(n);

	request_pending = TRUE;
	Send_request_amt(id, c_get_quantity(prompt, 0, max));
	request_pending = FALSE;
	return(1);
}
```

### Receive_request_cfr

```c
int Receive_request_cfr(void) {
	int n, id;
	char ch, prompt[MAX_CHARS];
	char default_choice = FALSE;

	if (is_newer_than(&server_version, 4, 5, 6, 0, 0, 1)) {
		char dy;

		if ((n = Packet_scanf(&rbuf, "%c%d%s%c", &ch, &id, prompt, &dy)) <= 0) return(n);
		default_choice = dy;
	} else {
		if ((n = Packet_scanf(&rbuf, "%c%d%s", &ch, &id, prompt)) <= 0) return(n);
	}

	request_pending = TRUE;
	Send_request_cfr(id, get_check3(prompt, default_choice));
	request_pending = FALSE;
	return(1);
}
```

### Receive_request_num

```c
int Receive_request_num(void) {
	int n, id, predef, min, max;
	char ch, prompt[MAX_CHARS];

	if ((n = Packet_scanf(&rbuf, "%c%d%s%d%d%d", &ch, &id, prompt, &predef, &min, &max)) <= 0) return(n);

	request_pending = TRUE;
	Send_request_num(id, c_get_number(prompt, predef, min, max));
	request_pending = FALSE;
	return(1);
}
```

### Receive_request_str

```c
int Receive_request_str(void) {
	int n, id;
	char ch, prompt[MAX_CHARS], buf[MAX_CHARS_WIDE];

	if ((n = Packet_scanf(&rbuf, "%c%d%s%s", &ch, &id, prompt, buf)) <= 0) return(n);

	request_pending = TRUE;
	if (get_string(prompt, buf, MAX_CHARS_WIDE - 1)) Send_request_str(id, buf);
	else Send_request_str(id, "\e");
	request_pending = FALSE;
	return(1);
}
```

### Receive_special_line

```c
int Receive_special_line(void) {
	int n, p;
	char ch, ab, ap;
	s32b max, line;
	byte attr;
	char buf[ONAME_LEN]; /* Allow colour codes! (was: MAX_CHARS, which is just 80) */
	int x, y, phys_line;
#ifdef REGEX_SEARCH
	bool regexp_ok = is_atleast(&server_version, 4, 9, 0, 0, 0, 0);
#endif

	if (is_newer_than(&server_version, 4, 4, 7, 0, 0, 0)) {
		if ((n = Packet_scanf(&rbuf, "%c%d%d%c%I", &ch, &max, &line, &attr, buf)) <= 0) return(n);
	} else {
		s16b old_max, old_line;

		if ((n = Packet_scanf(&rbuf, "%c%hd%hd%c%I", &ch, &old_max, &old_line, &attr, buf)) <= 0) return(n);
		max = old_max;
		line = old_line;
	}

	ab = attr;
	ap = TERM_WHITE;

	/* For searching, when we allow empty lines at the end of the file, we have to clear previously displayed stuff. */
	if (special_line_type && !line) clear_from(2);

	/* Hack - prepare for a special sized page (# of lines divisable by n) */
	if (line >= 21 + HGT_PLUS) {
		//21 -> % 1, 22 -> %2, 23 -> %3..
		special_page_size = 21 + HGT_PLUS - ((21 + HGT_PLUS) % (line - (20 + HGT_PLUS)));
		return(1);
	}

	/* Maximum (initialize / update) */
	if (max_line != max) {
		max_line = max;

		/* Are we still browsing or have we actually just quit? */
		if (special_line_type) {
			/* Update the prompt too (important if max_line got smaller).
			   (Prompt consistent with peruse_file() in c-files.c.)*/
			/* indicate EOF by different status line colour */
			if (cur_line + special_page_size >= max_line)
				c_prt(TERM_ORANGE, format("[Space/p/Enter/BkSpc/g/G/#%s navigate,%s ESC exit.] (%d-%d/%d)",
				    //(p_ptr->admin_dm || p_ptr->admin_wiz) ? "/s/d/D" : "",
#ifdef REGEX_SEARCH
				    regexp_ok ? "/s/d/D/r" : "/s/d/D",
#else
				    "/s/d/D",
#endif
				    my_strcasestr(special_line_title, "unique monster") ? " ! best," : "",
				    cur_line + 1, max_line , max_line), 23 + HGT_PLUS, 0);
			else
				c_prt(TERM_L_WHITE, format("[Space/p/Enter/BkSpc/g/G/#%s navigate,%s ESC exit.] (%d-%d/%d)",
				    //(p_ptr->admin_dm || p_ptr->admin_wiz) ? "/s/d/D" : "",
#ifdef REGEX_SEARCH
				    regexp_ok ? "/s/d/D/r" : "/s/d/D",
#else
				    "/s/d/D",
#endif
				    my_strcasestr(special_line_title, "unique monster") ? " ! best," : "",
				    cur_line + 1, cur_line + special_page_size, max_line), 23 + HGT_PLUS, 0);
		}
	}

#if 1 /* this on-the-fly recognition is probably only needed if the marker '21 lines' package is late to arrive? */
	/* Recognize a +1 extra line setup (usually divisable by 3 -> 21 instead of 20) - C. Blue */
	if (line == 20 + HGT_PLUS) special_page_size = 21 + HGT_PLUS;
#endif

	/* Also adjust our current starting line to the possibly updated max_line in case
	   max_line got smaller for some reason (for example when viewing equipment).
	   This is kept consistent with behaviour in peruse_file() in c-files.c and
	   do_cmd_help_aux() in files.c */
	if (cur_line > max_line - special_page_size &&
	    cur_line < max_line) {
		if (!line_searching) cur_line = max_line - special_page_size;
		if (cur_line < 0) cur_line = 0;
	}

	/* Cause inkey() to break, if inkey_max_line flag was set */
	inkey_max_line = FALSE;

	/* Print out the info */
	if (special_line_type) { /* If we have quit already, dont */
		/* remember cursor position */
		Term_locate(&x, &y);

		/* Hack: 'Title line' */
		if (line == -1) {
			/* Never scroll the title bar */
			phys_line = 0;
			strcpy(special_line_title, buf);
			c_put_str(attr, buf, phys_line, 0);
		} else { /* Normal line */
			/* Hack: Catch first content line for item inspections -> guide invocation hack */
			if (!cur_line && !line) strcpy(special_line_first, buf);

			phys_line = line +
			    (special_page_size == 21 + HGT_PLUS ? 1 : 2) + /* 1 extra usable line for 21-lines mode? */
			    (special_page_size < 20 + HGT_PLUS ? 1 : 0); /* only 40 lines on 42-lines BIG_MAP? -> slighly improved visuals ;) */

			/* Keep in sync: If peruse_file() displays a title, it must be line + 2.
			   Else line + 1 to save some space for 21-lines (odd_line) feature. */

			/* Always clear the whole line first */
			Term_erase(0, phys_line, 255);

			/* Apply horizontal scroll */
			/* For horizontal scrolling: Parse correct colour code that we might have skipped */
			if (cur_col) {
				for (p = 0; p < cur_col; p++) {
					if (buf[p] != '\377') continue;
					if (buf[p + 1] == '.') attr = ap;
					else if (isalphanum(buf[p + 1])) {
						ap = ab;
						attr = ab = color_char_to_attr(buf[p + 1]);
					}
				}
			}

			/* Finally print the actual line */
			if (strlen(buf) >= cur_col) /* catch too far horizontal scrolling */
				c_put_str(attr, buf + cur_col, phys_line, 0);
		}

		/* restore cursor position */
		Term_gotoxy(x, y);
	}

	return(1);
}
```

### Receive_special_line_pos

```c
int Receive_special_line_pos(void) {
	int n;
	char ch;

	if ((n = Packet_scanf(&rbuf, "%c%d", &ch, &cur_line)) <= 0) return(n);

	return(1);
}
```

### Receive_special_other

```c
int Receive_special_other(void) {
	int n;
	char ch;

	if ((n = Packet_scanf(&rbuf, "%c", &ch)) <= 0) return(n);

	/* Set file perusal method to "other" */
	special_line_type = SPECIAL_FILE_OTHER;

	/* Peruse the file we're about to get */
	peruse_file();

	return(1);
}
```

### Receive_store_info

```c
int Receive_store_info(void) {
	int n, max_cost;
	char ch, owner_name[MAX_CHARS], store_name[MAX_CHARS];
	s16b num_items;
	byte store_attr = TERM_SLATE;
	char store_char = '?';

	if (is_newer_than(&server_version, 4, 7, 4, 2, 0, 0)) {
		if ((n = Packet_scanf(&rbuf, "%c%hd%s%s%hd%d%c%c%c", &ch, &store_num, store_name, owner_name, &num_items, &max_cost, &store_attr, &store_char, &store_price_mul)) <= 0) return(n);
	} else if (is_newer_than(&server_version, 4, 4, 4, 0, 0, 0)) {
		if ((n = Packet_scanf(&rbuf, "%c%hd%s%s%hd%d%c%c", &ch, &store_num, store_name, owner_name, &num_items, &max_cost, &store_attr, &store_char)) <= 0) return(n);
	} else {
		if ((n = Packet_scanf(&rbuf, "%c%hd%s%s%hd%d", &ch, &store_num, store_name, owner_name, &num_items, &max_cost)) <= 0) return(n);
	}

	store.stock_num = num_items >= 0 ? num_items : 0; /* Hack: Use num_items to encode SPECIAL store flag */
	c_store.max_cost = max_cost;
	strncpy(c_store.owner_name, owner_name, 40);
	strncpy(c_store.store_name, store_name, 40);
	c_store.store_attr = store_attr;
	c_store.store_char = store_char;

	/* Only enter "display_store" if we're not already shopping */
	if (!shopping) {
		if (num_items >= 0) display_store(); /* Normal NPC or player store */
		else display_store_special(); /* Special NPC store */
	} else {
		/* Request a redraw of the store inventory */
		redraw_store = TRUE;
	}

	return(1);
}
```

### Receive_store_special_anim

```c
int Receive_store_special_anim(void) {
	int n;
	char ch;
	u16b anim1, anim2, anim3, anim4;
	int anim_step;

#ifdef ANIM_SLOT_SPINALL
	int anim_time1 = 0, anim_time2 = 0, anim_time3 = 0;
	int anim_step1, anim_step2, anim_step3, anim_steps_max;
	bool anim_changed = FALSE;
#endif

#ifdef USE_GRAPHICS
	bool use_gfx_d10f = !c_cfg.ascii_items;
#endif

	if ((n = Packet_scanf(&rbuf, "%c%hd%hd%hd%hd", &ch, &anim1, &anim2, &anim3, &anim4)) <= 0) return(n);
	if (!shopping) return(1);

	/* Casino: Wheel and Slot machine animations */
	switch (anim1) {
	case 0: //wheel
		Term_fresh(); //show initial 'I'll put you down for...' msgs
		anim_step = rand_int(ANIM_WHEEL_LENGTH) + ANIM_WHEEL_LENGTH * 2;
		while (--anim_step) { //decrement first, or final loop ends up same as final placement done afterwards, resulting in a perceived 'extra sfx' as there is no visible change
			Term_putstr(DICE_X - 13, DICE_Y + 4, -1, TERM_L_GREEN, "                              ");
			Term_putstr(DICE_X - 13 + 3 * ((anim_step + anim2) % 10), DICE_Y + 4, -1, TERM_POIS, "*");

			/* hack: hide cursor */
			Term->scr->cx = Term->wid;
			Term->scr->cu = 1;

			Term_fresh();
#ifdef USE_SOUND_2010
			sound(casino_wheel_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
#endif
#ifdef WINDOWS
			Sleep(anim_step > ANIM_WHEEL_SETTLE ? ANIM_WHEEL_SPEED : ANIM_WHEEL_SPEED + ANIM_WHEEL_SETTLE * ANIM_WHEEL_SETTLE_SLOWDOWN - anim_step * ANIM_WHEEL_SETTLE_SLOWDOWN);
#else
			usleep(1000 * (anim_step > ANIM_WHEEL_SETTLE ? ANIM_WHEEL_SPEED : ANIM_WHEEL_SPEED + ANIM_WHEEL_SETTLE * ANIM_WHEEL_SETTLE_SLOWDOWN - anim_step * ANIM_WHEEL_SETTLE_SLOWDOWN));
#endif
		}
#ifdef USE_SOUND_2010
		//sound(casino_wheel_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
#endif
		Term_putstr(DICE_X - 13, DICE_Y + 4, -1, TERM_L_GREEN, "                              ");
		Term_putstr(DICE_X - 13 + 3 * anim2, DICE_Y + 4, -1, TERM_L_GREEN, "*");
		break;

	case 1: //slot
		anim2--;
		anim3--;
		anim4--;

		/* 'init' sfx is playing, wait for a very little bit to harmonize with it ^^ */
		Term_fresh(); /* display the 'slot machine' immediately */
#ifdef WINDOWS
		Sleep(200);
#else
		usleep(200000);
#endif

#ifdef ANIM_SLOT_SPINALL
		anim_step1 = ANIM_SLOT_LENGTH * 1 + rand_int(ANIM_SLOT_LENGTH);
		anim_step2 = ANIM_SLOT_LENGTH * 3 + rand_int(ANIM_SLOT_LENGTH);
		anim_step3 = ANIM_SLOT_LENGTH * 5 + rand_int(ANIM_SLOT_LENGTH);
		anim_steps_max = ANIM_SLOT_LENGTH * 6;
		display_fruit(7, 26, (anim_step1 + anim2) % 6 + 1);
		display_fruit(7, 35, (anim_step2 + anim3) % 6 + 1);
		display_fruit(7, 44, (anim_step3 + anim4) % 6 + 1);

		while (TRUE) {
 #ifdef WINDOWS
			Sleep(1);
 #else
			usleep(1000);
 #endif
			if (anim_step1) {
				anim_time1++;
				if (anim_time1 == (anim_step1 > ANIM_SLOT_SETTLE ?
				    ANIM_SLOT_SPEED :
				    (ANIM_SLOT_SPEED + ANIM_SLOT_SETTLE * ANIM_SLOT_SETTLE_SLOWDOWN - anim_step1 * ANIM_SLOT_SETTLE_SLOWDOWN))) {
					anim_time1 = 0;
					anim_step1--;
 #ifdef USE_SOUND_2010
  #ifdef ANIM_SLOT_OPTIMIZE_SFX_DELAY
					if (anim_step1 == 1) sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #else
					if (!anim_step1) sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #endif
 #endif

					display_fruit(7, 26, (anim_steps_max - anim_step1 + anim2) % 6 + 1);
					anim_changed = TRUE;
				}
			}
			if (anim_step2) {
				anim_time2++;
				if (anim_time2 == (anim_step2 > ANIM_SLOT_SETTLE ?
				    ANIM_SLOT_SPEED :
				    (ANIM_SLOT_SPEED + ANIM_SLOT_SETTLE * ANIM_SLOT_SETTLE_SLOWDOWN - anim_step2 * ANIM_SLOT_SETTLE_SLOWDOWN))) {
					anim_time2 = 0;
					anim_step2--;
 #ifdef USE_SOUND_2010
  #ifdef ANIM_SLOT_OPTIMIZE_SFX_DELAY
					if (anim_step2 == 1) sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #else
					if (!anim_step1) sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #endif
 #endif

					display_fruit(7, 35, (anim_steps_max - anim_step2 + anim3) % 6 + 1);
					anim_changed = TRUE;
				}
			}
			if (anim_step3) {
				anim_time3++;
				if (anim_time3 == (anim_step3 > ANIM_SLOT_SETTLE ?
				    ANIM_SLOT_SPEED :
				    (ANIM_SLOT_SPEED + ANIM_SLOT_SETTLE * ANIM_SLOT_SETTLE_SLOWDOWN - anim_step3 * ANIM_SLOT_SETTLE_SLOWDOWN))) {
					anim_time3 = 0;
					anim_step3--;
 #ifdef USE_SOUND_2010
  #ifdef ANIM_SLOT_OPTIMIZE_SFX_DELAY
					if (anim_step3 == 1) sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #else
					if (!anim_step1) sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #endif
 #endif

					display_fruit(7, 44, (anim_steps_max - anim_step3 + anim4) % 6 + 1);
					anim_changed = TRUE;
				}
			}

			if (anim_changed) {
				anim_changed = FALSE;

				/* hack: hide cursor */
				Term->scr->cx = Term->wid;
				Term->scr->cu = 1;
				/* redraw */
				Term_fresh();
			}

			if (!anim_step1 && !anim_step2 && !anim_step3) break;
		}

#else

		anim_step = rand_int(ANIM_SLOT_LENGTH) + 3 * ANIM_SLOT_LENGTH;
		while (anim_step--) {
			display_fruit(7, 26, (anim_step + anim2) % 6 + 1);
			display_fruit(7, 35, (anim_step + anim3) % 6 + 1);
			display_fruit(7, 44, (anim_step + anim4) % 6 + 1);

			/* hack: hide cursor */
			Term->scr->cx = Term->wid;
			Term->scr->cu = 1;

			Term_fresh();
 #ifdef USE_SOUND_2010
  #ifdef ANIM_SLOT_OPTIMIZE_SFX_DELAY
			if (!anim_step) sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #endif
 #endif
 #ifdef WINDOWS
			Sleep(anim_step > ANIM_SLOT_SETTLE ? ANIM_SLOT_SPEED : ANIM_SLOT_SPEED + ANIM_SLOT_SETTLE * ANIM_SLOT_SETTLE_SLOWDOWN - anim_step * ANIM_SLOT_SETTLE_SLOWDOWN);
 #else
			usleep(1000 * (anim_step > ANIM_SLOT_SETTLE ? ANIM_SLOT_SPEED : ANIM_SLOT_SPEED + ANIM_SLOT_SETTLE * ANIM_SLOT_SETTLE_SLOWDOWN - anim_step * ANIM_SLOT_SETTLE_SLOWDOWN));
 #endif
		}
		display_fruit(7, 26, anim2 + 1);
 #ifdef USE_SOUND_2010
  #ifndef ANIM_SLOT_OPTIMIZE_SFX_DELAY
		sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #endif
 #endif

		anim_step = rand_int(ANIM_SLOT_LENGTH) + 2 * ANIM_SLOT_LENGTH;
		while (anim_step--) {
			display_fruit(7, 35, (anim_step + anim3) % 6 + 1);

			/* hack: hide cursor */
			Term->scr->cx = Term->wid;
			Term->scr->cu = 1;

			Term_fresh();
 #ifdef USE_SOUND_2010
  #ifdef ANIM_SLOT_OPTIMIZE_SFX_DELAY
			if (!anim_step) sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #endif
 #endif
 #ifdef WINDOWS
			Sleep(anim_step > ANIM_SLOT_SETTLE ? ANIM_SLOT_SPEED : ANIM_SLOT_SPEED + ANIM_SLOT_SETTLE * ANIM_SLOT_SETTLE_SLOWDOWN - anim_step * ANIM_SLOT_SETTLE_SLOWDOWN);
 #else
			usleep(1000 * (anim_step > ANIM_SLOT_SETTLE ? ANIM_SLOT_SPEED : ANIM_SLOT_SPEED + ANIM_SLOT_SETTLE * ANIM_SLOT_SETTLE_SLOWDOWN - anim_step * ANIM_SLOT_SETTLE_SLOWDOWN));
 #endif
		}
		display_fruit(7, 35, anim3 + 1);
 #ifdef USE_SOUND_2010
  #ifndef ANIM_SLOT_OPTIMIZE_SFX_DELAY
		sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #endif
 #endif

		anim_step = rand_int(ANIM_SLOT_LENGTH) + 1 * ANIM_SLOT_LENGTH;
		while (anim_step--) {
			display_fruit(7, 44, (anim_step + anim4) % 6 + 1);

			/* hack: hide cursor */
			Term->scr->cx = Term->wid;
			Term->scr->cu = 1;

			Term_fresh();
 #ifdef USE_SOUND_2010
  #ifdef ANIM_SLOT_OPTIMIZE_SFX_DELAY
			if (!anim_step) sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #endif
 #endif
 #ifdef WINDOWS
			Sleep(anim_step > ANIM_SLOT_SETTLE ? ANIM_SLOT_SPEED : ANIM_SLOT_SPEED + ANIM_SLOT_SETTLE * ANIM_SLOT_SETTLE_SLOWDOWN - anim_step * ANIM_SLOT_SETTLE_SLOWDOWN);
 #else
			usleep(1000 * (anim_step > ANIM_SLOT_SETTLE ? ANIM_SLOT_SPEED : ANIM_SLOT_SPEED + ANIM_SLOT_SETTLE * ANIM_SLOT_SETTLE_SLOWDOWN - anim_step * ANIM_SLOT_SETTLE_SLOWDOWN));
 #endif
		}
		display_fruit(7, 44, anim4 + 1);
 #ifdef USE_SOUND_2010
  #ifndef ANIM_SLOT_OPTIMIZE_SFX_DELAY
		sound(casino_slots_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
  #endif
 #endif
#endif
		break;

	case 2: //in-between
		Term_fresh();

#ifdef USE_SOUND_2010
		sound(casino_inbetween_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
#endif
#ifdef WINDOWS
		Sleep(600);
#else
		usleep(600000);
#endif
#ifdef GRAPHICS_BG_MASK
		if (use_gfx_d10f && use_graphics == 2) {
			Term_draw_2mask(DICE_X - 7, DICE_Y + 4, TERM_L_DARK, kidx_po_d10f_tl, 0, 0);
			Term_draw_2mask(DICE_X - 6, DICE_Y + 3, TERM_L_DARK, kidx_po_d10f_t, 0, 0);
			Term_draw_2mask(DICE_X - 5, DICE_Y + 4, TERM_L_DARK, kidx_po_d10f_tr, 0, 0);
			Term_draw_2mask(DICE_X - 7, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_bl, 0, 0);
			Term_draw_2mask(DICE_X - 6, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_b, 0, 0);
			Term_draw_2mask(DICE_X - 5, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_br, 0, 0);
			Term_putstr(DICE_X - 6, DICE_Y + 4, -1, TERM_L_DARK, format("%1d", anim2));
		} else
#endif
#ifdef USE_GRAPHICS
		if (use_gfx_d10f && use_graphics) {
			Term_draw(DICE_X - 7, DICE_Y + 4, TERM_L_DARK, kidx_po_d10f_tl);
			Term_draw(DICE_X - 6, DICE_Y + 3, TERM_L_DARK, kidx_po_d10f_t);
			Term_draw(DICE_X - 5, DICE_Y + 4, TERM_L_DARK, kidx_po_d10f_tr);
			Term_draw(DICE_X - 7, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_bl);
			Term_draw(DICE_X - 6, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_b);
			Term_draw(DICE_X - 5, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_br);
			Term_putstr(DICE_X - 6, DICE_Y + 4, -1, TERM_L_DARK, format("%1d", anim2));
		} else
#endif
		{
			Term_putstr(DICE_X - 8, DICE_Y + 2, -1, TERM_L_DARK, "  _");
			Term_putstr(DICE_X - 8, DICE_Y + 3, -1, TERM_L_DARK, " / \\");
			Term_putstr(DICE_X - 8, DICE_Y + 4, -1, TERM_L_DARK, format("/ %1d \\", anim2));
			Term_putstr(DICE_X - 8, DICE_Y + 5, -1, TERM_L_DARK, "\\___/");
		}
		/* hack: hide cursor */
		Term->scr->cx = Term->wid;
		Term->scr->cu = 1;
		Term_fresh();

#ifdef USE_SOUND_2010
		//sound(casino_inbetween_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
#endif
#ifdef WINDOWS
		Sleep(300);
#else
		usleep(300000);
#endif
#ifdef GRAPHICS_BG_MASK
		if (use_gfx_d10f && use_graphics == 2) {
			Term_draw_2mask(DICE_X + 5, DICE_Y + 4, TERM_L_DARK, kidx_po_d10f_tl, 0, 0);
			Term_draw_2mask(DICE_X + 6, DICE_Y + 3, TERM_L_DARK, kidx_po_d10f_t, 0, 0);
			Term_draw_2mask(DICE_X + 7, DICE_Y + 4, TERM_L_DARK, kidx_po_d10f_tr, 0, 0);
			Term_draw_2mask(DICE_X + 5, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_bl, 0, 0);
			Term_draw_2mask(DICE_X + 6, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_b, 0, 0);
			Term_draw_2mask(DICE_X + 7, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_br, 0, 0);
			Term_putstr(DICE_X + 6, DICE_Y + 4, -1, TERM_L_DARK, format("%1d", anim3));
		} else
#endif
#ifdef USE_GRAPHICS
		if (use_gfx_d10f && use_graphics) {
			Term_draw(DICE_X + 5, DICE_Y + 4, TERM_L_DARK, kidx_po_d10f_tl);
			Term_draw(DICE_X + 6, DICE_Y + 3, TERM_L_DARK, kidx_po_d10f_t);
			Term_draw(DICE_X + 7, DICE_Y + 4, TERM_L_DARK, kidx_po_d10f_tr);
			Term_draw(DICE_X + 5, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_bl);
			Term_draw(DICE_X + 6, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_b);
			Term_draw(DICE_X + 7, DICE_Y + 5, TERM_L_DARK, kidx_po_d10f_br);
			Term_putstr(DICE_X + 6, DICE_Y + 4, -1, TERM_L_DARK, format("%1d", anim3));
		} else
#endif
		{
			Term_putstr(DICE_X + 4, DICE_Y + 2, -1, TERM_L_DARK, "  _");
			Term_putstr(DICE_X + 4, DICE_Y + 3, -1, TERM_L_DARK, " / \\");
			Term_putstr(DICE_X + 4, DICE_Y + 4, -1, TERM_L_DARK, format("/ %1d \\", anim3));
			Term_putstr(DICE_X + 4, DICE_Y + 5, -1, TERM_L_DARK, "\\___/");
		}
		/* hack: hide cursor */
		Term->scr->cx = Term->wid;
		Term->scr->cu = 1;
		Term_fresh();

#ifdef WINDOWS
		Sleep(300);
#else
		usleep(300000);
#endif
#ifdef USE_SOUND_2010
		sound(casino_inbetween_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
#endif
#ifdef WINDOWS
		Sleep(600);
#else
		usleep(600000);
#endif
#ifdef GRAPHICS_BG_MASK
		if (use_gfx_d10f && use_graphics == 2) {
			Term_draw_2mask(DICE_X - 1, DICE_Y + 8, TERM_RED, kidx_po_d10f_tl, 0, 0);
			Term_draw_2mask(DICE_X, DICE_Y + 7, TERM_RED, kidx_po_d10f_t, 0, 0);
			Term_draw_2mask(DICE_X + 1, DICE_Y + 8, TERM_RED, kidx_po_d10f_tr, 0, 0);
			Term_draw_2mask(DICE_X - 1, DICE_Y + 9, TERM_RED, kidx_po_d10f_bl, 0, 0);
			Term_draw_2mask(DICE_X, DICE_Y + 9, TERM_RED, kidx_po_d10f_b, 0, 0);
			Term_draw_2mask(DICE_X + 1, DICE_Y + 9, TERM_RED, kidx_po_d10f_br, 0, 0);
			Term_putstr(DICE_X, DICE_Y + 8, -1, TERM_RED, format("%1d", anim4));
		} else
#endif
#ifdef USE_GRAPHICS
		if (use_gfx_d10f && use_graphics) {
			Term_draw(DICE_X - 1, DICE_Y + 8, TERM_RED, kidx_po_d10f_tl);
			Term_draw(DICE_X, DICE_Y + 7, TERM_RED, kidx_po_d10f_t);
			Term_draw(DICE_X + 1, DICE_Y + 8, TERM_RED, kidx_po_d10f_tr);
			Term_draw(DICE_X - 1, DICE_Y + 9, TERM_RED, kidx_po_d10f_bl);
			Term_draw(DICE_X, DICE_Y + 9, TERM_RED, kidx_po_d10f_b);
			Term_draw(DICE_X + 1, DICE_Y + 9, TERM_RED, kidx_po_d10f_br);
			Term_putstr(DICE_X, DICE_Y + 8, -1, TERM_RED, format("%1d", anim4));
		} else
#endif
		{
			Term_putstr(DICE_X - 2, DICE_Y + 6, -1, TERM_RED, "  _");
			Term_putstr(DICE_X - 2, DICE_Y + 7, -1, TERM_RED, " / \\");
			Term_putstr(DICE_X - 2, DICE_Y + 8, -1, TERM_RED, format("/ %1d \\", anim4));
			Term_putstr(DICE_X - 2, DICE_Y + 9, -1, TERM_RED, "\\___/");
		}
		break;

	case 3: //craps, or just any dice roll: wait for it to settle
#ifdef USE_SOUND_2010
		sound(casino_craps_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
#endif
		Term_fresh();
#ifdef WINDOWS
		Sleep(500); // xD
#else
		usleep(500000);
#endif
		break;

	case 4: //Blackjack, maybe just any card...
		/* hack: hide cursor, or after the card-stack result is printed into the shop screen,
		   a black cursor box (x11) will overwrite a part of a card */
		Term->scr->cx = Term->wid;
		Term->scr->cu = 1;
		//Term_set_cursor(0);
		//Term_fresh();

#ifdef USE_SOUND_2010
		sound(casino_card_sound_idx, SFX_TYPE_OVERLAP, 100, 0, 0, 0);
#endif
		display_card(anim2, anim3, anim4 / 14, anim4 % 14);
		break;

	default:
		c_msg_format("\377RERROR: Unknown store animation %d.", anim1);
	}

	/* hack: hide cursor */
	Term->scr->cx = Term->wid;
	Term->scr->cu = 1;

	return(1);
}
```

### Receive_store_special_char

```c
int Receive_store_special_char(void) {
	int n;
	char ch, line, col;
	byte attr;
	char c, str[2];

	if ((n = Packet_scanf(&rbuf, "%c%c%c%c%c", &ch, &line, &col, &attr, &c)) <= 0) return(n);
	if (!shopping) return(1);

	str[0] = c;
	str[1] = 0;
	c_put_str(attr, str, line, col);

	/* hack: hide cursor */
	Term->scr->cx = Term->wid;
	Term->scr->cu = 1;

	return(1);
}
```

### Receive_store_special_clr

```c
int Receive_store_special_clr(void) {
	int n;
	char ch, line_start, line_end;

	if ((n = Packet_scanf(&rbuf, "%c%c%c", &ch, &line_start, &line_end)) <= 0) return(n);
	if (!shopping) return(1);

	/* Hack: 'line_start+100' means 'force clear', for graphical special visuals */
	if (line_start >= 100) clear_force_from_to(line_start - 100, line_end);
	else
#if 0
	for (n = line_start; n <= line_end; n++)
		c_put_str(TERM_WHITE, "                                                                                ", n, 0);
#else
	clear_from_to(line_start, line_end);
#endif

	/* hack: hide cursor */
	Term->scr->cx = Term->wid;
	Term->scr->cu = 1;

	return(1);
}
```

### Receive_store_special_str

```c
int Receive_store_special_str(void) {
	int n;
	char ch, line, col;
	byte attr;
	char str[MAX_CHARS];

	if ((n = Packet_scanf(&rbuf, "%c%c%c%c%s", &ch, &line, &col, &attr, str)) <= 0) return(n);
	if (!shopping) return(1);

	/* Hack: Making this function usable for clearing graphical special images, which don't register in the char/attr matrix. */
	Term_erase(col, line, col + strlen(str));
	Term_redraw_section(col, line, col + strlen(str) - 1, line);

	c_put_str(attr, str, line, col);

	/* hack: hide cursor */
	Term->scr->cx = Term->wid;
	Term->scr->cu = 1;

	/* Hack still (redraw instantly instead of half a second later or so):
	   --moved this after hiding the cursor to try and prevent random glitch where part of the last dice is overwritten by a black cursor box (x11) */
	Term_fresh();

	return(1);
}
```

### Send_BBS

```c
int Send_BBS(void) {
	int	n;
	if ((n = Packet_printf(&wbuf, "%c", PKT_BBS)) <= 0) return(n);
	return(1);
}
```

### Send_King

```c
int Send_King(byte type) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_KING, type)) <= 0) return(n);
	return(1);
}
```

### Send_admin_house

```c
int Send_admin_house(int dir, cptr buf) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%s", PKT_HOUSE, dir, buf)) <= 0) return(n);
	return(1);
}
```

### Send_guild

```c
int Send_guild(s16b command, cptr buf) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%s", PKT_GUILD, command, buf)) <= 0) return(n);
	return(1);
}
```

### Send_guild_config

```c
int Send_guild_config(s16b command, u32b flags, cptr buf) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%d%d%s", PKT_GUILD_CFG, command, flags, buf)) <= 0) return(n);
	return(1);
}
```

### Send_master

```c
int Send_master(s16b command, cptr buf) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%s", PKT_MASTER, command, buf)) <= 0) return(n);
	return(1);
}
```

### Send_msg

```c
int Send_msg(cptr message) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%S", PKT_MESSAGE, message)) <= 0) return(n);
	return(1);
}
```

### Send_party

```c
int Send_party(s16b command, cptr buf) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%s", PKT_PARTY, command, buf)) <= 0) return(n);
	return(1);
}
```

### Send_purchase_house

```c
int Send_purchase_house(int dir) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_PURCHASE, dir, 0)) <= 0) return(n);
	return(1);
}
```

### Send_request_amt

```c
int Send_request_amt(int id, int num) {
	int n;
	if ((n = Packet_printf(&wbuf, "%c%d%d", PKT_REQUEST_AMT, id, num)) <= 0) return(n);
	return(1);
}
```

### Send_request_cfr

```c
int Send_request_cfr(int id, int cfr) {
	int n;
	if ((n = Packet_printf(&wbuf, "%c%d%d", PKT_REQUEST_CFR, id, cfr)) <= 0) return(n);
	return(1);
}
```

### Send_request_num

```c
int Send_request_num(int id, int num) {
	int n;
	if ((n = Packet_printf(&wbuf, "%c%d%d", PKT_REQUEST_NUM, id, num)) <= 0) return(n);
	return(1);
}
```

### Send_request_str

```c
int Send_request_str(int id, char *str) {
	int n;
	if ((n = Packet_printf(&wbuf, "%c%d%s", PKT_REQUEST_STR, id, str)) <= 0) return(n);
	return(1);
}
```

### Send_special_line

```c
int Send_special_line(int type, s32b line, char *srcstr) {
	int n;

	if (is_newer_than(&server_version, 4, 7, 4, 5, 0, 0)) {
		if ((n = Packet_printf(&wbuf, "%c%c%d%s", PKT_SPECIAL_LINE, type, line, srcstr ? srcstr : "")) <= 0) return(n); // <- just allow NULL pointer too, just in case..
	} else if (is_newer_than(&server_version, 4, 4, 7, 0, 0, 0)) {
		if ((n = Packet_printf(&wbuf, "%c%c%d", PKT_SPECIAL_LINE, type, line)) <= 0) return(n);
	} else {
		if ((n = Packet_printf(&wbuf, "%c%c%hd", PKT_SPECIAL_LINE, type, line)) <= 0) return(n);
	}

	return(1);
}
```

## Field and packet row reconciliation

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L32 → information.artifact-lore-* and monster-lore-*; resource-loading owner 11 must arrive with these D readers.**

| Monster/kind/artifact data and lore | `r/k/a` data resource loaders → names/lore/search/paste; multiple `my_fgets(...,1024)` paths, colon/opcode grammar | Bundled/user overrides; [c-init.c:415](../../../src/client/c-init.c#L415), [1531](../../../src/client/c-init.c#L1531), [1678](../../../src/client/c-init.c#L1678) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L33 → documents.*; setup MOTD owner 08; source-loading owner 11.**

| Server files / help / MOTD / special lines | Wire title/line bytes and local text resources → perusal; server search E=60, `%s`, regexp/version flags; local guide/note search distinct | Perusal transient; file transfer may write resource bytes; [c-files.c:1915](../../../src/client/c-files.c#L1915), [1951](../../../src/client/c-files.c#L1951), [nclient.c:5813](../../../src/client/nclient.c#L5813) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L37 → messages.recall-* / important-*; information.notes-*; persistence/export ownership 11.**

| Saved history, private notes, bookmarks, dumps/screenshots | History load `my_fgets(...,MSG_LEN)`; saved note/history byte strings, bookmark line/name, exported screen/message data | [c-init.c:3349](../../../src/client/c-init.c#L3349), [3384](../../../src/client/c-init.c#L3384), [3414](../../../src/client/c-init.c#L3414), [4492](../../../src/client/c-init.c#L4492), [c-files.c:2570](../../../src/client/c-files.c#L2570) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L53 → social.create-guild/add-guild-member/join-guild/remove-guild-member.**

| Guild create/name, add/join target, remove target | E=79, `%s`80. Receiver does not share party sanitizer; guild creation defers through request confirmation, then guild_name_legal length<20, ASCII letters/digits/` .,-'&_$%~#`, trim, censor, collision/similarity checks | Confirmation and access/mode checks server-owned; guild database persistence; [nserver.c:15109](../../../src/server/nserver.c#L15109), [party.c:1485](../../../src/server/party.c#L1485) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L54 → social.guild-toggle-adder/guild-minimum-level.**

| Guild authorized-adder name / min-level | E=NAME_LEN20 name (21 storage needed), E=4 numeric; name `%s`80 with command/flags; server uppercases first byte and loose name lookup; min-level converted to integer | Numeric editor is not string wire field; exact target/action restrictions remain server-owned; [c-cmd.c:8698](../../../src/client/c-cmd.c#L8698), [nserver.c:15143](../../../src/server/nserver.c#L15143) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L55 → housing.owner-player/owner-guild/tag/access.**

| House owner name / list tag / encoded permissions | Owner at buf+2 E60 (`O1` prefix), tag at buf+1 E19 (`T` prefix), masks/items ASCII numeric payload; all `%s`80 | Owner prompt checks cancellation; tag prompt ignores result and can send `T` after Esc. House command discriminator owns parsing/save; [c-cmd.c:9111](../../../src/client/c-cmd.c#L9111), [9193](../../../src/client/c-cmd.c#L9193), [Send_house](../../../src/client/nclient.c#L8082) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L56 → admin.level-*, admin.build-*, admin.summon-*, admin.player-*; exact operation owners retained below.**

| Master module save / load / size / entry / vault / sign / summon / player target / message | Each leaf is distinct prefix grammar. E19 at +1 module/size; E1 entry; E77 at +2 vault/sign; E76 at +3 summon; E15 at +1 target; E69 at +1 message. `%s`80 total incl prefixes | Many callers ignore cancellation and send prefix/empty suffix. Admin permissions do not change byte contract; [c-cmd.c:9489](../../../src/client/c-cmd.c#L9489), [9571](../../../src/client/c-cmd.c#L9571), [9736](../../../src/client/c-cmd.c#L9736), [10154](../../../src/client/c-cmd.c#L10154), [10304](../../../src/client/c-cmd.c#L10304) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L57 → admin.server-script/upload-script/local-script (local script has no network slot limit).**

| Remote script source / script upload filename | Script E80, storage81 → `%s`80 **mismatch at full editor length**. Upload name E30/storage81 → remote_update, file filename `%s`80; upload chunks 1024 only server>4.6.1.1.0.1, otherwise256 | Esc no upload/execute; arbitrary script language remains delegated authority, not text normalization; [c-cmd.c:10369](../../../src/client/c-cmd.c#L10369) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L58 → documents.search and existing items.details-search.**

| Server-file search | Two E60 prompts, `%s`80 on supported version branch; regex/search direction are separate flags | Cancel preserves owner's search state; server owns match/navigation. [c-files.c:2067](../../../src/client/c-files.c#L2067), [nclient.c:7977](../../../src/client/nclient.c#L7977) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L59 → request.answer-str/cancel-str and each server-flow string consumer.**

| Request string reply | Editor E159/storage160; incoming default `%s`80, outgoing `%s`80, server destination MSG_LEN256 but decoder still80 | Accepted reply or literal ESC byte+NUL; ID/type checked/cleared on server; **79 safe wire bytes vs159 editor**. See callback leaves below; [nclient.c:7214](../../../src/client/nclient.c#L7214), [9057](../../../src/client/nclient.c#L9057), [nserver.c:16212](../../../src/server/nserver.c#L16212) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L74 → server-flow.lose-memories.**

| `RID_LOSE_MEMORIES_I_SKILL` | RESET_SKILL; case-insensitive exact skill name, then confirmation and request_extra index. ESC is not generically suppressed: missing skill can yield error. [11532](../../../src/server/xtra1.c#L11532) |

**`.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md` L75 → server-flow.lose-memories.**

| `RID_LOSE_MEMORIES_II_SKILL` | Same name lookup, distinct fee/confirmation, server-owned reset. [11533](../../../src/server/xtra1.c#L11533) |

### Packet-state inventory

Original `docs/research/single-window-packet-state.md`, SHA-256 `1e3bcbb6eb15dcd412e206c721a11d85cae5fc8a0d5094430e632f84836db84d`. Every wire branch below is a required case for its mapped outcome, not a capability per field.

**L29 → information.read-sheet.**

| 26 | `PKT_CHAR_INFO` ([src/common/pack.h:48](../../src/common/pack.h#L48)) | `always`; [binding](../../src/client/nclient.c#L358) | `Receive_char_info` — character identity/mode |

**L35 → information.read-history.**

| 29 | `PKT_HISTORY` ([src/common/pack.h:51](../../src/common/pack.h#L51)) | `always`; [binding](../../src/client/nclient.c#L364) | `Receive_history` — character-history line |

**L37 → messages.read-live/recall-read/important-read and existing messages.read-occurrences.**

| 46 | `PKT_MESSAGE` ([src/common/pack.h:70](../../src/common/pack.h#L70)) | `always`; [binding](../../src/client/nclient.c#L366) | `Receive_message` — message/event stream |

**L57 → documents.open-other and existing items.details-read.**

| 50 | `PKT_SPECIAL_OTHER` ([src/common/pack.h:75](../../src/common/pack.h#L75)) | `always`; [binding](../../src/client/nclient.c#L386) | `Receive_special_other` — special-screen control |

**L64 → documents.read/navigate/search/close and items.details-*.**

| 64 | `PKT_SPECIAL_LINE` ([src/common/pack.h:91](../../src/common/pack.h#L91)) | `always`; [binding](../../src/client/nclient.c#L393) | `Receive_special_line` — special-screen formatted line |

**L67 → social.read-party.**

| 63 | `PKT_PARTY` ([src/common/pack.h:90](../../src/common/pack.h#L90)) | `always`; [binding](../../src/client/nclient.c#L396) | `Receive_party` — party identity/member/owner strings |

**L68 → social.read-guild.**

| 153 | `PKT_GUILD` ([src/common/pack.h:195](../../src/common/pack.h#L195)) | `always`; [binding](../../src/client/nclient.c#L397) | `Receive_guild` — guild identity/member/owner strings |

**L69 → social.read-guild and guild options.**

| 132 | `PKT_GUILD_CFG` ([src/common/pack.h:168](../../src/common/pack.h#L168)) | `always`; [binding](../../src/client/nclient.c#L398) | `Receive_guild_config` — guild policy/adders/hall |

**L70 → social.read-party.**

| 165 | `PKT_PARTY_STATS` ([src/common/pack.h:213](../../src/common/pack.h#L213)) | `always`; [binding](../../src/client/nclient.c#L399) | `Receive_party_stats` — party-member vitals row |

**L106 → existing request.answer-key/cancel-key; server-flow.go/craps/blackjack.**

| 184 | `PKT_REQUEST_KEY` ([src/common/pack.h:238](../../src/common/pack.h#L238)) | `always`; [binding](../../src/client/nclient.c#L439) | `Receive_request_key` — generic key request |

**L107 → request.answer-amt/cancel-amt; server-flow.donate/mail-item/wheel.**

| 185 | `PKT_REQUEST_AMT` ([src/common/pack.h:239](../../src/common/pack.h#L239)) | `always`; [binding](../../src/client/nclient.c#L440) | `Receive_request_amt` — bounded amount request |

**L108 → request.answer-num/cancel-num; server-flow.wheel.**

| 218 | `PKT_REQUEST_NUM` ([src/common/pack.h:280](../../src/common/pack.h#L280)) | `always`; [binding](../../src/client/nclient.c#L441) | `Receive_request_num` — bounded numeric request |

**L109 → request.answer-str/cancel-str; concrete server-flow consumers.**

| 186 | `PKT_REQUEST_STR` ([src/common/pack.h:240](../../src/common/pack.h#L240)) | `always`; [binding](../../src/client/nclient.c#L442) | `Receive_request_str` — string request |

**L110 → request.answer-cfr/cancel-cfr; concrete server-flow consumers.**

| 187 | `PKT_REQUEST_CFR` ([src/common/pack.h:241](../../src/common/pack.h#L241)) | `always`; [binding](../../src/client/nclient.c#L443) | `Receive_request_cfr` — confirmation request |

**L111 → request.abort-pending.**

| 188 | `PKT_REQUEST_ABORT` ([src/common/pack.h:242](../../src/common/pack.h#L242)) | `always`; [binding](../../src/client/nclient.c#L444) | `Receive_request_abort` — request cancellation |

**L112 → special-store.text.**

| 189 | `PKT_STORE_SPECIAL_STR` ([src/common/pack.h:243](../../src/common/pack.h#L243)) | `always`; [binding](../../src/client/nclient.c#L445) | `Receive_store_special_str` — positioned store text |

**L113 → special-store.glyph.**

| 190 | `PKT_STORE_SPECIAL_CHAR` ([src/common/pack.h:244](../../src/common/pack.h#L244)) | `always`; [binding](../../src/client/nclient.c#L446) | `Receive_store_special_char` — positioned store glyph |

**L114 → special-store.clear/force-clear.**

| 191 | `PKT_STORE_SPECIAL_CLR` ([src/common/pack.h:245](../../src/common/pack.h#L245)) | `always`; [binding](../../src/client/nclient.c#L447) | `Receive_store_special_clr` — store region clear |

**L115 → special-store.wheel/slots/in-between/dice/cards/unknown-animation.**

| 217 | `PKT_STORE_SPECIAL_ANIM` ([src/common/pack.h:279](../../src/common/pack.h#L279)) | `always`; [binding](../../src/client/nclient.c#L448) | `Receive_store_special_anim` — casino animation command |

**L128 → documents.navigate and items.details-navigate.**

| 206 | `PKT_SPECIAL_LINE_POS` ([src/common/pack.h:266](../../src/common/pack.h#L266)) | `always`; [binding](../../src/client/nclient.c#L466) | `Receive_special_line_pos` — special-screen cursor line |

**L192 → information.read-sheet.**

| `PKT_CHAR_INFO` → `Receive_char_info` step 1 | `%c%hd%hd%hd%hd%d%hd%s` ⇒ ch:i8/raw byte, race:i16be, class:i16be, trait:i16be, sex:i16be, mode:i32be, lives:i16be, cname:NUL string<MAX_CHARS> | >= 4.9.2.1.0.1 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2825](../../src/client/nclient.c#L2825) |

**L193 → information.read-sheet.**

| `PKT_CHAR_INFO` → `Receive_char_info` step 2 | `%c%hd%hd%hd%hd%hd%hd%s` ⇒ ch:i8/raw byte, race:i16be, class:i16be, trait:i16be, sex:i16be, mode:i16be, lives:i16be, cname:NUL string<MAX_CHARS> | >= 4.7.3.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2827](../../src/client/nclient.c#L2827) |

**L194 → information.read-sheet.**

| `PKT_CHAR_INFO` → `Receive_char_info` step 3 | `%c%hd%hd%hd%hd%hd%s` ⇒ ch:i8/raw byte, race:i16be, class:i16be, trait:i16be, sex:i16be, mode:i16be, cname:NUL string<MAX_CHARS> | > 4.5.2.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2829](../../src/client/nclient.c#L2829) |

**L195 → information.read-sheet.**

| `PKT_CHAR_INFO` → `Receive_char_info` step 4 | `%c%hd%hd%hd%hd%hd` ⇒ ch:i8/raw byte, race:i16be, class:i16be, trait:i16be, sex:i16be, mode:i16be | > 4.4.5.10.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2831](../../src/client/nclient.c#L2831) |

**L196 → information.read-sheet.**

| `PKT_CHAR_INFO` → `Receive_char_info` step 5 | `%c%hd%hd%hd%hd` ⇒ ch:i8/raw byte, race:i16be, class:i16be, sex:i16be, mode:i16be | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2833](../../src/client/nclient.c#L2833) |

**L203 → information.read-history.**

| `PKT_HISTORY` → `Receive_history` step 1 | `%c%hu%s` ⇒ ch:i8/raw byte, line:u16be, buf:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character-history line; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains `p_ptr->history[line]` string, but not original bytes beyond compatibility bound ([handler](../../src/client/nclient.c#L3144)) | none | [src/client/nclient.c:3150](../../src/client/nclient.c#L3150) |

**L213 → messages.read-live/recall-read/important-read and existing messages.read-occurrences.**

| `PKT_MESSAGE` → `Receive_message` step 1 | `%c%S` ⇒ ch:i8/raw byte, buf:NUL string<MSG_LEN> | always | invalid control byte causes silent successful drop; `FF 00` is clear-topline sentinel ([validation](../../src/client/nclient.c#L3397)) | message/event stream; latest snapshot; replace on packet, connection reset owns clear | raw buffer may be mutated for highlighting then inserted into message stores; exact received bytes and clear sentinel are not retained ([handler](../../src/client/nclient.c#L3339)) | none | [src/client/nclient.c:3346](../../src/client/nclient.c#L3346) |

**L249 → documents.open-other and existing items.details-read.**

| `PKT_SPECIAL_OTHER` → `Receive_special_other` step 1 | `%c` ⇒ ch:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | special-screen control; event/command; consume once | none; retain decoded fields ([handler](../../src/client/nclient.c#L4495)) | none | [src/client/nclient.c:4499](../../src/client/nclient.c#L4499) |

**L277 → documents.read/navigate/search/close and items.details-*.**

| `PKT_SPECIAL_LINE` → `Receive_special_line` step 1 | `%c%d%d%c%I` ⇒ ch:i8/raw byte, max:i32be, line:i32be, attr:i8/raw byte, buf:NUL string<ONAME_LEN> | > 4.4.7.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | special-screen formatted line; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | buffer may be stored/rendered; preserve exact raw line/attr/index because parsing mutates/annotates text ([handler](../../src/client/nclient.c#L5801)) | none | [src/client/nclient.c:5813](../../src/client/nclient.c#L5813) |

**L278 → documents.read/navigate/search/close and items.details-*.**

| `PKT_SPECIAL_LINE` → `Receive_special_line` step 2 | `%c%hd%hd%c%I` ⇒ ch:i8/raw byte, old_max:i16be, old_line:i16be, attr:i8/raw byte, buf:NUL string<ONAME_LEN> | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | special-screen formatted line; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | buffer may be stored/rendered; preserve exact raw line/attr/index because parsing mutates/annotates text ([handler](../../src/client/nclient.c#L5801)) | none | [src/client/nclient.c:5817](../../src/client/nclient.c#L5817) |

**L281 → social.read-party.**

| `PKT_PARTY` → `Receive_party` step 1 | `%c%s%s%s` ⇒ ch:i8/raw byte, pname:NUL string<MAX_CHARS>, pmembers:NUL string<MAX_CHARS>, powner:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | party identity/member/owner strings; latest snapshot; replace on packet, connection reset owns clear | retains party identity/member/owner strings ([handler](../../src/client/nclient.c#L5985)) | none | [src/client/nclient.c:5989](../../src/client/nclient.c#L5989) |

**L282 → social.read-guild.**

| `PKT_GUILD` → `Receive_guild` step 1 | `%c%s%s%s` ⇒ ch:i8/raw byte, gname:NUL string<MAX_CHARS>, gmembers:NUL string<MAX_CHARS>, gowner:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | guild identity/member/owner strings; latest snapshot; replace on packet, connection reset owns clear | retains guild identity/member/owner strings ([handler](../../src/client/nclient.c#L6036)) | none | [src/client/nclient.c:6040](../../src/client/nclient.c#L6040) |

**L283 → social.read-guild and guild options.**

| `PKT_GUILD_CFG` → `Receive_guild_config` step 1 | `%c%d%d%d%d%d%d%d` ⇒ ch:i8/raw byte, master:i32be, guild.flags:i32be, minlev_32b:i32be, guild_adders:i32be, guildhall_wx:i32be, guildhall_wy:i32be, ghp:i32be | always | incomplete variable adders reset `rbuf.ptr` to packet start and return 0 ([validation](../../src/client/nclient.c#L6144)) | guild policy/adders/hall; latest snapshot; replace on packet, connection reset owns clear | retains normalized guild fields/adders; dynamic record must commit atomically ([handler](../../src/client/nclient.c#L6064)) | none | [src/client/nclient.c:6072](../../src/client/nclient.c#L6072) |

**L284 → social.read-guild and guild options.**

| `PKT_GUILD_CFG` → `Receive_guild_config` step 2 | `%s` ⇒ dummy:NUL string<MAX_CHARS> | guild adder record when slot is not authorized; empty string placeholder | incomplete variable adders reset `rbuf.ptr` to packet start and return 0 ([validation](../../src/client/nclient.c#L6144)) | guild policy/adders/hall; latest snapshot; replace on packet, connection reset owns clear | retains normalized guild fields/adders; dynamic record must commit atomically ([handler](../../src/client/nclient.c#L6064)) | none | [src/client/nclient.c:6092](../../src/client/nclient.c#L6092) |

**L285 → social.read-guild and guild options.**

| `PKT_GUILD_CFG` → `Receive_guild_config` step 3 | `%s` ⇒ guild.adder[i]:NUL string<MAX_CHARS> | guild adder record when authorized | incomplete variable adders reset `rbuf.ptr` to packet start and return 0 ([validation](../../src/client/nclient.c#L6144)) | guild policy/adders/hall; latest snapshot; replace on packet, connection reset owns clear | retains normalized guild fields/adders; dynamic record must commit atomically ([handler](../../src/client/nclient.c#L6064)) | none | [src/client/nclient.c:6096](../../src/client/nclient.c#L6096) |

**L286 → social.read-party.**

| `PKT_PARTY_STATS` → `Receive_party_stats` step 1 | `%c%d%d%s%d%d%d%d%d` ⇒ ch:i8/raw byte, j:i32be, color:i32be, partymembername:NUL string<MAX_CHARS>, k:i32be, chp:i32be, mhp:i32be, cmp:i32be, mmp:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | party-member vitals row; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | all row fields are passed to direct rendering and lost ([handler](../../src/client/nclient.c#L5972)) | none | [src/client/nclient.c:5976](../../src/client/nclient.c#L5976) |

**L349 → existing request.answer-key/cancel-key; server-flow.go/craps/blackjack.**

| `PKT_REQUEST_KEY` → `Receive_request_key` step 1 | `%c%d%s` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | generic key request; modal request; replace pending, response/abort clears | request/prompt exist only during synchronous input; lost after response ([handler](../../src/client/nclient.c#L7177)) | `Send_request_key` | [src/client/nclient.c:7181](../../src/client/nclient.c#L7181) |

**L350 → request.answer-amt/cancel-amt; server-flow.donate/mail-item/wheel.**

| `PKT_REQUEST_AMT` → `Receive_request_amt` step 1 | `%c%d%s%d` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS>, max:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | bounded amount request; modal request; replace pending, response/abort clears | request/prompt/max lost after synchronous response ([handler](../../src/client/nclient.c#L7190)) | `Send_request_amt` | [src/client/nclient.c:7194](../../src/client/nclient.c#L7194) |

**L351 → request.answer-num/cancel-num; server-flow.wheel.**

| `PKT_REQUEST_NUM` → `Receive_request_num` step 1 | `%c%d%s%d%d%d` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS>, predef:i32be, min:i32be, max:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | bounded numeric request; modal request; replace pending, response/abort clears | request/prompt/default/min/max lost after synchronous response ([handler](../../src/client/nclient.c#L7202)) | `Send_request_num` | [src/client/nclient.c:7206](../../src/client/nclient.c#L7206) |

**L352 → request.answer-str/cancel-str; concrete server-flow consumers.**

| `PKT_REQUEST_STR` → `Receive_request_str` step 1 | `%c%d%s%s` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS>, buf:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | string request; modal request; replace pending, response/abort clears | request/prompt/default lost after synchronous response ([handler](../../src/client/nclient.c#L7214)) | `Send_request_str` | [src/client/nclient.c:7218](../../src/client/nclient.c#L7218) |

**L353 → request.answer-cfr/cancel-cfr; concrete server-flow consumers.**

| `PKT_REQUEST_CFR` → `Receive_request_cfr` step 1 | `%c%d%s%c` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS>, dy:i8/raw byte | > 4.5.6.0.0.1 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | confirmation request; modal request; replace pending, response/abort clears | request/prompt/default lost after synchronous response ([handler](../../src/client/nclient.c#L7227)) | `Send_request_cfr` | [src/client/nclient.c:7235](../../src/client/nclient.c#L7235) |

**L354 → request.answer-cfr/cancel-cfr; concrete server-flow consumers.**

| `PKT_REQUEST_CFR` → `Receive_request_cfr` step 2 | `%c%d%s` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS> | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | confirmation request; modal request; replace pending, response/abort clears | request/prompt/default lost after synchronous response ([handler](../../src/client/nclient.c#L7227)) | `Send_request_cfr` | [src/client/nclient.c:7238](../../src/client/nclient.c#L7238) |

**L355 → request.abort-pending.**

| `PKT_REQUEST_ABORT` → `Receive_request_abort` step 1 | `%c` ⇒ ch:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | request cancellation; event/command; consume once | sets transient abort flag only when a request is pending ([handler](../../src/client/nclient.c#L7247)) | none | [src/client/nclient.c:7251](../../src/client/nclient.c#L7251) |

**L356 → special-store.text.**

| `PKT_STORE_SPECIAL_STR` → `Receive_store_special_str` step 1 | `%c%c%c%c%s` ⇒ ch:i8/raw byte, line:i8/raw byte, col:i8/raw byte, attr:i8/raw byte, str:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | positioned store text; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | positioned text is drawn only; line/col/attr/raw string lost ([handler](../../src/client/nclient.c#L4663)) | none | [src/client/nclient.c:4669](../../src/client/nclient.c#L4669) |

**L357 → special-store.glyph.**

| `PKT_STORE_SPECIAL_CHAR` → `Receive_store_special_char` step 1 | `%c%c%c%c%c` ⇒ ch:i8/raw byte, line:i8/raw byte, col:i8/raw byte, attr:i8/raw byte, c:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | positioned store glyph; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | positioned glyph is drawn only; all fields lost ([handler](../../src/client/nclient.c#L4691)) | none | [src/client/nclient.c:4697](../../src/client/nclient.c#L4697) |

**L358 → special-store.clear/force-clear.**

| `PKT_STORE_SPECIAL_CLR` → `Receive_store_special_clr` step 1 | `%c%c%c` ⇒ ch:i8/raw byte, line_start:i8/raw byte, line_end:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store region clear; latest snapshot; replace on packet, connection reset owns clear | clear range is executed only; range lost ([handler](../../src/client/nclient.c#L4712)) | none | [src/client/nclient.c:4716](../../src/client/nclient.c#L4716) |

**L359 → special-store.wheel/slots/in-between/dice/cards/unknown-animation.**

| `PKT_STORE_SPECIAL_ANIM` → `Receive_store_special_anim` step 1 | `%c%hd%hd%hd%hd` ⇒ ch:i8/raw byte, anim1:i16be, anim2:i16be, anim3:i16be, anim4:i16be | always | unknown animation selector falls through legacy switch behavior; operands still consumed ([validation](../../src/client/nclient.c#L5061)) | casino animation command; event/command; consume once | animation operands are consumed by procedural drawing/sound and lost ([handler](../../src/client/nclient.c#L5045)) | none | [src/client/nclient.c:5061](../../src/client/nclient.c#L5061) |

**L379 → documents.navigate and items.details-navigate.**

| `PKT_SPECIAL_LINE_POS` → `Receive_special_line_pos` step 1 | `%c%d` ⇒ ch:i8/raw byte, cur_line:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | special-screen cursor line; latest snapshot; replace on packet, connection reset owns clear | retains `cur_line` cursor ([handler](../../src/client/nclient.c#L5934)) | none | [src/client/nclient.c:5938](../../src/client/nclient.c#L5938) |

**L410 → request.abort-pending.**

| generic requests | `PKT_REQUEST_ABORT` sets cancellation only while synchronous request is pending; each request otherwise exists until its response is sent. | modal request owner; [src/client/nclient.c:7183](../../src/client/nclient.c#L7183), [src/client/nclient.c:7247](../../src/client/nclient.c#L7247) |

**L452 → request.abort-pending.**

| Modal request | item/spell/direction/amount/number/string/confirmation prompts | dialog layout, focus ring, key hint | one active request identity; response or `PKT_REQUEST_ABORT` clears |


### Explicit non-family source gates and bounds

- Social party input 79 → `%s`80 → server clip40/control-byte replacement; legal party names have separate <20 alphabet rules. Guild receiver does not reuse the party sanitizer. Guild rename clips40 before its <20 legal check. Test signed-char/control/high-byte inputs under the approved field policy.
- STR editor E159 vs wire `%s`80 (79 payload bytes) and remote Lua E80 vs `%s`80 are different boundaries. Approved session-policy safe field behavior applies; preserve legal baseline bytes, never require unsafe framing to reproduce a legacy overflow. Exercise source, editor, transformed field and wire limits separately.
- House owner uses E60 after O1, tag E19 after T. Tag cancellation still transmits T. Access flags are a raw byte (not decimal text), followed by decimal minimum level only when nonzero; zero flags terminate the C string at M.
- Request contact-owner retains PLAYER_STORES, censor/quota/account handling and no generic Escape suppression; lose-memory skill callbacks retain RESET_SKILL and can report invalid skill on Escape. The quest string callback clips30 and passes Escape through to quest logic. No generic cancel policy accepts these callers.
- `DM_MODULES` controls level save/load/blank/entry. Admin build TEST_CLIENT gates menu labels for A–D, not their compiled switch cases; raw info payloads contain C-string termination behavior. Modern dungeon flags/theme shape requires >=4.9.0.5.0.0; theme prompt also has the >4.5.6.0.0.1 branch. Upload chunk size is 1024 only >4.6.1.1.0.1, otherwise256.

### Source-level menu reconciliation

The following owner excerpts resolve misleading historic nested-key summaries. Parameter alternatives (monster kind, placement, custom feature, dungeon flags, category filter) are required scenarios of their complete outcome, not an acceptance shortcut through a generic menu. In particular a single DM test cannot accept all leaves.

Source `src/client/c-cmd.c`, SHA-256 `fe190ffdb11ae636ec7cf96964a0e4e3cc895fd74e9bbdcd23dac5b51a9e9a8a`.

#### cmd_house_chown

```c
static void cmd_house_chown(int dir) {
	char i = 0;
	char buf[80];
	bool inkey_msg_old = inkey_msg;

	Term_clear();
	Term_putstr(0, 2, -1, TERM_BLUE, "Select owner type");
	Term_putstr(5, 4, -1, TERM_WHITE, "(1) Player");
	Term_putstr(5, 5, -1, TERM_WHITE, "(2) Guild");

	/* suppress hybrid macros */
	inkey_msg = TRUE;

	while (i != ESCAPE) {
		i = inkey();
		switch (i) {
		case '1':
			buf[0] = 'O';
			buf[1] = i;
			buf[2] = 0;
			if (get_string("Enter new name: ", &buf[2], 60))
				Send_admin_house(dir, buf);
			/* restore responsiveness to hybrid macros */
			inkey_msg = inkey_msg_old;
			return;
		case '2':
			buf[0] = 'O';
			buf[1] = '2';
			buf[2] = 0;
			Send_admin_house(dir, buf);
			/* restore responsiveness to hybrid macros */
			inkey_msg = inkey_msg_old;
			return;
		case ESCAPE:
		case KTRL('Q'):
			break;
		case KTRL('T'):
			xhtml_screenshot("screenshot????", 2);
			break;
		case ':':
			cmd_message();
			inkey_msg = TRUE; /* And suppress macros again.. */
			break;
		default:
			bell();
		}
		clear_topline_forced();
	}

	/* restore responsiveness to hybrid macros */
	inkey_msg = inkey_msg_old;
}
```

#### cmd_house_chmod

```c
static void cmd_house_chmod(int dir) {
	char buf[80];
	char mod = ACF_NONE;
	u16b minlev = 0;

	Term_clear();
	Term_putstr(0, 2, -1, TERM_BLUE, "Set new permissions");
	if (get_check2("Allow party access?", FALSE)) mod |= ACF_PARTY;
	if (get_check2("Restrict access to class?", FALSE)) mod |= ACF_CLASS;
	if (get_check2("Restrict access to race?", FALSE)) mod |= ACF_RACE;
	if (get_check2("Restrict access to winners?", FALSE)) mod |= ACF_WINNER;
	if (get_check2("Restrict access to fallen winners?", FALSE)) mod |= ACF_FALLENWINNER;
	if (get_check2("Restrict access to no-ghost players?", FALSE)) mod |= ACF_NOGHOST;
	minlev = c_get_quantity("Minimum level: ", 127, -1);
	if (minlev > 1) mod |= ACF_LEVEL;
	buf[0] = 'M';
	if ((buf[1] = mod)) sprintf(&buf[2], "%hd", minlev);
	Send_admin_house(dir, buf);
}
```

#### cmd_house_tag

```c
static void cmd_house_tag(int dir) {
	char buf[80];

	get_string("Enter a tag for the houses list (max 19 characters): ", &buf[1], 20 - 1);
	buf[0] = 'T';
	Send_admin_house(dir, buf);
}
```

#### cmd_purchase_house

```c
void cmd_purchase_house(void) {
	char i = 0;
	int dir;
	bool inkey_msg_old = inkey_msg;

	if (!get_dir(&dir)) return;

	/* suppress hybrid macros */
	inkey_msg = TRUE;

	Term_save();
	Term_clear();
	Term_putstr(0, 2, -1, TERM_BLUE, "House commands");
	Term_putstr(5, 4, -1, TERM_WHITE, "(1) Buy/Sell house");
	Term_putstr(5, 5, -1, TERM_WHITE, "(2) Change house owner");
	Term_putstr(5, 6, -1, TERM_WHITE, "(3) Change house permissions");
	Term_putstr(5, 7, -1, TERM_WHITE, "(4) Paint house");/* new in 4.4.6 */
	Term_putstr(5, 8, -1, TERM_WHITE, "(5) Add houses-list tag");/* new in 4.7.3.1 */
	Term_putstr(5, 10, -1, TERM_WHITE, "(s) Enter player store");/* new in 4.4.6 */
	Term_putstr(5, 11, -1, TERM_WHITE, "(k) Knock on house door");
	/* display in dark colour since only admins can do this really */
	if (p_ptr->admin_dm || p_ptr->admin_wiz)
		Term_putstr(5, 20, -1, TERM_L_DARK, "(D) Delete house (server administrators only)");

	while (i != ESCAPE) {
		i = inkey();
		switch (i) {
		case '1':
			/* Confirm */
			if (get_check2("Are you sure you really want to buy or sell the house?", FALSE)) {
				/* Send it */
				Send_purchase_house(dir);
				i = ESCAPE;
			}
			break;
		case '2':
			cmd_house_chown(dir);
			i = ESCAPE;
			break;
		case '3':
			cmd_house_chmod(dir);
			i = ESCAPE;
			break;
		case '4':
			cmd_house_paint(dir);
			i = ESCAPE;
			break;
		case '5':
			cmd_house_tag(dir);
			i = ESCAPE;
			break;
		case 'D':
			if (!p_ptr->admin_dm && !p_ptr->admin_wiz) bell();
			else {
				cmd_house_kill(dir);
				i = ESCAPE;
			}
			break;
		case 's':
			cmd_house_store(dir);
			/* Note that Term_load() at the end of this function, followed shortly after by
			   Term_save() in display_store(), leads to that quick visual flickering -_- no
			   good way to skip those two though, since we don't know whether house_admin()
			   will fail on server-side..  - C. Blue */
			i = ESCAPE;
			break;
		case 'k':
			cmd_house_knock(dir);
			i = ESCAPE;
			break;
		case ESCAPE:
		case KTRL('Q'):
			break;
		case KTRL('T'):
			xhtml_screenshot("screenshot????", 2);
			break;
		case ':':
			cmd_message();
			inkey_msg = TRUE; /* And suppress macros again.. */
			break;
		default:
			bell();
		}
		clear_topline_forced();
	}
	Term_load();

	/* restore responsiveness to hybrid macros */
	inkey_msg = inkey_msg_old;
}
```

#### cmd_master_aux_player

```c
static bool cmd_master_aux_player(void) {
	char i = 0;
	static char buf[80];
	bool success = FALSE;
	bool inkey_msg_old = inkey_msg;

	inkey_msg = TRUE;

	Term_clear();
	Term_putstr(0, 2, -1, TERM_BLUE, "Player commands");
	Term_putstr(5, 4, -1, TERM_WHITE, "(1) Editor (offline)");
	Term_putstr(5, 5, -1, TERM_WHITE, "(2) Acquirement");
	Term_putstr(5, 6, -1, TERM_WHITE, "(3) Invoke wrath");
	Term_putstr(5, 7, -1, TERM_WHITE, "(4) Static player");
	Term_putstr(5, 8, -1, TERM_WHITE, "(5) Unstatic player");
	Term_putstr(5, 9, -1, TERM_WHITE, "(6) Delete player");
	Term_putstr(5, 10, -1, TERM_WHITE, "(7) Telekinesis");
	Term_putstr(5, 11, -1, TERM_WHITE, "(8) Broadcast");

	Term_putstr(0, 13, -1, TERM_WHITE, "Command: ");

	while (i != ESCAPE) {
		/* Get a key */
		i = inkey();

		buf[0] = '\0';

		switch (i) {
		case ':':
			cmd_message();
			inkey_msg = TRUE; /* And suppress macros again.. */
			break;
		case KTRL('T'):
			xhtml_screenshot("screenshot????", 2);
			break;
		case '1':
			buf[0] = 'E';
			get_string("Enter player name: ", &buf[1], 15);
			break;
		case '2':
			buf[0] = 'A';
			get_string("Enter player name: ", &buf[1], 15);
			break;
		case '3':
			buf[0] = 'k';
			get_string("Enter player name (prefix with '!' for no-ghost kill): ", &buf[1], 15);
			break;
		case '4':
			buf[0] = 'S';
			get_string("Enter player name: ", &buf[1], 15);
			break;
		case '5':
			buf[0] = 'U';
			get_string("Enter player name: ", &buf[1], 15);
			break;
		case '6':
			buf[0] = 'r';
			get_string("Enter player name: ", &buf[1], 15);
			break;
		case '7':
			/* DM to player telekinesis */
			buf[0] = 't';
			get_string("Enter player name: ", &buf[1], 15);
			break;
		case '8': {
				int j;

				buf[0] = 'B';
				get_string("Message: ", &buf[1], 69);
				for (j = 0; j < 60; j++)
					if (buf[j] == '{') buf[j] = '\377';
			}
			break;
		case ESCAPE:
			break;
		default:
			bell();
		}
		if (buf[0]) {
			Send_master(MASTER_PLAYER, buf);
			/* Hack: After we issued a command, completely exit this menu,
			   as it is unlikely we want to perform another action.
			   We rather want to instantly look at the results... =p */
			success = TRUE;
			/* Leave on success */
			i = ESCAPE;
		}

		/* Flush messages */
		clear_topline_forced();
	}

	/* restore responsiveness to hybrid macros */
	inkey_msg = inkey_msg_old;

	return(success);
}
```

#### cmd_master_aux_level

```c
static void cmd_master_aux_level(void) {
	char i, l;
	char buf[80];
	bool inkey_msg_old = inkey_msg;

	/* suppress hybrid macros */
	inkey_msg = TRUE;

	/* Process requests until done */
	while (1) {
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = '\0';

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Level commands");

		/* Selections */
		l = 4;
		Term_putstr(5, l++, -1, TERM_WHITE, "(1) Static your current level");
		Term_putstr(5, l++, -1, TERM_WHITE, "(2) Unstatic your current level");
		Term_putstr(5, l++, -1, TERM_WHITE, "(3) Add dungeon");
		Term_putstr(5, l++, -1, TERM_WHITE, "(4) Remove dungeon");
		Term_putstr(5, l++, -1, TERM_WHITE, "(5) Town generation");
		Term_putstr(5, l++, -1, TERM_WHITE, "(6) Perma-static your current level");
		Term_putstr(5, l++, -1, TERM_WHITE, "(7) Un-perma-static your current level");
#ifdef DM_MODULES
		Term_putstr(5, l++, -1, TERM_WHITE, "(a) Save level to module file");
		Term_putstr(5, l++, -1, TERM_WHITE, "(b) Load level from module file");
		Term_putstr(5, l++, -1, TERM_WHITE, "(c) Generate a blank level");
		Term_putstr(5, l++, -1, TERM_WHITE, "(d) Set entry position");

		/* Prompt */
		Term_putstr(0, l + 2, -1, TERM_WHITE, "Command: ");
#else
		Term_putstr(0, l + 2, -1, TERM_WHITE, "Command: ");
#endif

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Take a screenshot */
		else if (i == KTRL('T')) xhtml_screenshot("screenshot????", 2);
		else if (i == ':') {
			cmd_message();
			inkey_msg = TRUE; /* And suppress macros again.. */
		}
		/* static the current level */
		else if (i == '1') Send_master(MASTER_LEVEL, "s");
		/* unstatic the current level */
		else if (i == '2') Send_master(MASTER_LEVEL, "u");
		else if (i == '3') {	/* create dungeon stair here */
			buf[0] = 'D';
			buf[4] = 0x01;//hack: avoid 0 byte
			buf[5] = 0x01;//hack: avoid 0 byte
			buf[6] = 0x01;//hack: avoid 0 byte
			if (is_newer_than(&server_version, 4, 5, 6, 0, 0, 1)) {
				char ts[4];
				int t; //hooray for signed char..

				strcpy(ts, "0");
				if (!get_string("Theme (ESC/0 = default vanilla, +100 to set type instead): ", ts, 3)) t = 0;
				else t = atoi(ts);

				if (t >= 100) {
					buf[7] = 100 - t;
					/* Predefined dungeon -> nothing left to do then. */
					buf[1] = 1;
					buf[2] = 127;
					buf[3] = 'd';
					buf[8] = '\0';
					Send_master(MASTER_LEVEL, buf);
					clear_topline_forced();
					/* restore responsiveness to hybrid macros */
					inkey_msg = inkey_msg_old;
					return;
				} else {
					if (is_atleast(&server_version, 4, 9, 0, 5, 0, 0)) buf[7] = t + 1; //hack: avoid 0 byte
					else buf[7] = t; //max len in this version is 8 anyway, so no info is lost even if this is 0.
				}
			} else buf[7] = 0;
			buf[1] = c_get_quantity("Base level: ", 127, -1);
			if (!buf[1]) buf[1] = 1; //pressed ESC? Apply a default value (or dungeon creation will fail)
			buf[2] = c_get_quantity("Max depth (1-127): ", 127, -1);
			if (!buf[2]) buf[2] = 3; //pressed ESC? Apply a default value (or dungeon creation will fail)
			buf[3] = (get_check2("Is it a tower?", FALSE) ? 't' : 'd');
			/*
			 * FIXME: flags are u32b while buf[] is char!
			 * This *REALLY* should be rewritten	- Jir -
			 */
			//--removed as currently ALL dungeons are RANDOM (or panic save occurs due to undefined dungeon size!):
			// if (get_check2("Random dungeon (default)?", TRUE)) buf[5] |= 0x02;//DF2_RANDOM
			if (get_check2("Hellish?", FALSE)) buf[5] |= 0x04;//DF2_HELL
			if (get_check2("Not mappable?", FALSE)) {
				buf[4] |= 0x02;//DF1_FORGET
				buf[5] |= 0x08;//DF2_NO_MAGIC_MAP
			}
			if (get_check2("Ironman?", TRUE)) {
				buf[5] |= 0x10;//DF2_IRON
				i = 0;
				if (get_check2("Recallable from, before reaching its end?", FALSE)) {
					if (get_check2("Random recall depth intervals (y) or fixed ones (n) ?", TRUE)) buf[6] |= 0x08;
					i = c_get_quantity("Frequency (random)? (1=often..4=rare): ", 4, 4);
					switch (i) {
					case 1: buf[6] |= 0x10; break;//DF2_IRONRNDn / DF2_IRONFIXn
					case 2: buf[6] |= 0x20; break;
					case 3: buf[6] |= 0x40; break;
					default: buf[6] |= 0x80;
					}
					i = 1; // hack for towns below
				}
				if (get_check2("Generate towns inbetween?", FALSE)) {
					if (i == 1 && get_check2("Generate towns when premature recall is allowed?", FALSE)) {
						buf[5] |= 0x20;//DF2_TOWNS_IRONRECALL
					} else if (get_check2("Generate towns randomly (y) or in fixed intervals (n) ?", TRUE)) {
						buf[5] |= 0x40;//DF2_TOWNS_RND
					} else buf[5] |= 0x80;//DF2_TOWNS_FIX
				}
			}
			if (get_check2("Disallow generation of simple stores (misc iron + low level)?", FALSE)) {
				buf[4] |= 0x08;//DF3_NO_SIMPLE_STORES
				if (get_check2("Generate at least the hidden library?", FALSE)) buf[4] |= 0x04;//DF3_HIDDENLIB
			} else if (get_check2("Generate misc iron stores (RPG rules style)?", FALSE)) {
				buf[6] |= 0x04;//DF2_MISC_STORES
			} else if (get_check2("Generate at least the hidden library?", FALSE)) buf[4] |= 0x04;//DF3_HIDDENLIB
			/* Allow any custom flags */
			if (is_atleast(&server_version, 4, 9, 0, 5, 0, 0)) {
				char fshex[9], fshextmp[9];

				fshextmp[0] = 0;
				(void)get_string("Custom DF1 flags (string of 8 hex chars, logical OR): ", fshextmp, 8);
				memset(fshex, '0', 8);
				strcpy(fshex + 8 - strlen(fshextmp), fshextmp);
				strncpy(buf + 8, fshex, 8);

				fshextmp[0] = 0;
				(void)get_string("Custom DF2 flags (string of 8 hex chars, logical OR): ", fshextmp, 8);
				memset(fshex, '0', 8);
				strcpy(fshex + 8 - strlen(fshextmp), fshextmp);
				strncpy(buf + 16, fshex, 8);

				fshextmp[0] = 0;
				(void)get_string("Custom DF3 flags (string of 8 hex chars, logical OR): ", fshextmp, 8);
				memset(fshex, '0', 8);
				strcpy(fshex + 8 - strlen(fshextmp), fshextmp);
				strncpy(buf + 24, fshex, 8);

				buf[32] = '\0'; /* Terminate */
			} else buf[8] = '\0'; /* Terminate */
			Send_master(MASTER_LEVEL, buf);
		}
		else if (i == '4') {
			buf[0] = 'R';
			buf[1] = '\0';
			Send_master(MASTER_LEVEL, buf);
		}
		else if (i == '5') {
			buf[0] = 'T';
			buf[1] = c_get_quantity("Base level: ", 127, -1);
			Send_master(MASTER_LEVEL, buf);
		}
		/* perma-static the current level */
		else if (i == '6') Send_master(MASTER_LEVEL, "p");
		/* perma-unstatic the current level */
		else if (i == '7') Send_master(MASTER_LEVEL, "P");

#ifdef DM_MODULES
		/* Kurzel - save/load a module file (or create a blank to begin with) */
		else if (i == 'a') {
			buf[0] = 'S';
			get_string("Save module name (max 19 char): ", &buf[1], 19);
			Send_master(MASTER_LEVEL, buf);
		}
		else if (i == 'b') {
			buf[0] = 'L';
			get_string("Load module name (max 19 char): ", &buf[1], 19);
			Send_master(MASTER_LEVEL, buf);
		}
		else if (i == 'c') {
			buf[0] = 'B';
			get_string("WxH string (eg. 1x1-5x5): ", &buf[1], 19);
			Send_master(MASTER_LEVEL, buf);
		}
		else if (i == 'd') {
			get_string("Set level entry (> < or +): ", &buf[0], 1);
			Send_master(MASTER_LEVEL, buf);
		}
#endif

		/* Oops */
		else {
			/* Ring bell */
			bell();
		}

		/* Flush messages */
		clear_topline_forced();
	}

	/* restore responsiveness to hybrid macros */
	inkey_msg = inkey_msg_old;
}
```

#### cmd_master_aux_build

```c
static void cmd_master_aux_build(void) {
	char i;
	int n;
	char buf[80];
	bool inkey_msg_old = inkey_msg;

	inkey_msg = TRUE;

	/* Process requests until done */
	while (1) {
		/* Clear screen */
		Term_clear();

		/* Initialize buffer */
		buf[0] = FEAT_FLOOR;

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Building commands");

		/* Selections */
#ifdef TEST_CLIENT
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Granite Mode          (A) Set this cave info");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Permanent Mode        (B) Set this cave info2");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) Tree Mode             (C) Cave info mode");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Evil Tree Mode        (D) Cave info2 mode");
#else
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Granite Mode");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Permanent Mode");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) Tree Mode");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Evil Tree Mode");
#endif
		Term_putstr(5, 8, -1, TERM_WHITE, "(5) Grass Mode");
		Term_putstr(5, 9, -1, TERM_WHITE, "(6) Dirt Mode");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Floor Mode");
		Term_putstr(5, 11, -1, TERM_WHITE, "(8) Special Door Mode");
		Term_putstr(5, 12, -1, TERM_WHITE, "(9) Signpost");
		Term_putstr(5, 13, -1, TERM_WHITE, "(0) Any feature");

#ifdef TEST_CLIENT
		Term_putstr(5, 15, -1, TERM_WHITE, "(a) Build/Set-info Mode Off");
#else
		Term_putstr(5, 15, -1, TERM_WHITE, "(a) Build Mode Off");
#endif

		/* Prompt */
		Term_putstr(0, 18, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		buf[1] = 'T';
		buf[2] = '\0';

		switch (i) {
		/* Take a screenshot */
		case ':':
			cmd_message();
			inkey_msg = TRUE; /* And suppress macros again.. */
			break;
		case KTRL('T'):
			xhtml_screenshot("screenshot????", 2);
			break;
		/* Granite mode on */
		case '1': buf[0] = FEAT_WALL_EXTRA; break;
		/* Perm mode on */
		case '2': buf[0] = FEAT_PERM_EXTRA; break;
		/* Tree mode on */
		case '3': buf[0] = FEAT_TREE; break;
		/* Evil tree mode on */
		case '4': buf[0] = FEAT_DEAD_TREE; break;
		/* Grass mode on */
		case '5': buf[0] = FEAT_GRASS; break;
		/* Dirt mode on */
		case '6': buf[0] = FEAT_DIRT; break;
		/* Floor mode on */
		case '7': buf[0] = FEAT_FLOOR; break;
		/* House door mode on */
		case '8':
			buf[0] = FEAT_HOME_HEAD; //note: FEAT_HOME == FEAT_HOME_HEAD
			{
				u16b keyid;

				keyid = c_get_quantity("Enter key pval: ", 0xffff, -1);
				sprintf(&buf[2], "%d", keyid);
			}
			break;
		/* Sign post */
		case '9':
			buf[0] = FEAT_SIGN;
			get_string("Sign: ", &buf[2], 77);
			break;
		/* Ask for feature */
		case '0':
			n = c_get_quantity("Enter feature value: ", 0, -1);
			if (n >= 256) {
				buf[0] = 1; //dummy
				buf[1] = 'X'; //extended codes^^ aka 2 byte instead of 1, since feat is now u16b
				buf[2] = (n & 0xff00) >> 8; //will never be zero, so it won't wrongly terminate the buf string early on transmission
				buf[3] = (n & 0xff);
				buf[4] = 0;
			} else buf[0] = n;
			break;

		/* Build mode off */
		case 'a': buf[0] = FEAT_FLOOR; buf[1] = 'F'; break;
		case 'A':
			n = c_get_quantity("Enter info value: ", 0, -1);
			buf[0] = 1; //dummy
			buf[1] = 'i';
			buf[2] = (n & 0xff);
			buf[3] = (n & 0xff00) >> 8;
			buf[4] = (n & 0xff0000) >> 16;
			buf[5] = (n & 0xff000000) >> 24;
			buf[6] = 0;
			break;
		case 'B':
			n = c_get_quantity("Enter info value: ", 0, -1);
			buf[0] = 1; //dummy
			buf[1] = 'j';
			buf[2] = (n & 0xff);
			buf[3] = (n & 0xff00) >> 8;
			buf[4] = (n & 0xff0000) >> 16;
			buf[5] = (n & 0xff000000) >> 24;
			buf[6] = 0;
			break;
		case 'C':
			n = c_get_quantity("Enter info value: ", 0, -1);
			buf[0] = 1; //dummy
			buf[1] = 'I';
			buf[2] = (n & 0xff);
			buf[3] = (n & 0xff00) >> 8;
			buf[4] = (n & 0xff0000) >> 16;
			buf[5] = (n & 0xff000000) >> 24;
			buf[6] = 0;
			break;
		case 'D':
			n = c_get_quantity("Enter info value: ", 0, -1);
			buf[0] = 1; //dummy
			buf[1] = 'J';
			buf[2] = (n & 0xff);
			buf[3] = (n & 0xff00) >> 8;
			buf[4] = (n & 0xff0000) >> 16;
			buf[5] = (n & 0xff000000) >> 24;
			buf[6] = 0;
			break;
		/* Oops */
		default : bell(); break;
		}

		/* If we got a valid command, send it */
		if (buf[0]) Send_master(MASTER_BUILD, buf);

		/* Flush messages */
		clear_topline_forced();
	}

	/* restore responsiveness to hybrid macros */
	inkey_msg = inkey_msg_old;
}
```

#### cmd_master_aux_summon

```c
static void cmd_master_aux_summon(void) {
	char i, redo_hack;
	char buf[80];
	char *race_name;
	bool inkey_msg_old = inkey_msg;

	inkey_msg = TRUE;

	/* Process requests until done */
	while (1) {
		redo_hack = 0;

		/* Clear screen */
		Term_clear();

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Summon...");

		/* Selections */
		Term_putstr(5, 4, -1, TERM_WHITE, "(1) Orcs");
		Term_putstr(5, 5, -1, TERM_WHITE, "(2) Low Undead");
		Term_putstr(5, 6, -1, TERM_WHITE, "(3) High Undead");
		Term_putstr(5, 7, -1, TERM_WHITE, "(4) Depth");
		Term_putstr(5, 8, -1, TERM_WHITE, "(5) Specific");
		Term_putstr(5, 9, -1, TERM_WHITE, "(6) Obliteration");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Summoning mode off");



		/* Prompt */
		Term_putstr(0, 13, -1, TERM_WHITE, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* get the type of monster to summon */
		switch (i) {
		case ':':
			cmd_message();
			inkey_msg = TRUE; /* And suppress macros again.. */
			break;
		/* Take a screenshot */
		case KTRL('T'):
			xhtml_screenshot("screenshot????", 2);
			break;
		/* orc menu */
		case '1':
			/* get the specific kind of orc */
			race_name = cmd_master_aux_summon_orcs();
			/* if no string was specified */
			if (!race_name) {
				redo_hack = 1;
				break;
			}
			buf[2] = 'o';
			strcpy(&buf[3], race_name);
			break;
		/* low undead menu */
		case '2':
			/* get the specific kind of low undead */
			race_name = cmd_master_aux_summon_undead_low();
			/* if no string was specified */
			if (!race_name) {
				redo_hack = 1;
				break;
			}
			buf[2] = 'u';
			strcpy(&buf[3], race_name);
			break;
		/* high undead menu */
		case '3':
			/* get the specific kind of low undead */
			race_name = cmd_master_aux_summon_undead_high();
			/* if no string was specified */
			if (!race_name) {
				redo_hack = 1;
				break;
			}
			buf[2] = 'U';
			strcpy(&buf[3], race_name);
			break;
		/* summon from a specific depth */
		case '4':
			buf[2] = 'd';
			buf[3] = c_get_quantity("Summon from which depth? ", 127, -1);
			/* if (!buf[3]) redo_hack = 1; - Allow depth 0 hereby. */
			buf[4] = 0; /* terminate the string */
			break;
		/* summon a specific monster or character */
		case '5':
			buf[2] = 's';
			buf[3] = 0;
			get_string("Summon which monster or character? ", &buf[3], 79 - 3);
			if (!buf[3]) redo_hack = 1;
			break;

		case '6':
			/* delete all the monsters near us */
			/* turn summoning mode on */
			buf[0] = 'T';
			buf[1] = 1;
			buf[2] = '0';
			buf[3] = '\0'; /* null terminate the monster name */
			Send_master(MASTER_SUMMON, buf);

			redo_hack = 1;
			break;

		case '7':
			/* disable summoning mode */
			buf[0] = 'F';
			buf[3] = '\0'; /* null terminate the monster name */
			Send_master(MASTER_SUMMON, buf);

			redo_hack = 1;
			break;

		/* Oops */
		default : bell(); redo_hack = 1; break;
		}

		/* get how it should be summoned */

		/* hack -- make sure our method is unset so we only send
		 * a monster summon request if we get a valid summoning type
		 */

		/* hack -- don't do this if we hit an invalid key previously */
		if (redo_hack) continue;

		while (1) {
			/* make sure we get a valid summoning type before summoning */
			buf[0] = 0;

			/* Clear screen */
			Term_clear();

			/* Describe */
			Term_putstr(0, 2, -1, TERM_WHITE, "Summon...");

			/* Selections */
			Term_putstr(5, 4, -1, TERM_WHITE, "(1) X here");
			Term_putstr(5, 5, -1, TERM_WHITE, "(2) X at random locations");
			Term_putstr(5, 6, -1, TERM_WHITE, "(3) Group here");
			Term_putstr(5, 7, -1, TERM_WHITE, "(4) Group at random location");
			Term_putstr(5, 8, -1, TERM_WHITE, "(5) Summoning mode");

			/* Prompt */
			Term_putstr(0, 10, -1, TERM_WHITE, "Command: ");

			/* Get a key */
			i = inkey();

			/* Leave */
			if (i == ESCAPE) break;

			/* get the type of summoning */
			switch (i) {
			case ':':
				cmd_message();
				inkey_msg = TRUE; /* And suppress macros again.. */
				break;
			case KTRL('T'):
				xhtml_screenshot("screenshot????", 2);
				break;
			/* X here */
			case '1':
				buf[0] = 'x';
				buf[1] = c_get_quantity("Summon how many? ", 1, -1);
				break;
			/* X in different places */
			case '2':
				buf[0] = 'X';
				buf[1] = c_get_quantity("Summon how many? ", 1, -1);
				break;
			/* Group here */
			case '3':
				buf[0] = 'g';
				break;
			/* Group at random location */
			case '4':
				buf[0] = 'G';
				break;
			/* summoning mode on */
			case '5':
				buf[0] = 'T';
				buf[1] = 1;
				break;

			/* Oops */
			default : bell(); redo_hack = 1; break;
			}
			/* if we have a valid summoning type (escape was not just pressed)
			 * then summon the monster */
			if (buf[0]) Send_master(MASTER_SUMMON, buf);
		}

		/* Flush messages */
		clear_topline_forced();
	}

	/* restore responsiveness to hybrid macros */
	inkey_msg = inkey_msg_old;
}
```

#### cmd_script_upload

```c
static void cmd_script_upload(void) {
	char name[81];
	unsigned short chunksize;

	name[0] = '\0';

	if (!get_string("Script name: ", name, 30)) return;

	/* Starting from protocol version 4.6.1.2, the client can receive 1024 bytes in one packet */
	if (is_newer_than(&server_version, 4, 6, 1, 1, 0, 1))
		chunksize = 1024;
	else
		chunksize = 256;

	remote_update(0, name, chunksize);
}
```

#### cmd_script_exec

```c
static void cmd_script_exec(void) {
	char buf[81];

	buf[0] = '\0';
	if (!get_string("Script> ", buf, 80)) return;

	Send_master(MASTER_SCRIPTS, buf);
}
```

#### cmd_script_exec_local

```c
static void cmd_script_exec_local(void) {
	char buf[81];

	buf[0] = '\0';
	if (!get_string("Script> ", buf, 80)) return;

	c_msg_format("%s", string_exec_lua(0, buf));
}
```

## Verification — 2026-09-22

Production validator passed with `--source-root tomenet=.` and history against
`399fdd38211035b0810cf721ba8e0f1efecf5629`: 462 active, 462 pending evidence,
zero accepted. All prior source/entity/relation records are unchanged. Only
`session.review-final-state` and `store.service` acquire new prerequisites.
The canonical source test passed separately and again after the final binding
context correction. Python compilation and `git diff --check` passed.

The full existing Linux suite passed **22/22 runner invocations**: 12 registry
process tests including 32 published negative fixtures; four headless sanitizer
suites; legacy HP; eight native suites on each of software and OpenGL. The first
sandbox run passed registry/legacy HP but blocked LeakSanitizer thread inspection
and SDL display access. The full approved run outside sandbox passed with actual
display access and sanitizers enabled. Logs: `/tmp/sv10-check-01.log` through
`/tmp/sv10-check-22.log`.

Reproduce from the repository root:

```sh
git show 399fdd382:docs/capabilities/manifest.json > /tmp/sv10-previous-manifest.json
/tmp/sv-capabilities-venv/bin/python tools/validate_capabilities.py \
  --manifest docs/capabilities/manifest.json \
  --ledger docs/capabilities/native-coverage.json \
  --previous-manifest /tmp/sv10-previous-manifest.json --source-root tomenet=.
/tmp/sv-capabilities-venv/bin/python tests/sv_capabilities_checks.py
python3 tests/sv_arch_checks.py
python3 tests/sv_message_checks.py
python3 tests/sv_request_checks.py
python3 tests/sv_lifecycle_checks.py
python3 tests/sv_hp_checks.py --legacy-only
for backend in software opengl; do
    python3 tests/sv_hp_checks.py --backend "$backend"
    python3 tests/sv_arch_native.py --backend "$backend"
    python3 tests/sv_message_native.py --backend "$backend"
    python3 tests/sv_request_native.py --backend "$backend"
    python3 tests/sv_lifecycle_native.py --backend "$backend"
    python3 tests/sv_geometry_native.py --backend "$backend"
    python3 tests/sv_timing_native.py --backend "$backend"
    python3 tests/sv_shell_smoke.py --backend "$backend"
done
```

This data-only change uses the existing approved production registry CLI seam;
no additional runtime implementation or test-only behavior was introduced.
Foundation regressions are not evidence of new native information/social flows.
Windows/Wine, live-server gameplay and human visual acceptance were not exercised.
The optional tracker bootstrap `docs/agents/issue-tracker.md` is absent; the
explicit local task and parent spec supplied review authority.

## Standards

Independent review found incorrect recall/chat/request gesture grouping and a
collision between party-root and guild-configuration input contexts. Bindings
now distinguish open, submit, cancel and local navigation; guild configuration
has its own source-backed context. Follow-up review: **0 unresolved findings**.

## Spec

Independent review found incorrect DM player outcome names, house-delete decline
restoration and house Ctrl-Q cancellation. Source-audited descriptions now match
acquirement/static/delete/broadcast, prefix-only dispatch after canceled editors,
house deletion closure and Escape-only cancellation. Follow-up review verified
these fixes and the corrected tag, guild-adder and local-script paths:
**0 unresolved findings**.

Final review: Standards 0 unresolved; Spec 0 unresolved.
