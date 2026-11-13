#!/usr/bin/env python3
"""old_checkpoint.py [--from REF ...] [--releases N] [--new REF | --bin PATH] [--seed S ...] [--depth N]
                  [--limit FRAMES] [--pointers] [--allow-refused]: a run an older release saved, continued and played
by this build (docs/devtools/old-checkpoint.md; issue #482, where 0.20.0's checkpoint stood still in 0.21.0 at the
first hit of any attack).

For each release (the newest two tags of v* before HEAD; --from names others) and a seed (1, or the next to 6
where a seed tells nothing; --seed names them):
  1. the release's host binary, built once in a worktree of its own under .build/cw/ref (tools/cw.py's);
  2. with it a new run, headless, the autopilot walking to layer --depth (2), whose arrival writes a checkpoint;
  3. the control: the same with this build (build/host/cyberworld; --bin another, --new REF a commit's), its own
     checkpoint continued and played as 4 and 5 play the release's. Where that does not go on, the seed tells
     nothing;
  4. the release's data folder copied, and this build started on it at the title: START, A (CONTINUE). The run
     must stand on the layer saved;
  5. the autopilot plays on until a battle is won and the next layer reached. FAIL where the game's own frame count
     (the state's gameframe, BN6_FRAME_COUNT) stands for 300 frames, where a battle's picture stands for 600, where
     the game leaves the run or stops, or the layer is not left within --limit frames (20000). MegaMan deleted is
     the autopilot's loss: the seed tells nothing;
  6. a new run by this build from the same folder, whose boot state (boot-3.state) the release wrote: played the
     same way to its second layer;
  7. what the state carried: the run continued once more and left at once (its quit's state is this build's RAM
     after the CONTINUE), beside this build's own checkpoint of the same layer continued and left the same way. The
     IWRAM routines' bytes that differ, and the words that point into the ROM copy's free space (0x08800000 on)
     that are the release's own still where this build's own run has another address or none, are listed with the
     region each lies in by src/emu/free_space.h (--pointers: every one, not three a region). A list to read, not
     a verdict: 4 to 6 fail.

Exit 0 where every release's run went on, 1 where one did not. A build that takes no run of the release's (a new
RUN_MAGIC with no reader of the old) shows no CONTINUE: a FAIL, unless --allow-refused says it is meant.

Local only: Docker for the releases' builds, the host build (tools/cw.py build host) and the ROMs
(CYBERWORLD_ROM_DIR, ~/.cache/mmbn-ref/roms). Its files stay under .build/old-checkpoint; nothing of a ROM, a save
or a state is printed but addresses and counts.
"""
import argparse
import hashlib
import os
import re
import shutil
import struct
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import cw  # noqa: E402
import play  # noqa: E402

WORK = os.path.join(ROOT, '.build', 'old-checkpoint')
SEEDS = (1, 2, 3, 4, 5, 6)   # tried in turn while a seed tells nothing (its autopilot loses, in this build's own run too)
STEP = 60             # frames a sample
STILL_GAME = 300      # the game's frame count standing this long: the game stands still
STILL_PICTURE = 600   # a battle's picture standing this long
# mGBA's state of a GBA: its IWRAM and EWRAM (GBASerializedState's iwram and wram)
STATE_IWRAM, STATE_EWRAM, STATE_SIZE = 0x19000, 0x21000, 0x61000
IWRAM, EWRAM = 0x03000000, 0x02000000
# BN6_IWRAM_CODE on: the routines the game copies in as it powers on (to 0x030079D0 a state's bytes are the ROM image's at
# BN6_IWRAM_IMAGE but for the engine's patches, measured; data and the stacks past it)
IWRAM_CODE, IWRAM_CODE_END = 0x03005B00, 0x030079D0
FREE, FREE_END = 0x08800000, 0x09000000               # EMU_FREE, FREE_END (src/emu/free_space.h)


class Failed(Exception):
    pass


class Lost(Failed):
    """the autopilot's MegaMan deleted: the seed's, not the state's, where another seed goes on"""


def git(*args):
    return subprocess.run(['git', *args], cwd=ROOT, capture_output=True, text=True)


def releases(n):
    """the newest n tags of v* in HEAD's history, HEAD's own left out"""
    head = git('rev-parse', 'HEAD').stdout.strip()
    tags = git('tag', '--list', 'v*', '--sort=-v:refname', '--merged', 'HEAD').stdout.split()
    return [t for t in tags if git('rev-parse', t + '^{commit}').stdout.strip() != head][:n]


