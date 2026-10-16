# Issue tracker: Local Markdown

Issues and specs for this repo live under `.scratch/`.

- One feature per directory: `.scratch/<feature-slug>/`.
- Its spec is `spec.md`.
- Tickets are separate files in `issues/`, named `01-<slug>.md`, `02-<slug>.md`, and so on.
- Record triage state in a `Status:` line near the top of each ticket.
- Append discussion under `## Comments`.

When a skill says “publish to the issue tracker”, create a file there.
When it says “fetch a ticket”, read the referenced local file.

For `/wayfinder`, use `.scratch/<effort>/map.md` and numbered child
tickets in `.scratch/<effort>/issues/`. A child ticket records its
`Type:`, `Status:` and, when needed, `Blocked by:` ticket numbers.
Claim by setting `Status: claimed`; resolve by recording `## Answer`,
setting `Status: resolved`, and adding a context pointer to the map.
