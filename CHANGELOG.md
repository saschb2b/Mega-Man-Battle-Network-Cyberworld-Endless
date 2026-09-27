# Changelog

## Unreleased

First playable version.

- The NaviCust is a run's second build (docs/NAVICUST.md). Every guardian's
  Guardian Data offers three programs of three builds (buster, hand, guard,
  field, HP), each with MegaMan's words for what it does, or B for
  BugFrags; the pool is 34 of BN6's programs by tier, the large ones as the
  board grows. The act 2 and act 4 guardians give an ExpMemry: the board
  grows from 4x4 to 5x4, then 5x5. When the NaviCust's bugs change,
  MegaMan names each and what it does (BN6's RUN says OK over a part left
  past the board's edge, which bugs).
- The Net Dealer says how an AquaNdl pick lands: its needles drop where the
  guardian stands a moment later, so fire when he stops (two of three
  missed a hopping BlastMan).
- Act 1 varies more. It is Central Area or the RoboDog Comp: the Robot
  Control Comp's battles that fit act 1 were all OldStove and Mettaur, and
  it waits for act 2, where Champy and Gunner join them. Act 1's guardian
  is BlastMan or DiveMan as likely (BlastMan was two in three), and a new
  run from the title takes the next seed's where its first guardian would
  be the last run's (the profile keeps it; an older profile reads as
  none). The pacing report lists each battle roll's virus families.
  `--guardian N` pins every area's guardian for a scripted capture, and
  `--net-biome` puts its area in the run's act as well.
- The way-on arrow leads around Mystery Data and navis where MegaMan
  stands right beside one (it walked him into it, from the object's own
  panel), and its look ahead keeps clear of them; the arrow test walks
  among them now, as round objects smaller than a panel.
- The way-on arrow keeps to the walk. It showed a turn's first panel when
  the walk cut a platform's corner, and a running MegaMan was two panels
  past the turn before it turned (a look every 15 frames, now 5), so on
  BlastMan's layer it led off the start platform's corner and round in
  circles. Its eight ways are the pad's: a walkway's run lies in the middle
  of a diagonal, where on the screen it lay a hair from "left" or "right",
  and the diagonals are drawn along the floor's. It leads around Mystery
  Data and navis. A ROM-free test follows it from every room of 120
  layers to the guardian or the exit pad (the old arrow lost 208 of 956
  walks). The playtest state names the arrow's way, and
  `CYBERWORLD_STATE_POS=map` draws the whole layer with its walk.
- A battle's viruses are counted from the battle the game set up: the
  pointer that names it kept the last battle's until the setup wrote it, so
  a battle after a re-roll could count the other battle's viruses (AREA
  CLEAR showed 11 of 12). The run log writes a battle once it is known.
- A Recovery Mr. Prog talked to again on a layer heals in one box ("ALL
  PATCHED UP!") instead of his whole greeting. BlastMan's warning names his
  flame dash and fire wall beside the bombs.
- The application ID is the repository's name,
  `io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless`, as
  Flathub asks. The AppImage moves a menu entry it made under the old ID,
  and `install.sh` replaces its old entry and icons.
- A Flatpak (`linux/flatpak/`, `build.py flatpak`, on every tagged
  release): the Steam Deck's way to install software, from Discover. It
  reads the ROM from Downloads and EmuDeck's and RetroDECK's folders, read
  only, and keeps its saves in its own folder.
- Steam Deck: started by Steam's Gaming Mode or Big Picture (or in
  gamescope) the game fills the screen, 5x on the Deck (a window was scaled
  to it by a fraction, and blurred), and skips the AppImage's menu offer.
  The ROM is looked for in EmuDeck's and RetroDECK's gba folders, on the
  Deck and its SD card, in ~/ROMs and in Downloads, found by its contents
  and copied in, before any dialog. On a controller, Back and Start held a
  second ask to quit, and held again quit, as Escape does on a keyboard.
- The Net Dealer is the only green armored navi on a layer: bystanders are
  the game's dark EvilNavi and the NaviCust vendor its pink GirlNavi. The
  sprites the bystanders and the vendor had were the dealer's on the map
  (their chat faces differed, which hid it). The
  vendor, talked to again, says "More programs? Take a look!" and opens
  his list, as the dealer does.
