#!/usr/bin/env python3
"""Loopback metaserver and process-ping fixture through production provider."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CC = os.environ.get("CC", "clang")
SDL = subprocess.check_output(["pkg-config", "--cflags", "--libs", "sdl3"], text=True).split()
with tempfile.TemporaryDirectory(prefix="sv-meta-live-") as temp:
    directory = Path(temp)
    ping = directory / "ping"
    subprocess.run([CC, "-std=c99", "-Wall", "-Wextra", "-Werror",
                    "tests/sv/ping-fixture.c", "-o", str(ping)], cwd=ROOT, check=True)
    binary = directory / "metaserver-live"
    subprocess.run([CC, "-std=c99", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined",
                    "-Isrc/client/sv", "tests/sv/metaserver-live.c",
                    "src/client/sv/input/metaserver.c", "src/client/sv/input/endpoint.c",
                    "src/client/sv/input/text-field.c", *SDL, "-o", str(binary)],
                   cwd=ROOT, check=True)
    environment = dict(os.environ, PATH=str(directory) + os.pathsep + os.environ["PATH"])
    subprocess.run([str(binary)], cwd=ROOT, env=environment, check=True)
