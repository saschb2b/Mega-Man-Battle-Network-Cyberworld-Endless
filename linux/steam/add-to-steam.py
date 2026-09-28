#!/usr/bin/env python3
"""Adds Cyberworld Endless to Steam as a non-Steam game, with its library
artwork (the capsules, the hero, the logo and the icon beside this script),
or brings an entry made before up to date: one that Steam's own "Add a
Non-Steam Game", the desktop's "Add to Steam" or an earlier run made keeps
its place and play time and gets the artwork.

Steam keeps its shortcuts in userdata/<account>/config/shortcuts.vdf, reads
the file when it starts and writes it back when it quits, so a running Steam
is closed first (after asking) and started again afterwards. The artwork
goes into the same account's config/grid folder under the shortcut's ID.
shortcuts.vdf is backed up beside itself before it is written.

    python3 add-to-steam.py            the install this script came with
    python3 add-to-steam.py --remove   the entry and its artwork taken away
    python3 add-to-steam.py --check    exit 0 when the entry is there, 1 when
                                       not, 2 without a Steam to add it to

--exe PATH adds that program or AppImage, --flatpak the Flatpak; --yes
closes and restarts Steam without asking; --dry-run only says what it would
do. Python 3.6 or newer, nothing else.
"""
import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import time
import zlib

APP_ID = 'io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless'
OLD_APP_ID = 'io.github.saschb2b.CyberworldEndless'
NAME = 'Cyberworld Endless'
HERE = os.path.dirname(os.path.realpath(__file__))
# the artwork beside this script -> its name in Steam's grid folder
ART = (('capsule.png', '{}p.png'), ('wide.png', '{}.png'), ('hero.png', '{}_hero.png'),
       ('logo.png', '{}_logo.png'), ('icon.png', '{}_icon.png'))
# where the logo sits on the hero: its lower left, half the hero's width
LOGO_POSITION = {'nVersion': 1, 'logoPosition': {'pinnedPosition': 'BottomLeft', 'nWidthPct': 50, 'nHeightPct': 50}}


# ---- Steam's binary VDF (shortcuts.vdf) ----

class Raw:
    """A value of a type this script leaves as it found it."""

    def __init__(self, kind, data):
        self.kind, self.data = kind, data


def vdf_load(data):
    """The file as nested dicts: maps, strings (type 1) and 32-bit ints
    (type 2); other types are kept as Raw."""
    pos = 0

    def string():
        nonlocal pos
        end = data.index(b'\0', pos)
        s = data[pos:end].decode('utf-8', 'surrogateescape')
        pos = end + 1
        return s

    def fields():
        nonlocal pos
        out = {}
        while True:
            kind = data[pos]
            pos += 1
            if kind == 0x08:
                return out
            key = string()
            if kind == 0x00:
                out[key] = fields()
            elif kind == 0x01:
                out[key] = string()
            elif kind == 0x02:
                out[key] = struct.unpack_from('<i', data, pos)[0]
                pos += 4
            elif kind in (0x03, 0x04, 0x06):     # float, pointer, colour: 4 bytes
                out[key] = Raw(kind, data[pos:pos + 4])
                pos += 4
            elif kind in (0x07, 0x0A):           # 64-bit ints
                out[key] = Raw(kind, data[pos:pos + 8])
                pos += 8
            else:
                raise ValueError(f'unknown field type {kind:#x} at byte {pos - 1}')

    root = fields()
    return root


def vdf_dump(root):
    out = bytearray()

    def fields(m):
        for key, value in m.items():
            name = key.encode('utf-8', 'surrogateescape') + b'\0'
            if isinstance(value, dict):
                out.extend(b'\x00' + name)
                fields(value)
            elif isinstance(value, str):
                out.extend(b'\x01' + name + value.encode('utf-8', 'surrogateescape') + b'\0')
            elif isinstance(value, Raw):
                out.extend(bytes([value.kind]) + name + value.data)
            else:
                out.extend(b'\x02' + name + struct.pack('<i', value))
        out.extend(b'\x08')

    fields(root)
    return bytes(out)


# ---- where Steam is ----

