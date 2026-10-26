"""0.16.0's trailer music, "Stake": an original piece in the manner of
Mega Man Battle Network 6's virus battle theme (song 0x15), the song a
Virus Battler's rounds play (BN6's battle records 0x1A-0x3A name it for
every round but a series' last, which plays 0x16; tools/virus_battler.py
reads them). Written by .claude/skills/trailer/music_rules.md and its
Discipline. None of Capcom's notes, chords in their order or samples.

What 0x15 does, read from the ROM with song_habits.py --detail --theory
--bars --offset 12 (its pickup: the kick and the crash on one, the snare
on two and four), numbers only:
- 150 beats a minute in C-sharp minor, its chords Aeolian with a
  Phrygian edge: i 24% of its half bars, bVImaj7 8, iv 8, IVsus4 6, isus2
  6, bare fifths 6, the bII as a fifth or a triad 6, bIIIsus2 4, the minor
  v 4; qualities minor sevenths 30%, fifths 14, major sevenths 12,
  suspensions 20; roots up a tone 16%, up a fourth 16, up a semitone 13,
  down a minor third 11; 1.36 changes a bar.
- Six tracks. Square 1 the backing: a 25% pulse that plucks (envelope
  0, 7, 3, 1), sixteenths (408 notes) and 32nds (200), accents 112, 100
  and 60, thrown hard left and right (pan 0 and 127, 167 times in 692
  notes: about a beat a side). Square 2 the lead: 50% duty held at 10
  (vibrato 5 after 14 ticks), eighths mostly, half its moves steps and a
  quarter repeats; a 75% voice for a riff of repeated notes. The wave
  the bass on a saw: sixteenths (538 of 590 notes), velocities 120 and
  60, between F-sharp 2 and G-sharp 4, its moves big leaps 34% (octaves)
  and leaps 29% (fifths), repeats 22%.
- The kit: the kick on 0, 7, 8, 10 and 14 (sixteenths of the bar), the
  snare on 4 and 12 with ghosts on 10 and 13, closed hats every sixteenth
  at three levels, an open hat on 2, 10 and 14, the crash on one at a
  phrase's head, three toms only in fills (5, 13-15).
- Two Direct Sound tracks with few notes (48 and 57 in the song): chord
  samples, a pad of slow attack, a stab struck three times on the
  beat's first three sixteenths at one pitch, a melodic sample stepping
  three notes a bar, bent 27 times (range 8).

Here, in C-sharp minor at 150 (a bar is 48 frames of the trailer's 30),
20 bars, a section's idea at its head and nowhere else:
- 0-3 the intro: the plucked 25% backing alone over C#sus2 and C#m7, the
  hats rising under a riser; bar 1 the saw bass's sixteenths under
  Amaj7 Bsus4-B; bar 2 the kit in half-time and a power chord on one
  (C#m7 Emaj7); bar 3 the kit in full to beat three, the toms, the lead's
  pickup over G#sus4-G# (its fourth falling to B-sharp where the chord
  turns major).
- 4-7 the A: the hook (C-sharp held and struck again, a step down and
  back, up to E, the D-natural of the bIImaj7 sung and falling to its
  seventh, then A) over C#m7 Dmaj7, the Phrygian step 0x15 leans on; its
  answer over Amaj7 Bsus4-B; the hook again, its tail rising to F-sharp;
  the cadence over F#m7 and the minor v. The chord samples only on the
  downbeats, as 0x15's few.
- 8-11 the B: the lead to the 75% voice, a riff of repeated notes
  climbing a third in sequence (Emaj7 F#m7 | G#m7 Amaj7), the backing in
  32nds, palm-muted chugs on the kick's places, the saw's three stepping
  notes a bar under it; the hook's head on the fake-out (bVImaj7 where
  the ear expects home); the turnaround over D#m7b5 and Bsus4-B.
- 12-13 the drop, kept short, the dark layer's: the squares out, the
  strings on C#m7 and a tritone away on Gm7b5, the bass held and falling;
  then Dmaj7 and G#sus4-G# (a tritone again, the major V's one surprise),
  the backing back in rising 32nds, a snare roll, a riser, the pickup.
- 14-17 the climax: the A again with everything in: chords on every half,
  power chords on the kick's places falling at the phrase ends, the
  brass's three stabs on beat four, a crash every two bars.
- 18-19 the logo and the end card: the turnaround over Amaj7 Bsus4-B and
  the toms; then C-sharp minor with its ninth struck and ringing, the
  bass falling away.

Instruments: the approved palette (0.14.0's), FluidR3 GM's (Frank Wen,
the MIT license; Debian's fluid-soundfont-gm in docker/Dockerfile.music's
image, never in git) cut into 10512 Hz 8-bit samples: the electronic
kit's kick, a snare (0x15's kit has no clap), the studio toms, the
strings' chords, the distortion and palm-muted guitars, the brass
section, the saw lead. The squares, the wave (a saw's table, as 0x15's)
and the noise are the Game Boy's own channels.

Its BANNER, the 3DS HOME Menu's sound while it is the current trailer:
the climax's first bar and a half, the hook at full strength. The piece
plays a soundfont, so it renders in docker/Dockerfile.music's image and
keeps its banner as our own audio in BANNER_AUDIO, written by
`python3 tools/trailer_music.py --write-banner --piece v0_16_0_music`.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_soundfont as sf  # noqa: E402
from trailer_music import Score, key, keys  # noqa: E402

SOUNDFONT = sf.SOUNDFONT
BPM = 150
BARS = 20
TAIL = 2.0
MIX = (('lead', ('sq1',), 0.11), ('arp', ('sq2',), 0.16), ('bass', ('wave',), 0.17),
       ('hats', ('noise',), 0.13), ('fx', ('fx',), 0.08))
DS_LEVEL = 1.0
REVERB = 40
HARDWARE = 32768
CHORD_SAMPLES = {'pad': (0, 2, 7, 12, 14), 'pad_m': (0, 3, 7, 12, 15), 'pad_M': (0, 4, 7, 12, 16),
                 'brass': (0, 4, 7, 12)}
BANNER = (14 * 4 * 60 / BPM, 2.9)      # (from bar 14's downbeat, seconds)
BANNER_AUDIO = '3ds/banner-0.16.0.wav'

DS_ENV = {'kick': (255, 250, 190, 208), 'snare': (255, 250, 190, 208), 'tom_hi': (255, 0, 255, 230),
          'tom_mid': (255, 0, 255, 230), 'tom_lo': (255, 0, 255, 230), 'gtr': (255, 0, 255, 188),
          'mute': (255, 0, 255, 160), 'pad': (60, 0, 235, 210), 'pad_m': (60, 0, 235, 210), 'pad_M': (60, 0, 235, 210),
          'dlead': (255, 0, 230, 200), 'brass': (255, 0, 230, 180)}
DS_GAIN = {'kick': 0.82, 'snare': 0.66, 'tom_hi': 0.6, 'tom_mid': 0.6, 'tom_lo': 0.65, 'gtr': 0.5, 'mute': 0.42,
           'pad': 0.72, 'pad_m': 0.72, 'pad_M': 0.72, 'dlead': 0.26, 'brass': 0.42}

LEAD = dict(duty=0.5, env=(0, 0, 10, 4), vib=(22, 14), pan=0.0)       # 0x15's square 2, voice 7
RIFF = dict(duty=0.75, env=(0, 0, 13, 2), vib=(18, 14), pan=0.0)      # ... and its 75% voice 8
ARP = dict(duty=0.25, env=(0, 7, 3, 1))                               # 0x15's square 1
SAW = [15 - i // 2 for i in range(32)]                                # the wave's table: a saw, as 0x15's
BASS = dict(env=(0, 0, 13, 1), wave=SAW)
HELD = dict(env=(0, 0, 15, 3), wave=SAW)
HAT = dict(clock=262144, env=(0, 0, 11, 0), pan=0.2)
OPEN = dict(clock=262144, env=(0, 2, 11, 3), pan=0.25)
CRASH = dict(clock=80000, env=(0, 7, 0, 1), pan=-0.15)


def instruments():
    C2 = key('C#2')
    S = {
        'kick': sf.drum(36, 0.14, program=24, drive=2.4),
        'snare': sf.drum(40, 0.12, layer=(38,), drive=1.8),
        'tom_hi': sf.drum(50, 0.35),
        'tom_mid': sf.drum(47, 0.35),
        'tom_lo': sf.drum(45, 0.38),
        'gtr': sf.instrument(30, [C2, C2 + 7, C2 + 12], 0.34, gains=[1.0, 0.85, 0.7]),
        'mute': sf.instrument(28, [C2, C2 + 7, C2 + 12], 0.12, gains=[1.0, 0.85, 0.7]),
        'dlead': sf.instrument(81, [72], 1.0, fade=0.1),
        'brass': sf.instrument(61, [60 + i for i in CHORD_SAMPLES['brass']], 0.5, base=60, fade=0.12),
    }
    for name in ('pad', 'pad_m', 'pad_M'):
        S[name] = sf.instrument(48, [60 + i for i in CHORD_SAMPLES[name]], 2.0, base=60, fade=0.3)
    return S


# ---- the harmony: (bass root, the backing's four notes, the chord
# sample and its root): a triad sample a third over the bass makes the
# seventh chord ----
H = {
    'C#sus2': ('C#3', 'G#4 C#5 D#5 G#5', 'pad', 'C#3'),
    'C#m7': ('C#3', 'G#4 B4 C#5 E5', 'pad_M', 'E3'),          # E over C#
    'Dmaj7': ('D3', 'F#4 A4 C#5 D5', 'pad_m', 'F#3'),         # F#m over D: the bIImaj7
    'Amaj7': ('A2', 'G#4 A4 C#5 E5', 'pad_m', 'C#3'),         # C#m over A
    'Bsus4': ('B2', 'F#4 B4 E5 F#5', 'pad', 'E3'),            # E F# B over B
    'B': ('B2', 'F#4 B4 D#5 F#5', 'pad_M', 'B2'),
    'Emaj7': ('E3', 'G#4 B4 D#5 E5', 'pad_m', 'G#3'),         # G#m over E
    'F#m7': ('F#2', 'F#4 A4 C#5 E5', 'pad_M', 'A3'),          # A over F#
    'G#m7': ('G#2', 'F#4 G#4 B4 D#5', 'pad_M', 'B3'),         # B over G#: the minor v
    'G#sus4': ('G#2', 'G#4 C#5 D#5 G#5', 'pad', 'C#3'),       # C# D# G# over G#
    'G#': ('G#2', 'G#4 C5 D#5 G#5', 'pad_M', 'G#3'),          # the major V, once
    'D#m7b5': ('D#3', 'F#4 A4 C#5 D#5', 'pad_m', 'F#3'),      # F#m over D#
    'Gm7b5': ('G2', 'G4 A#4 C#5 F5', 'pad_m', 'A#3'),         # Bbm over G: a tritone from home
}
BRASS = {'C#m7': 'E3', 'Dmaj7': 'D3', 'Amaj7': 'A2', 'B': 'B2', 'F#m7': 'A3', 'G#m7': 'B2'}

CHORDS = {0: ('C#sus2', 'C#m7'), 1: ('Amaj7', 'Bsus4', 'B'), 2: ('C#m7', 'Emaj7'), 3: ('F#m7', 'G#sus4', 'G#'),
          4: ('C#m7', 'Dmaj7'), 5: ('Amaj7', 'Bsus4', 'B'), 6: ('C#m7', 'Dmaj7'), 7: ('F#m7', 'G#m7'),
          8: ('Emaj7', 'F#m7'), 9: ('G#m7', 'Amaj7'), 10: ('Amaj7', 'Dmaj7'), 11: ('D#m7b5', 'Bsus4', 'B'),
          12: ('C#m7', 'Gm7b5'), 13: ('Dmaj7', 'G#sus4', 'G#'),
          14: ('C#m7', 'Dmaj7'), 15: ('Amaj7', 'Bsus4', 'B'), 16: ('C#m7', 'Dmaj7'), 17: ('F#m7', 'G#m7'),
          18: ('Amaj7', 'Bsus4', 'B'), 19: ('C#m7',)}

# ---- the lines: (tick, ticks, note) ----
HOOK = [(0, 12, 'C#6'), (12, 6, 'C#6'), (18, 6, 'B5'), (24, 12, 'C#6'), (36, 12, 'E6'), (48, 18, 'D6'), (66, 6, 'C#6'),
        (72, 24, 'A5')]
ANSWER = [(0, 6, 'G#5'), (6, 6, 'A5'), (12, 12, 'B5'), (24, 6, 'C#6'), (30, 6, 'B5'), (36, 12, 'A5'), (48, 12, 'B5'),
          (60, 12, 'E6'), (72, 24, 'D#6')]
HOOK2 = HOOK[:5] + [(48, 12, 'D6'), (60, 6, 'E6'), (66, 6, 'F#6'), (72, 12, 'E6'), (84, 12, 'D6')]
CADENCE = [(0, 12, 'C#6'), (12, 12, 'A5'), (24, 6, 'B5'), (30, 6, 'C#6'), (36, 12, 'B5'), (48, 24, 'B5'), (72, 12, 'D#6'),
           (84, 12, 'B5')]
A_LINES = {0: HOOK, 1: ANSWER, 2: HOOK2, 3: CADENCE}
RIFFS = {
    8: [(0, 6, 'B5'), (6, 6, 'B5'), (12, 12, 'G#5'), (24, 6, 'B5'), (30, 6, 'B5'), (36, 12, 'D#6'), (48, 6, 'C#6'),
        (54, 6, 'C#6'), (60, 12, 'A5'), (72, 12, 'C#6'), (84, 12, 'E6')],
    9: [(0, 6, 'D#6'), (6, 6, 'D#6'), (12, 12, 'B5'), (24, 6, 'D#6'), (30, 6, 'D#6'), (36, 12, 'F#6'), (48, 6, 'E6'),
        (54, 6, 'E6'), (60, 12, 'C#6'), (72, 12, 'E6'), (84, 12, 'G#6')],
    10: HOOK[:5] + [(48, 12, 'F#6'), (60, 12, 'E6'), (72, 24, 'C#6')],
    11: [(0, 6, 'A5'), (6, 6, 'A5'), (12, 6, 'C#6'), (18, 6, 'D#6'), (24, 12, 'F#6'), (36, 12, 'D#6'), (48, 24, 'E6'),
         (72, 24, 'D#6')],
}
# the saw's three stepping notes a bar under the B (0x15's melodic sample)
UNDER = {8: [(0, 42, 'G#4'), (48, 24, 'A4'), (72, 24, 'A4')], 9: [(0, 42, 'B4'), (48, 24, 'C#5'), (72, 24, 'C#5')],
         10: [(0, 42, 'C#5'), (48, 48, 'D5')], 11: [(0, 42, 'C#5'), (48, 24, 'B4'), (72, 24, 'B4')]}
TURN = [(0, 12, 'C#6'), (12, 6, 'C#6'), (18, 6, 'B5'), (24, 12, 'A5'), (36, 12, 'G#5'), (48, 12, 'B5'), (60, 12, 'E6'),
        (72, 24, 'D#6')]


def halves(names):
    if len(names) == 1:
        return [(0, 96, names[0])]
    if len(names) == 2:
        return [(0, 48, names[0]), (48, 48, names[1])]
    return [(0, 48, names[0]), (48, 24, names[1]), (72, 24, names[2])]


def lead(s, bar, line, voice=LEAD, vel=116, fall_last=False):
    for i, (t, n, k) in enumerate(line):
        o = dict(voice)
        if fall_last and i == len(line) - 1 and n >= 24:
            o['bend'] = [(0, 0), (n - 6, 0), (n, -2)]
        s.note('sq1', bar, t, n, k, vel - (12 if n <= 6 and t % 12 else 0), **o)


FIG = (0, 1, 2, 2, 3, 2, 1, 1)      # over the chord's four notes: one figure, a quarter of it repeats
ACCENT = (112, 60, 100, 60)


def backing(s, bar, t0, t1, name, run32=False, vel=0, soft=52):
    """The plucked 25% square: sixteenths (or each struck twice as 32nds,
    loud then soft), accented, thrown hard left and right a beat a side."""
    v = keys(H[name][1])
    for i, t in enumerate(range(t0, t1, 6)):
        k = v[FIG[(t // 6) % 8]]
        side = -0.9 if (t // 24) % 2 == 0 else 0.9
        a = ACCENT[(t // 6) % 4] + vel
        if run32:
            s.note('sq2', bar, t, 3, k, a, pan=side, **ARP)
            s.note('sq2', bar, t + 3, 3, k, soft, pan=side, **ARP)
        else:
            s.note('sq2', bar, t, 6, k, a, pan=side, **ARP)


# the bass's sixteenths over a half bar: (sixteenth, above the root, velocity)
BASS8 = ((0, 0, 120), (1, 0, 60), (2, 12, 120), (3, 0, 120), (5, 12, 60), (6, 7, 120), (7, 0, 120))


def bass(s, bar, names, soft=0):
    for t0, n, name in halves(names):
        r = key(H[name][0])
        steps = [p for p in BASS8 if 6 * p[0] < n]
        for i, (p, up, v) in enumerate(steps):
            end = steps[i + 1][0] if i + 1 < len(steps) else n // 6
            s.note('wave', bar, t0 + 6 * p, 6 * (end - p) - 1, r + up, max(40, v - soft), **BASS)


def chord(s, bar, t0, name, n, vel=92):
    _, _, pad, root = H[name]
    s.hit(pad, bar, t0, n - 2, key(root) + 12, vel, -0.35 if t0 % 96 < 48 else 0.35)


KICK = (0, 7, 8, 10, 14)


def hats(s, bar, until=16, opens=(2, 10, 14), lift=0):
    for p in range(until):
        if p in opens:
            s.note('noise', bar, 6 * p, 6, 0, 84 + lift, **OPEN)
        else:
            s.note('noise', bar, 6 * p, 3, 0, (96, 48, 72, 48)[p % 4] + lift, **HAT)


def kit(s, bar, until=16, ghost=0, crash=False, half=False):
    """0x15's kit: the kick on 0, 7, 8, 10, 14; the snare on two and four,
    a ghost on 10 or 13; hats; the crash at a phrase's head."""
    for p in ((0, 10) if half else KICK):
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=100 if p in (0, 8) else 86)
    for p in ((8,) if half else (4, 12)):
        if p < until:
            s.hit('snare', bar, 6 * p, 6, vel=98, pan=0.05)
    if ghost and ghost < until:
        s.hit('snare', bar, 6 * ghost, 6, vel=50, pan=-0.1)
    hats(s, bar, until, opens=() if half else (2, 10, 14))
    if crash:
        s.note('fx', bar, 0, 48, 0, 116, **CRASH)


