"""Sketch A for 0.15.0, the operation: the owner's pick of three (B, the
liberation's fanfare, and C, the darkness's move, were not chosen), then
written again with more care. Ten bars for the owner's ear, not a cut.
Original, in the manner of BN5's Liberation Mission board theme (Team
Colonel's song 0x22), by the rules of .claude/skills/trailer/
music_rules.md; none of its notes, nor its chords in their order.

    python3 tools/trailer_music.py OUT.wav --piece sketch_lib_a

What 0x22 does, read from the ROM with song_habits.py --detail --theory
(--offset 12, its pickup), numbers only:
- 140 beats a minute in D minor whose sixth is all but missing (B-flat 4%
  of its notes, C-sharp 3% at the turnarounds): a hexatonic minor, D E F G
  A C, between the Aeolian and the Dorian.
- Suspended and open: sus2 18% of its chords, sus4 10%, minor sevenths
  23%, the bIIImaj7 11%, the ii half-diminished 5%, bare fifths 5%; roots
  up a fourth 23%, up a tone 23%, up a fifth 17%, down a tone 15%, up a
  minor third 11%; 1.74 changes a bar.
- Seventeen bars looped: an A of eight with the lead on a square (50%
  duty, held full, vibrato 6 after 14 ticks), the other square striking
  each eighth twice as sixteenths at one level (75% duty, two notes in
  five the same as the last) and thrown to the other side about every
  half bar; a B of eight with the melody on a sampled lead (a few cycles
  looped) and its echo a few ticks late, two notes in three repeated, the
  square's pairs moved to 32nds struck 100 then 60, long pads, the lead
  square resting; one bar of fill.
- The wave's bass a narrow pulse (about three-eighths high) plucked (its
  envelope falling to 4), at full velocity, eleven notes a bar on 0, 2,
  4, 6, 7, 8, 9, 10, 11, 14, 15, legato, between F2 and E-flat 4: the root
  repeated (29% of its moves), a step into the next chord (a tone 27%),
  octaves only 8%.
- The kit: a kick on 0, 2, 6, 7 and 10 of nearly every bar (14 often),
  hats every sixteenth on the noise at three levels, an open hat on 10, a
  crash every four bars. In A a march: the snare on 4 and 12, a ghost on
  9 every other bar, a short bright hit right after the backbeat (5 and
  13), a dark hand drum answering itself a semitone up (1 and 8, 3 and
  11). In B a clap instead, the hand drums and the hit gone. Toms only
  at a section's end.
- Measured from its render by our MP2K (music_check.py): bass, body,
  mids, top 53/28/17/2%, a punch of 5.2 dB: a march more than a battle.

Here, in D minor at 144 (a bar 50 frames at 30 fps, as 0.12.0 made BN5's
142), the board theme's mood, the operation under way. One idea at a
time:
- Bar 0 the count-in: the square's pairs alone over Dsus2 and Dm7, the
  hand drums and the hats rising under a riser, a pickup of two notes.
- Bars 1-4 the A: a one-bar hook in 0x22's own motion, repeated notes
  and steps (three hammered D's, a step down and back, up to E, F
  hammered, falling by steps to C held) over Dm7 Csus2; its answer over
  Fmaj7 Gm7 on other rhythms; the hook again, its tail rising to G and
  falling; a cadence over Em7b5 into Asus4, whose fourth falls to the
  C-sharp on the beat the chord turns major.
- Bars 5-6 the B: the melody to the sampled saw and its echo, three
  pitches a bar, mostly repeated, climbing in sequence (Fmaj7 Csus2 | Gm7
  Am7), the square's pairs to 32nds, the clap kit.
- Bar 7 the hook's head back on the square on the fake-out bVImaj7, its
  tail hammering the high G; bar 8 the turnaround
  over Gm7 and Asus4-A, the toms falling; bar 9 D minor struck and
  ringing.

The bass repeats its root on the kick's places, steps to its neighbour
and back, leaps to the fifth once a bar, and steps into each chord on the
last sixteenth before it.
Instruments: FluidR3 GM's (Frank Wen, the MIT license; Debian's
fluid-soundfont-gm in docker/Dockerfile.music's image, never in git), as
0.14.0's, cut into 10512 Hz 8-bit samples: the electronic kit's kick,
the snare, the clap over a snare, the side stick, the low conga played as
a pitched sample, the studio toms, the strings' chords, the saw lead
(program 81), the orchestra hit, the brass section at the end.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_soundfont as sf  # noqa: E402
from trailer_music import Score, key, keys  # noqa: E402

SOUNDFONT = sf.SOUNDFONT
BPM = 144
BARS = 10
TAIL = 1.8
MIX = (('lead', ('sq1',), 0.11), ('arp', ('sq2',), 0.15), ('bass', ('wave',), 0.18),
       ('hats', ('noise',), 0.12), ('fx', ('fx',), 0.08))
DS_LEVEL = 1.0
REVERB = 40
HARDWARE = 32768
CHORD_SAMPLES = {'pad': (0, 2, 7, 12, 14), 'pad_m': (0, 3, 7, 12, 15), 'pad_M': (0, 4, 7, 12, 16),
                 'brass': (0, 4, 7, 12)}
BANNER = (7 * 4 * 60 / BPM, 2.9)      # the hook's return on the fake-out

DS_ENV = {'kick': (255, 250, 190, 208), 'snare': (255, 250, 190, 208), 'clap': (255, 250, 190, 208),
          'stick': (255, 0, 255, 196), 'hand': (255, 0, 255, 220), 'tom_hi': (255, 0, 255, 230),
          'tom_mid': (255, 0, 255, 230), 'tom_lo': (255, 0, 255, 230), 'pad': (60, 0, 235, 210),
          'pad_m': (60, 0, 235, 210), 'pad_M': (60, 0, 235, 210), 'dlead': (255, 0, 230, 190),
          'brass': (255, 0, 230, 190), 'impact': (255, 0, 255, 215)}
DS_GAIN = {'kick': 0.95, 'snare': 0.7, 'clap': 0.72, 'stick': 0.32, 'hand': 0.42, 'tom_hi': 0.58,
           'tom_mid': 0.58, 'tom_lo': 0.62, 'pad': 0.72, 'pad_m': 0.72, 'pad_M': 0.72, 'dlead': 0.3,
           'brass': 0.45, 'impact': 0.55}

LEAD = dict(duty=0.5, env=(0, 0, 13, 2), vib=(24, 14), pan=0.0)
PAIRS = dict(duty=0.75, env=(0, 1, 11, 1))
PAIRS32 = dict(duty=0.75, env=(0, 1, 10, 1))
PULSE = [13] * 12 + [0] * 20                      # a narrow pulse, three-eighths high
BASS = dict(env=(0, 4, 6, 1), wave=PULSE)
HELD = dict(env=(0, 0, 15, 3), wave=PULSE)
HAT = dict(clock=262144, env=(0, 0, 11, 0), pan=0.2)
OPEN = dict(clock=262144, env=(0, 2, 11, 3), pan=0.25)
CRASH = dict(clock=80000, env=(0, 7, 0, 1), pan=-0.15)


def instruments():
    S = {
        'kick': sf.drum(36, 0.14, program=24, drive=2.4),
        'snare': sf.drum(40, 0.12, layer=(38,), drive=1.8),
        'clap': sf.drum(39, 0.09, layer=(40,), drive=2.0),
        'stick': sf.drum(37, 0.07),
        'hand': sf.drum(64, 0.3),
        'tom_hi': sf.drum(50, 0.35),
        'tom_mid': sf.drum(47, 0.35),
        'tom_lo': sf.drum(45, 0.38),
        'dlead': sf.instrument(81, [72], 1.0, fade=0.1),
        'brass': sf.instrument(61, [60 + i for i in CHORD_SAMPLES['brass']], 0.5, base=60, fade=0.12),
        'impact': sf.instrument(55, [key('D3')], 0.45),
    }
    S['hand'].base = 60          # one hand drum, played a semitone apart
    S['impact'].base = None
    for name in ('pad', 'pad_m', 'pad_M'):
        S[name] = sf.instrument(48, [60 + i for i in CHORD_SAMPLES[name]], 2.0, base=60, fade=0.3)
    return S


# ---- the harmony: (bass root, the square's four notes, the chord sample
# and its root): a triad sample a third over the bass makes the seventh ----
H = {
    'Dm7': ('D3', 'A4 C5 D5 F5', 'pad_M', 'F3'),        # F over D
    'Dsus2': ('D3', 'A4 D5 E5 A5', 'pad', 'D3'),
    'Csus2': ('C3', 'G4 C5 D5 G5', 'pad', 'C3'),
    'Fmaj7': ('F2', 'A4 C5 E5 F5', 'pad_m', 'A3'),       # Am over F
    'Gm7': ('G2', 'Bb4 D5 F5 G5', 'pad_M', 'Bb3'),       # Bb over G
    'Am7': ('A2', 'A4 C5 E5 G5', 'pad_M', 'C4'),         # C over A
    'Em7b5': ('E3', 'Bb4 D5 E5 G5', 'pad_m', 'G3'),      # Gm over E
    'Asus4': ('A2', 'A4 D5 E5 A5', 'pad', 'D3'),         # D E A over A
    'A': ('A2', 'A4 C#5 E5 A5', 'pad_M', 'A3'),
    'Bbmaj7': ('Bb2', 'Bb4 D5 F5 A5', 'pad_m', 'D3'),    # Dm over Bb
}
SCALE = {0, 1, 2, 4, 5, 7, 9, 10}     # D minor's notes, the C-sharp for A

BARS_OF = {1: ('Dm7', 'Csus2'), 2: ('Fmaj7', 'Gm7'), 3: ('Dm7', 'Csus2'), 4: ('Em7b5', 'Asus4', 'A'),
           5: ('Fmaj7', 'Csus2'), 6: ('Gm7', 'Am7'), 7: ('Bbmaj7', 'Csus2'), 8: ('Gm7', 'Asus4', 'A')}

# ---- the lines: (tick, ticks, note): 0x22's lead repeats a note 44% of
# the time and steps 43%, its sampled lead repeats 61% on under three
# pitches a bar (music_check.py --source bn5 0x22 --offset 12) ----
HOOK = [(0, 6, 'D6'), (6, 6, 'D6'), (12, 6, 'D6'), (18, 6, 'C6'), (24, 12, 'D6'), (36, 12, 'E6'), (48, 6, 'F6'),
        (54, 6, 'F6'), (60, 6, 'E6'), (66, 6, 'D6'), (72, 24, 'C6')]
LINES = {
    1: HOOK,
    2: [(0, 12, 'E6'), (12, 6, 'E6'), (18, 6, 'F6'), (24, 18, 'E6'), (42, 6, 'C6'), (48, 6, 'D6'), (54, 6, 'D6'),
        (60, 6, 'D6'), (66, 6, 'D6'), (72, 24, 'F6')],
    3: HOOK[:6] + [(48, 6, 'F6'), (54, 6, 'G6'), (60, 12, 'F6'), (72, 12, 'E6'), (84, 12, 'D6')],
    4: [(0, 6, 'D6'), (6, 6, 'D6'), (12, 6, 'D6'), (18, 6, 'C6'), (24, 12, 'Bb5'), (36, 12, 'C6'), (48, 24, 'D6'), (72, 24, 'C#6')],
    7: HOOK[:4] + [(24, 12, 'D6'), (36, 12, 'F6'), (48, 6, 'G6'), (54, 6, 'G6'), (60, 6, 'F6'), (66, 6, 'G6'),
                   (72, 24, 'G6')],
    8: [(0, 6, 'G6'), (6, 6, 'G6'), (12, 6, 'F6'), (18, 6, 'G6'), (24, 12, 'F6'), (36, 12, 'D6'), (48, 6, 'E6'), (54, 6, 'E6'),
        (60, 12, 'D6'), (72, 12, 'C#6'), (84, 12, 'E6')],
}
CLIMB = {
    5: [(0, 6, 'A5'), (6, 6, 'A5'), (12, 6, 'A5'), (18, 6, 'C6'), (24, 12, 'C6'), (36, 12, 'C6'), (48, 6, 'G5'),
        (54, 6, 'G5'), (60, 6, 'G5'), (66, 6, 'G5'), (72, 24, 'A5')],
    6: [(0, 6, 'Bb5'), (6, 6, 'Bb5'), (12, 6, 'Bb5'), (18, 6, 'D6'), (24, 12, 'D6'), (36, 12, 'D6'), (48, 6, 'C6'),
        (54, 6, 'C6'), (60, 6, 'C6'), (66, 6, 'C6'), (72, 12, 'E6'), (84, 12, 'E6')],
}


def halves(names):
    """A bar's chords as (tick, ticks, name): two halves, or the second
    split in two."""
    if len(names) == 2:
        return [(0, 48, names[0]), (48, 48, names[1])]
    return [(0, 48, names[0]), (48, 24, names[1]), (72, 24, names[2])]


def lead(s, bar, line, vel=116, ch='sq1', voice=LEAD, octave=0):
    for t, n, k in line:
        s.note(ch, bar, t, n, key(k) + octave, vel - (10 if n <= 6 and t % 12 else 0), **voice)


def saw_lead(s, bar, line, vel=110, echo=3):
    """The B's melody on the sampled saw, its echo `echo` ticks late,
    softer, a little to the other side."""
    for t, n, k in line:
        s.hit('dlead', bar, t, n - 1, k, vel, -0.25)
        s.hit('dlead', bar, t + echo, n - 1, k, vel - 48, 0.4)


PAIR_FIGURE = (0, 0, 1, 1, 2, 2, 1, 1)
RUN_FIGURE = (0, 1, 2, 3, 2, 1, 2, 3)


def pairs(s, bar, t0, t1, name, vel=100):
    """The A's square: each eighth struck twice in sixteenths, thrown to
    the other side every half bar."""
    v = keys(H[name][1])
    side = -0.85 if t0 % 96 < 48 else 0.85
    for i, t in enumerate(range(t0, t1, 6)):
        s.note('sq2', bar, t, 6, v[PAIR_FIGURE[i % 8]], vel, pan=side, **PAIRS)


def pairs32(s, bar, t0, t1, name, vel=100, soft=60):
    """The B's square: each sixteenth struck twice as 32nds, 100 then 60."""
    v = keys(H[name][1])
    side = -0.85 if t0 % 96 < 48 else 0.85
    for i, t in enumerate(range(t0, t1, 6)):
        k = v[RUN_FIGURE[i % 8]]
        s.note('sq2', bar, t, 3, k, vel, pan=side, **PAIRS32)
        s.note('sq2', bar, t + 3, 3, k, soft, pan=side, **PAIRS32)


