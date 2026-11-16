#!/usr/bin/env python3
"""The fuzz of quits (docs/devtools/scenarios.md, A quit at any frame): a
run played by the autopilot, quit at frames a seeded dice names, started
again and CONTINUEd, and what came back held to what was kept at the quit.
Six of 0.22.0's fixes were a run that came back changed from a quit, each
found by hand; this looks for the next.

  tools/scenarios_fuzz.py [SEED[/GAME]] [--games N] [--quits N] [--jobs N]
      (build.py scenarios --fuzz ...) --games games (8) of --quits quits
      each (4), --jobs at a time (4), all made from SEED (none: one drawn,
      and printed): each game's own seed, the layer it starts on, its
      autopilot, and each quit's frame. SEED/GAME plays that one game
      again. A line a game; for a failure the quit, what was kept and
      found, and where its states, picture and logs are. Exit 1 where one
      failed.

What a CONTINUE must bring back depends on where the quit found the run
(`situation`), told from the state as a player sees it, not from what the
engine says a quit keeps (its `quit` line: printed with a failure, to
start the search from):

  map       on a layer's map, no box open: everything (WORDS), the same
  won       from a won battle's RESULT to the map: the same, zenny and
            BugFrags no less (its reward may still come)
            (either: the next layer where the engine ran the game on,
            `quit after`, and the layer's exit was under MegaMan)
  box       a chat box, a shop or the PET open: the run as it stood before
            it opened, each line as it was at some moment since the map
  battle    a battle under way: the last checkpoint saved, on this layer
  guardian  a guardian's fight and his end: the last checkpoint saved
  home      in the town: the last checkpoint saved

and after every CONTINUE the game goes on: MegaMan free within FREE
frames, the game's own frame count never standing STILL frames.

The games run as the scenarios' do (tools/scenarios.py), each in
.build/scenarios/fuzz-SEED-GAME. Local: the ROM.
"""
import concurrent.futures
import os
import random
import re
import shutil
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import cw  # noqa: E402
import play  # noqa: E402
import scenarios as S  # noqa: E402

WORDS = ('layer', 'area', 'zenny', 'bugfrags', 'hp', 'scrtdata', 'guardian')   # what a CONTINUE brings back
GROWS = ('zenny', 'bugfrags')    # ... of which a won battle's reward may still raise these
CHUNK = 30           # frames between two looks at the state
GAP = (20, 2400)     # a quit's frames after the start or the CONTINUE before it
EDGE = ('screen', 'doing', 'chat', 'talk', 'quit', 'battlestep', 'layer', 'where', 'saves')   # a change of one is an edge
EDGE_STEP = 6        # frames between two looks while an edge is sought
EDGE_MOST = 1800     # ... and the most frames it is sought for
EDGE_AFTER = 45      # a quit at an edge comes up to this many frames after it
FIRST = 1200         # frames a new run may take to its first checkpoint
FREE = 900           # frames MegaMan may stay held after a CONTINUE
STILL = 600          # frames the game's own frame count may stand
AFTER = 600          # frames played on after the last CONTINUE
DEPTHS = (1, 9)      # the layer a game starts on
# Bugs the fuzz has shown that are not fixed yet (docs/devtools/scenarios.md,
# A quit at any frame): a CONTINUE that came back changed from a quit one of
# these tells (its situation, and lines of the state kept) is reported KNOWN,
# no failure of the run, and the game is played on. An entry goes with its fix.
KNOWN = ()   # (none today: a liberation's words after a won battle, seed 3's game 12, was the first, fixed with the board's own save)


def plan(seed, game, quits):
    """Game `game` of fuzz `seed`: its own seed, the layer it starts on, its
    autopilot ('1', or 'weak': every battle won at once, so more of the map
    and of what follows a win) and its quits, each (frames after the start
    or CONTINUE before it, whether it waits for the next edge, frames after
    that edge)."""
    rng = random.Random(f'{seed}/{game}')
    return dict(seed=rng.randrange(1, 100000), depth=rng.randint(*DEPTHS), pilot=rng.choice(('1', 'weak', 'weak')),
                quits=[(rng.randint(*GAP), rng.random() < 0.5, rng.randint(0, EDGE_AFTER)) for _ in range(quits)])


def read(state):
    """A state's lines by their first word."""
    out = {}
    for line in cw.state_lines(state):
        k, _, v = line.partition(' ')
        out.setdefault(k, v)
    return out


