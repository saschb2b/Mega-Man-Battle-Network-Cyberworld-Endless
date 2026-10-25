#!/usr/bin/env python3
"""The research status, written from one hand-edited ledger.

docs/re/ledger.json is the source of truth. This writes from it:
  docs/re/state.json      what the dashboard reads
  docs/re/STATUS.md       the same, readable on GitHub
  docs/re/dashboard.html  the skill's page (.claude/skills/reverse-engineering/
                          assets/dashboard.html) with the state embedded, so
                          it still shows when opened as a file

  python3 tools/re_status.py           write them
  python3 tools/re_status.py --check   fail if any is out of step with the ledger
Standard library only.
"""
import argparse
import datetime
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / 'docs' / 're'
TEMPLATE = ROOT / '.claude' / 'skills' / 'reverse-engineering' / 'assets' / 'dashboard.html'

# A feature's state, in the words the page and STATUS.md show.
STATES = {
    'planned': 'Planned',
    'designed': 'Designed, not built',
    'proven': 'Proven in research, not built',
    'building': 'Being built',
    'reworking': 'Works, being reworked',
    'works': 'Works',
    'shipped': 'Shipped',
    'blocked': 'Blocked',
}
MONTHS = ('Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec')
STATE_BLOCK = re.compile(r'(<script type="application/json" id="re-state">\n)(.*?)(\n</script>)', re.S)


class LedgerError(ValueError):
    pass


def human(day):
    """'2026-10-18' as '18 Oct 2026'; an unknown date as a dash."""
    if not day:
        return '—'
    d = datetime.date.fromisoformat(day)
    return f'{d.day} {MONTHS[d.month - 1]} {d.year}'


def need(record, key, where, kind=str, empty=False):
    value = record.get(key)
    if not isinstance(value, kind) or (not empty and not value):
        raise LedgerError(f'{where}: "{key}" is missing or not a {kind.__name__}')
    return value


def date_or_none(record, key, where):
    value = record.get(key)
    if value is None:
        return None
    try:
        datetime.date.fromisoformat(value)
    except (TypeError, ValueError):
        raise LedgerError(f'{where}: "{key}" is not a date (YYYY-MM-DD) or null') from None
    return value


def check(ledger):
    """The ledger's shape; the page and STATUS.md read nothing else."""
    if ledger.get('schema') != 're-ledger/1':
        raise LedgerError('schema is not re-ledger/1')
    need(ledger, 'project', 'ledger')
    if not date_or_none(ledger, 'updated', 'ledger'):
        raise LedgerError('ledger: "updated" is needed')
    date_or_none(ledger, 'previous', 'ledger')
    for i, f in enumerate(need(ledger, 'features', 'ledger', list)):
        where = f'features[{i}]'
        need(f, 'name', where)
        if f.get('state') not in STATES:
            raise LedgerError(f'{where}: state is one of {", ".join(STATES)}')
        need(f, 'now', where)
        need(f, 'left', where, empty=True)
        date_or_none(f, 'updated', where)
    for i, d in enumerate(need(ledger, 'decisions', 'ledger', list, empty=True)):
        need(d, 'question', f'decisions[{i}]')
        need(d, 'options', f'decisions[{i}]', list)
        date_or_none(d, 'by', f'decisions[{i}]')
    for i, c in enumerate(need(ledger, 'changes', 'ledger', list, empty=True)):
        need(c, 'what', f'changes[{i}]')
        if not date_or_none(c, 'date', f'changes[{i}]'):
            raise LedgerError(f'changes[{i}]: "date" is needed')
    for i, e in enumerate(need(ledger, 'experiments', 'ledger', list, empty=True)):
        for key in ('feature', 'name', 'result', 'meaning'):
            need(e, key, f'experiments[{i}]')
        date_or_none(e, 'date', f'experiments[{i}]')
    need(ledger, 'method', 'ledger', dict)
    for i, g in enumerate(need(ledger, 'glossary', 'ledger', list, empty=True)):
        need(g, 'term', f'glossary[{i}]')
        need(g, 'meaning', f'glossary[{i}]')


def state_of(ledger):
    """What the page reads: the ledger, each feature's words, and only the
    changes since the previous update. No totals across experiments."""
    previous = ledger.get('previous')
    since = [c for c in ledger['changes'] if not previous or c['date'] > previous]
    since.sort(key=lambda c: c['date'], reverse=True)
    features = [dict(f, state_text=STATES[f['state']]) for f in ledger['features']]
    experiments = sorted(ledger['experiments'], key=lambda e: e.get('date') or '', reverse=True)
    return {
        'schema': 're-status/1',
        'written_by': 'tools/re_status.py from docs/re/ledger.json',
        'project': ledger['project'],
        'updated': ledger['updated'],
        'previous': previous,
        'features': features,
        'decisions': ledger['decisions'],
        'since': since,
        'experiments': experiments,
        'method': ledger['method'],
        'glossary': ledger['glossary'],
    }


