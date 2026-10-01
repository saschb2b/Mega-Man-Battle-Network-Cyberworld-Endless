# The loop's log

Newest last. One entry per report: score, what the session met, what it
raised, misreads, and one change to the loop. The persona's own files are in
`.build/play/kai/` (not committed): `diary.md`, `notes-sN.md`,
`report-sN.md`, `history*.txt`.

## Satisfaction

| Session | Score | What it met |
| --- | --- | --- |
| 1 | 4 | A soft-lock in the first five minutes |
| 2 | 5 | Reached layer 2; the first Net Dealer |
| 3 | 6 | A build plan, a heal to find, a close death |
| 4 | 7 | Fights won by thinking, loot kept, saves trusted |
| 5 | 8 | A build that paid off against a guardian; map and L trusted |
| 6 | 7 | A four-session run lost to a random pair after the guardian |
| 7 | 8 | Act 1 as wanted; calls spent on fights, not walls |
| 8 | 9 | All three wishes answered; guardian beaten by shop-prep-boss |
| 9 | 7 | EraseMan: a wall never dented, a move nobody warned of |
| 10 | 8 | The prep loop works; AquaSwrd one-shot BlastMan (too good) |
| 11 | 7 | CircusMan a real fight; the dealer's answer (SumnBlk1) a dud |
| 12 | 8 | BlastMan beaten with the dealer's BblStar1; R presses lost |
| 13 | 8 | HeatMan "the fairest, tensest boss yet", lost at 111/700; the R buffer works |
| 14 | 8 | BlastMan beaten with the dealer's AquaNdl2; 25 calls in circles on his layer |
| 15 | 9 | A whole act (Sky HP, SpoutMan) with no circles; every battle one virus pair |
| 16 | 7 | Act 3's fights varied at last; DustMan's two unannounced moves ended the run |
| 17 | 6 | Act 1 a joy, then DiveMan: his dive unannounced, the act's Elec plan fired into the water |
| 18 | 8 | DiveMan beaten with the warning and the dealer's chip; the draft a real choice |
| 19 | 8 | HeatMan lost on his last 29 HP, "the tensest fight yet"; 25 calls lost at a maze's V |

