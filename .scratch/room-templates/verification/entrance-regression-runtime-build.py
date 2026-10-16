#!/usr/bin/env python3
"""Build a private diagnostic server; prints its fresh /tmp directory.

Build the normal server with make -C src tomenet.server first. This script does
not replace repository objects or run the server. The caller owns /tmp cleanup.
"""
from pathlib import Path
import concurrent.futures
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
OUTPUT = Path(tempfile.mkdtemp(prefix='tomenet-entrance-tunnel-runtime-'))
subprocess.run(['cp', '-a', str(ROOT / 'lib'), str(OUTPUT / 'lib')], check=True)
plan = subprocess.check_output(['make', '-C', str(ROOT / 'src'), '-Bn', 'tomenet.server'], text=True)
link = shlex.split(next(line for line in plan.splitlines() if ' -o tomenet.server ' in line))
link[link.index('-o') + 1] = str(OUTPUT / 'tomenet.server')
base = ['clang', '-pipe', '-Wall', '-DUSE_GCU', '-D_XOPEN_SOURCE', '-D_BSD_SOURCE', '-DMEXP=19937',
        '-std=c99', '-D_DEFAULT_SOURCE', '-DACC32', '-fPIE', '-fsigned-char', '-DLINUX',
        '-DENABLE_ROOM_TEMPLATES', '-Iserver', '-Iserver/lua', '-g', '-O0', '-Wno-macro-redefined']


def compile_source(name):
    subprocess.run(base + ['-c', 'server/' + name + '.c', '-o', str(OUTPUT / (name + '.o'))],
                   cwd=ROOT / 'src', check=True)


with concurrent.futures.ThreadPoolExecutor() as pool:
    list(pool.map(compile_source, ['generate', 'main', 'go']))
for name in ['generate', 'main', 'go']:
    link = [str(OUTPUT / (name + '.o')) if argument == 'server/' + name + '.o' else argument for argument in link]
subprocess.run(link, cwd=ROOT / 'src', check=True)
(OUTPUT / 'link-args.txt').write_text('\n'.join(link))
print(OUTPUT)
