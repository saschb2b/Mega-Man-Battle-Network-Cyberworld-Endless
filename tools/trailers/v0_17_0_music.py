"""0.17.0's music: sketch A, "Busting" (sketch_bust_a.py), the one the
owner chose by ear on 26 October 2026, grown into the cut's 26 bars.
Original, in the manner of BN5's "Virus Busting" (BN5 Team Colonel's song
0x15), the song every layer of the run now plays at least once; none of
its notes, nor its chords in their order. Its habits, read from the ROM,
are in sketch_bust_lib.py's docstring; the palette is FluidR3 GM's
(SOUNDFONT), rendered in docker/Dockerfile.music's image, its 3DS banner
kept as our own audio (BANNER_AUDIO).

What it keeps of 0x15: 148 beats a minute (150 here, a bar 48 video
frames), F minor seldom on its tonic; the stab at one pitch on 3+3+2 twice
with its ghosts before the beat, on the distortion guitar near F4; the kit
(the kick on 0 2 6 8 10, the snare on 4 7 9 12, closed hats in pairs and
an open hat on every "and"); the bass's sixteenths with the "e" soft, the
root on the kick's places; the backing's sixteenths thrown hard left and
right by the beat; IV to bVII, IIm7b5, the Lydian #IVmaj7; a B where the
stab gives way to chords struck at the bar's head; a break of the drums
and the stab alone, the bass out.

The form, on the cut's slots (SKILL.md, The structure):
- Bars 0-1, the cold open: the stab's bar alone, the hats rising; then
  the band without its lead on F minor, the hook's pickup.
- Bars 2-3, the title card: the hook (F held, F, E-flat struck twice,
  D-flat, C held, a pickup back up) over Bbm7 Eb6, its answer over Abmaj7
  Dbmaj7; the toms into the first segment.
- Bars 4-7, the battle floor: the hook again over the same chord sample
  with G under it (Gm7b5), the cadence a semitone up over the Lydian Bmaj7
  then Csus2, the B climbing in sequence over Dbmaj7 Ebsus2 | Fm7 C (the
  major V once).
- Bars 8-11, the picks: the hook on a new chord (Dbmaj7 Eb6), its answer
  in another rhythm, the hook, the cadence.
- Bars 12-15, folders and finds: the break (two bars of the drums and the
  stab alone, the drop, kept short), the B with the band back.
- Bars 16-19, the rivals: the hook, its answer, the hook and the cadence
  with the saw an octave under the square, the climax.
- Bars 20-21, the recap: the B's climb, ending on the major V.
- Bar 22, the stinger: F minor with its ninth on the downbeat, ringing.
- Bars 23-25, the end card: the backing alone, softly, over Dbmaj7 Ebsus2
  and F minor; the strings' last chord under it.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_bust_lib as L  # noqa: E402
from trailers.sketch_bust_lib import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                      HARDWARE, MIX, REVERB, SAMPLED_LEADS, SOUNDFONT)

BARS = 26
TAIL = 2.0
BANNER = (16 * 4 * 60 / BPM, 2.9)        # (from bar 16's downbeat, seconds: the climax's hook)
BANNER_AUDIO = '3ds/banner-0.17.0.wav'


def instruments():
    return L.V.instruments('F2')


HOOK = [(0, 18, 'F5'), (18, 6, 'F5'), (24, 6, 'Eb5'), (30, 6, 'Eb5'), (36, 12, 'Db5'), (48, 30, 'C5'),
        (78, 6, 'C5'), (84, 6, 'Bb4'), (90, 6, 'C5')]
ANSWER = [(0, 12, 'Eb5'), (12, 6, 'Eb5'), (18, 6, 'F5'), (24, 24, 'Ab5'), (48, 12, 'F5'), (60, 6, 'F5'),
          (66, 6, 'Eb5'), (72, 18, 'Db5'), (90, 6, 'C5')]
ANSWER2 = [(0, 6, 'C5'), (6, 6, 'Eb5'), (12, 12, 'Eb5'), (24, 24, 'Ab5'), (48, 18, 'F5'), (66, 6, 'Eb5'),
           (72, 12, 'Db5'), (84, 6, 'C5'), (90, 6, 'C5')]
CADENCE = [(0, 18, 'F#5'), (18, 6, 'F#5'), (24, 6, 'D#5'), (30, 6, 'D#5'), (36, 12, 'C#5'), (48, 36, 'G5'),
           (84, 6, 'G5'), (90, 6, 'G5')]
CLIMB = [(0, 12, 'Ab5'), (12, 6, 'Ab5'), (18, 6, 'G5'), (24, 24, 'F5'), (48, 12, 'Bb5'), (60, 6, 'Bb5'),
         (66, 6, 'Ab5'), (72, 24, 'G5')]
CLIMB2 = [(0, 12, 'C6'), (12, 6, 'C6'), (18, 6, 'Bb5'), (24, 24, 'Ab5'), (48, 6, 'G5'), (54, 6, 'G5'),
          (60, 6, 'F5'), (66, 30, 'E5')]
PICKUP = [(78, 6, 'C5'), (84, 6, 'Bb4'), (90, 6, 'C5')]

# bar: (its section's kind, its chords, the lead's line)
PLAN = {
    0: ('intro', ('Fm9',), None), 1: ('band', ('Fm9',), PICKUP),
    2: ('a', ('Bbm7', 'Eb6'), HOOK), 3: ('a', ('Abmaj7', 'Dbmaj7'), ANSWER),
    4: ('a', ('Gm7b5', 'Eb6'), HOOK), 5: ('a', ('Bmaj7', 'Csus2'), CADENCE),
    6: ('b', ('Dbmaj7', 'Ebsus2'), CLIMB), 7: ('b', ('Fm7', 'C'), CLIMB2),
    8: ('a', ('Dbmaj7', 'Eb6'), HOOK), 9: ('a', ('Abmaj7', 'Dbmaj7'), ANSWER2),
    10: ('a', ('Bbm7', 'Eb6'), HOOK), 11: ('a', ('Bmaj7', 'Csus2'), CADENCE),
    12: ('break', ('Fm9',), None), 13: ('break', ('Fm9',), None),
    14: ('b', ('Dbmaj7', 'Ebsus2'), CLIMB), 15: ('b', ('Fm7', 'C'), CLIMB2),
    16: ('a', ('Bbm7', 'Eb6'), HOOK), 17: ('a', ('Abmaj7', 'Dbmaj7'), ANSWER),
    18: ('a', ('Gm7b5', 'Eb6'), HOOK), 19: ('a', ('Bmaj7', 'Csus2'), CADENCE),
    20: ('b', ('Dbmaj7', 'Ebsus2'), CLIMB), 21: ('b', ('Fm7', 'C'), CLIMB2),
    22: ('ring', ('Fm9',), None),
    23: ('end', ('Dbmaj7', 'Ebsus2'), None), 24: ('end', ('Fm9',), None), 25: ('end', ('Fm9',), None),
}
CRASH = (2, 4, 8, 14, 16)                  # the title and each segment's head but the break's
TOMS = (3, 7, 11, 15, 19, 21)              # into each of them, and into the stinger
CLIMAX = (16, 17, 18, 19)


def nxt(bar):
    return PLAN[bar + 1][1][0] if bar + 1 in PLAN else None


def a_bar(s, bar, names, line):
    cadence = line is CADENCE
    L.backing(s, bar, names, vel=8 if bar in CLIMAX else 0)
    L.chords(s, bar, names, vel=100 if bar in CLIMAX else 94)
    L.bass(s, bar, names, nxt(bar))
    L.stab(s, bar, names, vel=110 if bar in CLIMAX else 104, until=14 if cadence else 16)
    L.lead(s, bar, line, vel=120 if bar in CLIMAX else 116)
    if bar in CLIMAX:
        L.double(s, bar, line)
    L.kit(s, bar, crash=bar in CRASH, until=12 if bar in TOMS else 16)
    if bar in CRASH:
        L.power(s, bar, 0, 30, L.H[names[0]][0], vel=114)


def b_bar(s, bar, names, line):
    L.backing(s, bar, names, rest=True)
    L.chords(s, bar, names, vel=100)
    L.held(s, bar, names, vel=84)
    L.bass(s, bar, names, nxt(bar))
    L.lead(s, bar, line)
    L.kit(s, bar, crash=bar in CRASH, until=12 if bar in TOMS else 16)
    if line is CLIMB2:
        L.power(s, bar, 48, 30, 'C3', vel=112, fall=True)
    elif bar in CRASH:
        L.power(s, bar, 0, 30, 'Db3', vel=112)


def end_bar(s, bar, names):
    """the end card: the backing alone, softly, the strings' last chord"""
    L.backing(s, bar, names, rest=True, vel=-24, until=12 if bar == 25 else 16)
    if bar == 25:
        s.hit('pad_m', bar, 0, 140, 'F3', 84, -0.35)
        s.hit('pad', bar, 0, 140, 'F3', 70, 0.35)


