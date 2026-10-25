---
type: Feature
title: "5. Jobs (the Request BBS)"
description: "Jobs on the Request BBS (`jobs.c`, `director_jobs.c`, issue #88): three townsfolk post a request each visit, Lan takes one at most, the asker pays at the next visit, and how vows work."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-18T21:46:19Z }
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
