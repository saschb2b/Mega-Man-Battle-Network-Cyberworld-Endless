#!/usr/bin/env python3
"""xfamily.py [--spec fight|status|delete|native|all] [--quiet]: a family of
BN5's moved into BN6's battle engine, checked in BN6's own battles (issue
#82; docs/MULTIROM.md, Transplanted families).

BN5's CanGard is installed as the core starts wherever BN5's ROM is read
(src/emu/xvirus.c): its code and its reticle's copied from the player's
BN5 ROM, its calls aimed at BN6's routines, as AI 32 with enemy ids
0x200-0x205 (its six versions), BN6's tables grown for it
(src/emu/xvirus_grow.c). Each spec is headless runs of the host build
(build.py shot) with both ROMs, copied into .build/xfamily/rom, from the
same boot, layer (seed 15, whose first area is BN6's own: a layer dressed
as BN5's fights its random battles in BN5's engine) and keys, each a battle
the foes step starts, each traced (CYBERWORLD_BTRACE: EWRAM and IWRAM after
every frame of the battle, src/emu/battle_trace.c). MegaMan stands at
(2,2):

  fight    CanGard V1 at (5,2) beside a Mettaur at (4,3), MegaMan idle: its
           own states (8 decide, 9 guard, 10 fire), its reticle (the
           second-level dispatch's type 0xD2, run as BN5's 0x25 by the third
           kind's side table at 0x0203FF20) walking to MegaMan's panel and
           striking there for 10, the
           muzzle's blast (BN6's effect 0x6C), its records as BN6 reads
           them (HP 60, element 0, a shadow); pictures of its first shot
  status   the same battle, the status step (director_dev_status) giving
           CanGard freeze, bubble, paralysis and drag, each while its
           reticle is out: each must let the reticle go (the hooks of
           tables 7, 8, 6 and 10: AttackVars +0x30 cleared, the reticle
           gone), and CanGard come back to its own states and fire again;
           the ice block its size (0); the battle on to the end, no crash
  delete   each version alone (0x200-0x205, --dev onehit), MegaMan shooting,
           event flag 0x1D8 set first (the rare path's gate): its HP
           (struct2's row), its deletion, the results, BN6's delete counts
           (0x02000070-0x0200009F, versions 0-3's path) and the rare path's
           byte (0x0203EAEA, versions 4 and 5's) as they were at the
           battle's start (the installer's hooks: AI 32 has no count, and
           no new-virus-data chat); then BN6's own Mettaur, version 0 (id
           0x01) and 4 (0x05), the same way: the control, its deletion
           raising its count (0x02000071) and writing its AI index (1) to
           the rare path's byte, so that neither check holds by default
  native   BN6's own Mettaur fought by xcontrol.py's fight keys, CanGard
           not installed (--dev nofamilies) and installed: the same frame
           for frame, every byte, but a pointer into what the install moved
           (BN6's tables grown among it)

Needs both ROMs (local only). Exits 0 when every check holds.
"""
import argparse
import hashlib
import os
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xcontrol as X  # noqa: E402  (its trace reader and comparison)

ROOT = X.ROOT
WORK = os.path.join(ROOT, '.build', 'xfamily')
SHA1 = {'bn6.gba': X.BN6_SHA1, 'bn5.gba': '5f472f78d8de2df01d5039e045c043cb40969a39'}
SEED = 15
FAMILY_AI, FIRST_ID = 32, 0x200
DISPATCH_T3 = 0xD2             # its attack's type in BN6: the second-level dispatch's of the third kind (XV_T3_DISPATCH),
SIDE_T3 = 0x0203FF20           # ... which runs it as the number its side table holds by the object's slot (BN6_DISPATCH_SIDE + 0x20),
RETICLE = 0x25                 # ... BN5's own for it
MUZZLE = 0x6C                  # its muzzle's effect in BN6
MEGAMAN = 0x1A0                # MegaMan's NameID
T3_TYPE, T4_TYPE, T4_EFFECT = 1, 1, 4   # an attack's type, an effect object's type (0) and its effect id
T1_ACTION, T1_HP, T1_MAX_HP, T1_ELEMENT, T1_AI = 9, 0x24, 0x26, 0x0E, 0x58
ATTACK_VARS = 0xA0             # AIData + 0xA0: its AttackVars (+0x30 its reticle, 1 once it fired)
AI_INDEX = 1                   # AIData + 1
SHADOW = 1                     # its sprite's byte +3 (the sprite at the object + (byte 2 & 0xF0)): bit 0 a shadow
DELETE_COUNTS = (0x02000070, 0x020000A0)
EVENT_FLAGS = 0x02001C88       # (BN6_EVENT_FLAGS) flag n: bit 0x80 >> (n & 7) of byte n / 8
RARE_GATE = 0x1D8              # the event flag BN6's rare path (versions 4 and 5, 0x0802D65E) asks for first: clear, it does nothing
RARE_BYTE = 0x0203EAEA         # where it then puts the AI index (byte 0xA of the record at 0x0203EAE0, by 0x0800AB2E; flag
                               # 0x196 + AI clear), for the new-virus-data chat by that index after the results (0x0802C2E8)
