# The Arena

The Arena (epic #120) is a NetBattle of BN6's own between two players.
Each client runs **both** linked BN6 cores, joined by mGBA's own cable
(its `GBASIOLockstep`), and only the players' controller keys cross the
network. This is the research's design ([ONLINE_FRAME.md](ONLINE_FRAME.md)):
raw serial over a network stalls ([ONLINE_LINK.md](ONLINE_LINK.md)), while
keys sent ahead by a few frames let BN6's Practice NetBattle finish
identically at 0 to 150 ms one way.

This page is the engine (the pair, the way in, a carried MegaMan,
lockstep play over a link, `--dev arena` with a local stand-in for the
peer) and its network half: the link over the presence server, and the
way in from the Net Square, where one player's MegaMan asks another's for
a Practice NetBattle in BN6's own chat boxes ([From the Net
Square](#from-the-net-square), below).

## Run it

```sh
python3 build.py run -- --dev arena                       # a window: you are the first GBA
python3 build.py shot --dev arena --frames 2400 --input "80:,4:RIGHT,8:,4:RIGHT,8:,4:RIGHT,8:,4:RIGHT,8:,4:RIGHT,8:,4:A,200:,110:B,30:,..." --shot 600:/src/.build/arena.bmp
```

`--dev arena` starts on the Arena instead of the title, by full rules
where BN5's ROM is read (its installs made on the game's core first, as
a run's start makes them), else by BN6's; the run's MegaMan is carried
from the data folder's checkpoint as the rules take him (the first
folder that fits). The player is the
cable's first GBA from the keyboard or pad; the second is a script: on
the Custom screen it takes the cursor to OK and presses it, in the battle
it stands still. Its keys come over the local link, held back as a
network would hold them. `arenadelay=N` sets the input delay in frames
(default 6, 3-60), `arenalag=N` the local link's one way in ticks
(default 3, 50 ms), and `arenahp=N` makes the carried MegaMan's HP N
(the installer seen in the battle). The battle's end, or A+B+SELECT+START,
goes back to the title; the log says each step (`arena:` lines).

## What runs

| Part | File | What it does |
| --- | --- | --- |
| The pair | `src/online/arena_pair.c` | Two cores of the player's ROM: as it is under BN6's rules (no hook, no BN5 install: every client builds the same pair whatever ROMs each player has), with BN5's installs under full rules ([Full rules](#full-rules), below); flash saves in memory, the cable's scheduling as the research harness had it; one frame of the pair, a hash of both cores |
| The installs' record | `src/emu/xinstall_log.c` | Every byte BN5's installs write into the game's own core's ROM copy and every hook they set, in their order, as the core starts: what full rules put in each Arena core |
| The Souls across | `src/online/arena_souls.c` | Under full rules, a Soul one core's player committed carried into the other core's Souls' block as the pair's frame ends: BN6's link carries the form, not the Soul |
| The way in | `src/online/arena_boot.c` | One core boots as `src/emu/boot.c` does (NEW GAME, the game's warp through its own call, hooked on that core alone); each side's MegaMan is written into a copy of its state; then the pair, on the cable, goes through BN6's menus (PET, Comm, Link Cable, NetBattle, Single Battle, Practice) with the research's scripted keys to the Custom screen. No state is kept or committed; it runs in slices behind a "connecting" picture |
| A carried MegaMan | `src/online/arena_loadout.c`, `arena_carry.c` | The `OnlineLoadout` (`online_snapshot.h`) written into BN6's own RAM through `bn6.h`'s names: the first folder, the pack's own ids, the NaviCust's board, HP (full) and base HP, the buster, Regular and TagChips, the Crosses' and Beast Out's flags. Checked by the battle's rules, never by the game's own ROMs, so both games check a MegaMan alike. Refused, with the reason, where the rules' cores cannot carry it: under BN6's rules chips past BN6's id 0x13F (in the folder or the pack's extension) or on one of BN6's blank records (203-220, 291-300, 314: `BN6_CHIP_BLANK`, where BN5's chips are installed on a game with its ROM); under BN6's rules Souls; under either a code past `*`, a max HP of 0 or past 9999. `arena_carry.c` reads the data folder's checkpoint, its `run.state` loaded into a plain core that never runs |
| Keys | `src/online/arena_input.c` | Both players' keys by frame in a ring; each stream in order, a resent frame left, a gap or a frame past the ring refused |
| The link | `src/online/arena_link.h`, `arena_link_local.c` | The interface below; the local stand-in |
| The engine | `src/online/arena.c` | The way in, then the battle in lockstep, the hashes compared, the end |
| The scene | `src/online/arena_scene.c`, `arena_text.c` | `--dev arena`'s scene, its words |
| The picture | `src/online/arena_view.c` | My core's picture, the notes over it, the way in's bar and its slices at the frame's pace; both scenes' |
| The wire | `src/online/arena_wire.c` | ARENA and its four messages packed and read as the server's `protocol.py` packs them; a carried MegaMan as bytes |
| The network's link | `src/online/arena_link_net.c` | `ArenaLink` over the transport's fast outbox: keys, hashes, the ready, the peer's end |
| From the Square | `src/online/square_arena.c`, `arena_words.c`, `online_mail.c` | The challenge, its question and answer, the battle with the Square's core held, the word after; MegaMan's lines; the inbox handed out by kind |

