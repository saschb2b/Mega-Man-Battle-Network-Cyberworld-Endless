#!/usr/bin/env python3
"""xproxy.py [--seed S] [--story] [--quiet] [--no-run] [--mutations] [--souls]: the Souls' proxy form's
self-differential (issue #82, the Souls track's S1; docs/MULTIROM.md,
Transplanted Souls; docs/DEVTOOLS.md, The proxy form).

BN5's Souls will stand behind one form of BN6's own: form 8, Falzar's
TenguCross, which Gregar keeps in every table by form and never offers.
--dev soulproxy makes it an exact copy of HeatCross (src/emu/soulproxy.c:
entry 8 of each table by form written from entry 1, its NameID's object id
HeatCross's, hooks where Gregar's code reads the form itself), and the
dev step "cross 8" commits the next Cross chosen on CROSSSELECT as form 8
through CROSSSELECT's own commit. Four headless runs of the host build
(build.py shot), each from the same boot, layer and keys, each a battle
the foes step starts against a StarFish in the bottom row (an Aqua virus:
its bubbles are HeatCross's weakness), each traced (CYBERWORLD_BTRACE:
EWRAM and IWRAM after every frame of the battle, src/emu/battle_trace.c):

  native     no proxy, HeatCross chosen on CROSSSELECT
  installed  the proxy installed, HeatCross chosen: the install leaves
             BN6's own HeatCross alone, every byte
  proxy      the proxy installed, HeatCross chosen and committed as form 8:
             the same frame for frame as installed but the form byte and
             its copies (1 against 8), MegaMan's NameID (0x1AC against
             0x1B3) and the used bit (the row's, HeatCross's, in both)
  tengu      no proxy, committed as form 8: Gregar's own form 8, Falzar's
             TenguCross; it must differ from installed past those (the
             comparison sees what the copy changes)

Through the battle: the crossing (CROSSSELECT, the summon, the overlay),
a charged shot (HeatCross's flame), a chip (FireBrn1, HeatCross's Fire
bonus: a hooked reader), a buster shot from the row above (his arm's
flash: a hooked reader), a step into the StarFish's row, whose bubble
breaks the cross with a weakness hit, MegaMan's buster after it, and the
battle's end to its results (--dev onehit: the virus falls to one hit, in
all four runs). Each run is also checked to have done each of those. With
--mutations, each part of the install is left out in a run of its own
(--dev proxybreak=N): each the battle reaches must part the proxy from
HeatCross (its NameID's object id the battle never reads for him in a
Cross: his object is set up as a battle begins, in no form in a run;
reported, not counted). Needs
the ROM (local only), copied into .build/xproxy/rom alone (a layer of
BN5's would fight its battles in BN5's engine). Exits 0 when the proxy
is HeatCross frame for frame and tengu is not.

--souls runs the same four with the Souls installed over the proxy (the
Souls track's S2: UNITE on the Custom screen and a Soul's life, src/emu/
soulunite.c and soullife.c, which --dev soulproxy installs where BN5's ROM
is read beside BN6's), no Soul held: BN5's ROM beside BN6's in
.build/xproxy-souls/rom, the seed one whose layer is BN6's own (15: a
seed of a dressed layer fights its battles in BN5's engine, and the
checks say so), each install's line required. The same verdicts then say
that the Souls' install with no Soul held leaves BN6's battle alone every
byte (native against installed: the Custom screen, the turn's change, a
weakness hit), and that the proxy plays as HeatCross through a Soul's
life's two stubs, which its overlay entries 8 then point at (installed
against proxy). With --mutations, each break of the proxy must still
part it from HeatCross with the Souls over it (where a break leaves its
tables not HeatCross's, the Souls' install refuses to stand on it).
"""
import argparse
import functools
import hashlib
import os
import shutil
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xcontrol as X  # noqa: E402  (its trace reader, the image's places and the ROM's find)

