#!/usr/bin/env python3
"""SV-B-021 new-account success through production endpoint and protocol seams."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CC = os.environ.get("CC", "clang")
FLAGS = subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "sdl3", "sdl3-ttf", "sdl3-mixer", "freetype2", "libsecret-1", "libpng", "libcurl"],
    text=True,
).split()
SOURCES = [
    "src/client/sv/files/files.c", "src/client/sv/files/history.c", "src/client/sv/files/dna.c",
    "tests/sv/account-create.c",
    "src/client/sv/login/native-run.c", "src/client/sv/settings/config.c", "src/client/sv/settings/startup.c", "src/client/sv/settings/audio.c", "src/client/sv/login/flow.c", "src/client/sv/login/chronicle.c",
    "src/client/sv/result.c",
    "src/client/sv/login/account/credentials.c",
    "src/client/sv/login/characters/login-interaction.c",
    "src/client/sv/login/characters/native-login.c",
    "src/client/sv/login/characters/native-name.c",
    "src/client/sv/login/creation/native-birth.c",
    "src/client/sv/protocol/contact.c",
    "src/client/sv/protocol/contact-socket.c",
    "src/client/sv/protocol/login.c", "src/client/sv/settings/options.c", "src/client/sv/settings/preferences.c",
    "src/client/sv/protocol/login-identity.c",
    "src/client/sv/login/pregame.c",
    "src/client/sv/login/login-view.c",
    "src/client/sv/login/creation/birth.c",
    "src/client/sv/login/ui/endpoint-scene.c",
    "src/client/sv/login/ui/login-shell.c",
    "src/client/sv/ui/font.c",
    "src/client/sv/login/servers/endpoint.c",
    "src/client/sv/input/text-field.c",
    "src/client/sv/login/ui/native-endpoint.c",
    "src/client/sv/input/key-encoding.c",
    "src/client/sv/login/servers/metaserver.c",
    "src/common/md5.c",
]

with tempfile.TemporaryDirectory(prefix="sv-b021-") as temp:
    root = Path(temp)
    binary = root / "account-create"
    profile = root / "profile"
    profile.mkdir()
    subprocess.run(
        [CC, "-std=c99", "-D_DEFAULT_SOURCE", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-Isrc/client/sv", *SOURCES, *FLAGS,
         "-o", str(binary)],
        cwd=ROOT,
        check=True,
    )
    for result in ("saved", "unavailable", "locked", "refused", "invalid", "error",
                   "pending", "retry"):
        subprocess.run(
            [str(binary), str(profile), str(ROOT / "lib"), result],
            cwd=ROOT,
            env=dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                     ASAN_OPTIONS="detect_leaks=0"),
            check=True,
            timeout=15,
        )
print("SV-B-021 new-account production flow passed")
