#!/usr/bin/env python3
"""xdiff.py [SPEC ...] [--story] [--survey] [--poke A:V[:N]] [--call F:T]
[--mutations] [--expect-fail] [--no-run]: the differential of a virus
family moved into BN6's battle engine (issue #82; docs/MULTIROM.md,
Transplanted families; docs/DEVTOOLS.md, The differential).

BN5's CanGard and Handi run in BN6's battle engine by their own code,
moved from the player's ROM (src/emu/xvirus.c). Each spec
(tests/xdiff/SPEC.json) fights one battle twice with the host build's
--xdiff (src/dev/xdiff.c): in BN5's own engine on the guest core, as BN5
has it (the reference), and in BN6's with the families installed (the
subject), a frame of each in turn once each fights, each traced a fight
frame at a time (the battle's timer the clock in both, never the cores'
frames; the same keys by that timer; the same RNG words in both after the
fight's first frame, BN6's following BN5's each frame where the spec says
rng=follow, or each of BN6's draws given the word BN5's same draw took
where it says rng=replay, legit.json's twins passed along). Each core's
hooks log its draws, spawns, sounds, effects, setAttack, reservations,
blinks and sprites with their callers (src/dev/xdiff_events.c), read by
tools/xdiff_events.py. This compares the two:

  objects     matched by role, never by slot: the battle objects of the
              first kind by their enemy id, the third by their type, the
              fourth by their type (an effect object by its effect id too),
              each in the order it appeared; their bytes from their list
              node on (its links compared as the run order), their AI's
              data and their collision data, each through its own game's
              layout (legit.json, layouts: BN5's own parts of a word by
              their rule)
  numbers     each game's own numbers as BN6's (legit.json, maps: enemy
              ids, CurAction, the attack's type, the effect, the AI index,
              a song by its number, and the engine's own: a collision
              data's alliance words by the two games' tables, the
              renderer's places of the family's objects by their rank
              among the family's, an object's slot from its own); a
              pointer as what it names (an object
              by its role and its part, a slot let go, a panel, ROM that
              was moved by where it came from, each game's own ROM by the
              data there, a sprite's frames from its own data)
  per frame   the RNG words, the songs begun, the order the objects of
              both run in, the panels; the family's calls by their call
              sites (each event its code led to, in BN5's numbers), and
              under rng=replay each draw with no twin in the other game
  in BN6's    BN5's objects' reservation bit and blink phase read from
  terms       BN5's own events as BN6 would have written them; a sprite
              state no sprite was set up in left uncompared

Each difference must be explained: by a mask in tests/xdiff/legit.json
(an engine difference that is not the family's code, each with why and
the evidence, both games' routines by address; never on the family's own
records, where any difference fails) or it fails the spec, at the first
fight frame it shows. Whose a difference is comes from both traces: the
reference's word that a role is the family's, or the two traces
disagreeing, makes it the family's; the engine's spark over an actor,
lava's as it cools under it, is the panels' (BN6's engine's port of
BN5's lava and sea, src/emu/xpanels.c, under test as the family is). The
stop rule: where the first unexplained difference is MegaMan's own state
(his object and what he owns: his shots, his buster's flash, their
sparks) or an effect of the engine's, and nothing of the family's, the
spec is inconclusive from there (the battle is no longer the same battle
for the family), not failed; the family's objects are still compared
after it, to say how far they stayed the same. An enemy each game runs
its own code for (a spec's natives, their attacks and effects) does not
stop it: the family is compared on while its world (MegaMan, the RNG,
the songs, the run order, the panels) stays the same, and the first
difference there stops it. A spec's "expect" checks both traces against
what BN5's battle is known to do; its "both" writes into both games'
copies alike (an element given), its "mm_element" gives MegaMan one as
the fight begins. --poke and --call break BN6's copy after the install
(an immediate, a call's veneer; a poke in RAM after the boot, over the
code the game copied there): the comparator's own test, which
--mutations runs from tests/xdiff/mutations.json, each break required to
FAIL its spec. Needs both ROMs (local only), copied into
.build/xdiff/rom; the traces go to .build/xdiff/runs. Exits 0 when every
spec passes (a smoke spec may end inconclusive). The snapshot and its
differences are tools/xdiff_snap.py, whose a difference is and the masks
tools/xdiff_masks.py.
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from xdiff_snap import (ROOT, WORK, OBJ_NAME_ID, OBJ_ACTION, ROM as ROM_RANGE, Canon, Frame, assign_roles, frame_diffs,  # noqa: E402
                        read_trace, role_name, rom)
from xdiff_masks import attribute, mask_of, renderer_ranks, who  # noqa: E402
from xdiff_events import Translations, call_diffs, draw_diffs, matched_draws  # noqa: E402

SPECS = os.path.join(ROOT, 'tests', 'xdiff')
LEGIT = os.path.join(SPECS, 'legit.json')
MUTATIONS = os.path.join(SPECS, 'mutations.json')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
SHA1 = {'bn6.gba': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn5.gba': '5f472f78d8de2df01d5039e045c043cb40969a39'}
ROM_BASE = ROM_RANGE[0]


# ---- a spec ----

def find_roms():
    rom = os.path.join(WORK, 'rom')
    os.makedirs(rom, exist_ok=True)
    want = {k: v for k, v in SHA1.items() if not os.path.exists(os.path.join(rom, k))}
    for name in sorted(os.listdir(ROM_DIR)) if want else []:
        path = os.path.join(ROM_DIR, name)
        if name.lower().endswith('.gba') and os.path.isfile(path):
            with open(path, 'rb') as f:
                sha = hashlib.sha1(f.read()).hexdigest()
            for k, v in list(want.items()):
                if sha == v:
                    shutil.copy(path, os.path.join(rom, k))
                    del want[k]
    if want:
        sys.exit(f'no {" or ".join(want)} in {ROM_DIR} (BN6 Gregar and BN5 Team Colonel, USA)')
    return rom


def twins_check(legit):
    """Each twin of legit.json's rng is what it says: BN6's and BN5's bytes the same over its length, but where both hold a
    BL (to each game's own routine) or the same load from a literal pool (each game's own word); the twins as the spec's
    items (src/dev/xdiff.c)"""
    items = []
    for t in legit['rng']['twins']:
        a6, a5, n = (int(t[k], 0) for k in ('bn6', 'bn5', 'len'))
        b6, b5 = rom(6)[a6 - ROM_BASE:a6 - ROM_BASE + n], rom(5)[a5 - ROM_BASE:a5 - ROM_BASE + n]
        k = 0
        while k + 1 < n:
            h6, h5 = b6[k] | b6[k + 1] << 8, b5[k] | b5[k + 1] << 8
            if h6 >> 11 == 0x1E and h5 >> 11 == 0x1E and k + 3 < n:   # (a BL's two halves)
                k += 4
                continue
            if h6 != h5 and not (h6 >> 11 == 9 and h5 >> 11 == 9 and h6 & 0x700 == h5 & 0x700):
                sys.exit(f'legit.json\'s twin BN6 {a6:#x} / BN5 {a5:#x} differs at +{k:#x}: {h6:04x} | {h5:04x}')
            k += 2
        items.append(f'twin={a6:08X}:{a5:08X}:{n:X}' + (f':{t["word"]}' if 'word' in t else ''))
    return items


def spec_text(spec, out, pokes, calls=(), twins=()):
    keys = ','.join(f'{t}:{k}:{n}' for t, k, n in spec.get('keys', []))
    parts = [f'foes={spec["foes"]}', f'mm={spec.get("mm", "22")}', f'hp={spec.get("hp", 100)}', f'frames={spec.get("frames", 1200)}',
             f'seed={spec.get("seed", "12345678:9ABCDEF0")}', f'out={out}']
    if keys:
        parts.append(f'keys={keys}')
    if spec.get('rng'):
        parts.append(f'rng={spec["rng"]}')
    if spec.get('rng') == 'replay':
        parts += list(twins)
    if spec.get('field'):
        parts.append(f'field={spec["field"]}')
    if 'mm_element' in spec:
        parts.append(f'mm_element={spec["mm_element"]}')
    parts += [f'both={b}' for b in spec.get('both', [])]
    parts += [f'poke={p}' for p in pokes] + [f'call={c}' for c in calls]
    return ';'.join(parts)


def run(name, spec, pokes, calls, quiet, twins=()):
    os.makedirs(os.path.join(WORK, 'runs'), exist_ok=True)
    os.makedirs(os.path.join(WORK, 'data'), exist_ok=True)
    out = f'/src/.build/xdiff/runs/{name}'
    for g in ('bn5', 'bn6'):
        p = os.path.join(WORK, 'runs', f'{name}.{g}')
        if os.path.exists(p):
            os.remove(p)
    cmd = [sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--data-dir', '/src/.build/xdiff/data', '--xdiff',
           spec_text(spec, out, pokes, calls, twins)]
    env = dict(os.environ, CYBERWORLD_ROM_DIR=os.path.join(WORK, 'rom'))
    r = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True)
    lines = [l for l in (r.stdout + r.stderr).splitlines() if l.startswith(('xdiff', 'xvirus'))]
    if not quiet:
        for l in lines:
            print(f'  {l}')
    if r.returncode:
        sys.exit(f'{name}: the run failed\n' + r.stderr[-2000:])
    return [os.path.join(WORK, 'runs', f'{name}.{g}') for g in ('bn5', 'bn6')]


def story(trace, canon, upto=None):
    last, lines = None, []
    game = trace['game']
    for fr in trace['frames']:
        if upto and fr['t'] > upto:
            break
        parts = []
        for o in fr['objs']:
            if o.kind == 'T1':
                mm = canon.is_megaman(game, o.o16(OBJ_NAME_ID))
                parts.append(f'{role_name(o.role)} act {canon.action(game, mm, o.o8(OBJ_ACTION)):02X}.{o.o8(0x0A):02X} '
                             f'p{o.o8(0x12)}{o.o8(0x13)} hp{o.o16(0x24)}')
            elif o.kind == 'T3':
                parts.append(f'{role_name(o.role)} {o.o8(OBJ_ACTION):02X} p{o.o8(0x12)}{o.o8(0x13)}')
        snd = ''.join(f' snd {p}:{s:02X}' for p, s in fr['sounds'])
        view = '; '.join(parts)
        if view != last or snd:
            lines.append(f'  t{fr["t"]:5d}: {view}{snd}')
            last = view
    return lines


# ---- expectations: the timeline BN5's battle is known to have (in BN6's numbers) ----

def timeline(trace, canon):
    """per frame of the trace: its time, the family's virus (role, action, hp, panel), MegaMan's hp and row,
    the reticles spawned, the sounds"""
    game, out = trace['game'], []
    for fr in trace['frames']:
        row = dict(t=fr['t'], foes={}, mm=None, spawned=[], sounds=[(p, s) for p, s in fr['sounds']])
        for o in fr['objs']:
            if o.kind == 'T1':
                nid = canon.nid(game, o.o16(OBJ_NAME_ID))
                mm = nid in canon.megaman.values()
                rec = (canon.action(game, mm, o.o8(OBJ_ACTION)), o.o16(0x24), (o.o8(0x12), o.o8(0x13)))
                if mm:
                    row['mm'] = rec
                else:
                    row['foes'][o.role] = rec
            elif o.kind == 'T3':
                row.setdefault('t3', []).append((o.role, o.o8(OBJ_ACTION), (o.o8(0x12), o.o8(0x13)), o.o16(0x2C)))
        out.append(row)
    return out


def check_expect(name, game, trace, canon, expect):
    """the spec's expectations against one trace: the failures, as lines"""
    tl, bad = timeline(trace, canon), []
    by_t = {r['t']: r for r in tl}
    foe = lambda r: next(iter(r['foes'].values()), None)
    for t, act in expect.get('actions', []):
        r = by_t.get(t)
        got = foe(r)[0] if r and foe(r) else None
        if got != act:
            bad.append(f'{game}: the virus\'s CurAction at t{t} {got}, not {act}')
    reticle = int(expect.get('reticle_type', '0'), 0)
    seen = {}
    for r in tl:
        for role, act, panel, dmg in r.get('t3', []):
            if role[1] == reticle:
                seen.setdefault(role, []).append((r['t'], panel, dmg))
    spawns = [v[0][0] for v in seen.values()]
    if 'reticle_spawns' in expect and spawns[:len(expect['reticle_spawns'])] != expect['reticle_spawns']:
        bad.append(f'{game}: the reticles appeared at t{spawns}, not {expect["reticle_spawns"]}')
    first = next(iter(seen.values()), [])
    for t, panel in expect.get('reticle_path', []):
        got = next(('%d%d' % p for tt, p, _ in first if tt == t), None)
        if got != panel:
            bad.append(f'{game}: the first reticle at t{t} on {got}, not {panel}')
    if 'reticle_damage' in expect and any(d != expect['reticle_damage'] for v in seen.values() for _, _, d in v):
        bad.append(f'{game}: a reticle\'s damage not {expect["reticle_damage"]}')
    sounds = [[r['t'], s] for r in tl for p, s in r['sounds'] if s == int(expect.get('sound', '-1'), 0)]
    if 'sound_at' in expect and [t for t, _ in sounds][:len(expect['sound_at'])] != expect['sound_at']:
        bad.append(f'{game}: sound {expect["sound"]} at t{[t for t, _ in sounds]}, not {expect["sound_at"]}')
    lost, was = [], None
    for r in tl:
        hp = r['mm'][1] if r['mm'] else None
        if was is not None and hp is not None and hp < was:
            lost.append([r['t'], was - hp])
        was = hp
    if 'megaman_hit_damage' in expect and any(dmg != expect['megaman_hit_damage'] for _, dmg in lost):
        bad.append(f'{game}: MegaMan lost {sorted({d for _, d in lost})} a hit, not {expect["megaman_hit_damage"]}')
    if 'megaman_hit_at_least' in expect and len(lost) < expect['megaman_hit_at_least']:
        bad.append(f'{game}: MegaMan hit {len(lost)} times, fewer than {expect["megaman_hit_at_least"]}')
    if 'megaman_hp_lost' in expect and lost != expect['megaman_hp_lost']:
        bad.append(f'{game}: MegaMan lost HP at {lost}, not {expect["megaman_hp_lost"]}')
    if 'hit_only_in' in expect:
        was = {}
        for r in tl:
            for role, (act, hp, _) in r['foes'].items():
                if role in was and hp < was[role][1] and was[role][0] not in expect['hit_only_in'] and act not in expect['hit_only_in']:
                    bad.append(f'{game}: {role_name(role)} lost HP at t{r["t"]} in CurAction {act}')
                was[role] = (act, hp)
    if 'out_of_row_holds' in expect:
        # (each stretch MegaMan stands out of the virus's row: the virus in this CurAction as he comes back)
        out = None
        for i, r in enumerate(tl):
            foe_now = next(iter(r['foes'].values()), None)
            away = r['mm'] and foe_now and r['mm'][2][1] != foe_now[2][1]
            if away and out is None:
                out = i
            elif not away and out is not None:
                last = next(iter(tl[i - 1]['foes'].values()), None)
                if i - out >= 60 and (not last or last[0] != expect['out_of_row_holds']):
                    bad.append(f'{game}: MegaMan out of its row t{tl[out]["t"]}-t{tl[i - 1]["t"]}, the virus in {last[0] if last else None} '
                               f'at its end, not {expect["out_of_row_holds"]}')
                out = None
    for t, panel in expect.get('virus_panels', []):
        r = by_t.get(t)
        got = '%d%d' % foe(r)[2] if r and foe(r) else None
        if got != panel:
            bad.append(f'{game}: the virus at t{t} on {got}, not {panel}')
    for t, where, want in expect.get('panels', []):
        # (a panel's type at a fight frame, in BN6's numbers: BN5's lava 8 is 13, and so on)
        fr = next((f for f in trace['frames'] if f['t'] == t), None)
        got = fr['panels'][int(where[1]) * 8 + int(where[0])][2] if fr else None
        got = canon.panel_type.get(got, got) if game == 'BN5' else got
        if got != want:
            bad.append(f'{game}: panel {where} at t{t} of type {got}, not {want}')
    for t, want in expect.get('foe_hp', []):
        r = by_t.get(t)
        got = foe(r)[1] if r and foe(r) else None
        if got != want:
            bad.append(f'{game}: the virus\'s HP at t{t} {got}, not {want}')
    hits = sum(1 for i in range(1, len(tl)) for role, (_, hp, _) in tl[i]['foes'].items()
               if role in tl[i - 1]['foes'] and hp < tl[i - 1]['foes'][role][1])
    if 'virus_hits_at_least' in expect and hits < expect['virus_hits_at_least']:
        bad.append(f'{game}: the virus was hit {hits} times, fewer than {expect["virus_hits_at_least"]}')
    return bad, dict(spawns=spawns, sounds=[t for t, _ in sounds], lost=lost, hits=hits)