From session 5 the score swings 7 to 9: each session reaches ground no one
had tested (act 2's guardians, answer chips against them, traders) and finds
its problems there. The loop was reacting.

## Lessons so far

- **Pin the build.** Replaying a session on a newer build diverges. Each
  session runs its own copy (`scripts/pin.sh`), and replays name it with
  `CYBERWORLD_PLAY_BIN`.
- **Verify before fixing.** The persona sees stills. "Eaten" moves were the
  camera following MegaMan; an "eaten" A was a Sword swung into a hole.
  Replays with `CYBERWORLD_STATE_POS=1` settled both in minutes. Telling the
  persona in the patch notes ("From the developers, ...") and in
  `persona.md`'s harness notes stops the same report coming back.
- **Check "vanilla" in the disassembly.** Lost L/R presses near a full gauge
  were BN6's, and still worth a buffer. The Chip Trader giving BlastMan B
  every time was BN6's own rule (prizes the Library has, three in four) on a
  run's near-empty Library.
- **Sweep a class, not an instance.** EraseMan (s9) and CircusMan (s11)
  each surprised the persona with no warning; the other eight guardians had
  none either, found only when watched one by one (s13, `guardian_watch.sh`).
  The dealer's answer chip failed three ways over three sessions (a dud
  needing a hole, a sword that couldn't reach, a one-shot); the sweep of
  every act's answers per element (`build.py pacing`, s13) found three
  more at once: navi chips sold in twos, act 1's Elec answers over the
  cap, layers with none.
- **Budget the persona.** Sessions ran 320+ calls against 240: boss fights
  one action per call. The prompt now asks for bursts of two or three moves.
- **Where the engine writes a game record, carry every field the original
  has.** The battle record always put MegaMan on column 2 row 2; the user,
  playing, found him in a hole: the originals start him beside a hole or
  poison. Swept the record's other bytes after (only byte 1 varies, and
  ours is one of the originals'). And a record rewritten in place can
  change under a battle the game has already rolled: write in turn.
- **Watch the session, don't just wait for it.** In s14 the BlastMan fight
  ran 80 calls and over half an hour of wall time at 40 frames and eight
  pictures a call, past the call budget, while the loop did other work;
  the user noticed it first. Game time was normal (under three minutes):
  the pace was the harness's. The watchdog (`scripts/watch_session.py`)
  now wakes the loop every 15 minutes and on an alert, and the persona's
  prompt asks for 60-120 frames a call in battle.
- **Going in circles is measurable.** Kai's "the arrow led me round"
  (s14) looked like a misread until the layer was dumped with its walk
  (`CYBERWORLD_STATE_POS=map`) and the moment replayed: the arrow showed a
  turn's first panel, turned 30 frames late, and its eighths put a
  walkway's run 4 degrees from "left". A ROM-free test now follows the
  arrow from every room of 120 layers (the old arrow lost 22% of the
  walks); `scripts/follow_arrow.py` does the same in a session.
- **Count what the game counts.** AREA CLEAR's 11 viruses against Kai's
  12 led to a stale pointer: the battle's record was read before the game
  wrote it, and the run log wrote the rolled battle, not the fought one.
- **Sweep the next layers with the tools you just built.** Before s15's
  persona reached them, following the arrow on his exact next three
  layers (`follow_arrow.py`, the run's seed with `--net-biome` and
  `--guardian`) found it walking MegaMan into a Mystery Data: the session
  was restarted on the fix ten minutes in and confirmed it.
- **A network outage stalls the persona, not the game.** Twice in s15 the
  agent stalled; the game stays frozen between calls, so a SendMessage to
  the agent ("continue from frame F with play.py do") resumes it where it
  was. The watchdog's "no call for 10 minutes" is the signal.
- **Turn one report into a rate.** Kai's single unreachable Mystery Data
  (s16) was a Mystery Data one panel's gap behind his walkway, the gap
  hidden by its own floor's wall. A count over the test's 300 layers found
  15% of all Mystery Data, services and navis standing so; the fix brought
  it under 1%, and the count stays in the tests.
- **A carried-over run can miss the patch's headline.** Kai's s16 played an
  act-3 run begun before the NaviCust's draft existed, lost at its guardian
  and never saw the draft. Before a session, check that its patch notes'
  headline lies on ground the persona's save will reach; if not, say so in
  the notes (a new run, or which layer).
- **Check a patch note against the code before sending it.** S17's notes
  promised the gift anew after a layer restarts; a restart keeps the RAM
  and so the gift taken. The persona caught it in its first minutes and
  a correction had to follow mid-session.
- **A guardian's warning must cover when he cannot be hit.** DustMan's
  pull (s16) and DiveMan's dive (s17) were each watched once and still
  missing from their warnings; the act's weakness plan then became a trap
  (the Elec chip fired into the water). Re-watch every guardian against
  its warning for invulnerable spells, pulls and safe spots.
- **Save the report verbatim** the moment it arrives
  (`scripts/save_report.py`); the notes and diary are the persona's, the
  report is the developers'.

## Session 13 (8/10, keep playing: yes, new run started at once)

Build b24aaa3. Confirmed: the L/R buffer (4 times, the fight "flowed for
the first time"), "Back for more?", "my pick for the job", the Chip Trader
in L, a steadier arrow, and both developer notes (no eaten moves after the
map; the A after BATTLE START fires the chip at the bottom left).

Raised: the shops looked like the bystanders still (the s12 fix picked
sprites 64, 65 and 87, which are the dealer's green navi on the map: it was
never looked at, only its faces differed), no HeatMan warning, the trader's
three BlastMan B, services in walkway mouths, L's words against the arrow,
A turning to a vendor behind, the vendor's full chat every time, rows
misread in battle. All but the misreads were fixed before the report came
(875c7da, fa81690, d3f3265, 81e468d) or right after (3b13355, 5ecf688, the
vendor's repeat line); the misreads got a harness aid (b59dcc7: the battle
state names MegaMan's panel). Swept ahead: every guardian's warning, every
act's dealer answers (4c6bad0). The user, playing, found MegaMan starting a
battle in a hole (0d7d36b).

Cost: 257 calls, 82 minutes. Loop change: **a visual fix is verified in a
picture of the result**, not by its code: draw the sprites
(`build.py shot --scene gallery --sheet @6:56:48:/src/.build/l6.png`) or
capture the layer, and look.

## Session 14 (restarted)

Launched on 872ad88, whose new bystander and vendor sprites (Roll, GutsMan,
Glyde: compressed, unlisted) drew as white dots on the map, one of them
ElecMan's. The in-game capture after the launch caught it; the session was
stopped ten minutes in, Kai's profile put back from `data0` and relaunched
on 62a748e (EvilNavi bystanders, GirlNavi vendor, looked at on a layer).
Loop change: look **before** pinning. bn6f's `npcSpritePtrs` names list 6.

## Session 14 (8/10, keep playing: yes; recommend: yes)

Build 62a748e, 348 calls. Confirmed: the dealer's pick a Standard chip of
the named element (AquaNdl2 three times), A talks to the side MegaMan
faces, L says when the way winds, the battle state names MegaMan's panel,
the R buffer. BlastMan beaten at 20 HP with AquaNdl2 for the kill.

Raised: 25 calls going in circles on the guardian layer (the arrow: a
turn's first panel, late turns, screen eighths; fixed 52ec579 with a
follow-the-arrow test), every act 1 battle OldStove + Mettaur and BlastMan
the first guardian six runs running (c75757d: the Robot Control Comp waits
for act 2, BlastMan and DiveMan alike, a new run avoids the last one's),
AREA CLEAR's virus count one short (150d61d: a stale battle pointer), the
heal Mr. Prog's whole chat on an extra A (150d61d), BlastMan's warning
without his flame dash and fire wall (150d61d), the dealer silent on how
AquaNdl lands (d66ca3b). Possibly vanilla: CrakShot missing BlastMan in
front, hit right after BATTLE START. Found on the way: town folk block the
way to the port for the autopilot (seed 21; a task of its own).

Loop change: **reproduce navigation reports on the layer itself**
(`CYBERWORLD_STATE_POS=map`, `follow_arrow.py`), and look at every fix in a
picture before pinning (done: the heal repeat, the arrow, the dealer's
line).

## Session 15 (9/10, keep playing: yes; recommend: yes)

Build 506a757 (relaunched ten minutes in: the arrow walked into a Mystery
Data on his layer 4, found by sweeping ahead). Confirmed: the arrow on all
three layers (corner cuts, prompt turns, walkways, around Mystery Data and
navis), L's ways, the heal's one box, CONTINUE, SpoutMan's warning.
SpoutMan beaten with the dealer's DolThdr2 and act 1's BlastMan B.

Raised: every Sky HP battle Gunner and FgtrPlne (major; its own random
battle is one formation, as Green HP's: 190d0d0 shares the Sky's and the
Green Area's), a 1-damage buster against a lone back-row virus (the
NaviCust's draft answers it, 6c69ed6), HP+400 far cheaper per HP than an
HPMemory (4e970e6: the vendor stocks from the pool's tiers), A at a
Mystery Data going to a navi beside it, L's heal hint at 220 of 240, no
word on ScrtData, SpoutMan's whirl (all 4e970e6). AREA CLEAR's count is
left out after a CONTINUE by design (the next notes say so).

Meanwhile the user asked for a substantial NaviCust: designed with the
game-design skill (docs/NAVICUST.md) and built its first part (6c69ed6).

Loop change: **sweep the persona's exact next layers** with the run's
seed, and resume a stalled persona by message.

## Session 16 (7/10, keep playing: yes; recommend: yes)

Build 06994d0's parent (the NaviCust draft, the vendor's tiers, Sky HP's
battles). CONTINUE into act 3 (Judge Tree Comp): six fights, five
line-ups, BlastMan B's revenge on the corns, the dealer's ElecDrgn A
(a counter hit on DustMan). Deleted with DustMan at 550/900; a new run
started at once (DiveMan the first guardian, the gift's MegFldr1 taken to
test the PET, a bug built on purpose and named).

Raised: DustMan's thrown panels (their first hit stuns) and his pull-in
punch unannounced, a NaviCust bug named by its effect but not its cause,
the gift offering MegFldr1, the dealer naming Fire with no Fire chip in
stock, a Mystery Data that looked a step away and was a walk round (all
0da2ecb). DustMan's 900 HP sits in act 3's band (800-1000): left as
designed, with the warning's "keep a Recover chip ready". Vanilla: the
NaviCust's cursor after placing, the quit prompt's No. The draft, the
ExpMemry and the act-3 NaviCust vendor went untested.

Meanwhile: the title's NEW GAME question in BN6's chat box (06994d0),
pixel for pixel against a capture of the game's own.

Loop change: **turn one report into a rate** (the hidden gap: 15% of
objects), and **check the patch's headline is reachable** from the
persona's save.

## Session 17 (6/10, keep playing: yes, less eagerly; recommend: yes, with a warning)

Build f4e014b. CONTINUE into act 1 (RoboDog Comp): six quick fights,
three dealers all saying DiveMan can't stand Elec, ElcPuls1 from a
Mystery Data. Deleted with DiveMan at 450/500: every chip while he dove
did nothing, a torpedo read by its sprite (a row high) took the last 10
HP. A new run at once, its gift now a real choice (SuprArmr taken).
Confirmed: the title's question, the version, the area card fading under
a chat, the new gift, the dealers' Elec stock, no hidden gaps.

Raised: DiveMan's dive and his wave's safe column unannounced, the
torpedoes' shadows (57d3015), DiveMan the first guardian twice (57d3015),
100 HP at the first guardian (HP Memory 800z in act 1; left: the gift
offers two), Lan wedged by a hedge at a new town's start, exit pads warping
on a run-through, an A beside a Mystery Data (both left: BN6's own pads,
and the talk cone). The patch note on the gift was wrong (see above).

Meanwhile the user asked for the originals' props: the Net Dealer now
stands behind a counter cut from the ROM (f2a2cb4, docs/LEVEL_DESIGN.md).

Loop change: **check patch notes against the code**, and **re-watch every
guardian against its warning**.

## Session 18 (8/10, keep playing: yes; recommend: yes)

Build 7db7245 (the counters, DiveMan's warning). The act-1 arc worked end
to end: the warning read true, the dealer's Elec chip deleted DiveMan
mid-leap, the Guardian Data paid five HP Memory, DiveMan D and the
NaviCust draft (SlipRunr / Shield / Attack+1; Shield taken as the answer
to hits he can't dodge). Confirmed: the counter reads as a shop on layers
1 and 2, SuprArmr keeps his inputs through hits, the one-time CONTINUE.

Raised: NaviCust parts would not turn (BN6's own: rotation needs a key
item per colour; a run now holds all six, c3bc831), the dealer standing
bare in the room before the arena (the counter needs a rim there; left),
DiveMan's warning too strict (bombs reach him, c3bc831), OldStoves in four
of six act-1 fights (the area's own battles; left), Central's catwalk maze
reading as random floor. HeatMan, next, re-watched before the persona
meets him: his flamethrower and his leap onto a shadowed panel were
missing from his warning.

Loop change: **re-watch the persona's next guardian** before the session
(guardian_entry.sh with --guardian, then guardian_watch.sh), and check
every "vanilla" report for an item or setting BN6 hands out over its
story (the rotations were BN6's, missing from a run).

## Session 19 (8/10, keep playing: yes; recommend: yes)

Build af53461 (HeatMan's warning, the rotations). Act 2 in the Aquarium
Comp from a CONTINUE: the rotations worked (both ways, the overlap purple),
the dealer's tip for HeatMan read true ("its needles drop where he stood a
moment later"), DiveMan D from the last Guardian Data took HeatMan from
510 to 270, and he died on his last 29 HP after a minute at 20-40. Kai went
far past the budget (496 calls), 150 of them watching HeatMan in slices.

Raised: the leap's burst "burning an unlit panel" (the frames: an unlit
fire tower crawling in from column 3 reached his new row as he stepped off
the lit leap; the warning had said the towers run down lit panels, and now
says they crawl unlit and turn into our row), 25 calls lost at a V in the
water maze (the map now marks the way on over the floor he has seen), only
Piranhas and Quakers in seven fights (the Aquarium Comp's and the ACDC
HP's thin pools take one battle in three from their town's other area),
the ice freezing him unannounced (told in the area's first briefing), a
bystander on a platform's corner beside a walkway's end (a quarter of all
navis and services stood so; bystanders never do now). Left: the dealer's
list taking the A after its pitch and the NaviCust's quit prompt (BN6's
own), a summon fired into a jumping Quaker (vanilla), rotation not yet a
puzzle on a 4x4 board.

Meanwhile the user's net generator request: Central's comb, the plus
layout that had never built, the Graveyard's and the Undernet's cross
emblems, the Nest's and the Undernet's floors in their own stone.

Loop change: **read a death frame by frame before blaming the telegraph**
(the lit panels were honest; the unlit attack was another move), and
**watch a guardian's recording for what does not light up**: the warning
must name those moves too.


## Session 20 (8/10, keep playing: yes; recommend: yes)

Build of 08:46 (before 255e957; the dealer's list under "Welcome!", the
map's way marks). Act 1 from a CONTINUE in the RoboDog Comp to BlastMan
(the warning true for the rolling bombs and the flame dash; three dealers'
Aqua tips did 290 of his 400 HP), the Guardian Data (Attack+1 drafted and
installed), and into act 2's Aquarium HP, stopped on layer 4. 272 calls:
on budget for the first time in four sessions. The map's way marks were
checked ten times and no circle was walked.

Raised, all fixed in the next build: Gunners in six of seven act-1 fights
(a random battle sharing a family with the last is a third as likely,
8ea9a0e), a bystander beside a Mystery Data taking the A (bystanders stand
two panels from anything, 25719d2), the gift's menu taking the A meant for
its text (it waits half a second, 0bdcdec; its chip now always hits),
conveyor panels and the map's violet mark unexplained (the first briefing
names both, a951b6b), BlastMan the act-1 guardian again (BlastMan, DiveMan
and SpoutMan alike, dbf6ce6). Left: R presses before the gauge filled (the
persona's timing). "The homepage floor is one repeated tile" and "no
props" stood for the user's own report on the net's joins, which led to
the tile test (`build.py tiles`, 8e641eb, c305e47).

Loop change: **run `build.py tiles` before pinning and look over the
close-ups of the areas the persona's run is in and reaches next**: a
wedge of one floor in another is found there in minutes, never by a
persona who reads it as the area's look.

## Session 21 (8/10, keep playing: yes; recommend: yes)

Build c305e47 (the clean joins, the tile test). Act 2's Aquarium HP from a
CONTINUE that rebuilt layer 4, through layers 5 and 6 to CircusMan's
door, where the budget ran out (282 calls, the first pace note sent at
267). The joins held: "the floors look designed now", about 60 map
pictures on layers 4-6 without a wedge, a cut edge or a stray colour; the
dealers' picks, CircusMan's warning and the violet mark's line confirmed.

Raised, all fixed before the next pin: the Server's prize never named
(its talk ran unheld, and the A paging it talked to the Server, whose own
line took the box: 9899261, the prize a dozen rolls a tier richer), the
same virus family back to back (b033697: none of the last battle's
families where another fits), ice unmentioned in the Aquarium HP
(66e987c), a Chip Trader in a walkway's mouth (the legalizer had changed
a cell beside him after placement, 8 of 128 atlas layers: 2626d94). Left:
FighterPlane fights long (vanilla), A reaching a Mystery Data only
square on (vanilla), the exit pad's rim (vanilla collision). Misread: his
charged shots did 1 damage because a charge takes 100 frames at the
start, not 64 (from the disassembly's `powerAttackChargeTimes`; now in
persona.md). Meanwhile the user's own look found the framed pads' upper
edges stepped and their catwalk joins cut, from a pad-stamp change the
tile test could not see (it counted the picks under the stamp): fixed in
9d1ef07, and the metrics now count the map as drawn.

Loop change: **every pasted piece (pads, emblems, stairs) gets a look in
the game before a pin**: the tile test sees the classes' picks, not what
is set whole over them, and a regression there reached the user first.

## Session 22 (9/10, keep playing: yes; recommend: yes)

Build f7bdc91 (the Server's prize named, the charge time in the persona's
notes). From a CONTINUE that rebuilt layer 6 of the Aquarium HP, through
CircusMan (0:56, 39 HP left) to act 3's first layer (270 calls).
CircusMan's warning held line by line and the new tent tell was dodged
twice; the dealer's DolThdr2 P did 300 in one hit: "the best boss fight
of all 22 sessions".

Raised, fixed before the next pin: the Server's viruses back in the very
next fight (the next battle is rolled again every five seconds, and each
roll took the last battle's place in memory: 34 of 52 back-to-back
battles in the persona's run log shared a family; now only a fought
battle counts, 29199b9), early L or R lost (a 30-frame window, and a
press swallowed while MegaMan was hit: kept 50 frames and pressed again
until the Custom screen opens, 29199b9), a dealer in line with a walkway
(350 of 1166 services stood on such a line; now 19), the act's arrival
words eating an L, an eight-page briefing, the dealer's TankCan3 against
a hopping guardian, a Server's prize in a code that fit nothing. Fixed
after the pin already: the battle memory across CONTINUE and the prize
from a deeper pool (de5821e). Left: the tent's drain (vanilla), the exit
pad's rim (vanilla collision), "RUN... None" (the NaviCust's own words).
Also found: `CYBERWORLD_AUTOPILOT=1` is a blind button rhythm that loses
its first battle; `weak` is the run-through check.

Loop change: **count before judging a report either way**: the
persona's `runlog.txt` answered "does the no-repeat rule hold" across 52
battles in a second, where one report could only say "it failed once";
the unit tests now count services in line with a walkway the same way.

## Session 23 (8/10, keep playing: yes; recommend: yes)

Build d96aa0c (services off the ways across, the one-breath briefing).
From a CONTINUE that restarted layer 7 through act 3 on the Sky HP to
ChargeMan, who ended the five-session run at 300 of his 1000 HP (337
calls: the budget ran out at 278, mid-fight). The dealer's WideSht hit
him for 200, the Server paid AuraHed3 F, Mystery Data a Recov300 J: "the
richest act yet". Confirmed: no repeat after a Server, the kept early R
(25-50 frames), the shorter briefing, the dealer's Aqua pick.

Raised, fixed before the next pin: early R lost at 55-60 frames (the
gauge's last tenth takes a second: kept 90 now, e0c66bc), a kept R that
opened the Custom screen over the UP stepping off a lit panel (a d-pad
press drops it), the guardian unnamed in the briefing after a CONTINUE
(the arrival's words are not said again, e0c66bc), the dark warp
unexplained (2e7a43d), the Server's mark left on the map (2e7a43d),
ChargeMan's warning silent on when to hit him and on his cars blocking
chips, the Server's pair two fights later and planes in three of five Sky
HP fights (the last two battles' families kept clear), Recovery Progs
beside the way and a panel's gap on it (placement keeps off the way from
arrival to exit). Left: ChargeMan's tall sprites read a row high (vanilla;
the persona's notes say to read rows by the HP number), the Yes/No
cursor's first frames (vanilla), a NaviCust program that won't fit
(vanilla shops; the board grows at act 4), the vendor's shop portrait
(compressed keeper text).

Loop change: **an input buffer ships with its cancel rule**: the Custom
press went from 30 to 50 to 90 frames and gained retries across two
sessions before the rule that the latest intent wins, which was what made
the longer window safe. Ask what a kept input gives way to before tuning
how long it is kept.

## Session 24 (6/10, keep playing: yes, with a weaker pull; recommend: yes)

Build a119fef. Three new runs after the long run's loss: a layer 2 Server
(OldHeatr and Shaker, hits capped at 80 against MegaMan's 100) and then
Armadill with Gunner ended the first; a Quaker pair on a poison field the
second, on layer 3; the third stopped at the budget. Every patch held: the
early R kept at 50 to 75 frames and dropped by a d-pad press, no family
twice in three battles, no navi in the way on five layers, the Server's
mark gone, L's briefing after the arrival's words.

Raised: a restart repeats (the same town, comp, guardian and chats in the
first twenty minutes, a chore after a long run), which the owner answered
with the meta layer (docs/META.md); act 1's Server hitting for most of
MegaMan's HP, and asking without saying how strong it is. Left: the town's
arrow through a house, landing shadows read by row (vanilla).

Loop change: **after a run ends, the next session plays the restart**: a
returning player judges the first twenty minutes differently from a
first-timer, and this session's 6/10 came from there, not from a bug.

## Session 25 (8/10, keep playing: yes; recommend: yes)

Build df6aeaa, the meta layer's phase one: the setup after NEW GAME, the
short net, helpers. A new run with HP+ only; act 1 in RoboDog Comp under
DiveMan, new to the persona, beaten with 10 HP left (his warning held
true: the back column, the torpedoes' shadows), then UnderSht from the
draft (294 calls). Confirmed: one-page arrival words and gift greeting
for a returning player, the setup and its lines, HP+ through the gift,
no family twice in three battles, the kept early R.

Raised: the setup offers nothing but Help before a first win (the locked
rows read as goals, Storm unlisted), Help has no cost, the same town
street and RoboDog Comp again, a lone ice Piranha as the first fight
(fixed after the pin, bfbdb02), act 1's three families turning in a fixed
rotation, DiveMan's "only bombs reach him" without how to aim them, a
dealer's list that shifts when a row sells out, a bystander beside a heal
Prog, a Chip Trader in a walkway's mouth. Vanilla: "got" boxes taking two
A's, Quakers choosing their row in mid-jump.

Loop change: **a session launched right after a report continues the
run while its fixes are made**: s26 plays acts 2, 3 and the Nest on the
same build, so the first win, its summary and the marks get a player's
eyes while s25's fixes land for s27.

## Session 26 (8/10, keep playing: yes; recommend: yes)

Build ffd2921. CONTINUE of session 25's short-net run: act 2 in the Sky HP
under SpoutMan (600 HP, won from 280 to 80, ElcPuls3 the dealer's answer
for 281), HP+100 from the draft onto a 5x4 board, then act 3's first
layer in CopyBot's Comp (258 calls). Act 2 took 9:27 against act 1's
12.6 minutes: the short net fits a 45 to 50 minute sitting. Confirmed:
the footer line, true warnings, the kept early R at 65-70 frames.

Raised, fixed before the next pin: a Guardian Data program never installed
(major: the draft said how only on a profile's first; the board's RAM is
now read, docs/ROM_DATA.md, and L names a program left off it), a Net
Dealer's counter beside a heal Prog taking the A (placement now keeps
talkers apart, a test counts them), the dealer's greeting on every layer
of an act, the act card naming a deleted guardian after a CONTINUE.
Left: a sold-out row vanishing from the dealer's list and the Chip
Trader's list shifting (BN6's shop menus), the NaviCust's own chores,
services spread unevenly (six on one layer, two on others), a guardian's
chip on the result screen and again in the Guardian Data talk (the battle
reward's RAM is not yet known). Misread: SpoutMan's "You made waves last
time" (rivals.sav: MegaMan deleted him in an earlier run; DiveMan, "new
to me" in session 25, had been met three times).

Loop change: **a persona's "never met" is checked against the save**: its
memory is its diary, which does not reach back 26 sessions; the profile's
rivals.sav answered both greetings in a minute.

## Session 27 (8/10, keep playing: yes; recommend: yes)

Build 16264f3. CONTINUE of the short-net run to its end: act 3 in the
Seaside comps under SlashMan (about 600 HP, deleted with LilBolr2 on 22
HP, SlashCross from his Guardian Data), then the Cybeast Nest on layer 10
for the first time, where EraseMan EX deleted MegaMan with 720 of his 1200
HP left (about 383 calls, 120 over the budget to finish the fight).
Confirmed: L naming an unplaced program, the install line in the Guardian
Data talk, the dealer's one-line greeting within an act, dealer and heal
apart, the one-time CONTINUE with the right act card, the setup's Blade
and Storm lines. The Nest read as an ending: its arrival words and
EraseMan's line made the fight personal.

Raised, fixed: ScrtData unexplained at pickup, then repeated by L on every
layer (MegaMan now says what it is for as it is picked up; L counts them
only when the count changed), the summary announcing Blade and Storm as
this run's unlocks (earned in a run that never reached a summary; a new
run now marks earlier unlocks known), SlashMan's warning silent on his
stuck blades, EraseMan's on his ghosts (both lines rewritten).
Left, for the game-design skill: the final fight's dead hands (every
reward in its own code diluted the folder: four or five codes a hand),
EraseMan EX's ghosts as unreadable damage (BN6's own attacks; the warning
now says to dodge between rows and heal first), two DarkMechs in one
act-3 random battle (480 to 20 HP), a Guardian Data program left off the
board after RUN (wish: place it itself or ask), ScarCrow healing on Elec
with no hint, the layer-8 maze of parallel walkways, the dealer's list
taking the A meant for his last box, a bystander by the Nest's heal.

Loop change: **sweep the finale before the persona reaches it**: session
27 was the first to meet the Nest, and every Nest problem (the guardian's
unwarned attacks, the heal skipped, dead hands) surfaced there at once;
the next sweep plays each Nest guardian's first minute with the autopilot
and a folder of the run's rewards, and checks the warning against it.

## Session 28 (8/10, keep playing: yes; recommend: yes)

Build 189b23d. A NEW GAME on the short net with the Blade folder: act 1
in the RoboDog Comp, BlastMan deleted (0:44, 160 to 100 HP), saved on
layer 4 in the Aquarium Comp under CircusMan (269 calls, the budget spent
after a 30-call guardian fight). "The best start the game has given me":
LifeSword fired on layer 1, and every hand held only S, L and *.
Confirmed: the dealer's one-line greeting within an act, the install
lines in the gift's and the Guardian Data's talks, Blade and Storm in the
setup, BlastMan's warning line by line.

Raised, fixed: a Guardian Data draft offering SuprArmr beside the gift's
Custom1 on the 4x4 board, where the two cannot fit (the draft now fits
its programs and colours beside the board's, a packing solver over the
ROM's shapes; a program that cannot fit is named once a board size),
rewards off the folder's codes (the dealers' and the gift's chips lean to
the folder's, Mystery Data half the time, a V1 Navi chip comes in *),
the same town, comp and guardian four runs running (a third opening area;
a new run avoids the last one's town, first area and first guardian),
the exit pad's side rim (its trigger now whole but the far corner), an
early R dropped (kept 2.5 s, not 1.5). Built meanwhile, from the roadmap:
Cross starts, the Library across runs, the route choice after a guardian.
Left as BN6's own: "Quit programming?" defaulting to No before RUN,
AreaGrab's banner eating the next inputs, the chip cursor's wrap, battle
drops' codes (BN6's reward tables).

Misread: the dropped R "about 40 frames early" was pressed 126 frames
before the gauge filled (history.txt: call 212 at 30457). Check a timing
complaint's frames in the history before touching the code.

Loop change: **sweep a run's opening for sameness across the persona's
last runs**. Every run plays the town, act 1's area and its guardian;
their repeats (Central Town, the RoboDog Comp and BlastMan four runs
running) showed only in the diary, never in one session's notes. Before
each launch, read the last few runs' openings in `diary.md` and count
them against the pools.

## Session 29 (7/10, keep playing: yes; recommend: yes)

Build e667169. A CONTINUE of session 28's Blade run through the Aquarium
Comp: layers 4 and 5, stopped on layer 6 short of CircusMan (249 calls).
"The fights are the best they've been": three LifeSwords on layer 4, two
of them double deletes at Busting S, and the dealer's Elec tip won the
next fight in 1.4 seconds. Confirmed: an unfittable program named once,
the pad's side entry (2 of 2), the early R kept (5 of 5 within the
window), CONTINUE keeping the run, the Library carried over, the shop list
opening past the greeting's A presses.

Raised, fixed: the mazes ate the session (110 of 249 calls walking or on
the map): counted over 60 layers per area, the Aquarium's and Judge
Tree's catwalk mazes took 16 legs to the exit against 8 elsewhere, and
eleven Aquarium guardians' layers in twelve had fallen back to a plain
route for want of arena room. Their corridors now run straight on and
more walls are knocked through (8 legs), a guardian's layer gets a
smaller maze with its arena, and the tests check every area's walk; the
way-on arrow stays while MegaMan walks. The dealer's ElcPuls3 S at 2000
beside his A at 700 was fixed during the session (a chip once, whatever
its code). His wish for rewards in the folder's codes is now whole:
battle drops lean half the time (BN6's reward rows, found in romlab).
Built meanwhile, from the roadmap: programs found come back at later
vendors, threat rung 10, collector's vaults.

Left: platform rims sliding MegaMan along (BN6's movement: UP runs a grid
diagonal, the walkways' axes want UP+RIGHT and the like).

Loop change: **the watchdog counts the calls spent on the map**. The
session's walking showed in its notes but only the report said how much.
`watch_session.py` now prints this session's share of calls on the map
(past sessions 9 to 60%, most 20 to 40) and says "look:" past 80 calls
with more than 45% on it; a look then asks whether the persona is lost or
the layer long, and the layer's walk is counted before the report comes.

## Session 30 (7/10, keep playing: yes; recommend: yes)

Build 7985687. A CONTINUE of the Blade run on its rebuilt layer 6 (the
Aquarium Comp's smaller maze), CircusMan twice, deleted in the rematch at
his 290 of 700; the summary opened the SlashCross start, and a new run
began (243 calls, 10 to 13% of them on the map, against 46% before).
"The rematch LifeSword is why I play": AreaGrab into LifeSword, 410 in one
swing. Confirmed: the arrow on while walking, the dealer and the heal
beside the guardian's arena, a chip listed once, the code lean in the
dealer's stock and the gift, the new run's other town, area and guardian,
the gift's install line, the quit prompt's Yes.

Raised, fixed:
- **Running from a guardian put MegaMan back on his trigger**, twice at 5
  HP, and the rematch ended the run. BN6 keeps its story bosses from
  running: bit 0x20 of the battle record's options, which the engine set
  for every battle. Guardians now clear it: "Lan, this is no time to run
  away!"
- **Blank chips in a new run's Blade folder**, one or two in every hand:
  BN6 marks each chip it gives (a key byte XOR 0x17 in a per-chip table)
  and draws an unmarked chip blank, as a cheat's. The engine wrote the
  folder without the marks; the Standard folder's chips, given at NEW
  GAME, played. Session 28's Blade run played because the player opened
  the folder in the PET first. The folder's chips are now marked at the
  start, on every fresh layer and on CONTINUE.
- **The setup after an unlock** opened on JACK IN!, the new Cross unseen:
  it now opens on the new option's row, marked NEW.
- **The briefing's arrow faded** before the last box closed: it stays ten
  seconds after the words.

Misread, twice now: "early R fails" (0 of 6). Replayed without the second
R, the Custom screen slid in 10 to 20 frames after the gauge filled; the
second R came first. The persona's controls now say to wait 30 frames
past the full gauge.

Left: overshooting the arrow's turns in 20-30 frame bursts (the arrow
turns at the junction), the gift Prog's talk reopened by the A that
closed MegaMan's line, a lone Gunner at the back against a Blade folder
(its identity: the dealer's pick reaches).

Loop change: **bisect a regression's start path before its code.** The
blank chips took a long hunt because every "clean" control hand happened
to hold only Standard chips. A deterministic probe (a test folder of only
the suspect chips, an env switch the test sets) settled in one run what
three chance hands had not. For a bug that shows in some hands or some
runs, make the probe force it.

## Session 31: 8/10

Build 5053ea4. A NEW GAME with the Blade folder, the SlashCross start and
HP+: Central Town, act 1 (RoboDog Comp, SpoutMan deleted at 60 of 140 HP
by six charged SlashCross slashes), stopped on act 2's first layer (269
calls). "The cleanest start of any run": whole hands, a Cross that
answers the Blade folder's "nothing reaches the back", act 1 in ten
minutes. Keep playing: yes (HeatMan, whom he has never fought, and his
weakness in hand). Recommend: yes.

Confirmed: the folder's chips whole in every hand, the arrow kept after
L's words and while walking, an early R (2 of 2 without a dodge), the
setup's Cross row, the vendor naming a program used before, the Guardian
Data's Navi chip in *, the corner slide.

Raised, fixed:
- **A guardian's battle chip in A** to a folder of S, L and *: the Navi
  rule covered the second entry of each drop pair, BN6's coin picks
  either. Both now.
- **An early R dropped by a dodge**: any d-pad press dropped the kept
  press. It is kept now, and waits for the step.
- **An exit pad's rim that did not warp**, the fourth report, each fixed
  on one side: the trigger is round now, 26 units, tested from nine
  placements round a pad.
- **The last stop passed by**: the arrow led past the heal and the dealer
  to SpoutMan with 1150z unspent. MegaMan names them, and which way each
  is, stepping into the room before the arena.
- **Finds in dead codes** (2 of 7 Mystery Data, the trader's TrplShot V):
  half the Mystery Data roll again for a chip in the folder's codes
  (smart loot, 37% to 65% fitting for a Blade folder), and a Chip Trader's
  prize comes in the folder's code or * (the fusion that turns the rest
  into play).

Fixed after the pin: the arrow over the jack-in's flash, the gift Prog's
re-talk, the arrow's wobble, the same opening area again.

Misread: "R beside the statue did nothing". Replayed: the first R jacked
in; BN6 shows Lan's "Jack in!" line, which closes by itself, then Lan
raises the PET, about 220 frames to the flash, and the picture 90 frames
after R fell between the two. Vanilla: AreaGrab's pause, the Cross chosen
per battle.

Left: the arrow's next turn shown before the junction (his first wish;
the arrow turns 45 degrees a panel before a corner already, and a bend
drawn at every zigzag of an open room would mislead).

Cost: the report plus about three hours of fixes; a third town (Seaside)
landed meanwhile.

Loop change: **fix a spatial trigger's class, not its side.** The pad rim
came back four times, each fix covering the side a playtester stood on.
For a trigger, a wall or a talk radius, place MegaMan at the eight
compass offsets round it (the `place` step) and at two distances, and
check each, before calling it fixed.

## Session 32: 9/10, the stop criterion met

Build b617cc0. A CONTINUE of the Blade + SlashCross run: act 2 (Judge
Tree Comp) and HeatMan, lost to twice in earlier runs, deleted in 0:39 at
160 of 240 HP "to a plan the game helped me make": the dealer's hint and
his pick in the folder's code (BblStar3 S), MiniEnrg, the last-stop words,
two AreaGrabs and charged slashes (277 calls, over budget: the agent hit
an API limit mid-fight and was resumed). Keep playing: yes, for a run he
cares about ("the net splits after act 2... I hold HeatMan *"). Recommend:
yes, with play tips only (short bursts behind the arrow, swing at
warpers when they land).

Confirmed: the layer restart on a changed make, the guardian's drop in *
(HeatMan *), no running from a guardian, the last-stop words, an early R
kept through a dodge (twice), the exit pad from the ring, every find in
the folder's codes but the signal's prize, the arrow's turns in short
bursts (one overshoot all session), A after a chat not re-talking.

Raised, fixed:
- **A risky choice defaulting to Yes** (the strong virus signal, 150 HP)
  and **its prize off the folder's codes** (HeatManEX H): both fixed
  after the pin, before the report (Yes/No starts on No for fights and
  warps; the prize the strongest the folder can play).
- **At low HP, no word of the dealer** on a layer with no heal Prog: L's
  later answers name the dealer's MiniEnrg and his way.
- **The Judge Tree's "terraces"**: its walkways (brick blocks) ran one
  empty panel apart, which the isometric view draws as levels; 43-45% of
  the Aquarium's and the Judge Tree's floor faced floor across one panel,
  against 1-9% elsewhere. Their mazes keep two panels apart now (3%).

Corrected mid-session: the patch notes had told him SlashCross was weak
to Fire; BN6's own tutorial says Breaker. The setup's Cross row and a
won Cross now say each Cross's weakness.

Left: a timing hint for swords against warping guardians (BN6's AI;
unverified per guardian, so not written), the pad's decorative corner
cap (outside the ring, as intended), act 2's regular viruses gentle for
a strong build (the threat rungs are the answer).

Stop criterion (SKILL.md): 9 or more (9), wants to keep playing for his
own reasons (yes), recommends without a major caveat (yes), two sessions
in a row with nothing major (31 and 32: minor and polish only). Met. The
town screenshots were refreshed; the run's shots and clips still show
the game as it is.

Loop change: **count a class of confusion before designing its fix.**
"Couldn't see a way down" suggested stairs on the map; the layer had no
raised floor at all. One throwaway count (floor facing floor across one
empty panel, per area) named the two outlier areas at 43-45% against
1-9%, sized the fix and kept it as the measure (3% after).

## Session 33: 9/10 (keep playing: yes; recommend: yes)

Continued the Blade run from layer 6 (rebuilt by the LAYER_MAKE bump):
HeatMan again, "the best fight of the series" (the new timing line
turned into a plan and the finishing LongSwrd), the way on to ChargeMan,
act 3's first two layers on the ACDC HP. Stopped at the budget on layer
8, ChargeMan and the Nest next.

Confirmed: the layer restart, the Judge Tree's two-panel maze, HeatMan's
"strike then" timing, the guardian's drop in *, finds in the folder's
codes, the last-stop words.

Raised, fixed:
- **Act 3's battles repeat**: three of four were the same Catack pair.
  `build.py pacing` counted it: Catacks in 203 of 300 ACDC HP battles at
  depth 7 (its own three kinds fill the band only with them). Sharing
  two in three with Central Area, as the Sky and Green homepages do,
  brought it to 106.
- **The dealer's pick off the folder's codes** (WideSht Q beside S and
  *): a fitting answer counts a quarter harder. Preferred outright, the
  pacing report showed act 1's picks falling from 120 to Cannon's 40;
  the quarter keeps the element answers as they were.
- **Chip Trader prizes off-code** (SumnBlk2 H, GunDelS2 E): the pool's
  list holds only chips in the folder's codes or * now.
- **The heal's way told two ways**: fixed after the pin (fa5022c).
- **"When is it safe to stop?"**: "Run saved" in the corner at each
  checkpoint; the quit prompt names the checkpoint.

Misreads, BN6's own: A presses during the charged slash's follow-through,
the draft's "second A" (replayed: the press landed as the page finished
typing), "Sending chip data..." on OK before it is pressed.

The owner, reading the iteration's own work: the rumor briefing had
MegaMan tell what he could not know (EraseMan's name, a rumor), and the
first rework still let him sense a guardian's element (Aqua is SpoutMan
by name). The rule now: MegaMan knows a guardian from battle data, or
as hearsay from a Navi on the net; the card says "???". The persona now
reads the game as a story too (persona.md) and reports an immersion
category.

Loop change: **for every line a character says, ask who says it and how
they could know, and follow the same fact through every surface** (the
act card, L, the arrival and last-stop words, the way on, the state line
the persona reads). The owner found two slips in one fact that a
line-by-line fix missed; a fact is told in six places.

## Session 34: 8/10 (keep playing: yes; recommend: yes)

Continued the Blade run from layer 8 (restarted by LAYER_MAKE 52):
dealer picks in his codes, act 3's battles from Central Area at last,
"Run saved" at each arrival, ChargeMan briefed "from battle data" and
remembering the last fight. Lost to ChargeMan on layer 9 at 370 of
1000, the second time; a new run at once (Storm folder, the HeatCross
start the loss unlocked, Seaside Town), stopped on its layer 2.

Confirmed: the layer restart, dealer picks in the folder's codes (two
of three; the third had no answer in them), ACDC HP's variety, the
saved note, knowledge for known guardians (named, briefed, no rumor),
the timing lines, the last-stop words, directions held 10+ frames.

Raised, fixed:
- **Where to stand as ChargeMan's train passes** (the loss): watched in
  god mode, the cars roll down the other two rows a column or two
  behind him, never down his; his warning says so now.
- **The timing again at the arena** (wish 1): a known guardian's arena
  talk ends on "Remember our battle data, Lan: ...".
- **No new guardians for a veteran** (wish 2): eight of seventeen met in
  34 sessions; new runs prefer the never-met (two in acts 3-4 of every
  sample run on his profile).
- The vendor bringing an installed program; a bystander's dark warps
  against the sealed way; the gift's "2 HPMemory" before "HPMemory x2".

By design: no rumor for a known guardian; the arrow's half minute.

Beside the loop, from the owner: BN6's battlefield objects (rocks,
cubes) and the rare green Mystery Data were dropped; restored, the gem
at one battle in forty with a blue Mystery Data's reward, BN6's own
second reward on the results screen, and MegaMan explaining it after the
first.

Loop change: **when a run is lost to a guardian, watch that guardian in
god mode before the next session** (guardian_watch.sh): the loss names
what its warning lacks, and the watch checks the persona's own theory
in minutes (ChargeMan's cars, confirmed from four sheets). And a rule
for the harness: a player-honest state (the ??? guardian) must not blind
the dev scripts; they read it under CYBERWORLD_STATE_POS.

## Session 35: 9/10 (keep playing: yes; recommend: yes)

A new run (Storm folder, the HeatCross start the last loss unlocked, HP+)
from its layer 2 through DiveMan, "the best first-act guardian fight of
the series": the dealer's "word is" and an Elec pick in his code, the
arena's reminder ("he's only open when he surfaces") and a Thunder ball
he found waits on the water for a surfacing boss. Stopped on act 2's
first layer; the session was cut by an API limit and resumed.

Confirmed: the pre-battle reminder, the battlefield Mystery Data and its
second reward, dealer picks in his codes, the draft's green arrow, "Run
saved", the early R, the arrow at junctions.

Raised, fixed: the gem's find read as none (980 zenny, a blue Mystery
Data's): three in four now chips a tier above, in the folder's codes.

Kept as designed: the gem explained after the first battle that held one,
not before (discovery first, the owner's rule); no rocks in Seaside and
Central battles (their originals hold few); CONTINUE mid-layer where he
quit on the map (the quit saves there). BN6's own: a START right after a
shop, lost as the map reloads.

Beside the loop, from the owner: the PET's five entries (Comm's SciLab
link, Save, PLACE, Dad's battle-data mail, the profile's key items).

## Session 36: 8/10 (keep playing: yes; recommend: yes)

CONTINUE of the Storm/HeatCross run on the PET build: layers 4 to 9,
SpoutMan beaten on a plan (the dealer's Elec word, MegaMan's reminder, a
Thunder ball left as a mine), act 3 entered against a never-met guardian
("I don't recognize it" / "Then let's find out who", the session's best
moment). Stopped on the budget one layer short of ElementMan's arena.

Confirmed: the PET's entries (Save, PLACE, Dad's mails, KeyItem; Comm's
SciLab link, since moved into two mails at the owner's word), CONTINUE
where he quit, the arena's reminder, the early R, "Run saved".

Raised, fixed after the pinned build: the dealer's pick a chip the
folder was full of (3747852); the mail's band (91993f8); NaviCode's
"and more". Raised, fixed now: the first dealer of an act naming the
"???" guardian (the word comes from the act's second layer); SpoutMan's
HeelNavi body unexplained at a rematch and his lines in the bystanders'
face (named at every rematch, no face); "I'm ready this time!" at 3-0.

By design, or BN6's own: act 3's fights without bite (he runs HP+ and
took HP+100: 500 HP against a band sized for 300); the NaviCust's two
quit prompts (both in BN6's text); the battlefield Mystery Data drawn
above its panel, read by its shadow; input lost as a chat closes. Kept
for later: a hint per unknown Navi at the fork (the area's character,
which MegaMan may know).

Loop change: **let a session end at a payoff, not on the budget.** Told
to stop at 268 calls one layer short of a first meeting, he scored the
session as "the calm middle of a run": the budget now stretches by up
to 40 calls to finish the act's guardian when his layer is reached.

## Session 37: 9/10 (keep playing: yes; recommend: yes)

CONTINUE on layer 9 (rebuilt afresh by LAYER_MAKE 53), then ElementMan
at a first meeting: "I don't recognize it", Dad's "We don't know who
yet", the dealer's word one room before the arena, the reveal ("That
voice... it's ElementMan! But that's a HeelNavi's body!"), and a fight
won at 80 of 500 HP by reading his colours: "no weak element" turned out
to mean a weakness per form, which he found himself. "The best story
moment of the series." Stopped on the exit pad to the Nest, the budget
spent on the fight (the new 40-call extension used as meant).

Confirmed: Comm grey, the Dive report and Records mails, NaviCode's
count, the dealer's pick fitting the folder (no capped chip), the dealer's
word from the act's second layer, the HeelNavi guardian's first-meeting
words, the layer rebuilt afresh.

Raised, for after 0.3.0: ElementMan's HP falling by 1 every ~25 frames
with nothing hitting him (no poison in our fields: BN6's own battle, to
confirm in god mode); one attack (a shadow sliding across his panels)
with no yellow panel; batched A lands on the shop's "Are you sure? >
Yes" (BN6's default), twice; Dad's mails never NEW, ", code" terse; the
vault's count moving (30 to 60 by act) with nothing saying what a vault
is; a strong virus signal's prize a third copy of a chip just bought; a
bystander reciting button names.

Loop change: **the budget's guardian extension works**: the session
ended on the payoff it came for, where session 36 had ended one layer
short of it. Keep it.

Loop change, from the owner: **the loop does not stop for other work.**
After session 37 the release came first and the next session waited an
hour, until the owner said so. Triage and relaunch come right after a
report; a release, a feature or a question runs beside the loop, while
the persona plays.

## Session 38: 9/10 (keep playing: yes; recommend: yes)

CONTINUE on layer 9 (rebuilt afresh), then the Nest and EraseMan EX, the
Navi who ended session 27's run: won at 30 of 620 HP on a plan the
briefing and the dealer's pick gave ("strike then, with something that
reaches him": ElcPuls2 pulled him in). The first won run in 38 sessions.
Stopped at the setup (Endless, EraseCross, threat 1), the budget spent.

Confirmed: the layer rebuilt with the beaten guardian kept, Dad's mails
NEW (Records, the new guardian's mail, the Dive report on the Nest), the
Records' "code" legend, B backing out of the shop's Yes.

Raised: the ending thin (rewards and a program pick after the final
fight, the install nag on the way out, Dad's mails stale after the win,
two of four unlocks on the summary, the growl never answered; fixed in
a0d22cb); the last layer's dealer "tougher from here" (fixed); sold-out
rows shifting the shop list (open: check BN6's own shops); a guardian's
HP falling by 1 every ~25 frames, now on EraseMan too (BN6's own, below).

Misreads: DemonEye's beam a row up (BN6 draws it floating), the ElmntMan
chip's words (BN6's own).

Vanilla BN6: the guardians' HP drain is MoonBld's hidden HP bug.
Sessions 37 and 38 replayed on their own builds, the guardian's HP
logged per frame: each drain began on the frame a MoonBld hit landed
(ElementMan 860 to 730, EraseMan 738 to 608) and took 1 HP every 40
frames to the battle's end, about 50 HP a fight. A write watch in the
core traced it: MoonBld's sweep (chip 84, bn6f AIAttack 0x40, object t3
0x85) spawns hitboxes that carry bug 0x18, the battle HP bug, at level 1;
a hit raises a Navi's level in its side's battle NaviStats (+0x18 of
eBattleNaviStats1, bn6f sub_8013AE4; a virus keeps its own, sub_8013B20),
at most 7, and sub_8010230 takes 1 HP from it every 40, 35, 30 ... 10
frames by level, never below 1, paused on the Custom screen. The chip
says only "Slices enemies around". On a fresh run, idle in a guardian
fight with and without HeatCross: no drain; one MoonBld on EraseMan: the
drain, 40 frames a point. For the next notes: "From the developers, after
replaying your sessions: the guardians' HP ticking down was your
MoonBld. BN6 gives it a hidden HP bug: whatever it cuts loses 1 HP every
40 frames until the battle ends (faster after more MoonBld hits), never
below 1, paused on the Custom screen. Neither ElementMan's copy nor the
Nest had anything to do with it."

Loop change: **triage while the next session plays.** The report came
while the rival's phase two was half done; the fixes for session 38 went
in after session 39 had launched on the pinned build, since the new run
meets the rival first and the ending last. A fix the next session cannot
reach does not hold its launch.

## Session 39: 7/10 (keep playing: yes; recommend: yes)

A new endless run (Storm, EraseCross, threat 1). ProtoMan's first duel,
taken at 100 of 140 HP on layer 2, deleted MegaMan and ended the run: a
squad a notch above the layer's (Quakers aloft half the fight), nothing
saying the stake, and a target (0:10.00) out of reach of an act 1 hand
(0:27.53 in the second run's duel). A second run cleared act 1 on a
briefing's tip that won the SpoutMan fight outright. From 9 to 7: the
rival is a good idea told in too few words, and it cost a run.

Confirmed: the choice guard (an A as a menu drew, three A's through the
dealer's words buying nothing), the duel starting on No, every act's
second layer holding it, the DeleteTime quoted, the record kept, Dad's
Records counting the Nest win.

Raised, fixed after the session's pin (e8e56f5, f286120, fb1605a): the
stake said before the choice, the squad the act's own, the rival
remembering (the record in Chaud's calls; a deletion in a duel counted
as a loss), ProtoMan greeting an old rival, Lan answering, the summary
naming the duel, the duel's clock in the battle, looser first times,
what a win earns. Open: EraseCross's strength unexplained (the setup
names only its weakness); a batched A picking the act's route.

Loop change: **a new system meets the persona in its most punishing
form first.** The duel was checked by captures of its words and one won
fight, never lost; the persona's first contact was a loss that ended a
run. Before launching a session into new risk, capture its failure path
too (the loss, the deletion, the timeout), not only its success.

## Session 40: 8/10 (keep playing: yes; recommend: yes)

CONTINUE of the endless run into act 2 (Green HP): a collector's vault,
two ScrtData, ProtoMan's second duel lost on time (0:21.60 against his
0:17.50, the record 0-2), and CircusMan deleting MegaMan with 149 of his
700 HP left. From 7 to 8: the rival now says its stake and keeps score,
and the clock made the duel a race; both losses read as fair, and the
persona wants the rematches.

Confirmed: the record in Dad's Records and Chaud's call, Lan answering
the call, ProtoMan greeting an old rival, the stake said before the
choice, the act's own squad, the clock (hidden on the Custom screen, red
past his time), the record kept, the summary naming the killer, the
shorter A hold, walking without backward steps.

Raised, fixed after the pin (9fb551c): Lan answering a lost duel, the
rematch line saying the record, EraseCross's counter-erase told in the
setup and the PET, beginner tips kept to the first act. Open: Chaud's
prize has no face (official gates stand on one layer in four from act 2;
none on the persona's three), a sold-out row sliding the cursor onto the
next item (from session 38), a kept R and a fresh one opening a chip's
description, every Dad mail NEW after CONTINUE, the NaviCust's double
quit prompt (from session 36), the dealer's guardian pick off the
folder's codes. Walkway mouths stop MegaMan as in BN6, by the owner's
decision. Unverified, probably BN6's own: MegaMan unseen while AreaGrab
stops time.

Cost: 351 calls against 300; the guardian fight took about 70 in short
steps, walkway mouths about 20.

Loop change: **the watchdog tells one battle from the next by the run
log.** Its "this battle" joined every battle seen at consecutive
five-minute snapshots, so a layer's four fights read as one battle of
28 minutes and raised a false alarm; it now counts the run log's
finished battles and starts a new battle when the count moves.

## Session 41: 7/10 (keep playing: yes; recommend: yes)

NEW GAME (endless, Storm folder, EraseCross, threat 1, HP+): run 1 ended
on Seaside's layer 2 (a Piranha and a StarFish), after ProtoMan's third
duel was lost on time against two Quakers (0:27.30 to his 0:12.00, the
record 0-3); run 2 stopped on Central Area's layer 1 at 180/200. From 8
to 7: no guardian, gate or Spin met, a duel that felt like a Quaker
lottery, a sloppy death.

Confirmed: EraseCross said in the setup and the PET and seen erasing in
play, Lan answering a lost duel, the gift Prog's consolation after an
early death, Dad's line in town, the setup remembering its choices.

Raised, fixed after the pin: the dealer's list bought by an A carried
over from his words (c775af9, the first "Are you sure?" after a keeper's
words starts on No). Fixed meanwhile from GitHub issues: the BugFrag
Trader's trade (d050033), the arrival's card and words as one beat with
MegaMan held (27f83a8), layers built a quarter faster (dd91df8). Open:
the duel's squad (Quakers in three of four duels, only hittable as they
land, so the clock is a lottery; his terms say "a pair of viruses", and
BN6's enemy names are not located in the ROM yet); the town's port at
the mermaid fountain (hints contradicting a step apart, a bystander at
the port); every mail NEW in a new run. Misreads: AreaGrab's freeze
eating an input (the second session running: BN6's own time stop), the
Cross list's second A (found by the persona itself).

Cost: 261 calls against 260; the port in town took 7, walkway mouths
about 15.

Loop change: **the persona's build is named with its features.** The
official gate on every duel layer landed while session 41 played, so its
duel layer had none, and the persona reported "no gate seen" as a gap.
Patch notes now come from `git log` from the pinned commit, which
pin.sh prints, and the goals say on which layer the persona's save meets
each headline. The AreaGrab misread, seen twice, went into persona.md.

## Session 42: 8/10 (keep playing: yes; recommend: yes)

CONTINUE of run 2 through BlastMan's act (a planned win after his
briefing, 8-0) into act 2 (Sky HP, CircusMan's), ProtoMan's fifth duel
lost (0-4, a Quaker and a Gunner), 400 max HP. From 7 to 8: the guardian
fight played out as the briefing said, and the act change reads as one
moment.

Confirmed: the shop's first "Are you sure?" on No (A paced 66 frames
through the dealer's talk), the act card and words held as one beat,
Chaud's call naming the gate, no chip description from a kept R, layers
made without a felt pause.

Raised, fixed after the pin: Quakers in race squads (87cc31c, before the
report), official gates never saying what they hold (f714f93). Misread,
caused by the patch notes: "a gift you took stays taken" read as a gift
taken after the layer's start save; CONTINUE resumes at the save. Open:
one paid rematch per act (a loss locks the act's gates; a design
question), the NaviCust's double quit, mail NEW after CONTINUE, a pad's
green ring read as the exit pad. Misreads: the Custom screen's cursor
after choosing the Cross (the second session: into persona.md), a charge
started under a chip's finish, AreaGrab again (as told).

Cost: 251 calls; the agent stalled once on the harness side at 159 calls
and was resumed with its context.

Loop change: **patch notes say what a save keeps, not what the code
keeps.** "A gift you took stays taken" was true of the code (a guardian
beaten or a gift taken before the save) and false for the persona's save,
made at the layer's start: say where the persona's CONTINUE resumes and
what it will meet again.


## Session 43: 7/10 (keep playing: yes; recommend: yes)

CONTINUE of run 2 on Sky HP (CircusMan's act): layer 4 afresh after the
LAYER_MAKE bump, a strong virus signal taken and won, two dealers, the
NaviCust vendor, UnderSht installed, six fights; layer 5, the duel
layer, searched for ProtoMan until the budget ran out (294 calls). From
8 to 7: the session never reached its climax, neither the duel nor
CircusMan.

Confirmed: Chaud's call and the gate naming the official Chip Order (it
now pulls: "Before, I raced for a word. Now I want the prize"), the
shop's first "Are you sure?" on No at every counter, the NaviCust's Yes
path quitting in one prompt, CONTINUE restarting the saved layer.

Raised, fixed: ProtoMan and the official gate shared the map's violet
mark and L named no direction, so the persona found the gate alone and
never the rival (bd75489: the gate by him on 72 of 74 duel layers, his
own pink mark, L says where he waits); the strong virus signal's "a good
chip" (6bc80ad: "It pays DolThdr3 B"). Caused by the loop: the patch
notes said the gate stood "beside" ProtoMan, which the build did not do;
now it does. Kept: walkway corners stop MegaMan as in BN6 (the owner's
choice, c33d2e0), though they cost about a dozen calls again;
FIDELITY.md still described the removed assist and now does not.
Vanilla: the enemy side's AreaGrab (replayed: the engine gives viruses
no chips), the Armadill's rolls. Misreads into persona.md: EraseCross's
beam needs about 115 frames (two sessions), an A after a chat's last box
talks again (three times).

Cost: 294 calls against 260, about 12 on walkway corners and 30 on the
search; the watchdog's budget warning came at 270 and a SendMessage
closed the session at 294.

Loop change: **a goal that depends on finding something says where it
is.** The session's two goals sat on a layer whose rival had no mark of
his own: the persona spent its budget searching and the report scored
the missing climax. Before launching, check each goal's object is
findable in the persona's save (the map and L name it) with a capture,
and say in the goals where it stands when the build cannot.

## Session 44: 8/10 (keep playing: yes; recommend: yes)

CONTINUE of run 2 on layer 5, the duel layer: ProtoMan found in two calls
by his pink mark and L's "down and to the right, a ways off", the race
won (0:13.28 against 0:14.00, the first win in six duels), the official
gate opened for HeatDrgn G; layer 6's CircusMan deleted MegaMan with 118
of his 700 HP left, and the run ended. From 7 to 8: the climax came, a
rival won and a real boss fight lost; he wants one more run.

Confirmed: the rival's wayfinding (bd75489: the gate beside him, his
pink mark, L's direction), CONTINUE restarting the layer with Chaud's
call, no Quakers in the race, the named prize pulling him into the duel,
`hold B 120` for EraseCross's beam and the A after a chat's last box.

Raised, fixed: CircusMan's tent tell, "when only our panel lights", is
true but the panel lies under MegaMan's feet; the persona's own frames
showed the cue that can be seen, CircusMan fading from his panel, and a
HeatDrgn rising beside the tent while he was gone (01a0cc6: the briefing
names the fade and says to hold chips). The NaviCust's L and R turn a
program only where its colour's Spin is held, BN6's rule, while three of
MegaMan's lines said they turn any: twenty calls on an HP+50 (5b93d76:
the words come from the Spins held; after a RUN with a bug he answers
its "OK"). Lan silent after the first win (44d93b1). The official Chip
Order's names alone (364b9db: each chip's damage in the choice). Fixed
beside them, from issue #17 and its testing: a bought-out shop says so
and the vendor brings his whole list (a9d7ecd), CONTINUE no longer
restocks the shops (c1ed95d), Chaud's call comes once a layer (a8c6659).
Vanilla or misread: BN6's RUN wording itself, the Cross portrait eating
a press and START lost after a shop (both into persona.md), BlastMan's
fire wall under a jumping CircusMan, an L at 95% (the 30-frame note).

Cost: 301 calls, 83 minutes; the CircusMan fight alone 74 calls and 26
minutes at 48 frames a call, the NaviCust 20. The watchdog named an old
9999.png as the latest picture all session, as a name sort puts 11969
before it (0cb6208: by time); a budget SendMessage at 293 closed it.

Loop change: **read the persona's frames at a reported tell before
the code.** The report said the tent's tell could not be seen; the
frames around each tent showed which cue hides (the lit panel under
MegaMan) and which shows (the fade), and the fix followed from them in
one step. The class is a briefing that names a panel under MegaMan's own
sprite: sweep every guardian's tip for one (GroundMan's "from under the
lit panel", HeatMan's shadow "under us") against its fight's frames.

## Session 45: 7/10 (keep playing: yes; recommend: yes)

Run 3 from NEW GAME (endless, Storm, EraseCross, threat 1, HP+): act 1 in
the RoboDog Comp, the gift's SuperArmor, the layer-1 dealer's DolThdr3 V
for SpoutMan, layer 2's rung-1 duel won without a hit (0:04.66) and its
gate's FireBrn1, UnderSht from the vendor; SpoutMan deleted MegaMan on
layer 3 with 220 of his 600 HP left, the persona's own misplay by its
account. From 8 to 7: walkway corners ate about 20 calls again, and the
duel felt like a formality.

Confirmed: the gate's chip damage, Lan's answer to a win, the vendor's
four programs, the Cross portrait's wait, the first "Are you sure?" on
No.

Raised, fixed: ProtoMan's gate open before his duel, which paid only the
record (fab0faa: the gate beside him opens to the duel's winner, Chaud's
call names the stake, the verdict the next rung; the official Chip Order
draws uncommon and rare chips). The heal "a long way back" from
SpoutMan's arena (3aa2384: within a short walk of its door on every
guardian's layer, held by the tests; the golden hash caught the change
and LAYER_MAKE went to 63 with it). L and R silent on a program with no
word why (3c28b75: the reminder after a program is gotten says whether it
turns). Kept: walkway mouths and corners, BN6's own walking by the
owner's choice, the persona's top wish for the third session running.
Misreads into persona.md: a green Mystery Data "gave nothing" (a B held
after the A skipped "MegaMan got: AirSpin1 R"), START after a shop needs
120 frames. Left: the setup's Cross rows (what each Cross does, from
BN6's tutorials), L naming ProtoMan's way as the crow flies, the vendor's
shop face (the keeper's, a known limit), the shop prompts' remembered
cursor.

Cost: 301 calls, 72 minutes. Issue #19's guardrails landed during it:
three of this iteration's fixes grew functions build.py lint lists, and
each was split rather than the baseline raised.

Loop change: **the checks are part of a fix.** A fix is done when
`build.py test` (the golden layer hash, the heal walk) and `build.py
lint` pass; a check on `tools/play.py` needs `build.py linux` first
(SKILL.md's pitfalls).

## Session 46: 8/10 (keep playing: yes; recommend: yes)

Run 4 from NEW GAME (endless, Storm, EraseCross, threat 1, HP+): act 1
in the Seaside Area, DiveMan deleted on layer 3 by his briefing's own
words (an ElcPuls1 counter as he surfaced, torpedoes read by their
shadows), Custom1 from his Guardian Data, the split taken to CircusMan's
RoboDog Comp for the grudge; the budget ended on layer 4 at 260 HP. From
7 to 8: a clean act and a guardian beaten by listening; the rival gone
quiet kept it from 9.

Confirmed: the heal by the arena's door (on the arena's own platform),
the Mystery Data boxes let show (four of them), "V0.5.0 BETA".

Raised, fixed: the official gate beside ProtoMan's deferred netbattle was
sealed for good, a duel's prize with no duel (f38d439: it opens to the
clearance; Chaud and ProtoMan say "past the next two guardians" for "the
third act", and Chaud what a win opens); Lan silent after that call
(2502bf6); no word on L and R at the Guardian Data's pick (29bf460);
DiveMan's HP falling with nothing hitting him, which is EraseCross's own
Navi bug, and the setup's "Counters erase viruses" was two playtesters'
guess (60479a8: BN6's rule from its Cross mail, "A 4 in HP: plain chips
erase"); the arrow pointing past a walkway's mouth into the platform's
corner, the top wish for three sessions (6bb8569: the mouth first, "on
the line" measured in a replay: 0.375 of a panel off went in, 0.41 was
stopped). From session 45's leftovers: L names ProtoMan's way along the
walk (81b91d1), the setup says what each Cross gives (c4bda97). Beside
the loop, the owner's 3DS frame log: the bottom map cost a frame at each
redraw (b4d3b56: drawn into the GPU's memory).
Misread: the green Mystery Data "out of reach" on layer 1 stood on a pad
that meets the floor on its upper-right side (replayed). Vanilla: the
torpedo drawn a row above its row (the briefing's shadow line covers
it). Kept: Quaker pairs, the worst fight of the act three sessions
running, from BN6's formations: a candidate for act 1's pools.
Sweep (session 44's loop change): HeatMan's, GroundMan's, SlashMan's and
ElementMan's tells watched in god mode with MegaMan standing: each lit
panel shows past his sprite; CircusMan's was the only hidden one.

Cost: 262 calls, 76 minutes; DiveMan's fight 45 calls. The watchdog's
"a long battle" at 15 minutes found DiveMan at 67 HP: no step needed.

Loop change: **check the rival's rung before a duel goal.** The
session's main goal, the duel with its gate as the prize, could not
happen: two wins had moved ProtoMan to the third act's netbattle, and
acts 1 and 2 hold only his call. Before a goal about him, read the
persona's record (`profile.duel_won % 3`: 0 and 1 race in every act, 2
waits for the third act) or capture the act's second layer with `--dev
duels=N`, and say in the goals where he is.


## Session 47: 7/10 (keep playing: yes; recommend: yes)

Run 4 continued from layer 4 (Storm, EraseCross, threat 1, HP+):
Custom1 installed at MegaMan's word, two acts' dealers and an official
gate's Chip Order, then CircusMan on layer 6 deleted MegaMan with 286 of
his 700 HP left, the fifth loss to him. From 8 to 7: the fixes landed,
the same guardian ended the run, and ProtoMan's netbattle is still two
layers past him.

Confirmed: the official gate where ProtoMan names his netbattle opens to
the clearance; Chaud and ProtoMan both say "past the next guardian", and
Lan answers; a picked program says whether L and R turn it (Custom1,
HP+100); EraseCross by BN6's rule (a Cannon erased a 40-HP Gunner, two
erased 140-HP Shooters, WaveArm3 bugged CircusMan at 480); the arrow into
a walkway's mouth, then down it, twice.

Raised: CircusMan's tent, a wall for five sessions (major, open): two
tents took 150 of 360 HP though Kai moved a panel every 45 frames; the
briefing's second wording (his fade, then step off) worked once in three.
The lit panel hides under MegaMan's sprite (session 46's sweep), so the
tell is his fade alone, too quick to act on. Chaud's "every official
gate opens" to a player already cleared: already fixed after the pin
(7a81375: his full clearance, the vaults). The Chip Order's picks (no
description on R, no code of the folder's; open, wish 3). ProtoMan
unannounced on the layer where he names a later netbattle: by design
(the map leaves him out, map_left_out; Chaud's call says it). "It's a
long way yet" near the exit: 14 panels or more of walk, calibration, said
in the patch notes. The shop's remembered Yes/No cursor: BN6's own shop
screen. EraseCross's slow drain on a 700-HP Navi: BN6's rule.

Beside the loop: 0.5.2 and 0.5.3 released (the 3DS's smooth net and
faster layers; issues #20 and #21, a player's: a duel counted again after
a CONTINUE, the BugFrag Trader trading at 0), and Battle Network 5's maps
read through the map reader (docs/MULTIROM.md).

Cost: 312 calls (260, the guardian's 40 and a few to finish the fight);
CircusMan's fight 105 s of game time.

Loop change: **a guardian who wins twice gets his tell measured, not
reworded.** CircusMan's briefing was reworded in sessions 46 and 47 and
he won both. Before a third wording, watch the attack in god mode
(`guardian_watch.sh`) and count the frames between the first thing a
player can see or hear and the hit; under about 30 frames (a quarter
second to see, a step to move) no words fix it: the tell itself needs
lengthening, or a cue of the engine's own (a sound, a mark on the panel).

## Session 48: 8/10 (keep playing: yes; recommend: yes)

Run 5 begun (Endless, the Standard folder for the first time in five
runs, EraseCross, threat 1, HP+): act 1 in Central Area cleared, BlastMan
deleted at 40/140 HP with the dealer's AquaNdl2 fired the moment he stood
still, as both the dealer and the briefing had said; HP+100 from the
draft, HeatMan's act chosen over CircusMan's; saved on layer 4. Up from
7: a new folder changed how the fights played, and the dealer's tip, the
briefing and the kill were "one moment".

Confirmed: Chaud's call names what the duel's win adds (the official
vaults); the setup's Cross lines (a Cross picked from the screen alone);
EraseCross erasing 40-HP Mettaurs with a Cannon and a Vulcan's first
bullet.

Raised and fixed: the draft's HP+100 "takes the pink Spin" against the
vendor's blue one (both right, a program's two colours; the draft's
lines now name theirs, 570cf39); EraseCross's bug on a Navi unsaid
(MegaMan's guardian briefing and the PET's report say it, ee7d62d; the
setup had no room for a third line, captured); the None Cross line
(1bd4ae9); the arrow "pointing right, up-right, up" at a walkway running
down-right (replayed: the state's way was right each time, the dash had
run a panel past the mouth, and each picture, 4 frames after its step,
showed the arrow before its turn; it now turns at once as MegaMan stops,
aee9982); no arrow on a strip where it had faded, after the map (the map
now hands over to it, 37bc6f4).

Open: an arrow to off-route gates and vaults L names (wish 1's second
half; the layer-2 gate had its own walkway one strip west, and would have
stayed shut to his clearance); act 1's three random battles, "Viruses 6"
(BN6's own step counter, one session's count: watch it before touching
the rate); EraseCross's drain small on a Navi (BN6's rule).

Misreads: BlastMan's row (he hovers a row's height above his panel; four
charged beams down the wrong row) into persona.md beside the Piranhas and
ChargeMan's train; the arrow at the mouth (above), with the way to bring
a faded arrow back.

Beside the loop: Battle Network 5's ACDC, SciLab, End and Nebula Areas
dress BN6's Central, Sky, Seaside and Graveyard in about half the runs
that reach them (with the BN5 ROM beside BN6's, as the harness has it).

Cost: 298 calls (260 and 38 on the guardian's layer); BlastMan 1:20.

Loop change: **a cue "pointing wrong" in a report is checked against the
state's words first.** The pictures come a few frames after each step,
and anything the game turns on a delay (the arrow's looks every 5 frames,
a fade) shows the moment before it. Replaying with the state beside each
picture settled this one in three calls: the state was right three times
running and each picture one step behind. Where the two disagree, the lag
is the bug, not the cue.

## Session 49: 7/10 (keep playing: yes; recommend: yes)

Run 5 continued from layer 4 (Lab Comps, HeatMan's act): six battles over
three layers (13 viruses; act 2's count "felt right"), both dealers' Aqua
picks bought, a ScrtData; the budget ran out on layer 6, HeatMan's, a walk
short of the arena. Down from 8: no guardian, the act-1 prize chip "fired
backward", and menus took 65 of 300 calls.

Confirmed: the arrow turning as MegaMan stops (no stale arrow all
session), SELECT bringing the arrow back, HeatMan's briefing and the PET's
dive report naming EraseCross's Navi bug, HP+100's colour.

Raised: BlastMan B hitting nothing (major): replayed from two rows,
BlastMan goes for the nearest enemy-side thing, and the StarFish's
bubbles floating behind MegaMan were it; in session 43, with no bubbles,
the fire rolled forward and hit twice (BN6's own targeting, in the patch
notes). The NaviCust bug's cause unnamed (fixed after the pin, 92ed1fa).
The NaviCust reminder twice at CONTINUE (4bba08c). Walkway mouths, about
15 calls, the arrow right each time (BN6's walking, kept by the owner's
choice). The layer-5 dealer below a raised walkway's edge (one sighting,
not yet counted). Menus a fifth of the calls (wish 3: a bought chip into
the folder at the shop; a design question for the owner). No how-to for
the dealer's TrnArrw3 (only AquaNdl has one).

Beside the loop: BN5's areas play BN5's music (its net and Undernet
themes, copied into BN6's empty song slots) and have its HeelNavis as
bystanders; MegaMan names the NaviCust bug's program and rule.

Cost: 300 calls, 78 minutes.

Loop change: **the goals put the guardian first when the save is a few
layers short of one.** Sessions 47 and 49 began two or three layers before
a guardian; one reached him with the 40 extra calls, this one did not,
its calls spent on menus and a long last layer. When the save sits in an
act's first or second layer, the goals say: one folder pass per dealer,
then the arena; the fight is what the report most needs.

## Session 50: 8/10 (keep playing: yes; recommend: yes)

Run 5 from layer 6: HeatMan deleted first (0:47.75) with the dealer's
AquaNdl3 as MegaMan's briefing said, his HP drained by EraseCross's bug;
Custom1 from the Guardian Data, the unknown Aquarium Comp chosen; on layer
8 ProtoMan's netbattle at 1000 HP, run from after a hand at 180/460. Up
from 7: the guardian came first, as the goals asked, and menus took 30
calls where they took 65.

Confirmed: the draft's colours, BlastMan's targeting as the patch notes
told it (forward with no bubbles, two hits on ProtoMan), BN6's own
transmission panel and BATTLE START.

Raised and fixed: ProtoMan's netbattle a wall (major): half the act's
guardian band's top (500 in act 3) and his tells said as it is offered
(9862b5a, the game-design pass in its message); "Our second ScrtData!"
on CONTINUE with one held (7dbe7c0: the count read before the save
loaded); the act's AREA CLEAR count and the dealer's greeting lost on
CONTINUE (b6d1663, after the pin: the act kept beside the save); L's
"ProtoMan, waiting for our duel and an official gate" (5b6efb6); the
split's place against a Navi (e508c2c, after the pin: "??? (Aquarium)").

BN6's own: the shop's remembered Yes/No cursor (third report: into the
patch notes again). Kept: walkway mouths, about 20 calls, the arrow right
each time (the owner's call; wish 3 asks for a slide onto the line).

Cost: 266 calls, 74 minutes.

Loop change: **a fight the run promises is swept before the persona gets
there.** ProtoMan's netbattle was written to the act's guardian band and
never fought by anyone before Kai met it as a wall at the end of a
five-session thread. A new fight's numbers (HP, damage a second) are now
measured against the persona's state at that point in its run before the
session that reaches it: the HP, the folder's best chip and the guardian
just beaten, in the session's state and notes.

## Session 51: 8/10 (keep playing: yes; recommend: yes)

Run 5 from layer 8: healed, a StarFish-and-Shaker battle that took 350
HP, ElecMan (act 3's guardian, never met) deleted in 0:58.40 by reading
him live, UnderSht from the draft, the split's "??? (ACDC HP)" taken;
saved on layer 10 at 560/560. The same 8: "beating an unknown guardian by
reading him" is what BN6 alone never gave, and a third of the calls went
to walking.

Confirmed: no false ScrtData line on CONTINUE, the split's "???" labels,
act 3's card without a count as the patch notes said for an old save.

Raised and fixed: the arrow led to the exit while L sent a hurt MegaMan
to the heal (517a4c7: the arrow leads there first while he is below
three quarters, and L says so); StarFish bubbles eating every straight
shot unwarned (5dc6e52: the area warnings move into a table with
StarFish); "I don't recognize it" beside Lan naming ElecMan at sight
(7ed2f60: "one we've never faced down here").

Open: the dealer's pick out of reach (1000z with 450z earned in act 3,
whose battles paid chips): one sighting; income per act is measured
nowhere, so count it before moving the cap (400, 700, 1000). The ElecMan
gate's count (one deletion of two) unsaid after his deletion. Walking: a
notch in a platform's wall that looked like a way in (the owner's call on
BN6's walking stands; the heal arrow takes some of it).

Cost: 258 calls, 74 minutes.

Loop change: **when the persona names a wish that its notes show the game
half does, finish the half.** L already named the heal's way to a hurt
MegaMan; the arrow did not follow it, and the wish was the missing half.
Read the notes for a cue the game gives in words but not in the world (or
the reverse) before designing anything new.

## Session 52: 8/10 (keep playing: yes; recommend: yes)

Run 5 from layer 10: UnderSht installed in one NaviCust pass, BlastMnEX B
from a green Mystery Data, the second ScrtData, five battles over layers
10-11, and ProtoMan's netbattle at 650 HP: 442 dealt in two hands, run
from at 20 HP with him at 208. Saved on layer 11, a new best. "Hard but
winnable now. Keep 650": three of his four hits were the player's own
mistakes.

Confirmed: the heal arrow (three calls to the Recovery Mr. Prog), "one
we've never faced down here", ProtoMan's 650 and his tells said, L's own
sentence for the duel, the ScrtData count, the UnderSht reminder's colour.

Raised and fixed: one lit panel, MegaMan's own, stood for a WideSword
down its whole column (major: stepping a row up was hit anyway), and the
tip named only the row (c4282d0: every lit shape, the column, and what
gets past his shield). A capture of the new tip, paged to its end, showed
its last clause gone: the netbattle's prefix took the box past ta_pages'
200-character buffer, which the tip's own test could not see; the terms
moved where the tests check them composed. CI's test build had failed
since the BN5 commits on stubs without prototypes (49ebaa5), unseen
because `build.py test` ran without CI=1.

BN6's own, into the patch notes: RUN's "None" (an empty command-line
cell), the ElecMan chip's "surroundings" (MegaMan's own). Kept: the
dealer naming the guardian as the net's word; walkway branches, about 20
calls (the owner's call; wish 2). Open: the money came after the shop
(450z at the dealer, 2100z at the duel): the second sighting, so income
per act gets measured now.

Swept ahead: Colonel, act 4's guardian on Kai's next layer, fought by
no session before. A headless start at layer 12 gives MegaMan 100 HP,
which one Colonel hit deletes even in god mode; a dev switch (`hp=N`,
0212f8d) and `scripts/guardian_stand.sh` measure a guardian against the
persona's HP. At 560 HP, MegaMan on his panel firing the buster: Colonel
21 HP a second, ElecMan (act 3, whom Kai beat) 11; act 4's others 8
(TomahawkMan), 17 (ChargeMan), 29 (DustManEX). Colonel is hard, not a
wall, and his moves match his tip (the zigzag lights, the dark screen
before the big slash). Open: the bands hold a guardian's HP only, and
act 4's damage spans four times over; a guardian's damage a second could
join his fit for an act.

Cost: 282 calls, about 80 minutes.

Loop change: **a changed text is read in a capture to its last box.**
The first boxes rendered right, and the cut came at the end; the unit
tests check each text alone, while the game composes them. Page a
changed chat to its question or its close before committing.

## Session 53: 8/10 (keep playing: yes; recommend: yes)

Run 5 from layer 11: healed first, a green Mystery Data's 1460z before
the dealer, layer 12 a new best, the third ScrtData, the dealer's
DolThdr3 for Colonel, and Colonel himself (act 4, never met): 841 of
1200 dealt in 1:46, MegaMan deleted at Colonel's 359, UnderSht holding
once at 1 HP. Run 6 began at once with the ElecCross the loss unlocked:
"I lost a twelve-layer run to a guardian I'd never met, and I still
want the next one."

Confirmed: the heal arrow after CONTINUE, both act 4 dealers' returning
greetings (the act note), the guardian named as the net's word,
UnderSht, money before the shop this time.

Raised and fixed: Colonel's cape sweep along the row he lands in, only
his own panel lit (major), and his Cannons turned aside while he
readied a slash (3c9cfd5: his battle data names both, given now that
they have fought); no running from a guardian, unsaid at 1 HP (3c9cfd5:
the room before every arena says it); the third ScrtData's gate unsaid
(e1d1ed6, during the session: it stands in the Undernet); EraseCross's
drain on a Navi, about a point every 40 frames, promised as his HP
draining away (3c9cfd5).

Misreads: the exit pad "only from its centre" (his feet were on the
panel in front of it, his sprite over its ring: the trigger reaches 26
units, the ring 18), and "CHIP DATA TRANSMISSION" again (into
persona.md: seen twice). BN6's own: a diagonal lit about 14 frames
before it hit, during AquaNdl3's animation. Kept: walkway crossings,
about 25 calls (the owner's call; wish 3 again).

Beside the loop: the sweep before the session measured Colonel at act
4's middle (21 HP a second at 560 HP to ElecMan's 11), hard, not a
wall, and Kai's 70% bore it out. BN5's battle music and backgrounds;
GitHub issues #22 (every generated stair's top lacked the step cell
BN6's maps have: MegaMan fell under the raised room) and #24 fixed.

Cost: 292 calls, about 90 minutes; the Colonel fight 91 calls, 25
frames a call at 1 HP until a pace note.

Loop change: **the sweep of a guardian the persona meets for the first
time looks at his telegraphs, not just his damage.** Both majors of
sessions 52 and 53 were moves whose lit panels showed less than they
hit (ProtoMan's WideSword, Colonel's cape), and both were in the
fights' frames before the sessions: watch each such fight every four
frames (play.py `--every 4`), list each move with its lit panels against
its reach and when it can't be hurt, and make his tip cover what the
light doesn't.

## Session 54: 8/10 (keep playing: yes; recommend: yes)

Run 6, the first with the ElecCross start Colonel's loss unlocked: the
Storm folder chosen for it, Seaside Area's act cleared in 10:15 of game
time, DiveMan deleted in 0:20.43 (ElcPuls1 "100+50" took 300), and the
split chosen by the folder ("Aquarium's Aqua viruses take double from my
Elec chips, and Judge Tree's BombCorns are Wood"). Saved on layer 4,
HeatMan ahead. "The ElecCross start changed how I built the run."

Confirmed: the act's AREA CLEAR count and time, the exit pad from its
ring, no random battle in a guardian's staging, the heal arrow, a stair
walked down.

Raised and fixed: a talk opening while B was held to run was paged past
unseen, BN6's own page wait turning on ten frames of held B (0b87356:
the held B kept from it until let go); "StarFish here again" with none
in five battles (0b87356: "We may meet ... here again").

For the owner: walkway mouths, about 12 calls (wish 1 again); installing
a Guardian Data program and swapping its chip into the folder cost two
PET trips, 14 calls (wish 2: offers on the Guardian Data screen; a menu
and NaviCust design question). BN6's own: Piranhas leaving the row as a
charge lines up. Observation: act 1 a steamroll for an Elec folder
against an Aqua guardian, "earned, because I chose the folder for it".

Beside the loop: GitHub issues #22 and #24 fixed, #23 and #25 not
reproduced on the current build (they need their reporters); BN5's
battle music and backgrounds; TomahawkMan's telegraphs swept for a first
meeting (honest: a lit 2x2 for his axe, a lit row for his eagle).

Cost: 255 calls, about 65 minutes.

Loop change: **a talk the persona missed is found in the debug log, not
guessed at.** Kai could only say a box "was probably" the new no-running
line; `CYBERWORLD_EMU_DEBUG` now logs every director talk's text, so a
replay of the moment names what was said and when. Use it before
answering a "did it show?" in a report.

## Session 55: 8/10 (keep playing: yes; recommend: yes)

Run 6, act 2: HeatMan deleted in 0:54.25 with no type edge (ElcPuls1's
counter hit took 150, the ElecCross charged zigzag about 350 of his
550), act 2's AREA CLEAR "Viruses 7 Time 11:10", Custom1 taken from his
Guardian Data, the unknown "??? (Green HP)" chosen at the split. Saved on
layer 7, act 3. "I could trace the win to choices I'd made."

Confirmed: a held B no longer pages a talk unseen, "We may meet", "no
running from a guardian", the golden gate in the Undernet, no battle on
the last walkway, the AREA CLEAR count.

Raised and fixed: the arrow at walkway corners, about 15 calls on one
layer (f427984: a run into a corner stops MegaMan 12 units of 32 past
its middle, and the mouth's tolerance of 0.35 of a panel, 11.2 units,
pointed the arrow back at every corner; on the line now means within the
measured 12, and on the mouth panel itself the arrow points across, not
back); the official Chip Order taken by a batched A (d4ded6d: "Order
TrplShot J? We only get one!" on No); "Chaud's clearance" read as the
full one (ecf85cb: first and full, named); Custom1 fitting only after
two programs moved (67b5e0c: said as it comes, from the board's free
cells).

BN6's own: the DiveMan chip's wave stopping two columns in, HeatMan
unseen at 10 HP. Misread: an A pressed while a box typed only finished
it (into persona.md: seen in sessions 50 and 55). Open: the dealer's
pick without a word on how to land it (WideSht missed a teleporting
HeatMan; AquaNdl's line is the only one); walking still a third of the
calls, two Mystery Data on other branches never reached.

Beside the loop: GitHub #22 and #24 thanked and marked fixed for the
next version; #23 and #25 tried again with the reporters' seeds (the
Nest into layers 20 and 39, a real NaviCust bug and a Chip Trader on
0.4.0 and now), not reproduced, #25's reporter asked to retest.

Cost: 285 calls, about 85 minutes.

Loop change: **a threshold the arrow or a talk acts on is checked
against where the game itself puts MegaMan.** BN6 stops him at fixed
offsets (12 units past a corner's middle against the void), so a
tolerance just under one misfires at every such spot, not at random: a
replay with `CYBERWORLD_STATE_POS=1` gives the positions to check it
against before it ships.


## Session 56: 7/10 (keep playing: yes; recommend: yes)

Run 6, act 3 (Green HP): ProtoMan beaten in the netbattle in 0:11.75,
"my best fight in 56 sessions" (the DiveMan chip's wave mid-teleport,
Thunder's paralysis, a counter hit, ElcPuls1 point-blank), DiveManSP D
from the official vault, the blue Spin and ScrtData #2; then deleted on
layer 8 by two Armadil3s in a random battle walked into at 150/430,
past the Recovery Mr. Prog L had named. Run 7 begun at once (Endless,
Storm, no Cross, threat 1, HP+), waiting in ACDC Town.

Confirmed: the walkway arrow at corners (f427984), the one-time pick
starting on No (d4ded6d), the gates naming their clearance (ecf85cb),
the A at a typing box waited out.

Raised, open: the Net Dealer's "No word yet" two platforms after a Navi
named TenguMan (L repeated the rumour); MegaMan's briefing promising
that Navi chips get past ProtoMan's shield, HeatMan's flames doing
nothing while he guarded; the vault's list showing "DiveManSP D 1013"
(read as damage) and no description on R before a one-time pick; the
summary forgetting the duel, Chaud's record and the vault; the dealer's
confirm starting on No once and Yes after; walkway corners still a few
calls with the arrow right; ElecCross's setup text silent on a Wood hit
knocking the Cross off.

Broken-looking layers: none in 28 shots of Green HP, on 0dcd123,
before the tile sweep (it changed other areas' looks than Green HP's).

Beside the loop: the players' tile reports (stairs, the Undernet's
ramps, "tiles all kinds of broken") worked through area by area against
the originals: stairs with their own tiles alone, arenas in platform
floor (Weather, Robot Control, Central, Green, Sky), rims for the
Graveyard and Weather, Sky's catwalks thin and its rooms its fields,
Seaside's arrows and the Undernet's crosses kept out of joins, the
Nest's floating cubes gone, fields' middles by their period or a
stretch whole. The tile test's seams per hundred panels 68.7 to 62.5.
Tried and dropped: walkway runs laid by their period (the originals
have almost no straight run five panels long to learn from).

Cost: 268 calls.

Loop change: **what changed is put in front of the persona.** Kai
looked over 28 shots and found nothing broken, in an area the sweep
had not touched; a report of "none" says little about the areas it
never crossed. The patch notes now name the areas whose look changed
and ask for close looks at their floors, joins and stairs there.
