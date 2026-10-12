#!/usr/bin/env python3
"""xcontrol.py [--spec fight|status|all] [--seed S] [--story] [--quiet]:
the control family's differential (issue #82; docs/MULTIROM.md,
Transplanted families).

The installer that moves another game's virus families into BN6's battle
engine (src/emu/xvirus.c) is proven first on BN6's own Mettaur: --dev
xcontrol moves its code, records and tables through the same steps as a
family of BN5's, as AI 32 with enemy ids 0xBB-0xC0, and that moved Mettaur
must play frame for frame as BN6's own does. Each spec is three headless
runs of the host build (build.py shot), each from the same boot and the
same layer, with the same keys, each a battle the foes step starts against
one Mettaur at the same panel, each traced (CYBERWORLD_BTRACE: EWRAM and
IWRAM after every frame of the battle, src/emu/battle_trace.c):

  native     no family installed, BN6's Mettaur (id 0x01)
  installed  the control installed, BN6's Mettaur (id 0x01): the install
             leaves BN6's own battles alone
  moved      the control installed, the moved Mettaur (id 0xBB)

The specs:

  fight      MegaMan steps between the rows and shoots, charged and not,
             until the Mettaur is deleted: its own states 8-12
  status     the status step (director_dev_status) gives it each of the
             statuses a hit gives, one after another, each as the engine
             takes a hit's: freeze (shot at while frozen), paralysis and
             bubble (the same), drag, slide, a long freeze left to thaw;
             then the fight's keys until it is deleted: BN6's generic
             states 4-7 through its state table, the hooks of tables 6-10
             and the ice block's size by its AI index (BN6_ICE_SIZES)

With the replay on (the default; --no-replay leaves it off), the native
run keeps its battle's draws and the other two replay each of them from
it, keyed as the differential's rng=replay keys them (the control's moved
code as BN6's own: CYBERWORLD_XDRAWS, src/dev/xdiff_events.h): each must
be matched, and the runs still the same frame for frame.

The runs are compared frame by frame, every byte, and each byte that
differs must have its reason (why()): a word pointing into what one run
moved (the same once each run's installer's map reads it as the address it
was moved from), BN6's interrupt stack (dead between frames), and between
installed and moved alone the enemy id (0x01 as 0xBB) and the AI index (1
as 32) where the battle keeps them, and the delete counts
(BN6_DELETE_COUNTS, which the installer's hook keeps AI 32 out of); moved
is compared to the battle's results, where the reward comes from each
id's own drop row. With --story, the battle's objects (BN6_T1_OBJECTS,
BN6_T3_OBJECTS: CurAction and CurPhase, panel, HP, NameID) as they change,
the fight the frames compared. Needs the ROM (local only), copied into
.build/xcontrol/rom alone: a layer of BN5's would fight its battles in
BN5's engine. Exits 0 when the runs are the same frame for frame.
"""
import argparse
import hashlib
import os
import re
import shutil
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, '.build', 'xcontrol')
BN6_SHA1 = '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6'
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))

EWRAM, EWRAM_SIZE = 0x02000000, 0x40000
IWRAM, IWRAM_SIZE = 0x03000000, 0x8000
IMAGE = EWRAM_SIZE + IWRAM_SIZE
# (src/emu/bn6.h)
T1_OBJECTS, T3_OBJECTS, OBJ_SIZE, OBJ_COUNT = 0x0203A9B0, 0x0203CFE0, 0xD8, 0x20
BATTLE_PHASE, CUSTOM_WINDOW = 0x02034881, 0x02035292
DELETE_COUNTS = 0x02000070
AI_DATA_PTR = 0x58           # a battle object's AIData*: +1 its AI index
NAME_ID = 0x28
CONTROL_AI, CONTROL_ID, NATIVE_AI, NATIVE_ID = 32, 0xBB, 1, 0x01
IRQ_STACK = (0x03007F00, 0x03007F60)   # BN6's interrupt stack below its sp_irq (0x03007F60, set by its start code)
PHASE_RESULTS = 0x10                    # BN6_BATTLE_PHASE: the results, the reward picked from the enemy's drop row
BATTLE_REWARD = 0x0200A00C              # (BN6_BATTLE_REWARD)

KEYS_BEFORE = '300:,' + ','.join(['4:A,56:'] * 10)   # MegaMan's arrival words closed
CUSTOM_CLOSED = ['175:', '4:START', '10:', '4:A', '60:']   # the Custom screen closed with OK, after the foes step


