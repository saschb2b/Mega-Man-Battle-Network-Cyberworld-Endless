"""A soundfont's instruments as the trailers' kit samples, as a Game Boy
Advance game's composer cut them: a few notes of a synthesizer's sound (BN's
were an XV-1080's and SC-88s', Kaida says), a chord recorded as one
sample, brought down to 10512 Hz and 8 bits, cut short, for MP2K to play
(tools/trailer_music.py: its envelopes, nearest-sample pitch, 13379 Hz mix
and echo). Standard library only. The soundfont is FluidR3 GM (Frank Wen,
the MIT license), which docker/Dockerfile.music installs from Debian
(fluid-soundfont-gm); it is never in git, and a piece that uses it renders
in that image (CW_SOUNDFONT names the file). The image holds GeneralUser
GS and MuseScore General too, for the owner to hear a piece in each:
CW_SOUNDFONT_NAME=generaluser or musescore (FONTS).

    instrument(program, keys, seconds)       a sample of a melodic preset
    drum(key, seconds)                       one of the GM kit's (bank 128)
"""
import math
import os
import struct

from trailer_music import SAMPLE_RATE, Sample

# the soundfonts docker/Dockerfile.music holds, by the name CW_SOUNDFONT_NAME
# gives (FluidR3 GM by default, until the owner picks one by ear);
# CW_SOUNDFONT names any other file
FONTS = {
    'fluidr3': ('/usr/share/sounds/sf2/FluidR3_GM.sf2',
                'FluidR3 GM (Frank Wen, the MIT license; Debian\'s fluid-soundfont-gm)'),
    'generaluser': ('/usr/share/sounds/sf2/GeneralUser-GS.sf2',
                    'GeneralUser GS v2.0.3 (S. Christian Collins, its License v2.0)'),
    'musescore': ('/usr/share/sounds/sf2/MuseScore_General_Full.sf2',
                  'MuseScore General (the MIT license; Debian\'s musescore-general-soundfont-lossless)'),
}
FONT = os.environ.get('CW_SOUNDFONT_NAME', 'fluidr3')
if FONT not in FONTS:
    raise SystemExit(f'no soundfont {FONT}: {", ".join(FONTS)}')
PATH = FONTS[FONT][0] if 'CW_SOUNDFONT_NAME' in os.environ else os.environ.get('CW_SOUNDFONT', FONTS['fluidr3'][0])
SOUNDFONT = FONTS['fluidr3'][1]       # (what a piece names: the default's, whichever renders)


def available():
    return os.path.exists(PATH)


