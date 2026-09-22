# Ticket 09: items, combat, spells and ordinary stores
This source-backed allocation extends ticket 08. All new native coverage and evidence remain **pending**. The task is `.scratch/single-window-sdl3-client-stage-a/issues/09-register-item-combat-spell-and-store-outcomes.md`; starting revision `bbe7417e21d20470a7aeb075aa82650608f6e4ae`. No gameplay implementation, synthetic acceptance import or baseline change is included.
## Authority and scope
Current C/Lua sources define behavior and protocol; [item-policy.md](item-policy.md) snapshots approved acceptance/UX decisions, and [session-policy.md](session-policy.md) retains approved field-byte policy. Inventory snapshots below retain original paths, line numbers and SHA-256. Links inside quoted rows use the original document base. These are discovery records, not evidence of native execution. Source drift wins over stale inventory wording.
An outcome is a complete command or observable flow, not one packet field. `.result`, `.lifecycle`, `.wire` (when present) and `.input` obligations all apply. Inventory rows mapping several outcomes require separate caller scenarios; a single selector test never accepts all mapped outcomes.
## Allocation and dependencies
B includes read-only inventory/equipment/bags for final review, underfoot state and pickup confirmation for stay, and ordinary store entry/read/leave/kick reached by movement/open. Incoming autoinscription is B because receiving inventory can trigger it; required rule loading arrives with the existing session.load-profile-input flow, while editing/persistence menus remain owned by 11. Full game actions are C. Item details read/navigation/search/close are explicitly C prerequisites of inspect/examine; a D generic document is not silently substituted.
Existing `direction.*`, `target.*`, `request.*`, `world.*` and session IDs are reused. The old session.review-final-state and movement ledger rows now depend on the B children above. Ghost powers are C and do not follow from accepting death. No capability is marked accepted by this allocation.
| Future owner | Required handoff (not exclusion) |
| --- | --- |
| 10 | Reuse item-details IDs for item perusal; add other documents, party/housing and special-store canvas/animation outcomes. Read-only character/history children remain B. Chat invoked from store/skills/books must restore that caller and its queue. Generic amount/number/string/confirmation requests and specific item-order/mail consumers are owned here; a service claiming such a child brings it to C before acceptance. |
| 11 | Autoinscription rule editor/load/save/import and options/macros/files are E except the actual loaded rule/resource prerequisites of B/C. Book Lua data, monster-name lookup files, clipboard and capture routes arrive with any earlier caller that needs them. |
| 12 | Reconcile remaining whole-client inventories, full server slash grammar and raw keys. This client-side slash inventory does not enumerate arbitrary server commands or user Lua programs. |
| 13 | Validate scoped production-path evidence, fingerprints and pending-child/fallback rejection; no synthetic key request certifies item/spell cancellation. |

## Source corrections and required edge scenarios
- Drop and verified takeoff can send quantity zero; destroy and normal purchase abort on zero. Sell/home deposit can send zero; home gold sends item marker 9999 even with zero. TV_GOLD purchase has its own zero-send path. Do not normalize these into one cancellation policy.
- Store quantity wire becomes i32 at >=4.9.3.0.0.3, previously i16. Bag move/remove requires server >4.7.4.4.0.0; before 4.9.2.0.0.0 amount is omitted and the whole stack moves. Preserve encoded container identity.
- Inventory revision is an internal ordering/ack contract: Receive_inventory_revision stores the revision and sends Send_inventory_revision. Its old autoinscription block is inactive. It is a scenario on item identity, not a user capability per counter.
- Server item/spell requests defer while busy, but item has the bag-chemical exception. Escape consumes without reply. They do not set request_pending: generic REQUEST_ABORT must not be invented as their cancellation packet; the abort handler only marks a pending generic request. Disconnect and store kick still invalidate the owner/session.
- School Lua extra false is aux=0; canceled Lua item selection retains item_obj=-1 and still sends. Only book/spell/required direction cancel aborts the whole cast. Mimic form editor ignores the get_string boolean; breath name Escape stays in its selector, immunity/technique name Escape exits.
- Legacy spell uppercase is index+64; school selection uppercase describes; school browsing uppercase pastes to chat; item uppercase verifies or encodes CAPS_ALT according to caller. Preserve each meaning.
- Store input does not pass through gameplay keymap. Server action letters have first refusal, then p/g and s/d conveniences; key collisions gate local wield/takeoff/destroy, with W versus Ctrl-W and t versus T varying by keyset. `macros_in_stores` governs command macro availability.
- The historical input.command.inventory-view row is stale: current cmd_inven/cmd_equip can continue after child actions; uppercase item paste uses Send_paste_msg then exits. Equipment takeoff uses literal t even in roguelike mode. Item views do not use the gameplay keymap.
- Skill `dev` is expansion state (Enter/c/o and Send_skill_dev), not spending points; raising uses Send_skill_mod. Rune grammar contains four stages and backtracking, not a generic spell-name picker.
- Formatted item descriptions retain source title/line/color/page/position and unknown text. Item/store names remain source bytes plus derived spans; no terminal scraping or claim that text parsing yields authoritative item semantics.
## Outcome index

| Outcome | Stage | Production owner |
| --- | --- | --- |
| `capability.items.read-inventory` | B | `src/client/nclient.c`: `Receive_inven` |
| `capability.items.read-equipment` | B | `src/client/nclient.c`: `Receive_equip` |
| `capability.items.read-bag` | B | `src/client/nclient.c`: `Receive_subinven` |
| `capability.items.read-floor` | B | `src/client/nclient.c`: `Receive_floor` |
| `capability.items.pickup-accept` | B | `src/client/nclient.c`: `Receive_pickup_check` |
| `capability.items.pickup-decline` | B | `src/client/nclient.c`: `Receive_pickup_check` |
| `capability.store.enter` | B | `src/client/nclient.c`: `Receive_store_info` |
| `capability.store.read-stock` | B | `src/client/nclient.c`: `Receive_store` |
| `capability.store.read-actions` | B | `src/client/nclient.c`: `Receive_store_action` |
| `capability.store.leave` | B | `src/client/c-store.c`: `display_store` |
| `capability.store.kicked` | B | `src/client/nclient.c`: `Receive_store_kick` |
| `capability.items.select-slot` | C | `src/client/c-inven.c`: `c_get_item` |
| `capability.items.select-name` | C | `src/client/c-inven.c`: `get_item_hook_find_obj` |
| `capability.items.cancel-selection` | C | `src/client/c-inven.c`: `c_get_item` |
| `capability.items.toggle-selection-list` | C | `src/client/c-inven.c`: `c_get_item` |
| `capability.items.drop` | C | `src/client/c-cmd.c`: `cmd_drop` |
| `capability.items.cancel-drop` | C | `src/client/c-cmd.c`: `cmd_drop` |
| `capability.items.drop-gold` | C | `src/client/c-cmd.c`: `cmd_drop_gold` |
| `capability.items.cancel-drop-gold` | C | `src/client/c-cmd.c`: `cmd_drop_gold` |
| `capability.items.wield` | C | `src/client/c-cmd.c`: `cmd_wield` |
| `capability.items.cancel-wield` | C | `src/client/c-cmd.c`: `cmd_wield` |
| `capability.items.wield-secondary` | C | `src/client/c-cmd.c`: `cmd_wield2` |
| `capability.items.cancel-wield-secondary` | C | `src/client/c-cmd.c`: `cmd_wield2` |
| `capability.items.take-off` | C | `src/client/c-cmd.c`: `cmd_take_off` |
| `capability.items.cancel-take-off` | C | `src/client/c-cmd.c`: `cmd_take_off` |
| `capability.items.swap` | C | `src/client/c-cmd.c`: `cmd_swap` |
| `capability.items.cancel-swap` | C | `src/client/c-cmd.c`: `cmd_swap` |
| `capability.items.destroy` | C | `src/client/c-cmd.c`: `cmd_destroy` |
| `capability.items.cancel-destroy` | C | `src/client/c-cmd.c`: `cmd_destroy` |
| `capability.items.inscribe` | C | `src/client/c-cmd.c`: `cmd_inscribe` |
| `capability.items.cancel-inscribe` | C | `src/client/c-cmd.c`: `cmd_inscribe` |
| `capability.items.uninscribe` | C | `src/client/c-cmd.c`: `cmd_uninscribe` |
| `capability.items.cancel-uninscribe` | C | `src/client/c-cmd.c`: `cmd_uninscribe` |
| `capability.items.autoinscribe-one` | C | `src/client/c-cmd.c`: `cmd_apply_autoins` |
| `capability.items.cancel-autoinscribe-one` | C | `src/client/c-cmd.c`: `cmd_apply_autoins` |
| `capability.items.quaff` | C | `src/client/c-cmd.c`: `cmd_quaff` |
| `capability.items.cancel-quaff` | C | `src/client/c-cmd.c`: `cmd_quaff` |
| `capability.items.read` | C | `src/client/c-cmd.c`: `cmd_read_scroll` |
| `capability.items.cancel-read` | C | `src/client/c-cmd.c`: `cmd_read_scroll` |
| `capability.items.aim-wand` | C | `src/client/c-cmd.c`: `cmd_aim_wand` |
| `capability.items.cancel-aim-wand` | C | `src/client/c-cmd.c`: `cmd_aim_wand` |
| `capability.items.use-staff` | C | `src/client/c-cmd.c`: `cmd_use_staff` |
| `capability.items.cancel-use-staff` | C | `src/client/c-cmd.c`: `cmd_use_staff` |
| `capability.items.zap-rod` | C | `src/client/c-cmd.c`: `cmd_zap_rod` |
| `capability.items.cancel-zap-rod` | C | `src/client/c-cmd.c`: `cmd_zap_rod` |
| `capability.items.refill` | C | `src/client/c-cmd.c`: `cmd_refill` |
| `capability.items.cancel-refill` | C | `src/client/c-cmd.c`: `cmd_refill` |
| `capability.items.eat` | C | `src/client/c-cmd.c`: `cmd_eat` |
| `capability.items.cancel-eat` | C | `src/client/c-cmd.c`: `cmd_eat` |
| `capability.items.activate` | C | `src/client/c-cmd.c`: `cmd_activate` |
| `capability.items.cancel-activate` | C | `src/client/c-cmd.c`: `cmd_activate` |
| `capability.items.force-stack` | C | `src/client/c-cmd.c`: `cmd_force_stack` |
| `capability.items.cancel-force-stack` | C | `src/client/c-cmd.c`: `cmd_force_stack` |
| `capability.items.split-stack` | C | `src/client/c-cmd.c`: `cmd_force_stack` |
| `capability.items.cancel-split-stack` | C | `src/client/c-cmd.c`: `cmd_force_stack` |
| `capability.items.stow` | C | `src/client/c-cmd.c`: `cmd_subinven_move` |
| `capability.items.cancel-stow` | C | `src/client/c-cmd.c`: `cmd_subinven_move` |
| `capability.items.unstow` | C | `src/client/c-cmd.c`: `cmd_subinven_remove` |
| `capability.items.cancel-unstow` | C | `src/client/c-cmd.c`: `cmd_subinven_remove` |
| `capability.items.inspect` | C | `src/client/c-cmd.c`: `cmd_observe` |
| `capability.items.cancel-inspect` | C | `src/client/c-cmd.c`: `cmd_observe` |
| `capability.items.read-newest` | C | `src/client/nclient.c`: `Receive_item_newest` |
| `capability.items.assign-newest` | C | `src/client/c-cmd.c`: `cmd_message` |
| `capability.items.autoinscribe-all` | C | `src/client/c-cmd.c`: `apply_all_auto_inscriptions` |
| `capability.items.server-item-answer` | C | `src/client/nclient.c`: `Receive_item` |
| `capability.items.server-item-cancel` | C | `src/client/nclient.c`: `Receive_item` |
| `capability.items.server-spell-answer` | C | `src/client/nclient.c`: `Receive_spell_request` |
| `capability.items.server-spell-cancel` | C | `src/client/nclient.c`: `Receive_spell_request` |
| `capability.combat.fire` | C | `src/client/c-cmd.c`: `cmd_fire` |
| `capability.combat.cancel-fire` | C | `src/client/c-cmd.c`: `cmd_fire` |
| `capability.combat.throw` | C | `src/client/c-cmd.c`: `cmd_throw` |
| `capability.combat.cancel-throw` | C | `src/client/c-cmd.c`: `cmd_throw` |
| `capability.combat.steal` | C | `src/client/c-cmd.c`: `cmd_steal` |
| `capability.combat.cancel-steal` | C | `src/client/c-cmd.c`: `cmd_steal` |
| `capability.combat.spike` | C | `src/client/c-cmd.c`: `cmd_spike` |
| `capability.combat.cancel-spike` | C | `src/client/c-cmd.c`: `cmd_spike` |
| `capability.combat.sip` | C | `src/client/c-cmd.c`: `cmd_sip` |
| `capability.combat.telekinesis` | C | `src/client/c-cmd.c`: `cmd_telekinesis` |
| `capability.combat.cloak` | C | `src/client/c-cmd.c`: `cmd_cloak` |
| `capability.skills.read-tree` | C | `src/client/nclient.c`: `Receive_skill_info` |
| `capability.skills.navigate-tree` | C | `src/client/skills.c`: `do_cmd_skill` |
| `capability.skills.raise` | C | `src/client/skills.c`: `do_cmd_skill` |
| `capability.skills.develop` | C | `src/client/skills.c`: `do_cmd_skill` |
| `capability.skills.select-ability` | C | `src/client/skills.c`: `do_cmd_activate_skill_aux` |
| `capability.skills.cancel-ability` | C | `src/client/skills.c`: `do_cmd_activate_skill_aux` |
| `capability.spells.read-metadata` | C | `src/client/nclient.c`: `Receive_spell_info` |
| `capability.spells.choose-school` | C | `src/client/c-spell.c`: `get_school_spell` |
| `capability.spells.browse-book` | C | `src/client/c-cmd.c`: `cmd_browse` |
| `capability.spells.cast-school` | C | `src/client/skills.c`: `do_activate_skill` |
| `capability.spells.cancel-cast-school` | C | `src/client/skills.c`: `do_activate_skill` |
| `capability.spells.cast-legacy` | C | `src/client/skills.c`: `do_activate_skill` |
| `capability.spells.cancel-cast-legacy` | C | `src/client/skills.c`: `do_activate_skill` |
| `capability.spells.activate-generic` | C | `src/client/skills.c`: `do_activate_skill` |
| `capability.spells.cancel-activate-generic` | C | `src/client/skills.c`: `do_activate_skill` |
| `capability.spells.ghost` | C | `src/client/c-spell.c`: `do_ghost` |
| `capability.spells.cancel-ghost` | C | `src/client/c-spell.c`: `do_ghost` |
| `capability.spells.mimic-power` | C | `src/client/c-spell.c`: `do_mimic` |
| `capability.spells.cancel-mimic-power` | C | `src/client/c-spell.c`: `do_mimic` |
| `capability.spells.mimic-form` | C | `src/client/c-spell.c`: `do_mimic` |
| `capability.spells.cancel-mimic-form` | C | `src/client/c-spell.c`: `do_mimic` |
| `capability.spells.mimic-immunity` | C | `src/client/c-spell.c`: `do_mimic` |
| `capability.spells.cancel-mimic-immunity` | C | `src/client/c-spell.c`: `do_mimic` |
| `capability.spells.stance` | C | `src/client/c-spell.c`: `do_stance` |
| `capability.spells.cancel-stance` | C | `src/client/c-spell.c`: `do_stance` |
| `capability.spells.melee-technique` | C | `src/client/c-spell.c`: `do_melee_technique` |
| `capability.spells.cancel-melee-technique` | C | `src/client/c-spell.c`: `do_melee_technique` |
| `capability.spells.ranged-technique` | C | `src/client/c-spell.c`: `do_ranged_technique` |
| `capability.spells.cancel-ranged-technique` | C | `src/client/c-spell.c`: `do_ranged_technique` |
| `capability.spells.runecraft` | C | `src/client/c-spell.c`: `do_runecraft` |
| `capability.spells.cancel-runecraft` | C | `src/client/c-spell.c`: `do_runecraft` |
| `capability.spells.breath-preference` | C | `src/client/c-spell.c`: `do_pick_breath` |
| `capability.spells.cancel-breath-preference` | C | `src/client/c-spell.c`: `do_pick_breath` |
| `capability.spells.breath` | C | `src/client/c-spell.c`: `do_breath` |
| `capability.spells.cancel-breath` | C | `src/client/c-spell.c`: `do_breath` |
| `capability.spells.trap` | C | `src/client/skills.c`: `do_trap` |
| `capability.spells.cancel-trap` | C | `src/client/skills.c`: `do_trap` |
| `capability.spells.mycorrhiza` | C | `src/client/skills.c`: `do_mycorrhiza` |
| `capability.spells.cancel-mycorrhiza` | C | `src/client/skills.c`: `do_mycorrhiza` |
| `capability.spells.stop-mycorrhiza` | C | `src/client/skills.c`: `do_mycorrhiza` |
| `capability.store.page` | C | `src/client/c-store.c`: `store_process_command` |
| `capability.store.buy` | C | `src/client/c-store.c`: `store_purchase` |
| `capability.store.take-home` | C | `src/client/c-store.c`: `store_purchase` |
| `capability.store.sell` | C | `src/client/c-store.c`: `store_sell` |
| `capability.store.deposit-home` | C | `src/client/c-store.c`: `store_sell` |
| `capability.store.deposit-gold` | C | `src/client/c-store.c`: `store_process_command` |
| `capability.store.donate` | C | `src/client/c-store.c`: `store_sell` |
| `capability.store.accept-offer` | C | `src/client/nclient.c`: `Receive_sell` |
| `capability.store.decline-offer` | C | `src/client/nclient.c`: `Receive_sell` |
| `capability.store.examine` | C | `src/client/c-store.c`: `store_examine` |
| `capability.store.service` | C | `src/client/c-store.c`: `store_do_command` |
| `capability.store.raw-key` | C | `src/client/c-store.c`: `store_process_command` |
| `capability.store.paste-stock` | C | `src/client/c-store.c`: `store_chat` |
| `capability.store.child-item` | C | `src/client/c-store.c`: `store_process_command` |
| `capability.store.cancel-buy` | C | `src/client/c-store.c`: `store_purchase` |
| `capability.store.cancel-take-home` | C | `src/client/c-store.c`: `store_purchase` |
| `capability.store.cancel-sell` | C | `src/client/c-store.c`: `store_sell` |
| `capability.store.cancel-deposit-home` | C | `src/client/c-store.c`: `store_sell` |
| `capability.store.cancel-deposit-gold` | C | `src/client/c-store.c`: `store_process_command` |
| `capability.store.cancel-donate` | C | `src/client/c-store.c`: `store_sell` |
| `capability.store.cancel-examine` | C | `src/client/c-store.c`: `store_examine` |
| `capability.store.cancel-service` | C | `src/client/c-store.c`: `store_do_command` |
| `capability.store.cancel-paste-stock` | C | `src/client/c-store.c`: `store_chat` |
| `capability.store.transaction-rejected` | C | `src/client/c-store.c`: `store_purchase` |
| `capability.items.autoinscribe-on-update` | B | `src/client/nclient.c`: `Receive_apply_auto_insc` |
| `capability.items.details-read` | C | `src/client/c-files.c`: `peruse_file` |
| `capability.items.details-navigate` | C | `src/client/c-files.c`: `peruse_file` |
| `capability.items.details-search` | C | `src/client/c-files.c`: `peruse_file` |
| `capability.items.details-close` | C | `src/client/c-files.c`: `peruse_file` |
| `capability.items.use-selected` | C | `src/client/c-cmd.c`: `cmd_all_in_one` |
| `capability.items.cancel-use-selected` | C | `src/client/c-cmd.c`: `cmd_all_in_one` |
| `capability.items.close-inventory` | B | `src/client/c-cmd.c`: `cmd_inven` |
| `capability.items.paste-inventory` | C | `src/client/c-cmd.c`: `cmd_inven` |
| `capability.items.close-equipment` | B | `src/client/c-cmd.c`: `cmd_equip` |
| `capability.items.paste-equipment` | C | `src/client/c-cmd.c`: `cmd_equip` |
| `capability.items.close-bag` | B | `src/client/c-cmd.c`: `cmd_subinven` |
| `capability.items.paste-bag` | C | `src/client/c-cmd.c`: `cmd_subinven` |