def fight():
    """Charged shots (B held), steps between the rows and quick shots, until the Mettaur is deleted."""
    k = []
    for i in range(16):
        k += ['125:B', '20:', '6:UP' if i % 2 else '6:DOWN', '30:', '8:B', '10:', '8:B', '10:']
    return k


def statuses():
    """Each status a hit gives, one after another (the status step, as
    durations: '0:status ...' at the frame its place in the keys reaches),
    then the fight's keys."""
    k = ['90:']
    for kind in ('freeze 90', 'paralysis 90', 'bubble 90'):
        k += [f'0:status {kind}', '20:', '8:B', '10:', '8:B', '144:']
    k += ['0:status drag', '90:', '0:status slide', '90:', '0:status freeze 200', '260:']
    return k + fight()


SPECS = {'fight': fight, 'status': statuses}


def keys_of(spec, foe):
    """A spec's keys, as --input has them, and the frames its status steps
    fall on (counted as build.py shot counts them, from the run's start)."""
    k = KEYS_BEFORE.split(',') + [f'0:foes {foe:02X}@52'] + CUSTOM_CLOSED + SPECS[spec]()
    at, steps = 0, []
    for item in k:
        n, _, what = item.partition(':')
        if what.startswith('status '):
            steps.append(at)
        at += int(n)
    return ','.join(k), steps


def find_rom():
    for name in sorted(os.listdir(ROM_DIR)):
        path = os.path.join(ROM_DIR, name)
        if name.lower().endswith('.gba') and os.path.isfile(path):
            with open(path, 'rb') as f:
                if hashlib.sha1(f.read()).hexdigest() == BN6_SHA1:
                    return path
    sys.exit(f'no BN6 Gregar ROM in {ROM_DIR}')


def run(spec, name, dev, foe, seed, frames, quiet, draws=''):
    """One run traced (its btrace.bin), and its draws kept (draws 'keep') or replayed from another run's (a path in the
    image, /src/...), as the differential's rng=replay keys them (CYBERWORLD_XDRAWS, src/dev/xdiff_events.h): the path to
    its trace, and what its draws came to (the 'xdraws:' line)"""
    data = os.path.join(WORK, spec, name)
    os.makedirs(data, exist_ok=True)
    for f in os.listdir(data):
        if f in ('btrace.bin', 'xdraws.bin') or f.endswith('.bmp'):
            os.remove(os.path.join(data, f))
    boot = os.path.join(WORK, 'boot-3.state')
    if os.path.exists(boot):
        shutil.copy(boot, os.path.join(data, 'boot-3.state'))
    keys, steps = keys_of(spec, foe)
    # (pictures through the fight, and each status a few frames in)
    at = sorted({1100, 1500, 2000, 2600, 3200, 3800} | {t + d for t in steps for d in (12, 40)})
    shots = ','.join(f'{t}:/src/.build/xcontrol/{spec}/{name}/f{t}.bmp' for t in at)
    cmd = [sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--data-dir', f'/src/.build/xcontrol/{spec}/{name}',
           '--scene', 'emu', '--net-biome', '0', '--seed', str(seed), '--dev', dev, '--frames', str(frames),
           '--input', keys, '--shot', shots]
    env = dict(os.environ, CYBERWORLD_ROM_DIR=os.path.join(WORK, 'rom'), CYBERWORLD_BTRACE='1', CYBERWORLD_XDRAWS=draws)
    out = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True)
    lines = [l for l in out.stdout.splitlines() + out.stderr.splitlines() if l.startswith(('xvirus', 'foes', 'status', 'xdraws'))]
    if not quiet:
        for l in lines:
            print(f'  {name}: {l}')
    if out.returncode:
        sys.exit(f'{name}: the run failed\n' + out.stderr[-2000:])
    if not os.path.exists(boot) and os.path.exists(os.path.join(data, 'boot-3.state')):
        shutil.copy(os.path.join(data, 'boot-3.state'), boot)
    return os.path.join(data, 'btrace.bin'), next((l for l in lines if l.startswith('xdraws')), '')


