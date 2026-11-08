"""0.20.0's music: sketch A, "Bracket" (sketch_sun_a.py), grown into the
cut's 25 bars, with sketch B's anthem (sketch_sun_b.py) as its third
segment. Original, in the manner of BN4 Red Sun's tournament theme (BN4's
song 0x17), the release's own feature's theme; none of its notes, nor its
chords in their order. Its habits, read from BN4's ROM, are in
sketch_sun_lib.py's docstring; the palette is FluidR3 GM's (SOUNDFONT),
rendered in docker/Dockerfile.music's image, its 3DS banner kept as our
own audio (BANNER_AUDIO).

Every part plays FluidR3 GM's instruments (the owner's gate 3, SKILL.md):
the lead, 0x17's two squares in unison, on 81 Saw Wave doubled by square
1 at 12.5%, its echo four ticks later from the other side; the backing,
0x17's busy sampled arpeggio, on 13 Xylophone doubled by square 2 at 25%,
right of centre; the bass, its wave, on 34 Electric Bass (pick), short
(SAMPLED_PARTS); the held chord on 48 Strings, left; the bare fifths'
bite on 30 Distortion Guitar; the kit 24's. The GBA's noise channel keeps
the hats, the open hats and the crash.

The sketches went to the owner with this piece, unheard by him: the
agent grew A (music_check.py: "busier than the source: none", every part
on the soundfont, no lead note rubbing the harmony) and used B's held
anthem where the cut reaches the tournament. Should he choose B, its
anthem leads instead and the A's hook is its answer.

What it keeps of 0x17: 140 beats a minute (144 here, a bar 50 video
frames); a minor home with its neighbour a semitone up leaning on it, bare
fifths most of the time, maj7 and suspended colours, two chords a bar
over a bass that repeats the bar's root; the kick on every beat, the snare
on two and four with ghosts on 7 and 13, the hats skipping the snare,
the open hat on 2 and 10; the lead entering late, busy in the A (struck
twice before a leap, octave jumps, falls at its long notes), held in the
B; its echo from the other side; the arpeggio of four-tick notes right of
centre; the held chord left.

The form, on the cut's slots (SKILL.md, The structure):
- Bars 0-1, the cold open: the band without its lead, the kick alone under
  the arpeggio, the bass and the strings; then the snare and the crash.
- Bars 2-3, the title card: the hook (E struck twice, the leap to A, the
  fall by step, a long F falling) over A5 Bb5, its answer over A5 Bbmaj7.
- Bars 4-7, BN4 joins: the hook turned up onto B5, the cadence held,
  the hook again, the cadence; the toms into the next.
- Bars 8-10, its viruses: the B, held notes climbing in sequence over
  Fmaj7 Dsus4 | Bbmaj7 C5 | Dm7 E5; the toms.
- Bars 11-14, the tournament: sketch B's anthem, a long note a half bar
  over Fmaj7 Gsus2 | A5 Bb5 | Fmaj7 Gsus2 | Dm7 E5, the guitar out.
- Bars 15-18, BN4's Navis: the hook at full strength, answered, turned
  up, cadenced; the toms.
- Bars 19-20, the recap: one bar thinned to the arpeggio and the strings
  (the drop, kept short), then the band and the hook again.
- Bar 21, the stinger: A5 ringing.
- Bars 22-24, the end card: the arpeggio alone, softly, the strings' last
  chord under it.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_sun_lib as L  # noqa: E402
from trailers.sketch_sun_lib import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                     HARDWARE, MIX, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, instruments)
from trailers.sketch_sun_a import ANSWER, CADENCE, CLIMB, HOOK, HOOK_UP  # noqa: E402
from trailers.sketch_sun_b import ANTHEM  # noqa: E402

BARS = 25
TAIL = 2.0
BANNER = (2 * 4 * 60 / BPM, 2.9)         # (from bar 2's downbeat, seconds: the hook with the title)
BANNER_AUDIO = '3ds/banner-0.20.0.wav'

CHORDS = {0: ('A5', 'Bb5'), 1: ('A5', 'Bb5'), 2: ('A5', 'Bb5'), 3: ('A5', 'Bbmaj7'),
          4: ('A5', 'B5'), 5: ('Bb5', 'A5'), 6: ('A5', 'Bb5'), 7: ('Bb5', 'A5'),
          8: ('Fmaj7', 'Dsus4'), 9: ('Bbmaj7', 'C5'), 10: ('Dm7', 'E5'),
          11: ('Fmaj7', 'Gsus2'), 12: ('A5', 'Bb5'), 13: ('Fmaj7', 'Gsus2'), 14: ('Dm7', 'E5'),
          15: ('A5', 'Bb5'), 16: ('A5', 'Bbmaj7'), 17: ('A5', 'B5'), 18: ('Bb5', 'A5'),
          19: ('Fmaj7', 'Gsus2'), 20: ('A5', 'Bb5'), 21: ('A5',),
          22: ('A5', 'Bb5'), 23: ('Fmaj7', 'Gsus2'), 24: ('A5',)}
LINES = {2: HOOK, 3: ANSWER, 4: HOOK_UP, 5: CADENCE, 6: HOOK, 7: CADENCE,
         8: CLIMB[5], 9: CLIMB[6], 10: CLIMB[7],
         11: ANTHEM[1], 12: ANTHEM[2], 13: ANTHEM[3], 14: ANTHEM[4],
         15: HOOK, 16: ANSWER, 17: HOOK_UP, 18: CADENCE, 20: HOOK}


def band(s, bar, crash=False, fill=False, power=True, vel=0):
    names = CHORDS[bar]
    until = 12 if fill else 16
    L.arp(s, bar, names, vel=vel)
    L.bass(s, bar, names, until=until)
    L.pad(s, bar, names)
    if power:
        L.power(s, bar, names)
    L.kit(s, bar, crash=crash, until=until)
    if fill:
        L.fill(s, bar)


def arrangement():
    s = Score()
    # bars 0-1, the cold open: the band without its lead, the kick alone,
    # then the snare and the crash
    L.arp(s, 0, CHORDS[0], vel=-10)
    L.bass(s, 0, CHORDS[0], vel=-20)
    L.pad(s, 0, CHORDS[0], vel=70)
    L.kit(s, 0, snare=False)
    band(s, 1, crash=True, fill=True)
    # bars 2-7: the hook (the title, BN4 joins)
    for bar in range(2, 8):
        band(s, bar, crash=bar in (2, 4, 6), fill=bar == 7)
        L.lead(s, bar, LINES[bar], side=-1 if bar % 2 == 0 else 1)
    # bars 8-10: the B (its viruses), held notes climbing
    for bar in range(8, 11):
        band(s, bar, crash=bar == 8, fill=bar == 10, vel=6)
        L.lead(s, bar, LINES[bar], side=1)
    # bars 11-14: the anthem (the tournament), the guitar out
    for bar in range(11, 15):
        names = CHORDS[bar]
        fill = bar == 14
        until = 12 if fill else 16
        L.arp(s, bar, names, vel=-6)
        L.bass(s, bar, names, until=until)
        L.pad(s, bar, names, vel=88)
        L.kit(s, bar, crash=bar == 11, until=until)
        if fill:
            L.fill(s, bar)
        L.lead(s, bar, LINES[bar], vel=108, side=-1)
    # bars 15-18: the hook at full strength (BN4's Navis)
    for bar in range(15, 19):
        band(s, bar, crash=bar in (15, 17), fill=bar == 18, vel=6)
        L.lead(s, bar, LINES[bar], vel=118, side=-1 if bar % 2 else 1)
    # bar 19, the drop: the arpeggio and the strings alone, the hats rising
    L.arp(s, 19, CHORDS[19], vel=-8)
    L.pad(s, 19, CHORDS[19], vel=80)
    for p in range(16):
        s.note('noise', 19, 6 * p, 3, 0, 24 + 4 * p, **L.V.HAT)
    # bar 20, the hook again, into the stinger
    band(s, 20, crash=True, fill=True)
    L.lead(s, 20, LINES[20], vel=116)
    # bar 21, the stinger: A5 ringing
    L.ring(s, 21, 'A5', 'E5')
    # bars 22-24, the end card: the arpeggio alone, softly; the strings'
    # last chord
    for bar in (22, 23):
        L.arp(s, bar, CHORDS[bar], vel=-24)
    L.arp(s, 24, CHORDS[24], vel=-32, until=8)
    s.hit('pad_m', 24, 0, 150, 'A3', 70, -0.35)
    return s
