#!/usr/bin/env python3
"""SV-B-001 checks through production editor, SDL adapter and native scene."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CC = os.environ.get("CC", "clang")
SDL_FLAGS = subprocess.check_output(["pkg-config", "--cflags", "--libs", "sdl3"], text=True).split()
SCENE_FLAGS = subprocess.check_output(["pkg-config", "--cflags", "--libs",
                                      "sdl3", "sdl3-ttf", "freetype2"], text=True).split()
COMMON = ["src/client/sv/input/endpoint.c", "src/client/sv/input/text-field.c"]


def check(binary, sources, flags=()):
    subprocess.run([CC, "-std=c99", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined",
                    "-Isrc/client/sv", *flags, *sources, "-o", str(binary)], cwd=ROOT, check=True)
    subprocess.run([str(binary)], cwd=ROOT, check=True)


with tempfile.TemporaryDirectory(prefix="sv-endpoint-") as temp:
    directory = Path(temp)
    check(directory / "endpoint", ["tests/sv/endpoint.c", *COMMON,
                                   "src/client/sv/input/confirm.c"])
    for name, flags in (("native", ()), ("sticky", ("-DSDL3_STICKY_KEYS",))):
        check(directory / name, ["tests/sv/native-endpoint.c", *COMMON,
                                 "src/client/sv/input/native-endpoint.c",
                                 "src/client/sv/input/physical.c"], (*flags, *SDL_FLAGS))
    check(directory / "physical", ["tests/sv/physical.c",
                                   "src/client/sv/input/physical.c"], SDL_FLAGS)
    check(directory / "metaserver", ["tests/sv/metaserver.c", *COMMON,
                                     "src/client/sv/input/metaserver.c"], SDL_FLAGS)

    subprocess.run(["make", "-f", "makefile.sv", "tomenet-sv", "-j4"],
                   cwd=ROOT / "src", check=True, capture_output=True)
    binary = ROOT / "src/tomenet-sv"
    profile = directory / "profile"
    profile.mkdir()
    server_list = directory / "servers.txt"
    server_list.write_text("host.example 18349 Test server\nsecond.example 18350 Second server\n")
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software")
    command = [str(binary), "--endpoint", "--profile-root", str(profile),
               "--library", str(ROOT / "lib"), "--fixture-window", "1024x768", "--frames", "2"]
    selected = subprocess.run([*command, "--server", "host.example:18349"],
                              cwd=ROOT, env=env, text=True, capture_output=True, check=True)
    assert "SV endpoint selected host=host.example port=18349" in selected.stdout
    override = subprocess.run([*command, "--server", "host.example", "--port", "19000"],
                              cwd=ROOT, env=env, text=True, capture_output=True, check=True)
    assert "SV endpoint selected host=host.example port=19000" in override.stdout
    listing = subprocess.run([*command, "--server-list", str(server_list)],
                             cwd=ROOT, env=env, text=True, capture_output=True, check=True)
    assert "no contact attempted" in listing.stdout
    bad = subprocess.run([*command, "--server", "host.example:0"],
                         cwd=ROOT, env=env, text=True, capture_output=True)
    assert bad.returncode == 2 and "Invalid server address" in bad.stderr
    scene_sources = ["tests/sv/scene.c", "src/client/sv/endpoint-run.c",
                     "src/client/sv/protocol/contact.c", "src/client/sv/protocol/contact-socket.c",
                     "src/client/sv/ui/endpoint-scene.c", "src/client/sv/ui/font.c",
                     "src/client/sv/input/native-endpoint.c", "src/client/sv/input/physical.c",
                     "src/client/sv/input/metaserver.c", *COMMON]
    scene_sources += ["src/client/sv/" + name + ".c" for name in
                      ("app", "input/input", "session/alerts", "session/session",
                       "protocol/protocol", "protocol/version", "result")]
    scene_sources += ["src/temporary/sv/peer.c"]
    scene_sources += ["src/common/" + name + ".c" for name in
                      ("sockbuf", "z-util", "z-form", "z-virt")]
    scene_binary = directory / "scene"
    subprocess.run([CC, "-std=c99", "-D_DEFAULT_SOURCE", "-DCLIENT=",
                    "-Wall", "-Wextra", "-Werror", "-Wno-deprecated-non-prototype",
                    "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections",
                    "-fsanitize=undefined", "-Isrc/client/sv", *scene_sources,
                    *SCENE_FLAGS, "-o", str(scene_binary)], cwd=ROOT, check=True)
    subprocess.run([str(scene_binary), str(profile), str(ROOT / "lib"), str(server_list)],
                   cwd=ROOT, env=env, check=True)
print("SV-B-001 endpoint, native input, physical keys, and SDL scene passed")