HARD = ('megaman', "megaman's", 'engine')   # whose differences stop a spec as inconclusive (the stop rule)
SOFT = ('native', "native's")               # whose leave the family's world the same: compared on


def first_unexplained(t, open_, by, natives_at, show):
    """the verdict a frame's unexplained differences give (None: compare on), and the lines saying so"""
    fam, bat = by.get('family', []) + by.get('panels', []), by.get('battle', [])
    hard = [d for w in HARD for d in by.get(w, [])]
    if fam:
        verdict, why = 'FAIL', "the family's" if by.get('family') else "the lava's, BN6's engine's port of it"
    elif hard or bat and natives_at is not None:
        verdict, why = 'INCONCLUSIVE', ("MegaMan's own state, what he owns or an effect of the engine's" if hard else
                                        "the battle's, after the natives' own code differed") + ': the stop rule'
    elif bat:
        verdict, why = 'FAIL', "the battle's: its RNG, songs, run order or panels, nothing of anyone's before"
    else:
        return None, []
    ordered = fam + bat + hard + [d for d in open_ if d not in fam + bat + hard]
    lines = [f'  first unexplained difference at t{t} ({why}):'] + [f'    {d}' for d in ordered[:show]]
    if len(open_) > show:
        lines.append(f'    ... {len(open_) - show} more')
    return verdict, lines