def steam_roots():
    """Steam's folders that hold accounts (userdata), each once."""
    home = os.path.expanduser('~')
    seen, roots = set(), []
    for path in ('.local/share/Steam', '.steam/steam', '.steam/root'):
        root = os.path.join(home, path)
        if os.path.isdir(os.path.join(root, 'userdata')) and os.path.realpath(root) not in seen:
            seen.add(os.path.realpath(root))
            roots.append(root)
    return roots


def flathub_steam():
    return os.path.isdir(os.path.expanduser('~/.var/app/com.valvesoftware.Steam/.local/share/Steam/userdata'))


def accounts(root):
    """The account last signed in (loginusers.vdf's MostRecent), or every
    account there when none is marked."""
    userdata = os.path.join(root, 'userdata')
    ids = sorted(d for d in os.listdir(userdata) if d.isdigit() and d != '0')
    try:
        with open(os.path.join(root, 'config', 'loginusers.vdf'), encoding='utf-8', errors='replace') as f:
            text = f.read()
        for steam64, body in re.findall(r'"(\d{17})"\s*\{([^{}]*)\}', text):
            if re.search(r'"MostRecent"\s*"1"', body):
                account = str(int(steam64) - 76561197960265728)
                if account in ids:
                    return [account]
    except OSError:
        pass
    return ids


def steam_running():
    """Whether a Steam of this user runs."""
    for pid in os.listdir('/proc'):
        if not pid.isdigit():
            continue
        try:
            with open(f'/proc/{pid}/comm') as f:
                if f.read().strip() == 'steam' and os.stat(f'/proc/{pid}').st_uid == os.getuid():
                    return True
        except OSError:
            pass
    return False


# ---- what to start ----

class Target:
    def __init__(self, exe, start, options='', flatpak=''):
        self.exe, self.start, self.options, self.flatpak = f'"{exe}"', f'"{start}"', options, flatpak

    def describe(self):
        return f'{self.exe} {self.options}'.strip()


def flatpak_target():
    flatpak = shutil.which('flatpak') or '/usr/bin/flatpak'
    return Target(flatpak, os.path.dirname(flatpak) + '/', f'run {APP_ID}', APP_ID)


def flatpak_installed():
    try:
        return subprocess.run(['flatpak', 'info', APP_ID], stdout=subprocess.DEVNULL,
                              stderr=subprocess.DEVNULL).returncode == 0
    except OSError:
        return False


def find_target(args):
    if args.exe:
        exe = os.path.abspath(args.exe)
        if not os.path.isfile(exe):
            sys.exit(f'{exe}: no such file')
        return Target(exe, os.path.dirname(exe) + '/')
    if args.flatpak or f'/{APP_ID}/' in HERE:
        return flatpak_target()
    # the tar.gz: this script in steam/ beside the program
    beside = os.path.join(os.path.dirname(HERE), 'cyberworld-endless')
    if os.path.isfile(beside) and os.access(beside, os.X_OK):
        return Target(beside, os.path.dirname(beside) + '/')
    # the .deb: /usr/share/cyberworld-endless/steam
    if HERE.startswith('/usr/share/') and os.path.exists('/usr/bin/cyberworld-endless'):
        return Target('/usr/bin/cyberworld-endless', '/usr/bin/')
    if flatpak_installed():
        return flatpak_target()
    sys.exit('Which install should Steam start? Pass --exe PATH (the AppImage or the program) or --flatpak.')


def art_file(name):
    path = os.path.join(HERE, name)
    if name == 'icon.png' and not os.path.exists(path):
        path = os.path.join(HERE, '..', 'icons', '256.png')   # (in the source tree)
    return path if os.path.exists(path) else None


# ---- the entry ----

def ours(entry, target):
    get = lambda k: next((v for key, v in entry.items() if key.lower() == k.lower() and isinstance(v, str)), '')
    exe, options, name = get('Exe'), get('LaunchOptions'), get('AppName')
    return (exe == target.exe or name == NAME or get('FlatpakAppID') in (APP_ID, OLD_APP_ID)
            or any(i in options or i in exe for i in (APP_ID, OLD_APP_ID))
            or 'cyberworld-endless' in os.path.basename(exe.strip('"')).lower())


def shortcut_id(target):
    """A new shortcut's ID as Steam makes one: the CRC-32 of its program and
    name, with the top bit set."""
    return zlib.crc32((target.exe + NAME).encode('utf-8')) | 0x80000000


