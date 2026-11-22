---
type: Template
title: Status page
description: Give humans one page that says, in plain words, where each feature stands, what waits on a decision, what changed since the last update and what each experiment meant, written by a script from one hand-edited ledger at the end of every iteration.
tags: [reverse-engineering, reporting, dashboard]
generated: { by: claude-code/unversioned, at: 2026-10-18T12:00:00Z }
sources:
  - id: skill-asset
    resource: "scope:this skill's assets/dashboard.html and its re-status/1 state, authored with the page"
  - id: cwe-status
    resource: https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/blob/main/tools/re_status.py
---

# Who reads it

People who were not in the session: the owner deciding what comes next, a collaborator picking a feature up. They want to know what works, what is left and what waits on them, not the loop's internals. Write for them: plain words, whole sentences, dates as "18 Oct 2026", an unknown as a dash, and no number that adds up separate experiments.

# Set it up in phase 0

Keep one hand-edited ledger of the status next to the project's other records (for example `docs/re/ledger.json`), and a small script that checks it and writes three files from it:

- `state.json`, which the page reads (schema below);
- a Markdown copy (`STATUS.md`) that a code host shows without running anything;
- the page itself: `assets/dashboard.html` from this skill, with the state written into its `<script type="application/json" id="re-state">` block, so the page still shows when opened as a file, where a browser refuses to read `state.json`.

The page is never edited per project. Run the script's check in the project's tests, so a ledger edit without a rewrite fails. Cyberworld Endless's `tools/re_status.py` is a working example in the standard library alone.[^cwe-status]

# Rewrite it every iteration

At the end of phase 6 and before every report, edit the ledger and run the script:

- Take every figure from a tool read: the last oracle run, the ledger of claims. Never estimate, round, or carry a number over from memory. Leave out what you do not know; the page shows a dash.
- Set `updated`, and keep `previous` at the last update's date, so "since the last update" lists only what is new.
- One experiment, one line: its result and what it means for the goal. Never total specs, passes or faults across experiments; each has its own scope.
- Put a decision in `decisions` only while it waits on a person, with its options. When it is made, it becomes a change.
- Put only your own names and numbers in it. No text, code, or assets from the target, by the same legal line as the repository.

The page is a view, never a second source of truth. When it disagrees with the ledger or the oracle, they win and you rewrite the state.

# Schema `re-status/1`

The ledger holds the same fields but `schema` (`re-ledger/1`), `written_by`, `since` (the script picks the changes after `previous` from its `changes`), and each feature's `state_text`. Dates are `YYYY-MM-DD`, or `null` where unknown.

| Field | Holds |
| --- | --- |
| `schema`, `written_by` | `"re-status/1"`, and what wrote the state |
| `project`, `updated`, `previous` | The project's name; the dates of this update and the one before |
| `features` | One per feature the work serves, in the user's words: `{name, state, state_text, now, left, updated, link}`. `state` is `planned`, `designed`, `proven` (in research, not built), `building`, `reworking`, `works`, `shipped` or `blocked`; `state_text` says it in words; `now` is what is true today, `left` what remains |
| `decisions` | `{question, options, by}`: only what waits on a person; the section is left out when there are none |
| `since` | `{date, what}`: the changes since `previous`, newest first |
| `experiments` | `{feature, date, name, result, meaning, link}`: one line each, newest first |
| `method` | `target` (the build and its hash), `oracle`, `rules` (list), `records` (`{what, where}`: the ledgers of addresses and claims); shown folded |
| `glossary` | `{term, meaning}` for every word a newcomer would stop at |
| `example` | `true` only in this skill's copy, which shows a banner |

# What the page does

It reads `state.json` beside it, and falls back to its embedded copy, saying so, when that cannot be read. It shows the features first, then decisions (only when there are any), then the changes since the last update, the experiments one per line, the method folded, and the glossary. Every string is shown as text, never as markup; links go only to relative paths or https. It adds nothing up and checks nothing: the script refuses a ledger with a missing field, an unknown state or a malformed date, so the page never shows a half-written status.

[^cwe-status]: Cyberworld Endless, `tools/re_status.py`: writes `docs/re/state.json`, `STATUS.md` and `dashboard.html` from `docs/re/ledger.json`; `--check` runs in `build.py test`.
