---
type: Tool
title: "Chip texts: every chip's name and description its own"
description: "`tools/chip_texts.py` dumps every chip id's name and description as BN6's own routines find them in the core's copy (the host build's `--chip-texts`, all three ROMs read) and compares each with its own entry in BN6's, BN5's or BN4's archives, naming the entry a wrong text came from."
tags: [devtools, chips, multirom]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T03:45:00Z }
---

# Chip texts: every chip's name and description its own

A chip's name and description reach the player through BN6's own
routines: `BN6_CHIP_NAME_OF` and `BN6_CHIP_DESC_OF` (the shop, the battle
results) and the PET's two readers of a description (`BN6_CHIP_DESC_READ`,
`..._READ2`: the Folder and the Pack). Each takes the archive of the id's
high byte from the words its literal names, which the install points at the
core's four archives ([how the chips are built](../multirom/chip-registry.md);
ten-bit ids in [the chips' code](../rom/chips-code.md)), and its entry by the
id's low byte. This check reads them as those routines do and holds each
against the entry it should be: BN6's own id, or the registry row's id in
BN5's or BN4's archives.

```bash
python3 tools/chip_texts.py            # the host build's dump (build.py shot --chip-texts), then the comparison
python3 tools/chip_texts.py FILE.tsv   # a dump made before
```

It needs the ROMs (`CYBERWORLD_ROM_DIR`, else `~/.cache/mmbn-ref/roms`),
so it stays out of `build.py test`, which runs without them. It compares
letters, digits and spaces alone (the codes below 0x40, the same glyphs in
the three games; punctuation and the boxes' commands are transcoded, BN5's
DS mark spelt D and S). It fails where a chip's name or description is
another's or missing, naming the entry the wrong one came from, and where
the PET's readers take another archive than `BN6_CHIP_DESC_OF`.

## What it found

Run on 15 November 2026 (dev after 0.22.0's notes): 721 chips checked, none
wrong, and the three readers agree for all 1,024 ids. Two texts a
playtester read as wrong are the original games' own (session 77, kai5),
read from the ROMs' archives and seen in the Folder:

- MarkCannon1-3 (ids 411-413, BN5's 12-14): BN5 Team Colonel's own
  description of all three names DoomerEX. BN6 shows it as BN5 does.
- Thunder (BN6's id 30, XREG_SAME with BN5's 19): its description's
  "Pralyzing" is spelt so in BN6's archive and in BN5's.

Two of the games' own entries the comparison reads as their readers do:
BN6's name of id 284 has no end code, so its reader runs on into the next
entry's name (the core's copy keeps BN6's bytes), and BN4's descriptions
281-292 are empty boxes without one, which the core sets as empty boxes.