def as_unsigned(v):
    return v & 0xFFFFFFFF


def as_signed(v):
    return v - (1 << 32) if v >= 1 << 31 else v


def new_entry(target, appid, icon):
    return {
        'appid': as_signed(appid), 'AppName': NAME, 'Exe': target.exe, 'StartDir': target.start, 'icon': icon,
        'ShortcutPath': '', 'LaunchOptions': target.options, 'IsHidden': 0, 'AllowDesktopConfig': 1,
        'AllowOverlay': 1, 'OpenVR': 0, 'Devkit': 0, 'DevkitGameID': '', 'DevkitOverrideAppID': 0,
        'LastPlayTime': 0, 'FlatpakAppID': target.flatpak, 'tags': {},
    }


def set_field(entry, key, value):
    """Sets a field whatever case Steam wrote its name in."""
    for k in entry:
        if k.lower() == key.lower():
            entry[k] = value
            return
    entry[key] = value


def field(entry, key, default=None):
    return next((v for k, v in entry.items() if k.lower() == key.lower()), default)


def load_shortcuts(path):
    """The file's root and its shortcuts map."""
    if not os.path.exists(path):
        root = {'shortcuts': {}}
    else:
        with open(path, 'rb') as f:
            root = vdf_load(f.read())
    key = next((k for k in root if k.lower() == 'shortcuts' and isinstance(root[k], dict)), None)
    if key is None:
        key = 'shortcuts'
        root[key] = {}
    return root, root[key]


def save_shortcuts(path, root, dry):
    data = vdf_dump(root)
    if vdf_dump(vdf_load(data)) != data:
        sys.exit('refusing to write: the new shortcuts.vdf does not read back the same')
    if dry:
        return
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if os.path.exists(path):
        shutil.copy2(path, path + '.bak')
    tmp = path + '.tmp'
    with open(tmp, 'wb') as f:
        f.write(data)
    os.replace(tmp, path)


def renumber(shortcuts):
    """Steam numbers its shortcuts 0, 1, 2... with no gaps."""
    items = list(shortcuts.values())
    shortcuts.clear()
    for i, entry in enumerate(items):
        shortcuts[str(i)] = entry


def add(account_dir, target, dry, say):
    path = os.path.join(account_dir, 'config', 'shortcuts.vdf')
    grid = os.path.join(account_dir, 'config', 'grid')
    root, shortcuts = load_shortcuts(path)
    found = [e for e in shortcuts.values() if isinstance(e, dict) and ours(e, target)]
    if found:
        entry = found[0]
        appid = field(entry, 'appid')
        if not isinstance(appid, int):
            appid = as_signed(shortcut_id(target))
            set_field(entry, 'appid', appid)
        appid = as_unsigned(appid)
        say(f'updating "{field(entry, "AppName", NAME)}" ({appid}) to start {target.describe()}')
        for key, value in (('AppName', NAME), ('Exe', target.exe), ('StartDir', target.start),
                           ('LaunchOptions', target.options), ('FlatpakAppID', target.flatpak)):
            set_field(entry, key, value)
        if len(found) > 1:
            say(f'({len(found) - 1} more entries for the game left as they are)')
    else:
        appid = shortcut_id(target)
        say(f'adding "{NAME}" ({appid}) to start {target.describe()}')
        entry = new_entry(target, appid, '')
        renumber(shortcuts)
        shortcuts[str(len(shortcuts))] = entry
    # the artwork, and the icon in the entry
    icon = ''
    for name, pattern in ART:
        src = art_file(name)
        if not src:
            say(f'(no {name} beside the script: that picture is left out)')
            continue
        dst = os.path.join(grid, pattern.format(appid))
        if name == 'icon.png':
            icon = dst
        if not dry:
            os.makedirs(grid, exist_ok=True)
            shutil.copyfile(src, dst)
    if not dry:
        with open(os.path.join(grid, f'{appid}.json'), 'w') as f:
            json.dump(LOGO_POSITION, f)
    if icon:
        set_field(entry, 'icon', icon)
    save_shortcuts(path, root, dry)
    return appid