METTAUR_AI, METTAUR_IDS = 1, (0x01, 0x05)   # BN6's own Mettaur, versions 0 and 4: the delete spec's control
STATS_HP = (60, 120, 150, 180, 230, 280)   # (BN5's struct2 for CanGard, as the delete spec reads it from the battle)
STATUS_ACTION = {'freeze': 6, 'bubble': 7, 'paralysis': 4, 'drag': 5}
CUSTOM_CLOSED = ['260:', '4:START', '10:', '4:A']   # the Custom screen closed with OK, after the foes step
KEYS_FRAMES = sum(int(k.split(':')[0]) for k in X.KEYS_BEFORE.split(',') + CUSTOM_CLOSED)   # the frame a spec's keys begin at


def find_roms():
    """BN6's and BN5's ROMs, told by their SHA-1, copied into WORK/rom"""
    rom = os.path.join(WORK, 'rom')
    os.makedirs(rom, exist_ok=True)
    want = {name: sha for name, sha in SHA1.items() if not os.path.exists(os.path.join(rom, name))}
    for name in sorted(os.listdir(X.ROM_DIR)) if want else []:
        path = os.path.join(X.ROM_DIR, name)
        if name.lower().endswith('.gba') and os.path.isfile(path):
            with open(path, 'rb') as f:
                sha = hashlib.sha1(f.read()).hexdigest()
            for k, v in list(want.items()):
                if sha == v:
                    shutil.copy(path, os.path.join(rom, k))
                    del want[k]
    if want:
        sys.exit(f'no {" or ".join(want)} in {X.ROM_DIR} (BN6 Gregar and BN5 Team Colonel, USA)')
    return rom


def run(spec, name, foes, keys, frames, dev='', shots=(), before=()):
    """One headless run: its trace and its log's lines of the family and the
    steps; `before`: steps on the map before the foes step"""
    data = os.path.join(WORK, spec, name)
    os.makedirs(data, exist_ok=True)
    for f in os.listdir(data):
        if f == 'btrace.bin' or f.endswith('.bmp'):
            os.remove(os.path.join(data, f))
    boot = os.path.join(WORK, 'boot-3.state')
    if os.path.exists(boot):
        shutil.copy(boot, os.path.join(data, 'boot-3.state'))
    k = X.KEYS_BEFORE.split(',') + list(before) + [f'0:foes {foes}'] + CUSTOM_CLOSED + keys
    cmd = [sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--data-dir', f'/src/.build/xfamily/{spec}/{name}',
           '--scene', 'emu', '--net-biome', '0', '--seed', str(SEED), '--frames', str(frames), '--input', ','.join(k)]
    if dev:
        cmd += ['--dev', dev]
    if shots:
        cmd += ['--shot', ','.join(f'{t}:/src/.build/xfamily/{spec}/{name}/f{t}.bmp' for t in shots)]
    env = dict(os.environ, CYBERWORLD_ROM_DIR=os.path.join(WORK, 'rom'), CYBERWORLD_BTRACE='1')
    out = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True)
    lines = [l for l in out.stdout.splitlines() + out.stderr.splitlines() if l.startswith(('xvirus', 'foes', 'status'))]
    if out.returncode:
        sys.exit(f'{spec}/{name}: the run failed\n' + out.stderr[-2000:])
    if not os.path.exists(boot) and os.path.exists(os.path.join(data, 'boot-3.state')):
        shutil.copy(os.path.join(data, 'boot-3.state'), boot)
    moved, trace = X.read_trace(os.path.join(data, 'btrace.bin'))
    return moved, trace, lines


# ---- what a frame holds ----

def t1_of(img, name_id):
    """The battle object of this NameID in play, or None"""
    for k in range(X.OBJ_COUNT):
        o = X.T1_OBJECTS + k * X.OBJ_SIZE
        if X.u8(img, o) & 1 and X.u16(img, o + X.NAME_ID) == name_id:
            return o
    return None


