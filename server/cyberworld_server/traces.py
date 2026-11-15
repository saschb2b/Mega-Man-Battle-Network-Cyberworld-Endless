"""The week's traces on the shared dive's layers (protocol v19, issue #589,
the owner's calls on traces; shared/online/README.md, Protocol v19).

A layer of the week's net is one for every diver at its depth, side and
way: its key is (week, seed, depth, side, way), whatever room instance a
diver stands in (dive.py). Each layer keeps, for its week:

- marks: where a dive ended, deleted or won, by whom and by what;
- phrases: one of the Square's fixed phrases left at a place, one a diver
  (a new one replaces his), each with the Waves it was given;
- first finds: who opened each Mystery Data first this week, by its index;
- the opened set: every index anyone opened this week (the "opened glow");
- splits: each identity's quickest bust of each of its battles, by the
  party's size, and from the guardians' the week's fastest of each act
  (WEEK_SPLITS).

Every list is capped at traces_most (oldest out; a battle's times at
TIMES_MOST, the slowest out) and the layers of a week at LAYERS_MOST (the
one touched longest ago out). The server appends each trace to its state
folder's `dives.jsonl` as week.py appends the Board to `weeks.jsonl`,
reads it back at the start, and keeps the weeks the Board keeps (this one
and the last): `compact` rewrites it as the traces stand, pruned to them,
at the start, by the retention's pass and by `--admin hide` and `forget`.
A moderator's hide takes that identity's marks, phrases and first finds
of that week off every layer (the opened glow stays: it names nobody)."""

from . import protocol
from .identity import rewrite_log

SPEC = protocol.SPEC
TRACE_LOG = "dives.jsonl"
MOST = SPEC["traces_most"]
LAYERS_MOST = 512       # layers of one week with traces kept
TIMES_MOST = 64         # identities' quickest busts of one battle at one party size
BASE = ("week", "seed", "depth", "side", "way", "kind", "identity", "name")


