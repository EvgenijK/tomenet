# B2 interactive bug review — Linux, 2026-10-29

Independent review compared the task delta with `/tmp/sv-b2-interactive-bugs-baseline.tar`, rather than reviewing unrelated working-tree changes. Production scope: `ui/login-shell.c`, `input/native-endpoint.c`, `input/credentials.{c,h}`, `endpoint-run.c`; minimal regression additions use existing core/scene tests.

One actionable finding was returned and fixed: visible/focused Back must cancel the credential surface directly, since contextual Escape at password stage only clears the password and returns focus to account. The implementation now calls `sv_credentials_cancel`; contextual keyboard Escape keeps its established staged behavior. No open production findings after correction. Temporary diagnostic cleanup and full existing endpoint check results are owned by the implementation/orchestrator.

Actual X11 input validation used the production executable, a fresh isolated temporary profile, frozen background 0, reduced motion, a 1440×900 test window, native software and OpenGL renderers. XTest sends mouse motion, waits 200 ms, then actual presses/releases; actual current startup window is located by title and PID. OCR checks are confined to the central surface, and screenshots were inspected independently. Temporary gesture script: `.scratch/sv-b2-interactive-bugs/runtime.py` (one-off diagnostic, no new test framework).

Baseline red: row double click leaves the server list; door opens account, and account Escape closes the process. Baseline contextual Back after password clears the password but leaves account. `baseline2-software.json` contains additional failures downstream of those expected baseline failures; it is not a final acceptance result.

Both final renderer runs passed: first server row double click opens credentials; test account and password can be entered and revealed; actual visible Back returns to the list with the same client process/window alive; a different row double click opens the corresponding host; old account/password are empty and reveal state is reset; Escape from the empty account returns to list alive. Software additionally confirmed actual Alt-F4 closes the window/process. OpenGL Chronicle fetched all nine HTTPS sources and remained visible across Back and reentry. Network fetches vary per run; the first software final attempt returned unavailable, whereas subsequent evidence records the actual source results without substituting offline data.

Final screenshots show softened artwork with crisp logo/cards/text in both renderers. Original full-resolution PNG bytes are unchanged; explicit blur and contrast are prepared from decoded full-resolution pixels by the production rendering path. Blur numerical red/green metrics and memory/timing are recorded separately by the implementation agent.

Commands:

```sh
python3 .scratch/sv-b2-interactive-bugs/runtime.py /tmp/sv-bugs-final final software
python3 .scratch/sv-b2-interactive-bugs/runtime.py /tmp/sv-bugs-final final opengl
```

The named executable was copied from the Linux production build so test scripts could rebuild without an executable-in-use copy failure. These gesture checks do not authenticate or connect a real account; `nonsecret` is deliberately fabricated test data in a disposable profile.
