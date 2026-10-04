#!/usr/bin/env python3
"""symbols.py [--check | --full]: what this project has mapped of Mega Man
Battle Network 6: Cybeast Gregar (USA) and Battle Network 5: Team Colonel
(USA), written for others to use (docs/SYMBOLS.md): per ROM a symbol file
that mGBA's and no$gba's debuggers load, and the same symbols as JSON and
CSV.

Read from what the engine already keeps, so the files follow it:
src/emu/bn6.h and bn5.h (RAM and code addresses, the fields of the game's
structures, event flags and values, each with its comment), the ROM
offsets of RomLayout and XRomLayout in src/core/rom.c (rom.h's comments
say what each is) and docs/ROM_DATA.md (each table: where, how it was
found, how to verify it). Needs no ROM.

With --full, also the full map, for this machine only: the same files with
the bn6f disassembly's functions and labels that tools/bn6f_match.py
located in the ROMs (its tables in .build/symbols/match), written to
.build/symbols with a report of what was located; and SYMBOLS.md's numbers
of it, numbers only. bn6f states no license, so nothing of its names goes
into git (docs/SYMBOLS.md).

  python3 tools/symbols.py           write docs/symbols/ and SYMBOLS.md's numbers
  python3 tools/symbols.py --check   write nothing: fail where a name in bn6.h or
                                     bn5.h has no description, or a file in
                                     docs/symbols differs from what it would write
                                     (build.py lint; needs no bn6f)
  python3 tools/symbols.py --full    docs/symbols as above, and .build/symbols
                                     (build.py symbols --bn6f DIR runs the matcher first)
"""
import bisect
import copy
import csv
import io
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = 'docs/symbols'
FULL = '.build/symbols'           # the full map: never committed
MATCH = '.build/symbols/match'    # tools/bn6f_match.py's tables
DOC = 'docs/SYMBOLS.md'
ROM_DATA = 'docs/ROM_DATA.md'

# The two ROMs: the header with their addresses, rom.c's layout and its
# entry, the files' stem, the game as the tables name it and its short
# title, tools/bn6f_match.py's table.
GAMES = (
    dict(key='bn6', stem='bn6-gregar-us', header='src/emu/bn6.h', layout=('RomLayout', 'layouts', 'ROM_BN6_GREGAR_US'),
         game='BN6 Cybeast Gregar (USA)', title='BN6 Gregar', match='bn6f-gregar.csv'),
    dict(key='bn5', stem='bn5-colonel-us', header='src/emu/bn5.h', layout=('XRomLayout', 'xlayouts', 'XROM_BN5_COLONEL_US'),
         game='BN5 Team Colonel (USA)', title='BN5 Team Colonel', match='bn6f-bn5.csv'),
)

# The game's structures whose fields the headers name by their offset alone:
# a define's name prefix, the define of the structure it belongs to (its
# address, or the pointer field that points at it) or None for a record
# type found in many places, and the structure's type in bn6f
# (include/structs/). In a family, a define named _SIZE is the structure's
# size and _COUNT how many there are; one whose comment opens with another
# define's name and a colon is a value of that one, not a field.
FAMILIES = (
    ('BN6_TOOLKIT_', 'BN6_TOOLKIT', 'Toolkit'),
    ('BN6_STEPS_', 'BN6_TOOLKIT_STEPS', 'S2001c04'),
    ('BN6_NPC_', 'BN6_NPC_OBJECTS', 'OverworldNPCObject'),
    ('BN6_PANEL_', 'BN6_FIELD_PANELS', 'PanelData'),
    ('BN6_T1_', 'BN6_T1_OBJECTS', 'BattleObject'),
    ('BN6_TALK_PROBE_', 'BN6_TALK_PROBES', None),
    ('BN5_TOOLKIT_', 'BN5_TOOLKIT', 'Toolkit'),
    ('BN5_T1_', 'BN5_T1_OBJECTS', 'BattleObject'),
    ('BN5_T3_', 'BN5_T3_OBJECTS', 'BattleObject'),
    ('BN5_PANEL_', 'BN5_FIELD_PANELS', 'PanelData'),
    ('BN5_BATTLE_NAVI_', 'BN5_BATTLE_NAVI', 'NaviStats'),
    ('BN5_CHIP_KIND', 'BN5_CHIP_RECORDS', 'ChipData'),
    ('BN5_RECORD_', None, 'BattleSettings'),
)
# a define that is another one's size in bytes
SIZE_OF = {'BN6_EVENT_FLAG_BYTES': 'BN6_EVENT_FLAGS'}

# where on the bus an address lies
REGIONS = ((0x02000000, 0x02040000, 'ram'), (0x03000000, 0x03008000, 'ram'), (0x04000000, 0x04000400, 'io'),
           (0x08000000, 0x08800000, 'rom'))

# the tables' columns; the full map's add bn6f's name, its Falzar address
# and how it was located
COLUMNS = ('game', 'rom_sha1', 'name', 'kind', 'address', 'base', 'offset', 'size', 'count', 'value', 'description',
           'verified', 'how', 'source')
FULL_COLUMNS = COLUMNS + ('bn6f', 'falzar', 'confidence')


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8') as f:
        return f.read()


def clean(text):
    """A comment's words on one line."""
    return re.sub(r'\s+', ' ', text.replace('\n', ' ')).strip()


def hx(v, digits=8):
    return f'0x{v:0{digits}X}'


# ---- the headers ----

def header_items(path):
    """bn6.h's or bn5.h's defines and enumerators, in order: name, value text,
    its own comment (None), its line, the block comment over its run of
    defines (a run ends at a blank line), and the run before it."""
    lines = read(path).split('\n')
    items, run, block = [], [], None
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        no = i + 1
        if not line:
            run, block = [], None
        elif line.startswith('/*'):
            text = line
            while '*/' not in text and i + 1 < len(lines):
                i += 1
                text += '\n' + lines[i].strip().lstrip('*')
            run, block = [], clean(text[2:text.index('*/')])
        elif line.startswith('#define '):
            m = re.match(r'#define\s+(\w+)\s*(.*)$', line)
            name, rest, comment = m.group(1), m.group(2), None
            if '/*' in rest:
                rest, text = rest.split('/*', 1)
                while '*/' not in text and i + 1 < len(lines):
                    i += 1
                    text += '\n' + lines[i].strip().lstrip('*')
                comment = clean(text[:text.index('*/')])
            if re.fullmatch(r'BN[56]_\w+', name) and not name.endswith('_H'):
                item = dict(name=name, value=rest.strip(), comment=comment, line=no, block=block, run=list(run), path=path)
                items.append(item)
                run.append(item)
        elif line.startswith('enum'):
            m = re.match(r'enum\s*\{(.*)\}\s*;', line)
            n = 0
            for part in m.group(1).split(','):
                part = part.strip()
                if '=' in part:
                    part, v = (p.strip() for p in part.split('='))
                    n = int(v, 0)
                item = dict(name=part, value=str(n), comment=None, line=no, block=block, run=list(run), path=path)
                items.append(item)
                run.append(item)
                n += 1
        i += 1
    return items


def mentions(text, name, by):
    """Whether the comment of define `by` names define `name`: in full, or by
    the words of its name past those the two share, as a field's name is
    written (BN6_PLAYER_Y in X's "X, Y and Z", BN6_NAVI_MAX_HP in "MaxHP")."""
    if re.search(r'\b' + re.escape(name) + r'\b', text):
        return True
    a, b = name.split('_'), by.split('_')
    common = 0
    while common < min(len(a), len(b)) - 1 and a[common] == b[common]:
        common += 1
    for k in range(max(1, common - 1), len(a)):
        for form in ('_'.join(a[k:]), ''.join(a[k:])):
            for m in re.finditer(r'(?<![\w])' + re.escape(form) + r'(?![\w])', text, re.I):
                if m.group(0)[0].isupper() or m.group(0)[0].isdigit():
                    return True
    return False


