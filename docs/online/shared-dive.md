---
type: Design
title: "The shared dive"
description: "The week's net dived together (being built since 15 November 2026): a portal in the Square, every diver on the seed seen live, crews of four formed by walking together, join-in battles, a queue of four at each guardian's door fought on one field and scaled by BN6's own versions, and the owner's calls on fairness."
tags: [online, game]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-21T19:31:16Z }
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
  chip, zenny or layer comes from another diver; HP only once a dive, when
  a crew's win pulls back a crewmate deleted beside them, and the Board
  marks that dive (the owner's call of 21 November 2026, issue #718: A
  deletion in a crew, below). What is shared is information, open to
  every diver alike, and honour, which buys nothing. The standard profile
  and setup, and no trades on a dive, stay.
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
  the week's seed at one depth and side stands on the same layer: its map
  and its Mystery Data's places. Its n-th random battle was not (found
  building the join-in battles, below): the run's own roll
  (`make_encounter` in `src/core/loot.c`) draws on the run's RNG and its
  memory of the battles fought. On a dive it now is, the owner's call of
  15 November 2026 (every diver the same course, a split the same
  battle): `roll_encounter` takes a dive's battle from `dive_battle`
  (`director_crew.c`), rolled aside from the layer's seed and its index
  alone, the Glitch layer's hardening left out, so its rolls again every
  few seconds give the same battle; a crew's battle is that one with no
  Mystery Data on its field. Seen headless: two dives' layer 1 handed out
  the same battles in the same order. A normal run's battles are rolled
  as before.
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
| Crews | By nearness, no asking: divers within four panels of each other are a crew, up to four, and one who walks away is out of it (21 November 2026, the owner's direction after the team's test, issue #714; the details below, [Crews by nearness](#crews-by-nearness-21-november-2026)) | Up to four, Team up on a diver's card, lasting across layers until someone splits up, leaves or ends their dive (15 November's call, as built); one layer at a time; pairs |
| A crew at a fork | Each takes the portal he likes; those who take one together arrive at its first layer together and are a crew again there within a second (21 November 2026, under the owner's direction, his to turn over) | The portal opens only when the whole crew stands on it, or someone splits up (15 November's call); each picks and the crew splits; the captain picks for all |
| Random battles | Each diver's own; crewmates within a few panels fight it too, unasked (21 November 2026, after the team's test: walking close is the yes, walking apart the no); only crewmates, who since issue #714 are whoever stands near | Crewmates offered it, A to jump in (15 November's call, as played: a question and three seconds held still at every battle); anyone near; the battle's owner accepting each |
| A joined battle | One field; the foes as a solo diver meets them, unscaled; each fighter BN6's normal reward; joining resets the joiner's own encounter count, so each diver fights about as many battles as alone | Foes' HP times the fighters; their versions raised; no reward; a cap a layer |
| The door's queue | Up to four, each having walked to this door; divers behind on the same way get a flare | No flare; helpers summoned from behind |
| The leader | The first at the door starts any time; anyone leaves any time; a leader idle a minute passes the lead on | A countdown on a join; a ready check |
| The clock | Runs while the queue waits | Paused, capped; paused |
| The fight | One field: the party's MegaMen together against the guardian | Linked fights, each in their own battle with the guardian's HP shared |
| Scaling | A party of two meets the guardian's next version (V1 to EX), three or four the one after, its HP times the party; past SP or its last version only the HP grows | HP alone; none; harder hits past SP; a cap at SP |
| A drop | The missing MegaMen leave the field in each game, and each game finishes its own battle | Broken off, back to the door; the dropped run ended |
| Deleted in a party | At a guardian: the run is over, as alone. In a crew's random battle the crew wins: pulled back once a dive, paid in the crew's HP, the dive marked (21 November 2026, issue #718; A deletion in a crew, below) | Pulled out at 1 HP and carried by a win; pulled out to fight again; (21 November) as at a guardian; back once a dive, free; at a guardian's party too; a fixed 100 HP a helper; the helper with the most HP paying alone; revived dives ranked below the rest |
| Systems without the Arena | The New 3DS and the PSP join as linked fighters: their own battle, their damage into the field's guardian | Alone only; the whole party linked |
| After a dive ends | Dive again from layer 1 into the live pack, as often as one likes, the best kept | Once a day; the first dive alone counted |
| The Board | Its ranking unchanged (layer, a win, minutes); a mark for each guardian fought in a party; who dives now and on which layer; the week's fastest bust of each guardian, alone and by party size | (all four taken) |
| Build order | One field first, the linked fallback in the same release | Linked first; a spike first |

## Crews by nearness (21 November 2026)

The owner's direction after the team's test of 18 November (issue
#714): the shared dive is to feel like co-op in a modern MMO or Diablo
and Path of Exile: drop in, play together by being together, no
prompts, no waiting, always clear who is with you. Crews form and split
by themselves. The game-design skill's pass, the same day:

- **Dialectic: together against ahead,** as before, now chosen by where
  one walks: walking beside another diver is the choice to dive with
  him, walking off the choice to go on alone. No screen asks it.
- **Loop layer: the moment on a layer,** feeding the crew's battles
  (#715) and its guardian's door, the session's.
- **Patterns** (recalled from playing them, not checked in
  [Sources](../research/sources.md)): grouping by proximity as public
  events and open-world co-op do it (Guild Wars 2's events count whoever
  stands in them; Journey's companion is whoever walks beside you);
  hysteresis, a wider reach to stay than to join, as any threshold that
  is crossed by moving needs.

Each call below was taken under the owner's direction of 21 November
2026 by the session building it, the least friction that direction
points to, and is his to turn over.

| Question | Taken | Chosen over |
| --- | --- | --- |
| The radius | A crew forms within four panels (128 world units, the join-in battles' old reach, `crew_near`) and holds within six (192, `crew_far`): a crew at the edge does not form and split with every step | One radius both ways (flickers at its edge); a wider one (strangers across a room pulled in) |
| A moment first | Near for a second before a crew forms (`crew_form_ms`), so two passing each other are none; out of reach for two seconds before one is out (`crew_split_ms`) | At once (crews made and broken by a step past); longer (a wait the direction rules out) |
| Who decides | The server, from the places every game already sends (STATE), each tick: one answer for every game, `crews.py` holding the crews | Each client for itself (two games could disagree about who is in) |
| More than four near | The first there: the crew as it stands keeps its four, and a fifth near stays outside it until a place opens | The nearest (a crew reshuffled by every step, someone already in dropped) |
| Chains | One crew: A near B and B near C is a crew of three, so a line walking a corridor stays one | Everyone near everyone (a line splits at its ends) |
| Two crews meet | One crew where both fit in four, the older taking the other's | Each kept apart; the larger first |
| Across layers | A crewmate who takes a portal first, or is home between acts, stays in for ten seconds (`crew_away_ms`); the crew taking a portal together is still one on its first layer, and crewmates who are near there again are one anyway | Every layer a new crew (a flicker at each arrival); a crew kept until it splits |
| At home | No places are sent from Lan's HP: the crew stays as it was as its divers left the layer, and the fork's hold is dropped (above) | The fork's hold kept for those who came home together |
| Strangers | Anyone near is a crew, strangers too; walking away is the way to stay alone, and a switch for it waits for the next test to show it is needed | A setting to stay alone now; crews only between friends |
| The chat | "Team up,Lan?" and "Split up,Lan?" gone, and A on a diver does nothing of the crew's | Kept to hold a crew over a distance |
| What is said | Nothing at a forming or a split: the crew's sign (#716, [below](#the-crews-sign-21-november-2026)) and its display (#717) show it, the log says it (`dive: crew now: ...`) | "Alright! We're a team with Beta!" at every forming, which would fill the band |

The fairness rule is untouched: a crew shares battles and company, never
a chip, HP, zenny or a layer. Systems without the one field (the New 3DS,
the PSP) are in crews as anyone and are never offered their battles. The
statistics' `deepnet-together` keeps counting the biggest crew a dive
saw, now one formed by nearness rather than asked for.

## The crew's sign (21 November 2026)

Crews form and split by nearness and nothing is said when they do
(above), so a layer has to show who is in MegaMan's crew (issue #716,
from the testers' idea of a circle round a crew). The game-design
skill's pass, under the owner's direction of 21 November (always clear
who is with you):

- **Dialectic: together against ahead,** made visible: the sign is the
  feedback of walking beside someone, and its dotted form the
  feedforward of what one more step does.
- **Loop layer: the moment on a layer,** where the next battle is
  fought together or alone by who wears the sign.
- **Patterns:** party members marked by colour and a ground ring under
  each (Diablo and Path of Exile's party, an MMO's group), never by
  colour alone (the interface layer's readability floor); the shape is
  the ring, the colour says which crew.
- **BN6 has none:** a map of BN6 holds one Navi, and no sign of a party
  or a group is drawn on it (assumed; searched in bn6f: the map
  scripts' effect commands are the PET's, the scenario effects (the
  Soul weapons' counter, Mr. Weather's rainbow gauge) and the
  minigames'; nothing of a ring, circle or marker among its constants).
  So the sign is ours to draw, over the game's picture.

What is drawn (`dive_people.c`, `floor_ring.c`, the colour in
`crew_look.h`):

- **A crewmate's name in the crew's colour,** the PET's information
  box's green (`CREW_COLOUR`), the others' white. The marks on a
  party's field read the same value.
- **A ring on the floor under each crewmate's feet,** in that green,
  two pixels wide with a dark edge round it, so it reads on a light
  floor, a dark one and a green one; its back is left out where his body stands, so it lies round his
  feet rather than across his legs.
- **A dotted ring under a diver within the radius a crew forms in**
  (`DIVE_CREW_NEAR`, read from `dive_crew.h`) while the server's second
  has not yet passed and the crew has a place: about to be one.
- Nothing over BN6's chat box: a ring's rows under its top are left out
  while it is open, as the names are.

Each call was taken under the owner's direction of 21 November 2026 by
the session building it, and is his to turn over.

| Question | Taken | Chosen over |
| --- | --- | --- |
| The sign | The crew's colour on the names and a ring under each crewmate (the issue's options 1 and 3) | A ring at the radius round MegaMan (option 2): the radius, four panels, is an ellipse some 360 by 180 pixels in BN6's projection, wider and taller than the 240 by 160 picture, so it shows only as arcs in its corners, and a diver the picture shows at all stands within it; tried and looked at. One of BN6's effects as an object (option 4): an NPC slot each, which a layer has few of |
| The colour | One crew colour, the PET's green, the player's own the cursor's orange on a party's field | A colour of each crew's own (a crew is MegaMan's alone on his screen: others' crews are not shown) |
| About to be | A dotted ring for the second before a crew forms | No sign until it has (a crew appearing from nothing) |
| Others' crews | Not shown: only MegaMan's own crew wears the sign | Every crew on the layer in its own colour (the server tells each diver his own crew alone) |

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

- **Name tags** (21 November 2026, `dive_people.c`, `map_label.c`;
  issue #712, from the team's test, where the divers could not be told
  apart): each diver's name over his MegaMan while the layer's map
  shows, on the Square's plate in the Square's letters (`map_label.c`:
  where a place of the map shows in the picture and the plate, moved
  out of `online_labels.c` for both). The names are drawn from the
  puppets as they stand, so a name is there exactly while its MegaMan
  is. Divers on one spot, as at a layer's arrival, have their names a
  row over each other, and those the picture's top leaves no row for
  are counted on the topmost ("Beta +2"). With BN6's chat box open the
  names over it stay and none is drawn on it. MegaMan's own name is not
  shown, as in the Square. The state says `tags N: NAMES`. Seen headless
  on a local server (the scenario `online-week/dive-together` holds
  `tags` on both games): Beta's name going with his MegaMan as he walked
  off, standing over BN6's chat box with A on him, and six divers on a
  layer's arrival spot named in four rows, "Beta", "Gamma", "Delta" and
  "Kappa +1".

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
  battle left him. Linked is now the New 3DS's and the PSP's way (their
  HELLO says `hello_no_arena`), and every party's where seat 0's system is
  one of them; the others fight on one field (below). Seen headless with two games on a local server at layer 3's
  guardian (BN4's FireMan, 1200 HP for two): the queue's band on both, the
  leader's A, both in the fight; Beta's hits (1200 to 1193) on Alpha's
  guardian and Alpha's (to 1191) on Beta's; Alpha's last hit deleted him in
  both games at once, Beta's guardian falling to the HP written. Open: seat
  0 deleted leaves the others without the party's HP (each then fights the
  rest alone), and the leader's idle pass.

- **Crews** (15 November 2026, by nearness since 21 November, issue
  #714; `server/cyberworld_server/crews.py`, `dive_crew.c`; the wire's
  v20 in [shared/online/README.md](../../shared/online/README.md)): the
  server settles every room's crews at each tick from the places its
  divers' games send (STATE, counted only once sent in the room the diver
  stands in), by the calls above, and tells each member the crew as it
  stands (CREW_STATE formed) at every change, and a diver out of one that
  he is (split up). The game keeps what it was told
  (`dive_crew.h`: `dive_crew_count`, `dive_crew_member`,
  `dive_crew_has`, and the radius as `DIVE_CREW_NEAR` and
  `DIVE_CREW_FAR`, the one place the ring, the display and the battles
  read) and says nothing of it but its log's line. Until 21 November
  2026 a crew was asked for: A on a diver's MegaMan opened BN6's chat,
  "Team up,Lan?", and the fork's portal waited for every crewmate
  (`home_way_hold`); both are gone. Seen headless: the scenarios
  `online-week/crew-by-nearness` (two games: a crew where they arrive,
  none once one walks off, none again at five panels, one at three, and
  still one at five panels, looked at six times over three seconds) and
  `online-week/crew-of-four-and-a-fifth` (five games on one spot: four
  in a crew, the fifth outside until one of the four walks off and he
  takes the place); the server's tests (`server/tests/test_dive.py`)
  hold the forming, the edge, a chain, the fifth, a split, a portal
  taken, two crews met.

- **Join-in battles** (15 November 2026, `dive_join.c`, `director_crew.c`;
  `encounter.c`'s `emu_encounters_offer`): while a crewmate stands within
  four panels of MegaMan on a dive layer (128 world units, the room's
  SNAPSHOT), a random battle BN6's roll gives is held at the roll's
  branch (`BN6_ENCOUNTER_ROLLED` taken to `BN6_ENCOUNTER_SKIP`, as a
  bracket holds its rounds) and MegaMan stands still; CREW battle offers
  it by its index on the layer, the band saying "Battle! Calling the
  crew...". A crewmate near whose game gets the offer, MegaMan free on
  the map (no chat box, no door, no party's fight), sends CREW join at
  once, his band saying "Jumping in!": no question since 21 November
  2026 (issue #715, the owner's call after the team's test; until then
  the band offered it, "A:Jump in", for the window). The server starts
  the battle as the last one offered has joined, so the owner's hold is
  as long as that takes; a crewmate who is busy sends nothing, and the
  owner then waits the window out (`ONLINE_CREW_JOIN_MS`) as before,
  until a game can decline (#715's next step, a change to the wire). A
  newer offer replaces an older. The server's START takes every game in to
  the party's core (`party_play_crew`), the battle made alike on each from
  the layer's seed and the index, its kind by the index
  (`director_crew_battle`; no Mystery Data on its field, whose row is a
  record's own, nor a Glitch layer's hardening), the foes unscaled; each
  game's walk toward its next battle is reset there
  (`BN6_STEPS_WALKED`, `BN6_STEPS_CHECKED`), the joiner's run held still on
  its map under the field. Nobody joining (CREW_STATE alone), or no
  answer within the window and three seconds, the owner's battle begins
  as ever. At the end each run takes the battle as one of its own
  (`director_crew_over`): MegaMan's HP as the field left it, a deletion
  ending the run as alone (since 21 November 2026, a won battle's pulls
  him back once a dive: A deletion in a crew, below); won, its viruses counted and BN6's reward, the
  entry BN6's results screen on the field picked (`BN6_BATTLE_REWARD` on
  the party's core, given with `GiveChips`, `GiveZenny` or
  `GiveBugfrags`; a MegaMan deleted and pulled back by the crew takes
  none, A deletion in a crew below): that is BN6's normal reward for the battle, its own
  pick by the Busting Level the party earned, the one the player saw on
  the results, where rolling the run's own would have needed a Busting
  Level no run's battle had; it comes from the rows of each game's ROM
  copy, so the run's leaning of a row's codes reaches it only where an
  earlier battle of the run leaned that row. The server offers nothing to,
  or from, a game without the one field (HELLO's `hello_no_arena`: the
  New 3DS and the PSP), and a field the games built apart lets the
  server's battle go (ARENA leave), the owner fighting alone. The dev's
  `battle` step on a dive with the crew near comes through BN6's roll
  (its way on to StartBattle taken by hand, the branch reading the flags
  the roll left), and a headless game holding for a party's field now
  runs at the clock's pace. Seen headless with two crewed games on a
  local server (layer 1 of week 2966's seed, Beta four panels off): Alpha's
  battle 0 held and offered, Beta's band offering it, Beta's A; both
  made battle 0 alike (`0/23/0@4,1`, one virus), Beta's walk reset from
  22, both ready with `04c5352ceca6a5f3`, 6 hashes agreed on each (frames
  60 to 360), Beta's own Custom screen and picks, the battle won at frame
  378, Busting Level 9, both runs given BN6's MachGun1 T (`262b`) with 100
  HP, back on the layer, the layer's battles 1 on each. Nobody joining:
  Alpha's battle 1 held, "Calling the crew" for the window, then "nobody
  jumped in" and BN6's own battle against a Mettaur alone. A join sent
  from a stale offer showed the fields' check at work: the two games had
  built battles 1 and 2, the ready hashes differed, and each went back,
  Alpha into his battle alone. Again with Beta at 1 HP (`--dev hp=1`),
  two crew battles in a row: both runs given BN6's reward each time
  (`262b`, then `0083`), Beta's HP 1 written back, the layer's battles 2
  on each, so the next battle's index and split are the layer's. Not yet
  seen: a deletion in a crew's battle (Beta at 1 HP was never hit by the
  bots' layer-1 viruses; the guardian's path, `director_party_over`, is
  the same), a crew of three or four, and a crewmate on a New 3DS or a
  PSP in a real crew.
  Unasked (21 November 2026): two crewed games on a local server, Beta
  on Alpha's spot; Alpha's battle 0 held and the crew called, Beta's
  game "jumping into Alpha's battle 0, unasked" with no key pressed,
  both ready with `fe570f879458881d`, the fight begun: the scenario
  `online-week/crew-battle-unasked`, which fails at Beta's step where
  the join is left out.

- **Mystery Data found** (15 November 2026, `dive_finds.c`, `dive_traces.c`):
  a data MegaMan opens on a dive layer is told as DIVE found by its index
  among the layer's (the director's `director_md_count`, `director_md_opened`:
  BN6's flag `MAPSLOT_MD_FLAG` + index set as the game opens it), the same
  index on every game since the seed places them (two games on layer 1 of
  seed 3 logged the same four data at the same places). Once BN6's own box
  of what MegaMan got has closed, the feed's band names the week's first to
  open it ("dtB opened this one first", or "First to open this one this
  week!" where the server's answer names him): BN6 has no place in its own
  box for a name, and the band is where the dive's news already shows. Data
  others opened this week and MegaMan has not shimmer: BN6's own small star
  burst (sprite list 5's `0x05`, `BN6_FX_SPARKLE`, animations 0 and 8 in
  turn every 90 frames) over the crystal, a puppet in a slot the layer
  leaves free, since the crystal's palette is shared by every data of its
  colour and its animation is the game's to run; one sparkle shows, over
  the nearest such data or phrase. Seen headless with two games: the
  second's band after the first had opened data 3, and the burst over
  data 3 on the second's screen before he opened it.

- **Busting splits** (15 November 2026, `dive_splits.c`): a battle on a
  dive layer won is told as DIVE split: its index among the layer's
  battles counted from 0 as the layer is entered (a crew's battle fought
  on the party's core counted too; on a dive the n-th random battle is
  the same for every diver, What it rests on above; a
  guardian's battle is index 255 for
  everyone, since each diver meets him after his own count of random
  ones), its time in tenths from BN6's DeleteTime (`BN6_BATTLE_TIMER`, 60
  frames a second), BN6's Busting Level as the battle ends
  (`BN6_BATTLE_BUSTING`, 1-10 and 11 for S), the guardian's flag
  (`director_guardian_battle`) and a party of 1. The answer, DIVE_PLACE,
  shows in the band ("17.0 sec! Fastest of 1 this week!", or "21.4
  sec,3rd of 9 this week"). The battle's strongest foe (a Navi's NameID
  before a virus's) is kept as the cause a deletion's DIVE end carries.
  Seen headless: a Mettaur's battle won at 17.0 seconds, Busting Level 3,
  answered 1 of 1; DiveMan's guardian battle on layer 3 won with the dev
  menu's Win, sent with the guardian's flag, and WEEK_SPLITS sent to every
  session after it.

- **Deletion marks and phrases** (15 November 2026, `dive_traces.c`): the
  room's traces (DIVE_TRACES as the room is entered and as each comes) are
  kept for the layer, and two puppets in the NPC slots the layer leaves
  free, taken before the divers' (`dive_traces_more` from `dive_people.c`'s
  `layer_objs_more`; the layer's own objects and the seed untouched) show
  the nearest of them within 220 world units: the nearest mark as BN6's
  skull burst (list 5's `0x07` animation 3, `BN6_FX_BURST`, which holds its
  last frame), and the nearest phrase, or data others opened, as the
  sparkle; the marks' slot goes first where the layer leaves room for only
  one. A on a mark (within 22 units of the point 16 before MegaMan) opens
  BN6's chat box with MegaMan's words on who fell there and to what (the
  cause's name from BN6's own, `enemy_name`); A on a phrase shows who left
  it, the phrase and how many waved back, and MegaMan waves at it once a
  layer (DIVE wave; his own phrase is read, not waved at). A phrase is
  left with R on a dive layer's map: the Square's emote strip opens on its
  phrases page alone (`emote_strip_open_phrases`), and A leaves the one
  under the cursor on MegaMan's panel (DIVE phrase); B closes it, and SELECT
  held keeps R for the map and the dev menu. No screen is new: the strip is
  the Square's own. Seen headless with three games: one left "Nice build!"
  and another read it and waved (the server's count came back); a game
  deleted by HeatMan left a mark that the first saw as a skull where he
  fell, and A on it said who and "By HeatMan...". A deletion ends the run
  and leaves the scene in one frame, before the link's frame sees it, so
  the end is now told as the scene is left (`dive_link_scene_left`), and the
  transport sends what waits in its outbox before its goodbye (a dive's end
  had been lost behind it).

- **The Board** (15 November 2026, `square_week.c`, `square_week_words.c`,
  `dive_pack.c`): a "+" after a dive whose guardian was fought in a party
  (WEEK_BOARD's `week_party`) with a box saying what it means; who dives
  now ("Diving now:N", then each diver and his layer, a "!" while he
  fights in a party) from DIVERS; and the week's fastest guardian busts
  (WEEK_SPLITS: "L3 solo:1.5s dtA", "L10 x3:9.8s ..."), three to a box, in
  the Board's posts after the deepest dives. Seen headless in the Square
  while another game dived: "Diving now:1", "dtA:layer 1" (then "layer
  3"), and "L3 solo:1.5s dtA" after his guardian split. The party mark is
  held by `tests/test_online.c`, not yet seen on the Board in a game.

- **One field: the party's core** (15 November 2026, `party_core.c`,
  `party_field.c`): the guardian's fight on a BN6 core of its own that
  every game of the party builds alike. The Arena's boot to the net
  (`core_boot.h`, kept for the next battle); the run's ROM copy as its
  installs left it, each hook's original back under its BKPT
  (`emu_rom_clean`, `hook_originals`: the party's games stand on one
  layer of one seed, so their copies agree where a battle reads them);
  the guardian's record as the run's staging wrote it (`emu_encounter_set`,
  his BN4 Navi's sounds' pool pointed at him), moved to `FREE_RECORDS`'
  first slot; the lead's MegaMan (seat 0's loadout) in BN6's own RAM as an
  Arena core takes one (`arena_loadout_install`); the installs' hooks
  (`xlog_hook_core`) and the field's: the battle's list grown with seats
  1..n-1 as MegaMen on side 0's free panels (`BN6_SPAWN_ENTITIES`, the
  spike's, generalized to a state of its own a core), each one's keys into
  his AI data at `BN6_KEY_COPY` (directions and B: no hand of his own yet),
  each MegaMan's HP and max HP his own loadout's at the battle's first
  iteration, the guardian's HP raised for the party at his spawn
  (`BN6_SPAWN_HP`). The overworld's encounter check forced into
  StartBattle with the record (`BN6_ENCOUNTER_CHECK` to
  `BN6_ENCOUNTER_START`, encounter.c's own way), the core runs with no keys
  to the battle's first Custom screen: the ready. Staged by `boss.c`'s
  `boss_party_fight`: the run's core does not start the battle and holds
  still under the party's (`online_visit`'s holder), which shows the
  party's core's picture and plays its sound. Seen headless with `--dev
  partycore=2,partytwin=1,partyroll=30,partyend=200` at layer 3's guardian
  of `--seed 3` (BlastMan, 400 HP): two MegaMen and BlastMan on the
  field, a second core built beside it from the same keys agreeing at
  every 60th frame (20 of 20, the hash at frame 60 `ac164177e6d71616` in
  two runs), the field rolled back 30 frames every 30 and run again to
  the same hash (41 of 41), the lead's buster deleting BlastMan at
  iteration 309, the win back on the map at frame 1242 (the partners
  stood still then: `partycore` now gives them the spike's script).
- **Over the server** (15 November 2026, `party_play.c`, protocol v19's
  party seats): seat 0's START with format `arena_party` takes the next
  guardian's fight to the party's core. Each game sends its MegaMan
  (ARENA_LOADOUT, kept as the others read him, the pack left home), its
  keys a frame at a time with an input delay (ARENA_INPUT; the Arena's
  `arena_link_net` with a party's seats, `arena_link_net_party`) and its
  field's hash at its ready and every 60th frame (ARENA_HASH), all through
  the server's ARENA_SEAT. A frame runs at its tick: where a seat's keys
  for it have not come, on a guess (his last), at most 8 frames past the
  last frame every seat's keys have come for (`partyroll=W` another), the
  field kept before the first guess (`party_core_save`) and put back and
  run again where the keys that came proved a guess wrong, as the Arena
  rolls its pair back; a hash goes once every seat's keys before its
  frame have come. Keys go at the wall clock's 60 a second, never a burst
  past the server's relay bucket (a headless game twice dropped with
  "Message rate exceeded" before). A seat gone (ARENA_STATE `seat_gone`, or
  this game's link broken) takes his MegaMan off the field (his HP 0) and
  stops the hashes: each game then finishes its own battle. Seen headless
  with two games on a local server at layer 3's guardian of week 2966's
  seed (BN4's AquaMan moved into BN6's engine, his next version for two:
  1000 HP, 1200), `--dev partyend=600,partybot=1` on both (the Arena's bot's keys as each game's pad, the
  guardian at 1 HP at iteration 600): both ready with `69d5611be9442d5d`,
  21 hashes agreed on each (frames 60 to 1260), the battle over at frame
  1261 in both, won. Run again with `partydelay=1`, the frames mostly
  guessed (1250 of Alpha's 1261 frames on a guess, 30 rollbacks; 616 of
  Beta's, 16): the same 21 hashes agreed, each the lockstep run's own
  (`03c86e95f9bbd37b` at frame 60, `052d533a189ea788` at 1260, the bot's
  keys being the frame's), the battle over at 1261 again. Beta's game
  killed partway (`partyend=900`): 7 hashes agreed to frame 420, Alpha
  told "seat 1 gone" at frame 476, Beta's MegaMan off the field, Alpha
  fighting on alone until AquaMan deleted him at 658 (his run over).
  Three games (Alpha, Beta, Gamma; AquaMan two versions up for three,
  1800 HP), the aim passed in turn, the bots' keys: every seat ready with
  `802688165d7ce459`, 9 hashes agreed on each of the three (to frame 540,
  `efcc26a4d7355d52` there), all three MegaMen deleted at frame 586 in
  all three games, lost, their runs over.
- **The end into each run** (15 November 2026, `director_party_over`):
  BN6's results on the party's core, then won or lost and this seat's
  MegaMan's HP as the field last saw it written into the run; won, the
  guardian's course goes on as after a fight of its own (his last word,
  his log-out, his Guardian Data); HP 0, the run ends as a deletion ends
  it, whatever the party did (the owner's call). Seat 0 reports the end
  (ARENA_STATUS, ARENA over). In the run above Alpha, the lead, was
  deleted before the win and his run ended; Beta won with 100 HP and took
  AquaMan's data; every diver's feed said "Alpha's party of 2 beat layer
  3".
- **Switched on** (15 November 2026, `online_transport.c`'s HELLO): every
  system but the New 3DS and the PSP stops saying `hello_no_arena`, so the
  server seats them on one field; `dive_party.c` hands a START of format
  `arena_party` to the party's core and a seat gone to it, and fights
  `arena_linked` as before; a crew's join-in battles take the same START
  to the party's core (Join-in battles, above).
- **Who the enemies aim at** (15 November 2026, `party_field.c`'s
  `party_field_aim`): a probe first, side 0's first two slots of
  `BN6_BATTLE_PLAYERS` swapped at iteration 300 against BlastMan: his
  next hit (377) still struck the lead, every one after (694, 929, 1417,
  1706) the partner now in slot 0, who stood still; the panels under every
  MegaMan held the same flags. So BN6's enemies take their aim from side
  0's first slot (bn6f battle_findPlayer, `BN6_SIDE_PLAYER_GET`, returns
  the side's first player-like actor), deciding it some frames before the
  hit. On the party's core that slot now holds, while an object of side
  1's routine runs (`BN6_OBJECT_LOGIC_CALL`, bn6f RunBattleObjectLogic's
  call, to `BN6_OBJECT_LOGIC_BACK`), the MegaMan whose turn it is, each
  standing one 240 iterations in turn, the lead's object back as the
  routine returns; every other routine sees BN6's own order.
  (`partylead=1` gives BN6's own aim.) Against BlastMan with three
  MegaMen his hits went to all three in turn (377 actor 1, 694 actor 2,
  929 the lead, 1417 actor 2, 1692 the lead, 1955 actor 1, 2415 the
  lead), a twin core and 30-frame rollbacks agreeing every time (56 and
  113 of them); over the server with two games (`partybot=1`,
  `partydelay=1`, AquaMan for two) 16 hashes agreed on each to the
  battle's end at frame 966, both bots' MegaMen struck and deleted, lost
  in both. The turn's length is a first pick, for the owner's eye.
- **The partners' own chips** (15 November 2026, `team_shape.c`,
  `party_turn.c`; the two-a-side's [NaviStats](arena/two-a-side/navistats.md),
  [hands](arena/two-a-side/hands.md) and [Custom
  screen](arena/two-a-side/custom-screen.md) grown from a partner a side to
  three on side 0, not written again): `team_shape.c` names a core's
  partners, the Arena's (partner p on side p, the side's second slot) or a
  party's field's (side 0's slots 1-3, read through the aim's trade), and
  `arena_team_navi.c` and `arena_team_custom.c` take a partner by it. Each
  partner's record is his own MegaMan's NaviStats as BN6 compiles them from
  his NaviCust over the party's kept boot (`team_navicust_compile`, the
  two-a-side's, the same on every game), in the fourth kind's last three
  slots (the Arena's last two); his deck is dealt from his own folder as
  the field's first Custom screen opens and lives in the field's RAM
  (`RAM_TEAM_ROOMS`, which holds the Arena's link words and turns where a
  NetBattle runs: a party's field runs no link, so its three partners'
  rooms take the span); his folder is folded into the field's hash. His
  Custom screen is BN6's own on a core of his own game
  (`team_screen_one`: the field copied in, his deck and record written
  as side 0's, the screen's step put back so BN6 deals his hand and runs
  its rules), shown while his turn is open; only his picks cross, as the
  two-a-side's do (`TEAM_PICK`, a Program Advance's `TEAM_PA`), in his
  seat's stream (a party's seats 1-3 keep sixteen bits,
  `arena_link_net.c`), one a frame, never guessed again. Every game takes
  them at the same frame of the field: as each frame begins
  (`hook_run_on`, a hook of the field's core between frames) each
  partner's word goes to his turn (`arena_team_custom_party`), his picks
  set once the field's screen has closed and written into his own hand at
  the battle loop's next iteration; A then uses his chip. The lead's OK
  waits for the partners still picking through the two-a-side's hold on
  the screen's packet (`arena_team_wait.c`), 30 seconds at most after the
  first OK, then his picks none. Seen headless with `--dev
  partycore=2,partytwin=1,partyroll=30,partybot=1` at layer 3's guardian
  of `--seed 3` (BlastMan, 400 HP; partybot=1 now stands each partner in
  with his own screen on this game): partner 1's NaviCust compiled (HP
  100, Custom 5), his screen open at the field's frame 1 with his 30-chip
  deck, his OK's three picks (0C0, 09A, 036) said, his own hand taking
  them at iteration 1 while the lead's held its own, his first chip used
  at iteration 4831 (his hand's next 0 to 1, the lead's at 2); the twin
  core agreed at every 60th frame and every 30-frame rollback ran again to
  the same hash to the battle's end at frame 6176 (308 checks, none
  apart), lost. Over the server (two games through the portal, the dev
  menu's "Next guardian" and the door's queue on a local server, later the
  same day): both built the same field (`5bf0a16fb06d8b79`), "every seat
  ready", the fight begun; Beta's own screen opened with his own deck (30
  chips, the first 0x09A) and his pick crossed ("seat 1's turn 1: 1 picks
  said"). Not yet seen: a whole turn and the battle to its end over the
  server (two games driven at once by hand did not get the lead's OK
  through: a driver of its own is to write), a partner's own turn held
  past the lead's OK, a Program Advance of a partner's, his Cross and
  Souls (refused on a party's field for now: the Arena's).

## Who is who on a party's field (21 November 2026)

The team's test over Discord of 18 November
([issue #713](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/713)):
on a party's field, a guardian's or a crew's join-in battle, four of
BN6's MegaMen stand in one palette and nobody could tell which was his.
BN6 itself leaves an ally unlabelled (the Arena's finding,
[the partner's seat](arena/two-a-side/seat.md)).

**The call** (taken under the owner's direction of 21 November 2026, that
the online game is to feel like co-op in a modern MMO or Diablo: always
clear who is who; his to turn over): a mark by each MegaMan's feet, his
diver's name, the player's own told apart first. Chosen over a palette
per MegaMan, which changes what the core draws and so must be alike on
every game of the field or kept out of its hash; that waits for
[issue #694](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/694),
a look of the player's own, where colours are the first step. Read
through the game-design skill's interface layer: a spatial label on the
actor, the question "which one am I" answered by a shape as well as a
colour.

- **The name, cut to a panel.** Whole letters of the diver's name that
  fit 34 pixels of the engine's pixel font: a panel is 40 wide, so two
  MegaMen on neighbouring panels of a row keep their names apart, and
  rows stand 24 apart, more than a name's 9. Two names alike as cut get
  their seat's digit after them; a seat with no name (the dev's stood-in
  partners) is "P1" to "P4", the Arena's marks. Centred under his feet.
- **The player's own** in the PET cursor's orange behind its arrow, so
  it does not rest on colour alone; the others in the crew's colour,
  the PET's green, as their names on a layer
  ([The crew's sign](#the-crews-sign-21-november-2026): `CREW_COLOUR`
  in `src/online/crew_look.h`).
- **A partner's HP his own.** BN6's HP box shows side 0's first actor,
  the lead, on every game of the field; a partner sees his own over it,
  as a two-a-side's partner does (`arena_team_view_hp`).
- **When.** While the field fights, not over a partner's own Custom
  screen, and on the field's Custom screen the HP box alone, as the
  Arena's. A MegaMan deleted or gone has none. A linked fighter (New 3DS,
  PSP) fights alone and has nothing to mark.

As built (`src/online/party_marks.c`, drawn by `party_play_draw`): our own
drawing over the field's picture, read from the field's core as it
stands for the frame shown, so a rollback leaves nothing behind; it
writes nothing, and nothing of it is in the field's hash. The names are
the seats' sessions in START as the server's snapshot names them, this
game's own the visit's. A partner's HP stands under his name (The
crew's display, below). `--dev partyview=N` draws a
`partycore` field as seat N sees it, and the state says it (`partyfield`,
`hpbox`, `markK own|crew at X Y width W tag T`, `marks N apart|MEET`).

Seen headless (21 November 2026; the scenario
`tests/scenarios/online_party_marks.py`): three MegaMen against layer 3's
guardian of `--seed 3` (BlastMan) drawn as seat 1 sees them, "P2" his own
behind the arrow, P1 and P3 the crew's, his HP box his own (read from his
object, side 0's actor 1), no marks over the Custom screen; with
`partytwin=1,partyroll=30` 121 checks of the twin and the rollback the
same and none apart, the marks drawn. Over a local server, two crewed
divers in a crew's join-in battle, one named sixteen w's: each game his
own name in orange, the other's in white, "Wwwww" cut to the panel, both
marks apart, the joiner's HP box his own, every 60th frame's hash agreed.
Not yet seen over the server: a guardian's door (the same drawing, on the
same field).

## The crew's display (21 November 2026)

The team's test of 18 November asked for a display of the group, each
one's level, layer and HP
([issue #717](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/717)).
The owner's direction of 21 November 2026 is co-op as in a modern MMO
or Diablo: the party frame, at a glance who is in my crew, how they are
doing and where they are. BN6 draws no such thing: on the map its HP
box alone (seen in every capture of a layer, as in [Who is who on a
party's field](#who-is-who-on-a-partys-field-21-november-2026)'s), and
it has no players together on a map at all.

**The calls** (taken under the owner's direction of 21 November 2026;
his to turn over), through the game-design skill's interface layer (a
party frame is a non-diegetic overlay the eye returns to, so it stays
small, in one place, and covers nothing it needs to read):

- **The crew alone**, three others at most, never the room's seven: the
  crew is who plays together ([Crews by nearness](#crews-by-nearness-21-november-2026)).
  Chosen over every diver on the layer, whose plates would be noise.
- **Always while in a crew**, no key: the crew changes as people walk,
  and a key would hide it when it matters. Alone, or with the link down,
  nothing.
- **Where**: a plate a crewmate under BN6's HP box at the picture's top
  left, its column, covering nothing of BN6's (the box, the chat boxes at
  the foot, the band). Chosen over the band grown (one line, and the
  feed's), and over a second screen alone (most systems have none).
- **What**: his name, cut as the field's marks cut it; his HP as BN6
  writes HP, over his max HP; a bar of it under the plate, orange from a
  quarter down; and his layer ("L3") where it is not MegaMan's. **"Level"
  is max HP**: a dive starts from a standard profile, so the game has no
  level, and what grows with a dive is max HP (HPMemory, the guardians'
  data); the act and the layer say how deep, not how strong. Chosen over
  the act and over Busting Levels (a battle's, not a diver's).
- **In battle**: a partner's HP under his name on a party's field
  (`party_marks.c`), as BN6 writes an enemy's under him; MegaMan's own
  is the HP box. A crewmate fighting his own battle: his plate's HP
  falls as he is hit, read from his battle.
- **Later, each its own issue**: a second screen's page (`src/second/`,
  [issue #729](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/729)),
  and the divers' dots on the PET's map (decided 15 November, not built;
  [issue #728](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/728)).

As built (`src/online/dive_crew_view.c`, protocol v24 in [the
wire](../../shared/online/README.md)): each game sends its MegaMan's HP
and max HP as they change, and again where the crew's word of them
differs (DIVE vitals, half a second apart at most;
`director_megaman_hp`, the map's or his battle object's); the server
keeps it and tells his crew alone, in CREW_STATE's members with each
one's layer, as they change. The plates are our own drawing over the map
and change nothing; the state says them (`crewview N`, `mateK NAME hp
H/M layer D|here`), and the field's marks say a partner's HP (`markK ...
hp N`).

Seen headless (21 November 2026; the scenario
`tests/scenarios/online_crew_view.py`): two divers on the week's net
over a local server, a crew by standing together, each shown the other's
plate, "Beta 100/100"; Beta's own battle against a Mettaur, his HP 90 on
Alpha's plate while he was still fighting; Beta on to layer 2 by the dev
menu, "Beta L2 90/100" on Alpha's; ten seconds on, out of the crew, no
plate. With the server stopped mid-crew: no plates. In those runs
Beta's game logged "crew none" around his battle's end and the crew
formed again once both stood together (#725's crews by nearness): his
plate goes and comes back with it.

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

## A deletion in a crew (21 November 2026)

Issue #718, after the team's test: with crews formed by nearness and
their battles joined unasked (#714, #715), a diver is taken into battles
he did not choose, and a deletion there ended his dive. The design pass
(the game-design skill, [its comment on the
issue](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/718#issuecomment-7067358973))
and the owner's four answers:

- **The dialectic stays together against ahead.** Helping costs the
  helpers HP for the rest of the layer; the fix sits where the sting is,
  at the end of the battle the crew fought together.
- **Loop layer: the dive's own battles**, feeding the Board as before.
- **Patterns** (recalled from playing them, not checked in
  [Sources](../research/sources.md)): a revive that costs the helper
  (Borderlands' and Diablo III's hold-to-revive, without the downed
  state: in a joined battle the crew already stands beside the fallen);
  a graduated death penalty, a buffer of one before the dive's own
  permanent death (Path of Exile's and Diablo's hardcore stay what a
  second deletion is); honour on the Board, as the party's "+". Turned
  down: a ghost carried to the next layer (Risk of Rain 2, Enter the
  Gungeon's co-op), which gives a layer; a revive as long as someone can
  pay, which lets a crew carry one diver down; a crew's shared budget
  (Monster Hunter's carts), which makes one fall cost the others.
- **The rule, as built** (`src/director/crew_pull.c`): a crew's random
  battle won with a MegaMan of it deleted, each crewmate still standing
  gives a quarter of the HP he ends with (in seat order, a quarter of
  what he has left), and the fallen is back on the layer with the sum,
  half his own max HP at most and 1 at least. A giver is never deleted
  by it, and a win with nobody standing (every MegaMan deleted as the
  last foe fell, which the field can report) pulls nobody back. Once a dive: a second deletion ends the dive as alone. The
  fallen takes neither the battle's viruses nor BN6's reward; the dive's
  end carries a mark to the Board, `*` after its layer ("*:pulled back
  once by the crew."), ranked with the rest. A field lost (BN6 ends a
  battle when no MegaMan of side 0 stands), a guardian's party, a diver
  alone and a linked fighter (the New 3DS, the PSP: they get no crew
  battles, and their guardian is their own battle): as before.
- **How it is built.** Every game of the field reads each seat's HP as
  the field ended (`party_core_hp`), his max and whether his pull-back
  is spent (the loadout's flags' second bit, ARENA_LOADOUT, v22) and
  works out the same gifts (`director_crew_over`, `crew_pull.c`), so no
  message carries them; `director_crew_over` writes the HP and, pulled back,
  spends the dive's pull-back in its record (`weekly_pulled`:
  `weekly.sav`'s `pulled`, kept at once, so a quit and a CONTINUE from
  an earlier checkpoint find it spent), not in `Run`. There is no state
  of being down: it is decided in the battle's last frame. The band says
  "Phew... The crew pulled us back!" on the fallen's game and "Lan,we
  shared our HP. They're back!" on a giver's. `--dev partydel=MASK` with
  `partyend=N`, the same on every game of a field, deletes the MegaMen of
  those seats (bit 0 seat 0) at the battle loop's iteration N, as a hit
  would, and its enemies 60 iterations later: the scenarios' way to a deletion on a week's layer, which the
  server's seed makes another each run.

## Open before it is built

1. **The join window.** Built (Join-in battles, above): the two-a-side
   hook trades the battle's list at its spawn (`BN6_SPAWN_ENTITIES`), so a
   MegaMan cannot be added to a battle already begun, and the offer comes
   as the encounter starts, BN6's roll held while the server's window
   (`ONLINE_CREW_JOIN_MS`) waits for the answers. How the window felt
   (three seconds of MegaMan held still, and a question each battle) the
   team's test answered: since 21 November 2026 crewmates near join
   unasked (issue #715). Open: a crewmate who is busy still costs the
   window, until a game can decline.
2. **One field against a guardian.** Built (As built, above), the
   guardian's aim passed over the party in turn, each partner his own
   NaviStats, hand and Custom screen. Open: the turn's length, a partner's
   Cross and Souls, and the over-the-server proof of the partners' chips.

3. **The version ladder** for BN5's and BN4's Navis and for guardians
   with no SP of their own.
4. **A linked fighter in a party**: whether they count in its size, and
   which version their own battle's guardian is.
5. **Crews' battles unscaled.** Viruses end about half the lost short
   nets ([The core loop, reviewed](../game/core-loop.md)), so a crew dies
   to them less often, and the Board's party marks count guardians only.
   The owner's call stands; the statistics will show what it does. Since
   21 November 2026 a crew's win also pulls a deleted crewmate back once
   a dive, which the Board marks (A deletion in a crew, above).
6. **A quiet net.** With live divers only, the dive is as busy as the
   server: in October 31 players visited the Square and 6 posted dives
   (the same page). The statistics count it from the first release: as a
   dive ends, `deepnet-together` says the layers it entered, on how many
   another diver was too, the most divers on one, the biggest crew, the
   party fights and the biggest party (`analytics_dive_together`); joins
   come with the join-in battles.
7. **The wire.** Layer channels keyed by week, depth, side and way;
   positions; crews; the queue; the party's field; the feed, marks,
   phrases and first finds kept per week; a protocol version. A `Run`
   change only if a dive must hold a crew's or a party's state across a
   checkpoint.
8. **The words.** Every line this needs (the portal's, the offer to jump
   in, the flare, the marks) goes through [the voice rules](../voice/rules.md).
