---
type: Engine Mechanism
title: "A NetBattle through a dropped link"
description: "A NetBattle of two that survives a dropped link (issue #180, protocol v21): the server holds the battle with the place, each game keeps its keys and asks for the ones the link lost (ARENA_HASH's 0xFFFFFFFC), the battle fought to its end with every hash agreeing."
tags: [online, arena, netcode]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-22T01:00:00Z }
---

# A NetBattle through a dropped link

Issue #180's first slice (the owner's direction of 21 November 2026: a
hiccup of the link must not end a session). Before it, a NetBattle ended
the moment either game's link dropped: the game broke its battle link as
soon as its transport said it was down, and the server ended the battle
when it noticed the drop, though both kept the player's place in the
Square for 40 seconds and the transport took it back ([A dropped
link](../square/dropped-link.md), issue #153). Now a NetBattle of two goes
on through the drop, on every system that has NetBattles (the desktops,
Android, iOS, the browser, the Switch, the Vita, the PS4 and PS5; the New
3DS and the PSP offer none): the same battle, the same frame, nothing
redone.

**What each part does** (protocol v21, `shared/online/README.md`):

- *The server* (`arena.py`'s `dropped`, `holds.py`) holds a NetBattle of
  two under way with its player's place: the other game is told
  nothing, and what it sends meanwhile is lost with the link. A resume
  takes the place back and the battle with it (the same battle ID, side
  and delay; the relay finds the player by his session ID, which the
  resume keeps). The place freed after its 40 seconds ends the battle for
  the other; an end that came while he was away (the other left, the
  battle over) is sent again after his resume's WELCOME. A team bout, a
  2v2 and a party's battle still end at the drop.
- *The game's battle link* (`arena_link_net.c`) keeps the keys it sent
  (512 frames) and sends from the first not yet in the outbox, so an
  outbox that refuses them while the link is down only holds them back.
  A stream that comes with a gap is no longer broken: the keys past the
  gap are dropped and the first missing asked for again (ARENA_HASH's
  frame 0xFFFFFFFC, `ARENA_NET_HAVE`: the frame, and whose seat's), at
  most every half second while the gap lasts; keys that come again from
  frames already taken are taken from the first new one.
- *The game in the Square* (`square_arena.c`'s `through_a_drop`): while
  the transport tries again, nothing goes in the outbox and the battle
  waits on the other's keys as on any slow link (the link's badge, then
  the stall's note: [The way in, the battle and the
  result](from-square/way-in.md)). Back in the same session, it sends its
  own keys again from the frame after the other's last hash, all the
  other surely had (a game sends a frame's hash only once it has run that
  frame with both players' keys), and asks for the other's from the first
  it lacks. Back in another session (a server started afresh, which
  holds nothing), the battle stops as before.

The stall's end, 20 seconds of waiting, is unchanged and still bounds
the drop: the transport notices a dead socket at once and a silent one
after 8 seconds. While a battle waits on the link (a NetBattle's or a
party's field), the transport tries again every half second
(`online_transport_hurry`, said each frame of the wait), not after half
a second, a second, two and four as in the Square, so the wait is the
drop and at most half a second more. The other player can leave the
wait with B once it offers it, as ever.

**A party's field and a crew's battle** (the shared dive's battles,
`party_play.c`, `party.py`): the same ride through a drop, before the
owner's rule for a drop (the missing MegaMen leave the field, each game
finishing its own: [The shared dive](../shared-dive.md)) takes over. The
server holds the dropped diver's seat 12 seconds (`SEAT_HOLD`) rather
than telling the others it is gone: their fields wait on his keys as on
a slow link, and his resume within them takes the seat back, what was
said to him meanwhile said again (a battle over meanwhile: every other
seat gone, then its end, so his game finishes his own). His own game,
its link down, puts nothing in the outbox and waits; back in the same
session it sends its keys again from the others' last hash and asks for
theirs, as a NetBattle's does. Down past 10 seconds, or back in another
session, it lets the others go and fights on alone, and says its leave
once the link is back so that theirs need not wait out the hold. Before
this, a party seat whose own link dropped kept its field waiting with no
end once the server had let its seat go (#722 had made its link keep
keys rather than break at once).

**The diver's room after the link is back.** A dropped diver's place is
held in no room: the server takes him out of his layer's room and his
crew at the drop (`dive.py`'s `dropped`). His game now enters the room
again as soon as its resume is done (`dive_link.c`: the transport's
resumes counted, the room it was in forgotten so it is entered anew,
the crew it showed dropped until the server's next word), so the other
divers see him again, his crew forms again by nearness and a crewmate's
battle is joined as before. Before, the game kept the room it was in and
entered none until its layer changed: after a reconnect the other divers
and his crew were gone from his screen, and he from theirs, for the rest
of the layer. The scenario `online-week/crew-again-after-a-drop` cuts
the second diver's link for three seconds on the layer.

**A dropped link on a dive layer** (issue #751). A layer goes on while
the link is down; the Square's held wait would stop the run, so the dive
says it in its band instead, in the PET's plain status as the Square's
wait words it: past a second down, `Reconnecting...`, its dots moving,
until it is back (`Back online!`); the crew's plates and the other
divers stand as they were meanwhile. Before, nothing was said, and past
20 seconds the dive closed its link for the rest of the dive, never
trying again. Now, once the transport's own 30 seconds of tries are
over, the dive starts the link again itself (`online_transport_again`,
its HELLO with the resume bit, so a place the server still holds is
taken back, else a new one) after 3, 6, 12 and then every 15 seconds,
as long as the dive lasts; the band meanwhile says he dives alone
(`Diving alone  Reconnecting...`). Back, by a resume or a new session,
the room is entered again and the crew's plates wait for the server's
word. Only the server's own refusal (another version, maintenance, a
ban: an ERROR with its code) ends the link for the dive, and the band
says so plainly first, with how the next dive links (`Offline. Diving
alone this time`, `The Board's portal links the next dive`). The
scenarios `online-week/link-down-5s`, `-30s` and `-120s` cut the second
diver's own link on a layer for that long; each sees the other again,
their crew forms again and a crewmate's battle is joined after it.

**Tested.** `tests/test_arena.c` plays two network links back to back
through a pipe that loses both ways' messages for three ticks, then
holds one side's outbox for the rest of a 40- and a 180-tick drop, then
resumes it: every frame either side runs sees both sides' true keys, and
the asks and resends are counted. `server/tests/test_holds.py` holds a
battle through a drop, relays both ways after the resume, ends it with
the hold and says a missed end again. The scenario
`online-arena/link-drops-mid-battle` cuts the second game's own link for
four seconds in the middle of a NetBattle (the dev step `reconnect cut
4`, [The Net Square: two games at
once](../../devtools/net-square.md)): both games fight it to its end, the
result on both cards.

**Measured** (21 November 2026, this machine, the host build headless,
BN6, BN5 and BN4, two games through a local server, `arenaboth`,
`arenadelay=6`, Alpha 200 HP, Beta 5):

| Build | Beta's link cut for 4 s after battle frame 120 | Alpha | Beta |
| --- | --- | --- | --- |
| before (`dev` with the cut step alone) | the battle stopped on Beta's game at frame 131, "the connection to the Square was lost"; the server ended it at the drop | stopped, no result | stopped, no result |
| after | held 7.4 s (the cut and the transport's tries at 0.5, 1, 2 and 4 s), then fought to its end | won, over at frame 570, 445 ticks stalled, 9 hashes agreed | lost, over at frame 570, 439 ticks stalled, 9 hashes agreed; 14 frames sent again from 120, one ask, answered with 10 of Alpha's |

With the tries every half second (21 November 2026, the same run): the
same cut waited 230 ticks on Alpha's game and 232 on Beta's (3.8
seconds), over at frame 664 on both with 11 hashes agreeing; a 4-second
cut now costs a 4-second wait.

The 445 ticks were the whole wait: the cut's 4 seconds and the 3.5 more
the transport's backoff took to try again (its fourth try came at 7.5
seconds). No rollbacks either side (delay 6, window 8), the same hashes
on both, the server's log `arena_held`, `hold` and then `resume`. The
other five `online-arena` scenarios pass as before.

A crew's battle (the scenario `online-week/crew-battle-through-a-drop`:
two divers' crew fighting Alpha's random battle on one field, Beta's own
link cut for 3 seconds at field frame 133):

| Build | What happened |
| --- | --- |
| before (`dev` after #722) | Alpha's field let Beta go at frame 133, as the link dropped (the server's seat gone), and fought on alone; Beta's froze at frame 141 waiting on keys that never came, his resume 3.5 s later changing nothing, his game held with no end |
| after | held: the server's `party_held`, Beta's resume 3.0 s after the drop; Beta's keys sent again from frame 121; both fields on together, every seat's hash agreeing after it (7 on each, to frame 420) |

**A team match and two a side.** A team, its match and its bout under
way are held with the dropped player's place too (`teams.py`'s
`dropped`, `arena.py`'s `holdable` taking a team bout and a 2v2, the
first also before its START): the others' games wait on his keys, and
his resume takes all of it back, the last TEAM_STATE meant for him said
again, so his game goes into a bout called meanwhile or watches the one
under way again (`square_team.c`: a watch the drop ended watched anew).
A 2v2's four streams ask and resend as a NetBattle's two. His place
freed after its 40 seconds breaks the match off as a leave; before that
a battle's own stall (20 seconds) or a bout's accept (30) bounds the
wait. The scenario `online-team/side-by-side-through-a-drop` forms two
teams on four games, fights a 2v2 and cuts a partner's link for three
seconds.

| Build | Delta's link cut for 3 s after battle frame 120 (two a side) |
| --- | --- |
| before (`dev`) | Delta's battle stopped at once, "the connection to the Square was lost"; the other three's at frame 142, "the other player left the NetBattle"; the match broken off for both teams (result 3) |
| after | all four waited 170 to 177 ticks (2.8 to 3.0 s); over at frame 605 on all four, Alpha's team won, 9 or 10 hashes agreed on each, none apart; Delta's keys sent again from frame 120 |

A relay's waiting partner through a drop is held and told the last word
again (`server/tests/test_teams.py`); his game watching the bout again
after it is not yet seen in a scenario.

**Not yet.** A server
restarted mid-battle (it holds nothing; the battle stops as before); the
stands through a drop (a spectator's keys stop at the first gap the
server saw; with the resend from the last hash the server sees none in
the scenario's runs); a drop on a device's real network rather than a
socket closed by the game.
