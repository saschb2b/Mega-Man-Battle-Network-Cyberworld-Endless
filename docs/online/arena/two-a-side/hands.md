---
type: Feature
title: "The partners' own hands"
description: "The partners' own hands in a two a side battle: a hook on BN6's hand routine (`BN6_HAND_ADDR`, `0x08010018`) returns a partner's hand by object, measured with both GBAs agreeing through rollbacks."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T21:44:08Z }
---

# The partners' own hands

BN6 keeps one hand a side (`BN6_BATTLE_HAND`, `0x50` bytes: the chip up
next, six chips, their powers), every reader through one routine with a
side (`BN6_HAND_ADDR`, `0x08010018`: 20 calls in bn6f, and
`getCurChipInBattleHand_8010004`'s 7 through it, most with the side of
the object in r5). The same answer by object as the NaviStats': a hook
there returns a partner's own hand, in his room after his record, where
r5 is his or his marks' and the side asked for is his. His hand starts
as his side's as the battle sets it up (empty); at the hands' commit it
was dealt from his lead's each time the Custom screen dealt his lead's
anew, and since his own Custom screen (below) it takes his own picks
alone. Under the flag a lead takes the chip under the cursor before the
script's OK, so that a hand holds one (Cannon).

Measured with the runs above (the partners tapping A a few times a
cycle until they are free to use a chip): side 0's partner uses his
Cannon at iteration 177 and side 1's at 429, each one's own hand's chip
up next going from 0 to 1 while his lead's stays at 0 (each lead still
holds his Cannon, "Cannon 40" still at the foot of his GBA's screen);
side 1's partner's Cannon takes side 0's partner from 84 to 43 at
iteration 454 (40 and the poison's 1). On both screens the chip's icon
is drawn over each partner from his own hand and leaves him as he uses
it, his lead's staying; the second GBA's (`arenaother`) shows side 1's
partner firing it from the left and side 0's partner's own HP under him
falling 85, 52, 40. The hands answered some 5,000 to 9,000 asks each a
battle, all the partners' own objects'; the GBAs agree at every
iteration; with 221 rollbacks and with none all 44 hashes the same,
`arenaroll=8` 335 of 335 the same. Before the partners tapped A (one
press each, which met them busy), the hands as their leads' copies left
the battle's 272 changes of HP as with the hook switched off (a debug
run). (These numbers are the hands' commit's, with his lead's chips in
his hand; his own folder and Custom screen follow.)
