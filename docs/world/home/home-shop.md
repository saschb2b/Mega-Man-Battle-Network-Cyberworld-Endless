---
type: Feature
title: "6. The home shop"
description: "The home shop: AsterLand's clerk opens BN6's Order Service, one order a visit at twice a Net Dealer's price, sorted on START by BN6's seven keys (`order_sort.c`), beside the SubChip seller."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-20T19:17:54Z }
---

# 6. The home shop

AsterLand's counter in Central Town. Its **Order Service** is BN6's: a
chip the Library holds, its code and price named by the clerk, given at
once; one order a visit, priced above the net's dealers, so the net's
stock stays the gamble and home the sure thing. The town's **SubChip
seller** stands beside it, as BN6 has one in each town.

As built (issue #89): BN6's own clerk (its sprite and face, list 6's
`0x33`) stands at AsterLand's register and opens BN6's Order Service
(shop `0x12`): every chip of BN6's order list the Library holds, Standard
and Mega, each once a run as BN6 has it; the run sets each one's code to
the folder's where the chip comes in it, and its price to twice a Net
Dealer's (a common chip 1000 zenny, a Mega 8000: `build.py pacing`), and
marks the Library's chips owned as BN6 marks a chip it gives (the order
checks the mark). A facing the counter reaches the keeper behind it from
16 units either side of him and from a step before its front, where the
isometric view shows the counter close (BN6's own reach answered from 4:
a playtester faced the counter from four places and got the showcase's
text, session 69, and from a step before it nothing, session 70;
`indoors_counter`), L in AsterLand says where orders and SubChips are,
and the shopper stands at the NEW case, out of the SubChip seller's way.
One order a visit: after it the clerk says so until the
next visit. The SubChip seller (BN6's, the white-coated man of its SubChip
shops' window) stands beside him with BN6's Central Town shop (`0x0F`)
restocked for the run: the keys the act ahead's locks want (Unlockers,
RushFood, a WWW-ID), then MiniEnrg, FullEnrg, SneakRun and Untrap at BN6's
prices (its LocEnemy is no use in a run). BN6's In-Stock Chips (shop 4)
stays out: the Net Dealers are the run's stock. The second screen's shop
panel reads the list as the screen shows it (sorted, sold-out ones kept
as BN6 keeps them), so the order's chip under the cursor is the one it
names.

Sorted (issue #124, a player's ask: "the same options I have access to in
my chip folders"). BN6's Order Service lists the Library's chips by ID
alone, five rows a page, though its folder editor and the Chip Trader
beside the counter sort theirs on START by seven keys ([Sources](../../research/sources.md)):
a Library of a few runs is pages of paging for one chip. The list is
BN6's own screen, so the sort is BN6's own too, in the clerk's box: START
on his list goes on to the next key in the SORT window's order (ID,
ABCDE, Code, Attack, Element, No., MB), each the way BN6 first sorts by
it (Attack and No. down, the rest up, ties by ID), and SELECT turns it
round, as BN6 does for a key picked twice; the cursor goes back to the
top, BN6's select sound plays and his box says the sort over the buttons
("By Attack,high first!", `place_lines.c`), where BN6's prompt had L/R
and B alone. No. counts the copies of the entry, its chip in its code,
in the folder and the Pack, as the second screen does. The sort is the
session's: the list opens in it again, after a quit or at the next
visit. Nothing about what the order costs or gives changes: one order a
visit, at its price, the sure thing found sooner. As built
(`src/world/order_sort.c`, [ROM data](../../rom/overview.md)): the entries BN6 found as
the screen opened are sorted between frames by BN6's own keys, the
screen makes its list again from them, BN6's own way back to the list
runs his prompt, and a hook on the chat has ours said in its place, in
AsterLand alone, the one place the engine opens BN6's order screen. BN6
finds an order's entry by its chip and code, never by its place in the
list, so a sorted list orders and marks sold out what it shows.
