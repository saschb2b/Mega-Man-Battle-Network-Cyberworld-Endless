"""xdiff_bn4.py: the differential's BN4 view, for tools/xdiff.py (docs/devtools/differential.md, BN4).

A spec whose "game" is "bn4" fights BN4 Red Sun's own battle on the guest core, traced as BN5's is but in BN6's units
(src/dev/xdiff_bn4_trace.h: each part read from BN4's layout and written at BN6's offsets, BN4's other bytes kept), and
compares it frame by frame by the battle's timer:

  "kind": "control"   BN4's battle fought twice, each in a process of its own (game=bn4:trace): the two traces must be
                      the same at every frame, every byte, the pipeline's own check (its keys, its timer, its seeding);
                      with "again" (items of the spec's the second run takes instead) they must part, the comparator's
                      own check, and the spec passes where they do
  "against": "bn6"    BN4's battle against BN6's with the foes by BN6's ids (game=bn4:bn6): a family BN4 and BN6 both
                      have, or (to come) a BN4 family moved into BN6's engine

Objects are matched by role (xdiff_snap.assign_roles: an enemy by its id, an attack by its type, an effect by its type
and id), their bytes from their list node on, their AI's data and their collision data compared as BN6 lays them out,
a pointer by what it names (xdiff_snap.Frame.ptr); the panels, the RNG words, the songs begun and the run order each
frame. BN4's numbers are its own (its enemy ids, CurActions, sounds and effects: no map of BN4's to BN6's yet), so the
comparison against BN6 holds where the two games number alike. The fields BN4 has none of are masked (BN4_LACKS, each
with why), and only where BN4's trace holds nothing there; every other difference fails the spec at its first frame."""
import os

from xdiff_snap import NODE, OBJ_NAME_ID, SIZE, Diff, Frame, assign_roles, is_ptr, read_trace, role_name, u32

# The fields of BN6's battle BN4's has none of, in BN6's units (part, BN6's offset, bytes): each masked where BN4's trace
# holds none (zero, or no AI data at all) and the other is BN6's, never where BN4 holds something there
BN4_LACKS = [
    dict(name='bn4 no ai index', part='ai', offset=0x01, size=1,
         why="BN6's AI data +1 is the AI index its virus dispatcher runs a family's AI by (bn6.h, BN6_T1_AI_DATA; xvirus_dispatch.h); "
             "BN4's viruses have no AI data at all (bn4.h, BN4_T1_AI_DATA: 0 on a virus; its one AI data a side is MegaMan's and "
             "a Navi's, BN4_AI_DATA), its families' routines run by their object's type alone (BN4_T1_TYPES)"),
    dict(name='bn4 no ai data', part='ai', offset=0x00, size=0, present=True,
         why="a virus's AI data, BN6's own (its spawner gives each virus one of its eight, BN6_AI_DATA_SIZE), which BN4's viruses "
             "have none of (bn4.h, BN4_T1_AI_DATA: 0 on a virus): BN4 keeps a virus's state in its object"),
    dict(name='bn4 no ai data pointer', part='obj', offset=0x58, size=4,
         why="a virus's pointer to that AI data (bn6.h, BN6_T1_AI_DATA), 0 on every BN4 virus (bn4.h, BN4_T1_AI_DATA), as above"),
    dict(name='bn4 no freeze', part='coll', offset=0x2A, size=2,
         why="BN6's collision data +0x2A, the frames left frozen in an ice block (bn6.h, BN6_COLL_FREEZE): BN4 has no freeze "
             "(bn4.h, BN4_COLL_INVULNERABLE: its statuses paralysis, confusion, blindness and the hold, +0x10-+0x17)"),
    dict(name='bn4 no bubble', part='coll', offset=0x2C, size=2,
         why="BN6's collision data +0x2C, the frames left in a bubble (bn6.h, BN6_COLL_BUBBLE): BN4 has no bubble (bn4.h, as the "
             "freeze)"),
]