def read_trace(path):
    """The trace's moved ranges and its frames: (number, timer, image)."""
    with open(path, 'rb') as f:
        b = f.read()
    if b[:4] != b'BTRC':
        sys.exit(f'{path}: no trace')
    n = struct.unpack_from('<I', b, 8)[0]
    moved = [struct.unpack_from('<3I', b, 12 + 12 * i) for i in range(n)]
    p, image, frames = 12 + 12 * n, bytearray(IMAGE), []
    while p < len(b):
        if b[p:p + 4] != b'FRAM':
            sys.exit(f'{path}: broken at {p}')
        num, timer, runs = struct.unpack_from('<3I', b, p + 4)
        p += 16
        for _ in range(runs):
            off, ln = struct.unpack_from('<2I', b, p)
            image[off:off + ln] = b[p + 8:p + 8 + ln]
            p += 8 + ln
        frames.append((num, timer, bytes(image)))
    return moved, frames


def addr_of(off):
    return EWRAM + off if off < EWRAM_SIZE else IWRAM + off - EWRAM_SIZE


def off_of(addr):
    return addr - EWRAM if addr < IWRAM else EWRAM_SIZE + addr - IWRAM


def pointer(v, moved):
    """A word read as its run's map has it: an address of what was moved as the one it was moved from."""
    for frm, to, ln in moved:
        if to <= (v & ~1) < to + ln:
            return v - to + frm
    return v


def u8(img, a):
    return img[off_of(a)]


def u16(img, a):
    return struct.unpack_from('<H', img, off_of(a))[0]


def u32(img, a):
    return struct.unpack_from('<I', img, off_of(a))[0]


def objects(img):
    """The battle's objects in play: (kind, slot, CurAction, CurPhase, panel, HP, NameID, x, y)."""
    out = []
    for kind, base in (('T1', T1_OBJECTS), ('T3', T3_OBJECTS)):
        for k in range(OBJ_COUNT):
            o = base + k * OBJ_SIZE
            if not u8(img, o) & 1:
                continue
            out.append((kind, k, u8(img, o + 9), u8(img, o + 0xA), (u8(img, o + 0x12), u8(img, o + 0x13)), u16(img, o + 0x24),
                        u16(img, o + NAME_ID), u32(img, o + 0x34) >> 16, u32(img, o + 0x38) >> 16))
    return out


def why(off, ia, ib, ma, mb, ids):
    """Why the byte at `off` differs between two runs' images, or None where
    nothing explains it: a word pointing into what each run moved (the same
    once read through each map); BN6's interrupt stack below sp_irq, dead
    between frames, where an interrupt's entry lands a few cycles apart as the
    moved code runs (DISPSTAT's HBlank bit as the handler read it); and with
    `ids`, the moved Mettaur's enemy id (0x01 as 0xBB) and AI index (1 as 32,
    and as a table's offset, x 4) where the battle keeps them, and the delete
    counts' byte for AI 1, which the installer's hook keeps AI 32 out of."""
    w = off & ~3
    va, vb = struct.unpack_from('<I', ia, w)[0], struct.unpack_from('<I', ib, w)[0]
    if va != vb and pointer(va, ma) == pointer(vb, mb):
        return 'a pointer into what was moved'
    addr, a, b = addr_of(off), ia[off], ib[off]
    if IRQ_STACK[0] <= addr < IRQ_STACK[1]:
        return 'the interrupt stack (dead)'
    if not ids:
        return None
    if NATIVE_ID <= a < NATIVE_ID + 6 and b == a - NATIVE_ID + CONTROL_ID:
        return 'the enemy id'
    if (a, b) == (NATIVE_AI, CONTROL_AI):
        return 'the AI index'
    if (a, b) == (NATIVE_AI * 4, CONTROL_AI * 4):
        return 'the AI index as a table offset'
    if addr == DELETE_COUNTS + NATIVE_AI:
        return 'the delete count'
    return None


T4_OBJECTS, T4_SIZE = 0x02036870, 0xC8   # the battle's fourth kind of object (the ice block a freeze spawns among them), 32 of them
ICE_BLOCK = 0x89                          # its spawn index (+1), as 0x080EAF1C spawns it; +0x10 its animation: the size 0x080EAF46 reads (BN6_ICE_SIZES)


def ice_blocks(frames):
    """Each ice block as it appeared: (frame, its sizes over its life, as one number where it kept one)."""
    out, live = [], {}
    for num, _, img in frames:
        now = {}
        for k in range(OBJ_COUNT):
            o = T4_OBJECTS + k * T4_SIZE
            if u8(img, o) & 1 and u8(img, o + 1) == ICE_BLOCK:
                now[k] = live.get(k) or (num, set())
                now[k][1].add(u8(img, o + 0x10))
                if k not in live:
                    out.append(now[k])
        live = now
    return [(n, '/'.join(str(z) for z in sorted(sizes))) for n, sizes in out]


