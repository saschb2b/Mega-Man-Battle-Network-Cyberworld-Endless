---
type: Feature
title: "A partner's own Custom screen"
description: "A partner's own Custom screen (`arena_team_custom.c`, `arena_team_screen.c`, `arena_team_client.c`): his deck dealt in core RAM, BN6's screen run on a plain copy of the pair on his client, only his picks crossing."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T16:53:46Z }
---

# A partner's own Custom screen

A partner picks his chips on BN6's own Custom screen, from his own deck
(the owner's direction, 21 October 2026: BN6's screen, never one of
ours), and only his picks cross (22 October 2026; `arena_team_custom.c`,
`arena_team_screen.c`, `arena_team_client.c`):

- **His deck**, the pair's, every client's and spectator's alike: his
  folder (a carried MegaMan's where one is given,
  `arena_team_custom_folder`; under the dev flag his side's lead's, as his
  side's GBA holds it) dealt as BN6 deals a side's, the Regular chip
  first, the TagChips after it, the rest shuffled (our shuffle, seeded by
  the battle's random number as its first Custom screen opens, in the
  way in: each GBA deals it on its own at that frame, the same on both,
  `D07D8E18` in the runs below), a chip that sits out of BN6's battles
  left out. It lives in each core's RAM (`RAM_TEAM_CUSTOM`, docs/
  [Running the game's own logic](../../../engine/emulation/overview.md)), so a rollback puts it back and the pair's hash covers it;
  the folder itself, which no core holds, is folded into the pair's hash.
- **His screen, on his own client alone**: a partner has no GBA in the
  pair. A lone core taken from his side's GBA at a Custom screen will not
  do: within frames BN6 sees no other GBA on its cable and Lan's box says
  "Communication error" (seen). So his client makes a plain copy of the
  pair (`arena_pair_open_plain`: both GBAs on their own cable, BN5's
  bytes in their ROMs, no hook on them, never hashed, never on a
  spectator's client; where he holds Souls, his side's GBA there with
  BN5's installs' hooks for UNITE: [A partner's UNITE](unite.md)),
  once, as his side's GBA first opens its Custom screen. Each turn the pair is copied into it (`arena_pair_save`,
  `arena_pair_load`), his deck is written into BN6's battle deck on his
  side's GBA there and the Custom screen's phase step put back to 0
  (`BN6_BATTLE_STEP`): the next frame BN6 sets its Custom screen up anew
  from his deck (bn6f sub_8026840: his hand dealt, its icons loaded) and
  runs it on his keys, by its own rules (the same code or the same chip,
  five at most, Atk+ chips); the other GBA there holds still
  at its own. His client shows that copy's picture while his side's
  GBA's screen is open, the battle behind it the battle's own as the turn
  began.
- **Only his picks cross**: at his OK (`BN6_CUSTOM_MODE` going to
  `BN6_MODE_OK`) his client reads the hand's slots he picked, in order
  (`BN6_CUSTOM_PICKS`), and sends them in his stream as pick messages,
  one a frame (`TEAM_PICK`: bit 15 set, the count, the index, the slot;
  past the GBA's ten buttons, so a partner's stream keeps its sixteen
  bits, `ARENA_PARTNER_MASK`). Since the leads wait for him (below), his
  side's GBA's screen closes before his OK only at the wait's limit,
  when his client sends the picks he has so far.
- **Into his hand**: a pick message rides his side's GBA's packet as his
  keys do, and each GBA takes it from that packet at the same frame of
  its battle ([The partners' words in BN6's
  packet](partners-words.md)). Once the screen has
  closed and his picks are in (or as they come in after it), each GBA
  sets them for the battle loop's next iteration, the chips taken out
  of his deck, and writes them into his own hand there, as BN6's OK
  writes a side's (bn6f sub_8029110: the chips' ids, powers and folder
  entries, the screen they came from, an Atk+10, Atk+30 or Navi+20
  merged into the chip before it as bn6f sub_8029224 merges them). (The
  first design set them four iterations on from his side's GBA's frame,
  as a margin for the other GBA's distance; the packets need none.)
  While his side's GBA's screen is open his battle keys go in released.
  The leads wait for his picks (below). Picks are dropped only where his
  turn has gone past them: the wait's limit says them none, picks that
  come after his side's next turn has opened are dropped with that turn,
  and picks no screen of his could make are refused ([His messages
  checked](messages-checked.md)).

His client's cost (the host build, this machine; the numbers below): a
frame of his screen about a pair's frame, the turn's copy 0.14 ms to
some 20 ms, making it once some 155 ms (the battle's worst tick, 7 to 8
ms without it): a client that makes it at the way in pays that outside
the battle. A deleted partner has no turn.
