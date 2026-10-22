"""0.14.0's trailer music: an original battle theme in the manner of
Mega Man Battle Network 6's, grown from sketch A (tools/trailers/
sketch_a.py), the one the owner chose by ear, and written by
.claude/skills/trailer/music_rules.md. None of Capcom's notes, chords in
their order or samples.

The habits, read from BN6's songs with song_habits.py --detail --theory
(the boss battle 0x16, "Surge of Power!", the owner's reference, and the
virus battle 0x15), numbers only:
- 150 beats a minute, a minor key in its Aeolian and Dorian colours: i,
  bVII, bVI, bIII, the Dorian IV, the minor v, a bIImaj7; two chords a bar;
  sevenths and suspensions on most of them, made by a triad sample over
  another bass.
- The two squares: one the lead (50% duty, vibrato after 14 ticks, long
  notes pushed across the bar line), the other a 32nd-note arpeggio, each
  sixteenth struck loud then soft (velocities 100 and 52), thrown hard
  left and right note by note (0x16's: 212 pan changes between 0 and 127).
- The wave's bass in eighths and sixteenths on the kick's places, legato,
  root and octave, falling at the phrase ends (bends of range 12).
- The kit: short samples driven into the top (BN6's kick is 139 ms, half
  its samples at full scale; its clap 82 ms), the kick syncopated (0x16:
  sixteenths 0, 6, 7, 10, 14 of the bar), the clap on two and four with
  ghosts, hats in sixteenths at three levels on the noise channel, an
  open hat off the beat, the crash at the phrase heads, toms into a
  section.
- Direct Sound: a low distorted stab (a guitar's power chord) with bends,
  chord samples, a hit on the downbeats, a lead's double.

Here, in G minor at 150 (a bar is 48 frames of the trailer's 30), 20 bars:
- 0-3 the intro: the hook's arpeggio alone over i bVII | bVI IV, the hats
  coming in under a riser; then the band entering at bar 2, half-time,
  the bass held, the chugs muted; bar 3 in full, toms and the lead's
  pickup.
- 4-11 the main section, sketch A's eight bars: the hook (two hammered
  G's, a leap of a fifth, a turn back, a D held with vibrato that pushes
  across the bar line) over i bVII; its answer over bVI and the Dorian IV
  (its E natural); the hook again; a cadence onto the minor v; a climb in
  sequence over i bIII | bVI bVII; the hook on the fake-out bIImaj7; the
  turnaround IVsus4-IV Vsus4-V.
- 12-13 the drop: i hit, then one square alone, thin (12.5%), the hook
  slowed to half its speed over the bass's one G; bar 13 climbs back over
  bVI and bVII, the arpeggio rising, a clap roll and a riser, the pickup
  in octaves.
- 14-17 the climax: the hook in octaves on both squares, everything in:
  brass stabs, the arpeggio moved to a sampled lead, the guitar, the
  chords; the last bar the hook on bII.
- 18-19 the logo and the end card: the turnaround in octaves, then i,
  struck and ringing, the power chord and the bass diving, the lead's G
  falling away.

Its BANNER, the 3DS HOME Menu's sound while it is the current trailer: the
climax's first bar and a half, the hook in octaves.

Two sounds, the owner's pick between them (SOUND, or CW_TRAILER_SOUND):
'soundfont' plays the Direct Sound parts from FluidR3 GM (Frank Wen, the
MIT license; Debian's fluid-soundfont-gm in docker/Dockerfile.music's
image, never in git; tools/trailer_soundfont.py cuts its notes and chords
into 10512 Hz 8-bit samples for the kit's MP2K model); 'synth' plays the
kit's own synthesized ones. The squares, the wave and the noise are the
Game Boy's own channels in both. With the soundfont the piece renders in
the image (tools/trailer_music.py's render_any does it from the host) and
its banner is kept as our own audio in BANNER_AUDIO, written once by
`python3 tools/trailer_music.py --write-banner --trailer 0.14.0`.

    python3 tools/trailer_music.py OUT.wav --piece v0_14_0_music
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_music as tm  # noqa: E402
import trailer_soundfont as sf  # noqa: E402
from trailer_music import Score, key, keys  # noqa: E402
from trailers import v0_3_0_music as first  # noqa: E402

SOUND = os.environ.get('CW_TRAILER_SOUND', 'soundfont')
SOUNDFONT = sf.SOUNDFONT if SOUND == 'soundfont' else None

BPM = 150
BARS = 20
TAIL = 2.0
MIX = (('lead', ('sq1',), 0.11), ('arp', ('sq2',), 0.17), ('bass', ('wave',), 0.17),
       ('hats', ('noise',), 0.14), ('fx', ('fx',), 0.08))
DS_LEVEL = 1.0
REVERB = 40
HARDWARE = 32768
CHORD_SAMPLES = {'pad': (0, 2, 7, 12, 14), 'pad_m': (0, 3, 7, 12, 15), 'pad_M': (0, 4, 7, 12, 16),
                 'brass': (0, 4, 7, 12)}
BANNER = (14 * 4 * 60 / BPM, 2.9)      # (from bar 14's downbeat, seconds)
BANNER_AUDIO = '3ds/banner-0.14.0.wav' if SOUNDFONT else None

ENV = {'kick': (255, 250, 190, 208), 'clap': (255, 250, 190, 208), 'tom_hi': (255, 0, 255, 230),
       'tom_mid': (255, 0, 255, 230), 'tom_lo': (255, 0, 255, 230), 'gtr': (255, 0, 255, 188),
       'mute': (255, 0, 255, 160), 'impact': (255, 0, 255, 220), 'dlead': (255, 0, 224, 188),
       'brass': (255, 0, 230, 190)}
if SOUNDFONT:
    DS_ENV = dict(ENV, pad=(60, 0, 235, 210), pad_m=(60, 0, 235, 210), pad_M=(60, 0, 235, 210),
                  impact=(255, 0, 255, 215), dlead=(255, 0, 230, 190))
    DS_GAIN = {'kick': 0.95, 'clap': 0.75, 'tom_hi': 0.6, 'tom_mid': 0.6, 'tom_lo': 0.65, 'gtr': 0.55, 'mute': 0.42,
               'pad': 0.75, 'pad_m': 0.75, 'pad_M': 0.75, 'impact': 0.55, 'dlead': 0.14, 'brass': 0.45}
else:
    DS_ENV = dict(ENV, pad=(40, 0, 230, 200), pad_m=(40, 0, 230, 200), pad_M=(40, 0, 230, 200))
    DS_GAIN = {'kick': 1.0, 'clap': 0.8, 'tom_hi': 0.6, 'tom_mid': 0.6, 'tom_lo': 0.65, 'gtr': 0.45, 'mute': 0.38,
               'pad': 0.9, 'pad_m': 0.9, 'pad_M': 0.9, 'impact': 0.7, 'dlead': 0.16, 'brass': 0.4}

LEAD = dict(duty=0.5, env=(0, 0, 11, 3), vib=(26, 10), pan=0.0)
UNDER = dict(duty=0.25, env=(0, 0, 10, 3), vib=(22, 12), pan=0.35)    # the climax's octave below
THIN = dict(duty=0.125, env=(0, 0, 12, 4), vib=(34, 8), pan=0.0)      # the drop's square alone
ARP = dict(duty=0.25, env=(0, 7, 3, 1))
SQUARE = [15] * 13 + [8] + [0] * 17 + [8]
BASS = dict(env=(0, 0, 15, 1), wave=SQUARE)
HELD = dict(BASS, env=(0, 0, 15, 3))
HAT = dict(clock=262144, env=(0, 0, 11, 0), pan=0.25)
OPEN = dict(clock=262144, env=(0, 2, 10, 3), pan=0.3)
CRASH = dict(clock=80000, env=(0, 7, 0, 1), pan=-0.2)


def instruments():
    """The Direct Sound samples, the soundfont's or the kit's own."""
    if SOUNDFONT:
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
            'brass': sf.instrument(61, [60 + i for i in CHORD_SAMPLES['brass']], 0.5, base=60, fade=0.12),
        }
        S['impact'].base = None
        for name in ('pad', 'pad_m', 'pad_M'):
            S[name] = sf.instrument(48, [60 + i for i in CHORD_SAMPLES[name]], 2.0, base=60, fade=0.3)
        return S
    kit = first.instruments()
    S = {name: kit[name] for name in ('tom_hi', 'tom_mid', 'tom_lo')}
    S['kick'] = tm.punch_kick(low=74, high=160, drive=6.0, body=0.07)
    S['clap'] = tm.punch_clap()
    S['gtr'] = tm.power_stab(root=key('G2'))
    S['mute'] = tm.power_stab(root=key('G2'), seconds=0.12, mute=True)
    for name in ('pad', 'pad_m', 'pad_M'):
        S[name] = tm.chord_pad(CHORD_SAMPLES[name])
    S['impact'] = tm.impact()
    S['dlead'] = tm.cycle_lead()
    S['brass'] = tm.brass_section(CHORD_SAMPLES['brass'])
    return S


# ---- the harmony: two chords a bar ----
# (bass root, the arpeggio's two octaves, the chord sample and its root,
# the brass's triad): a triad sample a third over the bass makes the
# seventh chord
H = {
    'Gm': ('G2', 'G3 Bb3 D4 F4 G4 Bb4 D5 F5 G5', 'pad_M', 'Bb3', 'Bb3'),     # Bb over G: Gm7
    'F': ('F2', 'F3 A3 C4 F4 G4 A4 C5 F5 G5', 'pad', 'F3', 'F3'),           # Fsus2
    'Eb': ('Eb2', 'Eb3 G3 Bb3 D4 Eb4 G4 Bb4 D5 Eb5', 'pad_m', 'G3', 'Eb4'),  # Gm over Eb: Ebmaj7
    'C': ('C3', 'C3 E3 G3 Bb3 C4 E4 G4 Bb4 C5', 'pad_M', 'C4', 'C4'),       # the Dorian IV, major
    'Dm': ('D2', 'D3 F3 A3 C4 D4 F4 A4 C5 D5', 'pad_M', 'F3', 'F3'),        # F over D: Dm7
    'Bb': ('Bb2', 'Bb2 D3 F3 A3 Bb3 D4 F4 A4 Bb4', 'pad_m', 'D4', 'Bb3'),   # Dm over Bb: Bbmaj7
    'Ab': ('Ab2', 'Ab2 C3 Eb3 G3 Ab3 C4 Eb4 G4 Ab4', 'pad_m', 'C4', 'Ab3'),  # Cm over Ab: Abmaj7
    'Csus': ('C3', 'C3 F3 G3 C4 F4 G4 C5 F5 G5', 'pad', 'F3', 'F3'),        # F G C over C: Csus4
    'Dsus': ('D2', 'D3 G3 A3 D4 G4 A4 D5 G5 A5', 'pad', 'G3', 'G3'),        # G A D over D: Dsus4
    'D': ('D2', 'D3 F#3 A3 C4 D4 F#4 A4 C5 D5', 'pad_M', 'D4', 'D4'),
}
# the main section's eight bars (from bar 4) and the climax's four (from 14)
MAIN = [('Gm', 'F'), ('Eb', 'C'), ('Gm', 'F'), ('Eb', 'Dm'), ('Gm', 'Bb'), ('Eb', 'F'), ('Ab', 'Bb'),
        ('Csus', 'C', 'Dsus', 'D')]
CLIMAX = [('Gm', 'F'), ('Eb', 'C'), ('Gm', 'F'), ('Ab', 'Bb')]

# ---- the hook and its answers: (tick, ticks, note) ----
HOOK = [(0, 6, 'G5'), (6, 6, 'G5'), (12, 12, 'D6'), (24, 6, 'C6'), (30, 6, 'Bb5'), (36, 12, 'C6'), (48, 36, 'D6')]
ANSWER = [(0, 12, 'Eb6'), (12, 6, 'D6'), (18, 18, 'Bb5'), (36, 6, 'G5'), (42, 6, 'Bb5'), (48, 6, 'C6'), (54, 6, 'D6'),
          (60, 30, 'E6')]
LINES = [
    HOOK + [(84, 6, 'C6'), (90, 6, 'D6')],
    ANSWER + [(90, 6, 'D5')],
    HOOK + [(84, 6, 'C6'), (90, 6, 'Bb5')],
    [(0, 18, 'G5'), (18, 6, 'F5'), (24, 12, 'G5'), (36, 12, 'Bb5'), (48, 12, 'A5'), (60, 30, 'D5'), (90, 6, 'C6')],
    [(0, 6, 'D6'), (6, 6, 'D6'), (12, 6, 'C6'), (18, 6, 'D6'), (24, 12, 'F6'), (36, 12, 'D6'), (48, 6, 'F6'),
     (54, 6, 'F6'), (60, 6, 'Eb6'), (66, 6, 'F6'), (72, 24, 'F6')],
    [(0, 6, 'Eb6'), (6, 6, 'Eb6'), (12, 6, 'D6'), (18, 6, 'Eb6'), (24, 12, 'G6'), (36, 12, 'Eb6'), (48, 6, 'F6'),
     (54, 6, 'F6'), (60, 6, 'Eb6'), (66, 6, 'F6'), (72, 18, 'A6'), (90, 6, 'D5')],
    [(0, 6, 'G5'), (6, 6, 'G5'), (12, 12, 'Eb6'), (24, 6, 'C6'), (30, 6, 'Bb5'), (36, 12, 'C6'), (48, 36, 'D6'),
     (84, 6, 'Eb6'), (90, 6, 'E6')],
    [(0, 12, 'F6'), (12, 12, 'G6'), (24, 12, 'E6'), (36, 12, 'G6'), (48, 12, 'G6'), (60, 12, 'A6'), (72, 12, 'F#6'),
     (84, 12, 'A6')],
]
CLIMAX_LINES = [LINES[0], ANSWER + [(84, 6, 'D5'), (90, 6, 'F5')], LINES[2], LINES[6]]


def lead(s, bar, line, ch='sq1', voice=LEAD, vel=118, octave=0, fall_at=None):
    """A line on a square; the long note at `fall_at` falls away at its end."""
    for t, n, k in line:
        o = dict(voice)
        if t == fall_at and n >= 24:
            o['bend'] = [(0, 0), (n - 8, 0), (n, -2)]
        s.note(ch, bar, t, n, key(k) + octave, vel - (10 if n <= 6 and t % 12 else 0), **o)


def both(s, bar, line, fall_at=None):
    """The climax: the line on both squares, an octave apart."""
    lead(s, bar, line, fall_at=fall_at)
    lead(s, bar, line, ch='sq2', voice=UNDER, vel=110, octave=-12, fall_at=fall_at)


FIGURE = [0, 2, 4, 3, 5, 4, 6, 5]   # three-note turns against the sixteenths


def arp(s, bar, t0, t1, voicing, vel=104, soft=54, figure=FIGURE, lift=0):
    """Each sixteenth struck twice as 32nds, loud then soft, and thrown
    hard left, then hard right."""
    v = keys(voicing)
    for i, t in enumerate(range(t0, t1, 6)):
        k = v[figure[i % len(figure)] % len(v)] + lift
        side = -0.9 if (t // 6) % 2 == 0 else 0.9
        s.note('sq2', bar, t, 3, k, vel, pan=side, **ARP)
        s.note('sq2', bar, t + 3, 3, k, soft, pan=side, **ARP)


def sampled_arp(s, bar, t0, t1, voicing, vel=96):
    """The climax's arpeggio, the squares both on the hook: sixteenths on
    the sampled lead, still thrown left and right."""
    v = keys(voicing)
    for i, t in enumerate(range(t0, t1, 6)):
        side = -0.8 if (t // 6) % 2 == 0 else 0.8
        s.hit('dlead', bar, t, 3, v[FIGURE[i % len(FIGURE)] % len(v)], vel if i % 2 == 0 else vel - 36, side)


KICK = (0, 6, 8, 11, 14)       # sixteenths of the bar
CLAP = (4, 12)
OCTAVES = (3, 9, 13)


def drums(s, bar, until=16, ghost=False, crash=False, open_at=(10,)):
    for p in KICK:
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=120)
    for p in CLAP:
        if p < until:
            s.hit('clap', bar, 6 * p, 6, vel=118, pan=0.05)
    if ghost and until == 16:
        s.hit('clap', bar, 90, 6, vel=60, pan=-0.2)
    for p in range(until):
        if p in open_at:
            s.note('noise', bar, 6 * p, 6, 0, 96, **OPEN)
        else:
            s.note('noise', bar, 6 * p, 3, 0, (112, 40, 76, 40)[p % 4], **HAT)
    if crash:
        s.note('fx', bar, 0, 48, 0, 124, **CRASH)


def bass(s, bar, root, t0=0, t1=96, fall=False):
    """The root on the kick's places, its octave three times between, each
    held nearly to the next; a fall at the phrase's end."""
    r = key(root)
    end = t1 - 12 if fall else t1
    places = [p for p in range(t0 // 6, end // 6) if p in KICK or p in OCTAVES]
    for i, p in enumerate(places):
        nxt = places[i + 1] if i + 1 < len(places) else end // 6
        n = 6 * (nxt - p) - 1
        if p in KICK:
            s.note('wave', bar, 6 * p, n, r, 124, **BASS)
        else:
            s.note('wave', bar, 6 * p, min(n, 5), r + 12, 92, **BASS)
    if fall:
        s.note('wave', bar, t1 - 12, 12, r + 12, 120, **dict(BASS, bend=[(0, 0), (4, 0), (12, -12)]))


def power(root):
    k = key(root) + 12
    while k > key('C3'):
        k -= 12
    return k


def chugs(s, bar, root, t0=0, t1=96, dive=False, muted_only=False):
    """The power chord with the kick, palm-muted on the sixteenths
    between, still on the backbeat."""
    k = power(root)
    for p in range(t0 // 6, (t1 - 12 if dive else t1) // 6):
        if p in KICK and not muted_only:
            s.hit('gtr', bar, 6 * p, 6 if p in (6, 11) else 9, k, 112, -0.6)
        elif p not in CLAP and (p % 2 == 0 or muted_only):
            s.hit('mute', bar, 6 * p, 4, k, 96 if p % 2 == 0 else 70, 0.6)
    if dive:
        s.hit('gtr', bar, t1 - 12, 12, k, 118, -0.35, bend=[(0, 0), (3, 0), (12, -5)])


def chord(s, bar, t0, name, length, vel=100):
    _, _, pad, pad_root, _ = H[name]
    s.hit(pad, bar, t0, length - 2, key(pad_root) + 12, vel, -0.8 if t0 % 96 < 48 else 0.8)


def half(s, bar, t0, name, length=48, dive=False, fall=False, arp_on=True, brass=False):
    root, voicing, _, _, triad = H[name]
    chord(s, bar, t0, name, length)
    if arp_on:
        arp(s, bar, t0, t0 + length, voicing)
    else:
        sampled_arp(s, bar, t0, t0 + length, voicing)
    bass(s, bar, root, t0, t0 + length, fall)
    chugs(s, bar, root, t0, t0 + length, dive)
    if brass:
        for t, n, vel in ((t0, 14, 116), (t0 + 36, 8, 104)):
            if t < t0 + length - 6:
                s.hit('brass', bar, t, n, triad, vel, 0.45 if t % 96 < 48 else -0.45)


def bar_of(s, bar, names, falls=False, arp_on=True, brass=False):
    if len(names) == 2:
        half(s, bar, 0, names[0], arp_on=arp_on, brass=brass)
        half(s, bar, 48, names[1], dive=falls, fall=falls, arp_on=arp_on, brass=brass)
    else:
        for i, name in enumerate(names):
            half(s, bar, 24 * i, name, 24, dive=i == 3, arp_on=arp_on, brass=brass)


# ---- the sections ----

def intro(s):
    """Bars 0-3: the hook's arpeggio alone, the hats under a riser, then
    the band half-time and held; bar 3 in full, the toms, the pickup."""
    for bar, names in ((0, ('Gm', 'F')), (1, ('Eb', 'C'))):
        for i, name in enumerate(names):
            arp(s, bar, 48 * i, 48 * i + 48, H[name][1], vel=106 + 6 * bar, soft=52)
            chord(s, bar, 48 * i, name, 48, vel=96 + 12 * bar)
    for p in range(16):
        s.note('noise', 1, 6 * p, 3, 0, 40 + 3 * p, **HAT)
    s.note('fx', 1, 0, 92, 0, 104, sweep=(2000, 180000), ramp=(1, 10))
    # bar 2: the band comes in, half-time, the bass held, the chugs muted
    s.note('fx', 2, 0, 48, 0, 124, **CRASH)
    s.hit('impact', 2, 0, 24, vel=118)
    for i, name in enumerate(('Gm', 'F')):
        root, voicing = H[name][0], H[name][1]
        arp(s, 2, 48 * i, 48 * i + 48, voicing)
        chord(s, 2, 48 * i, name, 48)
        s.note('wave', 2, 48 * i, 46, root, 124, **HELD)
        chugs(s, 2, root, 48 * i, 48 * i + 48, muted_only=True)
        s.hit('kick', 2, 48 * i, 6, vel=120)
        s.hit('clap', 2, 48 * i + 24, 6, vel=112, pan=0.05)
    for p in range(16):
        s.note('noise', 2, 6 * p, 3, 0, (100, 40, 70, 40)[p % 4], **HAT)
    # bar 3: in full to beat three, the toms and the lead's pickup
    bar_of(s, 3, ('Eb', 'C'))
    drums(s, 3, until=12)
    for t, tom, pan in ((72, 'tom_hi', -0.4), (78, 'tom_hi', -0.4), (84, 'tom_mid', 0.0), (90, 'tom_lo', 0.4)):
        s.hit(tom, 3, t, 6, vel=118, pan=pan)
    s.note('sq1', 3, 84, 6, 'D5', 108, **LEAD)
    s.note('sq1', 3, 90, 6, 'F5', 112, **LEAD)


def main(s):
    """Bars 4-11: the hook, its answer, the hook, the cadence onto v; the
    climb; the hook on bII; the turnaround."""
    for i, names in enumerate(MAIN):
        bar = 4 + i
        bar_of(s, bar, names, falls=i in (3, 7))
        drums(s, bar, until=12 if i in (3, 7) else 16, ghost=i % 2 == 1, crash=i in (0, 4, 6))
        lead(s, bar, LINES[i], fall_at=60 if i in (1, 3) else None)
        if i >= 4:   # the climb and the return thickened: the lead's tone an octave under
            for t, n, k in LINES[i]:
                s.hit('dlead', bar, t, n, key(k) - 12, 100, 0.25)
    s.hit('impact', 4, 0, 24, vel=120)
    s.hit('impact', 8, 0, 24, vel=110)
    for t, tom, pan in ((72, 'tom_hi', -0.4), (78, 'tom_hi', -0.4), (84, 'tom_mid', 0.0), (90, 'tom_lo', 0.4)):
        s.hit(tom, 7, t, 6, vel=118, pan=pan)
    for t, tom, pan in ((72, 'tom_hi', -0.4), (78, 'tom_mid', 0.0), (84, 'tom_lo', 0.4), (90, 'tom_lo', 0.4)):
        s.hit(tom, 11, t, 6, vel=112, pan=pan)


def drop(s):
    """Bar 12: i struck, then one square alone, thin, the hook at half its
    speed over the bass's one G. Bar 13: the climb back over bVI and bVII,
    the arpeggio rising, a clap roll and a riser, the pickup in octaves."""
    s.note('fx', 12, 0, 72, 0, 120, **CRASH)
    s.hit('impact', 12, 0, 36, vel=120)
    s.hit('kick', 12, 0, 6, vel=124)
    s.note('wave', 12, 0, 190, 'G2', 110, **dict(HELD, ramp=(15, 6)))
    chord(s, 12, 0, 'Gm', 96, vel=84)
    for t, n, k in ((0, 12, 'G5'), (12, 12, 'G5'), (24, 24, 'D6'), (48, 12, 'C6'), (60, 12, 'Bb5'), (72, 24, 'C6')):
        s.note('sq1', 12, t, n, k, 116, **THIN)
    for t in (24, 48, 72):
        s.note('noise', 12, t, 3, 0, 56, **HAT)
    # bar 13: the climb
    s.note('sq1', 13, 0, 72, 'D6', 116, **dict(THIN, bend=[(0, 0), (60, 0), (72, 2)]))
    s.note('fx', 13, 0, 90, 0, 116, sweep=(2500, 240000), ramp=(2, 13))
    for i, name in enumerate(('Eb', 'F')):
        arp(s, 13, 48 * i, 48 * i + (48 if i == 0 else 36), H[name][1], vel=90 + 10 * i, soft=46, lift=12 * i)
        chord(s, 13, 48 * i, name, 48, vel=96 + 10 * i)
    for t in (0, 24, 48, 72):
        s.hit('kick', 13, t, 6, vel=118)
    roll = list(range(0, 48, 12)) + list(range(48, 72, 6)) + list(range(72, 96, 3))
    for i, t in enumerate(roll):
        s.hit('clap', 13, t, 3, vel=56 + 66 * i // len(roll), pan=-0.3 + 0.6 * i / len(roll))
    for (t, k) in ((84, 'D5'), (90, 'F5')):
        s.note('sq1', 13, t, 6, k, 116, **LEAD)
        s.note('sq2', 13, t, 6, key(k) - 12, 108, **UNDER)


def climax(s):
    """Bars 14-17: the hook in octaves on both squares, everything in."""
    for i, names in enumerate(CLIMAX):
        bar = 14 + i
        bar_of(s, bar, names, arp_on=False, brass=True)
        drums(s, bar, ghost=True, crash=i % 2 == 0)
        both(s, bar, CLIMAX_LINES[i], fall_at=60 if i == 1 else None)
    s.hit('impact', 14, 0, 24, vel=124)
    s.hit('impact', 16, 0, 24, vel=116)


def ending(s):
    """Bar 18: the turnaround in octaves; bar 19: i, struck and ringing,
    the power chord and the bass diving, the lead's G falling away."""
    bar_of(s, 18, ('Csus', 'C', 'Dsus', 'D'), arp_on=False, brass=True)
    drums(s, 18, until=12, crash=True)
    both(s, 18, LINES[7])
    for i, t in enumerate(range(72, 96, 3)):
        s.hit('clap', 18, t, 3, vel=70 + 6 * i, pan=-0.3 + 0.08 * i)
    s.note('fx', 19, 0, 96, 0, 127, **CRASH)
    s.hit('impact', 19, 0, 48, vel=127)
    s.hit('kick', 19, 0, 6)
    s.hit('clap', 19, 0, 6)
    s.hit('brass', 19, 0, 60, 'Bb3', 120, 0.4)
    s.hit('pad', 19, 0, 120, 'G3', 112, -0.5)
    s.hit('pad_M', 19, 0, 120, 'Bb3', 100, 0.5)
    s.hit('gtr', 19, 0, 90, key('G2'), 124, 0.0, bend=[(0, 0), (40, 0), (90, -12)])
    s.note('wave', 19, 0, 84, 'G2', 124, **dict(HELD, bend=[(0, 0), (48, 0), (84, -12)]))
    s.note('sq1', 19, 0, 90, 'G6', 120, **dict(LEAD, bend=[(0, 0), (66, 0), (90, -3)]))
    s.note('sq2', 19, 0, 90, 'D6', 108, **dict(UNDER, bend=[(0, 0), (66, 0), (90, -3)]))


def arrangement():
    s = Score()
    intro(s)
    main(s)
    drop(s)
    climax(s)
    ending(s)
    return s
