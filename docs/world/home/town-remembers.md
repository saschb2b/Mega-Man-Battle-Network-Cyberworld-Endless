---
type: Feature
title: "7. A town that remembers"
description: "A town that remembers (issue #87): the plaza Mr.Prog calls the Net's news, four townsfolk speak of the run (`town_words.c`), and people stand elsewhere at each visit (`town_folk.c`)."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-05T23:06:53Z }
---

# 7. A town that remembers

- **The run**: people speak of the last guardian deleted, the act,
  MegaMan's HP, the Cross he brought, the threat. One line each, BN6's
  voice ([Voice](../../voice/rules.md)); returning players hear less.
- **The runs**: how the last run ended, the best layer, a guardian beaten
  often ("Back for SpoutMan again, Lan?").
- **Crowds that move**: people stand elsewhere at each visit, and more of
  them walk (sprites with all four walks, [The overworld](../overworld.md)).
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
