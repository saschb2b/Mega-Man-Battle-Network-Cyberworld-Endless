# Home

How the real world takes part in a run: the town Lan comes home to after
every act, its ports, its people's jobs, its shop, and a town that
remembers. Reasoned with the game-design skill (the rework procedure, the
pattern catalog, the transitions budget) on 5 October 2026; the owner's
decisions are below. How the town is built: docs/OVERWORLD.md. What
carries over between runs: docs/META.md.

## Where it stands

A run's town (Central, ACDC, Seaside or Green Town) is the run's first
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

## The experience

**Home between dives.** The net is the frontier and the town is home: a
breath after each act, a place that has heard what MegaMan did down
there, and where Lan prepares the next dive with what only home has.

Not: a chore between acts, a second setup screen, a walk that taxes every
restart, a hub of menus.

## The dialectic

The run's own, **prepare or press on**, at home's scale: what Lan takes
on at home against what MegaMan finds below. Zenny spent at home now, or
carried to the net's dealers. A job taken, which bends the next act, or a
dive with nothing asked. The port at hand, or the other way.

## The loop stack

| Loop | Today | With home |
| --- | --- | --- |
| Progression (an act) | Ends on its guardian and "Which way?" | Ends at home: the ways are ports, the people have jobs, the shop has stock |
| Run | The town once, then only the net | Home after every act: danger between islands of safety |
| Meta | The town forgets | The town remembers the runs (recognition and variety, never power) |

## Patterns

- **The bonfire rest point** (Dark Souls, Hollow Knight): one safe stop
  per act, with a checkpoint; its spacing is the act.
- **The dual-life loop** (Moonlighter, Cult of the Lamb): the net
  supplies what home asks for, home gives the net its goals. The halves
  feed each other or one becomes a chore; home stays short against an
  act.
- **The draft with a skip** (Slay the Spire, Hades): a job from one of up
  to three people, or none.
- **The branching map** (Slay the Spire's map, Hades' doors): the next
  act's ways as lit ports.
- **Flavor as meta-narrative**, and its candidate the diegetic newspaper
  (Mina the Hollower): people and the news report the run.
- **The bonus with a drawback**: a job's constraint is its price.

## What BN6 has at home

Verified in bn6f (Falzar, its names; `~/.cache/mmbn-ref/bn6f`) unless
said otherwise, 5 October 2026:

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
  unless flag `0x16D0` + n is set (`sub_8034CB6`), each to its own comp:
  ACDC Town's squirrel statue and doghouse, Central Town's RoboDog, Lan's
  PC and bathroom, Seaside's fish stick fryer, water machine, aquarium
  and popcorn machine, Green Town's symbol, its TRUTH tablet, the Judge
  Tree's panel (destination table `byte_80984C8`).
- **Jacking out**: R on a net map asks "MegaMan, jack out?"
  (`CutsceneScriptAskJackOut_80988E4`); Lan stands again where he jacked
  in, and MegaMan's HP is made full at both the jack-in (`sub_8033FDC`)
  and the jack-out (`navi_80340F6`).
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

1. **Home after every act**, and a richer first visit: the run's start
   gets the jobs, the shop and a town that remembers too.
2. **The act's ways are lit ports in town**, not the Guardian Data's
   question.
3. **Home offers** jobs from the townsfolk, a town that remembers, a home
   shop, and time of day by act.

## The pieces

### 1. Home after every act

After an act's guardian the exit pad jacks MegaMan out (BN6's own
departure 8, as every exit pad plays it) and Lan stands in the town
beside its ports. The run is saved there, so a CONTINUE goes on at home.
The next act begins at a port. In the short net that is three visits,
then the Nest; in the endless net, one an act. BN6 makes MegaMan's HP
full at a jack-out; the guardian has already healed him, so home changes
no balance.

As built (issue #85): the exit's warp turns to the town by BN6's own
transition, internet to real world, which sets Lan down where he last
jacked in (docs/EMULATION.md); a run begun in the net (a headless
`--scene emu`) has him two cells short of the port, or at the town's
start where no walk on one floor joins them (Green Town). The act's AREA
CLEAR card shows over the town while Lan is held (A ends it, as on a
layer), then MegaMan and Lan say a word by how far the run has come
(`home_words.c`), and the run is saved ("From home" on a CONTINUE). The
short net's Nest (its win, and on threat 10 the way down to its second
guardian) and side layers lead on into the net as before; the endless
net comes home after its Nest too, "The Net's rebuilding".

