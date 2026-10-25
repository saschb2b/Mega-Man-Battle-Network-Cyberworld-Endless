#!/usr/bin/env python3
"""voice_catalog.py [--check]: every line the game's characters say, and
every word its own screens show, read from the words files (docs/VOICE.md,
Where the lines live) into docs/voice/catalog.json, which the voice review's
dashboard shows (docs/voice/README.md).

The words files stay the source of truth: this reads `src/*/*_words.c`,
`*_lines.c` and `*_text.c` (build.py's WORDS_FILES, the lint's `words`
smell) as C, and writes for each line

  - a stable id: the file's stem, the function or table it is in (a table
    inside a function as func.table), and its place there: the switch's
    case or the table's designator ([1] -> 1, .first -> first) or its
    element's index, then its count among the lines of that place. The id
    holds no text, so rewording a line keeps it;
  - its text as written (adjacent literals joined, a #define'd literal of
    the file's own spelled out, escapes decoded), printf's conversions as
    named placeholders ({name}, from the argument's expression where the
    call is in view, else {1}, {2}), split into boxes (`|`, and `\\f` in
    the PET's mails) of lines (`\\n`), each box's speaker from its mark
    (@M, src/layer/text.c's table), the call's face (FACE_MEGAMAN) or the
    file's and function's own (SPEAKERS below), and the box wrapped and
    paged as src/layer/text.c's wrap() does: twenty characters a line,
    three lines a page;
  - the domain (DOMAINS), the kind (speech, or a screen's words, which
    VOICE.md leaves to their own plain style), the comments that say what
    it is (the function's or table's own, in the .c and its header, and
    the nearest one above the line) and a picture of where it shows
    (SHOWS: a file in docs/screenshots or docs/clips, where one fits);
  - a hash of its text, which a review keeps to tell a line changed since;
  - flags: the checks of docs/VOICE.md a machine can make (FLAGS).

and a summary: per file, domain and speaker the counts, and per speaker
our numbers beside BN6's from VOICE.md's table (The numbers): words a box,
"!!" and "..." a box, stutters, openers. Numbers only of BN6.

For the review's page it writes the lines' order too: `conversations`, a
function's or table's lines together with a title from its comment, in
the order a run meets the areas (AREAS), and `sweep`, the voice sweep's
logs (docs/voice/sweep/*.md) read row by row, each row's lines found in
today's catalog by their words. The logs are inputs: a change to one makes
the catalog stale as a words file does.

  python3 tools/voice_catalog.py          write docs/voice/catalog.json
  python3 tools/voice_catalog.py --check  write nothing: fail when the file
                                          differs from what it would write
                                          (build.py lint)
"""
import glob
import hashlib
import json
import os
import re
import sys
from collections import Counter

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = 'docs/voice/catalog.json'
VOICE = 'docs/VOICE.md'
WORDS_FILES = ('_words.c', '_lines.c', '_text.c')   # (build.py's)
VERSION = 1

# ---- what each file is: its domain, its speaker, its kind, where it shows ----

# file stem -> (domain, default speaker, kind). The domains follow
# VOICE.md's table (Where the lines live); `screen` is a screen's own words
# (menus, labels, the second screen's panels), which VOICE.md leaves to
# their plain style: catalogued, not flagged.
DOMAINS = {
    'briefing_words': ('Briefing', 'MegaMan', 'speech'),
    'story_words': ('The story', 'MegaMan', 'speech'),
    'duel_words': ('Chaud and ProtoMan', 'Chaud', 'speech'),
    'board_words': ('NaviCust', 'MegaMan', 'speech'),
    'navicust_words': ('NaviCust', 'MegaMan', 'speech'),
    'dark_words': ('DarkChips', 'MegaMan', 'speech'),
    'nets_words': ('Where the nets meet', 'MegaMan', 'speech'),
    'lesson_words': ('Lessons', 'MegaMan', 'speech'),
    'powers_words': ('Powers', 'MegaMan', 'speech'),
    'pet_text': ('The PET', 'PET', 'speech'),
    'xchips_text': ('The PET', 'Screen', 'screen'),
    'guardian_lines': ('Guardians', 'Guardian', 'speech'),
    'super_lines': ('Super bosses', 'Bass', 'speech'),
    'layer_words': ('Layers: dealers and finds', 'Shop Navi', 'speech'),
    'service_words': ('Layers: services', 'MegaMan', 'speech'),
    'reward_words': ('Layers: rewards', 'MegaMan', 'speech'),
    'lock_words': ('Layers: locks', 'MegaMan', 'speech'),
    'npc_lines': ('Bystanders', 'Bystander', 'speech'),
    'rumor_lines': ('Bystanders', 'Bystander', 'speech'),
    'town_lines': ('The town', 'Townsfolk', 'speech'),
    'town_words': ('The town', 'Townsfolk', 'speech'),
    'place_lines': ('AsterLand and the Academy', 'Townsfolk', 'speech'),
    'job_words': ('Requests', 'Townsfolk', 'speech'),
    'home_words': ("Lan's HP and the gate", 'MegaMan', 'speech'),
    'gate_words': ("Lan's HP and the gate", 'MegaMan', 'speech'),
    'square_words': ('Net Square', 'MegaMan', 'speech'),
    'square_life_words': ('Net Square', 'Bystander', 'speech'),
    'square_week_words': ('Net Square', 'MegaMan', 'speech'),
    'online_text': ('Net Square', 'Screen', 'screen'),
    'netpass_text': ('The NetPass', 'Screen', 'screen'),
    'liberation_pick_text': ('Other', 'Screen', 'screen'),
    'trade_words': ('Trade Central', 'MegaMan', 'speech'),
    'trade_screen_text': ('Trade Central', 'Screen', 'screen'),
    'reconnect_text': ('Net Square', 'Screen', 'screen'),
    'arena_words': ('Arena', 'MegaMan', 'speech'),
    'arena_text': ('Arena', 'Screen', 'screen'),
    'stands_words': ('Arena', 'Bystander', 'speech'),
    'stands_text': ('Arena', 'Screen', 'screen'),
    'title_lines': ('Title', 'MegaMan', 'speech'),
    'title_setup_text': ('Title', 'Screen', 'screen'),
    'title_summary_text': ('Title', 'Screen', 'screen'),
    'launcher_text': ('The ROMs screen', 'Screen', 'screen'),
    'pick_text': ('The ROMs screen', 'Screen', 'screen'),
    'saves_text': ('SAVES', 'Screen', 'screen'),
    'analytics_text': ('Statistics', 'Screen', 'screen'),
    'options_text': ('Options', 'Screen', 'screen'),
    'second_text': ('The second screen', 'Screen', 'screen'),
}
DEFAULT_DOMAIN = ('Other', 'MegaMan', 'speech')