def reticles(img):
    """The attacks of CanGard's type in play: (its phase, panel); phase STRIKE
    the frame it strikes, its hit set on its panel"""
    out = []
    for k in range(X.OBJ_COUNT):
        o = X.T3_OBJECTS + k * X.OBJ_SIZE
        if X.u8(img, o) & 1 and X.u8(img, o + T3_TYPE) == DISPATCH_T3 and X.u8(img, SIDE_T3 + X.u8(img, o + 3)) == RETICLE:
            out.append((X.u8(img, o + 9), (X.u8(img, o + 0x12), X.u8(img, o + 0x13))))
    return out


STRIKE = 8   # the reticle's phase (+9) once its hit is set (BN5 0x080C972A)


def muzzles(img):
    n = 0
    for k in range(X.OBJ_COUNT):
        o = X.T4_OBJECTS + k * X.T4_SIZE
        n += X.u8(img, o) & 1 and X.u8(img, o + T4_TYPE) == 0 and X.u8(img, o + T4_EFFECT) == MUZZLE
    return n


def cangard(img, name_id=FIRST_ID):
    """CanGard as the frame has it: (CurAction, its AttackVars +0x30, panel, HP), or None"""
    o = t1_of(img, name_id)
    if o is None:
        return None
    ai = X.u32(img, o + T1_AI)
    return X.u8(img, o + T1_ACTION), X.u32(img, ai + ATTACK_VARS + 0x30), (X.u8(img, o + 0x12), X.u8(img, o + 0x13)), X.u16(img, o + T1_HP)


class Checks:
    def __init__(self, spec):
        self.spec, self.bad = spec, 0

    def __call__(self, ok, what):
        print(f'  {"ok  " if ok else "FAIL"} {what}')
        self.bad += not ok
        return ok


# ---- the specs ----

def fight(c, a):
    shots = range(1300, 1430, 6)
    _, trace, lines = run('fight', 'cangard', f'{FIRST_ID:02X}@52/01@43', ['1500:'], 2800, shots=shots)
    for l in lines:
        print(f'  {l}')
    first = next(img for _, _, img in trace if t1_of(img, FIRST_ID) is not None)
    o = t1_of(first, FIRST_ID)
    ai = X.u32(first, o + T1_AI)
    sprite = o + (X.u8(first, o + 2) & 0xF0)
    c(X.u8(first, ai + AI_INDEX) == FAMILY_AI and X.u16(first, o + T1_HP) == STATS_HP[0],
      f'CanGard V1 spawned as AI {X.u8(first, ai + AI_INDEX)} with {X.u16(first, o + T1_HP)} HP (32, 60)')
    c(X.u8(first, o + T1_ELEMENT) == 0 and X.u8(first, sprite + 3) & SHADOW,
      f'its element {X.u8(first, o + T1_ELEMENT)} and its shadow {X.u8(first, sprite + 3) & SHADOW} (struct1 laid out BN6\'s way: 0, 1)')
    acts, hits, spawned, walked, muzzle = [], [], 0, set(), 0
    was_hp, was_ret, was_mm = None, [], None
    for num, timer, img in trace:
        cg, mm = cangard(img), t1_of(img, MEGAMAN)
        if cg and (not acts or acts[-1] != cg[0]):
            acts.append(cg[0])
        ret = reticles(img)
        spawned += len(ret) > len(was_ret)
        walked |= {p for _, p in ret}
        muzzle = max(muzzle, muzzles(img))
        hp = X.u16(img, mm + T1_HP) if mm is not None else None
        here = (X.u8(img, mm + 0x12), X.u8(img, mm + 0x13)) if mm is not None else None
        if was_hp is not None and hp is not None and hp < was_hp:
            hits.append((timer, was_hp - hp, any(ph == STRIKE and p == here for ph, p in was_ret)))
        was_hp, was_ret, was_mm = hp, ret, mm
    seen = sorted(set(acts))
    print(f'  CanGard\'s CurActions in turn: {" ".join(f"{x:02X}" for x in acts[:24])}{" ..." if len(acts) > 24 else ""}')
    c({8, 9, 10} <= set(seen) and set(seen) <= {0, 1, 2, 8, 9, 10}, f'its own states 8, 9 and 10, no other but the spawn\'s ({seen})')
    c(spawned >= 3 and (2, 2) in walked, f'{spawned} reticles of BN5\'s type {RETICLE:#x} (the dispatch\'s {DISPATCH_T3:#x}), over panels {sorted(walked)} to MegaMan\'s (2, 2)')
    struck = [h for h in hits if h[2]]
    print('  MegaMan\'s HP lost: ' + ', '.join(f'{d} at t{t}{" (the reticle\'s strike)" if r else ""}' for t, d, r in hits))
    c(struck and all(d == 10 for _, d, _ in struck), f'{len(struck)} strikes of the reticle, each 10 (V1\'s damage)')
    c(muzzle > 0, f'the muzzle\'s blast drawn (effect {MUZZLE:#x}, its sprite past list 4\'s 92)')
    print(f'  pictures: .build/xfamily/fight/cangard/f{shots[0]}.bmp to f{shots[-1]}.bmp')