def situation(s, won=False):
    """Where a quit would find the run of state `s` (read), as the player
    sees it: 'home', 'guardian', 'battle', 'won' (`won`: this battle's
    RESULT was seen), 'box', 'map'; None where no run is under way."""
    if s.get('scene') != 'emu' or s.get('where') not in ('layer', 'town') or s.get('doing') == 'gameover':
        return None
    if s['where'] == 'town':
        return 'home'
    if s.get('guardian', '').split()[-1:] in (['fighting'], ['beaten']):
        return 'guardian'
    if won:
        return 'won'
    if s.get('doing') == 'battle':
        return 'battle'
    if s.get('chat') != 'closed' or s.get('talk') != 'none' or s.get('doing') == 'menu' or s.get('screen') != 'map':
        return 'box'
    return 'map'


def track(s=None):
    """What the fuzz knows of a game since its start or CONTINUE: the states
    around the last checkpoint saved (after a CONTINUE, the one it brought
    back), those since MegaMan was last on the map with no box open, the
    last seen, whether a battle's RESULT was seen, the frames the game's
    count has stood, and whether MegaMan has been free."""
    return dict(saved=[s] if s else [], before=[s] if s else [], last=s, won=False, still=0, free=False, frames=0)


def follow(t, s, frames=0):
    """One more state seen, `frames` after the last: the situation a quit
    would find now."""
    last = t['last']
    if last and s.get('saves') != last.get('saves'):
        t['saved'] = [last, s]   # (saved between the two: either's lines)
    if s.get('battlestep') == 'won':
        t['won'] = True
    elif s.get('quit') == 'here' or s.get('battlestep') in ('custom', 'fight') or s.get('where') != 'layer':
        t['won'] = False
    kind = situation(s, t['won'])
    t['before'] = [s] if kind == 'map' else t['before'] + [s]
    t['still'] = t['still'] + frames if last and s.get('gameframe') == last.get('gameframe') and s.get('screen') in ('map', 'battle', 'town') else 0
    t['free'] = t['free'] or s.get('quit') == 'here' or kind in ('battle', 'guardian', 'home')
    t['frames'] += frames
    t['last'] = s
    return kind


def number(value):
    m = re.search(r'-?\d+', value or '')
    return int(m.group()) if m else None


def same(w, was, now):
    """Whether line `w` came back: as it was; a guardian's by his state, his
    name too but where he was ??? before (the profile keeps whom it has met)."""
    if w != 'guardian' or not was or not now:
        return was == now
    return was.split()[-1] == now.split()[-1] and was.split()[0] in ('???', now.split()[0])


def judge(kind, kept, found, saved, before):
    """What a CONTINUE brought back that it may not, a line each: `kept`
    the state at the quit in situation `kind`, `found` the state after the
    CONTINUE, `saved` the states around the last checkpoint, `before` those
    since the map."""
    out = []
    # (the last checkpoint of a guardian's end or the town may be the layer's, or the town's)
    wheres = [s.get('where') for s in saved] if kind in ('guardian', 'home') else [kept.get('where')]
    if found.get('where') not in wheres:
        out.append(f'where {kept.get("where")} kept, {found.get("where")} found')
    # (a quit the engine runs the game on from, `quit after`, may find the
    # layer's exit under MegaMan: the next layer then, its area and guardian)
    moved = kind in ('map', 'won') and kept.get('quit') == 'after' and number(found.get('layer')) == (number(kept.get('layer')) or 0) + 1
    for w in WORDS:
        if w not in kept:
            continue   # (the town's state has no layer)
        k, f = kept[w], found.get(w)
        if kind in ('map', 'won'):
            if w in ('layer', 'area', 'guardian'):
                ok = same(w, k, f) or moved
            elif kind == 'won':
                ok = (number(f) or 0) >= (number(k) or 0) if w in GROWS else w == 'hp' or f == k   # (the RESULT's HP is the battle's)
            else:
                ok = f == k
            may = ''
        else:
            had = [s.get(w) for s in (before + [kept] if kind == 'box' else saved)]
            ok = any(same(w, x, f) for x in had) and (kind != 'battle' or w != 'layer' or f == k)
            may = f' ({" or ".join(dict.fromkeys(str(x) for x in had))} {"since the map" if kind == "box" else "at the last checkpoint"})'
        if not ok:
            out.append(f'{w} {k} kept, {f} found{may}')
    return out


