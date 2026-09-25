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
            setup += struct.pack('>IhBBBI', 5, 20, 1, 1, 1, 13)
            setup += bytes([50, 51, 49, 52, 48, 50]) + b'Human\0' + struct.pack('>I', 0x01020304)
            setup += bytes([50] * 6) + b'1 Hidden\0' + bytes([1, 2, 3, 4, 5, 6])
            setup += b'Trait\0' + struct.pack('>I', 0x09080706) + b'Hello'
            for byte in setup:
                peer.sendall(bytes([byte]))
                time.sleep(0.001)
            ping = bytes([166, 0]) + struct.pack('>III', 7, 8, 9) + b'xy\0'
            observed['ping'] = ping
            for byte in ping:
                peer.sendall(bytes([byte]))
                time.sleep(0.001)
            observed['echo'] = exact(peer, len(ping))
            peer.sendall(b'\x96')
            peer.sendall(b'\x07')
            observed['unknown_reply'] = exact(peer, 11)
            peer.sendall(bytes([13, 11]))
            time.sleep(3.5)
    except Exception as error:
        observed['error'] = error


def serve_old_setup(listener, observed):
    try:
        peer, _ = listener.accept()
        with peer:
            peer.settimeout(5)
            observed['magic'] = exact(peer, 4)
            observed['real'] = text_field(peer)
            observed['port_marker'] = exact(peer, 3)
            observed['account'] = text_field(peer)
            observed['host'] = text_field(peer)
            observed['version_offer'] = exact(peer, 26)
            peer.sendall(bytes([255, 0]) + struct.pack('>II6I', 0, 2, 4, 4, 3, 1, 0, 0))
            observed['verify'] = exact(peer, 1) + text_field(peer) + text_field(peer) + text_field(peer)
            setup = bytes([2, 1, 122, 6]) + struct.pack('>I', 12345)
            setup += struct.pack('>IhBBI', 3, 20, 1, 1, 12)
            setup += bytes([50] * 6) + b'R\0' + struct.pack('>I', 1)
            setup += bytes([50] * 6) + b'C\0' + b'Old'
            for byte in setup + bytes([13, 7]):
                peer.sendall(bytes([byte]))
                time.sleep(0.001)
            observed['unknown_reply'] = exact(peer, 11)
            time.sleep(3.5)
    except Exception as error:
        observed['error'] = error


def reject(listener, observed, status):
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
            peer.sendall(bytes([255, status]) + struct.pack('>II', 0, 0))
            observed['closed'] = peer.recv(1) == b''
    except Exception as error:
        observed['error'] = error


def fail_after_contact(listener, observed, phase):
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
            peer.sendall(bytes([255, 0]) + struct.pack('>II6I', 0, 2, 4, 9, 4, 0, 0, 0))
            exact(peer, 1)
            text_field(peer)
            text_field(peer)
            text_field(peer)
            if phase == 'verify':
                peer.sendall(bytes([2, 1, 0]))
            else:
                peer.sendall(bytes([2, 1, 122, 6]) + struct.pack('>I', 12345))
                peer.sendall(struct.pack('>IhBBBI', 999999, 20, 0, 0, 0, 13))
            time.sleep(0.1)
    except Exception as error:
        observed['error'] = error


def serve_without_reply(listener, observed):
    try:
        peer, _ = listener.accept()
        with peer:
            observed['accepted'] = True
            time.sleep(11)
    except Exception as error:
        observed['error'] = error


def native_command(port, profile):
    return [str(ROOT / 'src/tomenet-sv'), '--endpoint', '--server', '127.0.0.1',
            '--port', str(port), '--account', 'Test', '--password-stdin',
            '--profile-root', str(profile), '--library', str(ROOT / 'lib'),
            '--fixture-window', '1024x768', '--frames', '200']