def describe(items):
    """Each item's description: its own comment, joined with the comments it
    continues ("..." opens a comment that goes on from the one above); with
    none, the comment of a define above it in its run that names it, or the
    block comment over the run where it comes before any commented define.
    Returns the names that have none."""
    chains = {}
    for it in items:
        c, prev = it['comment'], (it['run'][-1] if it['run'] else None)
        if c and c.startswith('...') and prev is not None and prev['comment']:
            chain = chains.get(prev['name']) or [prev]
            chain.append(it)
            for member in chain:
                chains[member['name']] = chain
    for it in items:
        parts = []
        for member in chains.get(it['name'], [it]):
            part = member['comment'] or ''
            part = part[3:].strip() if part.startswith('...') else part
            parts.append(part[:-3].rstrip() if part.endswith('...') else part)
        it['description'] = ' ... '.join(p for p in parts if p) or None
    missing = []
    for it in items:
        if it['description']:
            continue
        for other in reversed(it['run']):
            if other['comment'] and mentions(other['description'], it['name'], other['name']):
                it['description'] = other['description']
                break
        else:
            if it['block'] and not any(o['comment'] for o in it['run']):
                it['description'] = it['block']
            else:
                missing.append(f"{it['path']}:{it['line']}: {it['name']} has no description")
    return missing


def c_number(text, known):
    """A C constant expression as a number, the names in it from `known`;
    None where it names one not known (yet)."""
    expr = re.sub(r'\b(0x[0-9A-Fa-f]+|\d+)[uUlL]+\b', r'\1', text.strip())
    for n in re.findall(r'\b[A-Za-z_]\w*\b', expr):
        if not isinstance(known.get(n), int):
            return None
        expr = re.sub(r'\b' + n + r'\b', str(known[n]), expr)
    if not expr or not re.fullmatch(r'[\s0-9a-fA-FxX()+\-*<>|&~]*', expr):
        return None
    return int(eval(expr, {'__builtins__': {}}))   # (digits and operators alone)


def evaluate(items):
    """Every define's value, the names they use resolved (bn5.h names
    BN5_BATTLE_STATE before it defines it)."""
    known = {}
    for _ in range(4):
        for it in items:
            v = it['value']
            if v.startswith('{'):
                known[it['name']] = [c_number(t, known) for t in v.strip('{} ').split(',') if t.strip()]
            else:
                known[it['name']] = c_number(v, known)
    return known


def width(description):
    """A field's size from the u8, u16, s32... its comment opens with (what
    the comment says before its first colon, its parentheses left out)."""
    head = re.sub(r'\([^()]*\)', '', description or '').split(':')[0]
    m = re.search(r'\b[us](8|16|32)\b', head)
    return int(m.group(1)) // 8 if m else None


def header_symbols(game):
    """The header's symbols, by name: addresses (RAM, I/O, ROM), the fields of
    the game's structures, event flags, values and constants. With the
    names that have no description."""
    items = header_items(game['header'])
    missing = describe(items)
    known = evaluate(items)
    names = {it['name'] for it in items}
    syms = {}
    for it in items:
        name, value, desc = it['name'], known[it['name']], it['description']
        s = dict(name=name, description=desc, source=f"{it['path']}:{it['line']}", origin='header', own=it['comment'])
        syms[name] = s
        m_field = re.fullmatch(r'\(\s*(BN[56]_\w+)\s*\+\s*(0x[0-9A-Fa-f]+|\d+)\s*\)', it['value'])
        m_value = re.match(r'(BN[56]_\w+)(?: \(its \+\w+\))?:', desc or '')
        family = next((f for f in FAMILIES if name.startswith(f[0])), None)
        if '_FLAG_' in name:
            s.update(kind='flag', value=value)
        elif isinstance(value, list):
            for k, v in enumerate(value):
                syms[f'{name}.{k}'] = dict(s, name=f'{name}.{k}', kind='rom', address=v)
            del syms[name]
        elif m_field:
            s.update(kind='field', base=m_field.group(1), offset=int(m_field.group(2), 0), size=width(desc))
            if value is not None:
                s['address'] = value
        elif it['value'] in names:
            s.update(kind='alias', of=it['value'])
        elif m_value and m_value.group(1) in names and m_value.group(1) != name:
            s.update(kind='value', value=value, of=m_value.group(1))
        elif name in SIZE_OF:
            s.update(kind='size', value=value, of=SIZE_OF[name])
        elif value is not None and value < 0x01000000 and (desc or '').startswith('(ROM offset'):
            s.update(kind='rom', address=0x08000000 + value)
        elif value is not None and any(a <= value < b for a, b, k in REGIONS):
            s.update(kind=next(k for a, b, k in REGIONS if a <= value < b), address=value)
        elif family and family[1] != name and name.endswith(('_SIZE', '_COUNT')):
            s.update(kind='size' if name.endswith('_SIZE') else 'count', value=value, of=family[1] or family[2])
        elif family and family[1] != name:
            s.update(kind='field', base=family[1] or family[2], offset=value, type=family[2], size=width(desc))
        else:
            s.update(kind='constant', value=value)
    return syms, missing


# ---- rom.c: RomLayout's and XRomLayout's ROM offsets ----

def struct_fields(text, typename):
    """A typedef'd struct's fields in order, from rom.h: (name, C type, array
    length or None, comment); a nested struct is one field whose type is the
    list of its own."""
    end = text.index('\n} ' + typename + ';')
    body = text[text.rindex('typedef struct {', 0, end) + len('typedef struct {'):end]

    def decls(lines):
        out, i = [], 0
        while i < len(lines):
            line = lines[i]
            comment = re.search(r'/\*(.*?)\*/', line)
            code = re.sub(r'/\*.*?\*/', '', line).strip()
            i += 1
            if code.startswith('struct {'):
                inner = []
                while not lines[i].strip().startswith('}'):
                    inner.append(lines[i])
                    i += 1
                name = re.match(r'\}\s*(\w+);', lines[i].strip()).group(1)
                out.append((name, decls(inner), None, clean(comment.group(1)) if comment else ''))
                i += 1
                continue
            m = re.match(r'(const char \*|[\w ]+?)\s*(\*?\w+(?:\[\w+\])?(?:\s*,\s*\*?\w+(?:\[\w+\])?)*)\s*;', code)
            if m:
                for d in m.group(2).split(','):
                    am = re.match(r'(\w+)(?:\[(\w+)\])?', d.strip().lstrip('*'))
                    out.append((am.group(1), m.group(1).strip(), am.group(2), clean(comment.group(1)) if comment else ''))
        return out
    return decls(body.split('\n'))


def split_top(text):
    """An initializer's elements at the top level, with their offsets in text."""
    parts, depth, start, quote = [], 0, 0, False
    for i, ch in enumerate(text):
        if ch == '"':
            quote = not quote
        if quote:
            continue
        if ch in '{(':
            depth += 1
        elif ch in '})':
            depth -= 1
        elif ch == ',' and depth == 0:
            parts.append((text[start:i], start))
            start = i + 1
    if text[start:].strip():
        parts.append((text[start:], start))
    return parts


def slots(fields, text, at, prefix=''):
    """(name, C type, value text, its offset in the file, comment) of each
    scalar an initializer gives a struct's fields: positional elements in
    the fields' order, or .name = designated ones; arrays as name.0, name.1."""
    out = []
    for k, (part, off) in enumerate(split_top(text)):
        part_at = at + off + len(part) - len(part.lstrip())
        part = part.strip()
        dm = re.match(r'\.(\w+)\s*=\s*', part)
        if dm:
            field = next((f for f in fields if f[0] == dm.group(1)), None)
            part, part_at = part[dm.end():], part_at + dm.end()
        else:
            field = fields[k] if k < len(fields) else None
        if field is None:
            continue
        name, ctype, count, comment = field
        if isinstance(ctype, list):
            out += slots(ctype, part[1:-1], part_at + 1, prefix + name + '.')
        elif count is not None and part.startswith('{'):
            for j, (sub, soff) in enumerate(split_top(part[1:-1])):
                out.append((f'{prefix}{name}.{j}', ctype, sub.strip(), part_at + 1 + soff + len(sub) - len(sub.lstrip()), comment))
        else:
            out.append((prefix + name, ctype, part, part_at, comment))
    return out