def act_of(depth):
    """The short net's act of a layer: three layers an act, the Nest the fourth (week.py's count)."""
    return min((depth - 1) // 3 + 1, (SPEC["week_layers"] - 1) // 3 + 1)


class Layer:
    def __init__(self):
        self.marks = []         # dicts, oldest first
        self.phrases = {}       # phrase id -> dict, oldest first
        self.finds = {}         # Mystery Data index -> dict (its first finder), oldest first
        self.opened = set()     # every index opened this week
        self.splits = {}        # (battle index, party) -> {identity: dict}, oldest first
        self.touched = 0


def capped(mapping, most):
    """The oldest of an insertion-ordered dict out past `most`."""
    while len(mapping) > most:
        del mapping[next(iter(mapping))]


def quicker(battle, identity, entry, most=TIMES_MOST):
    """An identity's bust kept where it is his quickest; past `most` the slowest out."""
    kept = battle.get(identity)
    if kept and kept["tenths"] <= entry["tenths"]:
        return False
    battle[identity] = entry
    if len(battle) > most:
        del battle[max(battle, key=lambda who: (battle[who]["tenths"], who))]
    return identity in battle


class Traces:
    def __init__(self):
        self.layers = {}        # (week, seed, depth, side, way) -> Layer
        self.guardians = {}     # (week, act, party) -> {identity: dict}: the guardians' busts
        self.next_phrase = 1
        self.clock = 0

    # -- keeping --

    def layer(self, key, make=True):
        layer = self.layers.get(key)
        if layer is None and make:
            week = [k for k in self.layers if k[0] == key[0]]
            if len(week) >= LAYERS_MOST:
                del self.layers[min(week, key=lambda k: self.layers[k].touched)]
            layer = self.layers[key] = Layer()
        if layer is not None:
            self.clock += 1
            layer.touched = self.clock
        return layer

    def apply(self, line):
        """A trace (a line of dives.jsonl, or one just made) kept: what it
        did (kind-dependent), or None where it is not a trace."""
        try:
            key = tuple(int(line[field]) for field in BASE[:5])
            kind, identity, name = str(line["kind"]), str(line["identity"]), str(line["name"])
            handler = {"mark": self.mark, "phrase": self.phrase, "wave": self.wave, "find": self.find,
                       "open": self.find, "split": self.split}[kind]
            return handler(key, identity, name, line)
        except (KeyError, TypeError, ValueError):
            return None

    def mark(self, key, identity, name, line):
        layer = self.layer(key)
        entry = dict(identity=identity, name=name, x=int(line["x"]), y=int(line["y"]), z=int(line["z"]),
                     cause=int(line["cause"]), how=int(line["how"]))
        layer.marks.append(entry)
        del layer.marks[:-MOST]
        return entry

    def phrase(self, key, identity, name, line):
        """A phrase left: his last on this layer replaced. What it did: (the new, the id it replaced or 0)."""
        layer = self.layer(key)
        old = next((pid for pid, p in layer.phrases.items() if p["identity"] == identity), 0)
        if old:
            del layer.phrases[old]
        pid = int(line.get("id") or self.next_phrase)
        self.next_phrase = max(self.next_phrase, pid + 1) % 0xffffffff or 1
        entry = dict(id=pid, identity=identity, name=name, phrase=int(line["phrase"]), x=int(line["x"]),
                     y=int(line["y"]), z=int(line["z"]), waves=int(line.get("waves", 0)))
        layer.phrases[pid] = entry
        gone = [old] if old else []
        while len(layer.phrases) > MOST:
            gone.append(next(iter(layer.phrases)))
            del layer.phrases[gone[-1]]
        return entry, gone

    def wave(self, key, identity, name, line):
        layer = self.layer(key, make=False)
        entry = layer.phrases.get(int(line["id"])) if layer else None
        if not entry:
            return None
        entry["waves"] = min(entry["waves"] + 1, 0xffff)
        return entry

    def find(self, key, identity, name, line):
        """A Mystery Data opened: the first finder of the week kept by name. What it did: (index, first or None)."""
        layer = self.layer(key)
        index = int(line["index"])
        if not 0 <= index <= 255:
            raise ValueError("index")
        first = None
        if index not in layer.opened and identity:
            first = layer.finds[index] = dict(identity=identity, name=name)
            capped(layer.finds, MOST)
        layer.opened.add(index)
        return index, first

    def split(self, key, identity, name, line):
        layer = self.layer(key)
        index, party, tenths = int(line["index"]), int(line["party"]), int(line["tenths"])
        if not (0 <= index <= 255 and 1 <= party <= SPEC["party_most"] and 1 <= tenths <= 0xffff):
            raise ValueError("split")
        entry = dict(name=name, tenths=tenths, level=int(line["level"]), guardian=bool(line["guardian"]))
        battle = layer.splits.pop((index, party), {})
        layer.splits[(index, party)] = battle
        capped(layer.splits, MOST)
        kept = quicker(battle, identity, entry)
        if entry["guardian"]:
            quicker(self.guardians.setdefault((key[0], act_of(key[2]), party), {}), identity, dict(entry))
        return kept

    # -- reading --

    def place(self, key, index, party, tenths, shown):
        """A bust's place among the week's at that battle and party size, how
        many busted it, and the quickest (tenths, name); `shown` an identity's not hidden."""
        layer = self.layers.get(key)
        battle = {who: e for who, e in (layer.splits.get((index, party), {}) if layer else {}).items() if shown(who)}
        place = 1 + sum(e["tenths"] < tenths for e in battle.values())
        best = min(battle.values(), key=lambda e: e["tenths"], default=None)
        return place, len(battle), (best["tenths"], best["name"]) if best else (0, "")

    def fastest(self, week, shown):
        """The week's fastest bust of each act's guardian, alone and by party size: 16 (tenths, name), (0, "") none."""
        out = []
        for act in range(1, 5):
            for party in range(1, SPEC["party_most"] + 1):
                busts = [e for who, e in self.guardians.get((week, act, party), {}).items() if shown(who)]
                best = min(busts, key=lambda e: e["tenths"], default=None)
                out.append((best["tenths"], best["name"]) if best else (0, ""))
        return out

    def entries(self, key, shown):
        """A layer's traces as DIVE_TRACES's messages, `shown` an identity's
        not hidden: its marks, phrases, first finds, the opened set and its
        battles' quickest busts."""
        layer = self.layers.get(key)
        if not layer:
            return []
        name = protocol.name16
        marks = [protocol.TRACE_MARK.pack(name(m["name"]), m["x"], m["y"], m["z"], m["cause"], m["how"])
                 for m in layer.marks if shown(m["identity"])]
        phrases = [phrase_entry(p) for p in layer.phrases.values() if shown(p["identity"])]
        finds = [protocol.TRACE_FIND.pack(index, name(f["name"])) for index, f in layer.finds.items() if shown(f["identity"])]
        opened = [bytes((index,)) for index in sorted(layer.opened)]
        splits = []
        for (index, party), battle in layer.splits.items():
            busts = [(e["tenths"], e) for who, e in battle.items() if shown(who)]
            if busts:
                splits.append(split_entry(index, party, min(busts, key=lambda b: b[0])[1]))
        out = []
        for kind, packed in ((SPEC["trace_marks"], marks), (SPEC["trace_phrases"], phrases), (SPEC["trace_finds"], finds),
                             (SPEC["trace_opened"], opened), (SPEC["trace_splits"], splits)):
            out += protocol.dive_traces(kind, packed)
        return out

    def lines(self, shown_in_week):
        """Every trace kept, as dives.jsonl's lines (the waves within their phrases)."""
        out = []
        for key, layer in self.layers.items():
            shown = shown_in_week(key[0])
            base = dict(zip(BASE[:5], key))
            out += [dict(base, kind="mark", **m) for m in layer.marks if shown(m["identity"])]
            out += [dict(base, kind="phrase", **p) for p in layer.phrases.values() if shown(p["identity"])]
            named = {index for index, f in layer.finds.items() if shown(f["identity"])}
            out += [dict(base, kind="find", index=index, **layer.finds[index]) for index in layer.finds if index in named]
            out += [dict(base, kind="open", index=index, identity="", name="") for index in sorted(layer.opened - named)]
            for (index, party), battle in layer.splits.items():
                out += [dict(base, kind="split", identity=who, index=index, party=party, **e)
                        for who, e in battle.items() if shown(who)]
        return out


def phrase_entry(p):
    return protocol.TRACE_PHRASE.pack(p["id"], protocol.name16(p["name"]), p["phrase"], p["x"], p["y"], p["z"], p["waves"])


def split_entry(index, party, e):
    flags = SPEC["split_guardian"] if e["guardian"] else 0
    return protocol.TRACE_SPLIT.pack(index, party, e["tenths"], e["level"], flags, protocol.name16(e["name"]))


def compact(directory, first_week, forget=None, hide=None):
    """dives.jsonl as its traces stand, from `first_week` on: `forget`'s
    identity out of every week, `hide` ((identity, week)) out of that week.
    The lines kept and how many went."""
    def keep(entries):
        traces = Traces()
        for entry in entries:
            try:
                if int(entry["week"]) < first_week:
                    continue
                if entry.get("identity") == forget or (hide and entry.get("identity") == hide[0] and int(entry["week"]) == hide[1]):
                    # (the opened glow names nobody: it stays)
                    if entry.get("kind") != "find":
                        continue
                    entry = dict(entry, kind="open", identity="", name="")
            except (KeyError, TypeError, ValueError):
                continue
            traces.apply(entry)
        return traces.lines(lambda week: lambda who: True)
    return rewrite_log(directory, TRACE_LOG, keep)
