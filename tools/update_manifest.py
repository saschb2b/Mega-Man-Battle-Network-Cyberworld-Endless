#!/usr/bin/env python3
"""A release's update.json: what the game reads at its start to know a
newer version is out, and where the copies that update themselves find
their file (src/update/, docs/engine/updates.md). The release workflow
attaches it to each release, so its address,

    https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/latest/download/update.json

always serves the newest one. For each file the release has, under the
key the game asks for: its address at the release's tag, its size and its
SHA-256, which the game checks before it puts the file in place.

    python3 tools/update_manifest.py vX.Y.Z build/release > update.json
"""
import hashlib
import json
import os
import sys

REPO = 'saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless'

# (the game's key for each file: src/update/update_install.c's, and the
# ones the systems that update themselves later will ask for)
FILES = {
    'appimage': 'cyberworld-endless-x86_64.AppImage',
    'windows-setup': 'cyberworld-endless-windows-x64-setup.exe',
    'windows-zip': 'cyberworld-endless-windows-x64.zip',
    'macos': 'cyberworld-endless-macos.dmg',
    'android': 'cyberworld-endless.apk',
    'portmaster': 'cyberworld-endless-portmaster.zip',
    '3ds-cia': 'cyberworld-endless.cia',
    '3ds-3dsx': 'cyberworld-endless.3dsx',
    'switch': 'cyberworld-endless.nro',
    'vita': 'cyberworld-endless.vpk',
    'ps4': 'cyberworld-endless.pkg',
    'ps5': 'cyberworld-endless-ps5.zip',
}


def digest(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def manifest(tag, folder):
    version = tag[1:] if tag.startswith('v') else tag
    files = {}
    for key, name in FILES.items():
        path = os.path.join(folder, name)
        if os.path.isfile(path):
            files[key] = {
                'url': f'https://github.com/{REPO}/releases/download/{tag}/{name}',
                'size': os.path.getsize(path),
                'sha256': digest(path),
            }
    return {
        'version': version,
        'notes': f'https://github.com/{REPO}/releases/tag/{tag}',
        'files': files,
    }


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    tag, folder = sys.argv[1], sys.argv[2]
    if not tag.startswith('v') or tag[1:].count('.') != 2:
        sys.exit(f'{tag}: not a release tag (vX.Y.Z)')
    json.dump(manifest(tag, folder), sys.stdout, indent=1)
    sys.stdout.write('\n')


if __name__ == '__main__':
    main()