# Where a line's own place says more than its file: (id regex, speaker,
# kind or None). The first match wins.
OVERRIDES = [
    (r'^guardian_lines:guardian_(tip|netbattle_terms)\b', 'MegaMan', None),
    (r'^guardian_lines:(replies|answer|first_sight)\b', 'MegaMan', None),
    (r'^guardian_lines:guardian_rumor\b', 'Bystander', None),
    (r'^super_lines:\w*cybeast\w*', 'Cybeast', None),
    (r'^online_text:online_phrases\b', 'Players', 'speech'),
    (r'^square_week_words:square_week_board\b', 'Narration', None),
    (r'^square_words:square_greeter_stamp_words\b', 'Mr.Prog', None),
    (r'^stands_words:stands_board_words\b', 'Narration', None),
    (r'^navicust_words:', 'MegaMan', None),
    # (a liberation's teammates, each his own table and command's case: liberation_team_words.c)
    (r'^liberation_team_words:(colonel|\w+:caseMATE_COLONEL)\b', 'Colonel', None),
    (r'^liberation_team_words:(shadowman|\w+:caseMATE_SHADOWMAN)\b', 'ShadowMan', None),
    (r'^liberation_team_words:(numberman|\w+:caseMATE_NUMBERMAN)\b', 'NumberMan', None),
    (r'^liberation_team_words:(tomahawkman|\w+:caseMATE_TOMAHAWKMAN)\b', 'TomahawkMan', None),
    (r'^liberation_team_words:(knightman|\w+:caseMATE_KNIGHTMAN)\b', 'KnightMan', None),
    (r'^liberation_team_words:(toadman|\w+:caseMATE_TOADMAN)\b', 'ToadMan', None),
    (r'^liberation_team_words:(heatman|\w+:caseMATE_HEATMAN)\b', 'HeatMan', None),
    (r'^liberation_team_words:(elecman|\w+:caseMATE_ELECMAN)\b', 'ElecMan', None),
    (r'^liberation_team_words:(slashman|\w+:caseMATE_SLASHMAN)\b', 'SlashMan', None),
    (r'^liberation_team_words:(eraseman|\w+:caseMATE_ERASEMAN)\b', 'EraseMan', None),
    (r'^liberation_team_words:(chargeman|\w+:caseMATE_CHARGEMAN)\b', 'ChargeMan', None),
    (r'^liberation_team_words:lib_soul_form_name\b', 'Screen', 'screen'),
    (r'^trade_screen_text:trade_text_(ask|waiting|swapping|traded)\b', 'MegaMan', 'speech'),
    (r'^story_words:(area_card|nets_card|clear_card)\b', 'Screen', 'screen'),
    (r'^pet_text:(report_text|records_text|where_text|codes_known|codes_text|install_descriptions|install_mails\.subjects)\b', 'Screen', 'screen'),
    (r'^pet_text:bbs_posts\b', 'Bystander', None),
    (r'^pet_text:(mail_body|cross_words)\b', 'Dad', None),
    (r':[\w.]*(options|buttons|labels?|titles?|hints?|names?)\b[\w.]*:', None, 'screen'),
]

# Files whose words are no chat box's (the PET's mails and BBS have screens
# of their own): the box's shape is not checked there.
NO_BOX = {'pet_text'}
SHAPE = {'pages', 'line-long', 'word-long', 'box-words', 'beats', 'long-speech'}
# Speakers whose words are the game's own voice, not a person's: only the
# box's shape is checked.
PLAIN = {'Narration', 'Screen'}

# A box's mark (src/layer/text.c, speaker()): its speaker.
MARKS = {'L': 'Lan', 'M': 'MegaMan', 'D': 'Dad', 'P': 'Mr.Prog', 'B': 'MegaMan', 'C': 'Chaud',
         'Y': 'Mayl', 'H': 'HeelNavi', 'N': 'Narration'}
# A call's face (src/layer/text.h): its speaker. FACE_NE and the like are
# facings, not faces, and are not here.
FACES = {'FACE_NONE': 'Narration', 'FACE_LAN': 'Lan', 'FACE_MAYL': 'Mayl', 'FACE_CHAUD': 'Chaud',
         'FACE_DAD': 'Dad', 'FACE_MEGAMAN': 'MegaMan', 'FACE_CLERK': 'Shop Navi', 'FACE_PROG': 'Mr.Prog',
         'FACE_NAVI': 'Shop Navi', 'FACE_TECH': 'Shop Navi', 'FACE_HEEL': 'HeelNavi', 'FACE_BEAST': 'MegaMan'}
# Our speakers in VOICE.md's table of numbers (The numbers).
NUMBERS_GROUP = {'Lan': 'Lan', 'MegaMan': 'MegaMan', 'Mr.Prog': 'Mr.Prog', 'Dad': 'Dad', 'Chaud': 'Chaud',
                 'Guardian': 'Navis (bosses, bystanders)', 'Bystander': 'Navis (bosses, bystanders)',
                 'HeelNavi': 'Navis (bosses, bystanders)', 'Shop Navi': 'Navis (bosses, bystanders)',
                 'ProtoMan': 'Navis (bosses, bystanders)', 'Bass': 'Navis (bosses, bystanders)',
                 'Colonel': 'Navis (bosses, bystanders)', 'ShadowMan': 'Navis (bosses, bystanders)',
                 'NumberMan': 'Navis (bosses, bystanders)', 'TomahawkMan': 'Navis (bosses, bystanders)',
                 'KnightMan': 'Navis (bosses, bystanders)', 'ToadMan': 'Navis (bosses, bystanders)'}

# Where a file's lines show: pictures in docs/ (screenshots and clips'
# posters), where one fits.
SHOWS = {
    'briefing_words': ['screenshots/voice-banter.png', 'screenshots/sense.png'],
    'story_words': ['screenshots/act-card.png', 'screenshots/area-clear.png'],
    'duel_words': ['screenshots/rival.png'],
    'board_words': ['screenshots/program-fit-now.png'],
    'dark_words': ['screenshots/dark-chip-price.png'],
    'nets_words': ['screenshots/bn5-visitor.png'],
    'pet_text': ['screenshots/pet-mail.png', 'screenshots/bbs.png', 'screenshots/pet-records.png'],
    'guardian_lines': ['screenshots/guardian-talk.png', 'screenshots/guardian-meet.png', 'screenshots/split-question.png'],
    'super_lines': ['clips/bass-arrival.png', 'clips/cybeast-arrival.png'],
    'layer_words': ['screenshots/hidden-hint.png', 'screenshots/compare-dealer.png'],
    'service_words': ['screenshots/heal-once.png', 'screenshots/heal-spent.png', 'screenshots/dark-flame-words.png'],
    'reward_words': ['screenshots/reward.png'],
    'lock_words': ['screenshots/cube-teller.png', 'screenshots/cube-code.png', 'screenshots/doors-skull.png'],
    'npc_lines': ['screenshots/rumor.png'],
    'rumor_lines': ['screenshots/rumor-2.png', 'screenshots/rumor-3.png'],
    'town_lines': ['screenshots/town-central.png'],
    'town_words': ['screenshots/home-news.png'],
    'place_lines': ['screenshots/home-academy.png'],
    'job_words': ['screenshots/home-request.png'],
    'home_words': ['screenshots/home-back.png', 'screenshots/home-courier.png'],
    'title_lines': ['screenshots/summary.png'],
    'title_summary_text': ['screenshots/summary.png', 'screenshots/summary-records.png'],
    'launcher_text': ['screenshots/launcher-desktop.png', 'screenshots/launcher-phone.png'],
    'saves_text': ['screenshots/saves-home.png', 'screenshots/saves-compare.png'],
    'analytics_text': ['screenshots/stats-ask.png'],
    'second_text': ['screenshots/second-screen.png', 'screenshots/second-status.png'],
}

# ---- the checks (docs/VOICE.md) ----

