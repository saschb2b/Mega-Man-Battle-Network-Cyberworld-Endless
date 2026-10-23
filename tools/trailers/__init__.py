"""The trailers, one for each release that earned one (AGENTS.md,
Trailers): a VERSION's footage and look in vX_Y_Z.py, its music in
vX_Y_Z_music.py. The newest is the current one: docs/clips/trailer.* are
its, and the 3DS HOME Menu banner plays its music's BANNER. An older one
still renders from here, into .build/ only, and its video stays at its
release's tag."""
import importlib

CURRENT = '0.14.0'
VERSIONS = ('0.3.0', '0.12.0', '0.13.0', '0.14.0', '0.15.0')


def module(version=None, music=False):
    """A trailer's module, the current one's by default: its look, or with
    `music` its music (which needs nothing outside the standard library,
    for the 3DS build's banner)."""
    version = version or CURRENT
    if version not in VERSIONS:
        raise SystemExit(f'no trailer {version}: {", ".join(VERSIONS)}')
    return importlib.import_module('trailers.v' + version.replace('.', '_') + ('_music' if music else ''))


def music(version=None):
    return module(version, music=True)