## Baseline family reconciliation

Recovered from `git show 87ead6ff5:docs/research/single-window-behavior-baseline.md`.

```text
| CMD-02 | Items | inventory/equipment, drop/gold, wear/remove/swap, destroy/stack, inscriptions, devices/consumables | `src/client/c-cmd.c:331-359` |
```
```text
| CMD-03 | Combat/magic/target | fire/throw, browse, skills/abilities/ghost, hostile/friendly target, look | `src/client/c-cmd.c:361-379` |
```
```text
| PROMPT-01 | Local prompts | item source/item, direction/target, amount, text editing, confirmation, repeat and context-local navigation | `/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:195-229`, `/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:482-496` |
```
```text
| PROMPT-02 | Server prompts | key, amount, bounded number, string, confirmation and asynchronous abort | `src/client/nclient.c:7176-7253` |
```
```text
| DATA-01 | Inventory/equipment/bags | Slots, color, type, subtype, weight/count/value, artifact, identification, directional/trade/set data and names | `src/client/nclient.c:2317-2405`, `src/client/nclient.c:2649-2725` |
```
```text
| DATA-02 | Skills/magic | skill tree/value updates, spell/power/technique requests and activation parameters | `src/client/nclient.c:381-406`, `src/client/nclient.c:3941-4012`, `src/client/nclient.c:6150-6192` |
```
```text
| STORE-01 | Server-driven UI | normal/wide/special store rows, actions, animation, sell/leave and generic requests | `src/client/nclient.c:387-409`, `src/client/nclient.c:445-448`, `src/client/nclient.c:4510-5515` |
```
```text
| SUBWIN-01 | Legacy information | inventory, equipment, character, non-chat/all/chat messages, clone map, lagometer, players, bonuses and bags | `src/client/c-tables.c:142-154` |
```
```text
| NET-02 | Outgoing intents | all movement/item/target/store/social/admin/account/request/setup/audio/font packets | `src/client/nclient.c:7529-8239`, `src/client/nclient.c:8987-9204` |
```
CMD-02/DATA-01 → items.*; CMD-03/DATA-02 → combat.*, skills.*, spells.* plus existing direction/target IDs; STORE-01 → store.* ordinary shell/transactions, special canvases delegated to 10 above. PROMPT-01 maps each caller in the input annex; PROMPT-02 retains generic owners and the explicit server-request handoff. SUBWIN-01 inventory/equipment/bag information is items.read-*, while terminal topology remains excluded. NET-02 serializer cases are listed below. CMD-05 automatic inscription application is items.autoinscribe-*; its editor is 11. Other families remain with 08/10/11/12 and are not accepted/excluded here.

## Scoped input contexts and function owners

Original: `docs/research/single-window-input-loops.md`; SHA-256 `40a47b0ae685a8a1ee2f1309c76e60722783b05aa494504f47b7a5ed8b88fef7`.

### Original line 31

Shared primitive: apply the exact row to every applicable item/spell/store caller’s `.input`/`.lifecycle` scenarios above. Not a second copy or blanket acceptance of all callers; cross-client primitive reconciliation is ticket 12.

```text
| `input.core.macro` | `inkey_aux()` / `inkey()` | longest matching trigger; backquote becomes Esc after matching; bytes 28/29/30/31 delimit fallback/action/special sequences | before keymap | policy from flags | unmatched trigger bytes are pushed back; multi-key wait tops out after increasing 10-unit delays; macro completion byte 29 ends `parse_macro` | `parse_macro`, `after_macro`, `parse_under`, `parse_slash`, `strip_chars`, `macro_missing_item` | none | activates main term, then restores old term and cursor | `ALLOW_NAVI_KEYS_IN_PROMPT`; `SOME_NAVI_KEYS_DISABLE_MACROS_IN_PROMPTS` | [`c-util.c:1160`](../../src/client/c-util.c#L1160), [`c-util.c:1262`](../../src/client/c-util.c#L1262), [`c-util.c:1376`](../../src/client/c-util.c#L1376), [`c-util.c:1618`](../../src/client/c-util.c#L1618) |
```

### Original line 32

Shared primitive: apply the exact row to every applicable item/spell/store caller’s `.input`/`.lifecycle` scenarios above. Not a second copy or blanket acceptance of all callers; cross-client primitive reconciliation is ticket 12.

```text
| `input.core.macro-wait` | macro `\\wDD` -> `sync_sleep()` | byte 96 plus exactly two decimal digits; waits `DD * 100ms` while pumping network/timers/redraw | before keymap | only inside expanded macro | completes on duration, nested input semaphore, or server `PKT_CONFIRM`; interactive cancellation is compiled out | `command_confirmed=-1`, `inkey_sleep=true`; reset sleep/semaphore at completion | keepalive/ping/network processing | spinner in last topline cell, then erase | `ACCEPT_KEYS` disabled by source; Windows timer vs `gettimeofday` | [`c-util.c:156`](../../src/client/c-util.c#L156), [`c-util.c:417`](../../src/client/c-util.c#L417), [`c-util.c:494`](../../src/client/c-util.c#L494), [`c-util.c:939`](../../src/client/c-util.c#L939) |
```

### Original line 33

Shared primitive: apply the exact row to every applicable item/spell/store caller’s `.input`/`.lifecycle` scenarios above. Not a second copy or blanket acceptance of all callers; cross-client primitive reconciliation is ticket 12.

```text
| `input.core.macro-xwait` | macro `\\WDDDD` encoding -> byte 30 -> `sync_xsleep()` | byte 30 plus exactly four decimal digits; waits `DDDD * 100ms`; fresh Esc aborts, Space resumes early, other fresh keys are preserved in old queue | before keymap | only inside expanded macro | duration/semaphore/confirm completes; Esc discards temporary queue; Space restores old queue | swaps `Term->keys` with temporary queue; same sleep flags | keepalive/ping/network processing | spinner in last topline cell, then erase | Windows timer vs `gettimeofday`; byte 30 legacy control-caret collision noted in source | [`c-util.c:157`](../../src/client/c-util.c#L157), [`c-util.c:568`](../../src/client/c-util.c#L568), [`c-util.c:651`](../../src/client/c-util.c#L651), [`c-util.c:1036`](../../src/client/c-util.c#L1036) |
```

### Original line 34

`capability.items.cancel-selection`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.core.missing-item` | `inkey_aux()` + `c_get_item()` + `bell*()` | state 1 drops an `@` and enters state 2, or drops one numeric tag; state 2 drops name bytes through CR; state 3 drops one numeric inscription byte | before keymap | expanded macro only | successful/consumed branch returns state 0; with `safe_macros`, a failed item loop sees `abort_prompt`, sets `command_gap=50`, flushes and exits | `macro_missing_item: 0 -> 1/3 -> 2/0`; `abort_prompt` is set only when a bell occurs during `parse_macro` | none | failed topline prompt is force-cleared; prompt can be restored at macro end when configured | `safe_macros`, `keep_topline` | [`c-inven.c:1325`](../../src/client/c-inven.c#L1325), [`c-inven.c:1420`](../../src/client/c-inven.c#L1420), [`c-util.c:1154`](../../src/client/c-util.c#L1154), [`c-util.c:1951`](../../src/client/c-util.c#L1951) |
```

### Original line 35

Shared primitive: apply the exact row to every applicable item/spell/store caller’s `.input`/`.lifecycle` scenarios above. Not a second copy or blanket acceptance of all callers; cross-client primitive reconciliation is ticket 12.

```text
| `input.core.prompt-navigation` | `inkey_combo()` / `scan_navi_key()` | arrow/Home/End/PgUp/PgDn/Delete become `NAVI_KEY_*`; optional edit modifier preserves legacy default text behavior | same | navigation may bypass macro matching | helper toggles `inkey_location_keys` only for the read, then clears it | cursor/key result | none | caller prompt | `ALLOW_NAVI_KEYS_IN_PROMPT`; optional `SOME_NAVI_KEYS_DISABLE_MACROS_IN_PROMPTS` makes `inkey_aux()` return decoded navigation before macro lookup | [`c-util.c:732`](../../src/client/c-util.c#L732), [`c-util.c:855`](../../src/client/c-util.c#L855), [`c-util.c:1241`](../../src/client/c-util.c#L1241) |
```

### Original line 36

Shared primitive: apply the exact row to every applicable item/spell/store caller’s `.input`/`.lifecycle` scenarios above. Not a second copy or blanket acceptance of all callers; cross-client primitive reconciliation is ticket 12.

```text
| `input.prompt.text` | `askfor_aux()` | Enter accept; Esc cancel; Backspace/Delete; arrows/Home/End; Ctrl-A/E/B/F; history up/down; Ctrl-R search, Ctrl-G cancel search; Ctrl-K copy; Ctrl-L paste; printable insertion | same | hybrid normally caller-suppressed | Esc false; Enter true, including empty; invalid/full input bells and retries | editable buffer, cursor, history cursor, search result; private mode masks output and disables initial edit | none | topline/caller cursor | `ALLOW_NAVI_KEYS_IN_PROMPT`; clipboard hooks; `ASKFOR_PRIVATE`, `ASKFOR_CHATTING`, `ASKFOR_LIVETRIM` | [`c-util.c:2613`](../../src/client/c-util.c#L2613), [`c-util.c:2699`](../../src/client/c-util.c#L2699), [`c-util.c:2720`](../../src/client/c-util.c#L2720), [`c-util.c:3385`](../../src/client/c-util.c#L3385) |
```

### Original line 37

Shared primitive: apply the exact row to every applicable item/spell/store caller’s `.input`/`.lifecycle` scenarios above. Not a second copy or blanket acceptance of all callers; cross-client primitive reconciliation is ticket 12.

```text
| `input.prompt.command` | `get_com()` / `get_com_bk()` | one logical key | same | inherited | Esc false; `_bk`: Backspace returns -2; every other key true | none | none | topline; flush queue after | none | [`c-util.c:3516`](../../src/client/c-util.c#L3516), [`c-util.c:3542`](../../src/client/c-util.c#L3542) |
```

### Original line 38

Shared primitive: apply the exact row to every applicable item/spell/store caller’s `.input`/`.lifecycle` scenarios above. Not a second copy or blanket acceptance of all callers; cross-client primitive reconciliation is ticket 12.

```text
| `input.prompt.confirm` | `get_check2/3()`, `get_3way()` | y/n; `get_3way`: y/a/n | same | inherited | `get_check2`: any key takes displayed default except explicit opposite; `get_check3(0)` retries until y/n; `get_3way` retries until y/a/n and, with default-no, Esc/Enter/Ctrl-Q mean no | none | none | topline; flush queue | none | [`c-util.c:3765`](../../src/client/c-util.c#L3765), [`c-util.c:3816`](../../src/client/c-util.c#L3816), [`c-util.c:3858`](../../src/client/c-util.c#L3858) |
```

### Original line 39

Shared primitive: apply the exact row to every applicable item/spell/store caller’s `.input`/`.lifecycle` scenarios above. Not a second copy or blanket acceptance of all callers; cross-client primitive reconciliation is ticket 12.

```text
| `input.prompt.quantity` | `c_get_quantity()`, `c_get_number()` | text number; `a/A/Space` can mean all through `inkey_letter_all` | same | caller policy | Esc/empty -> 0; clamps or rejects to caller range | numeric value | none | topline | none | [`c-util.c:5058`](../../src/client/c-util.c#L5058), [`c-util.c:5159`](../../src/client/c-util.c#L5159) |
```

### Original line 43

`capability.items.read-inventory`, `capability.items.read-equipment`, `capability.items.read-bag`, `capability.items.inspect`, `capability.items.drop`, `capability.items.destroy`, `capability.items.inscribe`, `capability.items.uninscribe`, `capability.items.stow`, `capability.items.unstow`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.command.inventory-view` | `cmd_inven()`, `cmd_subinven()`, `cmd_equip()` | item letter examine; x examine; d drop; normal k / rogue Ctrl-D destroy; `{`/`}` inscription; `:` chat; inventory additionally b/s; subinventory a/S; equipment t take off | explicit N/R aliases | inherited | Esc leaves child selectors; loop itself is one-key action then restore | selected item/subinventory, optional list switch | typed item commands | save/load | `ENABLE_SUBINVEN` | [`c-cmd.c:965`](../../src/client/c-cmd.c#L965), [`c-cmd.c:1119`](../../src/client/c-cmd.c#L1119), [`c-cmd.c:1297`](../../src/client/c-cmd.c#L1297) |
```

### Original line 44

