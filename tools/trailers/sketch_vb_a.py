"""Sketch A for 0.16.0, the Battler's round: nine bars for the owner's ear,
not a cut. Original, in the manner of BN6's virus battle theme (song
0x15), the song a Virus Battler's rounds play (BN6's battle records
0x1A-0x3A name it for every round but a series' last); none of its notes,
nor its chords in their order. Played on the approved palette's samples
(sketch_vb_lib.py).

    tools/trailer_music.py's render_any(OUT, trailers.sketch_vb_a), in
    docker/Dockerfile.music's image; on 26 October 2026 also through
    GeneralUser GS and MuseScore General (CW_SOUNDFONT_NAME), for the owner's
    comparison by ear

What 0x15 does, read from the ROM with song_habits.py --detail --theory
--offset 12 and the bars' rhythms in aggregate, numbers only:
- 150 beats a minute in C-sharp minor, Aeolian with a Phrygian edge: i
  24% of its half bars, bVImaj7 8, iv 8, IVsus4 6, the bII 6, bIIIsus2 4,
  the minor v 4; minor sevenths 30%, suspensions 20%.
- Square 1 the backing: a 25% pulse that plucks (envelope 0, 7, 3, 1),
  straight sixteenths in most bars (32nds in some), accents 112, 100, 60,
  thrown hard left and right about a beat a side.
- Square 2 the lead: 50% duty held at 10 with vibrato 5 after 14 ticks,
  eighths and sixteenths, half its moves steps and a quarter repeats,
  pushed off the beat (its bars' commonest onsets 1 2 4 6 8 10 11 13 14
  and 0 4 6 11 12 14).
- The wave the bass on a saw: sixteenths, at 120 and 60, octaves and
  fifths (big leaps 34%, leaps 29%), its commonest bar 0 2 4 5 6 8 10 11
  13 15.
- The kit: the kick on 0, 7, 8, 10 and 14, the snare on 4 and 12 with
  ghosts on 10 and 13, closed hats every sixteenth, an open hat on 2, 10
  and 14, the crash on one at a phrase's head, toms only into a section.
- Direct Sound: chord samples and a stab, its commonest bars 10 12 14 (a
  pickup into the next bar) and 0 4; a melodic sample stepping three
  notes a bar.

Here, in C-sharp minor at 150:
- Bar 0 the count-in: the plucked backing alone over C#m7, the hats rising,
  the guitar's pickup chugs on 10, 12 and 14.
- Bars 1-4 the A: the hook pushed off the beat over C#m7 Dmaj7 (the
  Phrygian bII), its answer over Amaj7 F#m7, the hook again, its tail
  higher, the cadence over Amaj7 and G#sus4 to G#m7 (the fourth falling to
  the minor third); the strings' chords on every half bar, the guitar's
  pickup on 10 12 14 every other bar, the saw bass in 0x15's bar.
- Bars 5-6 the B: the melody to the sampled saw, a note repeated and a
  step, as 0x15's sample does, its echo; the backing in 32nds; brass on the turnaround.
- Bar 7 the hook back on the square, the saw an octave under it.
- Bar 8 C-sharp minor with its ninth, struck and ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score, key, keys  # noqa: E402
from trailers import sketch_vb_lib as L  # noqa: E402
from trailers.sketch_vb_lib import (CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL, HARDWARE, MIX, REVERB,  # noqa: E402,F401
                                    SAMPLED_LEADS, SOUNDFONT)

BPM = 150
BARS = 9
TAIL = 1.8
BANNER = (1 * 4 * 60 / BPM, 2.9)


def instruments():
    return L.instruments('C#2')


LEAD = dict(duty=0.5, env=(0, 0, 10, 4), vib=(22, 14), pan=0.0)
ARP = dict(duty=0.25, env=(0, 7, 3, 1))
BASS = dict(env=(0, 0, 13, 1), wave=L.SAW)
HELD = dict(env=(0, 0, 15, 3), wave=L.SAW)

# (bass root, the backing's four notes, the chord sample and its root)
H = {
    'C#m7': ('C#3', 'G#4 B4 C#5 E5', 'pad_M', 'E3'),          # E over C#
    'Dmaj7': ('D3', 'F#4 A4 C#5 D5', 'pad_m', 'F#3'),         # F#m over D: the bII
    'Amaj7': ('A2', 'G#4 A4 C#5 E5', 'pad_m', 'C#3'),         # C#m over A
    'F#m7': ('F#2', 'F#4 A4 C#5 E5', 'pad_M', 'A3'),          # A over F#
    'G#sus4': ('G#2', 'G#4 C#5 D#5 G#5', 'pad', 'C#3'),       # C# D# G# over G#
    'G#m7': ('G#2', 'F#4 G#4 B4 D#5', 'pad_M', 'B3'),         # B over G#: the minor v
    'F#sus4': ('F#2', 'F#4 B4 C#5 F#5', 'pad', 'B2'),         # B C# F# over F#
    'Esus2': ('E3', 'E4 F#4 B4 E5', 'pad', 'E3'),             # the bIII suspended
    'Bsus4': ('B2', 'F#4 B4 E5 F#5', 'pad', 'E3'),
    'B': ('B2', 'F#4 B4 D#5 F#5', 'pad_M', 'B2'),
}
CHORDS = {0: ('C#m7',), 1: ('C#m7', 'Dmaj7'), 2: ('Amaj7', 'F#m7'), 3: ('C#m7', 'Dmaj7'), 4: ('Amaj7', 'G#sus4', 'G#m7'),
          5: ('F#sus4', 'Esus2'), 6: ('Amaj7', 'Bsus4', 'B'), 7: ('C#m7', 'Dmaj7'), 8: ('C#m7',)}

HOOK = [(0, 6, 'G#5'), (6, 6, 'B5'), (12, 6, 'C#6'), (18, 6, 'C#6'), (24, 6, 'B5'), (30, 6, 'C#6'), (36, 12, 'E6'),
        (48, 12, 'D6'), (60, 6, 'D6'), (66, 18, 'E6'), (84, 12, 'E6')]
LINES = {
    1: HOOK,
    2: [(6, 12, 'C#6'), (18, 6, 'C#6'), (24, 12, 'B5'), (36, 6, 'A5'), (42, 6, 'B5'), (48, 6, 'A5'), (54, 6, 'A5'),
        (66, 6, 'G#5'), (72, 24, 'G#5')],
    3: HOOK[:7] + [(48, 12, 'D6'), (60, 6, 'E6'), (66, 18, 'F#6'), (84, 12, 'F#6')],
    4: [(0, 6, 'E6'), (6, 6, 'E6'), (12, 12, 'C#6'), (24, 12, 'B5'), (36, 12, 'A5'), (48, 24, 'C#6'), (72, 24, 'B5')],
    7: HOOK,
}
SAWLINE = {5: [(0, 12, 'C#6'), (12, 12, 'C#6'), (24, 24, 'B5'), (48, 12, 'B5'), (60, 12, 'B5'), (72, 24, 'B5')],
           6: [(0, 12, 'C#6'), (12, 12, 'C#6'), (24, 24, 'C#6'), (48, 12, 'D#6'), (60, 12, 'D#6'), (72, 24, 'D#6')]}

ACCENT = (112, 60, 100, 60)
FIG = (0, 1, 2, 3, 2, 1, 2, 3)


def backing(s, bar, t0, t1, name, run32=False, vel=0):
    v = keys(H[name][1])
    for t in range(t0, t1, 6):
        k = v[FIG[(t // 6) % 8]]
        side = -0.9 if (t // 24) % 2 == 0 else 0.9
        a = max(30, ACCENT[(t // 6) % 4] + vel)
        if run32:
            s.note('sq2', bar, t, 3, k, a, pan=side, **ARP)
            s.note('sq2', bar, t + 3, 3, k, 52, pan=side, **ARP)
        else:
            s.note('sq2', bar, t, 6, k, a, pan=side, **ARP)


# 0x15's commonest bass bar: (sixteenth, above the root, velocity)
BASS_BAR = ((0, 0, 120), (2, 0, 60), (4, 12, 120), (5, 12, 60), (6, 0, 120))
BASS_HALF2 = ((8, 0, 120), (10, 7, 120), (11, 7, 60), (13, 12, 120), (15, 12, 60))


def bass(s, bar, names):
    hs = L.halves(names)
    second = hs[1][2] if len(hs) > 1 else hs[0][2]
    notes = [(p, key(H[hs[0][2]][0]) + up, v) for p, up, v in BASS_BAR]
    notes += [(p, key(H[second][0]) + up, v) for p, up, v in BASS_HALF2]
    for i, (p, k, v) in enumerate(notes):
        end = notes[i + 1][0] if i + 1 < len(notes) else 16
        s.note('wave', bar, 6 * p, 6 * (end - p) - 1, k, v, **BASS)


def chords(s, bar, names, vel=94):
    for t0, n, name in L.halves(names):
        _, _, pad, root = H[name]
        s.hit(pad, bar, t0, n - 2, key(root) + 12, vel, -0.35 if t0 < 48 else 0.35)


def kit(s, bar, crash=False, ghost=10, until=16):
    for p in (0, 7, 8, 10, 14):
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=104 if p in (0, 8) else 90)
    for p in (4, 12):
        if p < until:
            s.hit('snare', bar, 6 * p, 6, vel=104, pan=0.05)
    if ghost < until:
        s.hit('snare', bar, 6 * ghost, 6, vel=48, pan=-0.1)
    L.hats(s, bar, opens=(2, 10, 14), until=until)
    if crash:
        s.note('fx', bar, 0, 48, 0, 116, **L.CRASH)


def pickup(s, bar, name):
    """the guitar's pickup chugs on 10, 12 and 14 (0x15's stab bar)"""
    for p in (10, 12, 14):
        L.chug(s, bar, 6 * p, H[name][0], 100 if p == 14 else 88)


def lead(s, bar, line, vel=116):
    for t, n, k in line:
        s.note('sq1', bar, t, n, k, vel - (12 if n <= 6 and t % 12 else 0), **LEAD)


def arrangement():
    s = Score()
    # bar 0: the backing alone, the hats rising, the pickup
    backing(s, 0, 0, 96, 'C#m7', vel=-30)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 28 + 4 * p, **L.HAT)
    s.note('fx', 0, 0, 92, 0, 100, sweep=(1500, 120000), ramp=(1, 8))
    pickup(s, 0, 'C#m7')
    s.note('sq1', 0, 84, 6, 'E5', 104, **LEAD)
    s.note('sq1', 0, 90, 6, 'F#5', 108, **LEAD)
    # bars 1-4: the A
    for bar in range(1, 5):
        names = CHORDS[bar]
        for t0, n, name in L.halves(names):
            backing(s, bar, t0, t0 + n, name)
        chords(s, bar, names)
        bass(s, bar, names)
        lead(s, bar, LINES[bar])
        kit(s, bar, crash=bar == 1, ghost=13 if bar % 2 else 10, until=14 if bar == 4 else 16)
        if bar % 2 == 0:
            pickup(s, bar, L.halves(names)[-1][2])
    L.power(s, 1, 0, 30, 'C#3', vel=116)
    L.power(s, 3, 0, 30, 'C#3', vel=112)
    s.hit('tom_mid', 4, 84, 6, vel=108, pan=0.2)
    s.hit('tom_lo', 4, 90, 6, vel=112, pan=0.2)
    # bars 5-6: the B, the saw's melody
    for bar in (5, 6):
        names = CHORDS[bar]
        for t0, n, name in L.halves(names):
            backing(s, bar, t0, t0 + n, name, run32=True)
        chords(s, bar, names, vel=100)
        bass(s, bar, names)
        for t, n, k in SAWLINE[bar]:
            s.hit('dlead', bar, t, n - 1, k, 112, -0.25)
            s.hit('dlead', bar, t + 3, n - 1, k, 64, 0.4)
        kit(s, bar, crash=bar == 5, ghost=13 if bar == 5 else 10, until=12 if bar == 6 else 16)
    L.power(s, 5, 0, 30, 'F#2', vel=116)
    for t in (72, 78, 84):
        s.hit('brass', 6, t, 5, 'B2', 104 - (t - 72), 0.3)
    L.toms(s, 6, t0=72)
    # bar 7: the hook back, the saw an octave under it
    for t0, n, name in L.halves(CHORDS[7]):
        backing(s, 7, t0, t0 + n, name, run32=True)
    chords(s, 7, CHORDS[7], vel=100)
    bass(s, 7, CHORDS[7])
    lead(s, 7, LINES[7], vel=120)
    for t, n, k in LINES[7]:
        s.hit('double', 7, t, n - 1, key(k) - 12, 92, 0.25)
    kit(s, 7, crash=True, ghost=13, until=12)
    L.power(s, 7, 0, 30, 'C#3', vel=118)
    L.toms(s, 7)
    # bar 8: C-sharp minor with its ninth, ringing
    s.note('fx', 8, 0, 96, 0, 124, **L.CRASH)
    s.hit('kick', 8, 0, 6)
    s.hit('snare', 8, 0, 6)
    L.power(s, 8, 0, 90, 'C#3', vel=122, fall=True)
    s.hit('pad_m', 8, 0, 150, 'C#4', 108, -0.35)       # C# E G#
    s.hit('pad', 8, 0, 150, 'C#3', 94, 0.35)           # C# D# G#: the ninth
    s.hit('brass', 8, 0, 60, 'E3', 104, 0.3)           # E G# B over C#: the seventh
    s.note('wave', 8, 0, 92, 'C#3', 127, **dict(HELD, bend=[(0, 0), (54, 0), (92, -12)]))
    s.note('sq1', 8, 0, 96, 'C#6', 118, **dict(LEAD, bend=[(0, 0), (76, 0), (96, -2)]))
    return s
