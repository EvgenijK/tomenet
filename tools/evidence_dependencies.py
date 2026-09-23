"""Exhaustive directory fingerprints, including ignored and untracked files."""
import hashlib
import os
from pathlib import Path


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def inventory(path):
    files = {}
    if not path.is_dir():
        raise OSError('Dependency tree unavailable')

    def fail(error):
        raise error

    for directory, directories, names in os.walk(path, onerror=fail):
        for name in directories + names:
            file = Path(directory) / name
            if file.is_symlink():
                raise ValueError('Dependency tree contains symlink')
            if file.is_file():
                files[file.relative_to(path).as_posix()] = digest(file)
            elif not file.is_dir():
                raise ValueError('Special dependency file: ' + str(file))
    return files


def capture(roots, scope):
    return [{'repository': repository, 'path': directory, 'role': role,
             'impact': 'complete', 'files': inventory(roots[repository] / directory)}
            for repository, directory, role in scope]