def arrangement():
    s = Score()
    for bar in range(BARS):
        kind, names, line = PLAN[bar]
        if kind == 'intro':
            L.stab(s, bar, names, vel=96)
            for p in range(16):
                s.note('noise', bar, 6 * p, 3, 0, 28 + 4 * p, **L.V.HAT)
            s.note('fx', bar, 0, 92, 0, 100, sweep=(1500, 120000), ramp=(1, 8))
        elif kind == 'band':
            L.backing(s, bar, names)
            L.bass(s, bar, names, nxt(bar))
            L.stab(s, bar, names)
            L.kit(s, bar, crash=True, until=12)
            L.lead(s, bar, line, vel=108)
        elif kind == 'break':
            L.stab(s, bar, names, vel=100)
            L.kit(s, bar, crash=bar == 12, until=12 if bar == 13 else 16)
            if bar == 13:
                L.lead(s, bar, PICKUP, vel=108)
        elif kind == 'a':
            a_bar(s, bar, names, line)
        elif kind == 'b':
            b_bar(s, bar, names, line)
        elif kind == 'ring':
            L.ring(s, bar, 'Fm9', 'C5', 'F2')
            s.hit('pad', bar, 0, 150, 'F4', 92, 0.35)          # F G C: the ninth
        else:
            end_bar(s, bar, names)
        if bar in TOMS:
            L.toms(s, bar)
    return s