def layout_symbols(game):
    """RomLayout's (BN6) or XRomLayout's (BN5) ROM offsets in rom.c, with the
    ROM's name, SHA-1 and header code."""
    src, hdr = read('src/core/rom.c'), read('src/core/rom.h')
    typename, array, entry = game['layout']
    fields = struct_fields(hdr, typename)
    begin = src.index(f'[{entry}] = {{', src.index(f'{typename} {array}')) + len(f'[{entry}] = {{')
    depth, end = 1, begin
    while depth:
        depth += {'{': 1, '}': -1}.get(src[end], 0)
        end += 1
    info, syms = {}, {}
    for name, ctype, text, at, comment in slots(fields, src[begin:end - 1], begin):
        if name in ('name', 'sha1', 'code'):
            info[name] = text.strip('"')
        v = c_number(text, {}) if ctype == 'uint32_t' else None
        if v is not None:
            full = f'{typename}.{name}'
            syms[full] = dict(name=full, kind='rom', address=0x08000000 + v, description=comment,
                              source=f'src/core/rom.c:{src.count(chr(10), 0, at) + 1}', origin='layout')
    return info, syms


# ---- docs/ROM_DATA.md ----

def sentences(text):
    return [s.strip() for s in re.split(r'(?<=[.!?])\s+(?=[A-Z`(])', text) if s.strip()]


def how_verified(text):
    """A row's own words on how to verify it, else on how it was found."""
    ss = sentences(text)
    for s in ss:
        if re.match(r'(Verif|Check)', s):
            return s
    for s in ss:
        if re.search(r'\b(found|Found|verified|Verified|tested|traced|Traced)\b', s):
            return s
    return ''


def rom_data_rows():
    """docs/ROM_DATA.md's rows: line, title, the addresses the Offset cell
    gives (game, address, what it says of it) and the names the row puts in
    code spans."""
    rows = []
    for no, line in enumerate(read(ROM_DATA).split('\n'), 1):
        if not line.startswith('|') or line.startswith('| ---') or line.startswith('| Data |'):
            continue
        cells = [c.strip() for c in re.split(r'(?<!\\)\|', line)[1:-1]]
        if len(cells) < 3:
            continue
        title = re.sub(r'\s*\([^()]*(`|issue #)[^()]*\)\s*$', '', cells[0]).strip()
        title = re.sub(r'`', '', re.sub(r':? Battle Network 5 Team Colonel \(USA\)', '', title))
        rows.append(dict(line=no, title=title, entries=offset_entries(cells[1]),
                         names=set(re.findall(r'`(BN[56]_\w+)`', line)), how=how_verified(cells[2]), text=line))
    return rows


ADDRESS = r'`(0x[0-9A-Fa-f]{6}|0x[0-9A-Fa-f]{8})`'
NOT_LABELS = {'', 'and', 'or', 'at', 'its', 'their', 'bn5', 'bn6', 'gregar', "gregar's", "bn5's", "bn6's", 'ewram', 'ram',
              'literals', 'rom'}


def no_label(text):
    return text.lower() in NOT_LABELS or bool(re.match(r'[=+×x\d]|at\b', text))


def label_of(cell, m):
    """What an Offset cell says of the address at match m: the words after it
    to the next comma, semicolon or parenthesis (past a parenthesis right
    after it: "(its table ...) the HP bug a use adds"), else what that
    parenthesis says, else the words before it. Returns the label, where it
    was found ('after', 'paren', 'before' or ''), the names in code spans
    beside it and whether the address opens a range (A-B)."""
    after = cell[m.end():]
    # (a pair or a range: A/B, A-B, A and B take the words after B)
    pair = re.match(r'\s*(?:/|-|and)\s*' + ADDRESS, after)
    if pair:
        after = after[pair.end():]
    if re.match(r'\s*,?\s*(written|was)\b', after):
        return None, '', [], False   # (an address the row says was taken for another)
    right = re.match(r'\s*\(([^()]*)\)', after)
    label = re.split(r'[,;()]|`0x', after[right.end():] if right else after, maxsplit=1)[0].strip(' -/')
    where = 'after' if label and not no_label(label) else ''
    if not where and right and not re.search(ADDRESS, right.group(1)):
        inner = re.sub(r'`[^`]*`', '', right.group(1)).strip(' ,;')
        label, where = (inner, 'paren') if inner and not no_label(inner) else ('', '')
    if not where:
        before = re.split(r'[,;()]', cell[:m.start()])[-1].strip()
        before = re.sub(r'^(BN5|BN6|Gregar)(\'s)?\b\s*', '', before)
        before = re.sub(r'\s+(at|in|of)$', '', before).strip()
        label, where = (before, 'before') if before and not no_label(before) else ('', '')
    paren = re.match(r'\s*[^,;()`]*\(([^()]*)\)', after)
    names = [n for n in re.findall(r'`([A-Za-z_]\w*)`', paren.group(1))] if paren else []
    return label.strip(), where, names, bool(pair and '-' in pair.group(0)[:3])


def offset_entries(cell):
    """The addresses an Offset cell gives: (game, address, its words, the names
    beside it). A cell that opens with "BN5" gives BN5's; in each clause (up
    to a ;) a "BN5", "BN6" or "Gregar" turns to that game's. Words before a
    list name each address of it that has none of its own ("DarkInvs `a`,
    `b`, RAM `c`"); "its" before an address is the one before outside
    parentheses ("`a` (its table `b`) the HP bug a use adds": b is the HP
    bug's table)."""
    entries, first = [], 'bn5' if cell.startswith('BN5') else 'bn6'
    for clause_at, clause in clauses(cell):
        game, skip, lead, last, end = first, False, '', '', None
        for m in re.finditer(r'\b(BN5|BN6|Gregar)\b|' + ADDRESS, clause):
            if m.group(1):
                game = 'bn5' if m.group(1) == 'BN5' else 'bn6'
                continue
            listed = end is not None and re.fullmatch(r'[\s,]*(RAM\s*)?', clause[end:m.start()])
            end = m.end()
            if skip:
                skip = False
                continue
            text = m.group(2)
            v = int(text, 16)
            if len(text) == 8:
                v += 0x08000000
                if v >= 0x08800000:
                    continue
            label, where, names, ranged = label_of(clause, m)
            if where == 'before' and last and re.match(r'its\b', label):
                label = f'{last}, {label}'
            if where:
                lead = label if where == 'before' else ''
            elif lead and listed:
                label = lead
            else:
                lead = ''
            if label is not None:
                entries.append(dict(game=game, address=v, label=label, names=names, written='offset' if len(text) == 8 else 'bus'))
                outside = clause[:m.start()].count('(') == clause[:m.start()].count(')')
                last = label if label and outside else last
            skip = ranged
    return entries


def clauses(cell):
    """A cell's clauses: split at the semicolons outside parentheses."""
    out, depth, start = [], 0, 0
    for i, ch in enumerate(cell):
        depth += {'(': 1, ')': -1}.get(ch, 0)
        if ch == ';' and depth == 0:
            out.append((start, cell[start:i]))
            start = i + 1
    out.append((start, cell[start:]))
    return out


def slug(label, most=6):
    words = re.findall(r'[A-Za-z0-9]+', re.sub(r"'s\b", '', label))
    words = [w for w in words if w.lower() not in ('the', 'its', 'their', 'a', 'an', 'of', 'in', 'at', 's', 'and')]
    return '_'.join(w.lower() for w in words[:most])


IDENT = re.compile(r'[A-Za-z_][A-Za-z0-9_]*$')


def is_name(label, address=None):
    """A label that is a name (EnterMap, eToolkit, NPCList_maps80), not words;
    one in capitals only an I/O register's (DISPSTAT, not LZ77)."""
    if label.isupper() and address is not None and address >> 24 != 4:
        return False
    return bool(IDENT.match(label)) and bool(re.search(r'[A-Z_0-9]', label[1:]) or label[0].isupper())