def chord(s, bar, t0, name, n, vel=92):
    _, _, pad, root = H[name]
    s.hit(pad, bar, t0, n - 2, key(root) + 12, vel, -0.35 if t0 % 96 < 48 else 0.35)


def step_into(root):
    """The note a step from `root` the bass approaches it by: a tone
    above where the key has it, else a tone below."""
    for d in (2, -2, -1):
        if (root + d) % 12 in SCALE:
            return root + d
    return root


def bass(s, bar, first, second, nxt):
    """The root on the kick's places, a step to its neighbour and back,
    its fifth once, a step into the next chord on the last sixteenth of
    each half: 0x22's bass repeats 29%, steps 32%, leaps 22%."""
    a, b, c = key(H[first][0]), key(H[second][0]), key(H[nxt][0])
    notes = [(0, a), (2, a), (4, step_into(a + 1) if (a + 2) % 12 in SCALE else a), (6, a),
             (7, step_into(b) if b != a else a),
             (8, b), (9, b), (10, b + 7), (11, b), (14, b), (15, step_into(c) if c != b else b)]
    for i, (p, k) in enumerate(notes):
        end = notes[i + 1][0] if i + 1 < len(notes) else 16
        s.note('wave', bar, 6 * p, 6 * (end - p) - 1, k, 127, **BASS)


