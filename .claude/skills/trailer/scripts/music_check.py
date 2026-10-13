#!/usr/bin/env python3
"""A trailer piece checked without ears: rendered, then its loudness and
peaks bar by bar (no bar near clipping, the drop and the climax where the
plan put them), its balance of lows, mids and highs beside a reference
recording's (the game it is in the manner of), and a piano roll of its
score to look at (the squares, the wave's bass, the stabs and pads).

    python3 .claude/skills/trailer/scripts/music_check.py [VERSION] [--ref AUDIO]

VERSION is a trailer's (tools/trailers/), the current one by default. AUDIO
is any recording ffmpeg reads (the owner's, kept in .build/ and never
committed); it is decoded to 8 kHz mono in the build's ffmpeg image. The
wave, the roll and the decoded reference go to .build/trailer-VERSION/.
"""
import array
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


def bands(x, sr):
    """The shares of energy below 200 Hz, 200-1500 Hz and above, in %."""
    def lowpass(sig, fc):
        a, y, out = 1 - math.exp(-2 * math.pi * fc / sr), 0.0, []
        for v in sig:
            y += a * (v - y)
            out.append(y)
        return out
    l200, l1500 = lowpass(x, 200), lowpass(x, 1500)
    energy = [sum(v * v for v in l200), sum((a - b) ** 2 for a, b in zip(l1500, l200)),
              sum((a - b) ** 2 for a, b in zip(x, l1500))]
    total = sum(energy) or 1
    return [round(100 * e / total) for e in energy]


def roll(score, bars, path):
    """The score drawn: a pixel a tick, the squares blue (sq1) and yellow
    (sq2), the wave's bass green, the stabs pink, pads violet, brass orange."""
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
               'hit': (255, 255, 255), 'orch': (255, 255, 255)}
    for t, n, inst, k, _, _ in score.ds:
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
    version = args[0] if args else trailers.CURRENT
    piece = trailers.music(version)
    work = os.path.join(ROOT, '.build', 'trailer-' + version)
    os.makedirs(work, exist_ok=True)
    path = os.path.join(work, 'music-check.wav')
    tm.render(path, piece)
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
    roll(piece.arrangement(), piece.BARS, os.path.join(work, 'roll.png'))
    print('wrote', os.path.relpath(path, ROOT), 'and', os.path.relpath(os.path.join(work, 'roll.png'), ROOT))


if __name__ == '__main__':
    main()
