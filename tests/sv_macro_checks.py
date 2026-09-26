#!/usr/bin/env python3
"""SV-B-007 preference and macro production-path regression."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="sv-macros-") as temp:
    base = Path(temp)
    user, library = base / "U", base / "B"
    (user / "user").mkdir(parents=True)
    (user / "sv").mkdir()
    (library / "user").mkdir(parents=True)
    (library / "user/pref.prf").write_text("%:base.prf\n%:options.prf\nA:A\nP:a\nA:B\nP:ab\n")
    (library / "user/base.prf").write_text("X:censor_swearing\nY:instant_retaliator\n")
    (library / "user/options.prf").write_text("X:ring_bell\n")
    (user / "user/pref-sdl3.prf").write_text("#:{+hello\nS:71:59:8\n")
    (user / "sv/global-sv.opt").write_text("Y:censor_swearing\n")
    (user / "sv/Hero.opt").write_text("X:censor_swearing\n")
    (user / "user/global.prf").write_text("A:global\nP:z\n")
    (user / "user/Human.prf").write_text("A:race\nP:z\n")
    (user / "user/Maiar.prf").write_text("A:trait\nP:z\n")
    (user / "user/Warrior.prf").write_text("A:class\nP:z\nA:N\\w05S\nP:w\n")
    (user / "user/Hero.prf").write_text("A:character\nP:z\n!:N\\r\n")
    (user / "user/Hero^Wolf.prf").write_text("?:Z\nA:form\nP:z\nA:Q\\W0005R\nP:x\n")
    (user / "user/cycle.prf").write_text("%:cycle.prf\n")
    (user / "user/invalid.prf").write_text("%:missing.prf\nQ:bad\nA:C\nP:r\n")
    (user / "user/include-only.prf").write_text("%:missing.prf\n")
    (user / "user/manual.prf").write_text("A:8\nP:m\n")
    (user / "user/bad-action.prf").write_text("!:8\n%:missing.prf\n")
    with (user / "user/Warrior.prf").open("a") as stream:
        stream.write("A:8\nP:k\n")
    (user / "user/body.prf").write_text("?:Y\n!:N\n")
    sources = ["tests/sv/macros.c"]
    sources += ["src/client/sv/" + name + ".c" for name in
                ("preferences", "preferences-runtime", "options", "input/input", "input/macros", "input/command", "input/native-macro-loader", "ui/message-text",
                 "session/alerts", "app", "protocol/protocol", "result", "session/session",
                 "ui/status", "protocol/version")]
    sources += ["src/common/" + name + ".c" for name in
                ("sockbuf", "z-util", "z-form", "z-virt")]
    sources += ["src/temporary/sv/peer.c"]
    executable = base / "check"
    flags = subprocess.check_output(["pkg-config", "--cflags", "sdl3", "sdl3-ttf"],
                                    text=True).split()
    subprocess.run([os.environ.get("CC", "clang"), "-std=c99", "-D_DEFAULT_SOURCE",
                    "-DCLIENT=", "-Wall", "-Wextra", "-Werror",
                    "-Wno-deprecated-non-prototype", "-ffunction-sections",
                    "-fdata-sections", "-Wl,--gc-sections", "-Isrc/client/sv", *flags,
                    "-O1", "-g", "-fsanitize=address,undefined", *sources,
                    "-o", str(executable)], cwd=ROOT, check=True)
    subprocess.run([str(executable), str(user), str(library)], cwd=ROOT, check=True)