# Our own names for addresses docs/ROM_DATA.md cites by another project's
# label (dism-exe/bn6f's, which carries no licence: docs/SYMBOLS.md) or by a
# chip's name (the owner's call, 5 October 2026). A label of that kind not
# listed here is never a name: the row's words and the address are.
OWN_NAMES = {
    ('bn6', 0x080003A0): 'frame_wait',
    ('bn6', 0x08005148): 'map_enter',
    ('bn6', 0x08005A8C): 'battle_check_start',
    ('bn6', 0x08030904): 'real_world_map_jumps',
    ('bn6', 0x08032598): 'mugshot_sprites',
    ('bn6', 0x0803461C): 'check_tables',
    ('bn6', 0x08034638): 'real_world_npc_lists',
    ('bn6', 0x08040794): 'map_text_archive_table',
    ('bn6', 0x080A4F24): 'map_object_table',
    ('bn6', 0x086C92AC): 'map_name_texts',
    ('bn6', 0x080127FC): 'chip_use_dark_id_read',
    ('bn6', 0x02036120): 'dark_invis_state',
    ('bn6', 0x0802E4B8): 'dark_invis_control',
    ('bn6', 0x0802E4E4): 'dark_invis_start',
    ('bn6', 0x080EA908): 'time_freeze_object',
    ('bn6', 0x080EAAC4): 'time_freeze_dark_invis',
    ('bn5', 0x080054F6): 'battle_start',
    ('bn5', 0x080AD930): 'map_object_table',
}


def rom_data_symbols(game, rows, taken):
    """The addresses docs/ROM_DATA.md gives that the header and RomLayout do
    not: named by the word the row gives one where it is a name (EnterMap,
    eToolkit) or the name in code beside it, else by its words (the BugFrag
    spent: bugfrag_spent); a word alone, or words another address has too,
    after the row's first words (bn6_own_darkchips.check)."""
    seen = {}
    for row in rows:
        for e in row['entries']:
            if e['game'] == game['key'] and e['address'] not in taken:
                seen.setdefault(e['address'], []).append((row, e))

    def given(e):
        return next((n for n in e['names'] if not n.startswith(('BN5_', 'BN6_'))), None)

    def rank(pair):
        row, e = pair
        return not is_name(e['label']), not given(e), not e['label']
    picked = [(address,) + min(found, key=rank) for address, found in sorted(seen.items())]
    plain = {}
    for address, row, e in picked:
        plain[slug(e['label'])] = plain.get(slug(e['label']), 0) + 1
    syms = {}
    for address, row, e in picked:
        label, words = e['label'], slug(e['label'])
        own = OWN_NAMES.get((game['key'], address))
        foreign_label = is_name(label, address) and address >> 24 != 4 and not given(e)
        foreign = bool(own) or foreign_label
        if own:
            name = own
        elif foreign:
            name = f"{slug(row['title'].split(':')[0], 3)}.at_{address:08x}"
        elif is_name(label, address) or given(e):
            name = label if is_name(label, address) else given(e)
        elif '_' in words and plain[words] == 1 and not words[0].isdigit():
            name = words
        else:
            name = '.'.join(filter(None, (slug(row['title'].split(':')[0], 3), words)))
        base, k = name, 2
        while name in syms:
            name, k = f'{base}_{k}', k + 1
        kind = next((kd for a, b, kd in REGIONS if a <= address < b), 'constant')
        syms[name] = dict(name=name, kind=kind, address=address, written=e['written'],
                          description=f"{row['title']}: {label}" if label and label != name and not foreign_label else row['title'],
                          source=f"{ROM_DATA}:{row['line']}", origin='rom_data', given=given(e))
    return syms


# ---- tools/bn6f_match.py's tables (the full map) ----

def match_rows(game):
    """A ROM's table in .build/symbols/match and the bn6f commit it was made
    from; None where there is none."""
    path = os.path.join(ROOT, MATCH, game['match'])
    if not os.path.exists(path):
        return None, ''
    with open(path, encoding='utf-8', newline='') as f:
        rows = list(csv.DictReader(f))
    commit = next((r['name'] for r in rows if r['kind'] == '# bn6f'), '')
    return [r for r in rows if not r['kind'].startswith('#')], commit


class Functions:
    """The located functions of a ROM, sorted, for finding the one an address is in."""

    def __init__(self, rows):
        self.items = sorted((int(r['address'], 16), int(r['size'] or 0), r['name'], pools(r['pools']))
                            for r in rows if r['kind'] == 'function' and r['address'] and r['size'])
        self.starts = [i[0] for i in self.items]

    def find(self, a):
        k = bisect.bisect_right(self.starts, a) - 1
        if k >= 0 and a < self.items[k][0] + self.items[k][1]:
            return self.items[k]
        return None


def pools(text):
    out = []
    for part in filter(None, (text or '').split(';')):
        p, q = part.split('-')
        out.append((int(p, 16), int(q, 16)))
    return out


def match_record(r, a):
    """A row of tools/bn6f_match.py's table as a symbol: bn6f's name, its
    Falzar address, its line in bn6f, how it was located and how many
    references agree."""
    what = dict(function='', data="bn6f's label: data the located functions name",
                ram="bn6f's RAM label" if r['confidence'] != 'moved' else
                "bn6f's RAM label, read here by code that matches bn6f's (falzar: its address there)")
    refs = int(r['refs']) if r['refs'] else 0
    return dict(name=r['name'], kind=r['kind'], address=a, size=int(r['size']) if r['size'] else None, mode=r['mode'] or None,
                description=what[r['kind']], source=f"bn6f {r['source']}" if r['source'] else 'bn6f', origin='match',
                bn6f=r['name'], falzar=r['falzar'], confidence=r['confidence'], refs=refs,
                verified=(f'{refs} references agree' if refs > 1 else '1 reference agrees') if refs else '', how='')


# what a confidence of tools/bn6f_match.py's tables says
CONFIDENCE = (
    ('bytes', "its bytes (bn6f's, made at its Falzar address, what depends on where things are masked) found once in the ROM"),
    ('calls+bytes', 'its bytes found more than once; the calls, branches and pointers of located functions name this place'),
    ('neighbours', 'its bytes at the shift from Falzar the located functions before and after it in its file share'),
    ('calls', "named by the calls, branches and pointers of located functions, all agreeing; its own bytes differ from Falzar's"),
    ('shift', "estimated: at its neighbours' shift, where most of its known bits (the percentage) are Falzar's"),
)


def bn6f_names(s):
    """The bn6f names a symbol's source gives it: "bn6f NAME", a name its
    description opens with (GiveChips (chip, ...), eToolkit: ...), the name
    beside it in ROM_DATA.md. A define's own comment only: the words it
    shares with the defines its "..." continues name what they are."""
    desc = (s.get('own') or '') if s['origin'] == 'header' else s.get('description') or ''
    names = re.findall(r'bn6f `?([A-Za-z_]\w*)`?', desc)
    m = re.match(r'([A-Za-z_][A-Za-z0-9_]*)(?=[:,]| \(| \+|$)', desc)
    if m and re.search(r'[A-Z_0-9]', m.group(1)[1:]):
        names.append(m.group(1))
    if s.get('origin') == 'rom_data':
        names.append(s['name'])
        if s.get('given'):
            names.append(s['given'])
    return list(dict.fromkeys(names))


def crosscheck(syms, matches):
    """Each of our addresses that names a bn6f function or label as what it
    is or lies in, against where tools/bn6f_match.py put that: the same
    place; inside the function, or the structure at the offset our comment
    gives ("eJoypad +2"); a field of the structure the label is; or
    another place (a disagreement to look into: where it lies instead)."""
    table = {r['name']: r for r in matches if r['address']}
    functions = Functions(matches)
    out = []
    for s in syms.values():
        if s['origin'] not in ('header', 'rom_data', 'layout') or 'address' not in s:
            continue
        rom = s['kind'] in ('function', 'code', 'rom') or (s['kind'] == 'data' and s['address'] >> 24 == 8)
        for n in bn6f_names(s):
            r = table.get(n)
            if r is None or (r['kind'] == 'function') != (s['kind'] in ('function', 'code', 'rom')):
                continue   # (a RAM field's comment naming the code that reads it says nothing of where it is)
            g, size = int(r['address'], 16), int(r['size'] or 0)
            a = s['address'] & ~1 if r['kind'] == 'function' else s['address']
            base = syms.get(s.get('base') or '', {})
            plus = re.search(re.escape(n) + r' ?\+ ?(0x[0-9A-Fa-f]+|\d+)', s.get('description') or '')
            if a == g:
                verdict = 'same'
            elif r['kind'] == 'function' and g <= a < g + size:
                verdict = 'inside'
            elif plus and a - g == int(plus.group(1), 0):
                verdict = 'inside'
            elif s['kind'] == 'field' and base.get('address') == g:
                verdict = 'field'
            else:
                verdict = 'other'
            lies = functions.find(a) if rom and verdict == 'other' else None
            out.append(dict(name=s['name'], address=s['address'], bn6f=n, at=g, verdict=verdict, source=s['source'],
                            lies=lies[2] if lies else None))
    return out


