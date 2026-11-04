#!/usr/bin/env python3
"""One-off C001 extracted CLI observations; not a reusable test suite/model."""
import ctypes as C
import ctypes.util
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile
import time

archive = Path(sys.argv[1]).resolve()
results = Path(sys.argv[2]).resolve()
results.mkdir(parents=True, exist_ok=True)
work = Path('/tmp/sv-c001-observation')
work.mkdir(exist_ok=False)
(work / 'unrelated').mkdir()
with tarfile.open(archive) as source:
    source.extractall(work / 'unpacked', filter='data')
initial = next((work / 'unpacked').iterdir())
package = work / 'relocated' / 'installed'
package.parent.mkdir()
initial.rename(package)
report = {'archive': str(archive), 'sha256': hashlib.sha256(archive.read_bytes()).hexdigest(),
          'package': str(package), 'cwd': str(work / 'unrelated'),
          'environment': Path('/etc/os-release').read_text(), 'observations': [],
          'scope': 'Fedora41 isolated runtime; Xvfb OpenGL is Mesa software rendering, not physical GPU'}
subprocess.run(['sha256sum', '-c', 'SHA256SUMS'], cwd=package,
               stdout=(results / 'inventory-check.log').open('w'), stderr=subprocess.STDOUT, check=True)
for title, command in [('elf-header', ['readelf', '-h', str(package / 'tomenet-sv')]),
                       ('elf-dynamic', ['readelf', '-d', str(package / 'tomenet-sv')]),
                       ('elf-versions', ['readelf', '--version-info', str(package / 'tomenet-sv')]),
                       ('ldd', ['ldd', str(package / 'tomenet-sv')])]:
    with (results / (title + '.log')).open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
xvfb_log = (results / 'xvfb.log').open('w')
xvfb = subprocess.Popen(['Xvfb', ':77', '-screen', '0', '1280x900x24', '-nolisten', 'tcp', '-ac'],
                        stdout=xvfb_log, stderr=subprocess.STDOUT)


def environment(name, renderer='software'):
    isolated = work / name
    isolated.mkdir()
    for item in ('home', 'data', 'config', 'cache', 'runtime'):
        (isolated / item).mkdir(mode=0o700)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('LD_', 'SDL_', 'TOMENET_', 'SV_B2_', 'XDG_'))}
    env.update(HOME=str(isolated / 'home'), XDG_DATA_HOME=str(isolated / 'data'),
               XDG_CONFIG_HOME=str(isolated / 'config'), XDG_CACHE_HOME=str(isolated / 'cache'),
               XDG_RUNTIME_DIR=str(isolated / 'runtime'), DISPLAY=':77',
               SDL_VIDEO_DRIVER='x11', SDL_RENDER_DRIVER=renderer, LIBGL_ALWAYS_SOFTWARE='1',
               SV_B2_CAPTURE=str(results / (name + '.bmp')), SV_B2_REDUCED_MOTION='1')
    return isolated, env


def observe(name, env, root=package, expected='success', bare=False, trace=False):
    command = [str(root / 'tomenet-sv')] + ([] if bare else ['--frames', '2'])
    if trace:
        command = ['strace', '-f', '-e', 'trace=file', '-o', str(results / (name + '.strace'))] + command
    item = {'name': name, 'command': command, 'expected': expected,
            'overrides': {k: v for k, v in env.items() if k in ('HOME', 'DISPLAY', 'SDL_VIDEO_DRIVER', 'SDL_RENDER_DRIVER', 'LIBGL_ALWAYS_SOFTWARE') or k.startswith(('XDG_', 'TOMENET_', 'SV_B2_'))}}
    with (results / (name + '.log')).open('w') as log:
        proc = subprocess.Popen(command, cwd=work / 'unrelated', env=env, stdout=log, stderr=subprocess.STDOUT)
        try:
            if bare:
                item['close'] = close_own_window(proc.pid)
            item['exit'] = proc.wait(timeout=30)
        except Exception as error:
            item['error'] = repr(error)
            proc.kill()
            item['exit'] = proc.wait()
    item['output'] = (results / (name + '.log')).read_text()
    capture = Path(env['SV_B2_CAPTURE'])
    item['capture'] = capture.name if capture.exists() else None
    report['observations'].append(item)
    (results / 'observations.json').write_text(json.dumps(report, indent=2))


