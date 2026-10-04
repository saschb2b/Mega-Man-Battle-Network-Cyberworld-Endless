#!/usr/bin/env python3
"""symbols.py [--check]: what this project has mapped of Mega Man Battle
Network 6: Cybeast Gregar (USA) and Battle Network 5: Team Colonel (USA),
written for others to use (docs/SYMBOLS.md): per ROM a symbol file that
mGBA's and no$gba's debuggers load, and the same symbols as JSON and CSV.

Read from what the engine already keeps, so the files follow it:
src/emu/bn6.h and bn5.h (RAM and code addresses, the fields of the game's
structures, event flags and values, each with its comment), the ROM
offsets of RomLayout and XRomLayout in src/core/rom.c (rom.h's comments
say what each is) and docs/ROM_DATA.md (each table: where, how it was
found, how to verify it). Needs no ROM.

  python3 tools/symbols.py           write docs/symbols/ and SYMBOLS.md's numbers
  python3 tools/symbols.py --check   write nothing: fail where a name in bn6.h or
                                     bn5.h has no description, or a file differs
                                     from what it would write (build.py lint)
"""
import csv
import io
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = 'docs/symbols'
DOC = 'docs/SYMBOLS.md'
ROM_DATA = 'docs/ROM_DATA.md'

# The two ROMs: the header with their addresses, rom.c's layout and its
# entry, the files' stem, the game as the tables name it and its short title.
GAMES = (
    dict(key='bn6', stem='bn6-gregar-us', header='src/emu/bn6.h', layout=('RomLayout', 'layouts', 'ROM_BN6_GREGAR_US'),
         game='BN6 Cybeast Gregar (USA)', title='BN6 Gregar'),
    dict(key='bn5', stem='bn5-colonel-us', header='src/emu/bn5.h', layout=('XRomLayout', 'xlayouts', 'XROM_BN5_COLONEL_US'),
         game='BN5 Team Colonel (USA)', title='BN5 Team Colonel'),
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

# the tables' columns
COLUMNS = ('game', 'rom_sha1', 'name', 'kind', 'address', 'base', 'offset', 'size', 'count', 'value', 'description',
           'verified', 'how', 'source')


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
        s = dict(name=name, description=desc, source=f"{it['path']}:{it['line']}", origin='header')
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
        if is_name(label, address) or given(e):
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
                          description=f"{row['title']}: {label}" if label and label != name else row['title'],
                          source=f"{ROM_DATA}:{row['line']}", origin='rom_data')
    return syms


# ---- the ROM's symbols together ----

def rom_kind(s):
    """A ROM address's kind, by its comment: code, data, or not told."""
    desc = s.get('description') or ''
    if s['address'] & 1 and s.get('origin') == 'header' or re.search(CODE_WORDS, desc):
        return 'code'
    if re.search(DATA_WORDS, desc):
        return 'data'
    return 'rom'


# what says a ROM address is code: an instruction, a register argument, a
# call's arguments, what it returns
CODE_WORDS = (r'\b(ldrh?|ldrb|strh?|strb|movs?|cmp|beq|bne|bl|bx|pop|push|tst|adds?|subs?|lsls?)\s*(r\d|\{|pc|#)'
              r'|\br\d (the|its|how)\b|\b(hook|Thumb|returns)\b|bn6f sub_|^[A-Za-z_]\w* \([a-z, ]+\)')
# ... and data: what a table, an archive or a literal is called
DATA_WORDS = (r'\b(tables?|archives?|records?|rows?|lists?|literals?|pointers?|font|glyphs|widths|weights|text|tiles|'
              r'palettes?|descriptors|stats|entries|songs|data|map entries|frames|lists)\b')


