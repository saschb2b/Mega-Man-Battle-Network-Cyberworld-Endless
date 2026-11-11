---
type: Process
title: "Merging in parallel"
description: "How pull requests made side by side stay out of each other's way: what conflicted when `dev` was brought into a branch, measured; the files a tool writes, which `dev`'s workflow commits and no branch does (`tools/generated.py`); the registries kept a file an entry (`tools/shards.py`); the log folded from a file a change (`tools/log_fold.py`); what a branch does when it meets any of them; when a branch brings `dev` in, what that saves and what still conflicts."
tags: [project, process, git, ci]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-11T19:30:00Z }
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
`docs/log.md merge=union` helped a merge made on a machine and nothing else
([The log](#the-log)).

## Files a tool writes

A file a tool writes from other files of the tree changes with every
branch that changes its sources, in the same lines (a count, a date, a
sorted table), so any two such branches conflict there, though neither
did anything to the other. These are therefore **written on `dev` alone**:

| File | Written by | From |
| --- | --- | --- |
| `docs/re/state.json`, `docs/re/STATUS.md`, `docs/re/dashboard.html` | `tools/re_status.py` | `docs/re/ledger/` |
| `docs/symbols/*` | `tools/symbols.py` | `src/emu/bn6.h`, `bn5.h`, `bn4.h`, `src/core/rom.c`, `docs/rom/` |
| [Symbols](../rom/symbols.md)' numbers under What is mapped | `tools/symbols.py` | the same |
| `docs/voice/catalog.json` | `tools/voice_catalog.py` | the words files, `docs/voice/sweep/` |
| `docs/log.md`, and each concept's `generated` line | `tools/log_fold.py` | the entries' files in `docs/log.d/` ([The log](#the-log)) |

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
| No branch changes a written file | `build.py lint` on every branch, new: against the commit the branch left `dev` at (in CI, against `dev`'s tip under the pull request's merge). A branch that changes a tool itself (`tools/re_status.py`, `tools/symbols.py`, `tools/voice_catalog.py`) writes that tool's files, and is held to their being in step instead |

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

## The log

`docs/log.md` took a line per change at its top, under the day's heading,
so it conflicted in 158 of the 261 merges, in 47 of them alone. The
`merge=union` attribute joined both sides' lines on a machine, and GitHub
ignores it (above), so the attribute is gone and the log is a file a tool
writes:

- A branch logs a change as a file of its own,
  `docs/log.d/DATE-WORDS.txt` (`2026-11-12-merging-in-parallel.txt`): the
  day it is logged under and a few words that name it, holding `by:` and
  `at:` (who writes, and when), then the log's line as it is to stand
  (`* **Update**: ...`; more than one line where one change has more than
  one entry).
- `dev`'s workflow runs `tools/log_fold.py` with the other tools: each
  file's lines go into `docs/log.md` under its day's heading, above the
  lines the day has, a new day's heading where the days stay newest first,
  and the file is removed.
- `build.py lint` holds the files well formed (`tools/log_fold.py --lint`)
  and fails a branch that changes `docs/log.md` itself; on `main` it fails
  while an entry waits unfolded.

### A concept's `generated` line

Every concept opens with `generated: { by, at }`, and every branch that
changed a concept rewrote that line. So two branches that changed one
page conflicted there whatever else they changed: of 235 conflicts in
pages of `docs/` among the 261 merges, 114 were that line and nothing
else (`docs/devtools/differential.md` in 27 of its 30, `docs/rom/bn4.md`
in 25 of 35), and 45 of the merges had no other conflict left once the
log, the written files and the registries were out of the way. A coarser
time would not have helped: no two sides stamped the same day as the
same writer.

The fold therefore writes the line too. An entry's log lines name the
concepts changed, each by its path before what changed in parentheses
(`project/merging.md (what changed), code/tools.md (its row)`); the fold
writes the entry's `by` and `at` into the `generated` line of each (the
later time where the line holds one: a concept's time never goes back)
and drops a `verified` line there.
A path inside parentheses or code is a mention and stamps nothing;
`python3 tools/log_fold.py --lint` prints what each waiting entry will
stamp. A branch leaves the line of a concept that exists alone, and
`build.py lint` fails one that changes it; a new concept is written with
its own line.

This is the okf skill's rule (refresh `generated`, append a `log.md`
entry, never edit a past one) by another road: the skill's own files are signed and stay as they
came, and `AGENTS.md` says what this bundle does instead.

A branch that still added its lines to `docs/log.md` and stamped its
concepts loses nothing: `python3 tools/generated.py restore`, in its merge
of `dev` or outside one, writes the lines the branch had added as a file
in `docs/log.d/`, with the `by` and `at` the branch had stamped, before it
puts the log and the concepts' `generated` lines back as `dev` has them
(in a merge, a page whose only conflict was that line comes out
resolved).

## Registries

Three files held an entry for every feature, spec or family, and every
branch added its own at the same place, their end:
`docs/re/ledger.json`, `tests/xdiff/mutations.json` and
`tests/xdiff/legit.json`. Each is now a folder with a file an entry, which
`tools/shards.py` reads as the one value the file held (its `LAYOUTS` say
how each folder is laid out), so every tool reads what it read before:

| Folder | A file is | Read by |
| --- | --- | --- |
| `docs/re/ledger/` | A feature, a change or an experiment ([the research status](../re/README.md)); `ledger.json` the rest | `tools/re_status.py` |
| `tests/xdiff/mutations/` | A spec's mutations, a list; `about.json` what they are | `tools/xdiff.py --mutations` |
| `tests/xdiff/legit/` | A map, a layout, a mask, an allowed difference of BN4's, a sound of BN4's; `rng.json` and `about.json` | `tools/xdiff.py`, `tools/xdiff_bn4.py`, the recipe generators |

Two branches that each add an entry add two files, and merge in either
order to the same tree. A list's files are read in the order of their
names: a ledger's change or experiment is named by its date and its
words, the rest by a number and their words (`012-name.json`). Two
branches may take the same number; they differ in their words, and are
read in the words' order. What still conflicts is an entry two branches
change, in that entry's file alone, and the two places of `legit` where
branches add to one map (`maps/name_id.json`, `maps/effect.json`, a line
per moved Navi).

The ledger's two dates, `updated` and `previous`, were written by every
branch and are no longer held: `tools/re_status.py` reads them off the
entries (the newest date, and the one before it).

`build.py lint` runs `python3 tools/shards.py check`: each folder reads
whole, is written as the tool writes it, and none of the old single files
is back.

### A branch that still changed an old file

In its merge of `dev`, git reports the old file "deleted by them":

```sh
git merge origin/dev
python3 tools/generated.py restore
python3 tools/shards.py absorb    # the branch's entries carried into the folders, the old files removed, staged
```

`absorb` merges by entry, not by line: what the branch added or changed
since the commit it left `dev` at, put onto `dev`'s folder. An entry or a
value both changed keeps `dev`'s and is printed with the branch's, to
merge by hand.

## When a branch brings dev in

The rule is `AGENTS.md`'s (Pull requests, Merging). A branch brings `dev`
in when GitHub calls its pull request conflicting, which now means a
conflict in a file a person writes, or when it needs what merged since (a
function another branch changed, a counted value another took). It does
not because `dev` moved: GitHub merges a pull request without conflict
however old its base, and `dev`'s own CI on the merge checks the two
together.

## What it saves, and what is left

Against the 261 merges measured above:

| Merges of `dev` into a branch | Of 261 | Now |
| --- | ---: | --- |
| With no conflict at all | 39 | Not needed: `dev` having moved is no reason |
| With conflicts only in the log, the written files, the registries, the catalog | 81 | Not needed: none of those can conflict |
| With, beyond those, only a concept's `generated` line | 45 | Not needed: `dev`'s workflow writes the line |
| With a conflict in what a person wrote in `docs/`, and no code | 37 | Still needed, with fewer files to resolve |
| With a conflict in code, a tool, a test or the build | 59 | Still needed |

So 165 of the 261 (63%) would not have happened. What still conflicts, by
how often it did once the lines above are set aside:

| File | Of 261 | Why | What would end it |
| --- | ---: | --- | --- |
| `CHANGELOG.md` | 25 | A line per change under the coming version | Entries as files, folded as the log is |
| `docs/code/emu.md`, `tools.md`, `gfx-and-audio.md` | 18, 6, 4 | A row per new file, added at the end or beside the last one added | Rows in the files' order |
| `docs/multirom/index.md`, `docs/rom/index.md` and the other `index.md` | 15, 4 | A concept's line repeats its description | Written from the concepts' frontmatter, as the other generated files |
| `src/net/layer_make.h` | 13 | A counter and its history on one line | Counted from a file a bump |
| `tests/test_core.c` | 11 | Counts and asserts added in the same functions | Counts read off the tables |
| `docs/rom/bn4.md` | 10 | A row per find, at the table's end | Rows in address order, not at the end |
| `AGENTS.md` | 7 | The table of tasks, the rules, the checks: rows added side by side | None: a person's page, merged by hand |
| `src/emu/free_space.h`, `docs/engine/emulation/memory.md` | 4, 4 | Written from `free_space.plan`, but the build needs them | Regions claimed before the work starts |

None of these is built. The counters among them (`layer_make.h`, the
tests' counts, the free space's regions) wait on the owner's word.
