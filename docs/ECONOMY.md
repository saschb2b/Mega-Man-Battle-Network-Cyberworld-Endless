# Economy

What a run hands out (chips, their codes, zenny, Mystery Data), measured
act by act; how other games and BN6 itself pace theirs; and the opening
plan that makes the first act scarcer and opens the run up deeper, with a
guard against drift. [Issue #154](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/154),
the owner on 20 October 2026: "to make trading meaningful we should do a
balance sweep. I feel players could get too many chips early. And too many
good chips. And too convenient letters for chips early. And too much zenny
early. And too many mystery data early maybe? [...] And we need a way to
make sure it doesn't drift too hard in the future."

**As built.** The proposal below went to the owner on 20 October 2026
and he said yes on the 21st, every decision as proposed
([Decided](#decided-21-october-2026)): the opening plan is the game's
(`src/core/economy.c`), the traders pick codes as BN6 does, and the
run-end statistics count the economy. "Before" and "flat" below are the
rolls as they were; "after" and "opening" what a run hands out now.

## Summary

- **The first act was generous, and its finds came in the folder's
  codes.** A run's first three layers gave about **12 chips** (8 from
  Mystery Data, 3 from battles, the guardian's), a quarter of them good,
  and **3,500 zenny**, twice what the act's answer chips and HPMemory
  cost. **63% of the chips found came in the starting folder's codes or
  \*** (the engine leaned finds to the folder's codes), so by the first
  guardian a run held five chips of one code with the \*s: a whole
  hand.
- **Deeper, the economy was already rich, and stays so.** By act 6 a
  run had earned 44,000 zenny against 23,000 for the acts' answers and
  HPMemory, held 24 chips of one code, and seven in ten of its finds were
  good. The owner's complaint is the early run, so the plan leaves the
  sixth act as it is: no power creep, and nothing a later act gains.
- **The opening plan (in play):** five levers, each a curve from a floor
  at the first act to the flat plan's at the net's last, on a logarithm
  (fastest opening early): fewer green Mystery Data (half on act 1), a
  green data's chip kept 60% of the time, fewer good chips in the plain
  finds and fewer extra rich data (a quarter of their weight), half the
  zenny in a find, and **no find leaned to the folder's codes in act 1**.
  Act 1 gives **9.5 chips, 2,630 zenny, 11.6 Mystery Data, three chips of
  one code by the guardian**, and opens to the flat plan's by act 6.
  BN6's own battle drops and busting levels are untouched, so skill pays
  a larger share of the early chips (34% against 28%).
- **The traders** give a prize's code as BN6's own Chip Trader does,
  three times in four one the pack holds none of, rather than the
  folder's: a run's codes are its own, and worth a trade.
- **The guard:** `python3 build.py economy` measures both plans and holds
  the plan in play to bands a fifth either side of its own numbers (0
  outside on both nets); the curves have ROM-free tests; the run log
  keeps each battle's busting level, reward and zenny on hand, and each
  run's economy's counts; the anonymous statistics' run-end event carries
  those counts; and a rule in AGENTS.md's checks table.

## Measuring it

```sh
python3 build.py economy [RUNS]     # .build/economy.txt, 300 runs by default
```

`src/dev/economy_report.c` (`--economy PATH[:RUNS]`, docs/DEVTOOLS.md)
plays the endless net's first cycle (and the short net's ten layers) of
many runs' seeds without running the game. Two kinds of number, said
apart:

- **The engine's, simulated with its own code.** Every layer as
  `layer_generate` makes it, so the Mystery Data, dealers, Servers,
  traders and gates are the game's own; each Mystery Data's content by
  `loot_mystery_roll` (moved out of `layer_objs.c` for this, the same
  rolls in the same order); each Net Dealer's stock by
  `shop_dealer_stock`, the NaviCust vendor's by `shop_program_stock`; the
  requests' pay by `jobs_offers`; a guardian's Guardian Data (his Navi
  chip, five HPMemory).
- **The game's, read from the ROM.** Each random battle the engine rolls
  (`loot_random_battle`, the opening battles' rules as the director's)
  pays as BN6 picks a reward: one enemy at random, its reward row
  (`BN6_DROP_ROWS`, 20 entries), the busting level's row of BN6's busting
  table (16 pairs' offsets), a coin, the row's second half at low HP; the
  second entry of a pair leaned to the folder's codes as `encounter.c`'s
  hook leans it; a guardian's Navi chip entries his act's zenny
  (`loot_guardian_zenny`). docs/ROM_DATA.md, Battle rewards.

Assumptions, each a number in the code and said in the report's header:

| Assumption | Value | From |
| --- | --- | --- |
| Random battles a layer | 1 to 3, even | the playtest sessions' run logs (`.build/play/*/data/runlog.txt`, 94 sessions): 1.7 to 2.4 battles a layer, layers 1-12 |
| Busting levels | S 20%, 10, 9, 8 and 7 15% each, 6 10%, 5 and 4 5% each | assumed: a fair player; the autopilot's are lower (below) |
| Won at low HP (3/8 or less) | 10% | assumed |
| A battle's own Mystery Data still there at its end | 50% | assumed |
| Every Mystery Data opened, every battle won | | the most a run can take |
| The folder | BN6's starting one, codes S, A and B (`loot_folder_codes`) | the standard setup |
| The areas | BN6's alone, threat 0, no helpers | where BN5's dress an area, half the chips are BN5's of the same tiers |

Definitions:

- **Good chip:** tier 2 or more of `chip_pool.h` (BN6's rarity of 3 stars
  or more for a standard chip, every Mega and Giga chip). **Strong:** a
  straight hit of 100 or more.
- **Codes held:** the most chips gained in one letter, with the \*s,
  median run, at an act's end: how big a hand of one code the finds alone
  make (BN6's Custom screen takes up to five chips of one code; \* joins
  any). **Fit:** a chip found in one of the folder's three codes or \*.
- **Wanted:** what the act's middle-layer dealer asks for his first chip
  (the act's answer, two copies) and his HPMemory, summed act by act: the
  purchases a run plans around (docs/PROGRESSION.md, Growth).

**Checked against the game.** The run log keeps each won battle's busting
level and reward since this issue (docs/DEVTOOLS.md, Run log). Twelve
autopilot runs (seeds 1-12, fresh data folders, 40,000 frames; with `--dev
god` 90,000; with `--dev god,onehit` 20,000) won 110 random battles:

| | Autopilot (game) | Report at the same levels |
| --- | --- | --- |
| Busting levels | 1-10, median 7; 11 (S) with one-hit kills | the skill table: level 6 15% chips 143 zenny, level 8 62% chips 85 zenny |
| Act 1 battles paying a chip | 38-42% | 15-62% between levels 6 and 8 |
| Zenny a battle, act 1 | 89-131 | 85-143 |
| Guardian's reward, act 1 | 600 (his Navi chip's entry), 1000 or 2000 (his row's zenny) | the same words, 1,264 on average at the report's mix |

The autopilot opens no Mystery Data and buys nothing, so it checks the
battles' half; the Mystery Data are `layer_generate`'s and
`loot_mystery_roll`'s own, as the layers in play are (`LAYER_MAKE_HASH`
unchanged then). The opening plan's levers ran in play before it was
switched on: twelve one-hit autopilot runs under it reached acts 4 and
5, and their guardians' chip-entry zenny was the zenny curve's share of
the flat 600 to 1,800 to the zenny, act by act (300, 588, 921, 1,288,
1,683), BN6's own rows' zenny (1,000, 2,000, 3,000, 6,000) as before.

## Before: what a run handed out (the flat plan)

300 runs, the endless net (per run, an act's three layers together; the
Nest its one layer):

| Act | Chips gained | Good | Strong | Fit | Megas | Codes held | Mystery Data | HPMemory | Zenny earned | Earned / wanted so far |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 12.2 | 26% | 22% | 63% | 0.95 | 5 | 14.8 | 5.0 | 3,550 | 3,480 / 1,600 |
| 2 | 11.4 | 31% | 22% | 51% | 1.03 | 8 | 15.1 | 5.2 | 4,350 | 7,850 / 3,800 |
| 3 | 14.7 | 38% | 29% | 56% | 1.24 | 11 | 20.3 | 5.5 | 6,880 | 14,920 / 7,400 |
| 4 | 13.9 | 46% | 38% | 56% | 1.49 | 15 | 19.5 | 5.5 | 7,900 | 22,400 / 11,500 |
| 5 | 17.8 | 57% | 33% | 56% | 1.82 | 20 | 24.7 | 5.7 | 10,890 | 33,210 / 16,800 |
| 6 | 15.8 | 70% | 36% | 55% | 1.89 | 24 | 22.6 | 5.6 | 10,760 | 44,190 / 22,800 |
| Nest | 5.4 | 69% | 36% | 61% | 0.44 | 26 | 7.6 | 5.3 | 5,360 | 49,910 / 22,800 |

Where act 1's come from (per run): 6.1 battles give 3.4 chips and 530
zenny; the layers' Mystery Data 7.8 chips, 1,740 zenny and 7 BugFrags; the
guardian his Navi chip, five HPMemory and 1,260 zenny; the requests offer
765 zenny, 2.4 BugFrags and a chip. Offered beside them: 11 dealer chips
(9,700 zenny with the HPMemory), a Server's prize in about one run in
five (91% good; half the acts after), a Chip Trader in three runs in
four, and the start gift's choice of a 3-star chip, two HPMemory or a
program. A layer lays 2.6 green Mystery Data, 2 blue (1.3 of them rich:
the far detour's and three in ten besides) and a purple one in four on
its second layer; deeper, 3-4 green and 3-4 blue.

**What it says.**

- *Chips:* about four a layer from the first, 24 by the end of act 2 and
  a folder's worth (30) early in act 3; Mystery Data give two in three of
  them.
- *Good chips:* a quarter in act 1, three of them, one the guardian's
  Mega; the rich data at a detour's end give most of the rest.
- *Codes:* finds lean to the folder's codes three ways (a Mystery Data's
  chip rolled again for one half the time, a battle row's second entries,
  and every dealer's, trader's, request's, gift's, vault's and Server's
  chip in them), so 63% of act 1's finds fit the starting folder: the
  median run holds three chips of one code (with the \*s) by layer 2,
  five by layer 3, ten by layer 8, and its first \* by layer 3. BN6's own
  Chip Trader does the opposite: it gives a code the pack holds none of
  three times in four (below).
- *Zenny:* by the act's dealer (layer 2) a run has 1,460 zenny, almost the
  1,600 his answer and HPMemory cost; the guardian's 1,260 then pays act
  2's before it begins, and the surplus grows every act.
- *Mystery Data:* 15 in act 1, 20-25 deeper; with every one holding
  something, a layer has no dull corner.

## What BN6 does (docs/SOURCES.md)

- **The busting level picks the reward's pair** (verified: the table read
  in Gregar at `0x020B9C`, bn6f's `sub_80AAA98` that reads it; docs/
  ROM_DATA.md, Battle rewards). Levels 1-3 pay pair 0, 4 pair 0 or 1, 5
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

## Research: how other games pace it

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

## The opening plan

As proposed on 20 October 2026 and built on the 21st.

### Through the game-design skill

- **The dialectic: the folder you have against the folder you want.** It
  is BN's own, restated at every scale: in a battle, the hand you drew
  against the code you need; on a layer, the far detour's data against
  pressing on; in an act, the dealer's answer against his HPMemory; at
  Trade Central, a spare you hold against the code someone else holds.
  The flat early economy collapsed it: the finds came in the folder's
  codes, and the zenny bought both.
- **The loop layers.** Moment to moment, the busting level picks the
  reward (BN6's); the layer loop, the walk for Mystery Data; the act loop,
  the act card, the dealer and the guardian; the meta loop, the Library
  and its vault; and the social loop, Trade Central and its board. The
  plan thins the layer loop's output early, so the act loop's choices
  bite and the social loop has something to answer.
- **The patterns.** *Opening curve* (the sawtooth's rising floor, Dead
  Cells' tiers by biome): scarce first, open deeper. *Honest telegraph*
  (the skill's): a blue data at a detour's end stays the layer's prize,
  its odds whole; the plain finds thin. *The chase is the game* (Path of
  Exile): the right code reachable, rarely handed over. *Heterogeneity
  for trade* (version exclusives): with no lean early, each run's codes
  are its viruses' and its areas', its own. *Opportunity cost felt*: act
  1's zenny buys the answer or the HPMemory at the dealer, not both.
  *A floor against droughts* (pity): the start gift, the far detour's
  rich data and the dealer's answer stay certain.
- **What it does not touch** (the owner's standing decisions): BN6's
  battle drops and busting table (skill pays, as BN6's; its share of act
  1's chips rises from 28% to 34%); the guardians' HPMemory and the
  heals (growth and difficulty pacing, docs/PROGRESSION.md); the buster
  (NaviCust-only); DarkChips; the meta (variety, not power); prices.

### The curves

Each lever's share of the flat plan's at act `a` of a net whose last act before
its Nest is `A` (the endless net's sixth, the short net's third), `x =
a / A` (0 for the first act, 1 for the last, the Nest and later cycles):

```
f(x) = F + (1 - F) * ln(1 + b * x) / ln(1 + b)
```

`F` the first act's share, `b = 2` the bend (how soon it opens; near 0
a straight line). `f` is 1 for the flat plan everywhere, and never above 1
for the opening one (tested): no act hands out more than it did.

| Lever | Where | Flat | F | Act 1 | 2 | 3 | 4 | 5 | 6 | Short net 1 / 2 / 3 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Green Mystery Data | `place_data` (net_gen.c): their count | 2-4 a layer, +1 deeper | 0.50 | 0.50 | 0.65 | 0.77 | 0.86 | 0.93 | 1 | 0.50 / 0.82 / 1 |
| A green data's chip | `loot_mystery_roll`: kept by the share, else the data's zenny; a blue one keeps its odds | 50% of green data | 0.60 | 0.60 | 0.72 | 0.81 | 0.89 | 0.95 | 1 | 0.60 / 0.85 / 1 |
| Good chips | `roll_chip`: tiers 2-4's weight in the plain finds (green and blue data, a dealer's shelf, a request's chip); `place_data`: the extra rich data's 30% | as rolled | 0.25 | 0.25 | 0.48 | 0.65 | 0.79 | 0.90 | 1 | 0.25 / 0.72 / 1 |
| Zenny | `loot_mystery_roll`: a data's zenny; `loot_guardian_zenny`: a guardian's chip entry | 100-500 green, 800+ blue, 600+ guardian | 0.50 | 0.50 | 0.65 | 0.77 | 0.86 | 0.93 | 1 | 0.50 / 0.82 / 1 |
| Lean to the folder's codes | `loot_mystery_roll`'s second roll; the battle rows' second entries | half the data, every row | 0 | 0 | 0.31 | 0.54 | 0.72 | 0.87 | 1 | 0 / 0.63 / 1 |

Kept whole: the far detour's rich data, purple data, a Server's prize, a
vault's three, the Guardian Data, the gift, the dealer's answer (its two
copies and its price cap, in the folder's codes: convenience paid for).

### Before and after

300 runs each, the endless net (the report's first table):

| Act | Chips | Good | Mystery Data | Zenny | Codes held | Fit | Earned / wanted so far |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 12.2 → **9.5** | 26% → 23% (3.2 → **2.2** chips) | 14.8 → **11.6** | 3,550 → **2,630** | 5 → **3** | 63% → **35%** | 3,480 → 2,600 / 1,600 |
| 2 | 11.4 → 9.4 | 31% → 29% | 15.1 → 12.6 | 4,350 → 3,610 | 8 → 5 | 51% → 33% | 7,850 → 6,320 / 3,800 |
| 3 | 14.7 → 12.5 | 38% → 34% | 20.3 → 17.9 | 6,880 → 5,880 | 11 → 8 | 56% → 40% | 14,920 → 11,990 / 7,400 |
| 4 | 13.9 → 12.9 | 46% → 44% | 19.5 → 18.2 | 7,900 → 7,100 | 15 → 11 | 56% → 48% | 22,400 → 19,100 / 11,400 |
| 5 | 17.8 → 17.1 | 57% → 57% | 24.7 → 24.7 | 10,890 → 10,740 | 20 → 15 | 56% → 52% | 33,210 → 30,050 / 16,800 |
| 6 | 15.8 → 15.8 | 70% → 70% | 22.6 → 22.6 | 10,760 → 10,700 | 24 → 19 | 55% → 55% | 44,190 → 40,500 / 22,800 |

At the act's dealer (layer 2), a run had 1,460 zenny against his 1,600 and
has 930: the answer's two copies or the HPMemory. Three chips of one code
come by layer 3 rather than 2, five by layer 6 rather than 3; a layer
keeps its far detour's rich data (1.06 rich a layer on act 1, from
1.26). The short net (a new run's) follows its own curve, open by its
third act: act 1 as above (9.6 chips, 2,630 zenny, codes 3), act 2 10.0
chips and 4,000 zenny, act 3 the flat plan's.

### What it costs

- **A starved first act.** Act 1 keeps about three chips a layer (one
  from a battle), the gift's choice, a dealer whose answer costs 400 a
  copy, and 930 zenny at his counter. The answer was built to beat the
  first guardian beside the starting folder (docs/PROGRESSION.md,
  Growth); the plan leaves it, the gift and the heals as they are.
  Its risk is the run that finds little and then has to choose at the
  dealer: the act card names the guardian before, so the choice is
  informed. Watch the first guardian's win rate in the statistics before
  and after.
- **Fewer and plainer Mystery Data** make a layer's corners emptier. The
  plan takes half the green data out, and four in ten of the chips
  the rest held become zenny, half of the flat plan's (50-250 on act 1); a
  playtester's battle data of zenny "read as no find at all" (loot.c), so
  the count falls more than the content. Every blue data stays, its odds whole but for
  the good chips' weight in the plain blue ones.
- **Codes left to chance early** make the first folders patchier: three
  of one code by the guardian rather than five. That is the point for
  trading, and BN6's own trader still answers it (three chips for one,
  in a code the pack lacks, were the project's traders BN6's).
- **Fewer spares to trade early.** Trade Central swaps a spare (a copy
  past the run's three folders) at home after an act (docs/ONLINE.md):
  fewer chips mean fewer spares after act 1, each worth more. Most of a
  first act's finds are spares either way (a 30-chip folder takes few of
  them), and the codes they come in now differ from run to run, which is
  what makes one worth asking for.
- **Switching it on** bumped `LAYER_MAKE` to 96 (a layer made by the
  flat plan differs: a run saved before continues its layer afresh), and
  the battle rows' lean, which runs in a hook that draws no random number
  (`encounter.c`, `lean_row`), takes the curve by a hash of the layer, the
  enemy and the entry (`econ_keep_by`). Saves need no change (`Run` is
  untouched; the counts are a file of their own, `run.tally`).

### Not proposed, and why

- *Capping the busting pair by act* (the research's first finding): it
  would cut the early chips most, but it takes BN6's skill reward away
  where it is felt first.
- *A pity counter in the run* (Slay the Spire's offset): the far detour's
  rich data already gives one good chip a layer where the walk earns it,
  without saving a counter (a `Run` change, `RUN_MAGIC`).
- *Rising prices or new sinks:* deeper zenny is plenty and the dealers'
  shelves and the Order Service (twice a dealer's price) absorb it; a
  rising sink (Slay the Spire's removal, Balatro's reroll) is the next
  lever if the statistics show hoards.
- *The traders as BN6's* (a code the pack lacks three times in four):
  the strongest single step for trading, a change to the traders' rules,
  not their numbers; proposed to the owner apart, and built (below). Their
  prizes stay chips new to the Library first, as the layers' traders gave
  them.

## The guard

1. **The report with bands.** `python3 build.py economy` prints both
   plans on both nets and flags (`!`) any act's chips, good share,
   Mystery Data, zenny or codes held a fifth past its plan's measured
   number (`econ_band`, `src/core/economy.c`: the endless net's six acts
   and Nest, the short net's three and Nest); it prints how many of the
   plan in play's (the opening one's) are outside: 0. Run it wherever
   drops, prices, Mystery Data, the dealers' stock or the levers change.
   A change that moves a number on purpose sets its band's centre in the
   same commit, with the report's before and after in its message.
   `--economy-plan flat` plays a game under the flat plan, to set beside
   the one in play (docs/DEVTOOLS.md).
2. **The curves' tests.** `tests/test_core.c`'s `test_economy`: the
   opening plan is the game's; every curve of it starts at its floor, ends
   at 1, rises ever more slowly and never past the flat plan's; the flat
   plan is 1 everywhere and draws no random number; the way through the
   endless and the short net; the shares kept by draws and by the hook's
   hash (none leaned on act 1, the same for the same battle, no draw);
   every band around its number.
3. **The run log.** Each battle's line keeps the zenny and BugFrags on
   hand, and a win's busting level and reward; the run's last line its
   economy's counts (docs/DEVTOOLS.md, Run log). Playtests and autopilot
   runs check the report against the game.
4. **The statistics.** The run-end event carries the run's economy, where
   the player said yes, numbers only: `chips` found (a battle's reward, a
   Mystery Data's, a Guardian Data's Navi chip), `good` among them (tier 2
   or more), `md` opened, `zenny` earned (battles and Mystery Data), and
   the same for its first act (`chips1`, `good1`, `md1`, `zenny1`). They
   are counted whatever the answer by the run log (`runlog_tally_*`,
   `runlog_md_opened`, src/director/runlog.h; `analytics_count_*`), kept
   beside the run's checkpoint (`run.tally`, carried in a `.cwsave`), and
   written in the run log's last line; docs/GUIDE.md's table says so. Real runs' first acts can then be set beside the
   report's act 1 (9.5 chips, 2.2 good, 11.6 Mystery Data, 2,630 zenny,
   every one of them opened and won).
5. **AGENTS.md's checks table** has a row for drops, prices, Mystery Data
   and the economy's curves: `build.py economy` with 0 outside the bands,
   `build.py test`, `build.py pacing` where battles' rewards change, and
   the game-design skill first.

## Decided (21 October 2026)

The owner said yes to every decision as proposed:

1. **The opening plan's shape and floors:** green data 0.5, a green
   data's chip 0.6, good chips 0.25, zenny 0.5, lean 0, bend 2; nothing
   above the flat plan anywhere.
2. **The short net** is back to the flat plan's by its own third act.
3. **The codes:** no lean on act 1's free finds; the dealer's answer keeps
   the folder's codes; the Chip and BugFrag Traders pick a prize's code as
   BN6 does (`src/layer/trader.c`: their pools' records kept as BN6 has
   them, docs/ROM_DATA.md's Chip Traders, verified in Gregar,
   docs/SOURCES.md).
4. **BN6's battle drops** stay as BN6 has them: the busting levels'
   rewards uncapped.
5. **The statistics** carry the economy's counts (above).
6. **The bands** a fifth either side.

### Checked when it was switched on

- `build.py economy`: 0 of the opening plan's numbers outside their bands
  on either net; act 1 9.5 chips (from 12.2), 23% good (2.2 chips, from
  3.2), 11.6 Mystery Data, 2,630 zenny (from 3,550), three of one code
  by the guardian (from five); the sixth act as before.
- `build.py pacing`: 0 battles or guardians past their band.
- The autopilot, seeds 1-12, fresh data folders, before and after (the
  same build but the plan; the autopilot opens no Mystery Data and buys
  nothing, so it sees the battles and the layers' new shapes, not the
  thinner finds):

  | Autopilot, 12 seeds | Before (flat) | After (opening) |
  | --- | --- | --- |
  | Plain, 30,000 frames: deepest layer (median, most) | 3, 4 | 3, 3 |
  | ... deleted in act 1 | 11 of 12 (one in act 2) | 12 of 12 |
  | ... first guardian reached, beaten | 8, 1 | 11, 0 |
  | ... random battles won in act 1, their chips and zenny | 28, 11, 3,400 | 20, 13, 1,150 |
  | `god,onehit`, 40,000 frames: deepest layer (median, most) | 12, 15 | 13, 15 |
  | ... act 1: battles won, chips, zenny (guardians' included) | 38, 10, 15,000 | 36, 10, 12,800 |
  | ... act 2 | 36, 13, 18,000 | 35, 10, 14,540 |
  | ... act 3 | 35, 12, 17,350 | 31, 7, 13,780 |

  The plain autopilot loses act 1 either way (its known rate: about one
  first guardian in ten, the memory of earlier tallies); after, it
  reached the guardian more often and beat him once less, within that
  noise: a battle's timing reshuffles a seed's encounters. Its battles'
  chips are BN6's rows' and stay; its zenny falls with the guardians'
  chip-entry zenny (300 in act 1, from 600) and the battles fought. No
  sign of a starved act in what the autopilot can see; the finds it
  skips are the report's to measure.
- Not checked on hardware or in a playtest: a trade at a layer's Chip
  Trader under BN6's code rule, and how a first act feels to a player
  who opens every Mystery Data and shops (the playtest loop's next
  session is the place).

## A dark layer's prize (25 October 2026)

A dark layer (docs/LIBERATION.md) pays its par's prize as BN5's
Liberation Missions do, the owner's decision of 25 October 2026: a chip
two under par or better, a chip within par, zenny over par. The chips are
a Server's prize and one a tier lower (`src/layer/lib_prizes.c`); the
zenny is BN5's first mission's 1000 at act 2 and 250 more an act, by the
zenny curve (`lib_par_zenny`). `build.py economy` prints them act by act,
each board's (a run meets one at most an act, by its choice at a fork, in
place of a layer's finds: not in the acts' sums or bands):

| Act | Two under: good, price | At par: good, price | Over par: zenny (flat, opening) |
| --- | --- | --- | --- |
| 2 | 69%, 2,335 | 47%, 1,677 | 1,000, 653 |
| 3 | 76%, 2,697 | 57%, 1,875 | 1,250, 959 |
| 4 | 80%, 2,863 | 67%, 2,345 | 1,500, 1,288 |
| 5 | 81%, 2,807 | 73%, 2,430 | 1,750, 1,636 |
| 6 | 74%, 2,593 | 73%, 2,428 | 2,000, 2,000 |

The price is a dealer's for that chip (`chip_price`), the yardstick
between a chip and zenny. BN5's own zenny (1000 a mission to 5000) would
pay 3,435 to 5,000 from act 4, more than the chip at par is worth, and
dawdling would beat racing the par; the report counts an act outside
where the zenny over par reaches the chip at par's price (0 now). The
opening plan's chips are as the flat plan's here: a Server's prize rolls
its rarer tiers by their own weight, which the good chips' lever leaves
alone.

## A held Soul's kind (25 October 2026)

A Soul unites with a chip of its kind given up from the hand (docs/META.md,
Souls), and a run cannot farm: the run's starting folders hold almost
none of the six kinds (the Blade folder none), so a won Soul often went
unused. The owner's decision of 25 October 2026, as
recommended: BN5's rule for the chip stays, and the chips come to the
folder instead (`src/core/loot_souls.c`).

- **Through the game-design skill.** The dialectic is the Souls' own, what
  you bring against what you find: the Soul is found, and now the finds
  answer it. It feeds the run's loop (the folder between layers), never the
  meta loop. The patterns are smart loot (Diablo 3's Loot 2.0: a drop
  leaned to its finder, as the finds already lean to the folder's codes)
  and a build-around (a held Soul gives the folder a direction), the costed
  power left as it was (each unison still takes a chip of the hand).
- **Finds:** a quarter of the Mystery Data's chips (`LOOT_SOUL_LEAN`, 25%)
  are of a held Soul's kind, of the tier the chip rolled (no Giga early:
  the counts and tiers as they were), where its tier has one, from the
  nets the layer's chips come from; the code lean after it keeps the chip
  and sets its code to the folder's where it has one.
- **Stock:** a Net Dealer's fourth chip is of a held Soul's kind, of the
  roll's tier or the nearest under it, else over it up to a rare standard,
  never one the folder is full of (his first is the act's answer, his
  second sometimes the viruses').
- **Which kind:** of the held Souls' kinds, the one the folder holds fewest
  chips of, a draw among those tied. DarkChips are never leaned to.
- **When:** the Souls held as the layer is made (`souls_layer`, kept beside
  the checkpoint with the Souls), so a CONTINUE rebuilds the same stock; a
  Soul won on a layer counts from the next. A run that holds none draws
  nothing more: the plans' numbers above are as they were.

The share was set with `build.py economy`, which holds each Soul alone from
the first layer (300 runs, the opening plan): the chips of its kind gained
per run by act (the finds, the battles' rows, the field's data, the
Guardian Data), before (none held) and now, and on the dealers' lists.
Every chip gained over the net stays 82.6 (82.3 to 82.5 with a Soul held).

| Soul (kind) | Act 1 | 2 | 3 | 4 | 5 | 6 | Nest | Dealers' lists an act |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Colonel (Obstacle) | 0.5 → 1.5 | 0.5 → 1.5 | 1.0 → 2.7 | 1.0 → 3.0 | 1.1 → 4.0 | 1.1 → 3.6 | 0.3 → 1.2 | 3.0 to 3.5 |
| ShadowMan (Invisible) | 0.0 → 0.2 | 0.1 → 0.3 | 0.1 → 0.6 | 0.1 → 0.7 | 0.2 → 1.3 | 0.3 → 1.3 | 0.1 → 0.5 | 2.5 to 3.1 |
| NumberMan (Plus) | 0.2 → 1.4 | 0.1 → 1.2 | 0.2 → 2.1 | 0.2 → 2.2 | 0.2 → 3.2 | 0.1 → 2.9 | 0.0 → 1.1 | 2.6 to 3.1 |
| TomahawkMan (Wood) | 0.4 → 1.5 | 1.0 → 2.0 | 1.4 → 3.3 | 0.8 → 2.7 | 1.3 → 4.2 | 0.9 → 3.4 | 0.3 → 1.3 | 2.8 to 4.3 |
| KnightMan (Break) | 0.3 → 1.4 | 0.7 → 1.8 | 0.9 → 2.7 | 0.6 → 2.7 | 0.7 → 3.8 | 0.5 → 3.2 | 0.3 → 1.3 | 2.8 to 4.4 |
| ToadMan (Aqua) | 1.6 → 2.7 | 0.9 → 1.9 | 0.7 → 2.7 | 0.9 → 2.9 | 1.5 → 4.3 | 1.1 → 3.6 | 0.6 → 1.5 | 4.1 to 5.9 |

A Soul won at act 1's end so finds one or two more of its kind an act
from the second, three from the fifth, beside about three on the dealers'
lists: about four found by act 3's end, where a folder of thirty holding
four or five of them has one in a first hand of five more often than not,
while three finds in four stay as they roll. ShadowMan's kind
has the fewest chips of any (Invisibl, Mine, AntiDmg and BN5's Blinder,
none in most tiers), so his come mostly from the dealers.

## Where it is decided

| What | Where |
| --- | --- |
| The plans, their curves, the bands, the report's assumptions | `src/core/economy.c` |
| A Mystery Data's content | `src/core/loot.c` (`loot_mystery_roll`); its key items `src/layer/layer_objs.c` (`fill_mystery`) |
| Green and blue data on a layer | `src/net/net_gen.c` (`place_data`) |
| Chip rarity, codes leaned to the folder | `src/core/loot.c` (`roll_chip`, `loot_fit_code`, `loot_folder_code`) |
| A guardian's chip-entry zenny | `src/core/loot.c` (`loot_guardian_zenny`), `src/director/encounter.c` |
| A dark layer's par prize: its chips, the zenny over par | `src/layer/lib_prizes.c`, `src/director/liberation_board.c` (`lib_par_zenny`) |
| Battle rewards (BN6's rows, the lean) | `src/director/encounter.c` (`lean_row`) |
| Dealer stock, prices | `src/layer/shop.c`, `src/core/loot.c` (`chip_price`) |
| A held Soul's kind in the finds and on a dealer's list | `src/core/loot_souls.c` (`LOOT_SOUL_LEAN`), the Souls as the layer was made `src/director/souls.c` (`souls_layer`) |
| Traders: their pools as BN6 has them, its code pick | `src/layer/trader.c` |
| A dark layer's chips (docs/LIBERATION.md): its par's prizes, its item panels' and its BonusPanels' (one of four, the item panels' band, as BN5's are its mission's standard chips) | `src/layer/lib_prizes.c` |
| Requests' pay | `src/core/jobs.c` |
| The report | `src/dev/economy_report.c` |
| The run's counts, sent at its end | `src/director/runlog.c` (kept, saved as `run.tally`, a battle's reward and a Guardian Data's chip counted), `src/director/director_jobs.c` (the Mystery Data opened, as a layer is left), `src/analytics/analytics.c` (`analytics_count_*`, `analytics_run_end`) |