def results_at(frames):
    """The first frame of the battle's results, or None"""
    return next((i for i, (_, _, img) in enumerate(frames) if u8(img, BATTLE_PHASE) == PHASE_RESULTS), None)


def compare(a, b, ma, mb, ids):
    """Frame by frame, every byte: the differences each reason explains
    ({reason: {address: frames}}), and those none does ([(frame, address,
    a's byte, b's)]); a run that traced more frames than the other is one.
    With `ids`, to the battle's results, the same frame in both: there the
    reward comes from each enemy id's own drop row (BN6_DROP_ROWS), which
    the moved family's ids do not share with BN6's."""
    explained, bad = {}, []
    n = min(len(a), len(b))
    if ids:
        ra, rb = results_at(a), results_at(b)
        if ra != rb:
            bad.append((min(x for x in (ra, rb, n) if x is not None), 0, ra or 0, rb or 0))
        n = min(x for x in (ra, rb, n) if x is not None)
    elif len(a) != len(b):
        bad.append((n, 0, len(a), len(b)))
    for i in range(n):
        ia, ib = a[i][2], b[i][2]
        if ia == ib:
            continue
        off = 0
        while off < IMAGE:
            k = off
            # (fast past the equal stretches)
            while k < IMAGE and ia[k:k + 64] == ib[k:k + 64]:
                k += 64
            off = k
            if off >= IMAGE:
                break
            if ia[off] != ib[off]:
                reason = why(off, ia, ib, ma, mb, ids)
                if reason:
                    seen = explained.setdefault(reason, {})
                    seen[addr_of(off)] = seen.get(addr_of(off), 0) + 1
                elif len(bad) < 200:
                    bad.append((a[i][0], addr_of(off), ia[off], ib[off]))
            off += 1
    return explained, bad


def story(frames):
    """The fight as it went: each frame where an object's state changed."""
    last, lines = None, []
    for num, timer, img in frames:
        now = objects(img)
        view = [(o[0], o[1], o[2], o[3], o[4], o[5]) for o in now]
        if view != last:
            parts = [f'{o[0]}{o[1]:02d} act {o[2]:02X}/{o[3]:02X} p{o[4][0]}{o[4][1]} hp {o[5]} id {o[6]:03X}' for o in now]
            lines.append(f'  f{num:4d} t{timer:5d} ph {u8(img, BATTLE_PHASE):02X}: ' + '; '.join(parts))
            last = view
    return lines


FRAMES = {'fight': 4800, 'status': 7200}   # each spec's run, its Mettaur deleted and the results shown well before


def replayed(line, kept):
    """whether a run's draws were each replayed from the native run's (none unmatched or unmapped, as many as it kept)"""
    m = re.match(r'xdraws: (\d+) replayed, (\d+) unmatched, (\d+) unmapped', line)
    return bool(m) and int(m.group(1)) == kept and m.group(2) == m.group(3) == '0'


