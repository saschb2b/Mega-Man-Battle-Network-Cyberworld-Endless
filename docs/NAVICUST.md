# The NaviCust

The design that makes BN6's Navi Customizer a run's second build axis
beside the folder, reasoned with the game-design skill
(`.claude/skills/game-design`) from BN6's own rules. The facts about BN6
come from the bn6f disassembly and the Gregar ROM (offsets at the end);
what is built is in [Status](#status).

## Where it stands

Every run plays on BN6's starting 4x4 board with a four-cell command line
on its third row, and never grows: the engine boots a new game and never
gives an ExpMemry. A run meets programs in two places: one of three gifts
on layer 1, and the NaviCust vendor on an act's middle layer in 60% of
acts, with up to four of the 12 programs BN6's shops sell, for zenny. 34 of
BN6's 46 programs never appear.

The playtester in fifteen sessions never had more than one program
installed and never saw a bug. He called one purchase (SuperArmor, 1200 of
his 1330 zenny) a real trade-off, and left 1900 zenny unspent beside an
HP+400 he could not afford. Programs compete with chips and HPMemory for
the one currency, and lose.

Read with the skill's smell catalog:

- **Flat budget.** A 4x4 board holds two or three programs; a run holds
  one. The budget never bites, so the packing puzzle, the command line
  and the colours never come up.
- **Hollow loop.** A program feeds nothing larger: no build forms from
  one piece, and nothing in a run asks for one.
- **Lying signifier.** The vendor sells Reflect, ChpShufl and GigFldr1,
  which cannot sit on a 4x4 board without a bug, and says nothing.
- **Dead options.** Humor and Poem answer the L button, which the engine
  takes; OilBody, Fish, Battery and Jungle steer an encounter roll the
  engine replaces; Rush, Beat and Tango act only in VS battles; Millions
  would hang the game on the engine's Mystery Data.

## The experience

The target, from the experience down (MDA): **by the third act, the board
tells the story of the run.** "This is the run where MegaMan never flinched
and threw seven chips a turn", in pieces the player chose, fitted and
sometimes forced. The kinds of fun: Expression first (the build is mine),
Challenge (the packing puzzle and the bugs I chose), Discovery (programs I
have never had).

## The dialectic: fit it, or force it

The game already restates one tension: extra power at a price the player
sees and chooses. A Server challenge fights a harder battle for a better
chip; the dark flame into the Undernet trades safety for better Mystery
Data. BN6's NaviCust has the same tension in its own rules: a program that
fits cleanly costs space; one forced in (off the board's edge, touching
its colour, a fifth colour) runs with a bug. That is the cost-to-exceed-
budget pattern, and BN6 is the catalog's own example of it. The rework
keeps every one of BN6's rules and gives the player enough programs, and a
reason, to meet them.

## The loop stack

| Loop | Where the NaviCust sits |
| --- | --- |
| Battle (seconds) | What the board gives: a buster that matters, seven chips a turn, a shield on B+Left, no flinching, standing over holes |
| Layer (minutes) | Bugs at work: an HP bug in every battle, BugFrags paid for fighting bugged |
| Act (half an hour) | The act card names the guardian; the board is prepared for it, as the folder is with the dealer's answer |
| Run (hours) | The board grows twice and fills; its shape is the run's identity |
| Meta (runs) | Programs found in a run join the pool of later runs |

## The rework

### 1. The board is the budget, and it grows

A run starts on BN6's 4x4 board. The guardians of acts 2 and 4 each give
an ExpMemry, BN6's own key item (0x71): the board grows to 5 wide by 4
tall, then 5x5, and the command line to five cells. Growth at known
milestones gives the budget a rhythm: a cramped first two acts where
every block is a choice, room to build in the middle, and a full 5x5 for
the Undernet and the Graveyard. (The skill: a budget that never expands
feels punishing; one too generous stops biting. BN6 itself stops at 5x5.)

### 2. Programs come at the moments that matter

- **The guardian's draft.** Every Guardian Data adds a choice of three
  programs or none (card-draft-with-skip), but the run's last: nothing is
  left to run with (a playtester took HP+50 after the final fight, and was
  told on the way out to place it). One answers the next act's
  guardian or area where one does (AirShoes where the floor has holes,
  SuperArmor against a guardian that flinches MegaMan, Shield or Reflect
  against one that shoots); one fits the board's build (a Charge+1 beside
  an Attack+1); one is from the act's tier. "None" pays BugFrags, so a
  thin, clean board is a real choice.