def binary_of(ref):
    os.environ.setdefault('CYBERWORLD_DOCKER_NAME', 'old-checkpoint')
    return cw.ref_binary(ref)


def words(state):
    out = {}
    for line in cw.state_lines(state):
        k, _, v = line.partition(' ')
        out.setdefault(k, v)
    return out


def layer_of(state):
    m = re.match(r'\d+', words(state).get('layer', ''))
    return int(m.group()) if m else 0


def start(h, spec, seed, binary, data=None):
    shutil.rmtree(h, ignore_errors=True)
    os.makedirs(h)
    if data:
        shutil.copytree(data, os.path.join(h, 'data'))
    cw.begin(h, spec, False, seed, binary, keep_bin=False)


def stop(h, wait=20.0):
    """the game ended as a quit ends it (a run on a layer saved where MegaMan stands), and waited for"""
    try:
        pid = int(open(os.path.join(h, 'pid')).read())
    except (OSError, ValueError):
        return
    play.stop(h)
    t = time.time()
    while time.time() - t < wait:
        try:
            if os.waitpid(pid, os.WNOHANG)[0]:
                return
        except ChildProcessError:
            return
        time.sleep(0.02)


def log_tail(h, n=6):
    try:
        return [line.rstrip()[:200] for line in open(os.path.join(h, 'log.txt'), errors='replace').read().splitlines()[-n:]]
    except OSError:
        return []


def run(h, commands):
    """a batch's (frame, state, picture path); Failed where the game stopped"""
    try:
        end, _, shots, _, state = cw.batch(h, commands, record=False)
    except SystemExit as e:
        raise Failed('%s; its log ends:\n    %s' % (e.code, '\n    '.join(log_tail(h)))) from None
    for _, p in shots[:-1]:
        os.remove(p)
    return end, state, shots[-1][1]


def won(h):
    """battles the run's log says were won"""
    try:
        return sum(1 for line in open(os.path.join(h, 'data', 'runlog.txt'), errors='replace') if ' battle ' in line and '-> won' in line)
    except OSError:
        return 0


def keep_picture(h, path, name):
    out = os.path.join(h, name + '.png')
    try:
        from PIL import Image
        im = Image.open(path).convert('RGB')
        im.resize((im.width * 2, im.height * 2), Image.NEAREST).save(out)
    except Exception:   # (no Pillow: the BMP)
        out = os.path.join(h, name + '.bmp')
        shutil.copy(path, out)
    return os.path.relpath(out, ROOT)


def play_on(h, layer, limit):
    """The autopilot's game h watched until a battle is won and a layer past `layer` reached: (frames, battles won,
    whether the state told the game's frame count); Failed where it stands still, ends or runs out of frames."""
    won0, ran = won(h), 0
    game = picture = None       # (the value, the frame it was first seen at)
    told = False
    while ran < limit:
        end, state, shot = run(h, 'wait %d' % STEP)
        ran += STEP
        w = words(state)
        digest = hashlib.md5(open(shot, 'rb').read()).hexdigest()
        if w.get('scene') != 'emu':
            raise Failed('the game left the run for scene %s at frame %d: %s' % (w.get('scene'), end, keep_picture(h, shot, 'left')))
        if w.get('doing') == 'gameover':
            raise Lost('MegaMan was deleted on layer %d at frame %d (the autopilot lost): %s' % (
                layer_of(state), end, keep_picture(h, shot, 'deleted')))
        if 'gameframe' in w:
            told = True
            if not game or game[0] != w['gameframe']:
                game = (w['gameframe'], end)
            elif end - game[1] >= STILL_GAME:
                raise Failed("the game stands still: its frame count %s since frame %d, %d frames, doing %s on layer %d, %d battles won: %s" % (
                    game[0], game[1], end - game[1], w.get('doing'), layer_of(state), won(h) - won0, keep_picture(h, shot, 'still')))
        if w.get('doing') != 'battle' or not picture or picture[0] != digest:
            picture = (digest, end)
        elif end - picture[1] >= STILL_PICTURE:
            raise Failed("a battle's picture stands still since frame %d, %d frames, on layer %d, %d battles won: %s" % (
                picture[1], end - picture[1], layer_of(state), won(h) - won0, keep_picture(h, shot, 'still')))
        os.remove(shot)
        if layer_of(state) > layer and won(h) > won0:
            return ran, won(h) - won0, told
    raise Failed('layer %d not left with a battle won in %d frames (now layer %d, doing %s, %d battles won): %s' % (
        layer, limit, layer_of(state), w.get('doing'), won(h) - won0, keep_picture(h, run(h, 'shot')[2], 'limit')))


