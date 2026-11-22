#!/usr/bin/env python3
"""xbracket.py [bn4 | bn6 | compare] [CASE ...] [--mutation NAME]: BN4 Red Sun's tournament bracket (main mode 0x24)
against its move into BN6, frame for frame (docs/devtools/bracket-diff.md; docs/multirom/bn4-tournament.md, Moved into
BN6).

The oracle is BN4's own screen in romlab, opened as BN4's scripts open it: from tools/bn4_lab.py's state in Lan's room,
the case's eight entrants poked into its tournament's record (BN4_TOURNAMENTS + t * BN4_TOURNAMENT_SIZE), the
bracket's three bytes (BN4_BRACKET +5 to +7) and the game state's +0 made BN4_GAMESTATE_TO_BRACKET, which fades the
map out, sets the screen up and enters the mode. Each frame is kept with the screen's state (BN4_BRACKET's first 16
bytes), and the frames are aligned on the screen's own clock: its first frame in state 4 (shown), then every frame
after it to the case's last.

  python3 tools/xbracket.py bn4 [CASE ...]      BN4's frames, .build/xbracket/bn4/CASE/ (each frame's PPM and the
                                                screen's state)
  python3 tools/xbracket.py bn6 [CASE ...]      BN6's: the host build's --dev bracket route (build.py linux first);
                                                .build/xbracket/bn6/CASE/
  python3 tools/xbracket.py compare [CASE ...]  each case's BN6 frames against BN4's, pixel for pixel on the shared
                                                clock: PASS (every pixel of every frame), FAIL (the first frame and the
                                                pixels that differ), INCONCLUSIVE (a side has no frames, or never shows)
  python3 tools/xbracket.py compare --mutation NAME   the same with BN6's side broken on purpose (MUTATIONS): must FAIL

Cases (CASES): t0-t3 each tournament's first round, MegaMan and seven of BN4's Navis in, no round won; settled The Den
Battle Tournament after its first round, four through and four out this round. Local only: BN4's ROM and the
cyberworld-romlab image (tools/romlab); nothing of the ROM's is written outside .build/."""
import argparse
import glob
import hashlib
import os
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROMS = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
OUT = os.path.join(ROOT, '.build', 'xbracket')
LAB = os.path.join(ROOT, '.build', 'bn4lab')
SHA1 = 'a97e96a7da03abd70f7953328e511b9fa29179f1'
# src/emu/bn4.h, src/emu/bn4_tournament.h
TOOLKIT_GAMESTATE = 0x40
TOURNAMENTS, TOURNAMENT_SIZE, BRACKET = 0x02001460, 0x20, 0x0200ACC0
GAMESTATE_TO_BRACKET, SHOWN = 0x1C, 4
ENTRANT_IN, ENTRANT_OUT, MEGAMAN = 0x40, 0x80, 0x1E
FRAMES = 240   # (frames recorded from the poke: the map's fade out, the set-up and some 200 frames of the screen shown)
STATE_BYTES = 0x10
BN6_STEP = 600   # (the frame of BN6's run the bracket step comes at: MegaMan on the first layer, src/dev/capture.c's steps)
XBR_STATE = 0x02029A00   # (src/emu/xbracket.h: BN4_BRACKET's place in BN6, RAM_BRACKET)

# (id, flags, seed) per entrant in bracket order; MegaMan first, as BN4's draw puts him
FIRST = [(MEGAMAN, ENTRANT_IN, 0), (0x00, ENTRANT_IN, 1), (0x03, ENTRANT_IN, 2), (0x04, ENTRANT_IN, 3),
         (0x09, ENTRANT_IN, 4), (0x11, ENTRANT_IN, 5), (0x01, ENTRANT_IN, 6), (0x02, ENTRANT_IN, 7)]
SETTLED = [(MEGAMAN, ENTRANT_IN | 1, 0), (0x00, ENTRANT_OUT, 1), (0x03, ENTRANT_IN | 1, 2), (0x04, ENTRANT_OUT, 3),
           (0x09, ENTRANT_OUT, 4), (0x11, ENTRANT_IN | 1, 5), (0x01, ENTRANT_IN | 1, 6), (0x02, ENTRANT_OUT, 7)]
