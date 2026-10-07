#!/usr/bin/env python3
"""SV-B-022 name handoff and SV-B-023 choices through the production endpoint seam."""
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
    "tests/sv/name-editor-submit.c",
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

with tempfile.TemporaryDirectory(prefix="sv-b022-name-") as temp:
    root = Path(temp)
    binary = root / "name-editor-submit"
    profile = root / "profile"
    profile.mkdir()
    subprocess.run(
        [CC, "-std=c99", "-D_DEFAULT_SOURCE", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-Isrc/client/sv", *SOURCES, *FLAGS,
         "-o", str(binary)],
        cwd=ROOT,
        check=True,
    )
    for mode in ("navigate", "mouse", "random", "dna", "dna-missing", "quit",
                 "ctrl-quit", "class-navigate", "class-mouse", "class-random",
                 "class-dna", "class-dna-hidden", "class-dna-incompatible",
                 "class-dna-stale", "class-back", "class-ctrl-quit",
                 "race-navigate", "race-random", "race-dna",
                 "race-dna-incompatible", "race-dna-stale", "race-dna-missing",
                 "race-back", "race-ctrl-quit", "trait-navigate", "trait-random",
                 "trait-dna", "trait-dna-incompatible", "trait-dna-stale",
                 "trait-dna-missing", "trait-dummy-skip",
                 "trait-dummy-skip-back", "trait-unavailable-skip",
                 "trait-back", "trait-quit", "trait-ctrl-quit", "trait-mouse",
                 "body-navigate", "body-random", "body-dna",
                 "body-dna-stale", "body-dna-missing", "body-back",
                 "body-back-no-trait", "body-unavailable-skip", "body-mouse",
                 "body-quit", "body-ctrl-quit"):
        subprocess.run(
            [str(binary), str(profile), str(ROOT / "lib"), mode],
            cwd=ROOT,
            env=dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                     DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-b023-no-service",
                     ASAN_OPTIONS="detect_leaks=0"),
            check=True,
            timeout=15,
        )
print("SV-B-022 name handoff and SV-B-023 sex/class/race/trait/body production checks passed")