### 2. The ports are the ways

The act's ways (two, or three with the dark way) are ports in town, each
lit and named: standing by one, MegaMan says where it leads and who
guards it ("This one goes to Green Area. CircusMan's there."), and the
second screen lists them. Walking to one is choosing it. The town's own
landmark is the first way; the others are BN6's own jack-in objects where
the town keeps them (ACDC Town's doghouse, Central Town's RoboDog,
Seaside's aquarium and fish stick fryer), and a check made a port where
it does not. The run's first act has one port, as today.

### 3. Jobs (the Request BBS)

At each visit up to three townsfolk have a job for the next act, and Lan
takes one, or none: BN6's own rule, one request at a time. A job asks
what an act can give, in one of a few kinds:

- **Bring**: a chip in a code, or of an element, handed over at home.
- **Busting**: delete a virus family with an element, or win a battle
  fast.
- **Find**: the act's Mystery Data of a colour, a ScrtData.
- **A vow** (a job's drawback is its price): beat the guardian with no
  Recovery chip, reach his door above half HP.

It pays at the next visit, from the one who asked: zenny, BugFrags, a
chip of the Library in the folder's codes, a program, a key item. Pay is
the run's only; a job remembered across runs is a line, not a bonus.

### 4. The home shop

A counter in town: AsterLand's in Central Town, and a clerk at the
town's own shop elsewhere (Higsby's opens for the run in ACDC Town). Its
**Order Service** is BN6's: a chip the Library holds, its code and price
named by the clerk, given at once; one order a visit, priced above the
net's dealers, so the net's stock stays the gamble and home the sure
thing. The town's **SubChip seller** stands beside it, as BN6 has one in
each town.

### 5. A town that remembers

- **The run**: people speak of the last guardian deleted, the act,
  MegaMan's HP, the Cross he brought, the threat. One line each, BN6's
  voice (docs/VOICE.md); returning players hear less.
- **The runs**: how the last run ended, the best layer, a guardian beaten
  often ("Back for SpoutMan again, Lan?").
- **Crowds that move**: people stand elsewhere at each visit, and more of
  them walk (sprites with all four walks, docs/OVERWORLD.md).
- **The news**: a board or the TV reports the net (the guardians deleted,
  the depth, Lan's name beside the best layer).

### 6. Time of day by act

Morning at the start, afternoon after act 1, evening after act 2, night
before the Nest: the run's progress on the town's sky. BN6 has no time of
day, so this is the engine's, and it leans on BN6's own effects first:
its towns' palette dim (Green and Sky Town's), its rain, snow and wind,
then a tint of the town's palettes where BN6 has none. Judged by captures
of every town at every hour before it ships; docs/FIDELITY.md names it.

### 7. The second screen at home

The PET at home (issue #79) lists the ways (port, area, guardian), the
job taken and how far along it is, and the visit's hour.

## What must not move home

- **Power across runs.** Pay is the run's; the profile keeps lines.
- **The setup screen.** Walking to pick a folder would tax every restart;
  the setup stays a screen.
- **Long chats.** One line a person, shorter for returning players.
- **A heal that matters.** Home heals as BN6's jack-out does, after a
  guardian who healed already.
- **Jobs to farm.** One job at a time, one visit's worth.

## Time

About 15 seconds for a player who walks straight to a port, 60 to 90 for
one who takes a job and shops: three to five minutes in a short net of
about an hour. The ports stand near where Lan arrives; nothing at home is
required.

## What could go wrong

- **A chore**: visits that feel mandatory and empty. The ports at hand,
  the jobs optional, the lines short.
- **The economy**: jobs and the shop add zenny, chips and items. Pacing
  checks every act's band with them (`build.py pacing`).
- **A dominant order**: the Order Service always buying the best chip.
  One order a visit, at a premium.
- **Saves**: a checkpoint in town rebuilds the town and the next act's
  first layer before it loads the state.
- **Fidelity**: time of day is not BN6's; BN6's own effects first.

## Phases

The epic's stories, in order: home after every act; the ports; a town
that remembers; jobs; the home shop; time of day; the second screen;
the captures, docs and notes.
