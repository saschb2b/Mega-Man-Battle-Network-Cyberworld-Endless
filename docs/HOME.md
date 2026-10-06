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

Then, asked whether the run uses the school and the chip store (6
October 2026):

9. **AsterLand is a place of its own**: BN6's own AsterLand, its counter
   the home shop (piece 6), the Request BBS on its wall the jobs (piece
   5), and a Chip Trader where BN6 has one.
10. **The Cyber Academy opens for jobs**: no class today, so the gate is
   open for the clubs; classmates in Lan's classroom and the NetBattle
   club post some of the jobs and talk about the run. A few of its maps,
   no new systems.

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
("From Lan's HP"); within reach of a lit portal he says what he reads
through it (its data, the signal), once a visit. Stepping on one plays BN6's link
warp; the first way's layer was built at the exit, another way's is built
as the link plays. R asks BN6's "MegaMan, jack out?", and its yes (or the
blue pad) takes Lan to where he last jacked in, his PC since issue #94;
the PC takes him back to the blue pad. A run's start goes the same way:
Lan's room, his PC, Lan's HP with one portal ("Our HP,Lan! Home sweet
home!"), the first layer. The second screen shows the PET at home there.

### 3. Going back, and the Net's clock

An older portal goes back to an area the run has won: a layer of it at
its act's band, its battles, dealers and data of its tier. Each trip back
moves the Net's clock a notch: the layers ahead are made a notch harder
(the numbers come with the story, through the game-design skill and
`build.py pacing`), and L and the second screen say how far it has run.
The clock is the price that keeps going back a choice: grinding an old
area until the next act is trivial would be the dominant strategy if it
were free.

Reasoned with the game-design skill for issue #95. The dialectic is the
run's own, prepare or press on: an older portal is the preparing, the
next act's portals the pressing on, and the clock the price that keeps
the choice honest. It feeds the progression loop, the act: a trip's
chips, zenny and BugFrags go into the next guardian's fight, and the
price lands on that same fight, so the trade is read where it is paid.
The patterns: the waypoint hub (Diablo's waypoints) and its pitfall, the
farm; the time-difficulty clock (Risk of Rain), counted in trips rather
than in seconds, as nothing else in a run is timed; the bonus with a
drawback; and skipping as a choice of its own (a trip is worth taking
when the folder lacks something, not as a habit).

As built (issue #95):

- **The portals.** The two link squares the ways leave free (portals 3
  and 4: the link on the left, the link down front) go back to the last
  two acts the run has won, newest first (`pacing_older_acts`: across a
  new cycle into the last one's acts, never the Nest). Each opens once a
  visit: taken, it stays dark until a way is taken. Two trips a visit at
  most; a short net can take five (one after act 1, two after acts 2 and
  3).
- **A trip.** One layer of that area, of a kind of its own
  (`LAYER_BACK`), at the depth of its act's middle layer: its band, its
  battles, its Mystery Data and its stock of that tier, the Net Dealer and
  a Recovery Mr.Prog as an act's middle layer has them, the NaviCust
  vendor's chance, traders and Servers as rolled, its set pieces; no
  guardian, duel, gate, dark warp, ScrtData, HPMemory or RegUp (what the
  run's own layers hold stays there, `run.home_depth` the layer it comes
  home to). Its layout is new each trip (the clock is in its seed), dressed
  as its act was. Its card says "A trip back" and the clock, MegaMan "We're
  back in Seaside Area,Lan!", L "The exit pad takes us home." Its exit pad
  leads to Lan's HP: the run's own layer is made again, MegaMan says "Home
  again,Lan!", and the run is saved there.
- **What it pays.** What the area's layer holds: its battles' chips and
  zenny (MegaMan is stronger than when he left it, so they go quickly), its
  Net Dealer's chips (his pick answers the area's viruses: its guardian is
  deleted), its Mystery Data. The run's power, HPMemory and the guardians'
  data, stays where the run goes on: a trip prepares, it does not level.
- **The price.** Each trip moves the Net's clock a notch (`run.clock`),
  and every guardian after it has a tenth more HP a notch
  (`RUN_CLOCK_PERCENT`, `pacing_clock_hp`): BN6's as they spawn (the hook
  that caps the duel's, `emu_battle_clock`), BN5's in their stats row
  (`guest.c`). The pacing report's last section shows the effect: act 1's
  median guardian (500 HP) stands at act 2's band floor after two notches
  (600), act 2's (700) at act 3's after two (840). Two trips cost about an
  act of a guardian's HP. Random battles are left as they are: a version
  up brings BN6's better drops, which would pay the price back, and the
  guardian is the fight a trip prepares for.
- **Where it is said.** Beside an older portal MegaMan names the area (he
  has been there) and the price, all of it the first time ("But the Net's
  clock keeps running...|Each trip back,the guardians ahead get
  tougher!"), then a line; L in Lan's HP says in words how far it has run;
  on a guardian's layer L adds "The Net kept copying while we went
  back..."; the second screen's home panel shows the Net's clock ("2
  notches: guardians' HP +20%"), and Dad's report in the PET's Comm the
  trips and the percent.
- **What must not happen.** Grinding an old area until the next act is
  trivial (each older portal once a visit; the run's HP and the guardians'
  data stay ahead), a death spiral (a trip is safe, and its price a tenth
  of a guardian's HP), a price kept hidden (it is said before the first
  trip, on the card and on the second screen).
- **The fiction.** The Endless Net copies every battle MegaMan fights, its
  guardians too (the README's opening): while he goes back, whatever is
  at the bottom keeps copying.

For tests, `--dev clock=N` starts a run with the clock at N notches, and
`CYBERWORLD_AUTOPILOT_BACK=N` has the autopilot take N trips back from
Lan's HP before it takes a way (docs/DEVTOOLS.md).

### 4. Central Town, home

Every run's town is Central Town: Lan's house and his room join it as
BN6 has them, and his PC is the run's jack-in. The other towns' plans
stay for later uses (a job's trip by the Metroline), not as a run's
home. A home is the same place every run, so the people who remember can
be noticed.

As built (issue #94): the town is Central Town at its original width,
the same plan every run (`CYBERWORLD_TOWN_STYLE` and `_VARIANT` still
pick another for a test). A run begins in Lan's room, at the top of its
stairs where BN6 sets him down; the run's words play there, and R at his
PC jacks MegaMan in to Lan's HP (`lan_house.c`). BN6's own jack-in from
the PC (destination 1) runs a line of its story's that turns Lan back
("Lan,let's check out the town first!"), so the room's table goes
through the town's destination with Lan's plain "Jack in!". The house
and the room are BN6's maps, their furniture, checks and song (`0x04`)
kept, no people or story scripts; the room's stairs and the house's
front door and stairs are BN6's doors, the bathroom's left out, and the
front door lets Lan out where the planned town has it. The town keeps
its houses, people and checks, and Lan's front door (its warp 1, carried
with his house) is its door home (AsterLand's and the Academy's open
too, piece 10); it has no jack-in of its own. R in the
town, the house, or the room off the PC has MegaMan say where the PC is
("Home's right here,Lan! The PC's up in your room!"). BN6's jack-out from
Lan's HP sets Lan down at his PC.

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

As built (issue #88; `jobs.c`, `job_words.c`, `director_jobs.c`): at each
visit three people post a request for the act ahead. The NetBattler at
AsterLand's request board asks for a chip of an element from the Pack
(paid with a chip a tier up, in the folder's codes) or a vow, no
Mr.Prog's patch until the act's guardian falls (an HPMemory); the
NetBattle club's member in class 6-1 three wins in ten seconds each
(BugFrags) or clean wins, no HP lost (zenny); the man from the lab in
town every Mystery Data of one layer, three or more, or clean wins
(zenny). Each kind's price is what it bends: a busting style, a layer
searched through, a chip given up, a guardian fought without the heal
before him; a skip costs nothing. The kinds come from the run's seed and
the act, none twice at a visit; the pay grows by the act and stays under
what the act's own sources give (`build.py pacing` lists it: act 1's 500
zenny against its guardian's 600 and an HPMemory's 800). Lan takes one
with Yes; the others then say one at a time is enough (BN6's Request
BBS's rule), and the board reads its post until it is taken. In the Net
the request counts its own act's battles (BN6's and the older net's; a
guardian's counts for none), each layer's Mystery Data as MegaMan leaves
it, a Mr.Prog's patch and the guardian's fall; MegaMan says when one is
done or a patch broke the vow, and reminds of a vow as the act begins.
At the next visit the asker settles it: the reward in BN6's own words
("MegaMan got: ..."), a chip handed over from the Pack (BN6's own count,
which leaves out the Folder's copies), or a word where it was not done;
until then L at home says where the reward waits, and no other request
can be taken. The run's save holds the request (`RUN_MAGIC` "CWE9"; an
older save continues with none). Not yet: the objects' comps as a busting
errand; the second screen's line on it (piece 9).

### 6. The home shop

AsterLand's counter in Central Town. Its **Order Service** is BN6's: a
chip the Library holds, its code and price named by the clerk, given at
once; one order a visit, priced above the net's dealers, so the net's
stock stays the gamble and home the sure thing. The town's **SubChip
seller** stands beside it, as BN6 has one in each town.

As built (issue #89): BN6's own clerk (its sprite and face, list 6's
`0x33`) stands at AsterLand's register and opens BN6's Order Service
(shop `0x12`): every chip of BN6's order list the Library holds, Standard
and Mega, each once a run as BN6 has it; the run sets each one's code to
the folder's where the chip comes in it, and its price to twice a Net
Dealer's (a common chip 1000 zenny, a Mega 8000: `build.py pacing`), and
marks the Library's chips owned as BN6 marks a chip it gives (the order
checks the mark). One order a visit: after it the clerk says so until the
next visit. The SubChip seller (BN6's, the white-coated man of its SubChip
shops' window) stands beside him with BN6's Central Town shop (`0x0F`)
restocked for the run: the keys the act ahead's locks want (Unlockers,
RushFood, a WWW-ID), then MiniEnrg, FullEnrg, SneakRun and Untrap at BN6's
prices (its LocEnemy is no use in a run). BN6's In-Stock Chips (shop 4)
stays out: the Net Dealers are the run's stock. The second screen's shop
panel reads the list as the screen shows it (sorted, sold-out ones kept
as BN6 keeps them), so the order's chip under the cursor is the one it
names.

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

As built (issue #87): Central Town's plaza Mr.Prog, who explained the old
statue's port, calls the Net's news in a Mr.Prog's capitals: at a run's
start how the last run ended ("BLASTMAN'S COPY DELETED MEGAMAN! ON LAYER
3!", the Nest brought down, or the layer it reached; the very first run
the new Net under town), after an act the guardian just deleted and
where. Four people have lines of the run's (`town_words.c`, one line a
person, each visit its own): Lan's classmate ("Lan! You beat BlastMan's
copy? No way!", then the guardians counted), the neighbor (the threat,
then "Back home already?"), the man from the lab (the best layer Dad keeps
looking at, then the readings coming in) and the gossip by the statue (a
guardian MegaMan has beaten three times or more, else the Net that copies
every battle); the rest say what is said at their place. At each visit a
different one in four is out and the standing people trade places
(`town_folk.c`: from the run's depth, so a CONTINUE finds them where they
were; a walker keeps his walk, the Mr.Prog and the robot dog theirs).
The profile keeps how the last run ended (`last_lost_to`, `last_won`).
Not yet: more of them walking, and the TV in Lan's house.

### 8. Time of day by act

Morning at the start, afternoon after act 1, evening after act 2, night
before the Nest: the run's progress on the town's sky. BN6 has no time of
day, so this is the engine's, and it leans on BN6's own effects first:
its towns' palette dim (Green and Sky Town's), its rain, snow and wind,
then a tint of the town's palettes where BN6 has none. Judged by captures
at every hour before it ships; docs/FIDELITY.md names it.

As built (issue #90, the owner's look still to come before a release):
BN6's own dim is a palette animation of Green and Sky Town's (their map
scripts load it while event `0xA9C` is set), which Central Town has none
of, so the hour is the engine's. Its picture is tinted as it is drawn
(`scene_emu.c`; draws change nothing in the game): each colour channel of
the map's layers and of the sprites over it times the hour's, a touch
warm in the afternoon, orange in the evening, a dim blue at night. mGBA's
picture names each pixel's layer (its top byte), so the chat's text
(BG0) keeps its colours, and with a chat box open so do the sprites in
its rows (its frame and the speaker's face). Only the town's own map,
through its doors' fades too (the game is off the map while one fades,
and a door first flashed daylight), not under the PET's menu: Lan's
house, AsterLand, the Academy and Lan's HP keep BN6's light. The
hour is the run's (`home_hour`): morning at its start, then afternoon,
evening and night before the Nest (the endless net two acts an hour, a
new morning with each cycle). BN6's rain, snow and wind are left for
later.

### 9. The second screen at home

The PET at home (issue #79) and in Lan's HP lists the portals as MegaMan
reads them (their data, the guardian's signal named where he knows it;
the areas gone back to by name, as he has been there; the clock's notch
for going back), the job taken and how far along it is, and the visit's
hour.

As built (issue #91): the home panel names the visit's hour beside
MegaMan's name, and reads
Next, the request held ("The club: 3 wins in 10 s each, 1 so far", done
or broken; on a layer too), the ways ("Pink pad: Sky HP (CircusMan)", a
link's, "Back:" an older portal's, "???" for a guardian MegaMan has not
met), the Net's clock and the run's setup, the most wanted first as the
panel's height lets them (the 3DS's shows the first three).

### 10. AsterLand and the Cyber Academy

The town's doors to AsterLand (`0x01:04`, the town's warp 4) and the
Cyber Academy (its group `0x02`; the gate is the town's warp 2, into the
foyer `0x02:06`) open, as Lan's front door does: BN6's own maps, their
furniture, checks, doors and songs kept, no people or story scripts of
BN6's, the run's own people in them.

- **AsterLand**: the counter's clerk (the home shop, piece 6), the Request
  BBS (the jobs, piece 5) and BN6's own Chip Trader (the trade screen has
  AsterLand's prize list, map key `0x104`: 70 chips of rarity 1 to 3,
  docs/ROM_DATA.md, Chip Traders).
- **The Cyber Academy**: the foyer, the 1F hallway (`0x02:04`), the 2F
  hallway (`0x02:05`) and Lan's class, 6-1 (`0x02:00`): classmates who
  post jobs and speak of the run (the run's own Lan of class 6-1), the
  NetBattle club. The other rooms' doors stay shut.

Verified in Gregar's own tables (6 October 2026): Central Town's warp
lists (its group's loader `0x0804F4B0`, lists `0x0804F210`: the town's
five entries, AsterLand's one back to the town at -150, -16) and the
Academy's (loader `0x08053508`, lists `0x080530C4`: the foyer's gate back
to the town at 92, -174, its way to the 1F hallway, the hallways' doors
to the classrooms two each and their stairs); BN6's machines in
AsterLand by its check dispatch (bn6f `sub_8034E88`, the same addresses
in Gregar: the Chip Traders' maps at `0x08034E74`, AsterLand's check
`0xF7`; the Number Trader's at `0x08034E80`, AsterLand alone, check
`0xF9`; at `0x08034E84` AsterLand's chip table, check `0xF8`, a text).

As built (issue #96): both taken over as Lan's house is (`indoors.c`),
each in a part of free space of its own, their doors out where the
planned town has them; their people and what they say are
`place_lines.c`'s, sprites of the game's people that Central Town's own
crowd does not use.

- **AsterLand** (`aster_land.c`): BN6's checks read as BN6 has them (the
  virus panel, the showcases, the magazines, the locked register). The
  request board opens BN6's own Request BBS, which holds none of BN6's
  requests in a run: the jobs (piece 5) are to post there. The Chip
  Trader trades from AsterLand's own list in the folder's codes, as the
  layers' traders do (theirs moved to the tables' second entry, Sky
  Town's, a map the run never visits). The **Number Trader is off**: its
  codes are no secret, and every run would take BN6's prizes from it, a
  power no run earned (variety, not power: docs/META.md); its check reads
  an "Out of order" sign (BN6's own taken off its map list). The clerk
  stands behind the counter where BN6 stands its own and points at the
  Chip Trader; a NetBattler waits at the empty board; a shopper gulps at
  the rare chips' prices.
- **The Cyber Academy** (`academy.c`): the foyer's way to the 1F hallway,
  its stairs up, the 2F hallway's two doors into class 6-1, and each way
  back; the other classrooms, the teachers' room and the principal's
  office shut (their warp-off flags held each frame). No class today, as
  Lan's classmate says in town: the NetBattle club meets in class 6-1 (a
  tip on matching codes, a Navi beaten by a Mettaur), a first grader
  waits in the 1F hallway. The classroom PCs and the foyer's jack-in do
  nothing.
- L in either names the way out; R has MegaMan point it and remind Lan
  his PC is at home. The map's label and the second screen name the
  place.

A's reach in the real world is BN6's own again: the probe ahead of Lan
that A reads checks along is the one the layers widen (24 units ahead
for navis on platforms, docs/ROM_DATA.md, Talking reach), and widened it
read past every check a cell deep, the request board, the Chip Trader
and all of Lan's room's. It is widened only in the Net now.

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
ports in town (#86, built, moved); Lan's HP as the warp zone, the act's
exit there and the town optional (#93, built); Central Town as home,
with Lan's room and PC (#94, built); going back and the Net's clock
(#95, built); a town that remembers (#87, built); AsterLand and the
Cyber Academy as places (#96, built); jobs (#88, built), at the BBS, from the townsfolk and the
classmates, with the objects' comps; the home shop at AsterLand's
counter (#89, built); time of day (#90, built, to be judged); the second screen (#91, built); the captures, docs and notes.
