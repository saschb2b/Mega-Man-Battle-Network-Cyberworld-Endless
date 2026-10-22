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
    python3 tools/trailer_music.py OUT.wav --piece sketch_a
        (a piece in tools/trailers/ that is no trailer's: a sketch)
"""
import math
import random
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


def bent(points, tick):
    """A bend's semitones `tick` ticks into its note: its (tick, semitones)
    points joined by straight lines, held past the last."""
    if tick <= points[0][0]:
        return points[0][1]
    for (t0, v0), (t1, v1) in zip(points, points[1:]):
        if tick < t1:
            return v0 + (v1 - v0) * (tick - t0) / (t1 - t0)
    return points[-1][1]


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


# ---- a sampled kit: our own tones in the manner of MP2K's samples ----
#
# BN6's songs play 8-bit samples of a synthesizer's sounds (Kaida: an
# XV-1080 and SC-88s), cut short and normalised to the top: its kick spends
# half its 139 ms at full scale, its clap a quarter of its 82 ms; whole
# chords are one sample (sus voicings: a root, its fourth or second, its
# fifth, octaves), recoloured by the bass under them; its leads are a few
# cycles looped; its low stabs are distorted, played with pitch bends.
# These are made from sines, saws and noise here, never from the game's.

def clip(x, drive):
    return max(-1.0, min(1.0, x * drive))


def lowpass(xs, hz):
    a, y, out = 1 - math.exp(-2 * math.pi * hz / SAMPLE_RATE), 0.0, []
    for x in xs:
        y += a * (x - y)
        out.append(y)
    return out


def highpass(xs, hz):
    a, y, prev, out = math.exp(-2 * math.pi * hz / SAMPLE_RATE), 0.0, 0.0, []
    for x in xs:
        y = a * (y + x - prev)
        prev = x
        out.append(y)
    return out


def punch_kick(seed=1, seconds=0.14, low=48, high=170, drive=4.0, body=0.055, hold=0.045):
    """A short kick driven into the top: a falling sine, clipped, a click,
    then let go (BN6's is 20 dB down 66 ms in)."""
    rnd, sr, ph, out = random.Random(seed), SAMPLE_RATE, 0.0, []
    for i in range(int(seconds * sr)):
        t = i / sr
        ph += 2 * math.pi * (low + high * math.exp(-t / 0.012)) / sr
        x = math.sin(ph) * math.exp(-t / body) + 0.5 * math.exp(-t / 0.0015) * rnd.uniform(-1, 1)
        tail = 1.0 if t < hold else math.exp(-(t - hold) / 0.018)
        out.append(clip(x * min(1.0, t / 0.0015 + 0.3), drive) * tail)
    return Sample(out)


def punch_clap(seed=2, seconds=0.085, drive=3.2, body=200):
    """A clap and snare in one: three bursts of noise and a body, clipped."""
    rnd, sr, out, prev, hp = random.Random(seed), SAMPLE_RATE, [], 0.0, 0.0
    for i in range(int(seconds * sr)):
        t = i / sr
        n = rnd.uniform(-1, 1)
        hp = 0.5 * (hp + n - prev)
        prev = n
        bursts = max(math.exp(-(t - b) / 0.004) if t >= b else 0 for b in (0.0, 0.007, 0.014))
        x = hp * (bursts + math.exp(-t / 0.028)) + 0.5 * math.sin(2 * math.pi * body * t) * math.exp(-t / 0.02)
        out.append(clip(x, drive))
    return Sample(out)


def chord_pad(intervals, cycles=96, n=4096, attack=0.08, bright=2200, detune=1, seed=3):
    """A whole chord as one sample, its loop whole cycles of detuned saws
    (`intervals` above the root, rounded to whole cycles: just and a
    little wide), after an attack whose brightness swells; base: the root."""
    sr = SAMPLE_RATE
    parts = [round(cycles * 2 ** (i / 12)) for i in intervals]
    pre = int(attack * sr)
    out = []
    for i in range(pre + n):
        j = i - pre if i >= pre else i
        b = bright * (0.35 + 0.65 * min(1.0, i / max(1, pre)))
        x = 0.0
        for c in parts:
            x += saw(j, n, c, b) + saw(j, n, c + detune, b) * 0.8
        out.append(x * (min(1.0, i / max(1, pre)) if i < pre else 1.0))
    base = 69 + 12 * math.log2(cycles * sr / n / 440)
    return Sample(out, base=base, loop=pre)


def power_stab(root=48, seconds=0.32, drive=7.0, tone=1700, mute=False, seed=4, thin=260):
    """A distorted power chord (root, fifth, octave) with the pick's
    scrape, darkened as a cabinet darkens it and thinned under `thin` hertz
    (BN6's sit between 250 and 2000 Hz: its fundamentals are felt, not
    heard); `mute` palm-mutes it. Its base is `root`, to be bent a frame at
    a time (Score.hit's bend)."""
    rnd, sr = random.Random(seed), SAMPLE_RATE
    f = hz(root)
    raw = []
    for i in range(int(seconds * sr)):
        t = i / sr
        x = 0.0
        for ratio, a in ((1.0, 1.0), (1.4983, 0.8), (2.0, 0.6)):
            for d in (-0.003, 0.003):
                p = (f * ratio * (1 + d) * t) % 1.0
                x += a * (2 * p - 1)
        env = (1 - math.exp(-t / 0.002)) * (math.exp(-t / (0.05 if mute else 0.22)))
        x = math.tanh(drive * x * env / 3) + 0.4 * math.exp(-t / 0.004) * rnd.uniform(-1, 1)
        raw.append(x)
    return Sample(highpass(highpass(lowpass(lowpass(raw, 900 if mute else tone), 2 * tone), thin), thin), base=root)


def cycle_lead(shape='pulse', cycles=16, n=1024, attack=0.03, bright=3000):
    """A lead of a few cycles looped (a pulse with a saw's edge, or a
    rounder 'soft' tone), a brighter scrape before the loop."""
    sr = SAMPLE_RATE
    pre = int(attack * sr)
    out = []
    for i in range(pre + n):
        j = i - pre if i >= pre else i
        b = bright * (2 if i < pre else 1)
        if shape == 'soft':
            x = saw(j, n, cycles, b * 0.4) + 0.5 * math.sin(2 * math.pi * cycles * j / n)
        else:
            x = saw(j, n, cycles, b) - saw(j + n // (cycles * 4), n, cycles, b) + 0.35 * saw(j, n, cycles, b)
        out.append(x)
    base = 69 + 12 * math.log2(cycles * sr / n / 440)
    return Sample(out, base=base, loop=pre)


def slap_bass(root=40, seconds=0.5, seed=5):
    """A slapped bass: the thumb's pop, a saw body whose filter closes."""
    rnd, sr = random.Random(seed), SAMPLE_RATE
    f = hz(root)
    out, y, ph = [], 0.0, 0.0
    for i in range(int(seconds * sr)):
        t = i / sr
        ph = (ph + f / sr) % 1.0
        x = (2 * ph - 1) + 0.6 * math.sin(2 * math.pi * ph)
        cut = 450 + 3200 * math.exp(-t / 0.035)
        y += (1 - math.exp(-2 * math.pi * cut / sr)) * (x - y)
        pop = (0.9 * math.exp(-t / 0.003) * rnd.uniform(-1, 1)
               + 0.6 * math.sin(2 * math.pi * 3 * f * t) * math.exp(-t / 0.01))
        out.append(math.tanh(1.6 * (y * math.exp(-t / 0.4) + pop)))
    return Sample(out, base=root)


def fm_bell(root=72, seconds=0.7, ratio=3.5, index=2.2):
    """An electric piano's bell: a sine whose sidebands fade."""
    sr, f, out = SAMPLE_RATE, hz(root), []
    for i in range(int(seconds * sr)):
        t = i / sr
        m = index * math.exp(-t / 0.12)
        out.append(math.sin(2 * math.pi * f * t + m * math.sin(2 * math.pi * f * ratio * t)) * math.exp(-t / 0.35)
                   * (1 - math.exp(-t / 0.002)))
    return Sample(out, base=root)


def brass_section(intervals, root=60, seconds=0.45, seed=6):
    """A synth brass section as one chord: detuned saws whose filter opens
    on the blat and settles, a short fall of breath at its end."""
    rnd, sr = random.Random(seed), SAMPLE_RATE
    fs = [hz(root + i) for i in intervals]
    out, y, ph = [], 0.0, [rnd.random() for _ in fs for _ in (0, 1)]
    for i in range(int(seconds * sr)):
        t = i / sr
        x = 0.0
        for j, fr in enumerate(fs):
            for d, k in ((-0.004, 0), (0.004, 1)):
                p = ph[2 * j + k] = (ph[2 * j + k] + fr * (1 + d) / sr) % 1.0
                x += 2 * p - 1
        cut = 700 + 2600 * (1 - math.exp(-t / 0.025)) * math.exp(-t / 0.18) + 900
        y += (1 - math.exp(-2 * math.pi * cut / sr)) * (x - y)
        out.append(y * (1 - math.exp(-t / 0.012)) * (0.65 + 0.35 * math.exp(-t / 0.15)))
    return Sample(out, base=root)


def conga(root=60, seconds=0.22, seed=8, slap=0.6):
    """A hand drum: a sine bending down onto its pitch, the palm's slap."""
    rnd, sr, out, ph = random.Random(seed), SAMPLE_RATE, [], 0.0
    f = hz(root)
    for i in range(int(seconds * sr)):
        t = i / sr
        ph += 2 * math.pi * f * (1 + 0.25 * math.exp(-t / 0.008)) / sr
        out.append(math.sin(ph) * math.exp(-t / 0.07) + slap * rnd.uniform(-1, 1) * math.exp(-t / 0.004))
    return Sample(out, base=root)


def impact(seed=7, seconds=0.38):
    """A hit for the downbeats: a low thump, a burst and a ring of noise."""
    rnd, sr, out, lo, ph = random.Random(seed), SAMPLE_RATE, [], 0.0, 0.0
    for i in range(int(seconds * sr)):
        t = i / sr
        ph += 2 * math.pi * (55 + 90 * math.exp(-t / 0.03)) / sr
        n = rnd.uniform(-1, 1)
        lo += 0.25 * (n - lo)
        x = 1.2 * math.sin(ph) * math.exp(-t / 0.12) + 0.9 * lo * math.exp(-t / 0.09) + 0.5 * n * math.exp(-t / 0.01)
        out.append(clip(x, 1.8))
    return Sample(out)


# ---- the score ----

class Score:
    def __init__(self):
        self.psg = {c: [] for c in ('sq1', 'sq2', 'wave', 'noise', 'fx')}
        self.ds = []

    def note(self, ch, bar, tick, length, k, vel=127, **o):
        self.psg[ch].append((bar * BAR + tick, length, key(k), vel, o))

    def hit(self, inst, bar, tick, length, k=60, vel=127, pan=0.0, bend=None):
        """A Direct Sound note; `bend`, as a PSG note's, a list of (tick,
        semitones) points from its start (a fall, a dive, a scoop),
        followed a frame at a time as MP2K's pitch bend is."""
        if bend:
            self.ds.append((bar * BAR + tick, length, inst, key(k), vel, pan, bend))
        else:
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


def render_channel(kind, notes, L, R, gain, bpm, hardware=None):
    """One Game Boy channel's notes into L and R: the envelope steps a tick
    (a frame at 150 beats a minute). With `hardware` (a piece's HARDWARE,
    32768: the rate the Game Boy Advance's output samples its sound at) a
    square is not smoothed: its edges fall on that rate's samples, held,
    and fold back as the console's do (the grit around its notes)."""
    notes = sorted(notes, key=lambda n: n[0])
    phase = 0.0
    held = 0
    for i, (tick, length, k, vel, o) in enumerate(notes):
        cut = notes[i + 1][0] - tick if i + 1 < len(notes) else None
        levels = o.get('ramp') and [round(o['ramp'][0] + (o['ramp'][1] - o['ramp'][0]) * f / max(1, length - 1))
                                    for f in range(length)] or cgb_env(max(1, round(15 * vel / 127)), o['env'], length)
        if cut is not None:
            levels = levels[:cut]
        pan = o.get('pan', 0.0)
        gl, gr = gain * min(1, 1 - pan), gain * min(1, 1 + pan)
        vib, scoop, drop, bend = o.get('vib'), o.get('scoop', 0.0), o.get('drop'), o.get('bend')
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
                if bend:
                    cents += 100 * bent(bend, f)
                pitch = hz(k) * 2 ** (cents / 1200)
                if kind == 'wave':
                    inc = gb_wave(pitch) / RATE
                    amp = 1.0 if lvl >= 10 else 0.5 if lvl >= 6 else 0.25 if lvl >= 3 else 0.0
                    for j in range(pos, end):
                        phase = (phase + inc) % 1.0
                        v = (table[int(phase * 32)] - 7.5) / 7.5 * amp
                        L[j] += v * gl
                        R[j] += v * gr
                elif hardware:
                    inc = gb_square(pitch) / hardware
                    amp = lvl / 15
                    for j in range(pos, end):
                        h = j * hardware // RATE
                        if h - held > 64:
                            held = h
                        while held < h:
                            held += 1
                            phase += inc
                            if phase >= 1.0:
                                phase -= 1.0
                        v = amp if phase < duty else -amp
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
    for tick, length, inst, k, vel, pan, *bend in events:
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
        bend = bend[0] if bend else None
        base_step = step
        while i < n:
            if i >= next_frame:
                next_frame += frame
                if bend:
                    step = base_step * 2 ** (bent(bend, (i - start) / DS_RATE * bpm * 24 / 60) / 12)
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
                           s.psg[c], L, R, gain, bpm, getattr(piece, 'HARDWARE', None))
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
    import importlib
    import trailers
    args = sys.argv[1:]
    banner = '--banner' in args
    if banner:
        args.remove('--banner')
    seconds = option(args, '--seconds', float, None)
    rate = option(args, '--rate', int, RATE)
    name = option(args, '--piece', str, None)
    version = option(args, '--trailer', str, None)
    piece = importlib.import_module('trailers.' + name) if name else trailers.music(version)
    start = 0.0
    if banner:
        start, seconds = piece.BANNER
    render(args[0] if args else 'music.wav', piece, seconds, rate, start)
