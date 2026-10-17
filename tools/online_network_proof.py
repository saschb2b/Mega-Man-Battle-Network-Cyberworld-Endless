#!/usr/bin/env python3
"""Exercise the Network server menu in the real game, without connecting.

Build with build.py linux first; pass the folder containing your own ROM.
Fake identities, settings, logs and full-frame pictures stay in .build/.
Pillow converts the game's BMP captures to PNG without cropping or scaling.
"""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

from PIL import Image

from online_proof import ROOT
from online_social_proof import Script


OFFICIAL = 'wss://cyberworld.saschb2b.com/presence'
CUSTOM = ('ws://localhost:8765/presence', 'wss://two.example/presence',
          'wss://three.example/presence', 'wss://four.example/presence')
EDITED = 'ws://localhost:8766/presence'
KEYS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 _-.:/[]'


def read_prefs(directory):
    result = {'server': []}
    for line in (directory / 'network.ini').read_text().splitlines():
        if '=' not in line or line.startswith('#'):
            continue
        key, value = line.split('=', 1)
        if key == 'server':
            result[key].append(value)
        else:
            result[key] = value
    return result


def open_network():
    script = Script()
    script.wait(40)
    script.press('START')
    script.wait(60)
    script.press('DOWN')
    script.press('A')
    script.wait(20)
    # With no run, the title's second row is Network; its default is Join.
    script.press('UP')
    script.press('UP')
    return script


def tap(script, x, y):
    if not hasattr(script, 'taps'):
        script.taps = []
    script.taps.append(f'{script.frames + 1}:{x * 6},{y * 6}')
    script.wait(16)


def official_route(portrait):
    script = open_network()
    if portrait:
        tap(script, 117, 60)
    else:
        script.press('LEFT')
    script.shot('servers')
    if portrait:
        tap(script, 90, 74)
    else:
        script.press('A')
    script.shot('official-fixed')
    return script


def type_on_pad(script, text):
    row = col = 0
    for character in text:
        target_row, target_col = divmod(KEYS.index(character), 12)
        # Move horizontally on the full top row, avoiding unused end keys.
        for _ in range(row):
            script.press('UP')
        for _ in range(abs(target_col - col)):
            script.press('RIGHT' if target_col > col else 'LEFT')
        for _ in range(target_row):
            script.press('DOWN')
        script.press('A')
        row, col = target_row, target_col


def custom_route(portrait):
    script = open_network()
    if portrait:
        tap(script, 165, 60)
        tap(script, 90, 74)
    else:
        script.press('RIGHT')
        script.press('A')
    script.shot('custom-editor')
    for _ in range(10):
        if portrait:
            tap(script, 38, 141)
        else:
            script.press('L')
    if portrait:
        for character in '6/presence':
            row, col = divmod(KEYS.index(character), 12)
            tap(script, 12 + col * 13 + 6, 64 + row * 11 + 5)
        tap(script, 90, 141)
    else:
        type_on_pad(script, '6/presence')
        script.press('START')
    script.shot('custom-saved')
    return script


def capture(args, directory, size, label, script):
    pictures = ','.join(f'{frame}:{directory / (name + ".bmp")}'
                        for name, frame in script.shots.items())
    command = [str(args.binary.resolve()), '--headless', '--scene', 'title',
               '--rom-dir', str(args.rom_dir.resolve()), '--data-dir', str(directory),
               '--size', size, '--frames', str(script.frames + 1),
               '--input', ','.join(script.steps), '--screen-shot', pictures]
    if size == '1080x1920':
        command.extend(('--dpi', '420', '--taps', ';'.join(script.taps)))
    with (directory / f'{label}.log').open('w') as log:
        subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT,
                       check=True, timeout=60)
    for name in script.shots:
        with Image.open(directory / f'{name}.bmp') as picture:
            assert picture.size == tuple(map(int, size.split('x')))
            picture.save(directory / f'{name}.png', optimize=True)


def run(args, evidence):
    checks = {}
    for size in ('240x160', '1080x1920'):
        directory = evidence / size
        directory.mkdir()
        # An older client allowed all four slots to be custom. Consent was
        # given to slots 0, 1 and 3; adding Official must not inherit it.
        (directory / 'network.ini').write_text(
            'identity=0123456789abcdef0123456789abcdef\nname=GuestNavi\n'
            'selected=0\nconsent=11\n' + ''.join(f'server={url}\n' for url in CUSTOM))
        portrait = size == '1080x1920'
        capture(args, directory, size, 'official', official_route(portrait))
        prefs = read_prefs(directory)
        checks[size + '_official_first'] = prefs['server'] == [OFFICIAL, *CUSTOM]
        checks[size + '_migrated_consent'] = prefs['selected'] == '0' and prefs['consent'] == '22'
        capture(args, directory, size, 'custom', custom_route(portrait))
        prefs = read_prefs(directory)
        checks[size + '_custom_edit'] = prefs['server'] == [OFFICIAL, EDITED, *CUSTOM[1:]]
        checks[size + '_edited_consent_reset'] = prefs['selected'] == '1' and prefs['consent'] == '20'
        checks[size + '_no_checkpoint_created'] = not (directory / 'savedata/run.sav').exists()
    report = {'result': 'passed' if all(checks.values()) else 'failed', 'checks': checks,
              'evidence': str(evidence), 'screenshots_require_visual_review': True}
    (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return all(checks.values())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom-dir', required=True, type=Path)
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/linux/cyberworld')
    args = parser.parse_args()
    base = ROOT / '.build/online-network'
    base.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=base))
    raise SystemExit(0 if run(args, evidence) else 1)


if __name__ == '__main__':
    main()
