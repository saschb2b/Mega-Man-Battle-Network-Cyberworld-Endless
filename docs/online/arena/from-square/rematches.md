---
type: Feature
title: "Rematches and the referee"
description: "Rematches from the result card (issue #167): Again with NAME?, both Yeses needed, sent as ARENA's challenge answered at once (`square_arena.c`), and the referee by the ring calling bouts and keeping the count."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T21:51:43Z }
---

# Rematches and the referee

A battle with a result (issue #128) is offered again on its card (issue
#167): "Again with NAME?", Yes and No side by side with the cursor on
No, while NAME is still in the Square. Nothing is won or lost, so the
question is the whole cost: the default lets either player go (the
Square's pull between staying together and moving on), and both must
say Yes. Each sees the other's answer as it comes: "NAME wants another!"
once his Yes is in, "Waiting for NAME..." under one's own, "No rematch
with NAME." where he said No or left. B, or A on No, goes back to the
Square; the card's answer is kept 45 seconds from the battle's end
(`AGAIN_MS`), then it goes back by itself.

No new message: a rematch is ARENA's challenge again, answered at once.
Lan's Yes sends it quietly; the other game, its card still up, holds a
challenge from that player and shows it, takes it at once where its
player says Yes (or said it first), and turns it down where he said No
or closed the card. Both Yeses at once cross on the server, which turns
the second challenge down as busy; the first is taken (`again_in`,
`square_arena.c`). Both Yes: straight from the card into the next bout,
gone into from the last bout's pair as it began (protocol v13: [A rematch
in a second](rematch-in-a-second.md), below), the panel showing the set's
score. The folder carried is the last battle's, asked in the Square only
where it no longer fits. A Yes nobody answers ends as any challenge does,
after the server's 30 seconds. A battle stopped offers none.

The referee Mr.Prog by the ring keeps what this game saw: before any
bout he tells how a NetBattle is asked for; after one, who beat whom
("LAST BOUT: YOU BEAT ALPHA!", "LAST BOUT: ALPHA BEAT YOU!", a bout
broken off "LAST BOUT WITH ALPHA BROKE OFF!") and, from a second bout
with the same player on, the count ("THAT'S 2 TO 1. YOU LEAD!", "1 TO
1! ALL EVEN,ALL EVEN!"). As MegaMan comes back he calls the bout over
his head ("YOU WIN!", "ALPHA WINS!", "BROKE OFF! NO CALL!"), as the
greeter calls a log-in. A new opponent starts the count again; it is
this game's alone, kept while the game runs.

**Seen** (21 October 2026, two host builds through a local server, BN6
and BN5 in both, `--dev quiet,arenahp=10,arenaboth`): Alpha challenged,
the panel, the battle (over at frame 529, 8 hashes agreeing), both
cards; Beta's Yes on his, Alpha's card showing "Beta wants another!",
Alpha's Yes took the held challenge (`Beta's rematch taken (match 2)`),
the second way in (the same pair hash at the Custom screen) with "Set
1 - 0" on its panel, the second battle over at frame 1,323 (22 hashes
agreeing), the cards at 1 - 1; then Alpha's Yes and Beta's B: Alpha's
card "Waiting for Beta..." then "No rematch with Beta.", the referees
calling bout 2. A game held still mid-battle (`arenafreeze=400` on
Alpha, its link kept): Beta's note from a second, "B: leave" from ten,
the stop at twenty ("Alpha's PET stopped answering.", BROKE OFF), Alpha
"Beta left the NetBattle."; with B pressed (Alpha's process stopped):
Beta left at 648 ticks ("We left the NetBattle."); both referees "BROKE
OFF! NO CALL!" with the count 0 - 0.