ROOT = X.ROOT
WORK = os.path.join(ROOT, '.build', 'xproxy')
BN5_SHA1 = '5f472f78d8de2df01d5039e045c043cb40969a39'   # Team Colonel (USA), as tools/xdiff.py finds it
SOULS_SEED = 15                                          # --souls: a seed whose layer of --net-biome 0 BN5 does not dress
SOULS_LINE = "soulunite: UNITE on the Custom screen, the Souls' life on the proxy form"

# (src/emu/bn6.h, bn6_forms.h)
BATTLE_NAVI, FORM_AT = 0x0203CE00, 0x2C          # BN6_BATTLE_NAVI, BN6_BATTLE_FORM's offset
CROSS_RECORD, CROSS_PENDING, CROSSES_USED = 0x0203CED0, 0x0203A980, 0x020349A0
TURN_PACKETS = 0x020367F0                         # the turn's packets the record passes through (0x080147E4)
LINK_COPY_NAVI, LINK_COPY_RECORD = 0x0203CC34, 0x0203CC98   # NaviStats and the record copied for a link (0x0800B3A2)
MEGAMAN = X.T1_OBJECTS                            # his battle object, the first
NAME_ID = MEGAMAN + X.NAME_ID
HEAT, PROXY = 1, 8
CHIP = 0x2A                                       # BN6_T1_CHIP: the chip he uses next
FIREBRN1 = 0x14
FLAME = 0x27                                      # his CurAction firing a flame: HeatCross's charged shot and FireBrn1, one family
STARFISH = 0x55

# the places the form is kept: each a byte the form chosen is written to or copied into on its way to MegaMan (a battle
# against the game's own viruses passes the turn's record through the link's buffers too, and copies it to both sides)
SEND, RECEIVE = 0x02036780, (0x020399F0, 0x02039A00)       # the turn's send buffer and each side's receive buffer (0x0801FED2)
SIDE_PACKETS = (0x0203F558, 0x0203F658)                     # each side's packet as the turn starts (0x0800840C)
FORM_PLACES = {
    BATTLE_NAVI + FORM_AT: 'his battle NaviStats\' form (written as he crosses, 0x08014C5C)',
    CROSS_RECORD: 'the Custom screen\'s record of the form chosen (0x08015952)',
    LINK_COPY_RECORD: 'the record\'s copy for a link (0x0800B3A2)',
    SEND + 8: 'the turn\'s send buffer (the record in it)',
    RECEIVE[0] + 8: 'side 0\'s receive buffer (the send buffer copied to both, 0x0801FED2)',
    RECEIVE[1] + 8: 'side 1\'s receive buffer (the same copy)',
    SIDE_PACKETS[0]: 'side 0\'s packet as the turn starts (0x0800840C)',
    SIDE_PACKETS[1]: 'side 1\'s packet (the same record, mirrored)',
    TURN_PACKETS + 8: 'side 0\'s turn packet (0x080147E4)',
    TURN_PACKETS + 0x18: 'side 1\'s turn packet (0x080147E4)',
    CROSS_PENDING: 'side 0\'s pending form (0x080147E4)',
    CROSS_PENDING + 0x10: 'side 1\'s pending form (0x080147E4: no player of side 1 takes it)',
    LINK_COPY_NAVI + FORM_AT: 'the NaviStats\' copy for a link (0x0800B3A2)',
}

KEYS_BEFORE = '300:,' + ','.join(['4:A,56:'] * 10)   # MegaMan's arrival words closed (tools/xcontrol.py's)
CUSTOM = ['240:', '3:UP', '30:', '3:A', '90:', '3:A', '20:', '3:START', '8:', '3:A']   # CROSSSELECT: HeatCross, its first chip, OK
FIGHT = ['230:', '100:B', '100:', '4:A', '70:', '3:UP', '20:', '3:B', '30:', '3:DOWN', '20:', '3:DOWN', '300:'] + ['3:B', '7:'] * 60 + ['600:']   # a charged shot, the chip, a shot from the row above, the StarFish's row, its bubble, the buster


