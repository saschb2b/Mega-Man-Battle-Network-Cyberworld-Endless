---
type: Process
title: "Merging in parallel"
description: "How pull requests made side by side stay out of each other's way: what conflicted when `dev` was brought into a branch, measured; the files a tool writes, which `dev`'s workflow commits and no branch does (`tools/generated.py`); what a branch does when it meets one."
tags: [project, process, git, ci]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-11T16:40:00Z }
sources:
  - { id: merges, title: "origin/dev's last 600 merge commits (6 October to 11 November 2026), each merge of dev into a branch redone with git merge-tree", resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/commits/dev" }
  - { id: pr387, title: "Pull request #387 at 62c907ac against dev at dbecd569: GitHub's CONFLICTING, docs/log.md the only file", resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/pull/387" }
---

# Merging in parallel

About ten sessions work at once, each on a pull request to `dev`, and each
merge to `dev` used to make the others bring `dev` in again. This page says
where they collided, and what keeps them apart now. The rule a session
follows is in `AGENTS.md` (Pull requests, Merging); the files themselves
are described where they belong ([the research status](../re/README.md),
[the symbols](../rom/symbols.md), [the voice review](../voice/README.md)).

## What conflicted

Measured on 11 November 2026 [^merges]: of `dev`'s last 600 merge commits,
261 bring `dev` into a branch (their second parent on `dev`'s first-parent
line). Each was redone with `git merge-tree`, without `.gitattributes`, as
GitHub merges. 39 had no conflict at all; in the other 222, these files
conflicted (the middle column: the same over the last 40 pull requests
merged, #347 to #393, which brought `dev` in 103 times, 36 of the 40 at
least once and one of them nine times):

| File | Of 261 | Of the last 40 PRs' 103 | What it is |
| --- | ---: | ---: | --- |
| `docs/log.md` | 158 | 70 | The knowledge base's log, a line per change at its top |
| `docs/rom/symbols.md` | 88 | 38 | A page with the symbols' counts, written by `tools/symbols.py` |
| `docs/symbols/*` (9 files) | 57 | 15 | The symbol files, written by `tools/symbols.py` |
| `docs/re/ledger.json` | 56 | 20 | The research ledger, one file for every feature |
| `docs/re/STATUS.md`, `state.json`, `dashboard.html` | 49 | 10 | Written from the ledger by `tools/re_status.py` |
| `docs/rom/bn4.md` | 35 | 10 | BN4's ROM table, a row per find at its end |
| `docs/devtools/differential.md` | 30 | 17 | The differential's page, a paragraph per family |
| `CHANGELOG.md` | 25 | 0 | A line per change under the coming version |
| `docs/voice/catalog.json` | 21 | 0 | Written from the words files by `tools/voice_catalog.py` |
| `tests/xdiff/mutations.json` | 20 | 12 | The differential's mutations, one file for every spec |
| `docs/multirom/bn4-chips.md` | 19 | 17 | BN4's chips, a row and a paragraph per chip |
| `docs/code/emu.md` | 18 | 3 | What each file of `src/emu/` holds |
| `src/net/layer_make.h` | 13 | 4 | `LAYER_MAKE` and its hash, which two branches bump |
| `tests/test_core.c` | 11 | 5 | The unit tests, one file |
| `tests/xdiff/legit.json` | 4 | 4 | The differential's known differences |

By kind: 47 of the 222 conflicted in the log alone, 60 in the log and the
written files alone, 72 with the two registries (`ledger.json`, the
differential's two files) added, 81 with the voice catalog. The other 141
also conflicted in a file a person writes, 82 of them only in pages of
`docs/` where every branch adds its row at the same place.

GitHub does not read `.gitattributes`' `merge=union`: pull request #387
[^pr387] differed from `dev` in `docs/log.md` alone, merged cleanly here
with the union driver, and GitHub called it conflicting. So the line
`docs/log.md merge=union` helps a merge made on a machine and nothing else.

## Files a tool writes

A file a tool writes from other files of the tree changes with every
branch that changes its sources, in the same lines (a count, a date, a
sorted table), so any two such branches conflict there, though neither
did anything to the other. These are therefore **written on `dev` alone**:

| File | Written by | From |
| --- | --- | --- |
| `docs/re/state.json`, `docs/re/STATUS.md`, `docs/re/dashboard.html` | `tools/re_status.py` | `docs/re/ledger.json` |
| `docs/symbols/*` | `tools/symbols.py` | `src/emu/bn6.h`, `bn5.h`, `bn4.h`, `src/core/rom.c`, `docs/rom/` |
| [Symbols](../rom/symbols.md)' numbers under What is mapped | `tools/symbols.py` | the same |
| `docs/voice/catalog.json` | `tools/voice_catalog.py` | the words files, `docs/voice/sweep/` |

All three tools need no ROM and nothing outside git, so a workflow can run
them: `.github/workflows/generated.yml` runs `python3 tools/generated.py
write` after each push to `dev` and commits what changed, as
`github-actions[bot]`. Its push starts no other workflow. Where it fails,
`dev`'s commit shows it red, and the files stay as they were until the
next push.

What `build.py lint` held before is still held:

| Check | Where now |
| --- | --- |
| Every name in `bn6.h`, `bn5.h` and `bn4.h` described; the ledger's shape; the words files readable | `build.py lint` and `build.py test` on every branch (`--lint` of each tool) |
| The written files in step with their sources | The workflow, right after writing (`tools/generated.py check --strict`); `build.py lint` on `main`, on a tag and on a pull request to anything but `dev` (a release's) |
| No branch changes a written file | `build.py lint` on every branch, new: against the commit the branch left `dev` at (in CI, against `dev`'s tip under the pull request's merge) |

The counts of bn6f's functions on the Symbols page (its second marked
part) are not among them: only `tools/symbols.py --full` writes them, from
tables outside git.

### What a branch does

- **Changing a source** (a header's name, a `docs/rom/` row, a ledger
  entry, a line): commit the source alone. To look at what a tool makes
  of it, run the tool, then `python3 tools/generated.py restore`.
- **`build.py lint` says a generated file differs from dev's**:
  `python3 tools/generated.py restore`, and commit.
- **A branch cut before 11 November 2026, which still carries its own
  copies**: bring `dev` in once (`git merge origin/dev`), then
  `python3 tools/generated.py restore`, which takes `dev`'s copy of every
  written file, merges the Symbols page with its numbers set to `dev`'s,
  and stages them; resolve what is left by hand and commit. After that the
  branch conflicts there no more.
