"""0.15.0's trailer music: an original piece in the manner of BN5's
Liberation Mission board theme (Team Colonel's song 0x22), grown from
sketch A (tools/trailers/sketch_lib_a.py), the one the owner chose by
ear and heard again once written with care, by .claude/skills/trailer/
music_rules.md and its Discipline. None of Capcom's notes, chords in
their order or samples. 0x22's habits, read from the ROM, are in the
sketch's docstring; this piece keeps the sketch's parts, its helpers and
its sound (FluidR3 GM's samples, as 0.14.0's), and adds what a trailer
needs around them.

Here, in D minor at 144 (a bar is 50 frames of the trailer's 30), 20
bars, a section's idea at its head and nowhere else:
- 0-3 the intro: the square's pairs alone over Dsus2 and Dm7, the hats
  rising; bar 1 the strings and the hand drums under Bbmaj7 Csus2, the
  bass held; bar 2 the beat in half-time (Dm7 Csus2); bar 3 in full to
  beat three over Gm7 and Asus4-A, the toms, the pickup's leading tone.
- 4-11 the sketch's eight bars: the hook (three hammered D's, a step
  down and back, up to E, F hammered, falling by steps to C) over Dm7
  Csus2, its answer over Fmaj7 Gm7, the hook again, the cadence over
  Em7b5 into Asus4 and its C-sharp; the B, the saw and its echo climbing
  on repeated notes (Fmaj7 Csus2 | Gm7 Am7), the square's pairs in 32nds,
  the clap kit; the hook's head on the fake-out Bbmaj7; the turnaround
  over Gm7 and Asus4-A.
- 12-13 the drop, kept short: D minor struck, then one thin square alone
  with the hook at half its speed over the bass's one D and the hand
  drums; bar 13 the climb back over Bbmaj7 and Csus2, the pairs lifted an
  octave, a clap roll and a riser, the pickup.
- 14-17 the climax: the hook, its answer, the hook, its head on Bbmaj7,
  the saw doubling the square an octave down, the 32nds, brass stabs
  where a triad sample makes the chord (B-flat over G is Gm7, F over D
  Dm7), the march kit back with a crash every two bars.
- 18-19 the logo and the end card: the turnaround, the toms falling and
  a clap roll; then D minor struck and ringing, the bass falling away.

Its BANNER, the 3DS HOME Menu's sound while it is the current trailer:
the climax's first bar and a half, the hook doubled. The piece plays a
soundfont, so it renders in docker/Dockerfile.music's image
(tools/trailer_music.py's render_any) and keeps its banner as our own
audio in BANNER_AUDIO, written by
`python3 tools/trailer_music.py --write-banner --piece v0_15_0_music`.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score, key  # noqa: E402
from trailers import sketch_lib_a as a  # noqa: E402

SOUNDFONT = a.SOUNDFONT
BPM = a.BPM
BARS = 20
TAIL = 2.0
MIX, DS_LEVEL, REVERB, HARDWARE = a.MIX, a.DS_LEVEL, a.REVERB, a.HARDWARE
CHORD_SAMPLES = a.CHORD_SAMPLES
# the saw doubling the square an octave down: the saw's sample under a
# name of its own, so music_check.py reads the B's melody alone as the
# sampled lead (SAMPLED_LEADS) and the doubling as the lead's
DS_ENV = dict(a.DS_ENV, double=a.DS_ENV['dlead'])
DS_GAIN = dict(a.DS_GAIN, double=a.DS_GAIN['dlead'])
SAMPLED_LEADS = ('dlead',)
BANNER = (14 * 4 * 60 / BPM, 2.9)      # (from bar 14's downbeat, seconds)
BANNER_AUDIO = '3ds/banner-0.15.0.wav'


def instruments():
    S = a.instruments()
    S['double'] = S['dlead']
    return S

THIN = dict(duty=0.125, env=(0, 0, 12, 4), vib=(30, 8), pan=0.0)
BRASS = {'Dm7': 'F3', 'Gm7': 'Bb3', 'Fmaj7': 'F3', 'Bbmaj7': 'Bb3', 'A': 'A3'}
# the sketch's bars 1-8 are this piece's 4-11; the climax's chords
MAIN = {4 + i: a.BARS_OF[1 + i] for i in range(8)}
MAIN_LINES = {4 + i: a.LINES[1 + i] for i in (0, 1, 2, 3, 6, 7)}
CLIMAX = {14: (a.BARS_OF[1], a.LINES[1]), 15: (a.BARS_OF[2], a.LINES[2]), 16: (a.BARS_OF[3], a.LINES[3]),
          17: (a.BARS_OF[7], a.LINES[7])}


# every bar's chords, for the bass's step into the next
CHORDS = {1: ('Bbmaj7', 'Csus2'), 2: ('Dm7', 'Csus2'), 3: ('Gm7', 'Asus4', 'A'), **MAIN, 12: ('Dm7',),
          13: ('Bbmaj7', 'Csus2'), **{bar: names for bar, (names, _) in CLIMAX.items()}, 18: a.BARS_OF[8],
          19: ('Dm7',)}


def next_root(bar):
    return CHORDS[bar + 1][0]


def band(s, bar, names, *, run32=False, chord_vel=96):
    for t, n, name in a.halves(names):
        (a.pairs32 if run32 else a.pairs)(s, bar, t, t + n, name)
        a.chord(s, bar, t, name, n, vel=chord_vel)
    a.bass(s, bar, names[0], names[1], next_root(bar))


def brass(s, bar, names):
    for t, n, name in a.halves(names):
        if name in BRASS:
            s.hit('brass', bar, t, 14, BRASS[name], 112, 0.35 if t < 48 else -0.35)
            if n == 48:
                s.hit('brass', bar, t + 36, 8, BRASS[name], 98, 0.35 if t < 48 else -0.35)


# ---- the sections ----

def intro(s):
    """Bars 0-3: the pairs alone; the strings, the hand drums and the bass
    held; the beat in half-time; in full to beat three, the pickup."""
    for t, n, name in a.halves(('Dsus2', 'Dm7')):
        a.pairs(s, 0, t, t + n, name, vel=80 + t // 6)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 28 + 4 * p, **a.HAT)
    s.note('fx', 0, 0, 92, 0, 92, sweep=(1500, 120000), ramp=(1, 7))
    # bar 1: the strings, the hand drums, the bass held
    for t, n, name in a.halves(('Bbmaj7', 'Csus2')):
        a.pairs(s, 1, t, t + n, name, vel=100)
        a.chord(s, 1, t, name, n, vel=84 + t // 4)
        s.note('wave', 1, t, n - 2, a.H[name][0], 120, **a.HELD)
    for p, k in ((1, 60), (3, 61), (8, 60), (11, 61)):
        s.hit('hand', 1, 6 * p, 6, k, 62, 0.25 if k == 60 else -0.25)
    for p in range(16):
        s.note('noise', 1, 6 * p, 3, 0, (80, 36, 56, 36)[p % 4], **a.HAT)
    s.note('fx', 1, 0, 92, 0, 104, sweep=(2000, 180000), ramp=(1, 10))
    # bar 2: the beat in half-time
    s.note('fx', 2, 0, 48, 0, 118, **a.CRASH)
    s.hit('impact', 2, 0, 24, vel=114)
    band(s, 2, ('Dm7', 'Csus2'))
    for t, v in ((0, 120), (36, 104), (66, 110)):
        s.hit('kick', 2, t, 6, vel=v)
    s.hit('snare', 2, 48, 6, vel=118, pan=0.05)
    for p in range(0, 16, 2):
        s.note('noise', 2, 6 * p, 3, 0, 96 if p % 4 == 0 else 62, **a.HAT)
    for p, k in ((1, 60), (3, 61), (8, 60), (11, 61)):
        s.hit('hand', 2, 6 * p, 6, k, 64, 0.25 if k == 60 else -0.25)
    # bar 3: in full to beat three, the toms, the pickup
    band(s, 3, ('Gm7', 'Asus4', 'A'))
    a.kit(s, 3, until=12)
    a.toms(s, 3)
    s.note('sq1', 3, 84, 6, 'A5', 106, **a.LEAD)
    s.note('sq1', 3, 90, 6, 'C#6', 112, **a.LEAD)


def main(s):
    """Bars 4-11: the sketch's A, B, return and turnaround."""
    for bar, names in MAIN.items():
        b = bar - 3                      # the sketch's bar
        march, run32 = b <= 4, b >= 5
        band(s, bar, names, run32=run32, chord_vel=96 if march else 102)
        if b in (5, 6):
            a.saw_lead(s, bar, a.CLIMB[b])
        else:
            a.lead(s, bar, MAIN_LINES[bar], vel=116 if march else 120)
        if b in (7, 8):
            for t, n, k in MAIN_LINES[bar]:
                s.hit('double', bar, t, n - 1, key(k) - 12, 92, 0.25)
        a.kit(s, bar, march=march, until=14 if b == 4 else 12 if b == 8 else 16, crash=b in (1, 5, 7),
              ghost=b % 2 == 0)
    s.hit('impact', 4, 0, 24, vel=118)
    s.hit('impact', 8, 0, 24, vel=110)
    s.hit('impact', 10, 0, 24, vel=114)
    for t, tom in ((84, 'tom_mid'), (90, 'tom_lo')):
        s.hit(tom, 7, t, 6, vel=110, pan=0.2)
    a.toms(s, 11)


