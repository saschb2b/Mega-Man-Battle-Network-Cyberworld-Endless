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
| The stands | `src/online/arena_watch.c`, `watch_keys.c`, `square_watch.c`, `arena_report.c`, `stands_report.c`, `stands_wire.c`, `online_stands.c`, `stands_words.c`, `stands_text.c` | A battle watched: its pair built and run from both players' keys; the referee's offer and the watching screen; the report the cable's first sends and the channel's battles as the server tells them ([The stands](#the-stands), below) |

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
frames ahead of its partner; it then waits for it. My keys run at most
128 frames (half `ARENA_RING`) past the pair's next frame: past that a
tick samples none and waits, so the other game held still for seconds
(its app sent to the background, its window dragged) stalls the battle on
both and it goes on when that game does. Before this (issue #129, 19
October 2026), the waiting game kept sampling a key a tick, filled its
256-frame ring about four seconds after the other stopped, and stopped
the battle with "my keys could not be sent"; the other, on its return,
with "the other player's keys came out of order".

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

### Rematches and the referee

A battle with a result (issue #128) is offered again. Back in the
Square, MegaMan's word on it ("Alright! We won,Lan!!") goes on to
"Again with NAME?", the cursor on No, once no chat is open and while
NAME is still in the Square. Nothing is won or lost, so the question is
the whole cost: the default lets either player go (the Square's pull
between staying together and moving on), and both must say Yes.

No new message: a rematch is ARENA's challenge again, by the last
battle's rules, answered at once. Lan's Yes sends it quietly (nothing
changed, so MegaMan names no rules); the other game takes a challenge
from that player by those rules at once where its player said Yes too,
turns it down at once where he said No (or B), and, his question still
open or the result still on the picture, holds it for the answer. Both
Yeses at once cross on the server, which turns the second challenge down
as busy; the first is taken (`again_in`, `square_arena.c`). The folder
carried is the last battle's, asked again only where it no longer fits.
An answer is kept 45 seconds from the battle's end (`AGAIN_MS`); a
challenge then is an ordinary one, asked as ever. A Yes nobody answers
ends as any challenge does, after the server's 30 seconds ("Aw... No
NetBattle this time."). A battle stopped (a desync, a leave) offers none.
An older build, which knows no rematch, sees an ordinary challenge.

The referee Mr.Prog by the ring keeps what this game saw: before any
bout he tells how a NetBattle is asked for; after one, who beat whom
("LAST BOUT: YOU BEAT ALPHA!", "LAST BOUT: ALPHA BEAT YOU!") and, from a
second bout with the same player on, the count ("THAT'S 2 TO 1. YOU
LEAD!", "1 TO 1! ALL EVEN,ALL EVEN!"). As MegaMan comes back he calls
the bout over his head ("YOU WIN!", "ALPHA WINS!"), as the greeter calls
a log-in. A new opponent starts the count again; it is this game's
alone, kept while the game runs.

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
both stop before the first frame. Each game sends its record's hash just
before its ready (ARENA_HASH at frame 0xFFFFFFFE, `arena_link_net_record`;
the server relays it as any hash), so a ready apart whose records differ
says so on both games: "the two games installed BN5 apart (one took BN5
in after it started, or another build)" on the picture, then MegaMan's
"Huh? Our PETs don't match on BN5..." and "Lan,let's reboot the PET and
try again!"; a ready apart with the same records (or from a game that
sends none) keeps "the two games went apart before the battle". A game
whose other player left after his ready came apart from ours tells the
same cause, not "the other player left". A game whose core holds no
record (BN5's ROM read but the core's installs refused) is told so at
the way in ("BN5's installs are not in this game's core").

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

**What only one GBA writes** (issue #130). The Souls' block holds a few
things a GBA writes for its own player alone, as its own Custom screen
opens or closes; a pair holds both GBAs, so each was looked at for
whether the other GBA reads it (both pair cores' blocks compared every
frame through whole battles, a check kept out of the build):

- **The Soul's end.** The Custom screen's stand hook marked a screen
  opened with the Soul on (`BN6_SOUL_SCREEN`, which the end waits for)
  for its own player only, so the other GBA never ended his Soul: seen
  with ColonelSoul (`--dev arena`), side 0's Soul 0 on its own GBA and still 7 on the
  other's after the turn its three turns ran out. The hook now marks it
  for every side with a Soul on, before it asks whether this GBA's own
  player holds any (`soulunite.c`'s stand): in a link battle both
  players' screens open together. Seen since: the Soul gone on both GBAs
  at the same turn (a frame apart, as their cores run) and on both
  screens.
- **ColonelSoul's button.** The chip his OK keeps in the button
  (`BN6_SOUL_NEXT_CHIP`, `soulcolonel.c`) is his own GBA's alone, and BN6's
  link knows no button; the turn's start makes his charged shot the chip
  shot where it holds one, ScreenDivide where not, on each GBA from its
  own block. Seen without the carry (`--dev arena`): his GBA's block held
  MiniBomb (54), the other's 0, and from the turn's start his chip 54 on
  his own GBA and 0 on the other's, so his own screen showed the bomb
  thrown while the other GBA ran ScreenDivide: the two GBAs' battles
  apart, though both clients' pairs, alike, hashed the same. The pair
  now carries it as it carries a Soul, wherever it differs ("arena: side
  0's ColonelSoul chip 1 carried to the other GBA", then 0 at a screen
  with no chip).
- **BN5's mood** (`BN6_SOUL_MOOD`, its opening mark, the stand's and
  the slot's bytes): each GBA opens its own player's as its own Custom
  screen opens and reads it for him alone (UNITE's stand, the worried
  rule's DarkChips at the hand's end, `soulmood.c` and `soulunite.c`);
  the other side's stays 0 there and is never read (BN5's writers write
  nothing at 0). What it decides crosses as BN6's link carries a pick,
  and as the pair carries a Soul. A DarkChip used darkens its user on
  both GBAs (`soulmood_chip_used`, run where the chip is used, on both).
  Seen: side 0's mood 0x9E on its own GBA, side 1's 0x3A (worried) on
  its own, the rest of the block the same on both through the battle.
- **The Dark Soul's chip lists** (`BN6_DARK_LISTS`, what a failed Chaos
  Unison's Dark Soul and BN5's DarkInvs play, `soulchaos.c`). Each GBA
  made a side's list, as the Dark Soul or DarkInvs came, from its own
  player's folder, so the other GBA made it from the wrong one. BN5 makes
  each player's from his own save and its link exchange carries it at
  the battle's setup (`BN5_DARK_LISTS`, `0x08009B92`); BN6's link carries
  none. Seen (`--dev arena`, NumberSoul by DarkPlus, B released 70 frames
  into its charge, past phase 2): side 0's list DarkPlus (0x122) on his
  own GBA, Cannon (1, the other player's) on the other; then the Dark
  Soul at (6,2) on one and (6,1) on the other and MegaMan's HP 294 on one
  and 254 on the other: the two GBAs' battles apart, both clients' pairs
  alike. The pair now makes each side's list on its own player's GBA as
  both cores reach the first Custom screen and writes it into both
  (`arena_souls_lists`: "arena: side 0's chips for a Dark Soul, 1 of its
  folder's, on both GBAs"), and in a link battle the hook fills none
  (`soulchaos_lists_fill`). The two GBAs the same since, through the Dark
  Soul's stay.

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
passes, and with its foe a panel away as well, hit in both engines).
Against a target that moves, `chip_mrkcan1_moving` (19 October 2026:
BN5's Handi free, teleporting from (6,2) to (4,1) as the mark comes up)
plays the same in both engines: the mark, its lock on Handi's new panel
and the cannon's 70 at the same frame, the hit a counter in both; it is
held to its recorded FAIL at t75, Full Synchro's own object each
engine draws its own way (no other difference, `--survey`). So BN6's
moved code does what BN5's does with a target that moves; the
NetBattle's wandering marker, the same on both games, was not put
right, for nothing in it differs from BN5's yet seen. With Beta's copy of
Alpha's keys changed from frame 300 (`arenadesync=300`): both stopped,
apart by frame 359. With Alpha's core made without BN5's families
(`--dev nofamilies`: a record of 1,571,997 bytes and 115 hooks,
`3cc9e56dc3808e94`): the ready hashes differed and both stopped before
the first frame ("the two games went apart before the battle"; since
issue #130 "the two games installed BN5 apart", on both games, 19
October 2026).

**Every Soul, both players, Chaos Unison and ColonelSoul's button**
(19 October 2026, issue #130, the two-client harness above with
`souls=0x3F` and `arenahp=300` on both games, each Folder1 thirty of a
Soul's chip by the `chip` step: RockCube, Invisibl, BusterUp, Boomer,
BblWrap, JustcOne). The same record (`1efe82b0894607e2`) and every hash
agreeing on both games in each run:

| Run | Alpha (side 0) | Beta (side 1) | Seen on both screens | Hashes |
| --- | --- | --- | --- | --- |
| Each Soul alone | ColonelSoul, ShadowSoul, NumberSoul, TomahawkSoul, ToadSoul at the first screen | Cannon A | the summon (the Soul's Navi over MegaMan, on the other GBA from the other side), MegaMan in the Soul, its three turns; ScreenDivide's 50 a hit (ColonelSoul won at frame 2,668), the slash, the dice bomb, the whole field grass and the swing, the notes' 30 and their paralysis | 45, 57, 56, 57, 57 |
| Both uniting | ShadowSoul, NumberSoul, KnightSoul at the first screen | TomahawkSoul, ToadSoul, ColonelSoul at the second | both Souls at once, each GBA carrying its own player's ("side 1's Soul 10 (its turns 3) carried"), the second summon on the first player's screen | 74 each |
| ColonelSoul's button | ColonelSoul (three RockCubes in a folder of Cannon A), a Cannon put in the button at the second screen | Cannon A | the Cannon fired as his charged shot (Beta 250 to 210), ScreenDivide the turn after (no chip), the Soul ended at the fourth turn's start on both | 94 |
| Chaos Unison | ColonelSoul by a DrkSonic, NumberSoul by a DarkPlus (thirty of it) | Cannon A | the Chaos picture and the summon, the Soul in its dark palette, one turn; ColonelSoul's charged B its DrkSonic on Beta's screen | 67, 67 |

Each run's Souls' blocks were compared between the two pair cores every
frame (a check kept out of the build): the same but for each GBA's own
player's mood, stand and slot (above) and a frame's lag as the two cores
take a commit or a turn in turn; the Soul's end and the button's chip
apart until the two fixes above.

**What was left of them** (19 October 2026, issue #130, the same harness;
a full-rules `--dev arena` where named). Each run's two pair cores were
compared every frame, now wider (a check kept out of the build): the
battle's objects, both MegaMen's objects, AI data and collision data,
the panels, the Souls' block and the Dark Soul's lists, the same on both
but for each GBA's drawing of its own mirrored field (a MegaMan's +0x20,
+0x9A to +0x9F, +0xA3 and +0xA5), each GBA's own player's mood, stand and
slot, and a frame's lag; every hash agreed on both games.

| Run | Alpha (side 0) | Beta (side 1) | Seen on both screens | Hashes |
| --- | --- | --- | --- | --- |
| A failed Chaos Unison | NumberSoul by DarkPlus, B released 70 frames into its charge | Cannon A | the failure's flash, Alpha out of the Soul, the Dark Soul at the back of Beta's area firing at Alpha (300 to 243); before the lists' fix above, the two GBAs apart | 67 |
| Chaos Unison's attacks | ShadowSoul by DarkInvs, TomahawkSoul by DrkLance, KnightSoul by DarkDril, ToadSoul by DarkWide (thirty of each), B held 45 frames | Cannon A | DarkInvs: Alpha under BN5's chip AI, his LongSwrd, MegEnBom and HiCannon taking Beta from 300 to 14; DrkLance: the lances up Beta's back column, Beta at the front untouched, Alpha healing on his Soul's grass; DarkDril: Beta 300 to 200; DarkWide: Beta deleted from 300, over at frame 1,365 | 35, 67, 67, 23 |
| The passives | ShadowSoul, ToadSoul at the first screen | Cannon A | ShadowSoul's FloatShoe (his collision flags' 0x20, `BN6_OBJ_FLOATS`) and ToadSoul's flag (his AI data +0x48's 0x20) set as each unites, on both cores at the same frame; TomahawkSoul's grass above | 57, 57 |
| NumberSoul's hand of ten (`--dev arena`) | NumberSoul (BusterUp in fifteen of thirty), its second screen's two rows of five: the second row's first two picked, OK | the script's | CrakShot from the second row as "CrakShot 60+10", its 70 taking the second player from 100 to 30, then the BusterUp beside it; over at frame 3,883 | 64 |
| A Soul's chip charge | KnightSoul (thirty JustcOne J, two picked, the second given up for UNITE), A held 130 frames in its turn | Cannon A | "JustcOne440", the chip charged and doubled, Beta deleted; over at frame 1,507. ToadSoul's (BblWrap, a barrier: no number to see) the same on both GBAs | 25, 64 |
| BN5's DarkChips, worried | thirty Cannon A, the first four DarkDril D; his mood held worried on his own GBA once the opening set it (a step kept out of the build, as `--dev worried` holds it on the game's core) | Cannon A | the first screen none of them; the second dealt DarkDril at the hand's end, its fifth, the cursor on it and the screen dimmed as BN5's; picked, OK, used: "DarkDril 100" named and the drill on both screens, Alpha dark (his emotion window), the third screen dealing the next DarkDril at its end | 81, 65 |
| MrkCan1 against a MegaMan who steps | thirty MrkCan1 M, used each turn | stepping up and down every 32 frames | the mark locked on Beta and the cannon took him from 300 to 230 | 64 |

The Pack stays home in a NetBattle, so what a worried MegaMan is offered
is the carried folder's: its BN5 DarkChips are shelved as in a run's
battle and one comes to the hand's end while he worries or is dark
(`soulmood.c`'s worried rule, on his own GBA's Custom screen; the pick
crosses as any). His mood opens calm in a NetBattle, both MegaMen at
full HP (the opening's rule makes him worried at a quarter of his HP or
less), and hits lower it as BN5's do. A DarkChip used there takes its
own after-effect, as in a run's battle. BN5's dispatcher of them
(`0x0802C938`) gives the HP bug a step in its place where the battle
state's bit 0x40 is set (`0x0802C95E`, through `0x080090E8`), which
`soulmood.c` reads as BN5's link battle's; BN6's twin bit (`0x0800A8F8`
through `0x0802E09A`) is clear in the Arena's NetBattles, read on both
cores through the worried run, so nothing there asks for BN5's other
rule (what sets BN5's bit in its own NetBattles is not yet read:
docs/ROM_DATA.md, BN5's DarkChips' after-effects).

