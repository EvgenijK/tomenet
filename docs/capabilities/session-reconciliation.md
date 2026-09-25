# Ticket 08: session, navigation and targeting reconciliation

This is an allocation, not native acceptance. The source-backed registry retains
all four ticket 07 IDs and adds the session/navigation/target outcomes below.
Every row is pending; evidence IDs are empty. Synthetic A request/lifecycle tests
cannot certify login, death, navigation, or any command-specific cancellation.
The scope is ticket 08 of `.scratch/single-window-sdl3-client-stage-a/issues/08-register-session-movement-and-targeting-outcomes.md`.
The local tracker bootstrap is absent; this explicit local specification is used.

## Authority and method

Primary source bytes, anchors and observed revision are pinned in the manifest.
The resolved credential, text-boundary, persistence and stage decisions are
preserved in [session-policy.md](session-policy.md), so validation does not depend
on ignored planning files. This is a source audit: no account, password, server
session or personal profile was opened. Recovered baseline: `git show
87ead6ff5:docs/research/single-window-behavior-baseline.md`.

The annex snapshots every selected inventory row verbatim with original path,
line locator and inventory digest. Links inside snapshots retain the original
inventory's relative base. Their semantic destinations are explicit IDs; `.result`
and `.lifecycle` obligations on each destination are both required. Field paths,
version branches and replies are scenario requirements, not capabilities per field.
An inventory is a discovery aid; current production source and resolved decisions
win over stale summaries. In particular `cmd_target` cancels on an unmapped key,
`cmd_target_friendly` sends immediately, and `get_dir` is a single-read primitive.
The scoped minimap summary mixes locate exit keys with overview: current
`cmd_mini_map` uses Escape/M to close, Space/5/r to recenter, and Escape inside
sector selection only cancels that selection. The registered outcomes follow
these source branches, not that stale summary. MOTD returns to startup before
Net_start; RETRY_LOGIN sets skip_motd after the first presentation.

## Allocation and cross-domain handoff

B contains real connection/account/character lifecycle, map/navigation and their
actual prerequisites. Hostile target selection and target description are B:
`get_dir` in walk/run/environment commands already admits `*`. C retains friendly
targeting, standalone look and server direction requests. No movement test can
certify the later fire/throw/spell callers. B final-state review requires native
read-only inventory/equipment/character/history children; their full action/menu
families remain C/D. No future stage claim is imported by these allocations.

| Owner ticket | Shared obligation / disposition |
| --- | --- |
| 09 items/combat/spells/stores | Reuse direction/target IDs; add separate fire/throw/spell/mimic/rune/breath success, abort and parent-restoration scenarios. `Receive_item`, `Receive_spell_request`, `Receive_pickup_check`, store entry and ghost powers remain owned here. B stay/open must bring along the particular pickup/store child required for its claimed route; no automatic acceptance from generic prompts. |
| 10 information/social/server surfaces | Reuse session identity and final-review IDs; B death review needs its read-only children. Full character knowledge, social, party/guild, chat and arbitrary documents get their own outcomes. Nested chat in map/locate/look, startup guide/MOTD and alive-close save-chat must restore the exact owner. |
| 11 rendering/resources/settings/files | Reuse startup profile, transfer, DNA, credentials and map IDs. B must supply chosen/effective font/tiles, palette/weather/resize and file/Lua prerequisites. Guide/help from birth, raster capture and tomb dump have caller-specific return/error obligations. XHTML and legacy terminal topology are excluded; no map zoom or secret in config is reinstated. |
| 12 remaining reconciliation | Reconcile physical Linux/Windows/X11/SDL key decoding, macro precedence/waits/raw bypass, text/byte editors, account options and close handlers with these IDs. CLI/config/import producers keep distinct limits. Do not replace pending rows with umbrella primitive acceptance. |
| 13 evidence | Scoped executable/config/resource/source fingerprints and real round trips; dependency invalidation, Windows 10/11, Linux software/accelerated, pending child/fallback rejection. Schema success alone is not evidence. |

These handoffs are required reconciliation work, not approved exclusions. When a
later ticket discovers a required child, use its stable ID in prerequisites and
move its acceptance stage earlier where needed before claiming the parent.
No Stage A completeness or complete renderer/HUD inventory is claimed here.

## Outcome index

Canonical descriptions, applicability and evidence obligations are in the JSON
artifacts. All entries below have related surface/state; input-driven entries
also have action and context-specific binding references.

