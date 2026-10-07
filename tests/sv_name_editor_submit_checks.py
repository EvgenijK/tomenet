#!/usr/bin/env python3
"""SV-B-022 name handoff and SV-B-023 choices through the production endpoint seam."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CC = os.environ.get("CC", "clang")
FLAGS = subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "sdl3", "sdl3-ttf", "freetype2", "libsecret-1"],
    text=True,
).split()
SOURCES = [
    "tests/sv/name-editor-submit.c",
    "src/client/sv/endpoint-run.c",
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
    "src/client/sv/ui/font.c",
    "src/client/sv/input/endpoint.c",
    "src/client/sv/input/text-field.c",
    "src/client/sv/input/native-endpoint.c",
    "src/client/sv/input/physical.c",
    "src/client/sv/input/metaserver.c",
    "src/common/md5.c",
]
ONLY_CASE = os.environ.get("SV_BIRTH_CASE")
QUIT_CASES = tuple(
    prefix + owner
    for prefix in ("quit-", "ctrl-quit-")
    for owner in ("sex", "race", "trait", "class", "body", "stats", "mode")
)

with tempfile.TemporaryDirectory(prefix="sv-b022-name-") as temp:
    root = Path(temp)
    binary = root / "name-editor-submit"
    alternate_binary = root / "name-editor-submit-alternate"
    profile = root / "profile"
    profile.mkdir()
    subprocess.run(
        [CC, "-std=c99", "-D_DEFAULT_SOURCE", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-Isrc/client/sv", *SOURCES, *FLAGS,
         "-o", str(binary)],
        cwd=ROOT,
        check=True,
    )
    subprocess.run(
        [CC, "-std=c99", "-D_DEFAULT_SOURCE", "-DSV_CLASS_BEFORE_RACE=0",
         "-Wall", "-Wextra", "-Werror", "-fsanitize=address,undefined",
         "-Isrc/client/sv", *SOURCES, *FLAGS, "-o", str(alternate_binary)],
        cwd=ROOT,
        check=True,
    )
    for mode in ("guide-all-owners",
                 "birth-context-escape",
                 "mode-ordinary-ready", "mode-dna", "mode-dna-missing",
                 "session-interleave", "session-stale-input", "session-disconnect",
                 "session-retry-generation",
                 "mode-dedicated", "mode-dedicated-unavailable",
                 "mode-back", "mode-random", "mode-ctrl-quit",
                 "stats-order-reenter-completed", "stats-order-reenter-partial",
                 "stats-order-reenter-reject",
                 "stats-order-reenter-pvp-maia",
                 "mode-pvp-maia", "mode-pvp-maia-back", "mode-rpg",
                 "mode-dedicated-iddc",
                 "mode-dedicated-dna",
                 "navigate", "mouse", "random", "dna", "dna-missing", "quit",
                 "ctrl-quit", "class-random-empty", "hidden-derived",
                 "hidden-derived-back",
                 "hidden-derived-base-mismatch",
                 "hidden-derived-reconfirm", "hidden-derived-mouse",
                 "hidden-derived-dna", "hidden-derived-dna-missing",
                 "hidden-derived-dna-incompatible", "hidden-derived-dna-stale",
                 "class-navigate", "class-mouse", "class-random",
                 "class-dna", "class-dna-hidden", "class-dna-incompatible",
                 "class-dna-stale", "class-back", "class-ctrl-quit",
                 "race-navigate", "race-random", "race-dna",
                 "race-dna-incompatible", "race-dna-stale", "race-dna-missing",
                 "race-back", "race-ctrl-quit", "trait-navigate", "trait-random",
                 "trait-dna", "trait-dna-incompatible", "trait-dna-stale",
                 "trait-dna-missing", "trait-dummy-skip",
                 "trait-dummy-skip-back", "trait-unavailable-skip",
                 "trait-back", "trait-quit", "trait-ctrl-quit", "trait-mouse",
                 "body-navigate", "body-random", "body-dna",
                 "body-dna-stale", "body-dna-missing", "body-back",
                 "body-back-no-trait", "body-unavailable-skip", "body-mouse",
                 "body-quit", "body-ctrl-quit", "stats-order-navigate",
                 "stats-order-confirm", "stats-order-random", "stats-order-mouse",
                 "stats-back", "stats-back-no-trait-body", "stats-quit",
                 "stats-ctrl-quit", "stats-point-navigate",
                 "stats-point-confirm-warning", "stats-point-escape-warning",
                 "stats-point-confirm", "stats-point-escape-confirm",
                 "stats-point-mouse", "stats-point-dna",
                 "stats-point-dna-over-budget", "stats-point-dna-illegal",
                 "stats-point-dna-multiple-max", "stats-point-dna-missing",
                 "stats-point-lower-bound", "stats-point-escape-no-warning",
                 *QUIT_CASES):
        if ONLY_CASE and mode != ONLY_CASE:
            continue
        subprocess.run(
            [str(binary), str(profile), str(ROOT / "lib"), mode],
            cwd=ROOT,
            env=dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                     DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-b023-no-service",
                     ASAN_OPTIONS="detect_leaks=0"),
            check=True,
            timeout=15,
        )
    for mode in ("guide-all-owners",
                 "birth-context-escape",
                 "mode-ordinary-ready", "mode-dna", "mode-dna-missing",
                 "mode-dedicated", "mode-dedicated-unavailable",
                 "mode-dedicated-iddc", "mode-dedicated-dna", "mode-back",
                 "mode-random", "stats-order-reenter-completed",
                 "stats-order-reenter-partial",
                 "stats-order-reenter-reject",
                 "mode-ctrl-quit", "mode-rpg", "mode-pvp-maia",
                 "mode-pvp-maia-back", "stats-order-reenter-pvp-maia",
                 "alternate-order", "alternate-race-back",
                 "alternate-trait-back", "alternate-body-back",
                 "alternate-class-back", "alternate-class-back-no-body",
                 "alternate-class-back-no-trait-body", "alternate-navigate",
                 "alternate-random", "alternate-dna",
                 "alternate-class-dna-incompatible", "class-random-empty",
                 "hidden-derived",
                 "hidden-derived-base-mismatch",
                 "hidden-derived-mouse",
                 "hidden-derived-dna",
                 "hidden-derived-dna-missing", "hidden-derived-dna-incompatible",
                 "hidden-derived-dna-stale",
                 "alternate-mouse",
                 "alternate-ctrl-quit", "stats-order-navigate",
                 "stats-order-confirm", "stats-order-random", "stats-order-mouse",
                 "stats-back", "stats-back-no-trait-body", "stats-quit",
                 "stats-ctrl-quit", "stats-point-navigate",
                 "stats-point-confirm-warning", "stats-point-escape-warning",
                 "stats-point-confirm", "stats-point-escape-confirm",
                 "stats-point-mouse", "stats-point-dna",
                 "stats-point-dna-over-budget", "stats-point-dna-illegal",
                 "stats-point-dna-multiple-max", "stats-point-dna-missing",
                 "stats-point-lower-bound", "stats-point-escape-no-warning",
                 *QUIT_CASES):
        if ONLY_CASE and mode != ONLY_CASE:
            continue
        subprocess.run(
            [str(alternate_binary), str(profile), str(ROOT / "lib"), mode],
            cwd=ROOT,
            env=dict(os.environ, SDL_VIDEODRIVER="dummy",
                     SDL_RENDER_DRIVER="software",
                     DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-b023-no-service",
                     ASAN_OPTIONS="detect_leaks=0"),
            check=True,
            timeout=15,
        )
print("SV-B-022 name handoff and SV-B-023 birth choice production checks passed")
