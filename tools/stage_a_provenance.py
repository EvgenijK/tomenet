"""Reviewed Arch/Manjaro host package closure for the Stage A producer.

Metadata is an external dependency inventory, not a platform acceptance claim.
Other hosts must supply an equivalent producer before this checkpoint can pass.
"""
import json
import os
from pathlib import Path
import re

from evidence_dependencies import digest

# Includes tools invoked by make/SDK preparation and SDL's dynamically loaded
# graphics/window-system providers, beyond ELF's direct DT_NEEDED list.
PACKAGES = ('cmake', 'curl', 'clang', 'gcc', 'binutils', 'make', 'pkgconf', 'mingw-w64-gcc',
            'wine', 'python', 'sdl3', 'sdl3_ttf', 'freetype2', 'mesa', 'libglvnd',
            'vulkan-icd-loader', 'bash', 'coreutils', 'grep', 'sed', 'gawk',
            'findutils', 'file', 'fontconfig')


def fields(path):
    result = {}
    for block in path.read_text().strip().split('\n\n'):
        lines = block.splitlines()
        if lines:
            result[lines[0].strip('%')] = lines[1:]
    return result


def state(path):
    if path.is_symlink():
        return {'link': os.readlink(path)}
    if path.is_file():
        try:
            return {'sha256': digest(path)}
        except PermissionError:
            # Not usable as a compilation/runtime input by this unprivileged uid.
            stat = path.stat()
            return {'unreadable': [stat.st_mode, stat.st_uid, stat.st_gid, stat.st_size]}
    if path.is_dir():
        try:
            return {'entries': sorted(p.name for p in path.iterdir())}
        except PermissionError:
            stat = path.stat()
            return {'unreadable': [stat.st_mode, stat.st_uid, stat.st_gid, stat.st_size]}
    if not path.exists():
        return {'absent': True}
    raise OSError('Unsupported host dependency: ' + str(path))


def capture_host(destination):
    database = Path('/var/lib/pacman/local')
    packages, providers = {}, {}
    for desc in database.glob('*/desc'):
        data = fields(desc)
        name = data['NAME'][0]
        packages[name] = (desc.parent, data)
        for alias in data.get('PROVIDES', []):
            providers[re.split('[<>=]', alias)[0]] = name
    todo, selected = list(PACKAGES), set()
    while todo:
        name = re.split('[<>=]', todo.pop())[0]
        name = name if name in packages else providers.get(name, name)
        if name in selected:
            continue
        directory, data = packages[name]  # Unknown package fails closed.
        selected.add(name)
        todo.extend(data.get('DEPENDS', []))
    paths = set()
    for name in selected:
        directory, data = packages[name]
        paths.update([directory / 'desc', directory / 'files'])
        paths.update(Path('/') / f for f in fields(directory / 'files').get('FILES', [])
                     if f.split('/', 1)[0] in ('usr', 'etc', 'bin', 'sbin', 'lib', 'lib64', 'opt'))
    # Inventory the entire active Python environment, including dependency data
    # packages and aliases (attrs/attr), rather than a hand-picked import list.
    import sys
    if sys.prefix != sys.base_prefix:
        paths.add(Path(sys.prefix))
        paths.update(Path(sys.prefix).rglob('*'))
    paths.add(Path(sys.executable).resolve())
    # Include compiler/header search trees exhaustively, not just current .d inputs.
    for directory in ('/usr/include', '/usr/lib', '/usr/bin', '/usr/local/include',
                      '/usr/local/lib', '/etc/ld.so.conf.d'):
        path = Path(directory)
        if path.exists():
            paths.add(path)
            paths.update(path.rglob('*'))
    paths.add(Path('/etc/ld.so.cache'))
    # Resolve every linked input and retain each intermediate link, too.
    todo = list(paths)
    while todo:
        path = todo.pop()
        if path.is_symlink():
            target = Path(os.path.abspath(path.parent / os.readlink(path)))
            # mtab describes the current process mount namespace, not a static
            # compiler/runtime input. Retain the link itself, not /proc/self.
            if path == Path('/etc/mtab') and target == Path('/proc/self/mounts'):
                continue
            if target not in paths:
                paths.add(target)
                todo.append(target)
    # Each file's search directory detects additions/removals (including ignored
    # and untracked files); include subdirectories supplied by selected packages.
    paths.update(p.parent for p in list(paths) if not p.is_dir())
    import shutil
    for tool in ('cmake', 'curl', 'clang', 'gcc', 'ld', 'make', 'pkg-config', 'i686-w64-mingw32-gcc',
                 'wine', 'wineboot', 'bash', 'sh', 'head', 'sha256sum', 'cut', 'cp', 'file'):
        executable = shutil.which(tool)
        if executable is None or Path(executable).resolve() not in paths:
            raise ValueError('Tool outside reviewed package closure: ' + tool)
    data = {'schemaVersion': 1, 'packages': sorted(selected),
            'files': {str(p): state(p) for p in sorted(paths)}}
    destination.write_text(json.dumps(data, indent=2) + '\n')
    return data


def verify_host(path):
    data = json.loads(path.read_text())
    if data.get('schemaVersion') != 1 or not set(PACKAGES) <= set(data['packages']) or not data['files']:
        raise ValueError('Incomplete host provenance')
    for name, expected in data['files'].items():
        if state(Path(name)) != expected:
            raise ValueError('Host dependency changed: ' + name)


def audit_build_inputs(root, sdk, output):
    """Use build metadata as a cross-check of the reviewed scopes, not their proof."""
    import shlex
    allowed = [root / 'src', root / 'tests', Path('/usr/include'),
               Path('/usr/lib'), Path('/usr/i686-w64-mingw32')]
    if sdk is not None:
        allowed.append(sdk)
    metadata = []
    for name in ('build-linux', 'build-mingw'):
        log = (output / (name + '.log')).read_text()
        matches = re.findall(r'SV synthetic shell: (\.sv-build/[^\s]+/build.txt)', log)
        if len(matches) != 1:
            raise ValueError('Missing or ambiguous build metadata: ' + name)
        metadata.append(root / 'src' / matches[0])
    if not metadata:
        raise ValueError('Build metadata missing')
    for path in metadata:
        for line in path.read_text().splitlines():
            if not line.startswith(('cflags=', 'ldflags=', 'libs=')):
                continue
            tokens = shlex.split(line.split('=', 1)[1])
            for index, token in enumerate(tokens):
                directory = None
                if token in ('-I', '-L', '-isystem', '-iquote'):
                    directory = tokens[index + 1]
                elif token.startswith(('-I', '-L')):
                    directory = token[2:]
                if directory:
                    resolved = (root / 'src' / directory).resolve()
                    if not any(resolved.is_relative_to(scope) for scope in allowed):
                        raise ValueError('Unreviewed include/library search path: ' + str(resolved))
        for dep in path.parent.rglob('*.d'):
            # Current makefile emits MMD paths without spaces. Unsupported syntax
            # fails closed, rather than claiming closure from a partial parser.
            for token in dep.read_text().replace('\\\n', ' ').split():
                if token.endswith(':'):
                    continue
                resolved = (root / 'src' / token).resolve()
                if not any(resolved.is_relative_to(scope) for scope in allowed):
                    raise ValueError('Dependency outside reviewed roots: ' + str(resolved))