def remove(account_dir, target, dry, say):
    path = os.path.join(account_dir, 'config', 'shortcuts.vdf')
    grid = os.path.join(account_dir, 'config', 'grid')
    if not os.path.exists(path):
        return 0
    root, shortcuts = load_shortcuts(path)
    gone = [k for k, e in shortcuts.items() if isinstance(e, dict) and ours(e, target)]
    for k in gone:
        appid = field(shortcuts[k], 'appid')
        say(f'removing "{field(shortcuts[k], "AppName", NAME)}"')
        if isinstance(appid, int) and not dry:
            appid = as_unsigned(appid)
            for _, pattern in ART:
                p = os.path.join(grid, pattern.format(appid))
                if os.path.exists(p):
                    os.remove(p)
            if os.path.exists(os.path.join(grid, f'{appid}.json')):
                os.remove(os.path.join(grid, f'{appid}.json'))
        del shortcuts[k]
    if gone:
        renumber(shortcuts)
        save_shortcuts(path, root, dry)
    return len(gone)


def present(account_dir, target):
    path = os.path.join(account_dir, 'config', 'shortcuts.vdf')
    if not os.path.exists(path):
        return False
    return any(isinstance(e, dict) and ours(e, target) for e in load_shortcuts(path)[1].values())


# ---- closing and starting Steam ----

def ask(question, yes):
    if yes:
        return True
    if not sys.stdin.isatty():
        return False
    try:
        return input(f'{question} [Y/n] ').strip().lower() in ('', 'y', 'yes')
    except EOFError:
        return False


def close_steam(yes):
    if not steam_running():
        return False
    if not ask('Steam is running and would write over the new entry when it quits. Close Steam now?', yes):
        sys.exit('Close Steam and run this again (or pass --yes).')
    print('closing Steam...')
    try:
        subprocess.run(['steam', '-shutdown'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=30)
    except (OSError, subprocess.TimeoutExpired):
        pass
    for _ in range(120):
        if not steam_running():
            time.sleep(1)   # (its last writes)
            return True
        time.sleep(0.5)
    sys.exit('Steam did not close; quit it from its menu and run this again.')


def start_steam():
    try:
        subprocess.Popen(['steam'], stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                         stderr=subprocess.DEVNULL, start_new_session=True)
        print('starting Steam again.')
    except OSError:
        print('Start Steam again to see it.')


def main():
    p = argparse.ArgumentParser(description=f'Add {NAME} to Steam with its library artwork.')
    p.add_argument('--exe', help='the program or AppImage Steam should start')
    p.add_argument('--flatpak', action='store_true', help='start the Flatpak')
    p.add_argument('--remove', action='store_true', help='take the entry and its artwork away')
    p.add_argument('--check', action='store_true', help='exit 0 when the entry is there, 1 when not, 2 without Steam')
    p.add_argument('--yes', action='store_true', help='close and restart Steam without asking')
    p.add_argument('--dry-run', action='store_true', help='only say what would be done')
    args = p.parse_args()

    roots = steam_roots()
    if not roots:
        if args.check:
            sys.exit(2)
        if flathub_steam():
            sys.exit('Steam from Flathub runs in its own sandbox and cannot start the game; '
                     'add it there with "Add a Non-Steam Game" instead.')
        sys.exit('No Steam found (~/.local/share/Steam): install Steam and sign in once first.')
    target = find_target(args)
    dirs = [os.path.join(r, 'userdata', a) for r in roots for a in accounts(r)]
    if not dirs:
        if args.check:
            sys.exit(2)
        sys.exit('Steam has no account here yet: sign in once, then run this again.')
    if args.check:
        sys.exit(0 if all(present(d, target) for d in dirs) else 1)

    say = print
    if args.dry_run:
        say = lambda s: print('(dry run) ' + s)
    was_running = False if args.dry_run else close_steam(args.yes)
    for d in dirs:
        if args.remove:
            if not remove(d, target, args.dry_run, say):
                say(f'no entry for {NAME} in {d}')
        else:
            add(d, target, args.dry_run, say)
    if not args.dry_run:
        say('done.' if args.remove else
            f'done: {NAME} is in your Steam library with its artwork (on a Steam Deck, return to Gaming Mode).')
    if was_running:
        start_steam()


if __name__ == '__main__':
    main()