LINE_CHARS = 20      # src/layer/text.c's LINE_CHARS
PAGE_LINES = 3
BOX_WORDS = 12       # "Six words is normal,twelve the most."
BOX_BEATS = 3        # sentences in one box: "One beat a box"
SPEECH_BOXES = 3     # "three boxes is a lot for one person at once"
WIDTH = {'s': 8, 'd': 3, 'i': 3, 'u': 3, 'c': 1, 'x': 4, 'X': 4, 'ld': 4, 'lu': 4, 'zu': 3}   # a placeholder's typical width
FLAGS = {
    'pages': 'pages on: the box wraps past three lines of twenty characters',
    'line-long': 'a line broken by hand runs past twenty characters, so the box wraps it again',
    'word-long': 'a word (or words joined by commas) past twenty characters, which the box cuts',
    'box-words': 'more than twelve words in one box: one beat a box, six words is normal',
    'beats': 'three sentences or more in one box: one beat a box',
    'long-speech': 'more than three boxes in a row from one speaker',
    'prog-caps': 'Mr.Prog speaks in ALL CAPITALS',
    'comma-space': 'a space after a comma: BN6 writes Lan,look!',
    'colon': 'a colon in speech: the summary voice',
    'semicolon': 'a semicolon in speech: the summary voice',
    'dash': 'a dash in speech: the summary voice',
    'paren': 'parentheses in speech',
    'summary': '"which" or "whose" joining facts: the summary voice',
    'glue': 'a formal glue word (however, additionally, therefore, in order to, currently...): BN6 says But, So, And',
    'rule-talk': 'rule talk ("this layer", "this run", "the player"): characters speak in their world',
    'hedge': 'hedging ("it seems that", "you might want to"): BN6\'s people say it straight',
    'screen-talk': 'explaining the screen ("you can see", "on the screen")',
    'direction-hyphen': 'a direction with a hyphen ("down-left"): directions are words, "to the lower left"',
    'the-net': '"the net" without its capital: BN6 writes the Net',
    'spelling': 'a name not written as BN6 writes it',
    'meta': 'a meta word (saving, game over, a run, checkpoint, level up, stats): nobody in BN6 knows the game is a game',
    'button': 'a button or control named: BN6 names one once, at its first use, by whoever needs it',
}
GLUE = re.compile(r'\b(however|additionally|therefore|in order to|currently|furthermore|moreover|thus|hence|consequently|nevertheless)\b', re.I)
RULE_TALK = re.compile(r'\b(this layer|this run|the player\'?s?|per run)\b', re.I)
HEDGE = re.compile(r'\b(it seems that|it seems like|you might want to|you may want to|it appears that|perhaps you)\b', re.I)
META = re.compile(r"\b(sav(?:e|es|ed|ing)\b(?! up)(?! (?:dad|him|her|them|us|the net|it)\b)|save point|game over|levels? up|leveled up|stats\b|EXP\b|checkpoints?|retry|reload|respawn|this run|a run\b|the run\b|your run|our run|every run|new run)", re.I)
BUTTON = re.compile(r"\b(?:press|hold|tap|push)\s+(?:the\s+)?(?:A|B|L|R|START|SELECT|UP|DOWN|LEFT|RIGHT)\b|\b(?:A|B|L|R|START|SELECT) (?:Button|BUTTON)\b|\bbuttons?\b", re.I)
SCREEN_TALK = re.compile(r'\b(you can see|on the screen|on screen|the screen shows)\b', re.I)
DIRECTION = re.compile(r'\b(up|down|upper|lower)-(left|right)\b', re.I)
THE_NET = re.compile(r'\bthe net\b(?![\w-])')
SPELLINGS = [(re.compile(p), right) for p, right in [
    (r'\bMr\. Prog\b|\bMr Prog\b', 'Mr.Prog'), (r'\bzenny\b', 'Zenny'), (r'\b(Netbattle|net battle|Net Battle|netbattle)\b', 'NetBattle'),
    (r'\b(Navicust|Navi Cust|naviCust|navicust)\b', 'NaviCust'), (r'\b(Bugfrag|bugfrag|Bug Frag)\b', 'BugFrag'),
    (r'\b(HP Memory|HPmemory|Hpmemory)\b', 'HPMemory'), (r'\b(custom screen|Custom screen)\b', 'Custom Screen'),
    (r'\b(Gigachip|Giga Chip|gigachip)\b', 'GigaChip'), (r'\b(Levbus|Lev Bus)\b', 'LevBus'),
]]
# the game's own voice over a chat: what MegaMan got, his HP
NARRATION = re.compile(r"(MegaMan|Lan) (got|recovered|received|learned|obtained)\b|MegaMan's HP\b")
STUTTER = re.compile(r'\b([A-Za-z])-\1', re.I)
PLACEHOLDER = re.compile(r'\{([^{}]+)\}')

# ---- reading C ----

TOKEN = re.compile(r'''
    (?P<comment>/\*.*?\*/|//[^\n]*)
  | (?P<str>"(?:[^"\\\n]|\\.)*")
  | (?P<char>'(?:[^'\\\n]|\\.)*')
  | (?P<id>[A-Za-z_]\w*)
  | (?P<num>\.?\d[\w.]*)
  | (?P<punct>->|\+\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\|\||[-+*/%&|^!~<>=]=?|[{}()\[\];,?:.#])
  | (?P<ws>\s+)
''', re.S | re.X)
KEYWORDS = {'if', 'while', 'for', 'switch', 'return', 'sizeof', 'case', 'else', 'do'}
# calls whose strings are no one's words
NOT_WORDS = {'fopen', 'strcmp', 'strncmp', 'strstr', 'strchr', 'strrchr', 'strcspn', 'strspn', 'getenv', 'perror',
             'SDL_Log', 'log_msg', 'emu_log', 'fputs', 'puts', 'sscanf', 'ini_get', 'ini_set', 'ini_str', 'ini_int', 'debug'}


class Tok:
    __slots__ = ('kind', 'text', 'line', 'end')

    def __init__(self, kind, text, line, end):
        self.kind, self.text, self.line, self.end = kind, text, line, end


def decode(lit):
    """A C string literal's body (between its quotes) decoded."""
    out, i = [], 0
    simple = {'n': '\n', 't': '\t', 'r': '\r', 'f': '\f', 'v': '\v', 'a': '\a', 'b': '\b',
              '\\': '\\', '"': '"', "'": "'", '?': '?', '0': '\0'}
    while i < len(lit):
        c = lit[i]
        if c != '\\':
            out.append(c)
            i += 1
            continue
        n = lit[i + 1]
        if n == 'x':
            m = re.match(r'[0-9A-Fa-f]+', lit[i + 2:])
            out.append(chr(int(m.group(0), 16)))
            i += 2 + len(m.group(0))
        elif n in '01234567':
            m = re.match(r'[0-7]{1,3}', lit[i + 1:])
            out.append(chr(int(m.group(0), 8)))
            i += 1 + len(m.group(0))
        else:
            out.append(simple.get(n, n))
            i += 2
    return ''.join(out)


def lex(src):
    """The tokens of C source `src`, comments among them, and the lines
    that hold a preprocessor directive, with the file's string macros
    (#define NAME "...")."""
    toks, pp, macros = [], set(), {}
    line, i, bol = 1, 0, True
    while i < len(src):
        if bol:
            m = re.match(r'[ \t]*#', src[i:])
            if m:   # a directive, to its end (and its continuations)
                j = i
                while True:
                    e = src.find('\n', j)
                    e = len(src) if e < 0 else e
                    if src[e - 1:e] == '\\':
                        j = e + 1
                        continue
                    break
                text = src[i:e]
                n = text.count('\n')
                for k in range(line, line + n + 1):
                    pp.add(k)
                d = re.match(r'\s*#\s*define\s+(\w+)\s+((?:"(?:[^"\\]|\\.)*"\s*)+)$', text.replace('\\\n', ' '))
                if d:
                    macros[d.group(1)] = ''.join(decode(s) for s in re.findall(r'"((?:[^"\\]|\\.)*)"', d.group(2)))
                line += n
                i = e
                continue
        m = TOKEN.match(src, i)
        if not m:
            i += 1
            continue
        kind, text = m.lastgroup, m.group(0)
        n = text.count('\n')
        if kind != 'ws':
            toks.append(Tok(kind, text, line, line + n))
        bol = kind == 'ws' and '\n' in text or (kind == 'comment' and text.startswith('//'))
        if kind == 'ws' and '\n' in text:
            bol = True
        line += n
        i = m.end()
    return toks, pp, macros


def clean_comment(text):
    """A comment's words, without its markers, as one paragraph."""
    if text.startswith('//'):
        return text[2:].strip()
    body = text[2:-2]
    lines = [re.sub(r'^\s*\*\s?', '', ln).strip() for ln in body.split('\n')]
    return re.sub(r'\s+', ' ', ' '.join(ln for ln in lines if ln)).strip()


def header_comments(path):
    """The comments over each declaration in the header `path`: name -> comment."""
    if not os.path.exists(path):
        return {}
    with open(path, errors='replace') as f:
        toks, pp, _ = lex(f.read())
    out, last, head, depth = {}, None, [], 0
    for t in toks:
        if t.kind == 'comment':
            last = t
            continue
        if t.text == '{':
            depth += 1
        elif t.text == '}':
            depth -= 1
        if depth:
            continue
        if t.text in (';', '}'):
            name = None
            for k, h in enumerate(head):
                if h.kind == 'id' and k + 1 < len(head) and head[k + 1].text in ('(', '['):
                    name = h.text
                    break
            if name is None:
                ids = [h.text for h in head if h.kind == 'id']
                name = ids[-1] if ids else None
            if name and last is not None and head and last.end >= head[0].line - 1:
                out.setdefault(name, clean_comment(last.text))
            head, last = [], None if t.text == ';' else last
            continue
        head.append(t)
    return out


