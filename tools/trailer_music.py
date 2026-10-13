#!/usr/bin/env python3
"""The trailers' music, played the way the Game Boy Advance plays it: its
two squares, wave and noise channels stepped a frame at a time as MP2K
steps them, and Direct Sound, 8-bit samples mixed at 13379 Hz with MP2K's
short echo. Each trailer's piece is an original one in tools/trailers/
(vX_Y_Z_music.py: its arrangement, instruments, tempo, length and mix),
none of Capcom's sound in any.

    python3 tools/trailer_music.py OUT.wav [--trailer VERSION]
        (the current trailer's music, or VERSION's)
    python3 tools/trailer_music.py OUT.wav --banner --rate 32728
        (its BANNER, faded out: the 3DS HOME Menu banner's sound)
"""
import math
import struct
import sys
import wave

RATE = 48000                 # the output
DS_RATE = 13379              # MP2K's mixing rate for Direct Sound
SAMPLE_RATE = 10512          # the instruments' own rate, as BN6's
BAR = 96                     # ticks a bar

STEPS = {'C': 0, 'D': 2, 'E': 4, 'F': 5, 'G': 7, 'A': 9, 'B': 11}


def key(name):
    """A note's MIDI key: 'F#5' is 78."""
    if isinstance(name, int):
        return name
    k, i = STEPS[name[0]], 1
    while name[i] in '#b':
        k += 1 if name[i] == '#' else -1
        i += 1
    return k + 12 * (int(name[i:]) + 1)


def keys(names):
    return [key(n) for n in names.split()]


def hz(k):
    return 440.0 * 2 ** ((k - 69) / 12)


def seconds_at(tick, bpm):
    return tick * 60 / (bpm * 24)


def sample_at(tick, bpm):
    """Where a tick starts in the output: at 150 beats a minute, a frame's
    800 samples a tick."""
    return round(tick * RATE * 60 / (bpm * 24))


class Sample:
    """An 8-bit sample: its rate at its base key (None: one pitch), its loop."""

    def __init__(self, data, base=None, loop=None):
        peak = max(abs(x) for x in data) or 1
        self.data = [max(-128, min(127, round(x / peak * 120))) / 128 for x in data]
        self.base = base
        self.loop = loop


def saw(i, n, cycles, bright):
    """Band-limited saw at `cycles` per n samples, its harmonics under a
    low-pass at `bright` hertz."""
    f = cycles * SAMPLE_RATE / n
    out = 0.0
    k = 1
    while k * f < SAMPLE_RATE / 2:
        out += math.sin(2 * math.pi * k * cycles * i / n) / k / (1 + (k * f / bright) ** 2)
        k += 1
    return out


# ---- the score ----

class Score:
    def __init__(self):
        self.psg = {c: [] for c in ('sq1', 'sq2', 'wave', 'noise', 'fx')}
        self.ds = []

    def note(self, ch, bar, tick, length, k, vel=127, **o):
        self.psg[ch].append((bar * BAR + tick, length, key(k), vel, o))

    def hit(self, inst, bar, tick, length, k=60, vel=127, pan=0.0):
        self.ds.append((bar * BAR + tick, length, inst, key(k), vel, pan))

    def chord(self, inst, bar, tick, length, names, vel=127, spread=0.5):
        ks = keys(names)
        for i, k in enumerate(ks):
            self.hit(inst, bar, tick, length, k, vel, -spread + 2 * spread * i / max(1, len(ks) - 1))


# ---- the Game Boy's channels ----

def gb_square(f):
    """A square's pitch as the Game Boy can set it: 131072 / (2048 - x)."""
    x = min(2047, max(0, round(2048 - 131072 / f)))
    return 131072 / (2048 - x)


def gb_wave(f):
    x = min(2047, max(0, round(2048 - 65536 / f)))
    return 65536 / (2048 - x)


def cgb_env(peak, env, on):
    """The 4-bit volume a frame: MP2K steps a Game Boy channel's envelope
    once a frame, `on` frames held, then released."""
    a, d, s, r = env
    sus = round(peak * s / 15)
    lvl, phase, count = (0, 'a', 0) if a else ((peak, 'd', 0) if d else (sus, 's', 0))
    out = []
    for f in range(1200):
        if f == on:
            phase, count = 'r', 0
            if r == 0:
                break
        out.append(lvl)
        count += 1
        if phase == 'a' and count >= a:
            count, lvl = 0, lvl + 1
            if lvl >= peak:
                lvl, phase = (peak, 'd') if d else (sus, 's')
        elif phase == 'd' and count >= d:
            count, lvl = 0, lvl - 1
            if lvl <= sus:
                lvl, phase = sus, 's'
        elif phase == 'r' and count >= r:
            count, lvl = 0, lvl - 1
            if lvl <= 0:
                break
    return out


def lfsr(bits):
    reg, out = 0x7FFF, []
    for _ in range((1 << bits) - 1):
        b = (reg ^ (reg >> 1)) & 1
        reg = (reg >> 1) | (b << 14)
        if bits == 7:
            reg = (reg & ~0x40) | (b << 6)
        out.append(1.0 if reg & 1 else -1.0)
    return out


