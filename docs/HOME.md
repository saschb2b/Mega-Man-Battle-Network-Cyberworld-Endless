# Home

How the real world and MegaMan's own corner of the net take part in a
run: Central Town, where Lan lives and does his errands, and Lan's HP, the
warp zone the run's portals grow in; the people's jobs, the shop, a town
that remembers. Reasoned with the game-design skill (the rework procedure,
the pattern catalog, the transitions budget) on 5 October 2026, then
reworked the same day for a ludonarrative problem the owner saw; the
owner's decisions are below. How the town is built: docs/OVERWORLD.md.
What carries over between runs: docs/META.md.

## Where it stands

A run's town (Central, ACDC, Seaside or Green Town) was the run's first
minute: Lan walks from where he starts to the landmark's port and jacks
in, and the run never sees it again. Its people say a line about the
place, its checks describe the houses, a Mr. Prog explains R. The owner:
"just a theme park, a stark short walk to the jack-in and then
forgotten".

The skill's rework procedure names what is wrong:

- **No dialectic.** The town restates none of the run's tensions: it is
  decoration, not design.
- **No decision.** The walk has one way and one end.
- **A hollow loop.** Nothing the net gives reaches the town, and the town
  gives the net nothing.
- **Seen once.** Ten to thirty seconds a run.

