---
type: Status
title: "BN6 Gregar's own gaps"
description: "The audit of 11 November 2026: what BN6 Cybeast Gregar's own ROM holds that a run does not use, kind by kind (Program Advances, chips, fights, NaviCust programs, music, the net's looks and boards, requests and traders), each verified by a ROM read, the code or a capture, or marked assumed, sized, and split into what is built without a call and what waits on the owner."
tags: [multirom, bn6, status]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-11T11:40:00Z }
sources:
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6, read by scratch scripts (not in git) and fought in BN6's engine by `build.py xdiff`'s engine specs"
    title: "The player's BN6 ROM"
  - id: bn6f
    resource: "https://github.com/dism-exe/bn6f (Falzar; its labels and script commands, read against Gregar's addresses)"
    title: "The bn6f disassembly"
  - id: kb
    resource: "docs/game/bosses/super-bosses.md, docs/game/navicust.md, docs/game/core-loop.md, docs/game/fidelity.md, docs/world/home/, docs/rom/, src/core/chip_pool.c, src/core/rom.c, as of dev at cb756ccd"
    title: "The knowledge base and the code"
---

# BN6 Gregar's own gaps

The owner's goal of 11 November 2026, "everything from everything",
reaches BN6 Cybeast Gregar itself: [What is not ported yet](gaps.md)
audits BN5 and BN4, and this audits what BN6's own ROM holds that a run
does not use. Every row says how it was checked: a ROM read (scratch
scripts over the player's ROM, nothing written to git), the code that
reads it, a fight in BN6's engine (`build.py xdiff`'s engine specs:
[The differential](../devtools/differential.md)), or a capture; what was
not checked is marked assumed, and a negative claim names the search
that came up empty. Sizes are [What is not ported yet](gaps.md)'s: **S**
a day or less, **M** a few days, **L** a week or more. Each candidate is
weighed against [the core loop](../game/core-loop.md): its lacks, what
it froze (power meta, any change to BN6's battle) and the owner's calls.

## Program Advances

BN6's Custom screen checks 43 recipes (`BN6_PA_LIST`, `0x0802BCB0`: 13
listed, 30 of codes in a row), then a run's BN5 rows
([The chips' code](chips-code.md)). The 20 recipes after them in the
ROM (`0x0802BB98` on: Cannon, Cannon, TankCan1 for GigaCan1 and the
like) are the link battles' list's alone (`0x0802BC60`, 20 of its own
before the 43): the BattleChip Gate's, which no single-player battle
checks ([Chips, folders and the Library](../rom/chips.md), Program
Advances). Their results are all among the 43's.

Every part of the 43 is in the chip pool with every code its record has
(`src/core/chip_pool.c`, `roll_chip`'s code from the record's own;
Django's three aside, below), so every recipe can form in a run. Proved
in a run's engine, 11 November 2026: each recipe in question fought by
an engine spec, its parts picked in order on the first Custom screen,
MegaMan's next chip (the first kind's +0x2A, the timeline's `chip`) the
Program Advance's id from the first fight frame, used at t30
(`tests/xdiff/pa_bn6_*.json`, all PASS):

| Program Advance | Recipe as a spec forms it | Parts in a run | Spec |
| --- | --- | --- | --- |
| DblHero (344) | WideBlde B, LongBlde B, ProtoMnSP B | ProtoMnSP a tier-4 find | `pa_bn6_dblhero` |
| Darkness (345) | VDoll F, VDoll F, Bass F (or BassAnly F) | Bass and BassAnly Gigas, tier 4 | `pa_bn6_darkness`, `_alt` |
| TwinLdrs (348) | ProtoMnSP B, AntiNavi \*, Colonel \* (or ColonelSP C, AntiNavi \*, ProtoMan \*) | All four Navi chips in the pool | `pa_bn6_twinldrs`, `_alt` |
| SunMoon (347) | Meteors R, Atk+30 \*, Uninstll R | Uninstll and Atk+30 tier 3 (no list leaves them out) | `pa_bn6_sunmoon` |
| BodyGrd (343) | AntiNavi, AntiSwrd, AntiDmg in \* | All three | `pa_bn6_bodygrd` |
| DestPuls (334) | ElcPuls1-3 J | All three | `pa_bn6_destpuls` |
| MstrCros (346) | FireHit3, AquaNdl3, ElcPuls3, RskyHny3 A | All four | `pa_bn6_mstrcros` |
| StreamHd (336) | AuraHed3 F, G, H | All three AuraHeds | `pa_bn6_streamhd` |
| PitHocky (341) | AirHocky L, M, N | AirHocky | `pa_bn6_pithocky` |
| PoisPhar (342) | PoisSeed P, PoisSeed P, Anubis P | Both | `pa_bn6_poisphar` |
| CrosOver (349) | Django D, Django2 D, Django3 D | **None**: BN6's Django chips have no art of their own (cut from the US version), so the pool leaves them out | `pa_bn6_crosover` |

StreamHd's, PitHocky's and PoisPhar's own records hold no codes (`FF`
at +0 to +3; the other 27 hold A, B, C, \*), and BN6's check reads none
of them (`0x08029520`: the recipe's kind routine, then the per-battle
bit, `0x08029652`): they form and play all the same. CrosOver forms
from BN6's Django ids and its attack lands (the foe deleted at t67, as
DblHero's), so only its parts are missing; a run's Django chips are
BN5's V1, SP and DS (524-526, [The chip registry](chip-registry.md)).

| Gap | Verified how | Size | Who decides |
| --- | --- | --- | --- |
| CrosOver from a run's Django chips (V1, SP and DS in D, where BN5's ROM is read), a row beside BN5's in BN6's grown list | The spec above; the grown list's rows (`src/core/xchips_pas.c`) | **S** | Built without a call (below) |
| The BattleChip Gate's 20 recipes in single-player battles | ROM read (`0x0802BB98`-`0x0802BC5F`); BN6's check walks `0x0802BCB0` alone outside a link battle (`0x08029556`) | **S** | The owner: more ways to the same 20 results (two GigaCan1 and a TankCan1 for a GigaCan1) makes Program Advances easier; recommended no, as BN6 USA's single player never had them |
| The P.A. Memo | Out of scope by design ([The Library](../game/meta/library.md)) | | Decided |

## Chips

BN6's ids 1-313 that a run never hands out, by `chip_pool_tier`'s rules
(ROM read of every record, 11 November 2026):

| Chips | Why the pool leaves them out | Verified how | Size, who decides |
| --- | --- | --- | --- |
| Falzar's Navi chips: SpoutMan, TmhkMan, TenguMan, GrndMan, DustMan (V1, EX, SP each, 242-256) | Library flags bit 0, the other version's | Each has art of its own in Gregar's ROM, and an engine spec of each V1 (one chip, used at t30) forms and plays it: SpoutMan's, TenguMan's, GrndMan's and DustMan's delete the foe at t67 as ProtoMan's does; TmhkMan's battle runs its 600 frames without a crash | **S** (the flag test in the pool); the owner: Falzar's content without Falzar's ROM. Recommended yes, at the tiers of Gregar's Navi chips |
| GunDelEX (18), Otenko (153) | Bit 0, and no art of their own | ROM read | Out: no picture |
| HackJack V1-SP (275-277), Django 1-3 (278-280) | No art of their own (cut from the US version) | ROM read; HackJack's chip ends an engine spec's battle at t33, two frames after its use, so it does not play whole | Out; Django's are BN5's in a run (above) |
| PunchArm, NeedlArm, PuzzlArm, BoomrArm, SyncTrgr (281-285) | Flags 0x30, unused | ROM read | Out, as BN6 has them |
| DrkSword, DarkThnd, DrkRecov, DarkInvs, DarkPlus (286-290) | Flags 0x30: BN6's DarkChips | ROM read | Not a gap: a flame's ([BN6's DarkChips](../game/meta/bn6-darkchips.md)) |
| DblBeast, Gregar, Falzar (311-313) | Flags 0x10, the Beast chips | ROM read | Left for later with the super bosses ([Super bosses](../game/bosses/super-bosses.md)) |

Uninstll (185) and Atk+30 (195), with WhiCapsl, Atk+10 and Navi+20, carry
library flags 0x40, whose meaning is not traced; they pass the pool's
every rule and no list of loot, dealers, jobs or prizes leaves them out
(`src/core/loot.c`, `src/layer/shop.c`, `src/core/jobs.c`,
`src/layer/lib_prizes.c` read): a run hands them out.

## Fights

BN6's story battle records (`0x0B06E0`, 16 bytes each) and its enemy
ids (`0x0182C4`), read on 11 November 2026 with the callers bn6f names
and their bytes in Gregar ([BN6's battles](../rom/battle.md)). A run
builds every guardian's battle from an enemy id and a version
(`src/director/encounter.c`, `src/core/data.c` `enemy_id`), so any
version is a number away; what each needs is a place, a band and a
reward.

| Fight | In BN6 | In a run | Size, who decides |
| --- | --- | --- | --- |
| **GBeast SP** (`0x187`, 1800 HP) | Record `0x8C`, the Underground's cutscene before Bass BX (Gregar `0x08099668`); the SP Navi of Underground 1 and Graveyard 1's random tables (byte 7 = 11) | No ("Left for later: GBeast SP before Bass BX", [Super bosses](../game/bosses/super-bosses.md)) | **M**: its staging, its gate's line and its reward through the game-design skill; built without a call, as the super bosses' own plan |
| **GBeast** (`0x185`, 900 HP) | Record `0x79`, a story cutscene (Gregar `0x08093629`) | No | With GBeast SP: its first form, as Bass's chain has his |
| **The RV six**: BlastMan, DiveMan, CircusMan, JudgeMan, ElementMan, Colonel RV (version 3, 1900-2600 HP) | Records `0x8E`-`0x93`, Graveyard 2's six monuments (Gregar `0x080804D4` on): after Bass SP (event `0x1007`, then `0x1008` and `0x101C`), a monument bursts into flames once per jack-in and starts its Navi's RV | No: guardians come V1, EX, SP by band (`pacing_guardian_version`) | **M**: a band between SP and the super bosses, the names' marks (`boss_rounds.c`'s three; `director_layer.c` names version 3 " DS"), the reward (no RV Navi chip exists); the trigger is BN6's post-game, so the Graveyard's layers after Bass SP fit it |
| Every other Navi's RV, BX and version 5; the six's BX and version 5; GBeast EX, RV, BX | No record names them (list 0's 0x10D, list 1's 0xC0, every random table's SP slot, a ROM scan for their setups in `0x080B0000`-`0x080BA000`); their HP copies SP's or RV's | No | **L**, not recommended: nothing new to fight, their AI at those versions unproven |
| BassXX (`0x172`, version 5), Bass versions 2 and 4; Gregar EX, RV, BX (`0x174`, `0x176`, `0x177`) | No record or setup | No | **L**, not recommended, as above |
| AI 17, the cut Count (records `0x8A`, `0x8B`) | No caller | No | Out |
| Beast Out and the Cybeast in the short net | BN6's story | After the endless Nest only ([Super bosses](../game/bosses/super-bosses.md)) | The owner (below) |

Not verified: what the RV records' option word's bit 0x2 does, that
the flag reset (bn6f `sub_8033FDC`) runs at jack-out, and battles a
text script's chatbox command could start (not searched).

## NaviCust programs

The pool leaves out twelve of BN6's 46 programs ([The NaviCust](../game/navicust.md),
6); ten are inert or harmful in a run by design (Humor, Poem, Rush,
Beat, Tango, OilBody, Fish, Battery, Jungle, Millions). The other two,
traced on 11 November 2026 ([The NaviCust](../rom/navicust.md), SneakRun
and NumbrOpn):

| Program | What it does (verified in Gregar's code) | In a run | Size |
| --- | --- | --- | --- |
| NumbrOpn (15) | NaviStats +0x61; the Custom screen's hand becomes 10 chips (`0x0802A466`) where its check holds (`0x0802A59C`: not in a link mode, battle modes 1, 6, 7, 9; not in a form whose emotion code is 0x0A or 0x16) | Left out "until its condition is known": now it is. A run's battles are BN6's mode 0, so it works in every one | **S**, through the game-design skill: the Hand build's top program |
| SneakRun (16) | NaviStats +0x1E; BN6's random-battle roll (`0x080ABE1E`) calls the battle off where its rolled record's byte 1 is less than MegaMan's HP plus 4, over 5 | Left out "until its effect is known". It judges BN6's own rolled record, before the engine puts its battle in its place (`BN6_ENCOUNTER_ROLLED`): byte 1 is 20 on the comps and homepages, 40 in Central Area, 48 Seaside, 74 Green, 120 Sky, the Underground, the Undernet and the Graveyard (the random tables read), so from 97 HP it calls off every comp's battle and from 597 every area's; the battle floor's owed battle is the same roll, and a layer's shut exit pad would never open ([The structure](../game/progression/structure.md), The battle floor) | **S-M**, and the owner's call: the floor's owed battle exempt and a measure of the run's battle in place of BN6's byte; recommended out, as a run's battles are its loot |

## Music

BN6's songs 0x01-0x25, against every place a run picks a song (the
code read; two headless runs printing the song each second,
`CYBERWORLD_EMU_DEBUG`), and against where BN6 plays each (its map-music
lists, every one dumped; bn6f's scripts; its battle records' music
byte). Transmission (0x0F) does play: at every jack-in from the real
world (BN6's own jack-in screen, Gregar `0x0803420C`). Thirteen never
play outside the Jukebox:

| Song | Where BN6 plays it | Where a run could |
| --- | --- | --- |
| 0x02 Villains | Cutscenes of the WWW (Seaside Town, the Expo Site, Sky Town), 7+ text scripts | Before a guardian, or before the Undernet's Server |
| 0x06 Seaside Town, 0x07 Sky Town, 0x08 Green Town, 0x24 ACDC Town | Their towns' maps (lists from 0x10, 0x40, 0x30; ACDC Town's in every list) | Home's town styles (`town.c` has Seaside, Green and ACDC behind a dev variable; no Sky): towns are the owner's (below) |
| 0x0B Something's Up, 0x0C Crisis | The alarm lists (0x21-0x23, 0x31-0x32, 0x41-0x44, 0x52, 0x54) on the towns, Lan's house and the Academy; Crisis in eight cutscenes | Something's Up in town when an act brings news; Crisis on a layer's alarm (the Glitch layer, a dark layer's last phase) |
| 0x0D Sorrow | The opening in ACDC Town, 7 text scripts | After a lost run |
| 0x0E Good Times | Academy and Expo cutscenes, 7 text scripts | Home after an act won |
| 0x18 Pavilion, 0x25 Expo | The Expo Site's maps (lists 0x60, 0x62; 0x61) | The Net Square's halls, Trade Central |
| 0x22 Showdown | Story record 0's music byte, the one record naming it; no script found starts it | Chaud's duel ([The rival](../game/rival.md)) |
| 0x23 The Chase | Not found (no code immediate, decoded script or record names it; assumed a scene bn6f has not decoded) | A timed escape |

Each is **S** to place; where it plays changes what a moment says, so
each goes through the game-design skill, and the towns' wait on the
owner.

The run's area themes against BN6's own lists: Central Area (0x13),
the Graveyard (0x09), the Undernet (0x14), the comps and homepages
(0x13) and the five story comps match. Seaside Area (0x11), Green Area
(0x12) and Sky Area (0x0A) play their town's comp's theme where BN6
plays the Net's (0x13); Undernet Zero plays the Pavilion Comp's (0x20)
where BN6 plays the Undernet's; the Underground plays the Cybeasts'
(0x21), which BN6 gives it only in its last chapter (list 0x54).
Adapted, not a gap: three of BN6's net areas would otherwise share one
song. Recorded in [What is original](../game/fidelity.md)'s terms here;
the owner's call if it should be BN6's.

## The net's looks and boards

| Claim | Verified how | Size, who decides |
| --- | --- | --- |
| "Runs draw two comp looks and one homepage look of many" | **Refuted**: BN6's 32 device comps (`0x8C`, `0x8D`) share one tile set in two palettes (26 and 6) and two layouts; the homepages (ACDC, Aquarium, Green, Sky: `0x88`:1, 3, 5, 6) that tile set in a palette each; runs use every one (`src/core/rom.c` net_area: two comp biomes, the Aquarium HP's look and the ACDC, Green and Sky HPs) | None from BN6; more comp variety comes from BN5's comps ([What is not ported yet](gaps.md)) |
| The second tile layer's bridges and ornaments left out | [What is original](../game/fidelity.md), Net floors; [The generator](../world/level-design/generator.md) names Seaside's boardwalks, Central's bridges, Green's and Central's pad ornaments, CopyBot's spikes; the warp gem, framed pads and counters built | **M**: placement and `build.py tiles`, likely a `LAYER_MAKE` bump; no call |
| BN6's BBS boards: five (Seaside Area 1, the Green HP, the Sky HP, ACDC Area "ACDC Oasis", Undernet 2 "The Underworld"), about 151 titled posts | ROM read: the archives at `0x0813FE2C`, six LZ77 pairs (the sixth a copy of the fifth); a board opened by text command `FB 0A` | One post reused (the DarkChip post, a bystander's line on a dark flame's layer); Seaside's BBS a prop without a check. **S-M** to open board 0 on it; the owner: the posts talk of BN6's story (the Expo, the Mayor, the Cybeasts), a fiction check |
| The comps' own gimmicks (Robot Control, Aquarium, Judge Tree, Mr. Weather, CopyBot) | **Assumed**: their map scripts not read; the run's set pieces (security cubes, number and skull doors, arrow lanes, invisible paths, teleports) are BN6's | **L** with a reverse-engineering spike first; a game-design call |
| BN6's device comps as a busting errand | [Jobs](../world/home/jobs.md): "Not yet" | **L**, design first |

## Requests and traders

| Gap | Verified how | Size, who decides |
| --- | --- | --- |
| BN6's own requests (35, scripts 0-34, "Virus Deletion" to "RoadToSoulBtlr!"; 35-47 placeholders) | ROM read (titles `0x087ED244`, posts `0x087ED504`); the run's Request BBS posts its askers' requests over them ([Jobs](../world/home/jobs.md)) | **L**: their clients' scripts live on town and story maps (assumed, not read); the owner, with the towns |
| The BugFrag sink (the core loop's lack 6) | Built: the BugFrag workshop on every act's middle layer, the Trader its list's last entry, a battle won with bugs paying BugFrags ([The NaviCust](../game/navicust.md)); the lack's text predates it | None; the core loop's page to say so at its next review |
| The Number Trader | Off by design, "Out of order" ([AsterLand and the Academy](../world/home/asterland-and-academy.md)) | The owner: the step-out proposal's "A number in town" (**S-M**, [Stepping out](../world/step-out.md)) |

## What is built, and what waits

Built without a call, one pull request each, cheapest first:

1. **The Program Advances' proof** (this audit's PR): the thirteen specs
   above.
2. **CrosOver from a run's Django chips** (**S**).
3. **NumbrOpn** in the pool (**S**), through the game-design skill.
4. **Songs where BN6 plays them** (**S** each), through the
   game-design skill.
5. **GBeast SP before Bass BX** (**M**), as the super bosses' plan has it.
6. **The RV six** on the Graveyard after Bass SP (**M**), the trigger now
   traced.

The owner's, with a recommendation each:

- **Beast Out and the Cybeast in the short net.** Today Beast Out comes
  after the endless Nest alone. Recommended: keep it there; Beast Out is
  BN6's late power and the endless net's reward, and the short net's
  Nest already ends on its own guardian.
- **The Number Trader.** Recommended: "A number in town" from
  [Stepping out](../world/step-out.md), with the towns.
- **Towns and jack-ins** (postponed, discussion #323): the four town
  songs, BN6's requests and their clients come with them.
- **BN6 Falzar's own content** (its Crosses, Cybeast Falzar, its
  minigames), which needs the player's Falzar ROM as another optional
  ROM. Its five Navis' chips are the exception: they play from Gregar's
  ROM (Chips, above). Recommended: those chips now, the rest with a
  Falzar ROM.
- **SneakRun**, the **BattleChip Gate's recipes**, **BN6's BBS boards**:
  above.
