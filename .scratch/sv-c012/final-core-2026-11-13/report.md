# C012 — final existing focused core check, 2026-11-13

Result: **PASS for the existing Linux production core check on the final freeze3 sources**. No production or test source was changed. No visual/native-window, real-server, small/resize, Windows or broad matrix run was performed. The UI acceptance deferrals remain in force.

## Commands and snapshot

- `make -C src -f makefile.sv -j4 shell sv-status-check` — exit 0; `build.log`.
- `src/.sv-build/linux/bafdc534e11c1a7cb47f/checks/status` — exit 0; `status.log`.
- Exact before/after SHA-256 and argv/exit records: `final-checks.json`. All recorded source hashes and the shell binary were unchanged during the check.
- HUD SHA-256: `80ac4b283a7510c2330e3bdf23bbd63c0407ce237ecd155ef2dcde11efb3d423`.
- Shell SHA-256: `58554f922710f3f0629089e3cab8f73817d4ddbbb4aaf524f3c7ae4830a76c6b`.
- Focused check SHA-256: `c46d5671a809080fcc9780cc925aa85bc54d3ebe7570123491b46a3bbbf2bed8`.

The make target re-linked the existing check closure and reused up-to-date production objects. This is a successful normal incremental build, not a claimed clean rebuild. The focused target links production objects with its own test main; assertions call the app receive/step/view and actual `sv_hud_prepare` paths.

## Current correction coverage

- `tests/sv/status.c:169` (`stat_colour_boundaries`): raw stat packets through production path exercise natural base 117/118, boosted normal, and drain-over-boost priority. Both actual HUD/Character consumers use the resulting prepared row colour.
- `tests/sv/status.c:188` (`status_slot_formats`): raw encumbrance packets exercise three conflicting weapon flag combinations, confirming easy > heavy > awkward in the same prepared slot used by actual icon/tooltip draw. Shooter heavy > awkward is unchanged and source-audited, not separately claimed as a newly executed assertion.
- The same function sends a nine-segment server bar and word label with numeric payload, confirming segment count comes from the source label rather than recomputing its numeric ratio, and non-bar labels stay source text. `version_defaults` also checks all three Sanity wire layouts, availability and absent defaults. These are core assertions, not four-mode gameplay command-routing coverage.
- Source audit: `src/server/xtra1.c:320–363` selects word/star-bar/percent/exact labels, quantizes lower-skill numeric values and calls `Send_sanity`; `src/server/slash.c:5900` implements server-owned `/snbar` skill gating. `src/client/sv/protocol/status-update.c:59–63` preserves version-dependent received labels/numbers, while `src/client/sv/ui/hud.c:76–83` only recognizes an exact nine-character stars-then-dashes source label. No local client mode or extra precision was introduced. The configured huge representation uses only received, potentially quantized numeric data. Percent/exact label rendering and live `/snbar` interaction remain deferred UI/integration acceptance.
- Existing XP assertions compare no half-XP vs half-XP fills (`.5` vs `.525` in a twenty-XP interval), historical maximum fill, drained Level/XP colour and conditional maximum-player-level suffix. Source arithmetic converts to double before subtraction, uses the received half marker as `0.5`, and does not invent a fractional historical maximum.

## Existing wider core scope retained

The same existing runner executed latest field decoding, split/adjacent packet handling, version defaults, DEPTH/SPEED order and encoded conditions, HUD prepared-data boundaries, HP occurrence semantics, body/idle event retention, generation/reset, stale reads and malformed/out-of-range packet handling. Its printed “27 outcomes” is the runner's aggregate coverage label, **not** evidence of full user-visible acceptance of every one of the 81 obligations.

No new defect requiring code changes was found in this check. This report closes the previously unfinished final focused core run. Semantic contract interpretation and per-obligation disposition belong to the other C012 closure work; actual message/alert/audio/macro recipients and real gameplay lifecycle keep their later owners. The paused independent visual check is not promoted to PASS.
