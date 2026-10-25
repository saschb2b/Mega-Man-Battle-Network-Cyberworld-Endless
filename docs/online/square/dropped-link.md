---
type: Feature
title: "A dropped link"
description: "A dropped link in the Square (issue #153, protocol v10): the game holds under a Reconnecting panel (`reconnect_draw.c`), the server keeps the place 40 seconds (`holds.py`), maintenance and version answers, and the seeded Square's plan."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-23T14:25:12Z }
---

# A dropped link

[Issue #153](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/153),
the owner, 21 October 2026: "I just had a slight internet hiccup and got
jacked out. A small wait time with me proceeding online would feel
better." A link that drops in the Square no longer sends MegaMan home.

**The design** (the game-design skill, interface: guidance and feedback,
transitions). The dialectic is the Square's own, gather against go: a
hiccup is neither the player's choice to go nor the server's to send him,
so it should cost neither the place he stood in nor the people round him.
The patterns: a wait over the frozen world rather than a screen of its
own (the player sees where he will be put back; a short load's spinner
over the world, McBride-Charpentier); the time left only past a few
seconds, as a load's bar is; an emergency exit always shown (B home), so
being held is never helplessness (Nielsen's user control and freedom);
the return said in a moment, a band and not a dialog, as a player on a bad
link may see it often (the frequency budget of transitions). Loop layer:
the session between acts; nothing of a run changes.

**The game** (`online_transport.c`, `online_scene.c`, `online_visit.c`,
the screen `reconnect_draw.c` and its PET text `reconnect_text.c`). Once
the server has welcomed the visit:

- a link that has said nothing for 2.5 seconds (`ONLINE_QUIET_MS`;
  snapshots come 20 times a second, PONGs every two) is **quiet**: the
  game's core holds still (`online_visit_waiting`, in the frame's place,
  as a NetBattle holds it), and the wait is drawn over the Square, dimmed:
  a PET panel in BN6's PET look, "Net Square" on its band with B:GO HOME
  at the band's end ([The PET's screens: Battle Network's rules](../../engine/ui.md)), a signal searching, "Reconnecting...",
  "YOUR SPOT IN THE SQUARE IS KEPT", from five seconds on a bar of the
  time left.
  A link that speaks again within its limit goes on with nothing lost:
  "Back online!" a moment in the band, and play goes on;
- a link silent past 8 seconds, closed, or broken **without the
  server's ERROR** is given up and tried again: a new socket and a
  resume's HELLO (the channel MegaMan stood in, protocol v10's resume
  bit), after 0.5, 1, 2, then every 4 seconds, for 30 seconds from the
  moment it went quiet or dropped (`ONLINE_RECONNECT_MS`: long enough for a
  phone's Wi-Fi to come back or hand over to its mobile data, short
  enough that nobody waits on a player who is not coming back). Every
  system the same: a worker's thread on the desktops, the handhelds,
  Android, iOS, Windows and the 3DS (their sockets' own timeouts bound a
  try, 5 to 7 seconds), and the browser's WebSocket from its frame's poll,
  no wait on the frame;
- the resume is done at its WELCOME and the first SNAPSHOT after it: the
  same Square (the seed the Square was built from) and MegaMan where he
  stood, his card sent again, and what came in from the last session and
  was not yet read set aside. A WELCOME with another seed (a server
  restarted with another) ends the visit, MegaMan's "Whoa! The whole
  Square changed!"; a refusal (another version, full, barred,
  maintenance) ends it with its word, as at the gate; past the 30
  seconds, "Whoa! The link to the Square broke!", as before. B on the
  wait gives up at once, the same word;
- what was under way ends as on any leave: a NetBattle stops without a
  result ("the connection to the Square was lost", then the Square, held
  under the wait if the link is still down), watching stops, the Arena's
  question and Trade Central's trade are over (the server's word on the
  trade comes again with the resume: below; on a new session, a server
  restarted, the game calls an unfinished trade off itself). Nothing
  meant for the old session is sent on the new one;
- leaving with the link up says goodbye (BYE), so the server frees the
  place at once.

While the link is quiet the game sends its place twice a second rather
than twenty times, so what a stalled link lets through at once when it
clears stays small.

**The server** (`server/cyberworld_server/holds.py`). A session that
ends without a goodbye (BYE or a WebSocket close), its socket gone or
silent for 15 seconds, keeps its place for **40 seconds**: its slot in
its channel, its session ID, and its MegaMan standing where he stood in
the others' snapshots, so nobody sees him leave and come back. Forty: the
game's 30 seconds of tries, the last try's handshake, and the server
noticing a drop sooner than the game (a reset) or later (its 15 seconds of
silence). A resume's HELLO from the same identity takes the place back
(the same session ID and channel; the "resume" in the server's log, not a
"join" nor one of the day's joins), and the session's last trade end,
done or off, is said again to it: a trade committed as the link dropped
still comes home. A trade, a challenge or a battle aimed at a place held
is answered "That Navi is busy". A goodbye, a refusal or a moderator's
removal frees the place at once; a new visit from the gate (a HELLO
without the resume bit) gives it up and arrives anew. Positions have a
bucket of their own, 200 deep, so a stalled link's backlog is no flood.

**Maintenance.** `maintenance = until 14:00 UTC` (or any line of up to
40 printable characters) in `server.ini` turns every game away with ERROR
16 and that line, and `/health` says it (`"maintenance"`): the gate's
keeper says "SORRY! THE SQUARE'S UNDER MAINTENANCE!" and "THE SIGN ON IT
SAYS, UNTIL 14:00 UTC" rather than "no answer"; a visit whose link
dropped as the server went down for it comes home with the same word.
A game newer than its server is answered ERROR 17, and the keeper says
the server is behind; an older game hears "A NEWER VERSION IS OUT!" and
"UPDATE YOUR PET AT THE DOWNLOAD PAGE!". Before v10 neither could happen:
the WebSocket's subprotocol names the version, and the server refused
another at the upgrade, so a game of another version only ever heard "no
answer". The server now lets another version's subprotocol through to its
HELLO (seen with a copy of the server speaking v11: the keeper's two
boxes, and the server's log "Protocol version mismatch", code 2).

**The Square.** The server sends a seed; each game plans the same Square
from it (`square_plan.c`, `SQUARE_MAKE` 3) in one of BN6's busy daylight
nets (Central, Sky or Green Area; not Seaside, whose layers stand in
another map's slot). Whatever the area, the Square plays its own music:
BN6's Navi Customizer theme (song 0x1E, `BN6_SONG_NAVI_CUSTOMIZER`, the
owner's pick, 20 October 2026), not the area's. The plan's hash and its generator version
(`SQUARE_MAKE`, `SQUARE_HASH`) go in HELLO, so a server refuses a game
that would build another Square. `online_square.c` builds it with the
layers' map builder (`netmap_build`) and gives the other players'
MegaMen seven NPC scripts of BN6's own, moved from a 100 ms
interpolation of the server's 20 Hz snapshots; their names stand over
them. MegaMan walks as BN6 walks him. Each Square is a channel of eight;
the keeper moves MegaMan to the next or the last.
