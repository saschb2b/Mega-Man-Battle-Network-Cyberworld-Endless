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
