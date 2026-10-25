---
type: Overview
title: "The Arena"
description: "The Arena folder: BN6's Practice NetBattle between players, each client running both linked cores with only keys crossing: its engine, smoothed delay, rollback, the way in, the stands, Team NetBattles, two a side."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-18T13:19:48Z }
---

# The Arena

The Arena (epic #120) is a NetBattle of BN6's own between two players.
Each client runs **both** linked BN6 cores, joined by mGBA's own cable
(its `GBASIOLockstep`), and only the players' controller keys cross the
network. This is the research's design (its notes, `ONLINE_FRAME.md`, were never committed):
raw serial over a network stalls ([The link cable and the Arena](../link.md)), while
keys sent ahead by a few frames let BN6's Practice NetBattle finish
identically at 0 to 150 ms one way.

This page is the engine (the pair, the way in, a carried MegaMan,
lockstep play over a link, `--dev arena` with a local stand-in for the
peer) and its network half: the link over the presence server, and the
way in from the Net Square, where one player's MegaMan asks another's for
a Practice NetBattle in BN6's own chat boxes ([From the Net
Square](from-square/overview.md), below).

## Contents

- [Run it](run-it.md)
- [What runs](what-runs.md)
- [Measured](measured.md)
- [Smoothed delay](smoothed-delay.md)
- [Rollback](rollback/overview.md)
- [From the Net Square](from-square/overview.md)
- [The stands](stands.md)
- [Team NetBattles](teams.md)
- [Two MegaMen a side](two-a-side/overview.md)
- [Not yet](not-yet.md)
