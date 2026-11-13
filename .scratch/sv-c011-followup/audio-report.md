# C011 follow-up — startup audio and retained lifetime

Date: 2026-11-11. Scope: C011 startup projections only; Linux.
HEAD inspected: `c29c37a153053779dddb659844315abc37dbd197`, dirty workspace preserved.

## Finding and implementation boundary

The previous C011 native evidence exercised only disabled audio. Accepted C002
unit/startup evidence was useful traceability, but did not prove enabled packs,
fallback or no-device in the actual C011 caller and retained payload.

Production source audit found no change required in `settings/audio.c` or the
C011 native ownership closure:

- Requested configuration is copied into the initial audio result before device
  open. Failure produces an explicit optional status and zero advertised counts;
  it does not reject the session.
- Pack configuration search uses U config, bundled config, U `.default`, bundled
  `.default`; individual samples resolve U then bundled. Requested folders and
  subsets remain separate from effective pack metadata and configuration paths.
- Effective stereo device format, allocated mixer tracks, buffer and cache-start
  result are observed from the actual SDL3/SDL3_mixer initial open.
- Preparation copies the snapshot and named requested resource refs. Native
  handoff adopts the same application audio owner into the retained native bundle.
  Destroy waits for its cache thread, frees decoded samples/tracks/mixer and closes
  SDL audio before destroying the remaining native bundle.

Relevant production paths: `settings/audio.c:sv_settings_audio_start`,
`pack_config`, `pack`, `close_device`; `login/native-run.c:sv_endpoint_run`,
`destroy_preparation_native`; `login/preparation.c:sv_preparation_create`;
`protocol/login.c:sv_login_set_audio/sv_login_start_play/sv_login_send_audio`.

## Focused verification plan

The existing `tests/sv/first-session.c` and
`tests/sv_first_session_checks.py` editor is agent `preparation_contract`.
Three cases were requested in that existing managed-peer/native preparation path:

1. Enabled dummy device, four tracks, 48000 Hz requested, 128-frame requested
   buffer and initial cache. Selected sound subset 2 and music subset 3. U sound
   config/sample and bundled music `.default` config/sample exercise actual
   overlay/bundled fallback. Expect READY, event counts 1/1, exact selected
   names/versions and config paths, actual effective device values, retained
   requested refs and complete preparation.
2. Missing requested soundpack: PACK_FAILED, advertised counts 0/0 and empty
   protocol metadata, requested missing identity preserved and complete
   preparation with visible diagnostic.
3. Enabled configuration with unavailable SDL audio driver: NO_DEVICE,
   advertised counts 0/0, requested settings retained and complete preparation.

All three require exact audio count bytes in the actual PLAY and subsequent
PKT_AUDIO reply, retained snapshot checks after native return, unchanged requested
CFG, and destruction checks. Enabled audio additionally checks that SDL audio
remains initialized after return and is closed by preparation destruction.

## Commands and evidence

Read-only graph navigation:

```bash
.graphify-venv/bin/graphify query 'C011 startup audio packs fallback device preparation retained audio' --budget 1800
```

The graph supplied navigation; current sources were inspected directly because
this worktree already contains dirty production changes.

Runtime commands/results and exact artifact paths will be appended after the
coordinated authoritative focused run. No independent broad suite or second audio
harness was created. No production audio changes have been made by this audit.

## Limits

Dummy provider evidence concerns real SDL device/mixer APIs, not a physical sound
card or audible playback. Live playback, pack chooser/reconfiguration, explicit
Save and their C055/C056 owners are outside this request. No Windows/MinGW/Wine,
accelerated visual matrix, real account or public-server run is included.

## Coordinated helper artifacts

Prepared for the sole existing-harness editor, without editing its shared files:

- `audio-fixture.inc`: three deterministic pack/device fixtures and retained
  audio/configuration assertions; no alternate audio implementation.
- `identity-redraw-prefix.patch`: necessary C011 repeat-identity and coherent
  redraw assertions requested by root after reviewing the current task/evidence.
- `prefix-checks.inc`: two bounded new-framer compatibility paths (old RLE marker
  and modern dual-mask four-byte glyphs), with partial/exact consumption checks.

The existing full C011 suite remains the runtime authority. Helpers are integrated
by the assigned editor; scratch snippets are not standalone runtime evidence.

Environment review identified a necessary harness correction before the audio run:
installed SDL3 defines `SDL_HINT_AUDIO_DRIVER` as `SDL_AUDIO_DRIVER`; the historical
runner set only legacy `SDL_AUDIODRIVER`. The editor was asked to set both aliases
to `dummy` (or the intentionally unavailable driver for the no-device case), record
both values, and remove/explicitly control inherited audio buffer overrides. This
ensures failure/success comes from the selected production branch, not an unrelated
host provider. This is a harness correction; production audio policy is unchanged.

## Observed focused development results