def save_run(binary, seed, depth, h):
    """A new run by `binary` walked to layer `depth` by the autopilot, its checkpoint written: the data folder's
    copy as the checkpoint left it (the game still running: no quit's save over it)."""
    start(h, 'CYBERWORLD_AUTOPILOT=1 --scene emu', seed, binary)
    state_file = os.path.join(h, 'data', 'savedata', 'run.state')
    ran = 0
    stamp = lambda: os.stat(state_file).st_mtime_ns if os.path.isfile(state_file) else 0
    while ran < 40000:
        before = stamp()
        end, state, shot = run(h, 'wait %d' % STEP)
        os.remove(shot)
        ran += STEP
        w = words(state)
        if w.get('doing') == 'gameover':
            raise Lost('MegaMan was deleted before layer %d' % depth)
        if layer_of(state) >= depth:
            break
    else:
        raise Failed('layer %d not reached in %d frames' % (depth, ran))
    # (the checkpoint waits for MegaMan free, 60 frames in at least, and is written by the save queue)
    for _ in range(40):
        end, state, shot = run(h, 'wait 30')
        os.remove(shot)
        if stamp() != before and os.path.getsize(state_file) >= STATE_SIZE:
            size = os.path.getsize(state_file)
            time.sleep(0.2)
            if os.path.getsize(state_file) == size and words(state).get('doing') == 'map':
                break
    else:
        raise Failed("no checkpoint written on layer %d" % depth)
    data = h + '-data'
    shutil.rmtree(data, ignore_errors=True)
    shutil.copytree(os.path.join(h, 'data'), data)
    play.stop(h)
    return data, layer_of(state), end


def continue_run(binary, data, seed, h, autopilot=True):
    """`binary` started at the title on a copy of `data`, START and A pressed: the state after CONTINUE"""
    start(h, ('CYBERWORLD_AUTOPILOT=1 ' if autopilot else '') + '--scene title', seed, binary, data)
    run(h, 'wait 60')
    run(h, 'press START; wait 30')
    end, state, shot = run(h, 'press A; wait 90')
    return end, state, shot


# ---- what the state carried ----

def ram(path):
    b = open(path, 'rb').read()
    if len(b) < STATE_SIZE:
        raise Failed('%s is no state of a GBA (%d bytes)' % (os.path.relpath(path, ROOT), len(b)))
    return b[STATE_IWRAM:STATE_IWRAM + 0x8000], b[STATE_EWRAM:STATE_EWRAM + 0x40000]


def free_pointers(mem, base):
    return {base + i: v for i in range(0, len(mem), 4) for v in struct.unpack_from('<I', mem, i) if FREE <= v < FREE_END}


def regions(header):
    """free_space.h's regions, each (start, name), in order"""
    out = [(FREE + int(m.group(2), 16), m.group(1)) for m in re.finditer(r'^#define\s+(\w+)\s+\(EMU_FREE \+ 0x([0-9A-Fa-f]+)u\)', header, re.M)
           if not m.group(1).endswith('_END')]
    return sorted(set(out))


def region(table, addr):
    name = None
    for at, n in table:
        if at > (addr & ~1):
            break
        name = (n, at)
    return name


def left_state(binary, data, seed, h):
    """the run in `data` continued by `binary` and left: the state its quit saved, this build's RAM after the CONTINUE"""
    end, state, shot = continue_run(binary, data, seed, h, autopilot=False)
    run(h, 'wait 90')
    saved = os.path.join(h, 'data', 'savedata', 'run.state')
    before = hashlib.md5(open(saved, 'rb').read()).hexdigest()
    stop(h)
    if hashlib.md5(open(saved, 'rb').read()).hexdigest() == before:
        raise Failed('the quit saved no state in %s' % os.path.relpath(h, ROOT))
    return saved


