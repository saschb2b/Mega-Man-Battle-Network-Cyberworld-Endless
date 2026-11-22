"""xnavirecipe_bn4_init.py: what a BN4 Navi's init does past BN6's actor init, read for tools/xnavirecipe_bn4.py, in one
reading for every Navi (docs/multirom/bn4-navis.md, Companions):

- its spawns: each a call of a routine of a first-kind type's block that spawns that type (BN4_SPAWN_1, r0 a movs), its
  registers read (r4 a constant made by a movs, lsls and adds: FireMan's sprite word, WindMan's kinds; r0 hi << 16 | a
  table's byte by its version with r1 its version: ThunderMan's), the words its init keeps after it (the companion's +o
  given the Navi's +v, the Navi's +o given the companion); the type moved with it (its block walked from its routine,
  spawned through the second-level dispatch's first kind at a number of its own), all made again by a shim of its home
  (XnSpawn, XnInit; src/emu/xnavi_init.c);
- where: at BN6's companion start (its actor init's call after its collision set up, 0x08017064) where its init spawns them
  after its own collision's set-up; at its spawn's end (its table 7) where it spawns one before (FireMan's, WindMan's: BN6's
  companion start is past their place either way, its spawn's end past the whole of BN6's init, as theirs is past all its
  init's reads), or where its init updates its sprite (which BN6's actor init does after its companion start);
- its init's other work: its +0x11 written (FireMan's 0, BN6's actor init's 0xFF: its +0x10 given at its spawn's end), its
  sprite updated, its update run (BN6's Navi frame at its spawn's end), its collision's flags (BN4's set_flag, r0 a
  constant: BN6's struct2 flags that make the same, bn4_twins.INIT_FLAGS, its region the setup's r1), its panel kept at its
  +6 (AquaMan's, his decision's last panel: HOME_FORM);
- its own generic states (its table's 0-5 where BN4's generic Navi state is not), each in its place of BN6's table;
- its companions' interims: their routines no battle of theirs reaches kept as BN4's bytes (CUT, UNWALKED_TABLES), a mode
  run as another (COMPANION_MODES).

Imported by tools/xnavirecipe_bn4.py alone. Numbers alone: addresses in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA)."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_twins  # noqa: E402
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

SET_FLAG = X4.BN4['set_flag']   # BN4's 0x08012F82: r0 ORed into its collision data's +0x64 (BN6's twin 0x0801A152, LAYOUT)
SPAWN1 = 0x080032B8             # BN4_SPAWN_1: the first kind's spawner, r0 the type, r4 the parameters
REGION6 = (1, 0x10)             # BN6's actor init's collision region: 1, or 0x10 where its struct2's flags have 4 (0x08016FD8)
SPRITE_UPDATE4 = 0x0800285C     # BN4_SPRITE_UPDATE (BN6's BN6_SPRITE_UPDATE, 0x080026C4, which its spawn's end calls)
NAVI_FRAME6 = 0x080F367C        # BN6_NAVI_FRAME: its update at its spawn's end
# BN6's Navi generic states' names (bn6_navis.h), for the notes
GENERIC_NAVI6_NAMES = ('spawned', 'on to its first state', 'deleted', 'flinching', 'paralysed', 'dragged', 'frozen', 'bubbled')
# ThunderMan's form of its spawn's r0 (0x080C3868-0x080C3872), the instructions before the call: movs r0,#hi; lsls r0,#16;
# ldr r3,=table; ldrb r1,[r5,#4] (its version); ldrb r2,[r3,r1]; orrs r0,r2
TABLE_FORM = ('movs', 'lsls', 'ldr_lit', 'ldrb', 'ldrb_r', 'orrs')
# AquaMan's init's panel kept (0x080B8264): ldrh r0,[r5,#0x12] (its column and row, BN6's +0x12 too); strh r0,[r5,#6] (its
# spawn's +6 and +7, which BN6's spawn leaves 0 for a Navi and his decision reads as his last panel, 0x080B833E)
HOME_FORM = (('ldrh', (0, 5, 0x12)), ('strh', (0, 5, 6)))


# ---- its spawns ----

def t1_block(b4, t):
    """the first-kind type whose block holds `t` (its routine to the next type's): (type, lo, hi), else None"""
    types = [b4.u32(X4.BN4['t1'] + 4 * i) & ~1 for i in range(X4.BN4['nt1'])]
    starts = sorted(set(a for a in types if T.is_code_ptr(a | 1, b4)))
    lo = max((a for a in starts if a <= t), default=None)
    if lo is None or lo == starts[-1] or t - lo > 0x1000:
        return None
    tys = [k for k, a in enumerate(types) if a == lo]
    return (tys[0] if len(tys) == 1 else None), lo, starts[starts.index(lo) + 1]


def companion_spawner(b4, t):
    """`t` a routine of a first-kind type's block that spawns that type through BN4_SPAWN_1 (r0 the type, a movs): (type,
    lo, hi), else None"""
    hit = t1_block(b4, t)
    if hit is None or hit[0] is None:
        return None
    ty, lo, hi = hit
    w = T.walk(b4, t, region=(lo, hi))
    if not any(s == SPAWN1 and T.back_value(w, a, 0)[0] == ty for a, s in w.bls.items()):
        return None
    return hit


def const_chain(w, site, reg):
    """the number `reg` holds at `site` where the instructions before it make it by a movs, then lsls and adds of
    immediates (FireMan's r4 0x810: movs #8, lsls #8, adds #0x10); None where nothing before it writes it, or another"""
    ops = []
    for a in sorted((a for a in w.insns if a < site), reverse=True):
        i = w.insns[a]
        if not T.writes(i, reg):
            continue
        if i.form == 'movs' and i.f[0] == reg:
            v = i.f[1]
            for form, k in reversed(ops):
                v = (v << k if form == 'lsls' else v + k) & 0xFFFFFFFF
            return v
        if i.form == 'lsls' and tuple(i.f[:2]) == (reg, reg) or i.form == 'adds8' and i.f[0] == reg:
            ops.append((i.form, i.f[2] if i.form == 'lsls' else i.f[1]))
            continue
        return 'other'
    return None


def table_form(b4, w, site):
    """r0 at a spawn's call made as ThunderMan's init makes it (TABLE_FORM): (hi, the table), else None"""
    addrs = sorted(a for a in w.insns if a < site)[-len(TABLE_FORM):]
    run = [w.insns[a] for a in addrs]
    if tuple(i.form for i in run) != TABLE_FORM:
        return None
    if tuple(run[0].f[:1]) != (0,) or run[1].f[2] != 16 or tuple(run[3].f) != (1, 5, 4):
        return None
    table = b4.u32(run[2].lit)
    return (run[0].f[1], table) if table else None


def stores_after(f, w, site, nxt):
    """the words its init stores after a companion's spawn at `site`, to the next call `nxt`: (link, keep), link (v, o) the
    companion's +o given the Navi's +v, keep o the Navi's +o given the companion (0 none); its state's +8 4 (BN6's actor
    init writes the same) passed by; None where an instruction is another"""
    link, keep, r0, r1 = None, 0, 'it', None
    for a in sorted(x for x in w.insns if site < x < nxt):
        i = w.insns[a]
        if (i.form == 'tst' and i.f == (0, 0)) or (i.form == 'bcc' and i.f == (0,)) or (i.form == 'movs' and i.f[0] == 4):
            continue
        if i.form == 'movs' and i.f[0] == 1:
            r1 = ('imm', i.f[1])
        elif i.form == 'movs' and i.f == (0, 4):
            r0 = 4
        elif i.form == 'str' and i.f == (0, 5, 8) and r0 == 4:
            continue
        elif i.form == 'adds' and i.f == (1, 1, 5) and r1 and r1[0] == 'imm':
            r1 = ('navi', r1[1])
        elif i.form == 'str' and i.f[:2] == (1, 0) and r1 and r1[0] == 'navi' and link is None and r0 == 'it':
            link = (r1[1], i.f[2])
        elif i.form == 'str' and i.f[:2] == (0, 5) and not keep and r0 == 'it':
            keep = i.f[2]
        else:
            f.decide('init', f'{a:08X}: after its companion\'s spawn ({site:08X}) its init does {i.text}, which its shim does not make')
            return None
    return link or (0, 0), keep


def spawn_of(b4, f, w, site, t, nxt):
    """a companion's spawn of its init: its registers, its stores, its type moved with it (f.spawns, f.moved1)"""
    ty, lo, hi = companion_spawner(b4, t)
    r4, form, st = const_chain(w, site, 4), table_form(b4, w, site), stores_after(f, w, site, nxt)
    if st is None:
        return
    if r4 == 'other' or (r4 is None and form is None):
        f.decide('init', f'{site:08X}: its companion of first-kind {ty:#x} spawned with r4 {r4} and r0 not its version\'s table\'s')
        return
    s = dict(site=site, spawner=t, ty=ty, r4=r4, r4_6=r4, table=form[1] if form else 0, hi=form[0] if form else 0,
             link=st[0], keep=st[1], lo=lo, hi_end=hi)
    f.spawns.append(s)
    if ty not in f.moved1:
        f.moved1[ty] = dict(lo=lo, hi=hi, routine=lo)
        f.regions.append((lo, hi, ('1', ty)))
        f.regions_roots.append(lo)
    f.roots.append(t)
    regs = (f'r4 {r4:#x}' if r4 is not None else 'r4 as it is') + (f', r0 {s["hi"]:#x} << 16 | its table {s["table"]:08X}\'s byte by its '
                                                                  'version, r1 its version' if form else '')
    f.notes.append(f'{site:08X}: its companion, BN4\'s first-kind {ty:#x} ({t:08X}, {regs}), moved; its +{st[0][1]:#x} given its '
                   f'Navi\'s +{st[0][0]:#x}' + (f', its Navi\'s +{st[1]:#x} given it' if st[1] else ''))


def init_calls(b4, f, w, setup, setup_sites):
    """its init's calls past BN6's actor init's (tools/xnavirecipe_bn4.py navi_init): its companions' spawns, its collision's
    flags, its sprite updated, its update; f.init4 what its shims make (XnInit); the sites it answers"""
    done, f.spawns, f.moved1 = set(), [], getattr(f, 'moved1', {})
    work = dict(anim=False, sprite=False, frame=False, home=False)
    sites = sorted(w.bls)
    for k, site in enumerate(sites):
        t = w.bls[site]
        nxt = sites[k + 1] if k + 1 < len(sites) else max(w.insns) + 2
        if t == SET_FLAG:
            done.add(site) if collision_flags(f, w, site) else None
        elif t == f.update:
            work['frame'] = True
            f.notes.append(f'{site:08X}: its init runs its update (its first state in its spawn\'s frame): its spawn\'s end runs BN6\'s '
                           f'Navi frame ({NAVI_FRAME6:08X})')
            done.add(site)
        elif t == SPRITE_UPDATE4:
            work['sprite'] = True
            f.notes.append(f'{site:08X}: its init updates its sprite: its spawn\'s end updates it (BN6\'s 0x080026C4)')
            done.add(site)
        elif companion_spawner(b4, t):
            spawn_of(b4, f, w, site, t, nxt)
            done.add(site)
    if any(i.form == 'strb' and tuple(i.f[1:]) == (5, 0x11) for i in w.insns.values()):
        work['anim'] = True
        f.notes.append('its init writes its +0x11 (its animation set up again by its sprite step), where BN6\'s actor init writes '
                       '0xFF: its spawn\'s end gives it its +0x10')
    addrs = sorted(w.insns)
    if any(tuple((w.insns[b].form, tuple(w.insns[b].f)) for b in addrs[k:k + 2]) == HOME_FORM for k in range(len(addrs) - 1)):
        work['home'] = True
        f.notes.append('its init keeps its panel at its +6 (HOME_FORM), which BN6\'s spawn leaves 0: its spawn\'s end keeps it')
    coll = min(setup_sites) if setup_sites else None
    early = [s['site'] for s in f.spawns if coll is None or s['site'] < coll]
    at_end = bool(f.spawns) and (bool(early) or work['sprite'])
    if f.spawns:
        f.notes.append('its spawns made at its spawn\'s end (its table 7): ' + (
            f'its init spawns {", ".join(f"{a:08X}" for a in early)} before its collision\'s set-up' if early else
            'its init updates its sprite') if at_end else 'its spawns made at its companion start (BN6\'s actor init\'s, after its '
                                                          'collision set up, as its init makes them)')
    f.init4 = dict(work, at_end=at_end) if f.spawns or any(work.values()) else None
    region6 = REGION6[1] if f.flags & 0x04 else REGION6[0]
    if setup and (setup[0], setup[1]) != (region6, 2):
        f.decide('init', f'its collision set up with r1 {setup[0]} and r2 {setup[1]}: BN6\'s actor init gives {region6} and 2 by its '
                         f'flags {f.flags:#x}')
    return done


def collision_flags(f, w, site):
    """its init's set_flag (r0 a constant): BN6's struct2 flags that make the same (bn4_twins.INIT_FLAGS); True where so"""
    v, chain = T.back_value(w, site, 0)
    bits = {b: m for b, m in bn4_twins.INIT_FLAGS.items() if v is not None and v & b}
    if v is None or len(chain) != 1 or sum(bits) != v:
        f.decide('init', f'{site:08X}: its collision\'s flags {v} made by {chain}, not BN6\'s struct2 flags (INIT_FLAGS)')
        return False
    f.flags |= sum(bits.values())
    f.notes.append(f'{site:08X}: its collision\'s flags {v:#x}: BN6\'s struct2 flags {sum(bits.values()):#x}, which its actor init sets '
                   'alike (bn4_twins.INIT_FLAGS)')
    return True


def spawns1(f, alloc):
    """each spawn of a first-kind type it moves (its companions'): the dispatch's number of its own, taken in the order the
    Navis are derived (alloc.types[1], from the families' last: distinct across every Navi), the movs that names it changed,
    its call to the dispatch's spawner; xrecipe_bn4.spawns_of's decision for it answered"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t not in X4.SPAWNERS4 or X4.SPAWNERS4[t][0] != 1:
                continue
            kind, name = X4.SPAWNERS4[t]
            ty, chain = T.back_value(w, site, 0)
            if ty not in f.moved1 or len(chain) != 1 or chain[0][1] != 'movs':
                continue
            m = f.moved1[ty]
            if 'as' not in m:
                m['as'] = X4.type_number(f, alloc, kind)
                f.types.append((kind, m['as'], ty))
            f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=ty, now=m['as'],
                               why=f'its companion\'s type {ty:#x} (the first kind): moved, the dispatch\'s {m["as"]:#x}'))
            f.spawn_calls[t] = name
            f.decisions = [d for d in f.decisions if d != ('type', f'{site:08X}: it spawns a 1 type {ty:#x} which it does not move')]


def companion_art(b4, f, alloc):
    """a companion's sprite its type loads by its spawn's r4 (+4 the list's offset, +5 the number: FireMan's), where a load
    of its region is still open: a list 4 sprite copied as its objects' are, the r4 its shim passes given the copy's number"""
    for s in f.spawns:
        loads = [site for a, w in f.walks.items() if s['lo'] <= a < s['hi_end'] for site, t in w.bls.items() if t == X4.BN4['sprite_load']]
        if s['r4'] is None or not any(d[0] == 'sprite' and any(d[1].startswith(f'{x:08X}:') for x in loads) for d in f.decisions):
            continue
        lst, num = s['r4'] & 0xFF, s['r4'] >> 8 & 0xFF
        sp = X4.list4_sprite(b4, f, alloc, num) if lst == 0x10 else None
        if sp is None:
            f.decide('sprite', f'{s["site"]:08X}: its companion\'s sprite {lst // 4}:{num:#x}, which the generator moves none of')
            continue
        s['r4_6'] = s['r4'] & ~0xFF00 | sp['to_index'] << 8
        f.decisions = [d for d in f.decisions if not (d[0] == 'sprite' and any(d[1].startswith(f'{x:08X}:') for x in loads))]
        f.notes.append(f'its companion\'s sprite 4:{num:#x} at list 4 {sp["to_index"]:#x}: its spawn\'s r4 {s["r4"]:#x} made {s["r4_6"]:#x}')


def table_kinds1(f):
    """the kinds its companions are spawned with (their r4) of a first-kind type whose kinds' table bn4_twins.TABLES reads
    (the type's init loads it; AquaMan's 0x5C kind 1, WindMan's 2 and 3): table -> kinds"""
    out = {}
    for v, tab in bn4_twins.TABLES.items():
        mine = [s for s in f.spawns if s['lo'] <= tab['loads'][0] < s['hi_end'] and s['r4'] is not None]
        if mine:
            out[v] = tuple(sorted({s['r4'] & 0xFF for s in mine}))
            f.notes.append(f'its first-kind {mine[0]["ty"]:#x}\'s kinds by its table {v:08X}: {", ".join(str(k) for k in out[v])} '
                           '(its spawns\' r4)')
    return out


# ---- its own generic states ----

def wraps(b4, x, generic):
    """whether state `x` is only a call of BN4's generic Navi state `generic` (push {lr}; bl; pop {pc}: ThunderMan's 1)"""
    w = T.walk(b4, x & ~1, limit=0x10)
    return [w.insns[a].form for a in sorted(w.insns)] == ['push', 'bl', 'pop'] and list(w.bls.values()) == [generic & ~1]


def own_generic(b4, f, states, generic4, generic6):
    """its states 0-5: BN4's generic Navi state, BN6's of that place standing in, as for one only calling it (ThunderMan's
    1); or its own ending in a call of BN4's generic of that place (FireMan's 3-5, his companion told first; WindMan's 3
    and 4, his wind ended first), walked as its own in BN6's place, that call aimed at BN6's generic of the place"""
    for k in range(6):
        if k < len(states) and states[k] == generic4[k]:
            continue
        x = states[k] & ~1 if k < len(states) else None
        if x and wraps(b4, x, generic4[k]):
            f.notes.append(f'its state {k} ({x:08X}) a call of BN4\'s generic {k} alone: BN6\'s {k} ({GENERIC_NAVI6_NAMES[k]}) in its place')
            continue
        w = T.walk(b4, x, region=(f.lo, f.hi)) if x and f.lo <= x < f.hi else None
        if w is None or generic4[k] & ~1 not in w.bls.values():
            f.decide('states', f'its state {k} is its own, not BN4\'s generic Navi state nor a call of it')
            continue
        f.states[k] = ('OWN', x | 1)
        f.roots.append(x)
        f.aim[generic4[k] & ~1] = (generic6[k] & ~1, f'BN4\'s generic Navi state {k}, its own state {k}\'s tail: BN6\'s generic Navi '
                                                     f'state {k} ({GENERIC_NAVI6_NAMES[k]})')
        f.notes.append(f'its state {k} ({x:08X}) its own, ending in BN4\'s generic {k}: BN6\'s {k} ({GENERIC_NAVI6_NAMES[k]}) at its tail')


# ---- its companions' interims ----

# Routines of a companion's moved type no battle of its Navi reaches, not walked, its bytes kept as BN4's raw ones (its jump
# table's entry still moved to them), an interim each: entry -> why
CUT = {
    0x080CCDC8: "the first kind's 0x5C's state 4, step 0 (WindMan's companions': its sprite drawn again through BN4's tile buffer "
                "0x02037810, a copy of 0x0800087C's and 0x08003578's): its init takes state 4 only where the battle state's +0x32 "
                "has no bit 0 (0x08007A8C) and its settings' +6 has 0x20 (0x080CCD1E-0x080CCD2E), else 0 or 8; kept as BN4's bytes, "
                "unported (assumed unreached in a plain battle)",
}
# ... a table of sub-states not walked (FireMan's companion's mode 4: its appear through BN4's tile buffer, 0x02037810,
# 0x0800087C, 0x08003578, no BN6 twin read), its mode run as another (COMPANION_MODES: its mode 0, the appear waited on,
# 0x080CC5B4, then mode 8). Neither spec reaches it: in BN4's battle as the differential sets it up the bit is set and the
# companion starts in mode 8 (bn4_fireman_control's trace: its +9 8)
UNWALKED_TABLES = {0x080CC500: "FireMan's companion's mode 4's sub-states (0x080CC50C, 0x080CC584, 0x080CC5B4)"}
COMPANION_MODES = ((0x080CC464, 2, 4, 0), (0x080CCD2E, 2, 4, 0))   # (the movs, its register, BN4's mode, the mode run in its
# place: FireMan's companion's, WindMan's companions' (the first kind's 0x5C, its state 4 CUT))


def init_tail(f, w, done, std):
    """its init's own work past BN6's actor init's, where its init calls routines of its own block (BurnerMan's flames,
    0x080BDA30; MetalMan's gears): the instructions from the one past its last call BN6's actor init makes for it (its
    collision's set-up or flags) to its end, run at its spawn's end entered there (XnInit.tail: the spawn's end's shim
    pushes what its prologue pushed and a return, which its own pop takes); walked as one of its roots, its call of its
    update aimed at BN6's nullsub as the families' are (BN6's Navi frame at its spawn's end runs it). The address, 0 none"""
    f.init_tail, f.tail_push = 0, 0
    sites = sorted(w.bls)
    own = [a for a in sites if f.lo <= w.bls[a] < f.hi and w.bls[a] != f.update and a not in done]
    if not own:
        return 0
    engine = [a for a in sites if a < own[0] and (a in done or w.bls[a] in std)]
    if not engine:
        f.decide('init', f'{own[0]:08X}: its init\'s own calls before any of BN6\'s actor init\'s work')
        return 0
    tail = engine[-1] + 4
    late = [a for a in sites if a > tail and a not in done and w.bls[a] in std]
    first = w.insns[min(w.insns)]
    if late or first.form != 'push' or not first.text.startswith('push {') or 'lr' not in first.text:
        f.decide('init', f'{tail:08X}: its init\'s tail calls BN6\'s actor init\'s work again ({late}) or its prologue is not a push of lr')
        return 0
    regs = [r.strip() for r in first.text[first.text.index('{') + 1:first.text.index('}')].split(',')]
    f.tail_push = sum(1 << int(r[1:]) for r in regs if r.startswith('r'))
    f.init_tail = tail
    f.init4 = getattr(f, 'init4', None) or dict(at_end=False, anim=False, sprite=False, frame=False, home=False)
    f.init4['tail'] = tail
    f.notes.append(f'{tail:08X}: its init\'s own work past BN6\'s actor init\'s (its calls {", ".join(f"{w.bls[a]:08X}" for a in own)}) run at '
                   f'its spawn\'s end, entered there (XnInit.tail, its prologue\'s {regs} pushed by the shim)')
    return tail


def cut_before(b4, f):
    """the routines CUT, walked aside so that the walk of its code passes them by"""
    f.cut = {a: T.walk(b4, a, limit=0x80) for a in CUT if any(m['lo'] <= a < m['hi'] for m in f.moved1.values())}
    f.walks.update(f.cut)


def cut_after(f):
    """the walks set aside let go (their bytes BN4's raw ones in its block), each an interim"""
    for a in f.cut:
        del f.walks[a]
        f.interim.append(f'{a:08X}: {CUT[a]}')


def companion_modes(f):
    """the companion's modes run otherwise (COMPANION_MODES): each movs checked and given the mode run in its place"""
    for at, rd, was, now in COMPANION_MODES:
        w = next((w for w in f.walks.values() if at in w.insns), None)
        i = w.insns[at] if w else None
        if i is None or i.form != 'movs' or tuple(i.f) != (rd, was):
            if any(m['lo'] <= at < m['hi'] for m in f.moved1.values()):
                f.decide('companion', f'{at:08X}: not the movs r{rd},#{was} its companion\'s modes name')
            continue
        f.imms.append(dict(at=at, form='THUMB_MOVS', rd=rd, rn=0, was=was, now=now,
                           why=f'its companion\'s mode {was} (its appear through BN4\'s tile buffer, unported) run as its mode {now}'))


# ---- as C ----

def c_init(f, n):
    """its init's work as XnSpawn rows and its XnInit (src/emu/xnavi_recipe.h): (the arrays' text, its XNavi line), or ('', None)"""
    i = getattr(f, 'init4', None)
    if not i:
        return '', None
    rows = ''.join(
        f'\t{{ OWN(0x{s["spawner"] | 1:08X}), {"XN_R4_ANY" if s["r4_6"] is None else f"0x{s["r4_6"]:X}"}, 0x{s["table"]:08X}, '
        f'0x{s["hi"]:02X}, 0x{s["link"][0]:02X}, 0x{s["link"][1]:02X}, 0x{s["keep"]:02X} }},   /* {s["site"]:08X}: its first-kind '
        f'{s["ty"]:#x}{f", BN4 r4 {s["r4"]:#x}" if s["r4"] not in (None, s["r4_6"]) else ""} */\n' for s in f.spawns)
    spawns = f'{n}_spawns' if f.spawns else 'NULL'
    text = f'static const XnSpawn {n}_spawns[] = {{\n{rows}}};\n' if f.spawns else ''
    tail = i.get('tail')
    flags = ', '.join('true' if i[k] else 'false' for k in ('at_end', 'anim', 'sprite', 'frame') + (('home',) if i['home'] or tail else ()))
    if tail:
        flags += f', OWN(0x{tail | 1:08X}), 0x{f.tail_push:02X}'
    text += (f'static const XnInit {n}_init = {{ {spawns}, {len(f.spawns)}, {flags} }};   /* (at its spawn\'s end, its +0x11, its '
             f'sprite, its update{", its panel at its +6" if i["home"] else ""}{", its init\'s tail and its pushes" if tail else ""}) */\n')
    return text, f'\t.init = &{n}_init,   /* (its init\'s work past BN6\'s actor init: xnavi_init.c) */'


# ---- its update's calls before its runner, its records' list (JunkMan's; docs/multirom/bn4-navis.md, JunkMan built) ----

def navi_pres(f, uw):
    """its update's calls of its own between its pre-step and its runner (JunkMan's two): BN6's hook A (table 2, run after its
    pre-step, 0x080F368C, before its status dispatcher) aimed at a shim of its home that makes them in turn (XNavi.pre,
    src/emu/xnavi_init.c), each with its r0 where a movs gives it (a CurAction BN6's where the call compares it with its +9),
    else left as the call before leaves it"""
    f.pres = []
    for site, t in sorted(uw.bls.items())[1:-2]:
        v, chain = T.back_value(uw, site, 0)
        if v is not None and (len(chain) != 1 or chain[0][1] != 'movs'):
            f.decide('pre-step', f'{site:08X}: its call of {t:08X} with r0 from {chain}')
            continue
        if v is not None:
            w = T.walk(f.b4, t, region=(f.lo, f.hi))
            cur = {i.f[0] for i in w.insns.values() if i.form == 'ldrb' and tuple(i.f[1:]) == (5, 9)}
            if any(i.form == 'cmp' and len(i.f) == 2 and 0 in i.f and set(i.f) & cur for i in w.insns.values()):
                v = v + 2 if v >= 6 else v
            # (else a number of its own, as it is: BurnerMan's 0x60 and 0x68, the words of his flames' slots, 0x080BDD5E's
            # adds r4,r0,r5)
        f.pres.append((t, v))
    f.notes.append('its update calls ' + ', '.join(f'{t:08X}' + ('' if v is None else f' (r0 BN6\'s CurAction {v})') for t, v in f.pres) +
                   ' between its pre-step and its runner: BN6\'s hook A (table 2, which BN6\'s Navi frame runs after its pre-step, '
                   '0x080F368C), a shim of its home making them in turn (XNavi.pre)')


def record_list(b4, f):
    """records naming list 1 at its sprite's number, JunkMan's (f.record_list): BN4's preload, the records' sprite's one reader
    (0x08002B0E), unpacks an LZ77'd sprite alone (0x08002B44, bit 31), and list 1's of that number is raw, so nothing reads it"""
    f.record_list = 0
    p1, _ = X4.sprite(b4, X4.BN4['sprites'], 1, f.s1['index'])
    if all(r[3] == 4 and r[4] == f.s1['index'] for r in f.recs) and f.s1['list'] != 1 and p1 is not None and not p1 & 0x80000000:
        f.record_list = 1
        f.notes.append(f'its records name list 1 {f.s1["index"]:#x}, raw, which BN4\'s preload passes over (0x08002B44): its battle '
                       f'sprite its init\'s, list {f.s1["list"]} (XNavi.record_list)')
