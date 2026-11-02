#!/usr/bin/env python3
"""Actual SDL Guide callers and peer MOTD through the public SV endpoint."""
from pathlib import Path
import os
import re
import subprocess
import struct
import zlib
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CC = os.environ.get("CC", "clang")
FLAGS = subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "sdl3", "sdl3-ttf", "freetype2", "libsecret-1", "libpng", "libcurl"],
    text=True,
).split()
SOURCES = [
    "tests/sv/guide-motd.c",
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
    "src/client/sv/input/key-encoding.c",
    "src/client/sv/input/metaserver.c",
    "src/common/md5.c",
]
def encode_capture(path):
    """Losslessly encode SDL's 24-bit BMP frame as PNG with the standard library."""
    bmp = path.read_bytes()
    offset = struct.unpack_from("<I", bmp, 10)[0]
    width, height = struct.unpack_from("<ii", bmp, 18)
    assert width > 0 and height > 0 and struct.unpack_from("<H", bmp, 28)[0] == 24
    stride = (width * 3 + 3) & ~3
    rows = []
    for y in reversed(range(height)):
        row = bmp[offset + y * stride:offset + y * stride + width * 3]
        rgb = bytearray(width * 3)
        rgb[0::3], rgb[1::3], rgb[2::3] = row[2::3], row[1::3], row[0::3]
        rows.append(b"\0" + rgb)
    def chunk(kind, data):
        return struct.pack("!I", len(data)) + kind + data + struct.pack("!I", zlib.crc32(kind + data))
    path.with_suffix(".png").write_bytes(b"\x89PNG\r\n\x1a\n" +
        chunk(b"IHDR", struct.pack("!2I5B", width, height, 8, 2, 0, 0, 0)) +
        chunk(b"IDAT", zlib.compress(b"".join(rows))) + chunk(b"IEND", b""))
    path.unlink()

with tempfile.TemporaryDirectory(prefix="sv-guide-motd-") as temp:
    root = Path(temp)
    binary = root / "guide-motd"
    profile = root / "profile"
    (profile / "sv").mkdir(parents=True)
    (profile / "sv/tomenet.cfg").write_text("svSchemaVersion\t1\n")
    evidence = Path(os.environ.get("SV_GUIDE_EVIDENCE", str(root / "evidence")))
    evidence.mkdir(parents=True, exist_ok=True)
    for order in os.environ.get("SV_GUIDE_ORDERS", "1,0").split(","):
        subprocess.run([CC, "-std=c99", "-D_DEFAULT_SOURCE", "-Wall", "-Wextra", "-Werror",
            f"-DSV_CLASS_BEFORE_RACE={order}", "-fsanitize=address,undefined", "-Isrc/client/sv",
            *SOURCES, *FLAGS, "-Wl,--wrap=SDL_GetKeyFromScancode", "-Wl,--wrap=SDL_RenderPresent",
            "-Wl,--wrap=TTF_RenderText_Blended", "-o", str(binary)], cwd=ROOT, check=True)
        for case in os.environ.get("SV_GUIDE_CASES", "waiting,partial,ack,skip,point").split(","):
            mode = "ack" if case in ("skip", "point") else case
            fixture = ROOT / ("tests/sv/fixtures/birth-guide-point.hex" if case == "point" else
                              "tests/sv/fixtures/birth-current.hex")
            label = order + "-" + case
            env = dict(os.environ, SDL_VIDEODRIVER=os.environ.get("SDL_VIDEODRIVER", "dummy"),
                SDL_RENDER_DRIVER="software", DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-b024-no-service",
                ASAN_OPTIONS="detect_leaks=0")
            env.pop("SV_MOTD_SKIP", None)
            env.pop("SV_GUIDE_POINT", None)
            if case == "skip": env["SV_MOTD_SKIP"] = "1"
            if case == "point": env["SV_GUIDE_POINT"] = "1"
            run = subprocess.run([str(binary), str(profile), str(ROOT / "lib"), mode,
                str(fixture), str(evidence / (label + ".bin"))], cwd=ROOT, env=env,
                timeout=15, capture_output=True, text=True)
            for capture in evidence.glob(label + ".bin-*.bmp"):
                encode_capture(capture)
            (evidence / (label + ".log")).write_text(run.stdout + run.stderr)
            if run.returncode:
                print(run.stdout + run.stderr)
                run.check_returncode()
            reply = (evidence / (label + ".bin.reply")).read_bytes()
            acknowledgements = re.findall(
                r"^controlled peer: authoritative acknowledgement = (.*)$",
                run.stdout, re.MULTILINE)
            if mode == "ack":
                assert reply == bytes.fromhex("02 03 7a"), (label, reply.hex())
                assert len(acknowledgements) == 1, (label, acknowledgements)
                assert bytes.fromhex(acknowledgements[0]) == reply, (
                    label, "acknowledgement transcript disagrees with captured reply",
                    acknowledgements[0], reply.hex(" "))
            else:
                assert reply == (b"" if mode == "waiting" else bytes.fromhex("02 03")), (
                    label, reply.hex())
                assert not acknowledgements, (label, "incomplete reply claims acknowledgement")
            print(label + ": " + run.stdout, end="")
print("SV Guide caller / peer MOTD / startup handoff checks passed")