NOISE = lfsr(15)


def polyblep(t, dt):
    if t < dt:
        t /= dt
        return t + t - t * t - 1
    if t > 1 - dt:
        t = (t - 1) / dt
        return t * t + t + t + 1
    return 0.0


def render_channel(kind, notes, L, R, gain, bpm):
    """One Game Boy channel's notes into L and R: the envelope steps a tick
    (a frame at 150 beats a minute)."""
    notes = sorted(notes, key=lambda n: n[0])
    phase = 0.0
    for i, (tick, length, k, vel, o) in enumerate(notes):
        cut = notes[i + 1][0] - tick if i + 1 < len(notes) else None
        levels = o.get('ramp') and [round(o['ramp'][0] + (o['ramp'][1] - o['ramp'][0]) * f / max(1, length - 1))
                                    for f in range(length)] or cgb_env(max(1, round(15 * vel / 127)), o['env'], length)
        if cut is not None:
            levels = levels[:cut]
        pan = o.get('pan', 0.0)
        gl, gr = gain * min(1, 1 - pan), gain * min(1, 1 + pan)
        vib, scoop, drop = o.get('vib'), o.get('scoop', 0.0), o.get('drop')
        duty = o.get('duty', 0.5)
        table = o.get('wave')
        for f, lvl in enumerate(levels):
            pos, end = sample_at(tick + f, bpm), min(sample_at(tick + f + 1, bpm), len(L))
            if pos >= len(L):
                break
            if kind == 'noise':
                if 'sweep' in o:
                    a, b = o['sweep']
                    clock = a * (b / a) ** (f / max(1, len(levels) - 1))
                else:
                    clock = o['clock']
                step = clock / RATE
                amp = lvl / 15
                for j in range(pos, end):
                    phase += step
                    v = NOISE[int(phase) % 32767] * amp
                    L[j] += v * gl
                    R[j] += v * gr
            else:
                cents = 0.0
                if vib and f >= vib[1]:
                    x = ((f - vib[1]) * 22 % 256) / 256
                    cents = vib[0] * (4 * x - 1 if x < 0.5 else 3 - 4 * x)
                if scoop and f < 4:
                    cents -= 100 * scoop * (4 - f) / 4
                if drop:
                    cents -= drop * f / max(1, len(levels) - 1)
                pitch = hz(k) * 2 ** (cents / 1200)
                if kind == 'wave':
                    inc = gb_wave(pitch) / RATE
                    amp = 1.0 if lvl >= 10 else 0.5 if lvl >= 6 else 0.25 if lvl >= 3 else 0.0
                    for j in range(pos, end):
                        phase = (phase + inc) % 1.0
                        v = (table[int(phase * 32)] - 7.5) / 7.5 * amp
                        L[j] += v * gl
                        R[j] += v * gr
                else:
                    inc = gb_square(pitch) / RATE
                    amp = lvl / 15
                    for j in range(pos, end):
                        phase += inc
                        if phase >= 1.0:
                            phase -= 1.0
                        v = 1.0 if phase < duty else -1.0
                        v += polyblep(phase, inc)
                        t2 = phase - duty
                        v -= polyblep(t2 + 1.0 if t2 < 0 else t2, inc)
                        v *= amp
                        L[j] += v * gl
                        R[j] += v * gr


# ---- Direct Sound ----

def render_ds(events, S, n, bpm, env, gains, reverb):
    """The Direct Sound mix at 13379 Hz: nearest samples, MP2K's envelope a
    frame, its echo, 8-bit out."""
    L, R = [0.0] * n, [0.0] * n
    frame = DS_RATE / 60
    for tick, length, inst, k, vel, pan in events:
        smp = S[inst]
        a, d, sus, r = env[inst]
        step = SAMPLE_RATE / DS_RATE * (2 ** ((k - smp.base) / 12) if smp.base is not None else 1.0)
        start = round(seconds_at(tick, bpm) * DS_RATE)
        off = round(seconds_at(tick + length, bpm) * DS_RATE)
        g = gains[inst] * vel / 127
        gl, gr = g * min(1, 1 - pan), g * min(1, 1 + pan)
        lvl = min(255, a)
        phase = 'a' if lvl < 255 else ('d' if d else 's')
        next_frame = start + frame
        released = False
        data, size = smp.data, len(smp.data)
        pos = 0.0
        i = start
        while i < n:
            if i >= next_frame:
                next_frame += frame
                if not released and i >= off:
                    released = True
                if released:
                    lvl = lvl * r >> 8
                    if lvl <= 0:
                        break
                elif phase == 'a':
                    lvl = min(255, lvl + a)
                    if lvl == 255:
                        phase = 'd' if d else 's'
                elif phase == 'd':
                    lvl = max(sus, lvl * d >> 8)
                    if lvl <= sus:
                        phase = 's'
                elif phase == 's':
                    lvl = min(lvl, sus) if d else lvl
            idx = int(pos)
            if idx >= size:
                if smp.loop is None:
                    break
                pos -= size - smp.loop
                idx = int(pos)
            v = data[idx] * lvl / 255
            L[i] += v * gl
            R[i] += v * gr
            pos += step
            i += 1
    # MP2K's echo (reverb 40 in BN6's battle songs): the mix of 6 and 7
    # frames ago, both sides, fed back into both
    rev = reverb / 512
    d1, d2 = 7 * 224, 6 * 224
    for i in range(d1, n):
        e = (L[i - d1] + R[i - d1] + L[i - d2] + R[i - d2]) * rev
        L[i] += e
        R[i] += e
    peak = max(max(map(abs, L)), max(map(abs, R))) or 1
    g = 1.04 / peak
    q = [[max(-128, min(127, round(x * g * 127))) / 127 for x in ch] for ch in (L, R)]
    return q