# ---- the ROM's symbols together ----

def rom_kind(s, functions):
    """A ROM address's kind: a function's entry, code inside one, or data, by
    the functions located in the ROM (the full map); else by its comment:
    code, data, or not told. With the function it lies in."""
    a = s['address'] & ~1
    f = functions.find(a)
    if f is not None:
        start, size, name, inner = f
        if a == start:
            return 'function', name
        if any(p <= a - start < q for p, q in inner):
            return 'data', name
        return 'code', name
    desc = s.get('description') or ''
    if s['address'] & 1 and s.get('origin') == 'header' or re.search(CODE_WORDS, desc):
        return 'code', None
    if re.search(DATA_WORDS, desc):
        return 'data', None
    return 'rom', None


# what says a ROM address is code: an instruction, a register argument, a
# call's arguments, what it returns
CODE_WORDS = (r'\b(ldrh?|ldrb|strh?|strb|movs?|cmp|beq|bne|bl|bx|pop|push|tst|adds?|subs?|lsls?)\s*(r\d|\{|pc|#)'
              r'|\br\d (the|its|how)\b|\b(hook|Thumb|returns)\b|bn6f sub_|^[A-Za-z_]\w* \([a-z, ]+\)')
# ... and data: what a table, an archive or a literal is called
DATA_WORDS = (r'\b(tables?|archives?|records?|rows?|lists?|literals?|pointers?|font|glyphs|widths|weights|text|tiles|'
              r'palettes?|descriptors|stats|entries|songs|data|map entries|frames|lists)\b')


def build(game, rows):
    """A ROM's symbols from this project's own sources: the header's,
    RomLayout's and ROM_DATA.md's; each address once, the header's name
    first; each with the rows of docs/ROM_DATA.md that verify it. A ROM
    address whose source does not say what it is stays 'rom' (finish)."""
    header, missing = header_symbols(game)
    info, layout = layout_symbols(game)
    syms = dict(header)
    for s in syms.values():
        if s['kind'] == 'alias':   # (BN6_MAP_ID: the same field read another way)
            target = syms[s.pop('of')]
            s.update(kind='field', alias_of=target['name'], base=target.get('base'), offset=target.get('offset'))
            if 'address' in target:
                s['address'] = target['address']
    taken = set()
    for s in syms.values():
        if 'address' in s:
            taken |= {s['address'], s['address'] & ~1}
    for name, s in layout.items():
        if s['address'] in taken:
            other = next(o for o in syms.values() if o.get('address') == s['address'])
            other.setdefault('aliases', []).append(name)
            continue
        syms[name] = s
        taken.add(s['address'])
    syms.update(rom_data_symbols(game, rows, taken))
    # verified by: the rows that give its address or name it
    by_address, by_name = {}, {}
    for row in rows:
        for e in row['entries']:
            if e['game'] == game['key']:
                by_address.setdefault(e['address'], []).append(row)
        for n in row['names']:
            by_name.setdefault(n, []).append(row)
    for s in syms.values():
        found = list(by_address.get(s.get('address'), [])) + [r for r in by_name.get(s['name'], []) if r not in by_address.get(s.get('address'), [])]
        if not found and s['kind'] == 'field' and s.get('base') in syms:
            base = syms[s['base']]
            found = list(by_address.get(base.get('address'), [])) + by_name.get(base['name'], [])
        seen, refs = set(), []
        for r in found:
            if r['line'] not in seen:
                seen.add(r['line'])
                refs.append(r)
        s['verified'] = '; '.join(f"{ROM_DATA}:{r['line']} ({r['title']})" for r in refs)
        s['how'] = next((r['how'] for r in refs if r['how']), '')
        s['rows'] = [r['line'] for r in refs]
    # the structures' sizes and counts; an array's stride
    for s in list(syms.values()):
        if s['kind'] in ('size', 'count') and s.get('of') in syms:
            base = syms[s['of']]
            if s['kind'] == 'count' or base.get('count'):
                base['count' if s['kind'] == 'count' else 'stride'] = s['value']
            else:
                base['size'] = s['value']
    for s in syms.values():
        if s.get('count') and (s.get('size') or s.get('stride')):
            s['stride'] = s.get('stride') or s['size']
            s['size'] = s['stride'] * s['count']
        elif s.get('count') is None and s.get('stride'):
            s['size'] = s.pop('stride')
    # a field of a structure at one place, at its own address too
    for s in syms.values():
        base = syms.get(s.get('base'))
        if s['kind'] == 'field' and 'address' not in s and base and 'address' in base and not base.get('count') \
                and base['kind'] != 'field' and isinstance(s.get('offset'), int):
            s['address'] = base['address'] + s['offset']
    return info, syms, missing


def finish(syms, matches=()):
    """The symbols as a file holds them: each ROM address's kind (rom_kind)
    and each one's part of the game. With tools/bn6f_match.py's table (the
    full map), the disassembly's functions and labels located in the ROM
    join, each at an address none of ours names; where one of ours is, it
    carries bn6f's name."""
    syms = copy.deepcopy(syms)
    functions = Functions(matches)
    for s in syms.values():
        if s['kind'] == 'rom':
            s['kind'], inside = rom_kind(s, functions)
            if inside:
                s['in_function'] = inside
    at = {}
    for s in syms.values():
        if 'address' in s and s['kind'] in ADDRESS_KINDS:
            # (a Thumb routine's address may carry its bit 0; data and RAM never do)
            at.setdefault(s['address'] & ~1 if s['kind'] in ('function', 'code', 'rom') else s['address'], []).append(s)
    for r in matches:
        if not r['address']:
            continue
        a = int(r['address'], 16)
        ours = [o for o in at.get(a, []) if (o['kind'] in ('function', 'code')) == (r['kind'] == 'function')]
        if ours:
            for o in ours:
                o.update(bn6f=r['name'], falzar=r['falzar'], confidence=r['confidence'])
                if r['kind'] == 'function':
                    o.update(mode=r['mode'], size=o.get('size') or (int(r['size']) if r['size'] else None))
                    o['kind'] = 'function' if o['kind'] in ('rom', 'code') else o['kind']
                elif o['kind'] == 'rom':
                    o['kind'] = 'data'
            continue
        if r['name'] not in syms:
            syms[r['name']] = match_record(r, a)
    for s in syms.values():
        s['system'] = system_of(s)
    return syms


# the kinds of symbol that are a place on the bus
ADDRESS_KINDS = ('function', 'code', 'data', 'rom', 'ram', 'io', 'field')