### Lockstep

A key sampled at tick *t* (a 60th of a second) is frame *t + D*'s, *D*
the link's delay; the first *D* frames are released keys on both sides,
on every client. Frame *N* of the pair is due at tick *N* and runs once
both players' keys for *N* and the three frames after it are known: a
linked core can cross into its next frame up to two frames before its
partner, and each core takes the keys of its own frame clock, as the
research found BN6 needs. So a delay of 3 frames plus the link's one-way
latency never waits; a shorter one waits, once, as many ticks as it falls
short, and stays that far behind. A tick catches up two frames at most.
Once the cable is idle (the menus after the battle) a core can run four
frames ahead of its partner; it then waits for it.

Every 60 frames of the pair, the pair's hash (FNV-1a over both cores'
whole states, their flash and the cable's numbers) is sent over the
link and the peer's for the same frame compared; a difference stops the
battle and says at which frame (`arena: stopped at frame N: the two
games went apart`). Each hash is also printed: two runs of the same keys
print the same.

### `ArenaLink`

```c
typedef struct ArenaLink ArenaLink;
struct ArenaLink {
	bool (*send)(ArenaLink *link, uint32_t frame, uint16_t keys);       /* my keys for frame (tick + delay), in order, once each */
	bool (*poll)(ArenaLink *link, uint32_t frame, uint16_t *keys);      /* the peer's keys for frame, once they have come */
	void (*send_hash)(ArenaLink *link, uint32_t frame, uint64_t hash);  /* my pair's hash after frame */
	bool (*poll_hash)(ArenaLink *link, uint32_t frame, uint64_t *hash); /* the peer's for that frame, once it has come */
	void (*tick)(ArenaLink *link);                                      /* a tick has begun: once a tick, before the sends */
	const char *(*broken)(ArenaLink *link);                             /* the link gone: its reason, else NULL */
	void (*close)(ArenaLink *link);
	unsigned side;    /* my player: 0 the cable's first GBA, 1 the second */
	unsigned delay;   /* D: frames from a key's sampling to its frame (3-60) */
};
```