- The Net Dealer's answer is a Standard chip (he offered two DiveMan, and a
  folder takes one), the best of eight found rather than four (half of act
  1's Elec answers hit past a third of the guardian), and always of the
  element he names (a few layers had none). One over that third he brings
  alone, and says so.
- Net Dealers, vendors, Mr. Progs and bystanders stand off the walkways'
  mouths: a dealer stood where a walkway met his platform, and the way
  on went through him (the platform's box took in the walkway's end).
  Runs saved before this version continue their layer from its start.
- A battle starts MegaMan on the panel its original battle gives him:
  column 2 row 2 on most fields, beside it where that panel is a hole or
  poison (he always stood on column 2 row 2, in the hole too). The next battle, re-rolled every five seconds on a layer, is
  written to one of two records in turn, so a battle the game has rolled
  but not yet set up keeps its own field, foes and MegaMan's panel.
- The NaviCust is in the PET from the start of a run (MegaMan, then
  NaviCust): the gift's and the vendor's programs could not be installed
  before. The gift, the vendor and a bystander say where to install them.
- R near the town's statue says which way the statue is and shows the
  way-on arrow, and the townsfolk stand off the statue's approach.
- Walkways without lining up: holding one direction at a walkway's mouth
  or a spur lines MegaMan up and takes him in (never back into the one he
  came out of); a diagonal held along a walkway follows it round its
  turns, stepping to the middle of a lane first when he is off it; a
  diagonal held into a corner slides him round it, and a push into a navi
  frees him. A key held into a platform's corner leaves him there (it
  walked him along the edge and back), and pushing a while where the pad
  goes nowhere shows the way-on arrow. Starting to walk from rest off a
  panel's middle no longer steps MegaMan sideways first: the help read
  the game's two frames before he moves as being stuck.
- Walkway mouths' corners are walls of the corner's shape, as the
  originals have them; each was two edges in one cell, and the game took
  either, pushing MegaMan out sideways at a mouth's side.
- A talks to the navi or Mystery Data MegaMan stands at even when he faces
  past it: he turns to the one before him, else the nearest (walking into
  a navi slides him round it), and reaches about 54 units, which is how
  far a short Mr. Prog can be while looking a tile away. One a step or two
  short (up to two and a half panels before him) he walks up to and talks
  to; the pad or B stops the walk. With two navis side by side, A talks to
  the one he means (the game's check took the first its probe touched).
  Of those within reach, one on the side he faces comes before a nearer
  one behind him (a vendor a step behind turned him from Mystery Data).
- A navi's tip on running from a battle says how (on the Custom screen, hold
  L and press R); it said START, which only pauses.
- An act's first layer from act 2 on always has a Net Dealer, and every
  Net Dealer stocks two of a chip of the element that answers the act
  (its guardian's weakness, else its viruses'), the hardest hitting of a
  few that takes at most a third of the act's guardian, at a price a run
  has by then (400 zenny in act 1, 300 more an act),
  and says which, naming the guardian, and that it has two. A
  bystander's tip on where Net Dealers set up no longer says the middle
  layer only. Talked to again, the Net Dealer says "Back for more?" and
  opens his list. A shop's list takes no A for its first moment, so the A
  that closed its keeper's words twice over no longer asks to buy the
  first item.
- In battle, an L or R pressed just before the Custom gauge fills opens
  the Custom screen when it does (the game drops it; a playtester
  re-pressed in every fight).
- Every Mystery Data opens: some on a new run's first layer stood at the
  world's origin or said they were locked and printed stray text, their
  picks shared with the game's own Mystery Data of other maps.
- A Navi chip's version mark reads "EX" or "SP" in chat boxes: the chat
  font draws each mark as two letters stacked in one cell, which read as a
  kanji ("ProtoMn" and one came out of a Mystery Data), and now draws them
  side by side.
- Act 2's guardian is one of 600 to 700 HP (HeatMan, SpoutMan,
  CircusMan): EraseMan's 800 after BlastMan's 400 was a wall. Guardians
  are drawn from every navi whose HP suits the act, the area's own twice
  as likely, so act 1 is not always BlastMan (DiveMan too). A guardian
  of no element has the Net Dealer stock his hardest hitter, two of it,
  and say so. MegaMan's first word on a guardian's layer warns of its way
  of fighting, for every guardian (EraseMan's ghosts and his erasing
  blow, BlastMan's rolling bombs, HeatMan's fire towers and lit panels
  and more), each written from a fight watched.
