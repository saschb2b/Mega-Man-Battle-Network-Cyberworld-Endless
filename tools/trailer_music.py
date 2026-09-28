#!/usr/bin/env python3
"""The trailer's music: an original piece in the manner of Mega Man Battle
Network 6's battle themes, played the way the Game Boy Advance plays them.

How BN6 writes its battle music, read from its virus and boss battle songs
(0x15 and 0x16) with an MP2K reader; nothing of them is used here but these
habits:
- 150 beats a minute and six tracks, most of the song on the Game Boy's own
  channels. A 25% square plays sixteenth-note arpeggios of four- and
  five-note chords, sevenths and ninths, low in the fourth octave; the
  second square repeats them a sixteenth later as an echo, panned apart,
  until it takes the lead: 50% duty, vibrato on held notes, quick runs into
  them.
- The wave channel, a square in its 4-bit table, plays the bass: sixteenths
  that jump octaves and fifths, rest under the backbeat and walk into the
  next chord.
- The noise channel plays the hats, closed sixteenths with 32nd-note pairs
  and open ones on the off-beats, and the crash. Direct Sound (8-bit samples
  of about 10.5 kHz, mixed at 13379 Hz with MP2K's short echo) plays the
  kick, the snare and the toms, and a few stabs, hits and pads panned wide.
- Minor keys with chromatic turns, stabs on the syncopations, a fill into
  every new section.

Here, in E minor at the same tempo: the title's three words on bVI, bVII, i
hits; the groove under the jack-in; a theme in 3-3-2 over i-bVI-iv-V; a
deceptive turn to bVI for the guardians; the minor line cliché under the
Undernet; a climb through III and bVI-bVII to the theme again under the
logo; and the three hits once more, the last one major.

    python3 tools/trailer_music.py OUT.wav    (the music alone)
"""
import math
import random
import struct
import sys
import wave

RATE = 48000                 # the output
DS_RATE = 13379              # MP2K's mixing rate for Direct Sound
SAMPLE_RATE = 10512          # the instruments' own rate, as BN6's
SPF = RATE // 60             # samples a frame; at 150 beats a minute a tick (1/24 beat) is a frame
BAR = 96                     # ticks a bar
BARS = 13
TAIL = 1.6                   # seconds after the last bar

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


# ---- the voices ----

# Game Boy channels: duty, envelope (attack, decay, sustain, release as MP2K
# steps them: frames a step, sustain of 15), pan; the lead's vibrato (cents,
# frames before it starts)
ARP = dict(duty=0.25, env=(0, 2, 9, 1), pan=-0.55)
ECHO = dict(duty=0.125, env=(0, 0, 7, 1), pan=0.6)
LEAD = dict(duty=0.5, env=(0, 0, 13, 2), pan=0.05, vib=(24, 10))
BRIGHT = dict(LEAD, duty=0.25)
BASS = dict(env=(0, 0, 15, 0))
BASS_WAVE = [15, 15, 15, 15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11,
             0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4]
HAT = dict(clock=262144, env=(0, 0, 11, 0), pan=0.2)
OPEN = dict(clock=262144, env=(0, 0, 10, 3), pan=0.25)
CRASH = dict(clock=80000, env=(0, 5, 0, 5), pan=-0.15)

# Direct Sound: (attack, decay, sustain, release) as MP2K's 0-255 per frame
DS_ENV = {'kick': (255, 0, 255, 200), 'snare': (255, 0, 255, 200), 'tom_hi': (255, 0, 255, 230),
          'tom_mid': (255, 0, 255, 230), 'tom_lo': (255, 0, 255, 230), 'orch': (255, 0, 255, 236),
          'brass': (255, 244, 205, 190), 'strings': (16, 0, 255, 238)}
DS_GAIN = {'kick': 1.0, 'snare': 0.75, 'tom_hi': 0.7, 'tom_mid': 0.7, 'tom_lo': 0.75, 'orch': 0.62,
           'brass': 0.3, 'strings': 0.22}


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