def keys(cross):
    k = KEYS_BEFORE.split(',') + ['0:flags 0xE2 0xE2 1'] + ([f'0:cross {PROXY}'] if cross else []) + \
        [f'0:foes {STARFISH:02X}@53'] + CUSTOM + FIGHT
    return ','.join(k)


def run(name, dev, cross, seed, quiet):
    """one run traced: its btrace.bin's path"""
    data = os.path.join(WORK, name)
    os.makedirs(data, exist_ok=True)
    for f in os.listdir(data):
        if f in ('btrace.bin',) or f.endswith('.bmp'):
            os.remove(os.path.join(data, f))
    boot = os.path.join(WORK, 'boot-3.state')
    if os.path.exists(boot):
        shutil.copy(boot, os.path.join(data, 'boot-3.state'))
    sub = os.path.basename(WORK)
    shots = ','.join(f'{t}:/src/.build/{sub}/{name}/f{t}.bmp' for t in (1250, 1330, 1400, 1520, 1700, 1900, 2100))
    cmd = [sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--data-dir', f'/src/.build/{sub}/{name}', '--scene', 'emu',
           '--net-biome', '0', '--seed', str(seed), '--dev', dev, '--frames', '4400', '--input', keys(cross), '--shot', shots]
    env = dict(os.environ, CYBERWORLD_ROM_DIR=os.path.join(WORK, 'rom'), CYBERWORLD_BTRACE='1')
    env.pop('CYBERWORLD_XDRAWS', None)
    out = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True)
    lines = [l for l in out.stdout.splitlines() + out.stderr.splitlines() if l.startswith(('soulproxy', 'cross', 'foes', 'soulunite', 'soullife'))]
    if not quiet:
        for l in lines:
            print(f'  {name}: {l}')
    if out.returncode:
        sys.exit(f'{name}: the run failed\n' + out.stderr[-2000:])
    # (a break of --mutations that leaves the proxy's tables not HeatCross's has the Souls refuse it: soullife.c checks them first)
    if SOULS and 'soulproxy' in dev and 'proxybreak' not in dev and SOULS_LINE not in lines:
        sys.exit(f'{name}: the Souls were not installed over the proxy (no "{SOULS_LINE}")')
    if not os.path.exists(boot) and os.path.exists(os.path.join(data, 'boot-3.state')):
        shutil.copy(os.path.join(data, 'boot-3.state'), boot)
    return os.path.join(data, 'btrace.bin')


@functools.lru_cache(maxsize=None)
def souls_copies():
    """--souls: BN6's tables the Souls' install grows into copies a run with them reads instead (src/emu/soul_room.h), each
    (BN6's table, its copy, BN6's bytes of it): the emotion window's faces' tiles' words and their palettes, BN6's 23
    faces first in each (src/emu/soullook.c's faces_copy)"""
    import xdiff_snap
    d = xdiff_snap.c_defines('src/emu/free_space.h', 'src/emu/soul_room.h', 'src/emu/bn6_forms.h')
    faces = d['SOUL_FACE_FIRST']
    return ((d['BN6_FACES'], d['SOULS_FACES'], 4 * faces), (d['BN6_FACE_PALS'], d['SOULS_FACE_PALS'], 0x20 * faces))


def souls_pointer(off, ia, ib):
    """a word pointing into BN6's table in one run and the same place of the Souls' copy in the other (the same bytes)"""
    w = off & ~3
    va, vb = struct.unpack_from('<I', ia, w)[0], struct.unpack_from('<I', ib, w)[0]
    for table, copy, n in souls_copies():
        for x, y in ((va, vb), (vb, va)):
            if table <= x < table + n and y == copy + (x - table):
                return 'a pointer into a table the Souls\' install grew (BN6\'s bytes at the same place of its copy)'
    return None