def lacks(d, ta, tb):
    """the BN4_LACKS mask that explains d (BN4's side a, BN6's b), None where none does"""
    if ta['game'] != 4 or tb['game'] != 6:
        return None
    for m in BN4_LACKS:
        if d.part != m['part']:
            continue
        if m.get('present') and d.off == 0 and d.a is False and d.b is True:
            return m['name']
        if not m.get('present') and m['offset'] <= d.off < m['offset'] + m['size'] and d.a in (0, None, ('raw', 0)):
            return m['name']
    return None


def named(canon, game, o):
    """an object's bytes from its node with its NameID as BN6 numbers it (canon.nid: BN4's MegaMan, enemy id 0, BN6's)"""
    rec = bytearray(o.rec[:SIZE[o.kind]])
    if o.kind == 'T1':
        at = NODE + OBJ_NAME_ID
        rec[at:at + 2] = canon.nid(game, rec[at] | rec[at + 1] << 8).to_bytes(2, 'little')
    return rec


def cells(fa, fb, part, ba, bb, start=0):
    """the places two parts' bytes differ, a word that is a pointer in either by what it names (Frame.ptr: an object by
    its role and part, a panel; a pointer into a ROM as it is, BN4's ROM's data not yet read as BN6's)"""
    out, k = [], start
    while k < max(len(ba), len(bb)):
        if k % 4 == 0 and k + 4 <= min(len(ba), len(bb)):
            wa, wb = u32(ba, k), u32(bb, k)
            if is_ptr(wa) or is_ptr(wb):
                view = fa.ptr(wa), fb.ptr(wb)
                if view[0] != view[1]:
                    out.append((part, k, view[0], view[1]))
                k += 4
                continue
        a, b = ba[k] if k < len(ba) else None, bb[k] if k < len(bb) else None
        if a != b:
            out.append((part, k, a, b))
        k += 1
    return out


def frame_diffs(fa, fb):
    """each difference of one frame: the objects by role, the RNG words, the songs, the run order, the panels"""
    t, out = fa.fr['t'], []
    ra, rb = {o.role: o for o in fa.fr['objs']}, {o.role: o for o in fb.fr['objs']}
    for role in sorted(set(ra) | set(rb), key=role_name):
        oa, ob = ra.get(role), rb.get(role)
        if oa is None or ob is None:
            out.append(Diff(t, role, 'in play', 0, oa is not None, ob is not None))
            continue
        # (from its node's +8: its links compared as the run order)
        found = [(p, off - NODE, a, b) for p, off, a, b in cells(fa, fb, 'obj', named(fa.canon, fa.game, oa), named(fb.canon, fb.game, ob), 8)]
        for part, xa, xb in (('ai', oa.ai, ob.ai), ('coll', oa.coll, ob.coll)):
            if (xa is None) != (xb is None):
                found.append((part, 0, xa is not None, xb is not None))
            elif xa is not None:
                found += cells(fa, fb, part, xa, xb)
        out += [Diff(t, role, part, off, a, b) for part, off, a, b in found]
    if fa.fr['rng'] != fb.fr['rng']:
        out.append(Diff(t, 'battle', 'rng', 0, fa.fr['rng'], fb.fr['rng']))
    sa, sb = sorted(s for _, s in fa.fr['sounds']), sorted(s for _, s in fb.fr['sounds'])
    if sa != sb:
        out.append(Diff(t, 'battle', 'sounds', 0, sa, sb))
    oa, ob = [fa.ptr(x) for x in fa.fr['order']], [fb.ptr(x) for x in fb.fr['order']]
    if oa != ob:
        out.append(Diff(t, 'battle', 'run order', 0, oa, ob))
    for i, (pa, pb) in enumerate(zip(fa.fr['panels'], fb.fr['panels'])):
        out += [Diff(t, f'panel {i % 8},{i // 8}', part, off, a, b) for part, off, a, b in cells(fa, fb, 'panel', pa, pb)]
    return out


def said(d, tb):
    """a difference as a line, each side by its game"""
    fmt = lambda v: f'{v:#x}' if isinstance(v, int) and not isinstance(v, bool) else str(v)
    who = role_name(d.role) if isinstance(d.role, tuple) else d.role
    return f't{d.t} {who} {d.part}+{d.off:#04x}: BN4 {fmt(d.a)} | BN{tb["game"]} {fmt(d.b)}'


