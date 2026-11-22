"""Sketch A, the battle drive, with sampled instruments: the same eight
bars and the same Game Boy channels (the squares, the wave's bass and the
noise's hats and crash, tools/trailer_music.py), its Direct Sound played
from FluidR3 GM (Frank Wen, the MIT license; Debian's fluid-soundfont-gm,
installed by docker/Dockerfile.music, never in git) as a Game Boy Advance
game's composer would have cut a synthesizer's sounds: a note or a chord
as one sample, 10512 Hz and 8 bits, cut short, played by MP2K's model
(nearest-sample pitch, its envelopes, the 13379 Hz mix and its echo).

    docker run --rm -v "$PWD:/src" -w /src/tools cyberworld-music \\
        python3 trailer_music.py OUT.wav --piece sketch_a_sf

Instead of the kit's synthesized ones: the GM kit's electronic kick and a
clap layered on a snare, both driven into the top as BN6's are; the
studio toms; a distortion guitar's power chord (G, D, G as one sample,
with its dives) and the palm-muted guitar's for the chugs; string
ensemble chords as the chord samples (one sus2, one minor, one major
triad, recoloured by the bass as before); the orchestra hit on the
downbeats; a sawtooth lead doubling the square an octave down.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_soundfont as sf  # noqa: E402
from trailer_music import key  # noqa: E402
from trailers import sketch_a as synth  # noqa: E402

BPM, BARS, TAIL, REVERB, HARDWARE = synth.BPM, synth.BARS, synth.TAIL, synth.REVERB, synth.HARDWARE
MIX, DS_LEVEL, BANNER, CHORD_SAMPLES = synth.MIX, synth.DS_LEVEL, synth.BANNER, synth.CHORD_SAMPLES
SOUNDFONT = sf.SOUNDFONT
arrangement = synth.arrangement

DS_ENV = dict(synth.DS_ENV, pad=(60, 0, 235, 210), pad_m=(60, 0, 235, 210), pad_M=(60, 0, 235, 210),
              impact=(255, 0, 255, 215), dlead=(255, 0, 230, 190))
DS_GAIN = dict(synth.DS_GAIN, kick=0.95, clap=0.75, gtr=0.55, mute=0.42, pad=0.75, pad_m=0.75, pad_M=0.75,
               impact=0.55, dlead=0.14)


def instruments():
    G2 = key('G2')
    S = {
        'kick': sf.drum(36, 0.14, program=24, drive=2.4),
        'clap': sf.drum(39, 0.09, layer=(40,), drive=2.0),
        'tom_hi': sf.drum(50, 0.35),
        'tom_mid': sf.drum(47, 0.35),
        'tom_lo': sf.drum(45, 0.38),
        'gtr': sf.instrument(30, [G2, G2 + 7, G2 + 12], 0.34, gains=[1.0, 0.85, 0.7]),
        'mute': sf.instrument(28, [G2, G2 + 7, G2 + 12], 0.12, gains=[1.0, 0.85, 0.7]),
        'impact': sf.instrument(55, [key('G3')], 0.45),
        'dlead': sf.instrument(81, [72], 1.0, fade=0.1),
    }
    S['impact'].base = None
    for name, intervals in CHORD_SAMPLES.items():
        S[name] = sf.instrument(48, [60 + i for i in intervals], 2.0, base=60, fade=0.3)
    return S
