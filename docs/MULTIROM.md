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
| Viruses | The families both games have; its own need their AI. BN5 shares 5 of its 33 with BN6 (Mettaur, Catack, Champy, WindBox, Trumpy, by the first name of each family in both games' tables), and its areas' battles are mostly its own (ACDC Area's CanGards and Powies), so a mix from its data alone would take the BN5 out of them | limited |
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
3. **Into BN6**: another game's areas (`XRomLayout.areas`, the same
   `NetAreaDef` as BN6's own, numbered after BN6's 19) name the BN6 map
   their layers take over (`over`) and the BN6 area whose layouts and
   furnishings they take (`like`). A layer in another game's tiles copies
   that game's tile set (its two blocks decoded and encoded again, after a
   header of its own) and palette into BN6's in-memory free space
   (`+0x170000`, docs/EMULATION.md), and the taken-over map's descriptor
   points at them. BN5's tile sets fill the same VRAM as BN6's (their
   first block at 0, the second after it: 31200 bytes against 30496).
   The map's backdrop and animations come with them (`xbackdrop_map`):
   the learned map's backdrop (its record, tiles, map and palette) and
   its GFXAnim scripts (colour cycles, the backdrop's turning tiles) are
   copied into `+0x310000`, and the taken-over map's entries in its
   group's three tables (animation list, backdrop record, scroll
   callbacks) point at them; a layer of BN6's own puts BN6's back.
4. **Pools by source**: each area, town, song and bystander is an entry
   with the game it came from; BN6 alone is today's game.
5. **Runs follow the games there**: an area of another game dresses the BN6
   area it is like (`run_dress`): its layouts, guardian and battles are
   that area's, its tiles, name and theme its own. A theme (`xsong`, a song
   in its game's song table) is copied into BN6's free space with what it
   plays on (`src/audio/xsong.c`): its sequence with the pointers moved,
   the voices it selects, their key splits, drum kits and samples, and
   entered in one of BN6's 61 empty song slots, where BN6's own sound
   engine plays it as one of its own; its battles name its game's battle
   themes, copied the same way (`encounter_song`), and its areas' battle
   backgrounds (`encounter_backdrop`). The run's seed picks it,
   with the ROMs present, so nothing is saved: a run continued without
   the other ROM goes on in the BN6 area's tiles.
6. **Platforms**: desktop first. Phones keep copies: Android's
   `RomActivity` and iOS's ROM screen (`src/core/ios.m`) copy BN5's ROM
   beside BN6's from the folder the player chose, and look in it again at
   each start, so BN5 put there later comes in (issue #66). A New 3DS's
   96 MB heap holds BN6 twice (mGBA's copy and the engine's) with little
   room left: there another game's data would be taken in as a run needs
   it, its ROM not kept.

## Guest battles

Where BN5 dresses an area and the build can run a second core (desktop,
Android, iOS, PortMaster; not the 3DS, which keeps one ROM, nor the
browser), the layer's random battles are BN5's own, fought in BN5's
engine (`src/emu/guest.c`, epic #57):

- **When**: BN6's roll decides there is a battle, as on any layer; the
  hook gives BN6 none (`encounter_guest`), and the director begins a
  guest battle. BN6's core waits while the guest runs; the screen and the
  sound are the guest's.
- **Which**: the area's own maps' records in BN5's numbers (ACDC Area
  0x90:0, 0x90:1 and 0x86:0 for an act's first, middle and guardian's
  layers; Oran Area 0x91:0, 0x91:1, 0x86:1; SciLab 0x92:0, 0x92:1, 0x86:2;
  End Area 0x93:0, 0x86:3, 0x93:1; the Undernet 0x94:0, 0x94:1, 0x86:5;
  Nebula Area 0x86:6, 0x94:3, 0x86:7), one picked from the layer's seed
  and the battle's count, held to the act's band as BN6's battles are
  (pacing.c: their viruses' HP together and the strongest one's damage,
  from BN5's own enemy tables): the area's own records that fit, else its
  other maps', else those of BN5's other areas that fit (End Area in an
  opening act fights ACDC Area's viruses), else its own weakest. BN5's
  story battles (a record's byte 7 set, its roaming Navis at 2000 HP among
  them) never.
- **MegaMan as the run has him**: his HP and max HP, and the run's folder,
  each chip as BN5's chip of the same name with its own code (the names
  read from both ROMs and paired by `src/core/xchips.c`, which
  `tests/test_core.c` runs on made-up tables; 114 of BN6's 313 chips
  pair), owned in BN5's marks.
  A chip BN5 has none of sits out, its slot empty (a Standard folder keeps
  26 of its 30). His buster is the run's: its Attack, Speed and Charge as
  BN6's NaviCust makes them (Attack+1 to ChargMAX, BustPack), written into
  BN5's NaviStats before each battle, which keeps them at the same five
  levels and reads them with BN6's own code (a shot Attack + 1, a charged
  shot ten times that); BN5's boot state fired 1 a shot and 10 a charge
  whatever the run's programs. The NaviCust's other programs sit out:
  parity, no power of BN5's own.
- **The record's copy**: past BN5's 8 MB in the guest's ROM copy, without
  its GAME OVER bit, so a loss ends on the map with BN5's result 2 and the
  run ends as BN6's would (the deletion shown in BN5's battle).
