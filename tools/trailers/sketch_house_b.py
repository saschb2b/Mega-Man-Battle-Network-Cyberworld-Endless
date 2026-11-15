"""Sketch B for 0.22.0, "In step": nine bars for the owner's ear, not a
cut. A tune of its own on the house band (house_music.py: 0.17.0's, in the
manner of BN5's "Virus Busting", song 0x15; its habits in
sketch_bust_lib.py), none of 0x15's notes nor 0.17.0's:
the hook strikes each note twice before it moves (0x15 repeats four notes in
ten), two voices keeping step, its answer the same figure a third up: the
nearest of the three to 0x15's own lead.

    python3 tools/trailer_music.py OUT.wav --piece sketch_house_b
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailers import house_music as B  # noqa: E402
from trailers.house_music import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL, HARDWARE, MIX,  # noqa: E402,F401
                                  REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL, instruments)

# a bar is 96 ticks; (tick, ticks, note)
TUNE = {
    'hook': [(0, 6, 'F5'), (6, 6, 'F5'), (12, 12, 'Ab5'), (24, 6, 'F5'), (30, 6, 'F5'), (36, 12, 'Bb5'),
             (48, 18, 'G5'), (66, 6, 'G5'), (72, 12, 'Eb5'), (84, 6, 'F5'), (90, 6, 'G5')],
    'answer': [(0, 6, 'Ab5'), (6, 6, 'Ab5'), (12, 12, 'C6'), (24, 6, 'Ab5'), (30, 6, 'Ab5'), (36, 12, 'Eb5'),
               (48, 18, 'F5'), (66, 6, 'F5'), (72, 12, 'Ab5'), (84, 12, 'F5')],
    'answer2': [(0, 12, 'Eb5'), (12, 6, 'G5'), (18, 6, 'Ab5'), (24, 24, 'C6'), (48, 6, 'Ab5'), (54, 6, 'Ab5'),
                (60, 12, 'F5'), (72, 18, 'Eb5'), (90, 6, 'Eb5')],
    'cadence': [(0, 6, 'F#5'), (6, 6, 'F#5'), (12, 12, 'A#5'), (24, 6, 'F#5'), (30, 6, 'F#5'), (36, 12, 'D#5'),
                (48, 36, 'D5'), (84, 6, 'D5'), (90, 6, 'G5')],
    'climb': [(0, 6, 'Ab5'), (6, 6, 'Ab5'), (12, 12, 'C6'), (24, 24, 'Ab5'), (48, 6, 'Bb5'), (54, 6, 'Bb5'),
              (60, 12, 'F5'), (72, 24, 'Bb5')],
    'climb2': [(0, 6, 'C6'), (6, 6, 'C6'), (12, 12, 'Ab5'), (24, 24, 'F5'), (48, 12, 'G5'), (60, 6, 'G5'),
               (66, 30, 'E5')],
    'pickup': [(78, 6, 'C5'), (84, 6, 'Eb5'), (90, 6, 'Eb5')],
    'top': 'C5',
}

BARS = B.bars(B.SKETCH)
BANNER = (2 * 4 * 60 / BPM, 2.9)


def arrangement():
    return B.sketch(TUNE)
