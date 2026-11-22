"""0.12.0's trailer music: an original anthem in the manner of Mega Man
Battle Network 5 Team Colonel's title theme, played the way the Game Boy
Advance plays it (tools/trailer_music.py's model of its channels and Direct
Sound).

How BN5's title theme is written, read from its song 0x01 with an MP2K
reader and from a recording; nothing of it is used here but these habits:
- 142 beats a minute in B major, and bright: the raised fourth (E#) of
  Lydian and the lowered seventh (A) of Mixolydian, never the minor's
  thirds. The melody comes in on a pickup, two sixteenths before the bar.
- Six tracks. The second square, 50% and held at full volume, plays the
  line in sixteenths and eighths high in the fifth octave; the first,
  plucked (a quick decay to 11 of 15), runs arpeggios across three
  octaves. The wave channel plays the bass in eighths, plucked too.
- Direct Sound plays staccato chords five ticks long, of a soft looped tone
  that decays to a quarter in a few frames; chords held for a bar; and a
  busy kit: closed hats on every sixteenth, the kick paired with the
  sixteenth after it, the snare on two and four, an open hat on three and
  the crash on the downbeat and on the and of three. Its echo is 50.
- The form: chords held over a pedal before the beat comes in, a breakdown
  of the first square alone with the bass all but gone, then the main
  section twice.

Here, at 144 (a bar is 50 frames of the trailer's 30) in B: an intro over a
B pedal (I, bVII, bVI-bVII, then IV and V); the theme over I, II over I
(the Lydian lift), vi, IV-V, then I, bVII, IV (its melody on the raised
fourth again), V; a breakdown on the borrowed minor iv, as dark as the
piece gets, climbing through bVI and bVII; the theme again; and the logo's
Mixolydian cadence, bVII, IV, I, its last chord with the raised fourth on
top. Its BANNER, the 3DS HOME Menu's sound while it is the current
trailer: the logo's last two beats and that cadence, ringing.

    python3 tools/trailer_music.py OUT.wav --trailer 0.12.0
"""
import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_music as tm  # noqa: E402
from trailer_music import Sample, Score, key, keys, saw  # noqa: E402
from trailers import v0_3_0_music as first  # noqa: E402

BPM = 144
BARS = 20
TAIL = 2.6
MIX = (('arps', ('sq1',), 0.15), ('lead', ('sq2',), 0.23), ('bass', ('wave',), 0.3),
       ('hats', ('noise',), 0.07), ('fx', ('fx',), 0.07))
DS_LEVEL = 0.72
REVERB = 50
BANNER = (74 * 60 / BPM, 2.9)   # (from bar 18's third beat, seconds)

# ---- the voices ----