KICK = (0, 2, 6, 7, 10, 14)


def hats(s, bar, until=16):
    for p in range(until):
        if p == 10:
            s.note('noise', bar, 60, 6, 0, 88, **OPEN)
        else:
            s.note('noise', bar, 6 * p, 3, 0, (100, 40, 70, 40)[p % 4], **HAT)


def kit(s, bar, march=True, until=16, crash=False, ghost=False):
    """The A's march (the snare, the stick after it, the hand drum
    answering itself) or the B's claps."""
    for p in KICK:
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=118 if p != 14 else 100)
    back = 'snare' if march else 'clap'
    for p in (4, 12):
        if p < until:
            s.hit(back, bar, 6 * p, 6, vel=118, pan=0.05)
    if ghost and until > 9:
        s.hit(back, bar, 54, 6, vel=54, pan=-0.1)
    if march:
        for p in (5, 13):
            if p < until:
                s.hit('stick', bar, 6 * p, 4, vel=90, pan=-0.2)
        for p, k in ((1, 60), (8, 60), (3, 61), (11, 61)):
            if p < until:
                s.hit('hand', bar, 6 * p, 6, k, 66, 0.25 if k == 60 else -0.25)
    hats(s, bar, until)
    if crash:
        s.note('fx', bar, 0, 48, 0, 118, **CRASH)


