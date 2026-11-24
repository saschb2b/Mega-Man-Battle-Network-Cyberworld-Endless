"""Sketch A for 0.24.0, "Crew up": nine bars for the owner's ear, not a
cut. A tune of its own on the house band (house_music.py: 0.17.0's, in the
manner of BN5's "Virus Busting", song 0x15; its habits in
sketch_bust_lib.py), none of 0x15's notes nor 0.17.0's nor 0.22.0's:
the hook leaps up in the stab's own 3+3+2 (F, A-flat, B-flat) and
answers itself on C6 a beat later, two divers calling to each other; its
answer comes down by the chord's thirds: the one for a release about
diving with friends.

    python3 tools/trailer_music.py OUT.wav --piece sketch_together_a
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailers import house_music as B  # noqa: E402
from trailers.house_music import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL, HARDWARE, MIX,  # noqa: E402,F401
                                  REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL, instruments)

# a bar is 96 ticks; (tick, ticks, note)
TUNE = {
    'hook': [(0, 18, 'F5'), (18, 18, 'Ab5'), (36, 12, 'Bb5'), (48, 12, 'C6'), (60, 6, 'Bb5'), (66, 6, 'C6'), (72, 18, 'G5'), (90, 6, 'Eb5')],
    'answer': [(0, 12, 'C6'), (12, 12, 'Ab5'), (24, 12, 'Eb5'), (36, 6, 'G5'), (42, 6, 'Ab5'), (48, 24, 'F5'), (72, 12, 'Ab5'), (84, 12, 'C6')],
    'answer2': [(0, 6, 'Eb5'), (6, 6, 'G5'), (12, 12, 'C6'), (24, 24, 'Bb5'), (48, 12, 'Ab5'), (60, 12, 'F5'), (72, 24, 'Db5')],
    'cadence': [(0, 18, 'D#5'), (18, 18, 'F#5'), (36, 12, 'A#5'), (48, 30, 'G5'), (78, 6, 'D5'), (84, 12, 'G5')],
    'climb': [(0, 12, 'Ab5'), (12, 12, 'F5'), (24, 12, 'Ab5'), (36, 12, 'C6'), (48, 18, 'Bb5'), (66, 6, 'F5'), (72, 24, 'Bb5')],
    'climb2': [(0, 18, 'C6'), (18, 6, 'Ab5'), (24, 24, 'F5'), (48, 12, 'G5'), (60, 12, 'E5'), (72, 24, 'C5')],
    'pickup': [(78, 6, 'C5'), (84, 6, 'Db5'), (90, 6, 'Eb5')],
    'top': 'C6',
}

BARS = B.bars(B.SKETCH)
BANNER = (2 * 4 * 60 / BPM, 2.9)


def arrangement():
    return B.sketch(TUNE)
