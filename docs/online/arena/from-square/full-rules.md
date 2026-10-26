---
type: Feature
title: "Full rules"
description: "Full rules in the Arena: BN5's installs recorded once (`xinstall_log.c`) and taken into both pair cores, the record's hash in the pair's, Souls carried across the cable (`arena_souls.c`), what one GBA alone writes."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-20T16:36:26Z }
---

# Full rules

Each game runs both cores of the battle, so each installs BN5 into both
from its own BN5 ROM, the same way, and the two games' pairs stay the
same byte for byte. The installs are the game's own: as its core starts
(`src/emu/scene_emu.c`), every byte BN5's installs write into the core's
ROM copy and every hook they set is written down in their order
(`src/emu/xinstall_log.h`: `xinstall_core`'s tracks, the moved objects,
the Navis, the chips' code, the families, the sea and the obstacle
strike, then the chip registry's `xchips_install` and the Souls'
`soulproxy_install` and `soulunite_install`; measured 1,913,709 bytes and
132 hooks, 1,735,251 and 42 of them before the Souls'). In every NetBattle the boot core and both pair cores
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
"Huh? Our PETs don't match..." (before v9 "...on BN5...": since, no
character names a game) and "Lan,let's reboot the PET and
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
proof](two-client-proof.md), below, both games with BN5's ROM beside
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
[MegaMan's forms and BN5's Souls in BN6's engine](../../../rom/souls.md), BN5's DarkChips' after-effects).

MrkCan1: BN5's own link battle, MegaMan against MegaMan, is out of the
differential's reach (it runs one guest core, no cable). Its nearest is
`chip_mrkcan1_darksoul`: the Dark Soul, MegaMan's body under BN5's chip
AI, steps at t39 and t76 while the mark sweeps from t32 to t84, and
neither engine's mark locks or hits, frame for frame (PASS over 600
frames). So BN6's moved code does what BN5's does against a target that
steps; in a NetBattle it has locked on a stepping MegaMan (above) and
wandered without a lock (the earlier run), the same on both games each
time.
