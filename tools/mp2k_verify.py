#!/usr/bin/env python3
"""mp2k_verify.py [SONG...] [--frames N]: the engine's MP2K (src/audio/mp2k*.c)
against BN6's own, song by song (docs/engine/emulation/engine-sound.md); and
another game's song copied into BN6 (src/audio/xsong.c) against that game
playing it (xN/SONG: x0 BN5, x1 BN4).

For each song (hex; the title's 0x01, the credits' 0x1D, the net's 0x13 and
the menus' sounds by default): the plain ROM in romlab (tools/romlab, mGBA
0.10.5) with the song table's entry 1 pointed at the song, so BN6's title
starts it at its frame 294, its state and sound recorded each frame
(romlab's `sound`); the engine's --mp2k-trace of the song from the same
phases; tools/mp2k_diff.py between them.

For xN/SONG (xN/all: every song of its music; a sound effect by its
number, x1/9D, played on the music player in both games, its own player
the differential's to check): the song's own game in
romlab, its title's entry pointed at the song, recorded from its SoundInfo
and music player; BN6 in romlab with xsong's copy of the song
(--xsong-pack) written where the copies go, its title's entry pointed at
the copy, given the other game's phases the frame before; mp2k_diff.py
between them, the other game's pointers moved where the copy has them
(its RAM's places to BN6's, each piece of the song to its copy, the map
walked through both: docs/devtools/sound-check.md).

Needs the ROMs (the build's ROM folder, ~/.cache/mmbn-ref/roms) and the
romlab image; writes .build/mp2k/. Exits 1 where any song differs."""
import json
import os
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROMS = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
BUS = 0x08000000
SONG_TABLE = 0x08159F48       # RomLayout.song_table (docs/rom/)
START, C15, COUNTER = 294, 3, 7   # the title's song's first engine frame, and the phases the frame before
DEFAULT = [0x01, 0x1D, 0x13, 0x66, 0x67, 0x68, 0x81]
FREE_SONGS = 0x089A0000       # src/emu/free_space.h: where xsong's copies go
BN6_SI, BN6_PLAYERS = 0x030045C0, 0x08159DC8   # BN6_SOUND_INFO, the player table (docs/rom/sound.md)
# The other games (XRomId order): game code, song table, player table,
# SoundInfo, the title's song entry and its first engine frame (romlab from
# power-on), their music (docs/rom/bn5-world.md, docs/rom/bn4.md)
GAMES = {
    0: dict(code=b'BRKE', songs=0x08155BF4, players=0x08155A74, si=0x030045C0, title=1, start=198, music=range(0x01, 0x25)),
    1: dict(code=b'B4WE', songs=0x08155FF8, players=0x08155E78, si=0x030036A0, title=0, start=198, music=range(0x00, 0x22)),
}


def u32(data, addr):
    return struct.unpack_from('<I', data, addr - BUS)[0]


def rom(code=b'BR5E'):
    for name in sorted(os.listdir(ROMS)):
        path = os.path.join(ROMS, name)
        if name.endswith('.gba') and os.path.getsize(path) == 0x800000:
            with open(path, 'rb') as f:
                data = f.read()
            if data[0xAC:0xB0] == code:
                return name, data
    sys.exit(f'mp2k_verify: no {code.decode()} ROM in ' + ROMS)


def romlab(rom_name, script, tag):
    # (named as build.py names its containers where CYBERWORLD_DOCKER_NAME says)
    name = f"{os.environ.get('CYBERWORLD_DOCKER_NAME', 'cw-mp2k')}-{tag}-{os.getpid()}"
    r = subprocess.run(['docker', 'run', '--rm', '-i', '--name', name, '-v', f'{ROOT}:/src',
                        '-v', f'{ROMS}:/roms:ro', '-w', '/src', 'cyberworld-romlab', 'tools/romlab/romlab.bin',
                        f'/roms/{rom_name}'], input=script.encode(), check=True, capture_output=True)
    return r.stdout.decode()


def oracle(rom_name, data, song, frames, out):
    entry = data[SONG_TABLE - BUS + 8 * song:SONG_TABLE - BUS + 8 * song + 8]
    romlab(rom_name, f'pokes {SONG_TABLE + 8:08X} {entry.hex()}\nsound {START + frames} /src/{out}\n', f'oracle-{song:x}')