def struct_fields(toks):
    """The members of a struct's body, in order: const char *first, *then;
    int n; -> first, then, n."""
    names, decl = [], []
    for t in toks + [Tok('punct', ';', 0, 0)]:
        if t.kind == 'comment':
            continue
        if t.text != ';':
            decl.append(t)
            continue
        for i, d in enumerate(decl):
            if d.kind != 'id':
                continue
            before = decl[i - 1].text if i else ''
            after = decl[i + 1].text if i + 1 < len(decl) else ';'
            if before in ('*', ',') or (after in (',', ';', '[') and i > 0):
                names.append(d.text)
        decl = []
    return names


class Frame:
    """An open call's parentheses: the callee and where each argument starts."""
    __slots__ = ('callee', 'args', 'pending', 'face')

    def __init__(self, callee, at):
        self.callee, self.args, self.pending, self.face = callee, [at], [], None


NOT_NAMES = KEYWORDS | {'true', 'false', 'NULL', 'int', 'unsigned', 'char', 'const', 'size_t', 'bool'}


def arg_name(toks):
    """A placeholder's name from its argument's tokens: the last identifier
    at the expression's own level (guardian(navi)->name -> name), casts and
    brackets aside; of a ternary, its first branch's name, else its
    second's, else its condition's (stands ? "" : "|...", -> stands)."""
    level, q, c = 0, None, None
    for k, t in enumerate(toks):
        if t.text in ('(', '['):
            level += 1
        elif t.text in (')', ']'):
            level -= 1
        elif level == 0 and t.text == '?' and q is None:
            q = k
        elif level == 0 and t.text == ':' and q is not None and c is None:
            c = k
    if q is not None and c is not None:
        return expr_name(toks[q + 1:c]) or expr_name(toks[c + 1:]) or expr_name(toks[:q])
    return expr_name(toks)


def expr_name(toks):
    level, best = 0, None
    for t in toks:
        if t.text in ('(', '['):
            level += 1
        elif t.text in (')', ']'):
            level -= 1
        elif level == 0 and t.kind == 'id' and t.text not in NOT_NAMES:
            best = t.text
    if best is None:
        ids = [t.text for t in toks if t.kind == 'id' and t.text not in NOT_NAMES]
        best = ids[-1] if ids else None
    return best


SPEC = re.compile(r'%(?P<flags>[-+ 0#]*)(?P<width>\*|\d+)?(?:\.(?P<prec>\*|\d+))?(?P<len>hh|h|ll|l|z|j|t)?(?P<conv>[diouxXcsfeEgGp%])')
SIMPLE_SPEC = re.compile(r'%(?P<flags>)(?P<width>\d+)?(?P<prec>)(?P<len>l|z)?(?P<conv>[dsuc%])')


def placeholders(text, args, printf):
    """`text` with its conversions as {name}, and the placeholders: names
    from `args` (each a list of tokens) in order, else {1}, {2}."""
    found, out, k, pos = [], [], 0, 0
    pat = SPEC if printf else SIMPLE_SPEC
    for m in pat.finditer(text):
        out.append(text[pos:m.start()])
        pos = m.end()
        if m.group('conv') == '%':
            out.append('%')
            continue
        k += (m.group('width') == '*') + (m.group('prec') == '*')   # (a * takes an argument of its own)
        name = arg_name(args[k]) if k < len(args) else None
        k += 1
        name = name or str(len(found) + 1)
        name = re.sub(r'[^A-Za-z0-9_]', '_', name)
        conv = (m.group('len') or '') + m.group('conv')
        found.append({'name': name, 'spec': m.group(0), 'width': WIDTH.get(conv, WIDTH.get(m.group('conv'), 8))})
        out.append('{' + name + '}')
    out.append(text[pos:])
    return ''.join(out), found


# ---- the game's chat box ----

def word_width(w, widths):
    """A word's width in the box: placeholders at their typical width."""
    n, pos = 0, 0
    for m in PLACEHOLDER.finditer(w):
        n += len(w[pos:m.start()]) + widths.get(m.group(1), 8)
        pos = m.end()
    return n + len(w[pos:]) + sum(1 for c in w if '\x01' <= c <= '\x05')


def wrap(box, widths):
    """The box's lines as src/layer/text.c's wrap() makes them (words not cut),
    and its pages: the fewest pages of three, a page of one line and a page
    turned mid-sentence costing most."""
    lines, cur, n = [], [], 0
    for part in box.split('\n'):
        for w in part.split(' '):
            if not w:
                continue
            ww = word_width(w, widths)
            if cur and n + 1 + ww > LINE_CHARS:
                lines.append(' '.join(cur))
                cur, n = [], 0
            n += (1 if cur else 0) + ww
            cur.append(w)
        if cur:
            lines.append(' '.join(cur))
            cur, n = [], 0
    nl = len(lines)
    if not nl:
        return [], []
    pages = (nl + 2) // 3
    big = 1 << 20
    cost = [[big] * (pages + 1) for _ in range(nl + 1)]
    frm = [[0] * (pages + 1) for _ in range(nl + 1)]
    cost[0][0] = 0
    end = lambda s: s.endswith(('.', '!', '?'))
    for k in range(1, pages + 1):
        for i in range(1, nl + 1):
            for take in range(1, min(3, i) + 1):
                prev = cost[i - take][k - 1]
                if prev >= big:
                    continue
                c = prev + (3 if take == 1 and nl > 1 else 0) + (2 if i < nl and not end(lines[i - 1]) else 0)
                if c < cost[i][k]:
                    cost[i][k], frm[i][k] = c, i - take
    ends, i = [0] * pages, nl
    for k in range(pages, 0, -1):
        ends[k - 1] = i
        i = frm[i][k]
    out, i = [], 0
    for e in ends:
        out.append(lines[i:e])
        i = e
    return lines, out


# ---- one file ----

