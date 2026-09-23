#!/usr/bin/env python3
"""Exercise the actual Stage A shell; requires a native display (or Wine display)."""
import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--binary", type=Path, default=REPO / "src/tomenet-sv")
parser.add_argument("--wine", action="store_true")
parser.add_argument("--backend", default="software")
args = parser.parse_args()


def native(path):
    absolute = str(Path(path).resolve())
    return "Z:" + absolute.replace("/", "\\") if args.wine else absolute


def snapshot(root):
    return {str(p.relative_to(root)): p.read_bytes() for p in root.rglob("*") if p.is_file()}


with tempfile.TemporaryDirectory(prefix="tomenet-sv-smoke-") as temporary:
    work = Path(temporary)
    decoy = work / "personal"
    decoy.mkdir()
    (decoy / "tomenet.cfg").write_text("personal legacy settings: must remain untouched\n")
    original = snapshot(decoy)
    env = dict(os.environ, TOMENET_SDL3_USER_PATH=native(decoy), SDL_RENDER_DRIVER=args.backend)
    prefix = ["wine", str(args.binary.resolve())] if args.wine else [str(args.binary.resolve())]

    def run(options, success=True, contains=(), extra_env=None):
        completed = subprocess.run(prefix + options, env=env | (extra_env or {}),
                                   capture_output=True, text=True, timeout=45)
        output = completed.stdout + completed.stderr
        print(output, end="")
        assert (completed.returncode == 0) == success, (completed.returncode, output)
        for value in contains:
            assert value in output, (value, output)
        assert snapshot(decoy) == original, "Personal legacy settings changed"
        return output

    font = REPO / "lib/xtra/font/CascadiaMono-Regular.ttf"
    assert hashlib.sha256(font.read_bytes()).hexdigest() == "06520d032ec274fa5040b22c6f4a1d829081b24ba40b2da56dae89bf10c7b481"
    assert "SIL OPEN FONT LICENSE" in (font.parent / "CascadiaMono-LICENSE.txt").read_text()
    run([], success=False, contains=["requires explicit --synthetic"])
    run(["--synthetic"], success=False, contains=["Refusing existing unmarked profile"])
    root = work / "scenario"
    base = ["--synthetic", "--profile-root", native(root), "--library", native(REPO / "lib"), "--frames", "3"]
    run(base, contains=["windows=1", "fullscreen=true", "ui_scale=100", "submitted_frames=3", "fallback_routes=0", f"renderer={args.backend}",
                        "SV runtime architecture=" + ("i686" if args.wine else "amd64"),
                        " sdl=", " sdl_ttf=", " freetype="])
    # Existing settings are deliberately not loaded/saved by the synthetic shell.
    (root / "sv/tomenet.cfg").write_text("svWindowMode window\n")
    before = snapshot(root)
    run(base + ["--fixture-window", "1024x768"], contains=["fullscreen=false", "submitted_frames=3"])
    assert snapshot(root) == before, "Synthetic shell wrote settings"
    # Failed renderer startup must be nonzero with no terminal substitution.
    run(base, success=False, contains=["no terminal fallback"], extra_env={"SDL_RENDER_DRIVER": "sv-nonexistent-renderer"})
    # User overlay first; a damaged user resource recovers to the bundled TTF.
    overlay = root / "xtra/font"
    overlay.mkdir(parents=True)
    (overlay / font.name).write_bytes(b"broken font")
    run(base, contains=["trying declared fallback", "submitted_frames=3"])
    # A missing bundled TTF must use the declared direct FreeType PCF path.
    fallback_lib = work / "fallback-lib"
    (fallback_lib / "xtra/font").mkdir(parents=True)
    shutil.copyfile(font.parent / "16x24x.pcf", fallback_lib / "xtra/font/16x24x.pcf")
    fallback = base + ["--library", native(fallback_lib), "--fixture-window", "1024x768"]
    run(fallback, contains=["effective font fallback:", "16x24x.pcf", "submitted_frames=3"])
    run(base + ["--library", native(work / "missing-lib")], success=False,
        contains=["fatal resource failure", "no terminal fallback"])
    print(f"PASS: isolated shell, window/defaults, {args.backend}, resources and failures; environment={'Wine (not Windows)' if args.wine else 'Linux'}")