`capability.items.select-slot`, `capability.items.select-name`, `capability.items.toggle-selection-list`, `capability.items.cancel-selection`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.command.item-select` | `c_get_item()` | letter select; uppercase verify/alternate; digits inscription tag; Enter sole item; `@` name; `* ? Space` list; `/` inven/equip; `!` bag; `#` spell limit; `-` special return; `+` newest | same | command exception; safe failure flush | Esc false; bad key/item retry; uppercase rejected confirmation ends false; safe failed macro aborts | `command_wrk`, `command_see`, `using_subinven`; returns item, negative alternate, -3 special | none; caller emits | save/load if list shown, always redraw inventory/equipment | `ENABLE_SUBINVEN`; `ITEM_PROMPT_ALLOWS_SWITCHING_TO_SUBINVEN`; `CAPS_ALT`/mode flags | [`c-inven.c:1092`](../../src/client/c-inven.c#L1092), [`c-inven.c:1420`](../../src/client/c-inven.c#L1420), [`c-inven.c:1629`](../../src/client/c-inven.c#L1629), [`c-inven.c:2032`](../../src/client/c-inven.c#L2032) |
```

### Original line 56

`capability.store.enter`, `capability.store.read-stock`, `capability.store.read-actions`, `capability.store.leave`, `capability.store.kicked`, `capability.store.page`, `capability.store.buy`, `capability.store.take-home`, `capability.store.sell`, `capability.store.deposit-home`, `capability.store.deposit-gold`, `capability.store.donate`, `capability.store.accept-offer`, `capability.store.decline-offer`, `capability.store.examine`, `capability.store.service`, `capability.store.raw-key`, `capability.store.paste-stock`, `capability.store.child-item`, `capability.store.cancel-buy`, `capability.store.cancel-take-home`, `capability.store.cancel-sell`, `capability.store.cancel-deposit-home`, `capability.store.cancel-deposit-gold`, `capability.store.cancel-donate`, `capability.store.cancel-examine`, `capability.store.cancel-service`, `capability.store.cancel-paste-stock`, `capability.store.transaction-rejected`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.command.store` | `display_store()` / special -> `store_process_command()` | store-defined action letters first; Space/Backspace pages; 1-0 page; c paste; `:` chat; I inspect; i/e/b; `{}`; physical keyset-specific wield/takeoff/destroy aliases; `$` gold; Esc/Ctrl-Q | explicit N/R collisions | depends `macros_in_stores`; otherwise command macros blocked | Esc leaves and sends leave; child cancel returns; invalid/default is raw key | `shopping`, `store_top`, stock state | `Send_store_command`, typed item operations, or `Send_raw_key` | save/load | `USE_SOUND_2010`; subinventory; store flags | [`c-store.c:800`](../../src/client/c-store.c#L800), [`c-store.c:1127`](../../src/client/c-store.c#L1127), [`c-store.c:1249`](../../src/client/c-store.c#L1249) |
```

### Original line 57

`capability.store.buy`, `capability.store.take-home`, `capability.store.examine`, `capability.store.service`, `capability.store.cancel-buy`, `capability.store.cancel-take-home`, `capability.store.cancel-examine`, `capability.store.cancel-service`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.command.stock-select` | `get_stock()` | displayed stock letter through `get_com()` | same | store policy | Esc cancels; invalid stock retries | chosen stock index | caller purchase/examine | topline | stock bounds | [`c-store.c:187`](../../src/client/c-store.c#L187) |
```

### Original line 58

`capability.skills.read-tree`, `capability.skills.navigate-tree`, `capability.skills.develop`, `capability.skills.raise`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.command.skill-tree` | `do_cmd_skill()` | 2/j, 8/k move; 6/l raise; Enter expand; c collapse all; o expand all; g/7/Home, G/1/End; n/Space/3/PgDn; p/b/9/PgUp; # line; s// search; ? guide; `:`; Ctrl-T; Esc/Ctrl-Q | same aliases | inherited; hybrid suppressed for search | exit restores; cancel search stays | selection/start, local `dev` tree | `Send_skill_dev`, `Send_skill_mod` | save/load | available skills | [`skills.c:288`](../../src/client/skills.c#L288), [`skills.c:316`](../../src/client/skills.c#L316), [`skills.c:388`](../../src/client/skills.c#L388) |
```

### Original line 59

`capability.skills.select-ability`, `capability.skills.cancel-ability`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.command.ability-select` | `do_cmd_activate_skill_aux()` | a..t; `@` name/number; `* ? Space` show; +/- page; Ctrl-T; Esc | same | safe | Esc/missing safe macro -> -1; bad key retries (or safe flush) | list visibility/start, chosen skill | caller activates skill, may open item/direction prompt | conditional save/load | learned active skills | [`skills.c:535`](../../src/client/skills.c#L535), [`skills.c:586`](../../src/client/skills.c#L586), [`skills.c:624`](../../src/client/skills.c#L624) |
```

### Original line 73

Ticket 11 / E owns rule editing/navigation/files; actual application is `capability.items.autoinscribe-one`, `capability.items.autoinscribe-all`, `capability.items.autoinscribe-on-update`. c-util.c auto_inscriptions is the source owner.

```text
| `input.autoinscription.editor` | `auto_inscriptions()` | exact editor/navigation/file keys in subordinate map below | same | normal/hybrid suppressed | Esc exits; canceled child stays | ordered rules and tags | local `.ins` files; apply can affect item inscriptions | save/load | auto-inscription features | [`c-util.c:12043`](../../src/client/c-util.c#L12043), [`c-util.c:12175`](../../src/client/c-util.c#L12175), [`c-util.c:12741`](../../src/client/c-util.c#L12741) |
```

### Original line 99

`capability.items.read-inventory`, `capability.items.read-equipment`, `capability.items.drop`, `capability.items.drop-gold`, `capability.items.wield`, `capability.items.take-off`, `capability.items.swap`, `capability.items.destroy`, `capability.items.force-stack`, `capability.items.split-stack`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `i`, `e`, `d`, `$`, `w`, `t`, `x`, `k`, `K` | same | `i`, `e`, `d`, `$`, `w`, `T`, `S`, Ctrl-D, Ctrl-C | inventory/equipment/drop gold/wield/takeoff/swap/destroy/force-stack |
```

### Original line 100

`capability.items.inscribe`, `capability.items.uninscribe`, `capability.items.autoinscribe-one`, `capability.combat.steal`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `{`, `}`, `H`, `j` | same | `{`, `}`, Ctrl-G, Ctrl-A | inscribe/uninscribe/apply auto-inscriptions/steal |
```

### Original line 101

`capability.items.quaff`, `capability.items.read`, `capability.items.aim-wand`, `capability.items.use-staff`, `capability.items.zap-rod`, `capability.items.refill`, `capability.items.eat`, `capability.items.activate`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `q`, `r`, `a`, `u`, `z`, `F`, `E`, `A` | same | `q`, `r`, `z`, `Z`, `a`, `F`, `E`, `A` | quaff/read/aim/staff/rod/refill/eat/activate |
```

### Original line 102

`capability.combat.fire`, `capability.combat.throw`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `f`, `v` | same | `t`, `v` | fire/throw |
```

### Original line 103

`capability.spells.browse-book`, `capability.skills.read-tree`, `capability.skills.select-ability`, `capability.spells.ghost`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `b`, `G`, `m`, `U` | same | `P`, `G`, `m`, Ctrl-X | browse book/skill tree/activate skill/ghost power |
```

### Original line 107

`capability.combat.sip`, `capability.combat.telekinesis`, `capability.items.wield-secondary`, `capability.combat.cloak`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `_`, `p`, `W`, `V` | same | same | sip/telekinesis/secondary wield/cloak |
```

### Original line 110

`capability.items.use-selected`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `=`, `"`, `%`, `&`, `h`, `/` | same | `=`, `"`, `%`, `&`, Ctrl-E, `/` | options/load pref/macro editor/autoinscriptions/house/all-in-one |
```

### Original line 111

`capability.combat.spike`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| Ctrl-S, Ctrl-T, Ctrl-I | same | same | spike/screenshot/lagometer |
```

### Original line 157

`capability.items.use-selected`, `capability.items.cancel-use-selected`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `c-cmd.c` | `cmd_all_in_one`, `process_command`, movement direction wrappers, `cmd_mini_map`, `cmd_locate`, inventory/subinventory/equipment, destroy/inscribe, steal/device/activate direction prompts, target/look, character, guide/local-file, artifact/monster lore, spoilers, notes, misc index, message, guild, party, fire/throw, load-pref, house commands, suicide, all `cmd_master*`/script helpers, lagometer |
```

### Original line 158

`capability.items.select-slot`, `capability.items.select-name`, `capability.items.cancel-selection`, `capability.items.toggle-selection-list`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `c-inven.c` | `verify`, `get_item_hook_find_obj`, `c_get_item` |
```

### Original line 159

`capability.store.leave`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `c-store.c` | `get_stock`, purchase/sell quantity and confirmation prompts, `display_store`, `display_store_special` |
```

### Original line 162

`capability.skills.navigate-tree`, `capability.skills.raise`, `capability.skills.develop`, `capability.skills.select-ability`, `capability.skills.cancel-ability`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `skills.c` | `do_cmd_skill`, `do_cmd_activate_skill_aux`, activation direction prompts |
```

### Original line 250

`capability.store.page`, `capability.store.deposit-gold`, `capability.store.raw-key`, `capability.store.child-item`, `capability.store.cancel-deposit-gold`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `c-store.c: display_store, display_store_special` | loops | same `store_process_command` state machine over normal/special rendering |
```

Mapped/dispositioned rows: 29.

## Remaining spell, packet and Lua contexts

Original: `.scratch/single-window-sdl3-client/research/remaining-client-input-loops.md`; SHA-256 `76f85717b58f8a026a43dab05ee322150abea260f07f38316886186b71d47caf`.

### Original line 15

`capability.spells.cast-legacy`, `capability.spells.ghost`, `capability.spells.cancel-cast-legacy`, `capability.spells.cancel-ghost`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.spell.legacy-select` | `get_spell`, вызывается ghost/legacy spell activation | `a..a+num-1` select; `A..` select **index+64**, не verification; Space/`*`/`?` toggle list; `(Spells/Powers…, *=List, ESC=exit)` | same; inherited | Esc=false; нет usable spells: sn=-2; canceled=-1; invalid bell/retry | realm/book/sval; sn; redraw | local result; `do_ghost` → `Send_ghost`; caller skill → `Send_activate_skill` | list restore + flush | ghost → REALM_GHOST; server spell_info availability | [c-spell.c:242](../../../src/client/c-spell.c#L242), [read:309](../../../src/client/c-spell.c#L309), [uppercase:352](../../../src/client/c-spell.c#L352), [ghost:857](../../../src/client/c-spell.c#L857) |
```

### Original line 16

`capability.spells.mimic-power`, `capability.spells.cancel-mimic-power`, `capability.spells.mimic-form`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.spell.mimic-select` | `get_mimic_spell` → `do_mimic` | lower letters index accessible powers; Space/`*`/`?` list; `@` → text primitive `Power?` exact case-insensitive available innate name | same; inherited, safe_macros checks captured parse_macro | Esc=false; name Esc exits whole selector; invalid bell/retry; safe_input+abort_prompt flush/break; final abort_prompt reset | corresp maps four fixed powers + RF4/5/6/0 available bits; sn; redraw | local result; skill caller below | list; clipping flags cleared | MIMIC_LUA only changes metadata, not selector vocabulary; always_show_lists; safe_macros | [c-spell.c:412](../../../src/client/c-spell.c#L412), [read:490](../../../src/client/c-spell.c#L490), [name:523](../../../src/client/c-spell.c#L523), [cleanup:582](../../../src/client/c-spell.c#L582) |
```

### Original line 17

`capability.spells.mimic-immunity`, `capability.spells.cancel-mimic-immunity`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.spell.mimic-immunity` | `do_mimic`, selected power=3 | `a..h` check/none/electricity/cold/fire/acid/poison/water; Space/`*`/`?` list; `@` → `Immunity?`, case-insensitive substring Ch/No/El/Co/Fi/Ac/Po/Wa, priority in source order | same; inherited | Esc/no c → return without packet; name Esc returns entire flow; unmatched bell/retry | c=1..8; redraw | `Send_activate_skill(MKEY_MIMICRY,0,25000,c,0,0)` | list + clipping reset; name-cancel early return skips later clipping reset | server newer than4.4.9.1.0.0 | [c-spell.c:722](../../../src/client/c-spell.c#L722), [read:747](../../../src/client/c-spell.c#L747), [name:780](../../../src/client/c-spell.c#L780), [intent:819](../../../src/client/c-spell.c#L819) |
```

### Original line 18

`capability.spells.choose-school`, `capability.spells.cast-school`, `capability.spells.cancel-cast-school`, `capability.items.server-spell-answer`, `capability.items.server-spell-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.spell.school-select` | `get_school_spell`; optional book item primitive first | lower letter selects; upper letter describes and remains; Space/`*`/`?` list; `…Descs A-…, …which spell?` | same; inherited | Esc=-1; invalid or !is_ok_spell bell/retry; forced spell bypasses read but still availability check | item; spell; where/redraw; hack_force_spell/level | result `spell+1000*level`, item_book; caller sends | list restore; no owner Flush_queue at final restore | SFLG1_LIMIT_SPELLS; DISCRETE_SPELL_SYSTEM name lookup; book payload/Lua | [c-spell.c:1058](../../../src/client/c-spell.c#L1058), [book:1087](../../../src/client/c-spell.c#L1087), [read:1152](../../../src/client/c-spell.c#L1152), [desc:1207](../../../src/client/c-spell.c#L1207), [exit:1254](../../../src/client/c-spell.c#L1254) |
```

### Original line 19

`capability.spells.browse-book`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.spell.school-browse` | `browse_school_spell` | lower letter description; upper letter chat entry+description via Lua; Esc exit; no Space/`*`/`?` toggle (invalid) | same; inherited | invalid bell/retry; successful desc/paste remains | where/clipping; selected spell | local descriptions; uppercase Lua emits chat | unconditional save, Term_restore per key, load on exit; clipping reset | book data; USE_SOUND_2010 browse sound | [c-spell.c:1268](../../../src/client/c-spell.c#L1268), [read:1289](../../../src/client/c-spell.c#L1289), [paste:1321](../../../src/client/c-spell.c#L1321) |
```

### Original line 20

`capability.spells.stance`, `capability.spells.cancel-stance`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.spell.stance-select` | `get_combatstance` → `do_stance` | `a/b/c` balanced/defensive/offensive; `-` previous; Space/`*`/`?` list | same; inherited | Esc=false; invalid bell/retry | corresp=0/1/2/-1; redraw | `Send_activate_skill(MKEY_STANCE,stance,0,0,0,0)` | list; clipping reset | always_show_lists | [c-spell.c:1360](../../../src/client/c-spell.c#L1360), [read:1396](../../../src/client/c-spell.c#L1396), [intent:1471](../../../src/client/c-spell.c#L1471) |
```

### Original line 21

`capability.spells.melee-technique`, `capability.spells.ranged-technique`, `capability.spells.cancel-melee-technique`, `capability.spells.cancel-ranged-technique`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.spell.technique-select` | separate `get_melee_technique` and `get_ranged_technique`, equivalent context over different bitmask/name table | lower letters available bitmask; Space/`*`/`?` list; `@` text `Technique?`, default `Sprint` / `Flare Missile`, exact case-insensitive available name | same; inherited | Esc=false; name Esc exits entire flow; invalid bell/retry | corresp available bits; sn=-2 initial/-1 cancel; redraw/topline_icky | `Send_activate_skill(MKEY_MELEE/MKEY_RANGED,0,technique,0,0,0)` | list + final forced topline clear/icky reset; name Esc early returns before normal flag cleanup | available melee/ranged_techniques; always_show_lists | [melee:1499](../../../src/client/c-spell.c#L1499), [read:1549](../../../src/client/c-spell.c#L1549), [name:1582](../../../src/client/c-spell.c#L1582), [ranged:1679](../../../src/client/c-spell.c#L1679), [read:1726](../../../src/client/c-spell.c#L1726), [name:1760](../../../src/client/c-spell.c#L1760) |
```

### Original line 22

`capability.spells.runecraft`, `capability.spells.cancel-runecraft`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.spell.runecraft-compose` | `do_runecraft` | `a..a+rcraft_max(u)` choose available rune bit; Space/`*`/`?` list; Backspace undo; dynamic `rcraft_com(u)` prompt | same; inherited; get_com_bk | Esc return; Backspace at step0 return; invalid bell/retry; complete rcraft_end then optional direction primitive | u bitfield; u_prev[4]; step; list | `Send_activate_skill(MKEY_RCRAFT,low16(u),high16(u),dir,0,0)` | save/restore list per stage; final unconditional Term_load in source | Lua runecraft grammar; always_show_lists | [c-spell.c:1861](../../../src/client/c-spell.c#L1861), [read:1879](../../../src/client/c-spell.c#L1879), [back:1886](../../../src/client/c-spell.c#L1886), [direction/intent:1929](../../../src/client/c-spell.c#L1929), [runecraft.lua](../../../lib/scpt/runecraft.lua) |
```

### Original line 23

`capability.spells.breath-preference`, `capability.spells.cancel-breath-preference`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.spell.breath-preference` | `get_breath` → `do_pick_breath` | `a..g` check/none/lightning/frost/fire/acid/poison; TRAIT_POWER additionally h..m confusion/inertia/sound/shards/chaos/disenchantment; Space/`*`/`?` list; `@` → `Element?` substrings Ch-except-Cha/No/Li/Fr/Fi/Ac/Po/Co/In/So/Sh/Cha/Di | same; inherited | outer Esc=false; invalid bell/retry; **name Esc restores if shown then continues**, does not exit; name-match path accepts i7..12 without num check | num7 or13; corresp; redraw; br | `Send_activate_skill(MKEY_PICK_BREATH,br,0,0,0,0)` | list; clipping reset; child cancel leaves redraw true after load (source imbalance possibility) | caller only TRAIT_MULTI/POWER; power lineage described hypothetical by source | [c-spell.c:1987](../../../src/client/c-spell.c#L1987), [read:2023](../../../src/client/c-spell.c#L2023), [name:2053](../../../src/client/c-spell.c#L2053), [caller:2116](../../../src/client/c-spell.c#L2116) |
```

### Original line 26

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`, `capability.items.details-close`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `input.server-file.peruse` | `peruse_file` | exact transitions next section | same; inherited, inkey_interact_macros=false; nested search sets inkey_msg=true | Esc/Ctrl-Q exit; `?` HELP exits then guide; player-list inkey -1 redraw/re-request; invalid key no-op/re-request | cur_line/col; srcstr; searching/reverse/regexp; max_line/page_size; perusing | `Send_special_line(type,line,search)`; exit `SPECIAL_FILE_NONE`; chat/inscription delegates | save/load; clipping managed received lines; restore macro flag+Flush_queue on exit | REGEX_SEARCH and server>=4.9.0; SPECIAL_FILE_PLAYER auto-refresh; USE_SOUND_2010 | [c-files.c:1953](../../../src/client/c-files.c#L1953), [requests:1993](../../../src/client/c-files.c#L1993), [reads:2042](../../../src/client/c-files.c#L2042), [exit:2244](../../../src/client/c-files.c#L2244) |
```

### Original line 43

`capability.spells.mimic-power`, `capability.spells.runecraft`, `capability.spells.breath`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `do_mimic`:677 | `input.prompt.text`, length40, empty default; `Which form (name or number; 0 for player; -1 for previous) ?` | ignores get_string boolean; resulting empty returns; invalid name/number returns, no retry | strip leading@ for name; exact monster name; number0..2767 →20000+n; previous -1→32767; `Send_activate_skill(MKEY_MIMICRY,0,spell,0,0,0)` | topline | monster_list file for name; [c-spell.c:670](../../../src/client/c-spell.c#L670) |
```

### Original line 45

`capability.spells.choose-school`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `get_item_hook_find_spell`:891/936; `get_school_spell`:1087/1094 | text `Spell name?`, length79; existing item-select context with USE_EXTRA/USE_LIMIT, inventory+equip | cancel false/-1; old exact lookup; discrete supports partial multi-tier and strongest available duplicate by level | hack_force_spell and item; no packet itself | caller item/list | DISCRETE_SPELL_SYSTEM, ALLOW_DUPLICATE_NAMES, LIMIT; [c-spell.c:883](../../../src/client/c-spell.c#L883), [new:924](../../../src/client/c-spell.c#L924), [book:1085](../../../src/client/c-spell.c#L1085) |
```

### Original line 46

`capability.spells.mimic-power`, `capability.spells.runecraft`, `capability.spells.breath`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `do_mimic`:845; `do_runecraft`:1930; `do_breath`:2132 | `input.command.direction` | cancel returns no skill packet | uses_dir metadata / rcraft_dir gate / breath always; selected dir in typed skill intent | topline then parent | mimic server>4.4.5.10 and MIMIC_LUA metadata; [c-spell.c:825](../../../src/client/c-spell.c#L825), [breath:2129](../../../src/client/c-spell.c#L2129) |
```

### Original line 48

`capability.items.server-item-answer`, `capability.items.server-item-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_item`:3881/3925/3928 | `input.command.item-select`; Which item; different item_tester hooks/name prompts | cancel returns1 **without Send_item**; busy queues packet | immediate `Send_item(item)`; equip-only RUNE_ENCHANT; other USE_EXTRA/inven/equip + optional subinven | existing topline/item list | (!screen_icky&&!topline_icky) OR (ENABLE_SUBINVEN and using_subinven_item!=-1); ENABLE_SUBINVEN/version packet shape; [nclient.c:3831](../../../src/client/nclient.c#L3831), [prompt:3881](../../../src/client/nclient.c#L3881), [send:3929](../../../src/client/nclient.c#L3929) |
```

### Original line 49

`capability.items.server-spell-answer`, `capability.items.server-spell-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_spell_request` | school-select context | canceled return1 no Send_spell; busy queues | `Send_spell(item,spell)` | child list then original surface | !screen_icky&&!topline_icky; [nclient.c:3944](../../../src/client/nclient.c#L3944) |
```

### Original line 51

`capability.store.sell`, `capability.store.donate`, `capability.store.accept-offer`, `capability.store.decline-offer`, `capability.items.pickup-accept`, `capability.items.pickup-decline`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_sell`:5508; `Receive_pickup_check`:5963 | confirm default-no; donation/Accept price; runtime pickup prompt | declined no reply/command | sale →Send_store_confirm; no_verify_sell auto confirms; pickup →Send_stay | existing parent topline | store_num MATHOM; [nclient.c:5495](../../../src/client/nclient.c#L5495), [pickup:5955](../../../src/client/nclient.c#L5955) |
```

### Original line 64

Lua API delegates, not standalone outcomes: `capability.spells.cast-school`, `capability.spells.cancel-cast-school`. Preserve callback false/-1 behavior and binding reachability.

```text
| `toluaI_util_get_check200/300` | get_check2/get_check3 confirmation | wrapper pushes bool to Lua; no separate retry owner or packet | caller | [w_util.c:403](../../../src/client/w_util.c#L403), [427](../../../src/client/w_util.c#L427), declarations [util.pre:60](../../../src/client/util.pre#L60) |
```

### Original line 65

Lua API delegates, not standalone outcomes: `capability.spells.cast-school`, `capability.spells.cancel-cast-school`. Preserve callback false/-1 behavior and binding reachability.

```text
| `get_item_aux` and generated player wrappers | builds USE_EQUIP/INVEN/FLOOR mode; delegates c_get_item; other generated wrapper exposes c_get_item directly | bool plus cp returned; API exposure alone not a new user-visible state machine | caller item primitive | [lua_bind.c:357](../../../src/client/lua_bind.c#L357), [w_play.c:8708](../../../src/client/w_play.c#L8708), [8735](../../../src/client/w_play.c#L8735) |
```

### Original line 66

`capability.spells.cast-school`, `capability.spells.cancel-cast-school`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `pre_exec_spell_extra` → spell extra callback | confirmation primitive, concrete DIVINE extra asks `Cast on yourself?`, default-no | s_aux returns TRUE whenever extra callback exists, after assigning __pre_exec_extra=extra(); skills then assigns aux from that value (false numeric confirmation result becomes0). Only absent extra callback preserves prior aux (initialized0 or LIMIT_SPELLS-derived). Send_activate_skill still follows; callback false is not cast cancel | nested spell owner | [skills.c:1035](../../../src/client/skills.c#L1035), [s_aux.lua:1104](../../../lib/scpt/s_aux.lua#L1104), [s_divin.lua:12](../../../lib/scpt/s_divin.lua#L12) |
```

### Original line 67

`capability.spells.cast-school`, `capability.spells.cancel-cast-school`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `pre_exec_spell_item` | runtime get_item.prompt/mode/hook, get_item_aux | skills updates item_obj only true; canceled keeps -1; Send_activate_skill still follows | nested item primitive | [skills.c:1040](../../../src/client/skills.c#L1040), [s_aux.lua:1111](../../../lib/scpt/s_aux.lua#L1111) |
```

### Original line 68

`capability.spells.cast-school`, `capability.spells.cancel-cast-school`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `pre_exec_spell_dir`, `rcraft_*` | metadata controls native direction/composition contexts already above | direction false means no prompt; cancel real get_dir aborts native cast | native caller | [skills.c:1030](../../../src/client/skills.c#L1030), [s_aux.lua:1094](../../../lib/scpt/s_aux.lua#L1094) |
```

### Original line 69

Inactive/fallback API path: current bindings exist; m_aux is not loaded by c-init.lua. No missing shipped prompt outcome; exposed custom Lua APIs remain ticket 12.

```text
| `s_aux` compatibility fallback; m_aux power_get_check2 | older binding fallback calls get_check/power_get_check if absent; current generated get_check2 exists | current fallback not activated; m_aux not loaded by c-init path; commented mana warning not active | n/a | [s_aux.lua:16](../../../lib/scpt/s_aux.lua#L16), [m_aux.lua:8](../../../lib/scpt/m_aux.lua#L8) |
```

Mapped/dispositioned rows: 22.

## Registered incoming paths and all wire variants

Original: `docs/research/single-window-packet-state.md`; SHA-256 `1e3bcbb6eb15dcd412e206c721a11d85cae5fc8a0d5094430e632f84836db84d`.

### Original line 27

`capability.items.read-inventory`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 30 | `PKT_INVEN` ([src/common/pack.h:53](../../src/common/pack.h#L53)) | `always`; [binding](../../src/client/nclient.c#L356) | `Receive_inven` — inventory slot |
```

### Original line 28

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 31 | `PKT_EQUIP` ([src/common/pack.h:54](../../src/common/pack.h#L54)) | `always`; [binding](../../src/client/nclient.c#L357) | `Receive_equip` — equipment slot |
```

### Original line 51

`capability.items.server-item-answer`, `capability.items.server-item-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 61 | `PKT_ITEM` ([src/common/pack.h:88](../../src/common/pack.h#L88)) | `always`; [binding](../../src/client/nclient.c#L380) | `Receive_item` — server item-choice request |
```

### Original line 52

`capability.items.server-spell-answer`, `capability.items.server-spell-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 79 | `PKT_SPELL` ([src/common/pack.h:108](../../src/common/pack.h#L108)) | `always`; [binding](../../src/client/nclient.c#L381) | `Receive_spell_request` — server spell-choice request |
```

### Original line 53

`capability.spells.read-metadata`, `capability.spells.cast-legacy`, `capability.spells.ghost`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 48 | `PKT_SPELL_INFO` ([src/common/pack.h:72](../../src/common/pack.h#L72)) | `always`; [binding](../../src/client/nclient.c#L382) | `Receive_spell_info` — legacy spell metadata/formatted line |
```

### Original line 54

`capability.direction.answer-server`, `capability.direction.cancel-server`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 60 | `PKT_DIRECTION` ([src/common/pack.h:87](../../src/common/pack.h#L87)) | `always`; [binding](../../src/client/nclient.c#L383) | `Receive_direction` — direction request |
```

### Original line 55

`capability.world.clear-command-buffer`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 150 | `PKT_FLUSH` ([src/common/pack.h:192](../../src/common/pack.h#L192)) | `always`; [binding](../../src/client/nclient.c#L384) | `Receive_flush` — input flush control |
```

### Original line 57

`capability.items.details-read`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 50 | `PKT_SPECIAL_OTHER` ([src/common/pack.h:75](../../src/common/pack.h#L75)) | `always`; [binding](../../src/client/nclient.c#L386) | `Receive_special_other` — special-screen control |
```

### Original line 58

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 51 | `PKT_STORE` ([src/common/pack.h:76](../../src/common/pack.h#L76)) | `always`; [binding](../../src/client/nclient.c#L387) | `Receive_store` — store item slot |
```

### Original line 59

`capability.store.enter`, `capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 52 | `PKT_STORE_INFO` ([src/common/pack.h:77](../../src/common/pack.h#L77)) | `always`; [binding](../../src/client/nclient.c#L388) | `Receive_store_info` — store identity/owner/capacity/theme |
```

### Original line 60

`capability.store.sell`, `capability.store.donate`, `capability.store.accept-offer`, `capability.store.decline-offer`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 62 | `PKT_SELL` ([src/common/pack.h:89](../../src/common/pack.h#L89)) | `always`; [binding](../../src/client/nclient.c#L389) | `Receive_sell` — sell price prompt |
```

### Original line 61

`capability.target.read-description`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 53 | `PKT_TARGET_INFO` ([src/common/pack.h:78](../../src/common/pack.h#L78)) | `always`; [binding](../../src/client/nclient.c#L390) | `Receive_target_info` — target coordinates/description |
```

### Original line 64

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 64 | `PKT_SPECIAL_LINE` ([src/common/pack.h:91](../../src/common/pack.h#L91)) | `always`; [binding](../../src/client/nclient.c#L393) | `Receive_special_line` — special-screen formatted line |
```

### Original line 65

`capability.items.read-floor`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 49 | `PKT_FLOOR` ([src/common/pack.h:73](../../src/common/pack.h#L73)) | `always`; [binding](../../src/client/nclient.c#L394) | `Receive_floor` — floor-item presence/type |
```

### Original line 66

`capability.items.pickup-accept`, `capability.items.pickup-decline`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 56 | `PKT_PICKUP_CHECK` ([src/common/pack.h:81](../../src/common/pack.h#L81)) | `always`; [binding](../../src/client/nclient.c#L395) | `Receive_pickup_check` — pickup confirmation prompt |
```

### Original line 71

`capability.skills.read-tree`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 57 | `PKT_SKILLS` ([src/common/pack.h:82](../../src/common/pack.h#L82)) | `always`; [binding](../../src/client/nclient.c#L400) | `Receive_skills` — skill values batch |
```

### Original line 75

`capability.skills.read-tree`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 154 | `PKT_SKILL_INIT` ([src/common/pack.h:196](../../src/common/pack.h#L196)) | `always`; [binding](../../src/client/nclient.c#L404) | `Receive_skill_init` — skill metadata row |
```

### Original line 76

`capability.skills.read-tree`, `capability.skills.raise`, `capability.skills.develop`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 65 | `PKT_SKILL_MOD` ([src/common/pack.h:92](../../src/common/pack.h#L92)) | `always`; [binding](../../src/client/nclient.c#L405) | `Receive_skill_info` — skill value/mod/development flags |
```

### Original line 77

`capability.skills.read-tree`, `capability.skills.raise`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 160 | `PKT_SKILL_PTS` ([src/common/pack.h:204](../../src/common/pack.h#L204)) | `always`; [binding](../../src/client/nclient.c#L406) | `Receive_skill_points` — unspent skill points |
```

### Original line 78

`capability.store.kicked`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 108 | `PKT_STORE_LEAVE` ([src/common/pack.h:139](../../src/common/pack.h#L139)) | `always`; [binding](../../src/client/nclient.c#L407) | `Receive_store_kick` — store-close control |
```

### Original line 80

`capability.store.read-actions`, `capability.store.service`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 158 | `PKT_BACT` ([src/common/pack.h:200](../../src/common/pack.h#L200)) | `always`; [binding](../../src/client/nclient.c#L409) | `Receive_store_action` — store action row |
```

### Original line 88

`capability.spells.read-metadata`, `capability.spells.melee-technique`, `capability.spells.ranged-technique`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 173 | `PKT_TECHNIQUE_INFO` ([src/common/pack.h:225](../../src/common/pack.h#L225)) | `always`; [binding](../../src/client/nclient.c#L418) | `Receive_technique_info` — melee/ranged technique bitsets |
```

### Original line 90

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 175 | `PKT_INVEN_WIDE` ([src/common/pack.h:227](../../src/common/pack.h#L227)) | `always`; [binding](../../src/client/nclient.c#L420) | `Receive_inven_wide` — inventory slot + powers/bonuses |
```

### Original line 93

`capability.items.read-inventory`, `capability.items.select-slot`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 178 | `PKT_INVENTORY_REV` ([src/common/pack.h:230](../../src/common/pack.h#L230)) | `always`; [binding](../../src/client/nclient.c#L423) | `Receive_inventory_revision` — inventory revision counter |
```

### Original line 95

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 181 | `PKT_STORE_WIDE` ([src/common/pack.h:233](../../src/common/pack.h#L233)) | `always`; [binding](../../src/client/nclient.c#L425) | `Receive_store_wide` — store item slot + powers/bonuses |
```

### Original line 98

`capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 135 | `PKT_AUTOINSCRIBE` ([src/common/pack.h:171](../../src/common/pack.h#L171)) | `always`; [binding](../../src/client/nclient.c#L428) | `Receive_apply_auto_insc` — auto-inscription application request |
```

### Original line 99

`capability.items.read-newest`, `capability.items.select-slot`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 145 | `PKT_ITEM_NEWEST` ([src/common/pack.h:184](../../src/common/pack.h#L184)) | `always`; [binding](../../src/client/nclient.c#L430) | `Receive_item_newest` — latest-item index |
```

### Original line 100

`capability.items.read-newest`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 212 | `PKT_ITEM_NEWEST_2ND` ([src/common/pack.h:272](../../src/common/pack.h#L272)) | `always`; [binding](../../src/client/nclient.c#L431) | `Receive_item_newest_2nd` — secondary latest-item index |
```

### Original line 101

`capability.world.interrupt-repeat`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 146 | `PKT_CONFIRM` ([src/common/pack.h:185](../../src/common/pack.h#L185)) | `always`; [binding](../../src/client/nclient.c#L432) | `Receive_confirm` — sync confirmation token |
```

### Original line 107

Ticket 10: generic amount request and owner-specific server consumer, zero on cancellation. Any C service that uses it must take this child at C, never borrow generic key acceptance.

```text
| 185 | `PKT_REQUEST_AMT` ([src/common/pack.h:239](../../src/common/pack.h#L239)) | `always`; [binding](../../src/client/nclient.c#L440) | `Receive_request_amt` — bounded amount request |
```

### Original line 108

Ticket 10: generic bounded numeric request, predef/min/max and zero cancellation; required C child advances with its caller.

```text
| 218 | `PKT_REQUEST_NUM` ([src/common/pack.h:280](../../src/common/pack.h#L280)) | `always`; [binding](../../src/client/nclient.c#L441) | `Receive_request_num` — bounded numeric request |
```

### Original line 109

Ticket 10: request-specific string contracts (including item order/mail/owner), ESC sentinel is not local no-send. Required C service child advances with caller.

```text
| 186 | `PKT_REQUEST_STR` ([src/common/pack.h:240](../../src/common/pack.h#L240)) | `always`; [binding](../../src/client/nclient.c#L442) | `Receive_request_str` — string request |
```

### Original line 110

Ticket 10: generic default/strict confirmation and server consumer; required C child advances with caller.

```text
| 187 | `PKT_REQUEST_CFR` ([src/common/pack.h:241](../../src/common/pack.h#L241)) | `always`; [binding](../../src/client/nclient.c#L443) | `Receive_request_cfr` — confirmation request |
```

### Original line 111

`capability.request.cancel-key`, `capability.store.kicked`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 188 | `PKT_REQUEST_ABORT` ([src/common/pack.h:242](../../src/common/pack.h#L242)) | `always`; [binding](../../src/client/nclient.c#L444) | `Receive_request_abort` — request cancellation |
```

### Original line 112

Ticket 10 / D: special-store positioned strings, not ordinary keyed stock (nclient.c Receive_store_special_str). Reuse store metadata/action/leave IDs; do not claim a Go/casino canvas through store.read-stock.

```text
| 189 | `PKT_STORE_SPECIAL_STR` ([src/common/pack.h:243](../../src/common/pack.h#L243)) | `always`; [binding](../../src/client/nclient.c#L445) | `Receive_store_special_str` — positioned store text |
```

### Original line 113

Ticket 10 / D: special-store attributed glyph writes, not an item row (nclient.c Receive_store_special_char).

```text
| 190 | `PKT_STORE_SPECIAL_CHAR` ([src/common/pack.h:244](../../src/common/pack.h#L244)) | `always`; [binding](../../src/client/nclient.c#L446) | `Receive_store_special_char` — positioned store glyph |
```

### Original line 114

Ticket 10 / D: special-store region and force-clear canvas operations (nclient.c Receive_store_special_clr).

```text
| 191 | `PKT_STORE_SPECIAL_CLR` ([src/common/pack.h:245](../../src/common/pack.h#L245)) | `always`; [binding](../../src/client/nclient.c#L447) | `Receive_store_special_clr` — store region clear |
```

### Original line 115

Ticket 10 / D: ordered casino animation operations/resources; redraw must not replay effects (nclient.c Receive_store_special_anim).

```text
| 217 | `PKT_STORE_SPECIAL_ANIM` ([src/common/pack.h:279](../../src/common/pack.h#L279)) | `always`; [binding](../../src/client/nclient.c#L448) | `Receive_store_special_anim` — casino animation command |
```

### Original line 119

`capability.spells.read-metadata`, `capability.spells.mimic-power`, `capability.spells.mimic-form`, `capability.spells.mimic-immunity`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 139 | `PKT_POWERS_INFO` ([src/common/pack.h:175](../../src/common/pack.h#L175)) | `always`; [binding](../../src/client/nclient.c#L453) | `Receive_powers_info` — mimic power bitsets |
```

### Original line 125

`capability.items.read-floor`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 201 | `PKT_WHATS_UNDER_YOUR_FEET` ([src/common/pack.h:257](../../src/common/pack.h#L257)) | `always`; [binding](../../src/client/nclient.c#L460) | `Receive_whats_under_you_feet` — floor description/visibility |
```

### Original line 127

`capability.items.read-bag`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 204 | `PKT_SI_MOVE` ([src/common/pack.h:263](../../src/common/pack.h#L263)) | `ifdef ENABLE_SUBINVEN`; [binding](../../src/client/nclient.c#L464) | `Receive_subinven` — subinventory slot |
```

### Original line 128

`capability.items.details-navigate`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 206 | `PKT_SPECIAL_LINE_POS` ([src/common/pack.h:266](../../src/common/pack.h#L266)) | `always`; [binding](../../src/client/nclient.c#L466) | `Receive_special_line_pos` — special-screen cursor line |
```

### Original line 130

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 209 | `PKT_EQUIP_WIDE` ([src/common/pack.h:269](../../src/common/pack.h#L269)) | `always`; [binding](../../src/client/nclient.c#L468) | `Receive_equip_wide` — equipment slot + powers/bonuses |
```

### Original line 133

`capability.items.cancel-selection`, `capability.skills.cancel-ability`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| 219 | `PKT_MACRO_FAILURE` ([src/common/pack.h:281](../../src/common/pack.h#L281)) | `always`; [binding](../../src/client/nclient.c#L471) | `Receive_macro_failure` — macro-failure event |
```

### Original line 142

Declaration only, no registration/implementation (netclient.h); active mimic data is Receive_powers_info/Receive_spell_info, not a missing capability.

```text
| `Receive_mimic` | declaration only; no implementation or registration (dead API residue) | [src/client/netclient.h:37](../../src/client/netclient.h#L37) |
```

### Original line 184

`capability.items.read-inventory`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVEN` → `Receive_inven` step 1 | `%c%c%c%hu%hd%c%c%hd%hd%c%I` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name1:i16be, uses_dir:i8/raw byte, name:NUL string<ONAME_LEN> | > 4.5.2.0.0.0 | slot outside `a..x` returns 0 after decode (ambiguous partial-progress path) ([validation](../../src/client/nclient.c#L2342)) | inventory slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `inventory[]` and `inventory_name[]`; server markers stripped from `name` are lost ([handler](../../src/client/nclient.c#L2317)) | none | [src/client/nclient.c:2329](../../src/client/nclient.c#L2329) |
```

### Original line 185

`capability.items.read-inventory`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVEN` → `Receive_inven` step 2 | `%c%c%c%hu%hd%c%c%hd%c%I` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, uses_dir:i8/raw byte, name:NUL string<ONAME_LEN> | > 4.4.5.10.0.0 | slot outside `a..x` returns 0 after decode (ambiguous partial-progress path) ([validation](../../src/client/nclient.c#L2342)) | inventory slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `inventory[]` and `inventory_name[]`; server markers stripped from `name` are lost ([handler](../../src/client/nclient.c#L2317)) | none | [src/client/nclient.c:2332](../../src/client/nclient.c#L2332) |
```

### Original line 186

`capability.items.read-inventory`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVEN` → `Receive_inven` step 3 | `%c%c%c%hu%hd%c%c%hd%I` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name:NUL string<ONAME_LEN> | > 4.4.4.2.0.0 | slot outside `a..x` returns 0 after decode (ambiguous partial-progress path) ([validation](../../src/client/nclient.c#L2342)) | inventory slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `inventory[]` and `inventory_name[]`; server markers stripped from `name` are lost ([handler](../../src/client/nclient.c#L2317)) | none | [src/client/nclient.c:2335](../../src/client/nclient.c#L2335) |
```

### Original line 187

`capability.items.read-inventory`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVEN` → `Receive_inven` step 4 | `%c%c%c%hu%hd%c%c%hd%s` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name:NUL string<MAX_CHARS> | older | slot outside `a..x` returns 0 after decode (ambiguous partial-progress path) ([validation](../../src/client/nclient.c#L2342)) | inventory slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `inventory[]` and `inventory_name[]`; server markers stripped from `name` are lost ([handler](../../src/client/nclient.c#L2317)) | none | [src/client/nclient.c:2338](../../src/client/nclient.c#L2338) |
```

### Original line 188

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_EQUIP` → `Receive_equip` step 1 | `%c%c%c%hu%hd%c%c%hd%hd%c%I` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name1:i16be, uses_dir:i8/raw byte, name:NUL string<ONAME_LEN> | > 4.5.2.0.0.0 | slot outside `a..n` returns 0 after decode ([validation](../../src/client/nclient.c#L2674)) | equipment slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized equipment in `inventory[]`; literal `(nothing)/(unavailable)`, markers and packed `uses_dir` provenance are transformed/lost ([handler](../../src/client/nclient.c#L2649)) | none | [src/client/nclient.c:2661](../../src/client/nclient.c#L2661) |
```

### Original line 189

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_EQUIP` → `Receive_equip` step 2 | `%c%c%c%hu%hd%c%c%hd%c%I` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, uses_dir:i8/raw byte, name:NUL string<ONAME_LEN> | > 4.4.5.10.0.0 | slot outside `a..n` returns 0 after decode ([validation](../../src/client/nclient.c#L2674)) | equipment slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized equipment in `inventory[]`; literal `(nothing)/(unavailable)`, markers and packed `uses_dir` provenance are transformed/lost ([handler](../../src/client/nclient.c#L2649)) | none | [src/client/nclient.c:2664](../../src/client/nclient.c#L2664) |
```

### Original line 190

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_EQUIP` → `Receive_equip` step 3 | `%c%c%c%hu%hd%c%c%hd%I` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name:NUL string<ONAME_LEN> | > 4.4.4.2.0.0 | slot outside `a..n` returns 0 after decode ([validation](../../src/client/nclient.c#L2674)) | equipment slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized equipment in `inventory[]`; literal `(nothing)/(unavailable)`, markers and packed `uses_dir` provenance are transformed/lost ([handler](../../src/client/nclient.c#L2649)) | none | [src/client/nclient.c:2667](../../src/client/nclient.c#L2667) |
```

### Original line 191

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_EQUIP` → `Receive_equip` step 4 | `%c%c%c%hu%hd%c%c%hd%s` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name:NUL string<MAX_CHARS> | older | slot outside `a..n` returns 0 after decode ([validation](../../src/client/nclient.c#L2674)) | equipment slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized equipment in `inventory[]`; literal `(nothing)/(unavailable)`, markers and packed `uses_dir` provenance are transformed/lost ([handler](../../src/client/nclient.c#L2649)) | none | [src/client/nclient.c:2670](../../src/client/nclient.c#L2670) |
```

### Original line 231

`capability.items.server-item-answer`, `capability.items.server-item-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_ITEM` → `Receive_item` step 1 | `%c%c` ⇒ ch:i8/raw byte, th:i8/raw byte | > 4.5.2.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | server item-choice request; modal request; replace pending, response/abort clears | none; retain decoded fields ([handler](../../src/client/nclient.c#L3831)) | `Send_item`, `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:3836](../../src/client/nclient.c#L3836) |
```

### Original line 232

`capability.items.server-item-answer`, `capability.items.server-item-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_ITEM` → `Receive_item` step 2 | `%c` ⇒ ch:i8/raw byte | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | server item-choice request; modal request; replace pending, response/abort clears | none; retain decoded fields ([handler](../../src/client/nclient.c#L3831)) | `Send_item`, `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:3838](../../src/client/nclient.c#L3838) |
```

### Original line 233

`capability.items.server-spell-answer`, `capability.items.server-spell-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SPELL` → `Receive_spell_request` step 1 | `%c%d` ⇒ ch:i8/raw byte, item:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | server spell-choice request; modal request; replace pending, response/abort clears | none; retain decoded fields ([handler](../../src/client/nclient.c#L3941)) | `Send_spell`, `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:3945](../../src/client/nclient.c#L3945) |
```

### Original line 234

`capability.spells.read-metadata`, `capability.spells.cast-legacy`, `capability.spells.ghost`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SPELL_INFO` → `Receive_spell_info` step 1 | `%c%d%d%d%hu%hu%hu%s` ⇒ ch:i8/raw byte, spells[0]:i32be, spells[1]:i32be, spells[2]:i32be, realm:u16be, book:u16be, line:u16be, buf:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | legacy spell metadata/formatted line; latest snapshot; replace on packet, connection reset owns clear | retains spell bitsets and `spell_info[realm][book][line]`; preserve raw bounded string too ([handler](../../src/client/nclient.c#L3958)) | none | [src/client/nclient.c:3965](../../src/client/nclient.c#L3965) |
```

### Original line 235

`capability.direction.answer-server`, `capability.direction.cancel-server`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_DIRECTION` → `Receive_direction` step 1 | `%c` ⇒ ch:i8/raw byte | always | user cancellation returns 0 after packet consumption; busy UI queues request ([validation](../../src/client/nclient.c#L4018)) | direction request; modal request; replace pending, response/abort clears | none; retain decoded fields ([handler](../../src/client/nclient.c#L4012)) | `Packet_printf(wbuf/qbuf)` | [src/client/nclient.c:4016](../../src/client/nclient.c#L4016) |
```

### Original line 236

`capability.world.clear-command-buffer`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_FLUSH` → `Receive_flush` step 1 | `%c` ⇒ ch:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | input flush control; event/command; consume once | none; retain decoded fields ([handler](../../src/client/nclient.c#L4029)) | none | [src/client/nclient.c:4033](../../src/client/nclient.c#L4033) |
```

### Original line 249

`capability.items.details-read`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SPECIAL_OTHER` → `Receive_special_other` step 1 | `%c` ⇒ ch:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | special-screen control; event/command; consume once | none; retain decoded fields ([handler](../../src/client/nclient.c#L4495)) | none | [src/client/nclient.c:4499](../../src/client/nclient.c#L4499) |
```

### Original line 250

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE` → `Receive_store` step 1 | `%c%c%c%hd%hd%d%S%c%c%d%s` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:i16be, num:i16be, price:i32be, name:NUL string<MSG_LEN>, tval:i8/raw byte, sval:i8/raw byte, pval:i32be, powers:NUL string<MAX_CHARS> | >= 4.9.3.0.0.3 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store item slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized store slot/name; exact formatted name/powers representation is not separately retained ([handler](../../src/client/nclient.c#L4548)) | none | [src/client/nclient.c:4556](../../src/client/nclient.c#L4556) |
```

### Original line 251

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE` → `Receive_store` step 2 | `%c%c%c%hd%hd%d%S%c%c%hd%s` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:i16be, num:i16be, price:i32be, name:NUL string<MSG_LEN>, tval:i8/raw byte, sval:i8/raw byte, pval16b:i16be, powers:NUL string<MAX_CHARS> | >= 4.7.3.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store item slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized store slot/name; exact formatted name/powers representation is not separately retained ([handler](../../src/client/nclient.c#L4548)) | none | [src/client/nclient.c:4559](../../src/client/nclient.c#L4559) |
```

### Original line 252

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE` → `Receive_store` step 3 | `%c%c%c%hd%hd%d%S%c%c%hd` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:i16be, num:i16be, price:i32be, name:NUL string<MSG_LEN>, tval:i8/raw byte, sval:i8/raw byte, pval16b:i16be | > 4.4.7.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store item slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized store slot/name; exact formatted name/powers representation is not separately retained ([handler](../../src/client/nclient.c#L4548)) | none | [src/client/nclient.c:4563](../../src/client/nclient.c#L4563) |
```

### Original line 253

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE` → `Receive_store` step 4 | `%c%c%c%hd%hd%d%s%c%c%hd` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:i16be, num:i16be, price:i32be, name:NUL string<MAX_CHARS>, tval:i8/raw byte, sval:i8/raw byte, pval16b:i16be | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store item slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized store slot/name; exact formatted name/powers representation is not separately retained ([handler](../../src/client/nclient.c#L4548)) | none | [src/client/nclient.c:4567](../../src/client/nclient.c#L4567) |
```

### Original line 254

`capability.store.enter`, `capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE_INFO` → `Receive_store_info` step 1 | `%c%hd%s%s%hd%d%c%c%c` ⇒ ch:i8/raw byte, store_num:i16be, store_name:NUL string<MAX_CHARS>, owner_name:NUL string<MAX_CHARS>, num_items:i16be, max_cost:i32be, store_attr:i8/raw byte, store_char:i8/raw byte, store_price_mul:i8/raw byte | > 4.7.4.2.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store identity/owner/capacity/theme; latest snapshot; replace on packet, connection reset owns clear | retains normalized store globals; absent old fields default locally and exact provenance is lost ([handler](../../src/client/nclient.c#L5445)) | none | [src/client/nclient.c:5453](../../src/client/nclient.c#L5453) |
```

### Original line 255

`capability.store.enter`, `capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE_INFO` → `Receive_store_info` step 2 | `%c%hd%s%s%hd%d%c%c` ⇒ ch:i8/raw byte, store_num:i16be, store_name:NUL string<MAX_CHARS>, owner_name:NUL string<MAX_CHARS>, num_items:i16be, max_cost:i32be, store_attr:i8/raw byte, store_char:i8/raw byte | > 4.4.4.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store identity/owner/capacity/theme; latest snapshot; replace on packet, connection reset owns clear | retains normalized store globals; absent old fields default locally and exact provenance is lost ([handler](../../src/client/nclient.c#L5445)) | none | [src/client/nclient.c:5455](../../src/client/nclient.c#L5455) |
```

### Original line 256

`capability.store.enter`, `capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE_INFO` → `Receive_store_info` step 3 | `%c%hd%s%s%hd%d` ⇒ ch:i8/raw byte, store_num:i16be, store_name:NUL string<MAX_CHARS>, owner_name:NUL string<MAX_CHARS>, num_items:i16be, max_cost:i32be | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store identity/owner/capacity/theme; latest snapshot; replace on packet, connection reset owns clear | retains normalized store globals; absent old fields default locally and exact provenance is lost ([handler](../../src/client/nclient.c#L5445)) | none | [src/client/nclient.c:5457](../../src/client/nclient.c#L5457) |
```

### Original line 257

`capability.store.sell`, `capability.store.donate`, `capability.store.accept-offer`, `capability.store.decline-offer`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SELL` → `Receive_sell` step 1 | `%c%d` ⇒ ch:i8/raw byte, price:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | sell price prompt; modal request; replace pending, response/abort clears | none; retain decoded fields ([handler](../../src/client/nclient.c#L5496)) | `Send_store_confirm` | [src/client/nclient.c:5500](../../src/client/nclient.c#L5500) |
```

### Original line 258

`capability.target.read-description`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_TARGET_INFO` → `Receive_target_info` step 1 | `%c%c%c%S` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, buf:NUL string<MSG_LEN> | >= 4.9.0.1.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | target coordinates/description; latest snapshot; replace on packet, connection reset owns clear | passes fields into targeting display; retain coordinates and exact raw description explicitly ([handler](../../src/client/nclient.c#L5515)) | none | [src/client/nclient.c:5520](../../src/client/nclient.c#L5520) |
```

### Original line 259

`capability.target.read-description`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_TARGET_INFO` → `Receive_target_info` step 2 | `%c%c%c%s` ⇒ ch:i8/raw byte, x:i8/raw byte, y:i8/raw byte, buf:NUL string<MAX_CHARS> | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | target coordinates/description; latest snapshot; replace on packet, connection reset owns clear | passes fields into targeting display; retain coordinates and exact raw description explicitly ([handler](../../src/client/nclient.c#L5515)) | none | [src/client/nclient.c:5522](../../src/client/nclient.c#L5522) |
```

### Original line 277

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SPECIAL_LINE` → `Receive_special_line` step 1 | `%c%d%d%c%I` ⇒ ch:i8/raw byte, max:i32be, line:i32be, attr:i8/raw byte, buf:NUL string<ONAME_LEN> | > 4.4.7.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | special-screen formatted line; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | buffer may be stored/rendered; preserve exact raw line/attr/index because parsing mutates/annotates text ([handler](../../src/client/nclient.c#L5801)) | none | [src/client/nclient.c:5813](../../src/client/nclient.c#L5813) |
```

### Original line 278

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SPECIAL_LINE` → `Receive_special_line` step 2 | `%c%hd%hd%c%I` ⇒ ch:i8/raw byte, old_max:i16be, old_line:i16be, attr:i8/raw byte, buf:NUL string<ONAME_LEN> | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | special-screen formatted line; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | buffer may be stored/rendered; preserve exact raw line/attr/index because parsing mutates/annotates text ([handler](../../src/client/nclient.c#L5801)) | none | [src/client/nclient.c:5817](../../src/client/nclient.c#L5817) |
```

### Original line 279

`capability.items.read-floor`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_FLOOR` → `Receive_floor` step 1 | `%c%c` ⇒ ch:i8/raw byte, tval:i8/raw byte | always | no validation; payload intentionally ignored ([validation](../../src/client/nclient.c#L5948)) | floor-item presence/type; latest snapshot; replace on packet, connection reset owns clear | `tval` is explicitly ignored and lost ([handler](../../src/client/nclient.c#L5943)) | none | [src/client/nclient.c:5948](../../src/client/nclient.c#L5948) |
```

### Original line 280

`capability.items.pickup-accept`, `capability.items.pickup-decline`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_PICKUP_CHECK` → `Receive_pickup_check` step 1 | `%c%s` ⇒ ch:i8/raw byte, buf:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | pickup confirmation prompt; modal request; replace pending, response/abort clears | prompt is consumed synchronously and lost after optional `Send_stay` ([handler](../../src/client/nclient.c#L5956)) | `Send_stay` | [src/client/nclient.c:5960](../../src/client/nclient.c#L5960) |
```

### Original line 287

`capability.skills.read-tree`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SKILLS` → `Receive_skills` step 1 | `%c` ⇒ ch:i8/raw byte | always | incomplete one of 12 values rolls back already-read bytes and returns scanner result ([validation](../../src/client/nclient.c#L6159)) | skill values batch; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains 12 normalized skill values in `p_ptr`; packed blow/infra bits are normalized ([handler](../../src/client/nclient.c#L6150)) | none | [src/client/nclient.c:6155](../../src/client/nclient.c#L6155) |
```

### Original line 288

`capability.skills.read-tree`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SKILLS` → `Receive_skills` step 2 | `%hd` ⇒ tmp[i]:i16be | 12 sequential skill values; whole record rolls back if incomplete | incomplete one of 12 values rolls back already-read bytes and returns scanner result ([validation](../../src/client/nclient.c#L6159)) | skill values batch; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains 12 normalized skill values in `p_ptr`; packed blow/infra bits are normalized ([handler](../../src/client/nclient.c#L6150)) | none | [src/client/nclient.c:6161](../../src/client/nclient.c#L6161) |
```

### Original line 294

`capability.skills.read-tree`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SKILL_INIT` → `Receive_skill_init` step 1 | `%c%hd%hd%hd%hd%d%c%S%S%S` ⇒ ch:i8/raw byte, i:i16be, father:i16be, order:i16be, mkey:i16be, flags1:i32be, tval:i8/raw byte, name:NUL string<MSG_LEN>, desc:NUL string<MSG_LEN>, act:NUL string<MSG_LEN> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | skill metadata row; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains every field in `s_info[i]` allocated strings/scalars ([handler](../../src/client/nclient.c#L2996)) | none | [src/client/nclient.c:3005](../../src/client/nclient.c#L3005) |
```

### Original line 295

`capability.skills.read-tree`, `capability.skills.raise`, `capability.skills.develop`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SKILL_MOD` → `Receive_skill_info` step 1 | `%c%d%d%d%d%c%d` ⇒ ch:i8/raw byte, i:i32be, val:i32be, mod:i32be, dev:i32be, flags1:i8/raw byte, mkey:i32be | > 4.4.4.1.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | skill value/mod/development flags; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized fields in `p_ptr->s_info[i]`/`s_info[i]`; legacy hidden/dummy representation is normalized ([handler](../../src/client/nclient.c#L3046)) | none | [src/client/nclient.c:3054](../../src/client/nclient.c#L3054) |
```

### Original line 296

`capability.skills.read-tree`, `capability.skills.raise`, `capability.skills.develop`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SKILL_MOD` → `Receive_skill_info` step 2 | `%c%d%d%d%d%d%d%d` ⇒ ch:i8/raw byte, i:i32be, val:i32be, mod:i32be, dev:i32be, hidden:i32be, mkey:i32be, dummy:i32be | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | skill value/mod/development flags; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized fields in `p_ptr->s_info[i]`/`s_info[i]`; legacy hidden/dummy representation is normalized ([handler](../../src/client/nclient.c#L3046)) | none | [src/client/nclient.c:3058](../../src/client/nclient.c#L3058) |
```

### Original line 297

`capability.skills.read-tree`, `capability.skills.raise`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SKILL_PTS` → `Receive_skill_points` step 1 | `%c%d` ⇒ ch:i8/raw byte, pt:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | unspent skill points; latest snapshot; replace on packet, connection reset owns clear | none; retain decoded fields ([handler](../../src/client/nclient.c#L3031)) | none | [src/client/nclient.c:3036](../../src/client/nclient.c#L3036) |
```

### Original line 298

`capability.store.kicked`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE_LEAVE` → `Receive_store_kick` step 1 | `%c` ⇒ ch:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store-close control; event/command; consume once | none; retain decoded fields ([handler](../../src/client/nclient.c#L5479)) | none | [src/client/nclient.c:5483](../../src/client/nclient.c#L5483) |
```

### Original line 301

`capability.store.read-actions`, `capability.store.service`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_BACT` → `Receive_store_action` step 1 | `%c%c%hd%hd%s%c%c%d%u` ⇒ ch:i8/raw byte, pos:i8/raw byte, bact:i16be, action:i16be, name:NUL string<MAX_CHARS>, attr:i8/raw byte, letter:i8/raw byte, cost:i32be, flag:u32be | >= 4.9.3.0.0.3 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store action row; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `store_actions[pos]`; legacy cost/flag widths are normalized ([handler](../../src/client/nclient.c#L4510)) | none | [src/client/nclient.c:4520](../../src/client/nclient.c#L4520) |
```

### Original line 302

`capability.store.read-actions`, `capability.store.service`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_BACT` → `Receive_store_action` step 2 | `%c%c%hd%hd%s%c%c%hd%hu` ⇒ ch:i8/raw byte, pos:i8/raw byte, bact:i16be, action:i16be, name:NUL string<MAX_CHARS>, attr:i8/raw byte, letter:i8/raw byte, oldcost:i16be, oldflag:u16be | >= 4.9.2.1.0.1 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store action row; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `store_actions[pos]`; legacy cost/flag widths are normalized ([handler](../../src/client/nclient.c#L4510)) | none | [src/client/nclient.c:4522](../../src/client/nclient.c#L4522) |
```

### Original line 303

`capability.store.read-actions`, `capability.store.service`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_BACT` → `Receive_store_action` step 3 | `%c%c%hd%hd%s%c%c%hd%c` ⇒ ch:i8/raw byte, pos:i8/raw byte, bact:i16be, action:i16be, name:NUL string<MAX_CHARS>, attr:i8/raw byte, letter:i8/raw byte, oldcost:i16be, oldflag2:i8/raw byte | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store action row; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `store_actions[pos]`; legacy cost/flag widths are normalized ([handler](../../src/client/nclient.c#L4510)) | none | [src/client/nclient.c:4526](../../src/client/nclient.c#L4526) |
```

### Original line 313

`capability.spells.read-metadata`, `capability.spells.melee-technique`, `capability.spells.ranged-technique`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_TECHNIQUE_INFO` → `Receive_technique_info` step 1 | `%c%d%d` ⇒ ch:i8/raw byte, melee:i32be, ranged:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | melee/ranged technique bitsets; latest snapshot; replace on packet, connection reset owns clear | retains both bitsets ([handler](../../src/client/nclient.c#L3998)) | none | [src/client/nclient.c:4003](../../src/client/nclient.c#L4003) |
```

### Original line 315

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVEN_WIDE` → `Receive_inven_wide` step 1 | `%c%c%c%hu%hd%c%c%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%I%c` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name1:i16be, xtra1:i16be, xtra2:i16be, xtra3:i16be, xtra4:i16be, xtra5:i16be, xtra6:i16be, xtra7:i16be, xtra8:i16be, xtra9:i16be, name:NUL string<ONAME_LEN>, ident:i8/raw byte | > 4.7.1.1.0.0 | slot outside `a..x` returns 0 after decode ([validation](../../src/client/nclient.c#L2544)) | inventory slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `inventory[]` including xtra1..9; raw name markers and packed ident provenance are lost ([handler](../../src/client/nclient.c#L2499)) | none | [src/client/nclient.c:2514](../../src/client/nclient.c#L2514) |
```

### Original line 316

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVEN_WIDE` → `Receive_inven_wide` step 2 | `%c%c%c%hu%hd%c%c%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%I` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name1:i16be, xtra1:i16be, xtra2:i16be, xtra3:i16be, xtra4:i16be, xtra5:i16be, xtra6:i16be, xtra7:i16be, xtra8:i16be, xtra9:i16be, name:NUL string<ONAME_LEN> | > 4.7.0.0.0.0 | slot outside `a..x` returns 0 after decode ([validation](../../src/client/nclient.c#L2544)) | inventory slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `inventory[]` including xtra1..9; raw name markers and packed ident provenance are lost ([handler](../../src/client/nclient.c#L2499)) | none | [src/client/nclient.c:2518](../../src/client/nclient.c#L2518) |
```

### Original line 317

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVEN_WIDE` → `Receive_inven_wide` step 3 | `%c%c%c%hu%hd%c%c%hd%hd%c%c%c%c%c%c%c%c%c%I` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name1:i16be, xtra1b:i8/raw byte, xtra2b:i8/raw byte, xtra3b:i8/raw byte, xtra4b:i8/raw byte, xtra5b:i8/raw byte, xtra6b:i8/raw byte, xtra7b:i8/raw byte, xtra8b:i8/raw byte, xtra9b:i8/raw byte, name:NUL string<ONAME_LEN> | > 4.5.2.0.0.0 | slot outside `a..x` returns 0 after decode ([validation](../../src/client/nclient.c#L2544)) | inventory slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `inventory[]` including xtra1..9; raw name markers and packed ident provenance are lost ([handler](../../src/client/nclient.c#L2499)) | none | [src/client/nclient.c:2522](../../src/client/nclient.c#L2522) |
```

### Original line 318

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVEN_WIDE` → `Receive_inven_wide` step 4 | `%c%c%c%hu%hd%c%c%hd%c%c%c%c%c%c%c%c%c%I` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, xtra1b:i8/raw byte, xtra2b:i8/raw byte, xtra3b:i8/raw byte, xtra4b:i8/raw byte, xtra5b:i8/raw byte, xtra6b:i8/raw byte, xtra7b:i8/raw byte, xtra8b:i8/raw byte, xtra9b:i8/raw byte, name:NUL string<ONAME_LEN> | > 4.4.4.2.0.0 | slot outside `a..x` returns 0 after decode ([validation](../../src/client/nclient.c#L2544)) | inventory slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `inventory[]` including xtra1..9; raw name markers and packed ident provenance are lost ([handler](../../src/client/nclient.c#L2499)) | none | [src/client/nclient.c:2529](../../src/client/nclient.c#L2529) |
```

### Original line 319

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVEN_WIDE` → `Receive_inven_wide` step 5 | `%c%c%c%hu%hd%c%c%hd%c%c%c%c%c%c%c%c%c%s` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, xtra1b:i8/raw byte, xtra2b:i8/raw byte, xtra3b:i8/raw byte, xtra4b:i8/raw byte, xtra5b:i8/raw byte, xtra6b:i8/raw byte, xtra7b:i8/raw byte, xtra8b:i8/raw byte, xtra9b:i8/raw byte, name:NUL string<MAX_CHARS> | older | slot outside `a..x` returns 0 after decode ([validation](../../src/client/nclient.c#L2544)) | inventory slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `inventory[]` including xtra1..9; raw name markers and packed ident provenance are lost ([handler](../../src/client/nclient.c#L2499)) | none | [src/client/nclient.c:2536](../../src/client/nclient.c#L2536) |
```

### Original line 323

`capability.items.read-inventory`, `capability.items.select-slot`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_INVENTORY_REV` → `Receive_inventory_revision` step 1 | `%c%d` ⇒ ch:i8/raw byte, revision:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | inventory revision counter; latest snapshot; replace on packet, connection reset owns clear | retains `p_ptr->inventory_revision` and acknowledges it ([handler](../../src/client/nclient.c#L6619)) | `Send_inventory_revision` | [src/client/nclient.c:6624](../../src/client/nclient.c#L6624) |
```

### Original line 325

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE_WIDE` → `Receive_store_wide` step 1 | `%c%c%c%hd%hd%d%S%c%c%d%hd%hd%hd%hd%hd%hd%hd%hd%hd` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:i16be, num:i16be, price:i32be, name:NUL string<MSG_LEN>, tval:i8/raw byte, sval:i8/raw byte, pval:i32be, xtra1:i16be, xtra2:i16be, xtra3:i16be, xtra4:i16be, xtra5:i16be, xtra6:i16be, xtra7:i16be, xtra8:i16be, xtra9:i16be | >= 4.9.3.0.0.3 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store item slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized store slot plus xtra1..9; raw formatted name/packed provenance lost ([handler](../../src/client/nclient.c#L4595)) | none | [src/client/nclient.c:4605](../../src/client/nclient.c#L4605) |
```

### Original line 326

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE_WIDE` → `Receive_store_wide` step 2 | `%c%c%c%hd%hd%d%S%c%c%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:i16be, num:i16be, price:i32be, name:NUL string<MSG_LEN>, tval:i8/raw byte, sval:i8/raw byte, pval16b:i16be, xtra1:i16be, xtra2:i16be, xtra3:i16be, xtra4:i16be, xtra5:i16be, xtra6:i16be, xtra7:i16be, xtra8:i16be, xtra9:i16be | > 4.7.0.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store item slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized store slot plus xtra1..9; raw formatted name/packed provenance lost ([handler](../../src/client/nclient.c#L4595)) | none | [src/client/nclient.c:4609](../../src/client/nclient.c#L4609) |
```

### Original line 327

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE_WIDE` → `Receive_store_wide` step 3 | `%c%c%c%hd%hd%d%S%c%c%hd%c%c%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:i16be, num:i16be, price:i32be, name:NUL string<MSG_LEN>, tval:i8/raw byte, sval:i8/raw byte, pval16b:i16be, xtra1b:i8/raw byte, xtra2b:i8/raw byte, xtra3b:i8/raw byte, xtra4b:i8/raw byte, xtra5b:i8/raw byte, xtra6b:i8/raw byte, xtra7b:i8/raw byte, xtra8b:i8/raw byte, xtra9b:i8/raw byte | > 4.4.7.0.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store item slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized store slot plus xtra1..9; raw formatted name/packed provenance lost ([handler](../../src/client/nclient.c#L4595)) | none | [src/client/nclient.c:4614](../../src/client/nclient.c#L4614) |
```

### Original line 328

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_STORE_WIDE` → `Receive_store_wide` step 4 | `%c%c%c%hd%hd%d%s%c%c%hd%c%c%c%c%c%c%c%c%c` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:i16be, num:i16be, price:i32be, name:NUL string<MAX_CHARS>, tval:i8/raw byte, sval:i8/raw byte, pval16b:i16be, xtra1b:i8/raw byte, xtra2b:i8/raw byte, xtra3b:i8/raw byte, xtra4b:i8/raw byte, xtra5b:i8/raw byte, xtra6b:i8/raw byte, xtra7b:i8/raw byte, xtra8b:i8/raw byte, xtra9b:i8/raw byte | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store item slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized store slot plus xtra1..9; raw formatted name/packed provenance lost ([handler](../../src/client/nclient.c#L4595)) | none | [src/client/nclient.c:4622](../../src/client/nclient.c#L4622) |
```

### Original line 337

`capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_AUTOINSCRIBE` → `Receive_apply_auto_insc` step 1 | `%c%c` ⇒ ch:i8/raw byte, slotc:i8/raw byte | < 4.9.2.1.0.1 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | auto-inscription application request; event/command; consume once | none; retain decoded fields ([handler](../../src/client/nclient.c#L2302)) | none | [src/client/nclient.c:2308](../../src/client/nclient.c#L2308) |
```

### Original line 338

`capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_AUTOINSCRIBE` → `Receive_apply_auto_insc` step 2 | `%c%hd` ⇒ ch:i8/raw byte, slot:i16be | >= 4.9.2.1.0.1 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | auto-inscription application request; event/command; consume once | none; retain decoded fields ([handler](../../src/client/nclient.c#L2302)) | none | [src/client/nclient.c:2311](../../src/client/nclient.c#L2311) |
```

### Original line 339

`capability.items.read-newest`, `capability.items.select-slot`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_ITEM_NEWEST` → `Receive_item_newest` step 1 | `%c%c` ⇒ ch:i8/raw byte, item:i8/raw byte | < 4.8.1.1.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | latest-item index; latest snapshot; replace on packet, connection reset owns clear | retains resolved `item_newest`; primary-vs-fallback provenance is collapsed ([handler](../../src/client/nclient.c#L7269)) | none | [src/client/nclient.c:7276](../../src/client/nclient.c#L7276) |
```

### Original line 340

`capability.items.read-newest`, `capability.items.select-slot`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_ITEM_NEWEST` → `Receive_item_newest` step 2 | `%c%d` ⇒ ch:i8/raw byte, item_newest:i32be | >= 4.8.1.1.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | latest-item index; latest snapshot; replace on packet, connection reset owns clear | retains resolved `item_newest`; primary-vs-fallback provenance is collapsed ([handler](../../src/client/nclient.c#L7269)) | none | [src/client/nclient.c:7279](../../src/client/nclient.c#L7279) |
```

### Original line 341

`capability.items.read-newest`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_ITEM_NEWEST_2ND` → `Receive_item_newest_2nd` step 1 | `%c%d` ⇒ ch:i8/raw byte, item_newest_2nd:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | secondary latest-item index; latest snapshot; replace on packet, connection reset owns clear | retains secondary and may replace primary based on item type ([handler](../../src/client/nclient.c#L7288)) | none | [src/client/nclient.c:7292](../../src/client/nclient.c#L7292) |
```

### Original line 342

`capability.world.interrupt-repeat`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_CONFIRM` → `Receive_confirm` step 1 | `%c%c` ⇒ ch:i8/raw byte, ch_confirmed:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | sync confirmation token; event/command; consume once | retains latest `command_confirmed`, overwriting earlier confirmation ([handler](../../src/client/nclient.c#L7326)) | none | [src/client/nclient.c:7330](../../src/client/nclient.c#L7330) |
```

### Original line 350

Ticket 10: generic amount request and owner-specific server consumer, zero on cancellation. Any C service that uses it must take this child at C, never borrow generic key acceptance.

```text
| `PKT_REQUEST_AMT` → `Receive_request_amt` step 1 | `%c%d%s%d` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS>, max:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | bounded amount request; modal request; replace pending, response/abort clears | request/prompt/max lost after synchronous response ([handler](../../src/client/nclient.c#L7190)) | `Send_request_amt` | [src/client/nclient.c:7194](../../src/client/nclient.c#L7194) |
```

### Original line 351

Ticket 10: generic bounded numeric request, predef/min/max and zero cancellation; required C child advances with its caller.

```text
| `PKT_REQUEST_NUM` → `Receive_request_num` step 1 | `%c%d%s%d%d%d` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS>, predef:i32be, min:i32be, max:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | bounded numeric request; modal request; replace pending, response/abort clears | request/prompt/default/min/max lost after synchronous response ([handler](../../src/client/nclient.c#L7202)) | `Send_request_num` | [src/client/nclient.c:7206](../../src/client/nclient.c#L7206) |
```

### Original line 352

Ticket 10: request-specific string contracts (including item order/mail/owner), ESC sentinel is not local no-send. Required C service child advances with caller.

```text
| `PKT_REQUEST_STR` → `Receive_request_str` step 1 | `%c%d%s%s` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS>, buf:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | string request; modal request; replace pending, response/abort clears | request/prompt/default lost after synchronous response ([handler](../../src/client/nclient.c#L7214)) | `Send_request_str` | [src/client/nclient.c:7218](../../src/client/nclient.c#L7218) |
```

### Original line 353

Ticket 10: generic default/strict confirmation and server consumer; required C child advances with caller.

```text
| `PKT_REQUEST_CFR` → `Receive_request_cfr` step 1 | `%c%d%s%c` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS>, dy:i8/raw byte | > 4.5.6.0.0.1 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | confirmation request; modal request; replace pending, response/abort clears | request/prompt/default lost after synchronous response ([handler](../../src/client/nclient.c#L7227)) | `Send_request_cfr` | [src/client/nclient.c:7235](../../src/client/nclient.c#L7235) |
```

### Original line 354

Ticket 10: generic default/strict confirmation and server consumer; required C child advances with caller.

```text
| `PKT_REQUEST_CFR` → `Receive_request_cfr` step 2 | `%c%d%s` ⇒ ch:i8/raw byte, id:i32be, prompt:NUL string<MAX_CHARS> | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | confirmation request; modal request; replace pending, response/abort clears | request/prompt/default lost after synchronous response ([handler](../../src/client/nclient.c#L7227)) | `Send_request_cfr` | [src/client/nclient.c:7238](../../src/client/nclient.c#L7238) |
```

### Original line 355

`capability.request.cancel-key`, `capability.store.kicked`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_REQUEST_ABORT` → `Receive_request_abort` step 1 | `%c` ⇒ ch:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | request cancellation; event/command; consume once | sets transient abort flag only when a request is pending ([handler](../../src/client/nclient.c#L7247)) | none | [src/client/nclient.c:7251](../../src/client/nclient.c#L7251) |
```

### Original line 356

Ticket 10 / D: special-store positioned strings, not ordinary keyed stock (nclient.c Receive_store_special_str). Reuse store metadata/action/leave IDs; do not claim a Go/casino canvas through store.read-stock.

```text
| `PKT_STORE_SPECIAL_STR` → `Receive_store_special_str` step 1 | `%c%c%c%c%s` ⇒ ch:i8/raw byte, line:i8/raw byte, col:i8/raw byte, attr:i8/raw byte, str:NUL string<MAX_CHARS> | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | positioned store text; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | positioned text is drawn only; line/col/attr/raw string lost ([handler](../../src/client/nclient.c#L4663)) | none | [src/client/nclient.c:4669](../../src/client/nclient.c#L4669) |
```

### Original line 357

Ticket 10 / D: special-store attributed glyph writes, not an item row (nclient.c Receive_store_special_char).

```text
| `PKT_STORE_SPECIAL_CHAR` → `Receive_store_special_char` step 1 | `%c%c%c%c%c` ⇒ ch:i8/raw byte, line:i8/raw byte, col:i8/raw byte, attr:i8/raw byte, c:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | positioned store glyph; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | positioned glyph is drawn only; all fields lost ([handler](../../src/client/nclient.c#L4691)) | none | [src/client/nclient.c:4697](../../src/client/nclient.c#L4697) |
```

### Original line 358

Ticket 10 / D: special-store region and force-clear canvas operations (nclient.c Receive_store_special_clr).

```text
| `PKT_STORE_SPECIAL_CLR` → `Receive_store_special_clr` step 1 | `%c%c%c` ⇒ ch:i8/raw byte, line_start:i8/raw byte, line_end:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | store region clear; latest snapshot; replace on packet, connection reset owns clear | clear range is executed only; range lost ([handler](../../src/client/nclient.c#L4712)) | none | [src/client/nclient.c:4716](../../src/client/nclient.c#L4716) |
```

### Original line 359

Ticket 10 / D: ordered casino animation operations/resources; redraw must not replay effects (nclient.c Receive_store_special_anim).

```text
| `PKT_STORE_SPECIAL_ANIM` → `Receive_store_special_anim` step 1 | `%c%hd%hd%hd%hd` ⇒ ch:i8/raw byte, anim1:i16be, anim2:i16be, anim3:i16be, anim4:i16be | always | unknown animation selector falls through legacy switch behavior; operands still consumed ([validation](../../src/client/nclient.c#L5061)) | casino animation command; event/command; consume once | animation operands are consumed by procedural drawing/sound and lost ([handler](../../src/client/nclient.c#L5045)) | none | [src/client/nclient.c:5061](../../src/client/nclient.c#L5061) |
```

### Original line 363

`capability.spells.read-metadata`, `capability.spells.mimic-power`, `capability.spells.mimic-form`, `capability.spells.mimic-immunity`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_POWERS_INFO` → `Receive_powers_info` step 1 | `%c%d%d%d%d` ⇒ ch:i8/raw byte, spells[0]:i32be, spells[1]:i32be, spells[2]:i32be, spells[3]:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | mimic power bitsets; latest snapshot; replace on packet, connection reset owns clear | retains all four innate-spell bitsets ([handler](../../src/client/nclient.c#L3979)) | none | [src/client/nclient.c:3984](../../src/client/nclient.c#L3984) |
```

### Original line 375

`capability.items.read-floor`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_WHATS_UNDER_YOUR_FEET` → `Receive_whats_under_you_feet` step 1 | `%c%c%c%c%I` ⇒ ch:i8/raw byte, crossmod_item:i8/raw byte, cant_see:i8/raw byte, on_pile:i8/raw byte, o_name:NUL string<ONAME_LEN> | >= 4.7.4.1.0.0 | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | floor description/visibility; latest snapshot; replace on packet, connection reset owns clear | retains only object name; visibility/pile flags are consumed and lost ([handler](../../src/client/nclient.c#L7477)) | none | [src/client/nclient.c:7484](../../src/client/nclient.c#L7484) |
```

### Original line 376

`capability.items.read-floor`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_WHATS_UNDER_YOUR_FEET` → `Receive_whats_under_you_feet` step 2 | `%c%c%c%c%s` ⇒ ch:i8/raw byte, crossmod_item:i8/raw byte, cant_see:i8/raw byte, on_pile:i8/raw byte, o_name:NUL string<MAX_CHARS> | older | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | floor description/visibility; latest snapshot; replace on packet, connection reset owns clear | retains only object name; visibility/pile flags are consumed and lost ([handler](../../src/client/nclient.c#L7477)) | none | [src/client/nclient.c:7486](../../src/client/nclient.c#L7486) |
```

### Original line 378

`capability.items.read-bag`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SI_MOVE` → `Receive_subinven` step 1 | `%c%c%c%c%hu%hd%c%c%hd%hd%c%I` ⇒ ch:i8/raw byte, iposc:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name1:i16be, uses_dir:i8/raw byte, name:NUL string<ONAME_LEN> | ENABLE_SUBINVEN | container outside `0..INVEN_PACK` or slot outside `a..x` returns 0 ([validation](../../src/client/nclient.c#L2427)) | subinventory slot; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | retains normalized `subinventory[][]`/name; raw markers are lost ([handler](../../src/client/nclient.c#L2411)) | none | [src/client/nclient.c:2422](../../src/client/nclient.c#L2422) |
```

### Original line 379

`capability.items.details-navigate`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_SPECIAL_LINE_POS` → `Receive_special_line_pos` step 1 | `%c%d` ⇒ ch:i8/raw byte, cur_line:i32be | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | special-screen cursor line; latest snapshot; replace on packet, connection reset owns clear | retains `cur_line` cursor ([handler](../../src/client/nclient.c#L5934)) | none | [src/client/nclient.c:5938](../../src/client/nclient.c#L5938) |
```

### Original line 381

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_EQUIP_WIDE` → `Receive_equip_wide` step 1 | `%c%c%c%hu%hd%c%c%hd%hd%c%I%hd%hd%hd%hd%hd%hd%hd%hd%hd` ⇒ ch:i8/raw byte, pos:i8/raw byte, attr:i8/raw byte, wgt:u16be, amt:i16be, tval:i8/raw byte, sval:i8/raw byte, pval:i16be, name1:i16be, uses_dir:i8/raw byte, name:NUL string<ONAME_LEN>, xtra1:i16be, xtra2:i16be, xtra3:i16be, xtra4:i16be, xtra5:i16be, xtra6:i16be, xtra7:i16be, xtra8:i16be, xtra9:i16be | always | slot outside `a..n` returns 0 after decode ([validation](../../src/client/nclient.c#L2747)) | equipment slot + powers/bonuses; keyed entry/batch; replace addressed entry; explicit empty/end packet clears or completes | same as equipment plus retained xtra1..9; raw name/packed provenance lost ([handler](../../src/client/nclient.c#L2730)) | none | [src/client/nclient.c:2742](../../src/client/nclient.c#L2742) |
```

### Original line 392

`capability.items.cancel-selection`, `capability.skills.cancel-ability`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `PKT_MACRO_FAILURE` → `Receive_macro_failure` step 1 | `%c` ⇒ ch:i8/raw byte | always | scanf <=0 returns unchanged result; Net_packet rollback/wait on 0, clear+disconnect on -1 | macro-failure event; event/command; consume once | event may flush macro state; occurrence is otherwise lost ([handler](../../src/client/nclient.c#L7514)) | none | [src/client/nclient.c:7518](../../src/client/nclient.c#L7518) |
```

### Original line 419

`capability.direction.answer-server`, `capability.direction.cancel-server`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_direction` | `Packet_printf(wbuf, `%c%c`)` | [src/client/nclient.c:4023](../../src/client/nclient.c#L4023) |
```

### Original line 420

`capability.direction.answer-server`, `capability.direction.cancel-server`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_direction` | `Packet_printf(qbuf, `%c`)` | [src/client/nclient.c:4024](../../src/client/nclient.c#L4024) |
```

### Original line 424

`capability.items.read-inventory`, `capability.items.select-slot`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_inventory_revision` | `Send_inventory_revision` | [src/client/nclient.c:6627](../../src/client/nclient.c#L6627) |
```

### Original line 425

`capability.items.server-item-answer`, `capability.items.server-item-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_item` | `Send_item` | [src/client/nclient.c:3882](../../src/client/nclient.c#L3882) |
```

### Original line 426

`capability.items.server-item-answer`, `capability.items.server-item-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_item` | `Send_item` | [src/client/nclient.c:3929](../../src/client/nclient.c#L3929) |
```

### Original line 427

`capability.items.server-item-answer`, `capability.items.server-item-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_item` | `Packet_printf(qbuf, `%c%c`)` | [src/client/nclient.c:3932](../../src/client/nclient.c#L3932) |
```

### Original line 428

`capability.items.server-item-answer`, `capability.items.server-item-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_item` | `Packet_printf(qbuf, `%c`)` | [src/client/nclient.c:3934](../../src/client/nclient.c#L3934) |
```

### Original line 429

`capability.items.pickup-accept`, `capability.items.pickup-decline`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_pickup_check` | `Send_stay` | [src/client/nclient.c:5965](../../src/client/nclient.c#L5965) |
```

### Original line 431

Ticket 10: generic amount request and owner-specific server consumer, zero on cancellation. Any C service that uses it must take this child at C, never borrow generic key acceptance.

```text
| `Receive_request_amt` | `Send_request_amt` | [src/client/nclient.c:7197](../../src/client/nclient.c#L7197) |
```

### Original line 432

Ticket 10: generic default/strict confirmation and server consumer; required C child advances with caller.

```text
| `Receive_request_cfr` | `Send_request_cfr` | [src/client/nclient.c:7242](../../src/client/nclient.c#L7242) |
```

### Original line 435

Ticket 10: generic bounded numeric request, predef/min/max and zero cancellation; required C child advances with its caller.

```text
| `Receive_request_num` | `Send_request_num` | [src/client/nclient.c:7209](../../src/client/nclient.c#L7209) |
```

### Original line 436

Ticket 10: request-specific string contracts (including item order/mail/owner), ESC sentinel is not local no-send. Required C service child advances with caller.

```text
| `Receive_request_str` | `Send_request_str` | [src/client/nclient.c:7221](../../src/client/nclient.c#L7221) |
```

### Original line 437

Ticket 10: request-specific string contracts (including item order/mail/owner), ESC sentinel is not local no-send. Required C service child advances with caller.

```text
| `Receive_request_str` | `Send_request_str` | [src/client/nclient.c:7222](../../src/client/nclient.c#L7222) |
```

### Original line 438

`capability.store.sell`, `capability.store.donate`, `capability.store.accept-offer`, `capability.store.decline-offer`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_sell` | `Send_store_confirm` | [src/client/nclient.c:5503](../../src/client/nclient.c#L5503) |
```

### Original line 439

`capability.store.sell`, `capability.store.donate`, `capability.store.accept-offer`, `capability.store.decline-offer`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_sell` | `Send_store_confirm` | [src/client/nclient.c:5509](../../src/client/nclient.c#L5509) |
```

### Original line 440

`capability.items.server-spell-answer`, `capability.items.server-spell-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_spell_request` | `Send_spell` | [src/client/nclient.c:3951](../../src/client/nclient.c#L3951) |
```

### Original line 441

`capability.items.server-spell-answer`, `capability.items.server-spell-cancel`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `Receive_spell_request` | `Packet_printf(qbuf, `%c%d`)` | [src/client/nclient.c:3953](../../src/client/nclient.c#L3953) |
```

### Original line 469

Declaration only, no registration/implementation (netclient.h); active mimic data is Receive_powers_info/Receive_spell_info, not a missing capability.

```text
| `Receive_mimic` | Header residue only: declaration exists, but no implementation/call/registration was found. | [src/client/netclient.h:37](../../src/client/netclient.h#L37) |
```

Mapped/dispositioned rows: 136.

## Client slash grammar and item substitutions

Original: `.scratch/single-window-sdl3-client/research/slash-command-grammar-and-dispatch.md`; SHA-256 `61041690124c847a56b5c5fb14023aa0d8699d595a071d0e93219484fbccf580`.

### Original line 31

`capability.items.assign-newest`, `capability.items.read-newest`, `capability.items.cancel-selection`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `/new` | CI exact; без args | показывает previous newest; child item-select `Which item?`, extra name lookup `Item name?`; USE_INVEN/EQUIP/EXTRA/SUBINVEN/UNPREFER_SUBINVEN; cancel/no valid i exits; только 0<=i<INVEN_TOTAL принимается; задаёт item_newest, optional redraw_newest | ENABLE_SUBINVEN только previous diagnostic; child selector допускает subinventory, owner отклоняет его encoded result; [8325](../../../src/client/c-cmd.c#L8325) |
```

### Original line 32

`capability.items.assign-newest`, `capability.items.read-newest`, `capability.items.cancel-selection`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `/new ` | CS prefix с одним space; пустой tail вызывает тот же selector | substring `help` anywhere (CS) OR first argument byte `?` → usage. Иначе используется только buf[5], tail игнорируется; lower maps inventory, upper maps equipment; byte >= 'a'+INVEN_WIELD-1 заменяется на Z; bounds и nonempty tval проверяются; item_newest+redraw | нет support двухбуквенного subinventory arg (TODO). `/NEW a` не этот branch; [8344](../../../src/client/c-cmd.c#L8344) |
```

### Original line 42

Ticket 10 / D owns casino wager query/default; ordinary store.service retains BACT_F_GOLD_MORE default semantics. Variable std_wager and c-store.c store_do_command are the actual owners; setting a default is not placing a bet.

```text
| `/wager` | CI prefix6; если buf[6]==0 report; иначе atol(buf+6), delimiter не обязателен | std_wager query/set; clamps >PY_MAX_GOLD to max, <1 to1 after assignment to s32b; no packet. `/wager10` →10, `/wager abc` →1, `/wager 10junk` →10; no-arg branch leaves inkey_msg TRUE | PY_MAX_GOLD=2000000000L; atol overflow/platform long width не safe parsing contract; [8565](../../../src/client/c-cmd.c#L8565), [constant:2434](../../../src/common/defines.h#L2434) |
```

### Original line 43

`capability.items.autoinscribe-all`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| `/apply_auto_inscriptions`, `/aai` | CI exact | apply_all_auto_inscriptions(), all inventory/equipment and conditional subinventory; downstream inscription/uninscription/server-autoinscription packets; нет confirmation | ENABLE_SUBINVEN, REGEX_SEARCH, autoinscription flags/rules; server autoinscribe version gate ниже; [8582](../../../src/client/c-cmd.c#L8582) |
```

### Original line 52

`capability.store.paste-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| два backslash + a..inventory limit, A..equipment limit, `_`, `+` | inventory/equipment name, floor description, newest name; newest=-1 даёт empty item; subinventory newest gated ENABLE_SUBINVEN. Colour `FFs`, restore `FF-`, spacing, colon escaping и size check; overflow branch discards shortcut | [8150](../../../src/client/c-cmd.c#L8150) |
```

### Original line 53

Ticket 10 chat composition owns the item-name substitutions; items.read-inventory/read-equipment/read-floor/read-newest are required data inputs. Preserve colon/color escaping and size checks before Send_msg; no separate item command is emitted.

```text
| три backslash + uppercase pack slot + lowercase subslot | subinventory name; эта substitution branch **не окружена ENABLE_SUBINVEN** в cmd_message; exact available-build declarations отдельно от этой lexical observation. Overflow cleanup использует i+3, хотя consumed shortcut длиной5 | [8211](../../../src/client/c-cmd.c#L8211) |
```

### Original line 54

`capability.store.paste-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| три backslash + lower store slot | store_top+slot; a..l normally, a..z screen_hgt==MAX_SCREEN_HGT. store_paste_item/where; location only once/message, colon escaping. store stock/size failure discards shortcut | [8249](../../../src/client/c-cmd.c#L8249), [big_shop:8112](../../../src/client/c-cmd.c#L8112) |
```

Mapped/dispositioned rows: 7.

## Text fields, editor sites and string packet calls

Original: `.scratch/single-window-sdl3-client/research/source-text-and-server-field-byte-contracts.md`; SHA-256 `71585ae2b09367c05ef9ba4b25d24db218fd6f5f430f19231e7efae1d6da0fc3`.

### Original line 51

`capability.items.inscribe`, `capability.items.cancel-inscribe`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| Item inscription | Two E=59 prompts; wire `%s`80; server INSCR_LEN80 clamps79, semantic inscription tokens downstream; autoinscription tags are a separate producer | Prompt Esc no packet; item validity/replay may reject. Item object/quark/save owns persistence; [c-cmd.c:1700](../../../src/client/c-cmd.c#L1700), [nserver.c:13325](../../../src/server/nserver.c#L13325) |
```

### Original line 71

Ticket 10 owns this request-specific server string consumer. Ordinary service may not claim it via generic request evidence; instantiate the relevant child at C before acceptance. Preserve the row’s transforms, ESC handling and exact byte limits.

```text
| `RID_ITEM_ORDER` / ENABLE_ITEM_ORDER | Case-insensitive `cancel` has explicit active-order behavior; clip40, trim spaces, collapse repeated spaces into local `str2[40]`, parse article/count and case-insensitive item/spell names. **40-byte source plus NUL into 40-byte str2 boundary requires review.** Store/order state owns persistence. [10767](../../../src/server/xtra1.c#L10767) |
```

### Original line 72

Ticket 10 owns this request-specific server string consumer. Ordinary service may not claim it via generic request evidence; instantiate the relevant child at C before acceptance. Preserve the row’s transforms, ESC handling and exact byte limits.

```text
| `RID_SEND_ITEM` | Uppercase first byte, lookup player ID/account, validate item/fee/mode; unknown ESC suppresses unknown-addressee message. Mail state owns send. [11205](../../../src/server/xtra1.c#L11205) |
```

### Original line 76

Ticket 10 owns this request-specific server string consumer. Ordinary service may not claim it via generic request evidence; instantiate the relevant child at C before acceptance. Preserve the row’s transforms, ESC handling and exact byte limits.

```text
| `RID_CONTACT_OWNER` / PLAYER_STORES | Censor, quota/store/account validation; clip at MSG_LEN-CNAME_LEN-1=239 even though incoming wire delivers<=79; may notify online recipient and persist note. No explicit generic ESC suppression in this case. [11563](../../../src/server/xtra1.c#L11563) |
```

### Original line 88

`capability.items.select-name`, `capability.spells.choose-school`, `capability.spells.mimic-form`, `capability.spells.mimic-immunity`, `capability.spells.breath-preference`, `capability.skills.select-ability`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| Item/spell/power/technique/ability name lookups | E79; immunity/element E49; polymorph E40 (boolean ignored). Names use owner-specific exact case-insensitive or substring matching; return numeric ID, not submitted name bytes. [c-inven.c:523](../../../src/client/c-inven.c#L523), [c-spell.c:523](../../../src/client/c-spell.c#L523), [skills.c:624](../../../src/client/skills.c#L624) |
```

### Original line 93

Ticket 11 owns rule editor/files and its 80/70/54/18 byte fields; application producers are `capability.items.autoinscribe-one`, `capability.items.autoinscribe-all`, `capability.items.autoinscribe-on-update`.

```text
| Autoinscription search/index/filename/match/tag | Search/index E80; search backing array80 (off-by-one at primitive entry), index temp160; filenames70; match54/storage55; tag18/storage19. Match regex/substrings and tag gameplay grammar separate; apply can send inscription packets. `.ins` reader/writer owns files. [c-util.c:12239](../../../src/client/c-util.c#L12239), [12534](../../../src/client/c-util.c#L12534), [c-files.c:2985](../../../src/client/c-files.c#L2985) |
```

### Original line 130

`capability.items.inscribe`, `capability.items.cancel-inscribe`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-cmd.c:1700](../../../src/client/c-cmd.c#L1700) | `cmd_inscribe`; delegate | `if (!get_string("Inscription: ", buf, 59)) return;` |
```

### Original line 131

`capability.items.inscribe`, `capability.items.cancel-inscribe`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-cmd.c:1712](../../../src/client/c-cmd.c#L1712) | `cmd_inscribe`; delegate | `if (get_string("Inscription: ", buf, 59))` |
```

### Original line 147

`capability.items.assign-newest`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-cmd.c:8119](../../../src/client/c-cmd.c#L8119) | `cmd_message`; delegate | `if (get_string("Message: ", buf, sizeof(buf) - 1)) {` |
```

### Original line 186

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`, `capability.items.details-close`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-files.c:2057](../../../src/client/c-files.c#L2057) | `peruse_file`; delegate | `if (askfor_aux(tmp, 10, 0)) cur_line = atoi(tmp);` |
```

### Original line 187

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`, `capability.items.details-close`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-files.c:2067](../../../src/client/c-files.c#L2067) | `peruse_file`; delegate | `if (askfor_aux(tmp, 60, 0)) {` |
```

### Original line 188

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`, `capability.items.details-close`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-files.c:2084](../../../src/client/c-files.c#L2084) | `peruse_file`; delegate | `if (askfor_aux(tmp, 60, 0)) {` |
```

### Original line 190

`capability.items.select-name`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-inven.c:523](../../../src/client/c-inven.c#L523) | `get_item_hook_find_obj`; delegate | `if (!get_string(get_item_hook_find_obj_what, buf, 79)) return(FALSE);` |
```

### Original line 191

`capability.spells.mimic-power`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-spell.c:523](../../../src/client/c-spell.c#L523) | `get_mimic_spell`; delegate | `if (!get_string("Power? ", buf, 79)) {` |
```

### Original line 192

`capability.spells.mimic-power`, `capability.spells.cancel-mimic-power`, `capability.spells.mimic-form`, `capability.spells.cancel-mimic-form`, `capability.spells.mimic-immunity`, `capability.spells.cancel-mimic-immunity`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-spell.c:677](../../../src/client/c-spell.c#L677) | `do_mimic`; delegate | `get_string("Which form (name or number; 0 for player; -1 for previous) ? ", out_val, 40);` |
```

### Original line 193

`capability.spells.mimic-power`, `capability.spells.cancel-mimic-power`, `capability.spells.mimic-form`, `capability.spells.cancel-mimic-form`, `capability.spells.mimic-immunity`, `capability.spells.cancel-mimic-immunity`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-spell.c:780](../../../src/client/c-spell.c#L780) | `do_mimic`; delegate | `if (!get_string("Immunity? ", buf, 49)) {` |
```

### Original line 194

`capability.spells.choose-school`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-spell.c:891](../../../src/client/c-spell.c#L891) | `get_item_hook_find_spell`; delegate | `if (!get_string("Spell name? ", buf, 79))` |
```

### Original line 195

`capability.spells.choose-school`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-spell.c:936](../../../src/client/c-spell.c#L936) | `get_item_hook_find_spell`; delegate | `if (!get_string("Spell name? ", buf, 79)) return(FALSE);` |
```

### Original line 196

`capability.spells.melee-technique`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-spell.c:1582](../../../src/client/c-spell.c#L1582) | `get_melee_technique`; delegate | `if (!get_string("Technique? ", buf, 79)) {` |
```

### Original line 197

`capability.spells.ranged-technique`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-spell.c:1760](../../../src/client/c-spell.c#L1760) | `get_ranged_technique`; delegate | `if (!get_string("Technique? ", buf, 79)) {` |
```

### Original line 198

`capability.spells.breath-preference`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [c-spell.c:2053](../../../src/client/c-spell.c#L2053) | `get_breath`; delegate | `if (!get_string("Element? ", buf, 49)) {` |
```

### Original line 286

`capability.skills.navigate-tree`, `capability.skills.raise`, `capability.skills.develop`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [skills.c:388](../../../src/client/skills.c#L388) | `do_cmd_skill`; delegate | `if (askfor_aux(tmp, 10, 0)) {` |
```

### Original line 287

`capability.skills.navigate-tree`, `capability.skills.raise`, `capability.skills.develop`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [skills.c:431](../../../src/client/skills.c#L431) | `do_cmd_skill`; delegate | `if (!askfor_aux(tmp, MAX_CHARS, 0)) {` |
```

### Original line 288

`capability.skills.select-ability`, `capability.skills.cancel-ability`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [skills.c:624](../../../src/client/skills.c#L624) | `do_cmd_activate_skill_aux`; delegate | `if (!get_string("Skill action? ", buf, 79)) {` |
```

### Original line 338

`capability.items.read-inventory`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2329](../../../src/client/nclient.c#L2329) | `Receive_inven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, &uses_dir, name)` |
```

### Original line 339

`capability.items.read-inventory`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2332](../../../src/client/nclient.c#L2332) | `Receive_inven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &uses_dir, name)` |
```

### Original line 340

`capability.items.read-inventory`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2335](../../../src/client/nclient.c#L2335) | `Receive_inven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, name)` |
```

### Original line 341

`capability.items.read-inventory`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2338](../../../src/client/nclient.c#L2338) | `Receive_inven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%s", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, name)` |
```

### Original line 342

`capability.items.read-bag`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2422](../../../src/client/nclient.c#L2422) | `Receive_subinven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%c%hu%hd%c%c%hd%hd%c%I", &ch, &iposc, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, &uses_dir, name)` |
```

### Original line 343

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2514](../../../src/client/nclient.c#L2514) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%I%c", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, 		    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9, name, &ident)` |
```

### Original line 344

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2518](../../../src/client/nclient.c#L2518) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, 		    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9, name)` |
```

### Original line 345

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2522](../../../src/client/nclient.c#L2522) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%c%c%c%c%c%c%c%c%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b, name)` |
```

### Original line 346

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2529](../../../src/client/nclient.c#L2529) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%c%c%c%c%c%c%c%c%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b, name)` |
```

### Original line 347

`capability.items.read-inventory`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2536](../../../src/client/nclient.c#L2536) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%c%c%c%c%c%c%c%c%c%s", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b, name)` |
```

### Original line 349

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2661](../../../src/client/nclient.c#L2661) | `Receive_equip`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, &uses_dir, name)` |
```

### Original line 350

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2664](../../../src/client/nclient.c#L2664) | `Receive_equip`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &uses_dir, name)` |
```

### Original line 351

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2667](../../../src/client/nclient.c#L2667) | `Receive_equip`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, name)` |
```

### Original line 352

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2670](../../../src/client/nclient.c#L2670) | `Receive_equip`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%s", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, name)` |
```

### Original line 353

`capability.items.read-equipment`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:2742](../../../src/client/nclient.c#L2742) | `Receive_equip_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%c%I%hd%hd%hd%hd%hd%hd%hd%hd%hd", 	    &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, &uses_dir, name, 	    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9)` |
```

### Original line 358

`capability.skills.read-tree`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:3005](../../../src/client/nclient.c#L3005) | `Receive_skill_init`; inbound | `Packet_scanf(&rbuf, "%c%hd%hd%hd%hd%d%c%S%S%S", &ch, &i, 	    &father, &order, &mkey, &flags1, &tval, name, desc, act)` |
```

### Original line 367

`capability.spells.read-metadata`, `capability.spells.cast-legacy`, `capability.spells.ghost`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:3965](../../../src/client/nclient.c#L3965) | `Receive_spell_info`; inbound | `Packet_scanf(&rbuf, "%c%d%d%d%hu%hu%hu%s", &ch, &spells[0], &spells[1], &spells[2], &realm, &book, &line, buf)` |
```

### Original line 368

`capability.store.read-actions`, `capability.store.service`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4520](../../../src/client/nclient.c#L4520) | `Receive_store_action`; inbound | `Packet_scanf(&rbuf, "%c%c%hd%hd%s%c%c%d%u", &ch, &pos, &bact, &action, name, &attr, &letter, &cost, &flag)` |
```

### Original line 369

`capability.store.read-actions`, `capability.store.service`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4522](../../../src/client/nclient.c#L4522) | `Receive_store_action`; inbound | `Packet_scanf(&rbuf, "%c%c%hd%hd%s%c%c%hd%hu", &ch, &pos, &bact, &action, name, &attr, &letter, &oldcost, &oldflag)` |
```

### Original line 370

`capability.store.read-actions`, `capability.store.service`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4526](../../../src/client/nclient.c#L4526) | `Receive_store_action`; inbound | `Packet_scanf(&rbuf, "%c%c%hd%hd%s%c%c%hd%c", &ch, &pos, &bact, &action, name, &attr, &letter, &oldcost, &oldflag2)` |
```

### Original line 371

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4556](../../../src/client/nclient.c#L4556) | `Receive_store`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%d%s", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval, &powers)` |
```

### Original line 372

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4559](../../../src/client/nclient.c#L4559) | `Receive_store`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%hd%s", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b, &powers)` |
```

### Original line 373

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4563](../../../src/client/nclient.c#L4563) | `Receive_store`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%hd", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b)` |
```

### Original line 374

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4567](../../../src/client/nclient.c#L4567) | `Receive_store`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%s%c%c%hd", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b)` |
```

### Original line 375

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4605](../../../src/client/nclient.c#L4605) | `Receive_store_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%d%hd%hd%hd%hd%hd%hd%hd%hd%hd", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval, 		    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9)` |
```

### Original line 376

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4609](../../../src/client/nclient.c#L4609) | `Receive_store_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b, 		    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9)` |
```

### Original line 377

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4614](../../../src/client/nclient.c#L4614) | `Receive_store_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%hd%c%c%c%c%c%c%c%c%c", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b)` |
```

### Original line 378

`capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:4622](../../../src/client/nclient.c#L4622) | `Receive_store_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%s%c%c%hd%c%c%c%c%c%c%c%c%c", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b)` |
```

### Original line 380

`capability.store.enter`, `capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:5453](../../../src/client/nclient.c#L5453) | `Receive_store_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%d%c%c%c", &ch, &store_num, store_name, owner_name, &num_items, &max_cost, &store_attr, &store_char, &store_price_mul)` |
```

### Original line 381

`capability.store.enter`, `capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:5455](../../../src/client/nclient.c#L5455) | `Receive_store_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%d%c%c", &ch, &store_num, store_name, owner_name, &num_items, &max_cost, &store_attr, &store_char)` |
```

### Original line 382

`capability.store.enter`, `capability.store.read-stock`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:5457](../../../src/client/nclient.c#L5457) | `Receive_store_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%d", &ch, &store_num, store_name, owner_name, &num_items, &max_cost)` |
```

### Original line 383

`capability.target.read-description`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:5520](../../../src/client/nclient.c#L5520) | `Receive_target_info`; inbound | `Packet_scanf(&rbuf, "%c%c%c%S", &ch, &x, &y, buf)` |
```

### Original line 384

`capability.target.read-description`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:5522](../../../src/client/nclient.c#L5522) | `Receive_target_info`; inbound | `Packet_scanf(&rbuf, "%c%c%c%s", &ch, &x, &y, buf)` |
```

### Original line 385

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:5813](../../../src/client/nclient.c#L5813) | `Receive_special_line`; inbound | `Packet_scanf(&rbuf, "%c%d%d%c%I", &ch, &max, &line, &attr, buf)` |
```

### Original line 386

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:5817](../../../src/client/nclient.c#L5817) | `Receive_special_line`; inbound | `Packet_scanf(&rbuf, "%c%hd%hd%c%I", &ch, &old_max, &old_line, &attr, buf)` |
```

### Original line 387

`capability.items.pickup-accept`, `capability.items.pickup-decline`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:5960](../../../src/client/nclient.c#L5960) | `Receive_pickup_check`; inbound | `Packet_scanf(&rbuf, "%c%s", &ch, buf)` |
```

### Original line 407

`capability.items.read-floor`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:7484](../../../src/client/nclient.c#L7484) | `Receive_whats_under_you_feet`; inbound | `Packet_scanf(&rbuf, "%c%c%c%c%I", &ch, &crossmod_item, &cant_see, &on_pile, o_name)` |
```

### Original line 408

`capability.items.read-floor`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:7486](../../../src/client/nclient.c#L7486) | `Receive_whats_under_you_feet`; inbound | `Packet_scanf(&rbuf, "%c%c%c%c%s", &ch, &crossmod_item, &cant_see, &on_pile, o_name)` |
```

### Original line 409

`capability.items.inscribe`, `capability.items.autoinscribe-one`, `capability.items.autoinscribe-all`, `capability.items.autoinscribe-on-update`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nclient.c:7702](../../../src/client/nclient.c#L7702) | `Send_inscribe`; outbound | `Packet_printf(&wbuf, "%c%hd%s", PKT_INSCRIBE, item, buf)` |
```

### Original line 451

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`. Required: each outcome’s result, lifecycle, input and applicable wire obligations.

```text
| [nserver.c:14703](../../../src/server/nserver.c#L14703) | `Receive_special_line` | `Packet_scanf(&connp->r, "%c%c%d%s", &ch, &type, &line, srcstr)` |
```

Mapped/dispositioned rows: 64.

## Outgoing response and intent source ledger

All excerpts are from `src/client/nclient.c`, SHA-256 `cad90ba0b4505b41f31da5284d80448738e54cde6b6ee394aebb258ca8884853`. Primitives do not certify callers. Each associated outcome requires exact bytes and server result.

### Send_stay

`capability.items.pickup-accept`, `capability.items.pickup-decline`

```c
int Send_stay(void) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_STAND)) <= 0) return(n);
	return(1);
}
```

### Send_store_leave

`capability.store.leave`

```c
int Send_store_leave(void) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_STORE_LEAVE)) <= 0) return(n);
	return(1);
}
```

### Send_drop

`capability.items.drop`, `capability.items.cancel-drop`

```c
int Send_drop(int item, int amt) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_DROP, item, amt)) <= 0) return(n);
	return(1);
}
```

### Send_drop_gold

`capability.items.drop-gold`, `capability.items.cancel-drop-gold`

```c
int Send_drop_gold(s32b amt) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%d", PKT_DROP_GOLD, amt)) <= 0) return(n);
	return(1);
}
```

### Send_wield

`capability.items.wield`, `capability.items.cancel-wield`, `capability.items.swap`

```c
int Send_wield(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_WIELD, item)) <= 0) return(n);
	return(1);
}
```

### Send_wield2

`capability.items.wield-secondary`, `capability.items.cancel-wield-secondary`, `capability.items.swap`

```c
int Send_wield2(int item) {
	int	n;
	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_WIELD2, item)) <= 0) return(n);
	return(1);
}
```

### Send_take_off

`capability.items.take-off`, `capability.items.cancel-take-off`

```c
int Send_take_off(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_TAKE_OFF, item)) <= 0) return(n);
	return(1);
}
```

### Send_wield3

`capability.items.swap`, `capability.items.cancel-swap`

```c
int Send_wield3(void) {
	int	n;
	if ((n = Packet_printf(&wbuf, "%c", PKT_WIELD3)) <= 0) return(n);
	return(1);
}
```

### Send_destroy

`capability.items.destroy`, `capability.items.cancel-destroy`

```c
int Send_destroy(int item, int amt) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_DESTROY, item, amt)) <= 0) return(n);
	return(1);
}
```

### Send_inscribe

`capability.items.inscribe`, `capability.items.cancel-inscribe`, `capability.items.autoinscribe-on-update`, `capability.items.autoinscribe-one`, `capability.items.autoinscribe-all`

```c
int Send_inscribe(int item, cptr buf) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%s", PKT_INSCRIBE, item, buf)) <= 0) return(n);
	return(1);
}
```

### Send_uninscribe

`capability.items.uninscribe`, `capability.items.cancel-uninscribe`, `capability.items.autoinscribe-one`, `capability.items.autoinscribe-all`

```c
int Send_uninscribe(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_UNINSCRIBE, item)) <= 0) return(n);
	return(1);
}
```

### Send_autoinscribe

`capability.items.autoinscribe-one`, `capability.items.cancel-autoinscribe-one`, `capability.items.autoinscribe-all`

```c
int Send_autoinscribe(int item) {
	int n;

	if (!is_newer_than(&server_version, 4, 5, 5, 0, 0, 0)) return(1);
	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_AUTOINSCRIBE, item)) <= 0) return(n);
	return(1);
}
```

### Send_quaff

`capability.items.quaff`, `capability.items.cancel-quaff`

```c
int Send_quaff(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_QUAFF, item)) <= 0) return(n);
	return(1);
}
```

### Send_read

`capability.items.read`, `capability.items.cancel-read`

```c
int Send_read(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_READ, item)) <= 0) return(n);
	return(1);
}
```

### Send_aim

`capability.items.aim-wand`, `capability.items.cancel-aim-wand`

```c
int Send_aim(int item, int dir) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%c", PKT_AIM_WAND, item, dir)) <= 0) return(n);
	return(1);
}
```

### Send_use

`capability.items.use-staff`, `capability.items.cancel-use-staff`

```c
int Send_use(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_USE, item)) <= 0) return(n);
	return(1);
}
```

### Send_zap_dir

`capability.items.zap-rod`, `capability.items.cancel-zap-rod`

```c
int Send_zap_dir(int item, int dir) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%c", PKT_ZAP_DIR, item, dir)) <= 0) return(n);
	return(1);
}
```

### Send_fill

`capability.items.refill`, `capability.items.cancel-refill`

```c
int Send_fill(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_FILL, item)) <= 0) return(n);
	return(1);
}
```

### Send_eat

`capability.items.eat`, `capability.items.cancel-eat`

```c
int Send_eat(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_EAT, item)) <= 0) return(n);
	return(1);
}
```

### Send_activate_dir

`capability.items.activate`, `capability.items.cancel-activate`

```c
int Send_activate_dir(int item, int dir) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%c", PKT_ACTIVATE_DIR, item, dir)) <= 0) return(n);
	return(1);
}
```

### Send_force_stack

`capability.items.force-stack`, `capability.items.cancel-force-stack`

```c
int Send_force_stack(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_FORCE_STACK, item)) <= 0) return(n);
	return(1);
}
```

### Send_split_stack

`capability.items.split-stack`, `capability.items.cancel-split-stack`

```c
int Send_split_stack(int item, int amt) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_SPLIT_STACK, item, amt)) <= 0) return(n);
	return(1);
}
```

### Send_si_move

`capability.items.stow`, `capability.items.cancel-stow`

```c
int Send_si_move(int item, int amt) {
	int n;

	if (!is_newer_than(&server_version, 4, 7, 4, 4, 0, 0)) return(1);
	if (is_older_than(&server_version, 4, 9, 2, 0, 0, 0)) {
		if ((n = Packet_printf(&wbuf, "%c%hd", PKT_SI_MOVE, item)) <= 0) return(n); //discard amt, always move full stack
	} else {
		if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_SI_MOVE, item, amt)) <= 0) return(n);
	}
	return(1);
}
```

### Send_si_remove

`capability.items.unstow`, `capability.items.cancel-unstow`

```c
int Send_si_remove(int item, int amt) {
	int n, islot = item / SUBINVEN_INVEN_MUL - 1;

	if (!is_newer_than(&server_version, 4, 7, 4, 4, 0, 0)) return(1);
	if (is_older_than(&server_version, 4, 9, 2, 0, 0, 0)) {
		if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_SI_REMOVE, (short int)islot, (short int)(item % SUBINVEN_INVEN_MUL))) <= 0) return(n); //discard amt, always move full stack
	} else {
		if ((n = Packet_printf(&wbuf, "%c%hd%hd%hd", PKT_SI_REMOVE, (short int)islot, (short int)(item % SUBINVEN_INVEN_MUL), amt)) <= 0) return(n);
	}
	return(1);
}
```

### Send_observe

`capability.items.inspect`, `capability.items.cancel-inspect`

```c
int Send_observe(int item) {
	int n;

#if 0
#ifdef ENABLE_SUBINVEN
	if (using_subinven != -1) {
		/* Hacky encoding */
		if ((n = Packet_printf(&wbuf, "%c%hd", PKT_OBSERVE, item + (using_subinven + 1) * SUBINVEN_INVEN_MUL)) <= 0) return(n);
		return(1);
	}
#endif
#endif
	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_OBSERVE, item)) <= 0) return(n);
	return(1);
}
```

### Send_item

`capability.items.server-item-answer`, `capability.items.server-item-cancel`

```c
int Send_item(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_ITEM, item)) <= 0) return(n);
	return(1);
}
```

### Send_spell

`capability.items.server-spell-answer`, `capability.items.server-spell-cancel`

```c
int Send_spell(int item, int spell) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_SPELL, item, spell)) <= 0) return(n);
	return(1);
}
```

### Send_fire

`capability.combat.fire`, `capability.combat.cancel-fire`

```c
int Send_fire(int dir) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_FIRE, dir)) <= 0) return(n);
	return(1);
}
```

### Send_throw

`capability.combat.throw`, `capability.combat.cancel-throw`

```c
int Send_throw(int item, int dir) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%c%hd", PKT_THROW, dir, item)) <= 0) return(n);
	return(1);
}
```

### Send_steal

`capability.combat.steal`, `capability.combat.cancel-steal`

```c
int Send_steal(int dir) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_STEAL, dir)) <= 0) return(n);
	return(1);
}
```

### Send_spike

`capability.combat.spike`, `capability.combat.cancel-spike`

```c
int Send_spike(int dir) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_SPIKE, dir)) <= 0) return(n);
	return(1);
}
```

### Send_sip

`capability.combat.sip`

```c
int Send_sip(void) {
	int	n;
	if ((n = Packet_printf(&wbuf, "%c", PKT_SIP)) <= 0) return(n);
	return(1);
}
```

### Send_telekinesis

`capability.combat.telekinesis`

```c
int Send_telekinesis(void) {
	int	n;
	if ((n = Packet_printf(&wbuf, "%c", PKT_TELEKINESIS)) <= 0) return(n);
	return(1);
}
```

### Send_cloak

`capability.combat.cloak`

```c
int Send_cloak(void) {
	int	n;
	if ((n = Packet_printf(&wbuf, "%c", PKT_CLOAK)) <= 0) return(n);
	return(1);
}
```

### Send_skill_mod

`capability.skills.raise`

```c
int Send_skill_mod(int i) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%d", PKT_SKILL_MOD, i)) <= 0) return(n);
	return(1);
}
```

### Send_skill_dev

`capability.skills.develop`

```c
int Send_skill_dev(int i, bool dev) {
	int n;

	if (is_newer_than(&server_version, 4, 4, 8, 2, 0, 0)) {
		if ((n = Packet_printf(&wbuf, "%c%d%c", PKT_SKILL_DEV, i, dev)) <= 0) return(n);
	}
	return(1);
}
```

### Send_activate_skill

`capability.spells.cast-school`, `capability.spells.cancel-cast-school`, `capability.spells.cast-legacy`, `capability.spells.cancel-cast-legacy`, `capability.spells.activate-generic`, `capability.spells.cancel-activate-generic`, `capability.spells.mimic-power`, `capability.spells.cancel-mimic-power`, `capability.spells.mimic-form`, `capability.spells.cancel-mimic-form`, `capability.spells.mimic-immunity`, `capability.spells.cancel-mimic-immunity`, `capability.spells.stance`, `capability.spells.cancel-stance`, `capability.spells.melee-technique`, `capability.spells.cancel-melee-technique`, `capability.spells.ranged-technique`, `capability.spells.cancel-ranged-technique`, `capability.spells.runecraft`, `capability.spells.cancel-runecraft`, `capability.spells.breath-preference`, `capability.spells.cancel-breath-preference`, `capability.spells.breath`, `capability.spells.cancel-breath`, `capability.spells.trap`, `capability.spells.cancel-trap`, `capability.spells.mycorrhiza`, `capability.spells.cancel-mycorrhiza`, `capability.spells.stop-mycorrhiza`, `capability.items.cancel-use-selected`

```c
int Send_activate_skill(int mkey, int book, int spell, int dir, int item, int aux) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%c%hd%hd%c%hd%hd", PKT_ACTIVATE_SKILL, mkey, book, spell, dir, item, aux)) <= 0) return(n);
	return(1);
}
```

### Send_ghost

`capability.spells.ghost`, `capability.spells.cancel-ghost`

```c
int Send_ghost(int ability) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_GHOST, ability)) <= 0) return(n);
	return(1);
}
```

### Send_store_purchase

`capability.store.buy`, `capability.store.take-home`, `capability.store.cancel-buy`, `capability.store.cancel-take-home`

```c
int Send_store_purchase(int item, int amt) {
	int  n;

	if (is_atleast(&server_version, 4, 9, 3, 0, 0, 3)) { /* for TV_GOLD-into-home-depositing */
		if ((n = Packet_printf(&wbuf, "%c%hd%d", PKT_PURCHASE, item, amt)) <= 0) return(n);
	} else {
		if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_PURCHASE, item, amt)) <= 0) return(n);
	}
	return(1);
}
```

### Send_store_sell

`capability.store.sell`, `capability.store.deposit-home`, `capability.store.deposit-gold`, `capability.store.donate`, `capability.store.cancel-sell`, `capability.store.cancel-deposit-home`, `capability.store.cancel-deposit-gold`, `capability.store.cancel-donate`

```c
int Send_store_sell(int item, int amt) {
	int n;

	if (is_atleast(&server_version, 4, 9, 3, 0, 0, 3)) { /* for TV_GOLD-into-home-depositing */
		if ((n = Packet_printf(&wbuf, "%c%hd%d", PKT_SELL, item, amt)) <= 0) return(n);
	} else {
		if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_SELL, item, amt)) <= 0) return(n);
	}
	return(1);
}
```

### Send_store_confirm

`capability.store.accept-offer`

```c
int Send_store_confirm(void) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c", PKT_STORE_CONFIRM)) <= 0) return(n);
	return(1);
}
```

### Send_store_examine

`capability.store.examine`, `capability.store.cancel-examine`

```c
int Send_store_examine(int item) {
	int  n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_STORE_EXAMINE, item)) <= 0) return(n);
	return(1);
}
```

### Send_store_command

`capability.store.service`, `capability.store.cancel-service`

```c
int Send_store_command(int action, int item, int item2, int amt, int gold) {
	int  n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd%hd%hd%d", PKT_STORE_CMD, action, item, item2, amt, gold)) <= 0) return(n);
	return(1);
}
```

### Send_raw_key

`capability.store.raw-key`

```c
int Send_raw_key(int key) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%c", PKT_RAW_KEY, key)) <= 0) return(n);
	return(1);
}
```

### Send_msg

`capability.store.paste-stock`, `capability.store.cancel-paste-stock`, `capability.items.paste-inventory`, `capability.items.paste-equipment`, `capability.items.paste-bag`

```c
int Send_msg(cptr message) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%S", PKT_MESSAGE, message)) <= 0) return(n);
	return(1);
}
```

### Send_special_line

`capability.items.details-read`, `capability.items.details-navigate`, `capability.items.details-search`, `capability.items.details-close`

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

### Send_activate

`capability.items.use-selected`, `capability.items.activate`

```c
int Send_activate(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_ACTIVATE, item)) <= 0) return(n);
	return(1);
}
```

### Send_take_off_amt

`capability.items.take-off`, `capability.items.cancel-take-off`

```c
int Send_take_off_amt(int item, int amt) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd%hd", PKT_TAKE_OFF_AMT, item, amt)) <= 0) return(n);
	return(1);
}
```

### Send_zap

`capability.items.zap-rod`

```c
int Send_zap(int item) {
	int n;

	if ((n = Packet_printf(&wbuf, "%c%hd", PKT_ZAP, item)) <= 0) return(n);
	return(1);
}
```

### Send_inventory_revision

`capability.items.read-inventory`, `capability.items.select-slot`

```c
int Send_inventory_revision(int revision) {
	int	n;
	if ((n = Packet_printf(&wbuf, "%c%d", PKT_INVENTORY_REV, revision)) <= 0) return(n);
	return(1);
}
```

## Formatted surfaces reconciliation

The original `.scratch/single-window-sdl3-client/research/formatted-and-server-driven-surfaces.md` (SHA-256 `123d9fe4aa501466cf8cd11c5eef285cfb215165aeb298b2d4bd1a40f70f4d72`) separates ordinary keyed stores from coordinate-addressed special canvases. Ordinary-store staging/extent/actions/paging/transactions/leave paragraphs map to store.enter/read-stock/read-actions/page/buy/sell/examine/leave/kicked. Item-detail SPECIAL_FILE_OTHER content and peruse transitions map to items.details-read/navigate/search/close and items.inspect/store.examine. SPECIAL_FILE_NONE is close, not a document; other category-table entries belong to ticket 10, except existing B MOTD. Raw title/line/page/position bytes are retained by the C child, not deferred to a D renderer.

Special-store string/glyph/clear/animation rows and Go/casino semantics are ticket 10 / D, reusing the ordinary shell IDs without claiming its body. Generic KEY remains the existing request IDs; AMT/NUM/STR/CFR require distinct owner outcomes in 10. Generic ABORT only affects a pending generic request. No ordinary transaction is certified by those synthetic foundation tests.

## Explicit input and field cases

For every item command exercise inventory, equipment and enabled bag modes that its c_get_item flags actually allow; numeric inscription tags, @ names, newest +, sole-item Enter, source switching, uppercase verification or alternate slot, empty/filtered lists and disappearing/changed slots. Preserve accepted macro bytes and consumption after a missing item. A widget may not maintain a second command interpreter.

Per-field byte scenarios use 0/limit−1/limit/limit+1, typing, paste, defaults/imported values and macro→Enter. Inscription editor 59 bytes differs from wire `%s`80 including NUL and server limit79; name lookup E79, immunity/breath E49 and mimic-form E40 are local numeric-ID producers, not network string fields. Item/store received `%s`/`%S` branches preserve original colour/control bytes. Autoinscription match/tag files (54/18 entry limits) are separate producers and retain ordered rule semantics. School Lua callbacks preserve their exact numeric result conversion. Store/chat substitution must check the final transformed byte value, not only the visible text.

Packet cases require all registered shapes above, complete/incomplete/oversized/chained input, sentinel preservation, queue exhaustion, coherent keyed replacement and exact outgoing bytes. Server rejection never implies successful mutation. Unsupported build gates stay conditional pending, not removed from the denominator.

## Verification — 2026-09-22

Added 156 outcomes; total 269 active allocations, all implementation/evidence
pending, zero accepted. Production validator passed with local source bytes and
history against `bbe7417e21d20470a7aeb075aa82650608f6e4ae`. All previous source,
entity and relation records are unchanged. Existing allocation rows only acquire
explicit earlier-flow child prerequisites. No C runtime, shared source, schema
or validator implementation changed.

The canonical-source test passed separately. The complete existing Linux suite
then passed **22/22 runner invocations**: 12 registry tests (including 32 published
negative fixtures), four headless sanitizer suites, legacy HP, and eight native
suites on each of software/OpenGL. Initial sandbox attempts blocked LeakSanitizer
thread inspection and SDL display access; the full rerun outside sandbox passed
without disabling sanitizers or substituting a dummy renderer. Logs:
`/tmp/sv09-check-01.log` through `/tmp/sv09-check-22.log`. Python compilation and
`git diff --check` also passed.

Reproduce from the repository root:

```sh
git show bbe7417e2:docs/capabilities/manifest.json > /tmp/sv09-previous-manifest.json
/tmp/sv-capabilities-venv/bin/python tools/validate_capabilities.py \
  --manifest docs/capabilities/manifest.json \
  --ledger docs/capabilities/native-coverage.json \
  --previous-manifest /tmp/sv09-previous-manifest.json --source-root tomenet=.
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

These regressions exercise the existing SV foundation, not native implementations
of the newly allocated gameplay outcomes. Windows/Wine, live-server round trips
and human visual acceptance were not exercised for this data-only ticket.
No Stage A completeness or gameplay acceptance is claimed. TDD required no new
seam or test-only behavior: the already approved production registry CLI and its
existing process-level checks validate the added data.

### Standards

Independent review against the starting revision found one incorrect roguelike
split-stack binding. It is now explicitly normal `K` versus roguelike `Ctrl-C`,
followed by uppercase item selection. Follow-up review found **0 unresolved
Standards findings** and verified provenance, pending status and keysets.

### Spec

Independent review found a broad store-cancellation allocation and missing
store-child command prerequisites. Cancellation now has nine caller-specific
outcomes. The store child allocation explicitly depends on each invoked command,
its cancellation and list closure. Source audit also added all-in-one dispatch,
list closure and direct item-paste flows. Follow-up review found **0 unresolved
Spec findings**. Historical inventory-view discrepancies are recorded separately
as SV-IMP-008; no legacy behavior was changed.

Review used the explicit local ticket and parent spec; the optional tracker
bootstrap `docs/agents/issue-tracker.md` is absent. An external tracker was not
needed for this local task. Final findings: Standards 0 unresolved; Spec 0
unresolved.