def compared(spec, ta, tb, legit, canon, counts=None):
    """Each frame both traces hold, as compared: (BN5's frame, its view, BN6's view, their differences, each with the
    mask that explains it): the engine's own bits of BN5's objects read in BN6's terms first (a panel reserved, the
    blink's phase: xdiff_events.Translations, counted into `counts`), the family's calls compared by their call sites,
    and under rng=replay the draws with no twin among the differences"""
    ranks, prev, trans = {}, (), Translations()
    matched = matched_draws(tb) if spec.get('rng') == 'replay' else None
    bt = {f['t']: f for f in tb['frames']}
    for fra in ta['frames']:
        frb = bt.get(fra['t'])
        if frb is None:
            continue
        fa, fb = Frame(ta, fra, canon), Frame(tb, frb, canon)
        for k, v in trans.apply(fa, fb).items():
            if counts is not None:
                counts[k] = counts.get(k, 0) + v
        renderer_ranks(fa, fb, ranks)
        diffs = frame_diffs(fa, fb) + call_diffs(fa, fb) + (draw_diffs(fa, fb, matched) if matched is not None else [])
        for d in diffs:
            d.mask = mask_of(d, legit['masks'], fa, fb, prev)
        yield fra, fa, fb, diffs
        prev = (fa, fb)


