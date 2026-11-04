# Independent review — B2 design followup, 2026-10-29

Scope: exact task delta against `/tmp/sv-b2-design-followup-baseline.tar`, Linux only. Reviewed CONTEXT and ADR 1/2/3/5/6; Graphify queried before source exploration. Original prototype: `/home/svechnik/Projects/github_site/tomenet_interface/login.html`.

## Chronicle

No actionable source finding: single worker schedules refresh starts every 5000 ms; a batch exceeding the interval finishes before a successor begins. Per-source caches are bounded and worker-owned, successful responses replace their own cache; rebuilding/sorting the merged snapshot prevents repeated refresh duplication. Last good data survives transport/parser failures. Snapshot copying and revision comparison remain mutex-protected. Cancel is atomic; worker reference holds provider/mutex/cache through curl cleanup and sleep observes cancellation at <=50 ms intervals. Existing main-owner initialization/refcount contracts remain unchanged. Root separately captured live repeated nine-source batches at 5/10/15 s; this review does not independently re-run the network suite.

## Links and glyphs

All six production SDL_OpenURL destinations match the prototype. Visible circle and text share hit targets. Dispatch uses raw window coordinates once, with display density/scale conversion, before contact-scene coordinate transforms; stale-event generation guard remains. The modal direct-address surface blocks background links. Credential entry keeps text shortcuts unchanged. Birth/name-entry/fullwindow MOTD do not expose invisible link targets. Hover sets normal/hover resource selection and resets on leave/focus loss. New texture arrays and glow are freed on reset and close; unavailable placeholder removed.

Independent XML comparison confirmed all five nontext icons retain exact external prototype viewBoxes, path strings and child geometry. IRC follows source text role. Outer icon bounds are explicit circles per user's previous override; generated normal and hover PNGs remain derived from SVG. Rounded AA geometry ring/index capacities are valid and layout remains logical/DPI based. SHA manifest update and provenance confirmed by visual agent.

## Finding

[P2, corrected and re-reviewed] Fractional scale Q hit target differs from rendered hint line. `server_shortcuts` centers and wraps using measured TTF widths, but `sv_login_server_layout` uses fixed 552/294/402 logical constants. Independent linked-TTF measurement (`/tmp/sv-hints-metrics.c`) with LiberationMono at 12.5 physical pixels, scale 1.25, 1024 logical width (610 panel) produced total 584 versus available 568: renderer wraps, hardcoded input does not. The visible Q is on the second line while its hitbox is on the first. Sent to visual implementer and root. Correction now shares `sv_login_hint_piece_width/width` for both renderer and input wrapping/centering, and `hint_text` compensates raster hinting to the shared fixed logical widths. Reviewed prior render-scale restoration on success/failure. Root owns final actual fractional Q gesture check; no open source findings remain.

No new suites/frameworks or production edits made by reviewer. Root owns final existing endpoint suite/build and actual renderer/input validation.

Final source review: no open actionable findings. Asset SHA256 manifest independently checked: 57 entries, no mismatch. Root reports full existing `python3 tests/sv_endpoint_checks.py` green and software shell smoke green; after fractional hint correction, root will run relevant existing checks and actual X11 software/OpenGL evidence. Browser panel backdrop blur is explicitly an implementation limitation in PROVENANCE, not claimed pixel-perfect parity.
