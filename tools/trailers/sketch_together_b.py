"""Sketch B for 0.24.0, "Call from the lab": nine bars for the owner's ear, not a
cut. A tune of its own on the house band (house_music.py: 0.17.0's, in the
manner of BN5's "Virus Busting", song 0x15; its habits in
sketch_bust_lib.py), none of 0x15's notes nor 0.17.0's nor 0.22.0's:
the hook falls from D-flat 6 to E-flat in two breaths, the
way a call comes in; the answer rises back to C6, and the pickup climbs
into the hook's top note: the most singable of the three.

    python3 tools/trailer_music.py OUT.wav --piece sketch_together_b
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailers import house_music as B  # noqa: E402
from trailers.house_music import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL, HARDWARE, MIX,  # noqa: E402,F401
                                  REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL, instruments)

# a bar is 96 ticks; (tick, ticks, note)
TUNE = {
    'hook': [(0, 12, 'Db6'), (12, 12, 'Bb5'), (24, 6, 'Ab5'), (30, 6, 'F5'), (36, 12, 'Ab5'), (48, 18, 'Bb5'), (66, 6, 'G5'), (72, 24, 'Eb5')],
    'answer': [(0, 18, 'C6'), (18, 6, 'Bb5'), (24, 12, 'G5'), (36, 12, 'Eb5'), (48, 12, 'Ab5'), (60, 12, 'F5'), (72, 12, 'Ab5'), (84, 12, 'C6')],
    'answer2': [(0, 12, 'G5'), (12, 12, 'Ab5'), (24, 12, 'C6'), (36, 12, 'Eb5'), (48, 24, 'F5'), (72, 12, 'Db5'), (84, 12, 'F5')],
    'cadence': [(0, 12, 'F#5'), (12, 12, 'D#5'), (24, 12, 'F#5'), (36, 12, 'A#5'), (48, 24, 'G5'), (72, 12, 'D5'), (84, 12, 'C5')],
    'climb': [(0, 12, 'F5'), (12, 12, 'Ab5'), (24, 12, 'C6'), (36, 12, 'Ab5'), (48, 12, 'Bb5'), (60, 12, 'F5'), (72, 24, 'Eb5')],
    'climb2': [(0, 12, 'Eb5'), (12, 12, 'F5'), (24, 12, 'Ab5'), (36, 12, 'C6'), (48, 24, 'G5'), (72, 24, 'E5')],
    'pickup': [(78, 6, 'Ab5'), (84, 6, 'Bb5'), (90, 6, 'C6')],
    'top': 'Ab5',
}

BARS = B.bars(B.SKETCH)
BANNER = (2 * 4 * 60 / BPM, 2.9)


def arrangement():
    return B.sketch(TUNE)
