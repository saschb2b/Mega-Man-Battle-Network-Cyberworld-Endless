---
type: Design
title: "The shared dive"
description: "The week's net dived together (being built since 15 November 2026): a portal in the Square, every diver on the seed seen live, crews of four formed by walking together, join-in battles, a queue of four at each guardian's door fought on one field and scaled by BN6's own versions, and the owner's calls on fairness."
tags: [online, game]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-23T17:28:20Z }
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
| Random battles | Each diver's own; crewmates within a few panels fight it too, unasked and at once, a busy one saying so at once (21 November 2026, after the team's test: walking close is the yes, walking apart the no; [A crew's battles at once](#a-crews-battles-at-once-21-november-2026)); only crewmates, who since issue #714 are whoever stands near | Crewmates offered it, A to jump in (15 November's call, as played: a question and three seconds held still at every battle); anyone near; the battle's owner accepting each |
| A joined battle | One field; the foes as a solo diver meets them, unscaled; each fighter BN6's normal reward; joining resets the joiner's own encounter count, so each diver fights about as many battles as alone | Foes' HP times the fighters; their versions raised; no reward; a cap a layer |
| The door's queue | Up to four, each having walked to this door; divers behind on the same way get a flare, and those on the door's layer (#752) | No flare; helpers summoned from behind |
| The leader | The first at the door starts any time; anyone leaves any time; a leader idle a minute passes the lead on | A countdown on a join; a ready check |
| The clock | Runs while the queue waits | Paused, capped; paused |
| The fight | One field: the party's MegaMen together against the guardian | Linked fights, each in their own battle with the guardian's HP shared |
| Scaling | (22 November 2026, issue #754) The guardian's version a diver alone meets, his HP times the party: per head the same fight as going alone, measured ([A party's guardian](#a-partys-guardian-22-november-2026)) | (15 November's call, as built till then) two meet his next version, three or four the one after, HP times the party: a fighter paid up to 3.3 times the solo's time and 9.4 times his HP lost; one version up from act 3 on; one version up and HP 1.5 times a fighter; HP alone; none; harder hits past SP; a cap at SP |
| A drop | The missing MegaMen leave the field in each game, and each game finishes its own battle | Broken off, back to the door; the dropped run ended |
| Deleted in a party | At a guardian: the run is over, as alone. In a crew's random battle the crew wins: pulled back once a dive, paid in the crew's HP, the dive marked (21 November 2026, issue #718; A deletion in a crew, below) | Pulled out at 1 HP and carried by a win; pulled out to fight again; (21 November) as at a guardian; back once a dive, free; at a guardian's party too; a fixed 100 HP a helper; the helper with the most HP paying alone; revived dives ranked below the rest |
| Systems without the Arena | The New 3DS and the PSP join as linked fighters: their own battle, their damage into the field's guardian | Alone only; the whole party linked |
| After a dive ends | Dive again from layer 1 into the live pack, as often as one likes, the best kept | Once a day; the first dive alone counted |
| A quit mid-dive | (22 November 2026, issue #759) The title's CONTINUE straight back into the dive at its checkpoint, the link taken up as the portal does (Back to the dive, below) | A question at the title; the way back through the gate and the portal, as it was |
| The Board | Its ranking unchanged (layer, a win, minutes); a mark for each guardian fought in a party; who dives now and on which layer; the week's fastest bust of each guardian, alone and by party size; (22 November 2026, issue #759) a dive's end posted by the server from DIVE end, no visit to the Square waited for | (all four taken); the Board alone fixed, the diver's own post kept as the only way |
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
| The radius | A crew forms within four panels (128 world units, the join-in battles' old reach, `crew_near`) and holds within six (192, `crew_far`): a crew at the edge does not form and split with every step; since 22 November 2026 a box on screen instead ([On screen only](#on-screen-only-22-november-2026)) | One radius both ways (flickers at its edge); a wider one (strangers across a room pulled in) |
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

## On screen only (22 November 2026)

Issue #750, from the friction hunt: four panels is an ellipse some 360 by
180 pixels in BN6's projection, wider than the 240 by 160 picture, so a
crew formed and held with a diver the player could not see, and his
random battle pulled the other in from off screen, nothing on the
joiner's screen saying whose battle it was. **The owner's call (22
November 2026, on the issue): on screen only.** A crew forms only with a
diver you can see and holds no wider than the picture; a battle you are
pulled into names whose it is. Chosen over keeping the reach and drawing
an arrow at the picture's edge.

**The reach in the picture's pixels.** BN6 draws one who stands (dx, dy)
world units from MegaMan at dx + dy pixels across and (dy - dx) / 2 down
from MegaMan's feet (bn6f's sub_300631C, as `map_label.c` reads it), and
MegaMan's feet stand at (120, 82) of the picture. Seen from either of two
divers the other is at the same offset with its sign turned, so the
reach is a box of the same size both ways: across, his body (a Navi some
24 pixels wide) inside 120 pixels to either side; down, his head (31
pixels over his feet) inside the 82 rows above MegaMan's feet, the
tighter of the two ways, since the 78 rows under them leave more.

| | Across | Down | In world units along one axis |
| --- | --- | --- | --- |
| The picture: the other's whole body inside it | 108 | 51 | |
| Holds (`crew_far_wide`, `crew_far_high`) | 100 | 44 | 88 (2.75 panels) |
| Forms (`crew_near_wide`, `crew_near_high`) | 80 | 32 | 64 (two panels) |

The hold sits 8 and 7 pixels inside the picture's edge: room for a raised
floor's few rows and BN6's camera a step behind. The form is a fifth to
a quarter inside the hold, so a crew at the edge neither forms nor splits
with each step (the hysteresis kept). A diver walking off leaves the
crew `crew_split_ms` (two seconds) after he passes the hold, while he is
still in sight; meanwhile no battle is offered him.

What changed:

- **One place for the numbers:** `protocol.json`'s `crew_near_wide`,
  `crew_near_high`, `crew_far_wide`, `crew_far_high` (protocol.h's
  `ONLINE_CREW_*`), read by the server (`crews.py`'s `on_screen`) and the
  game (`dive_crew.h`'s `DIVE_CREW_NEAR` and `DIVE_CREW_FAR`,
  `dive_crew_within`): the dotted ring of a crew about to form (#716)
  follows the same box. The server's tests read them from there.
- **A battle offered only to those in sight:** the server offers a
  crewmate's battle only to those within the hold of its owner now, so one
  walking off in his last two seconds of the crew is offered none; and a
  game declines (CREW busy) the battle of an owner its own picture does
  not show (`dive_people_in_sight`, BN6's displayed camera), as a camera
  held at a layer's edge is not where MegaMan stands.
- **Whose battle:** the joiner's band says "Alpha's battle!" from the join
  for two and a half seconds, over the battle's start too (`dive_join.c`,
  the words in `dive_words.c`); the state's `band` says what it shows.
- The state's `insight N: NAMES` names the divers whose feet are in the
  picture.

Seen headless with two games on a local server (the scenarios
`online-week/crew-on-screen-only` and `crew-battle-named`): Beta 80 units
off along x still a crew and in Alpha's picture; at 96 past the hold,
still in sight and for two seconds still in the crew, Alpha's battle
offered him nothing; the crew gone while each still saw the other; and
Beta, taken into Alpha's battle, read "Alpha's battle!" over its start.

These numbers were taken under the owner's call of 22 November 2026 by
the session building it, and are his to turn over.

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

## A crew's battles at once (21 November 2026)

The rest of issue #715, under the same direction (no prompts, no
waiting). The game-design skill's pass: the dialectic is together against
ahead at the scale of a battle, the company's price paid in battles
fought rather than in waiting; the loop layer the moment on a layer.

How often a crew fights, from BN6's own odds (`BN6_ENCOUNTER_ODDS`,
[docs/rom/battle.md](../rom/battle.md)): a roll every 0x40 units walked,
its odds out of 32 rising from 0 with the walk since the last battle,
and a battle clearing the walk. Every crewmate rolls, the first battle
pulls in the others and clears their walks too, so a crew of N walks to
the first of N rolls. With the layers' classes (3, 5 and 6) a crew of
four fights 1.39, 1.29 and 1.12 times as many battles over the same
ground as a diver alone (two: 1.19, 1.15, 1.07; three: 1.31, 1.23, 1.10);
the rising odds keep it far under four times, since early in a walk no
roll can win.

Each call was taken under the owner's direction of 21 November 2026 and is
his to turn over.

| Question | Taken | Chosen over |
| --- | --- | --- |
| No window | Every crewmate free goes in at once; the owner stands still only while the server starts the battle (65 ms seen headless on one machine) | The three seconds' window (15 November's) |
| Who is left out | A crewmate not free (a chat or BN6's PET open, a shop, a guardian's door, Mystery Data opening, his own battle held) answers CREW busy at once (protocol v23) and stays where he is; the battle starts with the rest | Waiting the window out for him; pulling him in from where he stands |
| Saying no | None per battle: walking apart is the no, as the crew is by nearness ([Crews by nearness](#crews-by-nearness-21-november-2026)) | A button to stay out of a battle |
| How often | As it comes: up to about 1.4 times as many battles for a crew of four, each fought together, the company's price on the ground walked | Only one crewmate rolling (one game's walk deciding for all); the odds lowered for a crew (BN6's own roll changed) |
| What is said | Nothing while the owner's battle starts ("Battle! Calling the crew..." gone); "Jumping in!" while a joiner's starts | A line for a hold no one sees |

## Dots on the map (21 November 2026)

Decided on 15 November (every diver on the layer a dot on the PET's
map) and built for issue #728. The map in a run is the engine's own, not
a screen of BN6's: the layer's map SELECT holds over the picture and the
second screen shows (`src/director/director_map.c`; BN6's PET has no map
of a layer). MegaMan's dot there is his panel (`netmap_panel` of where
BN6 has him, the map's last look at him) put on the map's grid; each
diver's is his place in the server's SNAPSHOT of the layer's room put
through the same two steps, so a dot stands where his MegaMan stands.
The game-design skill's pass, under the owner's direction of 21
November (always clear who is with you and where they are):

- **Dialectic: together against ahead,** the crew's sign carried from
  the layer to the map: where the crew went when it is out of the
  picture, the feedback of walking apart before the crew splits.
- **Loop layer: the moment on a layer,** the walk to the exit or the
  guardian's door, and whom to walk it beside.
- **Patterns:** party members on the minimap (Diablo, Path of Exile, an
  MMO's group): the party's in its colour and at the frame's edge when
  out of it, others there but quieter; never by colour alone (the
  interface layer's readability floor): the shape tells them apart.

What is drawn (`director_map.c`, lent the divers by `dive_dots.c`
through `map_divers.h`, the director being under online play's layer;
the room's divers kept from each frame's snapshot as the puppets are,
and no one's dot once half a second has passed without one, the link
down):

- **A crewmate:** a round dot, white in a ring of the crew's green
  (`CREW_COLOUR`, the ring under his feet on the layer); past the
  frame, an arrowhead of that green on its edge his way, as a sensed
  service's.
- **Another diver:** a round dot of the PET's rails' grey, solid and
  dimmer, left out past the frame. A dot of three pixels was tried
  first and lost on the map's dark.
- Round, where the places marked are squares (a green one is Heal) and
  the data diamonds; MegaMan's square dot stays over them all.
- The key names them: "Crew" and "Divers" after "You", each where the
  layer has one.
- Nothing drawn over the layer's field stands on the map while SELECT
  shows it (`director_map_over`): a crewmate's name and his ring, drawn
  after the map, had stood on it where he stood in the picture, beside
  his dot.
- The state says `mapdots N: NAME(DX,DY)`, each diver's panel from
  MegaMan's, `*` a crewmate's. Seen headless with two games on a local
  server (the scenario `online-week/divers-on-the-map`): each game's
  dot of the other two panels off and ringed in the crew, eighteen off
  once he walked away, its ring gone with the crew, and moving with
  him again.

Each call was taken under the owner's direction of 21 November 2026 by
the session building it, and is his to turn over.

| Question | Taken | Chosen over |
| --- | --- | --- |
| Who is a dot | Every diver in the layer's room, the crew ringed in its colour, the others dimmer | Only the crew (the others then nowhere off the picture); every diver alike (the crew lost among them) |
| Another layer | No dot: the map is the layer, and a diver on another is in another room, which the server does not send | A mark for a crewmate gone ahead (the crew splits within ten seconds of his going) |
| Off the frame | The crew's arrowhead on the edge, as a sensed service's; the others' left out | Every diver's arrowhead (a frame's edge full of pips) |
| Unseen floor | A diver's dot shows where he stands, on floor MegaMan has not seen too | Hidden until MegaMan has seen that floor (where the crew is matters more than what the map reveals of a room) |

## Room for the crew (22 November 2026)

The other divers are puppets in BN6's overworld NPC objects
(`eOverworldNPCObjects`, a fixed array of 16 at `0x020057B0`,
`BN6_NPC_COUNT`, [the game's structures](../rom/game-state.md)). A layer's own
take them first: its service Mr.Progs and keepers, one object for each
Mystery Data, the exit pad, a guardian's seal, body, beam and data. The
dive's puppets get what is left. Issue #755 found a crew of four at layer
3's door, where the layer left two: each game drew two of the other
three, strangers before its own crew, so its plates named crewmates its
picture did not show.

**How much a layer leaves**, counted by the log's `layer: N of the 16 NPC
objects its own` over 40 seeds at each depth from 1 to 24 (a run's
layers; a dive's are the same generator's, from the week's seed):

| Depth | Slots left: least, median, most | Layers leaving 2 or fewer |
| --- | --- | --- |
| 1 | 4, 6, 8 | 0 of 40 |
| 2 to 7 | 0 to 2 at least, 3 to 6 median | 1 to 18 of 40 |
| 8 to 13 | none to 1 at least, 1 to 4 median | 9 to 32 of 40 |
| 14 to 24 | none at least, 0 to 3 median | 18 to 40 of 40 |

Past the first act most layers leave two slots or fewer and many none:
the Mystery Data take the most (six or seven objects on a deep layer),
then five talkers. Some leave less than none (16 to 17 of their own):
the layer's own last object is then never spawned. That is a fault of
its own, outside the dive, left with issue #755's follow-ups.

**What changed** (`dive_people.c`, `dive_seats.c`, `dive_traces.c`):

- **A full crew's MegaMen first.** As a layer is built, the dive takes up
  to three slots (a crew of four's others) before the week's traces take
  theirs; the traces' marks then take one where one is left, the sparkle
  one where three are, and further divers what remains. Before, the
  traces took theirs first wherever three were left.
- **The crew first, then the nearest.** Each frame the room's divers are
  ordered, crewmates first, then by distance to MegaMan, and the first
  as many as there are puppets get one (`dive_seats`). A diver keeps his
  puppet while he stays among them, so no MegaMan jumps from one diver to
  another as people walk.
- **A diver with no MegaMan still shows where he is.** His name stands
  over the spot where he stands and, for a crewmate, the crew's ring on
  the floor under it, exactly as for one drawn; only his body is missing.
  The crew's plates, the map's dots and his battles then agree with what
  the picture shows. The state's `undrawn N: NAMES` (`*` a crewmate)
  counts them.
- `--dev diveroom=N` gives each layer of a dive N puppets at most, so a
  test meets a layer with little room whatever the week's seed makes.

Seen headless with four games on a local server (the scenario
`online-week/crew-drawn-with-little-room`, `diveroom=2`, all four at
layer 3's door in one crew): each game drew two crewmates and gave the
third his name and ring, `undrawn 1`.

Each call was taken under the owner's direction of 21 November 2026 by
the session building it, and is his to turn over.

| Question | Taken | Chosen over |
| --- | --- | --- |
| Who gets a MegaMan | The crew, then the nearest | The snapshot's order (as built: strangers before the crew); the nearest alone (a crewmate across the room left out for a stranger beside you) |
| The traces against the crew | A full crew's three MegaMen before the traces' marks and sparkle | The traces first (as built); no traces on a dive with others |
| One with no MegaMan | His name and his ring where he stands | Nothing (as built: the plates and the map named someone the picture did not show); a count only ("+1 here"), which says nothing of where he is |
| More objects | Not here: BN6 runs 16 NPC objects, and moving the Mystery Data out of them, or a MegaMan drawn another way, is work of its own (issue #755's follow-up) | Raising the 16 (every loop over BN6's array, and the RAM after it) |

## Nothing over BN6's screens (22 November 2026)

Issue #781, from the second play of the dive: our drawing over the
game's picture landed on BN6's own and on our cards in five places, the
joiner's band over a crew battle's Custom screen, the divers' names and
rings across the act's and the guardian's title cards (the arrival's
mark running out unseen under them), a plate's layer run into its HP
("L1100/100"), a partner's mark on the field's bottom row cut at the
picture's foot, and a dropped game's crewmates drawn frozen under
"Reconnecting...".

**One answer** (`director_cover`, `src/director/director_cover.c`): what
stands over the layer's field now, nothing, BN6's chat box, a title card
of ours (an act's, an area's, a guardian's), BN6's fade or a warp, or no
field at all (a battle, BN6's menus, SELECT's map). Everything the dive
draws over the field asks it first: under a chat box only what stands
above the box, under the rest nothing at all. The names, rings, the
arrival's mark, the crew's plates and the band all go through it, so the
next thing drawn there cannot land on a screen of BN6's either. The
state's `cover` line names it.

- **The joiner's band** stands on the map while the join is sent, over
  the crew battle's staging (the still picture of the map, made in a
  burst, too short to read alone), and on the field for two and a half
  seconds from its first Custom screen's close; never over a Custom screen
  (`party_play_drawn_over`). `band` in the state says what is drawn.
- **The feed waits:** a line said while the band cannot show it (a card,
  a chat box, a fade, a battle) keeps its three and a half seconds for
  when it can, rather than spending them unseen.
- **Title cards:** no names, rings or arrival mark under one; the
  arrival's two seconds are counted from the card's end.
- **The plates:** six pixels between a crewmate's layer and his HP
  ("L1 100/100").
- **The field's marks:** a mark and its HP never reach BN6's chip name
  at the picture's foot; on the field's bottom row they take another spot
  beside the MegaMan ([A crew in one column](#who-is-who-on-a-partys-field-21-november-2026),
  issue #886).
- **A dropped link:** while it is down (dropped, tried again, or given
  up) the crew is put away, the puppets, names, rings, dots and plates
  none, until the room's divers are heard again; the band's
  "Reconnecting..." is what `band` says.

Seen headless with two games on a local server (the scenario
`online-week/drawn-over-nothing`): Beta arriving where Alpha stood, his
act's title card with no name or ring across it and the arrival's mark
after it; Beta's link cut for four seconds, his crew put away under
"Reconnecting..." and back with the link; Beta taken into Alpha's battle
reading nothing over the field's Custom screen and "Alpha's battle!" on
the field once it closed.

## As built

- **The portal** (15 November 2026; its words since 22 November one box,
  [The way in](#the-way-in-22-november-2026); `src/online/online_square.c`'s
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
  guardian is scaled for it (his HP times the party, at most 9999, at the
  version a diver alone meets since 22 November 2026: A party's guardian,
  below) and each fights him in
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

  **A full door, and the party by its names** (22 November 2026, issue
  #753, from the friction hunt's play with five games): a fifth at a door
  whose queue holds its four is refused by the server (NOTICE
  `notice_queue`, before any QUEUE_STATE), and the game now hears it: the
  band says "The party's full!  A:Go in alone", MegaMan stays held at the
  door, A takes him in alone, his other keys are his own (START opens
  the PET, walking off leaves the door), and the game asks the queue
  again every three seconds, so a place that opens (one of the four
  going alone or walking off) takes him in. While a party forms, the
  plates under BN6's HP box list it instead of the crew, its leader
  first: a crewmate with his HP as ever, another diver his name alone in
  white; the band keeps the count and the leader's name. Calls taken
  under the owner's direction of 21 November, his to turn over: the
  refusal read from the NOTICE already sent, rather than a new message
  on the wire (a join's only other refusal, "Not at that door", a game
  that sends its own room never meets); held at the door rather than let
  go (he came to fight it: waiting for a place, or A, are both a press
  away); asked again on a clock rather than told by the server when a
  place opens (no change to the wire); the names on the plates rather
  than in the band (four names do not fit its 240 pixels). Seen headless:
  the scenarios `online-week/door-full` (five games at layer 3's door on
  a local server: the four's plates naming the others, the leader first;
  the fifth told, START opening his PET, and in the queue once Delta
  went alone) and `online-week/door-full-alone` (the fifth's A: in alone,
  the arena closing round him, the four's party as it was).

  **The door waits for the crew** (22 November 2026, issue #752, from the
  same hunt: two crewmates each fought the guardian alone at one door,
  the first having paged MegaMan's words with A, which was "Go in" in a
  queue of one). The owner's call: the door waits up to about ten
  seconds for crewmates in reach and tells them, and the leader can still
  go in early by holding A, never by a single press (chosen over a second
  press with no wait, and over one press with the crew only told). As
  built: while a crewmate of the leader's stands on the door's layer
  (the crew's word of where he is) and is not yet in the queue, for ten
  seconds from the leader's arrival, the band counts the wait down
  ("Waiting for crew 9  Hold A:Go in"), a press of A does nothing and B
  does not take the leader in alone; A held three quarters of a second
  takes the party in. Every diver on the door's layer, and behind it in
  the act as before, is told by the flare ("Alpha waits at the door" on
  that layer, "... at layer 3's door" behind it); those already in the
  queue are not. The wait over (all of them in, or its ten seconds
  past), the band and A are as they were. The door's A is its own: a
  press counts only once no chat box or talk has been open for a third
  of a second, never the press that paged MegaMan's words on the way
  in. The band's heavy letters drew G with its bar shut against the
  stem, read as 6 ("A:60 IN"); the heavy G keeps that column open. Calls
  taken under the owner's direction, his to turn over: ten seconds and
  three quarters of a second held; the wait shown as a count in the band
  (a number about time, not the link), rather than a bar of BN6's own;
  the wait ending in the leader's A, rather than the party going in by
  itself once the crew is in (he keeps his say over a stranger on the
  way); crewmates in reach meaning on the door's layer, rather than
  within some panels of it. Seen headless: the scenario
  `online-week/door-waits-crew` (two crewed games: Alpha at the door,
  mashing A, still in the queue with the wait counting; Beta's band
  telling him; Beta in, mashing A; Alpha's A, and both on one field).

  **Whom the door waits for** (22 November 2026, issue #778, from the
  second hunt): since a crew lives only on screen (#770, the owner's call
  on #750), it splits two seconds after a crewmate leaves the picture, so
  whoever walked ahead to the door had no crew there and the wait almost
  never applied. The call joins the owner's two (the door waits for the
  crew, #752; a crew is who you can see, #750): the door waits for the
  crew as it was on the layer shortly before, not as it stands at the
  door: whoever was in MegaMan's crew within the last thirty seconds as
  he comes to the door, and is still on the layer (the room's divers) and
  free (`dive_crew_recent.c`, the game's own memory of the server's
  word; no change to the wire). Whom it waits for is fixed as he comes;
  the wait's ten seconds and the held A stay. Thirty seconds because a
  layer's whole walk, from the arrival to the guardian's door, took 16 to
  25 seconds in the autopilot's runs of layer 3 on six seeds (the clock
  read every three seconds), so a crew that stood together anywhere on
  the way is in it, and one met on an earlier layer is not. The flare
  goes once a door to each diver, from the first who joins its queue,
  never to one in it (`party.py`'s queue keeps whom it told); and at the
  door the queue's own line comes before the feed's, so a queued diver
  reads "Alpha leads 3  B:Go alone", never a flare. Taken under the
  owner's direction of 21 and 22 November, his to turn over: thirty
  seconds; the memory in the leader's game rather than the server (it
  needs no new message, and only the leader's door asks); the wait fixed
  at the leader's arrival. Seen headless: the scenario
  `online-week/door-waits-who` (three crewed games: Alpha ahead, his crew
  split before the door, "Waiting for crew 2" there; Beta and Gamma one
  after the other; Gamma's band his own line); the server's
  `test_one_flare_a_door_each`.

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
  it by its index on the layer (until 21 November 2026 the band said
  "Battle! Calling the crew..."). A crewmate whose game gets the offer,
  MegaMan free on the map (no chat box, no door, no party's fight), sends
  CREW join at once, his band saying "Jumping in!" (since 22 November
  2026 whose battle, "Alpha's battle!", [On screen only](#on-screen-only-22-november-2026)): no question since 21
  November 2026 (issue #715, the owner's call after the team's test;
  until then the band offered it, "A:Jump in", for the window); a game
  whose MegaMan is not free, or holds his own battle, sends CREW busy
  (protocol v23) as the offer comes. The server starts the battle as the
  last one offered has answered, so the owner's hold is as long as that
  takes (the log says it: "MegaMan held 65 ms"); the window
  (`ONLINE_CREW_JOIN_MS`) is left for a game that never answers. A
  newer offer replaces an older. Its background is chosen with it from
  the layer's seed and the index (`dive_battle`): it was rolled from each
  run's own RNG as the record was set, so in an area with two backgrounds
  two games drew two about half the time and built their fields apart
  ("the fields went apart before the first frame", two runs in twenty of
  `online-week/crew-battle-unasked` before, the battle then fought alone);
  the log names each of a field's inputs by its hash ("the field from rom
  ... record ... seat0 ..."), so the next such split names its part. The server's START takes every game in to
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
  `arena_link_net.c`), one a frame, never guessed again, and never lost:
  my keys go at the clock's pace, one a frame at most, so a game a moment
  ahead of the clock (a device catching up a frame, a headless game) has
  frames on which none of its words goes, and a message made on one of
  them waits for the next (`party_play.c`'s owed word). Until 22 November
  2026 it was dropped there, and where it was his last pick the field held
  its Custom screen to the turn's limit and gave him none
  (`online-week/crew-picks-through-an-early-frame`, `--dev partyearly=1`
  putting each message first on such a frame). Every game takes
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

**A crew in one column** (24 November 2026,
[issue #886](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/886),
seen in 0.24.0's pictures): the crew stands a row each in the lead's
column ([Viruses' aim](#viruses-aim-in-a-crews-battle-22-november-2026)), and a plate
under one MegaMan's feet lay over the MegaMan in the row below, so that
with three the lower two were mostly hidden and "Beta 100" read as
Gamma's. A MegaMan stands 47 pixels tall on rows 24 apart: no spot under
or over one is clear of the next in his column. **The call**, through the
game-design skill's interface layer (a spatial label on the actor, read
mid-fight while the number changes; no plate over an actor it does not
name): each plate, a partner's name over his HP or the player's own name
behind the PET's arrow, takes the first of four spots that covers no
MegaMan's body, no plate placed before it and none of BN6's own pieces:
under his feet, where BN6 writes an enemy's HP; in front of him, its foot
on his feet' line in the next column; behind him; over his head. A plate
keeps to the player's half of the picture (never over an enemy), under
BN6's gauge, beside its HP box and emotion window, over its chip name at
the foot. Where no spot is clear, the one that covers least. The player's
own is placed first, then the partners by seat. In the lead's column of
three the upper two partners' plates stand in front of them, the bottom
row's own name under its feet; with four, the one behind the lead keeps
his under his feet. A plate moves only when someone steps to another
panel. Chosen over the name over the head with the HP beside the feet
(the name lay on the MegaMan a row up and on BN6's chip icons over each
head), and over the names and HP in a strip under BN6's emotion window
with a letter by each MegaMan (no strip of three lines in BN6's digits
fits between the emotion window, the chip icons over a top-row MegaMan
and the field; and a letter to look up is a step more than a name beside
him). The Arena's two a side keeps its own marks (`arena_team_view.c`),
unchanged. The state says each plate's box and its MegaMan's
(`plateK X Y W H seat X Y W H`) and whether any plate meets a MegaMan or
another plate (`plates N clear|COVER`).

**His own Custom screen and his results** (22 November 2026,
[issue #757](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/757),
found by the friction hunt's play of the whole dive): BN6's own screens
there showed the lead's HP. BN6's HP box reads side 0's first actor's
battle object (its HP and max, `BN6_T1_HP`), the lead's on the field.
Now each is driven, not redrawn:

- **His Custom screen** runs on a core of his game alone
  (`party_turn.c`), never part of the field or its hash: as it opens, his
  HP and max go into side 0's first actor there (`team_screen_open_one`),
  the HP the field means him to have (`party_core_seat_hp`: before the
  battle's first iteration his going in, since his object holds his
  lead's till the field writes his then, `party_field.h`).
- **His results** run on each game's own pad once the battle is over and
  the hashes have stopped: as they begin, a partner's game writes his HP
  and max into side 0's first actor and the Navi's HP of its field's core
  (`party_core_results_as`), so BN6's results show his.
- **The fallen sees no results of BN6's**: a MegaMan deleted on the field
  gets no reward for that battle ([A deletion in a crew](#a-deletion-in-a-crew-21-november-2026)),
  so his game goes on to the run's own end of it at once, the results
  whose GET DATA is not his never shown.

Seen headless in `tests/scenarios/online_party_partner_hp.py` (`--dev
partycore=3,partybot=1,partyview=1,partyhp=60`, the stood-in partners
going in with 60 HP to the lead's 100): his own Custom screen's box 60
(`partyturn hp 60 own 60 agree`), his results 60 (`partyresults ... agree`,
`partyover shown hp 60`), and with `partyend=200,partydel=2` his deletion
on the field ending with no results (`partyover skipped hp 0`). Left open
on the issue: a dive's end screen of its own, and an HP reward that came
into the run twice.

**The screen that takes his keys, in the state** (22 November 2026): the
field's first Custom screen opens at the fight's frame 0 and takes keys
from frame 11 (BN6's `BN6_CUSTOM_MODE` 0x00, then `BN6_MODE_CHIPS`), at
the clock's pace. `online-week/crew-battle-named` pressed the lead's
START on a count of frames, now and then before frame 11: START was lost,
his A took a chip, and his OK, which no limit stands in for, never came.
The state now says the Custom screen this game's pad drives, the field's
for the lead and his own for a partner (`partycustom chips` where it
takes keys, else BN6's mode in hex: `00` opening, `08` OK pressed, `14`
waiting on the others; no line while there is none), and a course
presses once it says `chips`.

**Keys held as the field hands the game back** (22 November 2026): the
run's own core stands still under a crew's battle, its last keys those
from before it, so a button still held on the field's last frame came to
BN6 as a new press on the layer. A fallen MegaMan's game goes back at
once (above), and the A that a player, or `crew-down-told`, kept pressing
there talked to the Mr.Prog before him; in the chat his game answered the
crewmate's next battle busy, and he was never in it. Every button held as
a held battle hands the game back (a crew's, the Square's NetBattle) is
now kept from it until let go (`online_visit_keys`, as the Battle Lab's
screen does), the d-pad walking on.

## Comings and goings (22 November 2026)

What the friction hunt of 21 November found silent ([issue
#758](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/758)):
a crewmate who quit or dropped vanished in one frame, his return with no
word, a busy crewmate unseen, a fifth beside a full crew drawn the "about
to be" ring for good, and every diver arriving on one spot. The owner's
call of 22 November 2026: band lines and plate marks. A short line in the
dive's band for a crewmate who left, came back, or a crew that could not
take MegaMan; a mark on a plate for busy, fighting and away; strangers
passing say nothing. Chosen over plate marks alone, and over lines for
every diver on the layer.

As built (`dive_comings.c`, its lines in `dive_words.c`; protocol v29):

- **Gone and back.** A crewmate who leaves the crew and is gone from the
  layer's SNAPSHOT within 0.6 s (he quit, dropped, left the dive) is "Beta's
  signal is gone..."; the same name on the layer again is "Beta's signal is
  back!". MegaMan senses a signal; he never says why it went. One who walks
  off is still on the layer and says nothing (the plates and the ring show
  it); one who went on to another layer, or home, says nothing either.
- **A full crew.** The server tells a diver in no crew who stands near a
  crew of four that it is full (CREW_STATE full), once as he comes near
  it: his game draws no dotted ring for that crew and says "Alpha's crew is
  full", told again only after he has stood out of its reach.
- **Marks.** A plate's where is a tag as the PET's information box tags a
  key item (`look_tag`): BUSY (on the layer, not free: a chat, the PET, a
  shop; `where_busy`), BATTLE, HOME; another layer stays "L3".
- **Arrivals.** For two seconds after MegaMan arrives on a layer, while
  another diver stands on his spot, the PET's orange cursor points at his
  head.
- **The band never floods.** A crewmate's "gone" at most once every 20
  seconds and his "back" once for each, so a crew of four makes 18 lines a
  minute at most (three links flapping at once), the band's queue
  dropping its oldest past its few; a full crew once an approach.

Seen headless: `online-week/comings-and-goings` (three games: Beta's PET
open, his plate BUSY on Alpha's and Gamma's; Gamma's game quit, "Gamma's
signal is gone..." on Alpha's and Beta's band; Gamma's game started again and
his dive kept on, "Gamma's signal is back!" and the crew of
three again) and
`online-week/crew-of-four-and-a-fifth` (five games: the fifth's rings none,
"Alpha's crew is full" on his band; a crewmate walking off says nothing).

## The way in (22 November 2026)

The second friction hunt counted about fifteen presses of A from the
portal to walking on layer 1, the same on every dive of the week and after
a deletion, and the one fact a diver wants there, how many dive now, not
said ([issue
#779](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/779)).
Under the owner's direction of 21 November (no prompts, no waiting; less
is more), and his to turn over:

- **One box, then go.** A new dive's portal is one BN6 Yes/No box: "3 NAVIS
  DIVING! / DIVE IN?", the count the server's DIVERS (the Board's "Diving
  now"), "DIVE INTO THIS / WEEK'S DEEP NET?" before the server has told it.
  The week's first dive alone has one page before it, the week's dates and
  "EVERY NAVI DIVES THE SAME NET!", and its cursor starts on No; once
  MegaMan has dived this week it starts on Yes. Yes goes at once: "THE
  PORTAL OPENS INTO THIS WEEK'S DEEP NET!" is gone, and so are "THE NET
  SHIFTED!" and "DIVE IN NOW?". An unfinished dive's box is its options
  alone (Keep diving, Start over, Not now), "YOUR DIVE INTO THE DEEP NET
  ISN'T OVER!" gone. The Board's word of a dive it just took stays, a page
  first.
- **Once a week.** The first layer's words ("Whoa... It looks just like
  ...!"), the older net's news ("Wait... This Net's a copy of an older
  one!", which a dive's fresh profile heard every dive) and the act's title
  card play on the week's first dive and not again that week (`weekly_intro_shown`, the week's record's `intro_week`
  in `weekly.sav`, not `Run`).

- **The vortex's flat frame is its own.** Sampled every three frames
  with no box open, BN5's dark hole (list 7 `0x87`, moved plain) shows as a
  flat purple shape once in each 30-frame loop of its five frames: it is
  the moved sprite's, not the box's. Open: why that frame draws flat in
  BN6's engine (its frame's tiles or OAM as `xnavi_sprite_len` copies
  them, against BN5's own), a reverse-engineering task of its own.

A second dive of the week is now three presses from A on the portal to
walking on layer 1: the portal (A), Start over (DOWN, A), or a new dive's
Yes (A, A). Seen headless: `online-week/second-dive-of-the-week`.

## A diver gone home (22 November 2026)

A dive's games found by the friction hunt of 21 November ([issue
#756](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/756)):
a diver at home between acts sent no place, and his last stood, so the
others saw him standing on the layer, named and ringed, and the plates
could not say where he was. Since protocol v26 each game says where its
MegaMan is as it changes (DIVE where: on the layer, home, a battle;
`dive_crew_view.c`, a quarter of a second apart at most). The server
leaves a diver at home out of every SNAPSHOT, so his puppet, name, ring
and dot go within a tick of his word; he forms no crew from the place he
left and stays in the crew he had (crew_away_ms does not run for him),
as the crew's call at home was ([Crews by
nearness](#crews-by-nearness-21-november-2026), At home); entering the
next layer puts him on a layer again. The crew is told where each member
is (CREW_STATE's member grows a byte), and the plates say "Home" or
"Battle" before a layer. On a party's field the plate's HP is the
field's (`party_play_hp`), where the run's own core stands still under
it and its HP lagged. Seen headless: `online-crew-view/gone-home`, three
divers crewed, Beta sent home (the dev step `home`, an act's last exit
as a test's step): Alpha's and Gamma's layers without him within a
second, their plates "Home", the crew of three kept past crew_away_ms.

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
  quarter down; and where he is: his layer ("L3") where it is not
  MegaMan's, "Home" between acts, "Battle" in one (since issue #756).
  **"Level"
  is max HP**: a dive starts from a standard profile, so the game has no
  level, and what grows with a dive is max HP (HPMemory, the guardians'
  data); the act and the layer say how deep, not how strong. Chosen over
  the act and over Busting Levels (a battle's, not a diver's).
- **In battle**: a partner's HP under his name on a party's field
  (`party_marks.c`), as BN6 writes an enemy's under him and in BN6's own
  battle digits, read from the player's ROM and counted down in orange
  and up in green as an enemy's (`hp_plate.c`, since 23 November 2026:
  the engine's pixel font had made the two numbers of one frame look
  different; [Sprites, fonts and pictures](../rom/graphics.md), The HP
  under an enemy); MegaMan's own is the HP box. The name stays in the
  pixel font: BN6 has no letters of that size for it. Where under his feet would cover
  the MegaMan below him, the name and HP stand in front of him or
  elsewhere beside him (Who is who, A crew in one column). A crewmate fighting his own battle: his plate's HP
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

## The crew on the second screen (21 November 2026)

The second screen's half of the crew's display
([issue #729](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/729)).
**The calls** (taken under the owner's direction of 21 November 2026, a
party frame as an MMO has it, more where there is room; his to turn
over):

- **What the page adds**: on a layer the second screen shows the layer's
  map, so the crew goes at the top of that panel, a row a crewmate,
  rather than a page of its own the map would have to give way to. A row
  has his full name (the top screen's plate cuts it), his HP over max HP
  and a bar of it, his layer where it is not MegaMan's, and his link's
  bars (the Square's badge, `arena_view_bars`) where the server lists
  him on MegaMan's layer. His place on the map is the divers' dots'
  ([issue #728](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/728)),
  left to them.
- **The plates stay** on the top screen where a second screen shows the
  rows: the eye is on the top screen in play, the rows are a glance away
  and say more (the whole name, the link), so they are no repeat to drop.
  Chosen over hiding the plates where a second screen is shown.

As built (`src/second/second_crew.c`): `second_update` copies the rows
(`dive_crew_view_rows`, the plates' own words) and each one's link from
the server's snapshot into the second screen's state while its map panel
is up, and the map's draw gives the rows the panel's top. Seen headless
(21 November 2026) in `tests/scenarios/online_crew_view.py`'s course,
captured on both screens: Alpha's at an Android display's 640 x 480,
"Beta 90/100" and three green bars, then "Beta L2 90/100" once Beta had
gone on; Beta's at the 3DS's 320 x 240, "Alpha L1 100/100".

## A party's guardian (22 November 2026)

The friction hunt of 21 November found a party's door harder per head
than going alone
([issue #754](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/754)).
Measured with `tools/party_guardian.py` ([A party's guardian against
going alone](../devtools/party-guardian.md)): bot fighters on the party's
field, the guardian's HP and his damage a second on the party measured, a
fighter's time to the win and the HP he loses worked out against the
solo's (1.0), each fighter taken to deal the same damage a second. As
built till then (two met his next version, three or four the one after,
his HP times the party), four seeds:

| Act | Fighters | Guardian | Time to the win | HP a fighter loses |
| --- | --- | --- | --- | --- |
| 1 | 2 | v1 1800 | 2.0 | 3.3 |
| 1 | 4 | v2 6000 | 3.3 | 9.4 |
| 2 | 2 | v1/2 2500 | 1.8 | 2.0 |
| 2 | 4 | v2 6400 | 2.3 | 5.5 |
| 3 | 2 | v1 2600 | 1.4 | 1.05 |
| 3 | 4 | v2 7200 | 2.0 | 1.5 |

The version raised was what cost: his damage a second rose two to four
times with it, while the HP he was fought at grew with the party anyway.

**The owner's call** (22 November 2026, on the proposal on the issue,
through the game-design skill's dialectic, together against ahead: a
party is to be the better fight to be in, not the cheaper win): **the
guardian's version a diver alone meets, his HP times the party.** Per head
it is the fight a diver alone has; what a party has more of is the aim
passed round it, the pull-back, a bar that falls under four. Chosen over
the ladder as built; one version up only from act 3 on; and one version up
with his HP 1.5 times a fighter (measured: still 1.5 to 4.9 times the HP a
fighter loses). As built (`boss.c`, `party_version`: no step but the dev's
measure, `partysteps`), four seeds:

| Act | Fighters | Guardian | Time to the win | HP a fighter loses |
| --- | --- | --- | --- | --- |
| 1 | 2 | v0 900 | 1.0 | 0.55 |
| 1 | 3 | v0 1350 | 1.0 | 0.43 |
| 2 | 2 | v0/1 1400 | 1.0 | 0.81 |
| 2 | 3 | v0/1 2100 | 1.0 | 0.77 |
| 3 | 2 | v0 1800 | 1.0 | 0.70 |
| 3 | 3 | v0 2700 | 1.0 | 0.53 |

Four were not measured this way: four bot MegaMen on one game's field stand
still after its first iteration (`--dev partycore=4,partybot=1`, an older
fault of the dev's stood-in party, listed missing in the 0.22.0 trailer's
cut), so the rule's per head for four rests on it being the same rule.
The scenario `tests/scenarios/battle_guardian_party.py` holds it: at
layer 3's guardian of `--seed 3` a party of two and of four meet DiveMan's
V1, at 800 and 1600 HP to the solo's 400 (`guardianfought version 0 party
N`, `guardianhp`).

## Viruses' aim in a crew's battle (22 November 2026)

The second friction hunt found that in a crew's random battle the
viruses struck the battle's owner and nobody else, three battles of three
([issue #777](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/777)),
though `party_field_aim` passes a guardian's aim round the party. Found
(the ledger's `2026-11-22-viruses-aim-by-position`, `docs/rom/online.md`'s
row): **a virus takes no player from side 0's first slot; it finds its
target by position**, the MegaMan in its line. A Gunner's attack step asks
for the first panel in its front direction holding `BN6_PANEL_SIDE_0`; a
Mettaur stays in a row with a MegaMan in it and moves only once none
stands there. On a crew's field the owner stands in front (column 2 row
2) and the partners spawn behind him (column 1, rows 1 and 3), so a virus
in his line strikes him until he falls. Trading the slot during the
virus's routine, trading it for good, clearing the other MegaMen's panel
bits or moving their objects off the field during its routine did not
move a Mettaur's first target: what its row choice reads is still to
find (perhaps the collision panels). The viruses' bench to measure it:
`--dev party=3,partyaim=3` (the aim passed round on the spike's battles,
the partners standing still; `partyaim=2` the partners still alone,
`partylead=N` the lead's slot and partner 1's traded for good at
iteration N), with `foes 01@52` or `53@52` on a layer and the log's
`party: iteration ... HP` lines.

Measured on the bench, three MegaMen standing still, a virus in one row:
with the partners behind the lead, a Mettaur in row 1 or row 3 walked to
the lead's row and struck him, ten times running, and a Gunner in any row
struck the lead first. A virus goes for the front column, where the lead
alone stood. So nothing was wrong with the aim, and positions deciding who
is hit is BN6's own rule and stays; what was unfair is where the crew
stood.

**The crew side by side** (22 November 2026, taken under the owner's
direction of 21 November and on his answer on the issue; his to turn
over): on a party's field, a guardian's and a crew's alike, the partners
stand beside the lead in his column, rows 1 and 3, a fourth behind him
(column 1, row 2), then the rest (`party_field.c`'s `panels`), a hole of
the battle's field passed over. **The lead keeps the middle row**, BN6's
own place for him (column 2, row 2): chosen over the first partner in the
middle (the order of seats on screen top to bottom) and over the lead on
row 1. Every game of the field makes the same list, so the seats' hashes
agree as before; a linked fighter fights his own battle, untouched. On the
bench after it: a Mettaur in row 1 strikes the partner in row 1 and the
lead in row 2 is untouched, in row 3 the partner in row 3; a Gunner in
row 1 or 3 strikes the partner in its row first, then the others. The
scenario `tests/scenarios/battle_crew_rows.py` holds it (`spike N hit H
lead whole|hit|down`, the spike's battle in the state's words): seen to
fail with the old layout, the lead struck first.

## The spike: four on one field (15 November 2026)

`--dev party=N` (`src/online/party_field.c`) grows one of the run's own
battles to N MegaMen on side 0 on one core, the way `--dev arena2v2`
grows a NetBattle: the battle's list rewritten at its spawn
(`BN6_SPAWN_ENTITIES`, the partners on side 0's free panels, beside the
lead in his column since issue #777, then behind him) and each partner's keys written into his AI data at the
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

## A dive's end, and a reward taken once (22 November 2026)

Issue #769, left over from #757:

- **A dive's end screen of its own.** A dive that ends, deleted for
  good on a crew's field or in any other way, opens the run's RESULT
  as a dive's ([The engine's screens](../engine/ui.md)): DEEP NET on
  its band, "The dive is over", the layer reached, the crew's battles
  fought and won (counted in the dive's record as each ends,
  `weekly_crew_battle`, `weekly.sav`'s last two bytes), and the dive's
  place on the week's Board: the server posts the end (Back to the
  dive, below) and answers with its Board, so the dive's link is kept
  up to six seconds after the end for it (`dive_mail_told`, pumped by
  the screen through `weekly_end_pump`), the row "Asking..." meanwhile,
  then "3rd", or "No link" where the link was down at the end or the
  Board never came. MegaMan says "That's our dive,Lan..." and "The
  portal's open. Let's dive again!", never the normal run's "Dad's
  backup worked!".
- **An HP reward taken once.** BN6's results add an HP recovery to the
  field's own MegaMan as they show (bn6f's sub_802C170, phase `0x10`),
  and the battle's sub-mode runs a few frames on in phase `0x14` as
  they close, then `0x00` and `0x04` ([BN6's battles](../rom/battle.md)).
  The field read each MegaMan's HP in every phase but `0x10`, so those
  frames gave the battle's owner the recovery and `director_crew_over`
  gave it again. The field now reads HP only in the fight's own phases,
  the Custom screen's and the fight's (`0x08`, `0x0C`):
  every seat's HP as its fight ended, the reward given once in each
  run (`online-week/crew-reward-once`: the owner on 20 HP of 100 ends on 70 with an HP+50 reward, 100 before).

## Back to the dive (22 November 2026)

Issue #759, found playing the shared dive with three to five games: a
diver who quit mid-dive came back to the normal run's CONTINUE and a
long way to the portal, his crew told nothing; and a dive's end reached
the Board only at the diver's next visit to the Square, so a diver who
did not go back was never on it. The owner's calls:

- **CONTINUE goes straight back in.** The title's CONTINUE on an
  unfinished dive (its label "Dive L3") takes it up at its checkpoint
  (`weekly_continue`), and the dive's link calls the Square as the gate
  does (`director_gate_address`, `dive_link.c`'s `call_again`), the link
  on for the dive's own week and down until the Square answers: then
  MegaMan's room is entered again, the others on the layer seen, the
  crew forming again by nearness. Not answered, it is a dropped link
  (issue #751, [A dropped link on a dive layer](arena/dropped-link.md)):
  the band's "Reconnecting...", the dive alone meanwhile and never lost,
  the Square called again for as long as the dive lasts, "Back online!"
  once it answers; a refusal, the dive alone for good, said. The Square
  two weeks on or more (it keeps this week's net and the last's), the
  dive goes on alone and the band says "A new week's net! We'll dive
  alone."; its end is posted at a later visit while the Board still
  takes it. A server restarted meanwhile is called afresh like any
  other. Nothing
  of `Run` changed: what a quit kept (the checkpoint, the record's
  pull-back) is what CONTINUE finds. Once the dive ends, CONTINUE is the
  normal run's again.
- **The Board without a visit.** DIVE end carries what the Board ranks
  (v27: the dive's minutes, its flags, `week_pulled`, and the acts whose
  guardian was fought in a party), and the server posts it at once
  (`week.py`'s `from_dive`), its layer the room's own, checked as a
  WEEK_POST is. The game's own WEEK_POST at its next visit stays, the
  same entry, which changes nothing; it still posts a dive whose end
  never reached the server. One entry per diver and week, a worse end
  never over a better one. A dive the server had ended while its game
  was off (the end sent, the game closed before its record was written)
  is on the Board already; taken up again, its next end is held to it
  like any other.
- **Patterns** (recalled from playing them, not checked in
  [Sources](../research/sources.md)): Diablo's and Path of Exile's
  return to the party's game, an MMO's log-in where one logged out.
- **Proven** with games on a local server (`tests/scenarios/online_week.py`):
  `dive-kept-apart` (closed on the dive, CONTINUE back on its layer with
  its link, deleted there, CONTINUE the normal run's again and its run
  as it was), `dive-continue-crew` (Beta's game closed on the layer,
  CONTINUE, Alpha sees him again and the crew forms), `crew-pull-back-kept`
  (a pull-back spent stays spent through it) and `dive-end-on-the-board`
  (Alpha deleted, his game never back in the Square, Beta reads him on
  the Board).

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
  shared our HP. They're back!" on a giver's. While he is down and the
  field fights on, his band says what a win does for him (issue #780):
  "Down! The crew can pull us back", or, his pull-back spent, "Down...
  One more fall ends the dive"; only while the fight itself runs, never
  over a Custom screen or the results, the field otherwise BN6's
  (`party_play.c`'s `down_now`, `party_marks_down`; the scenario
  `online-week/crew-down-told`). A partner's HP box is his own then too,
  at 0: it had shown BN6's box, the lead's HP, once his MegaMan left the
  field (`party_marks.c`). `--dev partydel=MASK` with
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
   unasked (issue #715), and since the same day with no window: a busy
   crewmate says so at once ([A crew's battles at once](#a-crews-battles-at-once-21-november-2026)).
   Closed.
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
