"""xnavirecipe_bn4_init.py: what a BN4 Navi's init does past BN6's Navi init, read for tools/xnavirecipe_bn4.py
(docs/multirom/bn4-navis.md, WindMan built), WindMan's first:

- its companions set out at its spawn: each a call of a spawner in a first-kind type's block of BN4's (r4 its kind), that
  type moved with it (its block walked from its routine, run through the second-level dispatch's first kind), the words
  its init stores after the spawn read (the companion's +o given the Navi's +v, the Navi's +o given the companion), all
  made again by a companion start of its home (XnCompanion; src/emu/xnavi_install.c) where BN6's Navi init calls one;
- its collision's flags (BN4's set_flag, 0x08012F82, r0 a constant): BN6's struct2 flags that make the same
  (bn4_twins.INIT_FLAGS), its region (the setup's r1) the one BN6's actor init gives by them (0x08016FD8-0x08016FE4);
- its init's call of its update, noted (BN6's Navi init runs none);
- its own generic states (its table's 0-5 its own where BN4's generic Navi state is not): walked as its own, each in its
  place of BN6's table;
- its sounds named by a movs and an adds of 0xFF (BN4's sounds past 0xFF): the pool's slot by the movs, the adds made 0.

Numbers alone: addresses in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA)."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_twins  # noqa: E402
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

SET_FLAG = X4.BN4['set_flag']   # BN4's 0x08012F82: r0 ORed into its collision data's +0x64 (BN6's twin 0x0801A152, LAYOUT)
SPAWN1 = 0x080032B8             # BN4_SPAWN_1: the first kind's spawner, r0 the type, r4 the parameters
REGION6 = (1, 0x10)             # BN6's actor init's collision region: 1, or 0x10 where its struct2's flags have 4 (0x08016FD8)


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


def stores_after(f, w, site, nxt):
    """the words its init stores after a companion's spawn at `site`, to the next call `nxt`: (link, keep), link (v, o) the
    companion's +o given the Navi's +v, keep o the Navi's +o given the companion (0 none); None where an instruction is
    another"""
    link, keep, r1 = None, 0, None
    for a in sorted(x for x in w.insns if site < x < nxt):
        i = w.insns[a]
        if (i.form == 'tst' and i.f == (0, 0)) or (i.form == 'bcc' and i.f == (0,)) or (i.form == 'movs' and i.f[0] == 4):
            continue
        if i.form == 'movs' and i.f[0] == 1:
            r1 = ('imm', i.f[1])
        elif i.form == 'adds' and i.f == (1, 1, 5) and r1 and r1[0] == 'imm':
            r1 = ('navi', r1[1])
        elif i.form == 'str' and i.f[:2] == (1, 0) and r1 and r1[0] == 'navi' and link is None:
            link = (r1[1], i.f[2])
        elif i.form == 'str' and i.f[:2] == (0, 5) and not keep:
            keep = i.f[2]
        else:
            f.decide('init', f'{a:08X}: after its companion\'s spawn ({site:08X}) its init does {i.text}, which its companion start '
                             'does not make')
            return None
    return link or (0, 0), keep


def init_calls(b4, f, w, setup):
    """its init's calls past BN6's Navi init's (tools/xnavirecipe_bn4.py navi_init): each companion's spawn, its collision's
    flags, its update; the sites it answers"""
    done, f.companions, f.moved1 = set(), [], getattr(f, 'moved1', {})
    sites = sorted(w.bls)
    for k, site in enumerate(sites):
        t = w.bls[site]
        nxt = sites[k + 1] if k + 1 < len(sites) else max(w.insns) + 2
        if t == SET_FLAG:
            v, chain = T.back_value(w, site, 0)
            bits = {b: m for b, m in bn4_twins.INIT_FLAGS.items() if v is not None and v & b}
            if v is None or len(chain) != 1 or sum(bits) != v:
                f.decide('init', f'{site:08X}: its collision\'s flags {v} made by {chain}, not BN6\'s struct2 flags (INIT_FLAGS)')
                continue
            f.flags |= sum(bits.values())
            f.notes.append(f'{site:08X}: its collision\'s flags {v:#x}: BN6\'s struct2 flags {sum(bits.values()):#x}, which its actor '
                           'init sets alike (bn4_twins.INIT_FLAGS)')
            done.add(site)
        elif t == f.update:
            f.notes.append(f'{site:08X}: its init calls its update (its first state run in its init\'s frame): BN6\'s Navi init runs '
                           'none, its first a frame later')
            done.add(site)
        elif companion_spawner(b4, t):
            ty, lo, hi = companion_spawner(b4, t)
            kind, chain = T.back_value(w, site, 4)
            st = stores_after(f, w, site, nxt)
            if kind is None or len(chain) != 1 or chain[0][1] != 'movs' or st is None:
                f.decide('init', f'{site:08X}: its companion of first-kind {ty:#x} spawned with r4 {kind} ({chain})')
                continue
            f.companions.append(dict(at=site, spawner=t, kind=kind, link=st[0], keep=st[1], ty=ty))
            if ty not in f.moved1:
                f.moved1[ty] = dict(lo=lo, hi=hi, routine=lo)
                f.regions.append((lo, hi, f'T1:{ty:#x}'))
                f.regions_roots.append(lo)
            f.roots.append(t)
            f.notes.append(f'{site:08X}: its companion, BN4\'s first-kind {ty:#x} of kind {kind} (its spawner {t:08X}), moved; its '
                           f'+{st[0][1]:#x} given its Navi\'s +{st[0][0]:#x}{f", its Navi" + chr(39) + f"s +{st[1]:#x} given it" if st[1] else ""}')
            done.add(site)
    region6 = REGION6[1] if f.flags & 0x04 else REGION6[0]
    if setup and (setup[0], setup[1]) != (region6, 2):
        f.decide('init', f'its collision set up with r1 {setup[0]} and r2 {setup[1]}: BN6\'s actor init gives {region6} and 2 by its '
                         f'flags {f.flags:#x}')
    return done


def spawns1(f, alloc):
    """each spawn of a first-kind type it moves (its companions'): the dispatch's number of its own (the movs that names it
    changed), its call to the dispatch's spawner; xrecipe_bn4.spawns_of's decision for it answered"""
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
                m['as'] = alloc.types[kind]
                alloc.types[kind] += 1
                f.types.append((kind, m['as'], ty))
            f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=ty, now=m['as'],
                               why=f'its companion\'s type {ty:#x} (the first kind): moved, the dispatch\'s {m["as"]:#x}'))
            f.spawn_calls[t] = name
            f.decisions = [d for d in f.decisions if d != ('type', f'{site:08X}: it spawns a 1 type {ty:#x} which it does not move')]


def own_generic(f, states, generic4, generic6):
    """its table's 0-5 its own where BN4's generic Navi state is not: walked as its own in BN6's place of that state (its
    call into BN4's generic state of that place paired with BN6's, bn4_twins' audited pairs); its other states' places
    BN4's generic ones (a decision of tools/xnavirecipe_bn4.py)"""
    own = []
    for k in range(6):
        if k < len(states) and states[k] != generic4[k] and f.lo <= states[k] & ~1 < f.hi:
            f.states[k] = ('OWN', states[k] | 1)
            own.append(states[k] & ~1)
            f.notes.append(f'its state {k} its own ({states[k] & ~1:08X}), in BN6\'s place of its generic {k} ({generic6[k] & ~1:08X})')
    f.decisions = [d for d in f.decisions if not (d[0] == 'states' and any(d[1].startswith(f'its state {k} ') for k in range(6)
                                                                            if f.states[k][0] == 'OWN'))]
    return own


def sounds_wide(f, pool):
    """its sounds past 0xFF named by a movs and an adds of 0xFF (BN4's play's r0): each its pool slot's where that is a byte,
    the movs the slot and the adds 0; xrecipe_bn4.sounds_of's decision answered"""
    keep = []
    for d in f.decisions:
        if d[0] != 'sound' or 'named otherwise than by a movs' not in d[1]:
            keep.append(d)
            continue
        parts = d[1].split()
        v, site = int(parts[2], 16), int(parts[4], 16)
        w = next(w for w in f.walks.values() if site in w.bls)
        _, chain = T.back_value(w, site, 0)
        slot = pool.get(v)
        if slot is None or slot > 0xFF or len(chain) != 2 or chain[0][1] != 'adds8' or chain[1][1] != 'movs':
            keep.append(d)
            continue
        f.imms.append(dict(at=chain[1][0], form='THUMB_MOVS', rd=0, rn=0, was=chain[1][2], now=slot,
                           why=f'its sound {v:#x} (a movs and an adds): BN4\'s copied into BN6\'s slot {slot:#x}'))
        f.imms.append(dict(at=chain[0][0], form='THUMB_ADDS', rd=0, rn=0, was=chain[0][2], now=0,
                           why=f'its sound {v:#x}: the adds that took it past 0xFF, 0 (its slot by the movs alone)'))
    f.decisions = keep


# The routines of a companion's moved type no companion of a Navi moved reaches in a plain battle: not walked, its bytes kept
# as BN4's raw ones (its jump table's entry still moved to them), an interim each: entry -> why
CUT = {
    0x080CCDC8: "the first kind's 0x5C's state 4, step 0 (its sprite drawn again through BN4's tile buffer 0x02037810, a "
                "copy of 0x0800087C's and 0x08003578's): its init takes state 4 only where the battle state's +0x32 has no "
                "bit 0 (0x08007A8C) and its settings' +6 has 0x20 (0x080CCD1E-0x080CCD2E), else 0 or 8; kept as BN4's bytes, "
                "unported (assumed unreached in a plain battle)",
}


def cut_before(b4, f):
    """the routines CUT, walked aside so that the walk of its code passes them by"""
    f.cut = {a: T.walk(b4, a, limit=0x80) for a in CUT if any(m['lo'] <= a < m['hi'] for m in getattr(f, 'moved1', {}).values())}
    f.walks.update(f.cut)


def cut_after(f):
    """the walks set aside let go (their bytes BN4's raw ones in its block), each an interim"""
    for a in f.cut:
        del f.walks[a]
        f.interim.append(f'{a:08X}: {CUT[a]}')


def c_companions(n, f):
    """its companions as XnCompanion rows (src/emu/xnavi_recipe.h): its spawner its own code, its kind, the words stored"""
    rows = ''.join(f'\t{{ OWN(0x{c["spawner"] | 1:08X}), {c["kind"]}, 0x{c["link"][0]:02X}, 0x{c["link"][1]:02X}, 0x{c["keep"]:02X} }},   '
                   f'/* {c["at"]:08X}: its first-kind {c["ty"]:#x} of kind {c["kind"]} */\n' for c in f.companions)
    return f'static const XnCompanion {n}_companions[] = {{\n{rows}}};'
