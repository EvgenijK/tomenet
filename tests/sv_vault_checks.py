#!/usr/bin/env python3
"""SV-B-005 identity and provider failure through the production vault."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FLAGS = subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "sdl3", "libsecret-1"], text=True
).split()
with tempfile.TemporaryDirectory(prefix="sv-vault-") as temp:
    binary = Path(temp) / "checks"
    subprocess.run([os.environ.get("CC", "clang"), "-std=c99", "-Wall", "-Wextra",
                    "-Werror", "-fsanitize=address,undefined", "-Isrc/client/sv",
                    "tests/sv/vault.c", "src/client/sv/credential/vault.c", *FLAGS,
                    "-o", str(binary)], cwd=ROOT, check=True)
    credentials = Path(temp) / "credentials"
    subprocess.run([os.environ.get("CC", "clang"), "-std=c99", "-Wall", "-Wextra",
                    "-Werror", "-fsanitize=address,undefined", "-Isrc/client/sv",
                    "tests/sv/credentials.c", "src/client/sv/input/credentials.c",
                    "src/client/sv/input/native-endpoint.c", "src/client/sv/input/physical.c",
                    "src/client/sv/input/text-field.c", "src/client/sv/input/endpoint.c",
                    "src/client/sv/credential/vault.c", *FLAGS,
                    "-o", str(credentials)], cwd=ROOT, check=True)
    checked = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    subprocess.run([str(binary)], cwd=ROOT, env=checked, check=True)
    unavailable = dict(checked, DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-vault-no-service")
    subprocess.run([str(binary), "unavailable"], cwd=ROOT, env=unavailable, check=True)
    private_home = Path(temp) / "home"
    private_home.mkdir()
    runtime = Path(temp) / "runtime"
    runtime.mkdir(mode=0o700)
    session = r'''
import os, subprocess, sys
daemon = subprocess.run(["gnome-keyring-daemon", "--unlock", "--components=secrets"],
                        input=b"sv-synthetic-keyring-only\n", capture_output=True, check=True)
for line in daemon.stdout.decode().splitlines():
    if line.startswith("GNOME_KEYRING_CONTROL="):
        os.environ["GNOME_KEYRING_CONTROL"] = line.split("=", 1)[1]
subprocess.run([sys.argv[1], "provider"], check=True)
subprocess.run([sys.argv[2], "provider"], check=True)
'''
    isolated = dict(checked, HOME=str(private_home), XDG_DATA_HOME=str(private_home),
                    XDG_RUNTIME_DIR=str(runtime), SDL_VIDEODRIVER="dummy")
    result = subprocess.run(["dbus-run-session", "--", "python3", "-c", session,
                             str(binary), str(credentials)],
                            cwd=ROOT, env=isolated, capture_output=True)
    assert b"SV_B005_SECRET_NEVER_PRINT" not in result.stdout + result.stderr
    if result.returncode:
        raise RuntimeError(result.stderr.decode(errors="replace"))
    print(result.stdout.decode(), end="")