def ours(song, frames, out):
    subprocess.run([sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--mp2k-trace',
                    f'{song}:{frames}:/src/{out}:{C15}:{COUNTER}', '--data-dir', '/src/.build/mp2k/data'],
                   check=True, capture_output=True)


def diff(game, mine, start, frames, *more):
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'tools/mp2k_diff.py'), os.path.join(ROOT, game),
                        os.path.join(ROOT, mine), str(start), str(frames), *more], capture_output=True, text=True)
    return r.returncode, r.stdout.strip()


# ---- another game's song, copied into BN6 ----

def pack(x, song, out):
    """xsong's copy of the song (--xsong-pack), its bytes in `out`: its header's address"""
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--xsong-pack', f'x{x}/{song:#x}:/src/{out}',
                        '--data-dir', '/src/.build/mp2k/data'], capture_output=True, text=True)
    for line in r.stdout.splitlines():
        if line.startswith('xsong-pack:') and ' header ' in line:
            return int(line.split(' header ')[1].split(',')[0], 16)
    return 0


def copy_map(orig, copy, header, header2):
    """(from, length, to): each piece of the song in its own ROM and where
    xsong's copy has it, walked through both side by side (the copy laid
    out from FREE_SONGS); and what differs between the pieces' bytes"""
    def c8(a):
        return copy[a - FREE_SONGS] if FREE_SONGS <= a < FREE_SONGS + len(copy) else None

    def c32(a):
        return struct.unpack_from('<I', copy, a - FREE_SONGS)[0]

    bad = []

    def same(a, a2, n, what):
        if orig[a - BUS:a - BUS + n] != copy[a2 - FREE_SONGS:a2 - FREE_SONGS + n]:
            bad.append(f'{what} {a:08x} -> {a2:08x}')

    nt = orig[header - BUS]
    tracks = [u32(orig, header + 8 + 4 * t) for t in range(nt)]
    lo, hi = min(tracks), header + 8 + 4 * nt
    at = lo + header2 - header
    m = [(lo, hi - lo, at)]
    group, group2 = u32(orig, header + 4), c32(header2 + 4)
    m.append((group, at - group2, group2))

    def voice(v, v2, sub):
        kind, p, p2 = orig[v - BUS], u32(orig, v + 4), c32(v2 + 4)
        if not any(copy[v2 - FREE_SONGS:v2 - FREE_SONGS + 12]) or (kind & 0xC0 and sub) or not p2:
            return
        if kind & 0x80:
            m.append((p, 128 * 12, p2))
            for k in range(128):
                if c8(p2 + 12 * k) != 0x80 or c32(p2 + 12 * k + 4):
                    voice(p + 12 * k, p2 + 12 * k, True)
        elif kind & 0x40:
            q, q2 = u32(orig, v + 8), c32(v2 + 8)
            n = max(orig[q - BUS:q - BUS + 128]) + 1
            m.append((q, 128, q2))
            same(q, q2, 128, 'key table')
            m.append((p, 12 * n, p2))
            for k in range(n):
                voice(p + 12 * k, p2 + 12 * k, True)
        elif kind & 7 == 0:
            n = 16 + u32(orig, p + 12) + 1
            m.append((p, n, p2))
            same(p, p2, n, 'sample')
        elif kind & 7 == 3:
            m.append((p, 16, p2))
            same(p, p2, 16, 'wave')

    for k in range((at - group2) // 12):
        voice(group + 12 * k, group2 + 12 * k, False)
    return m, bad


def verify_copy(x, song, frames):
    g = GAMES[x]
    name, data = rom(g['code'])
    bn6_name, _ = rom()
    base = f'.build/mp2k/check/x{x}_{song:03x}'
    entry = data[g['songs'] - BUS + 8 * song:g['songs'] - BUS + 8 * song + 8]
    header = struct.unpack_from('<I', entry)[0]
    # (played on the music player in both games, whose state romlab's sound records: a sound effect's entry names
    # another player, which the differential checks it is begun on; its music's name 31 already)
    entry = struct.pack('<IHH', header, 31, 31)
    if not BUS <= header < BUS + len(data) or not data[header - BUS]:
        return None, 'no song'
    header2 = pack(x, song, base + '.pack')
    if not header2:
        return 1, 'FAIL: xsong does not copy it'
    with open(os.path.join(ROOT, base + '.pack'), 'rb') as f:
        copy = f.read()
    m, bad = copy_map(data, copy, header, header2)
    # (the music player and its tracks, the channels, where BN6 keeps them)
    plr, trk = u32(data, g['players'] + 31 * 12), u32(data, g['players'] + 31 * 12 + 4)
    plr6, trk6 = 0x02010890, 0x02007EE8
    romlab(name, f'pokes {g["songs"] + 8 * g["title"]:08X} {entry.hex()}\n'
                 f'sound {g["start"] + frames} /src/{base}_game {g["si"]:08X} {g["players"]:08X}\n', f'x{x}-{song:x}')
    rec = open(os.path.join(ROOT, base + '_game.si'), 'rb').read()
    size = 0xFB0 + 0x100 + 0x40 + 0x280
    before = rec[(g['start'] - 1) * size:g['start'] * size]
    cgb = struct.unpack_from('<I', before, 0x1C)[0]
    m += [(g['si'], 0xFB0, BN6_SI), (cgb, 0x100, 0x03005600), (plr, 0x40, plr6), (trk, 0x50 * 8, trk6)]
    with open(os.path.join(ROOT, base + '.map'), 'w') as f:
        json.dump(m, f)
    # (BN6 given the other game's phases the frame before, its PSG's
    # 15-frame count and its ring's, SoundInfo +0x0A and +0x04, and its
    # master volume, +0x07: BN4's m4aSoundInit sets 12, BN6's 15)
    entry6 = struct.pack('<IHH', header2, 31, 31)
    romlab(bn6_name, f'pokef {FREE_SONGS:08X} /src/{base}.pack\npokes {SONG_TABLE + 8:08X} {entry6.hex()}\nrun {START}\n'
                     f'poke8 {BN6_SI + 0x0A:08X} {before[0x0A]:02X}\npoke8 {BN6_SI + 4:08X} {before[4]:02X}\n'
                     f'poke8 {BN6_SI + 7:08X} {before[7]:02X}\nsound {frames} /src/{base}_ours\n', f'x{x}-{song:x}-bn6')
    code, out = diff(base + '_game', base + '_ours', g['start'], frames, '--map', os.path.join(ROOT, base + '.map'))
    if bad:
        code, out = 1, out + '\nFAIL the copy\'s bytes: ' + '; '.join(bad[:4])
    return code, f'{len(copy)} bytes copied, its game\'s master volume {before[7]}\n' + out


def main():
    args = sys.argv[1:]
    frames = 900
    if '--frames' in args:
        i = args.index('--frames')
        frames = int(args[i + 1])
        del args[i:i + 2]
    os.makedirs(os.path.join(ROOT, '.build/mp2k/check'), exist_ok=True)
    failed = []
    jobs = []
    for a in args or [f'{s:x}' for s in DEFAULT]:
        if a.startswith('x'):
            x, s = int(a[1:a.index('/')]), a[a.index('/') + 1:]
            jobs += [(x, n) for n in (GAMES[x]['music'] if s == 'all' else [int(s, 16)])]
        else:
            jobs.append((None, int(a, 16)))
    rom_name, data = rom()
    for x, song in jobs:
        if x is None:
            base = f'.build/mp2k/check/s{song:03x}'
            oracle(rom_name, data, song, frames, base + '_game')
            ours(song, frames, base + '_ours')
            code, out = diff(base + '_game', base + '_ours', START, frames)
            name = f'song {song:#04x}'
        else:
            code, out = verify_copy(x, song, frames)
            name = f'x{x}/{song:#04x}'
            if code is None:
                print(f'{name}: {out}')
                continue
        print(f'{name}: ' + out.replace('\n', '\n  '))
        if code:
            failed.append(name)
    print('PASS' if not failed else 'FAIL: ' + ', '.join(failed))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
