---
type: Design
title: "The shared dive"
description: "The week's net dived together (being built since 15 November 2026): a portal in the Square, every diver on the seed seen live, crews of four, join-in battles, a queue of four at each guardian's door fought on one field and scaled by BN6's own versions, and the owner's calls on fairness."
tags: [online, game]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T03:00:00Z }
sources:
  - id: nfsu2
    resource: "https://nfsu2.xyz/"
    title: "NFSU2 in the browser, its page as read on 15 November 2026"
---

# The shared dive

**Being built** ([issue #589](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/589)).
The owner's idea of 15 November 2026, put through
the game-design skill and settled the same day in six rounds of
questions, each answer the owner's (below). It changes [The week's
net](weekly-net.md): the dive stays a run on the server's weekly seed in a
save slot of its own, from a standard profile, posted to the Board, and
becomes one dived with everyone diving it at the same time.

## The idea

In the owner's words, summed up: the greeter Prog who offers the week's
net gives way to a portal; the portal warps MegaMan into the week's run,
where every Navi diving it on the server is seen in real time; the Board
still ranks who got deepest; at a guardian's door a queue waits for up to
four players, the first in it starting when they like, and the party
fights the guardian together; and what a run does alone today (Mystery
Data, random battles, the rest) is made social in ways that stay fair.

The inspiration is nfsu2.xyz, Need for Speed Underground 2 recompiled for
the browser, whose page (read 15 November 2026) lists crews of
up to four who see each other in free roam in the crew's colour, joining
a friend's world, quick chat by number keys, a horn and a ghost mode. It
names no fixed count of players. The crews and the free roam are what
this design takes: a crew's size is the queue's four, and the free roam
is the layer where divers see each other.

## The design

The game-design skill's lens, before any number:

- **Experience: a marathon's crowd.** Every diver runs their own run on
  one course and sees the others on it; the run's peaks, the guardians'
  doors, are where they meet and fight as one.
- **Dialectic: together against ahead.** Company costs time and pays in
  safety and information; going alone is faster and riskier. The Board's
  tie-break is minutes, so waiting is a real price. It is restated at
  every scale: the layer (walk with a crew or alone), the fork (a crew's
  portal opens for all or not at all), the battle (call a crewmate in),
  the door (wait for a party or go now), the week (dive again into the
  pack). It grows out of the Square's own, gather against go
  ([The Square's design](square/design.md)).
- **Loop layer: the week's dive, as today.** A session goal at the meta
  loop's pace that feeds only the Board; nothing comes home to a normal
  run and nothing from one goes in ([The week's net and the meta
  layer](../game/meta/weekly-net.md)).
- **The fairness rule: others change a dive's odds, never its prize.** No
  chip, HP, zenny or layer comes from another diver. What is shared is
  information, open to every diver alike, and honour, which buys nothing.
  The standard profile and setup, and no trades on a dive, stay.
- **Patterns** (recalled from playing them, not checked in
  [Sources](../research/sources.md)): traces left for later players (Dark
  Souls' messages and bloodstains: phrases and deletion marks); a feed of
  who fell (Tetris 99's); doors that show where the crowd went (Hades'
  doors with a count); co-op scaled by the game's own version ladder
  (BN6's V1, EX and SP); honour, not power (first finds, split records);
  a crew of four as nfsu2.xyz's.

## What it rests on

Verified on `dev`, 15 November 2026:

- **One layer for everyone at a depth.** `run_layer_seed` (`src/core/run.h`)
  is the run's seed with the depth and the side kind mixed in, and a main
  layer's area comes from its depth (`layer_biome_at` in
  `src/director/director_layer.c`, `biome_for_depth`). So every diver on
  the week's seed at one depth and side stands on the same layer: its map,
  its Mystery Data's places and its n-th random battle
  (`loot_random_battle` takes the layer's seed and the battle's number).
  The ways that part the pack are the ones a run chooses: Lan's HP's
  portals (the act's area, the other way, the Undernet, a special route;
  [Lan's HP after an act](../world/home/lans-hp.md)), side layers, and a
  trip back, whose seed takes the Net's clock. A Mystery Data's chip leans
  by its diver's folder, so one data holds different chips for different
  divers.
- **Presence.** The Square's puppets on the game's own core
  ([A visit](square/overview.md)) and the server's channels of eight
  (`ONLINE_MAX_PLAYERS`, `shared/online/protocol.h`).
- **Four MegaMen on one field.** The Arena's seats (protocol v15,
  [Over the server](arena/two-a-side/over-the-server.md)): four seats' keys
  with rollback, every game running the same battle, each MegaMan with his
  own folder, Custom screen, NaviCust and Cross. Built for two a side
  across two linked GBAs, not yet for a party against BN6's AI.
- **The dive's slot.** Its save slot and standard profile, and the
  checkpoint at a guardian arena's door.

## The owner's decisions (15 November 2026)

| Question | Decided | Chosen over |
| --- | --- | --- |
| The solo dive | Replaced: a dive alone is a quiet net, nothing more | Both, one Board; both, two Boards |
| A quiet net | Live divers only | Echoes of earlier dives; a weekly Dive Night; heats |
| The portal | By the Board in the greeter's place, the Prog gone: BN6's Yes/No with the week's dates and how many Navis dive now | The plaza's centre; the portal with the Prog beside it |
| Others on a layer | Walk-through MegaMen with name tags, up to seven others (a channel's eight); every diver on the layer a dot on the PET's map | The nearest only; dots only |
| Traces of the week | A deletion feed to every diver; a mark where a diver fell, for the week, A on it saying what deleted them; one of the Square's fixed phrases left on a panel, others able to Wave at it; dealers and portals saying live facts (Navis gone each way, chips bought) | (all four taken) |
| Mystery Data | Each diver opens their own, at the same places for everyone (the layer's seed), its chip by their own folder; the week's first finder's name on a data; data others opened this week shimmer faintly (kept so on 15 November 2026 over who got what there, an opened count, and a cheer for two opening one together) | A pair data opened by two divers together |
| Crews | Up to four, Team up on a diver's card; a crew lasts across layers until someone splits up, leaves or ends their dive | One layer at a time; pairs |
| A crew at a fork | The portal opens only when the whole crew stands on it, or someone splits up | Each picks and the crew splits; the captain picks for all |
| Random battles | Each diver's own; crewmates within a few panels are offered to jump in; only crewmates | Anyone near; the battle's owner accepting each |
| A joined battle | One field; the foes as a solo diver meets them, unscaled; each fighter BN6's normal reward; joining resets the joiner's own encounter count, so each diver fights about as many battles as alone | Foes' HP times the fighters; their versions raised; no reward; a cap a layer |
| The door's queue | Up to four, each having walked to this door; divers behind on the same way get a flare | No flare; helpers summoned from behind |
| The leader | The first at the door starts any time; anyone leaves any time; a leader idle a minute passes the lead on | A countdown on a join; a ready check |
| The clock | Runs while the queue waits | Paused, capped; paused |
| The fight | One field: the party's MegaMen together against the guardian | Linked fights, each in their own battle with the guardian's HP shared |
| Scaling | A party of two meets the guardian's next version (V1 to EX), three or four the one after, its HP times the party; past SP or its last version only the HP grows | HP alone; none; harder hits past SP; a cap at SP |
| A drop | The missing MegaMen leave the field in each game, and each game finishes its own battle | Broken off, back to the door; the dropped run ended |
| Deleted in a party | The run is over, as alone | Pulled out at 1 HP and carried by a win; pulled out to fight again |
| Systems without the Arena | The New 3DS and the PSP join as linked fighters: their own battle, their damage into the field's guardian | Alone only; the whole party linked |
| After a dive ends | Dive again from layer 1 into the live pack, as often as one likes, the best kept | Once a day; the first dive alone counted |
| The Board | Its ranking unchanged (layer, a win, minutes); a mark for each guardian fought in a party; who dives now and on which layer; the week's fastest bust of each guardian, alone and by party size | (all four taken) |
| Build order | One field first, the linked fallback in the same release | Linked first; a spike first |

## As built

- **The portal** (15 November 2026, `src/online/online_square.c`'s
  `portal`, `square_week.c`'s `square_week_portal`): where the greeter
  Mr.Prog stood by the Board, BN5's dark hole (Gregar's list 7 `0x87` once
  `xnavi_object` has copied it; every diver has BN5's ROM, the DeepNet
  stamp's set), BN6's exit pad where it is not read, spoken to as BN6's
  wide objects are (`npc_desk`, within 20 world units of its middle). It
  says the week's post without a mugshot; the greeter's welcome went
  with him (the channel's count and the warps' names), his call over his
  head as a Navi logs in stays, over the portal. Yes warps MegaMan
  straight into the dive's first layer (`weekly_dive` no longer starts in
  Lan's room). Seen headless against a local server: the vortex at the
  plaza's edge, its question, the dive begun on layer 1.

- **The dive's link and the divers on a layer** (15 November 2026,
  `dive_link.c`, `dive_people.c`, `puppet.c`; the wire's v19 in
  [shared/online/README.md](../../shared/online/README.md)): the Square's
  connection is kept as the portal warps MegaMan into the dive, and kept
  again by a visit to the Square from the dive and back. Each layer he
  stands on is a room of the server's (DIVE enter: the week, its seed,
  the layer's depth, side and area); his place goes as STATE, and the
  room's divers come back in SNAPSHOT as BN6's MegaMen, puppets in the
  NPC slots the layer leaves free (BN6 runs 16: layer 1 of seed 1 held
  11, so five there), walked through. At home between acts nothing is
  sent and his last place stands. The server's feed (a deletion, a win, a
  guardian beaten, a flare) shows in a band over the map's foot, a line
  at a time. A link lost for 20 seconds leaves the dive solo; a dive's
  end is told (DIVE end) and the link closed; a quit to the title closes
  it. Seen headless with two games on a local server: both in layer 1's
  room, each seeing the other walk.

- **The guardian's door and the party's fight, linked** (15 November
  2026, `dive_party.c`; `boss.c`'s `boss_at_door`, `boss_door_hold` and
  `boss_party_set`): MegaMan saved at the arena's door with other divers
  on the layer joins its queue (QUEUE join), and the staging waits there.
  A band over the map's foot says who waits: the leader's "PARTY OF 2!
  A:GO IN B:ALONE", the others' "ALPHA LEADS 2 B:GO ALONE"; alone on the
  layer he goes in as ever. The leader's A starts the party: each game's
  guardian is scaled for it (two meet his next version, three or four the
  one after, his HP times the party, at most 9999) and each fights him in
  its own battle, his HP the party's: seat 0's game counts every seat's
  damage (its own, PARTY_DAMAGE from the others) and tells the HP it keeps
  (PARTY_HP); no game ever raises its guardian's HP over what its own
  battle left him. Until the one field over the server is built, every
  game's HELLO says `hello_no_arena`, so the server makes every party
  linked. Seen headless with two games on a local server at layer 3's
  guardian (BN4's FireMan, 1200 HP for two): the queue's band on both, the
  leader's A, both in the fight; Beta's hits (1200 to 1193) on Alpha's
  guardian and Alpha's (to 1191) on Beta's; Alpha's last hit deleted him in
  both games at once, Beta's guardian falling to the HP written. Open: seat
  0 deleted leaves the others without the party's HP (each then fights the
  rest alone), and the leader's idle pass.

- **Crews** (15 November 2026, `dive_crew.c`): A on a diver's MegaMan
  (within reach of where MegaMan faces, `dive_people_ahead`) opens BN6's
  own chat, MegaMan's "Lan,it's Beta!" and Team NetBattles' "Team
  up,Lan?" (on a crewmate, "Split up,Lan?"); Yes asks (CREW ask), and the
  diver asked gets "Alpha wants to team up with us!" and the same
  question; the band says "Alright! We're a team with Beta!" or the
  split. The chats' words go in a room the layer gives them (as the
  Square's), their Yes in the Square's team flag `0x14C7`, which a layer
  otherwise never sets, cleared as read. Seen headless with two games on
  a local server: the ask, the answer, the crew formed on both. At Lan's
  HP a portal taken (a lit link's Yes, or the pink pad's art) waits while
  any crewmate stands on no other (`home_way_hold` in
  `director_town.c`; CREW way, its index past 0), a band saying "Waiting
  for the crew...", and is let go as MegaMan steps off: built, not yet
  seen with two crewed divers at home. BN6's own trigger at the pink
  pad's foot still warps at once, a crewmate stepping on it there
  splitting the way. Not yet: the crew's join-in battles (on the one
  field).

## The spike: four on one field (15 November 2026)

`--dev party=N` (`src/online/party_field.c`) grows one of the run's own
battles to N MegaMen on side 0 on one core, the way `--dev arena2v2`
grows a NetBattle: the battle's list rewritten at its spawn
(`BN6_SPAWN_ENTITIES`, the partners on side 0's free panels, the back
column first) and each partner's keys written into his AI data at the
battle loop's key copy (`BN6_KEY_COPY`), here a script. BN6 keeps four
actors a side (bn6f's `BATTLE_ACTORS_PER_SIDE`, read at
`BN6_BATTLE_PLAYERS`), so four fit its tables. Seen headless with
`tools/cw.py play` and `foes 101@52` (HeatMan V1) and `foes 85@52`
(a Gunner), the log's `party:` lines and the pictures:

- **Four stand and fight.** Each partner is BN6's MegaMan (name 0x1A0),
  with the player's HP; he moves and shoots from his keys, his hits
  land.
- **A partner deleted leaves the field and the battle goes on**
  (`partyhp=30`).
- **The lead deleted does not end it while a partner stands**, and the
  partner's win ends it as a win, back on the map (`hp=10,partyhp=500,
  partyend=600`): BN6 ends a battle when a side has no actor standing.
  Each game decides what a fall means for its own run.
- **Who the enemies aim at is open.** A Gunner's and HeatMan's hits went
  to the lead alone, all 15 and 16 of them. BN6's lookup of a side's
  player (`BN6_SIDE_PLAYER_GET`, bn6f battle_findPlayer, 149 callers)
  reads only the side's first actor, and a nearest-actor lookup (bn6f
  sub_80103F8, 45 callers) the nearest; answered for every enemy caller
  with each MegaMan in turn, neither changed one hit, frame for frame. The
  AIs read their target elsewhere: the next thing to find, since a
  guardian that aims at the lead alone makes the lead's fight the party's
  hardest (the scaling above assumes the aim spreads).

## Open before it is built

1. **The join window.** The two-a-side hook trades the battle's list at
   its spawn (`BN6_SPAWN_ENTITIES`), so a MegaMan cannot be added to a
   battle already begun. The offer likely has to come as the encounter
   starts, the owner's battle waiting a moment for the answer.
2. **One field against a guardian.** Four MegaMen stand and fight on one
   GBA (the spike, above). Open: what the enemies aim at, so the
   guardian's attacks can spread over the party.
3. **The version ladder** for BN5's and BN4's Navis and for guardians
   with no SP of their own.
4. **A linked fighter in a party**: whether they count in its size, and
   which version their own battle's guardian is.
5. **Crews' battles unscaled.** Viruses end about half the lost short
   nets ([The core loop, reviewed](../game/core-loop.md)), so a crew dies
   to them less often, and the Board's party marks count guardians only.
   The owner's call stands; the statistics will show what it does.
6. **A quiet net.** With live divers only, the dive is as busy as the
   server: in October 31 players visited the Square and 6 posted dives
   (the same page). The statistics count from the first release how often
   two dives share a layer, parties' and crews' sizes, and joins.
7. **The wire.** Layer channels keyed by week, depth, side and way;
   positions; crews; the queue; the party's field; the feed, marks,
   phrases and first finds kept per week; a protocol version. A `Run`
   change only if a dive must hold a crew's or a party's state across a
   checkpoint.
8. **The words.** Every line this needs (the portal's, the offer to jump
   in, the flare, the marks) goes through [the voice rules](../voice/rules.md).