def compare(spec, ta, tb, canon, show=6):
    """('PASS'|'FAIL', the frame of it, lines): the first frame with a difference no BN4_LACKS mask explains fails it"""
    bt, used, n = {f['t']: f for f in tb['frames']}, {}, 0
    for fra in ta['frames']:
        frb = bt.get(fra['t'])
        if frb is None:
            continue
        n += 1
        fa, fb = Frame(ta, fra, canon), Frame(tb, frb, canon)
        diffs = frame_diffs(fa, fb)
        for d in diffs:
            d.mask = lacks(d, ta, tb)
            if d.mask:
                used[d.mask] = used.get(d.mask, 0) + 1
        open_ = [d for d in diffs if not d.mask]
        if open_:
            return 'FAIL', fra['t'], [f'  {n} fight frames compared; first unexplained difference at t{fra["t"]}:'] + \
                [f'    {said(d, tb)}' for d in open_[:show]] + ([f'    ... {len(open_) - show} more'] if len(open_) > show else [])
    lines = [f'  {n} fight frames compared (t{ta["frames"][0]["t"]}-t{ta["frames"][-1]["t"]}), masks applied: ' +
             (', '.join(f'{k} x{v}' for k, v in sorted(used.items())) or 'none')]
    if len(ta['frames']) != len(tb['frames']):
        lines.append(f'  the traces are {len(ta["frames"])} and {len(tb["frames"])} fight frames long')
        return 'FAIL', min(len(ta['frames']), len(tb['frames'])), lines
    return 'PASS', None, lines


def spec_text(spec, out, game):
    """the spec's items as src/dev/xdiff_spec.c takes them, its battle BN4's (game: bn4:trace, bn4:bn6)"""
    keys = ','.join(f'{t}:{k}:{n}' for t, k, n in spec.get('keys', []))
    parts = [f'game={game}', f'foes={spec["foes"]}', f'mm={spec.get("mm", "22")}', f'hp={spec.get("hp", 100)}',
             f'frames={spec.get("frames", 1200)}', f'seed={spec.get("seed", "12345678:9ABCDEF0")}', f'out={out}']
    return ';'.join(parts + ([f'keys={keys}'] if keys else []) + (['safe=1'] if spec.get('safe') else []))


def one(name, spec, a, canon, run_xdiff, work):
    """one BN4 spec run and compared: its verdict and kind. run_xdiff(text) runs the host build's --xdiff (tools/xdiff.py)"""
    runs = os.path.join(work, 'runs')
    control = spec.get('kind') == 'control'
    if control:
        paths = [os.path.join(runs, f'{name}{k}.bn4') for k in ('', '_again')]
        # (the second run's spec the first's but for its "again" items: the control broken on purpose, which must FAIL)
        for k, sp in (('', spec), ('_again', dict(spec, **spec.get('again', {})))) if not a.no_run else ():
            run_xdiff(spec_text(sp, f'/src/.build/xdiff/runs/{name}{k}', 'bn4:trace'))
    else:
        paths = [os.path.join(runs, f'{name}.{g}') for g in ('bn4', 'bn6')]
        if not a.no_run:
            run_xdiff(spec_text(spec, f'/src/.build/xdiff/runs/{name}', 'bn4:bn6'))
    ta, tb = read_trace(paths[0]), read_trace(paths[1])
    for tr in (ta, tb):
        assign_roles(tr, canon)
    verdict, at, lines = compare(spec, ta, tb, canon, a.show)
    print('\n'.join(lines))
    kind = 'control' if control else 'bn4'
    what = 'against BN6\'s' if not control else 'fought twice, the second as "again" changes it, which must part' \
        if spec.get('again') else 'fought twice, its traces the same'
    if spec.get('again'):   # (the comparator's own test: the change must be found, at the frame it shows)
        verdict = 'PASS' if verdict == 'FAIL' else 'FAIL'
    print(f'{name}: {verdict}{f" at t{at}" if at is not None else ""} ({kind}: BN4\'s battle {what})')
    return verdict, kind