class Reader:
    """One words file read into its lines."""

    def __init__(self, rel, guardians):
        self.rel = rel
        self.stem = os.path.basename(rel)[:-2]
        self.guardians = guardians
        with open(os.path.join(ROOT, rel), errors='replace') as f:
            self.src = f.read()
        self.toks, self.pp, self.macros = lex(self.src)
        self.header = header_comments(os.path.join(ROOT, rel[:-2] + '.h'))
        self.containers = {}
        self.entries = []
        self.fields = None   # the members of the struct a table is declared with, in order

    def file_comment(self):
        for t in self.toks[:1]:
            if t.kind == 'comment' and t.line <= 2:
                return clean_comment(t.text)
        return ''

    def doc_before(self, k):
        """The comment right over the declaration starting at token k."""
        j = k - 1
        if j >= 0 and self.toks[j].kind == 'comment':
            c = self.toks[j]
            if not any(ln in self.pp for ln in range(c.end + 1, self.toks[k].line)) and c.end >= self.toks[k].line - 2:
                return clean_comment(c.text)
        return ''

    def read(self):
        toks = [t for t in self.toks]
        code = [k for k, t in enumerate(toks) if t.kind != 'comment']
        # brace stack: dicts with kind func/init/block/type, name, elem, desig
        stack, frames, case = [], [], {}
        head_start = None      # the first token of the current top-level declaration
        last_comment = None    # the last comment inside a container: (token, index)
        counters = Counter()
        group = None           # the literal group being read: list of (text, tok)
        prev = None            # the previous code token
        k = 0

        def container():
            names = [s['name'] for s in stack if s['kind'] in ('func', 'init_root') and s['name']]
            return '.'.join(names) if names else None

        def place():
            """The line's place in its container: designators or indexes of
            the initializer's levels, or the switch's case."""
            parts = []
            root = None
            for i, s in enumerate(stack):
                if s['kind'] == 'init_root':
                    root = i
            if root is not None:
                fields = stack[root].get('fields') or []
                for depth, s in enumerate(stack[root:]):
                    if s['kind'] in ('init_root', 'init'):
                        if s['desig'] is None and depth == 1 and s['elem'] < len(fields):
                            parts.append(fields[s['elem']])   # (a row's member by its name)
                        else:
                            parts.append(s['desig'] if s['desig'] is not None else str(s['elem']))
            labels = [case[d] for d in sorted(case) if d <= len(stack)]
            if labels:
                parts.insert(0, 'case' + labels[-1])
            return '.'.join(parts)

        def flush():
            nonlocal group
            if group is None:
                return
            g, group = group, None
            text = ''.join(s for s, _ in g)
            first = g[0][1]
            frame = frames[-1] if frames else None
            if frame and frame.callee in NOT_WORDS:
                return
            name = container() or '_'
            entry = {'text': text, 'tok': first, 'container': name, 'place': place(),
                     'frame': frame, 'arg': len(frame.args) - 1 if frame else None, 'note': None}
            if last_comment is not None and last_comment.end >= first.line - 4 and stack:
                entry['note'] = clean_comment(last_comment.text)
            if frame:
                frame.pending.append(entry)
            self.entries.append(entry)

        while k < len(toks):
            t = toks[k]
            if t.kind == 'comment':
                if stack:
                    last_comment = t
                k += 1
                continue
            is_str = t.kind == 'str' or (t.kind == 'id' and t.text in self.macros and group is not None) or \
                (t.kind == 'id' and t.text in self.macros and prev is not None and prev.text in ('(', ',', '?', ':', '=', '{', 'return'))
            if is_str:
                s = decode(t.text[1:-1]) if t.kind == 'str' else self.macros[t.text]
                if group is None:
                    group = []
                group.append((s, t))
                prev = t
                k += 1
                continue
            flush()
            if not stack and head_start is None and t.text not in (';', '}'):
                head_start = k
            if t.text == '(':
                callee = prev.text if prev is not None and prev.kind == 'id' and prev.text not in KEYWORDS else None
                frames.append(Frame(callee, k + 1))
            elif t.text == ',' and frames and self._level(frames[-1], toks, k):
                frames[-1].args.append(k + 1)
            elif t.text == ')' and frames:
                f = frames.pop()
                args = []
                for a, b in zip(f.args, f.args[1:] + [k + 1]):
                    args.append([x for x in toks[a:b - 1] if x.kind != 'comment'])
                self._resolve(f, args)
            elif t.text == '{':
                kind, name = self._brace_kind(toks, k, head_start, stack)
                if kind in ('func', 'init_root') and not stack:
                    doc = self.doc_before(head_start) if head_start is not None else ''
                    hdr = self.header.get(name, '')
                    self.containers.setdefault(name, {'line': toks[head_start].line if head_start is not None else t.line,
                                                      'comment': doc, 'header': hdr})
                elif kind == 'init_root' and name:
                    full = '.'.join([s['name'] for s in stack if s['kind'] in ('func', 'init_root') and s['name']] + [name])
                    self.containers.setdefault(full, {'line': t.line, 'comment': '', 'header': ''})
                stack.append({'kind': kind, 'name': name, 'elem': 0, 'desig': None, 'at': k})
                if kind == 'init_root' and self.fields is not None:
                    stack[-1]['fields'], self.fields = self.fields, None
                if kind in ('init_root', 'init'):
                    k = self._designator(toks, k + 1, stack[-1])
                    prev = t
                    continue
            elif t.text == '}':
                if stack:
                    s = stack.pop()
                    case.pop(len(stack) + 1, None)
                    if s['kind'] == 'type':
                        self.fields = struct_fields(toks[s['at'] + 1:k])
                        prev = t
                        k += 1
                        continue
                if not stack:
                    if head_start is not None and not self._initializer(toks, head_start, k):
                        head_start = None
                    last_comment = None
            elif t.text == ';':
                if not stack:
                    if head_start is not None and self._is_global_init(toks, head_start, k):
                        pass
                    head_start = None
                    last_comment = None
            elif t.text == ',' and stack and stack[-1]['kind'] in ('init_root', 'init') and not frames:
                stack[-1]['elem'] += 1
                stack[-1]['desig'] = None
                k = self._designator(toks, k + 1, stack[-1])
                prev = t
                continue
            elif t.text == 'case' or (t.text == 'default' and k + 1 < len(toks) and toks[k + 1].text == ':'):
                j = k + 1
                label = []
                while j < len(toks) and toks[j].text != ':':
                    if toks[j].kind != 'comment':
                        label.append(toks[j].text)
                    j += 1
                case[len(stack)] = re.sub(r'[^A-Za-z0-9_-]', '_', ''.join(label)) if t.text == 'case' else 'default'
            elif t.text == '=' and not stack and head_start is not None:
                pass
            prev = t
            k += 1
        flush()
        # top-level declarations initialized with a literal (const char x[] = "...";)
        for e in self.entries:
            if e['container'] == '_':
                e['container'] = self._global_name(e['tok'])
        return self

    def _global_name(self, tok):
        """The name a top-level `x[] = "..."` declares, for its literal."""
        k = self.toks.index(tok)
        j = k - 1
        while j >= 0 and self.toks[j].text != '=':
            j -= 1
        j -= 1
        while j >= 0 and self.toks[j].text in (']', '['):
            j -= 1
            while j >= 0 and self.toks[j].text not in ('[',) and self.toks[j + 1].text == ']':
                j -= 1
        while j >= 0 and self.toks[j].kind != 'id':
            j -= 1
        name = self.toks[j].text if j >= 0 else '_'
        if name not in self.containers:
            start = j
            while start > 0 and self.toks[start - 1].text not in (';', '}') and self.toks[start - 1].kind != 'comment':
                start -= 1
            self.containers[name] = {'line': self.toks[start].line, 'comment': self.doc_before(start) if start < len(self.toks) else '',
                                     'header': self.header.get(name, '')}
        return name

    @staticmethod
    def _level(frame, toks, k):
        """Whether the comma at k separates the frame's own arguments (not
        a nested brace's elements)."""
        depth = 0
        for j in range(frame.args[-1], k):
            if toks[j].text in ('{', '['):
                depth += 1
            elif toks[j].text in ('}', ']'):
                depth -= 1
        return depth == 0

    @staticmethod
    def _initializer(toks, start, k):
        return False

    @staticmethod
    def _is_global_init(toks, start, k):
        return any(t.text == '=' for t in toks[start:k])

    def _brace_kind(self, toks, k, head_start, stack):
        """What the brace at k opens: a function's body, an initializer (its
        root, or a level inside one), a struct's or enum's body, or a block."""
        j = k - 1
        while j >= 0 and toks[j].kind == 'comment':
            j -= 1
        before = toks[j] if j >= 0 else None
        if stack and stack[-1]['kind'] in ('init_root', 'init'):
            return 'init', None
        if before is not None and before.text == '=':
            # the name before =, past its [..]
            i = j - 1
            while i >= 0 and toks[i].text == ']':
                d = 0
                while i >= 0:
                    if toks[i].text == ']':
                        d += 1
                    elif toks[i].text == '[':
                        d -= 1
                        if d == 0:
                            break
                    i -= 1
                i -= 1
            while i >= 0 and toks[i].kind == 'comment':
                i -= 1
            name = toks[i].text if i >= 0 and toks[i].kind == 'id' else None
            if stack and stack[-1]['kind'] != 'func' and not any(s['kind'] == 'func' for s in stack):
                return 'init', None
            return 'init_root', name
        if before is not None and (before.text in ('struct', 'enum', 'union') or
                                   (before.kind == 'id' and j > 0 and toks[j - 1].text in ('struct', 'enum', 'union'))):
            return 'type', None
        if not stack and before is not None and before.text == ')':
            # the function's name: the identifier before the head's first (
            i = head_start if head_start is not None else 0
            while i < k and toks[i].text != '(':
                i += 1
            name = toks[i - 1].text if i > 0 and toks[i - 1].kind == 'id' else None
            return 'func', name
        return 'block', None

    def _designator(self, toks, k, level):
        """A designator at the start of an initializer's element ([X] = or
        .f =): read into `level`; the index of the token after it."""
        while k < len(toks) and toks[k].kind == 'comment':
            k += 1
        if k < len(toks) and toks[k].text == '[':
            j, parts = k + 1, []
            while j < len(toks) and toks[j].text != ']':
                parts.append(toks[j].text)
                j += 1
            if j + 1 < len(toks) and toks[j + 1].text == '=':
                level['desig'] = re.sub(r'[^A-Za-z0-9_-]', '_', ''.join(parts))
                return j + 2
        if k + 2 < len(toks) and toks[k].text == '.' and toks[k + 1].kind == 'id' and toks[k + 2].text == '=':
            level['desig'] = toks[k + 1].text
            return k + 3
        return k

    def _resolve(self, frame, args):
        """A closed call's lines: their placeholders' names and the call's face."""
        face = None
        for a in args:
            for t in a:
                if t.kind == 'id' and t.text in FACES:
                    face = FACES[t.text]
        for e in frame.pending:
            e['face'] = face
            e['after'] = args[e['arg'] + 1:] if e['arg'] is not None else []
            e['printf'] = bool(frame.callee and ('printf' in frame.callee or frame.callee == 'ADD')) or bool(e['after'])


