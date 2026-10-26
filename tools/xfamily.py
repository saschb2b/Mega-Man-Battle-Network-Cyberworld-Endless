#!/usr/bin/env python3
"""xfamily.py [--spec fight|status|delete|native|all] [--quiet]: a family of
BN5's moved into BN6's battle engine, checked in BN6's own battles (issue
#82; docs/multirom/families.md).

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
           and BN5's Navis not installed (--dev nofamilies,nonavis) and
           installed: the same frame for frame, every byte, but a pointer
           into what the install moved (BN6's tables grown among it), the
           code in IWRAM the lava's and the sea's install keeps patched
           (src/emu/xpanels.c, and the hit gate's tests of a hidden target,
           src/emu/xsea.c), and what the Souls do with no Soul held, which
           install with BN5's Navis since issue #82's run round (xproxy.py
           --souls's: a pointer into a table their install grew, BN5's mood
           in their own block)

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
BATTLE_FLAGS, TIME_STOP = 0x02034880 + 0x32, 4   # BN6_BATTLE_FLAGS (src/emu/bn6_panels.h): BN6_TIME_STOP while time stands still
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
                                  (('alone', 'nofamilies,nonavis,hp=1000'), ('installed', 'hp=1000')))
    # (BN6's tables grown in both where BN5's ROM is read: its chips' moved code takes them, xchips_code.c; CanGard's own
    # install line in the second alone, and BN5's Navis' too, docs/multirom/navis.md)
    c(not any(l.startswith(('xvirus: CanGard', 'xnavi:')) for l in la) and any(l.startswith('xvirus: CanGard') for l in lb),
      'CanGard installed in the second run alone (and BN5\'s Navis, docs/multirom/navis.md)')
    # (the Souls, which install with BN5's Navis, compared as xproxy.py --souls compares them: no Soul held)
    import xproxy
    xproxy.SOULS = True
    explained, bad = xproxy.compare(fa, fb, ma, mb, False)
    for reason, where in sorted(explained.items()):
        print(f'  {reason}: {len(where)} bytes ({" ".join(f"{x:08x}" for x in sorted(where)[:8])})')
    for frame, addr, x, y in bad[:12]:
        print(f'  unexplained: frame {frame} {addr:08x}: {x:02x} | {y:02x}')
    c(not bad and len(fa) == len(fb), f'BN6\'s Mettaur the same with CanGard installed: {len(fa)} frames, every byte '
      f'of EWRAM and IWRAM, {len(bad)} differences unexplained')


# ---- every family moved: the same checks, each family's own numbers (tests/xdiff/families.json) ----

FAMILIES_JSON = os.path.join(ROOT, 'tests', 'xdiff', 'families.json')
OWNER = 0x4C                   # an object's owner (+0x4C): the battle object it belongs to
ATTACK_WORDS = 0x18            # AIData + 0xA0: its AttackVars, 0x60 bytes (what a family holds: its attacks' objects among them)
STATUS_SPECS = (('paralysis', 'paralysis 60'), ('drag', 'drag 1'), ('freeze', 'freeze 60'), ('bubble', 'bubble 60'))
# (a status with a mark over it, the fourth kind's 6 of this kind, at its arm point 5: BN6 0x080E1CCA-0x080E1CEA, its owner's
# place plus 0x08018810's answer, x by its side and flip, 0x0800E2CE; issue #82, the chips track's find)
MARK_SPECS = (('confuse', 'confuse 120', 0), ('blind', 'blind 120', 1))
MARK_TYPE, MARK_POINT = 6, 5
# (a family that wears no mark of a status, by BN5's own rule: none expected)
NO_MARK = {('Appley', 'blind'): "its pre-step takes the hit's status bits 0xFE every frame its +0x60 and its held count are 0 "
                                "(BN5 0x0810455C, BN5_PRESTEP_TAKE 0x0800CD1E, ported), blindness's 0x20 among them, before the "
                                "status step reads it (BN5's the same bit, 0x0800CC4A-0x0800CC68); its confusion holds by its timer"}
BN5_ARM_VIRUSES_REF, BN5_ARM_VIRUS_SIZE = 0x080151BC, 0x3C   # (src/emu/bn5.h: BN5's arm points' records of list 1)
BACK_WITHIN, AGAIN_WITHIN = 600, 900   # frames: back in its own states after a status, then acting again
JOBS = 4                       # runs at once (each its own container)
FIGHT_AFTER = 124              # the frames from a spec's keys' first to the battle's timer's first (the Custom screen's OK
                               # closing it: a step at KEYS_FRAMES + 600 lands at battle frame 476, whatever the family)


def families(only=None):
    """the installed families (tests/xdiff/families.json, tools/xrecipe.py's), each with its V1 spec's panel and its
    versions' HP from BN5's struct2 (the player's ROM, as WORK/rom has it)"""
    import json
    with open(FAMILIES_JSON) as f:
        out = json.load(f)['families']
    with open(os.path.join(WORK, 'rom', 'bn5.gba'), 'rb') as f:
        bn5 = f.read()
    for fam in out:
        with open(os.path.join(ROOT, 'tests', 'xdiff', f'{fam["name"].lower()}_v1.json')) as f:
            fam['panel'] = json.load(f)['foes'].split('@')[1]
        at = int(fam['struct2'], 0) - 0x08000000
        fam['hp'] = [(bn5[at + 6 * v] | bn5[at + 6 * v + 1] << 8) & 0xFFF for v in range(6)]
        # (its arm point 5 as BN5 has it: its records of list 1 by its struct1's sprite number)
        table = int.from_bytes(bn5[BN5_ARM_VIRUSES_REF - 0x08000000:][:4], 'little') - 0x08000000
        rec = table + BN5_ARM_VIRUS_SIZE * bn5[int(fam['struct1'], 0) - 0x08000000 + 1]
        fam['arm5'] = tuple(b - 256 if b > 127 else b for b in bn5[rec + 2 * MARK_POINT:rec + 2 * MARK_POINT + 2])
        fam['ids'] = [int(x, 0) for x in fam['ids6']]
    return [f for f in out if not only or f['name'].lower() in only]


def shot(spec, name, foes, keys, frames, dev):
    """One headless run of the host build (built first: main), in a container of its own: its trace's path and its log's
    lines of the family and the steps"""
    import importlib.util
    bs = importlib.util.spec_from_file_location('build_py', os.path.join(ROOT, 'build.py'))
    bp = importlib.util.module_from_spec(bs)
    bs.loader.exec_module(bp)
    data = os.path.join(WORK, spec, name)
    os.makedirs(data, exist_ok=True)
    for f in os.listdir(data):
        if f == 'btrace.bin':
            os.remove(os.path.join(data, f))
    boot = os.path.join(WORK, 'boot-3.state')
    if os.path.exists(boot):
        shutil.copy(boot, os.path.join(data, 'boot-3.state'))
    k = X.KEYS_BEFORE.split(',') + [f'0:foes {foes}'] + CUSTOM_CLOSED + keys
    cmd = ['docker'] + (['--context', bp.CONTEXT] if bp.CONTEXT else []) + [
        'run', '--rm', '--name', f'fam-xf-{os.getpid()}-{spec}-{name}', '-u', f'{os.getuid()}:{os.getgid()}',
        '-v', f'{ROOT}:/src', '-w', '/src', '-v', f'{os.path.join(WORK, "rom")}:/rom:ro', '-e', 'CYBERWORLD_BTRACE=1', bp.IMAGE,
        'build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', f'/src/.build/xfamily/{spec}/{name}', '--scene', 'emu',
        '--net-biome', '0', '--seed', str(SEED), '--frames', str(frames), '--input', ','.join(k)] + (['--dev', dev] if dev else [])
    out = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    lines = [l for l in out.stdout.splitlines() + out.stderr.splitlines() if l.startswith(('xvirus', 'foes', 'status'))]
    if out.returncode:
        return None, lines + [f'the run failed: {out.stderr[-600:]}']
    return os.path.join(data, 'btrace.bin'), lines


def frames_of(path):
    """each traced frame as it is read: (its number, the battle's timer, the image), one image updated in place"""
    import struct
    with open(path, 'rb') as f:
        b = f.read()
    n = struct.unpack_from('<I', b, 8)[0]
    p, image = 12 + 12 * n, bytearray(X.IMAGE)
    while p < len(b):
        num, timer, runs = struct.unpack_from('<3I', b, p + 4)
        p += 16
        for _ in range(runs):
            off, ln = struct.unpack_from('<2I', b, p)
            image[off:off + ln] = b[p + 8:p + 8 + ln]
            p += 8 + ln
        yield num, timer, image


def look(img, ids):
    """what a frame holds of a family: its object (CurAction, HP, MaxHP, AI index, its AttackVars' words) or None, the
    objects of the third and fourth kinds in play that it holds (its AttackVars name them) or that name it their owner
    (+0x4C) (address: kind), BN6's delete counts, the rare path's byte and the battle's phase"""
    o = next((t1_of(img, i) for i in ids if t1_of(img, i) is not None), None)
    me, owned = None, {}
    if o is not None:
        ai = X.u32(img, o + T1_AI)
        words = tuple(X.u32(img, ai + ATTACK_VARS + 4 * k) for k in range(ATTACK_WORDS)) if 0x02000000 <= ai < 0x02040000 else ()
        me = dict(at=o, action=X.u8(img, o + T1_ACTION), hp=X.u16(img, o + T1_HP), max=X.u16(img, o + T1_MAX_HP),
                  ai=X.u8(img, ai + AI_INDEX) if words else None, words=words, nid=X.u16(img, o + X.NAME_ID))
        for base, size, kind in ((X.T3_OBJECTS, X.OBJ_SIZE, 3), (X.T4_OBJECTS, X.T4_SIZE, 4)):
            for k in range(X.OBJ_COUNT):
                a = base + k * size
                if X.u8(img, a) & 1 and (X.u32(img, a + OWNER) == o or a in words):
                    owned[a] = kind
    lo, hi = DELETE_COUNTS
    return dict(me=me, owned=owned, counts=bytes(img[X.off_of(lo):X.off_of(hi)]), rare=X.u8(img, RARE_BYTE),
                phase=X.u8(img, X.BATTLE_PHASE), stopped=bool(X.u16(img, BATTLE_FLAGS) & TIME_STOP))


def plain_run(fam):
    """its V1 alone, MegaMan idle (he cannot die: --dev god): its record as BN6 spawns it, its states, what it holds"""
    v1 = f'{fam["ids"][0]:X}@{fam["panel"]}'
    path, lines = shot('families', f'{fam["name"].lower()}-plain', v1, ['1500:'], KEYS_FRAMES + 1600, 'god')
    return path, lines


def status_run(fam, kind, spec, at):
    """its V1 given a status `at` frames into the battle (the status step, BN6's engine taking it on its next frame)"""
    v1 = f'{fam["ids"][0]:X}@{fam["panel"]}'
    keys = [f'{at}:', f'0:status {spec} {fam["ids"][0]:X}', f'{BACK_WITHIN + AGAIN_WITHIN}:']
    return shot('families', f'{fam["name"].lower()}-{kind}', v1, keys, KEYS_FRAMES + at + BACK_WITHIN + AGAIN_WITHIN + 100, 'god')


def delete_run(fam, v):
    """version v alone at (5,2), MegaMan shooting along his row and the rows beside it, then along the middle row alone
    (where a family that comes to his row, TinHawk, is hit only as it dives along it: BN5's own, tests/xdiff/
    tinhawk_v1_keys; two steps up and one down first, whatever row a hit left him in), --dev onehit; god and 9999 HP (a later version's hit takes more than his 100 at once, before god
    gives them back), the rare path's gate set"""
    keys = ['30:'] + (['2:B', '22:'] * 6 + ['4:UP', '20:'] + ['2:B', '22:'] * 6 + ['4:DOWN', '10:', '4:DOWN', '20:'] + ['2:B', '22:'] * 6 +
                      ['4:UP', '20:']) * 4 + ['4:UP', '20:', '4:UP', '20:', '4:DOWN', '20:'] + ['2:B', '18:'] * 150 + ['600:']
    before = [f'0:flags {RARE_GATE:#x} {RARE_GATE:#x} 1']
    k = before + keys
    return shot('families', f'{fam["name"].lower()}-delete{v}', f'{fam["ids"][v]:X}@52', k, 7200, 'onehit,god,hp=9999')


def families_spec(c, a):
    """each family moved: its V1's record and states, the four statuses each releasing what it holds, each version deleted
    with BN6's delete counts and its rare versions' byte untouched (CanGard's checks above, for every family)"""
    import concurrent.futures as cf
    fams = families(a.family)
    with cf.ThreadPoolExecutor(JOBS) as pool:
        plains = dict(zip((f['name'] for f in fams), pool.map(plain_run, fams)))
        jobs = {}
        for fam in fams:
            path, lines = plains[fam['name']]
            # (a status where it holds an object of its own: the first such frame of the plain run 30 fight frames in or
            # more, by the battle's timer, the step's wait counted from the Custom screen's OK, FIGHT_AFTER frames before
            # the timer runs; else 120 frames in. Not while it runs a chip's family, CurAction 0x10 on: the engine's chip
            # code, whose time freeze, Zomon's AreaGrab's, holds every status until it ends)
            # (nor while time stands still after it, BN6_TIME_STOP: Zomon's AreaGrab ends its user's attack in its first frame,
            # BN5's support family's timing, its time freeze on)
            # (a mark's status where it stands in its first state of its own, CurAction 8: a state of a family's may take the
            # hit's bits itself before BN6's status step reads them, Appley's 11 among them)
            at = mark_at = 120 + FIGHT_AFTER
            found = mark_found = False
            if path:
                for num, timer, img in frames_of(path):
                    seen = look(img, fam['ids'])
                    if timer < 30 or not seen['me']:
                        continue
                    if not found and seen['owned'] and seen['me']['action'] < 0x10 and not seen['stopped']:
                        at, found = timer + FIGHT_AFTER + 2, True
                    if not mark_found and seen['me']['action'] == 8:
                        mark_at, mark_found = timer + FIGHT_AFTER + 2, True
                    if found and mark_found:
                        break
            fam['status_at'] = at
            for kind, spec in STATUS_SPECS:
                jobs[(fam['name'], kind)] = pool.submit(status_run, fam, kind, spec, at)
            for kind, spec, _ in MARK_SPECS:
                jobs[(fam['name'], kind)] = pool.submit(status_run, fam, kind, spec, mark_at)
            for v in range(6):
                jobs[(fam['name'], f'delete{v}')] = pool.submit(delete_run, fam, v)
        done = {k: j.result() for k, j in jobs.items()}
    for fam in fams:
        name, ids = fam['name'], fam['ids']
        print(f'== {name} (AI {fam["ai6"]}, ids {ids[0]:#x}-{ids[-1]:#x}, its V1 at {fam["panel"]})')
        family_plain(c, fam, plains[name])
        for kind, spec in STATUS_SPECS:
            family_status(c, fam, kind, done[(name, kind)])
        for kind, _, mark in MARK_SPECS:
            family_mark(c, fam, kind, mark, done[(name, kind)])
        for v in range(6):
            family_delete(c, fam, v, done[(name, f'delete{v}')])


def family_plain(c, fam, run):
    path, lines = run
    if not c(path is not None, f'{fam["name"]}: its V1 run' + ('' if path else f': {lines[-1:]}')):
        return
    first, acts, held = None, [], 0
    for _, _, img in frames_of(path):
        f = look(img, fam['ids'])
        if f['me'] is None:
            continue
        if first is None:
            first = dict(f['me'])
        if not acts or acts[-1] != f['me']['action']:
            acts.append(f['me']['action'])
        held += bool(f['owned'])
    own = sorted({x for x in acts if 8 <= x < 0x10})
    chips = sorted({x for x in acts if x >= 0x10})   # (a held chip's family, run from the families of chips' table: CurAction 0x10 on)
    c(first is not None and first['ai'] == fam['ai6'] and first['hp'] == fam['hp'][0],
      f'{fam["name"]} V1 spawned as AI {first["ai"] if first else None} with {first["hp"] if first else None} HP ({fam["ai6"]}, '
      f'{fam["hp"][0]}: BN5\'s struct2)')
    c(set(acts) - set(chips) <= set(range(fam['states'])) and len(own) >= 1,
      f'{fam["name"]}: its CurActions within its table of {fam["states"]} (generic 0-7, its own from 8): {own} of its own'
      f'{f", a held chip used ({chips})" if chips else ""}, {len(acts)} changes; an object of its own out on {held} frames')


def family_status(c, fam, kind, run):
    path, lines = run
    name = fam['name']
    if not c(path is not None, f'{name} {kind}: its run' + ('' if path else f': {lines[-1:]}')):
        return
    took = next((int(l.rsplit(' ', 1)[1]) for l in lines if l.startswith(f'status: {kind}') and ' at battle frame ' in l), None)
    seen = [(timer, look(img, fam['ids'])) for _, timer, img in frames_of(path)]
    i = next((k for k, (timer, _) in enumerate(seen) if timer == took), None)
    if not c(i is not None and i + 1 < len(seen) and seen[i][1]['me'], f'{name} {kind}: the step at battle frame {took}, '
             f'{name} in play'):
        return
    after = seen[i + 1][1]['me']
    c(after is not None and after['action'] == STATUS_ACTION[kind],
      f'{name} {kind}: CurAction {after["action"] if after else None:} the frame after ({STATUS_ACTION[kind]})')
    # (what it holds as the status takes it: its AttackVars' words that name an object of its own then in play)
    me, owned = seen[i][1]['me'], seen[i][1]['owned']
    held = {k: w for k, w in enumerate(me['words']) if w in owned}
    dangling = []
    for j in range(i + 1, min(i + 4, len(seen))):
        f = seen[j][1]
        if f['me'] is None:
            continue
        for k, w in held.items():
            if k < len(f['me']['words']) and f['me']['words'][k] == w and w not in f['owned']:
                dangling.append((j - i, k, w))
    c(not dangling, f'{name} {kind}: what it held let go and forgotten ({len(held)} of its AttackVars\' words named its own '
      f'objects: {sorted(held)}{"; still named once let go: " + str(dangling[:3]) if dangling else ""})')
    back = next((j for j in range(i + 1, len(seen)) if seen[j][1]['me'] and seen[j][1]['me']['action'] >= 8), None)
    c(back is not None and back - i <= BACK_WITHIN, f'{name} {kind}: back in its own states {back - i if back else None} frames on')
    acts = 0
    for j in range(back + 1 if back else len(seen), len(seen)):
        a0, a1 = seen[j - 1][1]['me'], seen[j][1]['me']
        acts += bool(a0 and a1 and a0['action'] != a1['action']) or bool(set(seen[j][1]['owned']) - set(seen[j - 1][1]['owned']))
    c(acts >= 2, f'{name} {kind}: acting again after it ({acts} changes of state or objects of its own spawned)')
    last = seen[-1][1]
    c(last['me'] is not None and last['phase'] == 0x0C, f'{name} {kind}: the battle on to its last frame ({len(seen)} traced), '
      f'{name} in it')


def s32(v):
    return v - (1 << 32) if v & 0x80000000 else v


def marks_of(img, ids, mark):
    """the marks of this kind over the family's object in a frame: (dx, dz, its sign), each its place less its owner's in
    pixels, the sign of its side (0x0800E2CE: its alliance XOR its flip); None where the family is not in play"""
    me = look(img, ids)['me']
    if me is None:
        return None
    o, out = me['at'], []
    for k in range(X.OBJ_COUNT):
        a = X.T4_OBJECTS + k * X.T4_SIZE
        if not X.u8(img, a) & 1 or X.u8(img, a + 1) != MARK_TYPE or X.u32(img, a + OWNER) != o or X.u8(img, a + 4) != mark:
            continue
        sign = -(((X.u8(img, o + 0x16) ^ X.u8(img, o + 0x17)) << 1) - 1)
        out.append(((s32(X.u32(img, a + 0x34)) - s32(X.u32(img, o + 0x34))) >> 16,
                    (s32(X.u32(img, a + 0x3C)) - s32(X.u32(img, o + 0x3C))) >> 16, sign))
    return out


def family_mark(c, fam, kind, mark, run):
    """the status's mark over it (the fourth kind's 6 of this kind, its owner it) the frames after the step, each at its arm
    point 5 as BN5 has it: x its point's times its side's sign, z its point's height"""
    path, lines = run
    name = fam['name']
    if not c(path is not None, f'{name} {kind}: its run' + ('' if path else f': {lines[-1:]}')):
        return
    took = next((int(l.rsplit(' ', 1)[1]) for l in lines if l.startswith(f'status: {kind}') and ' at battle frame ' in l), None)
    seen = [(timer, marks_of(img, fam['ids'], mark)) for _, timer, img in frames_of(path)]
    i = next((k for k, (timer, _) in enumerate(seen) if timer == took), None)
    if not c(i is not None, f'{name} {kind}: the step at battle frame {took}'):
        return
    px, py = fam['arm5']
    found = [(j - i, m) for j in range(i + 2, min(i + 8, len(seen))) for m in (seen[j][1] or ())]
    if (name, kind) in NO_MARK:
        c(not found, f'{name} {kind}: no mark ({NO_MARK[(name, kind)]}): {len(found)} frames with one')
        return
    wrong = [(k, dx, dz) for k, (dx, dz, sign) in found if (dx, dz) != (px * sign, py)]
    c(found and not wrong, f'{name} {kind}: its mark over it at its arm point 5, BN5\'s ({px}, {py}): {len(found)} frames'
      f'{", off at " + str(wrong[:2]) if wrong else ""}')


def family_delete(c, fam, v, run):
    path, lines = run
    name, ident = fam['name'], fam['ids'][v]
    if not c(path is not None, f'{name} version {v}: its run' + ('' if path else f': {lines[-1:]}')):
        return
    first, died, res, counts, rare, frames = None, False, None, set(), set(), 0
    for k, (_, _, img) in enumerate(frames_of(path)):
        f = look(img, [ident])
        frames += 1
        if f['me'] and first is None:
            first = dict(f['me'])
        died |= bool(f['me'] and f['me']['action'] == 2)
        if res is None and f['phase'] == X.PHASE_RESULTS:
            res = k
        counts.add(f['counts'])
        rare.add(f['rare'])
    c(first is not None and first['max'] == fam['hp'][v], f'{name} version {v} (id {ident:#x}): spawned, its MaxHP '
      f'{first["max"] if first else None} (BN5\'s struct2: {fam["hp"][v]})')
    c(died and res is not None, f'{name} version {v}: deleted (CurAction 2), the results from traced frame {res}')
    lo, hi = DELETE_COUNTS
    c(len(counts) == 1, f'{name} version {v}: BN6\'s delete counts {lo:08x}-{hi - 1:08x} unchanged through its {frames} frames')
    c(len(rare) == 1, f'{name} version {v}: the rare path\'s byte {RARE_BYTE:08x} unchanged ({sorted(rare)}) through its {frames} frames')


SPECS = {'fight': fight, 'status': status, 'delete': delete, 'native': native, 'families': families_spec}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--spec', choices=sorted(SPECS) + ['all'], default='all')
    ap.add_argument('--family', nargs='*', type=str.lower, help='the families spec\'s families by name (all where none)')
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
