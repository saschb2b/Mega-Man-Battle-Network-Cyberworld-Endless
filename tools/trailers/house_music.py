"""The house band: what every trailer's piece from 0.22.0 on is played by
(.claude/skills/trailer/blueprint.md, The music). It is 0.17.0's, the
piece and the pace the owner named as the reference (15 November 2026):
sketch_bust_lib.py's parts in the manner of BN5's "Virus Busting" (song
0x15, the song every layer of a run plays), 150 beats a minute, a bar 48
video frames, F minor, FluidR3 GM. A release writes its tune (a hook, its
answers, a cadence, a climb, a pickup) and names its segments; the form
and the band are laid here on the blueprint's slots, so the cut and the
piece count the same bars.

Standard library only, as every piece: the 3DS build renders a banner.

A piece is then a few lines (v0_22_0_music.py):

    FORM = house_music.form(segments=(5, 5, 5, 4))
    TUNE = {'hook': [...], 'answer': [...], 'answer2': [...], 'cadence': [...],
            'climb': [...], 'climb2': [...], 'pickup': [...], 'top': 'Ab5'}
    BARS = house_music.bars(FORM)
    def arrangement(): return house_music.arrangement(FORM, TUNE)

The slots, in bars (blueprint.md, The slots):
- cold (2): the stab's bar alone under rising hats; the band without its
  lead, the tune's pickup.
- title (2): the hook and its answer, the toms into the narrator.
- hello (4): the narrator's bars. The band steps back so his boxes are
  the lead: the backing soft with its rests, the bass, the kick and hats,
  a chord held at each bar's head; no stab, no lead, no snare. Its last
  bar lifts (the snare's four, the toms, the pickup).
- a segment (3 to 6 bars, its first the section card): one of four kinds
  in turn, as 0.17.0's four segments ran: the hook and its cadence into
  the climb; the hook on a new chord with its second answer; the break
  (two bars of the drums and the stab alone, the drop) into the climb; the
  hook with the saw under it, the climax. The last segment is always the
  climax.
- recap (2): the climb, ending on the major V.
- thanks (3): the narrator again, as hello, a box a bar; its last bar
  lifts into the stinger.
- stinger (1): F minor with its ninth, ringing.
- end (3): the backing alone, softly; the strings' last chord.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_soundfont as sf  # noqa: E402
from trailer_music import Score  # noqa: E402
from trailers import sketch_bust_lib as L  # noqa: E402
from trailers.sketch_bust_lib import (BPM, CHORD_SAMPLES, DS_LEVEL, HARDWARE, REVERB,  # noqa: E402,F401
                                      SAMPLED_LEADS, SOUNDFONT)

TAIL = 2.0
# 0.17.0's piece played its lead, backing and bass on the squares and the
# wave alone; the owner's gate 3 (2 November 2026) asks for every melodic
# part on the soundfont, the squares doubling under it. So the band's
# notes are 0.17.0's and each of those three is doubled in unison by a
# FluidR3 GM instrument (0.20.0's programs: 81 Saw Wave, 13 Xylophone, 34
# Picked Bass), the GBA's channels turned down to sit under them.
SAMPLED_PARTS = {'lead': ('sflead',), 'backing': ('mallet',), 'bass': ('sfbass',)}
DOUBLES = {'sq1': 'sflead', 'sq2': 'mallet', 'wave': 'sfbass'}
MIX = (('lead', ('sq1',), 0.10), ('arp', ('sq2',), 0.14), ('bass', ('wave',), 0.12),
       ('hats', ('noise',), 0.12), ('fx', ('fx',), 0.08))
DS_ENV = dict(L.DS_ENV, sflead=(255, 0, 230, 200), mallet=(255, 0, 255, 150), sfbass=(255, 0, 220, 170))
DS_GAIN = dict(L.DS_GAIN, sflead=0.3, mallet=0.27, sfbass=0.6)
COLD, TITLE, HELLO, RECAP, THANKS, STINGER, END = 2, 2, 4, 2, 3, 1, 3


def form(segments, hello=HELLO, thanks=THANKS, recap=RECAP, end=END):
    """The cut's slots as (name, first bar, bars): cold, title, hello, a
    'segment' for each of `segments` (its bars, the card's among them),
    recap, thanks, stinger, end."""
    out, at = [], 0
    for name, n in ([('cold', COLD), ('title', TITLE), ('hello', hello)] + [('segment', n) for n in segments]
                    + [('recap', recap), ('thanks', thanks), ('stinger', STINGER), ('end', end)]):
        if n:
            out.append((name, at, n))
            at += n
    return tuple(out)


def bars(slots):
    return slots[-1][1] + slots[-1][2]


def first(slots, name, k=0):
    """the first bar of the k-th slot called `name`"""
    return [s for s in slots if s[0] == name][k][1]


def instruments():
    S = L.V.instruments('F2')
    S['sflead'] = sf.instrument(81, [74], 2.0, fade=0.2)
    S['mallet'] = sf.instrument(13, [76], 0.3, fade=0.05)
    S['sfbass'] = sf.instrument(34, [45], 0.9, fade=0.08)
    return S


def doubled(s):
    """the score with its squares' and its wave's notes doubled in unison
    on the soundfont (DOUBLES), each at its note's place, side and bend"""
    for ch, inst in DOUBLES.items():
        for t, n, k, vel, o in s.psg[ch]:
            note = (t, n, inst, k, vel, o.get('pan', 0.0))
            s.ds.append(note + (o['bend'],) if o.get('bend') else note)
    return s


# a segment's kinds in turn, each bar (its section, its chords, its line);
# a segment longer than its kind's bars goes on with the hook and its
# answer on the title's chords, a shorter one stops early
A1 = [('a', ('Dbmaj7', 'Eb6'), 'hook'), ('a', ('Bmaj7', 'Csus2'), 'cadence'),
      ('b', ('Dbmaj7', 'Ebsus2'), 'climb'), ('b', ('Fm7', 'C'), 'climb2')]
A2 = [('a', ('Bbm7', 'Eb6'), 'hook'), ('a', ('Abmaj7', 'Dbmaj7'), 'answer2'),
      ('a', ('Dbmaj7', 'Eb6'), 'hook'), ('a', ('Bmaj7', 'Csus2'), 'cadence')]
BREAK = [('break', ('Fm9',), None), ('break', ('Fm9',), 'pickup'),
         ('b', ('Dbmaj7', 'Ebsus2'), 'climb'), ('b', ('Fm7', 'C'), 'climb2')]
CLIMAX = [('a', ('Bbm7', 'Eb6'), 'hook'), ('a', ('Abmaj7', 'Dbmaj7'), 'answer'),
          ('a', ('Dbmaj7', 'Eb6'), 'hook'), ('a', ('Bmaj7', 'Csus2'), 'cadence')]
MORE = [('a', ('Bbm7', 'Eb6'), 'hook'), ('a', ('Abmaj7', 'Dbmaj7'), 'answer')]
KINDS = (A1, A2, BREAK)
TALK = [('talk', ('Dbmaj7', 'Ebsus2'), None), ('talk', ('Fm9',), None),
        ('talk', ('Dbmaj7', 'Ebsus2'), None), ('talk', ('Fm7', 'C'), None)]


def segment_bars(k, n, last):
    """segment k's n bars: its kind's, the climax for the last"""
    kind = CLIMAX if last else KINDS[k % len(KINDS)]
    out = list(kind[:n])
    while len(out) < n:
        # (a longer segment: the hook and its answer before the kind's last two bars)
        out[-2:-2] = [MORE[(len(out) - len(kind)) % 2]]
    return out


def plan(slots):
    """{bar: (section, chords, the tune's line or None, flags)}; flags a
    set of 'crash' (a slot's head), 'toms' (into the next slot), 'climax'
    (the saw under the lead), 'lift' (the narrator's last bar)."""
    out = {}
    segs = [s for s in slots if s[0] == 'segment']
    for name, at, n in slots:
        if name == 'cold':
            rows = [('intro', ('Fm9',), None), ('band', ('Fm9',), 'pickup')][:n]
        elif name == 'title':
            rows = [('a', ('Bbm7', 'Eb6'), 'hook'), ('a', ('Abmaj7', 'Dbmaj7'), 'answer')][:n]
        elif name in ('hello', 'thanks'):
            rows = (TALK * 2)[:n] if name == 'hello' else (TALK * 2)[-n:]
        elif name == 'segment':
            k = segs.index((name, at, n))
            rows = segment_bars(k, n, k == len(segs) - 1)
        elif name == 'recap':
            rows = [('b', ('Dbmaj7', 'Ebsus2'), 'climb'), ('b', ('Fm7', 'C'), 'climb2')][:n]
        elif name == 'stinger':
            rows = [('ring', ('Fm9',), None)]
        else:
            rows = ([('end', ('Dbmaj7', 'Ebsus2'), None)] + [('end', ('Fm9',), None)] * (n - 1))[:n]
        for i, (kind, names, line) in enumerate(rows):
            flags = set()
            if i == 0 and kind in ('a', 'b', 'break') or kind == 'band':
                flags.add('crash')
            if i == n - 1 and name in ('title', 'segment', 'recap'):
                flags.add('toms')
            if i == n - 1 and name in ('hello', 'thanks'):
                flags.add('lift')
            if name == 'segment' and (name, at, n) == segs[-1] and kind == 'a':
                flags.add('climax')
            if kind == 'end' and i == n - 1:
                flags.add('last')
            out[at + i] = (kind, names, line, flags)
    return out


def a_bar(s, bar, names, line, flags, nxt):
    top = 'climax' in flags
    L.backing(s, bar, names, vel=8 if top else 0)
    L.chords(s, bar, names, vel=100 if top else 94)
    L.bass(s, bar, names, nxt)
    L.stab(s, bar, names, vel=110 if top else 104, until=14 if 'toms' in flags else 16)
    L.lead(s, bar, line, vel=120 if top else 116)
    if top:
        L.double(s, bar, line)
    L.kit(s, bar, crash='crash' in flags, until=12 if 'toms' in flags else 16)
    if 'crash' in flags:
        L.power(s, bar, 0, 30, L.H[names[0]][0], vel=114)


def b_bar(s, bar, names, line, flags, nxt):
    L.backing(s, bar, names, rest=True)
    L.chords(s, bar, names, vel=100)
    L.held(s, bar, names, vel=84)
    L.bass(s, bar, names, nxt)
    L.lead(s, bar, line)
    L.kit(s, bar, crash='crash' in flags, until=12 if 'toms' in flags else 16)
    if names[-1] == 'C':
        L.power(s, bar, 48, 30, 'C3', vel=112, fall=True)
    elif 'crash' in flags:
        L.power(s, bar, 0, 30, L.H[names[0]][0], vel=112)


def talk_bar(s, bar, names, pickup, flags, nxt):
    """the narrator's bar: the band stepped back, his boxes the lead"""
    lift = 'lift' in flags
    L.backing(s, bar, names, rest=True, vel=-16)
    L.held(s, bar, names, vel=72)
    L.bass(s, bar, names, nxt, until=12 if lift else 16)
    for p in (0, 6, 8, 10):
        if not (lift and p > 8):
            s.hit('kick', bar, 6 * p, 6, vel=100 if p in (0, 8) else 84)
    L.hats(s, bar, 12 if lift else 16, level=-14)
    if lift:
        for p in (8, 9, 10, 11):
            s.hit('snare', bar, 6 * p, 6, vel=60 + 12 * (p - 8), pan=0.05)
        L.toms(s, bar)
        if pickup:
            L.lead(s, bar, pickup, vel=108)


def end_bar(s, bar, names, flags):
    """the end card: the backing alone, softly, the strings' last chord"""
    last = 'last' in flags
    L.backing(s, bar, names, rest=True, vel=-24, until=12 if last else 16)
    if last:
        s.hit('pad_m', bar, 0, 140, 'F3', 84, -0.35)
        s.hit('pad', bar, 0, 140, 'F3', 70, 0.35)


def arrangement(slots, tune):
    """The piece: the band on `slots`, playing `tune` (its lines by name:
    hook, answer, answer2, cadence, climb, climb2, pickup; `top` the
    stinger's long note)."""
    s = Score()
    p = plan(slots)
    for bar in range(bars(slots)):
        kind, names, name, flags = p[bar]
        line = tune[name] if name else None
        nxt = p[bar + 1][1][0] if bar + 1 in p else None
        if kind == 'intro':
            L.stab(s, bar, names, vel=96)
            for q in range(16):
                s.note('noise', bar, 6 * q, 3, 0, 28 + 4 * q, **L.V.HAT)
            s.note('fx', bar, 0, 92, 0, 100, sweep=(1500, 120000), ramp=(1, 8))
        elif kind == 'band':
            L.backing(s, bar, names)
            L.bass(s, bar, names, nxt)
            L.stab(s, bar, names)
            L.kit(s, bar, crash=True, until=12)
            L.lead(s, bar, line, vel=108)
        elif kind == 'break':
            L.stab(s, bar, names, vel=100)
            L.kit(s, bar, crash='crash' in flags, until=12 if line else 16)
            if line:
                L.lead(s, bar, line, vel=108)
        elif kind == 'a':
            a_bar(s, bar, names, line, flags, nxt)
        elif kind == 'b':
            b_bar(s, bar, names, line, flags, nxt)
        elif kind == 'talk':
            # (the pickup into a segment, never into the stinger's chord)
            talk_bar(s, bar, names, tune['pickup'] if p.get(bar + 1, ('',))[0] != 'ring' else None, flags, nxt)
        elif kind == 'ring':
            L.ring(s, bar, 'Fm9', tune.get('top', 'C5'), 'F2')
            s.hit('pad', bar, 0, 150, 'F4', 92, 0.35)          # F G C: the ninth
        else:
            end_bar(s, bar, names, flags)
        if 'toms' in flags:
            L.toms(s, bar)
    return doubled(s)


SKETCH = form(segments=(4,), hello=0, thanks=0, recap=0, end=0)


def sketch(tune):
    """A tune's nine bars for the owner's ear (gate 4), the band as the
    piece plays it: the count-in and the band's bar with the pickup, the
    hook and its answer, the hook with the saw under it, the answer, the
    hook and its cadence (a last segment's bars), F minor ringing."""
    return arrangement(SKETCH, tune)