def instruments():
    rnd = random.Random(11)
    sr = SAMPLE_RATE
    S = {}

    def env_len(sec):
        return range(int(sec * sr))

    kick = []
    ph = 0.0
    for i in env_len(0.26):
        t = i / sr
        ph += 2 * math.pi * (54 + 150 * math.exp(-t / 0.026)) / sr
        x = math.sin(ph) * math.exp(-t / 0.1) + 0.55 * math.exp(-t / 0.0016) * rnd.uniform(-1, 1)
        kick.append(math.tanh(2.2 * x))
    S['kick'] = Sample(kick)

    snare, hp, prev = [], 0.0, 0.0
    for i in env_len(0.22):
        t = i / sr
        n = rnd.uniform(-1, 1)
        hp = 0.7 * (hp + n - prev)
        prev = n
        x = (0.55 * math.sin(2 * math.pi * 186 * t) * math.exp(-t / 0.045)
             + 0.25 * math.sin(2 * math.pi * 332 * t) * math.exp(-t / 0.03)
             + 0.9 * hp * math.exp(-t / 0.075))
        snare.append(math.tanh(1.6 * x))
    S['snare'] = Sample(snare)

    for name, f0 in (('tom_hi', 172), ('tom_mid', 128), ('tom_lo', 94)):
        tom, ph = [], 0.0
        for i in env_len(0.45):
            t = i / sr
            ph += 2 * math.pi * f0 * (1 + 0.5 * math.exp(-t / 0.04)) / sr
            tom.append(math.sin(ph) * math.exp(-t / 0.2) + 0.3 * math.exp(-t / 0.006) * rnd.uniform(-1, 1))
        S[name] = Sample(tom)

    # the orchestra hit: a power chord of brass and strings on C, a noise
    # attack and a timpani under it
    orch = []
    parts = [(key('C3'), 0.45), (key('C4'), 0.6), (key('G4'), 0.5), (key('C5'), 0.4), (key('G5'), 0.22)]
    for i in env_len(0.62):
        t = i / sr
        bright = 900 + 4500 * math.exp(-t / 0.05)
        x = 0.0
        for k, a in parts:
            f = hz(k)
            for d in (-0.004, 0.004):
                h = 1
                while h * f < sr / 2 and h <= 12:
                    x += a * math.sin(2 * math.pi * h * f * (1 + d) * t) / h / (1 + (h * f / bright) ** 2)
                    h += 1
        env = (1 - math.exp(-t / 0.002)) * (0.75 * math.exp(-t / 0.16) + 0.25 * math.exp(-t / 0.4))
        x = x * env + 0.6 * math.exp(-t / 0.01) * rnd.uniform(-1, 1) + 0.5 * math.sin(2 * math.pi * 65.4 * t) * math.exp(-t / 0.3)
        orch.append(math.tanh(0.8 * x))
    S['orch'] = Sample(orch, base=key('C4'))

    # brass and strings loop 4096 samples holding whole cycles of each
    # detuned saw: 160 and 161 (brass), 159, 160 and 161 (strings)
    n = 4096
    base = 69 + 12 * math.log2(160 * sr / n / 440)
    attack = int(0.06 * sr)
    brass = []
    for i in range(attack + n):
        t = i / sr
        bright = 2200 - 1700 * math.exp(-t / 0.012) if i < attack else 2200
        brass.append(saw(i, n, 160, bright) + saw(i, n, 161, bright))
    S['brass'] = Sample(brass, base=base, loop=attack)
    strings = [saw(i, n, 159, 1800) + saw(i, n, 160, 1800) + saw(i, n, 161, 1800) for i in range(n)]
    S['strings'] = Sample(strings, base=base, loop=0)
    return S


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


PIVOT = [0, 3, 1, 4, 0, 3, 2, 4]
UPDOWN = [0, 1, 2, 3, 4, 3, 2, 1]
# the bass, sixteenths: (slot, length, tone): Root, Octave, Fifth, Seventh
# and an Approach to the next chord's root
BASS_BAR = [(0, 2, 'R'), (2, 1, 'O'), (3, 1, 'R'), (5, 1, 'R'), (6, 1, 'O'), (7, 1, 'F'),
            (8, 2, 'R'), (10, 1, 'O'), (11, 1, 'R'), (13, 1, 'S'), (14, 1, 'O'), (15, 1, 'A')]
BASS_HALF = [(0, 2, 'R'), (2, 1, 'O'), (3, 1, 'R'), (5, 1, 'F'), (6, 1, 'O'), (7, 1, 'A')]
BASS_TRESILLO = [(0, 3, 'R'), (3, 3, 'R'), (6, 1, 'O'), (7, 1, 'R')]