with tempfile.TemporaryDirectory(prefix='sv-contact-live-') as temp:
    profile = Path(temp) / 'profile'
    profile.mkdir()
    with socket.socket() as listener:
        listener.bind(('127.0.0.1', 0))
        listener.listen(1)
        observed = {}
        thread = threading.Thread(target=serve, args=(listener, observed), daemon=True)
        thread.start()
        command = native_command(listener.getsockname()[1], profile)
        run = subprocess.run(command, input='pw\n', text=True, capture_output=True,
                             cwd=ROOT, timeout=15,
                             env=dict(os.environ, SDL_VIDEODRIVER='dummy',
                                      SDL_RENDER_DRIVER='software'))
        thread.join(timeout=5)
        assert 'error' not in observed, observed.get('error')
        assert run.returncode == 0, (run.stdout, run.stderr)
        assert 'SV contact ready host=127.0.0.1' in run.stdout
        assert 'races=1 classes=1 traits=1 motd=5' in run.stdout
        assert observed['magic'] == struct.pack('>I', 12345)
        assert observed['real'] == b'PLAYER\0'
        assert observed['port'] == b'\0\0' and observed['marker'] == b'\xff'
        assert observed['account'] == b'Test\0' and observed['host'] == b'localhost\0'
        assert observed['version'] == b'\xff\xff'
        assert observed['verify'] == b'\x01PLAYER\0Test\0Z]\0'
        assert observed['echo'] == observed['ping'][:1] + b'\x01' + observed['ping'][2:]
        assert observed['unknown_reply'] == bytes([211, 0, 0, 0, 7,
                                                   0, 0, 0, 150, 111, 0]), observed['unknown_reply']
    with socket.socket() as listener:
        listener.bind(('127.0.0.1', 0))
        listener.listen(1)
        observed = {}
        thread = threading.Thread(target=serve_old_setup, args=(listener, observed), daemon=True)
        thread.start()
        run = subprocess.run(native_command(listener.getsockname()[1], profile),
                             input='pw\n', text=True, capture_output=True,
                             cwd=ROOT, timeout=15,
                             env=dict(os.environ, SDL_VIDEODRIVER='dummy',
                                      SDL_RENDER_DRIVER='software'))
        thread.join(timeout=5)
        assert 'error' not in observed, observed.get('error')
        assert run.returncode == 0, (run.stdout, run.stderr)
        assert 'server=4.4.3.1.0.0 races=1 classes=1 traits=0 motd=3' in run.stdout
        assert observed['magic'] == struct.pack('>I', 12345)
        assert observed['real'] == b'PLAYER\0'
        assert observed['port_marker'] == b'\0\0\xff'
        assert observed['account'] == b'Test\0' and observed['host'] == b'localhost\0'
        assert observed['version_offer'] == b'\xff\xff' + struct.pack('>6I',
            4, 9, 4, 0, 0, 402000000)
        assert observed['verify'] == b'\x01PLAYER\0Test\0Z]\0'
        assert observed['unknown_reply'] == bytes([211, 0, 0, 0, 7,
                                                   0, 0, 0, 13, 111, 0])
    for status_code, reason in [(12, 'temporarily banned'),
                                (1, 'client version too old'),
                                (13, 'incompatible version'),
                                (2, 'game is full')]:
        with socket.socket() as listener:
            listener.bind(('127.0.0.1', 0))
            listener.listen(1)
            observed = {}
            thread = threading.Thread(target=reject,
                                      args=(listener, observed, status_code), daemon=True)
            thread.start()
            command = native_command(listener.getsockname()[1], profile)
            run = subprocess.run(command, input='pw\n', text=True, capture_output=True,
                                 cwd=ROOT, timeout=15,
                                 env=dict(os.environ, SDL_VIDEODRIVER='dummy',
                                          SDL_RENDER_DRIVER='software'))
            thread.join(timeout=5)
            assert 'error' not in observed and observed.get('closed'), observed
            assert run.returncode == 1, (run.stdout, run.stderr)
            assert reason in run.stderr, (status_code, run.stderr)
    for phase, status in [('verify', 'Verification failed'), ('setup', 'Server setup failed')]:
        with socket.socket() as listener:
            listener.bind(('127.0.0.1', 0))
            listener.listen(1)
            observed = {}
            thread = threading.Thread(target=fail_after_contact,
                                      args=(listener, observed, phase), daemon=True)
            thread.start()
            run = subprocess.run(native_command(listener.getsockname()[1], profile),
                                 input='pw\n', text=True, capture_output=True,
                                 cwd=ROOT, timeout=15,
                                 env=dict(os.environ, SDL_VIDEODRIVER='dummy',
                                          SDL_RENDER_DRIVER='software'))
            thread.join(timeout=5)
            assert 'error' not in observed, observed.get('error')
            assert run.returncode == 1, (phase, run.stdout, run.stderr)
            assert status in run.stderr, (phase, run.stderr)
    with socket.socket() as reserved:
        reserved.bind(('127.0.0.1', 0))
        closed_port = reserved.getsockname()[1]
    run = subprocess.run(native_command(closed_port, profile), input='pw\n',
                         text=True, capture_output=True, cwd=ROOT, timeout=15,
                         env=dict(os.environ, SDL_VIDEODRIVER='dummy',
                                  SDL_RENDER_DRIVER='software'))
    assert run.returncode == 1 and 'Cannot open server socket' in run.stderr
    invalid_host = 'a' * 64 + '.invalid'
    dns_command = native_command(closed_port, profile)
    dns_command[dns_command.index('127.0.0.1')] = invalid_host
    run = subprocess.run(dns_command, input='pw\n', text=True, capture_output=True,
                         cwd=ROOT, timeout=15,
                         env=dict(os.environ, SDL_VIDEODRIVER='dummy',
                                  SDL_RENDER_DRIVER='software'))
    assert run.returncode == 1 and 'Cannot resolve server address' in run.stderr, run.stderr
    with socket.socket() as listener:
        listener.bind(('127.0.0.1', 0))
        listener.listen(1)
        observed = {}
        thread = threading.Thread(target=serve_without_reply,
                                  args=(listener, observed), daemon=True)
        thread.start()
        command = native_command(listener.getsockname()[1], profile)
        command[-1] = '800'
        run = subprocess.run(command, input='pw\n', text=True, capture_output=True,
                             cwd=ROOT, timeout=18,
                             env=dict(os.environ, SDL_VIDEODRIVER='dummy',
                                      SDL_RENDER_DRIVER='software'))
        thread.join(timeout=12)
        assert observed.get('accepted') and 'error' not in observed, observed
        assert run.returncode == 1 and 'Server timed out' in run.stderr, run.stderr
print('SV native TCP contact passed')