def close_own_window(pid):
    x = C.CDLL(ctypes.util.find_library('X11'))
    U = C.c_ulong
    P = C.c_void_p
    x.XOpenDisplay.argtypes = [C.c_char_p]; x.XOpenDisplay.restype = P
    x.XDefaultRootWindow.argtypes = [P]; x.XDefaultRootWindow.restype = U
    x.XQueryTree.argtypes = [P,U,C.POINTER(U),C.POINTER(U),C.POINTER(C.POINTER(U)),C.POINTER(C.c_uint)]
    x.XInternAtom.argtypes = [P,C.c_char_p,C.c_int]; x.XInternAtom.restype = U
    x.XGetWindowProperty.argtypes = [P,U,U,C.c_long,C.c_long,C.c_int,U,C.POINTER(U),C.POINTER(C.c_int),C.POINTER(U),C.POINTER(U),C.POINTER(C.POINTER(C.c_ubyte))]
    x.XFetchName.argtypes = [P,U,C.POINTER(C.c_char_p)]; x.XFetchName.restype = C.c_int
    x.XFree.argtypes = [P]; x.XCloseDisplay.argtypes = [P]; x.XFlush.argtypes = [P]
    class Data(C.Union):
        _fields_ = [('l', C.c_long * 5), ('b', C.c_char * 20)]
    class Client(C.Structure):
        _fields_ = [('type',C.c_int),('serial',U),('send_event',C.c_int),('display',P),('window',U),('message_type',U),('format',C.c_int),('data',Data)]
    class Event(C.Union):
        _fields_ = [('client',Client),('pad',C.c_long * 24)]
    x.XSendEvent.argtypes = [P,U,C.c_int,C.c_long,C.POINTER(Event)]
    display = x.XOpenDisplay(b':77')
    if not display:
        raise RuntimeError('XOpenDisplay failed')
    pid_atom = x.XInternAtom(display, b'_NET_WM_PID', False)
    def search(window):
        actual, fmt, count, after, data = U(), C.c_int(), U(), U(), C.POINTER(C.c_ubyte)()
        status = x.XGetWindowProperty(display,window,pid_atom,0,1,False,0,C.byref(actual),C.byref(fmt),C.byref(count),C.byref(after),C.byref(data))
        found = status == 0 and fmt.value == 32 and count.value == 1 and C.cast(data,C.POINTER(U))[0] == pid
        if data: x.XFree(data)
        if found:
            title = C.c_char_p()
            valid = x.XFetchName(display,window,C.byref(title)) and title.value == b'TomeNET SV - Server'
            if title: x.XFree(title)
            if valid: return window
        root, parent, children, size = U(), U(), C.POINTER(U)(), C.c_uint()
        if not x.XQueryTree(display,window,C.byref(root),C.byref(parent),C.byref(children),C.byref(size)): return None
        ids = [children[i] for i in range(size.value)]
        if children: x.XFree(children)
        for child in ids:
            found = search(child)
            if found: return found
        return None
    try:
        window = None
        for _ in range(100):
            window = search(x.XDefaultRootWindow(display))
            if window: break
            time.sleep(0.1)
        if not window: raise RuntimeError('No own X11 window within 10 seconds')
        # Allow initial production render and capture before ordinary close.
        time.sleep(0.8)
        window = search(x.XDefaultRootWindow(display))
        if not window: raise RuntimeError('Own startup window disappeared before close')
        event = Event()
        event.client.type=33; event.client.display=display; event.client.window=window
        event.client.message_type=x.XInternAtom(display,b'WM_PROTOCOLS',False)
        event.client.format=32
        event.client.data.l[0]=x.XInternAtom(display,b'WM_DELETE_WINDOW',False)
        if not x.XSendEvent(display,window,False,0,C.byref(event)): raise RuntimeError('XSendEvent failed')
        x.XFlush(display)
        return {'window': hex(window), 'pid': pid, 'event': 'WM_DELETE_WINDOW'}
    finally:
        x.XCloseDisplay(display)

try:
    time.sleep(0.6)
    if xvfb.poll() is not None:
        raise RuntimeError('Xvfb did not start')
    for name, renderer in [('default-software','software'), ('default-opengl','opengl')]:
        _, env = environment(name, renderer)
        observe(name, env, trace=True)
    _, env = environment('bare-close')
    observe('bare-close', env, bare=True)
    isolated, env = environment('user-overlay')
    profile = isolated / 'overlay'
    (profile / 'xtra/font').mkdir(parents=True)
    shutil.copyfile(package / 'lib/xtra/font/CascadiaMono-Regular.ttf', profile / 'xtra/font/CascadiaMono-Regular.ttf')
    sentinel = profile / 'tomenet.cfg'
    sentinel.write_text('unrelated legacy owner sentinel\n')
    env['TOMENET_SDL3_USER_PATH'] = str(profile)
    observe('user-overlay', env, trace=True)
    report['overlay_sentinel_unchanged'] = sentinel.read_text() == 'unrelated legacy owner sentinel\n'
    isolated, env = environment('unavailable-root')
    unavailable = isolated / 'root-is-file'
    unavailable.write_text('unrelated owner must remain unchanged\n')
    env['TOMENET_SDL3_USER_PATH'] = str(unavailable)
    observe('unavailable-root', env)
    report['unavailable_root_unchanged'] = unavailable.read_text() == 'unrelated owner must remain unchanged\n'
    broken = work / 'broken-fonts'
    shutil.copytree(package, broken)
    shutil.rmtree(broken / 'lib/xtra/font')
    _, env = environment('missing-required-fonts')
    observe('missing-required-fonts', env, root=broken, expected='nonzero resource failure')
    _, env = environment('invalid-renderer', 'sv-c001-nonexistent')
    observe('invalid-renderer', env, expected='nonzero renderer failure')
    broken = work / 'broken-loader'
    shutil.copytree(package, broken)
    libraries = sorted(broken.glob('libSDL3.so*'))
    if not libraries: raise RuntimeError('No bundled libSDL3.so to remove for loader observation')
    for item in libraries: item.unlink()
    _, env = environment('missing-loader-dependency')
    observe('missing-loader-dependency', env, root=broken, expected='nonzero dynamic loader failure')
finally:
    (results / 'observations.json').write_text(json.dumps(report, indent=2))
    xvfb.terminate()
    xvfb.wait(timeout=5)
    xvfb_log.close()
