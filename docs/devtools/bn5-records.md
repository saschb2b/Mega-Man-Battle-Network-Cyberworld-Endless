---
type: Tool
title: "BN5's records in BN6's engine"
description: "`python3 build.py xrecords [--all]` (`tools/xrecords.py`) lists every battle record of BN5's areas as BN6's engine fights it or why it cannot, and checks each area's random battles roll the same again within the band."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-12T01:31:34Z }
---

# BN5's records in BN6's engine

```bash
python3 build.py xrecords          # the areas' lines: how many records fightable, the families and fields holding them back
python3 build.py xrecords --all    # ... and every record's line
```

`tools/xrecords.py` (issue #82; [One net: the design](../multirom/one-net.md)) copies both
ROMs into `.build/xrecords/rom`, builds the host binary and runs its
`--xrecords PATH` (`src/dev/xrecords_report.c`, a tool run in the game's
place), which starts a core with the families installed as a run's core
starts, then lists every record of the three maps each of BN5's areas
takes its battles from: as BN5 has it (its field, its enemies by name and
id at their panels, row then column as its own bytes are, its Mystery
Data, rocks and Navis), and as BN6's engine fights it (`xbattle_translate`:
BN6's field, the foes by BN6's ids, the Mystery Data and its chance) or
why it cannot yet; per area how many are fightable, whether all are
(`xterritory_native`), the families still holding records back and the
fields of panels BN6 lacks; and, for an area with any, its random battles
as BN6's roll would pick them (`xterritory_encounter`, issue #82's M3):
six a layer at every depth of the first cycle, each record's HP against
the band, from one seed and again from it, `NOT THE SAME AGAIN` where the
two differ and `OVER` where one lies past the band. The listing is in
`.build/xrecords/list.txt`; it exits 1 where a roll was not the same
again.
