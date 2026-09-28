#!/usr/bin/env python3
"""linux/steam/add-to-steam.py against a made-up Steam folder: the binary
VDF read and written back byte for byte, a new entry with its artwork, a run
again that updates it, an entry Steam's own "Add to Steam" made that keeps
its ID, play time and tags, --check and --remove; and the artwork's sizes.
Standard library only; build.py test runs it after the C tests."""
import importlib.util
import os
import struct
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(ROOT, 'linux', 'steam', 'add-to-steam.py')
spec = importlib.util.spec_from_file_location('add_to_steam', SCRIPT)
ats = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ats)

failed = 0


def check(ok, what):
    global failed
    if not ok:
        failed += 1
        print('FAIL:', what)


def png_size(path):
    with open(path, 'rb') as f:
        head = f.read(24)
    return struct.unpack('>II', head[16:24])


def run(home, *args):
    env = dict(os.environ, HOME=home, PATH='/usr/bin:/bin')
    return subprocess.run([sys.executable, SCRIPT, *args], env=env, capture_output=True, text=True)


def entries(path):
    with open(path, 'rb') as f:
        return list(ats.vdf_load(f.read())['shortcuts'].values())


# the documented layout, by hand
hand = (b'\x00shortcuts\x00' + b'\x000\x00' + b'\x02appid\x00' + struct.pack('<i', -123456789)
        + b'\x01AppName\x00Some Emulator\x00' + b'\x01Exe\x00"/opt/emu/emu"\x00'
        + b'\x00tags\x00' + b'\x010\x00favorite\x00' + b'\x08' + b'\x08' + b'\x08' + b'\x08')
check(ats.vdf_dump(ats.vdf_load(hand)) == hand, 'a hand-made shortcuts.vdf reads and writes back the same')

# the artwork, at Steam's sizes
for name, size in (('capsule.png', (600, 900)), ('wide.png', (920, 430)), ('hero.png', (1920, 620))):
    check(png_size(os.path.join(ROOT, 'linux', 'steam', name)) == size, f'{name} is {size[0]}x{size[1]}')
w, h = png_size(os.path.join(ROOT, 'linux', 'steam', 'logo.png'))
check(w <= 1280 and h <= 720, 'logo.png fits 1280x720')

with tempfile.TemporaryDirectory() as home:
    steam = os.path.join(home, '.local', 'share', 'Steam')
    config = os.path.join(steam, 'userdata', '12345678', 'config')
    os.makedirs(config)
    os.makedirs(os.path.join(steam, 'userdata', '99', 'config'))
    os.makedirs(os.path.join(steam, 'config'))
    with open(os.path.join(steam, 'config', 'loginusers.vdf'), 'w') as f:
        f.write('"users"\n{\n\t"76561197960265827"\n\t{\n\t\t"MostRecent"\t\t"0"\n\t}\n'
                '\t"76561197972611406"\n\t{\n\t\t"MostRecent"\t\t"1"\n\t}\n}\n')
    shortcuts = os.path.join(config, 'shortcuts.vdf')
    with open(shortcuts, 'wb') as f:
        f.write(hand)
    game = os.path.join(home, 'Cyberworld Endless.AppImage')
    open(game, 'w').close()
    os.chmod(game, 0o755)

    check(run(home, '--exe', game, '--check').returncode == 1, '--check says 1 before')
    r = run(home, '--exe', game, '--yes')
    check(r.returncode == 0, f'adds ({r.stdout}{r.stderr})')
    got = entries(shortcuts)
    check(len(got) == 2 and got[0]['AppName'] == 'Some Emulator', 'keeps the other entry')
    ours = got[-1]
    appid = ours['appid'] & 0xFFFFFFFF
    check(ours['AppName'] == 'Cyberworld Endless' and ours['Exe'] == f'"{game}"', 'the new entry starts the game')
    grid = os.path.join(config, 'grid')
    for pattern in ('{}p.png', '{}.png', '{}_hero.png', '{}_logo.png', '{}_icon.png', '{}.json'):
        check(os.path.exists(os.path.join(grid, pattern.format(appid))), f'writes {pattern}')
    check(ours['icon'] == os.path.join(grid, f'{appid}_icon.png'), 'the entry names the icon')
    check(not os.listdir(os.path.join(steam, 'userdata', '99', 'config')), 'leaves the account not signed in last')
    check(run(home, '--exe', game, '--check').returncode == 0, '--check says 0 after')
    run(home, '--exe', game, '--yes')
    check(len(entries(shortcuts)) == 2, 'a second run adds no second entry')

    # an entry as Steam's own Add to Steam makes one for the Flatpak
    made = {'shortcuts': {'0': {'appid': -1111111111, 'AppName': 'Cyberworld Endless', 'Exe': '"/usr/bin/flatpak"',
                                'LaunchOptions': 'run --branch=master --arch=x86_64 --command=cyberworld-endless '
                                                 'io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless',
                                'LastPlayTime': 1790000000, 'tags': {'0': 'favorite'}}}}
    with open(shortcuts, 'wb') as f:
        f.write(ats.vdf_dump(made))
    r = run(home, '--flatpak', '--yes')
    check(r.returncode == 0, f'updates a Steam-made entry ({r.stdout}{r.stderr})')
    got = entries(shortcuts)
    check(len(got) == 1 and got[0]['appid'] == -1111111111, 'keeps its ID')
    check(got[0]['LastPlayTime'] == 1790000000 and got[0]['tags'] == {'0': 'favorite'}, 'keeps its play time and tags')
    check(got[0]['LaunchOptions'].endswith('Mega-Man-Battle-Network-Cyberworld-Endless'), 'starts the Flatpak')
    check(os.path.exists(os.path.join(grid, f'{-1111111111 & 0xFFFFFFFF}_hero.png')), 'its artwork under its ID')

    r = run(home, '--flatpak', '--remove', '--yes')
    check(r.returncode == 0 and entries(shortcuts) == [], 'removes it')
    check(not any(n.startswith(str(-1111111111 & 0xFFFFFFFF)) for n in os.listdir(grid)), 'removes its artwork')

with tempfile.TemporaryDirectory() as home:
    check(run(home, '--flatpak', '--check').returncode == 2, '--check says 2 without Steam')

if failed:
    sys.exit(f'{failed} Steam checks failed')
print('all Steam checks passed')
