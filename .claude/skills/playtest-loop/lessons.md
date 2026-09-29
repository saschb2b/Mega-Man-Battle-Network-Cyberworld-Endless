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
