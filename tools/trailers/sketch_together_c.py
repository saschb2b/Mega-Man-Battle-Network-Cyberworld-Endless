"""Sketch C for 0.24.0, "Side by side": nine bars for the owner's ear, not a
cut. A tune of its own on the house band (house_music.py: 0.17.0's, in the
manner of BN5's "Virus Busting", song 0x15; its habits in
sketch_bust_lib.py), none of 0x15's notes nor 0.17.0's nor 0.22.0's:
the hook strikes its first note twice and kicks off the off-beat, a
groove more than a song, its answer the same rhythm a step lower, two
MegaMen in step: the one that dances, for the colours and the crew.

    python3 tools/trailer_music.py OUT.wav --piece sketch_together_c
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailers import house_music as B  # noqa: E402
from trailers.house_music import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL, HARDWARE, MIX,  # noqa: E402,F401
                                  REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL, instruments)

# a bar is 96 ticks; (tick, ticks, note)
TUNE = {
    'hook': [(0, 6, 'Ab5'), (6, 6, 'Ab5'), (12, 12, 'F5'), (24, 6, 'Ab5'), (30, 6, 'Bb5'), (36, 12, 'Db6'), (48, 12, 'Bb5'), (60, 12, 'G5'), (72, 12, 'Bb5'), (84, 12, 'Eb5')],
    'answer': [(0, 6, 'G5'), (6, 6, 'G5'), (12, 12, 'Eb5'), (24, 6, 'G5'), (30, 6, 'Ab5'), (36, 12, 'C6'), (48, 12, 'Ab5'), (60, 12, 'F5'), (72, 24, 'Ab5')],
    'answer2': [(0, 12, 'C6'), (12, 12, 'G5'), (24, 24, 'Eb5'), (48, 12, 'F5'), (60, 12, 'Ab5'), (72, 24, 'C6')],
    'cadence': [(0, 6, 'A#5'), (6, 6, 'A#5'), (12, 12, 'F#5'), (24, 6, 'A#5'), (30, 6, 'B5'), (36, 12, 'F#5'), (48, 36, 'G5'), (84, 12, 'D5')],
    'climb': [(0, 12, 'C6'), (12, 12, 'Ab5'), (24, 12, 'F5'), (36, 12, 'Ab5'), (48, 24, 'Bb5'), (72, 12, 'F5'), (84, 12, 'Bb5')],
    'climb2': [(0, 12, 'Ab5'), (12, 12, 'C6'), (24, 24, 'Eb5'), (48, 12, 'E5'), (60, 12, 'G5'), (72, 24, 'C6')],
    'pickup': [(78, 6, 'Eb5'), (84, 6, 'F5'), (90, 6, 'G5')],
    'top': 'C6',
}

BARS = B.bars(B.SKETCH)
BANNER = (2 * 4 * 60 / BPM, 2.9)


def arrangement():
    return B.sketch(TUNE)
