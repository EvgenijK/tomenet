# C001 actual commands — 2026-11-04

Builder: `docker build -f .scratch/sv-c001-runtime/Containerfile -t tomenet-sv-c001-fedora41:local .scratch/sv-c001-runtime`.
Runtime: `docker build -f .scratch/sv-c001-runtime/Runtime.Containerfile -t tomenet-sv-c001-runtime41:local .scratch/sv-c001-runtime`.
Both Containerfiles are preserved here; runtime has no checkout/compiler/SDL development install.

Package (exit 0):

```sh
docker run --rm --network none --user 1000:1000 \
  --mount type=bind,src=/home/svechnik/Projects/tomenet_modern_client,dst=/workspace \
  --mount type=bind,src=/tmp/sv-c001-packages,dst=/packages \
  -w /workspace tomenet-sv-c001-fedora41:local \
  bash tools/package_sv_linux.sh /packages
```

First package built successfully; rebuilt after correcting sibling checksum to use basename instead of container absolute path. Final host `sha256sum -c` is PASS.

Images:

- Fedora base: `registry.fedoraproject.org/fedora@sha256:893f7eeffce8e3e2bb2bc62786ba5603fb1e0bd6ba07091acb892266b95bafdf`.
- Builder: `sha256:514ee49568cc0e6199fe167a8195e7fac4b91d2f4e1ff46597c6c82344d4cf62`.
- Runtime: `sha256:634e58f8fb8e3fdc8b1483b5c6cf24aa6ee3954fad8c4d679fffaace8bef763f`.

Initial sandbox Docker access was denied by socket permission; tool escalation was allowed. Existing project GHCR `latest` was unavailable (not found), so a minimal Linux-only Fedora41 builder was built; no common release dependency script or Windows toolchain ran.