The assigned editor/root ran the existing runner, using the production
`sv-first-session-check` make target:

```bash
python3 tests/sv_first_session_checks.py --case c011-audio-enabled
python3 tests/sv_first_session_checks.py --case c011-audio-missing-pack
python3 tests/sv_first_session_checks.py --case c011-audio-no-device
```

All three observed development runs exited 0. Artifacts are
`.scratch/sv-c011-followup/<case>/{command.json,build.log,stdout.log,stderr.log,file-transcript.log}`.

| Case | Observed actual preparation result |
|---|---|
| `c011-audio-enabled` | READY; requested 48000 Hz / 4 tracks / 128 frames; effective 48000 / 4 / 128, stereo=2, cache started, provided events 1/1, complete=1 |
| `c011-audio-missing-pack` | PACK_FAILED; requested values retained, effective values 0, provided events 0/0, visible diagnostic, complete=1 |
| `c011-audio-no-device` | NO_DEVICE with both SDL driver aliases intentionally unavailable; requested values retained, effective values 0, events 0/0, visible diagnostic, complete=1 |

The successful enabled initial projection reported exact pack metadata
`Sound chosen`, version `2`, `Music chosen`, version `3`. Its actual captured
PKT_AUDIO bytes were:

```text
c300010001536f756e642063686f73656e0032004d757369632063686f73656e003300
```

Both degradation cases emitted exact zero counts and four empty metadata fields:

```text
c30000000000000000
```

The peer independently asserted matching actual PLAY count bytes; retained checks
ran after `sv_endpoint_run` returned. They verified selected resource names/subsets,
U sound config and bundled music `.default` resolution, actual initial device
values, initialized audio lifetime until handle destruction, final audio shutdown,
and unchanged requested CFG bytes. Original repository `lib` hashes matched before
and after each run. Both driver aliases and the selected unavailable driver are
recorded in command metadata; inherited buffer override is explicitly removed by
the runner for these cases.

At this development review, missing-pack/no-device source hashes matched the then
current tree. Enabled used an earlier binary (`9f2c653a…`), preceding subsequent
shared-harness edits; therefore this section records development observations, not
the final frozen-source acceptance run. Root's coordinated final run must supersede
these snapshots before final evidence is claimed. No physical playback, live audio
or later-stage capability is inferred from these results.

## Earlier coordinated receipts (superseded)

Root completed the frozen-source combined run and production build:

```bash
python3 tests/sv_first_session_checks.py --case c011-preparation
make -C src -f makefile.sv -j4 shell
```

Both exited 0. The runner now dispatches all eleven scoped C011 cases, including
all three audio cases. Combined logs:
`.scratch/sv-c011-followup/final-checks.log` and
`.scratch/sv-c011-followup/build.log`.

All three final audio metadata files report exit 0 and the same check binary:

```text
70f93d6c5aacc02c01124e551b95989d81167030e95b74fc3a4f7cdf341b0d32
```

At that coordinated run I independently compared all 152 recorded production/check source and header
hashes with the then-current files for each of the three final audio cases: all match.
Each also records unchanged original repository library hashes. The final runs
reproduce the table above: READY 1/1 at 48000/4/128 stereo2/cache1, PACK_FAILED 0/0,
and NO_DEVICE 0/0, all with preparation complete=1. The captured exact PKT_AUDIO
bytes are unchanged from the values above; actual PLAY counts, retained requested
refs and snapshot, fallback paths, no implicit CFG write and final SDL audio
shutdown assertions passed in all applicable cases.

This final receipt supersedes the earlier development snapshot limitation for
startup audio. Independent review found no remaining C011 audio/resource blocker.
Production audio code required no fix; shared C011 controls/ingress fixes and all
late-owner/runtime limitations are documented separately by their owning reports.


## Final reconciled receipts

The final root-owned all-eleven run exited 0 after the necessary startup-prefix
and preparation UI fixes. `final-reconciliation.json` records the same binary for
all eleven cases, including enabled, missing-pack and no-device audio:

```text
efe3166a29eb34f4b6dac8945fbe3715110301b4e8c21a8b5800ecbac715a75b
```

All cases record 152 source/header hashes matching the final current tree,
unchanged original library and exitCode 0. The reconciliation also records
canonicalUnchanged=true. This receipt supersedes the earlier 70f93d6c… run;
that earlier hash remains above as development history. Commands and output are
retained in `.scratch/sv-c011-followup/final-checks.log`; reconciliation is in
`.scratch/sv-c011-followup/final-reconciliation.json`.

Independent final review: the P2 failure-status overwrite is resolved by the two
native-run gates: identity text requires `scene.preparation_identity`, and PAUSE
text requires `!flow_view.failed`. The final eleven-case run covers the resulting
source. No remaining C011 audio/resource blocker was found. These receipts do not
claim physical playback or later gameplay/audio capability.
