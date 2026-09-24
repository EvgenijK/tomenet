#!/usr/bin/env python3
"""SV-B-002 native TCP contact through the production endpoint process."""
from pathlib import Path
import os
import socket
import struct
import subprocess
import tempfile
import threading
import time

ROOT = Path(__file__).resolve().parents[1]
subprocess.run(['make', '-f', 'makefile.sv', 'tomenet-sv', '-j4'],
               cwd=ROOT / 'src', check=True, capture_output=True)


def exact(peer, size):
    data = b''
    while len(data) < size:
        part = peer.recv(size - len(data))
        if not part:
            raise AssertionError('early EOF')
        data += part
    return data


def text_field(peer):
    data = b''
    while not data.endswith(b'\0'):
        data += exact(peer, 1)
        assert len(data) <= 80
    return data


def serve(listener, observed):
    try:
        peer, _ = listener.accept()
        with peer:
            peer.settimeout(5)
            observed['magic'] = exact(peer, 4)
            observed['real'] = text_field(peer)
            observed['port'] = exact(peer, 2)
            observed['marker'] = exact(peer, 1)
            observed['account'] = text_field(peer)
            observed['host'] = text_field(peer)
            observed['version'] = exact(peer, 2)
            observed['extended'] = exact(peer, 24)
            response = bytes([255, 0]) + struct.pack('>II6I', 0, 2, 4, 9, 4, 0, 0, 0)
            for byte in response:
                peer.sendall(bytes([byte]))
                time.sleep(0.001)
            observed['verify'] = exact(peer, 1) + text_field(peer) + text_field(peer) + text_field(peer)
            setup = bytes([2, 1, 122, 6]) + struct.pack('>I', 12345)
            setup += struct.pack('>IhBBBI', 0, 20, 0, 0, 0, 13)
            for byte in setup:
                peer.sendall(bytes([byte]))
                time.sleep(0.001)
            ping = bytes([166, 0]) + struct.pack('>III', 7, 8, 9) + b'xy\0'
            observed['ping'] = ping
            for byte in ping:
                peer.sendall(bytes([byte]))
                time.sleep(0.001)
            observed['echo'] = exact(peer, len(ping))
            peer.sendall(b'\x07')
            observed['unknown_reply'] = exact(peer, 11)
            peer.sendall(bytes([13, 11]))
            time.sleep(0.05)
            time.sleep(0.1)
    except Exception as error:
        observed['error'] = error


def reject(listener, observed):
    try:
        peer, _ = listener.accept()
        with peer:
            peer.settimeout(5)
            exact(peer, 4)
            text_field(peer)
            exact(peer, 3)
            text_field(peer)
            text_field(peer)
            exact(peer, 26)
            peer.sendall(bytes([255, 12]) + struct.pack('>II', 0, 0))
            time.sleep(0.1)
    except Exception as error:
        observed['error'] = error


with tempfile.TemporaryDirectory(prefix='sv-contact-live-') as temp:
    profile = Path(temp) / 'profile'
    profile.mkdir()
    with socket.socket() as listener:
        listener.bind(('127.0.0.1', 0))
        listener.listen(1)
        observed = {}
        thread = threading.Thread(target=serve, args=(listener, observed), daemon=True)
        thread.start()
        command = [str(ROOT / 'src/tomenet-sv'), '--endpoint', '--server', '127.0.0.1',
                   '--port', str(listener.getsockname()[1]), '--account', 'Test',
                   '--password-stdin', '--profile-root', str(profile),
                   '--library', str(ROOT / 'lib'), '--fixture-window', '1024x768',
                   '--frames', '200']
        run = subprocess.run(command, input='pw\n', text=True, capture_output=True,
                             cwd=ROOT, timeout=15,
                             env=dict(os.environ, SDL_VIDEODRIVER='dummy',
                                      SDL_RENDER_DRIVER='software'))
        thread.join(timeout=5)
        assert 'error' not in observed, observed.get('error')
        assert run.returncode == 0, (run.stdout, run.stderr)
        assert 'SV contact ready host=127.0.0.1' in run.stdout
        assert observed['magic'] == struct.pack('>I', 12345)
        assert observed['real'] == b'PLAYER\0'
        assert observed['port'] == b'\0\0' and observed['marker'] == b'\xff'
        assert observed['account'] == b'Test\0' and observed['host'] == b'localhost\0'
        assert observed['version'] == b'\xff\xff'
        assert observed['verify'] == b'\x01PLAYER\0Test\0Z]\0'
        assert observed['echo'] == observed['ping'][:1] + b'\x01' + observed['ping'][2:]
        assert observed['unknown_reply'] == bytes([211, 0, 0, 0, 7,
                                                   0, 0, 0, 166, 111, 0])
    with socket.socket() as listener:
        listener.bind(('127.0.0.1', 0))
        listener.listen(1)
        observed = {}
        thread = threading.Thread(target=reject, args=(listener, observed), daemon=True)
        thread.start()
        command = [str(ROOT / 'src/tomenet-sv'), '--endpoint', '--server', '127.0.0.1',
                   '--port', str(listener.getsockname()[1]), '--account', 'Test',
                   '--password-stdin', '--profile-root', str(profile),
                   '--library', str(ROOT / 'lib'), '--fixture-window', '1024x768',
                   '--frames', '200']
        run = subprocess.run(command, input='pw\n', text=True, capture_output=True,
                             cwd=ROOT, timeout=15,
                             env=dict(os.environ, SDL_VIDEODRIVER='dummy',
                                      SDL_RENDER_DRIVER='software'))
        thread.join(timeout=5)
        assert 'error' not in observed, observed.get('error')
        assert run.returncode == 1, (run.stdout, run.stderr)
        assert 'temporarily banned' in run.stderr
print('SV native TCP contact passed')