@functools.lru_cache(maxsize=None)
def souls_mood():
    """--souls: the addresses of BN5's mood the Souls keep in their own block beside BN6's (src/emu/soulmood.c: each
    side's opening mark, mood and dark latch in BN6_SOULS), which a battle with them installed writes whoever holds a
    Soul and only their hooks read"""
    import xdiff_snap
    d = xdiff_snap.c_defines('src/emu/bn6_forms.h')
    return frozenset(d['BN6_SOULS'] + side * d['BN6_SOULS_SIDE'] + d[f] for side in (0, 1)
                     for f in ('BN6_SOUL_OPENED', 'BN6_SOUL_MOOD', 'BN6_SOUL_DARK'))


def why(off, ia, ib, ma, mb, allowed):
    """Why the byte at `off` differs between two runs' images, or None: xcontrol's (a pointer into what each run moved,
    BN6's interrupt stack, dead between frames), with --souls a pointer into a table the Souls grew, read through its
    copy, and BN5's mood in the Souls' own block, and with `allowed` the proxy's own: a place the form is kept, 1 against 8; MegaMan's NameID, 0x1AC against
    0x1B3; the used bits' word (its row's in both, so far)"""
    reason = X.why(off, ia, ib, ma, mb, False) or (SOULS and souls_pointer(off, ia, ib))
    if not reason and SOULS and X.addr_of(off) in souls_mood():
        reason = 'BN5\'s mood, kept in the Souls\' block beside BN6\'s (read by their hooks alone)'
    if reason or not allowed:
        return reason or None
    addr, a, b = X.addr_of(off), ia[off], ib[off]
    if addr in FORM_PLACES and (a, b) == (HEAT, PROXY):
        return 'the form byte: ' + FORM_PLACES[addr]
    if addr == NAME_ID and (a, b) == (0xAC, 0xB3):
        return 'his NameID (0x1AC, 0x1B3)'
    if CROSSES_USED <= addr < CROSSES_USED + 4:
        return 'the used bit'
    return None


def compare(a, b, ma, mb, allowed):
    """frame by frame, every byte: ({reason: {address: frames}}, [(frame, address, a's, b's)] unexplained)"""
    explained, bad = {}, []
    n = min(len(a), len(b))
    if len(a) != len(b):
        bad.append((n, 0, len(a), len(b)))
    for i in range(n):
        ia, ib = a[i][2], b[i][2]
        if ia == ib:
            continue
        off = 0
        while off < X.IMAGE:
            k = off
            while k < X.IMAGE and ia[k:k + 64] == ib[k:k + 64]:
                k += 64
            off = k
            if off >= X.IMAGE:
                break
            if ia[off] != ib[off]:
                reason = why(off, ia, ib, ma, mb, allowed)
                if reason:
                    seen = explained.setdefault(reason, {})
                    seen[X.addr_of(off)] = seen.get(X.addr_of(off), 0) + 1
                elif len(bad) < 200:
                    bad.append((a[i][0], X.addr_of(off), ia[off], ib[off]))
            off += 1
    return explained, bad


def timeline(frames):
    """what the battle did, from a run's trace: the frames his form was each value, his CurActions, the chips he used,
    the flames he fired in a form (CurAction 0x27 begun: the Cross's charged shot, FireBrn1), his HP lost, the results"""
    forms, acts, chips, hits, results, flames = {}, set(), set(), [], None, []
    was_hp, was_form, was_act = None, 0, None
    for num, timer, img in frames:
        form = X.u8(img, BATTLE_NAVI + FORM_AT)
        forms.setdefault(form, num)
        mm = next((o for o in X.objects(img) if o[0] == 'T1' and o[1] == 0), None)
        if mm:
            acts.add(mm[2])
            if mm[2] == FLAME and was_act != FLAME:
                flames.append((num, form))
            was_act = mm[2]
            chip = X.u16(img, MEGAMAN + CHIP)
            if chip != 0xFFFF and chip:
                chips.add(chip)
            if was_hp is not None and mm[5] < was_hp:
                hits.append((num, was_hp - mm[5], was_form, form))
            was_hp = mm[5]
        was_form = form
        if results is None and X.u8(img, X.BATTLE_PHASE) == X.PHASE_RESULTS:
            results = num
    return dict(forms=forms, acts=acts, chips=chips, hits=hits, results=results, flames=flames)