- **The end**: BN5's result (won, lost, escaped) and the HP the battle
  left (its BattleState's copy, as BN5 copies it back to its NaviStats
  only on its own maps' terms) go back into the run. The results screen's
  reward goes into BN6: a chip as BN6's chip of the same name, to the Pack
  (GiveChips), zenny as zenny (GiveZenny). Before each battle, its
  enemies' reward rows are rewritten where a chip has no BN6 chip of its
  name: 200 zenny instead, so the screen never shows a chip the run won't
  get. An HP+N its screen gives (a row's kind 2, as BN6's) is in the HP
  that comes back. `runlog.txt` has a line for each, as for BN6's battles,
  with its record, its viruses by BN5's ids and the reward as the run got
  it (docs/DEVTOOLS.md).
- **The world's reason**: the Nest's copy of the old net reads our data
  as it knew it, one rule for every seam of the translation. Chips it
  never had sit out; a chip whose code its chip of that name lacks fights
  with one it has (the same letter, else its *, else its first: BN5 draws
  a chip with a code it lacks as nothing); DarkChips are its own; a chip
  won there crosses back in a code our net knows. MegaMan names each the
  first time a profile meets it, with the chip ("Huh? In there our
  Thunder S was Thunder *!"), and says out here the codes are ours again:
  a quirk of two engines told as the net's own law, which a player can
  use (a folder of codes the old net lacks fights with wildcards there).
- **The switch**: arriving where the battles are BN5's, the first time a
  profile does, MegaMan says the older net had no Crosses and names the
  folder's chips it never had, which will sit out (three or fewer by name,
  more as two and how many others: the Storm folder's "Our ElcPuls1,
  DolThdr1 and Atk+10 didn't exist back then either, so they'll sit
  out.", the Standard's CrakShot and Atk+10, the Blade's Atk+10). BN6's
  frame fades to white while BN5's battle opens (its opening drew the
  room its boot state stands in), and BN5's picture flashes in once its
  battle is on its screen; after a profile's first, MegaMan says what it
  was and names the chips that sat out, the same way.
- **For tests**: `--talk guest:FRAME` begins a guest battle at once on such
  a layer (`--net-biome x0`, ACDC Area), `--talk dark:FRAME` opens the
  flame of darkness's talk (a middle layer: `--run-depth 2`), `--dev
  darkchips=MASK` starts a run holding DarkChips (bit k BN5's id 187 + k)
  and `--dev worried` keeps MegaMan worried in guest battles;
  `CYBERWORLD_EMU_DEBUG=1` names each battle, its folder's chips in and
  those that sat out, slot by slot ("guest: 4 sat out: CrakShot A, ..."),
  the buster it goes in with and the one its battle took ("guest: its battle's
  buster Attack 2, Speed 1, Charge 1"), its end, the flame and a
  DarkChip's price.
- **DarkChips** (docs/META.md, issue #64): the run's go into the guest's
  folder (three at most, one of each, as BN5's folder editor allows, in
  slots where chips sat out). BN5 draws them as any chip; the guest's
  deck compaction at each Custom screen (`0x080250EA`) is replaced by one
  that shelves a DarkChip behind the deck's end except where BN5's own
  worried rule put it, so one comes only when MegaMan worries (or is
  dark), as the rule offers it. Used in a battle, BN5 keeps it, and the
  run pays: 20 max HP, from the base HPMemory counts into. Each battle
  starts calm (BN5's mood and dark meter reset), as the run's battles
  share no darkness. A flame of darkness on the middle layer of such an
  act holds one the run lacks: BN5's flame, copied from its ROM and
  turned purple, in a bystander's place.
- **Still to come**: the NaviCust's other programs in (those both games
  have, as BN5's battle reads them), BN5's chips registered in BN6.

## Status

- [x] Battle Network 5: Team Colonel (USA): its map and coordinate
  tables, its maps drawn through the engine's map reader (`--atlas
  DIR:x0`: ACDC Town, Lan's room, the harbour, the net areas from 0x80).
- [x] One of its net areas drawn in the game: ACDC Area 1 (0x90:0), its
  cyan platforms and green walkways learned, its tile set and palette in
  BN6's free space, taking over Central Area 1 and laid out as Central
  Area (`--net-biome x0`; `--atlas DIR:a0:SEEDS` draws its layers).
- [x] Its ACDC Area in runs: it dresses Central Area in half the runs where
  Team Colonel sits beside BN6's ROM (`run_dress`, from the run's seed and
  the ROMs there: no save changes, and a run continued without it goes on
  in Central Area's tiles), named on the act's card, in MegaMan's words,
  L's and the PET's. Read where BN6's ROM is, on every build but the 3DS's
  and the browser's.
- [x] End Area (7-9, dressing Seaside Area) and Nebula Area (13-14, the
  Graveyard, under Green Area's map: the Graveyard's group sets its own
  palette), each tuned over the atlas's layers (`--atlas DIR:aN:3`);
  Nebula Area's floors told by shape among its hues, its cobbles turning
  purple and teal stone by stone.
- [x] Its net maps' backdrops and animations: each dressed area's layers
  stand on its learned map's own backdrop, scrolled as in BN5, with that
  map's tile animation and colour cycles (ACDC Area's blinking diamonds,
  End Area's red field of diamonds, Nebula Area's flickering static and
  pulsing emblems), where they stood on the BN6 map's backdrop with its
  cycles running over BN5's colours (`xbackdrop_map`, docs/ROM_DATA.md).
- [x] SciLab (maps 4-6) dresses Sky Area, with Sky's layouts as they
  are: its maps hold no platform bigger than a 3 x 3 octagon hub with
  ears at its corners, and Sky's rooms drawn in its tiles met their pale
  middles with no frame, so every room up to 64 panels and the guardian's
  arena are drawn in its hubs' look (`pad_rooms`): their rims and ears
  frame them. Sky's octagon rooms come out as twin-hub outlines; a big
  room of an odd shape can keep a rim line inside it.
- [x] Oran Area (2-3) dresses Green Area: orange octagon hubs and olive
  circuits, told by shape. Its first map's raised cream platforms and
  green walkways are drawn in other colour banks of the same ramps, so
  their tiles are read as the hubs' and circuits' banks (`recolour`);
  off its hubs, its brown planks and its arrow panels lend no tiles
  (`SKIP_OFF_PADS`): one walkway look. Its rooms are drawn as its hubs,
  as SciLab's are.
- [x] Its Undernet (10-12) dresses the Undernet: mauve stone plates with
  lava at their corners on cobbled paths, told by shape; its raised
  courts' pale stone and purple slime read as the plates' bank; its
  plates' purple cross its emblem; BN6's statue and braziers stand on it,
  which the number doors count. Named the Undernet, as BN6's is.
  Over the atlas's layers (`--atlas DIR:aN:3`: 3 seeds a layout and the
  guardian's layers; `build.py tiles` lists them too), seams per 100
  panels:
  ACDC Area 132, SciLab 126 (192 before, held), End Area 172, Oran Area
  168 (480 to 500 as first tuned), the Undernet 167 (310 to 440),
  Nebula Area 205.
- [ ] A dressed layer laid out as the BN6 area's own would be: the layer
  generator takes the kit of the area it draws in (`netmap_kit`: stairs,
  arrow lanes, the teleports' gem, the Net Dealer's counter, scenery),
  and BN5's areas learn no stairs, arrow panels, gem or counter, so their
  layers have no raised rooms, arrow lanes, teleports or counter where
  the BN6 area's would, and a run saved with BN5's ROM beside BN6's and
  continued without it builds its layer afresh, not the same. Such a
  CONTINUE starts that layer again from its arrival (the checkpoint keeps
  the area it was drawn in, `run.area`, and stops the saved song, which
  can be a copy of BN5's); the owner's call, 4 October 2026.
- [x] Its areas' themes, from its own map-music lists: ACDC Area, Oran
  Area, SciLab and End Area its net theme (song 0x13), its Undernet and
  Nebula Area its Undernet's (0x14). Its lists name no other song for any
  of its net areas: 0x13 for ACDC Area, the other areas' maps group 0x86
  holds (ACDC Area 3, Oran Area 3, SciLab 3, End Area 2 and 5), its
  homepages and its object comps, 0x14 for the Undernet and Nebula Area,
  0x10 for its first comp (group 0x80). Oran Area, SciLab and End Area's
  own groups (0x91-0x93) are in no list: a warp into them stops the music
  (song 0x63, as in BN6), and how BN5's own ways between its areas keep a
  theme playing there is not traced; End Area's maps that are listed (2
  and 5) play 0x13, its theme here. The rest of its music its scripts and
  code start (direct calls play 0x1A, 0x1B, 0x1D, 0x1E, 0x22 and 0x23; its
  Liberation Missions' among them, not traced).
- [x] Its battle music: its random battles' theme and its bosses' (songs
  0x15 and 0x16, named in its battle records as BN6's are), copied after
  its areas' themes and named in the records of battles on its areas'
  layers, guardians and ProtoMan's netbattle with the boss theme.
- [x] Its battle backgrounds: each area's own (ACDC Area's, Oran Area's,
  SciLab's, End Area's, its Undernet's, Nebula Area's), its record,
  tiles, map, palette and animations copied after BN6's 22 backgrounds,
  in copies of BN6's tables the game's loader is pointed at
  (`src/gfx/xbackdrop.c`), for the battles fought in BN6's engine there
  (guardians, ProtoMan's netbattle); a guest battle's record copy names
  the area's own by BN5's number (its byte +4, which BN5's records leave
  at 0xFF, the map's: the room the guest's boot state stands in, yellow
  rings in every area).
- [ ] Its towns' music. Its lists name ACDC Town's (group 0x00): the
  town 0x03, its houses (maps 1-6) 0x04, map 7 0x05, maps 8-11 0x0C;
  group 0x01's maps 0x05 and 0x0C; in one story state (its third list)
  0x0C throughout both. Its other real-world groups play what their
  scripts start. Each of its music songs (0x01-0x24, 0x0A empty) is one
  xsong.c can copy (sequences of 368 to 8324 bytes, no MEMACC, all on
  voice group `0x081547A8`); waiting on a town of its own for the run.
- [x] Its bystanders on its areas' layers: its purple HeelNavi, sprite
  and face, copied into BN6's free space and listed at a number Gregar
  leaves empty in both (`src/layer/xnavi.c`, docs/ROM_DATA.md); it stands,
  turns and speaks as BN6's own bystanders do. On ACDC Area's and End
  Area's layers its orange Navi takes turns with it (`xnavi2`), as both
  stand in those areas in BN5 (Nebula Area's are HeelNavis alone). Its
  Net Dealers are its green NormalNavi, the art of BN6's own Net Dealer
  already (and in Nebula Area a HeelNavi, left out so the dealer stands
  apart from the bystanders); its Mr. Progs are BN6's. Three free numbers
  are left.
- [x] Its map objects in the place of BN6's on its areas' layers
  (`NetAreaDef.xlooks`, `src/layer/xnavi.c`, docs/ROM_DATA.md): its
  Security Cube, the green cube with X eyes that bars the way to a
  friend's homepage in its ACDC Area until MegaMan has that friend's
  P-Code, stands for the security cubes of ACDC Area's and End Area's
  layers (`blockers.c`), and opens as BN5's does; it asks what BN6's
  asks there, a P-Code or a toll. Its wall of dark flames, which bars
  Nebula Area 6 until its flames of darkness' Navis are beaten, stands
  for the pillar of flames on Nebula Area's layers, across the walkway
  either way, and burns out when HeatMan's or ChargeMan's Cross clears
  it. A dark hole, the purple vortex BN5's Nebula Area and Undernet stand
  past a platform's edge, stands past the back rim of a Nebula Area
  layer's best room, where the Graveyard's monument would (about half of
  its layers have such a room), as scenery the layer was made without:
  a run continued without BN5 has the same layer. Each is copied into
  BN6's free space and listed at a list-7 number Gregar leaves empty, as
  the flame of darkness is; the dark hole stands as one of handler 3's
  objects that never opens (`blockers.c`), as BN6's map-object table has
  no room for an id of the engine's. Its net maps hold few map objects besides: the laser barrier with
  a skull is BN6's cube's own sprite.
- [ ] Its towns. Its real world is groups 0x00-0x05: ACDC Town (0x00:0,
  132 x 72 tiles of 256 colours: Lan's, Mayl's and Dex's houses,
  Higsby's shop, the park with the squirrel statue, the school), Oran
  Isle (0x02:0, beach, mine and dock), the harbour (0x03:0, a raised
  quay and the ship), a castle town on stone terraces (0x04:0) and a
  base's room (0x05:0). ACDC Town is the place BN6's own ACDC Town style
  already is (BN6's prologue map, docs/OVERWORLD.md). Its version would
  take: its tile set and 256 colours copied into a free range of their
  own (the town is built while the first layer's tiles may be another
  game's, at `XGFX_AT`), BN6's interface colours (208-255) kept; the map
  copied whole with its own walls and heights, as Seaside Town is (to be
  cut and stretched, its materials need a colour table of their own);
  its jack-in points, where Lan steps out and the checks' lines for its
  buildings. Oran Isle, the harbour and the castle town would come as
  Seaside and Green Town do, copied whole.
- [x] Its random battles in its own engine, on a second core, in the
  areas it dresses (Guest battles, above).
- [ ] Its guardians, in battles in its own engine.
- [ ] Team ProtoMan (the other version), Battle Network 4, BN6 Falzar.
