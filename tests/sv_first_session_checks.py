#!/usr/bin/env python3
"""SV-B-020 native M1 flow through production UI/model/input/transport/protocol."""
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
    "tests/sv/first-session.c",
    "src/client/sv/endpoint-run.c",
    "src/client/sv/result.c",
    "src/client/sv/credential/vault.c",
    "src/client/sv/input/credentials.c",
    "src/client/sv/input/login-interaction.c",
    "src/client/sv/input/native-login.c",
    "src/client/sv/protocol/contact.c",
    "src/client/sv/protocol/contact-socket.c",
    "src/client/sv/protocol/login.c",
    "src/client/sv/protocol/login-identity.c",
    "src/client/sv/session/pregame.c",
    "src/client/sv/session/login-view.c",
    "src/client/sv/ui/endpoint-scene.c",
    "src/client/sv/ui/font.c",
    "src/client/sv/input/endpoint.c",
    "src/client/sv/input/text-field.c",
    "src/client/sv/input/native-endpoint.c",
    "src/client/sv/input/physical.c",
    "src/client/sv/input/metaserver.c",
    "src/common/md5.c",
]

with tempfile.TemporaryDirectory(prefix="sv-b020-") as temp:
    root = Path(temp)
    binary = root / "first-session"
    profile = root / "profile"
    profile.mkdir()
    subprocess.run(
        [CC, "-std=c99", "-D_DEFAULT_SOURCE", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-Isrc/client/sv", *SOURCES, *FLAGS,
         "-o", str(binary)],
        cwd=ROOT,
        check=True,
    )
    environment = dict(
        os.environ,
        SDL_VIDEODRIVER="dummy",
        SDL_RENDER_DRIVER="software",
        DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-b020-no-service",
        ASAN_OPTIONS="detect_leaks=0",
    )
    for mode in ("success", "disconnect", "quit", "retry"):
        subprocess.run(
            [str(binary), str(profile), str(ROOT / "lib"), mode],
            cwd=ROOT,
            env=environment,
            check=True,
            timeout=15,
        )
print("SV-B-020 first-session managed-peer checks passed")
