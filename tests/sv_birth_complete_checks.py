#!/usr/bin/env python3
"""SV-B-024 current-baseline final handshake through the production endpoint."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CC = os.environ.get("CC", "clang")
FLAGS = subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "sdl3", "sdl3-ttf", "freetype2", "libsecret-1"],
    text=True,
).split()
SOURCES = [
    "tests/sv/birth-complete.c",
    "src/client/sv/endpoint-run.c",
    "src/client/sv/result.c",
    "src/client/sv/credential/vault.c",
    "src/client/sv/input/credentials.c",
    "src/client/sv/input/login-interaction.c",
    "src/client/sv/input/native-login.c",
    "src/client/sv/input/native-name.c",
    "src/client/sv/input/native-birth.c",
    "src/client/sv/protocol/contact.c",
    "src/client/sv/protocol/contact-socket.c",
    "src/client/sv/protocol/login.c",
    "src/client/sv/protocol/login-identity.c",
    "src/client/sv/session/pregame.c",
    "src/client/sv/session/login-view.c",
    "src/client/sv/session/birth.c",
    "src/client/sv/ui/endpoint-scene.c",
    "src/client/sv/ui/font.c",
    "src/client/sv/input/endpoint.c",
    "src/client/sv/input/text-field.c",
    "src/client/sv/input/native-endpoint.c",
    "src/client/sv/input/physical.c",
    "src/client/sv/input/metaserver.c",
    "src/common/md5.c",
]
with tempfile.TemporaryDirectory(prefix="sv-b024-") as temp:
    root = Path(temp)
    binary = root / "birth-complete"
    profile = root / "profile"
    (profile / "sv").mkdir(parents=True)
    (profile / "sv/tomenet.cfg").write_text("svSchemaVersion\t1\n")
    subprocess.run([CC, "-std=c99", "-D_DEFAULT_SOURCE", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=address,undefined", "-Isrc/client/sv", *SOURCES, *FLAGS,
        "-Wl,--wrap=SDL_GetKeyFromScancode", "-o", str(binary)], cwd=ROOT, check=True)
    evidence = Path(os.environ.get("SV_BIRTH_EVIDENCE", str(root / "evidence")))
    evidence.mkdir(parents=True, exist_ok=True)
    for mode in ("waiting", "partial", "ack"):
        run = subprocess.run([str(binary), str(profile), str(ROOT / "lib"), mode,
            str(ROOT / "tests/sv/fixtures/birth-current.hex"), str(evidence / (mode + ".bin"))],
            cwd=ROOT, env=dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-b024-no-service", ASAN_OPTIONS="detect_leaks=0"),
            check=True, timeout=15, capture_output=True, text=True)
        (evidence / (mode + ".log")).write_text(run.stdout + run.stderr)
        print(run.stdout, end="")
print("SV-B-024 focused production handshake checks passed")
