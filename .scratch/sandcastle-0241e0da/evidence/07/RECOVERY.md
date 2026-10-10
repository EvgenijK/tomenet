# Recovery acceptance records

Contract: `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

The isolated `codex/sandcastle-0241e0da-wave-4-1` checkout already contained
worker commit `7bcaf3a502883c5867bd4695a3bb2bb7f0b00c92` and prerequisites 01–06.
Recovery retained that assembly and all completed children. No other branch was
edited or merged. Recovery guidance explicitly authorized the full gates and
independent controller audit that the initial worker assignment reserved.

## Actual gate results

[gate-results.json](recovery/gate-results.json) records commands, actual exit
codes, environment, transcript sizes and SHA-256. Compressed transcripts retain
the original bytes; decompress with `gzip -dc`. All checks here ran against
worker HEAD `7bcaf3a502883c5867bd4695a3bb2bb7f0b00c92`; the resolution commit
changes documentation and gate records only.

| Check | Actual result | Transcript |
| --- | --- | --- |
| sv-build: `make -s -C src -f makefile.sv tomenet-sv` | exit 0 | [build](recovery/sv-build.log.gz) |
| First core attempt: `bash -s -- core < .sandcastle/checks.sh` | exit 1, system Python missing jsonschema | [failed environment attempt](recovery/sv-core-system-python.log.gz) |
| Runtime environment recheck: `PATH=/opt/sv-venv/bin:$PATH python3 -B tests/sv_runtime_checks.py` | exit 0 | [focused recheck](recovery/runtime-environment-check.log.gz) |
| sv-core: `bash -s -- core < .sandcastle/checks.sh`, configured venv PATH | exit 0, all 23 suites and software shell smoke | [complete core](recovery/sv-core.log.gz) |

The configured environment already includes `jsonschema==4.26.0` in
`/opt/sv-venv`, as declared in `.sandcastle/Dockerfile` and
`tools/requirements-capabilities.txt`. The first login shell chose system
Python; the complete core rerun used the declared venv. No dependencies, source,
runner checks, filters or limits were changed. The failed attempt remains a
failure in the record. Existing shared `atol` warnings remain covered by the
existing improvement entry; no new improvement was found or implemented.

## Fresh production evidence

Before resolution, the actual command was:

```sh
python3 tests/sv_birth_runtime_evidence.py /tmp/sv-b024-recovery/pre-final-runtime
```

Exit 0 at worker HEAD, six suites, 108 native endpoint sessions: ordinary 3,
versions 40, dedicated 20, submission 18, replies 17, Guide/MOTD 10. Both
independent reviewers verified all 347 artifact fingerprints, source
fingerprints, unchanged inputs and the exact contract/HEAD. Actual authoritative
success bytes are `02 03 7a`; the packet explains the incorrect current Guide
log prose. The [original scenario table](SCENARIOS.md) remains historical
parent-HEAD evidence; it is not relabelled as a later capture.

The final audit uses a fresh directory outside the Git tree:
`/home/agent/sv-b024-final-audit/<exact-final-HEAD>/runtime/manifest.json`.
Independent final verdicts and `acceptance.json` live beside `runtime/`.
These receipts bind the commit's own hash without making another commit that
would invalidate that hash. The final collector must run after the resolution
commit, and after any subsequent commit. A future integration HEAD requires
its own fresh audit; unchanged source fingerprints do not waive that gate.
The collector's own manifest keeps build/core/review pending because it does
not execute those checks; the separate gate records and independent final
verdicts establish their actual results.

## Controller review

Independent standards and spec reviewers assess production behavior and scope
separately; author evidence is not a controller self-review. Pre-resolution
verdicts are retained under `recovery/`; the exact final-HEAD verdicts are in the
external final audit directory above. The public trace remains native caller
`sv_endpoint_run` → SDL birth interaction → committed draft → production
`sv_login_start_play` serializer → TCP peer → `sv_login_receive` → production
login/pregame/lifecycle reply owner. Guide invokes the actual production caller;
peer MOTD ends at live-session handoff with gameplay inactive.

## Scope and disposition

P1-ac1–ac8 and P2-3/P2-4 use real production runtime evidence plus independent
review and the actual full gates. P1-ac9–ac11 retain the historical filename,
SV-B-023 dependency and startup screen flow boundary. SV-B-023 full/native
acceptance stays pending at its original owner. DNA persistence, profile/resource
workflows, real-server integration, session.enter-game and gameplay remain stage
C; handshake resource bytes are included in B. **P1-ac12/P1-entry-evidence are
deferred, owner SV-B-025**, neither executed nor passing evidence here.

P2-1/P2-2/P2-7: recovery changes no production, legacy or shared code and does
not impose consolidation. P2-5/P2-6: no new organizational/legacy/shared finding;
existing improvement proposals stay separate. P2-8–P2-10: local Markdown,
separate Labels/Status, root CONTEXT and ADR-0001–0006 govern terminology and
ownership; originating parent is unchanged and not closed. This remains an
evidence-only slice, with no behavior-changing implementation or claimed
red/green loop.
