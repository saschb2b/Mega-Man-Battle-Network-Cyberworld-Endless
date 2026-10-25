---
type: Research
title: "Research: how other games pace it"
description: "Research on how other games pace loot and money (Slay the Spire, Balatro, Dead Cells, Diablo 3, Path of Exile): floors against droughts, rising sinks, stepped power curves, and what keeps traded things worth trading."
tags: [game, economy]
generated: { by: claude-code/unrecorded, at: 2026-10-20T22:14:37Z }
---

# Research: how other games pace it

The research for this issue (20 October 2026), each source read through
its page or a search's excerpt where the site refuses fetches:

1. **Scarce early without droughts: a rising offset.** Slay the Spire's
   card rewards are 3% rare, 37% uncommon, 60% common in a normal fight
   (elites 10/40/50, bosses all rare), and an offset starting at -5%
   rises 1% with every common shown, resetting on a rare
   ([wiki](https://slay-the-spire.fandom.com/wiki/Card_Rewards)). Its
   potions drop at 40%, -10% after a drop and +10% after a miss
   ([wiki](https://slaythespire.wiki.gg/wiki/Potion)). Hearthstone gives a
   legendary within 40 packs and never a duplicate before the set's last
   ([wiki](https://hearthstone.wiki.gg/wiki/Card_pack_statistics)); Dota
   2's pseudo-random chance grows by a constant with each miss
   ([Liquipedia](https://liquipedia.net/dota2/Random_Distribution));
   Tetris deals bags of all seven pieces
   ([wiki](https://tetris.fandom.com/wiki/Random_Generator)). *A scarce
   first act needs a floor, not luck.*
2. **Income flat, the mix and the sinks rise.** Slay the Spire's gold is
   10-20 a fight, 25-35 an elite, about 100 a boss, its prices flat, and
   its card removal 75 gold plus 25 for every use
   ([gold](https://slaythespire.wiki.gg/wiki/Gold),
   [merchant](https://slaythespire.wiki.gg/wiki/Merchant)). Balatro pays
   $3-$5 a blind, raises a reroll $1 a use, and caps interest at $5 a
   round, which turns saving into skill
   ([shop](https://balatrowiki.org/w/Shop),
   [money](https://balatrowiki.org/w/Money)). Dead Cells gives more scrolls
   of power deeper and tiers its items by biome
   ([wiki](https://deadcells.wiki.gg/wiki/Biome)). *Scarcity comes from
   what a reward holds and from sinks that rise, not from inflation.*
3. **Power and challenge curves move together, stepped.** Schreiber:
   perceived difficulty is challenge minus skill and power; power should
   surge at new areas and bosses and avoid long dry stretches
   ([Level 7](https://gamebalanceconcepts.wordpress.com/2010/08/18/level-7-advancement-progression-and-pacing/)).
   Idle games price exponentially against linear income to make each step
   felt ([Pecorella](https://www.gamedeveloper.com/design/the-math-of-idle-games-part-i));
   diminishing returns keep stacked bonuses in check
   ([WoW](https://blog.askmrrobot.com/diminishing-returns-other-bfa-tank-formulas/),
   [Path of Exile](https://pathofexile.gamepedia.com/IIQ)). A logarithm
   rises fast early and never stops, which suits "opens up deeper" without
   exploding late; a logistic curve suits a share with a ceiling.
4. **Online economies inflate without sinks, and trade must not skip the
   loop.** Ultima Online had far too few drains, and its currency lost
   about a hundredfold after one expansion
   ([Koster](https://www.raphkoster.com/?p=16263)); Cook: match sources to
   sinks, keep sinks a little larger, swing within narrow bands
   ([Value chains](https://lostgarden.com/2021/12/12/value-chains/));
   Schreiber: small populations swing prices harder
   ([Level 10](https://gamebalanceconcepts.wordpress.com/2010/09/08/level-10-final-boss/));
   EVE's economy reports track its faucets and sinks every month
   ([CCP](https://www.eveonline.com/news/view/monthly-economic-report-october-2024)).
   Blizzard removed Diablo 3's auction house because it undermined "kill
   monsters to get cool loot"
   ([Blizzard](https://news.blizzard.com/en-us/article/10974978/iii)), and
   Path of Exile keeps trade's friction on purpose
   ([Trade Manifesto](https://www.pathofexile.com/forum/view-thread/2025870)).
5. **What keeps a traded thing worth trading:** scarcity, heterogeneity
   (players hold different things: Pokemon's and BN6's version
   exclusives), and demand for the exact variant (a chip in a code, not
   the chip). Smart loot (Diablo 3's Loot 2.0, which leans drops to the
   finder) and open trade pull opposite ways: the more a game hands each
   player what fits him, the less he needs anyone else.

Hades pays its rarest currencies once per Heat level and converts common
into rare at steep fixed rates; Monster Train's numbers were not found
documented. Seeded loot streams should be derived by a hash, not a seed
plus N: Slay the Spire 2's offset streams turned out correlated
([analysis](https://tck.mn/blog/correlated-randomness-sts2/)); this
project's layer rolls already mix their salt into the seed by a
multiplication (`rolls_of` in `layer_objs.c`).