The first answer (issues #85 and #86, built) brought Lan home to the
town after every act and made the act's ways ports in town. The owner
played it and named the problem: MegaMan jacks out, Lan stands in the
town, and then jacks in at a doghouse or a fountain to go on through the
Endless Net. In BN6 a jack-in point leads to its object's own comp; the
net beyond is reached from Lan's PC, through Lan's HP. The Endless Net
wants one place of its own.

## The experience

**Two homes, one for each of them.** Central Town is Lan's: his house,
his errands, people who have heard what MegaMan did. Lan's HP is
MegaMan's: the place every jack-in arrives, where the run's portals light
up as its guardians fall, and where he chooses to press on or go back.

Not: a chore between acts, a second setup screen, a walk that taxes every
restart, a hub of menus.

## The dialectic

The run's own, **prepare or press on**, made spatial. In Lan's HP the
newest portals press on; an older one goes back to an area already won,
to prepare, at a price: the Net's clock. In town, a job taken bends the
next dive, zenny spent at home is not carried to the net's dealers.

## The loop stack

| Loop | Before | With home |
| --- | --- | --- |
| Progression (an act) | Ends on its guardian and "Which way?" | Ends in Lan's HP: the next act's portals light up, and the run is saved there |
| Run | The town once, then only the net | Lan's HP between acts, the town when Lan has errands; the HP grows with the run |
| Meta | The town forgets | The town remembers the runs (recognition and variety, never power) |

## Patterns

- **The waypoint hub** (Diablo's waypoints, Darkest Dungeon's hamlet and
  its regions): one place the reached destinations hang from, the newest
  the push, the older ones the farm. The farm is the pitfall (below).
- **The time-difficulty clock** (Risk of Rain): the longer a run takes,
  the harder what lies ahead. Going back is allowed and priced, not
  forbidden.
- **The bonfire rest point** (Dark Souls, Hollow Knight): one safe stop
  per act, with a checkpoint; its spacing is the act.
- **The hub as the same place** (Hades' House, Cult of the Lamb): a home
  works when it is the same each run, so its people and changes can be
  noticed.
- **The dual-life loop** (Moonlighter, Cult of the Lamb): the net
  supplies what home asks for, home gives the net its goals. The halves
  feed each other or one becomes a chore; the town stays short and
  optional against an act.
- **The draft with a skip** (Slay the Spire, Hades): a job from one of up
  to three people, or none.
- **The branching map** (Slay the Spire's map, Hades' doors): the next
  act's ways as portals side by side.
- **Flavor as meta-narrative**, and its candidate the diegetic newspaper
  (Mina the Hollower): people and the news report the run.
- **The bonus with a drawback**: a job's constraint is its price.

## What BN6 has at home

Verified in bn6f (Falzar, its names; `~/.cache/mmbn-ref/bn6f`) and in
Gregar's ROM unless said otherwise, 5 October 2026:

- **Lan lives in Central Town**: its group (`0x01`) holds the town
  (`0x00`), Lan's house (`0x01`), his room (`0x02`), the bathroom (`0x03`)
  and AsterLand (`0x04`) (bn6f `constants/enums/GameAreas.inc`).
- **Lan's PC jacks in to Lan's HP**: the jack-in table of Lan's room
  (Gregar `0x0804F874`, read through the literal its group's map-enter
  routine stores to GameState `+0x64`) sends point `0x40` to destination
  1, which is Lan's HP (`0x88:00`, at -178, -14). Lan's HP is a small
  room in cyan frames with a dark circuit floor and two warp pads, blue
  and pink (drawn from the ROM with `--atlas DIR:g88`); no area of ours
  takes that map (the layers take ACDC, Aquarium, Green and Sky HPs,
  `0x88:01`, `03`, `05`, `06`).
- **The towns' other points lead to their objects' comps**: Central
  Town's two (`0x40`, `0x41`) to destination 0, the RoboDog Comp
  (`0x8C:00`); ACDC Town's squirrel statue to ACDC HP (`0x88:01`) and its
  doghouse to the doghouse comp (`0x8D:0E`); the bathroom to the
  bathroom comp (`0x8C:06`). BN6 has a comp per jack-in object: the fish
  stick shop's, the water machine's, the symbol's, the popcorn shop's
  and more (groups `0x8C` and `0x8D`).
- **The Request BBS** is in the real world, a board in AsterLand
  (Central Town): 35 requests in the US text, posted by students,
  shopkeepers, a film crew, the NetPolice, Mr. Famous and others; each
  names a meeting place; "You can only pick one request at a time."
  (`CompText87EFE14`). Ranks C to Master by points. Rewards: Virus
  Deletion pays 10 BugFrags (`CompText876C2A8`); others (RegUP2,
  HPMemory) are the community's word, not verified.
- **AsterLand's counter** (Central Town, its clerk Tab) has In-Stock Chips
  (shop 4) and the Order Service (shop `0x12`): a paged list built from
  per-chip flags, the Library's among them (`sub_8048DF8`); for a chip
  picked the clerk names one code and a price ("I've got some of that
  chip in code X! It will be N Z"), or says it is not in stock; the chip
  is given at once (`CompText87F2094`). Higsby's in ACDC Town is closed
  all game ("off in Netopia... again").
- **A SubChip seller** stands in each town (shops `0xB`-`0xF`: ACDC,
  Central, Seaside, Green, Sky).
- **Jack-in points in objects**: R on trigger cell `0x40` + n jacks in
  unless flag `0x16D0` + n is set (`sub_8034CB6`), each to the
  destination its map's table names (destination table `byte_80984C8`,
  Gregar `0x08099A00`, 43 of 20 bytes).
- **Jacking out**: R on a net map asks "MegaMan, jack out?"
  (`CutsceneScriptAskJackOut_80988E4`); Lan stands again where he jacked
  in, and MegaMan's HP is made full at both the jack-in (`sub_8033FDC`)
  and the jack-out (`navi_80340F6`). The jack-in also rolls every net
  map's green Mystery Data again (docs/ROM_DATA.md, Map takeover).
- **No day or night**: one palette per map, chosen by its group and
  number (`initMapTilesState_803037c`); nights are captions ("That
  night..."). BN6 does have story weather in its towns (rain, snow, wind)
  and a looping palette dim in Green and Sky Town (events `0xA9B`,
  `0xA9C`).
- **Its people change with the story**: four stage archives per map,
  picked by GameState `+0x08` (`chatbox_selectCompTextByMap_8040730`),
  and map scripts set out other people per chapter. The living room's TV
  is a check; news comes in cutscenes.

## The owner's decisions (5 October 2026)

First:

1. **Home after every act**, and a richer first visit: the run's start
   gets the jobs, the shop and a town that remembers too.
2. **The act's ways are lit ports in town**, not the Guardian Data's
   question.
3. **Home offers** jobs from the townsfolk, a town that remembers, a home
   shop, and time of day by act.

Then, after playing the ports ("MegaMan jacks out, Lan is in the city
and then jacks in somewhere to go on through the Endless Net"):

4. **Lan's HP is the warp zone**: every jack-in arrives there, and its
   portals are the ways on.
5. **Its portals are this run's**: they light up as the run's guardians
   and Nests fall, and a new run starts with Lan's HP bare.
6. **Going back is priced by the Net's clock**: an older portal goes back
   to an area already won, and each trip back makes what lies ahead a
   notch harder.
7. **An act's exit leads to Lan's HP; the town is optional**: R there
   jacks out to the town when Lan has errands.
8. **Central Town is always home**: Lan's house, his room and his PC.

## The pieces

### 1. After an act: Lan's HP

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
down where he last jacked in (docs/EMULATION.md); the AREA CLEAR card
over the town while Lan is held, MegaMan and Lan's word by how far the
run has come (`home_words.c`), the run saved ("From home"). That trip
stays, as the jack-out from Lan's HP, and its words move there.

### 2. Lan's HP, the warp zone

BN6's own Lan's HP (`0x88:00`), taken over as the town is: its arrival
pad where every jack-in lands, and a portal for each way the run has
opened, BN6's warp pads standing in rooms of its own look. It starts
bare: one portal, the first act's. Each guardian deleted adds the next
act's (two, or three with the dark way: the act's own area, the other
way, the Undernet once the Secret Area is cleared, as `run_ways` gives
them today), each Nest the next Net's first, and each lights up as
MegaMan arrives; the HP grows a room as its portals outgrow it, so the
run's progress is a place. Standing on a portal, MegaMan says where it
leads and who waits there (the guardian named as on the act's card, where
MegaMan has battled him); stepping on it takes him there. Its portals
are this run's: a new run starts with the HP bare. The second screen
lists them (piece 9).

Built first as ports in town (issue #86): the ways were the town's
jack-in points and the ground before its checks, MegaMan's words at each
(`home_port_words`) and their names in his arrival words. The words, the
ways (`run_ways`) and the build of a way's first layer at its port
(`home_take_way`) moved to the HP's portals; the town's ports keep their
cells for the jobs' comps (piece 5).

As built (issue #93): Lan's HP is BN6's own map whole (`lanhp.c`): its
tiles, walls and link markers, its song (`0x13`, every homepage's), no
people, map scripts or Mystery Data of BN6's, and a warp list of its own
in a space of its own (docs/EMULATION.md). BN6's HP is a hub already: its
blue pad, where the jack-in lands, is BN6's jack-out (warp entry 1,
departure `0x10`), its pink pad links to Central Area 1 and four link
squares on its floor to the town HPs (docs/ROM_DATA.md, Lan's HP). The
run's portals are the pink pad (the act's own way) and the links (the
other way, the dark way, the rest for going back, piece 3); a link square
shows BN6's own marker, and its warp works, while its story flag is set,
so a lit portal is BN6's open link and an unlit one BN6's locked link
(the flags held every frame in the HP: BN6's markers lock their links
each frame). The act's exit lands MegaMan on the blue pad, the act's card
over the HP, his word on the act and the ways open ("Two ways are open
this time! The pink pad,and the link up top!"), and the run saved there
("From Lan's HP"); within reach of a lit portal he names where it leads
and who waits there, once a visit. Stepping on one plays BN6's link
warp; the first way's layer was built at the exit, another way's is built
as the link plays. R asks BN6's "MegaMan, jack out?", and its yes (or the
blue pad) takes Lan to where he last jacked in; the town's jack-in takes
him back to the blue pad. A run's start goes the same way: the town, its
jack-in, Lan's HP with one portal ("Our HP,Lan! Home sweet home!"), the
first layer. The second screen shows the PET at home there.

### 3. Going back, and the Net's clock

An older portal goes back to an area the run has won: a layer of it at
its act's band, its battles, dealers and data of its tier. Each trip back
moves the Net's clock a notch: the layers ahead are made a notch harder
(the numbers come with the story, through the game-design skill and
`build.py pacing`), and L and the second screen say how far it has run.
The clock is the price that keeps going back a choice: grinding an old
area until the next act is trivial would be the dominant strategy if it
were free.

### 4. Central Town, home

Every run's town is Central Town: Lan's house and his room join it as
BN6 has them, and his PC is the run's jack-in. The other towns' plans
stay for later uses (a job's trip by the Metroline), not as a run's
home. A home is the same place every run, so the people who remember can
be noticed.

### 5. Jobs (the Request BBS)

At each visit up to three townsfolk have a job for the next act, and Lan
takes one, or none: BN6's own rule, one request at a time. A job asks
what an act can give, in one of a few kinds:

- **Bring**: a chip in a code, or of an element, handed over at home.
- **Busting**: delete a virus family with an element, or win a battle
  fast; or bust the viruses in an object's comp in town (the RoboDog's,
  the bathroom's: BN6's own comps, through the object's own jack-in
  point, a short errand of one layer).
- **Find**: the act's Mystery Data of a colour, a ScrtData.
- **A vow** (a job's drawback is its price): beat the guardian with no
  Recovery chip, reach his door above half HP.

It pays at the next visit, from the one who asked: zenny, BugFrags, a
chip of the Library in the folder's codes, a program, a key item. Pay is
the run's only; a job remembered across runs is a line, not a bonus.
Without a job an object's jack-in point is shut, with a word from
MegaMan.

### 6. The home shop

AsterLand's counter in Central Town. Its **Order Service** is BN6's: a
chip the Library holds, its code and price named by the clerk, given at
once; one order a visit, priced above the net's dealers, so the net's
stock stays the gamble and home the sure thing. The town's **SubChip
seller** stands beside it, as BN6 has one in each town.

### 7. A town that remembers

- **The run**: people speak of the last guardian deleted, the act,
  MegaMan's HP, the Cross he brought, the threat. One line each, BN6's
  voice (docs/VOICE.md); returning players hear less.
- **The runs**: how the last run ended, the best layer, a guardian beaten
  often ("Back for SpoutMan again, Lan?").
- **Crowds that move**: people stand elsewhere at each visit, and more of
  them walk (sprites with all four walks, docs/OVERWORLD.md).
- **The news**: a board or the TV reports the net (the guardians deleted,
  the depth, Lan's name beside the best layer).

### 8. Time of day by act

Morning at the start, afternoon after act 1, evening after act 2, night
before the Nest: the run's progress on the town's sky. BN6 has no time of
day, so this is the engine's, and it leans on BN6's own effects first:
its towns' palette dim (Green and Sky Town's), its rain, snow and wind,
then a tint of the town's palettes where BN6 has none. Judged by captures
at every hour before it ships; docs/FIDELITY.md names it.

### 9. The second screen at home

The PET at home (issue #79) and in Lan's HP lists the portals (area,
guardian, the clock's notch for going back), the job taken and how far
along it is, and the visit's hour.

## What must not move home

- **Power across runs.** Portals are the run's; pay is the run's; the
  profile keeps lines.
- **The setup screen.** Walking to pick a folder would tax every restart;
  the setup stays a screen.
- **A required town.** The town is for errands; a dive straight through
  Lan's HP never needs it.
- **Long chats.** One line a person, shorter for returning players.
- **A heal that matters.** The trips heal as BN6's jack-out and jack-in
  do, after a guardian who healed already.
- **Free farming.** Going back is priced by the clock; one job at a time.

## Time

Lan's HP: about 10 seconds for a player who steps on the newest portal.
The town: nothing for one who skips it, 60 to 90 seconds for one who
takes a job and shops. Three to five minutes in a short net of about an
hour, at most.

## What could go wrong

- **Two hubs, twice the walking**: the HP small and the newest portals
  beside the arrival pad; the town only when wanted.
- **The farm**: going back made the best play. The clock's notch is the
  dial, checked against `build.py pacing`.
- **A maze of portals**: a long endless run collects many; the HP shows
  the newest ways first, the older ones in rooms behind.
- **The economy**: jobs, the shop and going back add zenny, chips and
  items. Pacing checks every act's band with them.
- **A dominant order**: the Order Service always buying the best chip.
  One order a visit, at a premium.
- **Saves**: a checkpoint in Lan's HP or the town rebuilds them, and the
  layer behind each portal, before it loads the state.
- **Fidelity**: Lan's HP's rooms beyond BN6's one and time of day are
  the engine's; docs/FIDELITY.md names them.

## Phases

The epic's stories, in order: home after every act (#85, built); the
ports in town (#86, built, to move); Lan's HP as the warp zone, the
act's exit there and the town optional; Central Town as home, with Lan's
room and PC; going back and the Net's clock; a town that remembers;
jobs, with the objects' comps; the home shop; time of day; the second
screen; the captures, docs and notes.
