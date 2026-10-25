---
type: Design
title: "Props"
description: "Props on a layer, cut whole from BN6's maps (`src/map/props.c`) and placed by measured rules (`net_landmarks.c`): the Net Dealer's counter, each area's landmark, rows and signs, pads' warp centrepieces, framed pads and emblems."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-06T11:29:31Z }
---

# Props

The originals furnish their floors: a Net Dealer behind his capsule counter,
NetCafe desks with their staff behind them, avenues of cybertrees framing a
giant tree, a statue between two braziers, rows of gravestones. They are what
makes a map read as a place, and none of it is scattered. Each prop does a
job, and the generator sets one only where it does that job:

- **A signifier.** A counter says "a service stands here" before a word is
  read, as a shop's awning does. The navi behind it is spoken to across it.
- **A landmark.** One set piece per layer that the eye finds and the way can
  be told by ("past the statue"): the originals' hubs, craters and giant
  trees.
- **The area's identity.** Each area furnishes itself from its own maps:
  Green's trees and potted bushes, the Undernet's statues and braziers, the
  Graveyard's stones, Sky's field pictures and pods.

The pattern is the hand-authored unit in a generated layout (the game-design
skill's handcrafted-pcg-hybrid): the units are Capcom's own, cut whole from
the ROM with their tiles, walls and the navi's place, and the generator
decides only where they go, by rules measured on the originals.

## How the originals place them

Measured over the internet maps and their object lists (the research's
counts, maps 0x90-0x96):

- **Counters face the camera** (6 of 7): their front towards the screen's
  foot, the navi behind on the far side. A dealer's capsule sits one panel
  in from a platform's back edge, the aisle behind it on the rim, or closes
  a dead-end stub; a NetCafe desk sits at its room's far end, facing the way
  in, with the WELCOME sign beside the entrance.
- **Tall props stand at the back** of their platform, in its back third
  (signs 6 of 7, statues 8 of 11, boards 8 of 10, braziers 5 of 6).
- **Clear of the way**: decoration keeps two panels or more from a
  walkway's mouth (trees 25 of 25, signs 7 of 7, statues 10 of 11) and three
  or more from the shortest route between the warps (trees 16 of 25,
  statues 8 of 11, boards 10 of 10). What stands in a mouth is a story
  obstacle, a gate.
- **Symmetry and rhythm**: braziers flank a statue as a mirrored pair;
  Green Area 2's nineteen cybertrees form a V along its field's rim, mirrored
  about its axis (18 of 19 have their mirror), framing the giant tree in the
  back corner; gravestones and Sky's trees stand in rows of three a panel
  apart; Green's potted bushes repeat every second panel in the gaps between
  its planks.
- **Edge or hole**: signs and braziers stand in the void just past the
  floor's edge (11 of 13); gravestones, boards and statues fill walled holes
  of one panel inside platforms (33 of 39).

## Counters (built)

The Net Dealer stands behind a counter wherever the area has one:
Sky Area 3's capsule in Central, Seaside and Sky (the four surface areas
share one tile set, each drawing it in its own colours) and Green Area 2's
NetCafe desk in Green (`RomLayout.net_area[].counter`, [ROM data](../../rom/overview.md)).
`src/map/props.c` cuts it: its second-layer tiles from the floor it stands
on, its ring of walls, its navi one cell behind the ring's back row and the
navi's talk centre 8 towards the front; the map's mirror image gives the
other way it can face. The generator (`ng_counter` in `net_landmarks.c`) puts it one
panel in from a room's back edge, grid -x or -y, facing the camera: the
aisle behind it on the platform's rim (void beyond) is walled off
(`C_SOLID`), its own panels keep their floor under the ring (`C_PROPPED`),
the floor before it stays free, and nothing it closes off may be cut from
the rest. It is centred along the edge, trying outwards from the middle.
Where the dealer's room has no such place, the next room that has one takes
him; the room before a guardian's arena keeps its dealer, standing if need
be. Over 400 layers per layout the dealer stands behind a counter on 97% of
Central's routes, all its fields and 16% of its catwalks (their mazes have
few rooms with a rim), 99% of Seaside's, 79% of Sky's routes and all its
hubs, 99% of Green's routes and 75% of its ladders. The navi is drawn under
the second layer (script 0x1B) so the counter's art covers its legs, and the
director aims A at the talk centre (OverworldNPCObject +0x11-0x13).