def toms(s, bar, t0=72, vel=112):
    for t, tom, pan in ((t0, 'tom_hi', -0.3), (t0 + 6, 'tom_hi', -0.3), (t0 + 12, 'tom_mid', 0.0),
                        (t0 + 18, 'tom_lo', 0.3)):
        s.hit(tom, bar, t, 6, vel=vel, pan=pan)


# ---- the sections ----

def count_in(s):
    """Bar 0: the square's pairs alone, the hand drums and hats rising
    under a riser, the pickup."""
    for t, n, name in halves(('Dsus2', 'Dm7')):
        pairs(s, 0, t, t + n, name, vel=84 + t // 6)
        chord(s, 0, t, name, n, vel=70 + t // 3)
    for p, k in ((1, 60), (3, 61), (8, 60), (11, 61)):
        s.hit('hand', 0, 6 * p, 6, k, 54 + 2 * p, 0.25 if k == 60 else -0.25)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 30 + 4 * p, **HAT)
    s.note('fx', 0, 0, 92, 0, 96, sweep=(2000, 180000), ramp=(1, 9))
    s.note('sq1', 0, 84, 6, 'F5', 104, **LEAD)
    s.note('sq1', 0, 90, 6, 'G5', 110, **LEAD)


def section_a(s):
    for bar in range(1, 5):
        names = BARS_OF[bar]
        for t, n, name in halves(names):
            pairs(s, bar, t, t + n, name)
            chord(s, bar, t, name, n)
        bass(s, bar, names[0], names[1], BARS_OF[bar + 1][0])
        lead(s, bar, LINES[bar])
        kit(s, bar, until=14 if bar == 4 else 16, crash=bar == 1, ghost=bar % 2 == 0)
    s.hit('impact', 1, 0, 24, vel=116)
    for t, tom in ((84, 'tom_mid'), (90, 'tom_lo')):
        s.hit(tom, 4, t, 6, vel=110, pan=0.2)


def section_b(s):
    for bar in (5, 6):
        names = BARS_OF[bar]
        for t, n, name in halves(names):
            pairs32(s, bar, t, t + n, name)
            chord(s, bar, t, name, n, vel=100)
        bass(s, bar, names[0], names[1], BARS_OF[bar + 1][0])
        saw_lead(s, bar, CLIMB[bar])
        kit(s, bar, march=False, crash=bar == 5, ghost=bar == 6)
    s.hit('impact', 5, 0, 24, vel=110)


def return_and_end(s):
    """Bar 7: the hook on the fake-out; bar 8: the turnaround, the toms
    falling; bar 9: D minor struck and ringing."""
    for bar in (7, 8):
        names = BARS_OF[bar]
        for t, n, name in halves(names):
            pairs32(s, bar, t, t + n, name)
            chord(s, bar, t, name, n, vel=102)
        bass(s, bar, names[0], names[1], 'Dm7' if bar == 8 else BARS_OF[bar + 1][0])
        lead(s, bar, LINES[bar], vel=120)
        for t, n, k in LINES[bar]:
            s.hit('dlead', bar, t, n - 1, key(k) - 12, 92, 0.25)
        kit(s, bar, march=False, until=12 if bar == 8 else 16, crash=bar == 7)
    s.hit('impact', 7, 0, 24, vel=118)
    toms(s, 8)
    s.note('fx', 9, 0, 96, 0, 124, **CRASH)
    s.hit('impact', 9, 0, 48, vel=124)
    s.hit('kick', 9, 0, 6)
    s.hit('snare', 9, 0, 6)
    s.hit('pad_m', 9, 0, 140, 'D4', 108, -0.35)       # D F A
    s.hit('pad', 9, 0, 140, 'D3', 92, 0.35)           # D E A: the ninth's colour
    s.hit('brass', 9, 0, 54, 'F3', 100, 0.3)          # F A C over D: its seventh
    s.note('wave', 9, 0, 90, 'D3', 127, **dict(HELD, bend=[(0, 0), (54, 0), (90, -12)]))
    s.note('sq1', 9, 0, 96, 'D6', 118, **dict(LEAD, bend=[(0, 0), (76, 0), (96, -2)]))
    s.note('sq2', 9, 0, 96, 'A5', 96, **dict(LEAD, pan=0.35, bend=[(0, 0), (76, 0), (96, -2)]))


def arrangement():
    s = Score()
    count_in(s)
    section_a(s)
    section_b(s)
    return_and_end(s)
    return s
