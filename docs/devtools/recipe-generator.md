---
type: Tool
title: "The recipe generator"
description: "`tools/xrecipe.py` derives the recipes of BN5's 25 virus families BN6 lacks from both ROMs, numbers only, writing `src/emu/xvirus_bn5.c` and `tests/xdiff/families.json`, with `--check` to compare and `--report` for each family."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-12T23:50:43Z }
---

# The recipe generator

```bash
python3 tools/xrecipe.py              # write src/emu/xvirus_bn5.c: the installed families' recipes, from the two ROMs
python3 tools/xrecipe.py --check      # write nothing: fail where the file differs from what it would write
python3 tools/xrecipe.py --report     # every BN5 family derived, in install order: .build/xrecipe/report.md
python3 tools/xrecipe.py --report --ai 25 26   # the report's families by BN5 AI index (the installed first)
```

`tools/xrecipe.py` (issue #82; [Transplanted families](../multirom/families.md))
derives a BN5 family's recipe from BN5's and BN6's ROMs (found by their
SHA-1 in `CYBERWORLD_ROM_DIR` or `~/.cache/mmbn-ref/roms`), numbers only.
Its parts: `tools/xrecipe_thumb.py` decodes, walks and compares Thumb
(a body normalized with its calls, branches out and literals masked:
SAME, LAYOUT where only the offsets of loads and stores differ, DIFF);
`tools/xrecipe_pairs.py` pairs a BN5 routine with its BN6 twin (the
project's own in `tools/bn5_twins.py`, then the symbol map's in
`.build/symbols/match` when `build.py symbols --bn6f` made it, then a
BN6 routine of the same body, then where its neighbours put it; BN6's
bodies indexed once in `.build/xrecipe`); `tools/xrecipe_family.py`
reads a family (its blocks, data, calls, immediates, states, hooks,
records and draws). `tools/bn5_twins.py` holds the pairs and each
differing pair's audit with its conditions, which the generator checks
on each family's code ([Virus families moved into BN6's battle engine](../rom/virus-families.md), The engine's differing twins).
Numbers are given in install order (`FAMILIES` first: an AI index from
32, six ids from 0x200, the static layout's next home, code and art
places, sprite numbers and effect ids where the grown tables hold their
fill). A family is whole where no decision is left open; the report
says what each derives and what is left, and `--check` runs in no build
(it needs the ROMs): `build.py test` holds the generated CanGard and
Handi to the installed recipes, field for field.

Since issue #82's A2b it derives all 25 of the families BN6 lacks (the
`FAMILIES` list), each whole, and writes `tests/xdiff/families.json`
beside the C file, the numbers the differential reads a spec's families
by (their ids, AI indexes, effects, sounds, callbacks' twins, effect bytes,
attacks with a virus's states, moved types, struct2's place and their
states' count). What it has gained for them, each from the two ROMs or
read by hand in `tools/bn5_twins.py`: the held chips as the chip
registry's ids (a chip BN6 has as BN6's, one it lacks as the registry's own
id past 255), and a compare of a held chip's id with such an id made a
shim's call (`XvCmp`); a NameID range whose bounds its code makes in
instructions made a shim's call that loads BN6's bounds (`XvRange`); BN5's
virus dispatcher's case for Dominerd's and Appley's ids made their
pre-step (`PRESTEP`, ported by `src/emu/xvirus_shim.c`); a sound BN6 has
no song of copied into an empty slot (`XvSong`, by the songs' tracks and
voices, BN6's own number where one is the same); ids kept in tables of
its data given BN6's numbers byte by byte (`ID_TABLES`, `XvByte`); an
object type whose twin reads a table by the object's kind that BN6 laid
out otherwise moved with its code (`MOVE_TYPES`), and a type a family
before it moved, or BN5's engine objects moved once before every family
(`tools/bn5_objects.py`: a call into their module that would be copied
goes to their copy, WuNote's to the image's spawner, its kinds fixed
there), run as that copy (`XV_TYPE_SHARED`); a number its
code keeps as it is only where it was read in BN6 bit by bit (`KEPT`); an
audit may serve BN5's own body (`copy`: object_updateCollisionData, whose
BN6 twin writes the second word BN5's stores into the BIOS); each compare
of a CurAction read given BN6's number; a sprite of a list BN6 has not
grown placed in list 4; and a call into the dead bytes of its block (a
routine of its game's BN6 has a twin of, zeroed in the copy) goes to the
twin (`src/emu/xvirus.c`, dead_calls); its calls of BN5's chip writer
(`0x0800FE6A`, a held chip's use) go to the dispatch's own
(`CHIP_WRITER`: `XV_CHIP_WRITER`, BN6's writer with a support chip's
family run in BN5's support family's frame, Zomon's AreaGrab). Tuby and
the unnamed AI 31-33 are left out: no battle record of BN5's names
them.
