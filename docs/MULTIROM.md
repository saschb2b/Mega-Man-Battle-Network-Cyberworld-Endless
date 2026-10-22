# Other games' ROMs

Cyberworld Endless runs Battle Network 6: Cybeast Gregar in mGBA and
directs it. Where a player has other Battle Network games' ROMs beside
BN6's, they can lend a run what their data holds: more net areas, towns,
music, bystanders. BN6 stays the one ROM a run needs; each other is
optional, and the more there are, the bigger the pools a run draws from.

## Data, not code

Every battle of a run is fought in BN6's own engine; the engine directs
it and writes data into BN6's in-memory ROM (docs/EMULATION.md). Battle
Network 4, 5 and 6 share one engine family: their maps, tile sets,
palettes, coordinate data, sprites, text scripts, chip tables and MP2K
songs are the same formats or close to them. A second game's data can be
drawn and played by BN6; its code cannot run inside BN6 as it stands (it
calls its own game's addresses), but a virus family's, a Navi's, a
chip's or a Soul's can once it is moved: copied into BN6's free space,
its calls aimed at BN6's routines (Transplanted families, Navis and
Souls, The chips' code, below), each proved against its own game's
battle frame by frame.

| Pool | How another game fills it | Cost |
| --- | --- | --- |
| Net areas | Its maps learned as BN6's are (`tiles_learn.c`), its tile set and palette copied into BN6's free space | moderate |
| Towns | The same, from its real-world maps | moderate |
| Music, sounds | Its MP2K songs and their voice groups into BN6's song table | low |
| NPCs, mugshots | Its sprites as bystanders and dealers | low to moderate |
| Chips | Where BN6 has the behaviour (most standard chips); its own moved into BN6's engine one by one (The chips' code, below), registered in BN6's chip tables (The chip registry); its twelve DarkChips one chip each, BN6's five with BN6's code, its own seven with BN5's rule (docs/META.md, DarkChips) | limited; a unit per chip moved |
| Viruses | The families both games have, BN6's; its own moved whole into BN6's engine, their code with their calls aimed at BN6's (Transplanted families: every family its areas' records set) | a recipe per family moved |
| Guardians | Its Navis moved into BN6's engine as its families are (Transplanted Navis: Team Colonel's six and the Dark Soul) | a recipe per Navi moved |
| Souls | Moved with their look, abilities and passives onto a form BN6 keeps and never offers (Transplanted Souls) | high |

Every battle of a run is BN6's own engine's since issue #82's last
round (One net, below). Before it, what needed BN5's code was fought in
BN5's: a second core running BN5's ROM, BN6's waiting beside it (Guest
battles, below, kept as the history it is). That core stays, for one
use: the differential's reference, BN5's own battle frame by frame
against BN6's with the code moved (`build.py xdiff`, docs/DEVTOOLS.md,
The differential).

## How it is built

1. **Extra ROMs** (`src/core/rom.c`, `XR[]`): found in the ROM's folder by
   their header's game code and SHA-1, read and never run
   (`xrom_find`). Each has its layout (`XRomLayout`): its tables, found by
   their structure (`tools/rom_tables.py`) and noted in docs/ROM_DATA.md.