def checks(name, tl, form):
    """the battle did what the spec is for: crossed into `form`, fired the Cross's charged shot (CurAction 0x27, its
    flame), used FireBrn1, lost the Cross to a hit, reached its results"""
    bad = []
    if form not in tl['forms']:
        bad.append(f'never in form {form}')
    if 0x1C not in tl['acts']:
        bad.append('never crossed (CurAction 0x1C)')
    if sum(1 for _, f in tl['flames'] if f == form) < 2:
        bad.append(f'not two flames in form {form} (CurAction 0x27: the charged shot, then FireBrn1)')
    if FIREBRN1 not in tl['chips']:
        bad.append('no FireBrn1 in his hand')
    if not any(f0 == form and f1 == 0 for _, _, f0, f1 in tl['hits']):
        bad.append(f'no hit that broke form {form}')
    if tl['results'] is None:
        bad.append('no results')
    print(f'  {name}: form {form} from frame {tl["forms"].get(form)}, flames at {tl["flames"][:4]}, hits {tl["hits"][:4]}, '
          f'chips {sorted(tl["chips"])}, results from frame {tl["results"]}' + (f': {", ".join(bad)}' if bad else ''))
    return not bad


def story(frames):
    last, lines = None, []
    for num, timer, img in frames:
        objs = X.objects(img)
        mm = next((o for o in objs if o[0] == 'T1' and o[1] == 0), None)
        view = (X.u8(img, BATTLE_NAVI + FORM_AT), mm and mm[2], mm and mm[5], tuple((o[6], o[5]) for o in objs if o[0] == 'T1' and o[1]))
        if view != last:
            lines.append(f'  f{num:4d} t{timer:5d} ph {X.u8(img, X.BATTLE_PHASE):02X}: form {view[0]} act {view[1]} hp {view[2]} foes {view[3]}')
            last = view
    return lines


# the comparator's own test (--mutations): each part of the install left out (--dev proxybreak=N, soulproxy.h's
# SOULPROXY_BREAK_*), the proxy run must part from HeatCross where the battle reaches the part; one it does not reach
# says why (its run the same as HeatCross's is then what is expected, and it counts for nothing)
MUTATIONS = (
    (1, 'the tables by form left TenguCross\'s', None),
    (2, 'its NameID\'s object id left TenguCross\'s',
     'the battle makes none of the reads of his NameID\'s triple for him in a Cross: his object is set up as a battle '
     'begins, and every battle of a run begins in no form (docs/ROM_DATA.md, MegaMan\'s forms)'),
    (4, 'the chip bonus by form not hooked: FireBrn1 without HeatCross\'s +50', None),
    (8, 'the arm flash\'s look not hooked: TenguCross\'s arm as he fires', None),
    (16, 'the summon not hooked: TenguMan behind him as he crosses', None),
)


def mutations(a, installed):
    """each break of mutations run as the proxy and compared with installed: whether every one the battle reaches
    was caught"""
    caught = reached = 0
    for bit, what, unreached in MUTATIONS:
        name = f'mutation{bit}'
        path = os.path.join(WORK, name, 'btrace.bin') if a.no_run else run(name, f'soulproxy,proxybreak={bit},hp=1000,folder=20,onehit', True, a.seed, True)
        mb, fb = X.read_trace(path)
        _, bad = compare(installed[1], fb, installed[0], mb, True)
        first = f': first at frame {bad[0][0]} {bad[0][1]:08x} ({bad[0][2]:02x} | {bad[0][3]:02x})' if bad else ''
        if unreached:
            print(f'mutation {bit}, {what}: {"caught" if bad else "not caught"}{first}; not reached by this battle: {unreached}')
            continue
        print(f'mutation {bit}, {what}: {"caught" if bad else "NOT CAUGHT"}{first}')
        caught += bool(bad)
        reached += 1
    print(f'{caught} of {reached} mutations the battle reaches caught ({len(MUTATIONS) - reached} not reached)')
    return caught == reached


