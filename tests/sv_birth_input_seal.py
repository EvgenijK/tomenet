#!/usr/bin/env python3
"""Input discovery and observation for the existing B-024 producer, Linux/ELF.

No birth implementation lives here. runpy observes the unchanged runners and
compiler -M resolves their actual flags, including system headers.
"""
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import runpy
import shlex
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BINDING = '.scratch/sandcastle-0241e0da/batch-2.json'
PACKAGES = ('sdl3', 'sdl3-ttf', 'freetype2', 'libsecret-1')
RUN = subprocess.run
ACTIVE = None


def fingerprint(path):
    data = Path(path).read_bytes()
    return {'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()}


def fingerprints(paths):
    result = {}
    for path in sorted(paths):
        try:
            result[str(path)] = fingerprint(path)
        except (OSError, ValueError):
            result[str(path)] = {'missing': True}
    return result


def command(args):
    return RUN(args, cwd=ROOT, check=True, capture_output=True, text=True).stdout.strip()


def binding():
    data = json.loads((ROOT / BINDING).read_text())
    return next(row for row in data['executionPlan']['artifactChecks'] if row['id'] == 'birth-runtime')


def audit(event, args):
    if ACTIVE is not None and event == 'open' and isinstance(args[0], (str, bytes)):
        path = Path(os.fsdecode(args[0])).resolve()
        if path.is_file() and path.is_relative_to(ROOT) and '.git' not in path.parts:
            ACTIVE.add(str(path.relative_to(ROOT)))


sys.addaudithook(audit)


class Discovered(BaseException):
    pass


def observe(runner, env, output, discovery=False):
    """Observe a real runner; discovery stops at its first compiler invocation."""
    global ACTIVE
    saved_env = dict(os.environ)
    old_run = subprocess.run
    consumed = set()
    record = {'runner': runner, 'commands': [], 'generated': [], 'loaded_libraries': [],
              'effective_resources': [], 'repository_reads': []}
    ACTIVE = consumed
    os.environ.clear()
    os.environ.update(env)
    directory = output / runner.replace('.py', '')
    directory.mkdir(parents=True, exist_ok=True)

    def save_generated(path, producer, invocation):
        if not path.is_file() or path.is_relative_to(ROOT):
            return
        name = f'{len(record["generated"]):03d}-{path.name}'
        shutil.copyfile(path, directory / name)
        record['generated'].append({'original_path': str(path), 'artifact': str((directory / name).relative_to(output)),
            'fingerprint': fingerprint(path), 'producer': producer,
            'command': [sys.executable, '-B', 'tests/' + producer], 'consuming_command': invocation,
            'producer_inputs': ['tests/' + runner, 'tests/sv/fixtures/birth-current.hex'] if path.suffix in ('.hex', '.json') else sorted({'tests/' + runner, 'tests/sv_birth_complete_checks.py'} if runner == 'sv_birth_version_checks.py' else {'tests/' + runner})})

    def intercepted(args, *positional, **kwargs):
        args = [str(arg) for arg in args]
        compile_call = '-o' in args and any(arg.endswith('.c') for arg in args)
        if compile_call:
            # Evidence-only linker observer forwards the production query.
            args[args.index('-o'):args.index('-o')] = [
                'tests/sv/birth-input-observer.c', '-Wl,--wrap=sv_font_resource', '-Wl,--wrap=sv_font_open']
            record['commands'].append(args)
            if not discovery:
                dependency = dependency_record(runner, args, 'actual')
                record.setdefault('actual_dependencies', []).append(dependency)
                consumed.update(dependency['repository'])
            for path in Path(args[-1]).parent.rglob('tomenet.cfg'):
                save_generated(path, runner, args)
            if os.environ.get('SV_BIRTH_MATRIX'):
                matrix = Path(os.environ['SV_BIRTH_MATRIX'])
                save_generated(matrix, runner, args)
                for row in json.loads(matrix.read_text()):
                    save_generated(Path(row['fixture']), runner, args)
            if discovery:
                raise Discovered()
        native = Path(args[0]).is_file() and not compile_call and '-o' not in args
        if native:
            libraries = command(['ldd', args[0]])
            if 'not found' in libraries:
                raise RuntimeError('Missing native runtime library')
            record['linked_libraries'] = sorted(set(re.findall(r'(/[^\s()]+)', libraries)))
            record['binary'] = fingerprint(args[0])
            for arg in args[1:]:
                path = Path(arg).resolve()
                if path.is_file() and path.is_relative_to(ROOT):
                    consumed.add(str(path.relative_to(ROOT)))
                if path.is_dir() and (path / 'sv/tomenet.cfg').is_file():
                    save_generated(path / 'sv/tomenet.cfg', runner, args)
                elif path.suffix == '.hex':
                    save_generated(path, runner, args)
        result = RUN(args, *positional, **kwargs)
        if native:
            stderr = result.stderr or ''
            for name in re.findall(r'^SV sealed library: (.+)$', stderr, re.M):
                record['loaded_libraries'].append(str(Path(name).resolve()))
            for purpose, name in re.findall(r'^SV sealed (effective|loaded) font: (.+)$', stderr, re.M):
                resource = Path(name).resolve()
                consumed.add(str(resource.relative_to(ROOT)))
                record['effective_resources'].append({'path': str(resource.relative_to(ROOT)),
                    'handshake_basename': resource.name if purpose == 'effective' else None,
                    'purpose': purpose, 'fingerprint': fingerprint(resource)})
        return result

    subprocess.run = intercepted
    try:
        runpy.run_path(str(ROOT / 'tests' / runner), run_name='__main__')
    except Discovered:
        pass
    finally:
        subprocess.run = old_run
        os.environ.clear()
        os.environ.update(saved_env)
        ACTIVE = None
        record['repository_reads'] = sorted(consumed)
        (directory / 'observation.json').write_text(json.dumps(record, indent=2) + '\n')
    return record


def dependency_record(runner, compile_command, order):
    args = compile_command[:compile_command.index('-o')]
    args += ['-M', '-Wno-unused-command-line-argument']
    deps = command(args).replace('\\\n', '')
    paths = []
    for line in deps.splitlines():
        paths += shlex.split(line.split(':', 1)[1])
    resolved = {Path(path).resolve() for path in paths}
    return {'runner': runner, 'choice_order': order, 'compiler_command': compile_command,
            'dependency_command': args,
            'repository': sorted(str(path.relative_to(ROOT)) for path in resolved if path.is_relative_to(ROOT)),
            'external': sorted(str(path) for path in resolved if not path.is_relative_to(ROOT))}


def discover(suites, env, output):
    records = []
    repository = set()
    external = set()
    for _, runner, _ in suites:
        record = observe(runner, env, output, discovery=True)
        repository.update(record['repository_reads'])
        if len(record['commands']) != 1:
            raise RuntimeError(f'No unambiguous compiler command from {runner}')
        original = record['commands'][0]
        for order in ('0', '1'):
            args = [arg for arg in original if not arg.startswith('-DSV_CLASS_BEFORE_RACE=')]
            args.insert(1, f'-DSV_CLASS_BEFORE_RACE={order}')
            dependency = dependency_record(runner, args, order)
            repository.update(dependency['repository'])
            external.update(dependency['external'])
            records.append(dependency)
    return records, repository, external


def runtime(external=()):
    tools = {}
    paths = set(external)
    cc = os.environ.get('CC', 'clang')
    linker = command([cc, '-print-prog-name=ld'])
    for name, executable, flags in (
        ('python', sys.executable, ['--version']), ('compiler', cc, ['--version']),
        ('linker', linker, ['--version']), ('git', 'git', ['--version']),
        ('pkg_config', 'pkg-config', ['--version']), ('ldd', 'ldd', ['--version']),
        ('ldconfig', '/sbin/ldconfig', ['--version'])):
        found = shutil.which(executable)
        if not found:
            raise RuntimeError(f'Missing tool: {executable}')
        resolved = str(Path(found).resolve())
        paths.add(resolved)
        tools[name] = {'executable': found, 'resolved': resolved, 'version': command([found, *flags])}
        libs = command(['ldd', found]) if name in ('python', 'compiler', 'linker', 'git', 'pkg_config') else ''
        paths.update(re.findall(r'(/[^\s()]+)', libs))
    tools['compiler']['target'] = command([cc, '-dumpmachine'])
    tools['compiler']['search_dirs'] = command([cc, '-print-search-dirs'])
    tools['compiler']['resource_dir'] = command([cc, '-print-resource-dir'])
    # Sanitizers can be statically linked; seal the compiler's full runtime files.
    paths.update(str(p.resolve()) for p in Path(tools['compiler']['resource_dir']).rglob('*') if p.is_file())
    packages = {package: {flag: command(['pkg-config', flag, package])
                         for flag in ('--modversion', '--cflags', '--libs', '--variable=pcfiledir')}
                for package in PACKAGES}
    for package, info in packages.items():
        paths.add(str(Path(info['--variable=pcfiledir']) / (package + '.pc')))
    cache = command(['/sbin/ldconfig', '-p'])
    # Seal available dynamic dependencies before loading, including dlopen dependencies.
    paths.update(re.findall(r'=> (/\S+)', cache))
    # Linker startup objects, static support archives and link scripts.
    for directory in ('/usr/lib/gcc', '/usr/lib/x86_64-linux-gnu', str(Path(tools['compiler']['resource_dir']).parent / 'lib')):
        paths.update(str(p) for p in Path(directory).rglob('*')
                     if p.is_file() and p.suffix in ('.o', '.a', '.so'))
    # Python stdlib/extension bytes, not merely the interpreter version.
    import sysconfig
    for key in ('stdlib', 'platstdlib'):
        paths.update(str(p.resolve()) for p in Path(sysconfig.get_path(key)).rglob('*')
                     if p.is_file() and p.suffix in ('.py', '.so') and 'site-packages' not in p.parts)
    return {'tools': tools, 'packages': packages, 'ldconfig_cache': cache,
            'external_resolution': {str(path): str(Path(path).resolve()) for path in sorted(paths)},
            'external_files': fingerprints(paths),
            'package_build_identities': command(['dpkg-query', '-W', '-f=${Package}\t${Version}\t${Architecture}\n']),
            'platform': platform.platform(), 'backend': 'native SDL dummy/software',
            'environment': {k: v for k, v in sorted(os.environ.items())
                if k in ('PATH', 'CC', 'HOME', 'LANG', 'LC_ALL', 'DISPLAY', 'WAYLAND_DISPLAY', 'XDG_RUNTIME_DIR', 'CPATH', 'C_INCLUDE_PATH', 'CPLUS_INCLUDE_PATH',
                    'LIBRARY_PATH', 'COMPILER_PATH', 'GCC_EXEC_PREFIX', 'TMPDIR', 'TMP', 'TEMP', 'FONTCONFIG_FILE', 'FONTCONFIG_PATH')
                or k.startswith(('LD_', 'SDL_', 'ASAN_', 'UBSAN_', 'PKG_CONFIG', 'PYTHON', 'DBUS_', 'XDG_'))},
            'native_environment': {'SDL_VIDEODRIVER': 'dummy', 'SDL_RENDER_DRIVER': 'software',
                'DBUS_SESSION_BUS_ADDRESS': 'unix:path=/tmp/sv-b024-no-service', 'ASAN_OPTIONS': 'detect_leaks=0'}}


def provenance(sources):
    manifest = json.loads((ROOT / 'docs/capabilities/manifest.json').read_text())
    canonical = [row for row in manifest['sources'] if row['id'] in (
        'source.baseline.session-play', 'source.baseline.session-setup', 'source.baseline.session-dna')]
    for row in canonical:
        blob = RUN(['git', 'show', row['revision'] + ':' + row['path']], cwd=ROOT, check=True, capture_output=True).stdout
        row['historical_blob_fingerprint'] = {'size': len(blob), 'sha256': hashlib.sha256(blob).hexdigest()}
    return {'canonical_historical_sources': canonical,
            'current_runtime_fingerprints': {name: sources[name] for name in (
                'src/client/nclient.c', 'src/client/c-birth.c', 'src/client/c-tables.c',
                'src/client/c-init.c', 'src/client/c-util.c', 'lib/user/options.prf')},
            'anchors': {'src/client/c-tables.c': ['option_info'], 'src/client/c-util.c': ['handle_process_font_file'],
                        'src/client/c-birth.c': ['get_char_info', 'choose_mode', 'choose_trait'],
                        'src/client/nclient.c': ['Net_setup', 'Net_start']},
            'oracle_commands': ['python3 tests/sv/fixtures/birth-current-baseline.py',
                                'python3 tests/sv/fixtures/birth-dedicated-baseline.py',
                                'python3 tests/sv_birth_version_checks.py'],
            'derivation': 'Legacy option_info + shipped options.prf; literal baseline version gates and dedicated outcomes; '
                          'complete zero redefinition tables for unmapped default font. Expected packets never use SV output.',
            'font_mapping_absence': {name: not (ROOT / name).exists() for name in (
                'lib/user/font-sdl3.prf', 'lib/user/font-custom-CascadiaMono-Regular.ttf.prf')}}