def differential(spec, a):
    """One spec's three runs compared: whether they are the same frame for frame. With the replay on (the default),
    the native run keeps its battle's draws and the other two replay each of them from it, the control's moved code
    keyed as BN6's own (rng=replay's keys): the same frame for frame with it, every draw matched."""
    print(f'== {spec}')
    traces, draws, kept = {}, '', 0
    for name, dev, foe in (('native', 'hp=1000', NATIVE_ID), ('installed', 'xcontrol,hp=1000', NATIVE_ID),
                           ('moved', 'xcontrol,hp=1000', CONTROL_ID)):
        if not a.no_replay:
            draws = 'keep' if name == 'native' else f'/src/.build/xcontrol/{spec}/native/xdraws.bin'
        path, line = run(spec, name, dev, foe, a.seed, a.frames or FRAMES[spec], a.quiet, draws)
        traces[name] = read_trace(path)
        print(f'{name}: {len(traces[name][1])} frames of battle traced, {len(traces[name][0])} ranges moved' +
              (f'; {line}' if line else ''))
        if not a.no_replay and name == 'native':
            m = re.match(r'xdraws: (\d+) kept', line)
            kept = int(m.group(1)) if m else 0
            if not kept:
                print('  the native run kept no draws (its battle did not end in the run?)')
                return False
        elif not a.no_replay and not replayed(line, kept):
            print(f'  {name}: its draws not each replayed from the native run\'s {kept}')
            return False
    (m_n, f_n), (m_i, f_i), (m_m, f_m) = traces['native'], traces['installed'], traces['moved']
    if a.story:
        print('the fight (moved):')
        print('\n'.join(story(f_m)))
    ok = True
    for (la, fa, ma), (lb, fb, mb), ids in ((('native', f_n, m_n), ('installed', f_i, m_i), False),
                                             (('installed', f_i, m_i), ('moved', f_m, m_m), True)):
        explained, bad = compare(fa, fb, ma, mb, ids)
        upto = results_at(fa) if ids else None
        print(f'{la} against {lb}: {upto if upto is not None else min(len(fa), len(fb))} frames'
              f'{" (to the results)" if upto is not None else ""}, every byte of EWRAM and IWRAM: {len(bad)} differences unexplained')
        for reason, where in sorted(explained.items()):
            spots = ' '.join(f'{addr:08x}x{n}' for addr, n in sorted(where.items())[:12])
            print(f'  {reason}: {len(where)} bytes ({spots}{" ..." if len(where) > 12 else ""})')
        for frame, addr, x, y in bad[:20]:
            print(f'  unexplained: frame {frame} {addr:08x}: {x:02x} | {y:02x}')
        ok = ok and not bad
    acts = sorted({(o[2], o[3]) for _, _, img in f_m for o in objects(img) if o[0] == 'T1' and o[6] == CONTROL_ID})
    hp = sorted({o[5] for _, _, img in f_m for o in objects(img) if o[0] == 'T1' and o[6] == CONTROL_ID})
    t3 = sum(1 for i, (_, _, img) in enumerate(f_m) if any(o[0] == 'T3' for o in objects(img)))
    print(f'the moved Mettaur: actions/phases {" ".join(f"{x:02X}/{y:02X}" for x, y in acts)}; HP seen {hp[:3]}..{hp[-3:]}; '
          f'{t3} frames with an attack (T3) in play')
    for label, frames in (('native', f_n), ('moved', f_m)):
        sizes = ice_blocks(frames)
        if sizes:
            print(f'  ice blocks ({label}): ' + ', '.join(f'frame {n} size {z}' for n, z in sizes))
    r = results_at(f_i)
    if r is not None and r < len(f_i) and r < len(f_m):
        last_i, last_m = f_i[-1][2], f_m[-1][2]
        print(f'the results from frame {r}: the reward {u16(last_i, BATTLE_REWARD):04x} (id 0x01\'s drop row) against '
              f'{u16(last_m, BATTLE_REWARD):04x} (id 0xBB\'s), the delete counts {u8(last_i, DELETE_COUNTS + NATIVE_AI)} against '
              f'{u8(last_m, DELETE_COUNTS + NATIVE_AI)} for AI 1 and {u8(last_m, DELETE_COUNTS + CONTROL_AI)} past them for AI 32')
    elif not any(o[0] == 'T1' and o[6] in (NATIVE_ID, CONTROL_ID) and o[2] == 2 for _, _, img in f_m for o in objects(img)):
        print('  (the Mettaur was not deleted: the run ends before the results)')
    print(f'{spec}: ' + ('SAME frame for frame' if ok else 'DIFFERENT'))
    return ok


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--spec', choices=sorted(SPECS) + ['all'], default='all')
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--frames', type=int, default=0, help="each run's frames (the spec's own where 0)")
    ap.add_argument('--quiet', action='store_true')
    ap.add_argument('--story', action='store_true', help='print the fight frame by frame')
    ap.add_argument('--no-replay', action='store_true', help="the draws left as each run makes them (no CYBERWORLD_XDRAWS)")
    a = ap.parse_args()
    os.makedirs(os.path.join(WORK, 'rom'), exist_ok=True)
    rom = os.path.join(WORK, 'rom', 'bn6.gba')
    if not os.path.exists(rom):
        shutil.copy(find_rom(), rom)
    ok = [differential(spec, a) for spec in (sorted(SPECS) if a.spec == 'all' else [a.spec])]
    print('SAME frame for frame' if all(ok) else 'DIFFERENT')
    sys.exit(0 if all(ok) else 1)


if __name__ == '__main__':
    main()