# ---- a line ----

def is_line(text):
    """Whether a literal's text is someone's words, not a format's glue,
    a name or a path."""
    bare = SPEC.sub('', text)
    if len(re.findall(r'[A-Za-z]', bare)) < 2:
        return False
    if re.fullmatch(r'[\w./%:+-]+', text) and ('/' in text or '.' in text.strip('.') or '_' in text):
        return False
    if not re.search(r'[\s|]', text.strip()) and not re.search(r'[!?.]', text):
        return False   # one word: a name or a label
    return True


def box_speaker(box, default, kind, row_face):
    # (a mark may follow a placeholder that holds boxes of its own: "%s@C ...")
    m = re.match(r'((?:\{[^}]*\})*)@([A-Z]) ?', box)
    if m and m.group(2) in MARKS:
        return MARKS[m.group(2)], m.group(1) + box[m.end():]
    if row_face:
        return row_face, box
    if kind == 'speech' and NARRATION.match(re.sub(r'^(\{[^}]*\})+', '', box)):
        return 'Narration', box
    letters = re.sub(r'\{[^}]*\}', '', box)
    letters = re.findall(r'[A-Za-z]', letters)
    if kind == 'speech' and default not in ('Narration', 'Screen', 'Players') and len(letters) >= 6 and all(c.isupper() for c in letters):
        return 'Mr.Prog', box
    return default, box


def flag_box(speaker, text, widths, wrapped):
    """The flags of one box (VOICE.md's checks)."""
    out = []
    bare = PLACEHOLDER.sub('X', text)
    if len(wrapped) > PAGE_LINES:
        out.append(('pages', f'{len(wrapped)} lines'))
    if '\n' in text and any(word_width(ln, widths) > LINE_CHARS for ln in text.split('\n')):
        out.append(('line-long', max((ln for ln in text.split('\n')), key=lambda ln: word_width(ln, widths))))
    for w in text.split():
        if word_width(w, widths) > LINE_CHARS and len(PLACEHOLDER.findall(w)) < 2:
            out.append(('word-long', w))
            break
    words = len(re.findall(r"[A-Za-z0-9']+", bare))
    if words > BOX_WORDS:
        out.append(('box-words', f'{words} words'))
    beats = len(re.findall(r'(?:[!?]+|(?<!\.)\.(?!\.)|\.\.\.+)["\')]*(?=\s+["(]?[A-Z]|\s*$)', bare.strip()))
    if beats >= BOX_BEATS and words > 9:
        out.append(('beats', f'{beats} sentences'))
    if speaker in PLAIN:
        return out
    if speaker == 'Mr.Prog' and re.search(r'[a-z]', bare):
        out.append(('prog-caps', re.search(r'\w*[a-z]\w*', bare).group(0)))
    if re.search(r', ', bare):
        out.append(('comma-space', re.search(r'\S*, \S*', bare).group(0)))
    if re.search(r'(?<!\d):(?!\d)', PLACEHOLDER.sub('0', text)):
        out.append(('colon', ':'))
    if ';' in bare:
        out.append(('semicolon', ';'))
    if re.search(r' - |--|—|–', bare):
        out.append(('dash', '-'))
    if re.search(r'[()]', bare):
        out.append(('paren', '()'))
    m = re.search(r'(?<![.!?] )(?<!^)\b(which|whose)\b', bare)
    if m:
        out.append(('summary', m.group(0)))
    for code, pat in (('glue', GLUE), ('rule-talk', RULE_TALK), ('meta', META), ('button', BUTTON), ('hedge', HEDGE), ('screen-talk', SCREEN_TALK),
                      ('direction-hyphen', DIRECTION), ('the-net', THE_NET)):
        m = pat.search(bare)
        if m:
            out.append((code, m.group(0)))
    for pat, right in SPELLINGS:
        m = pat.search(bare)
        if m:
            out.append(('spelling', f'{m.group(0)} -> {right}'))
    return out


def make_entry(reader, e, ids):
    stem = reader.stem
    domain, default, kind = DOMAINS.get(stem, DEFAULT_DOMAIN)
    raw = e['text']
    text, ph = placeholders(raw, e.get('after') or [], e.get('printf', False))
    container = e['container']
    place = e['place']
    if place and not place.startswith('case'):
        # (an initializer's element: its place alone, a count only past the first)
        base = f'{stem}:{container}:{place}'
        n = ids[base]
        ids[base] += 1
        lid = base if n == 0 else f'{base}.{n}'
    else:
        base = f'{stem}:{container}:' + (place + '.' if place else '')
        n = ids[base]
        ids[base] += 1
        lid = base + str(n)
    speaker = None
    for pat, who, k in OVERRIDES:
        if re.search(pat, lid):
            if who and speaker is None:
                speaker = who
            if k:
                kind = k
            break
    default = speaker or default
    who = None
    if stem == 'guardian_lines' and container in ('lines', 'guardian_tip', 'guardian_rumor'):
        m = re.match(r'(?:case)?(\d+)', place)
        who = reader.guardians.get(int(m.group(1))) if m else None
    widths = {p['name']: p['width'] for p in ph}
    continues = text.startswith('|')
    body = text.strip('|') if kind == 'speech' else text
    boxes = []
    for b in re.split(r'\||\f', body):
        if not b.strip() and boxes:
            continue
        spk, words = box_speaker(b, default, kind, e.get('face'))
        lines, pages = wrap(words, widths)
        box = {'speaker': spk, 'text': words, 'pages': pages}
        if kind == 'speech':
            fl = [f for f in flag_box(spk, words, widths, lines) if stem not in NO_BOX or f[0] not in SHAPE]
            if fl:
                box['flags'] = [{'code': c, 'at': at} for c, at in fl]
        boxes.append(box)
    fragment = bool(re.match(r'[a-z ,.]', body)) or (not re.search(r'[.!?…"*)\]~]\s*$', body) and '|' not in body and kind == 'speech')
    flags = Counter()
    for b in boxes:
        for f in b.get('flags', []):
            flags[f['code']] += 1
    run, last = 0, None
    for b in boxes:
        run = run + 1 if b['speaker'] == last else 1
        last = b['speaker']
        if run > SPEECH_BOXES and kind == 'speech' and stem not in NO_BOX and last not in PLAIN:
            flags['long-speech'] = 1
    speakers = []
    for b in boxes:
        if b['speaker'] not in speakers:
            speakers.append(b['speaker'])
    out = {
        'id': lid, 'file': reader.rel, 'line': e['tok'].line, 'container': container,
        'domain': domain, 'kind': kind, 'speaker': speakers[0] if speakers else default, 'speakers': speakers,
        'text': text, 'boxes': boxes, 'text_hash': hashlib.sha1(text.encode()).hexdigest()[:12],
        'flags': sorted(flags),
    }
    if who:
        out['who'] = who
    if ph:
        out['placeholders'] = [{'name': p['name'], 'spec': p['spec']} for p in ph]
    if continues:
        out['continues'] = True
    if fragment:
        out['fragment'] = True
    if e.get('note'):
        out['note'] = e['note']
    return out


