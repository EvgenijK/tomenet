# TomeNET Quest Authoring Instructions for AI

Use this document as an implementation prompt when creating or modifying TomeNET
quests in `q_info.txt`.

Your goal is to produce a quest that is compatible with the exact TomeNET server
version in the target repository, behaves correctly at every stage, and can be
tested without guessing how the quest parser works.

## Core Rules

1. Treat the target server's source code and `lib/game/q_info.txt` as the source
   of truth.
2. Do not rely only on an editor, copied documentation, or examples from another
   TomeNET version.
3. Inspect the parser and existing working quests before adding an unfamiliar
   line type.
4. Keep changes limited to `q_info.txt` unless the user explicitly approves
   server code changes.
5. Do not build, restart, or otherwise control the server unless explicitly
   requested.
6. Never use a raw colon in a q_info text value because colon is the field
   delimiter.
7. Preserve unrelated quests and user changes.

## Required Investigation Before Editing

Before writing a quest:

1. Read the format comments at the beginning of the target `q_info.txt`.
2. Find at least one working example of every line type you intend to use.
3. Locate the corresponding parser in the target server source when field count,
   value type, or behavior is unclear.
4. Verify IDs for monsters, items, features, dungeons, towns, and other game
   records against the target server data files.
5. Confirm whether a feature is implemented in this server version. A documented
   line may still be unsupported, partially implemented, or incompatible.
6. Record the intended questor indexes and stage transitions before serializing
   the quest.

Do not invent field meanings or add placeholder values merely to satisfy an
expected field count.

## Design the Quest as a State Machine

Write down the complete stage flow before creating q_info lines. For every stage,
define:

- how the player enters the stage;
- which questors are relevant;
- what dialogue each questor displays;
- which items are created, held, delivered, or removed;
- which goals are active;
- what causes the transition to the next stage;
- what the quest log says;
- whether a reward is issued.

Use a separate stage when acceptance, item handout, delivery, return, and reward
need different behavior. Do not overload one stage with unrelated transitions.

For a delivery quest, prefer this structure:

1. Offer stage.
2. Optional technical handout stage.
3. Delivery stage.
4. Return stage.
5. Reward or ending stage.

An immediate `A` transition may be used for a technical stage that only exists to
make item handout or stage initialization deterministic.

## Validate Every Line Shape

The parser requires an exact number and order of colon-delimited fields. Before
adding a line:

1. Copy its field layout from the current target `q_info.txt` comments.
2. Count every field, including zero, `-1`, `-`, and empty-looking values.
3. Compare the result with a working line parsed by the same server version.
4. Confirm whether numbers are decimal, hexadecimal masks, character values, or
   record IDs.

This is especially important for `S` and `R` lines. Both caused parse failures
during the reference quest because their layouts did not match the active parser.

If an `S` line is not safely supported by the target server, do not use it as a
stage-spawn mechanism. Define the questor with `Q` and gate its useful behavior by
stage instead. Clearly document this workaround in the quest.

## Preserve Required Line Ordering

Some lines are associated by position, not only by their values. Keep these
relationships:

- `L`, optional `Ld`, optional `F`, and optional `K` immediately follow their
  corresponding `Q` line.
- `Bl` immediately follows its corresponding `B` line.
- `rI` and `rV` follow the corresponding `r` goal.
- `yY`, `yQ`, and `yR` follow the corresponding `y` reply.
- Other documented child lines must remain attached to the parent line shown in
  the target format comments.

Do not reorder related lines for visual grouping if it changes parser ownership.

## Manage Questor Indexes Explicitly

Questor indexes are positional:

- the first `Q` line is questor `0`;
- the second `Q` line is questor `1`;
- subsequent indexes continue in declaration order.

Before editing, create a small index map in comments or working notes. Check every
questor reference in `W`, `Y`, `y`, `M`, `Bl`, and all other relevant lines.

After inserting, deleting, or moving a `Q` line, audit every questor reference.
Do not assume indexes are stable after reordering.

For delivery goals, `M` must name the exact receiving questor. Using the Bree
questor's index for a Training Tower delivery can allow the wrong NPC to consume
the quest item. A value of `-1` disables questor delivery and uses the configured
location fields instead.

## Configure Questor Interaction Consistently

When a questor should talk on interaction, verify all of the following:

- the `F` line permits the required interaction and talk behavior;
- the `W` line uses the correct questor index;
- the `W` line uses the player's current stage;
- the acquisition stage is allowed by `C`;
- the questor is not unintentionally despawned;
- the questor location is valid and reachable.

Give each questor stage-specific dialogue. At minimum, distinguish:

- the initial offer;
- an active-task reminder;
- successful item delivery;
- the instruction to return;
- final thanks and reward.

Do not reuse pre-delivery dialogue after the delivery is complete.

## Design Keywords and Prompts Carefully

The first matching keyword affects prompt behavior. If the first matching keyword
is `y`, TomeNET may enter its special yes/no interaction mode, which can force the
player to answer before leaving the prompt.

When the player should be able to leave the conversation:

