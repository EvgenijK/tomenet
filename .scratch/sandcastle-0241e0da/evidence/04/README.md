# 04 single-submission focused evidence

Captured 2026-10-10 on `codex/sandcastle-0241e0da-wave-2-3` through public
`sv_endpoint_run(SvEndpointOptions)`, real loopback TCP and native SDL events
with SDL's dummy video driver/software renderer. No test-only serializer or
behavior implementation is involved. Immutable contract:
`0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

Reproduce all cases:

```sh
SV_BIRTH_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/04 python3 tests/sv_birth_single_submission_checks.py
```

Text logs trim trailing whitespace; binary captures are unmodified.

The 14 quit/cancel logs come from the complete 18-case run. The four completion
captures/logs were refreshed after adding the explicit two-second keepalive
interval check:

```sh
SV_SUBMISSION_CASE=waiting,partial,ack,retry SV_BIRTH_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/04 python3 tests/sv_birth_single_submission_checks.py
```

Each completion `.bin` is one full request compared byte-for-byte against
`tests/sv/fixtures/birth-current.hex`, whose independent baseline derivation is
in `tests/sv/fixtures/birth-current-baseline.py`. Ping/pong packets and the
explicitly consumed timer keepalive are excluded from these request captures.
The `.bin.reply` files contain respectively zero, two, three and three actual
reply bytes. Logs show pending versus acknowledged startup phases and retry's
prior/current generations. No gameplay surface is required or opened.

Quit cases observe each incomplete legal owner. Cancel cases backtrack first,
then close the native window. Both inject confirmations after closure and assert
TCP EOF with no final bytes; their logs record closed lifecycle state. The retry
case closes an unfinished generation and completes a new one through native R.

- `waiting.bin`: 16266 bytes; SHA-256 `a376d02cb3a5cab0bc6138daf27ec3fd5f3730e3f2d2e5da59ed6d036516f670`.
- `partial.bin`: 16266 bytes; SHA-256 `a376d02cb3a5cab0bc6138daf27ec3fd5f3730e3f2d2e5da59ed6d036516f670`.
- `ack.bin`: 16266 bytes; SHA-256 `a376d02cb3a5cab0bc6138daf27ec3fd5f3730e3f2d2e5da59ed6d036516f670`.
- `retry.bin`: 16266 bytes; SHA-256 `a376d02cb3a5cab0bc6138daf27ec3fd5f3730e3f2d2e5da59ed6d036516f670`.

These artifacts establish focused implementation evidence. The controller's
review/fresh final audit and orchestrator's full build/core gates remain pending.