# ---- the numbers ----

def bn6_numbers():
    """VOICE.md's table (The numbers): speaker -> its columns, numbers and
    openers as the page writes them."""
    with open(os.path.join(ROOT, VOICE)) as f:
        text = f.read()
    sec = text.split('## The numbers', 1)[1].split('\n## ', 1)[0]
    rows = {}
    for line in sec.split('\n'):
        cells = [c.strip() for c in line.strip().strip('|').split('|')]
        if len(cells) != 6 or cells[0] in ('Speaker', '---') or cells[0].startswith('**') or set(cells[0]) <= {'-', ' '}:
            continue

        def num(s):
            try:
                return float(s)
            except ValueError:
                return None
        rows[cells[0]] = {'words_per_box': num(cells[1]), 'bangs_per_box': num(cells[2]), 'dots_per_box': num(cells[3]),
                          'stutters': cells[4], 'openers': cells[5]}
    return rows


def opener(box):
    m = re.match(r"\s*(\.\.\.)?([A-Za-z'][A-Za-z']*|\{[^}]+\}|\*\w+\*|[.!?*]+)", box)
    if not m:
        return None
    w = m.group(2)
    return None if w.startswith('{') else (('...' if m.group(1) else '') + w)


def numbers(entries):
    """Our numbers per speaker: boxes, words a box, "!!" and "..." a box,
    the share ending in !, ... and ?, stutters, openers."""
    per = {}
    for e in entries:
        if e['kind'] != 'speech' or e.get('fragment'):
            continue
        for b in e['boxes']:
            t = PLACEHOLDER.sub('X', b['text'])
            if not t.strip():
                continue
            o = opener(b['text'])
            keys = [b['speaker']] + (['group:' + NUMBERS_GROUP[b['speaker']]] if b['speaker'] in NUMBERS_GROUP else [])
            for key in keys:
                s = per.setdefault(key, {'boxes': 0, 'words': 0, 'bangs': 0, 'dots': 0, 'end_bang': 0, 'end_dots': 0,
                                         'end_ask': 0, 'stutters': 0, 'openers': Counter()})
                s['boxes'] += 1
                s['words'] += len(re.findall(r"[A-Za-z0-9']+", t))
                s['bangs'] += t.count('!!')
                s['dots'] += len(re.findall(r'\.\.\.', t))
                end = t.rstrip()
                s['end_bang'] += end.endswith('!')
                s['end_dots'] += end.endswith('...')
                s['end_ask'] += end.endswith('?')
                s['stutters'] += bool(STUTTER.search(t))
                if o:
                    s['openers'][o.upper() if o.isupper() and len(o) > 1 else o] += 1
    out, groups = {}, {}
    for key, s in sorted(per.items()):
        n = s['boxes']
        who = key[len('group:'):] if key.startswith('group:') else key
        (groups if key.startswith('group:') else out)[who] = {
            'boxes': n, 'words_per_box': round(s['words'] / n, 1), 'bangs_per_box': round(s['bangs'] / n, 2),
            'dots_per_box': round(s['dots'] / n, 2), 'end_bang': round(s['end_bang'] / n, 2),
            'end_dots': round(s['end_dots'] / n, 2), 'end_ask': round(s['end_ask'] / n, 2),
            'stutters': f'1 box in {round(n / s["stutters"])}' if s['stutters'] else 'never',
            'openers': [o for o, _ in sorted(s['openers'].items(), key=lambda kv: (-kv[1], kv[0]))[:6]],
            'bn6': who if key.startswith('group:') else NUMBERS_GROUP.get(who),
        }
    return out, groups


# ---- the catalog ----

def words_files():
    files = []
    for path in sorted(glob.glob(os.path.join(ROOT, 'src', '*', '*.c'))):
        if not path.endswith(WORDS_FILES):
            continue
        with open(path, errors='replace') as f:
            head = ''.join(f.readline() for _ in range(3))
        if re.search(r'(written|drawn|made) by tools/', head):
            continue
        files.append(os.path.relpath(path, ROOT))
    return files


def guardian_names():
    path = os.path.join(ROOT, 'src', 'layer', 'guardians.c')
    if not os.path.exists(path):
        return {}
    with open(path, errors='replace') as f:
        return {int(n): name for n, name in re.findall(r'\[(\d+)\]\s*=\s*\{\s*"(\w+)"', f.read())}


# ---- the review's order: conversations, and the sweep's logs ----

# The areas in the order a run meets them, for the review's rail; an area
# not named here follows, by name.
AREAS = ['Title', 'The town', 'AsterLand and the Academy', 'Requests', "Lan's HP and the gate", 'Briefing',
         'The story', 'Layers: dealers and finds', 'Layers: services', 'Layers: rewards', 'Layers: locks',
         'Bystanders', 'Guardians', 'Super bosses', 'Chaud and ProtoMan', 'NaviCust', 'DarkChips', 'Powers',
         'Lessons', 'Where the nets meet', 'The PET', 'Net Square', 'Trade Central', 'Arena',
         'The second screen', 'The ROMs screen', 'SAVES', 'Statistics', 'Other']
# A table longer than this whose rows are a guardian's each (`who`) is a
# conversation a guardian.
SPLIT = 30
SWEEP = 'docs/voice/sweep'


def readable(name):
    """A function's or table's name as words: ta_heal.hello -> Heal: hello."""
    parts = [re.sub(r'^ta_', '', p).replace('_', ' ').strip() for p in name.split('.')]
    text = ': '.join(p for p in parts if p)
    return text[:1].upper() + text[1:]


def conversation_title(c, name):
    """A conversation's title: the first clause of its comment (in the .c,
    else its header) where it reads as one, after its name as words where
    the clause leans on the code around it ("Its people")."""
    for text in (c.get('comment'), c.get('header')):
        if not text or text.startswith(('...', '(')):
            continue
        first = re.split(r':\s|;\s|\.\s| \(| — ', text, maxsplit=1)[0].strip(' .,-')
        if not 6 <= len(first) <= 90:
            continue
        if re.match(r'(What )?(its|their|his|her|the same|same|as above)\b', first, re.I):
            return f'{readable(name)}: {first[:1].lower()}{first[1:]}'
        return first[:1].upper() + first[1:]
    return readable(name)


def conversations(entries, containers):
    """The lines as the review reads them: a function's or a table's lines
    together, in the code's order (a long table of guardians' rows, a
    guardian's together), the areas in AREAS' order, a file's in DOMAINS'
    order and then by its lines."""
    sizes = Counter(f"{e['id'].split(':')[0]}:{e['container']}" for e in entries)
    convs = {}
    for e in entries:
        key = f"{e['id'].split(':')[0]}:{e['container']}"
        split = sizes[key] > SPLIT and e.get('who')
        cid = f"{key}@{e['who']}" if split else key
        c = convs.get(cid)
        if c is None:
            meta = containers.get(key, {})
            title = conversation_title(meta, e['container'])
            c = convs[cid] = {'id': cid, 'container': key, 'title': f"{e['who']}: {title}" if split else title,
                              'when': meta.get('comment') or meta.get('header') or '', 'domain': e['domain'],
                              'file': e['file'], 'line': e['line'], 'speakers': [], 'lines': []}
        c['lines'].append(e['id'])
        c['line'] = min(c['line'], e['line'])
        for s in e['speakers']:
            if s not in c['speakers']:
                c['speakers'].append(s)
    stems = list(DOMAINS)

    def order(c):
        d, stem = c['domain'], c['container'].split(':')[0]
        return (AREAS.index(d) if d in AREAS else len(AREAS), d,
                stems.index(stem) if stem in stems else len(stems), stem, c['line'], c['id'])
    return sorted(convs.values(), key=order)


