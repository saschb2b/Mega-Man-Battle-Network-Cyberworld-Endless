---
type: Tool
title: "Encounter tags and danger"
description: "`tools/encounter_tags.py` tags every virus family in the engine from the ROMs (versions, element, weakness, role, reward row) and counts how each battle template fills per ROM set and act; `tools/encounter_danger.py` fights battles headless for their pressure, tricks and the autopilot's losses."
tags: [devtools, progression]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-12T21:00:00Z }
---

# Encounter tags and danger

The measuring tools of [encounter groups](../game/progression/encounter-groups.md):
what the families of every ROM are, what battles they can build, and what
those battles cost MegaMan. Their numbers are in
[Encounter groups, measured](../game/progression/encounter-groups-measured.md).
Neither changes what a player's run holds.

```bash
python3 tools/encounter_tags.py dump      # the ROMs' families into tools/encounter_data/tags.tsv (every ROM beside BN6's)
python3 tools/encounter_danger.py families --out tools/encounter_data/families.tsv   # each family alone: pressure and tricks
python3 tools/encounter_danger.py pairs --out tools/encounter_data/pairs.tsv   # every two families together at V1 (1,830 runs; --most N a portion, a run stopped goes on)
python3 tools/encounter_tags.py sample    # .build/encounter/battles.tsv: each act's own battles and the templates' fillings
python3 tools/encounter_danger.py battles .build/encounter/battles.tsv --out tools/encounter_data/danger.tsv
python3 tools/encounter_tags.py report [--write]   # the tables (--write: into the measured concept, after its mark)
python3 tools/encounter_tags.py engine [--check]   # src/core/group_tags.c: each family's pressure and tricks, for the engine's groups
```

## The tags' dump

`build/host/cyberworld --encounter-tags PATH` (`src/dev/encounter_tags.c`)
starts a core with the other games' families installed, as a run's does,
and writes tab-separated lines, numbers alone:

| Line | What it holds |
| --- | --- |
| `band` | An act's band as `pacing_band` gives it: floor, top, the cap on one hit, the easy half's top, the versions it rolls, the stage it has reached |
| `pool` | The areas acts 1 to 4 draw on (`pacing_area_pool`) |
| `fam` | A family in the engine: its net (0 BN6, 1 BN5, 2 BN4), BN6's AI index, its AI index in its own game, whether it was moved for a summon alone |
| `ver` | One version: enemy id, HP, hit, first-wheel element, the second wheel's element and weakness bits, its stats' flags byte, its traits' eight bytes, its reward row's twenty entries |
| `kin` | A net's records' placements of a family: the visitors' role, the enemy's first, middle and last column, its stage, bound to special panels, may visit ([One net](../multirom/one-net.md)) |
| `form` | One of a BN6 area's own random battles: field, weight, its entities (id, panel, type, AI index, version) |
| `xarea`, `xform` | Another game's area a run takes, and its records as BN6's engine fights them |

`tools/encounter_tags.py dump` runs it with every ROM in
`CYBERWORLD_ROM_DIR` and keeps the result in `tools/encounter_data/tags.tsv`,
so `sample` and `report` need no ROM. A family is named by its net and
BN6's AI index (`6:01`, `5:33`, `4:64`); no name, text or picture of the
games' reaches the file or the report. Each ROM set (BN6 alone, with BN5,
with BN4, with both) is the same dump with the absent nets' families left
out: a family both games have by name is BN6's in every set.

## A battle's pressure and tricks

`CYBERWORLD_BATTLE_PROBE=1` (`src/emu/debug.c`) prints a line every 30th
frame of a battle: its phase, MegaMan's HP, the enemies in play and their
HP together, and the field's 18 panels, a letter each from the top left
(the type from `a`, so `c` plain, `b` a hole, `d` cracked; upper case the
enemies' side).

`tools/encounter_danger.py` fights one battle a run through the `foes`
step ([A battle on demand](battle-on-demand.md)) with `--dev
quiet,hp=2000`, so no battle ends MegaMan, and reads those lines:

- **idle**: MegaMan leaves the Custom screen with no chip and stands. The
  HP lost in the fight's first 20 seconds is the battle's **pressure**; a
  panel of his side that changes type, a panel taken, or more enemies than
  the battle set are the family's **tricks** (`panels:`, `field:` for its
  own side, `area`, `summons`).
- **fight**: the test autopilot fights with the starting folder for 50
  seconds: whether it wins, how long it takes, the HP it loses. A family
  whose V1, alone, it does not delete in that time is tagged `tough` (it
  guards, hides or stays out of the buster's row).

A run is about ten seconds; `--jobs N` runs them side by side, each in its
own data folder under `.build/encounter/runs`.

## The report

`tools/encounter_tags.py report` reads the three files of
`tools/encounter_data/` and prints: the families' table; each ROM set's
pool by act (the families whose version fits the band alone, by lean,
element and trick); every template's count of fillings per act (those
mixing two nets in brackets, and in square brackets the least and most an
area of the act holds one of its own families in); how many families fill
three templates or more; and the danger sampled, each battle's pressure
against the sum its families' own predict. `--write` puts the tables into
the measured concept after its mark.

## The engine's table and its check

`tools/encounter_tags.py engine` writes `src/core/group_tags.c` from
`families.tsv`: a row a family measured, its net, BN6's AI index, its
pressure and its tricks as the engine's bits (`GROUP_BREAKS`,
`GROUP_SHAPES`, `GROUP_SUMMONS`, `GROUP_TOUGH`, `src/core/groups.h`);
and from `pairs.tsv` the pairs that press a standing MegaMan 40 HP or
more past the sum of their own pressures (`PAIR_EXTRA`), each with that
excess (`group_pairs`), which both the tool's and the engine's
prediction add for a group that holds the pair;
`--check` fails where the file is not what it would write. After a
family is moved in: `dump`, `encounter_danger.py families` for the new
one, `engine`, `report --write`.

The engine's groups (`src/core/groups.c`, behind `--dev groups`) and
this tool fill the templates by the same rule, each from its own
reading: the tool from the dump, the engine from the ROMs. `build.py
pacing` prints the engine's counts ([the pacing
report](pacing-report.md), the encounter groups), and they are compared
by hand with `fillings` here, per ROM set, act and template: all 308
agreed on 11 November 2026, before the pairs' pass and after it.
