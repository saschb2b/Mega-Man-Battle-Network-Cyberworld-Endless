---
type: Feature
title: "5. Jobs (the Request BBS)"
description: "Jobs on the Request BBS (`jobs.c`, `director_jobs.c`, `request_bbs.c`, issue #88): three townsfolk post a request each visit on BN6's own Request BBS at AsterLand, Lan takes one at most, the asker pays at the next visit or the courier in Lan's HP in their name, and how vows work."
tags: [world, home]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-23T15:00:00Z }
---

# 5. Jobs (the Request BBS)

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
(paid with a chip a tier up, in the folder's codes) or, from the second
visit on, a vow, no Mr.Prog's patch until the act's guardian falls (two
HPMemory); the
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
BBS's rule). In the Net
the request counts its own act's battles (BN6's and the older net's; a
guardian's counts for none), each layer's Mystery Data as MegaMan leaves
it, a Mr.Prog's patch and the guardian's fall; MegaMan says when one is
done or a patch broke the vow, and reminds of a vow as the act begins.
At the next visit the asker settles it: the reward in BN6's own words
("MegaMan got: ..."), a chip handed over from the Pack (BN6's own count,
which leaves out the Folder's copies), or a word where it was not done;
until then L at home says where the reward waits, and no other request
can be taken. Where more than one kind of chip in the Pack fits, the
player chooses which goes, on BN6's own Chip Trader screen ([The chip
given](#the-chip-given-23-november-2026), below). The run's save holds the request (`RUN_MAGIC` "CWE9"; an
older save continues with none). Not yet: the objects' comps as a busting
errand.

A request settled stays settled (issue #122, from Baine53): the lab's
man paid his 1000 zenny again to a Lan who had jacked in and out. BN6's
jack-in rolls the green Mystery Data of every net map it has and clears
their flags, and four of Underground's are the run's: the lab's request
taken, a request held, one settled, and the visit's order at AsterLand's
counter. Each layer now points every map but its own at an empty list,
BN6's own too ([The net's maps and their pieces](../../rom/net-maps.md), Map takeover).