- The NaviCust vendor's programs cost a quarter of the game's prices,
  which are its endgame's (2500 to 7100 zenny against a run's 100 to
  1000 a battle or Mystery Data), and 200 more an act; HP+200, which
  would triple a first act's HP, costs 2400 and 400 more an act.
- Servers, Net Dealers and other services stand off a room's exits, where
  they blocked the way on, and bystanders stand in the open, not in a
  panel-wide gap where they pinned MegaMan. None stands beside a floor
  of another height, where he looked a step away and was a stair's walk
  round. The map's key lists what the layer holds,
  Servers and dark warps as "Event", and shows the services MegaMan
  senses but has not reached: a ring where each stands, or a pip on the
  map's edge its way. An L pressed as a chat closes is heard, and a
  later L while MegaMan is hurt says where the Recovery Mr. Prog lies.
- A Server's prize is the hardest hitting of a few chips a tier better
  than Mystery Data (it could be a WhiCapsl). The prize and the Net
  Dealer's answer are chips that strike outright: an answer that needed a
  paralysed enemy (MchnSwrd) or a hole in the floor (SumnBlk) was no
  answer. The answer is no sword either: CircusMan kept to his back
  column, and two AquaSwrd never reached him.
- A new act eases in: every battle on its first layer comes from the lower
  half of its band (two BombCorns on the first layer after BlastMan took a
  playtester from 220 HP to 10).
- L says where the exit or the guardian lies when the walk there sets off
  another way ("The way winds, so follow the arrow!"), so the words hold
  still while the arrow shows the next stretch; the Recovery Mr. Prog,
  which heals every time, is named again while MegaMan is hurt. After a
  CONTINUE, L remembers it has told where they are, the map remembers what
  was seen, and the way-on arrow stays up for as many boxes as L speaks.
- Standing anywhere on the exit pad takes MegaMan on: its trigger covers
  the pad's whole panel (on its rims he stayed before).
- The map (hold Select) fills the screen: the floor seen so far with its
  panels apart, so walkways read as lines, the whole of it when it fits,
  marks for MegaMan, the exit, heals, shops and the guardian with a key
  under it, and until the goal is seen a mark on the frame the way it lies.
- A story for the run, the Endless Net: Dad's call on the first run, Lan
  and MegaMan talking on arriving somewhere new (the first layer, the
  Undernet, the Graveyard, the Nest, the side layers, the net rebuilt
  after the Nest as Net V2 and on), Chaud's call after the Secret Area,
  and the guardians as the Nest's copies of MegaMan's old battles. Every
  chat box shows its speaker's face: Lan, MegaMan, Dad, Chaud, Mr. Prog,
  the townsfolk, the bystanders, the Net Dealer's Normal Navi, the
  NaviCust vendor's technician navi (with its own shop screen) and the
  guardians. On the map L asks MegaMan where they are, what guards the
  area and how much ScrtData they carry.
- Bystander navis are other divers, with lines for the early, middle and
  deep net and for a rebuilt one; no two on a layer say the same thing,
  and every hint is true of this game. Services, choices and rewards talk
  like the game does ("MegaMan got: ..."), the Secret Area gate counts
  your ScrtData, and the Crosses and BeastOut come with their own words.
- The run summary shows Lan, where and by whom MegaMan was deleted, the
  layer reached and a new best; depths read "Layer N" everywhere. Area
  cards fit long names, and cards wait out shops, the PET and battles.
- Chat boxes that run past three lines turn the page evenly (two and two,
  never three and a lone line), at the end of a sentence where they can.
- From layer 20 on, acts begin where they should again: the second cycle's
  act cards, easy opening battles, Net Dealer layers and layouts were one
  layer off. The NaviCust vendor's and the gift's programs always come
  from the game's full program list at their own prices, however many
  layers came before.
- Aquarium Comp's glass pools keep their whole rims: where a water channel
  meets one, the channel ends at the rim, instead of its water stepping
  over the rim and the glass in 8x8 blocks. Each pool and the rest of the
  floor are drawn on their own and laid over each other on the game's two
  tile layers, as the original sets its pools on legs apart from the water.
  The guardian's arena is a pool too, no longer a patch of glass in water.
  `build.py atlas` draws the two tile layers in the game's order, the
  second over the first.
