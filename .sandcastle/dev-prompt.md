$implement

Work on the requested TomeNET SV change. Read AGENTS.md, CONTEXT.md, and the
relevant docs and ADRs first. Keep the SV implementation isolated as directed by
AGENTS.md. Build with `make -C src -f makefile.sv tomenet-sv` and run the relevant
production-path checks from `tests/`. Do not change legacy or shared code merely
to remove duplication. Follow the implement skill, including its TDD guidance
when the relevant test seams are already agreed. Commit the work to the current
branch before ending this session. Sandcastle runs the required code-review skill
as the next workflow stage, after your commit and the production-path checks.
Report what changed and which checks passed.
