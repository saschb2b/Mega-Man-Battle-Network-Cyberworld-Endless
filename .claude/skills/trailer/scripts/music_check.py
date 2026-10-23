#!/usr/bin/env python3
"""A trailer piece checked without ears: rendered, then its loudness and
peaks bar by bar (no bar near clipping, the drop and the climax where the
plan put them), its balance of lows, mids and highs and its punch (the
swing inside a beat) beside a reference recording's (the game it is in
the manner of), the lead's notes that rub the harmony (where the piece
names its CHORD_SAMPLES), and a piano roll of its score to look at (the
squares, the wave's bass, the stabs and pads). The numbers to aim for are
in ../music_rules.md.

    python3 .claude/skills/trailer/scripts/music_check.py [VERSION] [--ref AUDIO] [--piece NAME]

VERSION is a trailer's (tools/trailers/), the current one by default;
--piece NAME a module there that is no trailer's (a sketch: sketch_a).
--source GAME SONG [--offset TICKS] reads the song the piece is in the
manner of from the ROM (song_habits.py's reader) and sets each part's
motion beside its own: how many notes a bar, how many different pitches,
how often a note repeats, steps or leaps (music_rules.md, Discipline). AUDIO
is any recording ffmpeg reads (the owner's, kept in .build/ and never
committed); it is decoded to 8 kHz mono in the build's ffmpeg image. The
wave, the roll and the decoded reference go to .build/trailer-VERSION/.
"""
import array
import collections
import importlib
import math
import os
import shutil
import struct
import subprocess
import sys
import wave

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import build  # noqa: E402
import trailer_music as tm  # noqa: E402
import trailers  # noqa: E402


def bands(x, sr, edges=(200, 1500)):
    """The shares of energy between the `edges` (below 200 Hz, 200-1500 Hz
    and above by default), in %."""
    def lowpass(sig, fc):
        a, y, out = 1 - math.exp(-2 * math.pi * fc / sr), 0.0, []
        for v in sig:
            y += a * (v - y)
            out.append(y)
        return out
    lows = [lowpass(x, e) for e in edges] + [x]
    energy = [sum(v * v for v in lows[0])] + [sum((a - b) ** 2 for a, b in zip(hi, lo))
                                              for lo, hi in zip(lows, lows[1:])]
    total = sum(energy) or 1
    return [round(100 * e / total) for e in energy]


