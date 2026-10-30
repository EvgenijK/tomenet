#!/usr/bin/env python3
"""SV ordinary/dedicated creation availability through the production endpoint seam."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CC = os.environ.get("CC", "clang")
FLAGS = subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "sdl3", "sdl3-ttf", "freetype2", "libsecret-1", "libpng", "libcurl"],
    text=True,
).split()
SOURCES = [
    "tests/sv/ordinary-create-cancel.c",
    "src/client/sv/endpoint-run.c", "src/client/sv/input/chronicle.c",
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
    "src/client/sv/ui/login-shell.c",
    "src/client/sv/ui/font.c",
    "src/client/sv/input/endpoint.c",
    "src/client/sv/input/text-field.c",
    "src/client/sv/input/native-endpoint.c",
    "src/client/sv/input/physical.c",
    "src/client/sv/input/metaserver.c",
    "src/common/md5.c",
]

with tempfile.TemporaryDirectory(prefix="sv-b022-ordinary-") as temp:
    root = Path(temp)
    binary = root / "ordinary-create-cancel"
    profile = root / "profile"
    profile.mkdir()
    subprocess.run(
        [CC, "-std=c99", "-D_DEFAULT_SOURCE", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-Isrc/client/sv", *SOURCES, *FLAGS,
         "-o", str(binary)],
        cwd=ROOT,
        check=True,
    )
    config = profile / "sv" / "tomenet.cfg"
    for mode in ("ordinary-available", "ordinary-first-run", "ordinary-full",
                 "exclusive-available", "exclusive-unavailable",
                 "exclusive-first-run", "capacity-neither-available",
                 "capacity-neither-full", "capacity-iddc-available",
                 "capacity-iddc-full", "capacity-pvp-available",
                 "capacity-pvp-full", "capacity-both-available",
                 "capacity-both-full", "dedicated-iddc-available",
                 "dedicated-iddc-full", "dedicated-pvp-available",
                 "dedicated-pvp-full"):
        if mode.endswith("first-run"):
            config.unlink(missing_ok=True)
        else:
            config.parent.mkdir(exist_ok=True)
            config.write_text("svSchemaVersion\t1\n", encoding="ascii")
        subprocess.run(
            [str(binary), str(profile), str(ROOT / "lib"), mode],
            cwd=ROOT,
            env=dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                     DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-b022-no-service",
                     ASAN_OPTIONS="detect_leaks=0"),
            check=True,
            timeout=15,
        )
print("SV creation availability production checks passed")