def known(kind, kept):
    """The known bug a quit in situation `kind` with state `kept` tells: its words, or None."""
    for k in KNOWN:
        if k['kind'] == kind and all(kept.get(w) == v for w, v in k['kept'].items()):
            return k['what']
    return None


# ---- playing a game ----

def step(h, frames):
    """Game h run on `frames`: its state."""
    _, _, shots, _, state = cw.batch(h, f'wait {frames}', record=False)
    for _, p in shots:
        os.remove(p)
    return state


def watch(t, state, frames):
    """A state of the game going on, followed: its situation (None: the run
    is over); Failed where the game stands still or MegaMan stays held."""
    kind = follow(t, read(state), frames)
    if kind and t['still'] >= STILL:
        raise S.Failed(f'the game stands still: its frame count the same for {t["still"]} frames', state)
    if kind and not t['free'] and t['frames'] >= FREE:
        raise S.Failed(f'MegaMan still held {t["frames"]} frames after the start or CONTINUE', state)
    return kind


def play_on(h, t, frames):
    """Game h played on `frames`, watched: (state, situation, frames run)."""
    state, kind, ran = '', situation(t['last'] or {}, t['won']), 0
    while ran < frames and (kind or not ran):
        n = min(CHUNK, frames - ran)
        state = step(h, n)
        ran += n
        kind = watch(t, state, n)
    return state, kind, ran


def to_edge(h, t, extra):
    """Game h played on to the next edge (a line of EDGE changing), then
    `extra` frames: (state, situation, frames run)."""
    def edge_of(s):
        return tuple(s.get(w) for w in EDGE)
    was, state, kind, ran = edge_of(t['last']), '', None, 0
    while ran < EDGE_MOST:
        state = step(h, EDGE_STEP)
        ran += EDGE_STEP
        kind = watch(t, state, EDGE_STEP)
        if not kind or edge_of(t['last']) != was:
            break
    if kind and extra:
        state = step(h, extra)
        ran += extra
        kind = watch(t, state, extra)
    return state, kind, ran


def take_up(h, pilot, roms):
    """Game h quit as a player's system quits it, started again on its
    saves at the title, CONTINUE taken: (the state the run is first seen
    in, frames run)."""
    S.quit_game(h)
    state, ran = S.start(h, f'CYBERWORLD_AUTOPILOT={pilot} --scene title', False, roms)
    for st in (('repeat', 'press START; wait 40', 'title=menu'), ('expect', 'cursor=continue'),
               ('repeat', 'press A; wait 8', 'scene=emu', 60)):
        try:
            state, n = S.step_run(h, st, state, {}, roms)
        except S.Failed as e:
            raise S.Failed(f'no CONTINUE: {st!r}: {e.args[0]}', *e.args[1:])
        ran += n
    for _ in range(0, 2400, EDGE_STEP):
        if read(state).get('where') in ('layer', 'town'):
            return state, ran
        state = step(h, EDGE_STEP)
        ran += EDGE_STEP
    raise S.Failed('no run on a map 2400 frames after the CONTINUE', state)


def shown(s):
    return ' · '.join(f'{w} {s[w]}' for w in ('screen', 'where', 'doing', 'chat', 'talk', 'battlestep', 'quit', 'saves') + WORDS if w in s)


