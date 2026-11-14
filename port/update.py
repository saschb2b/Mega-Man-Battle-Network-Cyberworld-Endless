"""The PortMaster port's update, put in place after the game ends
(docs/engine/updates.md, A PortMaster handheld).

The game downloads the release's port zip into the update folder beside
the game's own (ports/.cyberworld-update/update.zip), checks its SHA-256
and writes ready.txt with its version (src/update/update_install.c);
the launcher then runs this, with PortMaster's "keep the device on"
message up:

    python3 update.py apply --game-dir ports/cyberworld --launcher "ports/Cyberworld Endless.sh"

The new folder is built from a copy of the old one, the player's files
(the saves, settings.ini, rom/) kept as they are and the zip's files over
them, and checked; then, under a journal, the old folder and launcher
step aside and the new ones take their names. The launcher starts again
on exit code 10. An update cut short (the device switched off) is undone
by `recover` at the launcher's next start, from the copy this leaves in
the update folder. The way was found first for Dungeons of Infinity and
Beyond's port (its installer/updater.py).
"""
import argparse
import json
import os
import shutil
import stat
import sys
from pathlib import PurePosixPath
from zipfile import BadZipFile, ZipFile

GAME = 'cyberworld'
LAUNCHER = 'Cyberworld Endless.sh'
BINARY = 'cyberworld.aarch64'
MAX_UNPACKED = 256 * 1024 * 1024
DONE = 10   # the launcher starts again


def work_dir(game):
    return os.path.join(os.path.dirname(game), '.' + os.path.basename(game) + '-update')


def write_json(path, value):
    temporary = path + '.tmp'
    with open(temporary, 'w') as f:
        json.dump(value, f)
        f.flush()
        os.fsync(f.fileno())
    os.replace(temporary, path)


def sync():
    if hasattr(os, 'sync'):
        os.sync()


def check_zip(zipped):
    """The release's zip holds the launcher and the game's folder, nothing else."""
    seen, total = set(), 0
    for entry in zipped.infolist():
        name, mode = entry.filename, entry.external_attr >> 16
        parts = PurePosixPath(name).parts
        total += entry.file_size
        if (name.startswith('/') or '..' in parts or '\\' in name or name in seen or stat.S_ISLNK(mode)
                or total > MAX_UNPACKED or not (name == LAUNCHER or name.startswith(GAME + '/'))):
            raise ValueError(f'the update holds a file it may not: {name}')
        seen.add(name)
    if LAUNCHER not in seen or GAME + '/' + BINARY not in seen:
        raise ValueError('the update lacks the launcher or the game')


def unpack(zipped, stage):
    """The zip's files over the staged copy (the player's files kept where the zip has none)."""
    for entry in zipped.infolist():
        target = os.path.join(stage, *PurePosixPath(entry.filename).parts)
        if entry.is_dir():
            os.makedirs(target, exist_ok=True)
            continue
        os.makedirs(os.path.dirname(target), exist_ok=True)
        with zipped.open(entry) as source, open(target + '.new', 'wb') as out:
            shutil.copyfileobj(source, out)
        os.replace(target + '.new', target)
        if target.endswith('.sh') or target.endswith('.aarch64'):
            os.chmod(target, 0o755)


def recover(game, launcher):
    """An update cut short undone: the previous folder and launcher back."""
    work = work_dir(game)
    journal = os.path.join(work, 'transaction.json')
    if not os.path.exists(journal):
        return False
    previous = os.path.join(work, 'previous')
    if os.path.isdir(previous):
        shutil.rmtree(game, ignore_errors=True)
        os.replace(previous, game)
    old_launcher = os.path.join(work, 'previous-launcher.sh')
    if os.path.exists(old_launcher):
        os.replace(old_launcher, launcher)
    os.remove(journal)
    sync()
    return True


def apply(game, launcher):
    """The ready update put in place: DONE, or 0 where none waits."""
    work = work_dir(game)
    ready = os.path.join(work, 'ready.txt')
    archive = os.path.join(work, 'update.zip')
    if not os.path.exists(ready) or not os.path.exists(archive):
        return 0
    with open(ready) as f:
        version = f.read().strip()
    os.remove(ready)
    stage = os.path.join(work, 'stage')
    shutil.rmtree(stage, ignore_errors=True)
    staged = os.path.join(stage, GAME)
    try:
        with ZipFile(archive) as zipped:
            check_zip(zipped)
            need = sum(e.file_size for e in zipped.infolist()) + folder_size(game) + (32 << 20)
            if shutil.disk_usage(work).free < need:
                raise ValueError('not enough free space for the update')
            shutil.copytree(game, staged, symlinks=True)
            unpack(zipped, stage)
        if os.path.getsize(os.path.join(staged, BINARY)) <= 0:
            raise ValueError('the new game is empty')
        swap(game, launcher, work, stage, staged)
    except Exception:
        recover(game, launcher)
        # (the download too: the next look offers it again)
        shutil.rmtree(stage, ignore_errors=True)
        os.remove(archive)
        raise
    shutil.rmtree(work, ignore_errors=True)
    sync()
    print(f'update: {version} in place', flush=True)
    return DONE


def swap(game, launcher, work, stage, staged):
    """The old folder and launcher aside and the new ones in, under the journal."""
    previous = os.path.join(work, 'previous')
    shutil.rmtree(previous, ignore_errors=True)
    shutil.copy2(launcher, os.path.join(work, 'previous-launcher.sh'))
    shutil.copy2(os.path.abspath(__file__), os.path.join(work, 'recovery.py'))
    write_json(os.path.join(work, 'transaction.json'), {'game': game})
    sync()
    os.replace(game, previous)
    os.replace(staged, game)
    os.replace(os.path.join(stage, LAUNCHER), launcher)
    sync()
    os.remove(os.path.join(work, 'transaction.json'))


def folder_size(path):
    return sum(os.path.getsize(os.path.join(base, n)) for base, _, names in os.walk(path) for n in names
               if not os.path.islink(os.path.join(base, n)))


def main():
    parser = argparse.ArgumentParser(description='Put the downloaded update of the PortMaster port in place.')
    parser.add_argument('command', choices=('apply', 'recover'))
    parser.add_argument('--game-dir', required=True)
    parser.add_argument('--launcher', required=True)
    args = parser.parse_args()
    game, launcher = os.path.abspath(args.game_dir), os.path.abspath(args.launcher)
    try:
        if args.command == 'recover':
            recover(game, launcher)
            return 0
        return apply(game, launcher)
    except (OSError, ValueError, BadZipFile) as error:
        print(f'update failed: {error}', file=sys.stderr, flush=True)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