def arp(s, bar, t0, t1, voicing, pattern=PIVOT, vel=112, echo=None):
    v = keys(voicing)
    for i, t in enumerate(range(t0, t1, 6)):
        k = v[pattern[i % len(pattern)]]
        s.note('sq1', bar, t, 6, k, vel, **ARP)
        if echo and echo[0] <= t + 6 < echo[1]:
            s.note('sq2', bar, t + 6, 6, k, vel * 0.8, **ECHO)


def bass(s, bar, t0, pattern, root, fifth, seventh, nxt, below=True):
    """The approach walks up a half step into the next root, or (below
    False, or the same root again) drops to its fifth, where the lead
    already holds that root."""
    r = key(root)
    step = key(nxt) - 1 if below and (key(nxt) - r) % 12 else key(nxt) - 5
    tone = {'R': r, 'O': r + 12, 'F': r + fifth, 'S': r + seventh, 'A': step}
    for slot, n, sym in pattern:
        s.note('wave', bar, t0 + slot * 6, n * 6, tone[sym], 124, **BASS)


def groove(s, bar, kick=(0, 36, 48, 66), snare=(24, 72), hats_until=96, open_hats=True):
    for t in kick:
        s.hit('kick', bar, t, 12)
    for t in snare:
        s.hit('snare', bar, t, 12, vel=118, pan=0.05)
    for t in range(0, hats_until, 6):
        if open_hats and t % 24 == 12:
            s.note('noise', bar, t, 6, 0, 100, **OPEN)
        else:
            s.note('noise', bar, t, 3, 0, 118 if t % 24 == 0 else 88, **HAT)
    for t in (45, 93):
        if t < hats_until:
            s.note('noise', bar, t, 3, 0, 72, **HAT)


def crash(s, bar, tick, vel=127, length=72):
    s.note('fx', bar, tick, length, 0, vel, **CRASH)


def lead(s, bar, notes, voice=LEAD):
    for t, n, k, *extra in notes:
        o = dict(voice)
        if 'scoop' in extra:
            o['scoop'] = 1.0
        s.note('sq2', bar, t, n, k, 120, **o)