## Sets (built)

Each area furnishes its layers from its own maps' objects (`RomLayout.
net_area[].looks`): the originals' map objects by their ids, each at its
panel's spot 14 and 18 in from the corner as the originals set theirs,
spawned with the layer's map ([The real world and home](../../rom/world.md), Map objects). The
generator composes them after the services and before the loose Mystery
Data and bystanders, as a map is built before it is filled
(`ng_landmark`, `ng_rows`, `ng_signs` in `net_landmarks.c`):

| Area | Landmark | Rows and signs |
| --- | --- | --- |
| Green | the giant cybertree in a walled hole one panel in from a room's back rim, cybertrees past the rim on both sides of it, mirrored, up to four pairs | rows of three cybertrees past rims; the WELCOME sign by the counter |
| Undernet, Secret Area | the statue past a back rim, a brazier two panels to either side, mirrored; where the court has no such rim, in a walled hole in its floor, the braziers in holes beside it | |
| Graveyard | the monument past a back rim | rows of three gravestones in one walled hole of three panels in a big slab |
| Central, Sky | an avenue of cybertrees past the signature's back rim, mirrored, a panel's gap in its middle, two to four pairs (as Green Area 2 frames its giant tree) | rows of three cybertrees past rims; the WELCOME sign by the counter |
| Seaside | | a BBS past the signature's rim, else a big room's; the WELCOME sign by the counter |

The rules, from the originals' numbers above:

- **The landmark** takes the layer's signature first, where its shape
  holds the set (Identity): the giant tree a grove, the statue a court,
  the monument any. Else the best room for it: not the arrival's, the
  exit's or the arena, no pad, twelve panels or more, no counter in or
  beside it, a back rim (grid -x or -y, the top of the screen) of three
  panels (five for the statue's pair) with void two panels deep past it,
  its middle off the way between the warps; bigger rooms, longer rims and
  more distance from the way first. The set slides along the rim from its
  middle until it fits.
- **A walled hole** (the giant tree, the gravestones) has floor all round
  it, keeps a cell clear of stairs, rooms' anchors, walkway mouths and
  objects, stands off the way, and cuts nothing off.
- **Past a rim** a prop stands in the void a panel out, with void a second
  panel out, so never in a gap between two platforms.
- **Rows** of trees go past the rims of up to two other rooms, trees two
  panels or more from the way; rows of gravestones go in slabs five panels
  or more across, up to two.
- **Signs**: the WELCOME sign in the void at the end of a counter's aisle,
  as the originals set theirs by a NetCafe's way in; a BBS past a big
  room's rim.

Over the tests' 300 layers with every look: 2452 props, 270 of them in
walled holes, a landmark on 153 layers (the areas' own looks give fewer),
the Undernet's statue on 16 of its 30 (1 before its court, issue #98).
The landmark is set before the services, so no counter crowds it.
A map loads at most 12 compressed sprites and 0x8800 bytes of them (the
game's loader); a prop whose sprite no longer fits is left out whole, not
shown as noise.

On Nebula Area's layers (BN5's, [Other games' ROMs](../../multirom/overview.md)), whose own maps set no
props of BN6's, BN5's dark hole stands where the Graveyard's monument would:
in the void past the back rim of the best room, by the landmark's rules,
read off the layer once it is made (`layer_landmark_void`), so the layer
is the same with BN5 or without it. It keeps out of a Rush gap's and an
invisible path's panels.

## Pads' centrepieces: warps only (built)

The originals' 3x3 pads carry art on their middle panel, second-layer tiles
over the floor: the red gem, the link ring and the cube on its round base,
each 28 tiles, the same tiles in Central, Seaside, Green and Sky in each
area's colours. `props_learn_ornament` finds them in an area's own maps by
their tiles (0x379, 0x372, 0x375) and cuts each on the corner of the panel
it lies on. None of them is decoration: all 29 in BN6's net maps stand on a
warp pad (the gem on a teleport within the map, always one of a pair; the
cube on a link to a homepage; the ring on a link to a comp or another
area). They were set on the middle of every pad, so most layers showed warps
that did nothing, which players take for warps; the owner ruled it out. Now
only a teleport pair's two pads carry the gem (`paste_ornaments`, Teleports
below), and a pad without one keeps its plain floor, or its frame's empty
recess, as BN6's pads without a warp do.

## Central's, Seaside's and Sky's framed pads (built)

Central Area 1's pads are one piece of art over their 3x3 panels: a pale
frame with violet corners, a cyan band and a blue recess that holds the
centrepiece, the catwalks meeting the frame's edges; Seaside's the same in
yellow with green corners, Sky Area 2's in lavender with pink ones. The
tile classes drew a layer's pads in Central's green floor (the framed
pads' blue lies in the walkway material, and classed as walkway their
pieces came apart). `props_learn_pad` takes the 3x3 islands of floor in the
area's maps whose middle has the recess's hue (`net_area.pad_hues`:
Central's blue, Seaside's yellow, Sky's lavender) and lays them over each
other: the first layer's tiles over their panels, the faces under them and
the rim their upper edges reach past the diamond, each tile from an island
where it draws the island alone, the one with the fewest bridges beside
its lower sides first (every island has a bridge somewhere, and a bridge's
start came along in the tiles beside it: Seaside's pads hung a scrap of
boardwalk off their lower edge, and where the rim was left to the classes
Central's showed tile-wide steps along its upper edges). `paste_pads` sets
it on every 3x3 pad at ground level away from the stairs, each tile only
where it covers the pad and the void alone (a catwalk's last panel came
out cut where the stamp's edge lay over it), and the centrepieces go on it
(never art that draws mostly black: Sky's gem tile is the pit in one of
its platforms). A pad itself is walkway floor in these areas, as the
originals' are, so the tiles the stamp leaves at a catwalk's join are the
originals' join of a catwalk into a pad (as platform floor they drew the
catwalk meeting a green field). Green's are not cut: its pads stand
raised on the originals, and would sink MegaMan into them.

## Green's potted bushes (built)

Green's maps set potted bushes, a 4x4 block of second-layer tiles, in the
void gaps between parallel planks, one every second panel, plain and in
flower in turn (32 in Green Area 1, 25 in Green Area 2). `paste_bushes`
does so in a void panel with floor on both sides along one axis and void
on the other two, every second one along the gap, never over the floor's
own art.

## The Graveyard's and the Undernet's emblems (built)

The Undernet's plateaus carry the same cross in magenta, alone or four in
a diamond; the Graveyard's slabs carry cyan crosses drawn in the floor
itself (the first layer, not second-layer art): one on each small platform's middle
panel, and on its big slabs in rows alongside the line of holes through
their middle, two panels to either side, at even steps, never at a rim or
beside a hole. `props_learn_floor_emblem` finds them by their colour
(BGR555 0x56C0, which the floors show nowhere else) and cuts the tiles
that draw one where it lies on a panel with floor all round; the
generator sets `PROP_EMBLEM`s by that rule in every room, on ground floor
with floor all round and nothing standing there (`ng_emblems` in
`net_landmarks.c`), and `nm_paste_emblems` (`netmap_paste.c`) draws them in place of the floor there.
The slabs' holes follow the originals too: a line along the middle rather
than a grid, three panels apart where the originals keep two (the floor
between two holes one panel apart reads as a walkway to the tiles, and
takes a bridge's link look).

## Next

The field pictures and a hub's centrepiece where the layout has a centre;
each a stamp cut like the counters, set by the same rules.
