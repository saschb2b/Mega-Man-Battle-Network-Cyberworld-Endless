---
type: Feature
title: "The way in, the battle and the result"
description: "The NetBattle's PET screens (issue #167): refusals in MegaMan's words, the way in's panel (`square_card.c`), the name plate and the bars-only link badge with its stall notes (`arena_view.c`), and the result card."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T14:31:15Z }
---

# The way in, the battle and the result

Issue #167's first two steps: the NetBattle told on PET screens from the
first word to the last, nothing added to BN6's own Custom screen wait
("Cstmzing."), its turn banner or its win banner. The design's lens is the
Square's pull between staying for one more bout and moving on, with
nothing at stake: staying is made cheap, leaving clean.

**Refusals.** A challenge the server turns down is said by MegaMan in the
chat box, never the corner's notice: the target busy (NOTICE 16: in a
NetBattle, watching one, at Trade Central, his place held) "Huh? NAME's
busy right now...", no longer here (17) "Huh? NAME isn't here
anymore...", the Arena closed on the server (18), a NetPass without the
Arena's stamp (28) (`square_arena_notice` keeps the notice out of the
corner, `refused`). A refusal that comes while the Square is held (a
rematch's challenge sent from the card) shows on the card, and never in
the corner afterwards.

**The way in** (`arena_card_way`, `square_card.c`): from white into a
PET Network panel in the project's PET frame (`second_frame.h`'s colours
and slots, drawn here, never taken from the ROM): NETBATTLE and
Practice in the band; a plate for each player with his name, his
MegaMan's HP, his Crosses and Souls as carried (the other's once his
MegaMan has come); under each plate whose wait it is: "Sending..." while
the other's MegaMan is on its way, my PET's bar as it links, "Linking..."
blinking on the other's side until his ready comes, "Ready!" on each that
is; and the set's score with this player ("Set 1 - 0") from the second
bout on. The panel opens with BN6's window sound and goes into the Custom
screen through white with its encounter sound (`arena_card_cue`).

**The battle** (`arena_view_over`, `arena_view_stall`): on the Custom
screen only, the other's name on a small plate under BN6's own label of
the other MegaMan (both are "MegaMan" there), the engine's small letters,
gone once the turn starts. Beside it, and at the top right corner once
the Custom screen closes, the link's badge: bars only, no number (the
owner's decision, 21 October 2026): three green bars steady, two gold
bars where a guess was undone or a moment waited in the last second
(`ARENA_LINK_SHAKY`), one red bar blinking while the battle waits on the
other's keys (`ARENA_LINK_STALL`, ten ticks in a row; `arena_link_state`).
A wait past a second (`STALL_NOTE`) names whose PET it waits for
("Waiting for NAME's PET...") over a bar that runs down to its end; past
ten seconds (`STALL_OFFER`) "B: leave the NetBattle" is offered (B
before that may be meant for the Buster); at twenty (`STALL_END`) the
battle stops by itself ("NAME's PET stopped answering."). A server that
drops a silent game's link first ends it sooner ("NAME left the
NetBattle.").

**The result** (`arena_card_result`): after BN6's own banner and fade,
a card rises from black: WIN, LOSE or OVER, "VS NAME", the HP each side
has left and the chips each used (counted as the stands' report counts
them, from the frames whose keys are all known: `arena_tally`), the time
fought (BN6's fight, the Custom screen left out) and the set's score. No
Busting Level, no reward: those are a virus battle's. A stop shows on
the card too, in the PET's words, the cause in the log alone: BROKE OFF
for a bout begun ("NAME left the NetBattle.", "Communication error! The
two PETs saw the battle apart.", "We left the NetBattle."), STOPPED for
one that never began ("NAME's PET never got ready.", "NAME's MegaMan never
came."); MegaMan's word follows in the Square. A broken-off bout is
counted neither won nor lost: the referee calls it ("BROKE OFF! NO
CALL!"), the Board says "broke off.", and the set's score stays. A stop's
card goes back by itself after ten seconds; A or B before.