def compare(name, spec, ta, tb, legit, canon, show):
    """The verdict: ('PASS'|'FAIL'|'INCONCLUSIVE', the frame of it, lines to print). The first unexplained
    difference decides. The family's fails the spec. MegaMan's own, what he owns or an effect of the engine's stops
    it as inconclusive (the stop rule: the battle is no longer the same battle for the family). A native enemy's
    own (each game runs its own code for it: a spec's natives, what they own) does neither: the family's world
    (MegaMan, the RNG, the songs, the run order, the panels) is still the same, so the family is still compared,
    until any of those differs (then the stop: the natives may be why). After a stop the frames are still compared,
    the family's roles alone, to say how far they went on the same (not a verdict)."""
    used, lines, read = {}, [], {}
    fa_all, fb_all = ta['frames'], tb['frames']
    verdict, at, n, after, natives_at = 'PASS', None, 0, None, None
    for fra, fa, fb, diffs in compared(spec, ta, tb, legit, canon, read):
        by = {}
        for d in diffs:
            w = who(d, fb, canon, fa)
            if d.mask and verdict == 'PASS':
                used[d.mask] = used.get(d.mask, 0) + 1
            elif not d.mask:
                by.setdefault(w, []).append(d)
        open_ = [d for d in diffs if not d.mask]
        if verdict == 'INCONCLUSIVE':
            if by.get('family') and after is None:
                after = by['family'][0]
            continue
        n += 1
        if [w for w in by if w in SOFT] and natives_at is None:
            natives_at = fra['t']
            lines.append(f'  the natives\' own code differs from t{natives_at} on (each game\'s own: compared on while the '
                         f'family\'s world stays the same): {next(d for w in SOFT for d in by.get(w, []))}')
        v, why = first_unexplained(fra['t'], open_, by, natives_at, show)
        if v is None:
            continue
        verdict, at = v, fra['t']
        lines += why
        if verdict == 'FAIL':
            break
    if verdict == 'INCONCLUSIVE':
        lines.append(f'  after the stop (no verdict), the family\'s objects: ' +
                     (f'the same to t{after.t - 1}, then {after}' if after else f'the same to the end (t{fa_all[-1]["t"]})'))
    if len(fa_all) != len(fb_all) and verdict == 'PASS':
        lines.append(f'  the traces are {len(fa_all)} and {len(fb_all)} fight frames long')
        verdict, at = 'FAIL', min(len(fa_all), len(fb_all))
    lines.insert(0, f'  {n} fight frames compared (t{fa_all[0]["t"]}-t{fa_all[-1]["t"]}), masks applied (none on the '
                 f'family\'s own records, where none may): ' + (', '.join(f'{k} x{v}' for k, v in sorted(used.items())) or 'none') +
                 '; BN5\'s records read in BN6\'s terms: ' + (', '.join(f'{k} x{v}' for k, v in sorted(read.items()) if v) or 'none') +
                 (f'; BN6\'s objects of the dispatch\'s types read as its side table names them: x{tb["dispatched"]}' if tb.get('dispatched') else ''))
    return verdict, at, lines