def carried(ref, new_ref, saved_state, old_state, mine_state, verbose):
    """What the release's run still holds after this build's CONTINUE that this build's own run does not: lines.
    (saved_state: the release's checkpoint as it was written; old_state: it continued by this build and left;
    mine_state: this build's own checkpoint of the layer continued and left)"""
    out = []
    (si, se), (oi, oe), (mi, me) = ram(saved_state), ram(old_state), ram(mine_state)
    lo, hi = IWRAM_CODE - IWRAM, IWRAM_CODE_END - IWRAM
    code = [IWRAM + i for i in range(lo, hi) if oi[i] != mi[i]]
    runs, last = [], None
    for a in code:
        if last is not None and a - last <= 4:
            runs[-1][1] = a
        else:
            runs.append([a, a])
        last = a
    out.append("IWRAM's routines (%08x-%08x): %s" % (IWRAM_CODE, IWRAM_CODE_END, 'the same bytes as in this build\'s own run' if not runs else
               '%d bytes differ from this build\'s own run, in %d places' % (len(code), len(runs))))
    for a, b in runs[:None if verbose else 12]:
        i, j = a - IWRAM, b - IWRAM + 1
        out.append('  %08x: %s  (this build: %s)' % (a, oi[i:j].hex(' '), mi[i:j].hex(' ')))
    old_table = regions(git('show', '%s:src/emu/free_space.h' % ref).stdout)
    new_src = git('show', '%s:src/emu/free_space.h' % new_ref).stdout if new_ref else open(os.path.join(ROOT, 'src', 'emu', 'free_space.h')).read()
    new_table = regions(new_src)
    old_p = {**free_pointers(oi, IWRAM), **free_pointers(oe, EWRAM)}
    new_p = {**free_pointers(mi, IWRAM), **free_pointers(me, EWRAM)}
    was_p = {**free_pointers(si, IWRAM), **free_pointers(se, EWRAM)}
    same = [a for a in old_p if new_p.get(a) == old_p[a]]
    # (a word this build wrote after the CONTINUE is its own: another song's place, another object's)
    kept = [a for a in old_p if new_p.get(a) != old_p[a] and was_p.get(a) == old_p[a]]
    out.append("words that point into the free space after the CONTINUE: %d; as this build's own run has them %d, written by this build since %d, "
               "the release's own still, where this build's run has another or none: %d" % (len(old_p), len(same), len(old_p) - len(same) - len(kept), len(kept)))
    by = {}
    for a in sorted(kept):
        r = region(new_table, old_p[a])
        was = region(old_table, old_p[a])
        moved = '' if not r or not was or was == r else ' (the release: %s)' % was[0]
        by.setdefault((r[0] if r else '?') + moved, []).append(a)
    for name, addrs in sorted(by.items(), key=lambda kv: -len(kv[1])):
        show = addrs if verbose else addrs[:3]
        out.append('  %s: %d, %s%s' % (name, len(addrs), ', '.join(
            '%08x -> %08x%s' % (a, old_p[a], ' (this build %08x)' % new_p[a] if a in new_p else '') for a in show), '' if len(show) == len(addrs) else ', ...'))
    return out


# ---- a release against this build ----

def continued(binary, data, seed, layer, h, a):
    """The run in `data` continued by `binary` and played on: its line; Failed (Lost) where it did not go on, None
    where the build shows no CONTINUE and that is allowed."""
    try:
        end, state, shot = continue_run(binary, data, seed, h)
        if words(state).get('scene') != 'emu' or layer_of(state) != layer:
            where = 'scene %s%s' % (words(state).get('scene'), ', layer %d' % layer_of(state) if layer_of(state) else '')
            pic = keep_picture(h, shot, 'refused')
            if a.allow_refused:
                return None
            raise Failed("the run was not continued on layer %d (%s: no CONTINUE on the title, this build takes no run of the release's?): %s" % (layer, where, pic))
        ran, battles, told = play_on(h, layer, a.limit)
        return 'ok, %d battle%s won and layer %d reached in %d frames%s' % (
            battles, '' if battles == 1 else 's', layer + 1, ran, '' if told else " (the build's state tells no gameframe: its pictures alone were watched)")
    finally:
        play.stop(h)


