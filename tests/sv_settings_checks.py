#!/usr/bin/env python3
"""SV-B-004: production settings save and reload with isolated U."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="sv-settings-") as temporary:
    base = Path(temporary)
    user = base / "U"
    (user / "sv").mkdir(parents=True)
    executable = base / "check"
    flags = subprocess.check_output(
        ["pkg-config", "--cflags", "--libs", "sdl3"], text=True
    ).split()
    subprocess.run(
        ["clang", "-std=c99", "-Wall", "-Wextra", "-Werror",
         "-I", str(ROOT / "src/client/sv"),
         str(ROOT / "tests/sv/settings-save.c"),
         str(ROOT / "src/client/sv/settings.c"),
         str(ROOT / "src/client/sv/profile.c"),
         str(ROOT / "src/client/sv/options.c"),
         *flags, "-o", str(executable)],
        check=True,
    )
    result = subprocess.run(
        [str(executable), str(user)], capture_output=True, text=True
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert "production save checks passed" in result.stdout
print("SV-B-004 settings production checks passed")

with tempfile.TemporaryDirectory(prefix="sv-settings-ui-") as temporary:
    base = Path(temporary)
    user = base / "U"
    (user / "sv").mkdir(parents=True)
    executable = base / "scene-check"
    flags = subprocess.check_output(
        ["pkg-config", "--cflags", "--libs", "sdl3", "sdl3-ttf", "freetype2"],
        text=True,
    ).split()
    subprocess.run(
        ["clang", "-std=c99", "-Wall", "-Wextra", "-Werror",
         "-I", str(ROOT / "src/client/sv"),
         str(ROOT / "tests/sv/settings-scene.c"),
         str(ROOT / "src/client/sv/ui/settings-scene.c"),
         str(ROOT / "src/client/sv/ui/font.c"),
         str(ROOT / "src/client/sv/resource.c"),
         str(ROOT / "src/client/sv/settings.c"),
         str(ROOT / "src/client/sv/profile.c"),
         str(ROOT / "src/client/sv/options.c"),
         *flags, "-o", str(executable)],
        check=True,
    )
    result = subprocess.run(
        [str(executable), str(user), str(ROOT / "lib")],
        env={"SDL_VIDEODRIVER": "dummy", "SDL_RENDER_DRIVER": "software"},
        capture_output=True, text=True,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert "native settings child checks passed" in result.stdout
print("SV-B-004 native settings child checks passed")
