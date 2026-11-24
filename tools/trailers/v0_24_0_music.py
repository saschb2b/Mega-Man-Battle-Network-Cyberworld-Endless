"""0.24.0's music: a tune of its own on the house band (house_music.py),
the second piece written to the blueprint (.claude/skills/trailer/
blueprint.md, The music). The band is 0.17.0's: original, in the manner of
BN5's "Virus Busting" (BN5 Team Colonel's song 0x15), none of its notes
nor its chords in their order; its habits, read from the ROM, are in
sketch_bust_lib.py's docstring, and what the band does on each of the
cut's slots in house_music.py's.

The owner's direction (24 November 2026): open in the tone of 0.22.0's
trailer, then go with the cut. So the cold open and the title are the
house band's as 0.22.0 had them (the stab under rising hats, the hook and
its answer), the tune another; and on the band's slots, read with this
cut:
- Dad's hello is six bars here, four boxes; its third box is where the
  picture pulls back from one game to four, and the band lifts under it
  (`widen`: a sweep up through the bar before, the crash on the bar the
  picture widens and the snare's backbeat coming in softly from there),
  still without the lead or the stab, so his box stays the line.
- The crew (the first segment) is the band's first kind, the hook and its
  cadence into the climb; the colours (the second) the hook on a new chord
  and its second answer, the groove; the forms (the third) the break, the
  drums and the stab alone under the transformations, then the climb; the
  party's guardian (the last) the climax, the saw under the hook, its
  cadence on the win; the recap's climb is the release.

The tune is one of three sketches for the owner's ear (sketch_together_a,
"Crew up"; _b, "Call from the lab"; _c, "Side by side"); `SKETCH` names
the one grown here, and his pick is that one line and a render.

FORM is the cut's slots in bars, which v0_24_0.py reads: the cold open,
the title, Dad's hello, four segments (a section card and its shots), the
recap, Dad's thanks, the stinger and the end card.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailers import house_music as B  # noqa: E402
from trailers import sketch_bust_lib as L  # noqa: E402
from trailers import sketch_together_a as SKETCH  # noqa: E402  (the owner's pick goes here)
from trailers.house_music import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL, HARDWARE, MIX,  # noqa: E402,F401
                                  REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL, instruments)

SEGMENTS = (5, 6, 5, 5)                      # each segment's bars, its section card the first
HELLO = 6                                    # four boxes: who he is, the game, friends, the hand-over
WIDEN = 3                                    # the hello's bar the picture widens on (from one game to four)
FORM = B.form(SEGMENTS, hello=HELLO)
BARS = B.bars(FORM)
TUNE = SKETCH.TUNE
# (the 3DS HOME Menu's banner: the climax's hook)
BANNER = (B.first(FORM, 'segment', len(SEGMENTS) - 1) * 4 * 60 / BPM, 2.9)
BANNER_AUDIO = '3ds/banner-0.24.0.wav'


def widen(s, bar):
    """the hello's lift as the picture pulls back: a sweep up through the
    bar before, then the crash, and the snare's backbeat soft for the rest
    of the hello (its last bar lifts as the band's own)"""
    s.note('fx', bar - 1, 0, 92, 0, 84, sweep=(900, 90000), ramp=(1, 8))
    s.note('fx', bar, 0, 48, 0, 104, **L.V.CRASH)
    for b in range(bar, B.first(FORM, 'hello') + HELLO - 1):
        for p in (4, 12):
            s.hit('snare', b, 6 * p, 6, vel=70, pan=0.05)


def arrangement():
    s = B.arrangement(FORM, TUNE)
    widen(s, B.first(FORM, 'hello') + WIDEN)
    return s