STATUSES = [('freeze 60', 167), ('bubble 60', 97), ('paralysis 60', 97), ('drag 1', 97)]   # each at the frames after the one before


def status(c, a):
    keys, shots, at = [], [], KEYS_FRAMES
    for spec, wait in STATUSES:
        keys += [f'{wait}:', f'0:status {spec} {FIRST_ID:02X}']
        at += wait
        shots += [at + 8, at + 30]   # (pictures of it in each status)
    _, trace, lines = run('status', 'cangard', f'{FIRST_ID:02X}@52/01@43', keys + ['900:'], 2800, shots=shots)
    print(f'  pictures: .build/xfamily/status/cangard/f{shots[0]}.bmp to f{shots[-1]}.bmp')
    at = {}
    for l in lines:
        print(f'  {l}')
        if l.startswith('status:') and ' at battle frame ' in l:
            at[l.split()[1]] = int(l.rsplit(' ', 1)[1])
    by_timer = {timer: i for i, (_, timer, _) in enumerate(trace)}
    for spec, _ in STATUSES:
        kind = spec.split()[0]
        t = at.get(kind)
        if not c(t is not None and t in by_timer, f'{kind}: the step took at battle frame {t}'):
            continue
        i = by_timer[t]
        before = cangard(trace[i][2])
        out = before and before[1] > 1 and reticles(trace[i][2])
        c(bool(out), f'{kind}: its reticle out as it struck (CurAction {before[0]:02X}, AttackVars +0x30 {before[1]:08x})' if before
          else f'{kind}: CanGard in play as it struck')
        took = cangard(trace[i + 1][2])
        c(took and took[0] == STATUS_ACTION[kind], f'{kind}: CurAction {took[0] if took else None:02X} the frame after ({STATUS_ACTION[kind]:02X})')
        gone = next((j - i for j in range(i + 1, min(i + 4, len(trace))) if not reticles(trace[j][2])), None)
        c(gone is not None and cangard(trace[i + 1][2])[1] == 0,
          f'{kind}: AttackVars +0x30 cleared the frame after, the reticle gone {gone} frame(s) after')
        back = next((j for j in range(i + 1, len(trace)) if (cangard(trace[j][2]) or (0,))[0] >= 8), None)
        again = back is not None and next((j for j in range(back, len(trace)) if reticles(trace[j][2])), None)
        c(back is not None and again, f'{kind}: back in its own states {back - i if back else None} frames on, '
          f'its next reticle {again - i if again else None} frames on')
    blocks = X.ice_blocks(trace)
    c(blocks and all(z == '0' for _, z in blocks), f'its ice block: {blocks} (size 0, the recipe\'s)')
    last = trace[-1][2]
    c(X.u8(last, X.BATTLE_PHASE) == 0x0C and cangard(last), f'the battle on to its last frame ({len(trace)} traced), CanGard in it')


def delete_one(name, ident):
    keys = ['30:'] + ['2:B', '22:'] * 60 + ['600:']
    return run('delete', name, f'{ident:02X}@52', keys, 3600, dev='onehit', before=[f'0:flags {RARE_GATE:#x} {RARE_GATE:#x} 1'])


