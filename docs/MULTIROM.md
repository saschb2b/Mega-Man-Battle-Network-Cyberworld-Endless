# Other games' ROMs

Cyberworld Endless runs Battle Network 6: Cybeast Gregar in mGBA and
directs it. Where a player has other Battle Network games' ROMs beside
BN6's, they can lend a run what their data holds: more net areas, towns,
music, bystanders. BN6 stays the one ROM a run needs; each other is
optional, and the more there are, the bigger the pools a run draws from.

## Data, not code

Every battle, chip effect, virus AI and boss AI in a run is BN6's own
code; the engine directs it and writes data into BN6's in-memory ROM
(docs/EMULATION.md). Battle Network 4, 5 and 6 share one engine family:
their maps, tile sets, palettes, coordinate data, sprites, text scripts,
chip tables and MP2K songs are the same formats or close to them. A
second game's data can be drawn and played by BN6; its code cannot run
inside BN6 (it calls its own game's addresses).

| Pool | How another game fills it | Cost |
| --- | --- | --- |
| Net areas | Its maps learned as BN6's are (`tiles.c`), its tile set and palette copied into BN6's free space | moderate |
| Towns | The same, from its real-world maps | moderate |
| Music, sounds | Its MP2K songs and their voice groups into BN6's song table | low |
| NPCs, mugshots | Its sprites as bystanders and dealers | low to moderate |
| Chips | Where BN6 has the behaviour (most standard chips); its own (Navi chips, DarkChips) have no code in BN6 | limited |
| Viruses | The families both games have; its own need their AI | limited |
| Guardians | Its Navis need their AI: a battle in its own engine | high |

For what needs its game's code, the way is a battle in that game: the
engine keeps BN6's state, switches the core to the other ROM from a state
made there, sets up its battle (MegaMan's HP, his folder in its chips),
runs it, reads the result and brings BN6 back. That takes a battle-level
port of the other game (its RAM, as `src/emu/bn6.h` is BN6's), a second
ROM in memory, and shows its battle screens.

## How it is built

1. **Extra ROMs** (`src/core/rom.c`, `XR[]`): found in the ROM's folder by
   their header's game code and SHA-1, read and never run
   (`xrom_find`). Each has its layout (`XRomLayout`): its tables, found by
   their structure (`tools/rom_tables.py`) and noted in docs/ROM_DATA.md.
2. **A map from any of them** (`area_src_load_x`): the map reader takes
   its maps from the game's ROM; what learns from a map (`tiles.c`,
   `props.c`, `decor.c`, `stairs.c`) works on the decoded map, whichever
   game it came from, and each map keeps where it was read (`AreaSrc.rom`).
3. **Into BN6** (next): a layer made in another game's tiles carries its
   tile set and palette into BN6's in-memory free space, under a map
   descriptor of its own for the map it takes over, one whose map scripts
   animate nothing over them.
4. **Pools by source**: each area, town, song and bystander is an entry
   with the game it came from; BN6 alone is today's game.
5. **Runs remember their games**: a run is made from the pools of the ROMs
   there, so it keeps which, and continues only with them (a `Run`
   change: `RUN_MAGIC`).
6. **Platforms**: desktop first. A New 3DS's 96 MB heap holds BN6 twice
   (mGBA's copy and the engine's) with little room left: there another
   game's data would be taken in as a run needs it, its ROM not kept.

## Status

- [x] Battle Network 5: Team Colonel (USA): its map and coordinate
  tables, its maps drawn through the engine's map reader (`--atlas
  DIR:x0`: ACDC Town, Lan's room, the harbour, the net areas from 0x80).
- [ ] One of its net areas as a biome: its tiles learned, its tile set
  and palette in BN6's free space, a host map that animates nothing.
- [ ] Its music.
- [ ] Its towns and bystanders.
- [ ] Its guardians, in battles in its own engine.
- [ ] Team ProtoMan (the other version), Battle Network 4, BN6 Falzar.
