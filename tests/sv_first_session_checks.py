#!/usr/bin/env python3
"""SV-B-020 native M1 flow through production UI/model/input/transport/protocol."""
from pathlib import Path
import argparse
import hashlib
import json
import shutil
import sys
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
def native_transfer_check(mode='c010-transfer'):
    c011 = mode.startswith('c011-')
    evidence = ROOT / (f'.scratch/sv-c011-followup/{mode}' if mode in ('c011-controls', 'c011-birth', 'c011-audio-enabled', 'c011-audio-missing-pack', 'c011-audio-no-device') else f'.scratch/sv-c011/{mode}' if c011 else '.scratch/sv-c010/006/native')
    evidence.mkdir(parents=True, exist_ok=True)
    def hashes(folder):
        return {str(path.relative_to(folder)): hashlib.sha256(path.read_bytes()).hexdigest()
                for path in sorted(folder.rglob('*')) if path.is_file()}
    before = hashes(ROOT / 'lib')
    build = ['make', '-C', 'src', '-f', 'makefile.sv', '-j4', 'sv-first-session-check']
    built = subprocess.run(build, cwd=ROOT, text=True, capture_output=True)
    (evidence / 'build.log').write_text(built.stdout + built.stderr)
    built.check_returncode()
    paths = [line.split('=', 1)[1] for line in built.stdout.splitlines()
             if line.startswith('SV_FIRST_SESSION_CHECK=')]
    assert len(paths) == 1, built.stdout
    binary = Path(paths[0])
    with tempfile.TemporaryDirectory(prefix='sv-c010-native-') as temporary:
        work = Path(temporary)
        library = work / 'lib'
        shutil.copytree(ROOT / 'lib', library, symlinks=False)
        if mode == 'c011-initial-repair':
            (library / 'scpt/c-init.lua').rename(library / 'scpt/c-init.restore')
        elif mode == 'c011-transfer-failure':
            (library / 'scpt/blocked.lua').mkdir()
        profile, shared = work / 'profile', work / 'shared'
        profile.mkdir(); shared.mkdir()
        environment = dict(os.environ, SDL_VIDEODRIVER=os.getenv('SV_C011_VIDEO_DRIVER','dummy'),
            SDL_VIDEO_DRIVER=os.getenv('SV_C011_VIDEO_DRIVER','dummy'),
            SDL_RENDER_DRIVER='software', SDL_AUDIODRIVER='dummy', SDL_AUDIO_DRIVER='dummy',
            TOMENET_SDL3_USER_PATH=str(shared), TOMENET_PATH=str(library),
            DBUS_SESSION_BUS_ADDRESS='unix:path=/tmp/sv-c010-no-service',
            **({'SV_C011_NATIVE_EVIDENCE': str(evidence)} if c011 else
               {'SV_C010_NATIVE_EVIDENCE': str(evidence)}),
            HTTPS_PROXY='http://127.0.0.1:1', https_proxy='http://127.0.0.1:1',
            HTTP_PROXY='http://127.0.0.1:1', http_proxy='http://127.0.0.1:1',
            ALL_PROXY='http://127.0.0.1:1', all_proxy='http://127.0.0.1:1',
            NO_PROXY='127.0.0.1,localhost', no_proxy='127.0.0.1,localhost')
        if mode == 'c011-audio-no-device':
            environment['SDL_AUDIODRIVER'] = 'sv-unavailable-driver'
            environment['SDL_AUDIO_DRIVER'] = 'sv-unavailable-driver'
        if mode.startswith('c011-audio-'):
            environment.pop('SDL_AUDIO_DEVICE_SAMPLE_FRAMES', None)
        if os.getenv('SV_C011_CAPTURE_DIR'):
            Path(os.environ['SV_C011_CAPTURE_DIR']).mkdir(parents=True, exist_ok=True)
        command = [str(binary), str(profile), str(library), mode]
        metadata = {'buildCommand': build, 'command': command,
            'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
            'dirtyState': subprocess.check_output(['git', 'status', '--short'], cwd=ROOT, text=True),
            'environment': {key: environment.get(key) for key in ('SDL_VIDEODRIVER', 'SDL_VIDEO_DRIVER',
                'SDL_AUDIO_DEVICE_SAMPLE_FRAMES', 'SV_C011_CAPTURE_DIR',
                'SDL_RENDER_DRIVER', 'SDL_AUDIODRIVER', 'SDL_AUDIO_DRIVER', 'TOMENET_SDL3_USER_PATH',
                'TOMENET_PATH', 'DBUS_SESSION_BUS_ADDRESS',
                'SV_C011_NATIVE_EVIDENCE' if c011 else 'SV_C010_NATIVE_EVIDENCE',
                'HTTPS_PROXY', 'https_proxy', 'HTTP_PROXY', 'http_proxy',
                'ALL_PROXY', 'all_proxy', 'NO_PROXY', 'no_proxy')},
            'binarySha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
            'originalLibraryBefore': before, 'timeoutSeconds': 30,
            'sanitizers': 'ordinary production objects; sanitizer coverage not claimed'}
        (evidence / 'command.json').write_text(json.dumps(metadata, indent=2) + '\n')
        try:
            run = subprocess.run(command, cwd=ROOT, env=environment, text=True,
                                 capture_output=True, timeout=30)
        except subprocess.TimeoutExpired as error:
            (evidence / 'stdout.log').write_bytes(error.stdout or b'')
            (evidence / 'stderr.log').write_bytes(error.stderr or b'')
            raise
        (evidence / 'stdout.log').write_text(run.stdout)
        (evidence / 'stderr.log').write_text(run.stderr)
        metadata['exitCode'] = run.returncode
        metadata['originalLibraryAfter'] = hashes(ROOT / 'lib')
        metadata['publishedGuideSha256'] = hashlib.sha256((library / 'scpt/guide.lua').read_bytes()).hexdigest()
        metadata['isolatedSharedFiles'] = hashes(shared)
        metadata['sourcesSha256'] = {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in [ROOT / 'tests/sv/first-session.c', ROOT / 'tests/sv_first_session_checks.py',
                         ROOT / 'src/makefile.sv',
                         *sorted((ROOT / 'src/client/sv').rglob('*.c')),
                         *sorted((ROOT / 'src/client/sv').rglob('*.h'))]}
        (evidence / 'command.json').write_text(json.dumps(metadata, indent=2) + '\n')
        assert metadata['originalLibraryAfter'] == before, 'original library changed'
        run.check_returncode()
        if c011:
            assert 'SV-C011 native preparation/identity/INS/redraw checks passed' in run.stdout
            print('SV-C011 focused production preparation check passed; evidence:', evidence)
            return
        marker = 'C010 native transferred guide marker'
        assert run.stderr.count(marker) == 1, run.stderr
        expected = (ROOT / 'lib/scpt/guide.lua').read_bytes() + b'\n_ALERT("C010 native transferred guide marker")\n'
        assert (library / 'scpt/guide.lua').read_bytes() == expected
        assert 'SV-C010 native socket/FILE/Lua outcome passed' in run.stdout
        print('SV-C010 focused native socket/login/FILE/Lua check passed; evidence:', evidence)


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--case', choices=['c010-transfer', 'c011-preparation',
    'c011-initial-repair', 'c011-transfer-failure', 'c011-startup-quit', 'c011-cancel', 'c011-retry', 'c011-controls', 'c011-birth',
    'c011-audio-enabled', 'c011-audio-missing-pack', 'c011-audio-no-device'])
