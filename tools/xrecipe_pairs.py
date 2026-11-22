"""xrecipe_pairs.py: a BN5 routine's twin in BN6, for tools/xrecipe.py (issue #82): the project's own pairs first
(tools/bn5_twins.py), then the symbol map's (tools/bn6f_match.py's tables in .build/symbols/match, by bn6f's names: names
stay there, never here), then a BN6 routine of the same body (its jump tables' entries the same bodies too), then the
same body where the routine's neighbours put it. Each pair is classed by its two bodies (xrecipe_thumb.compare): SAME,
LAYOUT (but for the offsets of loads and stores), DIFF (its audit in bn5_twins.AUDITS, or a decision)."""
import bisect
import csv
import functools
import hashlib
import os
import pickle

import bn5_twins
import xrecipe_thumb as T

BUS = T.BUS


class Pairs:
    """The two ROMs' routines paired; `cache` a folder for the BN6 index (the bodies of every routine found)"""

    def __init__(self, b5, b6, match_dir, cache, search=True):
        self.b5, self.b6, self.search = b5, b6, search
        self.cache = cache
        self.twins = {a5: a6 for a5, a6, _ in bn5_twins.TWINS}
        self.twin_src = {a5: src for a5, _, src in bn5_twins.TWINS}
        self.by5, self.by6name, self.map_ok = self.read_map(match_dir)
        self._index = None
        self._anchors = None
        self.memo = {}

    # ---- the symbol map: bn6f's functions located in both ROMs (kept out of git) ----

    @staticmethod
    def read_map(folder):
        by5, by6 = {}, {}
        paths = [os.path.join(folder, n) for n in ('bn6f-bn5.csv', 'bn6f-gregar.csv')]
        if not all(os.path.exists(p) for p in paths):
            return by5, by6, False
        for path, out, key in ((paths[0], by5, 'addr'), (paths[1], by6, 'name')):
            with open(path, newline='', encoding='utf-8') as f:
                for r in csv.DictReader(f):
                    if r['kind'] != 'function' or not r['address']:
                        continue
                    a = int(r['address'], 16) & ~1
                    if key == 'addr':
                        out[a] = (r['name'], r['confidence'])
                    else:
                        out.setdefault(r['name'], a)
        return by5, by6, True

    def map_twin(self, a5):
        """BN6's routine of the bn6f name the symbol map gives BN5's (none for a row the project knows wrong)"""
        r = self.by5.get(a5)
        if not r or a5 in bn5_twins.WRONG_ROWS or r[1] == 'twin':
            return None, None
        a6 = self.by6name.get(r[0].split('+')[0])
        return a6, r[1]

    # ---- BN6's routines by their bodies ----

    def bn6_entries(self):
        """Every BN6 routine entry to search: the symbol map's functions, the object types' routines, the per-family
        tables' entries and the state tables' states"""
        ents = set(self.by6name.values())
        for tab, n in ((0x08003C9C, 95), (0x08003EC4, 213), (0x080042C8, 146)):
            for k in range(n):
                v = self.b6.u32(tab + 4 * k)
                if T.is_code_ptr(v, self.b6):
                    ents.add(v & ~1)
        for k in range(11):
            for ai in range(32):
                v = self.b6.u32(0x0810A378 + 0x80 * k + 4 * ai)
                if k == 0 and self.b6.has(v, 4):
                    for j in range(16):
                        x = self.b6.u32(v + 4 * j)
                        if not T.is_code_ptr(x, self.b6):
                            break
                        ents.add(x & ~1)
                elif k >= 3 and T.is_code_ptr(v, self.b6):
                    ents.add(v & ~1)
        return ents

    def index(self):
        """{body's hash: [BN6 routines of that body]}, kept in the cache by the ROMs' hashes"""
        if self._index is not None:
            return self._index
        key = hashlib.sha1(self.b6.data[:0x800000] + repr(sorted(self.by6name.values())).encode()).hexdigest()[:16]
        path = os.path.join(self.cache, f'bn6-bodies-{key}.pkl')
        if os.path.exists(path):
            with open(path, 'rb') as f:
                self._index = pickle.load(f)
            return self._index
        idx = {}
        for e in sorted(self.bn6_entries()):
            n, _ = T.norm(self.b6, e)
            if len(n) >= 3:
                idx.setdefault(body_key(n), []).append(e)
        os.makedirs(self.cache, exist_ok=True)
        with open(path, 'wb') as f:
            pickle.dump(idx, f)
        self._index = idx
        return idx

    @functools.lru_cache(maxsize=None)
    def deep_same(self, a5, a6, depth=3):
        """the two routines' bodies the same, their known calls each other's twins, and their jump tables' entries the
        same routines too"""
        n5, w5 = T.norm(self.b5, a5)
        n6, w6 = T.norm(self.b6, a6)
        if n5 != n6 or not self.bls_agree(w5, w6):
            return False
        if depth <= 0:
            return True
        t5 = self.tables_of(self.b5, w5)
        t6 = self.tables_of(self.b6, w6)
        if len(t5) != len(t6):
            return False
        for x, y in zip(t5, t6):
            if len(x) != len(y) or not all(self.deep_same(p, q, depth - 1) for p, q in zip(x, y)):
                return False
        return True

    @staticmethod
    def tables_of(rom, w):
        out = []
        for v in sorted(v for v in w.lits.values() if v and not v & 3 and rom.has(v, 4) and T.is_code_ptr(rom.u32(v), rom)):
            ents = []
            for k in range(16):
                x = rom.u32(v + 4 * k)
                if not T.is_code_ptr(x, rom):
                    break
                ents.append(x & ~1)
            out.append(ents)
        return out

    def bls_agree(self, w5, w6):
        s5 = [w5.bls[a] for a in sorted(w5.bls)]
        s6 = [w6.bls[a] for a in sorted(w6.bls)]
        if len(s5) != len(s6):
            return False
        for t5, t6 in zip(s5, s6):
            k = self.known(t5)
            if k is not None and k != t6:
                return False
        return True

    def known(self, a5):
        """a BN5 routine's twin by the project's pairs and the symbol map alone"""
        if a5 in self.twins:
            return self.twins[a5]
        return self.map_twin(a5)[0]

    # ---- where BN6 likely keeps a BN5 routine ----

    def expected(self, a5):
        if self._anchors is None:
            self._anchors = sorted((a, self.by6name[r[0]]) for a, r in self.by5.items()
                                   if r[1] in ('bytes', 'calls+bytes') and r[0] in self.by6name)
            self._anchors.append((0x08104AC0, 0x0810A8F8))   # (the shared virus code follows the per-family tables in both)
            self._anchors.sort()
        k = bisect.bisect_right(self._anchors, (a5, 1 << 40)) - 1
        return None if k < 0 else self._anchors[k][1] + (a5 - self._anchors[k][0])

    def near(self, a5):
        exp = self.expected(a5)
        out = []
        if exp:
            for d in range(0, 0x200, 2):
                for c in (exp + d, exp - d):
                    if self.b6.has(c, 2) and c not in out and (self.b6.u16(c) & 0xFF00 == 0xB500 or self.b6.u16(c) == self.b5.u16(a5)):
                        out.append(c)
        # (an entry inside a routine the map pairs: the same distance into it)
        for back in range(2, 0x40, 2):
            r = self.by5.get(a5 - back)
            if r and r[0] in self.by6name:
                out.insert(0, self.by6name[r[0]] + back)
                break
        return out[:64]

    # ---- a twin ----

    def twin(self, a5):
        """(BN6's address or None, how it was found, its class (SAME, LAYOUT, DIFF) or None, the difference)"""
        a5 &= ~1
        if a5 in self.memo:
            return self.memo[a5]
        out = self._twin(a5)
        self.memo[a5] = out
        return out

    def _twin(self, a5):
        if a5 in self.twins:
            a6 = self.twins[a5]
            c, d, _ = T.compare(self.b5, a5, self.b6, a6)
            return a6, 'project:' + self.twin_src[a5], c, d
        a6, conf = self.map_twin(a5)
        if a6:
            c, d, _ = T.compare(self.b5, a5, self.b6, a6)
            return a6, 'map:' + conf, c, d
        if not self.search:
            return None, 'none', None, None
        n, _ = T.norm(self.b5, a5)
        if len(n) >= 3:
            hits = [h for h in self.index().get(body_key(n), []) if self.deep_same(a5, h)]
            if len(hits) == 1:
                return hits[0], 'search', 'SAME', None
            if hits:
                exp = self.expected(a5)
                near = sorted(hits, key=lambda h: abs(h - exp)) if exp else []
                if near and abs(near[0] - exp) < 0x400 and (len(near) == 1 or abs(near[1] - exp) > abs(near[0] - exp) + 0x100):
                    return near[0], 'search, by place', 'SAME', None
                return None, 'ambiguous: ' + ', '.join(f'{h:08X}' for h in hits[:6]), None, None
        for c in self.near(a5):
            if self.deep_same(a5, c):
                return c, 'near', 'SAME', None
        return None, 'none', None, None

    def audit(self, a5, a6):
        """the pair's audit (verdict, why, conditions), or None"""
        return bn5_twins.AUDITS.get((a5 & ~1, a6 & ~1))


def body_key(n):
    return hashlib.sha1(repr(n).encode()).hexdigest()
