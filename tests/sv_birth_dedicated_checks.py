#!/usr/bin/env python3
"""SV-B-024 dedicated completion through the production endpoint and native input."""
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
    "tests/sv/birth-dedicated.c",
    "src/client/sv/login/native-run.c", "src/client/sv/settings/config.c", "src/client/sv/settings/startup.c", "src/client/sv/settings/audio.c", "src/client/sv/login/flow.c", "src/client/sv/login/chronicle.c",
    "src/client/sv/result.c",
    "src/client/sv/credential/vault.c",
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
with tempfile.TemporaryDirectory(prefix="sv-b024-dedicated-") as temp:
    root = Path(temp)
    binary = root / "birth-dedicated"
    profile = root / "profile"
    (profile / "sv").mkdir(parents=True)
    (profile / "sv/tomenet.cfg").write_text("svSchemaVersion\t1\n")
    for order in (1, 0):
        subprocess.run([CC, "-std=c99", "-D_DEFAULT_SOURCE", "-Wall", "-Wextra", "-Werror",
            f"-DSV_CLASS_BEFORE_RACE={order}",
            "-fsanitize=address,undefined", "-Isrc/client/sv", *SOURCES, *FLAGS,
            "-Wl,--wrap=SDL_GetKeyFromScancode", "-o", str(binary)], cwd=ROOT, check=True)
        evidence = Path(os.environ.get("SV_DEDICATED_EVIDENCE", str(root / "evidence")))
        evidence.mkdir(parents=True, exist_ok=True)
        scenarios = os.environ.get("SV_DEDICATED_CASES", "iddc,iddc-soloist,iddc-hellish,iddc-skipped,pvp-maia").split(",")
        for scenario in scenarios:
            for phase in ("waiting", "ack"):
                name = f"{order}-{scenario}-{phase}"
                run = subprocess.run([str(binary), str(profile), str(ROOT / "lib"), phase,
                    str(ROOT / f"tests/sv/fixtures/birth-{scenario}.hex"),
                    str(evidence / (name + ".bin")), scenario], cwd=ROOT,
                    env=dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                        DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-b024-no-service", ASAN_OPTIONS="detect_leaks=0"),
                    timeout=15, capture_output=True, text=True)
                (evidence / (name + ".log")).write_text("\n".join(
                    line.rstrip() for line in (run.stdout + run.stderr).splitlines()) + "\n")
                print(run.stdout, end="")
                if run.returncode:
                    print(run.stderr, end="")
                    run.check_returncode()
print("SV-B-024 dedicated production handshake checks passed (both compile orders)")
