# Stage A ticket 20: complete renderer startup before session input

The first urgent submission exceeded 20 ms because deferred work from the first
Wine Direct3D presentation was paid by the next texture upload. SV now completes
the initial presentation and draws into its replacement backbuffer during normal
startup. A one-pixel readback completes that work before the second presentation;
no session has been opened and no request has been decoded at this point.

`sv_ui_start` owns this sequence; `main` calls it for every launch, including the
ordinary synthetic shell. Failures propagate through the existing startup error
path. Per-frame rendering, fonts, protocol/model/input/serializer behavior and
submission timing are unchanged. The timing scenario no longer performs its own
initial presentation. Its first decoded request remains sample 1, with the same
20 ms urgent and 50 ms interactive budgets and monotonic clock.

Readback occurs after drawing and before present, as required by
[SDL_RenderReadPixels](https://wiki.libsdl.org/SDL3/SDL_RenderReadPixels).
The pixels are discarded and never interpreted as semantic UI state. This is a
startup synchronization cost, not a per-frame readback or a measurement of visible
latency. The exact lower-level Wine/driver operation was not identified; the
measured boundary is the first presentation and the subsequent resource upload.

## Diagnosis

Baseline PE SHA-256:
`10a395f835f9640053ec7227c25fa2682af8823ec9246e88cd037e34a6879db0`.
Baseline reproduced in a fresh isolated Wine prefix with freshly staged PE/DLLs:
SDL video `windows`, actual renderer `direct3d`, 1024×768, display scale 1.0,
Wine 11.17, SDL 3.4.0, SDL_ttf 3.2.2, FreeType 2.13.3, i686.
First request: **44.137 ms**, remaining normal samples 0.641–1.184 ms.

Temporary instrumentation bracketed TTF text rasterization,
`SDL_CreateTextureFromSurface` (allocation and upload), render enqueue, texture
destruction (which flushes queued use), full draw and `SDL_RenderPresent`.
The measured first request was 34.636 ms: first texture allocation/upload
**33.971 ms**, total draw **34.614 ms**, present **0.011 ms**. That line's
rasterization took **0.004 ms**, enqueue **0.002 ms**, destruction **0.033 ms**.
Instrumentation printed between calls and could perturb timing; the original
uninstrumented failure and final uninstrumented checks are retained independently.
All temporary instrumentation was removed from production sources.

Two discarded experiments distinguish the deferred work from text rasterization:

- An ASCII atlas removed uploads after startup but moved a **34.357 ms** wait to
  a later submission. It was not retained as the fix.
- Readback before the very first presentation still left the first request at
  **37.534 ms**. Synchronizing after the first presentation removed the wait.
  The final sequence redraws before readback so it follows SDL's documented API
  order; it does not read an undefined post-present framebuffer.

Diagnostic runs and failed experiments are retained, not retried into a passing
result. No renderer substitution, budget increase, excluded sample or lowered
urgency is used.

## Independent final-build checks

Three consecutive invocations of the existing `tests/sv_timing_native.py` each
started new positive and delayed-control processes with temporary SV profiles.
Run 1 used a freshly initialized `/tmp/sv20-independent/wine-prefix`; runs 2–3
reused that prefix, never a client process. OS/driver disk caches were not purged,
so “fresh” means prefix/process/profile, not a physically cold machine.

| Run | First normal urgent submission | Result |
|---|---:|---|
| Fresh prefix | 1.447 ms | Positive pass; delayed control rejected |
| Existing prefix, new processes | 1.078 ms | Positive pass; delayed control rejected |
| Existing prefix, new processes | 1.310 ms | Positive pass; delayed control rejected |

All eight normal samples in each run passed their original budgets. Every delayed
control retained the intentional HP and cancellation violations. The full gate
uses another fresh Wine prefix and retains its own independent samples.

## Final acceptance

The final `tools/run_stage_a.py` run in `/tmp/sv20-complete-evidence` passed
**62/62 commands**, exit **0 / accepted**, on 2026-09-23. All eight scoped A
outcomes are covered by fresh production evidence; the independent human review
matches the final executable, fonts and host dependency closure. The user explicitly
confirmed: **«Все действия проверены, замечаний нет; условия прежние»**.
Native review exited successfully on Linux/Wayland software, initial output
1357×1018, logical 1024.151×768.302, display scale 1.325. This records subjective
operation/response in the stated conditions, not measured visible latency.

| Backend | Maximum normal urgent / 20 ms | Maximum normal interactive / 50 ms |
|---|---:|---:|
| Linux software | 3.569 ms | 2.535 ms |
| Linux OpenGL | 1.083 ms | 1.076 ms |
| Wine software | 0.791 ms | 0.669 ms |
| Wine Direct3D | 1.057 ms | 0.932 ms |

All four delayed controls rejected their intentional violations. Both SV and
legacy builds, all data/checker tests, five sanitizer suites, HP/message/request/
lifecycle scenarios, shell/resource checks and TTF/PCF virtual geometry passed.
The canonical registry remains 925 pending / 0 full accepted capabilities,
24 inventories and 2,077 reconciled rows. Scoped A acceptance does not promote
full-client claims or B–F, actual Windows, live gameplay or shipping ABI.

Final SHA-256:

- Linux: `a46805a94ca83caf0fb8be9f5b9f95d2d2e0b20f5e6cd60a038a414dbe09bb90`.
- MinGW/Wine: `305fe959faa32bf13ce02009eba95b2570760e55dbd6abf7d784237a8bddb3c5`.

[Command report](acceptance/stage-a-ticket-20-2026-09-23.json) and
[evidence archive](acceptance/stage-a-ticket-20-2026-09-23.tar.gz) retain all
normal/delayed timing samples, runtime/dependency evidence, configurations,
source/log fingerprints, diagnostic logs and probe patch, independent runs and
current human review. Archive SHA-256: `be6d419738e914f30e3ac7d152280f1b31802d3c4da502ccb9eff095f637f739`.
Large identical provenance artifacts use tar hard links; every extracted artifact
is inventoried by SHA-256. Binaries, SDK and Wine prefixes remain external roots.

An earlier full attempt in `/tmp/sv20-final-evidence` failed to copy the Linux
binary because the manual review process was running (`Text file busy`). Linux
checks were consequently skipped and candidate evidence failed. That report and
its logs are retained under `earlier-build-busy-gate/`. The final full rerun followed
successful manual-process exit; this was a resolved build scheduling failure,
not a timing retry. Its Wine Direct3D timing had also passed.

Reproduce with a fresh output directory after completing/closing the manual
review process (see [human workflow](sv-human-review.md)):

```sh
/tmp/sv15-venv/bin/python -B tools/run_stage_a.py \
  --output /tmp/sv20-new-evidence --mingw-sdk /tmp/sv16-sdk-final \
  --sdk-downloads /tmp/sv16-downloads --human-review /tmp/sv20-human/review.json
```

The automatic checkpoint object's `humanReview: pending` is its intentionally
separate machine-only status. The top-level report's `humanReview: approved`
and `acceptance: accepted` combine that checkpoint with the verified human record.

## Standards

Independent review against `a683d5df0b256cd7ee9d9d6132d72435a5463bc8`:
**0 findings**. Changes remain in SV; initialization belongs to UI, sequencing to
application startup. Tests exercise the production startup boundary.

## Spec

Independent source review: **0 implementation findings**. Independent repetitions,
full-gate publication and current human review are now complete as recorded above.