MrkCan1: BN5's own link battle, MegaMan against MegaMan, is out of the
differential's reach (it runs one guest core, no cable). Its nearest is
`chip_mrkcan1_darksoul`: the Dark Soul, MegaMan's body under BN5's chip
AI, steps at t39 and t76 while the mark sweeps from t32 to t84, and
neither engine's mark locks or hits, frame for frame (PASS over 600
frames). So BN6's moved code does what BN5's does against a target that
steps; in a NetBattle it has locked on a stepping MegaMan (above) and
wandered without a lock (the earlier run), the same on both games each
time.

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

**A rematch** (19 October 2026, issue #128, protocol v5 unchanged). Two
BN6 games, `arenahp=10`, the harness above: Alpha challenged, Beta took
it, and the battle was over at frame 848 (Alpha won, 14 hashes
agreeing). Back in the Square each referee called it ("YOU WIN!" on
Alpha's game, "ALPHA WINS!" on Beta's) and each MegaMan asked "Again
with NAME?", the cursor on No. Beta's LEFT and A said Yes and sent the
challenge; Alpha's question was still open, so his game held it
(`arena: Beta asks for a rematch (match 2)`) until his RIGHT and A said
Yes, and took it at once. The second battle's pair hash at the Custom
screen was the first's (`8dc89c5dad9af3e7`: the same rules and folders),
and it was over at frame 1,328 (Beta won, 22 hashes agreeing; all 36 the
same on both games). The referees then called it 1 to 1 on both. Declined:
the same first battle, then Beta's B on the question (No); Alpha's Yes
came after it, and Beta's game turned his challenge down at once (`arena:
Alpha's rematch turned down`), Alpha's MegaMan saying "Aw... No NetBattle
this time.".

## The stands

The owner's decision on issue #128 (19 October 2026): a NetBattle in the
Square is watched both ways. A game that can build the battle's pair
**watches it for real**, the same two cores run from the players' keys a
second or two behind them; every game, one that cannot watch among them
(the 3DS, the browser, a game without BN5's ROM before a battle by full
rules, a machine too slow to catch up), gets **the report**: each
MegaMan's HP, the last chip each used and the winner, from the referee by
the ring and on the Board.

The design (the game-design skill): the Square's dialectic, **gather
against go**, and the session loop (arrive, see who is here, do one thing
together, read what happened, go home). Watching is the one thing to do
together that needs no partner of one's own: it shows other players'
folders and plays (learning by watching, social proof), a diegetic
spectacle in the Arena's wing, the referee its barker. The patterns:
spectating as presence (the stands fill: "2 NAVIS WATCHING!"), cheers
that reach the players afterwards rather than during (a crowd felt, never
a coach), and a report for everyone so no system is left at the door. The
stands feed nothing into a run: no reward, no power, and nothing a
spectator sends reaches the battle.

### Watching

The server writes down every started battle as it relays it
(`server/cyberworld_server/stands.py`): both MegaMen (ARENA_LOADOUT), the
installs' record and the ready (ARENA_HASH at 0xFFFFFFFE and 0xFFFFFFFF),
the cable's first's hash every 60 frames, and both players' keys from
frame 0 (a side's first keys come at its input delay's frame, the frames
before released keys on every game). A player of the channel who asks
(WATCH) is sent the battle's setup (WATCH_STATE: the two sessions, the
rules, the delay), then the players' messages (WATCH_RELAY) and every key
from the first frame (WATCH_KEYS, 240 frames a message at most), at most
2 KB a tick and only while his output is under 4 KB, so a spectator who
came late catches up without the players' relay ever waiting on him. A
gap in a side's keys, or a battle past 20 minutes, stops its watching;
the battle itself goes on.