# what part of the game a symbol belongs to: by the header's name, the
# description of another's (ROM_DATA.md's row, rom.h's comment); first match
SYSTEMS = (
    ('PET, mail and key items', r'PET|MAIL|KEY_ITEM|KEYS_|KEY_NAMES|KEY_DESC|KeyItem|E-Mail|key item'),
    ('Shops and traders', r'SHOP|TRADER|VENDOR|[Ss]hop|[Tt]rader'),
    ('NaviCust', r'NAVICUST|PROGRAM_ITEMS|NaviCust|REG\b'),
    ('Chips and folders', r'CHIP|FOLDER|PACK|LIBRARY|[Cc]hip|[Ff]older|Library|Program Advance'),
    ('Battle', r'BATTLE|T1_|T3_|PANEL|CUSTOM|ENCOUNTER|REWARD|SPAWN|SUBTRACT|DECK|PHASE|GAUGE|SOUL|UNITE|DARK|FIGHT|'
               r'ENEMY|PA_STAR|MOOD|METER|BUSTER|NAVI_|STORY|ROLL|COMPACT|RECORD_|OPT_|RESULT|MEGAMAN|[Bb]attle|[Ee]nemy|'
               r'[Ee]ncounter|DarkChip|Soul|buster|Navis'),
    ('Text and fonts', r'CHAT|TEXT|NAMES|FONT|[Ff]ont|[Tt]ext|[Cc]hat|[Mm]essages|[Mm]ugshot'),
    ('Sound', r'MUSIC|SONG|[Ss]ong|[Mm]usic|MP2K'),
    ('Maps and the overworld', r'MAP|WARP|PLAYER|NPC|JACK|ENTER|CUTSCENE|OBJ_SPAWN|MYSTERY|TILEMAP|TALK|DIALOGUE|[Mm]ap|[Ww]arp|'
                               r'Rush|[Tt]ile|[Cc]oordinate|[Jj]ack|NPC|[Tt]eleport|[Ss]prite|[Oo]bstacle|[Pp]ads|[Tt]itle|[Bb]ackdrop'),
    ('Events and progress', r'FLAG|ZENNY|BUGFRAG|GIVE|TAKE|SAVE|[Ff]lag|Zenny|BugFrag|[Ss]tarting'),
    ('Engine core', r''),
)


def system_of(s):
    if s['origin'] == 'match':
        return file_system(s['source'])
    text = s['name'] if s['origin'] == 'header' else f"{s['name']} {s.get('description') or ''}"
    return next(name for name, rx in SYSTEMS if re.search(rx, text))


# bn6f's files by what most of their code does (read from the names bn6f
# gives their functions; a file holds more than one thing), for the full map
FILE_SYSTEMS = {
    'start': 'Engine core', 'main': 'Engine core', 'asm00_0': 'Engine core', 'sprite': 'Graphics', 'asm38': 'Engine core',
    'asm00_1': 'Battle', 'object': 'Battle', 'asm00_2': 'Battle', 'asm01': 'Battle', 'asm03_0': 'Battle', 'asm29': 'Battle',
    'asm30_0': 'Battle', 'asm31': 'Battle', 'asm32': 'Battle',
    'asm02': 'Chips and folders', 'asm03_2': 'Shops and traders', 'chatbox': 'Text and fonts', 'asm37_0': 'NaviCust',
    'asm03_1_0': 'Maps and the overworld', 'map_script_cutscene': 'Maps and the overworld', 'ow_player': 'Maps and the overworld',
    'npc': 'Maps and the overworld', 'asm28_0': 'Maps and the overworld', 'asm28_1': 'Maps and the overworld',
    'asm21': 'Maps and the overworld', 'asm22': 'Maps and the overworld', 'asm23': 'Maps and the overworld',
    'asm24': 'Maps and the overworld', 'asm25': 'Maps and the overworld', 'asm26': 'Maps and the overworld',
    'asm27': 'Maps and the overworld', 'asm03_1_1': 'Menus and screens', 'asm33': 'Menus and screens', 'asm34': 'Menus and screens',
    'asm35': 'Menus and screens', 'asm36': 'Menus and screens', 'reqBBS': 'Menus and screens',
    'asm37_1': 'Minigames and their maps', 'libs': 'Sound and libraries (m4a, link cable, wireless)',
}


def file_system(source):
    m = re.search(r'(?:^|\s)(asm|data|maps)/([\w/]+)\.s', source or '')
    if not m:
        return 'Other'
    if m.group(1) == 'maps' or re.fullmatch(r'dat2[1-8]', m.group(2)):
        return 'Maps and the overworld'
    return FILE_SYSTEMS.get(m.group(2), 'Other')


# ---- the files ----

def fmt(v, digits=None):
    if v is None or v == '':
        return ''
    if isinstance(v, int):
        return hx(v, digits or (8 if v >= 0x10000 else 2 if v < 0x100 else 4))
    return str(v)


def tree(syms):
    """The symbols with an address, each structure's fields under it (a
    pointer field's under it in turn); the record types, which have no
    address, with their fields; the flags, values and constants."""
    kids = {}
    for s in syms.values():
        if s['kind'] == 'field':
            kids.setdefault(s.get('base'), []).append(s)

    def node(s):
        e = entry(s)
        if s['name'] in kids:
            e['fields'] = [node(k) for k in sorted(kids[s['name']], key=lambda k: (k.get('offset') or 0, k['name']))]
        return e
    top = [node(s) for s in sorted(syms.values(), key=lambda s: (s.get('address', 0), s['name']))
           if s['kind'] in ADDRESS_KINDS and s['kind'] != 'field']
    types = [dict(name=t, fields=[node(k) for k in sorted(kids[t], key=lambda k: (k.get('offset') or 0, k['name']))])
             for t in sorted(k for k in kids if k and k not in syms)]
    rest = [entry(s) for s in sorted(syms.values(), key=lambda s: (s['kind'], s['name']))
            if s['kind'] in ('flag', 'value', 'constant', 'size', 'count')]
    return top, types, rest


def entry(s):
    """A symbol as the JSON writes it: the rows of docs/ROM_DATA.md that
    verify it by their lines (the file's rom_data_rows says what each is);
    in the full map, what bn6f calls it, its Falzar address, how it was
    located and how many references agree."""
    e = dict(name=s['name'], kind=s['kind'])
    if 'address' in s:
        e['address'] = hx(s['address'])
    for k in ('offset', 'value'):
        if isinstance(s.get(k), int):
            e[k] = fmt(s[k])
    for k in ('size', 'count', 'stride'):
        if s.get(k):
            e[k] = s[k]
    for k in ('of', 'mode', 'description', 'system'):
        if s.get(k):
            e[k] = s[k]
    if s.get('rows'):
        e['verified'] = s['rows']
    e['source'] = s['source']
    for k in ('in_function', 'bn6f', 'falzar', 'confidence', 'refs'):
        if s.get(k) and not (k == 'bn6f' and s[k] == s['name']):
            e[k] = s[k]
    if s.get('aliases'):
        e['aliases'] = s['aliases']
    if s.get('alias_of'):
        e['alias_of'] = s['alias_of']
    return e


def as_json(game, info, syms, rows, about):
    """The JSON: the ROM, the rows of docs/ROM_DATA.md its symbols cite
    (each row's title and its words on how to verify it, once), then a
    symbol a line (its fields within it), no spaces between the parts of a
    line."""
    top, types, rest = tree(syms)
    cited = sorted({n for s in syms.values() for n in s.get('rows', [])})
    by_line = {r['line']: r for r in rows}
    head = dict(game=info['name'], code=info['code'], sha1=info['sha1'], written_by='tools/symbols.py', about=about,
                counts=counts(syms),
                rom_data_rows={str(n): dict(title=by_line[n]['title'], how=by_line[n]['how']) for n in cited})

    def dump(v):
        return json.dumps(v, ensure_ascii=False, separators=(',', ':'))
    lines = ['{']
    for k, v in head.items():
        lines.append(f'{dump(k)}:{dump(v)},')
    for key, items in (('symbols', top), ('types', types), ('numbers', rest)):
        lines.append(f'{dump(key)}:[')
        lines += [dump(e) + (',' if k + 1 < len(items) else '') for k, e in enumerate(items)]
        lines.append(']' + (',' if key != 'numbers' else ''))
    lines.append('}')
    return '\n'.join(lines) + '\n'


ABOUT = ('Addresses, names and descriptions only: facts about the ROM, read from Cyberworld Endless\'s own sources '
         '(src/emu/*.h, src/core/rom.c, docs/ROM_DATA.md). No graphics, text, sound or code of the game. '
         'See docs/SYMBOLS.md.')
FULL_ABOUT = ('The full map, written on this machine by tools/symbols.py --full: Cyberworld Endless\'s own symbols '
              '(docs/symbols) and the functions and labels of the bn6f disassembly (github.com/dism-exe/bn6f, commit {}) '
              'that tools/bn6f_match.py located in this ROM, under bn6f\'s names. bn6f states no license, so its names '
              'are not redistributed: this file is for use here, not for sharing (docs/SYMBOLS.md). Addresses, names '
              'and descriptions only; no graphics, text, sound or code of the game.')