CASES = {'t0': (0, FIRST), 't1': (1, FIRST), 't2': (2, FIRST), 't3': (3, FIRST), 'settled': (0, SETTLED)}
# BN6's side broken as a move would be: each must FAIL the compare (the comparator's own check)
ALLOWED = ('the first frame shown one colour on both sides: the backdrop of the map the screen was opened from (BN4\'s Lan\'s room, '
           'BN6\'s net layer), every layer off by BN4_MAP_LEAVE\'s DISPCNT (BN4) and its copy in the entry (BN6) until the screen\'s own '
           'palette reaches PRAM at the next VBlank (BN4\'s 0x03002A50, BN6\'s 0x03001960)')
# the move broken on purpose (CYBERWORLD_XBRACKET_MUTATE, src/emu/xbracket.c): BN6's side run with it must differ from BN4's more
# than the move does (its rows, or frames the move matches)
MOVE_MUTATIONS = {
    'tournaments': 'the shims of eToolkit +0x3C give the next tournament\'s record (a RAM word of the move placed wrong); not t3, '
                   'whose plates come from the Free Tournament\'s own list (0x020014F0), not these records',
}
MOVE_MUTATION_CASES = {'tournaments': ('t0', 't1', 't2', 'settled')}
MUTATIONS = {
    'palette': 'one colour of the screen\'s BG palette changed (a palette word copied wrong)',
    'plate': 'a plate drawn a pixel lower (an OAM attribute moved wrong)',
    'late': 'the screen shown a frame late (the clock off by one)',
}


def rom_name():
    for path in sorted(glob.glob(os.path.join(ROMS, '*.gba'))):
        with open(path, 'rb') as f:
            if hashlib.sha1(f.read()).hexdigest() == SHA1:
                return os.path.basename(path)
    sys.exit(f'xbracket: no BN4 Red Sun (USA) in {ROMS}')


def entrants_hex(entrants):
    return ''.join(f'{i:02x}{fl:02x}{s:02x}00' for i, fl, s in entrants)


def game_state():
    path = os.path.join(LAB, 'toolkit.bin')
    if not os.path.exists(path) or not os.path.exists(os.path.join(LAB, 'room.state')):
        if subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'bn4_lab.py'), 'boot']).returncode:
            sys.exit('xbracket: tools/bn4_lab.py boot failed')
    with open(path, 'rb') as f:
        return struct.unpack_from('<I', f.read(), TOOLKIT_GAMESTATE)[0]


def bn4_case(rom, name):
    t, ents = CASES[name]
    out = os.path.join(OUT, 'bn4', name)
    os.makedirs(out, exist_ok=True)
    for old in glob.glob(os.path.join(out, 'f*')):
        os.remove(old)
    rel = os.path.relpath(out, ROOT)
    script = ['load /src/.build/bn4lab/room.state', 'run 2',
              f'pokes {TOURNAMENTS + t * TOURNAMENT_SIZE:08x} {entrants_hex(ents)}',
              f'poke8 {BRACKET + 5:08x} {t:x}', f'poke8 {BRACKET + 6:08x} 0', f'poke8 {BRACKET + 7:08x} 0',
              f'poke8 {game_state():08x} {GAMESTATE_TO_BRACKET:x}', f'watch {BRACKET:08x} {STATE_BYTES:x}',
              f'rec {FRAMES} /src/{rel}/f', '']
    p = subprocess.run(['docker', 'run', '--rm', '-i', '--name', f'cw-xbracket-{name}-{os.getpid()}', '-v', f'{ROOT}:/src',
                        '-v', f'{ROMS}:/roms:ro', '-w', '/src', 'cyberworld-romlab', 'tools/romlab/romlab.bin',
                        f'/roms/{rom}'], input='\n'.join(script).encode(), capture_output=True)
    if p.returncode:
        sys.exit(f'xbracket: romlab failed: {p.stderr.decode()[-400:]}')
    with open(os.path.join(out, 'f.w'), 'rb') as f:
        w = f.read()
    states = [w[k:k + STATE_BYTES] for k in range(0, len(w), STATE_BYTES)]
    with open(os.path.join(out, 'state.txt'), 'w') as f:
        for k, s in enumerate(states):
            f.write(f'{k} {s.hex()}\n')
    return out


def read_ppm(path):
    """a frame's size and its pixels as RGB bytes (romlab's PPM or the host build's BMP)"""
    from PIL import Image
    im = Image.open(path).convert('RGB')
    return im.size[0], im.size[1], im.tobytes()