def survey(spec, ta, tb, legit, canon):
    """every difference of every frame, by whose, part and place: how many, the first, its values"""
    seen = {}
    for _, fa, fb, diffs in compared(spec, ta, tb, legit, canon):
        for d in diffs:
            w = who(d, fb, canon, fa) if isinstance(d.role, tuple) or d.role == 'family' else str(d.role).split(' ')[0]
            key = (role_name(d.role) if w == 'family' and isinstance(d.role, tuple) else w, d.part, d.off, d.mask)
            if key not in seen:
                seen[key] = [0, d]
            seen[key][0] += 1
    return [f'  {n:5d}x {k[0]} {k[1]}+{k[2]:#04x}{f" [{k[3]}]" if k[3] else ""}: first {d}' for k, (n, d) in sorted(seen.items(), key=lambda x: x[1][1].t)]


def one(name, a, legit, canon, pokes=(), calls=(), run_name=None):
    """one spec run and compared: its verdict and kind"""
    with open(os.path.join(SPECS, f'{name}.json')) as f:
        spec = json.load(f)
    canon.natives = {int(x, 0) for x in spec.get('natives', [])}
    canon.native_attacks = {int(x, 0) for x in spec.get('native_attacks', [])}
    canon.native_effects = {int(x, 0) for x in spec.get('native_effects', [])}
    run_name = run_name or name
    paths = [os.path.join(WORK, 'runs', f'{run_name}.{g}') for g in ('bn5', 'bn6')] if a.no_run else \
        run(run_name, spec, list(pokes), list(calls), a.quiet, twins_check(legit))
    ta, tb = read_trace(paths[0]), read_trace(paths[1])
    ta['code'] = tb['moved']   # (the families' blocks: BN5's code where the install found it)
    for tr in (ta, tb):
        assign_roles(tr, canon)
        attribute(tr, canon)
    if a.story:
        for label, tr in (('BN5', ta), ('BN6', tb)):
            print(f' {label}:')
            print('\n'.join(story(tr, canon)))
    if a.survey:
        print('\n'.join(survey(spec, ta, tb, legit, canon)))
    verdict, at, lines = compare(name, spec, ta, tb, legit, canon, a.show)
    print('\n'.join(lines))
    bad = []
    for game, tr in (('BN5', ta), ('BN6', tb)):
        b, seen = check_expect(name, game, tr, canon, spec.get('expect', {}))
        bad += b
        if spec.get('expect'):
            print(f'  {game}: reticles at t{seen["spawns"][:6]}..., sounds at t{seen["sounds"][:6]}..., MegaMan lost {seen["lost"][:8]}, '
                  f'the virus hit {seen["hits"]} times')
    for line in bad:
        print(f'  EXPECT {line}')
    if bad and verdict == 'PASS':
        verdict = 'FAIL'
    kind = spec.get('kind', 'differential')
    print(f'{run_name}: {verdict}{f" at t{at}" if at is not None and verdict != "PASS" else ""} ({kind})')
    return verdict, kind


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('specs', nargs='*', help='specs in tests/xdiff (all where none)')
    ap.add_argument('--story', action='store_true', help='the battle as each game fought it')
    ap.add_argument('--poke', action='append', default=[], help='A:V[:N]: written into BN6\'s copy after the install')
    ap.add_argument('--call', action='append', default=[], help='F:T: the family\'s call to BN6\'s F aimed at T')
    ap.add_argument('--expect-fail', action='store_true', help='exit 0 when every spec FAILs (with --poke: the comparator\'s own test)')
    ap.add_argument('--no-run', action='store_true', help='compare the traces of the last run')
    ap.add_argument('--show', type=int, default=12, help='differences shown at the first unexplained frame')
    ap.add_argument('--quiet', action='store_true')
    ap.add_argument('--survey', action='store_true', help='every difference over every frame, counted by where (masks applied), no stop')
    ap.add_argument('--mutations', action='store_true', help='tests/xdiff/mutations.json: each must FAIL its spec')
    a = ap.parse_args()
    with open(LEGIT) as f:
        legit = json.load(f)
    canon = Canon(legit)
    if not a.no_run:
        find_roms()
        subprocess.run([sys.executable, os.path.join(ROOT, 'build.py'), 'host'], cwd=ROOT, check=True, capture_output=True)
    if a.mutations:
        with open(MUTATIONS) as f:
            muts = json.load(f)['mutations']
        caught = 0
        for k, m in enumerate(muts):
            print(f'== mutation {k + 1}, {m["name"]} ({m["what"]}), on {m["spec"]}')
            verdict, _ = one(m['spec'], a, legit, canon, m.get('poke', []), m.get('call', []), f'mutation{k + 1}')
            caught += verdict == 'FAIL'
            print(f'mutation {k + 1}: {"caught" if verdict == "FAIL" else "NOT CAUGHT"} ({verdict})')
        print(f'{caught} of {len(muts)} mutations caught')
        sys.exit(0 if caught == len(muts) else 1)
    names = a.specs or sorted(n[:-5] for n in os.listdir(SPECS) if n.endswith('.json') and n not in ('legit.json', 'mutations.json'))
    results = {}
    for name in names:
        with open(os.path.join(SPECS, f'{name}.json')) as f:
            print(f'== {name}: {json.load(f).get("about", "")}')
        results[name] = one(name, a, legit, canon, a.poke, a.call)
    print('; '.join(f'{k} {v}' for k, (v, _) in results.items()))
    if a.expect_fail:
        sys.exit(0 if all(v == 'FAIL' for v, _ in results.values()) else 1)
    sys.exit(0 if all(v == 'PASS' or (v == 'INCONCLUSIVE' and kind == 'smoke') for v, kind in results.values()) else 1)


if __name__ == '__main__':
    main()