The spectator's game (`src/online/arena_watch.c`) builds the same pair a
player's does (`arena_boot`, both carried MegaMen as their bytes came, by
the battle's rules; under full rules BN5's installs from its own record,
so only a game with BN5's ROM is offered a full-rules battle, the pair's
common ground). At the Custom screen its pair's hash must be the players'
ready, or it stops ("the two PETs here came out apart from theirs";
"this PET's BN5 is not theirs" where the installs' records differ). Then
it runs the pair as `arena.c` does, frame N once both sides' keys for N
and the three frames after it are in (`watch_keys.c` keeps the whole
battle's keys, about 240 bytes a second), each core on its own frame
clock. More than 110 frames behind the frames whose keys it has (BEHIND_KEPT and
BEHIND_LEAST), it catches up, silent: as many frames a tick as 12 ms allow (eight
a tick headless, so a capture's frames are the same on every machine),
until it is 90 behind; from there it runs one a tick with the watched
side's sound, and within 20 it waits. So it stays a second and a half
behind and rarely stalls; the keys' end (the battle over) runs the rest
as fast. Catching up that
gains nothing in ten seconds gives up ("this PET is too slow to catch
up"), and the referee does not offer that battle again: the next one.
Every 60 frames its pair's hash is checked against the cable's first's
for the same frame; a difference stops the watching ("the games went
apart by frame N"). The keys spent once the server has said the battle
is over, without both cores out of it, end it as broken off.

In the Square (`src/online/square_watch.c`, its words in
`stands_words.c` and `stands_text.c`): the referee's archive is his
words, then the channel's battle as the report says it, then, where this
game can watch it, "WANNA WATCH?" with the cursor on Yes. Yes sets the
Square's flag `0x14C4`; as the chat closes the game sends WATCH and, on
WATCH_STATE, holds the Square's core as a battle of its own does
(`online_visit_held`): the screen is the pair's picture, the cable's
first's side (its heading and keys shown four seconds: "WATCHING ALPHA VS
BETA", "R: other side  L: cheer  B: leave"; "Catching up..." while far
behind). R shows the other GBA's picture, L sends the Square's Cheer
emote, B leaves at once; the end shows "ALPHA won the NetBattle." until A
or B, or five seconds, and MegaMan is back where he stood. A spectator is
busy for the server: a challenge to him is refused (NOTICE 16), and his
game sends nothing of the battle.

### The report and the cheers

The cable's first game reads both MegaMen off its pair's first core each
tick (`arena_report.c`; `stands_report.c` the reading, ROM-free): each
side's player object (`BN6_BATTLE_PLAYERS`, the same on both GBAs, which
number the sides alike), its HP and max HP (`BN6_T1_HP`, `BN6_T1_MAX_HP`),
and a chip counted as used where his action turns to a chip's
(`BN6_T1_ACTION` 0x12-0x16) and the chip he uses next (`BN6_T1_CHIP`, the
hand's first) gives way the same frame: seen in `--dev arena` (MiniBomb,
54, picked and used: the field 0x36 until the action's 0x12, 0xFFFF
from that frame; a charged shot's action is 0x16 with no chip, not
counted). It sends what changed every 30 frames at most (ARENA_STATUS)
and the winner as the battle ends, before ARENA's over; `--dev arena`
prints it at the end ("arena: the report: HP 100/100 and 0/100, chips 1
and 0 (last 54 and 0), winner 1"). It only reads.

The server tells each channel its battles (STANDS) as they begin and end
and at most once a second as they change: who fights whom, the rules,
how many watch and cheer, the report. The referee says it in BN6's
Mr.Prog voice ("A NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 40 HP!
BETA HAS 20!", "ALPHA USED MINIBOMB!", "2 NAVIS WATCHING!", then "ALPHA
BEAT BETA! WHAT A BOUT!" and the cheers), and the Board posts it
(">NetBattle now! Alpha vs Beta", ">Alpha beat Beta! 3 cheers."). A
chip of BN5's is named only by a game with BN5's ROM; another leaves its
line out.

A phrase or emote said in the channel while a battle is on is a cheer:
the server counts it for the battle and holds its EVENT from the battle's
two players, and STANDS too until their battle is over. Back in the
Square, the referee tells them: "THE STANDS CHEERED FOR YOU! 3 CHEERS!".
A spectator cheers with L while he watches, or with the phrase chat in
the Square as anyone does.

### The three-game proof

20 October 2026, the player's Gregar ROM (and Team Colonel's where
named), protocol v7, a local server with seed 120's Square, three host
binaries headless (`SDL_VIDEODRIVER=dummy`), each with its own data
folder and `network.ini`, from `--scene home --run-depth 7 --seed 3`, the
harness of [the two-client proof](#the-two-client-proof) grown to three
(kept in `.build/`): Beta placed at 28,404 and Alpha beside him; Alpha
challenges, Beta says Yes, both pick a chip and OK and fire charged
shots, `arenahp=150`; Gamma, BN6's ROM alone, walks to the referee
(at -428,44: MegaMan at -428,28 facing him) and presses A through his
pages.

| Run | Gamma came in | Gamma's pair | Caught up | The battle's end | Hashes |
| --- | --- | --- | --- | --- | --- |
| Watched from the way in | as the players' pairs reached their Custom screen | the players' ready (`0f5e9abea55dced4`) | from 708 frames behind to 90 at frame 773 | frame 2,506 on all three, Alpha's win (the cable's first's result 1) | 41 agreed between the players, 41 checked by Gamma |
| The same, an earlier build | the same | the same | from 785 to 90 at frame 863 | frame 2,915 on all three | 48 and 48 |
| A spectator late | 28 seconds into the battle | the same | from 1,686 frames behind to 90 at frame 2,028, live after | frame 2,506 on all three | 41 and 41 |
| Gamma's copy changed (`arenadesync=1200` on Gamma) | at the way in | the same | at frame 841 | Gamma stopped at frame 1,260 ("the games went apart by frame 1259"), the players fought on to 2,506 | 41 agreed between the players |

On Gamma's screen: the referee's "A NETBATTLE'S ON! ALPHA VS BETA!",
"WANNA WATCH?" with the cursor on Yes, "TO THE STANDS! ENJOY THE BOUT!",
then the battle's Custom screen with "WATCHING Alpha VS Beta" and
"Catching up...", the battle with its keys shown four seconds, R turning
the picture to Beta's GBA and back (the HP in the corner each player's
own), and "Alpha won the NetBattle." at the end; back in the Square on
the referee's panel. The players stalled 0 to 6 ticks a battle, as
without a spectator, but for one run while the machine built other
targets for other work (402 ticks on Beta's game, the spectator's own
pace unchanged). Gamma's three L's while watching came back as three
cheers: the Board's "Alpha beat Beta! 3 cheers." and, to Alpha at the
referee after his battle, "LAST BOUT: YOU BEAT Beta!", then "THE STANDS
CHEERED FOR YOU! 3 CHEERS!" (the server holds a cheer's EVENT from the
two players while they fight: `server/tests/test_stands.py`). Alpha's
report, where his MiniBomb came up: HP 10 of 150 and 0 of 150, MiniBomb
(54) used once, the winner Alpha.

A battle by full rules (Alpha and Beta with Team Colonel's ROM beside
BN6's, Gamma with BN6's alone): Gamma's referee called it, "A
NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 150 HP! BETA HAS 150!", and
asked nothing; the server refuses a WATCH from a game without BN5's ROM
for such a battle (NOTICE 19, `server/tests/test_stands.py`). That
battle stood at its Custom screen until the players' frames ran out (the
scripted keys under full rules did not reach OK), so its end was not
seen there; its report and the end's are the same path as BN6 rules'.

The browser build, where the Arena is compiled out (`CW_ARENA` 0):
`tools/online_browser.html`'s probe served beside a local v7 server
allowing its origin, in headless Chrome (agent-browser), its MegaMan put
before the referee and A pressed through his pages while two host builds
fought (to frame 2,918, 48 hashes agreeing): "SORRY! NO NETBATTLES FROM
THIS PET!", then "A NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 150 HP!",
later "ALPHA HAS 120", "ALPHA USED MINIBOMB!" and "ALPHA HAS 10 HP!" as
the cable's first's reports came, and no question.

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
  picture after 20 ticks ("Waiting for the other PET"). Measured on the
  deployed server (19 October 2026, docs/ONLINE.md): delay 7, two
  full-rules battles to their end with every hash agreeing, the leading
  game waiting about one frame in eight (issue #128).
- The Souls under full rules ([Full rules](#full-rules), above): all of
  issue #130's seen on both GBAs. Not yet: a NetBattle in which a
  MegaMan grows worried by hits alone (the runs held his mood worried),
  ToadSoul's flag on BN5's sea and FloatShoe over a cracked panel (a
  NetBattle's field has neither; both flags were read set on both GBAs),
  and BN5's after-effect rule for a DarkChip where its battle's bit 0x40
  is set, if BN5's own NetBattles set it.
- BN5's own link battle as a reference: the differential runs one guest
  core and no cable, so MrkCan1 against a stepping MegaMan is compared
  against the Dark Soul (`chip_mrkcan1_darksoul`), not against BN5's
  NetBattle.
- The stands ([The stands](#the-stands)): watching on Android and iOS
  (built, untried), and a catch-up's pace in a window and on a handheld
  (measured headless, eight frames a tick); a full-rules battle watched
  to its end by a game with BN5's ROM (its way in and ready are full
  rules' as a player's; untried end to end); a spectator too slow to
  catch up (the refusal and the referee leaving that battle out are in
  the code, not seen); the report's chips past one a hand.
- Battle (not Practice) and its chip forfeit, the pack
  (it stays home), and any platform but the desktops: the 3DS and the
  browser build compile the Arena out (`CW_ARENA`), Android and iOS build
  it but are untried.