def cell(text):
    return (text or '—').replace('|', '\\|').replace('\n', ' ')


def link(text, href):
    return f'[{text}]({href})' if href else text


def markdown(state):
    # frontmatter, as every concept of the knowledge base opens (docs/index.md); dated by the ledger
    title = f'Research status: {state["project"]}'
    out = ['---', 'type: Status', f'title: {json.dumps(title, ensure_ascii=False)}',
           'description: "Where each reverse-engineering feature stands, written by tools/re_status.py from '
           'docs/re/ledger.json: its state, what is done and left, the changes since, experiments and decisions '
           'needed."',
           'tags: [re]', f'generated: {{ by: process:re_status, at: {state["updated"]}T00:00:00Z }}', '---', '',
           f'# {title}', '',
           f'Updated {human(state["updated"])}. Written by `tools/re_status.py` from '
           '[ledger.json](ledger.json); the [dashboard](dashboard.html) shows the same.', '',
           '| Feature | Status | Now | What is left | Updated |', '| --- | --- | --- | --- | --- |']
    for f in state['features']:
        out.append(f'| {link(f["name"], f.get("link"))} | {f["state_text"]} | {cell(f["now"])} | '
                   f'{cell(f["left"])} | {human(f.get("updated"))} |')
    if state['decisions']:
        out += ['', '## Decisions needed', '']
        for d in state['decisions']:
            by = f' (by {human(d["by"])})' if d.get('by') else ''
            out.append(f'- {d["question"]}{by}: {"; ".join(d["options"])}')
    out += ['', f'## Since the last update ({human(state["previous"])})' if state['previous'] else '## Changes', '']
    out += [f'- {human(c["date"])}: {c["what"]}' for c in state['since']] or ['- Nothing.']
    out += ['', '## Experiments', '', 'One line each, newest first; each stands on its own.', '',
            '| Date | Feature | Experiment | Result | What it means |', '| --- | --- | --- | --- | --- |']
    for e in state['experiments']:
        out.append(f'| {human(e.get("date"))} | {e["feature"]} | {link(cell(e["name"]), e.get("link"))} | '
                   f'{cell(e["result"])} | {cell(e["meaning"])} |')
    m = state['method']
    out += ['', '<details><summary>Method</summary>', '']
    if m.get('target'):
        out.append(f'- Target: {m["target"]}')
    if m.get('oracle'):
        out.append(f'- Compared against: {m["oracle"]}')
    out += [f'- {rule}' for rule in m.get('rules', [])]
    if m.get('records'):
        out += ['', '| Record | Where |', '| --- | --- |']
        out += [f'| {cell(r["what"])} | {cell(r["where"])} |' for r in m['records']]
    out += ['', '</details>', '', '## Glossary', '']
    out += [f'- **{g["term"]}**: {g["meaning"]}' for g in state['glossary']]
    return '\n'.join(out) + '\n'


def embedded(state):
    # "<" escaped, so the JSON can never close its script block early.
    return json.dumps(state, indent=2, ensure_ascii=False).replace('<', '\\u003c')


def page(state, template):
    if not STATE_BLOCK.search(template):
        raise LedgerError(f'{TEMPLATE.relative_to(ROOT)} has no re-state block')
    return STATE_BLOCK.sub(lambda m: m.group(1) + embedded(state) + m.group(3), template, count=1)


def outputs(ledger_path=DOCS / 'ledger.json', template_path=TEMPLATE):
    ledger = json.loads(ledger_path.read_text(encoding='utf-8'))
    check(ledger)
    state = state_of(ledger)
    return {
        DOCS / 'state.json': json.dumps(state, indent=2, ensure_ascii=False) + '\n',
        DOCS / 'STATUS.md': markdown(state),
        DOCS / 'dashboard.html': page(state, template_path.read_text(encoding='utf-8')),
    }


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--check', action='store_true', help='fail if a written file is out of step with the ledger')
    args = ap.parse_args()
    try:
        files = outputs()
    except (LedgerError, OSError, json.JSONDecodeError) as error:
        sys.exit(f're_status: {error}')
    stale = []
    for path, text in files.items():
        current = path.read_text(encoding='utf-8') if path.exists() else None
        if current == text:
            continue
        if args.check:
            stale.append(str(path.relative_to(ROOT)))
        else:
            path.write_text(text, encoding='utf-8')
            print(f'wrote {path.relative_to(ROOT)}')
    if stale:
        sys.exit('re_status: out of step with docs/re/ledger.json (run python3 tools/re_status.py): ' + ', '.join(stale))


if __name__ == '__main__':
    main()