# ---- the mix ----

def render(path, piece, seconds=None, rate=RATE, start=0.0):
    """A piece (a trailer's vX_Y_Z_music module) into the WAV at `path`;
    only `seconds` of it from `start`, faded out over the last half second,
    where given, and resampled to `rate`."""
    bpm = piece.BPM
    total = int(((start + seconds) if seconds else seconds_at(piece.BARS * BAR, bpm) + piece.TAIL) * RATE)
    s = piece.arrangement()
    parts = {}
    for name, chans, gain in piece.MIX:
        L, R = [0.0] * total, [0.0] * total
        for c in chans:
            render_channel('noise' if c in ('noise', 'fx') else 'wave' if c == 'wave' else 'square',
                           s.psg[c], L, R, gain, bpm)
        parts[name] = (L, R)
    S = piece.instruments()
    n_ds = int(total * DS_RATE / RATE) + 1
    dl, dr = render_ds(s.ds, S, n_ds, bpm, piece.DS_ENV, piece.DS_GAIN, piece.REVERB)
    L, R = [0.0] * total, [0.0] * total
    ratio = DS_RATE / RATE
    lp = 1 - math.exp(-2 * math.pi * 9000 / RATE)
    yl = yr = 0.0
    for i in range(total):
        j = int(i * ratio)
        yl += lp * (dl[j] - yl)
        yr += lp * (dr[j] - yr)
        L[i], R[i] = yl * piece.DS_LEVEL, yr * piece.DS_LEVEL
    parts['ds'] = (L, R)
    out_l, out_r = [0.0] * total, [0.0] * total
    for pl, pr in parts.values():
        for i in range(total):
            out_l[i] += pl[i]
            out_r[i] += pr[i]
    # a DC block, a soft limit, -1 dB
    hp = 1 - 2 * math.pi * 20 / RATE
    for ch in (out_l, out_r):
        x1 = y1 = 0.0
        for i, x in enumerate(ch):
            y1 = hp * (y1 + x - x1)
            x1 = x
            ch[i] = y1
    if start:
        # (an excerpt: the part before it dropped, a click's worth faded in)
        skip, fade = int(start * RATE), int(0.015 * RATE)
        out_l, out_r = out_l[skip:], out_r[skip:]
        total -= skip
        for ch in (out_l, out_r):
            for i in range(fade):
                ch[i] *= i / fade
    if seconds:
        fade = int(0.5 * RATE)
        for ch in (out_l, out_r):
            for i in range(max(0, total - fade), total):
                ch[i] *= (total - i) / fade
    if rate != RATE:
        n = int(total * rate / RATE)
        def at(ch, t):
            j = int(t)
            f = t - j
            return ch[j] * (1 - f) + ch[min(j + 1, total - 1)] * f
        out_l, out_r = ([at(ch, i * RATE / rate) for i in range(n)] for ch in (out_l, out_r))
    peak = max(max(map(abs, out_l)), max(map(abs, out_r))) or 1
    drive = 0.9 / peak
    norm = 0.89 / math.tanh(0.9)
    with wave.open(path, 'wb') as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(rate)
        buf = bytearray()
        for a, b in zip(out_l, out_r):
            buf += struct.pack('<hh', int(math.tanh(a * drive) * norm * 32767), int(math.tanh(b * drive) * norm * 32767))
        w.writeframes(bytes(buf))
    return parts


def option(args, name, cast, default):
    """The value after `name` in args (both taken out), else `default`."""
    if name not in args:
        return default
    i = args.index(name)
    value = cast(args[i + 1])
    del args[i:i + 2]
    return value


if __name__ == '__main__':
    import trailers
    args = sys.argv[1:]
    banner = '--banner' in args
    if banner:
        args.remove('--banner')
    seconds = option(args, '--seconds', float, None)
    rate = option(args, '--rate', int, RATE)
    piece = trailers.music(option(args, '--trailer', str, None))
    start = 0.0
    if banner:
        start, seconds = piece.BANNER
    render(args[0] if args else 'music.wav', piece, seconds, rate, start)
