---
type: Feature
title: "A visit"
description: "A visit to the Net Square: the gate in Lan's HP and its first questions (`director_gate.c`, `online_field.c`), the checkpoint at the gate, the Square built on the game's own core (`online_visit.c`), how visits end."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-24T07:19:34Z }
---

# A visit

**The gate.** In Lan's HP (`lanhp.c`) a white gate stands at the room's
front, a Mr.Prog beside it. The first step on it asks for the name other
players see, says what the server learns (the PET's address, that name,
MegaMan as he is) and asks before going, the cursor on No; after that
MegaMan asks "To the Net Square,Lan?" each time
(`director_gate.c`, its words in `gate_words.c`; flags `0x1473`-`0x1476`,
[Running the game's own logic](../../engine/emulation/overview.md)). The keeper changes the name or the server:
the official one, or another's address, typed once in the one screen
of the Square's own, a field with a keyboard on the pad and by touch and
the system's where there is one (`online_field.c`, `text_input.c`). What
is typed becomes the server's URL (`network_address.c`): a scheme or a
port typed is kept; an address that can only be on the player's own
network (a private or Tailscale's IPv4 address, localhost, a `.local` or
`.lan` name, a name with no dot, a ULA or link-local IPv6 one) is a
Square started there with nothing set, `ws://` on port 8765, so
`192.168.1.20` alone reaches it; any other is `wss://` on its own port.
The lens (the game-design skill, interface: error prevention and minimum
workload, Hodent and Nielsen): typing on a pad is friction nobody meant,
so the field asks for no more than the host's server names as it starts;
an explicit scheme or port always wins, the way out where the guess is
wrong (a home server behind its own TLS on 443). Loop layer: the visit
between acts; nothing of a run changes. Yes saves the run as a checkpoint at the gate
(`home_gate_save`), then the gate flickers while the server answers;
B calls it off.

**One core.** The visit runs on the game's own core
(`online_visit.c`): the Square is built into the running BN6, the run's
state (its `Run`, profile, RNG and loot) put aside, and the Square's RAM
thrown away on the way home. Coming home puts the state back and
`director_resume()` re-enters Lan's HP on the live core, MegaMan on the
gate as he left it; nothing is booted again. A game closed in the Square
continues from the gate's checkpoint, as any CONTINUE does. Every end has
a word for it at the gate: left, called off, no answer, a newer version
out (the keeper names the game's download page), a server older than the
game, another ROM, a name taken, a full Square, barred, removed, the
Square under maintenance (with the host's line), the connection lost (a
dropped link that did not come back, or B on its wait), another Square
after a dropped link came back (a server restarted with another seed).

## Contents

- [A dropped link](dropped-link.md)
- [The Square's design](design.md)
- [A hub and its parts](hub.md)
- [The layout](layout.md)
- [Its life](life.md)
- [Emotes](emotes.md)