- Joins look like the originals'. The tiles are learned from the originals
  as the game shows them, the second tile layer in front of the first, and
  a tile keeps what the originals set in front of a floor on that layer only
  where the floor needs it. Pieces of it no longer come along: Seaside's
  boardwalk railings across its platforms, Central's bridge, the broken red
  ornaments on Green's and Central's pads, CopyBot's spikes over its rims.
  Green's planks end in front of the grass's side face or behind its rim,
  and CopyBot's plateaus and red pads are drawn whole with their walkways
  ending at them (no more blocks of plateau face, nor rim strips in the red
  floor), as the Aquarium's pools are. The Judge Tree no longer draws its
  courts' flat red where a catwalk meets a room, nor stump rings on its
  pads, and Sky's pads lose the pieces of its round pods. A floor's corners
  may keep the outline the originals draw a pixel past a side face, so the
  atlas counts no fallbacks in any area. Floors, walls and objects are
  unchanged.
- Where two of Central's floors meet at an inner corner, the panels there
  no longer show scraps of yellow-green floor: they were pieces of Central
  Area 2's raised yellow plateau, whose corners fitted those joins better
  than any green floor's. Tiles of an area's other maps that draw colours
  its own map never shows on its floors are left out everywhere, so the
  brown and orange scraps at the ends of the Undernet's bridges, the
  orange blocks beside Robot Control Comp's walkways, the dark posts where
  Sky's walkways meet its platforms and the green arrow pieces under
  Mr. Weather Comp's edges are gone too. Seaside keeps Seaside 2 and 3's
  yellow panels, the only fields of its second floor, which draw its
  guardian's arena. `build.py atlas` counts such tiles per layer (other
  colours). Floors, walls, objects and scenery are unchanged.
- Robot Control Comp's white platforms keep their rims: the two light
  bands that frame the original's platforms run all the way round, with
  the dotted corner caps, where the layers showed flat white fields with
  ragged, broken edges. The bands reach half a panel into the floor,
  deeper than a tile is held to the plain floor's look, so the edges now
  take the original's own tiles; and the walls around the grey cubes on
  Comp 2's platforms no longer count as holes when its tiles are learned,
  so inner corners keep the bands too. Pads are small white platforms in
  the look of Comp 1's and the Pavilion's raised ones, no longer half the
  striped conveyor from before the robot's door. The atlas counts less
  than half the near misses and seams it did in the area. Floors, walls,
  objects and scenery are unchanged.

- The run begins in the real world, in Central Town or ACDC Town, each
  cut into pieces from the game's own map and set out again per run.
  Central Town: Lan steps out of his front door among Capcom's houses,
  Aster Land, the Academy gate, the Expo gates, the bus stop and the plaza
  with its trees and blue bird statue, the plaza, the main road and the
  houses' court sometimes wider. ACDC Town: he comes up the Metroline's
  stairs among the park with its squirrel statue, Higsby's, Dex's and
  Mayl's houses and the Ayano mansion, the blocks sometimes in another
  order. Townsfolk stand where they belong and talk about the place, and
  the houses, shops, signs and statues answer A. At the statue (or ACDC
  Town's doghouse) R plays the game's own jack-in to the first layer. The
  real world's maps, their tables, objects, checks and jack-in
  destinations are documented in docs/OVERWORLD.md and docs/ROM_DATA.md;
  `build.py town` and `build.py world` show them.
- Keyboard: the layout of Capcom's PC version (the Legacy Collection). W A S D
  move, J and K are A and B, Q and E are L and R, Enter is Start, R is Select;
  the arrows with X and Z still work. Keys are physical positions (Z Q S D on
  AZERTY). `keys.ini` in the save folder remaps them. Escape asks before it
  quits, and keys held while the window loses focus are let go.
- Linux: an AppImage (runs on any x86-64 distribution with glibc 2.34+,
  without libfuse2, and offers to add itself to the application menu; its
  update information lets AppImageUpdate or Gear Lever fetch new releases)
  and a `.deb` that installs into the menu like any package, beside the
  tar.gz. All three carry an application icon and AppStream data, and the
  window's class is the app ID, so a pinned dock icon matches the running
  game. Without a ROM the first start asks for the file (a file chooser with
  zenity or kdialog) instead of stopping.

- The game itself runs from the player's BN6 Cybeast Gregar (USA) ROM on an
  embedded mGBA core: its battles, net, PET, shops, traders and music.
  Nothing from the ROM is shipped.
- Generated net layers in the game's own map formats: floors learned from one
  of the game's maps per area (Central, Seaside, Sky, Green, Graveyard,
  Undernet, Undernet Zero, Underground), with walls the game's collision
  reads. Each area builds its layers in its own layouts, after how BN6's net
  maps are laid out (docs/LEVEL_DESIGN.md): Central's routes, crater fields
  and catwalk mazes, Seaside's fields framed by comb boardwalks, Sky's
  mirrored hubs, Green's plank ladders, Graveyard's holed slabs, the
  Undernet's webs of long bridges and lattices of crosses. Walkways wear the
  area's second floor (blue catwalks, yellow boardwalks, orange planks). Pads
  on spurs and dead ends hide most of the Mystery Data. Tiles made for one place in the original (bridges, cut corners,
  decoration hanging off an edge) are not used on plain walkways. Sky and
  Undernet layers can raise a dead-end room onto a stair taken from the
  area's own map.
