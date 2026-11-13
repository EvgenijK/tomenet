# Pregame native keyboard clock regression

Actual user traces are retained separately by the orchestrator. Observed fixed-key timestamps precede scene entry while their poll times and TEXT_INPUT are fresh. The old scene/generation guards prevent the handler from running.

Production freeze: `final.sha256`. No gameplay adapter, wire protocol or credentials implementation changes in this delta. No new test suites. New minimum persistent case is in existing `tests/sv/native-endpoint.c`.

## Red evidence

- `birth-red.log`: newly dispatched male/female shortcut carrying timestamp1 is ignored at a later since_ns.
- `name-red.log`: production caller polls Shift+N/Enter with timestamp1; controlled peer receives no character-name request.
- `birth-boundary-red.log`: old production adapter changes SEX→CLASS but leaves queued Enter/TEXT for the next owner; assertion fails. Same production call and queue assertions pass after the fix.
- Earlier credentials red is `.scratch/credential-producer/caller-red.log`: freshly polled keys4, handled0, Backspace0, Tab0.

## Green evidence

- `credentials-green.log`: fresh keys3, handled3, Backspace1, Tab1, old opening text leak0. Held Enter repeat does not submit.
- `birth-green.log`, `grid-green.log`, `guards-green.log`, `birth-boundary-green.log`: strict clang+UBSan production adapter checks pass. Sex/class/race/trait letter shortcuts and grid arrows work despite timestamp1; repeats/modifiers remain guarded; pending Enter/TEXT are discarded at owner change; USER event retained.
- `native-endpoint-green.log`: existing core test, including late-clock list/manual keys and queue boundary, passes.
- `name-birth-green.log`: full production endpoint with a controlled local 4.9.4 peer and synthetic credentials. Every freshly polled keyboard event gets timestamp1 by a scratch wrapper. Opening name-entry command also queues `OLD` TEXT and Enter before the caller boundary; neither leaks. The later Carol name is submitted, birth completes, exact16266-byte legacy golden PLAY packet matches, authoritative ACK reaches live-session-handoff.
- `debug-build.log`: same diagnostic binary rebuilt from frozen source, safe logging wrapper remains scratch only.

## Rebuild and run complete production feedback

```sh
python3 .scratch/pregame-clock/build.py
timeout 8s env SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software DBUS_SESSION_BUS_ADDRESS=unix:path=/tmp/sv-no-vault stdbuf -o0 .scratch/pregame-clock/endpoint .scratch/creation-next/profile /home/svechnik/Projects/tomenet_modern_client/lib ack tests/sv/fixtures/birth-current.hex .scratch/pregame-clock/play.bin 4.9.4.0.0.0
```

The controlled local peer needs sandbox network escalation. It never contacts or creates an account on a real game server. Chronicle uses the existing public background provider.

## Queue and release semantics

Barriers run only at ownership changes, not every frame. Pump first lets SDL update its internal pressed/released state; flushing pending app-level KEY_DOWN/UP/TEXT_INPUT/TEXT_EDITING does not fabricate or modify SDL state. A physically held opening key continues as repeat=true and remains suppressed in editor/birth activation paths. Close/window/render/user/network events are retained. Pointer gesture clock filters and session generation identity checks remain. Native keyboard/text timestamps alone no longer decide queue age.

The existing synthetic full-birth fixtures prequeue multiple owners in one batch, contrary to the new explicit ownership barrier; the scratch golden peer instead paces input. No broad fixture refactor was made in this task.
