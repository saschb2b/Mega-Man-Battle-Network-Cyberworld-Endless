---
type: Research
title: "The board"
description: "A BN5 Liberation board's panel kinds (`BN5_PANEL_*`: clean, dark, item, BonusPanel, DarkHole, the Darkloid's ring, BarrierPanels, start) and how BN5 places them, read off its nine boards."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:18:16Z }
---

# The board (ROM, play)

A board is a set of panels on a square lattice, following the map's floor:
platforms and one-wide walkways, as the net is ([Level design](../../../world/level-design/overview.md)). Its
kinds (`BN5_PANEL_*`):

- **Clean** (0): the team walks it.
- **Dark** (1): liberated by a battle. Guardians walk dark panels.
- **Item** (3): looks dark and holds an item or a trap.
- **BonusPanel** (9, "?"): a roulette (below), at the ends of stubs;
  freed by the Liberation command without a battle or the Navi's action
  (**ROM**, **play**: BonusPanels as BN5's, below).
- **DarkHole** (0x0A): in a ring of dark panels, its guardian beside it;
  every one must fall before the Darkloid can be fought (**text**).
- **The Darkloid's ring** (0x0B): the eight panels around him.
- **BarrierPanels** (5, 6, 7): walls numbered 1-3, opened by the item panel
  that holds their key (**ROM**: the key's handler; **text**: "You unlocked
  the BarrierPanel"). Mission 7 walls a region off; mission 9 is a maze.
- **Start** (0x0C): where the team stands (missions 3 and 5-9 place the
  team by its records instead).

Placement as BN5 does it, read off the nine boards: the start at an edge
(a cross or a column of panels on mission 1, a 3x3 on 7); the Darkloid's
3x3 on the far side; the DarkHoles spread out, each in a full ring of dark
panels; clean walkways near the start, dark thickening toward the holes and
the Darkloid; item panels inside dark clusters and at corners; BonusPanels
on the ends of stubs; barriers across walkways.
