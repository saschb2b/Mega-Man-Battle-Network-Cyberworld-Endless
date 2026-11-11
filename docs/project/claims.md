---
type: Process
title: "Claims"
description: "What each branch in flight may take of the numbers branches share (chip ids, enemy ids, AI numbers, list numbers, song slots, free-space spans, units of work, `Run`): a file a branch in `docs/project/claims/`, written before the branch starts and held apart by `tools/claims.py`."
tags: [project, process, git]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-11T21:31:59Z }
sources:
  - { id: notes, title: "The sessions' shared notes of 11 November 2026 (bn4-units-claims.md, team-protoman-claims.md): who moves which unit, which list numbers, rooms and song slots each takes", resource: "the sessions' notes folder on the owner's machine, outside the repository" }
---

# Claims

Several branches take numbers from the same ranges by hand: a chip's id,
a family's enemy ids and AI number, a type in the dispatch, a sprite
list's numbers, a song slot, a span of the free space. Two that take the
same one do not conflict in git (each writes its own table), so the clash
shows when the second merges, in a test or in the game, or not at all.
The sessions kept this apart in notes outside the repository; a claim is
the same promise where `build.py lint` can read it. It belongs with
[Merging in parallel](merging.md), which keeps branches' files apart; this
keeps their numbers apart.

## A claim

The session that starts others writes each one's claim before it starts
them, one small pull request for the batch, and names the file in each
brief. `docs/project/claims/BRANCH.json`, the branch's name:

```json
{
  "branch": "bn5-magnetman",
  "what": "MagnetMan moved, Team ProtoMan's fifth",
  "takes": {
    "enemy_ids": [["0x3A8", "0x3AB"]],
    "song_slots": [["0x45", "0x45"]],
    "free_rom": [["0x141000", "0x14FFFF"]]
  },
  "units": ["MagnetMan"],
  "run": false
}
```

- `takes`: a space and the ranges taken in it, each its first and last
  number, both taken (numbers, or strings as C writes them).
- `units`: pieces of work by name, where the thing shared is the work
  itself (a chip's unit, a Navi, a page being rewritten).
- `run`: the branch changes `Run` (`src/core/run.h`). Only one branch at a
  time may: two change its size, `RUN_MAGIC` and what `save_legacy.c`
  reads of the last, and no merge of the two is either's.

A branch takes numbers only from its claim. Its pull request removes the
file, so the folder holds what is in flight and nothing else. A session
that finds it needs more adds to its own claim's file in its branch;
`build.py lint` (`python3 tools/claims.py --lint`) fails where two claims
meet: a range in a space of the same name, a unit of the same name, or
`run` in more than one.

## The spaces

A space is whatever the claims call it; two claims meet only in a space
of one name, so the names below are the ones to use:

| Space | What is taken | Where it ends up |
| --- | --- | --- |
| `chip_ids` | Chip ids, the Pack's entries with them | `src/core/xchips_registry*.c` |
| `enemy_ids` | A family's or Navi's enemy ids | `src/emu/xvirus_*.c`, `src/emu/xnavi_*.c` |
| `virus_ai`, `navi_ai` | AI numbers | the same |
| `dispatch3`, `dispatch4` | The moved table's types of the third and fourth kind | `tools/xchiprecipe_bn4.py`, `tools/bn5_twins.py` |
| `list2`, `list4` | Sprite lists' numbers | `tools/xnavirecipe.py`, the families' recipes |
| `song_slots` | Slots of BN6's song table | `src/audio/xsong.c`, `src/emu/song_table.c` |
| `free_rom`, `free_ram` | Spans of the free space, as offsets from `EMU_FREE` and as RAM addresses | `src/emu/free_space.plan` |

## What it does not do

It holds claims apart from each other, not code inside its claim: nothing
checks yet that the ids a branch writes lie in what it claimed. For the
free space that check could read `free_space.plan`; for the rest it needs
a reader per table.