def table_cells(row):
    """A Markdown table's row as its cells (`\\|` is a | of the cell's)."""
    s = row.strip()
    s = s[1:] if s.startswith('|') else s
    s = s[:-1] if s.endswith('|') and not s.endswith('\\|') else s
    cells, cur, i = [], '', 0
    while i < len(s):
        if s.startswith('\\|', i):
            cur, i = cur + '|', i + 2
        elif s[i] == '|':
            cells.append(cur.strip())
            cur, i = '', i + 1
        else:
            cur, i = cur + s[i], i + 1
    cells.append(cur.strip())
    return cells


def sweep_ids(cell):
    """The ids a log's cell names: `a:b:1`, then `:2` (the same function's),
    `c:0` (the same file's) or `case29.0` (a range's end, `a:b:case1.0`-`case29.0`)."""
    ids = []
    for tok in re.findall(r'`([^`]+)`', cell):
        if ids and tok.startswith(':'):
            tok = ids[-1].rsplit(':', 1)[0] + tok
        elif ids and tok.count(':') == 1:
            tok = ids[-1].split(':', 1)[0] + ':' + tok
        elif ids and ':' not in tok:
            tok = ids[-1].rsplit(':', 1)[0] + ':' + tok
        if tok.count(':') == 2 and tok not in ids:
            ids.append(tok)
    return ids, '`-`' in cell


def plain_words(text):
    text = re.sub(r'\{[^{}]*\}|@[A-Z]\b|\*\*|`', ' ', text.replace('|', ' '))
    return re.findall(r"[a-z0-9']+", text.lower())


def sweep_live(row, by_container):
    """The lines of today's catalog a log's row is about. The logs' ids are
    the catalog's before the sweep, and a cut moves the count of the lines
    after it, so each id's own function is searched for the words logged:
    the line with the same words first, else the closest; a cut line
    matches only what is left of it (a cut in part)."""
    ids, cut = row['ids'], row['status'] == 'cut'
    if row.get('range'):
        return [e['id'] for e in by_container.get(ids[0].rsplit(':', 1)[0], [])]
    segments = row['text'].split(' / ')
    if len(ids) == 1 or len(segments) != len(ids):
        segments = [row['text']] * len(ids)
    live = []
    for old, seg in zip(ids, segments):
        want = Counter(plain_words(seg))
        best, score = None, 0.0
        for e in by_container.get(old.rsplit(':', 1)[0], []):
            have = Counter(plain_words(e['text']))
            if e['id'] in live or not have or not want or (cut and sum(have.values()) < 3):
                continue
            common = sum((want & have).values())
            if want == have:
                s = 2.0
            elif cut:
                s = common / sum(have.values())
            else:
                s = common / min(sum(want.values()), sum(have.values()))
            s += 0.001 * (e['id'] == old)
            if s > score:
                best, score = e, s
        if best and score >= (0.8 if cut else 0.6):
            live.append(best['id'])
    return live


def sweep(entries):
    """The voice sweep's logs (docs/voice/sweep/*.md), their Cut and Noted
    tables read row by row: the group (the log's name), the ids as the
    catalog had them, the line as it was, why (and where, a Noted table's
    fourth column), and the lines it is about today (`live`), for the
    review's Sweep view."""
    by_container = {}
    for e in entries:
        by_container.setdefault(e['id'].rsplit(':', 1)[0], []).append(e)
    rows = []
    for path in sorted(glob.glob(os.path.join(ROOT, SWEEP, '*.md'))):
        group = os.path.splitext(os.path.basename(path))[0]
        status, header = None, None
        with open(path, encoding='utf-8') as f:
            for raw in f:
                line = raw.rstrip('\n')
                if line.startswith('## '):
                    head = line[3:].strip().lower()
                    status = 'cut' if head.startswith('cut') else 'noted' if head.startswith('note') else None
                    header = None
                    continue
                if not line.startswith('|'):
                    header = None
                    continue
                cells = table_cells(line)
                if header is None:
                    header = cells if cells[0].lower() == 'id' else []
                    continue
                if not status or not header or set(''.join(cells)) <= set('-: '):
                    continue
                ids, rng = sweep_ids(cells[0])
                if not ids or len(cells) < 3:
                    continue
                row = {'group': group, 'status': status, 'id': ids[0], 'ids': ids,
                       'text': cells[1].replace('`', ''), 'why': cells[2]}
                if len(cells) > 3 and cells[3]:
                    row['where'] = cells[3]
                if rng:
                    row['range'] = True
                rest = re.sub(r'`[^`]*`', '', cells[0]).strip(' ,-')
                if re.sub(r'\W|and', '', rest):
                    row['scope'] = rest
                row['live'] = sweep_live(row, by_container)
                rows.append(row)
    return rows


def catalog():
    guardians = guardian_names()
    entries, files, containers = [], {}, {}
    for rel in words_files():
        r = Reader(rel, guardians).read()
        ids = Counter()
        mine = []
        for e in r.entries:
            if not is_line(e['text']):
                continue
            mine.append(make_entry(r, e, ids))
        domain, speaker, kind = DOMAINS.get(r.stem, DEFAULT_DOMAIN)
        shows = [p for p in SHOWS.get(r.stem, []) if os.path.exists(os.path.join(ROOT, 'docs', p))]
        files[rel] = {'domain': domain, 'speaker': speaker, 'kind': kind, 'comment': r.file_comment(),
                      'shows': shows, 'lines': len(mine), 'stem': r.stem}
        for name, c in sorted(r.containers.items()):
            if any(e['container'] == name for e in mine):
                containers[f'{r.stem}:{name}'] = {k: v for k, v in c.items() if v}
        entries += mine
    flags = Counter(f for e in entries for f in e['flags'])
    ours, groups = numbers(entries)
    domains = {}
    for e in entries:
        d = domains.setdefault(e['domain'], {'lines': 0, 'speech': 0, 'flagged': 0, 'files': []})
        d['lines'] += 1
        d['speech'] += e['kind'] == 'speech'
        d['flagged'] += bool(e['flags'])
        if e['file'] not in d['files']:
            d['files'].append(e['file'])
    return {
        'version': VERSION,
        'about': 'Every line in the words files (docs/voice/README.md), written by tools/voice_catalog.py: do not edit.',
        'flags': FLAGS,
        'files': files,
        'containers': containers,
        'conversations': conversations(entries, containers),
        'sweep': sweep(entries),
        'summary': {
            'lines': len(entries), 'speech': sum(e['kind'] == 'speech' for e in entries),
            'boxes': sum(len(e['boxes']) for e in entries if e['kind'] == 'speech'),
            'flagged': sum(bool(e['flags']) for e in entries), 'flags': dict(sorted(flags.items())),
            'domains': dict(sorted(domains.items())),
            'numbers': ours, 'groups': groups, 'bn6': bn6_numbers(),
        },
        'lines': entries,
    }


def render(cat):
    """The catalog as its file: its parts indented, and each line on a line
    of its own (its keys sorted), so that a reworded line is one line of
    the diff."""
    cat = dict(cat)
    lists = [(k, cat.pop(k)) for k in ('conversations', 'sweep', 'lines') if k in cat]
    head = json.dumps(cat, indent=1, sort_keys=True, ensure_ascii=False)[:-2]
    for k, items in lists:
        body = ',\n'.join('  ' + json.dumps(e, sort_keys=True, ensure_ascii=False, separators=(',', ':')) for e in items)
        head += f',\n "{k}": [\n' + body + '\n ]'
    return head + '\n}\n'


def main():
    check = '--check' in sys.argv[1:]
    text = render(catalog())
    path = os.path.join(ROOT, OUT)
    if check:
        try:
            with open(path, encoding='utf-8') as f:
                old = f.read()
        except OSError:
            old = None
        if old != text:
            print(f'voice: {OUT} is stale (a words file changed): python3 tools/voice_catalog.py')
            return 1
        cat = json.loads(text)
        print(f'voice: {cat["summary"]["lines"]} lines in {len(cat["files"])} words files, {cat["summary"]["flagged"]} flagged, in step')
        return 0
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(text)
    cat = json.loads(text)
    print(f'voice: {OUT}: {cat["summary"]["lines"]} lines in {len(cat["files"])} words files, {cat["summary"]["flagged"]} flagged')
    return 0


if __name__ == '__main__':
    sys.exit(main())