def as_csv(game, info, syms, columns=COLUMNS):
    out = io.StringIO()
    w = csv.writer(out, lineterminator='\n')
    w.writerow(columns)
    for s in sorted(syms.values(), key=lambda s: ('address' not in s, s.get('address', 0), s.get('base') or '',
                                                  s.get('offset') or 0, s['kind'], s['name'])):
        row = dict(game=game['game'], rom_sha1=info['sha1'], name=s['name'], kind=s['kind'], address=fmt(s.get('address'), 8),
                   base=s.get('base') or s.get('of') or '', offset=fmt(s.get('offset')), size=s.get('size') or '',
                   count=s.get('count') or '', value=fmt(s.get('value')) if isinstance(s.get('value'), int) else '',
                   description=s.get('description') or '', verified=s.get('verified') or '', how=s.get('how') or '',
                   source=s.get('source') or '', bn6f=s.get('bn6f') if s.get('bn6f') != s['name'] else '',
                   falzar=s.get('falzar') or '', confidence=s.get('confidence') or '')
        w.writerow([row[c] for c in columns])
    return out.getvalue()


SYM_HEAD = ('; Written by tools/symbols.py from Cyberworld Endless (github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless):',
            '; addresses, names and descriptions only. docs/SYMBOLS.md says what is here and how to load it:',
            '; beside the ROM with its name (game.gba, game.sym), or mGBA\'s debugger command load-symbols FILE.')
FULL_SYM_HEAD = ('; The full map, written on this machine by tools/symbols.py --full: Cyberworld Endless\'s own symbols and the',
                 '; bn6f disassembly\'s functions and labels located in this ROM (tools/bn6f_match.py, bn6f commit {}), bn6f\'s',
                 '; name after ours where both name one address. bn6f states no license: for use here, not for sharing',
                 '; (docs/SYMBOLS.md). Beside the ROM with its name (game.gba, game.sym), or mGBA\'s load-symbols FILE.')


def as_sym(game, info, syms, head=SYM_HEAD):
    """The no$gba symbol file (problemkaputt.de/gbahlp.htm, "Symbolic Debug
    Info"), which mGBA reads too (src/debugger/symbols.c,
    mDebuggerLoadARMIPSSymbols): an eight-digit address, a space, the name;
    ; opens a comment. no$gba's .thumb and .arm mark a function's code (mGBA
    skips what opens with a dot). In the full map, a function or label of
    bn6f's at one of our addresses carries its name after ours."""
    lines = [f'; {info["name"]}, SHA-1 {info["sha1"]} (header code {info["code"]})', *head, '']
    out = []
    for s in syms.values():
        if 'address' not in s or s['kind'] not in ADDRESS_KINDS:
            continue
        a = s['address'] & ~1 if s['kind'] in ('function', 'code') else s['address']
        names = [s['name']] + ([s['bn6f']] if s.get('bn6f') and s['bn6f'] != s['name'] else [])
        mode = s.get('mode') or ('thumb' if s['kind'] == 'code' and s['address'] & 1 else None)
        if s['kind'] == 'function' and mode:
            out.append((a, 0, '.' + mode))
        for k, n in enumerate(names):
            out.append((a, 1 + k, n))
    for a, k, n in sorted(set(out)):
        lines.append(f'{a:08X} {n}')
    return '\n'.join(lines) + '\n'


def counts(syms):
    c = {}
    for s in syms.values():
        c[s['kind']] = c.get(s['kind'], 0) + 1
    return dict(sorted(c.items()))


# ---- docs/SYMBOLS.md's numbers ----

MARK = '<!-- tools/symbols.py writes this part: {} -->'
END = '<!-- end of what tools/symbols.py writes -->'
# bn6f's names made of an address (IDA's), and names that end in one
AUTO_NAME = re.compile(r'(sub|nullsub|dead|unk)_[0-9A-Fa-f]+$')
ADDRESS_END = re.compile(r'_[0-9A-Fa-f]{6,8}$')


def pct(n, total):
    return f'{n:,} of {total:,} ({100 * n / total:.1f}%)' if total else '0'


def holdings(results):
    """What each ROM's files hold, by kind and by part of the game."""
    kinds = ('function', 'code', 'data', 'rom', 'ram', 'io', 'field', 'flag', 'value', 'constant')
    out = ['| Symbols | ' + ' | '.join(g['title'] for g, *_ in results) + ' |',
           '| --- | ' + ' | '.join('---:' for _ in results) + ' |']
    names = dict(function='Functions (entry points)', code='Code (routines, hooks, instructions)', data='ROM data (tables, text, literals)',
                 rom='ROM, code or data not told', ram='RAM (variables, structures)', io='I/O registers', field='Fields of structures',
                 flag='Event flags', value='Values of fields', constant='Constants, sizes and counts')
    for k in kinds:
        row = [sum(1 for s in syms.values() if s['kind'] == k or (k == 'constant' and s['kind'] in ('size', 'count')))
               for g, info, syms in results]
        if any(row):
            out.append(f'| {names[k]} | ' + ' | '.join(str(n) for n in row) + ' |')
    out.append('| All | ' + ' | '.join(str(len(syms)) for g, info, syms in results) + ' |')
    out += ['', '| Part of the game | ' + ' | '.join(g['title'] for g, *_ in results) + ' |',
            '| --- | ' + ' | '.join('---:' for _ in results) + ' |']
    for name, rx in SYSTEMS:
        row = [sum(1 for s in syms.values() if s['system'] == name) for g, info, syms in results]
        if any(row):
            out.append(f'| {name} | ' + ' | '.join(str(n) for n in row) + ' |')
    return '\n'.join(out)


def coverage(located, by_file=False):
    """bn6f's functions located in each ROM: in all, by how, named ones
    apart, by part of the game; by bn6f's file in the local report. The
    numbers only: no name of bn6f's."""
    every = [r for r in located[0][2] if r['kind'] == 'function']   # (Gregar's table lists every function)
    found = {g['key']: {r['name']: r for r in m if r['kind'] == 'function' and r['address']} for g, w, m, c, k in located}
    games = [g for g, *_ in located]
    out = ['| bn6f\'s functions | ' + ' | '.join(g['title'] + (' (partial)' if g['key'] == 'bn5' else '') for g in games) + ' |',
           '| --- | ' + ' | '.join('---:' for _ in games) + ' |']

    def row(title, subset):
        cells = [pct(sum(1 for r in subset if r['name'] in found[g['key']]), len(subset)) for g in games]
        out.append(f'| {title} | ' + ' | '.join(cells) + ' |')
    row('All (thumb_func_start, thumb_local_start, arm_func_start, arm_local_start)', every)
    row('... global (thumb_func_start, arm_func_start)', [r for r in every if r['scope'] == 'global'])
    named = [r for r in every if not AUTO_NAME.match(r['name'])]
    row('... named by bn6f (not sub_ and an address)', named)
    row('... of them with no address in the name', [r for r in named if not ADDRESS_END.search(r['name'])])
    for how, words in CONFIDENCE:
        cells = [f'{sum(1 for r in found[g["key"]].values() if r["confidence"].split()[0] == how):,}' for g in games]
        out.append(f'| Located by `{how}` | ' + ' | '.join(cells) + ' |')
    for kind, title, ok in (('data', 'Data labels the located functions name', ('calls', 'bytes', 'neighbours', 'calls+bytes')),
                            ('ram', "RAM labels their literals hold at Falzar's address", ('calls',)),
                            ('ram', 'RAM labels their literals hold elsewhere', ('moved',))):
        cells = [f'{sum(1 for r in m if r["kind"] == kind and r["address"] and r["confidence"] in ok):,}' for g, w, m, c, k in located]
        out.append(f'| {title} | ' + ' | '.join(cells) + ' |')
    groups = [('Part of the game (bn6f\'s files)', lambda r: file_system(r['source']))]
    if by_file:
        groups.append(('bn6f\'s file', lambda r: '`' + r['source'].split(':')[0] + '`'))
    for title, key in groups:
        out += ['', f'| {title} | ' + ' | '.join(g['title'] for g in games) + ' |', '| --- | ' + ' | '.join('---:' for _ in games) + ' |']
        parts = {}
        for r in every:
            parts.setdefault(key(r), []).append(r)
        for part in (sorted(parts, key=lambda p: -len(parts[p])) if not by_file or title.startswith('Part') else parts):
            cells = [pct(sum(1 for r in parts[part] if r['name'] in found[g['key']]), len(parts[part])) for g in games]
            out.append(f'| {part} | ' + ' | '.join(cells) + ' |')
    return '\n'.join(out)