LEAD = dict(duty=0.5, env=(0, 0, 15, 1), pan=0.1, vib=(22, 12))
THIN = dict(LEAD, duty=0.25)
ARP = dict(duty=0.25, env=(0, 1, 11, 1), pan=-0.45)
SAW = [15 - i // 2 for i in range(32)]
BASS = dict(env=(0, 5, 4, 1), wave=SAW)
BASS_HOLD = dict(env=(0, 0, 15, 2), wave=SAW)
HAT = dict(first.HAT, pan=0.25)
OPEN = dict(first.OPEN, pan=0.3)
CRASH = dict(first.CRASH, pan=-0.2)

DS_ENV = dict(first.DS_ENV, kick5=(255, 0, 255, 180), snare5=(255, 0, 255, 200), clap=(255, 0, 255, 200),
              hit=(255, 0, 255, 236), pluck=(255, 236, 69, 140), pad=(7, 0, 255, 240), timp=(255, 0, 255, 238),
              bell=(255, 0, 255, 246))
DS_GAIN = dict(first.DS_GAIN, kick5=1.0, snare5=0.78, clap=0.42, hit=0.58, pluck=0.24, pad=0.15, timp=0.72,
               bell=0.3, brass=0.24)


def instruments():
    """0.3.0's piece's kit and brass, and this one's own:
    a tighter kick and snare, a clap, a synth hit, the plucked stab, a pad,
    a timpani and a bell."""
    S = first.instruments()
    rnd = random.Random(5)
    sr = tm.SAMPLE_RATE

    def length(sec):
        return range(int(sec * sr))

    kick, ph = [], 0.0
    for i in length(0.2):
        t = i / sr
        ph += 2 * math.pi * (50 + 190 * math.exp(-t / 0.018)) / sr
        x = math.sin(ph) * math.exp(-t / 0.075) + 0.7 * math.exp(-t / 0.0012) * rnd.uniform(-1, 1)
        kick.append(math.tanh(2.6 * x))
    S['kick5'] = Sample(kick)

    snare, hp, prev = [], 0.0, 0.0
    for i in length(0.2):
        t = i / sr
        n = rnd.uniform(-1, 1)
        hp = 0.55 * (hp + n - prev)
        prev = n
        x = (0.5 * math.sin(2 * math.pi * 205 * t) * math.exp(-t / 0.035)
             + 0.2 * math.sin(2 * math.pi * 410 * t) * math.exp(-t / 0.02)
             + 1.1 * hp * math.exp(-t / 0.06))
        snare.append(math.tanh(2.0 * x))
    S['snare5'] = Sample(snare)

    # three bursts of band-passed noise, then the room
    clap, lo, hp, prev = [], 0.0, 0.0, 0.0
    for i in length(0.16):
        t = i / sr
        n = rnd.uniform(-1, 1)
        hp = 0.6 * (hp + n - prev)
        prev = n
        lo += 0.5 * (hp - lo)
        env = max(math.exp(-(t - b) / 0.006) if t >= b else 0 for b in (0.0, 0.009, 0.018)) + 0.5 * math.exp(-t / 0.05)
        clap.append(lo * env)
    S['clap'] = Sample(clap)

    # the hit: a power chord of detuned saws on B, its brightness falling,
    # a noise burst at its start
    hit = []
    parts = [(key('B2'), 0.5), (key('B3'), 0.6), (key('F#4'), 0.5), (key('B4'), 0.4), (key('F#5'), 0.2)]
    for i in length(0.5):
        t = i / sr
        bright = 1000 + 5000 * math.exp(-t / 0.04)
        x = 0.0
        for k, a in parts:
            f = tm.hz(k)
            for d in (-0.006, 0.0, 0.006):
                h = 1
                while h * f < sr / 2 and h <= 10:
                    x += a * math.sin(2 * math.pi * h * f * (1 + d) * t) / h / (1 + (h * f / bright) ** 2)
                    h += 1
        env = (1 - math.exp(-t / 0.002)) * (0.7 * math.exp(-t / 0.12) + 0.3 * math.exp(-t / 0.35))
        hit.append(math.tanh(0.7 * (x * env + 0.7 * math.exp(-t / 0.008) * rnd.uniform(-1, 1))))
    S['hit'] = Sample(hit, base=key('B3'))

    # the stab's tone and the pad's, looped: whole cycles of detuned saws
    n = 2048
    base = 69 + 12 * math.log2(80 * sr / n / 440)
    S['pluck'] = Sample([saw(i, n, 80, 1500) + 0.7 * saw(i, n, 81, 1500) + 0.4 * saw(i, n, 160, 1500)
                         for i in range(n)], base=base, loop=0)
    n = 4096
    base = 69 + 12 * math.log2(160 * sr / n / 440)
    S['pad'] = Sample([saw(i, n, 158, 1000) + saw(i, n, 160, 1000) + saw(i, n, 161, 1000) + saw(i, n, 163, 1000)
                       for i in range(n)], base=base, loop=0)

    timp, ph = [], 0.0
    f0 = tm.hz(key('B2'))
    for i in length(0.7):
        t = i / sr
        ph += 2 * math.pi * f0 * (1 + 0.06 * math.exp(-t / 0.05)) / sr
        x = (math.sin(ph) + 0.35 * math.sin(2.0 * ph) * math.exp(-t / 0.1)) * math.exp(-t / 0.3)
        timp.append(x + 0.25 * math.exp(-t / 0.01) * rnd.uniform(-1, 1))
    S['timp'] = Sample(timp, base=key('B2'))

    bell = []
    f0 = tm.hz(key('F#5'))
    for i in length(1.1):
        t = i / sr
        index = 0.6 + 2.4 * math.exp(-t / 0.08)
        bell.append(math.sin(2 * math.pi * f0 * t + index * math.sin(2 * math.pi * 2 * f0 * t)) * math.exp(-t / 0.4))
    S['bell'] = Sample(bell, base=key('F#5'))
    return S


# ---- the parts ----

def lead(s, bar, notes, voice=LEAD, vel=120):
    for t, n, k, *extra in notes:
        o = dict(voice)
        if 'scoop' in extra:
            o['scoop'] = 1.0
        s.note('sq2', bar, t, n, k, vel, **o)


def double(s, bar, notes, vel=92):
    """The line an octave down in Direct Sound's brass, for the climax."""
    for t, n, k, *_ in notes:
        s.hit('brass', bar, t, n, key(k) - 12, vel, -0.25)


# the arpeggio across three octaves: up, and back down a step short
WIDE = [0, 1, 2, 3, 4, 5, 4, 3, 1, 2, 3, 4, 5, 4, 3, 2]


def arp(s, bar, t0, t1, voicing, vel=92, step=6, pattern=WIDE, rise=0):
    v = keys(voicing)
    for i, t in enumerate(range(t0, t1, step)):
        s.note('sq1', bar, t, step, v[pattern[i % len(pattern)] % len(v)], min(127, vel + rise * i), **ARP)


# the bass in eighths: (tick, length, tone) of Root, Octave and an Approach
# to the next root, a half step under it
PUMP = [(0, 12, 'R'), (12, 12, 'R'), (24, 12, 'O'), (36, 6, 'R'), (42, 6, 'R'), (48, 12, 'R'), (60, 12, 'R'),
        (72, 12, 'O'), (84, 12, 'A')]
PUMP_HALF = [(0, 12, 'R'), (12, 12, 'R'), (24, 12, 'O'), (36, 12, 'R')]


def bass(s, bar, t0, root, nxt, pattern=PUMP):
    r = key(root)
    n = key(nxt)
    tone = {'R': r, 'O': r + 12, 'A': n - 1 if (n - r) % 12 else r + 7}
    for t, length, sym in pattern:
        s.note('wave', bar, t0 + t, length, tone[sym], 124, **BASS)


def stabs(s, bar, ticks, chord, vel=104):
    for t in ticks:
        s.chord('pluck', bar, t, 5, chord, vel, 0.45)


OFFBEATS = (12, 36, 60, 84)
PUSH = (12, 36, 54, 60, 84)


def hats(s, bar, until=96, soft=0):
    for t in range(0, until, 6):
        if t == 48:
            s.note('noise', bar, t, 6, 0, 104 - soft, **OPEN)
        else:
            s.note('noise', bar, t, 3, 0, (116 if t % 24 == 0 else 94 if t % 12 == 0 else 74) - soft, **HAT)


def groove(s, bar, push=False, until=96):
    """The kit: the kick paired with the sixteenth after it on one and
    three, the snare and a clap on two and four, the open hat on three;
    a bar that pushes moves three's pair to its and, with a crash."""
    for t in (0, 6, 48, 60) if push else (0, 6, 48, 54):
        if t < until:
            s.hit('kick5', bar, t, 6)
    for t in (24, 72):
        if t < until:
            s.hit('snare5', bar, t, 6, vel=118, pan=0.05)
            s.hit('clap', bar, t, 6, vel=70 if t == 24 else 96, pan=-0.15)
    if until == 96:
        s.hit('snare5', bar, 90, 3, vel=48, pan=0.05)
    if push:
        crash(s, bar, 60, 104, 36)
    hats(s, bar, until)


def crash(s, bar, tick, vel=127, length=72):
    s.note('fx', bar, tick, length, 0, vel, **CRASH)


def fill(s, bar, t0=72):
    """Toms down the kit over the bar's last beat, the snare under them."""
    for t, tom, pan in ((t0, 'tom_hi', -0.4), (t0 + 6, 'tom_hi', -0.4), (t0 + 12, 'tom_mid', 0.0),
                        (t0 + 18, 'tom_lo', 0.4)):
        s.hit(tom, bar, t, 6, vel=118, pan=pan)
    s.hit('kick5', bar, t0, 6)


def downbeat(s, bar, root, vel=124):
    crash(s, bar, 0)
    s.hit('hit', bar, 0, 24, root, vel)
    s.hit('kick5', bar, 0, 6)


# ---- the score ----

# the theme: two four-bar phrases, the second climbing where the first fell
THEME = [
    [(0, 12, 'B4'), (12, 12, 'D#5'), (24, 36, 'F#5'), (60, 12, 'E5'), (72, 24, 'D#5')],
    [(0, 24, 'E#5'), (24, 12, 'D#5'), (36, 12, 'E#5'), (48, 48, 'G#5', 'scoop')],
    [(0, 12, 'G#5'), (12, 12, 'F#5'), (24, 36, 'D#5'), (60, 12, 'C#5'), (72, 24, 'B4')],
    [(0, 12, 'B4'), (12, 12, 'C#5'), (24, 24, 'E5'), (48, 30, 'F#5'), (78, 6, 'E5'), (84, 6, 'C#5'), (90, 6, 'A#4')],
    [(0, 12, 'B4'), (12, 12, 'D#5'), (24, 36, 'F#5'), (60, 12, 'G#5'), (72, 24, 'A#5')],
    [(0, 24, 'B5', 'scoop'), (24, 12, 'A5'), (36, 12, 'F#5'), (48, 48, 'E5')],
    [(0, 12, 'E5'), (12, 12, 'G#5'), (24, 36, 'B5'), (60, 12, 'A#5'), (72, 24, 'G#5')],
    [(0, 24, 'F#5'), (24, 12, 'G#5'), (36, 12, 'A#5'), (48, 48, 'C#6', 'scoop')],
]
# (bar's chords: (tick, ticks, root, the stab's chord, the arpeggio, the pad)
B_I = ('B2', 'D#4 F#4 B4', 'B3 F#4 B4 D#5 F#5 B5', 'B3 D#4 F#4 C#5')
B_II = ('B2', 'C#4 E#4 G#4', 'B3 G#4 C#5 E#5 G#5 C#6', 'B3 C#4 E#4 G#4')
B_VI = ('G#2', 'D#4 G#4 B4', 'G#3 D#4 G#4 B4 D#5 G#5', 'G#3 B3 D#4 F#4')
B_IV = ('E2', 'E4 G#4 B4', 'E3 B3 E4 G#4 B4 E5', 'E3 G#3 B3 D#4')
B_V = ('F#2', 'C#4 F#4 A#4', 'F#3 C#4 F#4 A#4 C#5 F#5', 'F#3 A#3 C#4 E4')
B_SUS = ('F#2', 'B3 C#4 F#4', 'F#3 C#4 F#4 B4 C#5 F#5', 'F#3 B3 C#4 E4')
B_bVII = ('A2', 'C#4 E4 A4', 'A3 E4 A4 C#5 E5 A5', 'A3 C#4 E4 B4')
B_bVI = ('G2', 'B3 D4 G4', 'G3 D4 G4 B4 D5 G5', 'G3 B3 D4 A4')
B_iv = ('E2', 'D4 G4 B4', 'E4 G4 B4 D5 F#5 G5', 'E3 B3 D4 G4')


def section(s, bar, plan, nxt, push, stab_vel=104, pad_vel=0):
    """One bar of the main section: (start, ticks, chord) halves or wholes,
    the bass pumping into the next bar's root."""
    for i, (t0, n, (root, stab, wide, pad)) in enumerate(plan):
        after = plan[i + 1][2][0] if i + 1 < len(plan) else nxt
        bass(s, bar, t0, root, after, PUMP if n == 96 else PUMP_HALF)
        stabs(s, bar, [t for t in (PUSH if push else OFFBEATS) if t0 <= t < t0 + n], stab, stab_vel)
        arp(s, bar, t0, t0 + n, wide, pattern=WIDE[:n // 6] if n < 96 else WIDE)
        if pad_vel:
            s.chord('pad', bar, t0, n, pad, pad_vel, 0.6)
    groove(s, bar, push)


def intro(s):
    # bar 0: the transmission, a B pedal under I with its ninth, a signal on
    # the first square
    s.note('fx', 0, 0, 96, 0, 70, sweep=(3000, 50000), ramp=(1, 5))
    s.chord('pad', 0, 0, 96, 'B2 F#3 C#4 D#4 F#4', 108, 0.6)
    s.note('wave', 0, 0, 96, 'B2', 110, **BASS_HOLD)
    s.hit('timp', 0, 0, 24, 'B2', 120)
    for t0, top in ((0, 'D#6'), (48, 'E#6')):
        for i, k in enumerate(['B4', 'F#5', 'C#6', top]):
            s.note('sq1', 0, t0 + 6 * i, 6 if i < 3 else 30, k, 76 + 8 * i, **ARP)
    s.hit('bell', 0, 66, 30, 'F#5', 70, 0.4)
    # bar 1: bVII over the pedal
    s.chord('pad', 1, 0, 96, 'B2 E3 A3 C#4 E4', 108, 0.6)
    s.note('wave', 1, 0, 96, 'B2', 110, **BASS_HOLD)
    s.hit('timp', 1, 0, 24, 'B2', 116)
    for t0, top in ((0, 'C#6'), (48, 'E6')):
        for i, k in enumerate(['A4', 'E5', 'B5', top]):
            s.note('sq1', 1, t0 + 6 * i, 6 if i < 3 else 30, k, 80 + 8 * i, **ARP)
    for i, t in enumerate(range(48, 96, 6)):
        s.hit('timp', 1, t, 6, 'B2', 50 + 8 * i)
    # bar 2: the beat comes in, bVI and bVII over the pedal
    downbeat(s, 2, 'G3', 110)
    for t in (0, 6, 48, 54):
        s.hit('kick5', 2, t, 6, vel=112)
    s.hit('snare5', 2, 72, 6, vel=110)
    hats(s, 2, soft=24)
    s.chord('pad', 2, 0, 48, 'B2 G3 B3 D4', 104, 0.6)
    s.chord('pad', 2, 48, 48, 'B2 A3 C#4 E4', 108, 0.6)
    for t in range(0, 96, 12):
        s.note('wave', 2, t, 12, 'B2' if t % 24 == 0 else 'B3', 124, **BASS)
    arp(s, 2, 0, 48, 'B3 D4 G4 B4 D5 G5', vel=70, rise=2)
    arp(s, 2, 48, 96, 'B3 E4 A4 C#5 E5 A5', vel=86, rise=2)
    # bar 3: IV, then V suspended and resolved; the snare rolls, a riser,
    # the toms, and the pickup into the theme
    s.note('fx', 3, 0, 90, 0, 110, sweep=(2500, 240000), ramp=(2, 12))
    for t in (0, 24, 48, 72):
        s.hit('kick5', 3, t, 6)
    roll = list(range(0, 48, 12)) + list(range(48, 72, 6))
    for i, t in enumerate(roll):
        s.hit('snare5', 3, t, 6, vel=56 + 60 * i // len(roll), pan=0.05)
    fill(s, 3)
    hats(s, 3, until=72, soft=10)
    s.chord('pad', 3, 0, 48, 'B2 E3 G#3 B3 E4', 108, 0.6)
    s.chord('pad', 3, 48, 24, 'C#3 F#3 B3 C#4', 112, 0.6)
    s.chord('pad', 3, 72, 24, 'C#3 F#3 A#3 C#4', 116, 0.6)
    bass(s, 3, 0, 'E2', 'F#2', PUMP_HALF)
    bass(s, 3, 48, 'F#2', 'B2', PUMP_HALF)
    arp(s, 3, 0, 48, B_IV[2], vel=90, pattern=WIDE[:8])
    arp(s, 3, 48, 72, B_SUS[2], vel=100, pattern=WIDE[:4])
    arp(s, 3, 72, 96, B_V[2], vel=104, pattern=WIDE[:4])
    lead(s, 3, [(84, 6, 'F#4'), (90, 6, 'A#4')])


def main_a(s):
    """Bars 4-11: the theme over the full band, its second phrase with the
    pad under it."""
    plans = [[(0, 96, B_I)], [(0, 96, B_II)], [(0, 96, B_VI)], [(0, 48, B_IV), (48, 24, B_SUS), (72, 24, B_V)],
             [(0, 96, B_I)], [(0, 96, B_bVII)], [(0, 96, B_IV)], [(0, 48, B_SUS), (48, 48, B_V)]]
    for i, plan in enumerate(plans):
        bar = 4 + i
        nxt = plans[i + 1][0][2][0] if i + 1 < len(plans) else 'E2'
        if i % 4 == 0:
            downbeat(s, bar, 'B3')
        section(s, bar, plan, nxt, push=i % 2 == 1, pad_vel=96 if i >= 4 else 0)
        lead(s, bar, THEME[i])
    fill(s, 7)
    # (bar 11: the snare's sixteenths into the drop)
    for i, t in enumerate(range(72, 96, 6)):
        s.hit('snare5', 11, t, 6, vel=90 + 10 * i)


def breakdown(s):
    """Bars 12-13, the DarkChips: the borrowed minor iv, the first square
    alone over a pad, the bass one note; then bVI and bVII climbing."""
    s.note('sq2', 12, 0, 30, 'B5', 116, duty=0.5, env=(0, 0, 15, 2), pan=0.0, drop=2400)
    s.hit('hit', 12, 0, 36, 'E3', 112)
    s.hit('kick5', 12, 0, 6)
    s.hit('timp', 12, 0, 48, 'E3', 120)
    crash(s, 12, 0, 96, 96)
    s.chord('pad', 12, 0, 96, B_iv[3], 112, 0.6)
    s.note('wave', 12, 0, 84, 'E2', 100, **BASS_HOLD)
    arp(s, 12, 12, 96, B_iv[2], vel=66, rise=2)
    for t in (24, 48, 72):
        s.note('noise', 12, t, 3, 0, 70, **HAT)
    s.note('fx', 12, 48, 48, 0, 90, sweep=(5000, 120000), ramp=(1, 9))
    # bar 13: the climb, the band coming back under a roll
    s.note('fx', 13, 0, 90, 0, 116, sweep=(3000, 260000), ramp=(2, 13))
    for t in (0, 24, 48, 72, 78, 84, 90):
        s.hit('kick5', 13, t, 6)
    roll = list(range(0, 48, 12)) + list(range(48, 84, 6)) + [84, 87, 90, 93]
    for i, t in enumerate(roll):
        s.hit('snare5', 13, t, 3 if t >= 84 else 6, vel=60 + 64 * i // len(roll), pan=0.05)
    s.chord('pad', 13, 0, 48, B_bVI[3], 112, 0.6)
    s.chord('pad', 13, 48, 48, B_bVII[3], 116, 0.6)
    stabs(s, 13, (0, 36), B_bVI[1], 96)
    stabs(s, 13, (48, 60, 84), B_bVII[1], 108)
    bass(s, 13, 0, 'G2', 'A2', PUMP_HALF)
    bass(s, 13, 48, 'A2', 'B2', PUMP_HALF)
    arp(s, 13, 0, 48, 'G4 B4 D5 G5', vel=92, step=3, pattern=[0, 1, 2, 3, 2, 1])
    arp(s, 13, 48, 96, 'A4 C#5 E5 A5', vel=104, step=3, pattern=[0, 1, 2, 3, 2, 1])
    lead(s, 13, [(84, 6, 'A4'), (90, 6, 'A#4')])


def main_b(s):
    """Bars 14-17: the theme again, the brass under it an octave down,
    turning at its end through bVI and bVII to the logo."""
    tunes = [THEME[0], THEME[1],
             [(0, 12, 'G#5'), (12, 12, 'F#5'), (24, 24, 'D#5'), (48, 12, 'E5'), (60, 12, 'G#5'), (72, 24, 'B5')],
             [(0, 24, 'B5'), (24, 12, 'A5'), (36, 12, 'G5'), (48, 24, 'A5'), (72, 12, 'B5'), (84, 12, 'C#6')]]
    plans = [[(0, 96, B_I)], [(0, 96, B_II)], [(0, 48, B_VI), (48, 48, B_IV)], [(0, 48, B_bVI), (48, 48, B_bVII)]]
    for i, plan in enumerate(plans):
        bar = 14 + i
        if i % 2 == 0:
            downbeat(s, bar, 'B3' if i == 0 else 'G#3')
        nxt = plans[i + 1][0][2][0] if i + 1 < len(plans) else 'B2'
        section(s, bar, plan, nxt, push=i % 2 == 1, stab_vel=112, pad_vel=104)
        lead(s, bar, tunes[i], THIN)
        double(s, bar, tunes[i])
    s.hit('hit', 17, 48, 24, 'A3', 116)
    fill(s, 17)


def ending(s):
    """Bar 18, the logo: I with its major seventh and ninth; bar 19, the end
    card: bVII, IV, I, the last ringing with the raised fourth on top."""
    downbeat(s, 18, 'B3')
    tune = [(0, 60, 'D#6', 'scoop'), (60, 12, 'C#6'), (72, 12, 'B5'), (84, 12, 'F#5')]
    lead(s, 18, tune, THIN)
    double(s, 18, tune)
    s.chord('pad', 18, 0, 96, 'B3 D#4 F#4 A#4 C#5', 108, 0.6)
    bass(s, 18, 0, 'B2', 'A2')
    stabs(s, 18, OFFBEATS, 'D#4 F#4 A#4', 110)
    arp(s, 18, 0, 96, 'B3 F#4 A#4 C#5 F#5 A#5')
    groove(s, 18)
    for t, n, root, chord, top, bass_key in ((0, 36, 'A3', 'A3 C#4 E4 A4', 'C#6', 'A2'),
                                             (36, 36, 'E3', 'G#3 B3 E4 G#4', 'B5', 'E2'),
                                             (72, 120, 'B3', 'B3 D#4 F#4 A#4 E#5', 'D#6', 'B2')):
        s.hit('hit', 19, t, min(n, 36), root, 124)
        s.chord('brass', 19, t, n, chord, 118, 0.5)
        s.hit('kick5', 19, t, 6)
        s.hit('snare5', 19, t, 6, vel=100)
        crash(s, 19, t, 127, 24 if t < 72 else 140)
        s.note('wave', 19, t, n, bass_key, 124, **BASS_HOLD)
        lead(s, 19, [(t, n, top, 'scoop')] if t == 72 else [(t, n, top)], THIN)
    s.chord('pad', 19, 72, 120, 'B3 D#4 F#4 A#4 E#5', 112, 0.6)
    s.hit('timp', 19, 72, 48, 'B2', 124)
    for i, k in enumerate(keys('A#6 F#6 E#6 D#6 B5 F#5 D#5 B4')):
        s.hit('bell', 19, 78 + 6 * i, 30, k, 92 - 6 * i, 0.5 - 0.12 * i)
    for t in range(0, 72, 6):
        s.note('noise', 19, t, 3, 0, 100 if t % 24 == 0 else 72, **HAT)


def arrangement():
    s = Score()
    intro(s)
    main_a(s)
    breakdown(s)
    main_b(s)
    ending(s)
    return s