def frames_of(side, name):
    """the side's frames from its screen's first frame shown: [(path, state bytes)], or None where it never shows"""
    d = os.path.join(OUT, side, name)
    path = os.path.join(d, 'state.txt')
    if not os.path.exists(path):
        return None
    rows = [ln.split() for ln in open(path) if ln.strip()]
    states = [(int(k), bytes.fromhex(s)) for k, s in rows]
    first = next((k for k, s in states if s[0] == SHOWN), None)
    if first is None:
        return None
    ppms = sorted(glob.glob(os.path.join(d, 'f*.ppm')) + glob.glob(os.path.join(d, 'f*.bmp')))
    return [(ppms[k], s) for k, s in states if k >= first and k < len(ppms)]


def mutate(name, k, w, h, px):
    px = bytearray(px)
    if name == 'palette':
        # (the most common colour of the frame changed: a palette word copied wrong)
        counts = {}
        for i in range(0, len(px), 3):
            counts[bytes(px[i:i + 3])] = counts.get(bytes(px[i:i + 3]), 0) + 1
        top = max(counts, key=counts.get)
        for i in range(0, len(px), 3):
            if bytes(px[i:i + 3]) == top:
                px[i] ^= 0x08
    elif name == 'plate':
        # (rows 40-80 of the left half one row lower: where the first round's plates stand)
        for y in range(80, 40, -1):
            for x in range(0, w // 2):
                o, s = 3 * (y * w + x), 3 * ((y - 1) * w + x)
                px[o:o + 3] = px[s:s + 3]
    return bytes(px)


def backdrop_only(px):
    """the frame one colour throughout (every layer off: the backdrop alone)"""
    return px == px[:3] * (len(px) // 3)


def compare(name, mutation=None, side='bn6'):
    """BN6's frames against BN4's on the screen's clock: (verdict, why, rows). The one allowed difference (ALLOWED): the first frame
    shown where both are one colour, the backdrop of the map each was opened from"""
    b4, b6 = frames_of('bn4', name), frames_of(side, name)
    if not b4 or not b6:
        return 'INCONCLUSIVE', f'{"BN4" if not b4 else "BN6"}\'s side has no frame shown (.build/xbracket/{"bn4" if not b4 else side}/{name})', []
    if mutation == 'late':
        b6 = [b6[0]] + b6
    n = min(len(b4), len(b6))
    first, same, allowed, worst, rows = None, 0, 0, 0, set()
    for k in range(n):
        w4, h4, p4 = read_ppm(b4[k][0])
        w6, h6, p6 = read_ppm(b6[k][0])
        if mutation in ('palette', 'plate'):
            p6 = mutate(mutation, k, w6, h6, p6)
        if (w4, h4) != (w6, h6):
            return 'FAIL', f'frame {k} of the shown: BN4 {w4}x{h4}, BN6 {w6}x{h6}', []
        if p4 == p6:
            same += 1
            continue
        if k == 0 and backdrop_only(p4) and backdrop_only(p6):
            allowed += 1
            continue
        diff = [i // 3 for i in range(0, len(p4), 3) if p4[i:i + 3] != p6[i:i + 3]]
        rows |= {i // w4 for i in diff}
        worst = max(worst, len(diff))
        if first is None:
            first = (k, os.path.basename(b4[k][0]), os.path.basename(b6[k][0]), len(diff), diff[0] % w4, diff[0] // w4)
    if first is None:
        return 'PASS', f'{n} frames from the first shown, every pixel ({allowed} allowed: {ALLOWED})', []
    k, f4, f6, d, x, y = first
    return 'FAIL', (f'{same + allowed} of {n} frames the same ({allowed} allowed); the first that differs, frame {k} (BN4 {f4}, BN6 {f6}): '
                    f'{d} pixels, the first at {x},{y}; at most {worst} pixels a frame, in {len(rows)} rows'), sorted(rows)


def bn6_case(name, mutation=None):
    """BN6's frames: the host build headless (build.py shot) on a run's first layer, the case's entrants given to the bracket step
    (xbracket.h's dev step) once MegaMan stands on it, each frame from the step kept with the moved screen's state
    (XBR_STATE's first 16 bytes, --watch)"""
    t, ents = CASES[name]
    out = os.path.join(OUT, 'bn6' + (f'-{mutation}' if mutation else ''), name)
    os.makedirs(out, exist_ok=True)
    for old in glob.glob(os.path.join(out, 'f*')):
        os.remove(old)
    rel = os.path.relpath(out, ROOT)
    env = dict(os.environ, CYBERWORLD_XBRACKET_MUTATE=mutation) if mutation else None
    spec = f'{t} ' + '/'.join(f'{i:02x}{fl:02x}{s:02x}' for i, fl, s in ents)
    data = os.path.join(OUT, 'bn6-data')
    os.makedirs(data, exist_ok=True)
    cmd = [sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--scene', 'emu', '--run-depth', '1', '--size', '240x160', '--frames', str(BN6_STEP + FRAMES + 2),
           '--data-dir', f'/src/{os.path.relpath(data, ROOT)}', '--input', f'{BN6_STEP}:,0:bracket {spec},{FRAMES + 2}:',
           '--shot-range', f'{BN6_STEP}:{BN6_STEP + FRAMES - 1}:/src/{rel}/f', '--watch', f'{XBR_STATE:08x}:{STATE_BYTES:x}']
    p = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, env=env)
    if p.returncode or 'bracket: tournament' not in p.stdout:
        sys.exit(f'xbracket: BN6\'s run failed: {(p.stdout + p.stderr)[-600:]}')
    if f'tournament {t} shown' not in p.stdout:
        print(f'{name}: BN6 did not show it: ' + next((ln for ln in p.stdout.splitlines() if 'bracket' in ln), ''))
    with open(os.path.join(out, 'f.w'), 'rb') as f:
        w = f.read()
    with open(os.path.join(out, 'state.txt'), 'w') as f:
        for k in range(0, len(w), STATE_BYTES):
            f.write(f'{k // STATE_BYTES} {w[k:k + STATE_BYTES].hex()}\n')
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('what', choices=('bn4', 'bn6', 'compare'))
    ap.add_argument('cases', nargs='*')
    ap.add_argument('--mutation', choices=sorted(MUTATIONS), help='the comparator\'s own check: BN6\'s frames broken as read')
    ap.add_argument('--move-mutation', choices=sorted(MOVE_MUTATIONS), help='the move broken (BN6\'s side run again with it)')
    a = ap.parse_args()
    cases = a.cases or list(CASES)
    for c in cases:
        if c not in CASES:
            sys.exit(f'xbracket: no case {c} ({", ".join(CASES)})')
    if a.what == 'bn4':
        rom = rom_name()
        for c in cases:
            fr = frames_of('bn4', c) if bn4_case(rom, c) else None
            print(f'{c}: {len(fr) if fr else 0} frames shown from frame '
                  f'{os.path.basename(fr[0][0]) if fr else "none"} (.build/xbracket/bn4/{c})')
        return 0
    if a.what == 'bn6':
        for c in cases:
            fr = frames_of('bn6', c) if bn6_case(c) else None
            print(f'{c}: {len(fr) if fr else 0} frames shown from frame '
                  f'{os.path.basename(fr[0][0]) if fr else "none"} (.build/xbracket/bn6/{c})')
        return 0
    if a.move_mutation:
        ok = True
        for c in [c for c in cases if c in MOVE_MUTATION_CASES[a.move_mutation]]:
            bn6_case(c, a.move_mutation)
            v0, why0, rows0 = compare(c)
            v1, why, rows1 = compare(c, side='bn6-' + a.move_mutation)
            broke = v1 == 'FAIL' and (v0 == 'PASS' or bool(set(rows1) - set(rows0)) or why != why0)
            if v0 != 'PASS':
                print(f'{c}: the move itself does not pass ({v0}): the mutation\'s FAIL proves less')
            print(f'{c}: move mutation {a.move_mutation}: {v1}{", worse than the move" if broke else ", NOT caught"}: {why}')
            ok = ok and broke
        return 0 if ok else 1
    worst = 0
    for c in cases:
        verdict, why, _ = compare(c, a.mutation)
        print(f'{c}: {verdict}{" (mutation " + a.mutation + ")" if a.mutation else ""}: {why}')
        worst = max(worst, {'PASS': 0, 'INCONCLUSIVE': 2, 'FAIL': 1}[verdict])
    if a.mutation:
        return 0 if all(compare(c, a.mutation)[0] == 'FAIL' for c in cases) else 1
    return worst


if __name__ == '__main__':
    sys.exit(main())
