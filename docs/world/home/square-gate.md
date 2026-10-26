---
type: Feature
title: "11. The Net Square's gate"
description: "The Net Square's gate (epic #120): going online at BN6's white teleporter in Lan's HP, beside a Mr.Prog keeper (`lanhp.c`, `director_gate.c`), not from a menu; consent, name, the NetPass strip, servers in `network.ini`."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-20T23:52:02Z }
---

# 11. The Net Square's gate

Going online is a place in Lan's HP, not a menu (epic #120; the owner,
PR #126: "no new menu screens where the world can do the job", and Lan's
HP the place for it, after Warframe's Orbiter, whose stations route the
player). It was the PC's third choice (Jack in / Network / Cancel) and a
Network screen of its own; both are gone.

**The station.** BN6's great white teleporter (sprite list 7's `0x06`,
its core lit in animation 0 and dark in 2) stands on the free floor at
the room's front (world 16, 72: the band from y 48 to 96 between the
front link squares and the edge holds nothing of BN6's), drawn at layer
priority 2 so the room's floor lines stay under it; its keeper, a
Mr.Prog (list 6's 61), stands beside it facing the room. The blue pad
docks, the pink pad and the links lead on and back, the gate leads out:
one room, its stations each a way, as the Orbiter's are. Built in
`src/map/lanhp.c` (the gate, its keeper) and `src/director/director_gate.c`
(what they do), their words in `gate_words.c`.

**Stepped on**, the gate asks first, in BN6's chat boxes:

- the first time a server is used, the keeper says plainly what its
  server sees (the PET's address, the player's name and MegaMan as he
  is) and that nothing there changes the run, then asks "SO! GO
  ONLINE?", the cursor on No;
- with no name chosen yet, he asks for one first: the name field opens
  over the game, then his question;
- after that MegaMan asks "To the Net Square,Lan?" each time.

While he asks, the NetPass's stamps show in a strip over the top of the
screen: the pass's level ("Square pass", or "★ Full pass") and its four
stamps, Square, Arena, Trade and DeepNet, each held or open (issue #131,
[The NetPass](../../online/netpass.md)). The player sees what this visit opens
before saying Yes: with BN6's ROM alone the Square, with BN5's beside it
NetBattles, trades and the week's dive too. It is the pass Lan's PET
opens ([The PET](../../game/pet.md#the-netpass), The NetPass), the honest feedforward of the
game-design skill: no character says which game earns a stamp, and the
strip asks nothing of its own.

A Yes saves the run where MegaMan stands (the write confirmed: a failed
save keeps him home, the game's note "Run not saved" and MegaMan's "Let's
stay home for now,Lan.", no character naming a save), and the
visit begins (`src/online/online_visit.c`): the gate's core flickers
while the server answers, MegaMan held; B calls it off. The Square is
built into the running game and MegaMan warped there; the way back loads
the gate's checkpoint as CONTINUE does, no new boot, and he stands on the
gate again, the gate asking anew only once he steps off it and on. What
the visit ended in is said there: "We're home,Lan!", or the keeper's word
on a server that did not answer, runs another version, is full, or turned
the name down, never the server's own text; and a trade the visit made
is the run's now ([Trade Central](../../online/trade-central.md)).

**Talked to**, the keeper says where the gate leads, as a place ("IT
LEADS TO THE OFFICIAL SQUARE!", or "TO ANOTHER SERVER'S SQUARE!", never
its address, `--dev online`'s server counted), and sets where it leads
and the player's name: OFFICIAL SQUARE, ANOTHER SERVER (its address in a
field: host or host:port, `wss://` and `/presence` added), MY NAME. The
cursor starts on the first, so OFFICIAL SQUARE asks before it changes
the gate ("THE OFFICIAL SQUARE INSTEAD?", the cursor on No) and says it
leads there already where it does: one A too many while paging his
words had switched the server unasked (session 74). The two fields are
the only screen of its own going online has (`src/online/online_field.c`,
in the PET's colours, a keyboard on the pad and by touch, the system's
own keyboard and paste where there is one; on a phone held upright the
field stands whole in the screen's upper part, above the system's
keyboard). Settings live in the data
folder's `network.ini`: the name and whether it was chosen, the servers
(the official one first, then the last address typed), which one the
gate leads to, each one's consent, and the private identity each
server's public one is derived from.

There is no going online without a run: a new run starts in Lan's HP, so
a player with no run in progress starts one and walks to the gate.
`--dev online=URL,onlinename=NAME` points the gate at a server for a test
and skips the keeper's questions ([Dev tools](../../devtools/overview.md)).
