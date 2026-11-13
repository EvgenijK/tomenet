# Independent review — server details and selection rim, 2026-10-29

Graphify queried before source review; repository CONTEXT and relevant ownership/lifecycle ADRs already read. UI delta compared with `shell-before.c` in this directory; metadata additions compared with the original source in `/tmp/sv-design-review-base`.

No actionable findings. XML child fields are bounded within each server's closing tag. Named/numeric entity output respects destination capacity and rejects invalid scalar values/embedded markup; control characters are normalized. Missing/self-closing metadata remains unknown while complete child bodies with no players represent known zero. The bounded players string includes whole names that fit; total count includes all parsed players. New fields are NUL-validated before endpoint publication. Selection identity remains host+port and protocol selection is unchanged.

The provider remains one reference-counted worker with atomic cancellation and mutex snapshot delivery. Refreshes are serialized with pings and scheduled every 5000 ms without overlap; slow batches may exceed the interval. Failed attempts retain prior rows; successful attempts recover the failed flag and retain existing pings by host+port before probes update them. No provider rows are changed by the UI thread.

UI removes the standalone vertical selected stripe while retaining selected card fill/rounded outline. It uses real game/version and known player count, distinguishing unknown from zero, and fits player names to card width with UTF-8 boundary truncation. Layout keeps the existing logical geometry and input production path. `git diff --check` for touched sources/tests is clean.

Reviewer added no production code or test suite. Root owns final existing endpoint checks and actual live-feed/render evidence; this document records independent source review only.