| Outcome | Stage | Primary behavior anchor |
| --- | --- | --- |
| `capability.connection.select-server` | B | `src/client/c-birth.c`: `get_server_name` |
| `capability.connection.enter-host` | B | `src/client/c-birth.c`: `get_server_name` |
| `capability.connection.cancel-host` | B | `src/client/c-birth.c`: `get_server_name` |
| `capability.connection.contact` | B | `src/client/c-init.c`: `Net_verify` |
| `capability.connection.contact-failure` | B | `src/client/c-init.c`: `Net_verify` |
| `capability.account.enter-name` | B | `src/client/c-birth.c`: `choose_name` |
| `capability.account.cancel-name` | B | `src/client/c-birth.c`: `choose_name` |
| `capability.account.enter-password` | B | `src/client/c-birth.c`: `enter_password` |
| `capability.account.cancel-password` | B | `src/client/c-birth.c`: `enter_password` |
| `capability.account.reject-unencodable-password` | B | `src/client/nclient.c`: `Net_verify` |
| `capability.account.authenticate` | B | `src/client/nclient.c`: `Net_verify` |
| `capability.account.create` | B | `src/client/c-birth.c`: `choose_name` |
| `capability.account.login-rejected` | B | `src/client/nclient.c`: `Receive_login` |
| `capability.account.restore-secret` | B | `src/client/c-birth.c`: `enter_password` |
| `capability.account.save-secret` | B | `src/client/nclient.c`: `Net_verify` |
| `capability.account.secret-provider-failure` | B | `src/client/c-birth.c`: `enter_password` |
| `capability.account.read-information` | B | `src/client/c-util.c`: `Change account password` |
| `capability.account.change-password` | B | `src/client/c-util.c`: `Change account password` |
| `capability.account.cancel-password-change` | B | `src/client/c-util.c`: `Change account password` |
| `capability.account.password-change-failure` | B | `src/client/c-util.c`: `Change account password` |
| `capability.character.read-overview` | B | `src/client/nclient.c`: `Receive_login` |
| `capability.character.select-existing` | B | `src/client/nclient.c`: `Receive_login` |
| `capability.character.create-ordinary` | B | `src/client/nclient.c`: `Receive_login` |
| `capability.character.create-exclusive` | B | `src/client/nclient.c`: `Receive_login` |
| `capability.character.name` | B | `src/client/nclient.c`: `Receive_login` |
| `capability.character.cancel-name` | B | `src/client/nclient.c`: `Receive_login` |
| `capability.character.name-rejected` | B | `src/client/nclient.c`: `Receive_login` |
| `capability.character.swap` | B | `src/client/nclient.c`: `reorder_characters` |
| `capability.character.insert-before` | B | `src/client/nclient.c`: `reorder_characters` |
| `capability.character.append-after` | B | `src/client/nclient.c`: `reorder_characters` |
| `capability.character.cancel-reorder-first` | B | `src/client/nclient.c`: `reorder_characters` |
| `capability.character.cancel-reorder-second` | B | `src/client/nclient.c`: `reorder_characters` |
| `capability.character.quit-overview` | B | `src/client/nclient.c`: `Receive_login` |
| `capability.birth.sex` | B | `src/client/c-birth.c`: `choose_sex` |
| `capability.birth.race` | B | `src/client/c-birth.c`: `choose_race` |
| `capability.birth.trait` | B | `src/client/c-birth.c`: `choose_trait` |
| `capability.birth.class` | B | `src/client/c-birth.c`: `choose_class` |
| `capability.birth.body` | B | `src/client/c-birth.c`: `choose_body_modification` |
| `capability.birth.stats` | B | `src/client/c-birth.c`: `choose_stat_order` |
| `capability.birth.mode` | B | `src/client/c-birth.c`: `choose_mode` |
| `capability.birth.quit` | B | `src/client/c-birth.c`: `choose_sex` |
| `capability.birth.restore-dna` | B | `src/client/c-birth.c`: `Save Birth DNA` |
| `capability.birth.save-dna` | B | `src/client/c-birth.c`: `Save Birth DNA` |
| `capability.birth.complete` | B | `src/client/nclient.c`: `Net_start` |
| `capability.session.read-motd` | B | `src/client/c-files.c`: `show_motd` |
| `capability.session.enter-game` | B | `src/client/nclient.c`: `Net_start` |
| `capability.session.load-profile-input` | B | `src/client/c-init.c`: `Net_verify` |
| `capability.session.transfer-startup-files` | B | `src/client/nclient.c`: `Receive_file` |
| `capability.session.disconnect` | B | `src/client/nclient.c`: `Receive_quit` |
| `capability.session.reconnect` | B | `src/client/c-init.c`: `Net_verify` |
| `capability.session.portal-relogin` | B | `src/client/nclient.c`: `Receive_relogin` |
| `capability.session.quit` | B | `src/client/c-init.c`: `Net_verify` |
| `capability.session.death` | B | `src/client/nclient.c`: `Receive_quit` |
| `capability.session.close-tomb` | B | `src/client/c-util.c`: `c_close_game` |
| `capability.session.review-final-state` | B | `src/client/c-util.c`: `c_close_game` |
| `capability.session.suicide` | B | `src/client/c-cmd.c`: `void cmd_suicide` |
| `capability.session.cancel-suicide` | B | `src/client/c-cmd.c`: `void cmd_suicide` |
| `capability.world.walk` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.run` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.stay` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.stay-one` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.rest` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.search` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.toggle-search` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.ascend` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.descend` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.tunnel` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.open` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.close` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.bash` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.disarm` | B | `src/client/c-cmd.c`: `void cmd_walk` |
| `capability.world.cancel-directional-action` | B | `src/client/c-util.c`: `bool get_dir` |
| `capability.world.interrupt-repeat` | B | `src/client/c-cmd.c`: `void process_command` |
| `capability.world.clear-command-buffer` | B | `src/client/c-cmd.c`: `void process_command` |
| `capability.world.read-location` | B | `src/client/nclient.c`: `int Receive_depth` |
| `capability.world.read-movement-state` | B | `src/client/nclient.c`: `Receive_state` |
| `capability.map.view-overview` | B | `src/client/c-cmd.c`: `void cmd_mini_map` |
| `capability.map.pan-overview` | B | `src/client/c-cmd.c`: `void cmd_mini_map` |
| `capability.map.select-sector` | B | `src/client/c-cmd.c`: `void cmd_mini_map` |
| `capability.map.cancel-sector` | B | `src/client/c-cmd.c`: `void cmd_mini_map` |
| `capability.map.close-overview` | B | `src/client/c-cmd.c`: `void cmd_mini_map` |
| `capability.map.locate` | B | `src/client/c-cmd.c`: `void cmd_locate` |
| `capability.map.close-locate` | B | `src/client/c-cmd.c`: `void cmd_locate` |
| `capability.direction.choose-direction` | B | `src/client/c-util.c`: `bool get_dir` |
| `capability.direction.use-acquired` | B | `src/client/c-util.c`: `bool get_dir` |
| `capability.direction.choose-target` | B | `src/client/c-util.c`: `bool get_dir` |
| `capability.direction.cancel` | B | `src/client/c-util.c`: `bool get_dir` |
| `capability.target.select-hostile` | B | `src/client/c-cmd.c`: `int cmd_target(void)` |
| `capability.target.select-position` | B | `src/client/c-cmd.c`: `int cmd_target(void)` |
| `capability.target.cancel` | B | `src/client/c-cmd.c`: `int cmd_target(void)` |
| `capability.target.select-friendly` | C | `src/client/c-cmd.c`: `int cmd_target_friendly` |
| `capability.target.read-description` | B | `src/client/nclient.c`: `int Receive_target_info` |
| `capability.target.look` | C | `src/client/c-cmd.c`: `void cmd_look` |
| `capability.target.look-position` | C | `src/client/c-cmd.c`: `void cmd_look` |
| `capability.target.close-look` | C | `src/client/c-cmd.c`: `void cmd_look` |
| `capability.direction.answer-server` | C | `src/client/nclient.c`: `int Receive_direction` |
| `capability.direction.cancel-server` | C | `src/client/nclient.c`: `int Receive_direction` |
| `capability.session.acknowledge-final-scene` | B | `src/client/c-util.c`: `c_close_game` |
| `capability.session.open-final-review` | B | `src/client/c-util.c`: `c_close_game` |
| `capability.session.close-final-review` | B | `src/client/c-util.c`: `c_close_game` |
| `capability.session.read-identity` | B | `src/client/nclient.c`: `Receive_char_info` |
| `capability.world.read-map` | B | `src/client/nclient.c`: `Receive_line_info` |
| `capability.session.startup-file-failure` | B | `src/client/nclient.c`: `Receive_file` |
| `capability.birth.backtrack-race` | B | `src/client/c-birth.c`: `choose_race` |
| `capability.birth.backtrack-trait` | B | `src/client/c-birth.c`: `choose_trait` |
| `capability.birth.backtrack-class` | B | `src/client/c-birth.c`: `choose_class` |
| `capability.birth.backtrack-body` | B | `src/client/c-birth.c`: `choose_body_modification` |
| `capability.birth.backtrack-stats` | B | `src/client/c-birth.c`: `choose_stat_order` |
| `capability.birth.backtrack-mode` | B | `src/client/c-birth.c`: `choose_mode` |

## Baseline family reconciliation

| Original family | This ticket's destination or source-backed disposition |
| --- | --- |
| LIFE-01 | connection.enter-host, account.enter-name/password, character.select-existing and session.load-profile-input: explicit CLI/default producers. Remaining CLI/settings and package switches are ticket 11/12, not removed. |
| LIFE-02 | connection.select-server/enter-host/cancel-host/contact/contact-failure; account.authenticate/create/login-rejected. |
| LIFE-03 | character overview/select/create/name/reorder outcomes and per-step birth outcomes; live identity separate from birth draft. |
| LIFE-04 | session MOTD/play/disconnect/reconnect/portal/quit/death/final-scene/tomb/review, plus file/profile prerequisites. |
| CMD-01 | Every walk/run/stay/rest/search/stairs/tunnel/open/close/bash/disarm/locate/overview action has an outcome; Space interruption and right-parenthesis buffer clear remain distinct. |
| CMD-03 / PROMPT-01 | target/direction/look outcomes here; fire/throw/skills and their callers belong to 09. |
| CMD-05 | suicide/quit here; remaining utilities/admin are 10/11. |
| HUD-01 / HUD-02 / DATA-03 | identity, depth, movement state, target info and world/overview map fields map below; remaining HUD and renderer effects are 10/11 with B prerequisites when visible during these flows. |
| FILE-01 / FILE-02 | independent profile and shared DNA/resources plus startup transfer/reload/failures here; complete import/editor/export leaves are 11. |
| IN-01 / IN-02 / IN-03 | session.load-profile-input plus each actual caller scenario; exhaustive physical mapping and macro editing remain 12/11. |
| Remaining baseline families | Items/stores→09; social/information/server documents→10; renderer/audio/files/platform→11/12. This ticket does not turn these families into excluded or accepted outcomes. |

## Scoped and remaining input rows

### Scoped owners

Original: `docs/research/single-window-input-loops.md`; SHA-256 `40a47b0ae685a8a1ee2f1309c76e60722783b05aa494504f47b7a5ed8b88fef7`.

- Inventory line 30 → `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| `input.core.event` | SDL3 `key_press()` | printable text; F1-F12; arrows/Home/End/PgUp/PgDn/Insert; keypad; Ctrl/Shift/Alt | physical | before macros | key-up is irrelevant to logical loop | modifier bits become `inkey_shift_special`; special keys become `NAVI_KEY_*` sequences when enabled | enqueue bytes | current `term` | `USE_SDL3`; `ALLOW_NAVI_KEYS_IN_PROMPT`; `ENABLE_SHIFT_SPECIALKEYS`; `SDL3_STICKY_KEYS` | [`main-sdl3.c:1091`](../../src/client/main-sdl3.c#L1091), [`main-sdl3.c:1162`](../../src/client/main-sdl3.c#L1162), [`main-sdl3.c:1215`](../../src/client/main-sdl3.c#L1215) |
```

- Inventory line 31 → `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| `input.core.macro` | `inkey_aux()` / `inkey()` | longest matching trigger; backquote becomes Esc after matching; bytes 28/29/30/31 delimit fallback/action/special sequences | before keymap | policy from flags | unmatched trigger bytes are pushed back; multi-key wait tops out after increasing 10-unit delays; macro completion byte 29 ends `parse_macro` | `parse_macro`, `after_macro`, `parse_under`, `parse_slash`, `strip_chars`, `macro_missing_item` | none | activates main term, then restores old term and cursor | `ALLOW_NAVI_KEYS_IN_PROMPT`; `SOME_NAVI_KEYS_DISABLE_MACROS_IN_PROMPTS` | [`c-util.c:1160`](../../src/client/c-util.c#L1160), [`c-util.c:1262`](../../src/client/c-util.c#L1262), [`c-util.c:1376`](../../src/client/c-util.c#L1376), [`c-util.c:1618`](../../src/client/c-util.c#L1618) |
```

- Inventory line 32 → `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| `input.core.macro-wait` | macro `\\wDD` -> `sync_sleep()` | byte 96 plus exactly two decimal digits; waits `DD * 100ms` while pumping network/timers/redraw | before keymap | only inside expanded macro | completes on duration, nested input semaphore, or server `PKT_CONFIRM`; interactive cancellation is compiled out | `command_confirmed=-1`, `inkey_sleep=true`; reset sleep/semaphore at completion | keepalive/ping/network processing | spinner in last topline cell, then erase | `ACCEPT_KEYS` disabled by source; Windows timer vs `gettimeofday` | [`c-util.c:156`](../../src/client/c-util.c#L156), [`c-util.c:417`](../../src/client/c-util.c#L417), [`c-util.c:494`](../../src/client/c-util.c#L494), [`c-util.c:939`](../../src/client/c-util.c#L939) |
```

- Inventory line 33 → `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| `input.core.macro-xwait` | macro `\\WDDDD` encoding -> byte 30 -> `sync_xsleep()` | byte 30 plus exactly four decimal digits; waits `DDDD * 100ms`; fresh Esc aborts, Space resumes early, other fresh keys are preserved in old queue | before keymap | only inside expanded macro | duration/semaphore/confirm completes; Esc discards temporary queue; Space restores old queue | swaps `Term->keys` with temporary queue; same sleep flags | keepalive/ping/network processing | spinner in last topline cell, then erase | Windows timer vs `gettimeofday`; byte 30 legacy control-caret collision noted in source | [`c-util.c:157`](../../src/client/c-util.c#L157), [`c-util.c:568`](../../src/client/c-util.c#L568), [`c-util.c:651`](../../src/client/c-util.c#L651), [`c-util.c:1036`](../../src/client/c-util.c#L1036) |
```

- Inventory line 35 → `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| `input.core.prompt-navigation` | `inkey_combo()` / `scan_navi_key()` | arrow/Home/End/PgUp/PgDn/Delete become `NAVI_KEY_*`; optional edit modifier preserves legacy default text behavior | same | navigation may bypass macro matching | helper toggles `inkey_location_keys` only for the read, then clears it | cursor/key result | none | caller prompt | `ALLOW_NAVI_KEYS_IN_PROMPT`; optional `SOME_NAVI_KEYS_DISABLE_MACROS_IN_PROMPTS` makes `inkey_aux()` return decoded navigation before macro lookup | [`c-util.c:732`](../../src/client/c-util.c#L732), [`c-util.c:855`](../../src/client/c-util.c#L855), [`c-util.c:1241`](../../src/client/c-util.c#L1241) |
```

- Inventory line 36 → `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| `input.prompt.text` | `askfor_aux()` | Enter accept; Esc cancel; Backspace/Delete; arrows/Home/End; Ctrl-A/E/B/F; history up/down; Ctrl-R search, Ctrl-G cancel search; Ctrl-K copy; Ctrl-L paste; printable insertion | same | hybrid normally caller-suppressed | Esc false; Enter true, including empty; invalid/full input bells and retries | editable buffer, cursor, history cursor, search result; private mode masks output and disables initial edit | none | topline/caller cursor | `ALLOW_NAVI_KEYS_IN_PROMPT`; clipboard hooks; `ASKFOR_PRIVATE`, `ASKFOR_CHATTING`, `ASKFOR_LIVETRIM` | [`c-util.c:2613`](../../src/client/c-util.c#L2613), [`c-util.c:2699`](../../src/client/c-util.c#L2699), [`c-util.c:2720`](../../src/client/c-util.c#L2720), [`c-util.c:3385`](../../src/client/c-util.c#L3385) |
```

- Inventory line 37 → `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| `input.prompt.command` | `get_com()` / `get_com_bk()` | one logical key | same | inherited | Esc false; `_bk`: Backspace returns -2; every other key true | none | none | topline; flush queue after | none | [`c-util.c:3516`](../../src/client/c-util.c#L3516), [`c-util.c:3542`](../../src/client/c-util.c#L3542) |
```

- Inventory line 38 → `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| `input.prompt.confirm` | `get_check2/3()`, `get_3way()` | y/n; `get_3way`: y/a/n | same | inherited | `get_check2`: any key takes displayed default except explicit opposite; `get_check3(0)` retries until y/n; `get_3way` retries until y/a/n and, with default-no, Esc/Enter/Ctrl-Q mean no | none | none | topline; flush queue | none | [`c-util.c:3765`](../../src/client/c-util.c#L3765), [`c-util.c:3816`](../../src/client/c-util.c#L3816), [`c-util.c:3858`](../../src/client/c-util.c#L3858) |
```

- Inventory line 40 → `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| `input.command.gameplay` | `request_command()` -> `process_command()` | next available key; `\\` bypasses keymap; `^` reads a second key and makes Control | translated | command-only (`inkey_flag`) | scan mode returns immediately if empty; Esc/no mapping no-op | sets `command_cmd`, `command_dir`; clears topline | see command table | current game surface | none | [`c-util.c:3583`](../../src/client/c-util.c#L3583), [`c-cmd.c:274`](../../src/client/c-cmd.c#L274) |
```

- Inventory line 41 → `capability.direction.choose-direction`, `capability.direction.use-acquired`, `capability.direction.choose-target`, `capability.direction.cancel`; obligations `.result` and `.lifecycle`.

```text
| `input.command.direction` | `get_dir()` | direction; `*` open targeter; `-` acquired target; `+` acquired/manual | keymap direction | inherited | no valid direction false; target cancel false; otherwise confirm | returns 1..11 | nested target may emit target state; caller emits action | topline | none | [`c-util.c:3672`](../../src/client/c-util.c#L3672) |
```

- Inventory line 42 → `capability.map.view-overview`, `capability.map.pan-overview`, `capability.map.select-sector`, `capability.map.cancel-sector`, `capability.map.close-overview`, `capability.map.locate`, `capability.map.close-locate`; obligations `.result` and `.lifecycle`.

```text
| `input.command.minimap` | `cmd_mini_map()` | direction pan; Space/Esc/normal `L` or rogue `W` exit; `:` chat; Ctrl-T screenshot | directions translated locally | hybrid suppressed around chat | exit returns; invalid retries | viewport offsets | `Send_special_line()` supplies/refreshes map | save/load | map/server feature branches | [`c-cmd.c:555`](../../src/client/c-cmd.c#L555), [`c-cmd.c:638`](../../src/client/c-cmd.c#L638), [`c-cmd.c:788`](../../src/client/c-cmd.c#L788) |
```

- Inventory line 45 → `capability.target.select-hostile`, `capability.target.select-position`, `capability.target.cancel`, `capability.target.select-friendly`, `capability.target.look`, `capability.target.look-position`, `capability.target.close-look`; obligations `.result` and `.lifecycle`.

```text
| `input.command.target` | `cmd_target()`, `cmd_target_friendly()`, `cmd_look()` | directions move; t/5 choose; p player; q/Esc cancel; look adds x/l modes, `:` chat, Ctrl-T | directions via keymap | hybrid suppressed in look | cancel returns 0; invalid retries | cursor, target mode, selected grid/entity | target packets through target helpers | screen redraw/return | version/entity availability | [`c-cmd.c:2040`](../../src/client/c-cmd.c#L2040), [`c-cmd.c:2101`](../../src/client/c-cmd.c#L2101), [`c-cmd.c:2109`](../../src/client/c-cmd.c#L2109) |
```

- Inventory line 63 → `capability.account.read-information`, `capability.account.change-password`, `capability.account.cancel-password-change`, `capability.account.password-change-failure`; obligations `.result` and `.lifecycle`.

```text
| `input.options.account-window` | `do_cmd_options_acc()`, `_win()` | exact account/window keys in subordinate map below | same | hybrid suppressed | Esc exits; prompt cancel returns | account/window flags | account/server option messages where applicable | save/load | account privileges, `ANGBAND_TERM_MAX` | [`c-util.c:13070`](../../src/client/c-util.c#L13070), [`c-util.c:13106`](../../src/client/c-util.c#L13106), [`c-util.c:13199`](../../src/client/c-util.c#L13199) |
```

- Inventory line 74 → `capability.account.enter-name`, `capability.account.cancel-name`, `capability.account.enter-password`, `capability.account.cancel-password`, `capability.account.login-rejected`; obligations `.result` and `.lifecycle`.

```text
| `input.birth.credentials` | `choose_name()`, `enter_password()` | live-trim account text; private password | same | startup macro set not semantically applicable | name Esc exits process; password Esc goes back to name; empty retries | `nick`, `pass` | later login handshake | replace | `RETRY_LOGIN`, `SIMPLE_LOGIN` | [`c-birth.c:191`](../../src/client/c-birth.c#L191), [`c-birth.c:274`](../../src/client/c-birth.c#L274) |
```

- Inventory line 75 → `capability.birth.sex`, `capability.birth.race`, `capability.birth.trait`, `capability.birth.class`, `capability.birth.backtrack-race`, `capability.birth.backtrack-trait`, `capability.birth.backtrack-class`, `capability.birth.quit`, `capability.birth.restore-dna`; obligations `.result` and `.lifecycle`.

```text
| `input.birth.identity` | sex/race/trait/class | letter; 2/4/6/8 and +/-/<> navigate where listed; Enter highlighted; `*` random; `#` saved DNA; `%` auto-reincarnate; Backspace previous; ? guide; Q/Ctrl-Q quit; Ctrl-T | same | startup | invalid retries; Backspace returns false; random/DNA loops internally until legal | `sex`, `race`, `trait`, `class`, selection/highlight, `auto_reincarnation` | none until completed birth | replace | class/race compatibility, `CLASS_BEFORE_RACE`, `RETRY_LOGIN`, server setup | [`c-birth.c:323`](../../src/client/c-birth.c#L323), [`c-birth.c:438`](../../src/client/c-birth.c#L438), [`c-birth.c:626`](../../src/client/c-birth.c#L626), [`c-birth.c:887`](../../src/client/c-birth.c#L887) |
```

- Inventory line 76 → `capability.birth.stats`, `capability.birth.mode`, `capability.birth.body`, `capability.birth.backtrack-stats`, `capability.birth.backtrack-mode`, `capability.birth.backtrack-body`, `capability.birth.save-dna`, `capability.birth.complete`; obligations `.result` and `.lifecycle`.

```text
| `input.birth.stats-mode-body` | `choose_stat_order()`, `choose_mode()`, `choose_body_modification()` | selection letters/directions; random/DNA/reincarnate; Backspace; Q; ?; stat distribution +/- and confirmation if points remain | same | startup | invalid retry; Backspace parent; remaining-stat no returns to editor | stat order/points, mode bits, fruit-bat bit | none until birth submit | replace | server mode flags, arcade/RPG, auto-reincarnation | [`c-birth.c:1122`](../../src/client/c-birth.c#L1122), [`c-birth.c:1422`](../../src/client/c-birth.c#L1422), [`c-birth.c:1552`](../../src/client/c-birth.c#L1552), [`c-birth.c:1896`](../../src/client/c-birth.c#L1896) |
```

- Inventory line 77 → `capability.connection.select-server`, `capability.connection.enter-host`, `capability.connection.cancel-host`, `capability.connection.contact-failure`; obligations `.result` and `.lifecycle`.

```text
| `input.birth.server` | `get_server_name()` / manual | metaserver entry letter; Q/Ctrl-Q manual; manual hostname text/Esc | same | startup | invalid list key retries; manual Esc cancels/quit at caller; selection confirms | chosen host/port; ping subprocess bookkeeping | connect target | replace; first `inkey()` may raise SDL/X11 window | `EXPERIMENTAL_META`, `META_PINGS`, Windows/POSIX, `USE_SDL3` | [`c-birth.c:2380`](../../src/client/c-birth.c#L2380), [`c-birth.c:2470`](../../src/client/c-birth.c#L2470), [`c-birth.c:2742`](../../src/client/c-birth.c#L2742) |
```

- Inventory line 78 → `capability.session.death`, `capability.session.acknowledge-final-scene`, `capability.session.close-tomb`, `capability.session.open-final-review`, `capability.session.review-final-state`, `capability.session.close-final-review`; obligations `.result` and `.lifecycle`.

```text
| `input.shutdown` | `c_close_game()` | acknowledgements; tomb/reincarnation menu; Ctrl-T screenshot; filename prompt | same | macros suppressed in menus | key depends death/alive branch; exit eventually closes | death/reincarnation/screenshot state | disconnect/quit handled by caller | replace | death state, screenshot support | [`c-util.c:17705`](../../src/client/c-util.c#L17705), [`c-util.c:17796`](../../src/client/c-util.c#L17796), [`c-util.c:17907`](../../src/client/c-util.c#L17907) |
```

### Remaining owners and delegated entries

Original: `.scratch/single-window-sdl3-client/research/remaining-client-input-loops.md`; SHA-256 `76f85717b58f8a026a43dab05ee322150abea260f07f38316886186b71d47caf`.

- Inventory line 24 → `capability.character.read-overview`, `capability.character.select-existing`, `capability.character.create-ordinary`, `capability.character.create-exclusive`, `capability.character.name`, `capability.character.cancel-name`, `capability.character.quit-overview`; obligations `.result` and `.lifecycle`.

```text
| `input.account.character-overview` | `Receive_login` | runtime `a..` existing; `N` new; `E` slot-exclusive new; `S/I/A` swap/insert-before/append-after; `Q`/Ctrl-Q quit; Ctrl-T screenshot | same; inherited | invalid silent retry; name Esc → overview; reorder Esc → overview; successful reorder → re-read server characters | ch; new_ok/exclusive_ok/firstrun; names/modes/counts; reincarnate_previous | existing/new character selection consumed by login; reorder packet below | replace screen; hide cursor; clears before return | server max_cpa/dedicated slots; allow_reordering; RETRY_LOGIN; E forbidden firstrun | [nclient.c:762](../../../src/client/nclient.c#L762), [read:1134](../../../src/client/nclient.c#L1134), [reorder:1151](../../../src/client/nclient.c#L1151), [name:1189](../../../src/client/nclient.c#L1189) |
```

- Inventory line 25 → `capability.character.swap`, `capability.character.insert-before`, `capability.character.append-after`, `capability.character.cancel-reorder-first`, `capability.character.cancel-reorder-second`; obligations `.result` and `.lifecycle`.

```text
| `input.account.character-reorder` | `reorder_characters`, two separately owned stages in one two-step context | first slot a..charcount; second slot a..charcount; mode-specific prompts swap/before/after | same; inherited | Esc cancels either stage; invalid silent retry; second valid sends and returns | sortA then sortB; mode1/2/3 | `PKT_LOGIN`, string `***<sortA><sortB><mode>`; Net_flush; caller waits refreshed overview | inherits overview; prompt and first selection erased on cancel | allow_reordering in caller | [nclient.c:701](../../../src/client/nclient.c#L701), [first:715](../../../src/client/nclient.c#L715), [second:731](../../../src/client/nclient.c#L731), [packet:745](../../../src/client/nclient.c#L745) |
```

- Inventory line 41 → `capability.session.read-motd`; obligations `.result` and `.lifecycle`.

```text
| `show_browse`:403; `show_motd`:1937; `display_message`:3337; `Receive_pause`:6208 | one-key acknowledgement via core read; any logical key | browse/display/pause one read; motd loops while key0; display skipped quit_no_prompt | browse local; motd timed delay; pause flushes physical queue **before** read and Flush_queue after | browse/motd/display save/load; pause existing surface | [c-spell.c:386](../../../src/client/c-spell.c#L386), [c-files.c:1910](../../../src/client/c-files.c#L1910), [c-init.c:3268](../../../src/client/c-init.c#L3268), [nclient.c:6193](../../../src/client/nclient.c#L6193) |
```

- Inventory line 47 → `capability.character.name`, `capability.character.cancel-name`; obligations `.result` and `.lifecycle`.

```text
| `Receive_login`:1189 | `input.prompt.text`, CNAME_LEN-1, ASKFOR_LIVETRIM/PLUSPREFIX; New name, optional reincarnation `+` | Esc overview; empty generates random name then remains name editor; nonempty accepts | + prefix reincarnate; trim/titlecase; creation mode E dedicated flags | overview row, no extra top-level modal | previous/commandline/account defaults; [nclient.c:1168](../../../src/client/nclient.c#L1168) |
```

- Inventory line 50 → `capability.direction.answer-server`, `capability.direction.cancel-server`; obligations `.result` and `.lifecycle`.

```text
| `Receive_direction`:4020 | direction primitive | cancel returns0 no reply; busy qbuf | PKT_DIRECTION byte dir | topline | !screen_icky&&!topline_icky&&!shopping; [nclient.c:4012](../../../src/client/nclient.c#L4012) |
```

- Inventory line 121 → `capability.session.quit`; obligations `.result` and `.lifecycle`.

```text
| [main-win.c](../../../src/client/main-win.c) | 5706:get_3way | applicable save-chat delegate above; remaining event/window topology or file reads |
```

- Inventory line 122 → `capability.session.quit`; obligations `.result` and `.lifecycle`.

```text
| [main-x11.c](../../../src/client/main-x11.c) | 1571:get_3way | applicable save-chat delegate above; remaining event/window topology or file reads |
```

The remaining-loop rows `input.startup.bigmap-choice` and
`input.startup.graphics-choice` belong to the explicit B startup resource/geometry
prerequisite of `capability.session.load-profile-input`; the old topology-driven
size question is superseded by fullscreen/normal-or-big-grid policy, not copied
as an SV terminal-size question. Restart/unsupported graphics/error acknowledgement
paths remain obligations of the selected/effective resource policy (ticket 11).
Spell/Lua direction delegates reuse `capability.direction.*` but each complete
spell owner and safe-macro branch is 09. `Receive_pause` and generic key/amount/
number/string/confirm/abort requests are 10/12; the four existing A IDs retain
only their original synthetic scope. OS event pumps preserve close/input outcomes;
separate terminal-window management and unsupported AMI/AMY platforms are excluded.

## Versioned packet and response reconciliation

For each mapped packet row test all its stated version/build gates, complete
input, every split boundary and adjacent packet. Incomplete input publishes
nothing; malformed framing fails the connection without invented replies.
Resolved text policy supersedes unsafe legacy partial mutation/truncation.
The same incoming map packet can serve the live world or overview owner: test
both contexts. Outbound-only tokens below are intents, not missing receive handlers.

### Registered handlers, field variants and directional tokens

Original: `docs/research/single-window-packet-state.md`; SHA-256 `1e3bcbb6eb15dcd412e206c721a11d85cae5fc8a0d5094430e632f84836db84d`.

- Inventory line 18 → `capability.session.disconnect`, `capability.session.reconnect`, `capability.session.death`; obligations `.result` and `.lifecycle`.

```text
| 4 | `PKT_QUIT` ([src/common/pack.h:26](../../src/common/pack.h#L26)) | `always`; [binding](../../src/client/nclient.c#L344) | `Receive_quit` — connection termination reason/control |
```

- Inventory line 19 → `capability.session.portal-relogin`; obligations `.result` and `.lifecycle`.

```text
| 216 | `PKT_RELOGIN` ([src/common/pack.h:277](../../src/common/pack.h#L277)) | `ifdef SERVER_PORTALS`; [binding](../../src/client/nclient.c#L346) | `Receive_relogin` — portal reconnect credentials/control |
```

- Inventory line 21 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| 11 | `PKT_END` ([src/common/pack.h:34](../../src/common/pack.h#L34)) | `always`; [binding](../../src/client/nclient.c#L349) | `Receive_end` — end-of-update marker |
```

- Inventory line 22 → `capability.character.read-overview`, `capability.character.select-existing`, `capability.character.swap`; obligations `.result` and `.lifecycle`.

```text
| 12 | `PKT_LOGIN` ([src/common/pack.h:35](../../src/common/pack.h#L35)) | `always`; [binding](../../src/client/nclient.c#L350) | `NULL` — explicitly rejected/unknown path |
```

- Inventory line 23 → `capability.session.transfer-startup-files`, `capability.session.startup-file-failure`; obligations `.result` and `.lifecycle`.

```text
| 14 | `PKT_FILE` ([src/common/pack.h:39](../../src/common/pack.h#L39)) | `always`; [binding](../../src/client/nclient.c#L351) | `Receive_file` — file-transfer session/filesystem and Lua/audio reload |
```

- Inventory line 29 → `capability.session.read-identity`; obligations `.result` and `.lifecycle`.

```text
| 26 | `PKT_CHAR_INFO` ([src/common/pack.h:48](../../src/common/pack.h#L48)) | `always`; [binding](../../src/client/nclient.c#L358) | `Receive_char_info` — character identity/mode |
```

- Inventory line 36 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| 47 | `PKT_CHAR` ([src/common/pack.h:71](../../src/common/pack.h#L71)) | `always`; [binding](../../src/client/nclient.c#L365) | `Receive_char` — map cell |
```

- Inventory line 38 → `capability.world.read-movement-state`; obligations `.result` and `.lifecycle`.

```text
| 40 | `PKT_STATE` ([src/common/pack.h:64](../../src/common/pack.h#L64)) | `always`; [binding](../../src/client/nclient.c#L367) | `Receive_state` — activity state |
```

- Inventory line 40 → `capability.world.read-location`; obligations `.result` and `.lifecycle`.

```text
| 34 | `PKT_DEPTH` ([src/common/pack.h:57](../../src/common/pack.h#L57)) | `always`; [binding](../../src/client/nclient.c#L369) | `Receive_depth` — world position/location labels |
```

- Inventory line 54 → `capability.direction.answer-server`, `capability.direction.cancel-server`; obligations `.result` and `.lifecycle`.

```text
| 60 | `PKT_DIRECTION` ([src/common/pack.h:87](../../src/common/pack.h#L87)) | `always`; [binding](../../src/client/nclient.c#L383) | `Receive_direction` — direction request |
```

- Inventory line 56 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| 41 | `PKT_LINE_INFO` ([src/common/pack.h:65](../../src/common/pack.h#L65)) | `always`; [binding](../../src/client/nclient.c#L385) | `Receive_line_info` — map/minimap row cells |
```

- Inventory line 61 → `capability.target.read-description`, `capability.target.look`, `capability.target.select-hostile`; obligations `.result` and `.lifecycle`.

```text
| 53 | `PKT_TARGET_INFO` ([src/common/pack.h:78](../../src/common/pack.h#L78)) | `always`; [binding](../../src/client/nclient.c#L390) | `Receive_target_info` — target coordinates/description |
```

- Inventory line 63 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| 55 | `PKT_MINI_MAP` ([src/common/pack.h:80](../../src/common/pack.h#L80)) | `always`; [binding](../../src/client/nclient.c#L392) | `Receive_line_info` — map/minimap row cells |
```

- Inventory line 105 → `capability.map.view-overview`, `capability.map.select-sector`, `capability.map.cancel-sector`; obligations `.result` and `.lifecycle`.

```text
| 134 | `PKT_MINI_MAP_POS` ([src/common/pack.h:170](../../src/common/pack.h#L170)) | `always`; [binding](../../src/client/nclient.c#L437) | `Receive_mini_map_pos` — minimap cursor/world position |
```

- Inventory line 132 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| 214 | `PKT_CHAR_DIRECT` ([src/common/pack.h:274](../../src/common/pack.h#L274)) | `always`; [binding](../../src/client/nclient.c#L470) | `Receive_char` — map cell |
```

- Inventory line 169 → `capability.session.disconnect`, `capability.session.reconnect`, `capability.session.death`; obligations `.result` and `.lifecycle`.

```text
| `PKT_QUIT` → `Receive_quit` step 1 | `%c` ⇒ pkt:i8/raw byte | always | malformed => -1/connection failure | connection termination reason/control; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L2004)) | none | [src/client/nclient.c:2013](../../src/client/nclient.c#L2013) |
```

- Inventory line 170 → `capability.session.disconnect`, `capability.session.reconnect`, `capability.session.death`; obligations `.result` and `.lifecycle`.

```text
| `PKT_QUIT` → `Receive_quit` step 2 | `%s` ⇒ reason:NUL string<MAX_CHARS> | always | malformed => -1/connection failure | connection termination reason/control; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L2004)) | none | [src/client/nclient.c:2017](../../src/client/nclient.c#L2017) |
```

- Inventory line 171 → `capability.session.portal-relogin`; obligations `.result` and `.lifecycle`.

```text
| `PKT_RELOGIN` → `Receive_relogin` step 1 | `%c%s%d%s%s%s%s%c` ⇒ pkt:i8/raw byte, relogin_host:NUL string<MAX_CHARS>, relogin_port:i32be, relogin_accname:NUL string<MAX_CHARS>, relogin_accpass:NUL string<MAX_CHARS>, relogin_charname:NUL string<MAX_CHARS>, reason:NUL string<MAX_CHARS>, delay:i8/raw byte | SERVER_PORTALS | malformed => -1/connection failure | portal reconnect credentials/control; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L2053)) | none | [src/client/nclient.c:2063](../../src/client/nclient.c#L2063) |
```

- Inventory line 172 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_END` → `Receive_end` step 1 | `%c` ⇒ ch:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | end-of-update marker; event/command; consume once | none; retain decoded fields ([handler](../../src/client/nclient.c#L1965)) | none | [src/client/nclient.c:1969](../../src/client/nclient.c#L1969) |
```

- Inventory line 173 → `capability.session.transfer-startup-files`, `capability.session.startup-file-failure`; obligations `.result` and `.lifecycle`.

```text
| `PKT_FILE` → `Receive_file` step 1 | `%c%c%hd` ⇒ ch:i8/raw byte, command:i8/raw byte, fnum:i16be | always | short input rolls back; invalid fnum/subtype logs and ACK/ERR; filesystem errors reported | file-transfer session/filesystem and Lua/audio reload; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L477)) | `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:491](../../src/client/nclient.c#L491) |
```

- Inventory line 174 → `capability.session.transfer-startup-files`, `capability.session.startup-file-failure`; obligations `.result` and `.lifecycle`.

```text
| `PKT_FILE` → `Receive_file` step 2 | `%s` ⇒ fname:NUL string<MAX_CHARS> | always | short input rolls back; invalid fnum/subtype logs and ACK/ERR; filesystem errors reported | file-transfer session/filesystem and Lua/audio reload; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L477)) | `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:505](../../src/client/nclient.c#L505) |
```

- Inventory line 175 → `capability.session.transfer-startup-files`, `capability.session.startup-file-failure`; obligations `.result` and `.lifecycle`.

```text
| `PKT_FILE` → `Receive_file` step 3 | `%hd` ⇒ len:i16be | always | short input rolls back; invalid fnum/subtype logs and ACK/ERR; filesystem errors reported | file-transfer session/filesystem and Lua/audio reload; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L477)) | `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:527](../../src/client/nclient.c#L527) |
```

- Inventory line 176 → `capability.session.transfer-startup-files`, `capability.session.startup-file-failure`; obligations `.result` and `.lifecycle`.

```text
| `PKT_FILE` → `Receive_file` step 4 | `%s` ⇒ fname:NUL string<MAX_CHARS> | always | short input rolls back; invalid fnum/subtype logs and ACK/ERR; filesystem errors reported | file-transfer session/filesystem and Lua/audio reload; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L477)) | `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:594](../../src/client/nclient.c#L594) |
```

- Inventory line 177 → `capability.session.transfer-startup-files`, `capability.session.startup-file-failure`; obligations `.result` and `.lifecycle`.

```text
| `PKT_FILE` → `Receive_file` step 5 | `%u%u%u%u` ⇒ digest_net[0]:u32be, digest_net[1]:u32be, digest_net[2]:u32be, digest_net[3]:u32be | > 4.6.1.1.0.1 | short input rolls back; invalid fnum/subtype logs and ACK/ERR; filesystem errors reported | file-transfer session/filesystem and Lua/audio reload; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L477)) | `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:615](../../src/client/nclient.c#L615) |
```

- Inventory line 178 → `capability.session.transfer-startup-files`, `capability.session.startup-file-failure`; obligations `.result` and `.lifecycle`.

```text
| `PKT_FILE` → `Receive_file` step 6 | `%d` ⇒ csum:i32be | otherwise | short input rolls back; invalid fnum/subtype logs and ACK/ERR; filesystem errors reported | file-transfer session/filesystem and Lua/audio reload; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L477)) | `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:623](../../src/client/nclient.c#L623) |
```

- Inventory line 192 → `capability.session.read-identity`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_INFO` → `Receive_char_info` step 1 | `%c%hd%hd%hd%hd%d%hd%s` ⇒ ch:i8/raw byte, race:i16be, class:i16be, trait:i16be, sex:i16be, mode:i32be, lives:i16be, cname:NUL string<MAX_CHARS> | >= 4.9.2.1.0.1 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2825](../../src/client/nclient.c#L2825) |
```

- Inventory line 193 → `capability.session.read-identity`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_INFO` → `Receive_char_info` step 2 | `%c%hd%hd%hd%hd%hd%hd%s` ⇒ ch:i8/raw byte, race:i16be, class:i16be, trait:i16be, sex:i16be, mode:i16be, lives:i16be, cname:NUL string<MAX_CHARS> | >= 4.7.3.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2827](../../src/client/nclient.c#L2827) |
```

- Inventory line 194 → `capability.session.read-identity`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_INFO` → `Receive_char_info` step 3 | `%c%hd%hd%hd%hd%hd%s` ⇒ ch:i8/raw byte, race:i16be, class:i16be, trait:i16be, sex:i16be, mode:i16be, cname:NUL string<MAX_CHARS> | > 4.5.2.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2829](../../src/client/nclient.c#L2829) |
```

- Inventory line 195 → `capability.session.read-identity`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_INFO` → `Receive_char_info` step 4 | `%c%hd%hd%hd%hd%hd` ⇒ ch:i8/raw byte, race:i16be, class:i16be, trait:i16be, sex:i16be, mode:i16be | > 4.4.5.10.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2831](../../src/client/nclient.c#L2831) |
```

- Inventory line 196 → `capability.session.read-identity`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_INFO` → `Receive_char_info` step 5 | `%c%hd%hd%hd%hd` ⇒ ch:i8/raw byte, race:i16be, class:i16be, sex:i16be, mode:i16be | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | character identity/mode; latest snapshot; replace on packet, connection reset owns clear | retains normalized identity/mode in globals and `p_ptr`; pre-normalization mode bits/defaulted absent fields are not retained ([handler](../../src/client/nclient.c#L2812)) | none | [src/client/nclient.c:2833](../../src/client/nclient.c#L2833) |
```

- Inventory line 204 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR` → `Receive_char` step 1 | `%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[0]:i8/raw byte, a_back:i8/raw byte, pc_b[0]:i8/raw byte | GRAPHICS_BG_MASK + UG_2MASK + >= 4.9.2.1 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3183](../../src/client/nclient.c#L3183) |
```

- Inventory line 205 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR` → `Receive_char` step 2 | `%c%c%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte, a_back:i8/raw byte, pc_b[1]:i8/raw byte, pc_b[0]:i8/raw byte | same, 2-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3186](../../src/client/nclient.c#L3186) |
```

- Inventory line 206 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR` → `Receive_char` step 3 | `%c%c%c%c%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[2]:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte, a_back:i8/raw byte, pc_b[2]:i8/raw byte, pc_b[1]:i8/raw byte, pc_b[0]:i8/raw byte | same, 3-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3189](../../src/client/nclient.c#L3189) |
```

- Inventory line 207 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR` → `Receive_char` step 4 | `%c%c%c%c%u%c%u` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, c:u32be, a_back:i8/raw byte, c_back:u32be | same, >= 4.8.1 Unicode | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3193](../../src/client/nclient.c#L3193) |
```

- Inventory line 208 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR` → `Receive_char` step 5 | `%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[0]:i8/raw byte | non-2mask, 1-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3205](../../src/client/nclient.c#L3205) |
```

- Inventory line 209 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR` → `Receive_char` step 6 | `%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte | non-2mask, 2-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3208](../../src/client/nclient.c#L3208) |
```

- Inventory line 210 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR` → `Receive_char` step 7 | `%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[2]:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte | non-2mask, 3-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3211](../../src/client/nclient.c#L3211) |
```

- Inventory line 211 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR` → `Receive_char` step 8 | `%c%c%c%c%u` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, c:u32be | non-2mask + >= 4.8.1 Unicode | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3215](../../src/client/nclient.c#L3215) |
```

- Inventory line 212 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR` → `Receive_char` step 9 | `%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, c:i8/raw byte | legacy byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3218](../../src/client/nclient.c#L3218) |
```

- Inventory line 214 → `capability.world.read-movement-state`; obligations `.result` and `.lifecycle`.

```text
| `PKT_STATE` → `Receive_state` step 1 | `%c%hu%hu%hu` ⇒ ch:i8/raw byte, paralyzed:u16be, searching:u16be, resting:u16be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | activity state; latest snapshot; replace on packet, connection reset owns clear | all three fields are presentation-only and lost after `prt_state` ([handler](../../src/client/nclient.c#L3576)) | none | [src/client/nclient.c:3581](../../src/client/nclient.c#L3581) |
```

- Inventory line 216 → `capability.world.read-location`; obligations `.result` and `.lifecycle`.

```text
| `PKT_DEPTH` → `Receive_depth` step 1 | `%c%hu%hu%hu%c%c%c%s%s%s` ⇒ ch:i8/raw byte, x:u16be, y:u16be, z:u16be, town:i8/raw byte, colour:i8/raw byte, colour_sector:i8/raw byte, buf:NUL string<MAX_CHARS>, location_name2:NUL string<MAX_CHARS>, location_pre:NUL string<MAX_CHARS> | > 4.6.1.2.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | world position/location labels; latest snapshot; replace on packet, connection reset owns clear | retains position and several derived globals, but truncates one location label and loses exact wire/default provenance ([handler](../../src/client/nclient.c#L3624)) | none | [src/client/nclient.c:3634](../../src/client/nclient.c#L3634) |
```

- Inventory line 217 → `capability.world.read-location`; obligations `.result` and `.lifecycle`.

```text
| `PKT_DEPTH` → `Receive_depth` step 2 | `%c%hu%hu%hu%c%c%c%s%s` ⇒ ch:i8/raw byte, x:u16be, y:u16be, z:u16be, town:i8/raw byte, colour:i8/raw byte, colour_sector:i8/raw byte, buf:NUL string<MAX_CHARS>, location_name2:NUL string<MAX_CHARS> | > 4.5.9.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | world position/location labels; latest snapshot; replace on packet, connection reset owns clear | retains position and several derived globals, but truncates one location label and loses exact wire/default provenance ([handler](../../src/client/nclient.c#L3624)) | none | [src/client/nclient.c:3637](../../src/client/nclient.c#L3637) |
```

- Inventory line 218 → `capability.world.read-location`; obligations `.result` and `.lifecycle`.

```text
| `PKT_DEPTH` → `Receive_depth` step 3 | `%c%hu%hu%hu%c%c%c%s` ⇒ ch:i8/raw byte, x:u16be, y:u16be, z:u16be, town:i8/raw byte, colour:i8/raw byte, colour_sector:i8/raw byte, buf:NUL string<MAX_CHARS> | > 4.4.1.6.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | world position/location labels; latest snapshot; replace on packet, connection reset owns clear | retains position and several derived globals, but truncates one location label and loses exact wire/default provenance ([handler](../../src/client/nclient.c#L3624)) | none | [src/client/nclient.c:3642](../../src/client/nclient.c#L3642) |
```

- Inventory line 219 → `capability.world.read-location`; obligations `.result` and `.lifecycle`.

```text
| `PKT_DEPTH` → `Receive_depth` step 4 | `%c%hu%hu%hu%c%hu%s` ⇒ ch:i8/raw byte, x:u16be, y:u16be, z:u16be, town:i8/raw byte, old_colour:u16be, buf:NUL string<MAX_CHARS> | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | world position/location labels; latest snapshot; replace on packet, connection reset owns clear | retains position and several derived globals, but truncates one location label and loses exact wire/default provenance ([handler](../../src/client/nclient.c#L3624)) | none | [src/client/nclient.c:3647](../../src/client/nclient.c#L3647) |
```

- Inventory line 235 → `capability.direction.answer-server`, `capability.direction.cancel-server`; obligations `.result` and `.lifecycle`.

```text
| `PKT_DIRECTION` → `Receive_direction` step 1 | `%c` ⇒ ch:i8/raw byte | always | user cancellation returns 0 after packet consumption; busy UI queues request ([validation](../../src/client/nclient.c#L4018)) | direction request; modal request; replace pending, response/abort clears | none; retain decoded fields ([handler](../../src/client/nclient.c#L4012)) | `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:4016](../../src/client/nclient.c#L4016) |
```

- Inventory line 237 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 1 | `%c%hd` ⇒ ch:i8/raw byte, y:i16be | always | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4079](../../src/client/nclient.c#L4079) |
```

- Inventory line 238 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 2 | `%c%c%c%c` ⇒ pc[0]:i8/raw byte, a:i8/raw byte, pc_b[0]:i8/raw byte, a_back:i8/raw byte | PKT_LINE_INFO RLE + 2mask, 1-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4233](../../src/client/nclient.c#L4233) |
```

- Inventory line 239 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 3 | `%c%c%c%c%c%c` ⇒ pc[1]:i8/raw byte, pc[0]:i8/raw byte, a:i8/raw byte, pc_b[1]:i8/raw byte, pc_b[0]:i8/raw byte, a_back:i8/raw byte | same, 2-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4236](../../src/client/nclient.c#L4236) |
```

- Inventory line 240 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 4 | `%c%c%c%c%c%c%c%c` ⇒ pc[2]:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte, a:i8/raw byte, pc_b[2]:i8/raw byte, pc_b[1]:i8/raw byte, pc_b[0]:i8/raw byte, a_back:i8/raw byte | same, 3-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4239](../../src/client/nclient.c#L4239) |
```

- Inventory line 241 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 5 | `%u%c%u%c` ⇒ c:u32be, a:i8/raw byte, c_back:u32be, a_back:i8/raw byte | same, >= 4.8.1 Unicode | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4243](../../src/client/nclient.c#L4243) |
```

- Inventory line 242 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 6 | `%c%c` ⇒ pc[0]:i8/raw byte, a:i8/raw byte | PKT_LINE_INFO RLE, 1-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4259](../../src/client/nclient.c#L4259) |
```

- Inventory line 243 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 7 | `%c%c%c` ⇒ pc[1]:i8/raw byte, pc[0]:i8/raw byte, a:i8/raw byte | same, 2-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4262](../../src/client/nclient.c#L4262) |
```

- Inventory line 244 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 8 | `%c%c%c%c` ⇒ pc[2]:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte, a:i8/raw byte | same, 3-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4265](../../src/client/nclient.c#L4265) |
```

- Inventory line 245 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 9 | `%u%c` ⇒ c:u32be, a:i8/raw byte | same, >= 4.8.1 Unicode | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4269](../../src/client/nclient.c#L4269) |
```

- Inventory line 246 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 10 | `%c%c` ⇒ c:i8/raw byte, a:i8/raw byte | legacy cell | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4276](../../src/client/nclient.c#L4276) |
```

- Inventory line 247 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 11 | `%c%c` ⇒ a:i8/raw byte, rep:i8/raw byte | > 4.4.3.0.0.5 repeat attr+count | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4287](../../src/client/nclient.c#L4287) |
```

- Inventory line 248 → `capability.world.read-map`, `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_LINE_INFO` → `Receive_line_info` step 12 | `%c` ⇒ rep:i8/raw byte | older repeat count | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4302](../../src/client/nclient.c#L4302) |
```

- Inventory line 258 → `capability.target.read-description`, `capability.target.look`, `capability.target.select-hostile`; obligations `.result` and `.lifecycle`.

```text
| `PKT_TARGET_INFO` → `Receive_target_info` step 1 | `%c%c%c%S` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, buf:NUL string<MSG_LEN> | >= 4.9.0.1.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | target coordinates/description; latest snapshot; replace on packet, connection reset owns clear | passes fields into targeting display; retain coordinates and exact raw description explicitly ([handler](../../src/client/nclient.c#L5515)) | none | [src/client/nclient.c:5520](../../src/client/nclient.c#L5520) |
```

- Inventory line 259 → `capability.target.read-description`, `capability.target.look`, `capability.target.select-hostile`; obligations `.result` and `.lifecycle`.

```text
| `PKT_TARGET_INFO` → `Receive_target_info` step 2 | `%c%c%c%s` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, buf:NUL string<MAX_CHARS> | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | target coordinates/description; latest snapshot; replace on packet, connection reset owns clear | passes fields into targeting display; retain coordinates and exact raw description explicitly ([handler](../../src/client/nclient.c#L5515)) | none | [src/client/nclient.c:5522](../../src/client/nclient.c#L5522) |
```

- Inventory line 265 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 1 | `%c%hd` ⇒ ch:i8/raw byte, y:i16be | always | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4079](../../src/client/nclient.c#L4079) |
```

- Inventory line 266 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 2 | `%c%c%c%c` ⇒ pc[0]:i8/raw byte, a:i8/raw byte, pc_b[0]:i8/raw byte, a_back:i8/raw byte | PKT_LINE_INFO RLE + 2mask, 1-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4233](../../src/client/nclient.c#L4233) |
```

- Inventory line 267 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 3 | `%c%c%c%c%c%c` ⇒ pc[1]:i8/raw byte, pc[0]:i8/raw byte, a:i8/raw byte, pc_b[1]:i8/raw byte, pc_b[0]:i8/raw byte, a_back:i8/raw byte | same, 2-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4236](../../src/client/nclient.c#L4236) |
```

- Inventory line 268 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 4 | `%c%c%c%c%c%c%c%c` ⇒ pc[2]:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte, a:i8/raw byte, pc_b[2]:i8/raw byte, pc_b[1]:i8/raw byte, pc_b[0]:i8/raw byte, a_back:i8/raw byte | same, 3-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4239](../../src/client/nclient.c#L4239) |
```

- Inventory line 269 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 5 | `%u%c%u%c` ⇒ c:u32be, a:i8/raw byte, c_back:u32be, a_back:i8/raw byte | same, >= 4.8.1 Unicode | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4243](../../src/client/nclient.c#L4243) |
```

- Inventory line 270 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 6 | `%c%c` ⇒ pc[0]:i8/raw byte, a:i8/raw byte | PKT_LINE_INFO RLE, 1-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4259](../../src/client/nclient.c#L4259) |
```

- Inventory line 271 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 7 | `%c%c%c` ⇒ pc[1]:i8/raw byte, pc[0]:i8/raw byte, a:i8/raw byte | same, 2-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4262](../../src/client/nclient.c#L4262) |
```

- Inventory line 272 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 8 | `%c%c%c%c` ⇒ pc[2]:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte, a:i8/raw byte | same, 3-byte glyph | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4265](../../src/client/nclient.c#L4265) |
```

- Inventory line 273 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 9 | `%u%c` ⇒ c:u32be, a:i8/raw byte | same, >= 4.8.1 Unicode | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4269](../../src/client/nclient.c#L4269) |
```

- Inventory line 274 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 10 | `%c%c` ⇒ c:i8/raw byte, a:i8/raw byte | legacy cell | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4276](../../src/client/nclient.c#L4276) |
```

- Inventory line 275 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 11 | `%c%c` ⇒ a:i8/raw byte, rep:i8/raw byte | > 4.4.3.0.0.5 repeat attr+count | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4287](../../src/client/nclient.c#L4287) |
```

- Inventory line 276 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP` → `Receive_line_info` step 12 | `%c` ⇒ rep:i8/raw byte | older repeat count | any incomplete nested cell/RLE record resets `rbuf.ptr` to packet start and returns 0 ([validation](../../src/client/nclient.c#L4400)) | map/minimap row cells; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized panel-map cells for gameplay rows; RLE, exact glyph byte form and minimap-only provenance are lost ([handler](../../src/client/nclient.c#L4064)) | none | [src/client/nclient.c:4302](../../src/client/nclient.c#L4302) |
```

- Inventory line 346 → `capability.map.view-overview`, `capability.map.select-sector`, `capability.map.cancel-sector`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP_POS` → `Receive_mini_map_pos` step 1 | `%c%hd%hd%hd%c%u` ⇒ ch:i8/raw byte, x:i16be, y:i16be, y_offset:i16be, a:i8/raw byte, c:u32be | >= 4.8.1.2.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | minimap cursor/world position; latest snapshot; replace on packet, connection reset owns clear | position fields affect globals/drawing; preserve exact position event ([handler](../../src/client/nclient.c#L4462)) | none | [src/client/nclient.c:4470](../../src/client/nclient.c#L4470) |
```

- Inventory line 347 → `capability.map.view-overview`, `capability.map.select-sector`, `capability.map.cancel-sector`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP_POS` → `Receive_mini_map_pos` step 2 | `%c%hd%hd%c%u` ⇒ ch:i8/raw byte, x:i16be, y:i16be, a:i8/raw byte, c:u32be | >= 4.8.1.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | minimap cursor/world position; latest snapshot; replace on packet, connection reset owns clear | position fields affect globals/drawing; preserve exact position event ([handler](../../src/client/nclient.c#L4462)) | none | [src/client/nclient.c:4473](../../src/client/nclient.c#L4473) |
```

- Inventory line 348 → `capability.map.view-overview`, `capability.map.select-sector`, `capability.map.cancel-sector`; obligations `.result` and `.lifecycle`.

```text
| `PKT_MINI_MAP_POS` → `Receive_mini_map_pos` step 3 | `%c%hd%hd%c%c` ⇒ ch:i8/raw byte, x:i16be, y:i16be, a:i8/raw byte, c:i8/raw byte | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | minimap cursor/world position; latest snapshot; replace on packet, connection reset owns clear | position fields affect globals/drawing; preserve exact position event ([handler](../../src/client/nclient.c#L4462)) | none | [src/client/nclient.c:4475](../../src/client/nclient.c#L4475) |
```

- Inventory line 383 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_DIRECT` → `Receive_char` step 1 | `%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[0]:i8/raw byte, a_back:i8/raw byte, pc_b[0]:i8/raw byte | GRAPHICS_BG_MASK + UG_2MASK + >= 4.9.2.1 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3183](../../src/client/nclient.c#L3183) |
```

- Inventory line 384 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_DIRECT` → `Receive_char` step 2 | `%c%c%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte, a_back:i8/raw byte, pc_b[1]:i8/raw byte, pc_b[0]:i8/raw byte | same, 2-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3186](../../src/client/nclient.c#L3186) |
```

- Inventory line 385 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_DIRECT` → `Receive_char` step 3 | `%c%c%c%c%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[2]:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte, a_back:i8/raw byte, pc_b[2]:i8/raw byte, pc_b[1]:i8/raw byte, pc_b[0]:i8/raw byte | same, 3-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3189](../../src/client/nclient.c#L3189) |
```

- Inventory line 386 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_DIRECT` → `Receive_char` step 4 | `%c%c%c%c%u%c%u` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, c:u32be, a_back:i8/raw byte, c_back:u32be | same, >= 4.8.1 Unicode | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3193](../../src/client/nclient.c#L3193) |
```

- Inventory line 387 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_DIRECT` → `Receive_char` step 5 | `%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[0]:i8/raw byte | non-2mask, 1-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3205](../../src/client/nclient.c#L3205) |
```

- Inventory line 388 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_DIRECT` → `Receive_char` step 6 | `%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte | non-2mask, 2-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3208](../../src/client/nclient.c#L3208) |
```

- Inventory line 389 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_DIRECT` → `Receive_char` step 7 | `%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, pc[2]:i8/raw byte, pc[1]:i8/raw byte, pc[0]:i8/raw byte | non-2mask, 3-byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3211](../../src/client/nclient.c#L3211) |
```

- Inventory line 390 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_DIRECT` → `Receive_char` step 8 | `%c%c%c%c%u` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, c:u32be | non-2mask + >= 4.8.1 Unicode | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3215](../../src/client/nclient.c#L3215) |
```

- Inventory line 391 → `capability.world.read-map`; obligations `.result` and `.lifecycle`.

```text
| `PKT_CHAR_DIRECT` → `Receive_char` step 9 | `%c%c%c%c%c` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, a:i8/raw byte, c:i8/raw byte | legacy byte glyph | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | map cell; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | does not retain the addressed cell here; decoded coordinates/attrs/glyph layers are lost after `Term_draw*` ([handler](../../src/client/nclient.c#L3162)) | none | [src/client/nclient.c:3218](../../src/client/nclient.c#L3218) |
```

- Inventory line 404 → `capability.map.view-overview`, `capability.map.pan-overview`; obligations `.result` and `.lifecycle`.

```text
| minimap rows | `PKT_MINI_MAP` with `y == -1` commits/end-marks the transmission; late rows after leaving local map are consumed without drawing. | minimap batch; [src/client/nclient.c:4089](../../src/client/nclient.c#L4089) |
```

- Inventory line 462 → `capability.session.portal-relogin`; obligations `.result` and `.lifecycle`.

```text
| `SERVER_PORTALS` | Adds `PKT_RELOGIN`; model reconnect target/credentials/reason/delay as privileged transient control, never presentation text. | [src/client/nclient.c:345](../../src/client/nclient.c#L345), [src/client/nclient.c:2051](../../src/client/nclient.c#L2051) |
```

- Inventory line 489 → `capability.account.authenticate`; obligations `.result` and `.lifecycle`.

```text
| 1 / `PKT_VERIFY` | client→server only | live server `Receive_discard` binding | [pack.h:23](../../src/common/pack.h#L23); [nserver.c:323](../../src/server/nserver.c#L323) |
```

- Inventory line 491 → `capability.session.enter-game`, `capability.birth.complete`; obligations `.result` and `.lifecycle`.

```text
| 3 / `PKT_PLAY` | client→server only | live server `Receive_play` binding | [pack.h:25](../../src/common/pack.h#L25); [nserver.c:325](../../src/server/nserver.c#L325) |
```

- Inventory line 501 → `capability.world.walk`; obligations `.result` and `.lifecycle`.

```text
| 70 / `PKT_WALK` | client→server only | live server `Receive_walk` binding | [pack.h:99](../../src/common/pack.h#L99); [nserver.c:329](../../src/server/nserver.c#L329) |
```

- Inventory line 502 → `capability.world.run`; obligations `.result` and `.lifecycle`.

```text
| 71 / `PKT_RUN` | client→server only | live server `Receive_run` binding | [pack.h:100](../../src/common/pack.h#L100); [nserver.c:330](../../src/server/nserver.c#L330) |
```

- Inventory line 503 → `capability.world.tunnel`; obligations `.result` and `.lifecycle`.

```text
| 72 / `PKT_TUNNEL` | client→server only | live server `Receive_tunnel` binding | [pack.h:101](../../src/common/pack.h#L101); [nserver.c:331](../../src/server/nserver.c#L331) |
```

- Inventory line 509 → `capability.target.look`, `capability.target.look-position`, `capability.target.close-look`; obligations `.result` and `.lifecycle`.

```text
| 78 / `PKT_LOOK` | client→server only | live server `Receive_look` binding | [pack.h:107](../../src/common/pack.h#L107); [nserver.c:339](../../src/server/nserver.c#L339) |
```

- Inventory line 510 → `capability.world.open`; obligations `.result` and `.lifecycle`.

```text
| 80 / `PKT_OPEN` | client→server only | live server `Receive_open` binding | [pack.h:110](../../src/common/pack.h#L110); [nserver.c:341](../../src/server/nserver.c#L341) |
```

- Inventory line 514 → `capability.world.search`; obligations `.result` and `.lifecycle`.

```text
| 84 / `PKT_SEARCH` | client→server only | live server `Receive_search` binding | [pack.h:114](../../src/common/pack.h#L114); [nserver.c:344](../../src/server/nserver.c#L344) |
```

- Inventory line 520 → `capability.target.select-hostile`, `capability.target.select-position`, `capability.target.cancel`; obligations `.result` and `.lifecycle`.

```text
| 90 / `PKT_TARGET` | client→server only | live server `Receive_target` binding | [pack.h:121](../../src/common/pack.h#L121); [nserver.c:355](../../src/server/nserver.c#L355) |
```

- Inventory line 524 → `capability.world.bash`; obligations `.result` and `.lifecycle`.

```text
| 94 / `PKT_BASH` | client→server only | live server `Receive_bash` binding | [pack.h:125](../../src/common/pack.h#L125); [nserver.c:362](../../src/server/nserver.c#L362) |
```

- Inventory line 525 → `capability.world.disarm`; obligations `.result` and `.lifecycle`.

```text
| 95 / `PKT_DISARM` | client→server only | live server `Receive_disarm` binding | [pack.h:126](../../src/common/pack.h#L126); [nserver.c:363](../../src/server/nserver.c#L363) |
```

- Inventory line 528 → `capability.map.locate`, `capability.map.close-locate`; obligations `.result` and `.lifecycle`.

```text
| 98 / `PKT_LOCATE` | client→server only | live server `Receive_locate` binding | [pack.h:129](../../src/common/pack.h#L129); [nserver.c:366](../../src/server/nserver.c#L366) |
```

- Inventory line 529 → `capability.map.view-overview`; obligations `.result` and `.lifecycle`.

```text
| 99 / `PKT_MAP` | client→server only | live server `Receive_map` binding | [pack.h:130](../../src/common/pack.h#L130); [nserver.c:367](../../src/server/nserver.c#L367) |
```

- Inventory line 532 → `capability.world.close`; obligations `.result` and `.lifecycle`.

```text
| 103 / `PKT_CLOSE` | client→server only | live server `Receive_close` binding | [pack.h:134](../../src/common/pack.h#L134); [nserver.c:370](../../src/server/nserver.c#L370) |
```

- Inventory line 534 → `capability.world.ascend`; obligations `.result` and `.lifecycle`.

```text
| 105 / `PKT_GO_UP` | client→server only | live server `Receive_go_up` binding | [pack.h:136](../../src/common/pack.h#L136); [nserver.c:373](../../src/server/nserver.c#L373) |
```

- Inventory line 535 → `capability.world.descend`; obligations `.result` and `.lifecycle`.

```text
| 106 / `PKT_GO_DOWN` | client→server only | live server `Receive_go_down` binding | [pack.h:137](../../src/common/pack.h#L137); [nserver.c:374](../../src/server/nserver.c#L374) |
```

- Inventory line 540 → `capability.world.rest`; obligations `.result` and `.lifecycle`.

```text
| 112 / `PKT_REST` | client→server only | live server `Receive_rest` binding | [pack.h:144](../../src/common/pack.h#L144); [nserver.c:386](../../src/server/nserver.c#L386) |
```

- Inventory line 542 → `capability.session.suicide`, `capability.session.cancel-suicide`; obligations `.result` and `.lifecycle`.

```text
| 114 / `PKT_SUICIDE` | client→server only | live server `Receive_suicide` binding | [pack.h:146](../../src/common/pack.h#L146); [nserver.c:393](../../src/server/nserver.c#L393) |
```

- Inventory line 545 → `capability.target.select-friendly`; obligations `.result` and `.lifecycle`.

```text
| 117 / `PKT_TARGET_FRIENDLY` | client→server only | live server `Receive_target_friendly` binding | [pack.h:149](../../src/common/pack.h#L149); [nserver.c:356](../../src/server/nserver.c#L356) |
```

- Inventory line 551 → `capability.world.clear-command-buffer`; obligations `.result` and `.lifecycle`.

```text
| 123 / `PKT_CLEAR_BUFFER` | client→server only | live server `Receive_clear_buffer` binding | [pack.h:158](../../src/common/pack.h#L158); [nserver.c:400](../../src/server/nserver.c#L400) |
```

- Inventory line 568 → `capability.character.read-overview`; obligations `.result` and `.lifecycle`.

```text
| 162 / `PKT_SERVERDETAILS` | server→client direct handshake/login | login emits `%c%d%d%d%d`; `Receive_login`/reorder consume it outside play dispatch | [pack.h:210](../../src/common/pack.h#L210); [nserver.c:5317](../../src/server/nserver.c#L5317), [nclient.c:809](../../src/client/nclient.c#L809) |
```

- Inventory line 574 → `capability.account.change-password`, `capability.account.password-change-failure`; obligations `.result` and `.lifecycle`.

```text
| 180 / `PKT_CHANGE_PASSWORD` | client→server only | live server `Receive_change_password` binding | [pack.h:232](../../src/common/pack.h#L232); [nserver.c:424](../../src/server/nserver.c#L424) |
```

Direct handshake paths are not play-dispatch omissions: `Net_verify`, `Net_setup`,
`Net_login`/`Receive_login`, `Net_start` and `PKT_SERVERDETAILS` map to contact,
authentication, overview and enter-game obligations. Character list versions are
>4.5.7 (location), >4.4.9.2 (mode), older; reordering requires >=4.7.3 **and**
`allow_reordering` (only total slot counts <=15 in current source). Capacity and
mode restrictions retain >4.5.8.1 and 4.9.0.5 boundaries. Login initial address
suffix requires >=4.9.2.1.0.2. Empty choice requests overview; `***` + three bytes
requests reorder; selected name is a separate packet. Verify every timeout/refusal
branch, empty-list terminator and refreshed list before accepting these outcomes.
`Net_start` also negotiates options table sizes, glyph tables, resource names and
screen dimensions: `birth.complete` lists thresholds and `session.load-profile-input`
owns the corresponding B dependencies. Server `Receive_login`/`Receive_play`
validation remains authoritative; outbound bytes alone do not establish success.

`PKT_FLUSH`, `PKT_INKEY`, keepalive/ping, frame markers and setup/version replies
are support/control scenarios of session/input owners, not user outcomes per
packet. `PKT_FLOOR` intentionally discards tval in baseline (inventory pickup
semantics belong to 09). `PKT_HEALTH`, remaining HUD/status, weather/palette and
geometry feedback are 10/11 cross-domain B map/target prerequisites. Dead or
server-only directional entries are not to be installed as client receivers.

## Slash dispatch

The local slash inventory has no login, movement, target, reorder or password-change
verb. `cmd_message` sends unrecognized slash text through `Send_msg`; server-owned
`/quit` and related session controls must use that route and its chat byte/history
contract, not a new local parser. `Receive_quit` distinguishes literal `client quit`
for RETRY_LOGIN. Account information is the actual account-options menu, not an
invented `/password` local command.

`/opty`, `/optn`, `/optt` and verbose variants affecting movement, `big_map` or input
options are `session.load-profile-input`/`world.read-map` scenario dependencies;
full setting outcomes stay with 11. `/apickup`, `/adestroy`, `/daunmatched`, `/new`,
`/aai` are item/automation outcomes (09/11), not movement success. `/know`, guide
bookmarks, local clock/version, local Lua, wager, screenshots and audio reload
remain 10/11/12. Unknown/raw command transmission is input-dispatch parity, not
proof that arbitrary server commands succeeded. Shipped `<`/`>` escape-prefixed
macros, Ctrl-Q, keypad/navigation/Shift/Ctrl direction families and both keysets
must be exercised through production input for each affected owner. Macro editing
and arbitrary user macro contents are not enumerated by this ticket.

## Text and byte boundaries

The following rows preserve producer/editor/wire/server distinctions. The resolved
policy additionally requires fitting-prefix paste/macro/default insertion,
encoding-error draft retention, bounded whole-packet enqueue, NUL within each
slot and protocol failure without partial apply. Private credential transforms
never normalize bytes; star rejection is protocol-dependent. Imported/CLI/server
credential producers are validated even when they bypass an interactive editor.

### Field registry and atomic producer/consumer call sites

Original: `.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md`; SHA-256 `71585ae2b09367c05ef9ba4b25d24db218fd6f5f430f19231e7efae1d6da0fc3`.

- Inventory line 36 → `capability.connection.enter-host`, `capability.session.portal-relogin`, `capability.account.restore-secret`; obligations `.result` and `.lifecycle`.

```text
| Config, argv, environment, metaserver/host and relogin fields | Config/CLI/network/environment bytes → identity/connection/path consumers. Windows command parser accepts different lengths from interactive editor; relogin includes credential bytes and reason | Config/INI writers and process connection state; no actual values inspected; [client.c:129](../../../src/client/client.c#L129), [main-win.c:5997](../../../src/client/main-win.c#L5997), [nclient.c:2063](../../../src/client/nclient.c#L2063) |
```

- Inventory line 45 → `capability.account.enter-name`, `capability.account.login-rejected`; obligations `.result` and `.lifecycle`.

```text
| Account name | Birth E=15, private config/CLI can differ; `%s` 80. Live trim on interactive insertion. Enter_player `validstring` rejects bytes outside 32..122; Check_names requires first A..Z, length minimum and no colon. Account name also passes Trim_name in login | Cancel retains/retries birth owner, login failures disconnect/error. Account database owns identity; [c-birth.c:212](../../../src/client/c-birth.c#L212), [nserver.c:1377](../../../src/server/nserver.c#L1377), [2343](../../../src/server/nserver.c#L2343), [5126](../../../src/server/nserver.c#L5126) |
```

- Inventory line 46 → `capability.character.name`, `capability.character.name-rejected`, `capability.character.swap`; obligations `.result` and `.lifecycle`.

```text
| Character name / selection / reorder | E=15, CNAME_LEN16; `%s`80. `+` reincarnation prefix special; reorder `***` + three command bytes; server clips character choice at15, Trim_name replaces disallowed bytes with `_`, strips trailing whitespace, fixes existing name case, then validates ownership/forbidden names/punctuation | Name cancellation returns overview; empty login is overview request, not character name. Server saves character identity; [nclient.c:744](../../../src/client/nclient.c#L744), [1189](../../../src/client/nclient.c#L1189), [1595](../../../src/client/nclient.c#L1595), [nserver.c:5390](../../../src/server/nserver.c#L5390). This closes the prior audit's uncertainty about non-first high bytes: Trim_name transforms them, it does not admit them as raw name bytes |
```

- Inventory line 47 → `capability.account.enter-password`, `capability.account.reject-unencodable-password`; obligations `.result` and `.lifecycle`.

```text
| Login password | E=15 private, `%s`80; config/CLI/relogin separate producers. `my_memfrob` XOR42 only when server_protocol>=2; NUL-sensitive strlen/C-string transport; no normalization | Server credential verification/hash; account/config persistence path separate, no secret logged here. Private mode does not reject printable `*`; XOR42 maps it to NUL, so transformed C-string boundaries require explicit review; [c-birth.c:287](../../../src/client/c-birth.c#L287), [c-init.c:3824](../../../src/client/c-init.c#L3824), [c-util.c:17959](../../../src/client/c-util.c#L17959), [nclient.c:1430](../../../src/client/nclient.c#L1430) |
```

- Inventory line 48 → `capability.account.change-password`, `capability.account.cancel-password-change`, `capability.account.password-change-failure`; obligations `.result` and `.lifecycle`.

```text
| Old password / new password / confirmation | Each E=15 private; repeat only local. Old/new obfuscated `%s`80 each; server deobfuscates then GetAccount, strlen(new)>=6 and <=ACCFILE_PASSWD_LEN20, hashes and writes | Empty old rejected; mismatched repeat retries; Esc no send. Server messages distinguish wrong password/length/write failure/success; server wipes temporary buffers. [c-util.c:13106](../../../src/client/c-util.c#L13106), [nserver.c:16047](../../../src/server/nserver.c#L16047), [party.c:6059](../../../src/server/party.c#L6059) |
```

- Inventory line 49 → `capability.connection.contact`, `capability.connection.contact-failure`; obligations `.result` and `.lifecycle`.

```text
| Real name / host name / initial account handshake | OS/config producers; `%s`80 slots in initial ibuf and verification. Real/host nonempty check; `validstrings` vs `validstring` branch must not be collapsed | Connection metadata, not editable character labels; [c-init.c:4128](../../../src/client/c-init.c#L4128), [nserver.c:1013](../../../src/server/nserver.c#L1013), [2318](../../../src/server/nserver.c#L2318) |
```

- Inventory line 127 → `capability.account.enter-name`, `capability.account.cancel-name`; obligations `.result` and `.lifecycle`.

```text
| [c-birth.c:212](../../../src/client/c-birth.c#L212) | `choose_name`; delegate | `if (askfor_aux(tmp, ACCNAME_LEN - 1, ASKFOR_LIVETRIM)) strcpy(nick, tmp);` |
```

- Inventory line 128 → `capability.account.enter-password`, `capability.account.cancel-password`; obligations `.result` and `.lifecycle`.

```text
| [c-birth.c:287](../../../src/client/c-birth.c#L287) | `enter_password`; delegate | `if (askfor_aux(tmp, PASSWORD_LEN - 1, ASKFOR_PRIVATE)) strcpy(pass, tmp);` |
```

- Inventory line 129 → `capability.connection.enter-host`, `capability.connection.cancel-host`; obligations `.result` and `.lifecycle`.

```text
| [c-birth.c:2388](../../../src/client/c-birth.c#L2388) | `enter_server_name`; delegate | `return(askfor_aux(server_name, 79, 0));` |
```

- Inventory line 247 → `capability.account.change-password`, `capability.account.cancel-password-change`; obligations `.result` and `.lifecycle`.

```text
| [c-util.c:13106](../../../src/client/c-util.c#L13106) | `do_cmd_options_acc`; delegate | `if (!askfor_aux(tmp, PASSWORD_LEN - 1, ASKFOR_PRIVATE)) {` |
```

- Inventory line 248 → `capability.account.change-password`, `capability.account.cancel-password-change`; obligations `.result` and `.lifecycle`.

```text
| [c-util.c:13123](../../../src/client/c-util.c#L13123) | `do_cmd_options_acc`; delegate | `if (!askfor_aux(tmp, PASSWORD_LEN - 1, ASKFOR_PRIVATE)) {` |
```

- Inventory line 249 → `capability.account.change-password`, `capability.account.cancel-password-change`; obligations `.result` and `.lifecycle`.

```text
| [c-util.c:13134](../../../src/client/c-util.c#L13134) | `do_cmd_options_acc`; delegate | `if (!askfor_aux(tmp, PASSWORD_LEN - 1, ASKFOR_PRIVATE)) {` |
```

- Inventory line 284 → `capability.character.read-overview`, `capability.character.name`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1189](../../../src/client/nclient.c#L1189) | `Receive_login`; delegate | `if (!askfor_aux(c_name, CNAME_LEN - 1, ASKFOR_LIVETRIM \| ASKFOR_PLUSPREFIX)) {` |
```

- Inventory line 320 → `capability.character.swap`, `capability.character.insert-before`, `capability.character.append-after`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:744](../../../src/client/nclient.c#L744) | `reorder_characters`; outbound | `Packet_printf(&wbuf, "%c%s", PKT_LOGIN, format("***%c%c%c", sortA, sortB, mode))` |
```

- Inventory line 321 → `capability.character.read-overview`, `capability.character.name`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:998](../../../src/client/nclient.c#L998) | `Receive_login`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%hd%hd%s", &ch, &mode, colour_sequence, c_name, &level, &c_race, &c_class, loc)` |
```

- Inventory line 322 → `capability.character.read-overview`, `capability.character.name`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1000](../../../src/client/nclient.c#L1000) | `Receive_login`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%hd%hd", &ch, &mode, colour_sequence, c_name, &level, &c_race, &c_class)` |
```

- Inventory line 323 → `capability.character.read-overview`, `capability.character.name`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1001](../../../src/client/nclient.c#L1001) | `Receive_login`; inbound | `Packet_scanf(&rbuf, "%c%s%s%hd%hd%hd", &ch, colour_sequence, c_name, &level, &c_race, &c_class)` |
```

- Inventory line 324 → `capability.character.read-overview`, `capability.character.name`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1003](../../../src/client/nclient.c#L1003) | `Receive_login`; inbound; commented alternative | `Packet_scanf(&rbuf, "%c%s%s%hd%hd%hd", &ch, colour_sequence, c_name, &level, &c_race, &c_class)` |
```

- Inventory line 325 → `capability.connection.contact`, `capability.birth.race`, `capability.birth.trait`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1327](../../../src/client/nclient.c#L1327) | `Net_setup`; inbound | `Packet_scanf(&cbuf, "%c%c%c%c%c%c%s%d", &b1, &b2, &b3, &b4, &b5, &b6, str, &race_info[i].choice)` |
```

- Inventory line 326 → `capability.connection.contact`, `capability.birth.race`, `capability.birth.trait`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1338](../../../src/client/nclient.c#L1338) | `Net_setup`; inbound | `Packet_scanf(&cbuf, "%c%c%c%c%c%c%s", &b1, &b2, &b3, &b4, &b5, &b6, str)` |
```

- Inventory line 327 → `capability.connection.contact`, `capability.birth.race`, `capability.birth.trait`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1363](../../../src/client/nclient.c#L1363) | `Net_setup`; inbound | `Packet_scanf(&cbuf, "%s%d", str, &trait_info[i].choice)` |
```

- Inventory line 328 → `capability.account.authenticate`, `capability.account.reject-unencodable-password`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1430](../../../src/client/nclient.c#L1430) | `Net_verify`; outbound | `Packet_printf(&wbuf, "%c%s%s%s", PKT_VERIFY, real, nick, pass)` |
```

- Inventory line 329 → `capability.character.read-overview`, `capability.character.select-existing`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1595](../../../src/client/nclient.c#L1595) | `Net_login`; outbound | `Packet_printf(&wbuf, "%c%s", PKT_LOGIN, "")` |
```

- Inventory line 330 → `capability.character.read-overview`, `capability.character.select-existing`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1618](../../../src/client/nclient.c#L1618) | `Net_login`; outbound | `Packet_printf(&wbuf, "%c%s", PKT_LOGIN, cname)` |
```

- Inventory line 331 → `capability.birth.complete`, `capability.session.enter-game`, `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1691](../../../src/client/nclient.c#L1691) | `Net_start`; outbound | `Packet_printf(&wbuf, "%hd%hd%hd%hd%hd%hd%hd%s%s", sex, race, class, trait, audio_sfx, audio_music, use_graphics, graphic_tiles, fname)` |
```

- Inventory line 332 → `capability.birth.complete`, `capability.session.enter-game`, `capability.session.load-profile-input`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:1693](../../../src/client/nclient.c#L1693) | `Net_start`; outbound | `Packet_printf(&wbuf, "%hd%hd%hd%hd%hd%hd%hd%s%s", sex, race, class, trait, audio_sfx, audio_music, use_graphics, "NO_GRAPHICS", fname)` |
```

- Inventory line 334 → `capability.session.portal-relogin`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:2063](../../../src/client/nclient.c#L2063) | `Receive_relogin`; inbound | `Packet_scanf(&rbuf, "%c%s%d%s%s%s%s%c", &pkt, relogin_host, &relogin_port, relogin_accname, relogin_accpass, relogin_charname, reason, &delay)` |
```

- Inventory line 383 → `capability.target.read-description`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:5520](../../../src/client/nclient.c#L5520) | `Receive_target_info`; inbound | `Packet_scanf(&rbuf, "%c%c%c%S", &ch, &x, &y, buf)` |
```

- Inventory line 384 → `capability.target.read-description`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:5522](../../../src/client/nclient.c#L5522) | `Receive_target_info`; inbound | `Packet_scanf(&rbuf, "%c%c%c%s", &ch, &x, &y, buf)` |
```

- Inventory line 418 → `capability.account.change-password`; obligations `.result` and `.lifecycle`.

```text
| [nclient.c:8158](../../../src/client/nclient.c#L8158) | `Send_change_password`; outbound | `Packet_printf(&wbuf, "%c%s%s", PKT_CHANGE_PASSWORD, old_pass, new_pass)` |
```

- Inventory line 444 → `capability.character.name-rejected`, `capability.account.login-rejected`; obligations `.result` and `.lifecycle`.

```text
| [nserver.c:4987](../../../src/server/nserver.c#L4987) | `Receive_login` | `Packet_scanf(&connp->r, "%s", choice)` |
```

- Inventory line 457 → `capability.account.password-change-failure`; obligations `.result` and `.lifecycle`.

```text
| [nserver.c:16054](../../../src/server/nserver.c#L16054) | `Receive_change_password` | `Packet_scanf(&connp->r, "%c%s%s", &ch, old_pass, new_pass)` |
```

## Validation and limits

Run from repository root (Python with `jsonschema` installed):

```sh
python3 tools/validate_capabilities.py --manifest docs/capabilities/manifest.json --ledger docs/capabilities/native-coverage.json --source-root tomenet=.
python3 tests/sv_capabilities_checks.py
```

For historical retention, export the prior manifest with `git show
0ef6b362c:docs/capabilities/manifest.json` and pass `--previous-manifest` pointing
to that file. This checks all old IDs and source bytes; it does not certify the
semantic completeness of this audit or execute any live scenario. Review the
annex when source inventories change; snapshot counts are not timeless assertions.

## Verification — 2026-09-22

Source and history validation against `0ef6b362c` passed: 113 active outcomes,
113 pending evidence rows, zero accepted outcomes. All original ticket 07 source,
entity and relation records are unchanged. The registry suite passed all 12 tests,
including the 32 published negative cases. The canonical-count check now reads
the current active/pending denominator instead of freezing the original four-row
slice. Python compilation and `git diff --check` passed.

The full existing Linux regression passed 22/22 runner invocations: registry,
four sanitizer suites, legacy HP, and eight native suites on each of software
and OpenGL. Sandbox initially blocked LeakSanitizer's thread inspection and SDL
display access; rerunning outside the sandbox passed without disabling sanitizers
or replacing native rendering. After the MOTD data correction, the registry suite
and source/history validation passed again. Reproduce the full run:

```sh
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

No new game implementation or legacy/common source change was needed. These
regressions exercise the existing synthetic foundation, not the future live
outcomes added here. Windows/Wine, live servers and human visual acceptance were
not exercised for this data-only ticket. Source auditing and passing validation
do not constitute Stage A or B acceptance.

### Standards

Independent review against starting commit `0ef6b362c` found no documented
standard violations or actionable smell findings. Existing SV isolation, stable
IDs and pending evidence rules are preserved.

### Spec

Independent review found one incorrect MOTD restoration description: MOTD runs
before `Net_start`, and `RETRY_LOGIN` skips it after initial presentation. The
capability/action, scope, conditions and evidence obligation were corrected;
follow-up review verified the fix. No unresolved findings.

Final review: Standards 0; Spec 0 unresolved (one finding corrected).