The engine asks `poll` for the peer stream's next frame until it says
no; keys are ten bits (`ARENA_KEYS_MASK`). Both clients must agree on
`delay` and on who is side 0 before the battle (the network's half), and
both build the pair with the two loadouts in the same order:
`loadout[0]` side 0's, `loadout[1]` side 1's.

## Measured

On the Linux host in the build image, 18 October 2026, the player's
Gregar ROM, a fresh data folder, the script above (player 1 picks OK and
fires twelve charged shots; player 2 the local script):

| Delay / one way | Way in | Battle | Stalls | Hashes | Result |
| --- | --- | --- | --- | --- | --- |
| 6 frames / 3 ticks (50 ms) | 1,817 frames, 2,858 transfers | over at frame 1,881 | 0 | 31 agreed | 1 and 2 (won, lost) |
| 12 / 9 (150 ms) | the same | over at frame 1,887 | 0 | 31 | 1 and 2 |
| 4 / 5 (83 ms, short of 3 + 5) | the same | over at frame 1,879 | 4 | 31 | 1 and 2 |

The 6-frame runs printed the same hashes at every 60th frame on every
run (`18a9a8c5beb9b412` at frame 59), before and after the engine's
pacing changed. The whole headless run, the way in included, takes
about five seconds. With a checkpoint in the data folder its MegaMan was
carried (`arena: side 0: HP 300 ...` with `arenahp=300`), and the battle's
picture showed 300 HP against the second player's 100. The pictures stay
in `.build/`.

## From the Net Square

Facing another player's MegaMan, A reads his card and then offers
`Battle`, `Wave` and `More...`, the second box `Mute`, `Block` and `Back`
(three to a box, as BN6's chat box holds them). Battle sends ARENA's
challenge, naming its rules (Formats, below), and MegaMan says "BN6
rules! Let's see if NAME takes us on!". The server answers the challenger
WAITING and the other player CHALLENGED, each with the rules; his
MegaMan asks, once no chat is open: "NAME's MegaMan wants a Practice
NetBattle!", "BN6 rules: no Souls,and BN6 chips only.", then "Take
it,Lan?" with the cursor on No. Yes accepts; No, B, or a challenge from a
player he has blocked declines (the challenger's MegaMan: "Aw... No
NetBattle this time."). Before the challenge or the Yes goes, the folder
MegaMan carries is asked where Lan has a choice (Formats). A MegaMan the
rules leave nothing to carry, or one the last check refuses (a NaviCust
past what a message holds), does not go: MegaMan says so and no
challenge or acceptance is sent. The 3DS and the browser build, which
compile the Arena out (`CW_ARENA`), never offer Battle and decline a
challenge.

### Formats

Every NetBattle goes by the pair's common ground
([ONLINE.md](ONLINE.md#battle-network-5-there-or-not-the-pairs-common-ground)):
its rules are chosen by the challenger's game from both players' ROMs
(each peer's in the SNAPSHOT, `arena_format_for`), carried in ARENA's
challenge and every ARENA_STATE (`protocol.json`'s `format_bn6` 0 and
`format_full` 1), and named by both MegaMen before the battle. The server
refuses full rules unless both games have BN5's ROM (NOTICE 19), and a
game refuses rules it cannot fight by (`arena_format_ok`).

- **BN6 rules** (0): any pair. The pair cores are plain BN6. MegaMan's
  Souls stay home, and so do BN5's chips in his pack; he fights with a
  folder of BN6's own chips (`arena_format_carry`: the folder picked, its
  Regular and TagChips, as the one the core's Folder1 holds).
- **Full rules** (1): two games with BN5's ROM. BN5's chips go, in the
  folder and the pack's extension (every id of the chip registry, BN6's
  blanks among them), and so do MegaMan's Souls: UNITE on the Custom
  screen as in a run; both pair cores hold BN5's installs ([Full
  rules](#full-rules), below). The challenger hears "Full rules! Let's
  see if Beta takes us on!", the challenged game "Full rules: Souls and
  BN5 chips too!" before its Yes.

**The folder.** A run's MegaMan holds Folder1, Folder2 and the Extra
Folder (BN6's names; the second's Regular and TagChips at NaviStats
+0x2F and +0x58, bn6f's NaviStats; the Extra Folder has none of its own
here). `arena_folders_fitting` says which can go (an empty folder never).
A run holds Folder1 alone (no run hands out Folder2 or the Extra
Folder), so under BN6's rules its BN5 chips **stand in**
(`arena_standin.c`, the owner's decision of 18 October 2026, to be
refined): for that battle alone each sits out of the carried copy and a
chip of BN6's from the run's Pack takes its place, the same kind
(standard, Mega, Giga) and element where the Pack has a spare past the
folder's copies, its power the nearest, within BN6's folder rules (four
of a chip, the run's Mega and Giga limits, one of each); BN6's first
standard chip the folder has room for, Cannon, where it has none. A
Regular or TagChip whose chip sat out is let go. The carrying game alone
picks them (the other reads the folder as sent), each in the log
(`arena: chip 411 code 12 sits out, chip 1 code 0 stands in`). Where a
folder holds BN5's chips, or more than one can go, MegaMan asks in BN6's
chat box (`arena_folder.c`, flags `0x14A1`-`0x14A4`): "Folder1: 2 BN5
chips sit out,Lan. Chips from our Pack stand in for them, like Cannon A
for MrkCan1 M!", the Souls left home where he holds some, then "BN6
rules,Lan. Which folder goes?"; B sends nothing. Where MegaMan has one
folder of BN6's chips alone, it goes unasked. The carried MegaMan then
passes `arena_loadout_check`, the same last guard on both games. The
other folders show only with the `folder` dev step (DEVTOOLS.md).

START gives each game its side of the cable (the challenger's 0) and the
input delay. Each sends its MegaMan (ARENA_LOADOUT: the folder picked, the
NaviCust's placed programs and grid, max and base HP, the buster,
Regular and TagChips, Crosses, Souls and Beast Out; the pack stays home)
and builds the pair from both, its own as the other reads it, so that the
two pairs are the same byte for byte; the other's is checked with the
local refusals. The Square's core holds still meanwhile (the scene runs
the battle's frames in its place: `online_visit_held`), its connection
kept, MegaMan standing where he stood on the server's view.

After the way in each game says it is ready with the pair's hash at the
Custom screen (ARENA_HASH at frame 0xFFFFFFFF), and starts its ticks when
the other's has come, so both start within the link's one way; a
different hash there stops both before the first frame. My keys go a
frame a message as each tick samples them (two frames a message, tried
first, left the earlier side waiting half its ticks at a delay of 6:
`ARENA_NET_FLUSH`); where the transport's outbox is full they wait and go
together, eight a message at most, and a ninth breaks the link. The
pair's hash every 60 frames goes too, and a difference stops the battle
on both games with a plain word on the picture ("The NetBattle stopped:
the two games went apart by frame N") and MegaMan's after it ("Whoa! The
NetBattle broke off!").

The battle's end is the cable's first's to tell the server (ARENA over),
after its last keys; as is a stop both games see (the hashes apart),
lest the server answer the second with a notice that the battle is over.
A stop of one game alone (a link broken, the cable stuck, the reset
chord: back to the gate) leaves it. The server's ENDED to the other
game: its link spends the keys already come, the battle over on the same
frame there as here, and breaks after `ARENA_NET_GRACE` ticks
("the other player left the NetBattle"; MegaMan: "Huh? They're gone...").
The connection lost breaks it at once. The result shows on the picture
(A or B after half a second, or five seconds, goes back), then MegaMan
says it in the Square where he stood ("Alright! We won,Lan!!", "Ugh...
They got us." and "Next time,Lan!").

### Full rules

Each game runs both cores of the battle, so each installs BN5 into both
from its own BN5 ROM, the same way, and the two games' pairs stay the
same byte for byte. The installs are the game's own: as its core starts
(`src/emu/scene_emu.c`), every byte BN5's installs write into the core's
ROM copy and every hook they set is written down in their order
(`src/emu/xinstall_log.h`: `xinstall_core`'s tracks, the moved objects,
the Navis, the chips' code, the families, the sea and the obstacle
strike, then the chip registry's `xchips_install` and the Souls'
`soulproxy_install` and `soulunite_install`; measured 1,913,709 bytes and
132 hooks, 1,735,251 and 42 of them before the Souls'). Under full rules the boot core and both pair cores
take those bytes into their ROM copies before they start
(`arena_core_new`), and each pair core takes the hooks on its own table
(`hook_core`, after its state is loaded); the boot core meets none of
them, its ROM copy has the bytes without their BKPTs. Nothing is
installed again: the installers' own state (the moved code's places, the
chips' sit-outs) is the game core's, made once from the same ROMs.

Their RAM: an install that patches IWRAM code keeps it patched
(`emu_keep`) and patches the ROM image the game copies it from too
(`BN6_IWRAM_IMAGE`), so a core started on the recorded ROM has it from
its power-on. The record is the ROM's alone.

The hooks run on whichever core meets them, each reading and writing that
core (`hook_read*`, `hook_write*`), the two cores' frames run by one
thread an instruction at a time in turn. Each hook BN5's chips need is a
function of the CPU's registers and its core's memory, with no host state
but what its install read from the ROMs (the Souls' too): the deck's sit-outs
(`xchips_install.c`), BN5's green flash (`xchips_flash.c`), the moved
families' and Navis' names, pre-steps and delete counts, the Dark Soul's
region, the obstacle strike's test, the lava's and sea's panels, a
Soul's look, life, powers, passives, chips and mood. What a hook keeps
between two calls of its own (`xsea.c`'s and `xpanels.c`'s way back
through their stubs, a sea or lava panel's; Chaos Unison's charge
stepped, `soulchaos.c`), which both cores could meet at the same
instruction, is kept one a core (`hook_slot`), and so are the Souls UNITE
offers (`soulunite_held_core`: each pair core its own player's, from his
carried MegaMan).

Two games whose records differ (a game that took BN5 in after its core
started, another build) cannot fight: the record's hash is part of the
pair's (`arena_pair_hash`), so the ready at the Custom screen differs and
both stop before the first frame ("the two games went apart before the
battle"). A game whose core holds no record (BN5's ROM read but the
core's installs refused) is told so at the way in ("BN5's installs are
not in this game's core").

**The Souls across the cable.** UNITE stands in the pair's link battles
(`unite_stands`: a run's battles are never link ones, and the rule kept
them out before the pair could carry a Soul). Its commit writes the
turn's record (the proxy form, its chaos, its turns), which BN6's link
carries to the other GBA as it carries a Cross, and the Soul in the
Souls' block of the GBA whose player chose it, which BN6's link does
not carry. Both GBAs number the sides alike (`BN6_BATTLE_SIDE`: the
challenger's core 0, the other's 1, read on both pair cores), so the pair
carries the Soul: as each frame of the pair ends, a Soul one core's
player has committed (his side's `BN6_SOUL_NEXT`, its chaos, turns and
ColonelSoul's chip, the Souls used) goes into the other core's same side
(`arena_souls.c`), once a commit, before either GBA's turn takes it up;
both GBAs then unite the same side with the same Soul (read on both
cores: form 8, Soul 11, its used bit 0x800, from the same frame on). The
Soul's sprite is drawn by the side its MegaMan stands on (`soullook.c`:
the sprite selector's caller's r5 where it is a side's player, else
MegaMan's own side, as before), so the other GBA draws him in his Soul
too. The 52 differentials of the Souls and Chaos Unison
(`build.py xdiff soul_* chaos_*`) pass with the change.

**The proofs** (18 October 2026, the player's Gregar and Team Colonel
ROMs). `--dev arena` with a checkpoint whose Folder1 is thirty MrkCan1 M
(`--scene emu --dev folder=411`, then `--dev arena` on the same data
folder): "full rules: BN5's installs in every core, 1913709 bytes and 132
hooks (record 1efe82b0894607e2)", the same record a run's start makes,
both cores at the Custom screen after 1,817 frames, side 0's
folder `199b` (MrkCan1 M), the Custom screen showing MrkCan1 with BN5's
picture and 70; picked and used, its marker locked on the other MegaMan
and the cannon took him from 100 to 30, a second MrkCan1 the next turn
deleted him ("MEGAMAN WIN!"): over at frame 1,502, result 1 and 2, 24
hashes. With a checkpoint holding every Soul and a folder of thirty
JustcOne J (`--dev souls=0x3F,folder=140`): A on the first JustcOne, OK,
UP to the emblem's place, now UNITE, lit, A, then OK: KnightMan's
summon, MegaMan in KnightSoul with his three turns; both pair cores read form 8 and Soul 11 on side 0, its used bit 0x800,
from the same frame on, and the second core's picture showed the summon
and side 0 in KnightSoul as well.

Two clients on a local server (the harness of [the two-client
proof](#the-two-client-proof), below, both games with BN5's ROM beside
BN6's, seed 120's Square, `arenahp=100`, the `chip` dev step's `chip 411
M 30` making Alpha's Folder1 thirty MrkCan1 M): Alpha read Beta's card
("His PET holds BN5 chips too!"), chose Battle and said "Full rules!
Let's see if Beta takes us on!"; Beta heard "Alpha's MegaMan wants a
Practice NetBattle!" and "Full rules: Souls and BN5 chips too!", and said
Yes. Both games printed the same record (1,913,709 bytes, 132 hooks,
`1efe82b0894607e2`), both pairs reached the Custom screen after 1,817
frames with Alpha's folder `199b` (MrkCan1 M) on side 0, both sent the
same ready hash (`f5111e6c47a0e5b1`) and the same hash at every 60th
frame (22 agreed). Alpha's MrkCan1, named on both screens, took Beta from
100 to 30 on both; charged shots ended it, over at frame 1,336 on both,
Alpha's result 1 and Beta's 2, both back in the Square. Alpha holding
every Soul (`souls=0x3F`) and thirty JustcOne J (`chip 140 J 30`), Beta
none: UNITE lit KnightSoul on Alpha's Custom screen, "arena: side 0's
Soul 11 (its turns 3) carried to the other GBA" on both games, and on
both screens KnightMan's summon, Alpha in KnightSoul with his three
turns and his wrecking ball's hits (Beta from 100 to 98; Beta's charged
shots took Alpha down), the same ready hash (`1a03915af762605a`)
and 36 hashes agreeing to the end, over at frame 2,163, Beta's result 1
and Alpha's 2. With both folders MrkCan1 (the build before the Souls':
ready `e000d63d5380d5e0`, 38 hashes agreed, over at frame 2,311 by
charged shots) Alpha used his while Beta stood a panel away and moved,
and its marker wandered without a lock, the same on both games. The marker is BN5's code moved (the differential `chip_mrkcan1`
passes, and with its foe a panel away as well, hit in both engines); a
target that moves is not in a differential yet. With Beta's copy of
Alpha's keys changed from frame 300 (`arenadesync=300`): both stopped,
apart by frame 359. With Alpha's core made without BN5's families
(`--dev nofamilies`: a record of 1,571,997 bytes and 115 hooks,
`3cc9e56dc3808e94`): the ready hashes differed and both stopped before
the first frame ("the two games went apart before the battle").

### The two-client proof

Two host binaries on this machine (`SDL_VIDEODRIVER=dummy
SDL_AUDIODRIVER=dummy`), each with its own `--data-dir` and a
`network.ini` naming a local server (`python3 server/build.py`, then
`build/server/cyberworld-server.pyz --config` with a free port and its
`state_dir` under `.build/`), BN6 alone in `--rom-dir`, `--scene home
--run-depth 7 --seed 3 --dev quiet,arenahp=40` (`arenahp=N` makes each
carried MegaMan's max HP N, so the battle is short). Beta starts first
(session 1, arriving at -24,364), Alpha a second later; after the gate's
steps Alpha is put beside Beta (`place -8 364 5`), reads his card (A
three times) and picks Battle (A); Beta, 635 frames on, presses A, LEFT
(to Yes: the question's options lie side by side) and A. Both then press
RIGHT five times and A six times over (the Custom screen's OK) and fire
charged shots (B held 110 frames). The harness stays in `.build/`.

18 October 2026, the player's Gregar ROM, delay 6 (the server's least):

| Run | Way in | Ready | Battle | Stalls (side 0 / 1) | Hashes | Result |
| --- | --- | --- | --- | --- | --- | --- |
| Two keys a message | 1,817 frames on both | the same hash `fd2fa7d630cf9080` on both | over at frame 1,271 | 636 / 0 | 21 agreed | 2 and 1 (Beta won) |
| A key a message | the same | the same | over at frame 1,270 | 2 / 0 | 21 | 2 and 1 |
| The same, three pairs of games at once | the same | the same | over at frame 1,070 | 0 / 131 | 17 | 2 and 1 |
| `arenadesync=300` on Beta (Alpha's keys from frame 300 changed there) | the same | the same | stopped on both: apart by frame 359 (299 agreed) | | 5 | the stop's note, then MegaMan's word |
| Alpha's reset chord mid-battle | the same | the same | Beta stopped at frame 309: the other player left | | | Alpha at the gate ("We're home,Lan!"), Beta in the Square ("Huh? They're gone...") |

Both games printed the same hashes at every 60th frame. The two
processes' wall clocks put their battles' start within 20 ms and their
ends within 40 ms of each other (15 s of battle). Back in the Square,
both MegaMen stood where they had stood, the loser's word and the
winner's in their chat boxes.

**A mixed pair, BN6 rules** (the same day, protocol v4). Beta with BN5's
ROM beside BN6's, Alpha with BN6's alone, both through the gate as above
(seed 120's Square), each with Folder1 copied into Folder2 by the
`folder 2` dev step and Beta's Folder1 given MrkCan1 M (`chip 411 M 1`)
before it; `arenahp=40`. Alpha read Beta's card ("His PET holds BN5
chips too!"), chose Battle, was asked "BN6 rules,Lan. Which folder
goes?" between Folder1 and Folder2 and took Folder1 ("BN6 rules! Let's
see if Beta takes us on!"). Beta heard "BN6 rules: no Souls,and BN6
chips only.", said Yes, then "Folder1 has BN5 chips. It stays home." with
Folder2 alone offered, and took it (`arena: we took Alpha's challenge,
rules 0, folder 2`). The pair's hash at the Custom screen was the same on
both (`fd2fa7d630cf9080`), the battle over at frame 1,568 with 26 hashes
agreeing, Beta's MegaMan the winner, both back in the Square.

## Not yet

- The battle's sound: the scene plays the player's core only where the
  game's own core has made its sound ring (`arena_pair_sound`).
- The NaviCust's programs are written as the board; BN6 makes HP from
  the base HP and the board again in the menus (seen: the max HP alone
  was put back to 100), but the other programs' effects in a NetBattle
  are not yet checked one by one.
- After the battle the engine stops once both cores have left it; the
  game's own way back to the Comm menu is not shown.
- Over a real network: the server's delay is 3 frames, the one way
  and 2 more (6 at least); a phase of up to a tick between the two
  games' clocks comes out of that margin, and stalls show on the
  picture after 20 ticks ("Waiting for the other PET").
- The Souls under full rules beyond KnightSoul's unison and its charged
  shot ([Full rules](#full-rules), above): the other five, Chaos Unison
  and ColonelSoul's button in a NetBattle, and BN5's mood across the
  cable, each still to be seen on both GBAs; and both players uniting in
  one battle (the second GBA's Soul carried to the first was seen in the
  logs, "side 1's Soul 11 (its turns 3) carried to the other GBA" on
  both games with every hash agreeing, its pictures not yet).
- A referee Mr.Prog in the Square's Arena wing, rematches, Battle (not
  Practice) and its chip forfeit, the pack
  (it stays home), and any platform but the desktops: the 3DS and the
  browser build compile the Arena out (`CW_ARENA`), Android and iOS build
  it but are untried.
