#!/usr/bin/env python3
"""SV-B-006 production login parser and contact handoff checks."""
from pathlib import Path
import os
import subprocess
import tempfile

SDL_FLAGS = subprocess.check_output(["pkg-config", "--cflags", "--libs", "sdl3"],
                                    text=True).split()

root = Path(__file__).resolve().parents[1]
cc = os.environ.get("CC", "clang")
with tempfile.TemporaryDirectory(prefix="sv-login-") as temp:
    for name, sources in (
        ("login", ["tests/sv/login.c", "src/client/sv/protocol/login.c"]),
        ("handoff", ["tests/sv/contact-login-handoff.c",
                     "src/client/sv/protocol/contact.c", "src/client/sv/protocol/login.c"]),
    ):
        binary = Path(temp) / name
        subprocess.run([cc, "-std=c99", "-Wall", "-Wextra", "-Werror",
                        "-fsanitize=undefined", "-Isrc/client/sv", *sources,
                        "-o", str(binary)], cwd=root, check=True)
        subprocess.run([str(binary)], cwd=root, check=True)
    first = Path(temp) / "profile-a"
    second = Path(temp) / "profile-b"
    first.mkdir()
    second.mkdir()
    identity = Path(temp) / "identity"
    subprocess.run([cc, "-std=c99", "-Wall", "-Wextra", "-Werror",
                    "-Isrc/client/sv", "tests/sv/login-identity.c",
                    "src/client/sv/protocol/login-identity.c", "src/common/md5.c",
                    *SDL_FLAGS, "-o", str(identity)], cwd=root, check=True)
    subprocess.run([str(identity), str(first), str(second)], cwd=root, check=True)
    interaction = Path(temp) / "interaction"
    subprocess.run([cc, "-std=c99", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=undefined", "-Isrc/client/sv",
                    "tests/sv/login-interaction.c",
                    "src/client/sv/protocol/login.c",
                    "src/client/sv/input/login-interaction.c",
                    "src/client/sv/session/login-view.c",
                    "-o", str(interaction)], cwd=root, check=True)
    subprocess.run([str(interaction)], cwd=root, check=True)
    native = Path(temp) / "native-login"
    subprocess.run([cc, "-std=c99", "-Wall", "-Wextra", "-Werror",
                    "-Isrc/client/sv", "tests/sv/native-login.c",
                    "src/client/sv/input/native-login.c", *SDL_FLAGS,
                    "-o", str(native)], cwd=root, check=True)
    subprocess.run([str(native)], cwd=root, check=True)
print("SV-B-006 login parser and contact handoff passed")
