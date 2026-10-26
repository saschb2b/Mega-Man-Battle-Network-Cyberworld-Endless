---
type: Overview
title: "Progression"
description: "Progression, how a run's difficulty is decided (`src/core/pacing.c`): depth sets difficulty and the area only flavour, per-act HP budgets and damage caps, guardians by HP band, a fixed heal rhythm, and the research behind it."
tags: [game, progression]
generated: { by: claude-code/unrecorded, at: 2026-09-24T20:25:55Z }
---

# Progression

Research into how a run's difficulty was decided, what BN6's own data says
about difficulty, and how other roguelikes keep a run fair, and the
structure built from it. The rules live in `src/core/pacing.c`; the
[Status](status.md) section lists what differs from the first proposal and
what is still open.

## Summary

A run started in any of 15 areas with equal odds. The only difficulty
control was the virus version, set from depth. BN6 ties difficulty to the area
itself: the viruses a player meets, and how many of them there are, rise
from Central to the Graveyard. A random first area can therefore open with
three times the virus HP of another. The first guardian (layer 3) came
from the area's full pool, so it could be BlastMan with 400 HP or Colonel,
against a MegaMan who still has the starting 100 HP and the default
folder. Healing was left to chance on layers 1 and 2, and MegaMan's growth
(+60 max HP per guardian) fell behind the V3 viruses of acts 5 and 6.

The structure now: depth sets the difficulty and the area only sets the
flavour. Each act has an HP budget per battle and a cap on how hard one
virus may hit, both measured against the max HP MegaMan is expected to have
by then. The first area comes from the easy areas. Guardians are chosen by HP band for
their act. Healing follows a fixed rhythm. MegaMan grows faster, and more of
the risk is something the player chooses.

## Contents

- [How a run worked before](before.md)
- [What BN6's data says](bn6-data.md)
- [What other roguelikes do](other-roguelikes.md)
- [The structure](structure.md)
- [Status](status.md)
- [Measuring it](measuring.md)
- [Where it is decided](where-decided.md)