def toms(s, bar, t0=72, vel=112):
    for t, tom, pan in ((t0, 'tom_hi', -0.3), (t0 + 6, 'tom_hi', -0.3), (t0 + 12, 'tom_mid', 0.0),
                        (t0 + 18, 'tom_lo', 0.3)):
        s.hit(tom, bar, t, 6, vel=vel, pan=pan)


def power(s, bar, t, n, name, vel=112, fall=False):
    """the distortion guitar's power chord on the root, a fall at a phrase's end"""
    r = key(H[name][0])
    while r > key('E2'):
        r -= 12
    s.hit('gtr', bar, t, n, r + 12, vel, 0.0, bend=[(0, 0), (n - 10, 0), (n, -8)] if fall else None)


def chugs(s, bar, names):
    """palm-muted chugs on the kick's places"""
    for t0, n, name in halves(names):
        r = key(H[name][0])
        while r > key('E2'):
            r -= 12
        for p in KICK:
            if t0 <= 6 * p < t0 + n:
                s.hit('mute', bar, 6 * p, 5, r + 12, 104 if p in (0, 8) else 88, 0.0)


def stabs(s, bar, name):
    """the brass's three stabs on beat four's first three sixteenths"""
    if name in BRASS:
        for i, t in enumerate((72, 78, 84)):
            s.hit('brass', bar, t, 5, BRASS[name], 108 - 8 * i, 0.3)


