#!/usr/bin/env python3
"""mp2k_verify.py [SONG...] [--frames N]: the engine's MP2K (src/audio/mp2k*.c)
against BN6's own, song by song (docs/engine/emulation/engine-sound.md).

For each song (hex; the title's 0x01, the credits' 0x1D, the net's 0x13 and
the menus' sounds by default): the plain ROM in romlab (tools/romlab, mGBA
0.10.5) with the song table's entry 1 pointed at the song, so BN6's title
starts it at its frame 294, its state and sound recorded each frame
(romlab's `sound`); the engine's --mp2k-trace of the song from the same
phases; tools/mp2k_diff.py between them. Needs the ROM (the build's ROM
folder, ~/.cache/mmbn-ref/roms) and the romlab image; writes .build/mp2k/.
Exits 1 where any song differs."""
import os
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROMS = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
SONG_TABLE = 0x08159F48       # RomLayout.song_table (docs/rom/)
START, C15, COUNTER = 294, 3, 7   # the title's song's first engine frame, and the phases the frame before
DEFAULT = [0x01, 0x1D, 0x13, 0x66, 0x67, 0x68, 0x81]


def rom():
    for name in sorted(os.listdir(ROMS)):
        path = os.path.join(ROMS, name)
        if name.endswith('.gba') and os.path.getsize(path) == 0x800000:
            with open(path, 'rb') as f:
                data = f.read()
            if data[0xAC:0xB0] == b'BR5E':
                return name, data
    sys.exit('mp2k_verify: no BN6 Gregar ROM in ' + ROMS)


def oracle(rom_name, data, song, frames, out):
    entry = data[SONG_TABLE - 0x08000000 + 8 * song:SONG_TABLE - 0x08000000 + 8 * song + 8]
    script = f'pokes {SONG_TABLE + 8:08X} {entry.hex()}\nsound {START + frames} /src/{out}\n'
    subprocess.run(['docker', 'run', '--rm', '-i', '--name', f'cw-mp2k-oracle-{song:x}', '-v', f'{ROOT}:/src',
                    '-v', f'{ROMS}:/roms:ro', '-w', '/src', 'cyberworld-romlab', 'tools/romlab/romlab.bin',
                    f'/roms/{rom_name}'], input=script.encode(), check=True, capture_output=True)


def ours(song, frames, out):
    subprocess.run([sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--mp2k-trace',
                    f'{song}:{frames}:/src/{out}:{C15}:{COUNTER}', '--data-dir', '/src/.build/mp2k/data'],
                   check=True, capture_output=True)


def main():
    args = sys.argv[1:]
    frames = 900
    if '--frames' in args:
        i = args.index('--frames')
        frames = int(args[i + 1])
        del args[i:i + 2]
    songs = [int(a, 16) for a in args] or DEFAULT
    rom_name, data = rom()
    os.makedirs(os.path.join(ROOT, '.build/mp2k/check'), exist_ok=True)
    failed = []
    for song in songs:
        base = f'.build/mp2k/check/s{song:03x}'
        oracle(rom_name, data, song, frames, base + '_game')
        ours(song, frames, base + '_ours')
        r = subprocess.run([sys.executable, os.path.join(ROOT, 'tools/mp2k_diff.py'), os.path.join(ROOT, base + '_game'),
                            os.path.join(ROOT, base + '_ours'), str(START), str(frames)], capture_output=True, text=True)
        print(f'song {song:#04x}: ' + r.stdout.strip().replace('\n', '\n  '))
        if r.returncode:
            failed.append(song)
    print('PASS' if not failed else 'FAIL: ' + ', '.join(f'{s:#04x}' for s in failed))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