def build(game, rows):
    """A ROM's symbols: the header's, RomLayout's and ROM_DATA.md's; each
    address once, the header's name first; each with the rows of
    docs/ROM_DATA.md that verify it."""
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
    for s in syms.values():
        if s['kind'] == 'rom':
            s['kind'] = rom_kind(s)
        s['system'] = system_of(s)
    return info, syms, missing


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
    text = s['name'] if s['origin'] == 'header' else f"{s['name']} {s.get('description') or ''}"
    return next(name for name, rx in SYSTEMS if re.search(rx, text))


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
    verify it by their lines (the file's rom_data_rows says what each is)."""
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
    if s.get('aliases'):
        e['aliases'] = s['aliases']
    if s.get('alias_of'):
        e['alias_of'] = s['alias_of']
    return e


def as_json(game, info, syms, rows):
    """The JSON: the ROM, the rows of docs/ROM_DATA.md its symbols cite
    (each row's title and its words on how to verify it, once), then a
    symbol a line (its fields within it), no spaces between the parts of a
    line."""
    top, types, rest = tree(syms)
    cited = sorted({n for s in syms.values() for n in s.get('rows', [])})
    by_line = {r['line']: r for r in rows}
    head = dict(game=info['name'], code=info['code'], sha1=info['sha1'], written_by='tools/symbols.py', about=ABOUT,
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


def as_csv(game, info, syms):
    out = io.StringIO()
    w = csv.writer(out, lineterminator='\n')
    w.writerow(COLUMNS)
    for s in sorted(syms.values(), key=lambda s: ('address' not in s, s.get('address', 0), s.get('base') or '',
                                                  s.get('offset') or 0, s['kind'], s['name'])):
        row = dict(game=game['game'], rom_sha1=info['sha1'], name=s['name'], kind=s['kind'], address=fmt(s.get('address'), 8),
                   base=s.get('base') or s.get('of') or '', offset=fmt(s.get('offset')), size=s.get('size') or '',
                   count=s.get('count') or '', value=fmt(s.get('value')) if isinstance(s.get('value'), int) else '',
                   description=s.get('description') or '', verified=s.get('verified') or '', how=s.get('how') or '',
                   source=s.get('source') or '')
        w.writerow([row[c] for c in COLUMNS])
    return out.getvalue()


def as_sym(game, info, syms):
    """The no$gba symbol file (problemkaputt.de/gbahlp.htm, "Symbolic Debug
    Info"), which mGBA reads too (src/debugger/symbols.c,
    mDebuggerLoadARMIPSSymbols): an eight-digit address, a space, the name;
    ; opens a comment. no$gba's .thumb marks Thumb code (mGBA skips what
    opens with a dot)."""
    lines = [f'; {info["name"]}, SHA-1 {info["sha1"]} (header code {info["code"]})',
             '; Written by tools/symbols.py from Cyberworld Endless (github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless):',
             '; addresses, names and descriptions only. docs/SYMBOLS.md says what is here and how to load it:',
             '; beside the ROM with its name (game.gba, game.sym), or mGBA\'s debugger command load-symbols FILE.', '']
    out = []
    for s in syms.values():
        if 'address' not in s or s['kind'] not in ADDRESS_KINDS:
            continue
        a = s['address'] & ~1 if s['kind'] in ('function', 'code') else s['address']
        mode = s.get('mode') or ('thumb' if s['kind'] == 'code' and s['address'] & 1 else None)
        if s['kind'] == 'function' and mode:
            out.append((a, 0, '.' + mode))
        out.append((a, 1, s['name']))
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


def main():
    check = '--check' in sys.argv[1:]
    rows = rom_data_rows()
    files, problems, results = {}, [], []
    for g in GAMES:
        info, syms, missing = build(g, rows)
        problems += missing
        results.append((g, info, syms))
        files[f'{OUT}/{g["stem"]}.sym'] = as_sym(g, info, syms)
        files[f'{OUT}/{g["stem"]}.json'] = as_json(g, info, syms, rows)
        files[f'{OUT}/{g["stem"]}.csv'] = as_csv(g, info, syms)
    doc = doc_with(read(DOC), dict(holdings=holdings(results))) if os.path.exists(os.path.join(ROOT, DOC)) else None
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
        print(f'symbols: {sum(len(s) for _, _, s in results)} in {len(files)} files, {len(problems)} problems, '
              f'{len(stale)} stale')
        return 1 if problems or stale else 0
    os.makedirs(os.path.join(ROOT, OUT), exist_ok=True)
    for path, text in files.items():
        with open(os.path.join(ROOT, path), 'w', encoding='utf-8', newline='\n') as f:
            f.write(text)
    for g, info, syms in results:
        print(f'{g["stem"]}: {len(syms)} symbols ({", ".join(f"{n} {k}" for k, n in counts(syms).items())})')
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