- Mystery Data, shops and traders draw from the whole chip library: every
  standard chip, the Megas and, deep in a run, the Gigas, by rarity.
- The story's Robot Control, Aquarium, Judge Tree, Mr. Weather and CopyBot
  comps and the ACDC, Green and Sky homepages join the first four acts, with their own battles, music,
  backgrounds and guardians. Every area now learns its floors from all of
  its maps in the same tiles and at each of their heights, so raised pools,
  fields and platforms have floor to copy; holes that faces hang over are
  told from floor by the walls around them. The originals' free-standing
  scenery (the Aquarium's coral, shells and starfish) stands beside the
  floor, and pads take the look of the original's pads (the Aquarium's
  yellow frames on legs). CopyBot's comp, whose floors no colour tells
  apart, learns them by shape: purple plateaus in stone rims with pods
  beneath, pink and white walkways with teal discs, magenta octagon pads. Runs saved before carry over.
- Three more areas for the first four acts, built from the game's computers
  and homepages: a comp in orange and green, a homepage in pink and teal and
  a comp in blue and pink, each with its own battles, background and
  guardians (CircusMan, Colonel, BlastMan, ElementMan, JudgeMan, DiveMan).
  SpoutMan and TenguMan join Seaside's and Sky's guardians, and most areas
  alternate between two battle backgrounds.
  Every guardian leaves three HPMemory. Runs saved before this version carry
  over.
- Each layer places the game's own exit pads, Mystery Data, Normal Navis,
  Mr. Progs, the Net Dealer, the program vendor, the Chip Trader and the
  BugFrag Trader, all running on the game's NPC and text scripts. Chip
  Traders speak the game's own lines and hand out chips from its own prize
  pools, stronger with depth; deeper layers can hold a Chip Trader Special
  (10 chips for 1). Their prize is a chip new to the Library: the game's
  three in four from those it has, a whole story's there, gave a run's
  near-empty Library the same chip (BlastMan B) trade after trade.
- Random encounters from the game's own roll and its own formations: each
  area fights the battles of its original maps, 28 of the 29 virus families
  (all but WindBox) where Capcom put them, on their battlefields (grass, ice,
  holes, poison), with versions that grow with depth; viruses the story
  meets late (the dragons, Nightmare) wait for the later acts. Every third
  layer ends in a guardian's arena, staged after Hades' bosses
  (docs/BOSSES.md): a safe room with a heal and the Net Dealer before it;
  the arena seals, the guardian logs in over the boss prelude with a title
  card and lines that remember earlier battles, and the battle starts
  without a question. Deleted, it says a last word and logs out; its
  Guardian Data holds the reward and taking it makes the exit appear. An
  area-clear card and the next area's title card mark each act.
- Exits are the game's own warp pads: MegaMan jacks out and into the next
  layer, built while he jacks out. The Undernet and the Secret Area are
  entered the same way.
- Beaten Navis give their Cross, and the Graveyard's guardian Beast Out,
  through the story's own flags; the game's chat box says so.
- MegaMan stays in the run: R does not jack out, the PET's Save is off, and
  any other map sends him back to the layer.
- Choices on the game's text boxes: a strong virus signal, a dark flame into
  the Undernet, and a gate that three ScrtData open into the Secret Area.
