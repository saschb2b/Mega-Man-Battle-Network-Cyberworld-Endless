"""What 0.20.0's second round of sketches share (sketch_red_a, _b and _c;
not a piece of its own): a Battle Network battle and tournament track's
habits, read from BN4's and BN6's ROMs, as parts to compose with; never
their notes. The owner on the first round (sketch_sun_a and _b, 8
November 2026): "the music sounds flat and not complex. I miss the mmbn
dna and good use of the soundfont". Those sketches had matched 0x17's
calmest numbers (a bass on one note a bar, one arpeggio, bare fifths)
and lost what makes a BN battle track busy.

What the sources do, read with song_habits.py --detail --theory (numbers
only):
- BN4's virus battle (0x14, 156 beats a minute): the wave's bass in
  staccato sixteenths (554 notes of four ticks) across two octaves
  (keys 39-64), bent 48 times; a sampled arpeggio in 32nds (960 notes of
  three and four ticks, 16 velocity levels) thrown hard left and right;
  the lead on square 2 (50% and 25%) with square 1 beside it thrown left
  and right 33 times; a held chord sample; m7 24% of its half bars, sus4
  12, aug 10, maj7 10, the borrowed bIIImaj7 and bVIImaj7 and an
  augmented V; roots by fifths and thirds; 1.61 chord changes a bar.
- BN4's boss battle (0x15, its tempo climbing to 170): a sampled
  arpeggio of sixteenths over three octaves (keys 36-75, skips and leaps
  only) re-panned 135 times; the lead on both squares (25% and 12.5%)
  with an echo, bent 51 times, held notes at 50% for its second
  section; the wave's bass in eighths and sixteenths with octave falls;
  two sampled counter-voices answering each other; sus4 21%, m7b5 17%,
  m7 13%; crashes and cymbals on the off-beats in its climax.
- BN4's tournament (0x17, 140): the kick on every beat, the snare's
  ghosts on 7 and 13, toms falling through a bar in its fills, the lead's
  echo four ticks behind from the other side, a bar of the drums alone
  before its loop.
- BN6's boss battle (0x16, "Surge of Power!", music_rules.md): the kick
  syncopated on 0 6 7 10 14, 32nd-note pairs on a square thrown hard left
  and right, a low distorted stab with bends, suspended chord samples,
  two chords a bar, a fake-out every eight bars.

The parts here, every melodic one on FluidR3 GM (the owner's gate 3),
the GBA's channels doubling them for the top and the colour:
- the lead (81 Saw Wave over square 1, vibrato, falls and scoops, its
  echo three ticks behind from the other side);
- a counter-melody that answers it (61 Brass Section, 40 Violin or 11
  Vibraphone, a section each), its line the chords' guide tones (thirds
  and sevenths) moving by step;
- two arpeggio layers: 32nd-note pairs over the chord (13 Xylophone over
  square 2 at 25%, thrown hard left and right note by note), and a
  sixteenth arpeggio over three octaves (5 Electric Piano 2, wide);
- the bass (34 Electric Bass (pick) over the wave): sixteenths with
  octave jumps, walking passages into the next chord, falls at phrase
  ends;
- chords: 48 Strings' chord samples recoloured by the bass, 61's brass
  chord struck on the syncopations, 30 Distortion Guitar's power chord
  and 28's palm-muted chugs, 55's orchestra hit on a section's one;
- the kit: 24's electronic kick, a clap over a snare, toms in fills, the
  noise channel's hats, open hats and crash; a break of the drums alone.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_soundfont as sf  # noqa: E402
from trailer_music import key  # noqa: E402
from trailers import sketch_vb_lib as V  # noqa: E402
from trailers.sketch_vb_lib import HARDWARE, REVERB, SOUNDFONT  # noqa: E402,F401

TAIL = 2.0
DS_LEVEL = 1.0
MIX = (('lead', ('sq1',), 0.10), ('arp', ('sq2',), 0.13), ('bass', ('wave',), 0.11),
       ('hats', ('noise',), 0.15), ('fx', ('fx',), 0.08))
CHORD_SAMPLES = dict(V.CHORD_SAMPLES)
DS_ENV = dict(V.DS_ENV, sflead=(255, 0, 230, 200), echo=(255, 0, 230, 200), mallet=(255, 0, 255, 150),
              sfbass=(255, 0, 220, 170), ep2=(255, 0, 255, 160), horn=(255, 0, 235, 190),
              vln=(200, 0, 240, 200), vibes=(255, 0, 255, 175), hit=(255, 0, 255, 215), bell=(255, 0, 255, 170))
DS_GAIN = dict(V.DS_GAIN, kick=0.88, snare=0.62, clap=0.6, pad=0.42, pad_m=0.42, pad_M=0.42, pad_d=0.42,
               sflead=0.3, echo=0.15, mallet=0.27, sfbass=0.6, ep2=0.24, horn=0.3, vln=0.26, vibes=0.26,
               hit=0.45, bell=0.16, brass=0.34, gtr=0.36, mute=0.32)
SAMPLED_LEADS = ('dlead',)
# every melodic part on FluidR3 GM's instruments (gate 3), by role for
# music_check.py: the lead the saw, the backing the xylophone's 32nds, the
# bass the picked bass (the wave doubles it; the squares the lead and the
# backing)
SAMPLED_PARTS = {'lead': ('sflead',), 'backing': ('mallet',), 'bass': ('sfbass',)}
LEAD = dict(duty=0.5, env=(0, 1, 11, 2), vib=(24, 14))
LEAD_THIN = dict(duty=0.25, env=(0, 1, 11, 2), vib=(24, 14))
BLIP = dict(duty=0.25, env=(0, 1, 9, 1))
PULSE = [13] * 12 + [0] * 20                      # the wave's narrow pulse (0.14.0's)
WAVE = dict(env=(0, 3, 7, 1), wave=PULSE)


def instruments():
    S = V.instruments('D2')
    S['sflead'] = sf.instrument(81, [74], 2.0, fade=0.2)
    S['echo'] = S['sflead']
    S['mallet'] = sf.instrument(13, [76], 0.3, fade=0.05)
    S['ep2'] = sf.instrument(5, [67], 0.5, fade=0.08)
    S['sfbass'] = sf.instrument(34, [45], 0.9, fade=0.08)
    S['horn'] = sf.instrument(61, [65], 1.2, fade=0.15)
    S['vln'] = sf.instrument(40, [76], 1.4, fade=0.2)
    S['vibes'] = sf.instrument(11, [72], 0.8, fade=0.15)
    S['bell'] = sf.instrument(9, [84], 0.6, fade=0.1)
    S['hit'] = sf.instrument(55, [key('D4')], 0.45)
    S['hit'].base = None
    return S


# ---- chords: a name's root and quality, its tones, its sample ----

QUAL = {'m7': (0, 3, 7, 10), 'maj7': (0, 4, 7, 11), '7': (0, 4, 7, 10), 'sus4': (0, 5, 7, 10), 'sus2': (0, 2, 7, 12),
        'm': (0, 3, 7, 12), '': (0, 4, 7, 12), 'm7b5': (0, 3, 6, 10), 'aug': (0, 4, 8, 12), 'add9': (0, 4, 7, 14),
        'm9': (0, 3, 7, 14), '5': (0, 7, 12, 19)}
PC = {'C': 0, 'D': 2, 'E': 4, 'F': 5, 'G': 7, 'A': 9, 'B': 11}


def parse(name):
    m = re.fullmatch(r'([A-G])([#b]?)(.*)', name)
    pc = (PC[m.group(1)] + {'#': 1, 'b': -1, '': 0}[m.group(2)]) % 12
    return pc, m.group(3)


def tones(name):
    pc, q = parse(name)
    return [pc + i for i in QUAL[q]]


def sample_of(name):
    """the strings' chord sample and the key it is struck at (Kaida's way:
    a maj7 the minor triad on its third, a sus4 the sus2 on its fourth)"""
    pc, q = parse(name)
    if q in ('m', 'm7', 'm9'):
        return 'pad_m', pc
    if q == 'maj7':
        return 'pad_m', pc + 4
    if q in ('sus4',):
        return 'pad', pc + 5
    if q in ('sus2', '5'):
        return 'pad', pc
    if q in ('m7b5',):
        return 'pad_d', pc
    if q == 'aug':
        return 'pad_M', pc
    return 'pad_M', pc


def near(pc, lo):
    """pitch class pc at or above midi `lo`"""
    k = lo + (pc - lo) % 12
    return k


def halves(names):
    if len(names) == 1:
        return [(0, 96, names[0])]
    if len(names) == 2:
        return [(0, 48, names[0]), (48, 48, names[1])]
    return [(0, 48, names[0]), (48, 24, names[1]), (72, 24, names[2])]


def at(names, t):
    return next(nm for t0, n, nm in halves(names) if t0 <= t < t0 + n)


# ---- the parts ----

def lead(s, bar, line, vel=114, side=-1, echo=True, thin=False, octave=0):
    """the lead: the saw over square 1, vibrato on the held notes; a note
    may carry a fall or scoop (a fourth item, semitones over its last
    third); its echo three ticks behind from the other side"""
    for item in line:
        t, n, k = item[:3]
        k = key(k) + 12 * octave
        fall = item[3] if len(item) > 3 else 0
        bend = None
        if fall > 0:
            bend = [(0, -fall), (min(4, n // 3), 0)]              # a scoop up into the note
        elif fall < 0:
            bend = [(0, 0), (n * 2 // 3, 0), (n, fall)]           # a fall at its end
        v = vel - (12 if t % 12 else 0)
        s.hit('sflead', bar, t, n, k, v, 0.3 * side, bend)
        voice = LEAD_THIN if thin else LEAD
        s.note('sq1', bar, t, n, k, v - 16, pan=0.3 * side, **(dict(voice, bend=bend) if bend else voice))
        if echo and t + 3 < 96:
            s.hit('echo', bar, t + 3, max(2, n - 3), k, v - 16, -0.8 * side, bend)


def counter(s, bar, line, inst='horn', vel=96, pan=0.45):
    """a counter-melody's notes on one of the section's voices"""
    for item in line:
        t, n, k = item[:3]
        fall = item[3] if len(item) > 3 else 0
        bend = [(0, 0), (n * 2 // 3, 0), (n, fall)] if fall else None
        s.hit(inst, bar, t, n, key(k), vel - (10 if t % 24 else 0), pan, bend)


def guide(names_by_bar, start_key, rhythm=((0, 30), (36, 12), (48, 30), (84, 12))):
    """a counter-line from the chords' guide tones (thirds and sevenths),
    each moving to the nearest one of the next chord: {bar: [(t, n, k)]}"""
    prev = key(start_key)
    out = {}
    for bar, names in names_by_bar.items():
        line = []
        for t, n in rhythm:
            nm = at(names, t)
            pc, q = parse(nm)
            ts = tones(nm)
            cands = [ts[1], ts[3] if len(ts) > 3 else ts[2]]
            if t % 48:
                cands = ts[:3]                    # (the pickup: a chord tone a step away)
            best = min((near(c % 12, prev - 6) for c in cands), key=lambda k: (abs(k - prev), k))
            if best == prev and t % 48:
                best = min((near(c % 12, prev - 6) for c in ts if near(c % 12, prev - 6) != prev),
                           key=lambda k: abs(k - prev))
            line.append((t, n, best))
            prev = best
        out[bar] = line
    return out


def pairs32(s, bar, names, until=16, vel=0, lo=67):
    """32nd-note pairs over the chord (BN6's 0x16, BN4's 0x14): each
    sixteenth struck loud then soft, climbing the chord's tones, thrown
    hard left and right note by note; the xylophone over square 2"""
    n = 0
    for p in range(until):
        nm = at(names, 6 * p)
        ts = sorted(near(t % 12, lo) for t in tones(nm))
        k = ts[(0, 1, 2, 3, 2, 1, 2, 3)[p % 8] % len(ts)]
        for half in (0, 1):
            side = -0.9 if n % 2 == 0 else 0.9
            v = (100 if p % 4 == 0 else 84) + vel if not half else 50 + vel
            s.hit('mallet', bar, 6 * p + 3 * half, 3, k, max(24, v), side)
            s.note('sq2', bar, 6 * p + 3 * half, 3, k, max(16, v - 22), pan=side, **BLIP)
            n += 1


def sweep(s, bar, names, vel=78, until=16):
    """a sixteenth arpeggio over three octaves (BN4's 0x15), up and down,
    on the electric piano, wide"""
    order = (0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1)
    for p in range(until):
        nm = at(names, 6 * p)
        ts = sorted(near(t % 12, 48) for t in tones(nm)[:3])
        ladder = [k + 12 * o for o in range(3) for k in ts]
        k = ladder[order[p] % len(ladder)]
        s.hit('ep2', bar, 6 * p, 5, k, vel - (16 if p % 2 else 0), -0.7 if p % 4 < 2 else 0.7)


def bass_root(nm):
    pc, _ = parse(nm)
    return near(pc, key('D2'))


def bass(s, bar, names, nxt=None, style='drive', until=16, vel=0, fall=False):
    """the bass: 'drive' sixteenths with octave jumps (BN4's 0x14), 'walk'
    eighths and sixteenths walking into the next chord, 'pump' eighths on
    the root with a jump on the off-beat; the wave doubling it, short"""
    notes = []
    for t0, n, nm in halves(names):
        r = bass_root(nm)
        ts = tones(nm)
        fifth = r + (ts[2] - ts[0])
        third = r + (ts[1] - ts[0])
        if style == 'drive':
            fig = [r, r, r + 12, r, r, r + 12, r, fifth] if n == 48 else [r, r + 12, r, fifth]
            for i, k in enumerate(fig):
                notes.append((t0 + 6 * i, 5, k, 120 if i % 2 == 0 else 96))
        elif style == 'walk':
            tgt_name = nxt if t0 + n >= 96 else at(names, t0 + n)
            tgt = bass_root(tgt_name) if tgt_name else r
            app = tgt - 1 if tgt > r else tgt + 1
            fig = [(0, 12, r), (12, 6, r + 12), (18, 6, r), (24, 12, fifth), (36, 6, third), (42, 6, app)]
            if n < 48:
                fig = [(0, 12, r), (12, 6, fifth), (18, 6, app)]
            for t, ln, k in fig:
                notes.append((t0 + t, ln - 1, k, 120 if t % 12 == 0 else 100))
        else:
            for i in range(n // 12):
                k = r + 12 if i % 2 else r
                notes.append((t0 + 12 * i, 10, k, 120 if i % 2 == 0 else 104))
    for t, ln, k, v in notes:
        if t >= 6 * until:
            continue
        bend = None
        if fall and t >= 84:
            bend = [(0, 0), (ln // 2, 0), (ln, -5)]
        s.hit('sfbass', bar, t, ln, k, min(127, v + vel), 0.0, bend)
        s.note('wave', bar, t, max(2, ln - 1), k, v, **WAVE)


def pads(s, bar, names, vel=72, up=12):
    """the strings' chord samples, a chord a half bar, recoloured by the
    bass, a little to each side"""
    for t0, n, nm in halves(names):
        smp, pc = sample_of(nm)
        s.hit(smp, bar, t0, n - 2, near(pc, key('G2')) + up, vel, -0.35 if t0 < 48 else 0.35)


STABS = (0, 6, 14)               # 3-3-2's last two and the and of four: the brass's places


def stabs(s, bar, names, places=(6, 10, 14), vel=96):
    """the brass's chord struck on the syncopations"""
    for p in places:
        nm = at(names, 6 * p)
        pc, q = parse(nm)
        k = near(pc + (4 if q == 'maj7' else 0), key('A3'))
        s.hit('brass', bar, 6 * p, 5, k, vel, 0.25)


def power(s, bar, names, places=(0, 6, 12), vel=96, chug=True, dive=False):
    """the distortion guitar's power chord on the 3-3-2, palm-muted chugs
    between; a dive at the end where asked"""
    for p in range(16):
        nm = at(names, 6 * p)
        r = V.low(bass_root(nm) + 12) + 12
        if p in places:
            n = 16 if p == places[-1] else 10
            bend = [(0, 0), (6, 0), (n, -7)] if dive and p == places[-1] else None
            s.hit('gtr', bar, 6 * p, n, r, vel, 0.0, bend)
        elif chug and p % 2 == 0 and p not in (2, 8):
            s.hit('mute', bar, 6 * p, 5, r, vel - 20, 0.0)


def kit(s, bar, style='battle', until=16, crash=False, vel=0, hit=False):
    """'battle' BN6's 0x16's syncopated kick (0 6 7 10 14), the clap on 4
    and 12; 'four' BN4's 0x17's kick on every beat with the snare's ghosts;
    'half' the kick on 0 and 10, the clap on 8 (a half-time drop); hats on
    every sixteenth at three levels, the open hat on the off-beats"""
    kicks = {'battle': (0, 6, 7, 10, 14), 'four': (0, 4, 8, 12), 'half': (0, 10)}[style]
    claps = {'battle': (4, 12), 'four': (4, 12), 'half': (8,)}[style]
    for p in kicks:
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=(112 if p in (0, 8) else 98) + vel)
    for p in claps:
        if p < until:
            s.hit('clap', bar, 6 * p, 6, vel=108 + vel, pan=-0.05)
            s.hit('snare', bar, 6 * p, 6, vel=96 + vel, pan=0.05)
    if style == 'four':
        for p in (7, 13):
            if p < until:
                s.hit('snare', bar, 6 * p, 6, vel=44, pan=0.05)
    for p in range(until):
        if style != 'half' and p % 4 == 2:
            s.note('noise', bar, 6 * p, 6, 0, 76, **V.OPEN)
        else:
            s.note('noise', bar, 6 * p, 3, 0, (90, 44, 64, 44)[p % 4], **V.HAT)
    if crash:
        s.note('fx', bar, 0, 64, 0, 108, **V.CRASH)
    if hit:
        s.hit('hit', bar, 0, 24, 'D4', 110, 0.0)


def fill(s, bar, kind='toms', vel=110):
    """a fill on the bar's last beat or half: toms falling, or the snare's
    sixteenths rising"""
    if kind == 'toms':
        for p, tom, pan in ((10, 'tom_hi', -0.4), (11, 'tom_hi', -0.4), (12, 'tom_mid', -0.1), (13, 'tom_mid', -0.1),
                            (14, 'tom_lo', 0.3), (15, 'tom_lo', 0.3)):
            s.hit(tom, bar, 6 * p, 6, vel=vel, pan=pan)
    else:
        for p in range(8, 16):
            s.hit('snare', bar, 6 * p, 6, vel=60 + 8 * (p - 8), pan=0.05)


def break_bar(s, bar, names, vel=0):
    """a break: the drums alone but the bass's first beat and the hit,
    the toms rolling into the next section"""
    s.hit('hit', bar, 0, 24, 'D4', 108, 0.0)
    bass(s, bar, names, style='pump', until=4, vel=vel)
    for p in (0, 3, 6, 8, 10):
        s.hit('kick', bar, 6 * p, 6, vel=112)
    for p in (4, 12):
        s.hit('clap', bar, 6 * p, 6, vel=108)
    for p in range(0, 16, 2):
        s.note('noise', bar, 6 * p, 3, 0, 70, **V.HAT)
    fill(s, bar, 'toms')


def ring(s, bar, name, top, lead_vel=112):
    """the last chord ringing: the crash and the hit, the strings and the
    guitar held, the bass falling, the lead's long note with a fall"""
    s.note('fx', bar, 0, 96, 0, 116, **V.CRASH)
    s.hit('kick', bar, 0, 6)
    s.hit('hit', bar, 0, 30, 'D4', 112, 0.0)
    smp, pc = sample_of(name)
    s.hit(smp, bar, 0, 150, near(pc, key('G2')) + 12, 92, -0.3)
    s.hit('gtr', bar, 0, 80, V.low(bass_root(name) + 12) + 12, 100, 0.0, [(0, 0), (60, 0), (80, -5)])
    s.hit('sfbass', bar, 0, 90, bass_root(name), 127, 0.0, [(0, 0), (60, 0), (90, -3)])
    bend = [(0, 0), (80, 0), (96, -2)]
    s.hit('sflead', bar, 0, 96, key(top), lead_vel, 0.0, bend)
    s.note('sq1', bar, 0, 96, key(top), lead_vel - 18, **dict(LEAD, bend=bend))


def up(line, n):
    """a line moved by n semitones (the key's lift)"""
    return [(it[0], it[1], key(it[2]) + n) + tuple(it[3:]) for it in line]


def play(s, chords, plan):
    """each bar of `plan` ({bar: {part: setting}}) over `chords` ({bar:
    names}): lead, counter (inst, line), arp ('pairs', 'sweep', 'both'),
    bass (style), pads, stabs, power, kit (style), crash, hit, fill,
    bell (the lead doubled an octave up), brk (a break bar), ring (name,
    top)"""
    for bar, p in sorted(plan.items()):
        names = chords[bar]
        nxt = chords.get(bar + 1, (None,))[0]
        if p.get('ring'):
            ring(s, bar, *p['ring'])
            continue
        if p.get('brk'):
            break_bar(s, bar, names)
            continue
        until = 12 if p.get('fill') == 'toms' else 16
        arp = p.get('arp')
        if arp in ('pairs', 'both'):
            pairs32(s, bar, names, vel=p.get('arp_vel', 0))
        if arp in ('sweep', 'both'):
            sweep(s, bar, names)
        if p.get('bass'):
            bass(s, bar, names, nxt, style=p['bass'], until=until, fall=bool(p.get('fill')))
        if p.get('pads', True):
            pads(s, bar, names, vel=p.get('pad_vel', 72))
        if p.get('stabs'):
            stabs(s, bar, names)
        if p.get('power'):
            power(s, bar, names, dive=bool(p.get('fill')))
        if p.get('kit'):
            kit(s, bar, p['kit'], until=until, crash=p.get('crash', False), hit=p.get('hit', False))
        if p.get('fill'):
            fill(s, bar, p['fill'])
        if p.get('lead'):
            lead(s, bar, p['lead'], vel=p.get('lead_vel', 114), side=p.get('side', -1), thin=p.get('thin', False))
            if p.get('bell'):
                for it in p['lead']:
                    s.hit('bell', bar, it[0], min(it[1], 12), key(it[2]) + 12, 70, 0.5)
        if p.get('counter'):
            inst, line = p['counter']
            counter(s, bar, line, inst, vel=p.get('counter_vel', 96), pan=0.4 if p.get('side', -1) < 0 else -0.4)