- use a named first keyword such as `accept`;
- optionally support aliases such as `yes`;
- provide a non-transitioning response such as `later`;
- use next stage `255` when the response must not change the stage.

Do not require a keyword to confirm an action that the goal system already
completes on interaction. Combining automatic delivery with a mandatory keyword
can create an ambiguous state in which the item is gone but the expected dialogue
or transition was not completed.

For a simple hand-in, prefer a delivery goal targeted at the correct questor plus
a `G` transition.

## Create and Deliver Quest Items Consistently

A quest item flow should use one coherent identity across all related lines:

- `B` creates the special quest item;
- `Bl` hands it out through a questor or places it at a location;
- `r`, `rI`, and `rV` define how it is recognized as a retrieval goal;
- `M` defines where or to whom it must be delivered;
- `G` advances the quest when the required goal is complete.

Keep the identifying `pval` or equivalent quest-item key consistent. Confirm that
the item is created on the stage where the player can actually receive it.

If handout timing is unreliable during the same transition that accepts the
quest, use a dedicated technical stage and an immediate `A` transition.

## Format Custom Item Names Correctly

Custom object names must follow TomeNET's object naming syntax. A malformed name
may appear correctly in detailed inspection but as a blank line in the normal
inventory.

Use a name template such as:

```text
& Bundle~ of Enchantment Scrolls
```

The `&` and `~` markers participate in article and plural handling. Always test
both:

- the regular inventory listing;
- detailed item inspection.

Do not assume that a correct detailed description means the inventory name is
valid.

## Configure Rewards Precisely

The `R` line has a strict field order. Gold must be placed in the documented
`gold` field. For a gold-only reward, use zero item identifiers, including
`tval=0` and `sval=0`.

Each `R` line creates one item. To reward five identical potions, emit five
correctly formed `R` lines unless the target parser explicitly supports another
quantity mechanism.

Before finalizing rewards:

- verify `tval` and `sval` against the target item data;
- count every `R` field;
- confirm the gold amount is in the correct position;
- ensure rewards belong only to the intended reward stage;
- verify the reward cannot be obtained repeatedly without restarting the quest.

## Configure Repeatability Explicitly

Set the quest's repeatability value in `N` deliberately. Use `U` to define the
ending stage and cooldown according to the current format.

For a repeatable quest, test:

- reacquisition after completion;
- reset of the current stage and goals;
- a fresh copy of every required quest item;
- no duplicate reward before a new completion;
- cooldown behavior, if enabled.

Do not rely on administrator state alone when verifying normal repeatability.

## Validate Locations in the Running Game

Location values that parse successfully may still place a questor in an
unreachable or unsuitable tile.

For every `L` and location-based goal:

1. Verify world position, dungeon, floor, and local grid coordinates.
2. Check the actual tile in the game.
3. Confirm that terrain, walls, trees, entrances, and buildings do not block the
   questor or player interaction.
4. Adjust coordinates from a known reference point when screenshots or player
   coordinates are available.
5. Retest after map regeneration when the floor is not static.

Do not claim that dynamic-floor questor persistence is solved by q_info
configuration. Floor unload and reattachment behavior belongs to server code.

## Keep Code Issues Separate

Do not present the following as q_info authoring solutions:

- preserving questors across floor deallocation and regeneration;
- changing internal dialogue prompt behavior;
- changing how the engine selects a delivery recipient;
- fixing crashes introduced by modifications to `quests.c`;
- altering server-side questor lifecycle behavior.

If one of these blocks the quest, document:

1. the observed behavior;
2. the minimal reproducer in `q_info.txt`;
3. why configuration cannot safely solve it;
4. the proposed server-code change;
5. the tests required for that code change.

Do not modify the server code until the user explicitly approves it.

## Required Validation Pass

After editing `q_info.txt`, perform or request these checks:

1. Confirm that the server parses the complete file without reporting a record or
   line error.
2. Confirm that every questor appears at the intended reachable location.
3. Approach and interact with every questor on every relevant stage.
4. Verify that refusal or conversation exit does not leave the player trapped in
   a prompt.
5. Accept the quest and confirm that the starting item is issued exactly once.
6. Check that the item has a visible inventory name and correct detailed
   description.
7. Attempt to deliver the item to every questor that should reject it.
8. Deliver it to the intended questor and verify item removal, dialogue, quest
   log text, and stage transition.
9. Return to the original questor and verify that the reward is granted exactly
   once.
10. Verify every item quantity, item type, and gold amount.
11. Complete and reacquire the quest when it is repeatable.
12. Reload or regenerate relevant floors and separately report any questor
   persistence issue as a server-code concern.

## Expected AI Output

When asked to create or modify a quest, provide:

- the exact `q_info.txt` changes;
- a concise stage-flow summary;
- a questor index map;
- the IDs and source files used for game records;
- any assumptions that could not be verified;
- validation performed and validation still requiring a running server;
- a clear warning for any behavior that requires server-code changes.

Never state that a quest is complete merely because the file parses. A complete
quest must also pass stage, dialogue, item, recipient, reward, repeatability, and
location checks.
