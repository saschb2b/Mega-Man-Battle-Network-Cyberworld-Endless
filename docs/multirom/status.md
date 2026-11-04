---
type: Status
title: "Status"
description: "Checklist of what BN5's ROM brings a run: done, its areas dressing BN6's, music, bystanders, battles, chips, Navis and Souls in BN6's engine, the 3DS; open, its towns, Team ProtoMan and Falzar; and BN4 Red Sun's five net areas, dressing BN6's for review, held out of runs."
tags: [multirom, bn5]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-04T11:30:00Z }
---

# Status

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
  L's and the PET's. Read where BN6's ROM is, on every build (the
  browser's from the files its page took, issue #68; the 3DS's where its
  heap has room, Platforms above).
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
  cycles running over BN5's colours (`xbackdrop_map`, [ROM data](../rom/overview.md)).
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
  code start, every one traced ([BN5's maps, songs, text and objects](../rom/bn5-world.md), Where BN5 plays its
  songs); three of them play in a run where BN5 plays them (BN5's music in
  a run, below).
- [x] Its battle music: its random battles' theme and its bosses' (songs
  0x15 and 0x16, named in its battle records as BN6's are), copied after
  its areas' themes and named in the records of battles on its areas'
  layers, guardians and ProtoMan's netbattle with the boss theme.
- [x] Its battle backgrounds: each area's own (ACDC Area's, Oran Area's,
  SciLab's, End Area's, its Undernet's, Nebula Area's), its record,
  tiles, map, palette and animations copied after BN6's 22 backgrounds,
  in copies of BN6's tables the game's loader is pointed at
  (`src/gfx/xbackdrop.c`), for every battle fought there: its random
  battles, its guardian, ProtoMan's netbattle.
- [ ] Its towns' music. Its lists name ACDC Town's (group 0x00): the
  town 0x03, its houses (maps 1-6) 0x04, Higsby's (map 7) 0x05;
  SciLab's (group 0x01) five maps 0x05; in one story state (its third
  list) 0x0C throughout both. Its other real-world
  groups (0x02-0x05, among them the beach and the old mine, 0x02,
  and Nebula's base, 0x05, seen in romlab) are in no list, and entering one stops the music
  (romlab: the BGM's player paused); their maps' scripts start only their
  scenes' songs. Each of its music songs (0x01-0x24, 0x0A empty) is one
  xsong.c can copy (sequences of 368 to 8324 bytes, no MEMACC, all on
  voice group `0x081547A8`); waiting on a town of its own for the run.
- [x] Its bystanders on its areas' layers: its purple HeelNavi, sprite
  and face, copied into BN6's free space and listed at a number Gregar
  leaves empty in both (`src/layer/xnavi.c`, [ROM data](../rom/overview.md)); it stands,
  turns and speaks as BN6's own bystanders do. On ACDC Area's and End
  Area's layers its orange Navi takes turns with it (`xnavi2`), as both
  stand in those areas in BN5 (Nebula Area's are HeelNavis alone). Its
  Net Dealers are its green NormalNavi, the art of BN6's own Net Dealer
  already (and in Nebula Area a HeelNavi, left out so the dealer stands
  apart from the bystanders); its Mr. Progs are BN6's. Three free numbers
  are left.
- [x] Its map objects in the place of BN6's on its areas' layers
  (`NetAreaDef.xlooks`, `src/layer/xnavi.c`, [ROM data](../rom/overview.md)): its
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
  already is (BN6's prologue map, [The overworld](../world/overworld.md)). Its version would
  take: its tile set and 256 colours copied into a free range of their
  own (the town is built while the first layer's tiles may be another
  game's, at `XGFX_AT`), BN6's interface colours (208-255) kept; the map
  copied whole with its own walls and heights, as Seaside Town is (to be
  cut and stretched, its materials need a colour table of their own);
  its jack-in points, where Lan steps out and the checks' lines for its
  buildings. Oran Isle, the harbour and the castle town would come as
  Seaside and Green Town do, copied whole.
- [x] Its random battles in the areas it dresses: in its own engine on a
  second core at first ([Guest battles](guest-battles.md)), in BN6's engine since D1
  (One net: native territories); the second core left runs in the last
  round, the differential's reference alone.
- [x] Its viruses and BN6's meet: from act 2 one battle in four takes a
  visitor of the other net, and the deep's acts draw on both nets' late
  areas (One net: D1).
- [x] Its chips in runs: half the finds and stock on its areas and in the
  deep, its viruses' own rewards, the Library's count and the second
  screen's list (One net: D1).
- [x] Its guardians, in BN6's engine: KnightMan, ShadowMan, TomahawkMan,
  NumberMan, ToadMan and Colonel guard the acts of the areas BN5 sets
  them in, in half the runs, and share the deep's acts with BN6's Navis
  by band (One net, the last round; [Guardians and super bosses](../game/bosses/overview.md)).
- [x] Their Souls: beaten, each gives his for every battle of the run,
  UNITE on BN6's Custom screen by BN5's rule (Transplanted Souls;
  [The meta layer](../game/meta/overview.md)).
- [x] Its DarkChips in runs: all twelve kinds one chip each, BN5's flame
  on its acts, one price in either net (One net, the last round).
- [x] Its chips in BN6's own chip tables ([The chip registry](chip-registry.md)):
  257 of them registered by id, those BN6 has by name BN6's, 138 with
  ids of their own and BN5's records, names, descriptions and art in
  BN6's tables grown to 512, placeholders where its ROM is not read.
  AirHoc, WideSht1-3, Spreader and Geddon1-3 play in BN6's battles by
  BN6's code, those of The chips' code by BN5's own code moved; the rest
  sit out until theirs is. Those that play come into runs (One net: D1);
  a chip that sits out never enters a pool.
- [x] Its chips judged against BN5's own ([The chips' code](chips-code.md)): each
  used in both games and compared frame by frame; BN6's own and eight of
  C1's twelve the same as BN5's, DrilArm1-3 and Z Saver BN5's own code
  since (decided).
- [ ] Its chips' code in BN6's engine: the chips of The chips' code
  moved and proven (Moved); the rest wait, each for what The chips' code
  names (a found difference to trace, the mood port), with its Program
  Advances and DarkChips.
- [x] Its Navis in BN6's own engine ([Transplanted Navis](navis.md)): Team
  Colonel's six and the Dark Soul moved, each proved against BN5's
  battle frame by frame (ToadMan's SP with BN5's sea's submerge, made
  again in BN6's engine; the Dark Soul with BN5's chip AI, its lists
  empty in BN6 for want of BN5's record of the chips MegaMan uses);
  fought where a battle names their ids, and as the run's guardians
  since the last round.
- [x] On the New 3DS too (Platforms, above): BN5's ROM read whole beside
  BN6's where the heap has room, in the 16 MB mGBA's 3DS setup had held
  unused; its layers' learned maps kept for the layer's own area alone
  there ([The 3DS's memory](../engine/emulation/3ds-memory.md); `tools/heap3ds.py`). Built
  and modelled on a computer; a New 3DS's own run with both ROMs is
  3ds/README.md's checklist.
- [ ] Battle Network 4: Red Sun: researched, its tables found and its
  maps drawn, Billy moved into BN6's engine, and everything a player
  needs to bring its ROM built on every platform behind its research
  switch ([How it is built](how-it-is-built.md), Platforms); nothing
  offered to players yet ([Battle Network 4 in a run](bn4.md), its plan
  and the owner's open calls); its areas below, held.
- [ ] Team ProtoMan (the other version), BN4 Blue Moon, BN6 Falzar.

## BN4 Red Sun's areas

Its net's five large areas dress BN6's as BN5's do, each held out of runs
(`NetAreaDef.held`) until the owner admits it, as the walk's gate of
[the core loop](../game/core-loop.md) asks of every area, and how three
games share BN6's areas is the owner's call ([Battle Network 4 in a run](bn4.md),
The owner's calls). Their tables in [BN4 Red Sun's tables](../rom/bn4.md).

- [x] Read for review alone: BN4's ROM is read for research
  (`XRomLayout.research`), and its areas are shown only to a dev tool that
  reviews them (`xarea_review`): `--net-biome x15` (ACDC Area) to `x19`
  (the Undernet), numbered after BN5's fifteen, and the atlas
  (`--atlas DIR:a15:3`, `build.py tiles`). A run, or a dev tool reading BN4
  for its families or the differential, meets none of them: no song,
  backdrop or bystander of theirs copied in, frames the same as before
  (autopilot runs from seeds 1 and 3, BN5's ACDC, Oran and Undernet layers,
  a run with `--dev bn4families`, each frame for frame against the build
  before them).
- [x] Their maps in 256 colours: BN4's net maps are 256-colour where BN6's
  and BN5's are 16, and no packing of their tiles fits BN6's banks, so a
  layer in BN4's tiles is shown in 256 colours: the map reader draws a map
  by its tile map's header (`area_src.c`), and while BN6 shows the net's
  map the director sets BG1 and BG2's 256-colour bit (`netmap_colours`;
  [BN4 Red Sun's tables](../rom/bn4.md), Net maps in 256 colours; [The net's
  maps and their pieces](../rom/net-maps.md), the map's BG controls). BN6's chat box,
  its tiles above BN4's and its colours apart from theirs, shows as on any layer.
- [x] Their backdrops and battle backgrounds: each layer stands on its
  learned map's backdrop, animated, and its battles on the map's own
  background (BN4's map backdrops' byte: ACDC Area 4, Town Area 5, Park
  Area 6, the countries' third map 0x14, the Undernet 8), BN4's 9-word
  records copied as BN6's 7 (`XRomLayout.bg_words`) and its LoadBGAnim
  found as its EnterMapGroup's fifth call (`src/gfx/xbackdrop.c`).
- [x] Each tuned over the atlas's layers (`--atlas DIR:aN:3`), its looks
  learned by shape (BN4's floors share hues across looks: the hue buckets
  part few of them), then polished against BN4's own maps drawn beside
  them (`--atlas DIR:x1`). What its first tuning left, area by area: two
  walkway looks laid tile by tile in one (ACDC Area's cyan roads among its
  white paths), pieces of hubs strewn on walkways and room corners (red
  studs, blue and yellow arcs), the walkways' middles in the platforms'
  (the countries' green blotches in their cream), a lattice's hole rims
  laid over every field (Town Area, a tile drawn off for each of its
  panels and more), cushions cut and joints at random along the Undernet's
  paths and its rooms' middles a scatter of cross pieces. Seams per 100
  panels and tiles drawn off (`build.py tiles`' magenta), before and
  after:
  ACDC Area (x15, like Central Area, over 0x90:0) 144 to 106, off 0 to 5:
  its third map, then the first two; white walkways in green rims, rounded
  at their turns; its framed squares of four by four are its pads
  (`TILES_SQUARE_PADS`, islands of 9 to 16 panels), so every room up to
  64 panels and the arena are drawn as one, framed with its arrows; its
  cyan roads, a second walkway look, and its hubs, ringed in azure, lend
  no tiles (`SKIP_ANY_PIXEL`, hue bucket 6).
  Town Area (x16, like Seaside Area, over 0x91:1) 28 to 153, off 168 to
  25: its maps lay no floor wider than a walkway but Town Area 3's
  blue-framed squares (Town Area 1 is a lattice of walkways round holes,
  whose hole rims had filled every field), so it learns Town Area 3 and 1,
  its squares its pads, every room drawn as one and the way kept one wide
  (`TILES_ONE_WIDE`, as End Area's); Town Area 3's yellow roads and lower
  blue paths lend no tiles (hue buckets 2, 3 and 7 in their middles). The
  seams are where the squares meet the walkways, the picture cleaner.
  Park Area (x17, like Green Area, over 0x92:1) 85 to 81: its first map
  alone (its other maps mixed in, 149), pink walkways, peach framed
  platforms, the pale pink walkway floor however wide (`TILES_WALK_HUE`);
  its orange and yellow conveyors lend none.
  The countries (x18, like Sky Area, over 0x94:1) 32 to 40: the four are
  four looks, so one is learned, the third (green floor in red and yellow
  rims, cream framed platforms, drawn apart; the others 50 to 175); the
  green floor walkway however wide (`TILES_WALK_HUE`: its wide stretches'
  middles had stood in the platforms' as green blotches), its cyan arrows
  lending none (hue bucket 6, their specks gone); which country it is is
  not traced, so the area is named the countries.
  The Undernet (x19, like the Undernet, over 0x92:0) 197 to 185: the
  Undernet 5, then 3; orange cushions a panel each on paths a panel wide,
  grey plates framed in steel with orange corners, drawn apart; its steel
  joints, grey among the cushions, in a look of their own at a path's
  ends, turns and crossings (`joint_hues` `JOINT_GREY`, as BN6's Undernet's
  gems), so a straight path is whole cushions; its plates plain as the
  Undernet 5 lays them, and the Undernet 3's blue cross its emblem, set
  whole in the rooms (`emblem` 0x75C0, learned from the map that draws it,
  `netmap_learn.c`); Black Earth (0x94:6-7) not drawn.
  BN6's and BN5's layers are the same as before, byte for byte (the
  atlas's pictures of all 244, `build.py tiles`).
- [ ] What the dressing still misses, which the generator's layouts
  cannot give from BN4's maps alone: BN4's net is one-wide paths between
  small pieces (hubs, studs, squares of four by four), where BN6's
  layouts lay fields and widened ways, so a generated room's edges and a
  walkway's join to it are neighbourhoods BN4 never draws (ACDC Area's and
  Town Area's seams, Park Area's cut joins); ACDC Area's round studs at its
  paths' ends, its ringed hubs and its exit arrows, Town Area's and the
  countries' arrow conveyors, the Undernet's flames and red paths are not
  drawn; a generated pad is three panels a side, BN4's squares four. What
  would draw them: a layout kit of BN4's own, as `netmap_kit` gives BN6's
  areas theirs (one-wide ways ending in studs, rooms of its squares'
  size, hubs on spurs, its exit arrows at the layer's ends), and the hubs,
  studs and squares cut whole as pads are (`props_learn_pad` takes
  islands of three by three); the owner's eye on the pictures first.
- [x] Their themes named: BN4's net theme (song 0x12) for ACDC, Town,
  Park Area and the countries, its Undernet's (0x13) for the Undernet
  (`.xsong`, BN4's map lists, `BN4_MAP_MUSIC`); copied by `xsong.c` as a
  layer of one is reviewed, its one drum kit whole (219 KB where a BN5
  theme is 8: copying the keys played is the music's work, [Music](music.md));
  its battle themes (0x14, 0x15) named once an area is admitted, as a
  game that lends areas has its battle themes copied as a run starts.
- [ ] Their battles, guardians and bystanders: BN6's (no `xbattles`,
  `xguard` or `xnavi`), until BN4's families, Navis and Navis' sprites come
  (Phases 1 and 2 of [Battle Network 4 in a run](bn4.md)).
- [ ] Admitted: each area's walk measured against the core loop's gate
  (its review, Phase 3) and the owner's call on sharing BN6's areas
  between three games; `run_dress` gives each area of another game a coin
  of the run's seed by its number (bits 16 on), which BN4's numbers (15-19)
  run past, so admitting them asks a draw of its own.
