"""Sketch C for 0.22.0, "Wide ways": nine bars for the owner's ear, not a
cut. A tune of its own on the house band (house_music.py: 0.17.0's, in the
manner of BN5's "Virus Busting", song 0x15; its habits in
sketch_bust_lib.py), none of 0x15's notes nor 0.17.0's:
the hook holds its first note thirty ticks (0x15's long notes are 18 and
30), an anthem over the same drive: the calmest lead, the band doing the
running.

    python3 tools/trailer_music.py OUT.wav --piece sketch_house_c
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailers import house_music as B  # noqa: E402
from trailers.house_music import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL, HARDWARE, MIX,  # noqa: E402,F401
                                  REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL, instruments)

# a bar is 96 ticks; (tick, ticks, note)
TUNE = {
    'hook': [(0, 30, 'F5'), (30, 6, 'Eb5'), (36, 12, 'F5'), (48, 18, 'Bb5'), (66, 6, 'Bb5'), (72, 18, 'G5'),
             (90, 6, 'G5')],
    'answer': [(0, 30, 'Ab5'), (30, 6, 'G5'), (36, 12, 'Eb5'), (48, 18, 'F5'), (66, 6, 'F5'), (72, 12, 'Ab5'),
               (84, 6, 'Ab5'), (90, 6, 'C6')],
    'answer2': [(0, 18, 'C6'), (18, 6, 'C6'), (24, 24, 'Ab5'), (48, 30, 'F5'), (78, 6, 'Eb5'), (84, 6, 'Eb5'),
                (90, 6, 'F5')],
    'cadence': [(0, 30, 'F#5'), (30, 6, 'D#5'), (36, 12, 'F#5'), (48, 36, 'G5'), (84, 6, 'G5'), (90, 6, 'G5')],
    'climb': [(0, 18, 'Ab5'), (18, 6, 'Ab5'), (24, 24, 'F5'), (48, 18, 'Bb5'), (66, 6, 'Bb5'), (72, 24, 'F5')],
    'climb2': [(0, 18, 'C6'), (18, 6, 'C6'), (24, 24, 'Ab5'), (48, 18, 'G5'), (66, 30, 'E5')],
    'pickup': [(78, 6, 'C5'), (84, 6, 'Eb5'), (90, 6, 'F5')],
    'top': 'F5',
}

BARS = B.bars(B.SKETCH)
BANNER = (2 * 4 * 60 / BPM, 2.9)


def arrangement():
    return B.sketch(TUNE)
