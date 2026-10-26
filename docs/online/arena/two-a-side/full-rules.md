---
type: Feature
title: "What a partner meets under full rules"
description: "What a partner meets under full rules: BN5's installs keyed by a side's player now answered by object, a room of BN5's size for each MegaMan's chip AI (issue #182, `darkinvs_room.h`), Chaos Unison, Souls, mood."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T15:15:49Z }
---

# What a partner meets under full rules

Every NetBattle runs BN5's installs on both GBAs (the Souls with their
mood, ColonelSoul's button and the Chaos Unison, DarkInvs's machine,
BN5's chips and Navis, the green flash), recorded by `xinstall_log.c`
and hooked on the pair's cores. What of it was keyed by a side's player
rather than by object was read in the code (22 October 2026) and is now
answered by object, a side's partner (`BN6_BATTLE_PLAYERS`' second word
of a side: `soul_partner`) a Navi of his own; none of it changes a
NetBattle without the flag, nor the installs' record (no hook or byte
added: `--dev arena,arenaboth,arenalag=4,arenadelay=1` over at frame
2690, its last hash `86d2f837337a1494`, as before). The proofs, 22
October 2026, the player's Gregar and Team Colonel ROMs, `arenadelay=1,
arenalag=4`, `CYBERWORLD_EMU_DEBUG=1` (each answer said by core: the
pair's cores 1 and 2 are the two GBAs):

- **BN5's chip AI: a room of its own for each MegaMan** (issue #182).
  BN5's chip AI keeps its state past BN6's 0x100 bytes of AI data, so a
  MegaMan under it needs BN5's 0x140; side 0's player's AI data moves
  into `RAM_PLAYER_AI` at his set-up. In a plain 1v1 DarkInvs on side 1
  moved side 1's AI data into that same block: seen on both GBAs with a
  checkpoint of thirty DarkInvs carried by both sides (`arenapeer=2`,
  [Dev tools](../../../devtools/overview.md)) "side 0 020115e0 side 1 020115e0", then side 0 grey under
  side 1's DarkInvs, both MegaMen standing still and the battle going
  nowhere (BN5's machine ran for side 0 alone, on the keys both copies
  wrote). Now each other MegaMan of a NetBattle (side 1's player, the
  partners) moves into a room of BN5's size of his own at the end of
  BN6's AI data's pool as the chip AI first takes him (`darkinvs_room.h`,
  [Memory the engine writes](../../../engine/emulation/memory.md#bn6s-ram), BN6's RAM; [Navis moved into BN6's battle engine](../../../rom/navis.md), BN6's AI data's pool),
  and the machine, its time and its idle frames run for any MegaMan whose
  AI data is of BN5's size. Seen: side 1 under the chip AI (HiCannon,
  MegEnBom, LongSwrd) and side 0 himself, "seat 2's AI data 02034180
  moved into his room 02034740" on both GBAs, over at frame 1179 (side 1
  won); both sides at once (`arenapeer=3`), each driven apart; with
  `arenaroll=8` 147 rollbacks of 8, the state the same after each, its 19
  hashes those of `arenawindow=0`. Two a side with all four under it
  (`arena2v2=1,arenapeer=3,teamhp=500`): side 1's player's room and the
  partners' ("seat 2's AI data 02034280 moved into his room 02034740",
  seat 3's into `0x020344C0`, on both GBAs) taken, both GBAs alike at
  all 2,372 iterations, over at frame 2675; `arenaroll=8` 319 rollbacks,
  the state the same after each, its 42 hashes those of
  `arenawindow=0`; `chipaibreak=2` (no room on the second GBA) apart from
  iteration 340. On this build (the partners' words in BN6's packets,
  the same checkpoint): side 1's DarkInvs in a 1v1 over at 1179 as
  before; two a side over at 2261, 1,954 iterations 0 apart (193
  rollbacks); `arenawindow=0` and `arenaroll=8` over at 2399, 2,092
  iterations 0 apart, 299 rollbacks of 8, the same 39 hashes;
  `chipaibreak=2` apart from iteration 340 again. A partner was left out of the chip AI before
  (`held_short`): he is not now.
- **The Chaos Unison's hooks act for the side's player alone**
  (`soulchaos.c`: the charge, its phases, the release, the request taken,
  the counter's test, the glow; the Navis' buster arm's chaos,
  `darkinvs.c`). A partner charging B while his lead was in a Chaos
  Unison stepped his lead's unison and took its release: his charged B
  became his lead's chaos attack or its failure. Seen with side 0's lead
  in NumberSoul by DarkPlus (thirty DarkPlus, `souls=0x04`, UNITE by the
  keys of `tests/xdiff/chaos_number_45.json`, B held 45 frames six
  times), his partner holding B 110 frames twice a turn: "side 0's
  partner's charged B his own, his side's player in a Chaos Unison" on
  both GBAs, his lead's chaos attack his lead's; over at frame 3659, both
  GBAs alike at all 3,123 iterations; `arenaroll=8` 456 rollbacks, the
  state the same after each, its 60 hashes those of `arenawindow=0`;
  `partnerbreak=2` (the partner his lead on the second GBA: the rule
  before) apart from iteration 222, the second GBA's lead releasing in
  phase 2 where the first's failed. On this build, the same checkpoint
  and keys: the partner's charged B his own on both GBAs, over at 3419,
  2,883 iterations 0 apart (287 rollbacks); `arenawindow=0` and
  `arenaroll=8` over at 3415, 426 rollbacks of 8, the same 56 hashes;
  `partnerbreak=2` apart from iteration 219.
- **A Soul's powers and look are its MegaMan's** (`soulchips.c`: the chip
  charge and its double, TomahawkSoul's grass, KnightSoul's guard,
  ShadowSoul's sword and shadow; `soullook.c`: the palette of an object
  drawn in his side's form, the arm flash and its height, the crossing's
  sound; `soullife.c`: the turns counted, the end and the weakness break,
  each refused for a partner). Read, and seen in the run above: the
  partner's charged shots plain, his look his own. (Since [A partner's
  UNITE](unite.md) each reads the MegaMan's own Souls state:
  his side's for a lead, his own block for a partner, who has a Soul of
  his own to read.)
- **BN5's mood and its DarkChip rule by object** (`soulmood.c`). A
  partner's mood and dark are kept in a block of his own after the
  sides' (`BN6_SOULS_PARTNERS`, [Running the game's own logic](../../../engine/emulation/overview.md)), his NaviStats his own
  (`soul_navi_stats`, his room's in `arena_team_navi.c`, told by
  `arena_team_rules.c`): a BN5 DarkChip he uses darkens him and lays its
  after-effect on his NaviStats, a hit on him is his mood's, BN5's dark
  gate reads his. Before, all of it was his lead's (his DarkChip zeroed
  his lead's mood, BN6's own mood byte with it: no Cross, no Beast Out, a
  weak buster for the lead; every hit on him lowered his lead's mood, the
  poison panels under the partners from the battle's first second). So a
  2v2 NetBattle hashes otherwise from frame 59 than before: the run of
  the proofs below (`arena2v2=1,teamhp=500,arenalag=4`, a fresh data
  folder) still ends over at frame 3426, side 0 won, 57 hashes, 2,700
  iterations 0 apart, and with that one line of the hit as it was it
  hashes as the build before did, all 57 (`219d892589f3cb16` at frame
  3419). Seen: "side 0's partner dark for the battle:
  his mood at 0203ffbe, his NaviStats at 02037fd0" (the fourth kind's
  slot 30), side 1's partner's at `0x0203FFDE` and `0x02038098`; with
  `partnerbreak=2` the second GBA's says "side 0's player ... 0203ce00"
  at the same use. A partner's mood is never opened (his Custom screen is
  a copy with no hook), so BN5's writers leave it 0 and the worried rule
  never offers him a DarkChip; a hit's gain still goes to the attacker's
  side's player (BN5 has one a side).
- **BN5's Navis' and chips' targets: nothing in a NetBattle asks.** The
  audit took five chips of BN5's (ZSaber, Katana, MoonBld, CustSwrd,
  DarkCirc) for asks of BN6's side-player getter (`BN6_SIDE_PLAYER_GET`,
  which names a side's player alone). Read again: their recipes copy a
  block of BN5's (`0x08012530`-`0x08012704`) holding a routine that reads
  a side's player's chaos flag (BN5's `0x080125EE`) which none of their
  code calls (BN5's one caller lies outside every unit); a hook counting
  the getter's asks from moved code saw none in 2v2 NetBattles with
  thirty Katana, MoonBld, CustSwrd or ShadoMan (kept out of the build).
  BN5's Navis that ask it (ShadowMan's, NumberMan's and TomahawkMan's
  AIs) aim at MegaMan as enemies in a run's battles and never stand in a
  NetBattle. Were one ever to, his ask from moved code would be answered
  with the nearer of the side's two (columns and rows apart, the side's
  player where as near), the shape tried and taken out again since
  nothing reaches it and the getter runs some hundred times a frame.
- **Left out still:** a united Soul carried to the other GBA is the
  lead's alone (a partner's crosses in his stream: [A partner's
  UNITE](unite.md), which since gives him UNITE, his Soul's
  life and its powers by object). The
  Dark Soul's chip lists, which DarkInvs's machine plays too, are a
  side's, made from the lead's folder (`arena_souls.c`): a partner under
  the chip AI plays his lead's list. The green flash (`xchips_flash.c`)
  is counted once a side but tints both MegaMen of the side. BN6's own
  DarkChips' rule (`darkbn6.c`) is a run's, not on the pair.
- **A Cross left him as he was** (read in bn6f; his own since His Cross,
  above, which builds what this asks for): BN6's
  crossing controller (`0x020367F0`, bn6f dword_20367F0) keeps an entry
  a side (+8 and +0x18: +0 a Cross chosen, +4 a Beast Out, 0xFF none,
  +8 the side's player's object). Its first mode (sub_801486C) flags a
  Beast Out's object (its AI data +0x44 bit 0x4000000, sub_802DCDE) and
  sends a Cross to its second (sub_80148CC: sub_80148EC,
  activateCrossHappensHere_8014944, sub_801498E), which crosses the
  entries' objects; its third (sub_8014A00) waits for both. A crossing
  object reads its form by its alliance (sub_801595E from sub_8014A38),
  but only the entries' objects cross: a lead's Cross leaves his partner
  as he is. His own Cross would need entries of his own in that
  controller (or its modes run again for his object), his own pending
  record answered by object at `0x0801595E`, and the controller's wait
  taught to wait for him.