- **The layer-1 gift** keeps its program option: one that changes how a
  first act plays at once (SuperArmor, Custom1, Attack+1, Charge+1), in
  one of the colours the ROM's program records give it. MegFldr1 left
  it: room for a Mega chip beside a starting folder was no choice.
- **Blue Mystery Data**, as BN6 hides programs: on the Undernet's layers
  and as a Server challenge's prize, where the run already takes a risk.

About twelve program choices a cycle against a board that holds three to
six: the budget bites.

### 3. BugFrags are the NaviCust's currency

Every currency is a verb (the skill). Zenny buys chips, HPMemory and
SubChips from the Net Dealer. BugFrags buy programs and their compression
from the NaviCust vendor, as BN6's own BugFrag shop sold programs. The two
stop competing: a player saving for the guardian's answer chip no longer
walks past every program.

BugFrags come from green Mystery Data (as now), from skipping a draft, and
from **fighting bugged**: each battle won with bugs active pays one BugFrag
per bug level. A light bug the player chose is an income; a heavy one is a
gamble. This pays risk in resources, not only in survival (the skill's
move from Expedition 33's parry).

### 4. The vendor becomes a workshop

The NaviCust vendor stands on every act's middle layer. He sells three or
four programs of the act's tier for BugFrags, and **compresses** a program
the player owns: BN6's compression code, entered for him, gives it its
compressed shape for good (fusion-economy: the same power in fewer
blocks). He says what fits: "Reflect won't sit on a 4x4 without a bug,
unless I compress it."

### 5. Bugs are a price the player can read

BN6's rules stay whole: a program part off the command line does nothing
and bugs; a plus part on it works and bugs; same colours touching bug both;
five colours or six bug at random at battle start; a block past the
board's edge bugs. What changes is that the player sees the price the
moment it lands: after the NaviCust runs, MegaMan names each active bug and
what it does in plain words ("An HP bug: I'll lose HP slowly in battle,
faster each time I'm hit"), then which program broke which rule, read
from the game's own board ("HP+100 is a plus part on the command line:
plus parts go anywhere else"), and L's status and the SELECT map show the
bugs. A bug the player cannot read is a punishment; one they can read is a
decision.

BugStop, which cancels every bug, is the overcharm of this board: eight
blocks (six compressed) that let a player pack the rest past its edge. It
comes late and rarely.

### 6. The pool, and the builds it makes

The pool holds 34 of BN6's 46 programs, by tier (the act they can appear
from), and leaves out the inert and the harmful: Humor, Poem, Rush, Beat,
Tango, OilBody, Fish, Battery, Jungle, Millions, SneakRun until its effect
on the engine's battles is known, and NumbrOpn until its condition is. Four builds come out of BN6's own
programs, each needing more space than a 4x4 board has:

| Build | Programs | It changes |
| --- | --- | --- |
| Buster | Attack+1, Speed+1, Charge+1, BustPack, AttckMAX, SpeedMAX, ChargMAX | The buster between chips, and a charge shot worth holding |
| Hand | Custom1, Custom2, MegFldr1, MegFldr2, GigFldr1, FldrPak1, FldrPak2, ChpShufl | More chips a turn, more Megas and a Giga in the folder |
| Guard | SuprArmr, FstBarr, Shield, Reflect, AntiDmg, UnderSht, BodyPack | Defense verbs on B+Left, no flinching, a barrier, surviving a lethal hit |
| Field | FlotShoe, AirShoes, AutoHeal, SlipRunr, Collect, BugStop | The panels, the map, the drops, the bugs |

HP+50 to HP+500 stay plus parts of every build, held to the early tiers
(+50 to +200 in the first cycle) so they do not outgrow the HPMemory the
guardians give: more HP is magnitude, and the skill's treadmill smell.

Tiers: acts 1 and 2 draw the small programs (Attack+1, Speed+1, Charge+1,
HP+50, HP+100, UnderSht, FstBarr, Shield, SlipRunr, Custom1, MegFldr1,
SuprArmr); acts 3 and 4 add the middle ones (Reflect, AntiDmg, FlotShoe,
AirShoes, AutoHeal, HP+200, Collect, FldrPak1, BustPack, MegFldr2,
ChargMAX, SpeedMAX); from act 5 the large ones (Custom2, GigFldr1,
AttckMAX, BodyPack, FldrPak2, ChpShufl, BugStop, HP+300 and up
in later cycles).

### 7. Programs found join later runs

Meta-progression unlocks variety, not power. A run starts from the twelve
shop programs and the tutorial's three; any other program a run finds (a
draft, blue Mystery Data) and installs is remembered in the profile and can
be offered in later runs. The first BugStop found in the Undernet is a
story; afterwards it is an option.

