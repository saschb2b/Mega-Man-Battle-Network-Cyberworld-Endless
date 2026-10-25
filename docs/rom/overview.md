---
type: Overview
title: "ROM data"
description: "Where BN6 Gregar's ROM data lives: every offset is for the USA ROM by its SHA-1 and lives in `RomLayout` (`src/core/rom.c`), found through bn6f and checked in `tools/romlab`; the tables by subsystem."
tags: [rom]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-25T12:29:26Z }
sources:
  - id: rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6, read in tools/romlab and the engine"
    title: "The player's BN6 ROM"
  - id: bn6f
    resource: "https://github.com/dism-exe/bn6f"
    title: "bn6f, the Cybeast Falzar disassembly"
---

# ROM data

All offsets are for Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1
`89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`, and live in `RomLayout` in
`src/core/rom.c`. The [bn6f disassembly](https://github.com/dism-exe/bn6f)
targets Cybeast Falzar; most code-region tables sit at the same addresses in
Gregar, and the rest were found by searching the Gregar ROM for the
structures the disassembly describes. `tools/romlab` runs the ROM in libmgba
to confirm findings against real frames and VRAM.

## Contents

`tools/symbols.py` reads the tables in this order, and a symbol two rows name takes
the first one's name: a new table goes at the end, so no published name changes.

- [Symbols](symbols.md)
- [Sprites, fonts and pictures](graphics.md)
- [The game's structures, boot and events](game-state.md)
- [BN6's battles](battle.md)
- [Chips, folders and the Library](chips.md)
- [BN5's Liberation Missions](liberation.md)
- [The net's maps and their pieces](net-maps.md)
- [The real world and home](world.md)
- [Scripts, text and shops](scripts-and-shops.md)
- [Sound and music](sound.md)
- [The NaviCust](navicust.md)
- [The PET and the second screen](pet-and-second-screen.md)
- [The chip registry and BN5's chips' code](chips-code.md)
- [Online: the Square, the Arena and the link battle](online.md)
- [BN5's maps, songs, text and objects](bn5-world.md)
- [BN5's battles, records and rewards](bn5-battles.md)
- [Virus families moved into BN6's battle engine](virus-families.md)
- [Navis moved into BN6's battle engine](navis.md)
- [MegaMan's forms and BN5's Souls in BN6's engine](souls.md)
- [BN6's Virus Battler](virus-battler.md)
