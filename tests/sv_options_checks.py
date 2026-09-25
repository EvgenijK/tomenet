#!/usr/bin/env python3
"""SV-B-003: isolated OPT layers and shared resource owner production seams."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="sv-options-") as temporary:
    base = Path(temporary)
    user = base / "U"
    library = base / "B"
    (user / "sv").mkdir(parents=True)
    (user / "xtra/font").mkdir(parents=True)
    (library / "xtra/font").mkdir(parents=True)
    (user / "global.opt").write_text("X:basic_players_symb\n")
    (user / "sv/options.prf").write_bytes(
        b"Y:basic_players_col\nY:basic_players\r\n"
        b"X:recall_flicker\nY:autoloot_depth\nX:autoloot_off\n"
        b"X:kind_diz\nY:auto_inscribe\nX:hilite_chat\nX:hibeep_chat\n"
        b"X:view_animated_lite\nX:view_lite_extra\nY:no_lite_fainting\n"
        b"Y:hilite_player\nY:colourize_prices\nX:sp_huge_bar\n"
        b"Y:auto_insc_off\nX:stack_allow_wands\nX:view_reduce_lite\n")
    (user / "sv/global.opt").write_bytes(
        b"X:censor_swearing\nX:instant_retaliator\r\n")
    (user / "sv/global-sv.opt").write_text("Y:censor_swearing\n")
    (user / "sv/Hero.opt").write_text("X:censor_swearing\nX:instant_retaliator\n")
    (user / "xtra/font/test.ttf").write_text("overlay")
    (library / "xtra/font/test.ttf").write_text("bundled")
    source = ROOT / "tests/sv/options-profile.c"
    executable = base / "check"
    flags = subprocess.check_output(["pkg-config", "--cflags", "--libs", "sdl3"], text=True).split()
    subprocess.run(["clang", "-std=c99", "-Wall", "-Wextra", "-Werror",
                    "-I", str(ROOT / "src/client/sv"), str(source),
                    str(ROOT / "src/client/sv/options.c"),
                    str(ROOT / "src/client/sv/resource.c"), *flags, "-o", str(executable)],
                   check=True)
    before = {p: p.read_bytes() for p in (user / "sv").iterdir()}
    result = subprocess.run([str(executable), str(user), str(library)],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
    assert "checks passed" in result.stdout, result.stdout
    assert before == {p: p.read_bytes() for p in (user / "sv").iterdir()}
    assert (user / "global.opt").read_text() == "X:basic_players_symb\n"
    assert (library / "xtra/font/test.ttf").read_text() == "bundled"
print("SV-B-003 isolated options/resource checks passed")