def drop(s):
    """Bar 12: D minor struck, one thin square alone with the hook at half
    its speed over the bass's one D and the hand drums. Bar 13: the climb
    back over Bbmaj7 and Csus2, the pairs an octave up, a clap roll, a
    riser, the pickup."""
    s.note('fx', 12, 0, 72, 0, 118, **a.CRASH)
    s.hit('impact', 12, 0, 36, vel=120)
    s.hit('kick', 12, 0, 6, vel=124)
    s.note('wave', 12, 0, 190, 'D3', 112, **dict(a.HELD, ramp=(15, 6)))
    a.chord(s, 12, 0, 'Dm7', 96, vel=80)
    for t, n, k in ((0, 12, 'D6'), (12, 12, 'D6'), (24, 12, 'D6'), (36, 12, 'C6'), (48, 24, 'D6'), (72, 24, 'E6')):
        s.note('sq1', 12, t, n, k, 114, **THIN)
    for p, k in ((8, 60), (11, 61), (13, 60)):
        s.hit('hand', 12, 6 * p, 6, k, 60, 0.25 if k == 60 else -0.25)
    for t in (24, 48, 72):
        s.note('noise', 12, t, 3, 0, 52, **a.HAT)
    # bar 13: the climb back
    s.note('sq1', 13, 0, 72, 'F6', 114, **dict(THIN, bend=[(0, 0), (60, 0), (72, 1)]))
    s.note('fx', 13, 0, 90, 0, 112, sweep=(2500, 240000), ramp=(2, 13))
    for i, (t, n, name) in enumerate(a.halves(('Bbmaj7', 'Csus2'))):
        a.pairs32(s, 13, t, t + n - 12 * i, name, vel=88 + 10 * i, soft=48)
        a.chord(s, 13, t, name, n, vel=94 + 8 * i)
        s.note('wave', 13, t, n - 2, a.H[name][0], 122, **a.HELD)
    for t in (0, 24, 48, 72):
        s.hit('kick', 13, t, 6, vel=116)
    roll = list(range(0, 48, 12)) + list(range(48, 72, 6)) + list(range(72, 96, 3))
    for i, t in enumerate(roll):
        s.hit('clap', 13, t, 3, vel=56 + 64 * i // len(roll), pan=-0.25 + 0.5 * i / len(roll))
    s.note('sq1', 13, 84, 6, 'G5', 112, **a.LEAD)
    s.note('sq1', 13, 90, 6, 'C6', 116, **a.LEAD)


def climax(s):
    """Bars 14-17: the hook again, doubled by the saw an octave down, the
    32nds, the brass, the march kit."""
    for bar, (names, line) in CLIMAX.items():
        band(s, bar, names, run32=True, chord_vel=104)
        a.lead(s, bar, line, vel=122)
        for t, n, k in line:
            s.hit('double', bar, t, n - 1, key(k) - 12, 96, 0.25)
        brass(s, bar, names)
        a.kit(s, bar, crash=bar % 2 == 0, ghost=bar % 2 == 1)
    s.hit('impact', 14, 0, 24, vel=124)
    s.hit('impact', 16, 0, 24, vel=116)


def ending(s):
    """Bar 18: the turnaround, the toms and a clap roll; bar 19: D minor
    struck and ringing."""
    band(s, 18, a.BARS_OF[8], run32=True, chord_vel=104)
    a.lead(s, 18, a.LINES[8], vel=122)
    for t, n, k in a.LINES[8]:
        s.hit('double', 18, t, n - 1, key(k) - 12, 96, 0.25)
    brass(s, 18, a.BARS_OF[8])
    a.kit(s, 18, march=False, until=12, crash=True)
    a.toms(s, 18)
    for i, t in enumerate(range(72, 96, 3)):
        s.hit('clap', 18, t, 3, vel=64 + 6 * i, pan=-0.25 + 0.07 * i)
    s.note('fx', 19, 0, 96, 0, 126, **a.CRASH)
    s.hit('impact', 19, 0, 48, vel=126)
    s.hit('kick', 19, 0, 6)
    s.hit('snare', 19, 0, 6)
    s.hit('pad_m', 19, 0, 150, 'D4', 110, -0.35)      # D F A
    s.hit('pad', 19, 0, 150, 'D3', 94, 0.35)          # D E A: the ninth's colour
    s.hit('brass', 19, 0, 60, 'F3', 112, 0.3)         # F A C over D: its seventh
    s.note('wave', 19, 0, 92, 'D3', 127, **dict(a.HELD, bend=[(0, 0), (54, 0), (92, -12)]))
    s.note('sq1', 19, 0, 96, 'D6', 120, **dict(a.LEAD, bend=[(0, 0), (76, 0), (96, -2)]))
    s.note('sq2', 19, 0, 96, 'A5', 98, **dict(a.LEAD, pan=0.35, bend=[(0, 0), (76, 0), (96, -2)]))


def arrangement():
    s = Score()
    intro(s)
    main(s)
    drop(s)
    climax(s)
    ending(s)
    return s