Built: the profile keeps a bit per program MegaMan has run with (on the
board, or in the PET where it fits the board), read at every checkpoint and
at the run's end. A NaviCust vendor lists two of them first, shuffled (but
those MegaMan has now), when
the act's tier (6) may offer them, at the price the others have (a quarter
of the game's shop price, or of its tier's where the shops don't sell it),
and his greeting names them: "I hear MegaMan's used Collect and AutoHeal
before. I brought them along!" BN6's own picks fill the rest of his four,
a program once whatever its colour.

### 8. The Spins, found in the net

A program turns on the board with L and R only where its colour's Spin is
held (BN6's key items 0x50-0x55: bn6f `sub_8136364` checks 0x4F + the
program record's colour). A run finds at most one, a colour the profile
lacks, in a blue Mystery Data on a layer of 4-8, and keeps it for good
(docs/META.md). The drafts and the vendor's fitting pack each program
turned only where its colour's Spin is held (`navicust_set_spins`).

### 9. Compression codes, kept once entered

BN6's compression code, ten buttons on the NaviCust screen with RIGHT held
on a program, sets the compressed-shape flags of its four colours; a run
starts from a fresh game, its flags clear, and a player looked every code
up again each run (issue #50). The first time a code is entered in any run
the profile keeps it: Dad's Compression mail lists it, and MegaMan names it
where a program that won't fit would fit compressed. The buttons stay the
player's to press in every run, and nothing names a code not yet entered
(docs/META.md). MegaMan's fit checks read the shape the flags choose
(issue #54).

## Teaching it

Introduce, develop, twist, test (the skill's onboarding shape), each at
the moment it is needed:

1. **Introduce:** the layer-1 gift, and MegaMan saying where the NaviCust
   is in the PET.
2. **Develop:** act 1's guardian draft: a second program, and the 4x4
   board starts to ask where things go.
3. **Twist:** the first program that only fits with a bug; the vendor or
   the draft says so, and after the run MegaMan names the bug, and the
   first bugged win pays BugFrags.
4. **Test:** the grown board, the large programs, BugStop.

## What could go wrong

- **Complexity over depth.** BN6's NaviCust has many rules. The rework
  adds no rule to them; it adds supply, a currency and legibility.
- **A dominant build.** UnderSht (two blocks, survive a lethal hit) and
  SuperArmor are strong in any build; HP+ programs scale past everything
  if left in late tiers. Watch the playtests and the pick rates; the
  tiers are the lever.
- **Bugs as a trap.** If fighting bugged pays too little nobody forces a
  program; too much and every board is bugged. One BugFrag per bug level
  a battle is a start.
- **Menus.** Installing takes BN6's PET menus (the playtester: about seven
  calls). That friction is the puzzle's own, and stays.

## Measuring it

- The playtester's reports: does he build, does he force a program, does
  he read the bug line, does he skip a draft.
- `build.py pacing`: programs offered per act, BugFrag income against the
  vendor's prices.
- The run log: the board at each guardian.

## BN6's facts it uses

| What | Where (Gregar) |
| --- | --- |
| Program records, 16 bytes per colour variant (category, part or plus, colour, bug type, shape and compressed shape) | ROM 0x13B22C |
| Board maps for 0, 1 and 2 ExpMemry (4x4, 5x4, 5x5) | ROM 0x13CFCC, 0x13D0AD, 0x13D18E |
| Compression codes, ten bytes a program (0 L, 2 R, 4 A, 6 B; 0xFF none) | ROM 0x13D302 |
| Programs owned (KeyItems 0x90 + variant, cap 9) | RAM 0x020031C4 |
| The board (7x7) and the placed programs (49 x 8 bytes) | RAM 0x0200414C, 0x02004190 |
| Bug counts, one per type | RAM 0x0200431C |
| The counts rewritten: each reload of MegaMan's stats zeroes them and counts again, as a sub-screen such as a Chip Trader's opens and closes, over a frame boundary at times; with BugStop (MegaMan's stat +0x1F) they stay zero | bn6f applyNaviStatsMaybe_813C458, sub_813CBCC, sub_813C490 |
| NaviCust results (MegaMan's stats) | RAM 0x020047CC |
| Compressed shape flags | event flags 0x2660 + program variant |
| Give a program | text command `EF 1B program amount colour` |

A program is given with the game's own command; writing the counts in RAM
reads as none (the game checks each against a mirror). The details and how
each was found go into `docs/ROM_DATA.md` as the parts are built.

## Status

Built (`src/layer/navicust.c`, the Guardian Data script in
`src/layer/scripts.c`, the bug watch in `src/director/director.c`):

- The pool of 34 programs with their tiers and builds, and MegaMan's
  words for each.
- A draft offers only programs that fit beside those on MegaMan's board
  (read as each layer is made, kept with the run), on the board its
  Guardian Data leaves, each in a colour that fits too: a playtester's
  draft offered SuprArmr, which could not share the 4x4 board with the
  gift's Custom1. The packing (`navicust_pack`) turns each program as L
  and R do and allows no bug. A program left off because it cannot fit is
  named once a board size ("won't fit ... until the board grows"), not on
  every layer.
- The guardian's draft on every normal layer's guardian: three programs
  of three builds, or B for BugFrags (10, and 5 more an act); the run's
  first draft says how the board works.
- ExpMemry at the act 2 and act 4 guardians: the board grows to 5x4, then
  5x5 (checked on the NaviCust screen).
- The bug note: at the RUN itself, the PET still open, a red note over the
  screen names the bug's cause for five seconds ("A bug! Attack+1 is a
  plus part on the command line: plus parts go anywhere else."): BN6's
  RUN lists errors only and says "RUN complete!" over a bug, which a
  playtester read as clean twice (sessions 60 and 61). It follows the
  counts while the PET's pages are open (main mode 0x28), changed from
  what they were as it opened and held ten frames.
- The bug line: as the map is back, before a step, MegaMan names each bug
  that changed and what it does. BN6's RUN says OK over a part left past
  the board's edge, which does bug; the line is the only place the player
  learns it. It follows the board, not the counts: only a change of the
  programs' places (a RUN) or of the ExpMemry makes him speak, from
  counts that held ten frames, since BN6 rewrites them on its own (the
  row above; a player heard the bug explained after every Chip Trader
  trade, issue #25).

- Compression codes kept once entered (issue #50): the profile's codebook,
  Dad's Compression mail, and MegaMan's word on a known code where a
  program would fit compressed; the fit checks read the compressed shapes
  (issue #54).

Next: BugFrags as the vendor's currency with the workshop and the
bugged-battle pay, the vendor's stock from this pool (it still sells
HP+400 at 2100 zenny beside the dealer's 20-HP HPMemory at 1200), then
blue Mystery Data programs, the next act's answer in the draft, then the
profile's programs.
