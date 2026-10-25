---
type: Feature
title: "Trade Central"
description: "Trade Central (`trade_central.c`): one spare chip for another player's, kept in `trade.pending` and settled at home, one trade a visit, the spares list and chip trade screens (`trade_screen.c`, issue #145), and the trade board."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-23T14:26:53Z }
---

# Trade Central

A Mr.Prog at a desk in the part warped to up-left of the plaza (-x) trades one
spare chip (a copy past those in the run's three folders) for another
player's (`trade_central.c`, words in `trade_words.c`). Both games hold
the NetPass's Trade stamp (above), so every chip of either game's
registry trades, BN5's too: its ids are the registry's, fixed in
`src/core/xchips_registry.c`, so they name the same chip on every game
with BN5's ROM. To a game without the stamp the clerk says it has none,
and the trade board's offers still read; the server refuses that game a
trade, a post and an answer (NOTICE 28).

1. MegaMan picks a Navi in the channel whose pass holds the Trade stamp
   (the clerk lists no other) and the chip he offers, on the **spares
   list** (below).
2. The other player's MegaMan hears the offer, goes to the desk and picks
   a chip back on the same list (its note says what the offer is), or
   says no.
3. Both see the pair on the **chip trade** (below) and say Deal!; the
   server commits the trade only when both accepted the same pair, and
   writes it to `trades.jsonl` (`server/cyberworld_server/trade.py`). A
   stale or mismatched step, a player gone, or five minutes without a
   step calls it off.
4. Each game keeps the done trade in `trade.pending` the moment the
   server says it is done, wherever MegaMan stands, and applies it at
   home: on the way back from the Square, or at the next CONTINUE if the
   game was closed there. It runs BN6's own TakeChips and GiveChips on
   the gate's checkpoint (`online_trade_settle`), refuses a swap whose
   chip is no longer spare or one this game cannot hold, deletes the file
   before it saves, and MegaMan says what came and went. BN6's own
   routines count a chip past 319 in the pack's extension (docs/
   [Running the game's own logic](../engine/emulation/overview.md)), so a BN5 chip comes and goes as BN6's do.

One trade a visit: a game keeps one swap until it is home
(`trade.pending` holds one), so after a trade is done the clerk says
"ONE TRADE A VISIT!" and the server refuses that session a second
(NOTICE 21). Before this, a second trade in a visit overwrote the
first's file: its partner got the chip, this game never gave it.

## The two screens

[Issue #145](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/145),
the owner's model the Mega Man Battle Network Legacy Collection's trade
screens: the trade was told in chat boxes, and the exchange itself, one
that can't be undone, was never shown. Two PET screens of our own now
show it, drawn by the engine over the game's picture as the NetPass is
(`trade_screen.c` their course, `trade_screen_draw.c` their picture in
the NetPass's colours and parts, `trade_screen_text.c` their PET text,
`trade_list.c` the list's rows, sorts and filter; the game held under
them, no key reaching the core, their closing A or B kept from BN6 till
it is let go). The clerk's chat boxes stay for the talk around them
(hello, whom, the board, the wants), MegaMan's words in `trade_words.c`.

- **The spares list**, used wherever a chip is picked: an offer to a
  Navi ("Offer a chip"), one back to his ("Offer one back", its note
  "Alpha offers FireBrn1 F"), an answer to a board offer ("Answer an
  offer", only the spares that meet its want, its note "Spares that fit:
  a Fire chip") and a post ("Post an offer"). On the left a card of the
  chip under the cursor: its picture and palette, name, code, element
  icon, power and description, read from the player's ROM as it is drawn
  (`second_art.c`'s readers, the chip registry's for BN5's chips, so
  nothing of Capcom's is kept in a file); on the right MegaMan's spares
  (copies past those his three folders hold), each row its icon, name,
  code and count, the cursor's lit, six at a time with a scroll bar.
  UP and DOWN move (round at the ends), LEFT and RIGHT a page, START
  opens a SORT pop-up of BN6's folder editor's first three keys, ID, ABCDE and Code
  (the record's IDSortPos and AlphabetSortPos; the session keeps the
  sort), A picks, B goes back. It replaces BN6's Order Service screen
  (`trade_order.c`, gone) and the chat box's list it fell back on for a
  BN5 chip past BN6's 319, whose list builder would have written past
  its end; the screen lists any id the registry has.
- **The chip trade** ("Chip trade", "With Beta"): both cards, ours on
  the left, the arrows between them, and MegaMan in BN6's NAVIGATOR box
  under them asking "Lan,trade our FireBrn1 F for Beta's MrkCan1 M?" with
  Yes and No, the cursor on No as "Deal,Lan?" had it; B is No. After Yes
  the cards hold, MegaMan says "Hmm... Beta's still thinking." and the
  arrows pulse, B calling it off; when the server commits, "Here it
  comes!", the cards cross (MegaMan's over the top, the other's under it,
  40 frames) and land with BN6's jingle (`SFX_GOT`, as the NetPass's full
  pass lands), "Yeah! We got MrkCan1 M!", the card come in the ChipFolder's
  EQUIP corners, and A closes it; MegaMan then says "Deal! FireBrn1 F is ours once we're home!" as
  before. Called off by the other player or the server meanwhile, it
  closes and MegaMan says the trade's off. MegaMan's own answer or take
  opens it at once; an answer to his offer, or his board offer taken,
  first has his word ("Lan! Beta answered us!", "Lan! Alpha took our
  offer!"), wherever he stands.

The design (the game-design skill, interface): an exchange that can't be
undone gets a screen that shows exactly what changes hands (error
prevention, honest feedforward), with the friction matched to the
stakes: one deliberate press past a default on the safe side, not a
hold or a second question. The list makes the spares visible before
anyone is asked (what is mine to offer, and what fits a want), so the
board's answer is a choice among what fits rather than a guess. The
swap is the session loop's payoff, juiced only as far as the state stays
legible: the cards move, the sound plays once, and what landed is framed
in gold. A second sort answers the same request AsterLand's Order
Service answered (issue #124): ID, ABCDE and Code are BN6's own keys.

## The trade board

A trade needs two players at the desk at once and a blind first offer;
in a channel of a few players, most visits never find out who wants
what. The board says it: each player may put **one offer** up at the
clerk, a spare he **has** (picked on the spares list as a trade's chip
is) and what he **wants** for it, from a fixed list: anything, a Fire,
Aqua, Elec, Wood, Sword, Wind, Break or Cursor chip (BN6's own icon
element, `BN6_CHIP_ELEMENT`), or a Mega (`BN6_CHIP_LIBRARY_TYPE`). The
clerk reads the channel's offers one a box ("Alpha HAS Cannon A! WANTS A
FIRE CHIP!") with Answer it and Next offer under each, and MegaMan's own
to post or take down; the Board in the welcome plaza lists them among its
posts. Answering one lists only the spares that meet its want; the trade begins with both chips named (TRADE's
take, protocol v5), and both say Deal as in any trade. The poster's
game checks the chip against the want it posted and calls off one that
is not (the server never reads a chip's element); MegaMan then says
"Lan! Beta took our offer!" before the Deal. An offer lives on the
server while its poster is in the channel: taken down, a channel
changed, the server left or a trade done, it goes, and no offer is
kept on disk (`trade.py`; `online_board.c` holds the board as last
said, `trade_central.c` the clerk's chats, `trade_words.c` the words).
Posting and answering take the Trade stamp; reading does not. A game
without it reads the clerk's list and the Board's, and the server refuses
its post or answer (NOTICE 28). It is never sent an offer whose chip is
BN5's, which it could not name, and reads the rest.

The design (the game-design skill): the dialectic is the Square's, **the
run's integrity against the community's generosity**, and the board
serves the session loop (arrive, see who's here, do one thing together,
go home). Its patterns are a diegetic board and honest feedforward:
what a player wants is said before anyone walks to the desk, so a trade
starts from a match instead of a cold offer, and the Deal still shows
the exact pair. A want is a **kind**, not a chip: a fixed list needs no
free text (the audience has children in it) and many chips meet one
want, so a small crowd still matches. One offer a player is a budget
that keeps the board to eight lines and stops hoarding posts; offers
that live only while their poster is there keep every answer one that
can finish now (no asynchronous market, no appointment to keep). It
stays one spare for one spare and no currency: there is no price to
rise, and Giga chips are not on the list, so the board advertises
variety, not power. One trade a visit follows from the swap kept at
home, and fits the loop: a trade is the visit's one thing.
