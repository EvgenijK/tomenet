"""One isolated local server startup probe; never use original save/data."""
from pathlib import Path
import hashlib
import json
import os
import re
import signal
import socket
import subprocess
import time

root = Path(__file__).resolve().parents[2]
with_client = os.environ.get('SV_REAL_CLIENT') == '1'
out = root / ('.scratch/sv-c011-followup/real-server' if with_client else '.scratch/sv-c011-followup/local-server')
if os.environ.get('SV_REAL_OUTPUT'):
    out = root / '.scratch/sv-c011-followup' / os.environ['SV_REAL_OUTPUT']
out.mkdir(parents=True, exist_ok=True)
assert os.environ['SV_PARENT_NETNS'] != os.readlink('/proc/self/ns/net'), 'network namespace was not isolated'
subprocess.run(['ip', 'link', 'set', 'lo', 'up'], check=True)
import shutil
library = out / 'lib'
assert not library.exists(), 'use a fresh probe destination'
library.mkdir()
for name in ('config', 'game', 'modules', 'scpt', 'text', 'user'):
    shutil.copytree(root / 'lib' / name, library / name, symlinks=False)
for name in ('save', 'data', 'xtra'):
    (library / name).mkdir()
if with_client:
    for name in ('font', 'graphics', 'sv'):
        shutil.copytree(root / 'lib/xtra' / name, library / 'xtra' / name, symlinks=False)
config = library / 'config/tomenet.cfg'
text = config.read_text()
values = {'REPORT_TO_METASERVER': 'false', 'WORLDSERVER': '""',
          'WORLDPASS': '""', 'GAME_PORT': '23648', 'CONSOLE_PORT': '23649', 'GW_PORT': '23650'}
for key, value in values.items():
    text, count = re.subn(r'^' + key + r'\s*=.*$', key + ' = ' + value, text, flags=re.M)
    assert count == 1, key
config.write_text(text)
binary = root / 'src/tomenet.server'
command = [str(binary), '-z', '-m' + str(config)]
metadata = {'command': command, 'cwd': str(out), 'TOMENET_PATH': str(library),
            'binarySha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
            'networkNamespace': os.readlink('/proc/self/ns/net'),
            'parentNetworkNamespace': os.environ['SV_PARENT_NETNS'],
            'originalSaveOrDataCopied': False, 'externalNetworkAvailable': False}
with (out / 'stdout.log').open('wb') as stdout, (out / 'stderr.log').open('wb') as stderr:
    child = subprocess.Popen(command, cwd=out, env=dict(os.environ, TOMENET_PATH=str(library)),
                             stdout=stdout, stderr=stderr, start_new_session=True)
    deadline = time.monotonic() + 20
    listening = False
    try:
        while child.poll() is None and time.monotonic() < deadline:
            try:
                with socket.create_connection(('127.0.0.1', 23648), timeout=.1):
                    listening = True
                    break
            except OSError:
                time.sleep(.1)
        metadata['listeningLoopback'] = listening
        metadata['exitBeforeStop'] = child.poll()
        metadata['listeningSockets'] = subprocess.run(['ss', '-lnt'], text=True, capture_output=True).stdout
        if with_client and listening:
            profile = out / 'profile'
            (profile / 'sv').mkdir(parents=True)
            (profile / 'sv/tomenet.cfg').write_text('svSchemaVersion 1\nnick C011Probe\nsound 0\n')
            captures = out / 'captures'
            captures.mkdir()
            client_command = [str(root / '.scratch/sv-c011-followup/real-client'), str(profile), str(library)]
            environment = dict(os.environ, SDL_VIDEODRIVER='x11', SDL_RENDER_DRIVER='software',
                SDL_AUDIODRIVER='dummy', SV_C011_CAPTURE_DIR=str(captures),
                DBUS_SESSION_BUS_ADDRESS='unix:path=/tmp/sv-c011-no-service',
                HTTPS_PROXY='http://127.0.0.1:1', https_proxy='http://127.0.0.1:1',
                HTTP_PROXY='http://127.0.0.1:1', http_proxy='http://127.0.0.1:1',
                ALL_PROXY='http://127.0.0.1:1', all_proxy='http://127.0.0.1:1',
                NO_PROXY='127.0.0.1,localhost', no_proxy='127.0.0.1,localhost')
            with (out / 'client-stdout.log').open('wb') as client_stdout, (out / 'client-stderr.log').open('wb') as client_stderr:
                try:
                    result = subprocess.run(client_command, cwd=out, env=environment,
                        stdout=client_stdout, stderr=client_stderr, timeout=20)
                    metadata['clientExitCode'] = result.returncode
                except subprocess.TimeoutExpired:
                    metadata['clientTimeoutSeconds'] = 20
            metadata['clientCommand'] = client_command
            metadata['clientBinarySha256'] = hashlib.sha256(Path(client_command[0]).read_bytes()).hexdigest()
            metadata['sourceSha256'] = {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                for path in [*sorted((root / 'src/client/sv').rglob('*.c')),
                             *sorted((root / 'src/client/sv').rglob('*.h')),
                             root / '.scratch/sv-c011-followup/real-client.c',
                             root / '.scratch/sv-c011-followup/birth-wrapper.inc']}
            metadata['capturesSha256'] = {path.name: hashlib.sha256(path.read_bytes()).hexdigest()
                for path in captures.glob('*.bmp')}
    finally:
        if child.poll() is None:
            os.killpg(child.pid, signal.SIGTERM)
        child.wait(timeout=5)
metadata['exitCode'] = child.returncode
(out / 'command.json').write_text(json.dumps(metadata, indent=2) + '\n')
print(json.dumps(metadata, indent=2))
