"""0.22.0's music: a tune of its own on the house band (house_music.py),
the first piece written to the blueprint (.claude/skills/trailer/
blueprint.md, The music). The band is 0.17.0's, the piece and the pace
the owner named as the reference: original, in the manner of BN5's "Virus
Busting" (BN5 Team Colonel's song 0x15), none of its notes nor its chords
in their order; its habits, read from the ROM, are in
sketch_bust_lib.py's docstring, and what the band does on each of the
cut's slots in house_music.py's.

The tune is one of three sketches for the owner's ear (sketch_house_a,
"Carry on"; _b, "In step"; _c, "Wide ways"); `SKETCH` names the one grown
here, and his pick is that one line and a render.

FORM is the cut's slots in bars, which v0_22_0.py reads: the cold open,
the title, Dad's hello, five segments (a section card and its shots),
the recap, Dad's thanks, the stinger and the end card.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailers import house_music as B  # noqa: E402
from trailers import sketch_house_a as SKETCH  # noqa: E402  (the owner's pick goes here)
from trailers.house_music import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL, HARDWARE, MIX,  # noqa: E402,F401
                                  REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL, instruments)

SEGMENTS = (5, 6, 4, 4, 6)                   # each segment's bars, its section card the first
FORM = B.form(SEGMENTS)
BARS = B.bars(FORM)
TUNE = SKETCH.TUNE
# (the 3DS HOME Menu's banner: the climax's hook)
BANNER = (B.first(FORM, 'segment', len(SEGMENTS) - 1) * 4 * 60 / BPM, 2.9)
BANNER_AUDIO = '3ds/banner-0.22.0.wav'


def arrangement():
    return B.arrangement(FORM, TUNE)
