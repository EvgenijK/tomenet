# C011 actual-server probe 05 — MOTD/resource assessment

2026-11-11. Scoped independent review; no production source changes.

Probe 05 reached phase 7 (MOTD) after the actual PLAY reply/startup END and
returned result 1. Its generic final SDL error was `Parameter 'texture' is invalid`.
This was initially interpreted as a failed `draw_scene`, but the actual-server
agent confirmed that no explicit false draw call had been observed or instrumented.

The one-off probe's native birth driver did not acknowledge MOTD and had a
600-frame limit. Exiting that limit retains the attempt's result 1. Current
assessment is a probe/gesture limitation, not a confirmed production resource bug;
the next probe will acknowledge the actual MOTD view through a native Return event.
Probe 05 remains historical incomplete evidence and is not relabeled successful.

Independent resource audit:

- MOTD uses the shell's separately opened LiberationMono font, checked at shell
  initialization. The identical copied library contains that font and shell open
  succeeds. It does not borrow the C011 retained MapFont snapshot.
- The new C011 resource adoption/destruction runs after the preparation/scene
  boundary, so it had not released native fonts/textures at the observed MOTD
  phase. No path demonstrating C011 ownership invalidation was found.
- A short replay called the actual compiled production `sv_login_shell_motd`
  with the same probe-05 library and simple, whitespace-only and colored text.
  All returned true; all nevertheless left the same SDL texture error.
- `login/ui/login-shell.c:811–815` moves the new MOTD texture into its cache and
  later calls `SDL_DestroyTexture` on the now-null local pointer. That successful
  cleanup explains a stale texture error and makes the final error string
  unsuitable as evidence of the original failure. The unrelated diagnostic
  improvement was reported to root for the separate improvements list.

Replay artifacts:

- `.scratch/sv-c011-followup/motd-resource-probe.c`
- `.scratch/sv-c011-followup/motd-resource-probe.stderr.log`

The replay linked existing production `login/ui/login-shell.o`, `ui/font.o` and
`files/files.o` with SDL3/TTF/FreeType/PNG; dummy video and software rendering.
It did not connect to a server, use an account, implement alternate behavior or
change production code. Its three return values were 1. It is a diagnostic,
not a replacement for exact-field/native actual-server evidence.

Verdict: no own C011 production fix is justified by probe 05. Correct the fixture
MOTD acknowledgement and rerun the bounded actual-server path. Preserve the
unconfirmed result and the frame/gesture limitation in final evidence.

## Exact actual setup field replay

The actual-server agent subsequently captured the 2760-byte MOTD setup field at
`.scratch/sv-c011-followup/real-server-07/captures/actual-motd.bin`.
The same production replay, still using probe-05 copied assets, returned true for
that exact field (`columns=55`) and again left the stale texture error.

```bash
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software \
  .scratch/sv-c011-followup/motd-resource-probe \
  .scratch/sv-c011-followup/real-server-05/lib \
  .scratch/sv-c011-followup/real-server-07/captures/actual-motd.bin
```

Exit 0; production `sv_login_shell_motd` result 1. This independently rules out
missing probe-05 font/assets or the actual MOTD field as a reproducible cause of
the claimed draw failure. The actual-server agent will verify native gesture
completion separately in probe 08.


## Final native actual-server receipt

The corrected bounded actual-server probe 12 completed native MOTD acknowledgement
and reached phase 8 with preparation complete for Born666; retained deferred
frames remained available. Root's final reconciliation records client exit 0,
isolated networking, and all 149 production source hashes matching the final
current tree. Receipt: `.scratch/sv-c011-followup/real-server-12/command.json`.
The actual-client binary is
`e1c6fc594bb8e4dd814bdc1fc0e43536079261d92a5eaffa6357354585170862`.

The final managed eleven-case run also exited 0 using binary
`efe3166a29eb34f4b6dac8945fbe3715110301b4e8c21a8b5800ecbac715a75b`;
all 152 recorded source/header hashes match current files, and original libraries
remain unchanged. See `.scratch/sv-c011-followup/final-reconciliation.json` and
`final-checks.log` in the same directory.

Final reviewer verdict: the native MOTD path passes; probe 05 remains an
unconfirmed draw-failure claim limited by the fixture frame cap and missing
acknowledgement, rather than evidence of a production ownership defect. The later
P2 preparation-status overwrite was fixed by root's two native-run display gates
and independently checked: identity status requires the post-MOTD nonfailed
preparation surface, and PAUSE status cannot replace a failure. The final eleven
checks include that source. No additional production MOTD/resource fix is needed.
