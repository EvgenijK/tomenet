#!/usr/bin/env python3
"""Drive the production SV contact and login path with a deterministic peer."""
from pathlib import Path
import os
import socket
import struct
import subprocess
import tempfile
import threading
import time

ROOT = Path(__file__).resolve().parents[1]


def exact(peer, count):
    data = b""
    while len(data) < count:
        part = peer.recv(count - len(data))
        if not part:
            raise AssertionError("peer closed early")
        data += part
    return data


def field(peer):
    data = b""
    while not data.endswith(b"\0"):
        data += exact(peer, 1)
        assert len(data) < 80
    return data


def serve(listener, observed):
    try:
        peer, _ = listener.accept()
        with peer:
            peer.settimeout(8)
            observed["contact"] = exact(peer, 4) + field(peer) + exact(peer, 3)
            observed["account"] = field(peer)
            observed["host"] = field(peer)
            exact(peer, 26)
            peer.sendall(bytes([255, 0]) + struct.pack(">II6I", 0, 2, 4, 9, 4, 0, 0, 0))
            observed["verify"] = exact(peer, 1) + field(peer) + field(peer) + field(peer)
            setup = bytes([2, 1, 122, 6]) + struct.pack(">I", 12345)
            setup += struct.pack(">IhBBBI", 5, 20, 0, 0, 0, 13) + b"Hello"
            for at in range(0, len(setup), 3):
                peer.sendall(setup[at:at + 3])
                time.sleep(.001)
            observed["login_start"] = exact(peer, 8)
            flags = bytes([162]) + struct.pack(">IIII", 8, 0, 0, 0)
            row = bytes([12]) + struct.pack(">h", 1) + b"\xffW\0Hero\0"
            row += struct.pack(">hhh", 42, 2, 3) + b"at home\0"
            end = bytes([12]) + struct.pack(">h", 0) + b"\0\0"
            end += struct.pack(">hhh", 0, 0, 0) + b"\0"
            for byte in flags + row + end:
                peer.sendall(bytes([byte]))
                time.sleep(.001)
            observed["choice"] = exact(peer, 1) + field(peer)
            peer.sendall(b"\0")
            time.sleep(4)
    except Exception as error:
        observed["error"] = error


def reject_after_setup(listener, observed):
    try:
        peer, _ = listener.accept()
        with peer:
            peer.settimeout(8)
            exact(peer, 4)
            field(peer)
            exact(peer, 3)
            field(peer)
            field(peer)
            exact(peer, 26)
            peer.sendall(bytes([255, 0]) + struct.pack(">II6I", 0, 2, 4, 9, 4, 0, 0, 0))
            exact(peer, 1)
            field(peer)
            field(peer)
            field(peer)
            setup = bytes([2, 1, 122, 6]) + struct.pack(">I", 12345)
            setup += struct.pack(">IhBBBI", 0, 20, 0, 0, 0, 13)
            peer.sendall(setup)
            observed["login_start"] = exact(peer, 8)
            peer.sendall(bytes([4]) + b"Wrong password\0")
            observed["closed"] = peer.recv(1) == b""
    except Exception as error:
        observed["error"] = error


subprocess.run(["make", "-f", "makefile.sv", "tomenet-sv", "-j4"],
               cwd=ROOT / "src", check=True, capture_output=True)
with tempfile.TemporaryDirectory(prefix="sv-login-live-") as temp:
    profile = Path(temp) / "profile"
    profile.mkdir()
    with socket.socket() as listener:
        listener.bind(("127.0.0.1", 0))
        listener.listen(1)
        observed = {}
        worker = threading.Thread(target=serve, args=(listener, observed), daemon=True)
        worker.start()
        command = [str(ROOT / "src/tomenet-sv"), "--endpoint", "--server", "127.0.0.1",
                   "--port", str(listener.getsockname()[1]), "--account", "Test",
                   "--password-stdin", "--character", "hero", "-m", "--profile-root", str(profile),
                   "--library", str(ROOT / "lib"), "--fixture-window", "1024x768", "--frames", "100"]
        run = subprocess.run(command, input="pw\n", text=True, capture_output=True,
                             cwd=ROOT, timeout=12, env=dict(os.environ,
                                 SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                                 DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-login-no-service"))
        worker.join(timeout=6)
        assert "error" not in observed, observed.get("error")
        assert run.returncode == 0, (run.stdout, run.stderr)
        assert observed["account"] == b"Test\0"
        assert observed["verify"] == b"\x01PLAYER\0Test\0Z]\0"
        assert observed["login_start"][:4] == b"\x0c\0\xf4\x43"
        assert observed["choice"] == b"\x0cHero\0"
        assert "SV character selected" in run.stdout, (run.stdout, run.stderr)
    with socket.socket() as listener:
        listener.bind(("127.0.0.1", 0))
        listener.listen(1)
        observed = {}
        worker = threading.Thread(target=reject_after_setup,
                                  args=(listener, observed), daemon=True)
        worker.start()
        command[command.index("--port") + 1] = str(listener.getsockname()[1])
        run = subprocess.run(command, input="pw\n", text=True, capture_output=True,
                             cwd=ROOT, timeout=12, env=dict(os.environ,
                                 SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                                 DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/sv-login-no-service"))
        worker.join(timeout=6)
        assert "error" not in observed, observed.get("error")
        assert observed["closed"] and observed["login_start"][:4] == b"\x0c\0\xf4\x43"
        assert run.returncode == 1 and "Wrong password" in run.stderr
        assert "SV character selected" not in run.stdout
print("SV-B-006 login production socket path passed")