def game(seed, n, quits, roms):
    """Game n of fuzz `seed` played: dict(name, ok, seconds, frames, lines, note)."""
    p = plan(seed, n, quits)
    name = f'{seed}/{n}'
    h = os.path.join(S.WORK, f'fuzz-{seed}-{n}')
    shutil.rmtree(h, ignore_errors=True)
    os.makedirs(h)
    began, frames, state, lines, kinds, knowns = time.time(), 0, '', [], [], []
    what = f'--seed {p["seed"]} --run-depth {p["depth"]}, autopilot {p["pilot"]}'
    again = f'     again: python3 build.py scenarios --fuzz {name}' + (f' --quits {quits}' if quits != 4 else '')
    at = 'its start'
    try:
        state, frames = S.start(h, f'CYBERWORLD_AUTOPILOT={p["pilot"]} --scene emu --seed {p["seed"]} --run-depth {p["depth"]}', True, roms)
        t = track()
        kind = watch(t, state, 0)
        # (a run has nothing to continue before its first checkpoint)
        while kind and not number(t['last'].get('saves')):
            if t['frames'] >= FIRST:
                raise S.Failed(f'no checkpoint saved in {FIRST} frames of a new run', state)
            state, kind, ran = play_on(h, t, CHUNK)
            frames += ran
        for i, (gap, edge, extra) in enumerate(p['quits'], 1):
            at = f'quit {i}'
            state, kind, ran = play_on(h, t, gap)
            frames += ran
            if kind and edge:
                state, kind, ran = to_edge(h, t, extra)
                frames += ran
            if not kind:
                break
            kept = t['last']
            kinds.append(kind)
            at = f'quit {i} ({kind}, frame {kept.get("frame")} of its start; the engine: quit {kept.get("quit")})'
            state, ran = take_up(h, p['pilot'], roms)
            frames += ran
            found = read(state)
            wrong = judge(kind, kept, found, t['saved'], t['before'])
            if wrong:
                told = '; '.join(wrong) + f'\n     kept:  {shown(kept)}\n     found: {shown(found)}'
                if not known(kind, kept):
                    state = '\n'.join(['# kept at the quit', *(f'{k} {v}' for k, v in kept.items()), '# found after the CONTINUE', state])
                    raise S.Failed(told, state)
                knowns += [f'     KNOWN: {known(kind, kept)}', f'     {at}: {told}']
            t = track(found)
        else:
            at = 'the play after the last CONTINUE'
            state, kind, ran = play_on(h, t, AFTER)
            frames += ran
        ok = True
    except (S.Failed, SystemExit) as e:
        ok = False
        if isinstance(e, S.Failed) and len(e.args) > 1:
            state = e.args[1]
        lines.append(f'     {at}: {e.args[0] if isinstance(e, S.Failed) else e.code}')
        lines.append('     files: ' + ', '.join(os.path.relpath(f, ROOT) for f in S.failure(h, state)) + " (log-N.txt: each start's)")
    finally:
        play.stop(h)
    if knowns or not ok:
        lines = knowns + lines + [again]
    note = ' '.join(kinds) + ('' if len(kinds) == quits or not ok else ', then the run over')
    return dict(name=name, ok=ok, known=bool(knowns), seconds=time.time() - began, frames=frames, lines=lines,
                note=f'{what}: {note or "the run over at once"}')


def main(argv):
    rest, values = cw.options(argv, {}, {'--games': 'games', '--quits': 'quits', '--jobs': 'jobs'})
    m = re.match(r'^(\d+)(?:/(\d+))?$', rest[0]) if rest else None
    if rest and (not m or len(rest) > 1):
        sys.exit(__doc__)
    seed = int(m.group(1)) if m else random.SystemRandom().randrange(1, 1000000)
    quits = int(values.get('quits') or 4)
    games = [int(m.group(2))] if m and m.group(2) else list(range(1, int(values.get('games') or 8) + 1))
    reply = cw.Reply()
    if not cw.ready(reply):
        sys.exit(reply.text)
    roms = S.have_roms()
    if 'bn6' not in roms:
        sys.exit(f"BN6's ROM is not in {play.ROM_DIR} (CYBERWORLD_ROM_DIR): the fuzz plays the game")
    folder = S.rom_folder(roms, ())
    print(f'fuzz {seed}: {len(games)} game{"s" if len(games) > 1 else ""} of {quits} quits', flush=True)
    began, failed, knowns = time.time(), 0, 0
    with concurrent.futures.ThreadPoolExecutor(int(values.get('jobs') or 4)) as pool:
        for r in pool.map(lambda n: game(seed, n, quits, folder), games):
            failed += not r['ok']
            knowns += r['ok'] and r['known']
            print(f'{"FAIL " if not r["ok"] else "KNOWN" if r["known"] else "PASS "} {r["name"]:<{len(str(seed)) + 3}} {r["seconds"]:5.1f} s  {r["frames"]:6} frames  {r["note"]}', flush=True)
            for line in r['lines']:
                print(line, flush=True)
    print(f'fuzz {seed}: {len(games) - failed - knowns} passed, {failed} failed' + (f', {knowns} known' if knowns else '') +
          f' in {time.time() - began:.0f} s', flush=True)
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