def arrangement():
    s = Score()
    # bar 0, the title: static rising under the first beat, then bVI, bVII, i
    # under THE NET, IS, ENDLESS, and a run and a tom fill into the jack-in
    s.note('fx', 0, 0, 24, 0, 110, sweep=(2000, 200000), ramp=(2, 13))
    for t, root, brass, top, n in ((24, 'C', 'C4 E4 G4 C5', 'E5', 12), (48, 'D', 'D4 F#4 A4 D5', 'F#5', 12),
                                   (72, 'E', 'E4 G4 B4 E5', 'G5', 8)):
        s.hit('orch', 0, t, 18, root + '4')
        s.hit('kick', 0, t, 12)
        s.hit('tom_lo', 0, t, 12, vel=100, pan=0.2)
        s.chord('brass', 0, t, n, brass, 112)
        s.note('wave', 0, t, 12, root + '2', 124, **BASS)
        crash(s, 0, t, 100 if t < 72 else 127, 24 if t < 72 else 96)
        lead(s, 0, [(t, n, top)])
    for i, k in enumerate(keys('E4 G4 B4 D5 E5 G5 B5 D6')):
        s.note('sq1', 0, 72 + 3 * i, 3, k, 100, **ARP)
    for t, tom, pan in ((80, 'tom_hi', -0.4), (84, 'tom_hi', -0.4), (88, 'tom_mid', 0.0), (92, 'tom_lo', 0.4)):
        s.hit(tom, 0, t, 4, vel=112, pan=pan)

    em9, em9_arp = ('E2', 7, 10), 'B3 E4 F#4 G4 D5'
    # bar 1, JACK IN!: the groove, the arpeggio and its echo
    crash(s, 1, 0)
    s.hit('orch', 1, 0, 18, 'E4')
    s.chord('brass', 1, 0, 12, 'E4 G4 B4 D5', 110)
    groove(s, 1)
    bass(s, 1, 0, BASS_BAR, *em9, 'E2')
    arp(s, 1, 0, 96, em9_arp, echo=(6, 84))
    lead(s, 1, [(84, 6, 'B4'), (90, 6, 'D5')])

    # bars 2-5, the theme: i, bVI, iv, V
    groove(s, 2)
    bass(s, 2, 0, BASS_BAR, *em9, 'C2')
    arp(s, 2, 0, 96, em9_arp)
    lead(s, 2, [(0, 18, 'E5'), (18, 18, 'B5'), (36, 12, 'A5'), (48, 12, 'G5'), (60, 12, 'F#5'), (72, 24, 'G5')])

    crash(s, 3, 0)
    s.hit('orch', 3, 0, 18, 'C4')
    s.chord('brass', 3, 0, 12, 'E4 G4 B4 D5', 110)
    s.chord('brass', 3, 18, 12, 'E4 G4 B4 D5', 96)
    groove(s, 3)
    bass(s, 3, 0, BASS_BAR, 'C2', 7, 11, 'A2')
    arp(s, 3, 0, 96, 'C4 E4 G4 B4 D5')
    lead(s, 3, [(0, 18, 'E5'), (18, 18, 'G5'), (36, 12, 'B5'), (48, 12, 'A5'), (60, 12, 'G5'), (72, 12, 'E5'),
                (84, 6, 'D5'), (90, 6, 'E5')])

    groove(s, 4)
    s.chord('brass', 4, 48, 12, 'C4 E4 G4 B4', 112)
    bass(s, 4, 0, BASS_BAR, 'A2', 7, 10, 'B2', below=False)
    arp(s, 4, 0, 96, 'A3 C4 E4 G4 E5')
    lead(s, 4, [(0, 18, 'E5'), (18, 18, 'C6'), (36, 12, 'B5'), (48, 12, 'A5'), (60, 12, 'G5'), (72, 12, 'A5'),
                (84, 12, 'B5')])

    # bar 5, BEASTOUT!: 3-3-2 stabs on the sus chord, the hit on the flash
    crash(s, 5, 0)
    s.hit('orch', 5, 0, 18, 'B3')
    for t, vel in ((0, 116), (18, 104), (36, 104)):
        s.chord('brass', 5, t, 12, 'B3 E4 F#4 A4', vel)
        s.hit('kick', 5, t, 12)
    crash(s, 5, 48)
    s.hit('orch', 5, 48, 24, 'B3')
    s.hit('kick', 5, 48, 12)
    s.chord('brass', 5, 48, 24, 'B3 D#4 F#4 A4', 120)
    s.hit('kick', 5, 66, 12)
    s.hit('snare', 5, 24, 12, vel=118)
    s.hit('snare', 5, 72, 12, vel=118)
    for t, tom, pan in ((78, 'tom_hi', -0.4), (84, 'tom_mid', 0.0), (90, 'tom_lo', 0.4)):
        s.hit(tom, 5, t, 6, vel=118, pan=pan)
    for t in range(0, 72, 6):
        s.note('noise', 5, t, 3, 0, 110 if t % 24 == 0 else 84, **HAT)
    bass(s, 5, 0, BASS_TRESILLO, 'B2', 7, 10, 'B2')
    bass(s, 5, 48, BASS_HALF, 'B2', 7, 10, 'C2')
    arp(s, 5, 0, 48, 'B3 E4 F#4 A4 B4')
    arp(s, 5, 48, 96, 'B3 D#4 F#4 A4 B4')
    lead(s, 5, [(0, 30, 'B5', 'scoop'), (30, 6, 'A5'), (36, 6, 'B5'), (42, 6, 'A5'), (48, 12, 'F#5'), (60, 12, 'A5'),
                (72, 6, 'B5'), (78, 6, 'C6'), (84, 12, 'D#6')])

    # bars 6-7, the guardians: V to bVI, deceptive, and back round to V
    crash(s, 6, 0)
    s.hit('orch', 6, 0, 18, 'C4')
    groove(s, 6)
    s.chord('brass', 6, 0, 18, 'C4 E4 G4 B4', 112)
    s.chord('brass', 6, 42, 18, 'D4 F#4 A4 B4', 108)
    s.chord('strings', 6, 0, 48, 'E4 G4 B4', 100, 0.6)
    s.chord('strings', 6, 48, 48, 'F#4 A4 B4', 100, 0.6)
    bass(s, 6, 0, BASS_HALF, 'C2', 7, 11, 'D2')
    bass(s, 6, 48, BASS_HALF, 'D2', 7, 9, 'A2')
    arp(s, 6, 0, 48, 'C4 E4 G4 B4 E5', UPDOWN)
    arp(s, 6, 48, 96, 'D4 F#4 A4 B4 E5', UPDOWN)
    lead(s, 6, [(0, 36, 'E6', 'scoop'), (36, 6, 'D6'), (42, 6, 'B5'), (48, 18, 'A5'), (66, 18, 'B5'), (84, 12, 'D6')])

    groove(s, 7, hats_until=78)
    for t, vel in ((78, 92), (84, 106), (90, 122)):
        s.hit('snare', 7, t, 6, vel=vel)
    s.chord('brass', 7, 0, 18, 'C4 E4 G4 B4', 108)
    s.chord('brass', 7, 42, 18, 'D#4 F#4 A4 C5', 112)
    s.chord('strings', 7, 0, 48, 'C4 E4 G4', 100, 0.6)
    s.chord('strings', 7, 48, 48, 'D#4 F#4 A4 C5', 108, 0.6)
    bass(s, 7, 0, BASS_HALF, 'A2', 7, 10, 'B2', below=False)
    bass(s, 7, 48, BASS_HALF, 'B2', 7, 10, 'E2')
    arp(s, 7, 0, 48, 'A3 C4 E4 G4 A4', UPDOWN)
    arp(s, 7, 48, 96, 'B3 D#4 F#4 A4 C5', UPDOWN)
    lead(s, 7, [(0, 18, 'E6'), (18, 18, 'C6'), (36, 12, 'B5'), (48, 12, 'A5'), (60, 12, 'B5'), (72, 12, 'C6'),
                (84, 12, 'D#6')])

    # bar 8, GO DEEPER: a dive, half time, the line cliché (E, D#, D, C#
    # under the arpeggio's top and the strings) over a tritone in the bass
    s.note('fx', 8, 0, 24, 0, 110, sweep=(200000, 3000), ramp=(12, 2))
    crash(s, 8, 0, 110)
    s.hit('orch', 8, 0, 24, 'E3')
    for t in (0, 60):
        s.hit('kick', 8, t, 12)
    s.hit('snare', 8, 48, 12, vel=118)
    for t in range(0, 84, 12):
        s.note('noise', 8, t, 3, 0, 96 if t % 24 == 0 else 76, **HAT)
    for t in (84, 88, 92):
        s.hit('tom_lo', 8, t, 4, vel=96, pan=0.3)
    for i, (t, n, k) in enumerate([(0, 36, 'E2'), (36, 6, 'E3'), (42, 6, 'E2'), (60, 12, 'E2'), (72, 12, 'B2'),
                                   (84, 6, 'A#2'), (90, 6, 'B2')]):
        s.note('wave', 8, t, n, k, 124, **BASS)
    line = ['E5', 'D#5', 'D5', 'C#5']
    for beat, top in enumerate(line):
        for j, k in enumerate(['E4', 'G4', 'B4', top]):
            t = beat * 24 + j * 6
            s.note('sq1', 8, t, 6, k, 88, **ARP)
            if t + 6 >= 30:
                s.note('sq2', 8, t + 6, 6, k, 76, **ECHO)
        s.hit('strings', 8, beat * 24, 24, top, 96, 0.3)
    s.chord('strings', 8, 0, 96, 'G4 B4', 96, 0.6)
    lead(s, 8, [(0, 24, 'E6', 'scoop')])

    # bars 9-10, what a run leaves: III, and the climb bVI, bVII
    crash(s, 9, 0)
    groove(s, 9, hats_until=84)
    for t, vel in ((87, 84), (90, 98), (93, 112)):
        s.hit('snare', 9, t, 3, vel=vel)
    s.chord('brass', 9, 0, 18, 'B3 D4 F#4 A4', 108)
    s.chord('brass', 9, 42, 18, 'A3 D4 F#4 A4', 108)
    s.chord('strings', 9, 0, 48, 'G4 B4 D5', 104, 0.6)
    s.chord('strings', 9, 48, 48, 'F#4 A4 D5', 104, 0.6)
    bass(s, 9, 0, BASS_HALF, 'G2', 7, 11, 'F#2')
    bass(s, 9, 48, BASS_HALF, 'F#2', 3, 8, 'C2')
    arp(s, 9, 0, 48, 'G3 B3 D4 F#4 A4', UPDOWN, vel=100)
    arp(s, 9, 48, 96, 'F#3 A3 D4 F#4 A4', UPDOWN, vel=104)
    lead(s, 9, [(0, 18, 'D6'), (18, 18, 'B5'), (36, 36, 'A5'), (72, 12, 'F#5'), (84, 12, 'A5')])

    # bar 10, the build: four on the floor, a snare roll, a riser and a
    # break on the last eighth for the lead's run
    s.note('fx', 10, 0, 90, 0, 120, sweep=(3000, 260000), ramp=(2, 13))
    for t in (0, 24, 48, 72):
        s.hit('kick', 10, t, 12)
    roll = [0, 12, 24, 36, 48, 54, 60, 66, 72, 75, 78, 81, 84, 87]
    for i, t in enumerate(roll):
        s.hit('snare', 10, t, 3 if t >= 72 else 6, vel=60 + 67 * i // (len(roll) - 1), pan=0.05)
    for t, k in ((0, 'C2'), (12, 'C3'), (24, 'C2'), (36, 'C3'), (48, 'D2'), (60, 'D3'), (72, 'D2'), (78, 'D2'),
                 (84, 'D2')):
        s.note('wave', 10, t, 12 if t < 72 else 6, k, 124, **BASS)
    for t in (0, 24):
        s.chord('brass', 10, t, 12, 'E4 G4 B4 D5', 100 + t // 2)
    for t in (48, 72):
        s.chord('brass', 10, t, 12, 'F#4 A4 B4 D5', 112 + t // 8)
    s.chord('strings', 10, 0, 48, 'E4 G4 B4 D5', 96, 0.6)
    s.chord('strings', 10, 48, 42, 'F#4 A4 B4 D5', 118, 0.6)
    arp(s, 10, 0, 48, 'C4 E4 G4 B4 D5', UPDOWN, vel=100)
    arp(s, 10, 48, 90, 'D4 F#4 A4 B4 D5', UPDOWN, vel=120)
    lead(s, 10, [(0, 12, 'E5'), (12, 12, 'G5'), (24, 12, 'B5'), (36, 12, 'D6'), (48, 12, 'A5'), (60, 12, 'D6'),
                 (72, 4, 'F#5'), (76, 4, 'G5'), (80, 4, 'A5'), (84, 4, 'B5'), (88, 4, 'C6'), (92, 4, 'D6')], BRIGHT)

    # bar 11, the logo: the theme from the top, brass under it
    crash(s, 11, 0)
    s.hit('orch', 11, 0, 24, 'E4')
    s.chord('strings', 11, 0, 96, 'E4 G4 B4 D5', 112, 0.6)
    groove(s, 11)
    bass(s, 11, 0, BASS_BAR, *em9, 'C2')
    arp(s, 11, 0, 96, em9_arp)
    theme = [(0, 18, 'E6'), (18, 18, 'B5'), (36, 12, 'A5'), (48, 12, 'G5'), (60, 12, 'F#5'), (72, 12, 'G5'),
             (84, 6, 'A5'), (90, 6, 'B5')]
    lead(s, 11, theme, BRIGHT)
    for t, n, k in theme:
        s.hit('brass', 11, t, n, key(k) - 12, 96, -0.2)

    # bar 12, the end card: bVI, bVII and I, the last one ringing
    for t, root, brass, top, n in ((0, 'C', 'C4 E4 G4 C5', 'E5', 12), (24, 'D', 'D4 F#4 A4 D5', 'F#5', 12),
                                   (48, 'E', 'E4 G#4 B4 E5', 'G#5', 96)):
        s.hit('orch', 12, t, n, root + '4')
        s.hit('kick', 12, t, 12)
        s.hit('snare', 12, t, 12, vel=96)
        s.hit('tom_lo', 12, t, 12, vel=104, pan=0.2)
        s.chord('brass', 12, t, n, brass, 124)
        s.note('wave', 12, t, n, root + '2', 124, **BASS)
        crash(s, 12, t, 127, 24 if t < 48 else 120)
        lead(s, 12, [(t, n, top, 'scoop')] if t == 48 else [(t, n, top)])
    s.chord('strings', 12, 48, 72, 'E4 G#4 B4 E5', 112, 0.6)
    for i, k in enumerate(keys('E5 G#5 B5 E6 B5 G#5 E5 B4 E5 G#5 B5 E6')):
        s.note('sq1', 12, 60 + 6 * i, 6, k, 96 - 6 * i, **ARP)
    return s


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


def render_channel(kind, notes, L, R, gain):
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
        vib, scoop = o.get('vib'), o.get('scoop', 0.0)
        duty = o.get('duty', 0.5)
        pos = tick * SPF
        for f, lvl in enumerate(levels):
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
                for j in range(pos, min(pos + SPF, len(L))):
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
                pitch = hz(k) * 2 ** (cents / 1200)
                if kind == 'wave':
                    inc = gb_wave(pitch) / RATE
                    amp = 1.0 if lvl >= 10 else 0.5 if lvl >= 6 else 0.25 if lvl >= 3 else 0.0
                    for j in range(pos, min(pos + SPF, len(L))):
                        phase = (phase + inc) % 1.0
                        v = (BASS_WAVE[int(phase * 32)] - 7.5) / 7.5 * amp
                        L[j] += v * gl
                        R[j] += v * gr
                else:
                    inc = gb_square(pitch) / RATE
                    amp = lvl / 15
                    for j in range(pos, min(pos + SPF, len(L))):
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
            pos += SPF


# ---- Direct Sound ----

def render_ds(events, S, n):
    """The Direct Sound mix at 13379 Hz: nearest samples, MP2K's envelope a
    frame, its echo, 8-bit out."""
    L, R = [0.0] * n, [0.0] * n
    frame = DS_RATE / 60
    for tick, length, inst, k, vel, pan in events:
        smp = S[inst]
        a, d, sus, r = DS_ENV[inst]
        step = SAMPLE_RATE / DS_RATE * (2 ** ((k - smp.base) / 12) if smp.base is not None else 1.0)
        start = round(tick / 60 * DS_RATE)
        off = round((tick + length) / 60 * DS_RATE)
        g = DS_GAIN[inst] * vel / 127
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
    # MP2K's echo (reverb 40, as BN6's battle songs): the mix of 6 and 7
    # frames ago, both sides, fed back into both
    rev = 40 / 512
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

def render(path):
    total = int((BARS * BAR / 60 + TAIL) * RATE)
    s = arrangement()
    parts = {}
    for name, chans, gain in (('arps', ('sq1',), 0.2), ('lead', ('sq2',), 0.24), ('bass', ('wave',), 0.3),
                              ('hats', ('noise',), 0.085), ('fx', ('fx',), 0.08)):
        L, R = [0.0] * total, [0.0] * total
        for c in chans:
            render_channel('noise' if c in ('noise', 'fx') else 'wave' if c == 'wave' else 'square',
                           s.psg[c], L, R, gain)
        parts[name] = (L, R)
    S = instruments()
    n_ds = int(total * DS_RATE / RATE) + 1
    dl, dr = render_ds(s.ds, S, n_ds)
    L, R = [0.0] * total, [0.0] * total
    ratio = DS_RATE / RATE
    lp = 1 - math.exp(-2 * math.pi * 9000 / RATE)
    yl = yr = 0.0
    for i in range(total):
        j = int(i * ratio)
        yl += lp * (dl[j] - yl)
        yr += lp * (dr[j] - yr)
        L[i], R[i] = yl * 0.62, yr * 0.62
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
    peak = max(max(map(abs, out_l)), max(map(abs, out_r))) or 1
    drive = 0.9 / peak
    norm = 0.89 / math.tanh(0.9)
    with wave.open(path, 'wb') as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(RATE)
        buf = bytearray()
        for a, b in zip(out_l, out_r):
            buf += struct.pack('<hh', int(math.tanh(a * drive) * norm * 32767), int(math.tanh(b * drive) * norm * 32767))
        w.writeframes(bytes(buf))
    return parts


if __name__ == '__main__':
    render(sys.argv[1] if len(sys.argv) > 1 else 'music.wav')