def punch(x, sr, bpm):
    """How much the sound swings inside a beat: the median over the beats
    of the 95th against the 10th percentile of a 10 ms loudness, in dB.
    BN6's "Surge of Power!" swings 7 dB, 0.13.0's piece 3.4: a wall of
    held notes reads as synthetic, a beat that breathes as the game."""
    h = max(1, int(0.01 * sr))
    env = [math.sqrt(sum(v * v for v in x[i:i + h]) / h) for i in range(0, len(x) - h, h)]
    beat = max(1, int(60 / bpm / 0.01))
    swings = []
    for i in range(0, len(env) - beat, beat):
        seg = sorted(env[i:i + beat])
        top, floor = seg[int(0.95 * (len(seg) - 1))], seg[int(0.1 * (len(seg) - 1))]
        swings.append(20 * math.log10((top + 1e-9) / (floor + 1e-9)))
    swings.sort()
    return swings[len(swings) // 2] if swings else 0.0


def clashes(piece):
    """The first square's notes on a beat or held (12 ticks or more) that
    rub a semitone against the harmony under them and are not in it: the
    wave's bass, the second square and the chord samples a piece names in
    CHORD_SAMPLES (instrument: its intervals over the key it is played
    at). A rub on purpose (a maj7 over its root) is read and kept."""
    chords = getattr(piece, 'CHORD_SAMPLES', None)
    if chords is None:
        return
    s = piece.arrangement()
    names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']

    def sounding(t):
        pcs = {k % 12 for ch in ('wave', 'sq2') for t0, n, k, _, _ in s.psg[ch] if t0 <= t < t0 + max(n, 6)}
        return pcs | {(k + i) % 12 for t0, n, inst, k, *_ in s.ds if inst in chords and t0 <= t < t0 + n
                      for i in chords[inst]}
    strong = [(t, n, k) for t, n, k, _, _ in sorted(s.psg['sq1']) if t % 24 == 0 or n >= 12]
    rubs = []
    for t, n, k in strong:
        pcs = sounding(t + 1)
        if k % 12 not in pcs and any((k - pc) % 12 in (1, 11) for pc in pcs):
            rubs.append(f'bar {t // tm.BAR} tick {t % tm.BAR} {names[k % 12]}')
    print(f'lead notes rubbing the harmony: {len(rubs)} of {len(strong)}' + (': ' + ', '.join(rubs) if rubs else ''))


MOTION = ('same', 'step', 'skip', 'leap', 'big leap')


def motion(notes):
    """A part's motion from its (tick, key) onsets: notes and different
    pitches a bar (means over the bars it plays in) and its intervals'
    shares, %: the same note, a step (1-2 semitones), a skip (3-4), a leap
    (5-7), a big leap (more)."""
    notes = sorted(notes)
    ks = [k for _, k in notes]
    ivs = [b - a for a, b in zip(ks, ks[1:])]
    size = {m: 0 for m in MOTION}
    for i in ivs:
        a = abs(i)
        size['same' if a == 0 else 'step' if a <= 2 else 'skip' if a <= 4 else 'leap' if a <= 7 else 'big leap'] += 1
    bars = {}
    for t, k in notes:
        bars.setdefault(t // tm.BAR, []).append(k)
    nb = len(bars) or 1
    tot = len(ivs) or 1
    return {'notes': sum(len(b) for b in bars.values()) / nb, 'pitches': sum(len(set(b)) for b in bars.values()) / nb,
            **{m: 100 * size[m] / tot for m in MOTION}}


def piece_parts(piece):
    """The piece's parts by role: the lead square, the backing square, the
    wave's bass, and the sampled lead (its echo, the same key within four
    ticks, left out)."""
    s = piece.arrangement()
    parts = {'lead': [(t, k) for t, n, k, *_ in s.psg['sq1']], 'backing': [(t, k) for t, n, k, *_ in s.psg['sq2']],
             'bass': [(t, k) for t, n, k, *_ in s.psg['wave']]}
    leads = getattr(piece, 'SAMPLED_LEADS', ('dlead',))
    seen, sampled = {}, []
    for t, n, inst, k, *_ in sorted(s.ds, key=lambda e: e[0]):
        if inst in leads:
            if k in seen and t - seen[k] <= 4:
                continue
            seen[k] = t
            sampled.append((t, k))
    if sampled:
        parts['sampled lead'] = sampled
    return parts


def source_parts(game, song_id, offset):
    """The source song's parts by the same roles, read from the ROM: of
    its squares the one with fewer notes a bar the lead, the other the
    backing; its wave the bass; its highest Direct Sound melody the
    sampled lead."""
    import song_habits as sh
    data, table = sh.rom(game)
    song = sh.Song(data, table, song_id)
    squares, waves, ds = [], [], []
    for pc in song.tracks:
        notes, _, _ = song.walk(pc)
        if not notes:
            continue
        kinds = collections.Counter(song.voice(n[4])['kind'] for n in notes if n[4] is not None)
        kind = kinds.most_common(1)[0][0] if kinds else ''
        part = [(t - offset, k) for t, k, *_ in notes if t >= offset]
        if kind.startswith('square'):
            squares.append(part)
        elif kind == 'wave':
            waves.append(part)
        elif kind == 'Direct Sound':
            ds.append(part)
    parts = {}
    squares.sort(key=lambda p: motion(p)['notes'])
    if squares:
        parts['lead'] = squares[0]
    if len(squares) > 1:
        parts['backing'] = squares[-1]
    if waves:
        parts['bass'] = waves[0]
    if ds:
        parts['sampled lead'] = max(ds, key=lambda p: sum(k for _, k in p) / len(p))
    return parts


def discipline(piece, source=None):
    """Each part's motion, beside the source song's where it is given: a
    part far busier than its counterpart (half again as many different
    pitches a bar, or 15 points more leaping, or 15 fewer repeated notes)
    is flagged. The owner's word on a first sketch for 0.15.0, its bass
    walking to a new note on every place and every part moving at once:
    "all over the place" (music_rules.md, Discipline)."""
    mine = {role: motion(notes) for role, notes in piece_parts(piece).items()}
    theirs = {role: motion(notes) for role, notes in source_parts(*source).items()} if source else {}
    print('parts\' motion (notes a bar, different pitches a bar; intervals %: ' + ', '.join(MOTION) + ')'
          + (', the source\'s under each' if theirs else ''))
    flags = []
    for role, m in mine.items():
        print(f'  {role:12s} {m["notes"]:5.1f} {m["pitches"]:4.1f}  ' + ' '.join(f'{m[k]:3.0f}' for k in MOTION))
        o = theirs.get(role)
        if not o:
            continue
        print(f'  {"(source)":12s} {o["notes"]:5.1f} {o["pitches"]:4.1f}  ' + ' '.join(f'{o[k]:3.0f}' for k in MOTION))
        if m['pitches'] > 1.5 * o['pitches'] + 0.5:
            flags.append(f'{role}: {m["pitches"]:.1f} different pitches a bar against {o["pitches"]:.1f}')
        if m['leap'] + m['big leap'] > o['leap'] + o['big leap'] + 15:
            flags.append(f'{role}: leaps {m["leap"] + m["big leap"]:.0f}% against {o["leap"] + o["big leap"]:.0f}%')
        if m['same'] < o['same'] - 15:
            flags.append(f'{role}: repeats {m["same"]:.0f}% against {o["same"]:.0f}%')
    if theirs:
        print('busier than the source: ' + ('; '.join(flags) if flags else 'none'))
    else:
        print('(no --source: nothing to hold the parts against)')


def roll(score, bars, path):
    """The score drawn: a pixel a tick, the squares blue (sq1) and yellow
    (sq2), the wave's bass green, the stabs pink, pads violet, brass orange,
    the power chord red, a slapped bass teal, a bell pale yellow."""
    from PIL import Image, ImageDraw
    w, h = bars * tm.BAR + 20, 512
    im = Image.new('RGB', (w, h), (10, 10, 20))
    d = ImageDraw.Draw(im)
    for b in range(bars + 1):
        d.line([(b * tm.BAR, 0), (b * tm.BAR, h)], fill=(60, 60, 80))
    colours = {'sq1': (80, 200, 255), 'sq2': (255, 220, 80), 'wave': (120, 255, 120)}
    for ch, notes in score.psg.items():
        for t, n, k, _, _ in notes if ch in colours else ():
            y = h - (k - 24) * 6
            d.rectangle([t, y, t + n - 1, y + 4], fill=colours[ch])
    outline = {'pluck': (255, 100, 200), 'pad': (120, 80, 160), 'strings': (120, 80, 160), 'brass': (255, 140, 60),
               'hit': (255, 255, 255), 'orch': (255, 255, 255), 'pad_m': (120, 80, 160), 'gtr': (255, 60, 60),
               'mute': (160, 40, 40), 'dlead': (80, 140, 255), 'slap': (60, 220, 160), 'bell': (220, 220, 120),
               'horns': (255, 140, 60)}
    for t, n, inst, k, *_ in score.ds:
        if inst in outline:
            y = h - (k - 24) * 6
            d.rectangle([t, y + 1, t + n - 1, y + 3], outline=outline[inst])
    im.save(path)


def main():
    args = sys.argv[1:]
    ref = None
    if '--ref' in args:
        i = args.index('--ref')
        ref = args[i + 1]
        del args[i:i + 2]
    name = None
    if '--piece' in args:
        i = args.index('--piece')
        name = args[i + 1]
        del args[i:i + 2]
    source = None
    if '--source' in args:
        i = args.index('--source')
        source = [args[i + 1], int(args[i + 2], 0), 0]
        del args[i:i + 3]
        if '--offset' in args:
            i = args.index('--offset')
            source[2] = int(args[i + 1], 0)
            del args[i:i + 2]
    version = name or (args[0] if args else trailers.CURRENT)
    piece = importlib.import_module('trailers.' + name) if name else trailers.music(version)
    work = os.path.join(ROOT, '.build', 'trailer-' + version)
    os.makedirs(work, exist_ok=True)
    path = os.path.join(work, 'music-check.wav')
    tm.render_any(path, piece)
    w = wave.open(path)
    n, sr = w.getnframes(), w.getframerate()
    d = struct.unpack('<%dh' % (2 * n), w.readframes(n))
    bar = sr * 60 / piece.BPM * 4
    print(f'{n / sr:.1f} s at {piece.BPM} beats a minute, a bar {bar / sr:.3f} s ({bar / sr * 30:.2f} video frames)')
    for b in range(int(n / bar) + 1):
        seg = d[int(b * bar) * 2:int((b + 1) * bar) * 2]
        if not seg:
            break
        rms = math.sqrt(sum(x * x for x in seg) / len(seg)) / 32768
        peak = max(abs(x) for x in seg) / 32768
        near = sum(1 for x in seg if abs(x) > 32000)
        print(f'  bar {b:2d}: loudness {20 * math.log10(rms + 1e-9):6.1f} dB, peak {20 * math.log10(peak + 1e-9):5.1f} dB'
              + (f', {near} samples near clipping' if near else ''))
    mono = [(d[2 * i] + d[2 * i + 1]) / 65536 for i in range(n)]
    step = sr // 8000
    mine = [sum(mono[i:i + step]) / step for i in range(0, len(mono) - step, step)]
    print('balance low/mid/high %:', bands(mine[8000 * 2:8000 * 30], 8000))
    print('bass/body/mids/top % (120, 500, 2000 Hz):', bands(mine[8000 * 2:8000 * 30], 8000, (120, 500, 2000)))
    print(f'punch (the swing inside a beat): {punch(mine[8000 * 2:8000 * 30], 8000, piece.BPM):.1f} dB')
    if ref:
        local = os.path.join(work, 'ref' + os.path.splitext(ref)[1])
        shutil.copy(ref, local)   # (inside the repository: Docker mounts no /tmp)
        raw = os.path.join(work, 'ref.raw')
        rel = lambda p: '/src/' + os.path.relpath(p, ROOT)  # noqa: E731
        subprocess.check_call(['docker', 'run', '--rm', '--name', f'cw-music-check-{os.getpid()}', '-u',
                               f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '--entrypoint', 'ffmpeg',
                               build.FFMPEG_IMAGE, '-y', '-loglevel', 'error', '-i', rel(local), '-ac', '1', '-ar', '8000',
                               '-f', 's16le', rel(raw)])
        x = [v / 32768 for v in array.array('h', open(raw, 'rb').read())]
        print('reference low/mid/high %:', bands(x[8000 * 10:8000 * 40] or x, 8000))
        print('reference bass/body/mids/top %:', bands(x[8000 * 10:8000 * 40] or x, 8000, (120, 500, 2000)))
        print(f'reference punch: {punch(x[8000 * 10:8000 * 40] or x, 8000, piece.BPM):.1f} dB')
    clashes(piece)
    discipline(piece, source)
    roll(piece.arrangement(), piece.BARS, os.path.join(work, 'roll.png'))
    print('wrote', os.path.relpath(path, ROOT), 'and', os.path.relpath(os.path.join(work, 'roll.png'), ROOT))


if __name__ == '__main__':
    main()