2. **A map from any of them** (`area_src_load_x`): the map reader takes
   its maps from the game's ROM; what learns from a map (`tiles_learn.c`,
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
   `RomLook` and iOS's `src/core/ios.m`, behind the ROMs screen's
   pickers (`src/launcher/`), copy BN5's ROM beside BN6's from the folder
   the player chose, and look in it again at each start, so BN5 put there
   later comes in (issue #66). On the desktops, Android and iOS, R on the
   title opens the ROMs screen again to add BN5 (issue #97). The browser's
   play page (`web/play/app.js`) takes BN5's file beside BN6's, either
   told by its header's game code and checked by its SHA-1, and keeps it
   in IndexedDB beside BN6's (`rom/bn5c.gba`), where `xrom_find_beside`
   reads it (issue #68). The New 3DS reads BN5's file whole as well,
   beside BN6's or in the 3DS's other ROM folders (`src/core/start_3ds.c`),
   where its 96 MB heap has room for it: mGBA's 3DS setup had taken a
   16 MB ROM buffer it never used, which the game now gives back, so with
   BN5's 8 MB read the heap's fixed part is 8 MB smaller than the 3DS's
   with BN6 alone was (docs/EMULATION.md, The 3DS's memory). A 3DS short
   of room leaves BN5's ROM unread, a run of BN6's, and its log and PET
   say so.

## Guest battles

What runs now: none. Every battle of a run is BN6's engine's, BN5's
areas' too (One net, below), and the guest core (`src/emu/guest.c`) is
the differential's alone: BN5's own battle as BN5 has it, booted once on
the main thread and kept as `xdiff-bn5.state` in the differential's data
folder (`build.py xdiff`, docs/DEVTOOLS.md, The differential). A data
folder a run's guest kept its boot state in (`guest-bn5-1.state`) has it
removed once at the next start (`src/core/startup.c`). What it took a
run, gone with it: BN5's first boot (7210 of its frames: five seconds of
a Retroid Nova's, a thread of its own on native builds, the browser's
frames' spare time), the switch of engines and its flash, the older
net's wait screen, the chips that sat out of its battles for want of a
BN5 namesake and the codes it read its own way, the guest's ROM copy in
the browser's memory (below), and some 2,500 lines of run code.

The history (epic #57, issues #61-#69 and #102, until issue #82's last
round), for what it taught the engine's port:

- **When and which**: where BN5 dressed an area and the build could run a
  second core (every build but the 3DS's), BN6's roll decided a battle
  and a hook gave BN6 none; the director began a guest battle instead,
  BN6's core waiting while BN5's ran. Its record was one of the area's
  own maps' in BN5's numbers (ACDC Area 0x90:0, 0x90:1 and 0x86:0 for an
  act's first, middle and guardian's layers; Oran Area 0x91:0, 0x91:1,
  0x86:1; SciLab 0x92:0, 0x92:1, 0x86:2; End Area 0x93:0, 0x86:3, 0x93:1;
  the Undernet 0x94:0, 0x94:1, 0x86:5; Nebula Area 0x86:6, 0x94:3,
  0x86:7), held to the act's band from BN5's own enemy tables; BN5's
  story battles never. The same maps' records are a territory's now, in
  BN6's engine (`NetAreaDef.xbattles`).
- **MegaMan as the run had him**: his HP, the run's folder as BN5's chips
  of the same name (114 of BN6's 313 paired; a chip BN5 had none of sat
  out, a code its chip lacked fought as its * or first), his buster as
  the NaviCust made it, the run's DarkChips by BN5's worried rule
  (a shelf behind the deck's padding), the run's Souls as BN5's event
  flags so its own Custom screen offered UNITE. Its random numbers were
  the run's: both words written from the battle's seed as it began
  (issue #102; docs/ROM_DATA.md, BN5's random numbers), which the
  differential's reference still does.
- **Its results**: BN5's result, the HP the battle left and its results
  screen's rewards (the busting level's and a green Mystery Data's
  find) went into the run as BN6's chips of their names, zenny and
  BugFrags. BN5's own rows are BN6's now, through the chip registry
  (One net, below).
- **Its guardians and Souls** (issue #69): BN5's Navis guarded the acts
  its areas dress in half the runs, fought in BN5's engine at three
  quarters of BN6's guardians' band (no Cross could come in), never on
  the short net's last act or Nebula Area, whose Souls would have served
  no battle of BN5's after them; their Souls woke only in BN5's battles.
  Since the last round they fight in BN6's engine at BN6's band, and
  their Souls serve every battle (docs/BOSSES.md, BN5's Navis;
  docs/META.md, Souls).
- **Its boot and wait**: BN5's first boot ran in the background from the
  game's start (a thread of its own on native builds, measured 3.4 to 3.6
  s on an Intel Core Ultra X7 358H; the browser's frames' spare time),
  and a battle that came first waited behind the older net's wait, a
  screen designed through the game-design skill's interface layer: the
  switch's white, MegaMan's three lines in BN6's chat box, a gauge in the
  battle HUD's form filling with the boot.
- **In the browser** (issue #68): the page's WebAssembly memory was 97 MB
  on a first layer with BN6 alone, 153 MB on BN5's first layer with the
  guest booted (BN5's ROM, 8 MB, and the guest core's ROM copy, which
  mGBA copies to 32 MB at its first patch), 197 MB at most; without the
  second core a run's page holds BN6's core and BN5's ROM file alone
  (measured: docs/EMULATION.md, The browser's memory).

## One net: the design (issue #82)

What a run meets once BN5's viruses, Navis, chips, Souls and DarkChips
all run in BN6's own engine (Transplanted families, below). Reasoned with
the game-design skill and decided on the owner's delegation (12 October
2026: "finish it completely, then come back"); each call stands until the
owner overturns it. It replaces #57's "territories, no wild mixing",
which two engines forced.

- **The experience:** one Battle Network. The older net's areas keep
  their own viruses and Navis, everything the run carries (its folder,
  its Cross, its Souls, its DarkChips) works in every battle, and where
  the nets meet their viruses fight side by side.
- **The dialectic** stays the meta layer's, what you bring against what
  you find (docs/META.md): one engine lets everything brought work
  everywhere (no chip sits out, no Cross waits at the older net's door),
  and the nets' meeting widens what is found.
- **The loop layers:** the moment (a battle of both nets' viruses, UNITE
  beside CROSS), the session (a layer's battles, some with a visitor), the
  run (Souls and DarkChips carried into every act). The meta loop is
  unchanged: variety, not power.
- **The patterns:** an honest telegraph (each area keeps its identity,
  and a seam is announced), the hand-made units of a procedural run (a
  formation stays its game's own record; a visitor takes one place in
  it), a bonus with a drawback (a Soul costs a chip, darkness costs max
  HP), meta as variety.

The calls:

1. **One engine for every battle.** The older net's areas fight in BN6's
   engine: their battles are BN5's own records, their ids translated,
   scaled to the act's band as now (docs/PROGRESSION.md, BN5's battles).
   The guest core leaves runs: no engine switch, no older net's wait, no
   chip sitting out, no code read as another. It stays as the reference
   the differential compares with (`build.py xdiff`).
2. **Territories keep their identity; the nets meet at their seams.**
   - *Visitors:* from act 2 on, where BN5's ROM is read, one battle in four
     takes one virus of the other net in place of one of its own: on its
     panel, a family whose version keeps the battle in its band, standing
     where the one it replaces stood (the front three columns or the back),
     never a family that battle already holds. BN6's areas meet BN5's
     viruses, BN5's areas BN6's. The run's first battles and the first
     layer after a guardian stay their own net's (the easing rule).
   - *The deep converges:* the Undernet's and the Graveyard's acts and
     the Nest draw their formations from both nets' late areas alike.
   - *The telegraph:* the act card says where the nets meet; MegaMan names
     the first visitor a profile meets, once.
   - *Not chosen:* free mixing everywhere (areas lose their identity and
     the telegraph its truth); territories alone (one engine and nothing
     the player could see of it).
3. **Guardians:** BN5's Navis fight in BN6's engine. Where they stand
   stays (half the runs on the acts BN5 dresses, where BN5 sets them
   roaming); in the deep acts both nets' Navis share one pool by band.
4. **Chips:** every BN5 chip is a chip of the run, registered in BN6's
   tables; a chip both games have by name is one chip, BN6's. They come
   into the pack, the folder, the PET's Library (after BN6's, numbered
   on), the shops and traders of the older net's areas (half their stock),
   and BN5's viruses drop their own rewards. The starting folders stay
   BN6's. A run continued without BN5's ROM keeps its BN5 chips: they sit
   out of battles until the ROM is back, and MegaMan says so.
5. **Souls serve every battle**, not the older net's alone: UNITE on
   BN6's Custom screen, by BN5's rule (the last chip picked, of the Soul's
   kind, given up), once a battle each, for the turns BN5 gives it. One
   form at a time: uniting ends a Cross for the Soul's turns, no unison
   while beasted out and no Beast Out while united. Chaos Unison as BN5
   has it, unpriced (the owner's call of 4 October 2026).
6. **DarkChips: one price, each net's own rule on top.** Every battle a
   DarkChip's dark power ran in costs 20 max HP, in every battle (the
   owner's call B, unchanged). BN6's own burn a BugFrag a use and fall
   back to their base chip without one; BN5's come into the hand only
   while MegaMan worries (BN5's mood, brought into BN6's battles: the
   finer calls below). One of each kind, three in a folder (BN6's rule). The flames:
   BN5's on its acts, offering its kinds; BN6's on its own, offering its
   four.
7. **Nothing reaches the meta loop:** variety, not power, as before.

The finer calls, made once the run's code was mapped (12 October 2026):

- **A family both games have is one family, BN6's** (Mettaur, Catack,
  Champy, WindBox, Trumpy), as a chip both games have is: BN5's records
  name BN6's family there, and the Library keeps one entry per name.
  BN5's six versions (V1, V1EX, V2, V2EX, V3, V3EX) meet BN6's three by
  tier: V1 and V1EX as V1, and so on.
- **BN5's version ladder under the act's cap** by the same tiers: where
  the act allows V1, BN5's records go up to V1EX; V2 to V2EX; V3 to V3EX.
  The guest battles capped BN5's index with BN6's number, so V2 never
  passed V1EX.
- **BN5's guardians take BN6's band.** Their three-quarters discount stood
  for the Cross that could not come into BN5's engine; every Cross comes
  now.
- **Where they guard:** with Souls in every battle, the rules that kept
  BN5's Navis off the short net's last act and off Nebula Area go; the
  Undernet's and the Graveyard's acts share one pool of both nets' Navis;
  the Nest stays the Cybeast's.
- **Visitors:** a family's role is where its own game sets it (the back
  when three of four of its placements are in the last column, the front
  when most are in the first enemy column, else either); random battles
  only (not a Server's, the duel's or the Gentle helper's eased ones);
  none in the deep acts, which mix already; a family its game sets only on
  special fields never visits.
- **The deep's pools:** the Undernet with BN5's Undernet, the Graveyard
  with Nebula Area, the Nest with both; a coin each battle between the two
  nets.
- **Green Mystery Data in BN5's records** by BN5's own rows; **Mystery
  Data in its areas** half BN5's chips, as its shops.
- **A BN5 Navi's Guardian Data** is his own Navi chip, as BN6's guardians
  give theirs: BN5's chips of every kind are in the run's pools now, so the
  key chip of his Soul's kind is no longer needed.
- **DarkPlus** comes as BN5's chip, by BN5's code, where its ROM is read
  (NumberMan's Chaos Unison takes it), and BN5's flame offers it there
  (the owner's call of 13 October 2026); without it, it stays out as now.
- **DarkInvs** (the owner's call of 13 October 2026): BN5's own replaces
  BN6's DarkInvs rule wherever BN5's ROM is read, BN5's code in BN6's
  record 289 as DarkPlus's in 290, the game driving MegaMan; BN6's rule (a
  BugFrag a use, Invisibl without one, BN6's flame's words) only where
  BN5's ROM is absent. The price as decided: 20 max HP a battle its dark
  power ran in, once; one given up for Chaos Unison nothing. Built the
  same day (The chips' code, DarkInvs, moved).
- **A territory's Servers and the duel's race** fight its own records.
- **Repeats** are kept apart in BN5's pools as in BN6's.
- **The chips a run can meet are registered**, no more: BN5's standard
  chips, Megas and DarkChips, Team Colonel's Gigas, the Navi chips of the
  six Navis who guard runs (V1, SP and DS each) and BN5's Program
  Advances whose parts are registered. Team ProtoMan's version chips, the
  Darkloids' and Django's Navi chips (nothing in a run gives them),
  BugCharg (it reads BN5's NaviCust bugs) and BN5's unfinished Phoenix
  and DethPhnx stay out. BN6's chip ids are nine bits, and this is what
  fits.
- **The Library:** BN6's own Library screen keeps its numbering (its
  standard tab is a list of 200 and a byte); a BN5 chip shows there only
  where a tab has room by its own numbers (Mega, Giga, PA), and the run's
  collection counts every registered chip, the second screen's Library
  listing BN5's under the older net. (This amends call 4's "numbered on"
  where BN6's screen cannot.)
- **UNITE and the Beast Out emblem share one place** on the Custom screen
  (BN6's emblem is BN5's UNITE button, the same slot): it is UNITE while
  the last chip picked lights a held Soul, and the Beast Out emblem
  otherwise, where the run has Beast Out. A player who wants the beast
  picks his Soul's chip earlier. The slot's art says which it is.
- **Every Soul fights three turns** (BN5's rule for all six, not
  NumberMan's alone), Chaos Unison one. A Chaos Unison released at the
  wrong moment fails as in BN5: the Dark Soul rises on the enemy's side.
- **BN5's mood comes into BN6's battles** for BN5's DarkChips and UNITE:
  a hit lowers it, a landed hit lifts it, a counter calms him, a battle
  opens as worried as his HP says (the guest battles' rule), and a BN5
  DarkChip used makes him dark for the battle. BN5's DarkChips come into
  the hand while he worries or is dark; UNITE stands only while he does
  not. Dark is BN6's dark too: no Cross, no Beast Out, a weak buster. The
  rise after falling dark, BN5's, does not come with it: BN6's engine has
  its own dark, and that is enough of a price beside the max HP.

The calls D1 made where the design left a number or a rule open (12
October 2026; reasoned with the game-design skill: the dialectic what you
bring against what you find, the session loop's battles and finds, the
patterns of an honest telegraph and the hand-made units of a procedural
run, meta as variety):

- **A family's role is measured over the records runs fight**: BN6's
  areas' random battles (each formation once), BN5's records of the areas
  it lends a run (NetAreaDef.xbattles: 214), not every map of BN5's (its
  battle tables' groups past those maps are not mapped). So measured
  (`build.py pacing` lists them): BN6's Armadill the front, Trumpy,
  WindBox and Nightmare the back; BN5's WuNote the front, MetFire, Shakey,
  Trumpy and WindBox the back; every other family either.
- **A family bound to special panels** is one none of whose placements in
  its game's records stands on a plain or cracked panel (BN5's Marina, all
  on sea; BN6's Nightmare): it never visits. A visitor takes only a plain
  or cracked panel, so none stands on lava or sea it was never set on.
- **The visitor's place and kind**: the replaced foe is a virus on such a
  panel, picked at random among those; the visitor is a family of the
  same role (a back family only for one in the last column, a front one
  only for one in the first enemy column), of the other net alone (a
  family both games have is BN6's and visits neither), one the battle
  does not hold, met in its own net no later than the act (BN6's stages:
  the surface's families before the Undernet's, the Graveyard's last;
  BN5's by the area holding it: its Undernet's at stage 1, Nebula Area's
  at 2), at the highest version up to the act's (BN5's by tier) that
  keeps the battle's HP to the band's top and its hit to the cap. None
  fitting, no visitor: the act's battles are then their own net's.
- **The gate**: from depth 4 (the run's second act) on, one battle of
  each four on a layer (by its battles fought so far), which one of the
  four by the layer's seed, so a reroll and a checkpoint's layer keep it;
  plain random battles alone (an opening battle, a Server's, the duel's,
  and every battle under the Gentle helper never). The deep's acts never
  take a visitor (the Undernet's and the Graveyard's areas, the Nest, the
  dark warp's Undernet layers and trips back to them).
- **The deep takes the place of a territory**: where BN5's Undernet or
  Nebula Area dresses the deep's area, its battles are still the coin's,
  both nets'; without BN5's ROM, or where its area there could not be
  fought in BN6's engine, BN6's alone.
- **The older net's half of the finds is its catalogue**: every chip of
  the registry that plays in BN6's battles (BN5's own ids, and those BN6
  has by name at BN6's id: a chip both nets have comes in either half),
  of the tier rolled, BN6's of that tier where BN5's has none; never a
  DarkChip (flames bring them) nor a Program Advance. Half the time on the
  older net's areas and in the deep, where BN5's ROM is read: the Net
  Dealers' stock, the Mystery Data, the vaults', Servers' and the gift's
  picks (all of roll_chip); a Chip or BugFrag Trader's list half each
  rarity's, BN6's pool's records fitted as before and as many of BN5's
  catalogue's of each rarity. Home's requests pay in BN6's chips. As
  BN5's chips' code moves in (Not yet, The chip registry), its own come
  more often: today a dozen of its own play.
- **The Library**: the profile keeps ids to 511 (Profile.library_x past
  BN6's 320); the classes' marks (STD, MEGA, GIGA COMP) stay BN6's, so no
  mark needs BN5's ROM; the older net's own chips are counted apart
  ("Older Net": those held of those a run can find now) and listed on the
  second screen's Library, under its counts where it has room (a tab
  with no card, a larger display); the collection (a vault's count, new
  this run) counts every chip a run can hold of either net.
- **BN5's viruses' rewards and green Mystery Data** come through the
  registry: its id for the chip where it plays, the stand-in's 200 zenny
  where it sits out.
- **Words**: the act's card says "Older Net" in a territory, "Both Nets"
  in the deep, "Nets meet" in a BN6 area from act 2 where visitors can
  come (none under the Gentle helper); MegaMan names the first native
  territory, the first visitor and the first deep act once a profile
  (Profile.native_taught, visitor_taught, deep_taught); L's briefing names
  the older net's viruses or both nets'. (Until the last round a
  territory's guardian of BN5's fought on the guest core, and the words
  said so; a profile told that hears once that its Cross and its Navis
  are ours now: Profile.navis_ours_told.)

The calls the last round made (12 October 2026, the run's side of issue
#82: BN5's guardians, Souls in every battle, DarkChips in runs, the
guest core gone from runs; reasoned with the game-design skill: the
dialectic what you bring against what you find, the session loop's
guardian and finds, the run loop's Souls and DarkChips carried, the
patterns of parity across the nets, an honest telegraph, costed power):

- **A BN5 guardian's Guardian Data** gives his own Navi chip, BN5's V1 of
  him in the first three acts and his SP from the fourth on (and on later
  cycles), as BN6's guardians give V1 there and EX or SP after: BN5 has
  no EX between them, and its SP stands where BN6's EX and SP do. Where
  his SP sits out of BN6's battles (its code not moved yet: The chip
  registry), his V1, so the data never gives a chip that cannot play;
  never his DS (BN5's dark gate, a chip of the darkness the run prices
  apart). His reward rows' Navi chips pay an Unlocker's price, as BN6's
  guardians' do.
- **His HP**: his own at the version his act's band takes, capped at
  its top as he spawns (BN6_SPAWN_HP), the Net's clock's tenth a notch on
  the capped HP, BN6's guardians' band (no three-quarters discount: the
  Cross comes into every battle now).
- **The deep's pool by band**: in the Undernet's and the Graveyard's acts
  (and the short net's dark way), a coin of the seed and the act weighed
  by how many Navis of each net fit the act's band, each as likely: one of
  BN5's six where it lands on his, else BN6's own pick, which keeps its
  area's own Navis favoured (run.boss_order). None that already guards a
  surface act of the run. Everywhere else as before: half the runs on the
  acts BN5's areas dress, by the Navi BN5 sets roaming there; the short
  net's last act and Nebula Area allowed now; the Nest the Cybeast's.
- **UNITE on the second screen**: the Custom screen's emblem place reads
  UNITE where the last chip picked lights a held Soul, its card naming
  the Soul, its turns (three, Chaos Unison one), the chip it takes and
  that a Soul and a Cross are not worn together; grey, what lights it;
  ColonelSoul's chip button its own card; MegaMan's form line names the
  Soul he is united with and its turns left. The PET's plain words, as
  its other cards.
- **The flames**: BN5's flame (its sprite, turned purple) on the acts
  BN5's areas dress, where its DarkChips play in BN6's battles, offering
  every kind (DarkPlus where BN5's code plays it: the owner's call of 13
  October 2026); BN6's own elsewhere, offering its four, never DarkPlus
  (BN6's US DarkPlus does nothing). A kind is one chip whichever flame
  gives it, and its words follow the rule it plays by: BN6's four by
  BN6's (a BugFrag a use), BN5's own seven and DarkPlus by BN5's (they
  come while MegaMan worries; used, he is dark for the battle; DarkPlus's
  words say what it does, 50 more for the next attack chip).
- **One price, counted as each net counts a use**: 20 max HP a battle a
  dark power ran in, once a battle however many ran: counted where BN6
  runs a chip's after-effects (`0x0800B79A`, from every chip use,
  `0x080128EA`), for every DarkChip of the registry, either net's
  (`dark_chip_is`): BN6's where its dark power ran (a BugFrag paid; its
  base chip's id reaches it where it fell back), BN5's by its rule. A
  DarkChip given up for a Chaos Unison never enters the hand, and the
  Soul's DarkChip attack goes through the chip writer alone, never that
  routine, so it costs nothing (the owner's call of 4 October 2026).
  Until the last integration round BN5's were counted off the hand
  (BN6_HAND_AT moved past one), the same uses by another road; one hook
  counts both nets now. Checked in play (`tools/play.py`, `--net-biome
  x0`, `--dev worried,god`): DrkSonic used costs 100 to 80 after its
  battle; DrkSonic and DrkSword in one battle 100 to 80 once, a BugFrag
  paid for DrkSword; ShadowSoul's Chaos Unison with DarkInvs given up and
  its DarkInvs fired leaves 100 of 100 and the BugFrags as they were. The autopilot picks no DarkChip of
  either net and never presses A on UNITE or the emblem.
- **Without BN5's ROM**: its chips sit out and its Souls sleep; MegaMan
  says both once after a CONTINUE ("I can't feel the old Souls,Lan...").

What to watch, and measure (docs/META.md, Measuring it): a visitor whose
pairing the band does not see (two that pin a row between them); the
strongest Soul for a folder taken every battle now that it serves every
battle (each still costs a chip and comes once); a Soul and a Cross read
as one thing; DarkChips' uses and the max HP they took, per net.

**Its groundwork (M0, in place; nothing a player meets changes yet):** a
record of BN5's becomes an Encounter of BN6's (`src/core/xbattles.c`): its
foes by BN6's ids (a family both games have by name BN6's, at the tier of
its version; a family moved into the engine its installed ids), its field
as BN6's (`src/emu/xfields.c`: BN5's layouts written over fields of BN6's
no battle names, their panels' types as BN6's; docs/ROM_DATA.md,
Battlefields and their panels' types), its green Mystery Data by BN5's
rule with BN5's finds as BN6's, its rocks; a moved family's reward rows
are BN5's own, as BN6's (`src/emu/xvirus.c`); and a territory's random
battle can be rolled from its records as BN6's are from its areas'
(`xterritory_encounter`: the act's band with BN6's easing, BN5's versions
by tier, BN6's repeat rule, the run's random numbers), which
`make_encounter` takes where `loot_territory` names the area, set by
nothing yet (M3). BN5's text reads in BN6's codes (docs/ROM_DATA.md,
BN5's text). `build.py xrecords` lists what each territory can fight so
far: 53 of its 214 random records with CanGard and Handi moved in and
lava and sea as BN6's panels (ACDC Area 21 of 34, SciLab 26 of 36,
Nebula Area 3, Oran Area 2, the Undernet 1, End Area none; 44 with
CanGard alone, before Handi and the lava); the xrecord step fights one
(docs/DEVTOOLS.md). Not fought yet: its Navis (M2).

**Native territories, chips in runs, visitors and the deep (D1, in place,
issue #82):** a territory's random battles are its own records fought in
BN6's engine where every one of its records can be (all 214 can, with
BN5's ROM read and its families moved in): `loot_layer_nets` sets the
layer's territory, `make_encounter` rolls it (`xterritory_encounter`),
BN6's step counter starts it, and its results, HP, GAME OVER, run log
(`x` and the record's address before its foes) and second screen are
BN6's. Where a territory is not native (an install that failed) its
layers fight the BN6 area's own battles (until the last round, the guest
core's). A territory's Servers and the duel's race are its records too;
`loot_families_here`, the act's viruses' weaknesses and the dealers'
counter element read its records. Visitors (`src/core/visitors.c`, their
families' roles from `src/core/visitor_families.c`) and the deep's coin
(`nets_battle_area`) as the calls above; the run log marks a visitor `+`.
The chip pools are per net (`src/core/chip_pool.c`), the older net's half
of finds and stock comes through `loot_pool_pick`, a trader's list of
both nets is written at `EMU_FREE + 0x168000` (docs/EMULATION.md). The
profile's Library holds every id to 511. `build.py pacing` lists the
native territories, the deep's share of the older net's battles and the
visitors (none past the band); `--dev visitors` gives every battle the
rules allow a visitor (docs/DEVTOOLS.md). LAYER_MAKE 92 (the stock rolls
changed).

**The last round: guardians, Souls and DarkChips in runs, the guest
gone (in place, issue #82):**

- *BN5's guardians* (`src/layer/xguardian.c`, `src/director/boss.c`,
  `src/core/run.c`): his battle is BN6's, `xguardian_encounter` his moved
  id at his version where BN6's guardians stand, no running from him,
  the boss theme and the area's background as BN6's guardians'; his HP
  his own held to BN6's guardians' band and raised by the Net's clock as
  he spawns (`emu_battle_hp`); the results' rows his own, his Navi chips
  in them zenny (`xguardian_navi_chip`, encounter.c's lean_row); his
  Guardian Data his Navi chip (above); rival records keyed on navi
  24-29 as before; where he stands by `run_xguardian_at` (above),
  his Navi moved in at every version a run fights him at
  (`xguardian_fightable`) where it was the guest's boot before. The run
  log's guardian line names him (`g28 n36.0 foehp 600`: navi 28, BN6's
  family 36 at version 0, his HP as he spawns). LAYER_MAKE 94.
- *Souls in every battle* (`src/emu/game_hooks.c`): the proxy form and
  the Souls installed with the core wherever BN5's Navis are (BN5's ROM
  read, `--dev nonavis` aside), no dev switch; the run's Souls handed to
  each battle as it begins (`emu_encounter_started`). The second
  screen's UNITE card, button card and form line (`src/second/`); the
  run log's `unite N` (a Soul by BN5's number, `c` its Chaos Unison) on a
  battle's line; the autopilot's `custom_guard`.
- *DarkChips* (`src/director/darkchips.c`, `director_dark.c`): every kind
  one chip (`dark_chip_id`, `dark_chip_kind`: BN6's 286-290 and the
  registry's 214-220), the Pack given each kind that plays, the flames
  and the price above (`dark_after_battle`; the hook in `darkbn6.c`), BN5's
  flame's words rewritten (`ta_dark_flame`; the line on rising after a
  fall, a guest battle's, gone with it).
- *The guest gone from runs*: `director_guest.c`, `guest_words.c` (its
  where-the-nets-meet lines to `nets_words.c`), `guest_reward.c`,
  `guest_wait.c` and its lines deleted; the encounter roll's guest
  branch, its event, the director's state for it, the run log's guest
  lines, the scene's switch and flash, the autopilot's BN5 battle, the
  dev switches `slowboot` and the guest's `gem`, main.c's boot and its
  slices gone; `guest.c` trimmed to the differential's reference. The
  profile's fields for it kept, unused (save.h).

**Lava and sea in BN6's engine (P1, in place):** BN5's lava and sea are
panels BN6 lacks (its 8 is a volcano that erupts, which a step does not
hurt; it has no sea), so the fields with them (the Undernet's and Nebula
Area's 0x73-0x76, End Area's 0x42-0x45) had no BN6 field. They come into
BN6's engine at its free panel types 13 and 14 where BN5's ROM is read
(`src/emu/xpanels.c`; docs/ROM_DATA.md, BN5's lava and sea in BN6's
engine), behaving as BN5's: an object on lava, not of Fire nor spared by
its flags, takes 50 (100 where Fire beats it) and the lava cools plain
under a spark; a Fire object on sea takes 1 HP each 7 frames, as on
poison; an Aqua hit cools lava; each runs out after 960 frames, blinking
its last 60, from a field's start or a chip's making. BN5's code for
them has a twin in BN6 that knows neither: hooks where BN5 tests or
calls for them run BN5's rules in C, and the panel's cooling, which must
call the game's own routines (the panel set, the spark and the random
number its place draws), runs as a few instructions of the project's own
Thumb; BN6's panel flags table grows to 16 words and its element hit's
tests take lava beside the volcano, both in IWRAM's routines, patched in
their image and kept so in RAM (`emu_keep`), out of the states saved.
They look as BN5's: its tiles of them, put in VRAM after BN6's panels'
on a battle's first frame with them (a tile set grown with them would
make every battle, BN6's own too, start later), BN6's tile table grown
to their types; every BG palette is in use, so their colours stand where
BN5 pairs them, lava's in the volcano's place in the panel palettes it
shares with poison, the sea's in the conveyors' beside ice, animated as
BN5 animates them, in a battle that shows them and none of those; the
frames round them are BN6's. BN6's own battles keep every byte of their
state with them brought in (`tools/xfamily.py --spec native`: BN6's
Mettaur with BN5's install and without), but for the IWRAM code the
install patches, the dead stack, and what the Souls do with none held
(installed with BN5's Navis since issue #82's run round: a pointer into
the faces' tables they grow, BN5's mood in their own block, as
`tools/xproxy.py --souls` explains them), where the flags, now read from ROM,
move an interrupt a few cycles. Their records now translate
(`src/emu/xfields.c`: BN6's fields 0x56, 0x5F, 0x61, 0x65 for lava,
0x66, 0x67, 0x69, 0x6A for the sea), held back by their families alone:
Nebula Area's `811E33C` (two Champys and a Mettaur on lava) is
fightable. The differential fights each field in both engines frame by
frame, every panel's type, flags and timer compared (`build.py xdiff`,
`field_lava*`, `field_sea*`), lava under MegaMan, under a Fire and a
non-Fire CanGard, an Aqua hit on it, a Fire CanGard on sea: the same.
What only BN5's chips bring is not compared yet: a panel a chip makes
lava or sea gets BN5's 960 frames in BN6's engine (BN5_SET_PANEL_TYPE
writes 0x3C0 where the panel is one), but no chip of BN6's makes them,
so no battle has made one; BN5's LavaSeed and SeaSeed (chips 43 and 47
by its ROM's names) are assumed by their names to make them, and what
else its chips and Souls do with lava and sea is not looked up yet.

**The sea's submerge and hold (in place):** BN5's sea does two more
things, which BN6's engine dropped with it (`src/emu/xsea.c`;
docs/ROM_DATA.md, BN5's sea's submerge and hold in BN6's engine). An
object whose AI's traits hold 0x20 (ToadMan's SP; ToadSoul sets the same
bit) goes under on sea: hidden, a ripple following it (BN5's marker, the
fourth kind's 0x3E, moved with the Navis' engine objects), and only an
Elec attack or one of the flags 0x1008 reaches it. A non-Aqua object
that comes to a sea panel, neither floating nor going under, is held 20
frames under a splash (BN5's effect 0x63, which BN6 lacks, copied in as
0xAC): MegaMan too. BN5 keeps the submerge in the collision data's flag
0x80000000 and timer at +0x2C, which BN6 uses for its bubble, so it is
kept apart, in 0x90 bytes of the per-battle RAM issue #82's plan gives
it (`RAM_SEA`, docs/EMULATION.md, BN6's RAM): a mask of the submerged by
each collision data's own bit, and each one's marker. BN6's code reaches
it through hooks where BN5's steps stand in its twins (the status step of
MegaMan and the Navis, a collision data made, the arrival on a panel,
two tests of the invisible that BN5 makes of the hidden), and the hit
gate's two tests of an invisible target in IWRAM, patched to call a test
of the project's own and kept so (`emu_keep`). A moved family that goes
under in its own code (one of BN5's sets the flag itself) sets its bit
in the mask instead (`xsea.h`). The differential proves it: ToadMan's SP
under the sea (`toadman_sp`), the buster's shots passing over ToadMan
under (`toadman_sea`) and Elec ones reaching him (`toadman_sea_elec`),
MegaMan and TomahawkMan held as they come to sea (`toadman_sea`,
`tomahawkman_sea`); `--mutations` break the gate both ways, the marker
and the splash. No other sea effect on movement: every test of a panel's
type against the sea in BN5's code is listed in that row.

## Transplanted families

Issue #82: BN5's virus families moved into BN6's own battle engine, so a
battle of BN6's holds them beside BN6's viruses. BN5's and BN6's battle
code is one engine a version apart: a family is its own few routines
(its states, their phases, its hooks) over a closed set of the engine's,
with the same objects and fields in both games, so its code can be copied
and its calls aimed again (`src/emu/xvirus.c`):

- **A recipe of numbers** (`src/emu/xvirus_families.c`): the family's
  blocks of code in its game (each with the CRC32 of its bytes) and where
  they go, the ranges in them that are data (literal pools, jump tables,
  tables of numbers), the routines its calls go to and BN6's in their
  place, the literals that name RAM or ROM outside its code and BN6's
  values for them, the immediates to change (each checked against the
  instruction it sits in), its state table in BN6's layout (BN6's generic
  states 0-7, its own from 8), its hooks, its records, its versions'
  names, its AI index (32 on) and its six enemy ids; and what it draws
  (its sprites by their lists and numbers in its game and in BN6, its
  effects and its attacks' types by their ids in both) and, where its
  game lays struct1 out otherwise, the numbers BN6's layout takes. Nothing
  of the game's is in it: the code, records, sprites and names are copied
  from the player's ROMs as the core starts.
- **The install**, once a core, all or nothing (`xvirus_install_family`):
  every write is staged and made only when every check has passed. Each
  block is copied whole to a fixed place past `EMU_FREE` (docs/
  EMULATION.md, `+0x368000`), the same distance from a word boundary, so
  its loads from its own pools and its calls within itself hold; its
  code is walked instruction by instruction (a load from the pool must
  find one of the recipe's pools, a branch stay in its block, no ARM
  code); each word of its pools and tables that points into a block is
  moved with it, one that names RAM or ROM elsewhere must be one of the
  recipe's; each BL out goes through a 12-byte veneer (`push {r0,r1};
  ldr r0,[pc,#4]; str r0,[sp,#4]; pop {r0,pc}` and the target: every
  register, lr and the flags reach the routine as the BL left them, and
  it returns straight to the moved code), BN6's routines being beyond a
  BL's 4 MiB reach from there (`src/emu/thumb_reloc.c`). Its entries go
  into BN6's tables as they were grown for every family (the engine's
  room, below), each slot checked to hold the fill first: the eleven
  per-family tables, its ice block's size and its held-chip lists' word
  at its AI index; its ids' triples; its names in the families' own
  archive; each word of BN6's changed is read first and must be the one
  expected. What it draws is copied too (a sprite decompressed where its
  game's is LZ77: BN6 draws a compressed battle sprite only once its
  battle's start put it in a cache), listed in the grown copy of its list
  at a number that holds the placeholder sprite there (one of BN6's own
  free numbers, or past the list's end), its effects past BN6's 108 in
  the grown effects; its attacks take a type the grown table leaves
  empty. data.c's readers are told of its ids (`EnemyMoved`), so the run's own
  code (the foes step, the second screen's names, a foe's HP and element)
  reads them as the engine does, not as BN6 has them (Mettaur, a blank
  name). Hooks
  keep AI 32 and up out of BN6's 32-byte delete count and out of its
  rare versions' chat of new virus data (docs/EMULATION.md, Hooks). Every read of the engine's by AI index that a virus reaches was
  looked for (docs/ROM_DATA.md, the ice block's size): the eleven tables,
  the held chips and two more 35-word tables, the delete count and the ice
  block's size.
- **The engine's room** (`src/emu/xvirus_grow.c`; docs/ROM_DATA.md, the
  engine's room for them): every family at fixed numbers, all at once
  (the static model). Once a core, before any family's install and all or
  nothing, each table a family is found through is copied past
  `EMU_FREE` (docs/EMULATION.md, `+0x330000`) with BN6's entries and a
  fill after them, the one word of the ROM that names it pointed at the
  copy: the eleven per-family tables, the ice blocks' sizes and the three
  35-word tables by AI index to 256 AI indexes; sprite lists 1, 2, 4 and
  5, the effects and the first, third and fourth kinds' types to 256
  (three types taken by the second-level dispatch, below: the first
  kind's 0x5C, the third's 0xD2, the fourth's 0x91); the enemy ids'
  triples and their reward rows to 0x400 ids, the ids past BN6's read as
  its filler 0xBB. A family takes an AI index from 32 and six ids from
  0x200, past every id BN6 has (its Navis end at 0x190, the player's
  forms at 0x1C3); the battle reads 12 bits of an id as it spawns one,
  and its sprite preload, which read 9, is made to read as many; the
  names of the ids from 0x200 are an archive of their own, which a hook on
  BN6's name lookup hands out (it took an id past 0xFF for a Navi's).
  BN6's own battles read the copies as BN6's: with the room made they
  are the same at every byte (`tools/xcontrol.py`, `tools/xfamily.py
  --spec native`). Past it the plan goes on (docs/EMULATION.md, Memory
  the engine writes): the chip registry at `+0x400000` (The chip
  registry, below), the Souls at `+0x480000` (Transplanted Souls,
  below), the lava and sea at `+0x4C0000`, room kept for the Navis at
  `+0x500000` (their battle sprites first) and the chips' moved code at
  `+0x600000` (The chips' code, below). The mode byte of a
  family's struct1 indexes BN6's 32 bytes for its families (read in one
  battle mode): kept, a family whose game's mode is past them given 0.
- **The second-level dispatch** (`src/emu/xvirus_dispatch.c`;
  docs/ROM_DATA.md, the second-level dispatch): BN6 runs an object by
  its type, a byte, from its kind's table, and BN5's families spawn
  objects of their own types (CanGard's reticle is BN5's third kind's
  0x25, which BN6 runs as another attack). One type of each kind, dead in
  BN6 (the first kind's 0x5C, the third's 0xD2, the fourth's 0x91), runs
  whatever moved routine its object was spawned as: a moved family's
  calls to its game's spawners go to the dispatch's (one for each of
  BN5's spawners, the same kind and place in the run order), which look
  the type up in the kind's TWIN table and spawn BN6's own number (where
  the object is BN6's too) or else the kind's dispatch type, and write
  the game's number into the kind's side table in RAM by the object's
  slot (`0x0203FF00`, `0x0203FF20`, `0x0203FF40`, 32 bytes each, cleared
  by every battle's setup; docs/EMULATION.md, BN6's RAM);
  each frame the dispatch type's routine reads the number back and goes
  on to the kind's MOVED table's routine for it, as BN5's runner calls
  one. A recipe names its types (`XvType`: the kind, its game's number,
  BN6's twin or moved) and its moved objects' routines are its own
  blocks; MOVED's entries no family set let a stray object go as it first
  runs. The code is the project's own Thumb, made field by field, past
  `EMU_FREE` with the grown tables (docs/EMULATION.md, `+0x345000`); a
  moved block that reads its object's type is refused, so the moved code
  never sees the dispatch's. BN6's own battles are the same at every byte
  with it made (`tools/xcontrol.py`, `tools/xfamily.py --spec native`).
- **The control** (`--dev xcontrol`): BN6's own Mettaur, AI 1, moved
  through the same steps as AI 32 with ids 0x200-0x205. It must play frame
  for frame as BN6's: `tools/xcontrol.py` fights it and BN6's own under
  the same keys and compares every byte of EWRAM and IWRAM after each
  frame (docs/EMULATION.md, Testing), in two specs: a fight of shots and
  steps to its deletion (its own states 8-12), and the statuses a hit
  gives, one after another (BN6's generic states 4-7: paralysed, dragged,
  frozen in its ice block, bubbled; pushed along), then the fight. In
  both it does, to the battle's results, where the reward comes from each
  id's own drop row (the control's ids take Mettaur's). That is what is proven: seed 1, one Mettaur at one
  panel, the paths those keys reach. Not reached: state 3 (no way in for
  a virus in BN6) and its table-9 hook, a real chip's hit (the status step
  sets what a hit leaves; the hit's resolution before it is BN6's own
  code, which reads a family only through the tables the installer grows),
  the statuses that are no state (confusion,
  blindness, holds), and a moved and a native Mettaur compared in one
  battle (seen together by eye only).
- **The status step** (`--input "0:status freeze 90"`, play.py's
  `status freeze 90`): the battle's enemies (or one enemy id's) given
  paralysis, freeze, bubble, drag, slide, confusion or blindness as a hit
  leaves it, for the frames named; BN6's engine acts on it on their next
  frame.
- **The foes step** (`--input "0:foes 200@52/01@43"`, play.py's `foes`): a
  battle at once in BN6's engine with the enemies named by id at their
  panels, the moved families among them.
- **BN5's CanGard** (`xvirus_cangard`, AI 14 in BN5; docs/ROM_DATA.md,
  BN5's CanGard): installed wherever BN5's ROM is read (`--dev
  nofamilies` leaves it out), as AI 32 with ids 0x200-0x205 (CanGard,
  CanGrdEX, CanRaid, CanRadEX, Canlada, CanladEX). Two blocks of BN5's:
  CanGard's own code (its decide, guard and fire states, its per-frame
  hook, its dying wrapper and its release hook) and its attack's, BN5's
  type 0x25, a reticle that walks to MegaMan's panel and strikes there.
  Their 39 calls out go through 39 veneers to 38 of BN6's routines, each
  paired by comparing the two bodies, and to the dispatch's spawner of
  the third kind. Five immediates change, each read first: its two
  states' CurActions (7 and 8 in BN5, 9 and 10 in BN6, whose two status
  states come before a family's own), its muzzle's effect (BN5's 0x57
  copied as BN6's 0x6C), the reticle's sprite (list 4's 0x24 as 0x5C),
  and the collision data's flags it reads (BN5's +0x68, BN6's +0x70). The
  reticle keeps BN5's type: spawned through the dispatch as its third
  kind's type with 0x25 in the side table, and run as BN5's 0x25 (before
  the dispatch its immediate was made BN6's empty 0xD1). Its state
  table is BN6's generic 0-7 but its own dying (2), its own from 8; its
  hooks BN5's per frame, and its release in BN6's tables 6 (paralysed), 7
  (frozen) and 8 (bubbled), which BN5 lacks, and 10 (dragged), as BN6's
  AI 17 gives one hook all four. Its struct1 laid out BN6's way (BN5
  keeps the shadow at +5, where BN6 ORs in an element: copied as it is
  it would be Heat with no shadow); its sprite at list 1's placeholder
  0x1E; no held chips; the smallest ice block (BN5 freezes none).
  `tools/xfamily.py` checks it in BN6's battles (docs/EMULATION.md,
  Testing): it fires at MegaMan beside a Mettaur, its reticle striking
  for its version's damage; freeze, bubble, paralysis and drag each let
  its reticle go, and it fires again; each version is deleted with BN6's
  delete counts and its rare versions' byte untouched (BN6's own
  Mettaur, deleted the same way, changes them); and BN6's own Mettaur
  plays the same frame for frame with CanGard installed. What is not
  checked there: a real chip's hit on it (the status step sets what a hit
  leaves), and its guard against each kind of chip.
- **The differential** (`--xdiff`, `tools/xdiff.py`, `build.py xdiff`;
  docs/DEVTOOLS.md, The differential): CanGard in BN6's engine against
  CanGard in BN5's own, one battle fought in each, frame by frame by each
  battle's timer, every byte of its objects (and MegaMan's, the panels,
  the order they run in, the sounds begun, the RNG words) read in BN6's
  numbers and each structure through its own layout, every difference
  explained or a failure. Its specs (`tests/xdiff`): `cangard_v1`, V1
  alone at (6,2) against MegaMan idle at (2,2) with 100 HP: the same over
  1200 fight frames, and in both what BN5's battle is known to do (its
  states 8, 9 at t3, 8, 10 at t5, a cycle of 129 frames then 97; its
  reticle on (6,2) at t17, a panel on at t23 and every 12 frames to (2,2)
  by t59, sound 0xAE at t76 as it fires; MegaMan 10 HP down at t85, 279,
  473, 667, 861 and 1055, every other strike, the others while he still
  flashes). `cangard_v1_keys`, the same with 62 inputs by the timer (41 taps
  of B, three held to charge, 18 steps between rows and columns) and
  MegaMan at 2000 HP: the same over 1200 frames, CanGard struck 15 times
  and only in its firing state (10), holding its guard (9) each time
  MegaMan stands out of its row. Its five other versions alone, as
  `cangard_v1` (2000 HP; CanladEX 3000): the same over 1200 frames each,
  its reticle striking for 40, 70, 100, 150 and 200 (its own table, BN5's
  `0x0810912E`, not struct2's) and walking a panel in 10 frames, then in
  5 from V3 (V1's 12). `cangard_mettaur`,
  the issue's formation (CanGard at (5,2), a Mettaur at (4,3)), a smoke
  spec: the Mettaur's own AI data differs from t35 (each game runs its own
  Mettaur, its shockwave and its step's effect: the spec's natives), and
  CanGard is compared on while its world stays the same: the same to
  t134, inconclusive at t135, where BN6's shockwave (22 frames a panel
  against BN5's 25) reaches MegaMan's panel three frames before BN5's;
  CanGard's objects stay the same after it to t362. No difference found
  was CanGard's code, its state table, an id, an offset, a call's twin or
  a table's entry: each was the engine's, each explained in
  `tests/xdiff/legit.json` by both games' routines. No mask touches
  CanGard's own records, its reticle's or its effects' in any spec: what
  the engine writes there in its own numbers is read as BN6's, as an
  enemy id is: the collision data's alliance words (BN6's entries of the
  table by alliance and region mostly carry bit 0x80 where BN5's do not,
  so each BN5 word is read as the BN6 entry at its place, and a hit's word
  as the OR of those it is made of), the renderer's tile place and palette
  slot (their rank among the family's own that frame: the counts run over
  everything drawn, MegaMan's blinking sprite among it), an object's slot
  (from its own). On what is not CanGard's:
  each game's panel restore timers (their bytes and values pinned);
  the blink of an object flashing after a hit, on the other frames (since
  the tooling's milestone, below, only where an object's bit follows its
  owner's: MegaMan's buster flashes);
  MegaMan's own sprites and the place of his charge glow; the player's AI
  data +0x0F BN6 sets as the Custom screen closes; BN6's panel
  reservation flag (since the milestone read from BN5's own reservations,
  no mask); BN5's buster's one more random draw; BN6's buster
  sound on a player a hit's sound holds; the hit spark's frame more of
  life. BN5's split of a hit's result (by the hitter's side, +0x6C and
  +0x70) and of its modifier (+0x18, +0x19) is no mask but held to its
  rule, the OR BN5 makes of them, compared with BN6's one word; BN6's own
  bytes of a hit (its multiplier and elements, +0x74-0x7B) must be 0,
  as they are where no element is in play. The comparator can fail: eleven
  breaks of the install as a transplant bug would make them (an
  immediate, a state number and an offset left as BN5's, a call aimed at
  nothing or at its wrong twin, a grown table's entry, a number in its
  data, the muzzle's effect id left as BN5's, alone and beside the
  Mettaur, the strike's effect id, the reticle's collision region) each
  fail their spec, nine of them where every timeline check still holds (`build.py xdiff
  --mutations`); since the dispatch, three more fail `cangard_v1`: the
  reticle's MOVED entry left the stray's, its TWIN entry made BN6's own
  0x25, and its spawn aimed at BN6's spawner as before the dispatch. What it did not reach then (12 October 2026), each
  reached since by the tooling's milestone (below): an extra object the
  subject spawns whose owner names MegaMan's or a native's, a family that
  draws random numbers, and a family whose objects flash or step onto
  panels. What it still does not reach: CanGard deleted, struck by a
  chip, or beside BN5's other families; an element in play; BN6's freeze
  and bubble (BN5 has neither; `tools/xfamily.py` covers them in BN6
  alone).
- **The differential's tooling milestone** (issue #82): each core
  carries hooks of its own (src/emu/hook.c keeps a table and a queue per
  core, found by the CPU that meets the BKPT; the guest's are the
  differential's alone: the run's guest battles, while they lasted, had
  none, their pictures and logs the same as before), on each
  game's own routines: its draws, every spawn of the first, third and
  fourth kinds, its sounds, effects, setAttack, a panel reserved, the
  blink and a sprite set up, each logged with its frame, its caller, the
  family's code that led to it and the outermost caller
  (`src/dev/xdiff_events.c`; docs/DEVTOOLS.md, The differential). The
  comparator compares the family's calls by their call sites and gives an
  object to the family whose code spawned it. With `rng=replay` each of
  BN6's draws is given the word BN5's draw of the same key (the family's
  call site, or the caller's BN5 twin) drew from, so the family draws
  BN5's numbers whatever the engine drew around it; BN6's camera shake,
  which draws from its second word where BN5's draws from its first, is
  given BN5's numbers the same way, and BN5's buster's extra draw is the
  one draw left unmatched, the engine's own (both routines cited in
  `tests/xdiff/legit.json`, rng). BN6's reservation bit and the blink's
  phase are read from BN5's own events into its records as BN6 would have
  written them: translations, not masks. The control runs with the
  replay on and stays the same frame for frame (`tools/xcontrol.py`: each
  of the native run's 8800 and 10783 draws replayed, none unmatched).
- **BN5's Handi** (`xvirus_handi`, AI 4 in BN5; docs/ROM_DATA.md, BN5's
  Handi), the second family, the first that draws random numbers and
  moves: installed after CanGard as AI 33 with ids 0x206-0x20B (Handi,
  HandiEX, Hando, HandoEX, Handum, HandumEX), the static layout's second
  family (its home, code and sprite the second of each). One block of BN5's, its
  decision (a coin for its wait), the engine's wait and its attack: a
  teleport to a panel its callback picks at random (BN5's
  `0x08012DCC`, BN6's `0x08015FDC`, which draws), reserved as it goes,
  then a bomb dropped in front of it (the third kind's type 0x4B, BN6's
  own, an obstacle with HP that counts down and bursts for its version's
  damage, 50 to 200). Three entries of its attack's jump table name the
  engine's own steps (the teleport's start, the teleport, the wait): the
  installer takes them from the recipe's words, BN6's twins (each body
  compared; BN5's teleport also sets the facing again at its end, to the
  0 an enemy spawns with on a plain field, where BN6's leaves it: not
  reached on a field of another column pattern). 14 calls to 11 routines, two
  immediates (its wait's and its attack's CurAction). Its bomb is spawned
  by the bomb's own spawner, BN5's `0x080CF37E` paired with BN6's
  `0x080CF4EC` (BN6's type 0x4B in both), so none of Handi's code calls a
  spawner of the engine's: nothing of Handi's goes through the dispatch. Its specs, all
  with `rng=replay`: `handi_v1`, Handi alone against MegaMan idle: the
  same over 1200 frames, its three teleports (to 4,1, 4,2, 4,3) and its
  four bombs as BN5's battle has them, MegaMan 50 down at t306, t636 and
  t966; `handi_v1_keys`, cangard_v1_keys' 62 inputs: the same, Handi hit
  9 times, its bombs shot down but one, its teleports to other panels;
  its five other versions alone (`handi_ex` to `handi_v3ex`): the same,
  their bombs for 70, 90, 120, 160 and 200. Its breaks fail (`--mutations`):
  its wait's CurAction left as BN5's (t2), its panel's pick rebased to
  the routine after it (t21: it teleports onto MegaMan's side), its draws
  shifted onto the other word (t2: no draw of BN5's to give them).
- **The recipe generator** (`tools/xrecipe.py`; docs/DEVTOOLS.md, The
  recipe generator; docs/ROM_DATA.md, The recipe generator's reads of
  BN5): every BN5 family's recipe derived from the two ROMs, numbers
  only; the installed families' written into `src/emu/xvirus_bn5.c`,
  which it writes and nothing else does, the rest reported
  (`.build/xrecipe/report.md`). Per family, in install order (AI 32 on, six ids each from
  0x200, code, homes and art at the static layout's next places): its
  blocks (a walk from its states and hooks to the next family's struct1),
  their data by how the code reads it, its calls with their twins (the
  project's pairs in `tools/bn5_twins.py`, then the symbol map's, then a
  BN6 routine of the same body, then where its neighbours put it), each
  pair's two bodies compared (the same; the same but for the offsets of
  loads and stores; or differing, served only once audited for what the
  family's code asks of it: docs/ROM_DATA.md, The engine's differing
  twins), the immediates and pool words that are numbers in each game's
  own terms, its states, hooks, records, its held chips as BN6's chip ids
  (a chip BN6 has none of held out), what it draws, and its types for the
  dispatch. What the recipe format has gained for them, each checked by
  the validator and the staged installer: pool words that are numbers
  (`XvNumber`), bytes of its data that are ids (`XvByte`), a routine
  copied once more with a call aimed otherwise (`XvClone`, a status
  wrapper's), its types for the dispatch (`XvType`) and its held chips. A
  family is whole where no decision is open, and only a whole one is
  written; `build.py test` holds the generated CanGard and Handi to the
  installed recipes field for field (they are the same). Since issue
  #82's A2b every family BN5 has and BN6 lacks is whole and installed
  (BN5's families, all of them, below), what each derives in
  `.build/xrecipe/report.md`. Tuby and the unnamed AI 31-33 are left
  out: no battle record of BN5's names them (all 386 of its enemy lists
  read, none with an id from 0xB5 on; AI 31 and 32 have no ids at all).
- **BN5's families, all of them** (issue #82's A2b): the 23 after CanGard
  and Handi, installed after them in BN5's AI order wherever BN5's ROM is
  read, each by its generated recipe and nothing written by hand: AI
  34-56, six ids each from 0x20C to 0x295, their homes, code and art one
  after another (docs/EMULATION.md, the families' room). Per family
  (`.build/xrecipe/report.md` has the rest): its blocks and their bytes,
  its calls to the engine (each through a veneer to BN6's twin), its
  immediates and pool words given BN6's numbers, the object types it
  moved through the second-level dispatch (`shared`: run by the copy of a
  family before it, or of the engine's objects moved once), what it draws (its sprites copied and the effects
  given ids of their own, where BN6 has none of the same bytes), its
  sounds BN6 has no song of (each at its fixed slot), its held chips as
  BN6's ids, and what was made for it of the project's own Thumb:

| Family | BN5 AI | BN6 AI, ids | Code: blocks, bytes | Calls | Immediates | Moved types | Art: sprites, effects | Sounds | Held chips (BN6 ids) | Shims and pre-step |
|---|---|---|---|---|---|---|---|---|---|---|
| CanGard | 14 | 32, 0x200-0x205 | 2, 1432 | 38 | 5 | 3:0x25 | 3, 1 | - | - | - |
| Handi | 4 | 33, 0x206-0x20b | 1, 524 | 11 | 2 | - | 1, 0 | - | - | 1 NameID range |
| Powie | 2 | 34, 0x20c-0x211 | 3, 1464 | 36 | 9 | - | 2, 2 | 0xC0, slot 0x62 | 211 | 1 NameID range |
| Bugtank | 3 | 35, 0x212-0x217 | 3, 898 | 26 | 9 | 3:0x24 | 2, 0 | - | 157, 158, 163 | - |
| Dominerd | 5 | 36, 0x218-0x21d | 3, 1416 | 35 | 8 | - | 1, 0 | - | - | 4 NameID ranges, the pre-step, 2 clones |
| MetFire | 6 | 37, 0x21e-0x223 | 5, 1122 | 35 | 5 | 3:0x56 | 2, 0 | - | 143, 163 | 1 NameID range, 1 id in its data |
| Shakey | 7 | 38, 0x224-0x229 | 4, 1242 | 25 | 5 | - | 1, 0 | - | - | - |
| NinJoy | 9 | 39, 0x22a-0x22f | 3, 1200 | 28 | 6 | - | 2, 1 | - | - | - |
| Lark | 11 | 40, 0x230-0x235 | 2, 824 | 18 | 2 | - | 1, 0 | - | 211, 450, 456, 460 | 1 NameID range, 2 chip compares |
| BomBoy | 12 | 41, 0x236-0x23b | 6, 1562 | 40 | 7 | 3:0x3E | 2, 0 | - | - | - |
| Zomon | 13 | 42, 0x23c-0x241 | 2, 1388 | 23 | 7 | - | 2, 2 | - | 163 | 1 NameID range, 2 ids in its data |
| Drixol | 16 | 43, 0x242-0x247 | 5, 1572 | 37 | 7 | 4:0x78 | 2, 0 | - | - | 1 NameID range |
| Batty | 17 | 44, 0x248-0x24d | 2, 1372 | 43 | 5 | 3:0x6A | 3, 1 | - | - | - |
| Skarab | 18 | 45, 0x24e-0x253 | 2, 1612 | 47 | 11 | 3:0xC1 | 2, 0 | 0xB7, slot 0x61 | 163, 456 | - |
| Draggin | 20 | 46, 0x254-0x259 | 3, 1226 | 40 | 7 | 3:0x87 | 2, 0 | - | 156, 459 | 1 NameID range |
| Whirly | 21 | 47, 0x25a-0x25f | 3, 1822 | 43 | 7 | - | 2, 0 | - | 450 | 1 NameID range, 2 ids in its data |
| Marina | 22 | 48, 0x260-0x265 | 6, 2122 | 52 | 9 | 3:0x0B, 3:0x8F | 3, 1 | - | - | 1 NameID range, 4 ids in its data |
| Flashy | 23 | 49, 0x266-0x26b | 4, 1628 | 45 | 13 | 3:0x6F | 2, 0 | 0xBA, slot 0x60 | - | 1 NameID range |
| Eleogre | 24 | 50, 0x26c-0x271 | 3, 1300 | 25 | 7 | 3:0x6F shared | 1, 0 | - | - | 1 NameID range |
| Cactikil | 25 | 51, 0x272-0x277 | 5, 1698 | 57 | 5 | 1:0x11 | 2, 0 | - | - | - |
| WuNote | 26 | 52, 0x278-0x27d | 6, 2344 | 65 | 11 | 1:0x05 shared (the engine's objects'), 1:0x16, 3:0x84 | 1, 0 | - | - | - |
| Appley | 27 | 53, 0x27e-0x283 | 6, 2218 | 59 | 10 | 1:0x2F, 3:0xAC, 4:0x90 | 3, 1 | - | - | the pre-step |
| BigBrute | 28 | 54, 0x284-0x289 | 3, 1338 | 35 | 7 | 3:0x61 | 1, 2 | - | - | - |
| TinHawk | 29 | 55, 0x28a-0x28f | 4, 2384 | 37 | 11 | - | 2, 1 | - | - | - |
| Bladia | 30 | 56, 0x290-0x295 | 4, 1186 | 29 | 6 | - | 2, 1 | - | - | - |

- **What they asked of BN6's engine, and how it is given** (docs/ROM_DATA.md,
  The recipe generator's reads of BN5, their shims and pre-steps, their
  sounds): a held chip is the chip registry's, BN6's own chip of the
  name or, for a chip BN6 lacks, the registry's id of its own (MetaGel
  450, Lark's and Whirly's; Blinder 456, Lark's and Skarab's; AntiWatr
  459, Draggin's; AntiElec 460, Lark's), written into the six lists of its home, which BN6's
  table of held chips by AI index names; a chip id the code compares past
  an immediate's 255 (Lark's) goes through a shim (`XvCmp`), and one the
  code gives its object by hand (Zomon's AreaGrab, `movs r0,#117` before
  `strh r0,[r5,#0x2A]`) is BN6's in the immediate. A held chip whose code
  is not moved yet still works as a held chip: BN6's engine picks it from
  the list as any virus's, the family's code compares it by its id and
  hands it to the chip writer (BN6's twin, audited: `held_bn6`), which
  writes its numbers from the registry's record, whose family is the
  inert 0x1C and subfamily 3 until the chips track moves its code; the
  virus's setAttack0 then runs BN6's family 0x1C (`0x080ED6DC`), whose
  subfamily 3 (`0x08010488`) spawns an effect in front of a virus
  (AttackVars +0x1B 0), plays sound 0x90 and ends the attack: the virus
  uses it with a puff and goes on with its states (no spec's battle gives
  one: in every spec only Zomon's object holds a chip, its own AreaGrab).
  A family's lookups by a range of NameIDs make their bounds in
  instructions, past an immediate's 255 for BN6's ids: a shim loads them
  (`XvRange`, 12 families). BN5's virus dispatcher steps Dominerd and
  Appley before their state each frame, BN6's for no family's id: a hook
  on BN6's dispatcher runs BN5's step, ported (`XV_PRESTEP`). Dominerd wraps
  a generic state in a routine of its own, which is copied once more for
  each of BN6's two status states (frozen and bubbled), its call aimed at
  BN6's (`XvClone`). The ids in a table of a family's
  data are byte fix-ups (`ID_TABLES`: Zomon's effects, Whirly's sprites,
  Marina's water's records, the fire column's kinds). A sound BN6 has no
  song of is copied once, at the slot issue #82's tracks share for it
  (Powie's 0xC0 at 0x62, Skarab's 0xB7 at 0x61, Flashy's and Eleogre's
  0xBA at 0x60), on the players BN5's own entry names, as the Navis'
  copies are, and noted with `xsong.c` so that the other tracks' installs
  find it there and the differential reads it by BN5's number. A sprite is BN6's own where BN6
  has the same bytes, else copied, decompressed, at the families' numbers
  (lists 1, 4 and 5 upward from BN6's end, list 1's placeholders 0x1E and
  0x00 first; list 2 from 0x30; a sprite of list 3, where it is not
  grown, in list 4), and an effect of other art takes an id from 0x6C. Every object type a family's
  code spawns is decided once for every track (`bn5_twins.TYPES`): moved
  through the dispatch unless BN6's type of the number runs the same code
  and draws the same sprites by their bytes (19 moved, 6 twins, the
  fourth kind's 0x83 unreachable: BN5's panel claims, which BN6's engine
  never asks for, go to the dispatch's stray). Flashy's and Eleogre's
  flash is one type, Flashy's copy run for both. BN5's image object (the
  first kind's 0x05, which WuNote spawns with kind 0x30) is the engine's
  objects' module, moved once for every track before any family (The
  engine's objects moved once, below): WuNote's call to its spawner goes
  to that copy and its type is run as that install's (`XV_TYPE_SHARED`);
  its kind 0x30 is fixed in the one table (`tools/bn5_objects.py`, its art
  BN5's 4:0x21 at the families' list 4 number 0x6C), and a family spawning
  a kind not fixed there is refused. A family's numbers its code keeps as they
  are (an alliance's bits, a collision region's spawn words) are read by
  hand in BN6 first (`KEPT`). BN6 finds a virus's arm points (where a
  status's mark stands over it) by its battle sprite's number in list 1,
  past BN6's records for every family: list 1's records are grown and
  each family's install writes its own, BN5's 30 points and BN6's four
  after them (docs/ROM_DATA.md, their arm points; the chips track found
  CanladEX's marks 10 pixels to one side). The rare versions (4 and 5) reach BN6's
  rare path only behind its flag, as BN6's own do, and their deletes
  leave its byte alone (`tools/xfamily.py`). Their reward rows are BN5's,
  each chip as BN6's of its name (else 200 zenny, until BN5's chips are
  BN6's too).
- **Each fought in both engines**: seven specs a family (`tests/xdiff`:
  V1 alone where BN5's own records set it against MegaMan idle, V1
  against cangard_v1_keys' 62 inputs, and each other version alone), all
  with `rng=replay` and `"sprites": "bytes"`: all 175 the same over
  every fight frame, among them every one of CanGard's and Handi's as
  before, and Zomon's seven since its chip's family keeps BN5's timing
  (below). What the families showed the
  comparator of the engine's own, each in `tests/xdiff/legit.json` with
  both games' routines: the hit spark a family's attack strikes with and
  the effect a twin routine of the engine's spawns are each game's art
  (their kind, place, owner and id compared, their frames not), MegaMan's
  status effect is BN5's 2 pixels higher and drawn from each game's own
  confusion picture, BN6's Elec damage of a hit, a status's saved
  CurAction (+0x5D), and one routine of BN6's that fixes a bug of BN5's a
  family can see, BN5's own copied in its place (the collision data's
  update, Marina's). Their breaks (`--mutations`): for each of the 23 an
  immediate (the CurAction it gives setAttack0 or compares, left as
  BN5's) and a rebased call (its veneer to setAttack0 aimed at the
  families' empty hook), each failing its V1 spec. `tools/xfamily.py
  --spec families` holds each one's V1 to its record and states, its
  four statuses taken and let go with what it held, its confusion's and
  blindness's marks over it at BN5's arm point, and each of its six
  versions deleted with BN6's delete counts and the rare path's byte
  untouched.
- **Zomon** failed all seven from the frame after its first AreaGrab
  (V1 t391, EX t363, V1's keys t410, V2 t344, V2EX t325, V3 t307, V3EX
  t280), every byte the same before it, until issue #82's final engine
  round. Its set-up gives its object AreaGrab by hand, BN6's own chip of
  the name (the registry's 117 as 163), which BN6's own chip code runs:
  both games spawn the time freeze at the same frame, take MegaMan's
  front column and play its sounds at the next, but BN6's family of
  time-freeze chips (0x15, `0x080ED0DC`) ends its user's attack a frame
  later than BN5's (`0x080EC400`, which spawns and ends in one frame:
  `bl 0x0800F29E` at `0x080EC448`; BN6's marks its AttackVars +1 at
  `0x080ED12A`-`0x080ED12C` and ends at the next frame's `0x080ED130`),
  so Zomon's own states ran a frame behind. Now a moved family's call
  to the chip writer (BN5's `0x0800FE6A`) goes to the dispatch's
  `XV_CHIP_WRITER`, which calls BN6's (`0x080126E4`) and answers its
  support family as 0x65 (`XV_SUPPORT_FAMILY`), whose routine runs
  BN6's support routine twice in its first frame, the chip's and the
  attack's end (`XV_SUPPORT`): BN5's timing for a virus's chip, MegaMan's
  support chips BN6's family as before (docs/ROM_DATA.md, A virus's
  support chip in BN6's engine). BN6 plays the hurt sound of a lethal
  hit before its HP test (`0x0801BA3C`), BN5 none (`0x080185C4`): the
  engine's, a mask. All seven PASS; two mutations (the writer's call
  aimed at BN6's own, the family's entry BN6's routine) fail V1's.
  `tools/xfamily.py` gives its statuses outside the time freeze its
  AreaGrab leaves standing once its attack has ended (BN6's status step
  waits while time stands still, `BN6_TIME_STOP`): at battle frame 120,
  Zomon holding no object of its own there.

- **Its rewards**: its reward rows are its game's own for its versions,
  each entry as BN6's (a chip as BN6's of its name, else 200 zenny until
  BN5's chips are BN6's too), written into the rows of its ids in the
  grown copy, which the battle's results then lean as BN6's
  (docs/ROM_DATA.md, BN5's records in BN6's engine).
- **Not yet**: the families' place in formations and a run's battles (a
  game-design pass). The held chips whose code is BN5's alone (MetaGel,
  Blinder, AntiWatr, AntiElec) wait for the chips track to move it; until
  then a virus uses one with a puff. Zomon's AreaGrab is BN6's own chip,
  run with BN5's support family's timing. Every family is installed
  wherever BN5's ROM is read, the New 3DS too (Platforms, above), and
  BN5's records that name one are fought in BN6's engine (`build.py
  xrecords`). A state saved while a moved family is in battle needs
  the same families installed to load (they are, at every boot with the
  same ROMs and switches); one saved on a map is the same byte for byte
  with the room made and the families installed or not (a checkpoint's
  after two of BN6's battles, docs/EMULATION.md, Testing): the install
  stands in the way of no run continued with BN5's ROM taken away or put
  in.

## Transplanted Navis

Issue #82's Navi track: BN5's Team Colonel Navis fight in BN6's own battle
engine, each proved against BN5's own battle frame by frame before it is
installed. A Navi is a family with more (`src/emu/xnavi_install.c`): its
code and records are moved by the families' code mover (`xvirus_move.h`,
the same blocks, pools, veneers and checks), and it is found through
BN6's Navi tables instead of the families' (docs/ROM_DATA.md, Navis moved
into the battle engine).

- **The room** (`src/emu/xnavi_room.c`), made once a core after the
  families' and before any Navi: BN6's thirteen per-AI Navi tables (the
  state tables, the pre-step, hook A, struct1, struct2, the post hook,
  spawn start and end, paralysed, frozen, bubbled, flinching, dragged)
  and the four triples' Navi words (the ice block's size, the held chips,
  the companion's start and end) copied to 256 AI indexes, the companion
  overlay's kinds to 64, each word that names one pointed at the copy;
  the arm points' records of list 2 grown (`xnavi_sprites.c`); and the
  moved Navis' names, an archive of their own handed out by a hook on
  BN6's name lookup for their ids (BN6's own Navis read BN6's archive as
  before). BN5's Navi n of Team Colonel (7-12) is Navi AI 32 + n - 7 with
  four ids from 0x1C4 + 4 (n - 7): V1, V2, V3 and SP (the Dark Soul's
  AI 38, id 0x1DC); its battle sprite list 2's 0x20 + n - 7.
- **The control** (`--dev xnavicontrol`, `tools/xcontrol.py --navi`):
  BN6's own HeatMan derived by the same walk from BN6's ROM and moved as
  Navi AI 25 with ids 0x1C4-0x1C9. Fought by the same keys as the native
  HeatMan, his battle is the same byte for byte in EWRAM and IWRAM frame
  for frame (fight, statuses, deletion), each difference the id, the AI
  index or the copies' places.
- **A Navi's recipe** (`src/emu/xnavi_bn5.c`, written by
  `tools/xnavirecipe.py`; docs/DEVTOOLS.md, The Navis' recipe generator):
  its code walked from its states, hooks and companion routines; the
  objects it spawns that BN6 runs otherwise moved through the
  second-level dispatch, one decision a type for every track
  (`tools/bn5_twins.py`, TYPES: moved unless BN6 runs the same code and
  draws the same art); its calls paired with BN6's twins (the families'
  pairs and audits, `tools/bn5_navi_twins.py`'s beside them); its state
  numbers from 6 made BN6's from 8; its pre-step and hooks as BN6's
  tables take them (hook A the empty hook, its frozen and bubbled hooks
  its paralysis hook: BN5 freezes and bubbles no Navi); its companion's
  start and end, a routine of BN5's copied where BN6's twin spawns an
  overlay kind BN6 lacks, the kind then one of its own past BN6's 19
  (`XN_KIND_OF`) with its masks copied; its panel types BN6's (BN5's
  lava 8 and sea 0xA as BN6's 13 and 14); KnightMan's chip by its name;
  Colonel's table of two entries by side moved from BN5's `0x02034000` to
  `BN6_COLONEL_TABLE` (`0x0203FF60`, zeroed with each battle's RAM) and
  his slash's three effects in a row. What BN6's engine lacks goes to the
  Navis' stubs (`XN_STUBS`): BN5's panel claims, which no battle in BN6's
  engine reaches (their gate, BattleState +0x32 bit 0x40, is set only by
  BN5's battle setup at `0x0802D594`, which BN6 has no twin of: r0 0, as
  BN5 answers without it), and the DS version's chip AI, which no recipe
  installs (logged).
- **What it draws**, in issue #82's ranges for the Navis: its sprites
  copied once into the shared art whichever Navi draws them, at list 4's
  0xA0-0xB5 and list 5's 0xA0-0xA4 (a sprite of list 3 copied into list
  4, its load's list offset made list 4's), BN6's own where BN6 lists the
  same bytes; its effects BN6's of the same sprite, animation and palette,
  else its own from 0xA0 (to 0xBF; 0xA0-0xAC taken, one number a BN5
  effect whichever Navi or engine object spawns it); its sounds BN6 numbers otherwise copied in by `xsong.c` at
  the slot issue #82's plan gives each BN5 sound whichever track plays it
  (0xC0 at 0x62, 0xB7 at 0x61, 0x70 at 0x5F, 0x117 at 0x5E, 0xE1 at 0x5B,
  0x115 at 0x5A, 0x105 at 0x59, 0x10C at 0x58, 0x10D at 0x57, 0xCF at
  0x56), copied only where the slot still holds BN6's empty header.
- **The engine's objects moved once** (`src/emu/xnavi_objects.c`,
  `tools/bn5_objects.py`): BN5's image object, the first kind's type 5,
  which ShadowMan and Colonel spawn through its spawner, is BN6's type 5
  in name only (BN6 reads a kinds table of its own, its sprites at other
  numbers). Its module whole, `0x080B9A5C`-`0x080B9D10` (the kinds table,
  58 kinds of five bytes; the type's routine; the spawner every attack
  calls; its helpers), is one block installed before any family or Navi
  at `XN_OBJECTS` (`xinstall_core`), its type moved through the
  dispatch for every track; a Navi's or a family's call to its spawner
  goes to the copy (WuNote's, its image kind 0x30). Its kinds' sprites, one
  table for every track (`tools/bn5_objects.py`): the twelve kinds whose
  sprite BN6 lists with the same bytes name BN6's (two at another
  number), kind 3 (ShadowMan's and Colonel's, BN5's 3:0x00) a copy at list
  4's 0xB5, kinds 0x0D and 0x23 the Navis' at list 5's 0xA5 and 0xA6,
  WuNote's 0x30 (BN5's 4:0x21) at list 4's 0x6C of the families' range,
  the other 41 keep BN5's bytes, reached by no installed code
  (the generator refuses a recipe that spawns one; a track that needs one
  adds its art there from its own range). BN5's sea's marker, the
  fourth kind's 0x3E (BN6's 0x3E is its invisibility's shadow, drawn
  otherwise and spawned by nothing), is a second module of the same unit,
  `0x080E4C4C`-`0x080E4D3C`, for the sea's submerge (below); the sea's
  splash, BN5's effect 0x63, which BN6 lacks, comes with them as BN6's
  0xAC.
- **The proof**: `build.py xdiff`, BN5's own battle against BN6's with the
  Navi moved, every draw of BN6's replayed from BN5's: each Navi's V1
  alone with MegaMan kept from harm (`*_v1`), V1 against MegaMan's 62
  inputs (`*_v1_keys`), SP alone (`*_sp`); its objects, its moved types'
  objects, its effects and its overlay compared in BN6's numbers, and
  what each draws compared by its sprite's bytes, never its number
  (`src/dev/xdiff_sprite.c`). `--mutations` breaks each Navi's copy the
  way a transplant bug would (a state number, an effect, an overlay's
  kind, a sprite's number, a sprite table's entry, the image object's
  kind and its twin's number, a copied sound): each FAILs its spec.
- **Where they fight**: a battle the `foes` step starts names their ids
  beside BN6's (`0:foes 1C8@52` ShadowMan V1; docs/DEVTOOLS.md, A battle
  on demand), and the run's guardians are BN5's Navis in BN6's engine by
  the same ids since the last round (docs/BOSSES.md, BN5's Navis;
  `--guardian 24`-`29`; One net, The last round).
- **Status** (as built): the six pass all three specs, ToadMan's SP with
  BN5's sea's submerge in BN6's engine (One net, The sea's submerge and
  hold, above), which his own specs on the sea prove beside it
  (`toadman_sea`, `toadman_sea_elec`; `tomahawkman_sea` the hold).
  Colonel's write of BattleState +0x5C's bits (0x10 or 0x20, his side,
  as his spawn ends), whose 0x10 BN6 reads as the field's slide at a
  battle's end, goes to a field of the Navis' own instead (docs/ROM_DATA.md,
  Colonel's sides' bits); BN5's one reader, the obstacles' dark hole
  beside his panels, has no twin in BN6 (open, with the obstacles).
- **The Dark Soul** (AI 0x16, id 0x18D; BN6's AI 38, id 0x1DC):
  moved with BN5's chip AI, which BN6 dropped (docs/ROM_DATA.md, The
  Dark Soul in BN6's engine): its machine (`0x0802BA18`), its 31
  routines (`0x0802BD96`-`0x0802C0E0`) and its approach
  (`0x08029B40`-`0x0802AE70`) copied with it, a dead call into
  `0x081BC994` (data, behind a count nothing raises) answered 0, the
  Navis' buster (BN5's attack setup 0x3E, family 0x16) made by a stub as
  BN6's family 0x66 in the grown table, the chip it plays started as BN6
  starts a chip (setAttack with BN6's record's family), BN6's twins of
  BN5's two tests of its id (its collision's region 0x13; arriving in a
  fight under way, as a failed Chaos Unison brings it, no entry in the
  foes' list of four that shows their HP, BN6's `0x0801DC7C`, where at a
  battle's start both games list it as any foe) made by hooks. Its name
  is DarkSoul, the word BN5's script calls it by (`BN5_DARK_SOUL_NAME`):
  BN5's names archive gives its id DrkCloud, which BN5's own first Custom
  screen shows for it, and BN6's second screen names it as it fights. What BN6's engine has no room
  for is kept apart: its AI data's 0x140 bytes (BN6's are 0x100; BN5's
  chip AI keeps its state from +0xF0) in the battle's RAM, its two
  lists there too, and the chip records it reads, BN6's with the four
  bytes BN5 gives a meaning of their own (the approach, the region it
  aims from, two flags), by BN6's id. Its lists are BN5's battle's own
  making, from the save's record of the chips MegaMan uses most, which
  BN6 keeps none of: in BN6 they are empty, so it moves and fires its
  buster; a list given in BN5 and written into BN6's by the chip
  registry is proved too (`darksoul_cannon`, `darksoul_sword`: it takes
  the chip, goes where it reaches MegaMan, uses it, turns its list). Its
  DS versions of the six are left out (the design). Where a failed Chaos
  Unison spawns it, BN5's `0x08006AAE`, BN6's twin is
  `BN6_SPAWN_ENEMY_1` (`0x08007696`, the same body with its two entries
  in the other order), for the Souls track.

## The chip registry

Issue #82's C1, the ground call 4 stands on: Team Colonel's chips in
BN6's own chip tables, so the pack, the folder, the Custom screen and
BN6's battles take them by their ids as they take BN6's, the same ids at
every boot, with BN5's ROM or without it. What is registered is the
finer calls' subset (One net, above). Those that play in BN6's battles
come into runs' finds, shops, traders and rewards since D1 (One net); the
chip step gives any one (docs/DEVTOOLS.md).

- **The registry** (`src/core/xchips_registry.c`, numbers only): 257 of
  BN5's chips, each with its id in BN6's tables and how it comes in. One
  chip per name: the 116 BN6 has by name are BN6's own (its id, record
  and code), and so are three its US release names otherwise (Attck+10
  and Attck+30, BN6's Atk+10 and Atk+30, which BN6's hand merges into the
  chip before them by their ids; PitHoky, BN6's PitHocky). The other 138
  take ids of their own: BN6's records no chip uses (203-220, 291-300,
  314 and 350-367: blank-named, named by none of its tables, docs/
  ROM_DATA.md, The chip registry) and 411-511, past its 411 records.
  203-207 and 211-213 are the eight BN6's code already plays under other
  names (AirHoc over AirHocky's family; WideSht1-3 over WideSht's;
  Spreader over Spreadr1's; Geddon1-3 over Geddon's, which cracks, breaks
  or poisons by its first parameter as BN5's three do), 208-210 DrilArm1-3
  (C1 played them over BN6's DrilArm family; BN5's own drill since, moved:
  The chips' code), 214-220 the seven DarkChips, 291-294 the Megas (Z
  Saver among them: BN6's Z Saver is a pseudo-chip of its battles, which
  no folder holds, so BN5's has an id of its own, its code BN5's, moved),
  295-300 the first chips to be moved (Guard1-3, CrakOut, DublCrak,
  TripCrak: the three crack chips moved, The chips' code below), 314
  HolyDrem beside BN6's Gigas, 350-364 the fifteen
  Program Advances beside BN6's, 411-504 the rest, the Navi chips last
  (489-504; Colonel and Colonel SP are BN6's). 365-367 and 505-511 stay
  free. A row of its own keeps its codes, library type and MB: what its
  placeholder needs. The pairs were made by name from both ROMs, which
  their SHA-1s fix; the unit tests hold the table to its rules
  (`xreg_check`: every BN5 id once, every own id once and free, every
  BN6 id one of BN6's folder chips or Program Advances, every twin one of
  BN6's records). Of BN5's other chips of a name BN6 has, none is in the
  subset (Team ProtoMan's GunDelEX and Otenko, the Navi chips of Navis
  who guard no run, BN5's pseudo-chips).
- **The tables, one place to read them** (`src/core/chips.c`): BN6's
  records cannot grow where they are (other data follows them), so all
  512 are made anew from the player's ROMs: BN6's 411 as they are, the
  registry's at their ids, a blank record at every id past them; the
  names' and the descriptions' text archives for ids 0-511, BN6's own
  entries copied and the registry's in BN6's text codes (BN5's read
  through `xrom_text_bn6`, its descriptions' box commands kept); BN5's
  icons, pictures and palettes, a picture or palette two chips share
  copied once. The same bytes go into the core's copy (below) and are
  what the run's code reads: every reader of a chip's record, name,
  description or art (data.c's chip info, the chip pool, BN6's
  DarkChips' power, the guardians' rarity, the second screen's cards and
  icons, gfx.c's tiles, the All * helper's codes) takes them from
  `chips_record`, `chips_name`, `chips_desc` or `chips_bytes`, never
  from the player's file; what writes into the records in the core's
  copy (the All * helper, BN6's DarkChips) writes where the game reads
  them (`chips_table_bus`). 174560 bytes with BN5's ROM, 69632 without.
- **A record field by field** (`src/core/xchips_record.c`): BN5's
  record has BN6's layout, its numbers taken as they are (codes,
  elements, rarity, type, MB, parameters, lockout, power, gate) but four:
  its icon element (+6) as BN6's (BN5 numbers its Recovery, Invisible
  and the rest otherwise: measured over the 151 chips both games name
  alike), its lock-on type (+0x17, another meaning in BN5) and its family,
  subfamily and lock-on from its twin (the BN6 chip whose code plays it),
  its Library number and flags 0 (out of BN6's Library screen: the
  finer calls' Library), its ID sort key its own id, its ABC place after
  the BN6 chip whose name comes last before its own, no DarkChipID. A
  power BN5 computes (1001 and up, an index into BN5's own table, past
  BN6's) is shown as none. A chip whose code BN6 lacks keeps an inert
  family (BN6's blank records', 0x1C sub 3), no lock-on and no
  parameters. Its icon is drawn in BN6's icons' palette (BN5's outline is
  grey where BN6's is teal; its other colours the same or BN6's nearest).
- **Without BN5's ROM, placeholders**: the registry's chips keep their
  ids, codes, library type and MB, so a saved folder or pack never names
  garbage and the folder's rules (MB, Mega and Giga counts) still hold;
  each reads "BN5-<its BN5 number>", its card "Needs BN5 / ROM back / to
  battle", Null, BN6's blank art. BN5's ROM put in later (the ROMs
  screen) brings its chips in for theirs as a game is next entered: the
  image is made again and written over the placeholders'.
- **The install** (`src/emu/xchips_install.c`), at every start of the
  core before the boot, all or nothing (every word and halfword it
  changes read first and BN6's own or already its; its place in the free
  space 0xFF or already its): the image past `EMU_FREE` + `0x400000`
  (docs/EMULATION.md), getChip's one literal pointed at the 512 records,
  the 23 words that name the four text archives pointed at the new ones,
  and BN6's pack routines split so ids 320-511 count in an extension of
  the pack in EWRAM BN6 leaves alone (`0x02010CE0`-`0x020115DF`, 12 bytes
  an id: cleared at power-on with the rest of EWRAM, held by every state
  and so by `run.state`): the count's and the total's lookups, the pack's
  totals, its list in the folder editor (to id 511) and its clear (the
  folder editor's exit clears the pack and gives its list back: a clear
  that left the extension counted it twice). BN6's support and Navi
  chips tested a pseudo-chip by its id (every id from 0x170): the test is
  kept to its own (0x170-0x19A), so a chip from 411 is none. Every write
  and its original bytes: docs/ROM_DATA.md, The chip registry.
- **Sitting out** (a hook at the end of the battle deck's deal, bn6f
  `sub_800A3E4`): the chips that sit out are taken out of the deck, the
  rest kept in their order (the Regular chip's and the TagChips' marks
  cleared where theirs went). They are the registry's chips whose code is
  not moved yet (pending; a moved chip, `XREG_MOVED`, plays where BN5's
  ROM is read, its code installed; none pending since issue #82's final
  engine round), BN5's DarkChips but where BN5's dark rule runs (the
  Souls installed: shelved, and dealt by the worried rule, Transplanted
  Souls, S7), its Program Advances (formed on the Custom screen from
  BN6's list grown with their recipes, never dealt, as BN6's own), and
  every one of BN5's where its ROM is not read. A run
  continued without BN5's ROM whose folder holds one of BN5's chips hears
  MegaMan say so once, as he stands free on the map ("Some of our chips
  came from the older Net...", `nets_words.c`).
- **The ids BN6's code tests**: every literal and immediate of a chip
  bound in Gregar (411, 0x170, 0x140, 0x13F, 0x13A) and every range test
  (221-280, the Navi chips) read in bn6f; the pack's bounds and the
  pseudo-chip test patched, the rest out of the registry's way or out of
  a run's reach (the link battle's copy of the pack), and the tables an id
  indexes (the ownership marks and their keys, the Library's flags) sized
  for nine bits by BN6 itself: docs/ROM_DATA.md, The chip registry.

Checked: the unit tests (the registry's rules, records made from made-up
BN5 records, placeholders, the ABC place, the icons' colours, the deck's
sit-out, the image's archives read back); in the game, WideSht1 L on
BN6's Custom screen with BN5's picture (L, Aqua, 70), fired at a Mettaur
(BN5's water wave over WideSht's family, the Mettaur deleted), MrkCan1
(id 411) in the PET's Chip Folder and in the folder editor's pack (its
count in the extension, the same after the editor is left and opened
again) with BN5's description, a folder of 29 MrkCan1 dealing a hand of
its one other chip, and a run saved with BN5's chips continued without BN5's ROM
(its placeholders in the folder, three sitting out of a battle, MegaMan's
words); the autopilot's seeds 1-12 the same, run log for run log, as
before the tables moved.

Not yet, each its later milestone's: CustSwrd's power (BN5's computed power 44,
past BN6's table) and the AntiFire, AntiWatr, AntiElec and AntiWood
traps (BN6's traps trigger by chip id and it has no elemental kinds);
BN5's DarkChips and its Program Advances came in issue #82's final
engine round (the dark rule with the Souls, the recipes in BN6's list:
the battle's Program Advance bits at `0x0203CA48` hold ids to 383 on
side 0, the link list, side 1's, BN6's own: docs/ROM_DATA.md, BN5's
Program Advances in BN6's lists); its Navi chips (M7: AntiNavi's
range, 221-280, and the battle's record of the last Navi chip at
`0x0203C960`); the Library (M9: BN6's Mega, Giga and P.A. tabs by their
own numbers; the profile's Library of 512 ids and the second screen's
came with D1, One net); the folder editor's pack list, 0xA000 bytes, holds
1280 chip-codes where ids to 511 could make 2044 (no run comes near); the
order stamps' clear at a NEW GAME covers BN6's 320 ids (the extension's
start at 0 from the power-on clear); the link battle's copy of the pack
(0xF00 bytes, no run reaches it).

## The chips' code

Issue #82's C2 and C3: a chip of BN5's proven to play as BN5 plays it,
and BN5's own code moved into BN6's engine for the chips BN6 has no code
of. The registry's ids, records and art stay C1's (above).

- **The chips' differential (C2)** (`tests/xdiff/chip_*.json`; docs/
  DEVTOOLS.md, The differential: a chip as the subject): MegaMan is the
  subject; each game gets a folder of the chip (BN5's id and the
  registry's, owned in BN6's pack as its deal asks), the first Custom
  screen's chip picked and OK, the chip used at t30 against BN5's
  CanladEX held paralysed (each game's own paralysis timer and struck
  bit, set each fight frame: BN5's collision data +0x1E, BN6's +0x1C).
  Compared frame by frame: MegaMan's CurAction (a C1 chip's family as
  its BN6 twin's, a moved chip's as its moved number), his attack
  variables and AI data, every object the chip spawns (by each game's
  type, through the dispatch's TWIN and MOVED, and the picture it draws
  by its sprite's bytes where both games' are in their ROMs: the same
  code drawing another picture at the same number is a difference, an
  effect object's picture its effect id's), effects, sounds, draws and
  the target's HP. What differs and is the engine's, not the chip's,
  is explained in `tests/xdiff/legit.json` with both games' routines
  (never on a chip's own records): BN6's chip use ORs into the attack's
  element a hit flag by its icon element (`0x080129E4`: Sword 0x80,
  Cursor 0x40, Wind 0x20, Break 0x10; BN5's has none), carried on by the
  chip's objects; BN6's support family ends its attack a frame later than
  BN5's; BN6's hit-clear keeps a wrong word in a time stop
  (`0x0801A022`); a sword's slash's last frame (each game's own sprite);
  each game's muzzle flashes by its own table (WideSht's flash 0x2A in
  BN5 and 0x59 in BN6 the same picture); the tables a chip's objects index
  by kind (the thrown bombs', the spread shot's, the pucks', the guards');
  NrthWind's gusts drawn by a routine the same in both (a twin of the
  replay). docs/ROM_DATA.md, BN5's chips' code in BN6's engine, has
  each with its addresses.
- **Its verdicts**: BN6's own chips in both games (Cannon, Sword,
  MiniBomb, AreaGrab) PASS, each game drawing its own chip with its own
  pictures (BN6 redrew MegaMan's sword, list 3's 0, and his bomb's hand,
  list 3's 2; there a sprite is compared by its frames' layout); of the
  C1 plays with BN6's code under BN5's records, WideSht1-3, Spreader,
  AirHoc and Geddon1-3 PASS, their pictures the same bytes (each
  difference the engine's: WideSht's wave handed another spawn word,
  Spreader's shot another collision region, AirHocky's wait counted
  from another frame to the same end, Geddon's subfamily BN6's). C2
  found BN6's DrilArm family (0x2E) winding up 9 frames longer than
  BN5's 0x4F and BN6's Z Saver family (0x5B) slashing a frame sooner with
  a collision flag and an AI field BN5's has not: decided (below), they
  now run BN5's own code and PASS.
- **The design calls, decided** (the owner's; issue #82): BN5's chips
  behave as BN5's. DrilArm1-3 and Z Saver are move-class (BN5's own
  code, moved: they sat out until it was, the registry's
  `XREG_PENDING`, now `XREG_MOVED`). BlakWing's and HolyDrem's dark gates
  (their records' +0x15: BN5's chip use lets BlakWing out only to a dark
  MegaMan, the dark meter under 470, and HolyDrem only to a light one)
  are BN5's own rule and came with the mood port (the Souls track's S7):
  both play, their units moved (Their rules in place, below). The
  plus chips stay BN6's rule (one chip per name: Attck+10, Navi+20 and
  Attck+30 are BN6's own ids, merged into the chip before them as BN5's
  merges its own, a puff used alone), but DarkPlus is BN5's: used alone,
  +50 to the next chip (BN5's misc family 0x1A's subfamily 3,
  `0x0800E190`): done (Their rules in place, below).
- **The units (C3)**: a unit is one of BN5's attack families (the
  routine a chip's CurAction runs) or one of its support or Navi chips'
  routines (by subfamily), with every routine and object it reaches that
  BN6 has no twin of. `tools/xchiprecipe.py` derives each from both ROMs
  on the families' generator (docs/DEVTOOLS.md, The chip recipe
  generator) and writes `src/emu/xchips_bn5.c` (numbers only) and
  `src/core/xchips_moved.c` (each moved chip's id, family and
  subfamily); `src/emu/xchips_code.c` installs them on xvirus's
  machinery as the core starts, before the registry's records, all or
  nothing. A twin of an object's code counts only where it is the same
  code to its depth and draws the same pictures: the sprites both load
  by number must be each game's of the same bytes (BN6's own T3 0x45 is
  MetaGel's gel drawing another sprite, its animation 14 frames longer;
  BN6's T3 0x18, CrakOut's crack, and T4 0x3A, NrthWind's gust, draw
  others at the numbers BN5's do: the differential's runs found them);
  an object BN6's twin would spawn of a type BN6 runs other code for
  moves with its spawner; the hit area and the effect object are BN6's
  own, audited. Each object type a unit spawns through the second-level
  dispatch has one decision for every track, moved or twin, written in
  `tools/bn5_twins.py` (`TYPES`; the generator fails on a type it spawns
  that has none or another), and a moved type one copy, the first
  track's to install it (`src/emu/xinstall.c`: the moved objects and
  the Navis, then the chips' code, then the families): a unit spawns the
  Navis' copy of their attacks' objects (the Navi chips' NumberMan,
  KnightMan and ToadMan attacks run the Navis' own; a unit's call into
  that code, ToadMan's note spawner, goes to the Navis' copy too, one
  copy run by both) and of the moved objects', and the families spawn
  the chips' of the types both move
  (SideBub's water and bubbles Marina's, the crack bombs' Bugtank's,
  BoyBomb's bomb BomBoy's, the misc family's meteor MetFire's fire
  column, Pulsar's Batty's, the wide shot Lark's: each kind's row the
  chips' copy fixes).
- **What a unit's code is given**:
  - *Sprites*: one BN6 keeps another of at the number BN5's code loads is
    copied from BN5's ROM into a number of the chips' in BN6's grown list
    (lists 1-5, the chips' from 0xFF down, the families' from the bottom
    up, list 2's 0x20-0x2F the Navis'; list 3, the attacks', grown once
    for every track, `XV_LIST3`), the load set to it: by its immediate,
    the packed word of either packed setup (`0x0800F198`, `0x08018926`), or
    a kind table's row (`TABLE_SPRITES`: the bubbles', the obstacles', the
    shockwave's, the shield's, the geyser's); MegaMan's own sprite (the
    swords' afterimage, BN5's family 0x49's) is each game's own.
  - *Effects*: one whose entry differs takes BN6's of the same sprite
    bytes, animation and palette, else an id of the chips' (0xD0-0xFF,
    from the top down) whose entry names BN5's sprite, copied; by its
    immediate (a number added to a value shifted past its byte too:
    Slasher's), a byte table's row (`TABLE_EFFECTS`: Z Saver's, CustSwrd's,
    Katana's, ElcReel's, the crack bombs', the seeds', the spread shot's),
    an id handed to the engine's routine that spawns effects over a
    pattern of panels (`EFFECT_CALLS`, `0x08018838`), or a run of them by
    an object's kind (`EFFECT_RANGES`: Colonel's soldiers', base + kind,
    one run in BN6's ids).
  - *Sounds*: a song BN6 has another of at BN5's number is copied by
    `src/audio/xsong.c` (`xsong_install_at`) into one of BN6's empty song
    slots, the one fixed for that sound whichever track copies it (the
    shared allocations: 0xBC 0x55, 0xB7 0x61, 0x6F 0x54, 0xE4 0x53, 0x70
    0x5F, 0xBA 0x60, 0xC0 0x62; their second amendment: 0x105 0x59, 0x115
    0x5A, 0x117 0x5E, 0x119 0x4D, 0x8E 0x4C, the fixed ones from 0x40 up,
    the per-run themes 0x26-0x3F; the chips' own from 0x52 down: 0x12E 0x52,
    0xA6 0x51, 0xC6 0x50, 0xBD 0x4F, 0x10E 0x4E, 0xE1 0x4B, 0x4A next),
    taken only where the slot still holds BN6's empty header; the
    immediates name the slot, a table's row its bytes (`TABLE_SOUNDS`: the
    Anti chips' counter's, the seeds', the crack bombs', the spread
    shot's, DrkSonic's sonic's).
  - *Panels*: BN5's lava, sea and holy (8, 0xA, 9) are BN6's 13, 14 and 5
    (src/emu/xpanels.c brings lava and sea in), set or compared by
    immediate or a table's row (`TABLE_PANELS`: the panel changer's kinds,
    the seeds'; `PANEL_LOOKUPS`: ElemRage's element by MegaMan's panel);
    BN5's magnet panel (5), which BN6 lacks, is a type no panel has (0xFE)
    where a unit compares with it and left where no moved unit reads its
    row. A panel's fields past its first 0x10 bytes are read at BN6's
    place (the flags, BN5 +0x18, BN6 +0x14: legit.json's panel layout).
  - *Collision data and MegaMan's points*: an offset into an object's
    collision data is BN6's (legit.json's layout), the immediates of loads
    and stores through it and a number added to its address
    (`COLL_SUMS`: the Anti trap's damage by element, BN5 +0x7C, BN6
    +0x84). A point of MegaMan's sprite a unit asks for (`0x08015140`'s
    r1, `0x08015164`'s r2) is BN6's of the same number where his rows
    give the same, else BN6's that `POINTS` names (CusVolt's 0x1A as BN6's
    0x1B, the centre, 2 pixels further right as every point of his), else
    a stub answering BN5's numbers (`UNIT_STUBS`: ElemRage's 0x14, (-1,
    26) in every row of BN5's, none of BN6's).
  - *Helpers*: BN5's code several units call whose object one copy alone
    may move (the dispatch moves a type once) is installed once before
    the units, at `XC_HELPERS`, and the units' calls aimed at it: the
    buster arm (`0x080EBB96`, BN5's plain buster's too), the shake
    (`0x080D3286`, the third kind's 0x68: SpShake's and ShakPar's), and the
    computed powers' routines (below). The muzzle flash (MegaMan's arm as
    a chip takes it: its spawner `0x080B9CAE`, the first kind's 0x05, its
    58-entry table at `0x080B9A5C`) is BN5's image object, moved once for
    every track with the Navis' install (The engine's objects moved once):
    a unit's calls into its module go to that copy, and the entries the
    units name are fixed there (`tools/bn5_objects.py`: BN6's twin, or
    BN5's sprite copied at the Navis' place or a number of the chips' in
    list 3). The differential counts no helper's calls as a unit's.
  - *Computed powers*: a record's power of 1000 and up is an index into
    each game's table of routines that compute it (BN6's 45 at
    `0x080109DC`, BN5's 76 at `0x0800E728`). BN6's is grown once to 256 at
    `XC_POWERS` (its word `0x080109D4` pointed there), its own 45 kept,
    BN5's entry i at 0x80 + i where a helper moved its routine
    (CustSwrd's 1045 and CusVolt1-3's 1073-1075 by the Custom gauge, the
    Navi chips' SP by BN5's fixed link powers, `UNIT_STUBS` answering the
    link bit 1 as BN6 keeps no deletion times of BN5's Navis, and DS by the
    side's battle counter 3), the rest a stub (power 0); a playing chip's record
    takes BN6's entry (`xchips_power_of`), still computed and shown.
  - *Subfamilies a unit plays*: a family's table of routines by
    subfamily (`SUB_TABLES`: the bombs', the misc family's) keeps the
    entries of the subfamilies its moved rows play (moved, Program
    Advances and the DarkChips of `DARK_MOVED`, waiting for their rules);
    the rest, BN6's own chips' (`XREG_SAME`, run by BN6's code), aimed at
    a stub and not walked.
  - *Another family started*: a chip whose code starts another of BN5's
    families (Slasher's `setAttack0(0x49)`, the player's 0x80000 reaction)
    gives the immediate the started family's moved number (0x99).
  - *Stubs*: a panel claim's spawners (`0x080EA08C`, `0x080EA0BA`) answer
    r0 0. Proved inert in every battle a run fights: they spawn the
    fourth kind's 0x83 (the one object that uses BN5's panel-claim pool,
    `0x0802E920`-`0x0802EAB4`, and its RAM) only where the battle's bit
    0x40 is set (`0x080090E8`, BattleState +0x32); the moved spawner's
    gate is BN6's twin (`0x0800A8F8`), BN6's bit set only by `0x0802E112`
    for a link battle (the link's state `0x0200AD04`, or the Beast Link
    Gate's event flag 0x1722), as BN5 sets its own only at `0x0802D594`.
    No RAM is taken for the pool. Every stub returns by `mov pc, lr`, BN6's
    own idiom: the readers of the support, Navi and computed powers' tables
    call through `mov lr, pc` (BN6 `0x080ED100`, `0x080E2BE6`,
    `0x08017B1E`, `0x080109CA`), a return address without the Thumb bit,
    which a `bx lr` would go on from in ARM state (the differential's
    mutation of a computed power found it: BN6's Custom screen never
    opened).
- **The numbers, one table for every track**: BN6's attack families
  grown once to 240 entries, CurAction 0x10-0xFF (`src/emu/xattack_grow.c`:
  BN6's 0x10-0x5E, the chips' 0x60-0xBF, the Souls' 0xC0-0xCF, the rest
  spare, every entry not given a stub that ends the attack); BN5's
  family F is BN6's 0x50 + F. BN6's support and Navi chips' routines
  grown to 256 entries each, BN6's own first; BN5's support subfamily S
  is BN6's 0x2A + S, its Navi subfamily S BN6's 0x1D + S. Fixed by BN5's
  numbers, the same at every boot and in every build.
- **Moved** (94 chips, each family's spec PASS, its objects' pictures
  compared by their bytes, the registry's `XREG_MOVED`, playing where
  BN5's ROM is read): CrakOut, DublCrak, TripCrak (BN5's family 0x1B as
  0x6B: its crack moved with its sprite), HotBody1-3 (0x58 as 0xA8),
  Jealousy and NrthWind (support 0x13 and 0x15 as 0x3D and 0x3F),
  MetaGel (support 0x06 as 0x30: its gel's sprite into list 3, sound 0x105
  at slot 0x59), Blinder (0x08 as 0x32, sound 0xBC at 0x55), Skully1-3
  (0x55 as 0xA5, sound 0xB7 at 0x61), DrilArm1-3 (0x4F as 0x9F: its flash
  0x2F's drill arm copied), RainyDay (support 0x3D as 0x67: its effects
  0x61 and 0x29, its sea's compare BN6's 14), WoodNos1-3 (0x28), CactBal1-3
  (0x48), SpShake1-3 (0x51), Pulsar1-3 (0x52), each through the buster
  arm, MrkCan1-3 (0x4E: its mark's sprite by the second packed setup, its
  flashes 0x2C-0x2E, sound 0xBD), SideBub1-3 (0x1C: the bubbles' kind
  table), AqWhirl1-3 (0x47: its effect 0x4F, sound 0x70 at 0x5F, its sea
  BN6's 14, its claim spawner stubbed), Z Saver (0x34 as 0x84: the swords'
  reach `0x08012530` copied, its flashes and slashes' effects BN5's),
  Katana1-3 (0x54), ElcReel1-3 (0x23), BoyBomb1-3 (support 0x3A as 0x64),
  Guard1-3 and CrsShld1-3 (0x1F as 0x6F), MoonBld1-3 (0x53 as 0xA3),
  CusVolt1-3 (0x27 as 0x77, their powers computed), CustSwrd (BN5's sword
  family 0x13 as 0x63: BN6's own runs its subfamily 7 with its effect
  0x28, a spark, where BN5's is CustSwrd's slash), the Anti chips
  (support 0x0C as 0x36: a foe's strike made Fire or Elec in both games'
  copies springs the trap, the counter's sound 0x12E at 0x52), Slasher
  (0x29 as 0x79, and BN5's 0x49 as 0x99, the family it starts: a foe on
  MegaMan's side of the field), ElemRage (0x56 as 0xA6: on grass Wood, on
  lava Fire), the bombs (0x12 as 0x62: Quake1-3, CrakBom, ParaBom,
  ResetBom, LavaSeed, CannBall, Geyser, SeaSeed, SeaSeed's sound 0xE1 at
  0x4B) and the misc family (0x1A as 0x6A: RedFrut1-3, Voltz1-3,
  Astroid1-3, WavePit on the sea, RedWave on lava, MudWave, whose waves
  rise from holes, which no field of BN5's has: its use alone compared),
  and the Navi chips NumbrMan, ShadoMan, TmhwkMan, KnightMn and ToadMan
  (their V1s: BN5's Navi routines 0x00, 0x0B, 0x0C, 0x05 and 0x0D as
  0x1D, 0x28, 0x29, 0x22 and 0x2A, each drawing its Navi's own battle
  sprite, the Navis' list 2's `0x20` + AI - 7, and running the Navis'
  copies of their attacks' objects: NumberMan's dice, KnightMan's
  wrecking ball, ToadMan's notes; KnightMan's companion begun and ended
  by BN6's AI index for him, 0x24, `NAVI_AI_CALLS`, and his object's own
  states kept BN5's numbers, `OWN_STATES`). Skully's spec holds a
  recorded FAIL (`verdict`): the engine's status mark over the foe (the
  fourth kind's 0x06, the same code in both) drawn from each game's own
  list 5's 0x0B; `--survey`: nothing else differs. Blinder's recorded
  FAIL (its blind mark at CanladEX's arm point) no longer holds since the
  virus families bring their own arm points: it passes. Mutations (`--mutations`)
  break each kind of number a unit is given (a record's family or
  subfamily, a grown entry, a number in its code, an object's type, a
  sound and its slot, an effect's id, a copied sprite's row in the
  flash's and a kind table) and each fails its spec.
- **What the differential found, and why** (each its cause, the fix the
  generator's and the cause in `tools/xchiprecipe.py`'s tables):
  - The one copy of a type two tracks spawn: BomBoy's specs ran the
    chips' copy of the bomb (the third kind's 0x3E) once the chips' code
    installed first, and at a paralysis (bomboy_v1_keys) its states mixed
    two numberings: BN5's own actions table by the obstacle runner, whose
    generic states BN6's twins number as BN6's. The bomb's update is now
    BN6's twin (`FORCE_TWIN`: BN6's table, as the families' copy had it),
    its CurAction read as BN6's (legit.json, t3_types); a family's spec
    counts the chips' copy it runs as its own code (`xd_log_shared`).
    KnightMn's object froze BN6's battle (its own states' table by +9 read
    at 0xE: `OWN_STATES`) and summoned another AI's companion (BN6's AI
    index for him: `NAVI_AI_CALLS`); the SP and DS powers' routines read
    their records' powers less BN5's bases (`POWER_BASES`).
  - BoyBomb: moved BN6's battle stood still once its thrower's time stop
    ended. Its bomb's actions are a table of routines BN5's code hands to
    the obstacle's state runner (`0x08017FCC`), which the deriver cut at
    its first entry far from it: the rest BN5's addresses in BN6's ROM.
    Now a table handed in a register is walked whole (`TABLE_ARGS`), and a
    table whose next word is a routine too is a decision.
  - Guard: the shield's collision data recorded the reticle's strike with
    bit 0x80 more in BN6's: the comparator's own reading of an alliance
    byte dropped the rest bits (`tools/xdiff_snap.py`, alliance_or), not
    the chip's. CrsShld's shield kept another word at +0x64: BN6's own
    shield (the third kind's 0x2B, the same code) reads its kinds from its
    own table (BN6 `0x080CAED4`), BN5's (`0x080CA2D8`) another sprite at
    CrsShld3's row: moved with BN5's table (`FORCE_MOVED`, `FORCE_COPY`).
  - MoonBld: the foe's AI data apart from the blade's hit on, and 1 HP: the
    hit's stat effect 0x18 is each game's own rule (BN6's virus pre-step
    `0x08013B20` adds the parameter to the AI data's +0x12, BN5 none): the
    sweep copied, its hit's parameter given in BN6's terms (`HIT_STATS`);
    the blade's register its spawn keeps from the calls before it (+0x3C,
    its first frame alone, read by nothing) a mask.
  - CusVolt: the volt a panel ahead in BN6's: its place by MegaMan's
    point 0x1A, another point in BN6's rows (`POINTS`).
  - The Anti chips: no counter in BN6's: the trap reads the struck's
    damage by element at BN5's place in the collision data (`COLL_SUMS`);
    its counter's explosion spawner differs below (moved).
  - DrkSonic: its sonic's time read from a table its BN6 twin keeps
    otherwise (BN5's 60, 60, 60, 60, 120 by kind, BN6's 170, 60, 170, 30),
    the same code: a twin now also compares the data its literals name
    (`same_data`), which moved that phase and found the explosion's and
    MrkCan's, CannBall's and Astroid's tables (each audited or moved).
  - Geyser, CannBall: each game's own spark sprite (list 5's 5) lives
    shorter in BN6's (masks).
- **Their rules in place** (issue #82's final engine round; their code
  moved and installed, `xchips_moved.c` naming their moved family and
  subfamily so their records take it):
  - The Program Advances InfVulc1-3 (BN5's Vulcan family 0x19 as 0x69),
    SuprSpr1-3 (the spread family 0x25 as 0x75), Boxer1-3, ShakPar1-3
    and CacDanc1-3 (the misc family's subfamilies 0x15-0x17) are formed
    on BN6's Custom screen: BN6's list of recipes grown with BN5's
    (`src/core/xchips_pas.c`, written by `tools/xchiprecipe.py` from
    BN5's list `0x08028020` in BN6's ids; docs/ROM_DATA.md, BN5's Program
    Advances in BN6's lists), BN6's 43 first, so BN6's recipe wins where
    two are the same, the P.A. screen drawing every part's code letter.
    Their rows sit out of the deck as BN6's own Program Advances do (a PA
    is formed, never dealt). `pa_infvulc1`, `pa_suprspr1`, `pa_shakpar1`
    and `pa_cacdanc1` PASS: the same three picks form it in both games,
    the P.A. screen shown, used at t30 and played by its moved code frame
    for frame to the foe's deletion. Never formed in BN6's, and so they
    stay (the decision of issue #82's last integration round: BN6's codes
    and recipes come first, no rule of ours replaces BN5's; kept as BN5's
    recipes, `PA_UNFORMED`): InfVulc2 and InfVulc3, whose parts' codes in
    BN6's records (Vulcan2 D, F, L; Vulcan3 A, G, R) never make three in
    a row, and Boxer1-3, whose three FireHit are BN6's own FlmHook1-3's
    recipes, first in the list. Their rows stay registered (a save that
    names one reads the same chip), unformed. The Library's P.A. tab lists BN6's own alone (its
    builder bound to 349: the registry's 350 would land before its
    buffer); BN5's in BN6's Library are M9's.
  - The DarkChips DarkCirc (support 0x2A as 0x54), DrkSonic (support
    0x1B as 0x45), DarkWide (the spread family's subfamily 2), DarkMetr
    (the misc family's 0x11), DrkLance (over its twin's family), DarkDril
    (BN5's drill, 0x4F as 0x9F), DarkTorn (BN5's Tornado family 0x22 as
    0x72, its flash 0x17), BlakWing (support 0x20 as 0x4A) and DarkPlus
    play by BN5's dark rule with the Souls installed (S7: the mood, the
    gate, the use, each one's after-effect on his battle NaviStats
    `0x0802C938`, his dark palette): their specs PASS, their recorded
    verdicts gone, BlakWing's dark in both (BN5's meter under 470, BN6's
    latch). DarkPlus is BN5's: the registry's DarkPlus is BN6's own 290
    by name, whose record takes BN5's family and parameters where BN5's
    ROM is read (`SAME_MOVED`), so it stores its power, +50, in the
    hand's next attack chip and flashes him green 24 frames (the flash
    kept apart from his AIData +0x3C, BN6's resting height; docs/ROM_DATA.md,
    BN5's DarkPlus and its green flash), a puff with no attack after it
    (`chip_darkplus`, `chip_darkplus_merge` with Atk+10, `chip_darkplus_puff`).
  - The Navi chips' SP and DS (NumbrMan, ShadoMan, TmhwkMan, KnightMn
    and ToadMan's SP and DS, ColonelDS) play: an SP's power is BN5's
    computed one by its best deletion time, which BN6 keeps none of for
    BN5's Navis (`XC_SP_TIMES`, a table of none: BN5's power for a save
    without one, 40 for NumbrMn[SP]: `chip_numbrmnsp`, `chip_knigtmnsp`);
    a DS is dealt as any chip and BN5's dark gate lets it out to a dark
    MegaMan alone (refused, the chip of nothing in both: `chip_numbrmnds`,
    `chip_colonelds`; dark, NumberMan's and Colonel's attacks in both:
    `chip_numbrmnds_dark`, `chip_colonelds_dark`). ColonelDS's object
    calls the companion's triple with its third table, the same as the
    second in both games, which the Navis grow in BN6 (`NAVI_TRIPLES`).
  - HolyDrem (support 0x23 as 0x4D: its double of MegaMan drawn from
    his own sprite in each game, its shots from BN5's point 0x1D of his,
    answered with BN5's numbers), Poltrgst (support 0x31 as 0x5B: the
    obstacles it throws by its own table of kinds, MegaMan's double as
    HolyDrem's) and the Tornado family move whole: `chip_holydrem`,
    `chip_poltrgst` (a RockCube to throw), `chip_darktorn` PASS. A double
    of MegaMan is set up through his form's overlay entry with the double
    as its object: with a Soul united BN5 runs the Soul's own entry for
    it (`0x0800F01C`), where the proxy form's gave it HeatCross's overlay
    (the first kind's 0x56): `src/emu/soullife.c` routes it to the
    Soul's (`soul_knight_holydrem`: HolyDrem used in KnightSoul, FAIL at
    t624 before, PASS after).
- **DarkInvs, moved** (13 October 2026; the owner's call of that day:
  BN5's rule wherever its ROM is read). BN5's DarkInvs puts MegaMan under
  its chip AI for the player for 600 frames, invisible, the game driving
  him; BN6 dropped that AI (its twin of BN5's idle states, `0x080F1866`,
  has no path to it). Now, where BN5's ROM is read:
  - *The machine*: BN5's chip AI for the player (`0x0802B4B0`-`0x0802B804`
    and its own side's heads) is copied with the Dark Soul's chip AI,
    whose routines it shares (`tools/bn5_navi_twins.py`: COPIED,
    EXTRA_ROOTS, and two CURACTIONS, its Navis' buster `0x0802B62C` and
    its dead state `0x0802B6D2`): the Dark Soul's code 14 blocks, 8856
    bytes. `src/emu/darkinvs.c` runs it from BN6's idle state past the
    chip request's test (`0x080F1870`) while MegaMan's AI data's +0x48
    holds BN5's flag 0x200, through a stub (`FREE_CHIP_AI`) that leaves
    the state as BN5's does, and clears its state on every other idle
    frame as BN5's does (`0x0802C03E`).
  - *His AI data*: BN5's is 0x140 bytes, BN6's 0x100 with the next
    slot's after it, and BN6's +0xF0-+0xFF and +0x36 are its own (+0x36
    a drain of 1 HP a frame, `0x08014498`). Where BN5's ROM is read his
    AI data is moved into BN5's 0x140 bytes as he is set up
    (`RAM_PLAYER_AI`, `0x080172F6`, before anything of his names it: a
    move at DarkInvs's use left his charge glow's object, the fourth
    kind's 8, pointing at the old place), and BN6's drain is held back
    while the chip AI has him. In a NetBattle the other MegaMen (side
    1's player; two a side, the partners) each move into a room of
    BN5's size of his own as the chip AI first takes him, in the last
    entries of BN6's AI data's pool (issue #182: side 1's player had
    moved into side 0's block, both reading one AI data;
    `src/emu/darkinvs_room.h`, docs/EMULATION.md, BN6's RAM); the
    machine, its time and its idle frames run for whoever's AI data is
    of BN5's size, either side.
  - *The unit* (`tools/xchiprecipe.py` UNITS: BN5's support 0x3B as
    BN6's 0x65, 3 blocks, 860 bytes; its object, the fourth kind's 0x18,
    and his double, the first kind's 0x09, moved): BN5's calls with no
    twin go to three of the chips' stubs (`XC_STUB_DARKINVS`, UNIT_STUBS)
    that darkinvs.c hooks: the chip AI's state cleared (`0x0802C03E`),
    entered with its time (`0x0802C114`: AIData +0x36 600, +0xF0 1, the
    flag; his side's list from his folder, as a failed Chaos Unison's),
    his statuses cleared at BN6's places of the collision data
    (`0x08017060`); his calm (`0x08011A8C`) is BN6's twin `0x080143A6`
    (FORCE_TWIN; soulmood.c's keep_dark holds its mood back while he is
    dark, BN5's mood calmed with the statuses' stub); his double loads
    his own sprite (MEGAMAN_LOADS). SAME_MOVED makes BN6's record 289
    BN5's chip where BN5's ROM is read, as DarkPlus's 290, and the run's
    side follows it (`darkbn6_bn5_code`): BN5's worried rule deals it, it
    makes him dark, its art BN5's (src/core/chips.c), its flame's words
    BN5's rule's and what it does ("For ten seconds,the darkness takes
    over!"); without BN5's ROM it is BN6's own as before.
  - *Its frame*: BN6's support family ends its attack a frame after
    BN5's (legit.json's "BN6's support family's extra frame"), which put
    the machine's first run, and every draw after, a frame behind BN5's:
    for DarkInvs's subfamily the attack ends in BN5's frame (a hook at
    `0x080ED12A`).
  - *Its end*: BN5 counts the time down at the player's frame's end
    (`0x0800DBD8`), BN6 nowhere: a hook at BN6's (`0x0801AD10`) does, at 0
    his request 0x80000, which BN6 answers with its own family 0x49
    (another): where the chip AI has him, BN5's family 0x49 starts instead
    (`0x080EBA9C`), BN5's code moved as BN6's 0x99 (the chips'
    Reaction49), which clears the flag and the invisibility, its
    afterimage and its sound as BN5's.
  - *In a Chaos Unison*: ShadowSoul's chaos attack is BN5's DarkInvs, the
    machine firing the Navis' buster, whose arm reads his Chaos Unison
    from BN5's AIData +0x12 (`0x080EBBFE`): answered from the Souls'
    block (`BN6_SOUL_CHAOS`), the arm's flash BN5's chaos one.
  - *The proof*: `chip_darkinvs` (DarkInvs used at t30, the machine from
    t34 in both, Cannons and the Navis' buster, walking, to t355 before
    the foe falls), `chip_darkinvs_end` (its time made 240 in each game's
    copy, `each`: the end family at t297, MegaMan his own from t321, to
    t420) and `chaos_shadow_45` (its recorded FAIL at t107 gone: PASS to
    t500) PASS; `chaos_shadow_60` as before. The comparator learnt BN5's
    0x16 as the Navis' 0x66 under the chip AI and BN5's 0x49 as 0x99
    (maps, action), a moved type drawing a sprite both games name by one
    pair as each game's own (tools/xdiff_snap.py: his double's and the
    afterimage's MegaMan), DarkInvs's object's chip at +0x30 (maps,
    chip_id) and, with rng=follow, the emotion window's glitch's draw a
    frame apart (masks: "the emotion window's glitch's draw",
    tools/xdiff_masks.py). Mutations: the time a frame short, the end a
    frame long (FAIL). In play (`tools/play.py`, `--net-biome x0 --dev
    worried,god`, `chip 289 * 1`): DarkInvs dealt worried, its name over
    the frozen field, MegaMan grey and untouched, the game firing his
    buster and throwing his MiniBomb, the Mettaur deleted, max HP 140 to
    120 once, the next battle his own.
  - *Its exits*: BN5 takes him out of the chip AI (`0x0802C12A`: +0x36
    and +0xF0 cleared, the flag dropped) where a Soul's unison begins or
    ends (`0x0801214A`, `0x08012276`, beside its calm): `darkinvs_leave`
    from soullife.c's enter and leave. `chaos_shadow_turn` (ShadowSoul's
    one turn ending at the second Custom screen while the machine runs)
    PASS: the machine stops in both and every byte of his AI data and
    object is the same to t760. Its recorded FAIL at t524 (his battle
    NaviStats +0x0E read 0x08 in BN5 against BN6's 0x80 from the Soul's
    end) was the differential's own: its reading of BN5's mood in BN6's
    terms took DarkInvs's hits' gains off a mood BN5's calm had set to
    0x80; since the release candidate's fixes it takes them off only
    since the mood was last set (`BN5_MOOD_SET`, `BN5_MOOD_CALM`
    logged). Not ported:
    BN5's other three exits, `0x0802CCA4`, `0x0802CE3A` and `0x0802D028`,
    each in a routine that sets his HP from his NaviStats, clears his
    statuses, calms him and his invisibility (BN5's own resets of him; no
    run's battle seen to reach them). A battle that ends under the chip AI
    leaves the flag in `RAM_PLAYER_AI`, which the next battle's set-up
    overwrites with his fresh AI data (seen: the next battle his own).
    *His look*: under the chip AI both engines give him their palette 1
    (the same branch in BN5's 0x0800DD8C and BN6's 0x0801002C), each
    sprite's own grey. BN5's dark palettes (2 by the meter, 3 after a
    DarkChip) are its sprite's dark blues, BN6's MegaMan's 2 and 3 yellow
    and tan (his NaviCust's), read from both ROMs' MegaMan sprite: given
    BN5's numbers he was drawn tan after every BN5 DarkChip. Since 13
    October 2026 BN6's engine gives him his own (0, BN6 having no dark
    palette), never a colour BN5 does not show; legit.json's mask "BN5's
    dark palettes" cites both routines.
- **A moved virus's deletion**: the specs stop before a foe is deleted
  (Boxer1, InfVulc1, SuprSpr1, DarkMetr): a moved family's deleted virus
  spawns the fourth kind's 0x11 with a RAM pointer at +0x34 (BN5
  `0x02034A90`, BN6 `0x02034880`) the comparator does not read in BN6's
  terms, the virus families' track's.

## Transplanted Souls

Issue #82's fifth call (One net, above): Souls serve every battle, so
BN5's six Team Colonel Souls move into BN6's own engine as the families
do, a milestone at a time, each proved against BN5's own battle before
the next.

- **What a Soul is to BN6's engine**: BN6's Cross is BN5's Soul Unison
  renumbered (issue #82's research, each place read in both ROMs): one
  form byte in the battle NaviStats (+0x2C: a Soul by its Navi's AI index
  in BN5, Team Colonel's 7-12; a Cross 1-10 and Beast Out 0xB-0x18 in
  BN6), the same 16-byte pending form a side, taken as a turn starts, the
  same requests and phases of the change, the same six bytes of loadout
  in MegaMan's AI data. A Soul is a whole sprite where a Cross is MegaMan
  with a palette and an overlay; BN6 writes a Cross's turns and never
  counts them, and dropped Chaos Unison's charge.
- **S0, MegaMan as the differential's subject** (done; docs/DEVTOOLS.md,
  The differential): a spec can make MegaMan the subject, his setup the
  same in both games (BN5's event flags for the Souls, a folder in each,
  his buster, kept from harm) and each Custom screen passed by its own
  keys in each game, his object, his attacks, effects, sounds and panels,
  his player block (his battle NaviStats, the Souls or Crosses used, his
  pending form) compared in BN6's numbers, and the stop rule flipped: a
  difference of his fails the spec. `megaman_plain` (the buster, a
  Cannon, steps) passes; `soul_knight_ref`, BN5's own KnightSoul from
  UNITE to its fourth turn, was traced in BN5 alone until BN6's engine
  could unite (S2 made it the two-game `soul_knight`). What it found:
  BN5 raises the mood of a side that lands a hit, BN6 never (read as a
  translation until BN5's DarkChip rule is ported), and BN5's plain
  charged shot fires five frames sooner than BN6's (docs/ROM_DATA.md,
  MegaMan as the differential's subject).
- **S1, the proxy form** (done; docs/DEVTOOLS.md, The proxy form): the
  Souls will stand behind form 8, Falzar's TenguCross, which Gregar keeps
  in every table by form and never offers. `--dev soulproxy` makes it an
  exact copy of HeatCross (its eleven tables' entry 8, its NameID's object
  id, hooks on the three places Gregar's code reads the form itself), and
  `tools/xproxy.py` proves it: HeatCross committed as form 8 plays as
  HeatCross every byte of EWRAM and IWRAM, every frame, through the
  crossing, a charged shot, a Fire chip, the buster, a weakness hit that
  breaks it and the battle's end, but the form byte's copies and the
  NameID. Gregar does compare the form with 8 once (TenguCross's Wind
  bonus, `0x0800EFE6`, behind HeatCross's hooked compare), and from form
  8 Beast Out is TenguBeast's (0x14): the Souls grey Beast Out while one
  is united (docs/ROM_DATA.md, MegaMan's forms).
- **S2, UNITE and a Soul's life** (done; in every battle of a run since
  the last round, wherever BN5's Navis are; docs/DEVTOOLS.md, The proxy
  form, and The differential; docs/ROM_DATA.md, UNITE on BN6's Custom
  screen): UNITE on BN6's Custom screen by BN5's rule, and a Soul's life
  as BN5 runs it, each Soul behind the proxy (its look and abilities
  BN5's from S3 and S4, below). Element 0xB under OK is BN6's Beast Out
  emblem and BN5's UNITE button in one place (One net: the design's finer
  calls): UNITE while the last chip picked lights a held Soul (BN5's check:
  the chip's kind as BN5 has it, which for BN6's chips is BN5's chip of the
  same name, as the guest battles crossed them, or BN6's own kind by name; a
  DarkChip of it for Chaos Unison; each once a battle), the emblem by
  BN6's own rule otherwise where Beast Out stands, a grey UNITE where only
  UNITE stands. UNITE stands while the run holds a Soul and MegaMan is
  neither worried nor dark by BN5's mood (as BN6's battles keep it
  beside their own since S7, below), nor beasted out. A on it runs BN5's own code for it, moved into
  BN6's copy, which enters BN6's still-present mode 0x24 for a UNITE
  chosen: the last pick given up, UNITE first among the picks, the Soul's
  picture from BN5's ROM; B gives the chip back, as BN5's does; the chip
  given up leaves the deck, as BN5's (BN6 does so for its emblem in a
  link battle alone); OK commits the proxy form, the Soul's turns (3, 1
  for Chaos Unison) and its used bit. The Soul takes MegaMan as he enters
  the proxy, its turns count down as each Custom screen opens (BN5 counts
  them there, in the step that opens the screen, before its window slides
  in: its code and a frame's trace agree), and it ends as the turn after
  its last starts: BN6 checks a form's end before its Custom screen as
  well, for Beast Out, so the Soul's end waits for the turn's start after
  the screen, the one check BN5 makes in a battle not linked. Another Soul
  or a Cross chosen while one is united replaces it; Beast Out stays grey
  while one is (from the proxy it would be TenguBeast's); no weakness
  break for a Soul. The differential holds it to BN5's own battle frame
  by frame: `soul_knight` (KnightSoul united at the first turn, its turns
  3, 2, 1, 0, its end at t1564 in both engines), `soul_replace`
  (KnightSoul then ToadSoul), `soul_grey` (UNITE grey for a chip of no
  held Soul's kind) and `soul_chaos` (NumberSoul's Chaos Unison, one
  turn); `soul_cross` and `soul_beast` hold BN6's alone (a Cross over a
  Soul, Beast Out greyed); five mutations fail; and `tools/xproxy.py
  --souls` finds BN6's own battle the same every byte with the Souls
  installed and none held, the proxy HeatCross through their stubs.
  What it found: the given-up chip's leaving the deck and the end's order
  against the Custom screen both differ in BN6's code and are put right;
  a turn's change takes BN6 four frames more than BN5 (its form check
  before the screen), and a Soul's crossing six fewer than BN5's unison
  (the proxy's summon), frames whose draws the differential explains.
- **S3, a Soul's look** (done; docs/ROM_DATA.md, A Soul's look): united,
  MegaMan is BN5's sprite of the Soul (BN5's sprite list 0, 7-12, copied
  into the Souls' room and BN6's list 0 grown with them at 0x45-0x4A,
  past the words BN6's code reads through list 0; list 0 the Souls'
  alone by the tracks' allocations), its
  palette 0 (2 in Chaos Unison); no Cross's overlay but what BN5's Soul
  spawns as he enters it (NumberSoul's companion); the unison's phase 1
  is BN5's, moved: the Soul's image (BN6's own dead copy of BN5's object,
  drawn from the Soul's Navi's battle sprite, which BN5's Navis' track
  copies into BN6's list 2, `xnavi_sprite_install`, 0x20-0x25), its
  sounds BN5's (0x8D, 0x143 for Chaos Unison); TomahawkSoul's landing
  shakes the camera as BN5's phase 2 does; his face in the emotion window
  and on the Custom screen BN5's of the Soul, its turns counted under it
  in BN5's digits; his arm's flash BN5's own object (the image object,
  the first kind's type 5, moved with its kinds table: the tracks'
  allocations have every track run BN5's, BN6's table grown by none),
  at its own height, and the arm's points of his sprite BN5's.
- **S4, a Soul's abilities** (done; docs/ROM_DATA.md, The Souls' code in
  BN6's engine, A Soul's abilities, BN6's attack tables grown): the code
  each Soul's charged shot runs is BN5's, moved by a recipe
  (`tools/xsoul.py` writes `src/emu/soulcode_bn5.c`, numbers alone: 21
  blocks since the tracks' merge, their calls aimed at BN6's twins or at
  the other tracks' copies, BN5's numbers BN6 has otherwise renumbered,
  every block's CRC32 checked as it is copied from the player's ROM): its
  power attack in BN6's power-attack table grown (0x94 on) with BN5's
  charge times, its family in BN6's attack families grown once for every
  track (`xattack_grow.c`: 0x10-0xFF, the Souls' 0xC0-0xC6), the objects
  it spawns through the families' second-level dispatch (each BN5 type
  moved once, its decision and whose copy in `tools/bn5_twins.py`'s
  TYPES: the obstacle strike's soldier the Souls', the rest the Navis' or
  the chips', below), its effects (the Souls' 0xC0 on) and sprites (list
  0 from 0x4B) where BN6 has none of the same bytes (or whose own
  animates otherwise) and no other track copied them first, its sounds
  copied into BN6's empty song slots at the slot the allocations give
  each sound (0x70 at 0x5F, 0x117 at 0x5E, 0x115 at 0x5A, `xsong_install_at`,
  found there where another track copied it first). His loadout is the Soul's row in BN6's
  ids, his element the Soul's (TomahawkSoul's Wood, ToadSoul's Aqua),
  no weakness, no buster bonus. ColonelSoul's ScreenDivide, ShadowSoul's
  slash, NumberSoul's bomb, TomahawkSoul's swing, KnightSoul's wrecking
  ball (its confusion) and ToadSoul's sound waves (their paralysis) each
  play as BN5's frame for frame (docs/DEVTOOLS.md, The differential).
- **S5, a Soul's passives** (done; docs/ROM_DATA.md, A Soul's
  passives, A Soul's chips, ColonelSoul's button, chip shot and obstacle
  strike): the passives BN5 gives a Soul as it unites
  and as his form's state is set again (its two tables by form, the
  Souls' routines moved, the proxy's entries of BN6's twins routed to
  them): ShadowSoul's FloatShoe, TomahawkSoul's grass under MegaMan,
  ToadSoul's flag (BN5's sea, which ToadSoul stands on unharmed, harms in
  BN6's engine only a Fire actor as poison does: the lava and sea's own
  port, xpanels.c; its sinking, an object of that flag on the sea, comes
  with the Navi track's port of it); NumberSoul's hand of ten and its +10
  to an attack chip of no kind. The chips (`src/emu/soulchips.c`): the
  chip charge, A held with a chip of the Soul's kind (ShadowSoul's swords,
  KnightSoul's Break chips, ToadSoul's Aqua chips), the chip used charged
  where BN6 would run the power attack, KnightSoul's and ToadSoul's
  doubled; TomahawkSoul's Wood chip doubled on grass and the panel plain
  after, and his heal on grass (BN6's own for a Wood actor, his element
  BN5's); KnightSoul's 50 frames unharmed after an attack chip used from
  his front column; ShadowSoul's B+Left stance and its counter, the
  counter's first frame run at once as BN5's (the Souls' own Thumb);
  ShadowSoul's charged sword run by BN5's sword family, with an enemy in
  reach and without (its family's branch into the next routine's return
  kept in one block; BN6's reserve routine kept, the differential's
  evidence); his shadow none. ColonelSoul's three (`src/emu/
  soulcolonel.c`): his Custom screen's button (BN6's DustCross's elements
  8 and 9, BN5's set-up, rule, phases, draw and art: lit by a standard
  chip of no kind picked last, A drops that chip into it, B gives it
  back, OK keeps it and spends it), his chip shot (the turn's start makes
  his charged shot fire the button's chip, ScreenDivide without one) and
  his obstacle strike (while he is united, an obstacle on the enemy's
  area with an enemy before it, or one of his side behind it, turns into
  BN5's soldier, which fires three shots: BN5's third kind's 0x30 and
  0x12 moved, BN5's block in the obstacles' four states, his side's bit
  in `BN6_COLONEL_SIDES`, never BN6's bits of BattleState +0x5C; BN5's
  Colonel's table at `RAM_COLONEL`, laid out as the Navi track's Colonel
  has it; since the tracks' merge BN5's rule for either Colonel,
  `src/emu/xstrike.c`, below). Each held to BN5's battle frame by frame by a spec of its own
  (the strike's spec leaving out the RockCube's own use, each game's
  chip), a mutation of each part failing. The effects BN6 draws
  otherwise (0x02, 0x03, 0x14) are BN5's copies (0x02 and 0x14 the
  Navis', since the tracks' merge); the flashes' sprites
  BN5's 3:0x00 and 3:0x03 stand at the Navis' numbers (list 4's 0xB5,
  list 5's 0xA6: one place for each sprite's bytes once the tracks
  merge, tools/bn5_objects.py), and ColonelSoul's button's art in the
  room issue #82 granted (`FREE_SOULS_ART`).
- **S6, Chaos Unison** (done; issue #82's final engine round,
  `src/emu/soulchaos.c`, `soulchaos_thumb.c`; docs/ROM_DATA.md, Chaos
  Unison): a Soul united by a DarkChip of its kind charges B as BN5's
  does, its time its chaos power's row by the chaos level (40 frames at
  level 0, to 150 at 4), then a cycle of phases by the level's row (BN5's
  `0x08010648`: 42 frames at level 0, phase 2 its first 21), shown by
  BN5's dark charge glow (the Souls' copy, its animation the phase).
  Released in phase 2 it is the Soul's DarkChip attack through the chip
  writer, BN6's chip use of the registry's id (ColonelSoul's DrkSonic,
  NumberSoul's DarkPlus, TomahawkSoul's DrkLance, KnightSoul's DarkDril,
  ToadSoul's DarkWide, each by BN5's code), the level one up (4 at
  most); released in another phase it fails as BN5's family 0x39 does:
  MegaMan out of the Soul and the Dark Soul (the moved Navi) on the
  other side, its chip lists, where the battle gave none, MegaMan's
  folder's chips, the most of a chip first (BN5 keeps the chips he uses
  most, BN6 no record of them). BN6 reads the chaos request 0x8000 as a
  counter's: its four tests of it drop that bit while the Souls' request
  stands. The power attacks are entries of the Souls' grown power table
  (BN6's 0x9F-0xA4), the failure the attack family 0xC7. Held to BN5's
  battle frame by frame by 19 specs (`chaos_*`: each Soul's success at a
  hold of 45 frames and failure at 60, NumberSoul's holds 75, 90, 105,
  120, 150, 180 and 210: 45, 90, 180 and 210 succeed, the rest fail, as
  BN5's), two mutations failing (the cycle a frame longer, the Dark Soul
  a frame later). ShadowSoul's attack is BN5's DarkInvs since 13 October
  2026 (The chips' code, DarkInvs, moved: MegaMan under BN5's chip AI,
  its arm's chaos flash answered from the Souls' block): `chaos_shadow_45`
  PASS to t500, its recorded FAIL at t107 gone.
- **S7, BN5's mood and its DarkChip rule** (done; docs/ROM_DATA.md, BN5's
  mood in BN6's battles): `src/emu/soulmood.c` keeps BN5's mood in the Souls' block
  beside BN6's own, by BN5's rules (a hit taken lowers it, a hit landed
  raises it, a counter calms him to Full Synchro, BN6's twins of BN5's
  other writers mirrored, a Soul's unison and end calm him, the battle
  opens as worried as his HP says), never through BN6's emotion values.
  BN5's DarkChips (the registry's) come into the deck, shelved behind the
  padding until BN5's worried rule deals one into the hand's end as a
  Custom screen opens while he worries or is dark; used, he is dark for
  the battle (his mood 0, held there; BN6's own mood 0 with it: no Cross,
  no Beast Out, a weak buster). BN6's own DarkChips keep BN6's rule.
  UNITE stands only while he neither worries nor is dark. BN5's dark gate
  holds for the registry's chips that name it (BlakWing only while dark,
  HolyDrem and a Navi's V1 and SP chips only while not, the DS chips only
  while dark): refused, the chip is BN5's chip of nothing; the battle's
  dark stands for BN5's dark meter, which no run has. Held to BN5's own
  battle by `soul_mood_dark`, `soul_mood_unite`, `soul_mood_gate` and
  `soul_mood_chaos` (Chaos Unison as BN5's play reaches it: DarkDril
  offered worried, kept, a Cannon's landed hit lifting him by its 0x1E,
  BN5's gain, where the buster's hits lift none, and KnightSoul's Chaos
  Unison on the calm screen after), the mood compared in every Soul's
  spec, a mutation of each rule failing. Since
  issue #82's final engine round each DarkChip's after-effect on his
  battle NaviStats (`0x0802C938`, ported) is BN5's too (his dark palette
  BN6's own since 13 October 2026: BN6's MegaMan has none, BN5's numbers
  drew him tan; The chips' code, DarkInvs, moved), the worried
  rule held on the battle's first screen as well (`soul_mood_first`:
  the DarkChip into deck[5], past the five the screen deals, in both),
  and BN5's DarkChips' own attacks play by their code (The chips' code,
  Their rules in place).
- **At the tracks' merge** (issue #82's four tracks in one tree): one
  copy of each moved type, the first install's (`src/emu/xinstall.c`:
  the moved objects and the Navis, the chips' code, the families, the
  sea, then the Souls' code), so the Souls' code runs the others' copies
  of what it spawns and calls into (`tools/xsoul.py`'s BEFORE and
  shared_blocks, the C's routes): the image object the moved objects'
  (MegaMan's buster's flash in a Soul sent to its spawner there,
  `soullook.c`), NumberSoul's companion and bomb, KnightSoul's ball and
  ToadSoul's hit the Navis', the obstacle strike's shot and ShadowSoul's
  afterimage the chips' (the shot's row the strike spawns it with fixed
  in the chips' copy to the Souls' effect 0xC1: `tools/bn5_twins.py`'s
  SHARED_ROWS); one BN6 number a BN5 effect and a BN5 sprite, the
  first track's (ScreenDivide's three and TomahawkSoul's swing the
  Navis' 0xA8-0xAA and 0xA6, ShadowSoul's slash 0xA3, 0x14 and 0x02
  0xA0 and 0xA2; the strike's hit's sprite the chips'); BN6's
  collision update's audit serves BN5's own body, which the Souls copy
  as the families and the chips do. The obstacle strike's soldier draws
  BN5's sprite 4:0x20, set from a packed word (`0x0800F198`), which the
  Souls' generator now numbers: BN6's 4:0x20 is another picture (the
  differential's sprite bytes, soul_colonel_strike's t634). The obstacle
  strike is BN5's rule for either Colonel (`src/emu/xstrike.c`): the
  Souls' code and the strike are installed with the core where BN5's
  Navis are, sprite list 0 grown by the Souls' code for its own art, and
  a Navi Colonel's spawn arms his side in `BN6_COLONEL_SIDES` (his moved
  write, `XN_STUB_SIDES`) as ColonelSoul's ready does, one field the
  strike reads: `colonel_strike` holds a Navi Colonel's soldier to BN5's
  (MegaMan's RockCube before him, the three shots, frame for frame in
  both; its comparison stops at the cube's own use, BN5's RockCube
  object being the code Colonel's moved copy runs, its expectations held
  in both battles), a mutation of his side's bit failing it. BN5's
  DarkChips' specs hold the Souls (KnightSoul) so that BN5's mood and
  DarkChip rule are in place: since the final engine round they PASS,
  their recorded FAILs gone (each chip's after-effect and his dark
  palette ported), BlakWing's run dark in both (BN5's meter under 470,
  BN6's latch) and HolyDrem's moved.
- **Next**: the Souls' engine is whole, ShadowSoul's chaos attack
  (BN5's DarkInvs) since 13 October 2026; what is left of DarkInvs is The
  chips' code's (DarkInvs, moved: three of its exits). The masks the differential kept "until the Souls track's S6"
  are gone. The Souls' state in RAM is the engine's
  plan's (`0x0203FF70`-`0x0203FFEF`, `BN6_SOULS`), their code and data
  in `EMU_FREE` + `0x480000` to `+0x4C0000` and their art past it at
  `+0x700000` to `+0x710000` (docs/EMULATION.md; laid out in
  `src/emu/soul_room.h`).

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
  (`src/gfx/xbackdrop.c`), for every battle fought there: its random
  battles, its guardian, ProtoMan's netbattle.
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
- [x] Its random battles in the areas it dresses: in its own engine on a
  second core at first (Guest battles, above), in BN6's engine since D1
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
  by band (One net, the last round; docs/BOSSES.md).
- [x] Their Souls: beaten, each gives his for every battle of the run,
  UNITE on BN6's Custom screen by BN5's rule (Transplanted Souls;
  docs/META.md).
- [x] Its DarkChips in runs: all twelve kinds one chip each, BN5's flame
  on its acts, one price in either net (One net, the last round).
- [x] Its chips in BN6's own chip tables (The chip registry, above):
  257 of them registered by id, those BN6 has by name BN6's, 138 with
  ids of their own and BN5's records, names, descriptions and art in
  BN6's tables grown to 512, placeholders where its ROM is not read.
  AirHoc, WideSht1-3, Spreader and Geddon1-3 play in BN6's battles by
  BN6's code, those of The chips' code by BN5's own code moved; the rest
  sit out until theirs is. Those that play come into runs (One net: D1);
  a chip that sits out never enters a pool.
- [x] Its chips judged against BN5's own (The chips' code, above): each
  used in both games and compared frame by frame; BN6's own and eight of
  C1's twelve the same as BN5's, DrilArm1-3 and Z Saver BN5's own code
  since (decided).
- [ ] Its chips' code in BN6's engine: the chips of The chips' code
  moved and proven (Moved); the rest wait, each for what The chips' code
  names (a found difference to trace, the mood port), with its Program
  Advances and DarkChips.
- [x] Its Navis in BN6's own engine (Transplanted Navis, above): Team
  Colonel's six and the Dark Soul moved, each proved against BN5's
  battle frame by frame (ToadMan's SP with BN5's sea's submerge, made
  again in BN6's engine; the Dark Soul with BN5's chip AI, its lists
  empty in BN6 for want of BN5's record of the chips MegaMan uses);
  fought where a battle names their ids, and as the run's guardians
  since the last round.
- [x] On the New 3DS too (Platforms, above): BN5's ROM read whole beside
  BN6's where the heap has room, in the 16 MB mGBA's 3DS setup had held
  unused; its layers' learned maps kept for the layer's own area alone
  there (docs/EMULATION.md, The 3DS's memory; `tools/heap3ds.py`). Built
  and modelled on a computer; a New 3DS's own run with both ROMs is
  3ds/README.md's checklist.
- [ ] Team ProtoMan (the other version), Battle Network 4, BN6 Falzar.
