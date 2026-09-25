# SV-B-004 settings implementation evidence

Status: partial production implementation; none of the ten owned capability
IDs has full acceptance.

## 2026-09-25 Linux implementation

The production endpoint executable offers a native settings child from the
server-selection surface (F10). Its opening snapshot drives immediate preview
of UI scale and window mode. Text-font changes use an exact resource
preparation path; a failed preparation retains the previous font and requested
value. Dirty close offers Save, Cancel changes and Return. Return leaves the
draft active; Cancel restores the opening settings and working font. Normal
endpoint exit disposes an unsaved draft without publishing CFG/OPT.

`src/client/sv/settings.c` is the writer used by this child. It reads the
concrete `U/sv/tomenet.cfg` once on opening and again before Save. It merges
changed recognized CFG records into the latest file, preserves unrelated
unknown lines, reports same-key conflicts and keeps the attempted value dirty.
The OPT production API selects independent character, global, class and named
targets in S, using baseline X:/Y: option meanings. Explicit global/class/named
Save writes a full snapshot; character Save writes changed options. A class
Save does not alter the startup layer list. Named load is explicit, and Save
does not touch a macro PRF.

Publication writes and syncs a unique sibling temporary before SDL's replace
operation. A sibling backup of the prior nonsecret bytes is removed after the
operation. A CFG containing `pass` does not create a backup; explicit Save
removes the `pass` record from the destination. Read, write and publication
failures retain the old destination and active dirty draft. There is no
atomicity claim against uncoordinated legacy writers after the final reread.

## Checks

- Linux amd64: `make -C src -f makefile.sv tomenet-sv -j4` passed with the
  SV `-Wall -Wextra -Werror` compile.
- `python3 tests/sv_settings_checks.py` passed. Its C harnesses call the
  production settings and native settings-child APIs with isolated U and B
  roots and the SDL dummy/software renderer. Exact assertions cover
  preview/Cancel/Return, failed font preparation, explicit Save and reload,
  unsaved exit, unknown record preservation, same-key conflict on repeated
  Save, denied-directory failure with unchanged destination, global/class/
  named OPT snapshots, no class autoload, named load and unchanged shared
  macro PRF.
- `python3 tests/sv_profile_checks.py`,
  `python3 tests/sv_options_checks.py`,
  `python3 tests/sv_endpoint_checks.py` and
  `python3 tests/sv_contact_checks.py` passed on Linux after the settings
  scene was added to the endpoint check's production link list.
- Two-axis review of `891506467...a104fef8d` found and corrected exact-capacity
  append handling, opening-snapshot Cancel after an intermediate Save, fresh
  CFG Save writing unedited runtime values, missing named OPT load success,
  and a noncanonical SV include path. The corrected focused settings,
  profile, options and endpoint checks passed with the Linux SV rebuild.
- The full Stage A runner was invoked once at
  `/tmp/sv-b004-stage-a-20260925a/report.json`. Its overall result was
  **blocked**. Linux SV and both legacy builds passed. MinGW failed because
  SDL3/SDL3_ttf/FreeType development packages were unavailable. Registry
  checks could not import `jsonschema`; sanitizer cases reported the
  sandbox ptrace/LeakSanitizer restriction; native display cases reported
  no available video device. These failures are retained as failures, not
  counted as B-004 acceptance.

## Pending obligations

The child is currently reachable before contact. There is no in-game settings
surface or character/class/name selection UI, so ordinary character-default
Save, explicit scope actions, gameplay updates during preview/Cancel and
already-delivered effects cannot be accepted. The B-003 map, graphics, pack and
audio effective consumers are not implemented; this work does not claim
resource-wide preview or device behavior. The named API currently accepts
safe single filenames in S; absolute/custom paths and their reviewed owner
semantics remain pending. Fault injection for every stat/read/write/replace
phase, cross-process publication races, filename-byte boundaries, all
protocol-dependent callers, history/DNA/bookmark lifecycle, accelerated
renderers, MinGW/Wine and actual Windows 10/11 observations remain pending.

Source authority: `docs/tasks/stage-b/SV-B-004-save.md`,
`docs/capabilities/settings-policy.md`, `CONTEXT.md`,
`src/client/client.c`, `src/client/c-cmd.c`, `src/client/c-util.c`.
The test fixtures create temporary U roots and do not contain user secrets.
