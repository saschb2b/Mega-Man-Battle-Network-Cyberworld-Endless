#!/usr/bin/env python3
"""tools/voice_catalog.py's reading of C, on a words file of its own: adjacent
literals joined, escapes decoded, a file's string macro spelled out, the
branches of a ternary, tables with designators and rows, a switch's cases,
printf's conversions as named placeholders, a box's speaker from its mark
and a call's face, ids that hold no text, the chat box's wrap and the
flags. Standard library only; build.py test runs it."""
import os
import shutil
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import voice_catalog as vc  # noqa: E402

SOURCE = r'''/* What the test says. */
#include "demo_words.h"

#include <stdio.h>

#define TAIL ".|@M It's a winding walk"

/* MegaMan's word on a place: how far it is */
const char *far_words(bool winds) {
	return winds ? "Far off" TAIL : "Close by,Lan!";
}

/* the rows: a number and a line */
static const struct { int n; const char *said; } rows[] = {
	{ 1, "Row one!" },
	{ 2, "Row " "two!" },
};

static const char *const marks[] = {
	[2] = "Two boxes!|@L Yeah! Let's go!",
	[5] = "HELLO,HELLO!\nI'M A MR.PROG!",
};

const char *beaten(char *buf, size_t n, int navi, int count, int len) {
	snprintf(buf, n, "%s beat us %d times! 100%% sure.|@M %.*s again...", guardian(navi)->name, count, len, name_of(navi));
	return buf;
}

const char *by_case(int k) {
	switch (k) {
	case 1: return "One!";
	case 2: return "Two!";
	default: return NULL;
	}
}

void dad(TextArchive *t) {
	ta_say(t, FACE_DAD, "Be careful,OK?");
	ta_say(t, FACE_NONE, "MegaMan got:\n\"Cannon A\"!!");
}

const char *escapes(void) { return "Tab\there,\x21 \"quoted\" \\ end."; }

const char *flagged(void) {
	return "Lan, this layer has a shop. However,it costs a lot of Zenny to use the shop here!";
}

const char *noise(void) { return strcmp(a, "not words at all") ? "settings.ini" : "%s/%s"; }
'''

HEADER = r'''#ifndef DEMO_WORDS_H
#define DEMO_WORDS_H
/* The test's header word on far_words */
const char *far_words(bool winds);
#endif
'''

failed = 0


def check(ok, what):
    global failed
    if not ok:
        failed += 1
        print(f'  FAILED: {what}')


def read(src, header=HEADER):
    d = tempfile.mkdtemp()
    try:
        path = os.path.join(d, 'demo_words.c')
        with open(path, 'w') as f:
            f.write(src)
        with open(os.path.join(d, 'demo_words.h'), 'w') as f:
            f.write(header)
        r = vc.Reader(path, {}).read()
        ids = vc.Counter()
        entries = [vc.make_entry(r, e, ids) for e in r.entries if vc.is_line(e['text'])]
        return r, {e['id'].split(':', 1)[1]: e for e in entries}
    finally:
        shutil.rmtree(d)


def main():
    r, by = read(SOURCE)
    # adjacent literals and a #define'd one joined; a ternary's two branches
    check(by['far_words:0']['text'] == "Far off.|@M It's a winding walk", f"far_words:0 {by.get('far_words:0', {}).get('text')!r}")
    check(by['far_words:1']['text'] == 'Close by,Lan!', 'far_words:1, the ternary\'s other branch')
    check([b['speaker'] for b in by['far_words:0']['boxes']] == ['MegaMan', 'MegaMan'], 'a mark and the file\'s default speaker')
    # the function's comment, and its header's
    c = r.containers['far_words']
    check(c['comment'] == "MegaMan's word on a place: how far it is", f'the function\'s comment: {c["comment"]!r}')
    check(c['header'] == "The test's header word on far_words", f'the header\'s comment: {c["header"]!r}')
    # rows: the row's index and the member's name
    check(by['rows:0.said']['text'] == 'Row one!' and by['rows:1.said']['text'] == 'Row two!', f'rows: {sorted(k for k in by if k.startswith("rows"))}')
    # designators, and a box's mark
    check(by['marks:2']['boxes'][1]['speaker'] == 'Lan' and by['marks:2']['boxes'][1]['text'] == "Yeah! Let's go!", 'marks:2, @L')
    check(by['marks:5']['speaker'] == 'Mr.Prog', 'capitals read as Mr.Prog')
    # conversions: names from the arguments, %% kept, %.*s taking two
    e = by['beaten:0']
    check(e['text'] == '{name} beat us {count} times! 100% sure.|@M {name_of} again...', f'beaten: {e["text"]!r}')
    check([p['spec'] for p in e['placeholders']] == ['%s', '%d', '%.*s'], 'the specs')
    # a switch's cases
    check(by['by_case:case1.0']['text'] == 'One!' and by['by_case:case2.0']['text'] == 'Two!', f'cases: {sorted(by)}')
    # a call's face, and the game's own voice
    check(by['dad:0']['speaker'] == 'Dad', 'FACE_DAD')
    check(by['dad:1']['speaker'] == 'Narration' and 'colon' not in by['dad:1']['flags'], 'FACE_NONE is narration, not flagged for its colon')
    # escapes
    check(by['escapes:0']['text'] == 'Tab\there,! "quoted" \\ end.', f'escapes: {by["escapes:0"]["text"]!r}')
    # flags
    f = by['flagged:0']['flags']
    for code in ('comma-space', 'rule-talk', 'glue', 'pages', 'box-words'):
        check(code in f, f'flag {code}: {f}')
    check(not by['marks:5']['flags'], f'Mr.Prog in capitals is fine: {by["marks:5"]["flags"]}')
    # not words: a strcmp's argument, a path, a format's glue
    check(not any('settings' in e['text'] or 'not words' in e['text'] for e in by.values()), 'noise left out')
    # ids hold no text: a reworded line keeps its id
    _, by2 = read(SOURCE.replace('Row one!', 'The first row, reworded!'))
    check(by2['rows:0.said']['text'] == 'The first row, reworded!' and by2['rows:0.said']['text_hash'] != by['rows:0.said']['text_hash'],
          'a reworded line keeps its id, not its hash')
    # the chat box: twenty characters a line, three lines a page, pages
    # spread as the game spreads them (four lines are two pages of two)
    lines, pages = vc.wrap("A strong Navi's signal waits at the end of the layer,Lan. Careful now!", {})
    check(all(len(ln) <= 20 for ln in lines), f'wrap: {lines}')
    check([len(p) for p in pages] == [2, 2] if len(lines) == 4 else len(pages) == (len(lines) + 2) // 3, f'pages: {pages}')
    lines, _ = vc.wrap('{name} waits!', {'name': 8})
    check(lines == ['{name} waits!'], f'a placeholder at its width: {lines}')
    # the catalog renders the same twice (deterministic)
    cat = vc.catalog()
    check(vc.render(cat) == vc.render(vc.catalog()), 'the catalog is deterministic')
    check(len({e['id'] for e in cat['lines']}) == len(cat['lines']), 'ids are unique')
    print(f'voice catalog: {failed} failed')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
