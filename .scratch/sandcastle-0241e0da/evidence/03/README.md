# Dedicated completion endpoint evidence

Contract: `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

These captures exercise `sv_endpoint_run(SvEndpointOptions)` with native SDL
input and a controlled TCP peer, using the production birth interaction and
final serializer inherited from SV-B-023 and slice 01. No serializer is mocked.
This evidence supports slice 03, not controller review or full acceptance.

Reproduce the retained IDDC/PvP evidence:

```sh
SV_DEDICATED_CASES=iddc,pvp-maia SV_DEDICATED_EVIDENCE=.scratch/sandcastle-0241e0da/evidence/03 python3 tests/sv_birth_dedicated_checks.py
```

Run all five dedicated scenarios with `python3 tests/sv_birth_dedicated_checks.py`.
Both commands compile each supported class/race order under ASan/UBSan and strict
warnings. `0-` means race before class; `1-` means class before race. Each `.bin`
is the complete 16,266-byte request, compared byte for byte to the independent
baseline literal. `.reply` records the actual peer acknowledgement bytes (empty
in waiting cases); `.log` records the resulting public endpoint startup phase
with trailing line whitespace normalized.

The IDDC identity is male/Dwarf/Mage/Phoenix/fruit bat, mode `0x344`, stat order
`0,1,2,3,4,5`. PvP Maia is male/Maia/Warrior/Corrupted/fruit bat, mode `0x390`,
stat order `5,4,3,2,1,0`. PvP inputs first reach post-mode trait, return to stats,
recommit the reversed order, return to post-mode trait, then confirm Corrupted.
Peer checks prohibit final send at each intermediate point, including mere
trait highlight. Duplicate confirms, resize/focus and a ping do not duplicate
final play. Without full acknowledgement the phase is `play-pending`; after the
three-byte success reply it is `play-acknowledged`, still at the startup owner.

Additional focused cases compare Soloist (`0x345`), Hellish (`0x346`) and skipped
unavailable trait (draft -1, wire 0) against complete literal requests in both
orders. Current-baseline options/resources/visual tables are identical to the
ordinary baseline outcome: dedicated creation does not modify those defaults.
See `tests/sv/fixtures/birth-dedicated-baseline.py` and the independent ordinary
oracle `birth-current-baseline.py` for provenance. Fixtures are expectations,
not a second client behavior implementation.