class SoundFont:
    """An SF2's presets, instruments and sample headers (its pdta), and
    where its 16-bit samples are (its smpl), read once."""

    def __init__(self, path):
        self.f = open(path, 'rb')
        riff, size, kind = struct.unpack('<4sI4s', self.f.read(12))
        if riff != b'RIFF' or kind != b'sfbk':
            raise ValueError(f'{path}: not a soundfont')
        chunks = {}
        end = 12 + size - 4
        pos = 12
        while pos < end:
            self.f.seek(pos)
            cid, csize = struct.unpack('<4sI', self.f.read(8))
            if cid == b'LIST':
                sub = self.f.read(4)
                inner, stop = pos + 12, pos + 8 + csize
                while inner < stop:
                    self.f.seek(inner)
                    iid, isize = struct.unpack('<4sI', self.f.read(8))
                    chunks[(sub, iid)] = (inner + 8, isize)
                    inner += 8 + isize + (isize & 1)
            pos += 8 + csize + (csize & 1)
        self.smpl = chunks[(b'sdta', b'smpl')][0]

        def table(name, fmt):
            at, n = chunks[(b'pdta', name)]
            self.f.seek(at)
            raw = self.f.read(n)
            step = struct.calcsize(fmt)
            return [struct.unpack_from(fmt, raw, i) for i in range(0, n - step + 1, step)]
        self.phdr = table(b'phdr', '<20sHHHIII')
        self.pbag = table(b'pbag', '<HH')
        self.pgen = table(b'pgen', '<HH')
        self.inst = table(b'inst', '<20sH')
        self.ibag = table(b'ibag', '<HH')
        self.igen = table(b'igen', '<HH')
        self.shdr = table(b'shdr', '<20sIIIIIBbHH')

    @staticmethod
    def _zones(bags, gens, first, last):
        """Each zone's generators {operator: amount}, the global zone's
        merged into the others."""
        zones = []
        for b in range(first, last):
            g0, g1 = bags[b][0], bags[b + 1][0]
            zones.append({op: amount for op, amount in gens[g0:g1]})
        if zones and 53 not in zones[0] and 41 not in zones[0]:
            glob = zones.pop(0)
            zones = [{**glob, **z} for z in zones]
        return zones

    @staticmethod
    def _covers(z, key, vel):
        lo, hi = (z[43] & 0xFF, z[43] >> 8) if 43 in z else (0, 127)
        vlo, vhi = (z[44] & 0xFF, z[44] >> 8) if 44 in z else (0, 127)
        return lo <= key <= hi and vlo <= vel <= vhi

    def zone(self, program, key, bank=0, vel=100):
        """The sample a preset plays for key at vel: (header, generators)."""
        for i, (_, prog, bnk, bag, *_) in enumerate(self.phdr[:-1]):
            if prog == program and bnk == bank:
                break
        else:
            raise KeyError(f'no preset {bank}:{program}')
        for pz in self._zones(self.pbag, self.pgen, bag, self.phdr[i + 1][3]):
            if 41 not in pz or not self._covers(pz, key, vel):
                continue
            n = pz[41]
            for iz in self._zones(self.ibag, self.igen, self.inst[n][1], self.inst[n + 1][1]):
                if 53 in iz and self._covers(iz, key, vel) and self.shdr[iz[53]][9] in (1, 4):
                    return self.shdr[iz[53]], iz
            for iz in self._zones(self.ibag, self.igen, self.inst[n][1], self.inst[n + 1][1]):
                if 53 in iz and self._covers(iz, key, vel):
                    return self.shdr[iz[53]], iz
        raise KeyError(f'no zone for key {key} in {bank}:{program}')

    def pcm(self, start, end):
        self.f.seek(self.smpl + 2 * start)
        raw = self.f.read(2 * (end - start))
        return [v / 32768 for v in struct.unpack(f'<{end - start}h', raw)]

    def note(self, program, key, seconds, bank=0, vel=100, fixed=False):
        """The note at SAMPLE_RATE for `seconds` (its loop run as long as
        that needs), smoothed under the new rate's half first. `fixed`
        plays the sample at its own pitch (a drum)."""
        head, gens = self.zone(program, key, bank, vel)
        _, start, end, loop0, loop1, rate, root, corr, _, _ = head

        def signed(v):
            return v - 65536 if v > 32767 else v
        start += signed(gens.get(0, 0)) + 32768 * signed(gens.get(4, 0))
        end += signed(gens.get(1, 0)) + 32768 * signed(gens.get(12, 0))
        loop0 += signed(gens.get(2, 0)) + 32768 * signed(gens.get(45, 0))
        loop1 += signed(gens.get(3, 0)) + 32768 * signed(gens.get(50, 0))
        looped = gens.get(54, 0) in (1, 3) and loop1 > loop0 + 8
        root = gens.get(58, root)
        tune = signed(gens.get(51, 0)) * 100 + signed(gens.get(52, 0)) + corr
        data = self.pcm(start, end)
        step = rate / SAMPLE_RATE * (1.0 if fixed else 2 ** ((key - root) / 12 + tune / 1200))
        # the source smoothed under the target's half, twice
        speed = step * SAMPLE_RATE / rate          # (the pitch's ratio)
        a = 1 - math.exp(-2 * math.pi * min(0.45 * SAMPLE_RATE / max(1.0, speed), rate / 2.2) / rate)
        for _ in range(2):
            y = 0.0
            for i, v in enumerate(data):
                y += a * (v - y)
                data[i] = y
        n = int(seconds * SAMPLE_RATE)
        out, pos = [], 0.0
        l0, l1 = loop0 - start, loop1 - start
        for _ in range(n):
            j = int(pos)
            if looped and j >= l1:
                pos -= l1 - l0
                j = int(pos)
            if j + 1 >= len(data):
                break
            fr = pos - j
            out.append(data[j] * (1 - fr) + data[j + 1] * fr)
            pos += step
        return out


_FONT = None


def font():
    global _FONT
    if _FONT is None:
        if not available():
            raise SystemExit(f'no soundfont at {PATH}: this piece renders in docker/Dockerfile.music\'s image')
        _FONT = SoundFont(PATH)
    return _FONT


def _finish(mix, seconds, fade, drive):
    n = min(len(mix), int(seconds * SAMPLE_RATE))
    mix = mix[:n]
    f = max(1, int(fade * SAMPLE_RATE))
    for i in range(max(0, n - f), n):
        mix[i] *= (n - i) / f
    if drive:
        peak = max(abs(v) for v in mix) or 1.0
        mix = [max(-1.0, min(1.0, v / peak * drive)) for v in mix]
    return mix


def instrument(program, keys, seconds, base=None, bank=0, vel=100, fade=0.05, drive=None, gains=None):
    """A sample of a melodic preset: one key, or a chord (several keys
    recorded as one sample, Kaida's way); `base` the key it is played at
    (the first of `keys` by default), `seconds` long, faded out at its
    end, and optionally driven into clipping by `drive`."""
    sf = font()
    notes = [sf.note(program, k, seconds, bank, vel) for k in keys]
    gains = gains or [1.0] * len(keys)
    n = max(len(x) for x in notes)
    mix = [sum(g * x[i] for g, x in zip(gains, notes) if i < len(x)) for i in range(n)]
    return Sample(_finish(mix, seconds, fade, drive), base=keys[0] if base is None else base)


def drum(key, seconds, fade=0.02, drive=None, program=0, layer=()):
    """One of the GM kit's sounds at its own pitch (bank 128), cut to
    `seconds`, optionally with other keys layered under it."""
    sf = font()
    try:
        sf.zone(program, key, 128)
    except KeyError:
        # (a soundfont without that kit: its standard kit, said aloud)
        import sys
        print(f'{FONT}: no drum kit {program}, its standard kit (0) instead', file=sys.stderr)
        program = 0
    notes = [sf.note(program, k, seconds, 128, 100, fixed=True) for k in (key, *layer)]
    n = max(len(x) for x in notes)
    mix = [sum(x[i] for x in notes if i < len(x)) for i in range(n)]
    return Sample(_finish(mix, seconds, fade, drive))