arguments = parser.parse_args()
if arguments.case:
    for mode in (['c011-preparation', 'c011-initial-repair', 'c011-transfer-failure',
                  'c011-startup-quit', 'c011-cancel', 'c011-retry', 'c011-controls', 'c011-birth',
                  'c011-audio-enabled', 'c011-audio-missing-pack', 'c011-audio-no-device']
                 if arguments.case == 'c011-preparation' else [arguments.case]):
        native_transfer_check(mode)
    sys.exit(0)

CC = os.environ.get("CC", "clang")
FLAGS = subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "sdl3", "sdl3-ttf", "sdl3-mixer", "freetype2", "libsecret-1", "libpng", "libcurl"],
    text=True,
).split()
SOURCES = [
    "src/client/sv/files/files.c", "src/client/sv/files/history.c", "src/client/sv/files/dna.c",
    "tests/sv/first-session.c",
    "src/client/sv/login/native-run.c", "src/client/sv/settings/config.c", "src/client/sv/settings/startup.c", "src/client/sv/settings/audio.c", "src/client/sv/login/flow.c", "src/client/sv/login/chronicle.c",
    "src/client/sv/result.c",
    "src/client/sv/credential/vault.c",
    "src/client/sv/login/account/credentials.c",
    "src/client/sv/login/characters/login-interaction.c",
    "src/client/sv/login/characters/native-login.c",
    "src/client/sv/login/characters/native-name.c",
    "src/client/sv/login/creation/native-birth.c",
    "src/client/sv/protocol/contact.c",
    "src/client/sv/protocol/contact-socket.c",
    "src/client/sv/protocol/login.c", "src/client/sv/settings/options.c", "src/client/sv/settings/preferences.c",
    "src/client/sv/protocol/login-identity.c",
    "src/client/sv/login/pregame.c",
    "src/client/sv/login/login-view.c",
    "src/client/sv/login/creation/birth.c",
    "src/client/sv/login/ui/endpoint-scene.c",
    "src/client/sv/login/ui/login-shell.c",
    "src/client/sv/ui/font.c",
    "src/client/sv/login/servers/endpoint.c",
    "src/client/sv/input/text-field.c",
    "src/client/sv/login/ui/native-endpoint.c",
    "src/client/sv/input/key-encoding.c",
    "src/client/sv/login/servers/metaserver.c",
    "src/common/md5.c",
]

with tempfile.TemporaryDirectory(prefix="sv-b020-") as temp:
    root = Path(temp)
    binary = root / "first-session"
    profile = root / "profile"
    profile.mkdir()
    subprocess.run(
        [CC, "-std=c99", "-D_DEFAULT_SOURCE", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-Isrc/client/sv", *SOURCES, *FLAGS,
         "-o", str(binary)],
        cwd=ROOT,
        check=True,
    )
    environment = dict(
        os.environ,
        SDL_VIDEODRIVER="dummy",
        SDL_RENDER_DRIVER="software",
        DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-b020-no-service",
        ASAN_OPTIONS="detect_leaks=0",
    )
    for mode in ("success", "disconnect", "quit", "retry"):
        subprocess.run(
            [str(binary), str(profile), str(ROOT / "lib"), mode],
            cwd=ROOT,
            env=environment,
            check=True,
            timeout=15,
        )
print("SV-B-020 first-session managed-peer checks passed")