def deleted(c, what, trace, ident):
    """Its deletion, and what it left over the traced frames: (its first frame, BN6's delete counts, the rare path's byte)"""
    first = next((img for _, _, img in trace if t1_of(img, ident) is not None), None)
    died = any((cangard(img, ident) or (0,))[0] == 2 for _, _, img in trace)
    res = X.results_at(trace)
    c(first is not None and died and res is not None, f'{what}: spawned, deleted (CurAction 2), the results from traced frame {res}')
    lo, hi = DELETE_COUNTS
    gate = trace and X.u8(trace[0][2], EVENT_FLAGS + RARE_GATE // 8) & 0x80 >> (RARE_GATE & 7)
    c(gate, f'{what}: event flag {RARE_GATE:#x} set from the battle\'s first frame (the flags step before the foes step)')
    return first, [bytes(img[X.off_of(lo):X.off_of(hi)]) for _, _, img in trace], [X.u8(img, RARE_BYTE) for _, _, img in trace]


def delete(c, a):
    lo, hi = DELETE_COUNTS
    for v in range(6):   # (one after another: each run makes the build first)
        ident = FIRST_ID + v
        _, trace, _ = delete_one(f'v{v}', ident)
        first, counts, rare = deleted(c, f'version {v} (id {ident:#x})', trace, ident)
        hp0 = X.u16(first, t1_of(first, ident) + T1_MAX_HP) if first else None
        c(hp0 == STATS_HP[v], f'version {v}: its MaxHP {hp0} (its struct2 row: {STATS_HP[v]})')
        c(counts and all(x == counts[0] for x in counts), f'version {v}: BN6\'s delete counts {lo:08x}-{hi - 1:08x} unchanged '
          f'through its {len(counts)} frames')
        c(rare and all(x == rare[0] for x in rare), f'version {v}: the rare path\'s byte {RARE_BYTE:08x} unchanged ({rare[0] if rare else None}) '
          f'through its {len(rare)} frames')
        gone = next((i for i, (_, _, img) in enumerate(trace) if (cangard(img, ident) or (0,))[0] == 2), len(trace))
        c(not any(reticles(img) for _, _, img in trace[gone + 2:]), f'version {v}: no reticle left after its deletion')
    for ident in METTAUR_IDS:   # the control: BN6's own virus, its deletion seen by the same checks
        version = ident - METTAUR_IDS[0]
        _, trace, _ = delete_one(f'mettaur{version}', ident)
        _, counts, rare = deleted(c, f'Mettaur version {version} (id {ident:#x}), BN6\'s own', trace, ident)
        if version < 4:
            mine = [x[METTAUR_AI] for x in counts]
            at = next((i for i, x in enumerate(mine) if x != mine[0]), None)
            c(at is not None and mine[at] == mine[0] + 1, f'Mettaur version {version}: its delete count {lo + METTAUR_AI:08x} '
              f'{mine[0] if mine else None} to {mine[at] if at is not None else None} at traced frame {at} (raised by one)')
        else:
            at = next((i for i, x in enumerate(rare) if x != rare[0]), None)
            c(rare and rare[0] == 0 and at is not None and rare[at] == METTAUR_AI, f'Mettaur version {version}: the rare path\'s byte '
              f'{RARE_BYTE:08x} {rare[0] if rare else None} to {rare[at] if at is not None else None} at traced frame {at} (0 to its AI index, {METTAUR_AI})')


def native(c, a):
    keys = X.fight()
    (ma, fa, la), (mb, fb, lb) = (run('native', name, '01@52', keys, 4800, dev) for name, dev in
                                  (('alone', 'nofamilies,hp=1000'), ('installed', 'hp=1000')))
    c(not any(l.startswith('xvirus') for l in la) and any(l.startswith('xvirus') for l in lb),
      'CanGard installed in the second run alone')
    explained, bad = X.compare(fa, fb, ma, mb, False)
    for reason, where in sorted(explained.items()):
        print(f'  {reason}: {len(where)} bytes ({" ".join(f"{x:08x}" for x in sorted(where)[:8])})')
    for frame, addr, x, y in bad[:12]:
        print(f'  unexplained: frame {frame} {addr:08x}: {x:02x} | {y:02x}')
    c(not bad and len(fa) == len(fb), f'BN6\'s Mettaur the same with CanGard installed: {len(fa)} frames, every byte '
      f'of EWRAM and IWRAM, {len(bad)} differences unexplained')


SPECS = {'fight': fight, 'status': status, 'delete': delete, 'native': native}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--spec', choices=sorted(SPECS) + ['all'], default='all')
    ap.add_argument('--quiet', action='store_true')
    a = ap.parse_args()
    find_roms()
    subprocess.run([sys.executable, os.path.join(ROOT, 'build.py'), 'host'], cwd=ROOT, check=True, capture_output=True)
    bad = 0
    for spec in sorted(SPECS) if a.spec == 'all' else [a.spec]:
        print(f'== {spec}')
        c = Checks(spec)
        SPECS[spec](c, a)
        print(f'{spec}: ' + ('every check holds' if not c.bad else f'{c.bad} checks failed'))
        bad += c.bad
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