After the first playtest of home (session 69, 5 October 2026), through
the game-design lens: the vow is press on against prepare, a bonus with
its drawback, and the playtester's bet broke both its terms. It was
blind: taken at a run's first visit, before Lan's HP showed the act's
ways, it met an act in the older net that took his Cross, and MegaMan
was deleted on layer 1. It was dishonest: L and the green arrow led to
the heal it forbade. And it paid one HPMemory, less than a Net Dealer's
800 zenny for one, for an act's patches, two or three halves of max HP
and the run with them. So the board posts no vow at a run's first visit;
a kept vow pays two HPMemory, +40 max HP for the rest of the run; while
one holds, the heal is no heal for L, the arrow or the room before the
arena (L names the vow and the dealer's MiniEnrg), and a Recovery
Mr.Prog's patch asks first ("Patch up anyway?", on No). A vow's reminder
waits for the act's card and arrival words, which it had ended unseen.
When a run ends holding a request, its asker says so at the next run's
first visit (the profile's `last_job`), and L in the town names, once a
visit, the requests posted and an order AsterLand can still make, and
which way AsterLand's door is (a playtester walked round it twice,
session 70). Home
after an act, the PET's Save saves where Lan stands, as the HP's arrival
does, and CONTINUE goes on from there (a playtester's purchase and
request in the town were left unsaved, under "Saves begin on layer 1",
session 70).

## On BN6's Request BBS (26 October 2026)

The course check of 26 October 2026 ([The core loop](../../game/core-loop.md),
Phase 1) found the vow, the best-rated job, behind a walk: the three
posts stood in three places, and the courier said where they waited, not
what they were. A home visit measured 2.5 minutes for one purchase
against a design of 60 to 90 seconds. Through the game-design skill's
interface layer: a usability fault, not an engageability one (the player
could not compare a draft of three without walking it), so the fix is
where the information lives, not what it says. The draft serves home's
dialectic, prepare or press on ([Home's design](design.md)), on the
session loop between acts, and its patterns are a pick of one from three
(each kind's price what it bends) and feedforward: the decision readable
before the walk, so a player who wants none of them presses on.

So all three posts are on BN6's own Request BBS, the board on
AsterLand's wall (`request_bbs.c`), and nothing is redrawn: the board's
check opens BN6's screen, which lists them (the NetBattler's on top), reads
a post out, asks its own "Take this request?" and keeps its own rule, "You
can only pick one request at a time.", while one is held. Each visit the
askers' requests are posted as BN6's requests 0 to 2, asker k's request k
(the one held among them, marked taken), their titles and posts the run's
(`job_words.c`: what it asks, the pay, where the poster waits) in place of
BN6's own (35 requests, scripts 0-34, the rest of its 48 flag slots placeholders), which a run never posts; BN6's taken flag for one is its
asker's Yes, which their script then follows. The askers and the Academy
stay as places (decision 10): each still offers in person, settles where
they stand, and hands a chip over there. The courier in Lan's HP names
each offer's kind and pay, a box each ("3 CLEAN WINS FOR 1000 ZENNY!"),
and L in the town says the posts are all on AsterLand's BBS. A vow taken
on the board starts its question on BN6's Yes; the NetBattler's own
starts on No. The addresses and flags are in [The real world and
home](../../rom/world.md), The Request BBS; the statistics' requests taken
and vows ([The run log](../../devtools/run-log.md)) will say whether the
draft is read more.

## Settled in Lan's HP (issue #841, 23 November 2026)

A player, agergo100, in the discussion "What should the net hold next?":
collecting a reward meant a jack-out, a walk to its asker and a walk
back. The owner wanted it taken in Lan's HP.

Through the game-design skill. The walk restated no tension: a finished
request's pay was already earned, so going for it was never a choice,
only a toll on every visit that had one, and a request not done held the
board shut until Lan walked to its asker for a word. That is a usability
fault (the skill's respect for the player's time), not an engageability
one, and home's own design names it as what home must not be: "a chore
between acts" ([Home's design](design.md)). The town keeps its decisions:
the board's draft of three, the counter, the order, an object's errand.
What is lost is the asker's thanks in person, the town that remembers
([A town that remembers](town-remembers.md)), and that stays for whoever
goes: the asker still settles a request at the visit, and once it is
settled either place, they thank Lan again, or let a request not done go
again, and pay nothing more (the one settled flag, `0x146A`, both scripts
check).

So the courier settles it, in the asker's name. He already relays the
town's posts to Lan's HP ([Lan's HP](lans-hp.md), the courier); now he
relays both ways, which is what a homepage's helper program is for: the
record MegaMan brings home goes out through him, and the asker had left
the pay with him for it. Every kind settles there, as at the asker's:

- **Done** (busting, readings, a vow kept): "DELIVERY,DELIVERY!", the
  asker named, then BN6's own "MegaMan got: ..." and the pay.
- **Not done**, a vow broken among them: the asker's word through him, no
  pay, and the board open for a new request at once.
- **Bring**: the NetBattler's chip goes from the Pack through him ("I'LL
  RUN IT OVER TO ASTERLAND!"), BN6's own take of a chip, and the trade
  chip the NetBattler left with him comes back. With none of the element
  in the Pack, he asks whether to give it up; on No the request is kept,
  for him or for AsterLand later in the visit. Which chip goes is the
  handover's own rule, one script for both places (`job_words.c`,
  `bring_script`). His words' room in Lan's HP grew from 768 bytes to
  1024 (`lanhp.c`, `COURIER_TEXT`): a Bring request with seven chips of
  its element held, and the keys after, took 780.

He stands in Lan's HP while a request is up for settling, done or not,
his "!!" over him; L there names him ("That Mr.Prog has the club's
reward!"), and in the town still names the asker. MegaMan's words in the
Net ("Let's tell them next time we're home!") hold as they are: home is
where the courier hears it. Nothing of `Run` changed: the request, its
flags and the visit are what they were. The scenarios `home_requests`'
`courier-pays`, `courier-not-done` and `courier-bring-kept` keep it,
beside `paid-once` and `failed-pays-nothing`, the asker's own settling.
## The chip given (23 November 2026)

A Bring request took the first chip of its element the Pack held, by
BN6's ID order, and a player saw it take a chip he wanted to keep
(agergo100, issue 842). Through the game-design skill: the request's
price is a chip given up, and a price the game picks is a tax, not a
choice; the player who chooses pays with the chip he needs least. It
serves home's dialectic, prepare or press on, on the session loop
between acts, and its patterns are error prevention (nothing given up
unasked) and feedforward (the chips that fit shown, and only those).

So where two kinds or more of the element's chips are in the Pack (a
chip in a code is a kind; copies of one are one kind), the NetBattler
says "Whoa,you've got a few!" and opens BN6's own Chip Trader screen for
one chip, its counter "0/ 1", over the Pack's chips of his element alone
(`src/world/bring_choice.c`): BN6's list, its card, its SORT, its put-in
and take-back. With the chip put in, MegaMan asks "Lan,trade this one?"
in the box where BN6's machine asks "Add these 3?"; Yes leaves the screen
with none of BN6's trade done (no prize, no save), and back on the map
the NetBattler takes that chip with BN6's own command, as before, and
pays. B out of the screen takes nothing: "I'll wait,then!", and the
request stays held, offered again at the next talk. One kind alone goes
as before, with no screen: a choice of one is no choice. The courier in Lan's
HP, who settles a request in its asker's name ([Settled in Lan's
HP](#settled-in-lans-hp-issue-841-23-november-2026), above), offers the
same screen ("YOU HAVE A FEW!", "WHICH ONE SHOULD I TAKE HIM?"), and
hands over or keeps the request in his own words after it. The Folder's
copies stay out of it, as before: BN6's Chip Trader lists the Pack, and
the hand-over's check counts the Pack's alone. The screen is the trade
his offer names ("I'll trade you my ...!"), so its title is no stranger
there. How the screen is driven is in [The real world and
home](../../rom/world.md), The Chip Trader's screen as a Bring's choice.
