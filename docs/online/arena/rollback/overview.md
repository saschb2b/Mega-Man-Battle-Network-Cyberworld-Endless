---
type: Engine Mechanism
title: "Rollback"
description: "Rollback in every NetBattle (issue #128): the other player's missing keys guessed as his last held, the pair's snapshot kept, frames run again after a wrong guess, the input delay down to 2 frames."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-20T00:21:12Z }
---

# Rollback

Issue #128's second step (the owner's decisions of 19 and 20 October
2026): rollback on every system the Arena runs on, measured on the
weakest first, its window and delay tuned per system; the pair made
cheaper first, then measured again on the Nova, then turned on. It runs
in every NetBattle now. Each frame of the pair runs at its tick; the
other player's keys not yet come are guessed (his last held), the pair's
snapshot kept before each frame that runs on a guess, and a key that
comes otherwise than guessed puts the pair back before the first frame
that read it and runs the frames since again, unheard, to where they
were. On a link the window covers, the input delay is 2 frames (the
smoothed delay's 8 to 11 on the same links). It serves what the smoothed
delay served: a NetBattle that answers the player's own presses.

## Contents

- [A cheaper pair frame](cheaper-pair-frame.md)
- [The battle (arena.c)](battle.md)
- [The window and the delay, per system](window-and-delay.md)
- [Measured](measured.md)
- [In the browser](browser.md)
- [The New 3DS](new-3ds.md)
- [The measurement: --dev rollbench](rollbench.md)
- [What a rollback puts back](what-it-puts-back.md)
- [Quicker on the wire](quicker-on-the-wire.md)
