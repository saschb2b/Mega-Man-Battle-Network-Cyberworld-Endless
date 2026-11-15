---
type: Feature
title: "The way-on arrow"
description: "The arrow over MegaMan that shows the way on along the floor (to the exit, the guardian, at home the port), green and to the Recovery Mr.Prog first while he is hurt until he turns away from it, the map's way drawn in the same colour as its key's Heal (`goal_way`, `heal_first`, `heal_turn_watch`, `map_way`, `hp_way`)."
tags: [world, level-design, guidance, interface]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T12:00:00Z }
sources:
  - id: code
    resource: "src/director/director_way.c (goal_way, heal_first, heal_turn_watch, arrow_update, arrow_quiet), src/director/director_map.c (map_way), src/director/director_town.c (hp_way), src/director/cinema.c (draw_arrow, arrow_settle, megaman_feet)"
    title: "The arrow and the map's way as built"
  - id: s76
    resource: "Playtest session 76 (kai5, kai8), replayed on their pinned build e2005dac6"
    title: "Two players' arrow at Lan's HP's wall and the green arrow"
  - id: s77
    resource: "Playtest session 77 (kai9, frames 22577-23265), replayed on kai7's layer 2 (seed 9230599) on dev"
    title: "A player heading for the exit led back to the heal"
  - id: s77-kai5
    resource: "Playtest session 77 (kai5), replayed on its pinned build 0f2643619; the cases staged on the fix's build"
    title: "The arrow over MegaMan, a chat box and a guardian's log-in"
---

# The way-on arrow

BN6 has no arrow: its maps are few and learned. A run's layers are new each
time, so the engine draws a cyan arrow (two heads, `>>`) a little off the
picture's middle, where the camera keeps MegaMan, pointing one of the d-pad's
eight ways. It shows while L's words last and ten seconds after, while
MegaMan walks (up to half a minute), after the map closes, and after a while
pushing where the pad goes nowhere (`arrow_update`, `push_arrow`).

## What it points at

Along the floor, not as the crow flies (`net_route.c`): the exit pad, or the
guardian while he stands; at home the town's walk to the port, and in
Lan's HP the pink pad by its corridor (below). It turns as MegaMan walks,
only when a new way comes twice running and lies past the edge of the one
shown, so it does not wobble at a walkway's mouth.

**The heal first.** While MegaMan is below three quarters of his HP and the
layer's one patch is unspent (`heal_first`: not given yet, no vow that
forbids it), the arrow leads to the layer's Recovery Mr.Prog, as L's words
do, and is **green**. That holds beside the exit pad too. The pad on the
screen shows itself, and the arrow shows what the screen does not: a heal
behind him that leaving the layer gives up. Stepping on the pad still
warps.

**The colour says which.** The green is the map key's Heal, and while the
arrow leads there the map (SELECT) draws its way in that green to the Heal's
mark, where it drew the exit's yellow way before. Two playtesters in
session 76 followed a green arrow away from an exit pad on the screen
without knowing what the colour meant (one at 80 of 140 HP with the heal 4
panels back). The map's own key now names it, with no words added. The game-design
call: colour alone carries nothing, so the colour is paired with the map's line
and its labelled square.

**Turning away from it.** The heal is a recommendation, and the player
may pass it up. Once MegaMan's walk to the heal has grown three panels
past the shortest it has been since L last named it (`heal_turn_watch`,
a look every ten frames), the way on is the exit's again: the arrow
cyan, the map's way yellow, for the rest of the layer, until L is
pressed again, which names the heal again and leads there afresh. A
walk followed toward the heal only shortens, so it never trips; a short
detour (ProtoMan two panels further on) does not either. In session 77
a player heading for the exit past the heal L had named followed the
green arrow back toward it and lost ten moves at a junction (kai9). The
game-design call, from three candidates: once the heal is used kept
leading a player who had chosen the exit back to it, and at the next L
asked for a press nobody knew to make; the walk itself says what the
player chose, the guidance-versus-discovery axis's opt-in help: L is the
ask, the walk the answer.

## Where it stands, and when it waits

The arrow rings MegaMan where BN6's camera draws him (his feet projected
from the camera's place, `megaman_feet`), its heads 32 pixels across and 30
down from a point 21 above his feet: the picture's (120, 66) where the
camera keeps him in the middle, and with him where it does not. Pointing
down it stands farther out, clear of his sprite (it had stood on his legs).

It waits unseen, its time running on, while MegaMan cannot walk its way
(`arrow_settle`): through a chat that is not its own words (a Mystery
Data's, a Prog's, a dealer's, a navi's), a cutscene of BN6's, and a
guardian's staging from his log-in to the battle. L's words, and R's at
home, are the arrow's own, and it shows through them as before, but where
it would cross the open chat box (pointing down) it waits for the box to
close. Kai5 saw it over MegaMan's legs and the box's top edge as he took a
Mystery Data, and over SpoutMan as he logged in (session 77). Off the map
and under BN6's fade it hides at once, as the cards and notes do (the
fade's byte, [game state](../../rom/game-state.md)).

Two of kai5's arrows that pointed against the map's line followed their
walks: in a band two panels wide the fewest-turns walk sets off one panel
across onto the band's other row (one step up-right, then on up-left), and
at a walkway's crossing the arrow turns as MegaMan reaches it.

## Lan's HP's corridor

The way from the blue pad to the pink pad runs up a corridor whose mouth at
x 119 is open from y -24 to -1 ([Lan's HP](../home/lans-hp.md); walked
again in session 76 from x 40 and 90: -25 and 0 stop at the mouth's side).
Within that band the arrow points at the pad, which a walk up its way
reaches. Off it, before the mouth, the arrow points at the mouth's middle
(119, -12): from far off that is up-right, from near it the diagonal into
the band, and from the wall beside the mouth across to it.

It had wanted a narrower band (-22 to -6), and from x 55 pointed straight
across to its line. From y -5 and -1, where a walk goes in, it sent MegaMan
upper-left; held 30 frames he overshot and it turned lower-right (kai5,
frames 5033-5163). Followed from nine starts round the room now, the
arrow takes him into the corridor each time.