- A project site on GitHub Pages in Battle Network's own interface: the title
  screen beside its menu, act cards over each section, a ChipFolder of
  screenshots, the Net Dealer for downloads (the latest release, 0 zenny)
  and the PET's E-Mail for the docs. The player moved to /play/. Screenshots
  of the running game (`build.py screenshots`) now appear in the README and
  on the site, and short videos (`build.py clips`: the title, the net, a
  battle, a guardian logging in, the Undernet, jacking in) play on the site
  while they are on screen; extracted assets stay out of the repository.
- A browser build on GitHub Pages (https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/): the same
  game in WebAssembly. The player chooses their ROM, which the page checks
  and keeps with the saves in the browser's IndexedDB, never uploaded.
- CI on every push and pull request: every target built with -Werror, the
  unit tests under AddressSanitizer and UBSan, script and workflow linting.
  A push to main publishes the browser build; a v* tag publishes a release
  with the PortMaster zip, the Linux archive and the site.
- A key or button press shorter than a frame now counts.
- A Linux desktop build: the game in a resizable window at a whole-number
  scale, F11 or Alt+Enter for fullscreen, keyboard and controllers, saves
  and the ROM in `~/.local/share/cyberworld-endless`. It is built on Debian
  bookworm with its own SDL2 (backends loaded at run time), so it runs on
  glibc 2.34 and newer. `build.py run` plays it here; `build.py release`
  writes the PortMaster zip and the Linux archive.
- A fair difficulty curve (docs/PROGRESSION.md). Depth, not the area, sets
  how hard a battle is: every random battle is sized from the ROM's virus HP
  and damage to its act's limits, at the highest version that fits, and the
  first battles of a run and after each guardian are gentler. The first act
  visits one of the gentler areas and the hardest come fourth; the Undernet
  now comes before the Graveyard. Guardians are chosen by HP for their act
  (BlastMan, DiveMan or SpoutMan first; Colonel from act 4), and the act's
  card names its guardian. Every act's second layer has a heal and the Net
  Dealer. A guardian leaves five HPMemory, its own Navi chip and a full
  heal; a Mr. Prog on the first layer offers a gift; rich Mystery Data may
  hold an HPMemory; a won Server challenge pays a chip. `build.py pacing`
  checks every act against its limits, and `runlog.txt` records each battle.
- Colonel fought as the enemy table's unnamed navi 17 (4000 HP at V1); he is
  navi 18, and saved runs move over.
- A title screen of its own in the game's style: the Battle Network logo from
  the ROM with an infinity mark where the 6 stood and a CYBERWORLD ENDLESS
  plate, over the battle backgrounds of the net's areas in turn (Central to
  the Cybeast Nest), animated and scrolling as in battle. CONTINUE shows the
  saved run's depth, the corner the best one; choosing either jacks in with
  the net rushing past into white. After the game's GAME OVER a run summary
  shows over the area where MegaMan was deleted.
- Checkpoints on arrival at each layer; CONTINUE returns there. A profile
  keeps the best depth. Run saves from before this version are converted.
- PortMaster launcher for ROCKNIX, made for the Retroid Nova and the Retroid
  Pocket Flip 2.
- Floor edges no longer step: every tile is tested against the layer's own
  floor (Sky's raised maps measured theirs higher), and a tile map
  avoids tiles that the original maps never set side by side, or whose floor
  stops on a tile's edge. Central Area's corners, Sky HP's faces and the Sky
  arena's lower edges were the worst. Where a catwalk meets a platform some
  joins still show.
- Layers keep to shapes the original maps draw: before its tiles are
  picked, a layer's floor fills notches, trims stray cells and widens
  walkways' bends and branches into small platforms, without changing how
  anything connects. About a third fewer tiles are approximated.
- Aquarium Comp and Mr. Weather Comp look like their originals: the
  Aquarium is a maze of water channels between rimmed glass pads (its
  platforms drawn as pads, none wider than one), and Mr. Weather's comp one
  great slab with rooms reached by its conveyor belts, now its walkways.
- Robot Control Comp's platforms are its long white slabs joined by circuit
  walkways, as in the original, instead of wide octagons around a hub.
- Dev tools (docs/DEVTOOLS.md): Select+R opens a dev menu for test runs (no
  random battles, can't die, one-hit enemies, up to 8x speed, win the battle,
  heal, zenny, the next layer or guardian, a chosen area). `build.py atlas`
  draws every area's layers with a report on their tiles; `build.py tour` has
  the game show every room of each area.
