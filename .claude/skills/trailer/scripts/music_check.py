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
--piece NAME a module there that is no trailer's (a sketch: sketch_a). AUDIO
is any recording ffmpeg reads (the owner's, kept in .build/ and never
committed); it is decoded to 8 kHz mono in the build's ffmpeg image. The
wave, the roll and the decoded reference go to .build/trailer-VERSION/.
"""
import array
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
    roll(piece.arrangement(), piece.BARS, os.path.join(work, 'roll.png'))
    print('wrote', os.path.relpath(path, ROOT), 'and', os.path.relpath(os.path.join(work, 'roll.png'), ROOT))


if __name__ == '__main__':
    main()
