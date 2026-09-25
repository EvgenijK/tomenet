#!/usr/bin/env python3
"""SV-B-003 startup checks through the production SV executable."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
subprocess.run(["make", "-C", str(ROOT / "src"), "-f", "makefile.sv", "tomenet-sv", "-j4"],
               check=True, capture_output=True)

with tempfile.TemporaryDirectory(prefix="sv-profile-") as temporary:
    user = Path(temporary) / "user"
    user.mkdir()
    (user / "tomenet.cfg").write_text("svWindowMode\twindow\nsvUiScalePercent\t150\n")
    listing = Path(temporary) / "servers.txt"
    listing.write_text("host.example 18348 Example\n")
    env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software")
    command = [str(ROOT / "src/tomenet-sv"), "--endpoint", "--profile-root", str(user),
               "--library", str(ROOT / "lib"), "--server-list", str(listing), "--frames", "1"]

    def run():
        result = subprocess.run(command, env=env, text=True, capture_output=True, timeout=30)
        assert result.returncode == 0, result.stdout + result.stderr
        return result.stdout + result.stderr

    output = run()
    assert "SV profile window=fullscreen ui_scale=100" in output, output
    assert not (user / "sv").exists()

    profile = user / "sv"
    profile.mkdir()
    cfg = profile / "tomenet.cfg"
    cfg.write_bytes(b"svSchemaVersion\t1\r\nsvWindowMode\twindow\r\n"
                    b"svUiScalePercent\t150\nsvUiScalePercent\t105\n"
                    b"audioSampleRate\t48000\n"
                    b"soundpackSubset\t2\n")
    before = cfg.read_bytes()
    output = run()
    assert "SV profile window=window ui_scale=105" in output, output
    assert "rate=48000" in output and "sound_subset=2" in output, output
    assert f"map-font requested=16x24x.pcf source={ROOT}/lib/xtra/font/16x24x.pcf availability=found" in output, output
    assert f"sound-pack requested=sound source={ROOT}/lib/xtra/sound availability=found" in output, output
    assert cfg.read_bytes() == before
    override = subprocess.run([*command, "--window-mode", "fullscreen", "--ui-scale", "115"],
                              env=env, text=True, capture_output=True, timeout=30)
    assert override.returncode == 0, override.stdout + override.stderr
    assert "SV profile window=fullscreen ui_scale=115" in override.stdout, override.stdout
    assert cfg.read_bytes() == before

    cfg.write_text("svUiScalePercent\t150\nsvUiScalePercent\t103\n"
                   "svWindowModeExtra\twindow\n"
                   "soundpackSubset\t3\nsound\t0\n"
                   "svTextFont\tmissing.ttf\n")
    output = run()
    assert "SV profile window=fullscreen ui_scale=100" in output, output
    assert "sound_subset=3" in output, output
    assert "invalid value for svUiScalePercent" in output, output
    assert "requested=missing.ttf effective=" in output, output
    assert "CascadiaMono-Regular.ttf" in output, output

    (user / "xtra/font").mkdir(parents=True)
    (user / "xtra/graphics").mkdir(parents=True)
    (user / "xtra/customsound").mkdir(parents=True)
    (user / "xtra/font/custom.pcf").write_bytes(b"fixture")
    (user / "xtra/graphics/custom.bmp").write_bytes(b"fixture")
    cfg.write_text("svMapFont\tcustom.pcf\ngraphic_tiles\tcustom\n"
                   "soundpackFolder\tcustomsound\nmusicpackFolder\tmissingpack\n")
    output = run()
    assert f"map-font requested=custom.pcf source={user}/xtra/font/custom.pcf availability=found" in output, output
    assert f"graphics requested=custom source={user}/xtra/graphics/custom.bmp availability=found" in output, output
    assert f"sound-pack requested=customsound source={user}/xtra/customsound availability=found" in output, output
    assert "music-pack requested=missingpack source=missing availability=missing" in output, output

    overlay = user / "xtra/font"
    shutil.copyfile(ROOT / "lib/xtra/font/CascadiaMono-Regular.ttf",
                    overlay / "CascadiaMono-Regular.ttf")
    output = run()
    assert f"effective={user}/xtra/font/CascadiaMono-Regular.ttf" in output, output

    cfg.write_text("svTextFont\t16x24x.pcf\n")
    output = run()
    assert f"effective={ROOT}/lib/xtra/font/16x24x.pcf" in output, output

    cfg.write_text("  svSchemaVersion\t2\nsvWindowMode\twindow\n")
    output = run()
    assert "SV profile window=fullscreen ui_scale=100" in output, output
    assert "incompatible" in output, output

    cfg.write_bytes(b"svWindowMode\twindow\npass\tsecret\n\x00")
    output = run()
    assert "SV profile window=fullscreen ui_scale=100" in output, output
    assert "secret" not in output
    assert "corrupt CFG" in output, output

    cfg.write_text("svSchemaVersion\t2\nsvWindowMode\twindow\n")
    output = run()
    assert "SV profile window=fullscreen ui_scale=100" in output, output
    assert "incompatible" in output, output

print("SV-B-003 production profile startup checks passed")