def crosschecks(located, named=False):
    """Our addresses that name a bn6f function or label, against the matcher;
    and the match tables' own checks: RAM as Falzar's, no two functions
    over each other, no reference naming another place. Counts only; with
    named (the local report), which ones."""
    out = []
    for g, whole, m, cross, commit in located:
        verdicts = {}
        for c in cross:
            verdicts[c['verdict']] = verdicts.get(c['verdict'], 0) + 1
        out.append(f'- {g["title"]}: {len(cross)} times an address of ours names a bn6f function or label the matcher '
                   f'located: {verdicts.get("same", 0)} at the same place, {verdicts.get("inside", 0)} inside the function or '
                   f'structure, {verdicts.get("field", 0)} a field of the structure it is, {verdicts.get("other", 0)} elsewhere.')
        for c in cross if named else ():
            out.append(f'  - `{c["name"]}` ({c["source"]}) at {hx(c["address"])} names {c["bn6f"]}: {c["verdict"]}'
                       + (f', the matcher put it at {hx(c["at"])}' if c['verdict'] == 'other' else '')
                       + (f'; the address lies in {c["lies"]}' if c['lies'] else '') + '.')
        ram = [r for r in m if r['kind'] == 'ram' and r['address']]
        moved = [r for r in ram if r['confidence'] == 'moved']
        if g['key'] == 'bn6':
            out.append(f'- {g["title"]}: of the {len(ram):,} RAM labels the located functions\' literals hold, '
                       f'{len(ram) - len(moved):,} read bn6f\'s (Falzar\'s) address'
                       + (': ' + ', '.join(f'`{r["name"]}` at {r["address"]}, not {r["falzar"]}' for r in moved) if named and moved else '')
                       + '.')
        spans = sorted((int(r['address'], 16), int(r['address'], 16) + int(r['size']), r['name']) for r in m
                       if r['kind'] == 'function' and r['address'] and r['size'] and int(r['address'], 16) >> 24 == 8)
        over = [(a, b) for a, b in zip(spans, spans[1:]) if b[0] < a[1]]
        against = [r for r in m if r['kind'] == 'function' and r['against']]
        out.append(f'- {g["title"]}: {len(spans):,} located functions laid out as in Falzar, {len(over)} lying over another; '
                   f'{len(against)} that a located reference names elsewhere'
                   + (': ' + ', '.join(f'`{r["name"]}` ({r["against"]})' for r in against) if named and against else '') + '.')
    return '\n'.join(out)


def located_block(located):
    """SYMBOLS.md's numbers of the full map: what was located, and the checks."""
    commits = {commit for *_, commit in located if commit}
    made = f'Made from bn6f at commit `{min(commits)[:7]}`.' if len(commits) == 1 else 'Made from bn6f.'
    return made + '\n\n' + coverage(located) + '\n\n' + crosschecks(located)


def report(located):
    """The full map's report, for this machine: the numbers by bn6f's file,
    and each check by name."""
    commits = sorted({commit for *_, commit in located if commit})
    return ('# The full map: what tools/bn6f_match.py located\n\n'
            'Written by `tools/symbols.py --full` from bn6f ' + (', '.join(commits) or '(commit not known)') + '. '
            'It names bn6f\'s functions and labels, so it stays on this machine (docs/SYMBOLS.md).\n\n'
            + coverage(located, by_file=True) + '\n\n' + crosschecks(located, named=True) + '\n')


def doc_with(text, blocks):
    """The page with each of its marked parts written anew; None where a mark is missing."""
    for name, block in blocks.items():
        mark = MARK.format(name)
        if mark not in text or END not in text[text.index(mark):]:
            return None
        start = text.index(mark) + len(mark)
        end = text.index(END, start)
        text = text[:start] + '\n\n' + block + '\n\n' + text[end:]
    return text


def full_map(results, rows, files, problems):
    """The full map into files (.build/symbols): each ROM's symbols with
    tools/bn6f_match.py's table merged in, and the report. Returns
    SYMBOLS.md's block of its numbers, or None where Gregar's table is
    missing."""
    located = []
    for g, info, raw, syms in results:
        matches, commit = match_rows(g)
        if matches is None:   # (BN5's ROM is not everyone's: its map is left out, Gregar's is not)
            note = f'{MATCH}/{g["match"]}: none, as its ROM was not found (python3 build.py symbols --bn6f DIR makes it)'
            if g['key'] == 'bn6':
                problems.append(note)
            else:
                print(f'  {note}')
            continue
        whole = finish(raw, matches)
        located.append((g, whole, matches, crosscheck(whole, matches), commit))
        files[f'{FULL}/{g["stem"]}.sym'] = as_sym(g, info, whole, tuple(h.format(commit[:7]) for h in FULL_SYM_HEAD))
        files[f'{FULL}/{g["stem"]}.json'] = as_json(g, info, whole, rows, FULL_ABOUT.format(commit[:7]))
        files[f'{FULL}/{g["stem"]}.csv'] = as_csv(g, info, whole, FULL_COLUMNS)
    if not located or located[0][0]['key'] != 'bn6':
        return None
    files[f'{FULL}/located.md'] = report(located)
    for g, whole, m, cross, commit in located:
        print(f'{g["stem"]} (full map): {len(whole)} symbols; cross-check: {sum(c["verdict"] != "other" for c in cross)} agree, '
              f'{sum(c["verdict"] == "other" for c in cross)} not')
    return located_block(located)


def main():
    check, full = '--check' in sys.argv[1:], '--full' in sys.argv[1:]
    rows = rom_data_rows()
    files, problems, results = {}, [], []
    for g in GAMES:
        info, raw, missing = build(g, rows)
        problems += missing
        syms = finish(raw)
        results.append((g, info, raw, syms))
        files[f'{OUT}/{g["stem"]}.sym'] = as_sym(g, info, syms)
        files[f'{OUT}/{g["stem"]}.json'] = as_json(g, info, syms, rows, ABOUT)
        files[f'{OUT}/{g["stem"]}.csv'] = as_csv(g, info, syms)
    blocks = dict(holdings=holdings([(g, info, syms) for g, info, raw, syms in results]))
    if full and not check:
        block = full_map(results, rows, files, problems)
        if block is not None:
            blocks['located'] = block
    doc = doc_with(read(DOC), blocks) if os.path.exists(os.path.join(ROOT, DOC)) else None
    if doc is None:
        problems.append(f'{DOC} lacks the marks between which tools/symbols.py writes its numbers')
    else:
        files[DOC] = doc
    for p in problems:
        print(f'  {p}')
    if check:
        stale = [path for path, text in files.items()
                 if not os.path.exists(os.path.join(ROOT, path)) or read(path) != text]
        for path in stale:
            print(f'  {path} differs from what tools/symbols.py writes: python3 tools/symbols.py')
        print(f'symbols: {sum(len(r[3]) for r in results)} in {len(files)} files, {len(problems)} problems, '
              f'{len(stale)} stale')
        return 1 if problems or stale else 0
    for path, text in files.items():
        os.makedirs(os.path.dirname(os.path.join(ROOT, path)), exist_ok=True)
        with open(os.path.join(ROOT, path), 'w', encoding='utf-8', newline='\n') as f:
            f.write(text)
    for g, info, raw, syms in results:
        print(f'{g["stem"]}: {len(syms)} symbols ({", ".join(f"{n} {k}" for k, n in counts(syms).items())})')
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
