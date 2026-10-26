"""What 0.16.0's three sketches share (sketch_vb_a, _b and _c; not a piece
of its own): the approved palette, 0.14.0's and 0.15.0's, FluidR3 GM's
instruments (Frank Wen, the MIT license; Debian's fluid-soundfont-gm in
docker/Dockerfile.music's image, never in git) cut into 10512 Hz 8-bit
samples by tools/trailer_soundfont.py: the electronic kit's kick, the
snare, the clap over a snare, the studio toms, the low conga, the
strings' chords (major, minor, suspended, diminished), the distortion and
palm-muted guitars' power chords, the brass section, the saw lead.
Rendered with tools/trailer_music.py's render_any, in that image."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_soundfont as sf  # noqa: E402
from trailer_music import key  # noqa: E402

SOUNDFONT = sf.SOUNDFONT
DS_LEVEL = 1.0
REVERB = 40
HARDWARE = 32768
MIX = (('lead', ('sq1',), 0.11), ('arp', ('sq2',), 0.15), ('bass', ('wave',), 0.17),
       ('hats', ('noise',), 0.12), ('fx', ('fx',), 0.08))
CHORD_SAMPLES = {'pad': (0, 2, 7, 12, 14), 'pad_m': (0, 3, 7, 12, 15), 'pad_M': (0, 4, 7, 12, 16),
                 'pad_d': (0, 3, 6, 12, 15), 'brass': (0, 4, 7, 12)}
DS_ENV = {'kick': (255, 250, 190, 208), 'snare': (255, 250, 190, 208), 'clap': (255, 250, 190, 208),
          'tom_hi': (255, 0, 255, 230), 'tom_mid': (255, 0, 255, 230), 'tom_lo': (255, 0, 255, 230),
          'hand': (255, 0, 255, 220), 'gtr': (255, 0, 255, 188), 'mute': (255, 0, 255, 160),
          'pad': (60, 0, 235, 210), 'pad_m': (60, 0, 235, 210), 'pad_M': (60, 0, 235, 210), 'pad_d': (60, 0, 235, 210),
          'dlead': (255, 0, 230, 200), 'double': (255, 0, 230, 200), 'brass': (255, 0, 230, 180)}
DS_GAIN = {'kick': 0.9, 'snare': 0.7, 'clap': 0.72, 'tom_hi': 0.6, 'tom_mid': 0.6, 'tom_lo': 0.65, 'hand': 0.42,
           'gtr': 0.55, 'mute': 0.45, 'pad': 0.78, 'pad_m': 0.78, 'pad_M': 0.78, 'pad_d': 0.78, 'dlead': 0.3, 'double': 0.26,
           'brass': 0.48}
SAMPLED_LEADS = ('dlead',)

HAT = dict(clock=262144, env=(0, 0, 11, 0), pan=0.2)
OPEN = dict(clock=262144, env=(0, 2, 11, 3), pan=0.25)
CRASH = dict(clock=80000, env=(0, 7, 0, 1), pan=-0.15)
SAW = [15 - i // 2 for i in range(32)]                    # the wave's table: a saw
SQUARE = [15] * 13 + [8] + [0] * 17 + [8]                 # ... or a square's


def instruments(low='C#2'):
    """the samples; `low` the power chords' root as recorded"""
    g = key(low)
    S = {
        'kick': sf.drum(36, 0.14, program=24, drive=2.4),
        'snare': sf.drum(40, 0.12, layer=(38,), drive=1.8),
        'clap': sf.drum(39, 0.09, layer=(40,), drive=2.0),
        'tom_hi': sf.drum(50, 0.35),
        'tom_mid': sf.drum(47, 0.35),
        'tom_lo': sf.drum(45, 0.38),
        'hand': sf.drum(64, 0.3),
        'gtr': sf.instrument(30, [g, g + 7, g + 12], 0.34, gains=[1.0, 0.85, 0.7]),
        'mute': sf.instrument(28, [g, g + 7, g + 12], 0.12, gains=[1.0, 0.85, 0.7]),
        'dlead': sf.instrument(81, [72], 1.0, fade=0.1),
        'brass': sf.instrument(61, [60 + i for i in CHORD_SAMPLES['brass']], 0.5, base=60, fade=0.12),
    }
    S['hand'].base = 60
    # (the saw doubling a square, under a name of its own: music_check.py
    # reads dlead alone as the sampled lead)
    S['double'] = S['dlead']
    for name in ('pad', 'pad_m', 'pad_M', 'pad_d'):
        S[name] = sf.instrument(48, [60 + i for i in CHORD_SAMPLES[name]], 2.0, base=60, fade=0.3)
    return S


def halves(names):
    if len(names) == 1:
        return [(0, 96, names[0])]
    if len(names) == 2:
        return [(0, 48, names[0]), (48, 48, names[1])]
    return [(0, 48, names[0]), (48, 24, names[1]), (72, 24, names[2])]


def low(k, floor='E2'):
    k = key(k)
    while k > key(floor):
        k -= 12
    return k


def power(s, bar, t, n, root, vel=112, fall=False, pan=0.0):
    """the distortion guitar's power chord, falling at a phrase's end"""
    s.hit('gtr', bar, t, n, low(root) + 12, vel, pan, bend=[(0, 0), (n - 10, 0), (n, -8)] if fall else None)


def chug(s, bar, t, root, vel=96):
    s.hit('mute', bar, t, 5, low(root) + 12, vel, 0.0)


def toms(s, bar, t0=72, vel=112):
    for t, tom, pan in ((t0, 'tom_hi', -0.3), (t0 + 6, 'tom_hi', -0.3), (t0 + 12, 'tom_mid', 0.0),
                        (t0 + 18, 'tom_lo', 0.3)):
        s.hit(tom, bar, t, 6, vel=vel, pan=pan)


def hats(s, bar, opens=(), levels=(96, 48, 72, 48), until=16, every=1):
    for p in range(0, until, every):
        if p in opens:
            s.note('noise', bar, 6 * p, 6, 0, 84, **OPEN)
        else:
            s.note('noise', bar, 6 * p, 3, 0, levels[p % 4], **HAT)