SOULS = False


def find_bn5():
    """BN5 Team Colonel's ROM beside BN6's (by its SHA-1), for --souls"""
    for name in sorted(os.listdir(X.ROM_DIR)):
        path = os.path.join(X.ROM_DIR, name)
        if name.lower().endswith('.gba') and os.path.isfile(path):
            with open(path, 'rb') as f:
                if hashlib.sha1(f.read()).hexdigest() == BN5_SHA1:
                    return path
    sys.exit(f'no BN5 Team Colonel ROM in {X.ROM_DIR} (--souls needs it)')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--story', action='store_true', help='the proxy run\'s battle, frame by frame')
    ap.add_argument('--quiet', action='store_true')
    ap.add_argument('--no-run', action='store_true', help='compare the traces of the last runs')
    ap.add_argument('--mutations', action='store_true', help='also each part of the install left out: each must part the proxy from HeatCross')
    ap.add_argument('--souls', action='store_true', help='the Souls installed over the proxy (BN5\'s ROM beside BN6\'s), no Soul held')
    a = ap.parse_args()
    global WORK, SOULS
    SOULS = a.souls
    if a.souls:
        WORK = os.path.join(ROOT, '.build', 'xproxy-souls')
        if a.seed == 1:
            a.seed = SOULS_SEED
    os.makedirs(os.path.join(WORK, 'rom'), exist_ok=True)
    rom = os.path.join(WORK, 'rom', 'bn6.gba')
    if not os.path.exists(rom):
        shutil.copy(X.find_rom(), rom)
    rom5 = os.path.join(WORK, 'rom', 'bn5.gba')
    if a.souls and not os.path.exists(rom5):
        shutil.copy(find_bn5(), rom5)
    if not a.no_run:
        subprocess.run([sys.executable, os.path.join(ROOT, 'build.py'), 'host'], cwd=ROOT, check=True, capture_output=True)
    base = 'hp=1000,folder=20,onehit'
    runs = {}
    for name, dev, cross in (('native', base, False), ('installed', 'soulproxy,' + base, False), ('proxy', 'soulproxy,' + base, True),
                             ('tengu', base, True)):
        path = os.path.join(WORK, name, 'btrace.bin') if a.no_run else run(name, dev, cross, a.seed, a.quiet)
        runs[name] = X.read_trace(path)
        print(f'{name}: {len(runs[name][1])} frames of battle traced')
    if a.story:
        print('the battle (proxy):')
        print('\n'.join(story(runs['proxy'][1])))
    ok = all(checks(name, timeline(runs[name][1]), form) for name, form in (('installed', HEAT), ('proxy', PROXY)))
    for la, lb, allowed, want_same in (('native', 'installed', False, True), ('installed', 'proxy', True, True), ('installed', 'tengu', True, False)):
        (ma, fa), (mb, fb) = runs[la], runs[lb]
        explained, bad = compare(fa, fb, ma, mb, allowed)
        print(f'{la} against {lb}: {min(len(fa), len(fb))} frames, every byte of EWRAM and IWRAM: {len(bad)} differences unexplained'
              + ('' if want_same else ' (it must have some: Gregar\'s own form 8 is not HeatCross)'))
        for reason, where in sorted(explained.items()):
            spots = ' '.join(f'{addr:08x}x{n}' for addr, n in sorted(where.items())[:8])
            print(f'  {reason}: {len(where)} bytes ({spots}{" ..." if len(where) > 8 else ""})')
        for frame, addr, x, y in bad[:12]:
            print(f'  unexplained: frame {frame} {addr:08x}: {x:02x} | {y:02x}')
        ok = ok and (not bad) == want_same
    print('the proxy form: ' + ('HeatCross frame for frame' if ok else 'DIFFERENT'))
    if a.mutations:
        ok = mutations(a, runs['installed']) and ok
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
