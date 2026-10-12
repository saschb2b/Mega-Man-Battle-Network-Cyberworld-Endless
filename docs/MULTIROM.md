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
inside BN6 as it stands (it calls its own game's addresses), but a virus
family's can once it is moved: copied into BN6's free space, its calls
aimed at BN6's routines (Transplanted families, below).

| Pool | How another game fills it | Cost |
| --- | --- | --- |
| Net areas | Its maps learned as BN6's are (`tiles_learn.c`), its tile set and palette copied into BN6's free space | moderate |
| Towns | The same, from its real-world maps | moderate |
| Music, sounds | Its MP2K songs and their voice groups into BN6's song table | low |
| NPCs, mugshots | Its sprites as bystanders and dealers | low to moderate |
| Chips | Where BN6 has the behaviour (most standard chips); its own (Navi chips, seven of its twelve DarkChips) have no code in BN6, which keeps the other five DarkChips with theirs, four of which play in BN6's battles (docs/ROM_DATA.md, BN6's own DarkChips; docs/META.md) | limited |
| Viruses | The families both games have; its own need their AI. BN5 shares 5 of its 33 with BN6 (Mettaur, Catack, Champy, WindBox, Trumpy, by the first name of each family in both games' tables), and its areas' battles are mostly its own (ACDC Area's CanGards and Powies), so a mix from its data alone would take the BN5 out of them; or a family of its own moved whole into BN6's engine, its code with its calls aimed at BN6's (Transplanted families: CanGard so far, met only by the dev steps) | limited; a recipe per family moved |
| Guardians | Its Navis need their AI: a battle in its own engine | high |

For what needs its game's code, the way is a battle in that game: the
engine keeps BN6's state, switches the core to the other ROM from a state
made there, sets up its battle (MegaMan's HP, his folder in its chips),
runs it, reads the result and brings BN6 back. That takes a battle-level
port of the other game (its RAM, as `src/emu/bn6.h` is BN6's), a second
ROM in memory, and shows its battle screens.

A second way is under way for viruses (issue #82, Transplanted families,
below): a family's own code copied from the other game's ROM into BN6's,
its calls aimed at BN6's routines, so its viruses fight in BN6's battles
beside BN6's own. BN5's CanGard is the first: installed wherever BN5's
ROM is read, it fights in BN6's battles that the dev steps start; no
run's battles hold it yet.

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
   reads it (issue #68). A New 3DS's 96 MB heap holds BN6 twice (mGBA's
   copy and the engine's) with little room left: there another game's
   data would be taken in as a run needs it, its ROM not kept.

## Guest battles

Where BN5 dresses an area and the build can run a second core (desktop,
Android, iOS, PortMaster, the browser; not the 3DS, which keeps one ROM),
the layer's random battles were BN5's own, fought in BN5's engine
(`src/emu/guest.c`, epic #57). Since issue #82's D1 they are fought in
BN6's engine (One net, below) wherever every record of the area can be,
which with BN5's ROM read is everywhere; the guest's battles below remain
where a territory is not native (an install that failed) and for BN5's
Navis as guardians, until they move into BN6's engine:

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
- **Its rolls**: BN5's random numbers are the run's. On the frame BN5
  begins a battle, both its RNG words are written from the battle's seed
  (a random battle's from the layer's seed and its battles so far, as its
  record is picked; a guardian's from the layer's seed), so its first
  hand, its green Mystery Data and find and its reward differ from run to
  run as BN6's do, and the same seed fights the battle again as it was
  (a CONTINUE, or MegaMan walking into it another way). Its core began
  each session from its boot state, and every run's first battle in its
  engine had drawn alike, whatever the seed (issue #102; docs/ROM_DATA.md,
  BN5's random numbers).
- **The record's copy**: past BN5's 8 MB in the guest's ROM copy, without
  its GAME OVER bit, so a loss ends on the map with BN5's result 2 and the
  run ends as BN6's would (the deletion shown in BN5's battle).
- **The end**: BN5's result (won, lost, escaped) and the HP the battle
  left (its BattleState's copy, as BN5 copies it back to its NaviStats
  only on its own maps' terms) go back into the run. The results screen's
  rewards go into BN6, both as BN5 gives them to its own save
  (docs/ROM_DATA.md, BN5's results screen): the busting level's, and
  where a green Mystery Data stood on the enemies' side unbroken as the
  battle was won, its find (BN5's own battlefield Mystery Data: some of
  its records set one, there in about a third of ACDC Area's battles that
  name one and in every one of the other areas', and any hit breaks it;
  its find is 1000 to 3000 zenny, 1 to 3 BugFrags or, outside ACDC Area,
  now and then a chip, as BN5's rows have them). A chip goes in as BN6's chip of
  the same name, to the Pack (GiveChips), zenny as zenny (GiveZenny),
  BugFrags as BugFrags (GiveBugfrags): a playtester's screen said "50 z",
  then "BugFrag 1", and the run had kept the zenny alone (session 69).
  Before each battle, its enemies' reward rows and the Mystery Data's
  finds are rewritten where a chip has no BN6 chip of its name: 200 zenny
  instead, so the screen never shows a chip the run won't get (of the
  finds' chips, AirHoc S alone); with All * the others are in *; and on
  every other row of an enemy's (a reward's row comes of the busting
  level) a chip both games have is put in the first of the folder's codes
  both games' records of it have (`xchips_fit`), so about half the chips
  come in the folder's codes, as BN6's rewards lean to them. An HP+N its
  screen gives (a row's kind 2, as BN6's) is in the HP that comes back.
  `runlog.txt` has a line for each, as for BN6's battles, with its
  record, its viruses by BN5's ids and the reward as the run got it, a
  find after it (docs/DEVTOOLS.md).
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
  room its boot state stands in), breaking into blocks wider than tall
  as it goes, as BN6's own battle switch draws the map (16 pixels across
  at most, captured in a BN6 battle), and BN5's picture flashes in once
  its battle is on its screen; after a profile's first, MegaMan says what
  it was and names the chips that sat out, the same way.
- **For tests** (issue #61, docs/DEVTOOLS.md): the battle step
  (`--input "300:battle"`, play.py's `battle`) starts the layer's next
  random battle once MegaMan is free on its map, a guest battle on such a
  layer (`--net-biome x0`, ACDC Area), and `--talk guest:FRAME` one at a
  frame; a headless run waits for the boot's thread at its first such
  battle, and `--dev slowboot=N` runs BN5's first boot at N of its frames
  a frame on the main thread instead (the same frames every run), so a
  fresh data folder's first BN5 battle waits behind the older net's wait
  (`--dev quiet,slowboot=12 --input "300:battle"`, the autopilot paging
  the arrival's words: open from frame 370, the battle at about 640);
  `--talk dark:FRAME` opens the flame of darkness's talk (a middle
  layer: `--run-depth 2`), `--dev darkchips=MASK` starts a run holding
  DarkChips (bit k BN5's id 187 + k), `--dev worried` keeps MegaMan
  worried in guest battles and `--dev gem` sets a green Mystery Data on
  the enemies' side of every one, as in BN6's (its find of row 0, 1000
  zenny or a BugFrag; with `onehit` it more often stands to the end: with
  `quiet,gem,onehit` and the battle step, the first guest battle on ACDC
  Area found "BugFrag 1" on seeds 1-8, before its random numbers were the
  run's); `--guardian 24` to `29` puts one of BN5's
  Navis (Colonel, ShadowMan, NumberMan, TomahawkMan, KnightMan, ToadMan)
  in every act BN5 dresses (`--net-biome x0 --run-depth 3 --guardian 28`:
  KnightMan at ACDC Area's first act), and `--dev souls=MASK` starts a
  run holding Souls (bit k Team Colonel's Soul 7 + k: 1 Colonel's, 2
  ShadowMan's, 4 NumberMan's, 8 TomahawkMan's, 0x10 KnightMan's, 0x20
  ToadMan's); `--dev god`, `onehit` and `fragile` and the
  dev menu work in them as in BN6's. `CYBERWORLD_AUTOPILOT=1` fights them
  as it fights BN6's, from BN5's memory (`src/emu/bn5.h`: its Custom
  screen's picks, its panels, its viruses and their attacks; never a
  DarkChip), `weak` keeping their viruses at 1 HP; the run log has a line
  for each (`guest`, its record, its reward). `CYBERWORLD_EMU_DEBUG=1` names each
  battle, its folder's chips in and those that sat out, slot by slot
  ("guest: 4 sat out: CrakShot A, ..."), the buster it goes in with and the
  one its battle took, a line every 30 of its frames (MegaMan's mood
  among them), its end, the flame and a DarkChip's price, and with Souls
  held whether UNITE stood on the first Custom screen and each unison
  ("guest: MegaMan united with Soul 9, Chaos Unison"); `build.py pacing`
  lists every act's guest battles against its band, and
  `tests/test_emu.c` runs two cores side by side.
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
  turned purple, in a bystander's place. Four kinds BN6 keeps too
  (DrkSword, DarkThnd, DrkRecov, DarkInvs; docs/META.md, BN6's own
  DarkChips) play in BN6's battles from its folder: a kind is one chip
  in both nets, and one in BN6's folder never goes into the guest's as a
  chip beside the run's copy; BN5's rule brings it, first among the
  run's three.
- **Its boot** (the owner's Retroid Nova froze five seconds entering a
  run's first BN5 area, 5 October 2026: "guest: booted in 7210 frames,
  5.01 s of its frames' time", the whole boot inside one frame of the
  layer's setup): BN5's first boot (7210 of its frames, kept as the state
  `guest-bn5-1.state` in the data folder) begins as the game starts
  (its boot screen, else the title: `guest_warm`) where BN5's ROM is
  there and runs in the background, never in a frame's way. On native builds (desktop, Android, iOS, PortMaster) it
  runs on a thread of its own at a low priority (`boot_main`), which alone
  touches the guest core until it is ready and nothing of BN6's: the chips
  of both games are paired, mGBA's logger set and the sound's rate read on
  the main thread before it starts, mGBA 0.10.5's GBA core shares nothing
  else between cores (read in its source: the default logger's pointer,
  its thread logger's key made once, constant tables), the core's first
  write to its ROM (mGBA copies the ROM to 32 MB then) is the thread's, and
  the main thread takes its end in at a frame's end (`guest_tick`, a join),
  `guest_read*` answering 0 until then. `tests/test_emu.c` makes and runs a
  guest core on a thread while BN6's frames run with their hook, under the
  sanitizers. The browser, without threads, runs it in each frame's spare
  time (`guest_tick`: the frame's work and the slice within 10 ms of its
  1/60 s, 12 ms while a battle waits for it), from the title through the
  town and the layers, its first write to the ROM held for the first
  battle's white; there a kept state loads as a layer is made, the page's
  memory BN6's until then. A layer never waits for the boot or hangs on
  it: its battles are BN5's wherever BN5's ROM is read and the build runs
  a second core (`guest_possible`, a boot that failed aside), finished or
  not, so a run stays the same from its seed whenever the boot ends.
  A headless run (the captures, the autopilot, `tools/play.py`) waits for
  a thread's boot at the battle that needs it, without frames, so its
  frames are the same from run to run: the autopilot over BN5's ACDC Area
  (seeds 1-3, fresh data folders, and 1-2 with the state kept) gave run
  logs and frames byte for byte the old build's, which booted inside the
  layer's setup, and the state the thread keeps is the same file the old
  boot kept. The boot runs the same frames in the same order everywhere
  (its keys a function of the frame, `boot_keys`). Measured on an Intel
  Core Ultra X7 358H (headless, in Docker): the thread's boot took 3.4 to
  3.6 s of its frames' time; BN6's frames on a layer beside it 0.4 ms of
  the GBA's and at most 0.8 ms of update and drawing in each second, as
  after it, where the old build stopped 3.44 s in the layer's setup. On
  the Nova (untested there yet) its 5.01 s, on a core of its own, would end
  before a player who presses NEW GAME at once is through the town, its
  main thread keeping its GBA frames of about 3 ms. In Chrome 152
  (headless, agent-browser, on a loaded machine): at the title (measured
  before the boot screen came in) the boot took 8.5 s of its frames and
  ended 15 s after Jack in, the page's frames
  16.8 ms at most from the game's first second on; on ACDC Area's first
  layer it ran in BN6's frames' spare time through the arrival's words to
  87% when the battle came, which waited about 2 s behind the older net's
  wait (below), every frame at 16.8 ms at most from the layer's first
  second (making it paused the page 0.3 s, as before).
- **The older net's wait** (`src/emu/guest_wait.c`): where a battle comes
  before the boot is done (a fast player, a slow device, a phone's
  browser), it waits on the white the switch fades to, as BN6's own
  battle switch holds white. Designed through the game-design skill's
  interface layer: a rare loading screen in the encounter's seam, the
  moment loop's, which must never add time (the frequency-as-budget
  rule: the switch is seen every battle, the wait at most once a data
  folder), so it opens only where the wait needs it (the white held 12
  frames with more than 0.7 s of the boot left at its pace, or 45 frames
  whatever is left) and closes as the boot ends, the battle opening
  behind it. Immersion against information is settled in the game's own
  channels: MegaMan says what happens in BN6's chat box (diegetic: the
  PET), his mugshot and the chat font from the player's ROM, typed a
  character a frame, three pages as the wait goes on ("Hold on,Lan! / The
  older Net's / still starting up...", "It only starts up / this once,Lan.
  / Then it stays ready!", from 85% "Almost there,Lan... / Just a moment
  more!", in BN6's voice, docs/VOICE.md); how far it is shows in the battle
  HUD's own form, where BN6's stands, so BN5's own HUD takes its places as
  the battle opens: a gauge drawn to the measure of BN6's Custom gauge
  (its rim, track, fill and pins, captured in a battle as chatbox.c's
  frame was in a chat), labelled OLD NET where BN6's says CUSTOM, filling
  with the boot and never ahead of it (an honest telegraph: the boot's
  frames take about the same time each), and the percent in a box drawn
  to its HP box's, in the battle font from the ROM. Between them the net's
  light streams over the white in the chat box's blues, as a jack-in's
  does. The box opens as BN6's does (three frames from its middle row
  out), the HUD drops in from the top; no flashes, no sound (the switch's
  silence). The browser's note it replaces said "Waking the older net...
  40%" in plain text. Captured with `--dev slowboot=12` (For tests).
- **In the browser** (issue #68): the page runs one frame at a time with
  no threads, so the guest's battle frames take BN6's frames' place, never
  both in one 1/60 s, and its boot's slices fill what a frame leaves
  (above); the state it keeps goes into IndexedDB (`platform_persist`).
  The page's WebAssembly memory, 97 MB on a first layer with BN6 alone,
  was 153 MB on BN5's first layer with the guest booted (BN5's ROM, 8 MB,
  and the guest core's ROM copy, which mGBA copies to 32 MB at its first
  patch, as it does BN6's), and 197 MB at most, after a move from Central
  Area into End Area (the BN5 maps its tiles are learned from); it never
  shrinks. The JavaScript heap was 21 MB (14 MB with BN6 alone), beside
  the two ROM files the page keeps in memory. A phone's browser would
  need about 250 MB for the page, and its slower WebAssembly (three or
  four times) the title, the town and the first layers' spare time for
  the boot, else a while behind the older net's wait; untested on a
  phone.
- **Its guardians** (docs/BOSSES.md, BN5's Navis; issue #69): on the
  guardian layer of an act such an area dresses, in half the runs, one of
  BN5's own Navis guards it, where BN5's net maps set him roaming
  (`NetAreaDef.xguard`: KnightMan ACDC Area, ShadowMan Oran Area,
  TomahawkMan SciLab, NumberMan or ToadMan End Area, Colonel its
  Undernet), decided from the seed and the ROMs present (`run_guardian`),
  never on the short net's last act. He stands in his own BN5 sprite and
  face (copied in at Gregar's 78) and is staged as BN6's guardians are; his
  battle is BN5's, from BN5's own record for him at the version the act's
  band takes (the story's V1-V3, the net's SP), his stats row's HP in the
  guest's ROM copy capped at the band's top as he spawns, the record's
  copy with no running (options `0x20` cleared) and its results screen on
  (`0x02` set: his V1 records leave it off), his reward rows paying an
  Unlocker's price, as BN6's guardians' do. Won, his staging goes on as
  BN6's (his last word, his Guardian Data); lost, the run ends by him.
- **Souls** (docs/META.md, Souls in BN5 territory; issue #69): a guardian
  of BN5's, beaten, gives his Soul with his Guardian Data; the run holds
  it to its end, kept beside its checkpoint (`run.souls`,
  `src/director/souls.c`), and a new run holds none. Before each guest
  battle the run's Souls go into BN5's own event flags (eToolkit +0x44):
  Double Soul's (0x0000) and Chaos Unison's (0x0236) while it holds one,
  and each Soul's own (0x0008 + k, Colonel's first); BN5's boot state,
  early in its story, holds none of them, so a run with no Soul fights as
  before. BN5's Custom screen then offers them by its own rule: UNITE
  stands under OK while MegaMan is neither worried nor dark as the screen
  opens, lights when the last chip picked is of a held Soul's kind (a
  DarkChip of it for Chaos Unison), and each Soul unites once a battle,
  its Chaos Unison once besides. Given up for Chaos Unison, a DarkChip is
  no DarkChip used (BN5's use flag stays clear): no max HP for it. A
  DarkChip comes only while MegaMan worries and UNITE stands only while he
  does not, so Chaos Unison asks for one offered hurt and kept in the hand
  to a calmer turn; the shelf (DarkChips, above) keeps a DarkChip once in
  the hand, worried or not. On BN6's net a Soul does nothing.
- **Still to come**: the NaviCust's other programs in (those both games
  have, as BN5's battle reads them), BN5's chips registered in BN6.

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
  (NumberMan's Chaos Unison takes it); without it, it stays out as now.
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
  where it sits out (and in the guest's battles the same).
- **Words**: the act's card says "Older Net" in a territory, "Both Nets"
  in the deep, "Nets meet" in a BN6 area from act 2 where visitors can
  come (none under the Gentle helper); MegaMan names the first native
  territory, the first visitor and the first deep act once a profile
  (Profile.native_taught, visitor_taught, deep_taught); L's briefing names
  the older net's viruses or both nets'. Until BN5's Navis move into
  BN6's engine (the Navi branch), a territory's guardian of BN5's still
  fights on the guest core, and the words say so: its own Navis fight the
  old way, no Crosses against them; a Soul wakes against them.

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
BN6's. The guest core fights only where a territory is not native (an
install that failed) and its guardians (the Navi branch brings them). A
territory's Servers and the duel's race are its records too;
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
install patches and the dead stack, where the flags, now read from ROM,
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
  `+0x500000` and the chips' moved code at `+0x600000`. The mode byte of a
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
  differential's alone, and the run's guest battles have none: a guest
  battle's 128 pictures and its log are the same as before), on each
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
  family before it), what it draws (its sprites copied and the effects
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
| WuNote | 26 | 52, 0x278-0x27d | 7, 3032 | 67 | 11 | 1:0x05, 1:0x16, 3:0x84 | 2, 0 | - | - | 1 id in its data |
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
  Marina's water's records, the fire column's kinds, the image object's
  kinds). A sound BN6 has no song of is copied once, at the slot issue
  #82's tracks share for it (Powie's 0xC0 at 0x62, Skarab's 0xB7 at 0x61,
  Flashy's and Eleogre's 0xBA at 0x60). A sprite is BN6's own where BN6
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
  first kind's 0x05, which WuNote spawns with kind 0x30) is moved whole
  with its 58 kinds, the same module the Navis' track installs: only the
  kinds a family spawns are fixed (WuNote's 0x30, its art copied into the
  families' list 4), the others keep BN5's bytes, and a family spawning a
  kind not fixed is refused. A family's numbers its code keeps as they
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
  with `rng=replay` and `"sprites": "bytes"`: 168 of the 175 the same
  over every fight frame, among them every one of CanGard's and Handi's
  as before; Zomon's seven fail, below. What the families showed the
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
- **Zomon** fails all seven from the frame after its first AreaGrab
  (V1 t391, EX t363, V1's keys t410, V2 t344, V2EX t325, V3 t307, V3EX
  t280), every byte the same before it. Its set-up gives its object
  AreaGrab by hand, BN6's own chip of the name (the registry's 117 as
  163), which BN6's own chip code runs: both games spawn the time freeze
  at the same frame, take MegaMan's front column and play its sounds at
  the next, but BN6's family of time-freeze chips (0x15, `0x080ED0DC`)
  ends its user's attack a frame later than BN5's (`0x080EC400`, which
  spawns and ends in one frame: `bl 0x0800F29E` at `0x080EC448`; BN6's
  marks its AttackVars +1 at `0x080ED12A`-`0x080ED12C` and ends at the
  next frame's `0x080ED130`), so Zomon's own states run a frame behind
  from there and its later draws part. An engine difference, not its
  code's: installed, its AreaGrab as BN6 runs its own.

- **Its rewards**: its reward rows are its game's own for its versions,
  each entry as BN6's (a chip as BN6's of its name, else 200 zenny until
  BN5's chips are BN6's too), written into the rows of its ids in the
  grown copy, which the battle's results then lean as BN6's
  (docs/ROM_DATA.md, BN5's records in BN6's engine).
- **Not yet**: the families' place in formations and a run's battles (a
  game-design pass). The held chips whose code is BN5's alone (MetaGel,
  Blinder, AntiWatr, AntiElec) wait for the chips track to move it; until
  then a virus uses one with a puff. Zomon's AreaGrab is BN6's own, a
  frame longer than BN5's. Every family is installed wherever BN5's ROM is
  read, and BN5's records that name one are fought in BN6's engine
  (`build.py xrecords`). The New 3DS keeps no BN5 ROM: no family is
  installed there. A state saved while a moved family is in battle needs
  the same families installed to load (they are, at every boot with the
  same ROMs and switches); one saved on a map is the same byte for byte
  with the room made and the families installed or not (a checkpoint's
  after two of BN6's battles, docs/EMULATION.md, Testing): the install
  stands in the way of no run continued with BN5's ROM taken away or put
  in.

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
  203-213 are the eleven BN6's code already plays under other names
  (AirHoc over AirHocky's family; WideSht1-3 over WideSht's; Spreader
  over Spreadr1's; DrilArm1-3 over DrilArm's; Geddon1-3 over Geddon's,
  which cracks, breaks or poisons by its first parameter as BN5's three
  do), 214-220 the seven DarkChips, 291-294 the Megas (Z Saver among
  them: BN6's Z Saver is a pseudo-chip of its battles, which no folder
  holds, so BN5's has an id of its own over that pseudo-chip's family),
  295-300 the first chips to be moved (Guard1-3, CrakOut, DublCrak,
  TripCrak), 314 HolyDrem beside BN6's Gigas, 350-364 the fifteen
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
  not moved yet (pending), BN5's DarkChips until BN5's dark rule comes
  into BN6's battles, its Program Advances until BN6's lists hold their
  recipes, and every one of BN5's where its ROM is not read. A run
  continued without BN5's ROM whose folder holds one of BN5's chips hears
  MegaMan say so once, as he stands free on the map ("Some of our chips
  came from the older Net...", `guest_words.c`). BN5's own battles (the
  guest's) take the registry's chips as BN5's own, by its ids.
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

Not yet, each its later milestone's: the 104 pending chips' code (Guard
and the crack chips first); CustSwrd's power (BN5's computed power 44,
past BN6's table) and the AntiFire, AntiWatr, AntiElec and AntiWood
traps (BN6's traps trigger by chip id and it has no elemental kinds);
BN5's DarkChips (M8, the dark rule); its Program Advances (M6: their
recipes in BN6's lists, and the battle's 32 Program Advance bits a side
at `0x0203CA48`, which ids from 352 pass); its Navi chips (M7: AntiNavi's
range, 221-280, and the battle's record of the last Navi chip at
`0x0203C960`); the Library (M9: BN6's Mega, Giga and P.A. tabs by their
own numbers; the profile's Library of 512 ids and the second screen's
came with D1, One net); the folder editor's pack list, 0xA000 bytes, holds
1280 chip-codes where ids to 511 could make 2044 (no run comes near); the
order stamps' clear at a NEW GAME covers BN6's 320 ids (the extension's
start at 0 from the power-on clear); the link battle's copy of the pack
(0xF00 bytes, no run reaches it).

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
- **S2, UNITE and a Soul's life** (done, behind `--dev soulproxy` or
  `souls=MASK` until the track is whole; docs/DEVTOOLS.md, The proxy
  form, and The differential; docs/ROM_DATA.md, UNITE on BN6's Custom
  screen): UNITE on BN6's Custom screen by BN5's rule, and a Soul's life
  as BN5 runs it, each Soul behind the proxy (its look and abilities
  HeatCross's until S3 and S4). Element 0xB under OK is BN6's Beast Out
  emblem and BN5's UNITE button in one place (One net: the design's finer
  calls): UNITE while the last chip picked lights a held Soul (BN5's check:
  the chip's kind as BN5 has it, which for BN6's chips is BN5's chip of the
  same name, as the guest battles cross them, or BN6's own kind by name; a
  DarkChip of it for Chaos Unison; each once a battle), the emblem by
  BN6's own rule otherwise where Beast Out stands, a grey UNITE where only
  UNITE stands. UNITE stands while the run holds a Soul and MegaMan is
  neither worried nor dark by BN5's mood (his battle mood with BN5's
  thresholds, a battle opened as worried as his HP says: the part of
  BN5's mood UNITE needs; the rest, the gains and the DarkChip rule, is
  S7), nor beasted out. A on it runs BN5's own code for it, moved into
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
- **Next**: a Soul's look (BN5's sprites, its image in the summon's
  place, its sound), its charged shots and abilities, its passives (a
  ToadSoul's and a ShadowSoul's flags, NumberSoul's hand of ten),
  Chaos Unison's charge, and BN5's DarkChip rule (the mood's gains, the
  worried hand). Each part's masks in the differential (tests/xdiff/
  legit.json, "until the Souls track's S3" to "S6") go as it comes.
  Their state in RAM is the engine's plan's (`0x0203FF70`-`0x0203FFEF`,
  `BN6_SOULS`), their code and data in `EMU_FREE` + `0x480000` to
  `+0x4C0000` (docs/EMULATION.md).

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
  (the browser's from the files its page took, issue #68).
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
  areas it dresses (Guest battles, above); since D1 in BN6's engine
  (One net: native territories), the second core only where a territory
  is not native.
- [x] Its viruses and BN6's meet: from act 2 one battle in four takes a
  visitor of the other net, and the deep's acts draw on both nets' late
  areas (One net: D1).
- [x] Its chips in runs: half the finds and stock on its areas and in the
  deep, its viruses' own rewards, the Library's count and the second
  screen's list (One net: D1).
- [x] Its guardians, in battles in its own engine: KnightMan, ShadowMan,
  TomahawkMan, NumberMan, ToadMan and Colonel guard the acts of the areas
  BN5 sets them in, in half the runs (Guest battles, above; docs/BOSSES.md).
- [x] Their Souls: beaten, each gives his for the run's BN5 battles, Soul
  Unison and Chaos Unison on BN5's Custom screen by its own rule (Guest
  battles, Souls; docs/META.md).
- [x] Its chips in BN6's own chip tables (The chip registry, above):
  257 of them registered by id, those BN6 has by name BN6's, 138 with
  ids of their own and BN5's records, names, descriptions and art in
  BN6's tables grown to 512, placeholders where its ROM is not read.
  AirHoc, WideSht1-3, Spreader, DrilArm1-3, Geddon1-3 and Z Saver play
  in BN6's battles; the rest sit out until their code is moved. Those
  that play come into runs (One net: D1); a chip that sits out never
  enters a pool.
- [ ] Team ProtoMan (the other version), Battle Network 4, BN6 Falzar.