# ---- the sections ----

def intro(s):
    for t0, n, name in halves(CHORDS[0]):
        backing(s, 0, t0, t0 + n, name, vel=-30 + t0 // 4)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 26 + 4 * p, **HAT)
    s.note('fx', 0, 0, 92, 0, 96, sweep=(1500, 120000), ramp=(1, 8))
    # bar 1: the bass enters
    for t0, n, name in halves(CHORDS[1]):
        backing(s, 1, t0, t0 + n, name, vel=-8)
    chord(s, 1, 0, 'Amaj7', 48, vel=84)
    bass(s, 1, CHORDS[1], soft=30)
    hats(s, 1, opens=(), lift=-20)
    s.note('fx', 1, 0, 92, 0, 104, sweep=(2000, 180000), ramp=(1, 10))
    # bar 2: the kit in half-time, a power chord on one
    s.note('fx', 2, 0, 48, 0, 118, **CRASH)
    power(s, 2, 0, 30, 'C#m7', vel=118)
    for t0, n, name in halves(CHORDS[2]):
        backing(s, 2, t0, t0 + n, name)
    bass(s, 2, CHORDS[2])
    kit(s, 2, half=True)
    # bar 3: in full to beat three, the toms, the pickup
    for t0, n, name in halves(CHORDS[3]):
        backing(s, 3, t0, t0 + n, name)
    chord(s, 3, 0, 'F#m7', 48, vel=90)
    bass(s, 3, CHORDS[3])
    kit(s, 3, until=12)
    toms(s, 3)
    s.note('sq1', 3, 84, 6, 'G#5', 104, **LEAD)
    s.note('sq1', 3, 90, 6, 'C6', 110, **LEAD)        # (B-sharp: G#'s third)


def section_a(s, first, climax=False):
    """Four bars from `first`: the hook, its answer, the hook, the cadence."""
    for i in range(4):
        bar = first + i
        names = CHORDS[bar]
        for t0, n, name in halves(names):
            backing(s, bar, t0, t0 + n, name, run32=climax)
            if climax:
                chord(s, bar, t0, name, n, vel=100)
        if not climax:
            chord(s, bar, 0, names[0], 48, vel=90)
        bass(s, bar, names)
        lead(s, bar, A_LINES[i], vel=122 if climax else 116, fall_last=i == 3)
        kit(s, bar, ghost=10 if i % 2 else 13, crash=climax and i % 2 == 0 or (not climax and i == 0))
        if climax:
            power(s, bar, 0, 22, names[0], vel=116)
            power(s, bar, 42, 18 if i < 3 else 40, names[1], vel=104, fall=i == 3)
            stabs(s, bar, halves(names)[-1][2])
        elif i == 0:
            power(s, bar, 0, 30, names[0], vel=116)
    if not climax:
        s.hit('tom_mid', first + 3, 84, 6, vel=108, pan=0.2)
        s.hit('tom_lo', first + 3, 90, 6, vel=112, pan=0.2)


def section_b(s):
    """Bars 8-11: the riff on the 75% voice, the backing in 32nds, the
    chugs, the saw's three notes; the hook's head on the fake-out; the
    turnaround."""
    for bar in range(8, 12):
        names = CHORDS[bar]
        for t0, n, name in halves(names):
            backing(s, bar, t0, t0 + n, name, run32=True)
        chord(s, bar, 0, names[0], 48, vel=94)
        bass(s, bar, names)
        chugs(s, bar, names)
        lead(s, bar, RIFFS[bar], voice=RIFF, vel=112)
        for t, n, k in UNDER[bar]:
            s.hit('dlead', bar, t, n - 2, k, 100, -0.3)
        kit(s, bar, until=12 if bar == 11 else 16, ghost=13 if bar % 2 else 10, crash=bar in (8, 10))
    power(s, 8, 0, 30, 'Emaj7', vel=116)
    power(s, 10, 0, 30, 'Amaj7', vel=116)
    toms(s, 11)


def drop(s):
    """Bar 12: the squares out; the strings on C#m7, then a tritone away on
    Gm7b5, the bass held and falling. Bar 13: Dmaj7 and G#sus4-G#, the
    backing back in rising 32nds, a snare roll, a riser, the pickup."""
    s.note('fx', 12, 0, 72, 0, 118, **CRASH)
    power(s, 12, 0, 40, 'C#m7', vel=120, fall=True)
    s.hit('kick', 12, 0, 6, vel=124)
    for t0, name in ((0, 'C#m7'), (48, 'Gm7b5')):
        chord(s, 12, t0, name, 48, vel=104)
        r = key(H[name][0])
        s.note('wave', 12, t0, 46, r, 112, **dict(HELD, bend=[(0, 0), (36, 0), (46, -1)]))
    for t in (24, 72):
        s.note('noise', 12, t, 3, 0, 60, **HAT)
    s.hit('snare', 12, 72, 6, vel=96)
    # bar 13: the climb back
    s.note('fx', 13, 0, 90, 0, 112, sweep=(2500, 240000), ramp=(2, 13))
    for i, (t0, n, name) in enumerate(halves(CHORDS[13])):
        backing(s, 13, t0, t0 + n, name, run32=True, vel=-40 + 20 * i, soft=40 + 8 * i)
        chord(s, 13, t0, name, n, vel=96 + 6 * i)
    bass(s, 13, CHORDS[13], soft=20)
    for t in (0, 24, 48, 72):
        s.hit('kick', 13, t, 6, vel=116)
    roll = list(range(0, 48, 12)) + list(range(48, 72, 6)) + list(range(72, 96, 3))
    for i, t in enumerate(roll):
        s.hit('snare', 13, t, 3, vel=52 + 66 * i // len(roll), pan=-0.2 + 0.4 * i / len(roll))
    s.note('sq1', 13, 84, 6, 'G#5', 108, **LEAD)
    s.note('sq1', 13, 90, 6, 'C6', 114, **LEAD)


def ending(s):
    """Bar 18: the turnaround over Amaj7 Bsus4-B, the toms; bar 19: C-sharp
    minor with its ninth, struck and ringing, the bass falling away."""
    for t0, n, name in halves(CHORDS[18]):
        backing(s, 18, t0, t0 + n, name, run32=True)
        chord(s, 18, t0, name, n, vel=100)
    bass(s, 18, CHORDS[18])
    lead(s, 18, TURN, vel=122)
    power(s, 18, 0, 22, 'Amaj7', vel=116)
    stabs(s, 18, 'B')
    kit(s, 18, until=12, crash=True)
    toms(s, 18)
    s.note('fx', 19, 0, 96, 0, 126, **CRASH)
    s.hit('kick', 19, 0, 6)
    s.hit('snare', 19, 0, 6)
    power(s, 19, 0, 90, 'C#m7', vel=124, fall=True)
    s.hit('pad_m', 19, 0, 150, 'C#4', 110, -0.35)     # C# E G#
    s.hit('pad', 19, 0, 150, 'C#3', 94, 0.35)         # C# D# G#: the ninth's colour
    s.hit('brass', 19, 0, 60, 'E3', 110, 0.3)         # E G# B over C#: its seventh
    s.note('wave', 19, 0, 92, 'C#3', 127, **dict(HELD, bend=[(0, 0), (54, 0), (92, -12)]))
    s.note('sq1', 19, 0, 96, 'C#6', 120, **dict(LEAD, bend=[(0, 0), (76, 0), (96, -2)]))
    s.note('sq2', 19, 0, 96, 'G#5', 98, **dict(LEAD, pan=0.35, bend=[(0, 0), (76, 0), (96, -2)]))


def arrangement():
    s = Score()
    intro(s)
    section_a(s, 4)
    section_b(s)
    drop(s)
    section_a(s, 14, climax=True)
    ending(s)
    return s
