---
type: Overview
title: "Liberation Missions"
description: "BN5's Liberation Missions dissected and made afresh from a seed as a dark layer: MegaMan and the run's Soul Navis liberate dark panels toward a Darkloid; the dissection, generator, prototype, stages built and decisions."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:28:59Z }
---

# Liberation Missions

Battle Network 5's Liberation Missions, taken apart to their rules, and how
Cyberworld Endless makes them afresh from a seed: a turn-based board where a
team of Navis liberates dark panels toward a Darkloid. The owner's brief
(23 October 2026): every mission generated, dissected "into its very
details to create a fantastic generator that spits out fun missions every
time", "feeling it could come straight out of mmbn5". A player's request
started it: a liberation-mission roguelike, co-op, maps that grow toward
the last boss, special routes to other bosses, each Navi's ability a reason
to try him.

Where it stands: the dissection (from BN5's ROM, its own text, and the
players' guides), the design decision, which the owner approved on 23
October 2026 (The owner's decisions, at the end), a ROM-free prototype
of the generator with its measures, and stages 1 to 3 in the engine: the
dark layer and its board ([Stage 1, as built](building/stage-1.md)), BN5's liberation
battle on BN6's own battle loop (Stage 2, as built), the team, the run's
Soul Navis beside MegaMan, each fought as MegaMan in his Soul (Stage 3,
as built), and a sealed region's route kept for the run, a third way at
the next fork (Special routes).

## Summary

- **BN5's missions are data**, nine records in Team Colonel's ROM
  (`BN5_LIB_MISSIONS`, `src/emu/bn5_liberation.h`): each a list of panels
  laid over one of its net maps, their kinds, the team, the Darkloid and
  guardians, item panels, traps, BonusPanels, the par and the rewards. The
  boards were opened in romlab by setting the mission and entering its map.
- **The decision (the owner's, 23 October 2026):** a liberation is a kind
  of layer the endless net may hold (a "dark layer"), from act 2, chosen at
  a route fork and announced on the act card; the team is MegaMan and the
  Navis whose Souls the run has won, which in Team Colonel are exactly
  BN5's liberation team, each fought as MegaMan in his Soul. Co-op comes
  later over the server; a separate mode may follow on the same generator.
- **The generator** lays a board out in the net's shape language, places
  BN5's kinds by BN5's rules, rejects what breaks a guarantee, and sets the
  par from a planner that plays it by BN5's rules, scaled by the planner's
  own result on BN5's nine.
- **The team, as built:** MegaMan and the Navis whose BN5 Souls the run
  holds, as many as BN5's team of the stage, each with BN5's HP for it,
  his command and his turn a phase, play passing on as BN5's board passes
  it (romlab, mission 2); the Navi in play walks as BN6's player in his own
  BN5 body; a teammate's battle is MegaMan in his Soul from the first turn
  with his HP, and deleted there he leaves the board, not the run.
- **The prototype** (`tools/liberation.py`; `tools/liberation_bn5.py` for
  BN5's own boards) makes 144 missions over acts 2-6 and measures them
  against BN5's nine: the boards keep to BN5's bands of shape and dark,
  route choice opens up from act 3, and every teammate's command earns
  its Order Point on the boards kept.

## Contents

- [BN5's missions, dissected](bn5/overview.md)
- [The decision](decision.md)
- [The quality bar](quality-bar.md)
- [The generator](generator.md)
- [Special routes](special-routes.md)
- [The prototype's measures](prototype.md)
- [Building it](building/overview.md)
- [The owner's decisions](decisions.md)
