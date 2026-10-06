# 01: Launch an isolated single-window SV client

**What to build:** A developer can build and launch the separate SV client in an explicitly identified synthetic mode, using an isolated profile and exactly one native SDL window. This is the runnable shell for the approved Stage A foundation, without requiring an account or live server.

**Blocked by:** None (can start immediately).

**Status:** implemented; Fedora41 runtime/dependency verification pending

- [x] Linux amd64 and MinGW i686 produce tomenet-sv and tomenet-sv.exe with independent SV object, generated and executable outputs; switching configurations cannot reuse incompatible outputs or overwrite a legacy client.
- [ ] Record toolchains and feature switches, respect the Fedora41-class Linux baseline, and verify relevant existing client build paths after any shared changes. Historical ignored modern binaries/objects are not implementation dependencies.
- [x] Startup uses desktop fullscreen, 100% UI scale and the approved bundled Cascadia Mono asset/profile, including licensing and explicit resource fallback/failure behavior; fixture geometry overrides do not redefine product defaults.
- [x] Keep the established SDL3 user-root identity and independent SV settings ownership while directing all scenario data to an isolated profile; checks leave personal legacy settings untouched.
- [x] The shell uses one system window with logical native surfaces and no virtual Terms. Renderer startup failure is reported explicitly and cannot count as a successful terminal fallback run.
- [x] Demonstrate Linux accelerated and explicitly forced software rendering and a MinGW smoke under Wine, including software rendering; record the actual backend and distinguish Wine from actual Windows evidence.
- [x] Publish reproducible build, launch and smoke commands with expected results. Missing tools or runtime environments remain explicitly unverified.

Implementation and verification record (2026-09-20): [SV shell build/run/smoke documentation](../../../docs/sv-shell.md). Linux software/OpenGL and Wine software/default Direct3D checks passed. Configuration switching preserves isolated outputs and the existing legacy executable. No shared legacy client source/build rule was changed. The remaining unchecked criterion has recorded toolchains and feature switches, but Fedora41 runtime/transitive dependency closure cannot be certified from the available Manjaro host. Actual Windows and human UX approval remain unverified; Wine is not Windows acceptance. Only shell ASCII UI glyphs are in the implemented font profile; no gameplay/map coverage is claimed.
