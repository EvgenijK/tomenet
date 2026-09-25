# Issue tracker: local Markdown

GitHub remotes host code; agent skills use local Markdown issues in this repo.

## Locations

- Wayfinder planning: `.scratch/<effort>/map.md` and one decision ticket per file at `.scratch/<effort>/issues/NN-<slug>.md`.
- Existing SV implementation tickets: `docs/tasks/`, including `docs/tasks/stage-b/`. Resolve a supplied SV ticket ID through the stage index and its file; keep the stage's allocation and coverage rules.
- New Wayfinder efforts use `.scratch/<effort>/`. New tickets for an existing SV implementation stage follow that stage's `docs/tasks/` conventions. Other local feature efforts use `.scratch/<feature>/issues/`.
- Keep existing tickets in place; do not create a second copy in another location.

## Operations

- Fetch a ticket by its path or unambiguous SV ID. A bare number is ambiguous across Wayfinder efforts: ask for the effort or path.
- Publish a new file at the owning location and add a link to its map or stage index when that index applies.
- Append discussion under `## Comments`; preserve earlier decisions and history.
- Represent triage roles in a separate `Labels:` line, for example `Labels: enhancement, needs-triage`. Wayfinder `Status:` records `claimed`/`resolved`; SV `Статус:` records implementation and acceptance. Do not replace either with a triage role.
- Existing allocated SV tickets do not enter the triage intake merely because they have no `Labels:` line.

## Wayfinding operations

- Map: `.scratch/<effort>/map.md`; child tickets: `.scratch/<effort>/issues/NN-<slug>.md`.
- A child records `Type:`, `Status:`, `Assignee:` and optional `Blocked by:` near the top. Open, unblocked, unclaimed children form the frontier; choose the first by number.
- Claim by setting `Status: claimed` before work. Resolve by appending `## Answer`, setting `Status: resolved`, and adding a short link under the map's `## Decisions so far`.
