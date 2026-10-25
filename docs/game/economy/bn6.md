---
type: Research
title: "What BN6 does ([Sources](../../research/sources.md))"
description: "BN6's economy in its ROM and bn6f: the busting level picks the reward row's pair, the Chip Trader gives a code the pack lacks three times in four, codes scatter by virus version."
tags: [game, economy]
generated: { by: claude-code/unrecorded, at: 2026-10-20T22:52:50Z }
---

# What BN6 does ([Sources](../../research/sources.md))

- **The busting level picks the reward's pair** (verified: the table read
  in Gregar at `0x020B9C`, bn6f's `sub_80AAA98` that reads it; docs/
  [BN6's battles](../../rom/battle.md), Battle rewards). Levels 1-3 pay pair 0, 4 pair 0 or 1, 5
  pair 1 or 2, 6 pair 2, 7 pair 2 or 3, 8 pair 3, 9 pair 3 and one time
  in 16 pair 4, 10 pair 3 or 4, S (11, verified in the run log with
  one-hit kills) pair 4. The rows put zenny low and chips high: at act 1
  level 4 pays no chip and 103 zenny, level 8 a chip 62% of the time, S
  88% (the report's skill table). A guardian's row holds zenny and his
  Navi chip, whose entries the engine pays as his act's zenny (his
  Guardian Data gives the chip): the run logs saw 600, 1,000 and 2,000 in
  act 1. *This is BN6's skill economy, and the opening plan keeps it whole.*
- **The Chip Trader spreads codes** (verified in bn6f, Falzar's
  addresses: `sub_804BF18` picks a code the pack holds none of with weight
  0xC0 against 0x40 at `byte_804B068`, its comment "75% new code 25% old
  code"; read for this document). Its prize is three in four a chip the
  Library has (`dword_804B008`, 0x40C0) and its 4- and 5-star prizes are
  drawn again more often the more copies the pack holds (the research's
  reading of `chipTraderRedrawChance_804BEEC`, not checked again here).
  This project's traders take only chips new to the Library and give
  their prizes in the folder's codes (`src/layer/trader.c`): convenient,
  and the opposite of BN6's spread.
- **Codes are scattered by the virus's version** (the research's decode of
  the reward rows, approximate): a family's chip comes in one code at pair
  3 and another at pair 4, each version its own, the rare versions in \*;
  of 143 chips the rows give, 57 come in one code and 60 in two. Which
  versions a run meets decides its codes: heterogeneity between players
  that BN6 builds in.
- **Version exclusives** (BN6's Gregar and Falzar Crosses and Navis,
  [TCRF](https://tcrf.net/Mega_Man_Battle_Network_6)) exist so that
  players trade, as Pokemon's do
  ([Bulbapedia](https://bulbapedia.bulbagarden.net/wiki/Version-exclusive_Pok%C3%A9mon)).
- **Assumed, not checked in code:** the busting score (delete time 7 to 4
  points, hits taken to minus 3, two or three deleted at once plus 2 or 4,
  0-2 steps plus 1, over 10 S; [The Rockman EXE Zone](https://www.therockmanexezone.com/wiki/Busting_Level)
  through a search's excerpt, for the series); a folder's copies by the
  chip's MB (5 under 20 MB to 1 from 50; `loot_folder_full` keeps it).
