# Independent source review — overview style, 2026-10-30

Exact delta against `/tmp/sv-b2-overview-baseline.tar`; Graphify query before reading sources. Reviewer edits no production code/tests. Reviewed endpoint scene/layout, shell overview backing and native Back mapping; minimum1024 screenshot inspected.

Findings corrected and re-reviewed:

- P2 closed: `b2_character_text` now intersects an enabled parent list clip, skips empty intersections and restores prior clip. Child text cannot replace/expand its parent viewport.
- Small-track robustness closed: empty input/nonpositive boxes and empty post-ellipsis text return before TTF render.
- Mode compatibility closed: combined0x44→IDDC mapping retained; standalone1/2/0x40/0x80 names restored; canonical combined modes also recognized. Unknown bits keep hexadecimal fallback; sex/body/eligibility flags do not become mode names.

Other reviewed paths: layout changes restricted by `character_name_entry`, preserving ordinary-name geometry; overview Back click/focus-Enter now CANCEL while Q remains QUIT; renderer and native hit testing share the same rects. Overview backing is scoped to overview&&!name_entry. Capacity subtitle derives count+free slots. Unsupported S/I/A reorder hint is omitted instead of advertising an unimplemented binding. Creation flags/disabled states remain model-owned. Minimum1024 layout visible without gross overlapping/footer geometry issues. Existing free-slot order and wire modes remain authoritative.

Final source review: no open functional findings after corrections. List AA clip expands outward by one output pixel consistently with prior server fix. `git diff --check` is clean. Root owns final existing Linux checks/runtime evidence. Minor visual precision observation sent implementer: source level number is bold70010px serif while current heading helper uses regular serif; no protocol/input effect, do not claim pixel-perfect equivalence.

Final check adaptation reviewed: two resize/scroll assertions now compare against `viewport.rows`, the row count recalculated by production sync after changing `first`, rather than the stale by-value input layout.rows. This keeps the selected-visible invariant and follows the renderer/native production recomputation, not a weakened acceptance criterion. Back mouse and focused Enter regression assertions remain production adapter calls. Conditional disabled-action explanation reviewed: only focused/hovered ordinary/exclusive action replaces normal count text with creation unsupported / first-login / no-slot cause; actual enabled flags stay model-owned. No new findings.
