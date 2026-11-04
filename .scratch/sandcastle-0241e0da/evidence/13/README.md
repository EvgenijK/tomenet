# Ticket 13 producer input seal

The public collector command remains:

```sh
python3 -B tests/sv_birth_runtime_evidence.py /absolute/new-empty-directory
```

This command is for the controller after assembly. It discovers all six runners
(including the version runner's delegated ordinary runner), generates the full
40-row version matrix, and resolves compiler `-M` dependencies with the runners'
actual flags for both `SV_CLASS_BEFORE_RACE` orders. Capture rechecks dependencies
for each actual compilation. Python repository reads, compiler dependencies,
font resources and ELF dependencies must belong to the sealed inventories.
Undeclared repository inputs stop capture: reconcile the individually enumerated
`birth-runtime.inputPaths` in `batch-2.json` before retrying. The collector seals
that declaration itself and uses it as its source inventory. No directories are
manifest sources. The prior committed inventory remains included.

The evidence observer forwards `sv_font_open` and `sv_font_resource` unchanged;
it reports the real loaded resource, the basename passed to `sv_login_start_play`
(where there is a send), and loaded ELF libraries. No-send cases retain their
case logs and loaded-resource records without requiring request/reply bytes.
The runners still call `sv_endpoint_run(SvEndpointOptions)` with native SDL input
and a controlled TCP peer. No serializer or game behavior is implemented here.

`discovery/` and `observations/` retain actual commands, generated profile config,
version matrix and version fixtures with producer/consumer commands, producer
inputs and hashes. `manifest.json` separates repository `sources` from
`runtime_before`/`runtime_after`: Python/stdlib, compiler/target/search paths and
sanitizer bytes, linker, Git, pkg-config package flags/versions and `.pc` files,
system header closure, dynamic libraries and linker support objects/archives,
package build identities, environment and native SDL backend. Tool, file,
symlink resolution and package identity changes fail the unchanged-input check.
Linux/ELF, Debian package identities and installed tools are required; missing
identities fail closed. Coverage is intentionally broader than only the loaded
libraries so dlopen dependencies already have before fingerprints.

`oracle_provenance` retains canonical source revision/anchors and historical
Git-blob fingerprints separately from current legacy source hashes. Defaults
come from legacy `option_info` plus shipped `options.prf`; version gates,
dedicated outcomes and complete unmapped-font redefinition tables remain
independent baseline oracles. Expected bytes never come from SV output.

Focused worker check (not final acceptance):

```sh
python3 -B tests/sv_birth_inventory_checks.py --output /absolute/new-check-directory
python3 -B tests/sv_birth_runtime_evidence.py --verify-inputs /absolute/new-check-directory/capture
```

The check discovers the complete compiler closure, captures only the ordinary
three production cases, compares declared/collected paths, rejects an omitted
common header, and detects controlled file/runtime-record mutations with rollback.
`capture_scope=focused-integrity` prevents confusing this check with full evidence.
The default full command still requires ordinary 3, versions 40, dedicated 20,
submission 18, replies 17, Guide/MOTD 10 (108 total). Logs, not send artifacts,
determine case counts. Build/core/controller gates remain pending; downstream
P1-entry-evidence remains deferred to SV-B-025 with its original reason.

Local worker evidence is retained under `/home/agent/sv-b024-input-seal-13/`.
Any worker capture precedes its delivery commit and is historical; the controller
must recollect at the final integrated HEAD and immutable contract digest.
