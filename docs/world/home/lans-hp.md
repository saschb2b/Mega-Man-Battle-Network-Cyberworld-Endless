---
type: Feature
title: "Lan's HP after an act, the warp zone"
description: "Lan's HP as the run's warp zone (`lanhp.c`): an act's exit pad leads there, BN6's link squares and pink pad become this run's portals as guardians fall, the courier, ways on and back."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:18:26Z }
---

# Lan's HP after an act, the warp zone

## 1. After an act: Lan's HP

The exit pad of an act's last layer, the guardian deleted, takes MegaMan
to Lan's HP. The act's AREA CLEAR card shows there, the next act's portals
light up, MegaMan says a word about the act, and the run is saved ("From
Lan's HP" on a CONTINUE). R there asks BN6's own "MegaMan, jack out?":
yes takes Lan to his room, where the town and its errands are; his PC
jacks in to Lan's HP again. BN6 makes MegaMan's HP full at a jack-out and
a jack-in; the guardian has already healed him, so the trip changes no
balance. The short net's Nest leads on as before (its win, and on threat
10 the way down to its second guardian); the endless net's Nest lights the
next Net's first portal.

Built first as home after every act (issue #85): the exit's warp turns to
the town by BN6's own transition, internet to real world, which sets Lan
down where he last jacked in ([Running the game's own logic](../../engine/emulation/overview.md)); the AREA CLEAR card
over the town while Lan is held, MegaMan and Lan's word by how far the
run has come (`home_words.c`), the run saved ("From home"). That trip
stays, as the jack-out from Lan's HP, and its words move there.

## 2. Lan's HP, the warp zone

BN6's own Lan's HP (`0x88:00`), taken over as the town is: its arrival
pad where every jack-in lands, and a portal for each way the run has
opened, BN6's warp pads standing in rooms of its own look. It starts
bare: one portal, the first act's. Each guardian deleted adds the next
act's (two, or three with the dark way: the act's own area, the other
way, the Undernet once the Secret Area is cleared, as `run_ways` gives
them today; or a special route's third, found on a dark layer, at the
link on the right, and at the Nest a second way to it at the link up
top: [Special routes](../../game/liberation/special-routes.md)), each Nest the next Net's first, and each lights up as
MegaMan arrives; the HP grows a room as its portals outgrow it, so the
run's progress is a place. Beside a portal, MegaMan says what he reads
through the link, never where it leads: he has not been there (the
owner, after the first build: "megaman does not know the next layer
yet"). He reads its data ("Whoa,salty data! Like the sea!", "The data
over there is old!" for the older net's) and a strong Navi's signal,
which he names only where he has battled that Navi; the act's card is
the reveal, as on a layer. Stepping on a portal takes him there. Its
portals are this run's: a new run starts with the HP bare. The second
screen lists them as MegaMan reads them (piece 9).

Built first as ports in town (issue #86): the ways were the town's
jack-in points and the ground before its checks, MegaMan's words at each
(`home_port_words`) and their names in his arrival words. The words, the
ways (`run_ways`) and the build of a way's first layer at its port
(`home_take_way`) moved to the HP's portals; the town's ports keep their
cells for the jobs' comps (piece 5).

As built (issue #93): Lan's HP is BN6's own map whole (`lanhp.c`): its
tiles, walls and link markers, its song (`0x13`, every homepage's), no
people, map scripts or Mystery Data of BN6's, and a warp list of its own
in a space of its own ([Running the game's own logic](../../engine/emulation/overview.md)). BN6's HP is a hub already: its
blue pad, where the jack-in lands, is BN6's jack-out (warp entry 1,
departure `0x10`), its pink pad links to Central Area 1 and four link
squares on its floor to the town HPs ([The real world and home](../../rom/world.md), Lan's HP). The
run's portals are the pink pad (the act's own way) and the links (the
other way, the dark way, the rest for going back, piece 3); a link square
shows BN6's own marker, and its warp works, while its story flag is set,
so a lit portal is BN6's open link and an unlit one BN6's locked link
(the flags held every frame in the HP: BN6's markers lock their links
each frame). The act's exit lands MegaMan on the blue pad, the act's card
over the HP, his word on the act and the ways open ("Two ways are open
this time! The pink pad,and the link up top!"), and the run saved there
("From Lan's HP"); within reach of a lit portal he says what he reads
through it (its data, the signal), once a visit. Stepping on one plays BN6's link
warp; the first way's layer was built at the exit, another way's is built
as the link plays. R asks BN6's "MegaMan, jack out?", and its yes (or the
blue pad) takes Lan to where he last jacked in, his PC since issue #94;
the PC takes him back to the blue pad. A run's start goes the same way:
Lan's room, his PC, Lan's HP with one portal ("Our HP,Lan! Home sweet
home!", once a profile, then a word), the first layer. The second screen
shows the PET at home there. The way from the blue pad to the pink pad
runs up a corridor whose mouth (x 119, open from y -24 to -4) a walk
straight from the blue pad meets a few units off its line; a playtester
pushed at its lip for sixteen calls, the arrow pointing at the pad
through the wall (session 69). The arrow leads onto the corridor's line
first, and L names that first stretch, as the town's way does: from the
wall beside the mouth too, where MegaMan stops at its x, and where the
arrow had pointed at the pad through the wall and L said "straight up"
(session 74, eight calls pushing there); at home too, pushing a while
where the pad goes nowhere brings the arrow, as on a layer.

The courier (issue #108): playtests jacked out only when told to, and the
owner wanted no more "take time to explore" talk, so the choice is shown
where it is made. While the town holds something for the visit a Mr.Prog
stands in Lan's HP (`lanhp.c`, `director_courier.c`), on the floor below
the way from the blue pad to the corridor (world -84, 38: 52 units off the
way's line and from the nearest link square), facing the blue pad, with
BN6's "!!" burst over his head (sprite list 5's `0x07`, the burst AsterLand
paints beside its counter; there it is the map's own tiles, so the HP
shows BN6's sprite of it). Talked to, he names what waits and where in a
Mr.Prog's capitals, the most valuable first, a box each (`home_words.c`):
a finished request's pay with its asker ("REWARD WAITING! THE NETBATTLE
CLUB,THE ACADEMY!"), else the new requests posted while none is held, by
place ("NEW REQUESTS POSTED! ASTERLAND,THE ACADEMY AND TOWN!"; at the
run's first visit, before its first layer, when nothing is new yet,
"REQUESTS ARE UP AT ASTERLAND,THE ACADEMY AND TOWN!", session 74), then the
keys AsterLand's SubChip seller stocked ("ASTERLAND HAS UNLOCKERS!"). He
relays the town's posts and the shop's stock, never the Net ahead. What
the visit holds is fixed as it begins, from the run's state, so a
CONTINUE finds him as he was; what still waits follows the askers' flags
every frame at home (a reward taken, a request taken), which keeps flag
`0x146C` set while nothing waits: then he and his burst are gone, and his
words are written again as what waits changes. Nothing waiting, no
courier. He speaks only when spoken to. Heard once, his burst goes, as a
BN6 person's does once heard: his words end by setting flag `0x1472`,
which sends the burst off and which the director clears as his words
change. And the requests are left out of them once L has named them in
the town this visit (a playtester heard the same news from both,
session 73).

Going on against going back (issue #110, the owner after playing):
MegaMan arrives on the blue pad facing +x, up the corridor to the pink
pad, so a step out goes the way on, from a layer and from Lan's PC alike
(the PC's jack-in faced the camera, session 73). Nothing is said as he passes a
portal (he had stopped the player beside each, every visit): his words
on one come when the player checks it, A facing it from its edge
(`lanhp_portal_ahead`), as BN6's checks work. Going on and going back
look different in BN6's own art, nothing drawn over the picture (the
owner turned down a first build's text tags as unlike BN6): a way on keeps
BN6's link marker with its town's emblem (list 7's `0x88`, animations
1-4, the map objects `0xCB`-`0xCE`), a way back the same marker's plain
animation 0, an orange pad with a red ring, a floor sprite of ours on
the link square while it is a way back (flags `0x1470`-`0x1471`, set
while it is not). The marker's sprite is compressed and was never asked
for, so every lit link showed BN6's placeholder, a white ball: Lan's HP
now asks for its objects' sprites, and the courier's and the marks' (of the 12 a
map takes). A way back is locked by BN6's own rule, its story flag clear
(its emblem hidden, its warp turned off every frame by BN6's object):
stepped on or checked, MegaMan says where it goes back to and its price,
then asks "Go back,Lan?", the cursor on No as the vow's patch-up asks
(flag `0x146F` on Yes, which takes its link by BN6's own warp:
`lanhp_take`); a talk's question now takes left and right. A way on
through a link asks too, "Go this way,Lan?" after what MegaMan reads
through it, its warp held off as a way back's (`lanhp_lit`): a
playtester leaving a way back's pad stepped onto the link beside it on
his way to the pink pad, and it chose his act unasked (session 73). The
pink pad, the act's own way at the corridor's end, warps by BN6's own
warp, unasked. A portal draws in front of its trigger cells, BN6's own
as ours: MegaMan seen on a link marker's middle stood 14 units short of
the cells' middle in x and 12 past it in y, outside them; and BN6's
trigger for the pink pad stops short of its cells' front corner, on the
line x - y = 201 ([The real world and home](../../rom/world.md), Lan's HP), so MegaMan on its pink
middle, at 194, 0, stood outside it, and a playtester stood there in
four runs without a warp (session 74; v0.12.0 the same). A portal's
zone for the checks and the questions is its cells and its art as
MegaMan stands on it (`lanhp_portal_near`, `pad_art.c`: his feet on the
art or in front of it as far as his sprite covers it; stretched only
towards the screen's front, the pink pad's zone left out its left rim,
where a playtester stood 150 frames, session 75), and on the pink pad's
the director takes BN6's warp itself (`lanhp_pad_take`). A layer's exit
pad is taken the same way, MegaMan standing still on it as drawn
([The net's maps and their pieces](../../rom/net-maps.md), Warp pads). Its name stays
in MegaMan's words and on the second screen, whose home panel lists each
way as On or Back (the ways back were left out of it before: it looked
them up by the ways' own portals).