def check(ref, seed, a, new_bin, new_ref):
    """One release's run on one seed: (True it went on, False it did not, None the seed tells nothing; lines)."""
    name = '%s-s%d' % (re.sub(r'[^\w.]+', '-', ref), seed)
    base = os.path.join(WORK, name)
    shutil.rmtree(base, ignore_errors=True)
    old_bin, sha = binary_of(ref)
    head = '%s (%s) seed %d' % (ref, sha, seed)
    whose = "the release's run"
    try:
        data, layer, frames = save_run(old_bin, seed, a.depth, os.path.join(base, 'saved'))
        # the control: this build's own checkpoint of the layer, continued and played the same way
        whose = "this build's own run"
        mine, _, _ = save_run(new_bin, seed, layer, os.path.join(base, 'mine'))
        continued(new_bin, mine, seed, layer, os.path.join(base, 'mine-continued'), a)
    except Failed as e:
        return None, ['%s: tells nothing, %s: %s' % (head, whose, e)]
    finally:
        play.stop(os.path.join(base, 'saved'))
        play.stop(os.path.join(base, 'mine'))
    lines, ok = ['%s: its checkpoint on layer %d after %d frames' % (head, layer, frames)], True

    try:
        line = continued(new_bin, data, seed, layer, os.path.join(base, 'continued'), a)
        lines.append('  CONTINUE: ' + (line or 'not taken (--allow-refused)'))
    except Lost as e:
        return None, ['%s: tells nothing, %s' % (head, e)]
    except Failed as e:
        ok = False
        lines.append('  CONTINUE: FAIL, %s' % e)

    h = os.path.join(base, 'new-game')
    try:
        start(h, 'CYBERWORLD_AUTOPILOT=1 --scene emu', seed, new_bin, data)
        booted = any(line.startswith('boot:') for line in open(os.path.join(h, 'log.txt'), errors='replace'))
        ran, battles, _ = play_on(h, 1, a.limit)
        lines.append("  NEW GAME on %s: ok, %d battle%s won and layer 2 reached in %d frames" % (
            'a boot of its own (it did not take the release\'s boot state)' if booted else "the release's boot state", battles, '' if battles == 1 else 's', ran))
    except Lost as e:
        lines.append("  NEW GAME on the release's boot state: tells nothing, %s" % e)
    except Failed as e:
        ok = False
        lines.append('  NEW GAME on the release\'s boot state: FAIL, %s' % e)
    finally:
        play.stop(h)

    try:
        old_state = left_state(new_bin, data, seed, os.path.join(base, 'left'))
        mine_state = left_state(new_bin, mine, seed, os.path.join(base, 'mine-left'))
        lines += ['  ' + line for line in carried(ref, new_ref, os.path.join(data, 'savedata', 'run.state'), old_state, mine_state, a.pointers)]
    except Failed as e:
        lines.append('  what the state carried: not compared, %s' % e)
    finally:
        for d in ('left', 'mine-left'):
            play.stop(os.path.join(base, d))
    return ok, lines


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0], formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__.split('\n\n', 1)[1])
    ap.add_argument('--from', dest='refs', action='append', metavar='REF', help='a release (a tag or commit) whose run is continued; several times for several')
    ap.add_argument('--releases', type=int, default=2, help='without --from: the newest N tags of v* before HEAD (2)')
    ap.add_argument('--new', metavar='REF', help="the build that continues: this commit's, built as the releases are")
    ap.add_argument('--bin', help='the build that continues: this binary (build/host/cyberworld)')
    ap.add_argument('--seed', type=int, action='append', help="the run's seed; several times for several (without: 1, then 2 to 6 while a seed tells nothing)")
    ap.add_argument('--depth', type=int, default=2, help="the layer whose arrival's checkpoint is continued (2)")
    ap.add_argument('--limit', type=int, default=20000, help='frames a continued run has to win a battle and leave its layer (20000)')
    ap.add_argument('--pointers', action='store_true', help='every word the state carried, not three a region')
    ap.add_argument('--allow-refused', action='store_true', help="a run this build does not continue (a new RUN_MAGIC) is no failure")
    a = ap.parse_args()
    if not os.path.isdir(play.ROM_DIR):
        sys.exit('no ROMs in %s (CYBERWORLD_ROM_DIR)' % play.ROM_DIR)
    new_ref = a.new
    if a.new:
        new_bin, sha = binary_of(a.new)
        what = '%s (%s)' % (a.new, sha)
    else:
        new_bin = os.path.abspath(a.bin) if a.bin else cw.BIN
        what = os.path.relpath(new_bin, ROOT)
        if not a.bin:
            reply = cw.Reply()
            if not cw.ready(reply):
                sys.exit('the host build failed:\n' + reply.text())
    if not os.path.isfile(new_bin):
        sys.exit('no %s: python3 tools/cw.py build host' % what)
    refs = a.refs or releases(a.releases)
    if not refs:
        sys.exit('no tag of v* before HEAD: --from REF')
    print('continued by %s' % what)
    bad = 0
    for ref in refs:
        told = False
        for seed in a.seed or SEEDS:
            ok, lines = check(ref, seed, a, new_bin, new_ref)
            print('\n'.join(lines), flush=True)
            if ok is None:
                continue
            told = True
            bad += not ok
            if not a.seed:
                break
        if not told:
            bad += 1
            print('%s: FAIL, no seed on which a run goes on' % ref)
    print('old checkpoints: %s' % ('ok' if not bad else '%d FAILED (their games in %s)' % (bad, os.path.relpath(WORK, ROOT))))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
