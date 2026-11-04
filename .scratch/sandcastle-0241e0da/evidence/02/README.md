# Slice 02: version boundary endpoint evidence

Contract: `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.
Seam: `sv_endpoint_run(SvEndpointOptions)` with native SDL gestures and a real
loopback TCP controlled peer, exercising production `sv_login_start_play`.

Reproduce with:

```sh
SV_BIRTH_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/02 python3 tests/sv_birth_version_checks.py
```

Every version has a complete captured `.bin` request, a `.bin.reply` containing
the actual authoritative `02 03 7a` acknowledgement sent by the peer, and a log
showing `play-acknowledged`. The peer compares every request byte, confirms no
request before legal birth completion, services an interleaved ping, and checks
no trailing or duplicated request after repeated confirmations and UI events.

The independent oracle is the explicit example matrix in
`tests/sv_birth_version_checks.py`: it does not read the SV serializer or captured
bytes, and has no production version-gate helper. It uses literal worked identity,
stat and geometry bytes, explicit per-example widths/counts, and the 199-slot
baseline-derived option literal in `tests/sv/fixtures/birth-current.hex`. The
latter's provenance is `birth-current-baseline.py` in that directory. Complete
zero visual tables are the baseline text-font server-identity sentinel, as
explained by slice 01; they do not replace the nonzero production option defaults.
The same class/race/body/mode/stat outcome is selected in each version. Traits
are absent and automatically skipped when the baseline has no trait setup field.
Audio/graphics remain the native executable's disabled outcome; resources remain
NO_GRAPHICS and CascadiaMono-Regular.ttf where the protocol carries them.

Canonical source: `source.baseline.session-play` / `session-setup`, revision
`0ef6b362cc7676805fdd0becff680dac50fe83c1`, `src/client/nclient.c`, anchors
`Net_start` and `Net_setup`. The local file matches the manifest's SHA-256
`cad90ba0b4505b41f31da5284d80448738e54cde6b6ee394aebb258ca8884853`.
Counts are baseline constants from `src/common/defines.h`, not its stale feature
compatibility-version comment (recorded separately in `docs/sv-improvements.md`).

| Baseline condition | At/below outcome | Enabled outcome |
| --- | --- | --- |
| class recommendations > 4.4.3.1.0.0 | omitted | six setup bytes per class |
| traits/audio > 4.4.5.10.0.0 | three identity shorts, no setup trait count | six identity shorts, setup trait count/rows |
| geometry > 4.4.9.1.0.1 | omitted | two network i32 values: 66, 22 |
| options > 4.5.5.0.0.0 | 96 | 128 |
| options > 4.5.8.1.0.1 | 128 | 154 |
| K/R tables > 4.6.1.2.0.0 | K=1024, R=1152 | K=1280, R=1280 |
| characters >= 4.8.1.0.0.0 | below: attr + u8 | at/above: attr + u32 |
| resources >= 4.8.1.2.0.0 | below: six identity shorts | at/above: graphics short, graphics/font strings |
| options > 4.9.1.0.0.0 | 154 | 199 |
| F table > 4.9.1.2.0.2 | 256 | 512 |

Unknown table count is always 127. The feature branch at > 4.6.1.2.0.0
has the same 256 count in both arms in the canonical baseline, so it adds no
separate feature boundary. Its distinguishable K/R change is tested.

The matrix also probes initial-login address >= 4.9.2.1.0.2 and overview mode
> 4.4.9.2.0.0 / location > 4.5.7.0.0.0 (baseline `Net_login`), so older layouts
reach birth through correct setup/login formats. Every listed condition has a
below/at/above example, plus current 4.9.4.0.0.0: 40 endpoint sessions total.
These tuples are controlled protocol probes, not claims of historical releases.
No distinguishable Net_start/Net_setup gate is excluded or called inapplicable.
Earlier versions sharing the original wire format are represented by the oldest
examples; no runtime verification of unlisted versions or real servers is claimed.
BREAK_GRAPHICS/GCU compile variants are not the accepted native SDL text-font
outcome; their compile-time alternative is not a protocol-version exemption.

This is focused implementation evidence. Full sv-build/sv-core, controller
review and fresh audit remain orchestrator-owned; upstream SV-B-023 full/native
acceptance and downstream SV-B-025 entry evidence remain pending with their owners.
