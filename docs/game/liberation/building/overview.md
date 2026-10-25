---
type: Feature
title: "Building it"
description: "The staged plan for Liberation Missions: the board in the engine, the three-turn battle in BN6's engine, the Soul team, the Darkloids, a separate mode with co-op; and each piece as built."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-23T13:56:43Z }
---

# Building it

A staged plan, each stage its own pull request:

1. **The board in the engine** (the director's): the generated layer's
   floor as the board, its state in C from the layer's seed (reproducible
   as a layer is), units as BN6's map objects with BN5's and BN6's
   overworld sprites (both already used), dark panels drawn from BN5's
   board tiles read at run time (their place in BN5's ROM to be found), the
   command menu and the phase banners in BN6's own chat boxes and PET look
   (BN6 has no board screen, so the drawing is ours). Risk: the camera and
   cursor over a map BN6 walks; cost: large.
2. **The liberation battle in BN6's engine:** a three-turn limit on BN6's
   own battle loop (BN6's BattleState keeps the turn byte at +7 too,
   assumed from bn6f), the field's columns by the dark around the Navi,
   the one-turn win's report to the board. Risk: small; BN6's battles
   already end by the director's hooks.
3. **The team:** a Soul's Navi on the board, his battles as MegaMan in his
   Soul from the first turn with his own HP (the Souls' code is in BN6's
   engine: [Transplanted Souls](../../../multirom/souls.md)).
4. **The Darkloids:** the run's guardians first (already in BN6's engine);
   BN5's Darkloids (BlizzardMan, ShadeMan, CloudMan, CosmoMan) moved as its
   Navis were, later.
5. **The separate mode and co-op:** a setup option; co-op over the
   server's channels, the board shared turn by turn, each battle its
   player's own (no linked cores needed). Risk: the online rules of the
   common ground ([Online: the Net Square](../../../online/overview.md)).

## Contents

- [Stage 1, as built](stage-1.md)
- [Stage 2, as built: the liberation battle](stage-2-battle.md)
- [The battle's field, as BN5's](battle-field.md)
- [The quality bar, as built](quality-bar.md)
- [The board on the map's floor, as built](board-on-the-floor.md)
- [The Darkloids' phase as BN5's, as built](darkloids-phase.md)
- [The par's prize, as built](par-prize.md)
- [The board's HP as BN5's, as built](board-hp.md)
- [BonusPanels as BN5's, as built](bonus-panels.md)
- [Each mission's own look, as built](mission-look.md)
- [The board's words and banners as BN5's, as built](words-and-banners.md)
- [Guardians' strikes and returns as BN5's, as built](guardians.md)
- [Stage 4, as built: BN5's Darkloids](stage-4-darkloids.md)
- [Stage 3, as built: the team](stage-3-team.md)
