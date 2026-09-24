# Progression

Research into how a run's difficulty was decided, what BN6's own data says
about difficulty, and how other roguelikes keep a run fair, and the
structure built from it. The rules live in `src/core/pacing.c`; the
[Status](#status) section lists what differs from the first proposal and
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

## How a run worked before

The state this research started from. The points that decide difficulty
are listed in [Where it is decided](#where-it-is-decided).

- **Acts.** 19 layers per cycle: acts 1-4 of 3 layers each in four areas
  drawn from 15 (`run.c`), act 5 the Graveyard, act 6 the Undernet, layer
  19 the Cybeast Nest. Layers 3, 6, 9, 12, 15, 18 and 19 have guardians, so
  18 and 19 follow each other. Later cycles repeat the same areas and
  guardians.
- **First area.** Any of Central, Seaside, Sky, Green, the Robot Control,
  Aquarium, Judge Tree, Mr. Weather and CopyBot comps, the two groups of
  home computers, and the Aquarium, ACDC, Green and Sky homepages.
- **Virus version by depth** (`virus_version`, `loot.c`): V1 on layers 1-4,
  V1 or V2 on 5-9, mostly V2 on 10-13, V3 on 14-19, one more per cycle.
  These steps do not fall on act boundaries. 3% of rolls at any depth,
  layer 1 included, turn one virus rare and the rest SP.
- **Virus families** are gated by where they first appear: families of the
  surface areas from layer 1, Undernet families from 7, Graveyard and Nest
  families from 13. Any family that Central, Seaside, Sky or Green lists
  counts as a surface family, so Sky's Catack, FgtrPlne and ScarCrow count
  as early. If no formation of an area passes, every formation is allowed.
- **Guardian version:** V1 on layers 3-9, EX on 12-18, SP in the Nest, the
  Secret Area and every later cycle. HP is the game's value for that version.
- **Healing:** a Recovery Mr. Prog on 30% of layers, and always in the room
  before an arena. HP carries over between layers; nothing heals after a
  battle or a guardian.
- **Growth:** 3 HPMemory (+60 max HP) per guardian, one HPMemory in each Net
  Dealer at `(10 + 2 × depth) × 100` zenny, a Cross from five of the Navis,
  Beast Out from the Graveyard's guardian. Guardians drop no chip.
- **MegaMan's start:** the game's own new game. 100 HP and the default
  folder of 30 chips: Cannon ×4, AirShot ×2, Vulcan1 ×3, MiniBomb ×4,
  Sword ×4, WideSwrd ×2, CrakShot ×2, AreaGrab, Atk+10 ×2, Recov10 ×4,
  BusterUp and WhiCapsl. That is about 50 damage per attack chip.
- **Optional fight:** a Server on `20 + depth` % of layers fights three
  layers deeper and one version higher. On layer 1 that is V2.

## What BN6's data says

All numbers come from the ROM (enemy stats at `0x00F260`, the random
battles at `0x020170`, the starting folder at `0x0213AC`, base navi stats at
`0x0210DD`), each with its note in `docs/ROM_DATA.md`.

### Viruses

Each family has six records (V1, V2, V3, SP and two rare forms) holding HP
and the damage its attacks read. Examples, V1/V2/V3/SP:

| Family | HP | Damage |
| --- | --- | --- |
| Mettaur | 40/80/120/160 | 10/20/40/60 |
| Gunner | 60/140/220/250 | 10/20/40/80 |
| Champy | 60/120/180/220 | 50/100/150/200 |
| Swordy | 90/140/160/200 | 30/60/90/120 |
| Puffy | 80/120/200/240 | 80/140/200/260 |
| Catack | 130/160/220/260 | 50/100/150/200 |
| Trumpy | 80/100/130/160 | 90/120/150/180 |
| WindBox | 130/130/160/160 | 100/100/150/150 |
| DarkMech | 180/240/270/300 | 50/100/150/200 |
| ErthDrgn | 200/230/260/300 | 100/120/170/190 |

Some attacks hit more than once or scale the value, so damage is a guide,
not an exact figure.

### Areas

Total virus HP of one random battle in each area, with the viruses at the
versions the game gives them there (median, and the highest):

| Area | HP per battle | Main families |
| --- | --- | --- |
| Robot Control Comp | 140 / 200 | Mettaur, Gunner, Champy, OldStov |
| Central 1-3 | 120-200 / 260 | Mettaur, Gunner, Champy, OldStov, Swordy, Quaker |
| Aquarium HP | 160 / 220 | Piranha, Puffy |
| Aquarium Comp | 200 / 360 | Piranha, Puffy, Quaker, StarFish |
| Sky HP | 200 / 260 | FgtrPlne, Gunner |
| Home computers (0x8C, 0x8D) | 80-420 by map | mixed, mostly V1 |
| Seaside 1-3 | 210-220 / 420 | Piranha, Puffy, Swordy, Mettaur2 |
| Judge Tree Comp | 230-320 / 550 | Armadill, Cragger, HonyBmbr, Shrubby |
| Green 1-2, Green HP | 270-300 / 410 | BombCorn, HonyBmbr, Armadill |
| Mr. Weather Comp | 280-390 / 520 | Catack, FgtrPlne, PulsBulb, ScarCrow, V2s |
| Sky 1-3 | 340-360 / 540 | FgtrPlne, ScarCrow, Catack |
| ACDC HP | 370 / 370 | Catack, Champy2, Mettaur3 |
| CopyBot's (Pavilion) comps | 330-510 / 670 | V2 and V3 |
| Undernet 1-4 | 360-450 / 660 | V2, BigHat, SnakeArm, DarkMech |
| Underground 1-2 (Nest) | 390-400 | Mettaur3, HeadyA, Nghtmare, ErthDrgn |
| Graveyard 1-3 | 500-580 | V3 and SP |

In the story these areas come roughly in this order, and the SP navis
hidden in them (BlastMan, DiveMan, CircusMan, JudgeMan, ElementMan, Colonel,
Bass) follow the story's bosses. Undernet battles are lighter than the
Graveyard's, which BN6 keeps for after the story.

The four random-battle tables are not story stages, as `docs/ROM_DATA.md`
and `formations.c` said. They are identical except for Central and Seaside,
where event flags `0x67F`-`0x681` swap in other battles for story scenes.

### Guardians

V1 HP per Navi (EX / SP in brackets):

| HP | Navis |
| --- | --- |
| 400-600 | BlastMan 400 (800/1400), DiveMan 500 (1000/1500), SpoutMan 600 (1300/1700) |
| 700-800 | HeatMan 700, CircusMan 700, SlashMan, EraseMan, TenguMan, JudgeMan 800 |
| 900-1000 | ElecMan, DustMan, ElementMan 900; ChargeMan, TomahawkMan, GroundMan 1000 |
| 1200+ | Colonel 1200 (1600/2000), ProtoMan 1800 (2000/2000) |

### MegaMan

BN6 starts at 100 HP; each HPMemory adds 20, up to 1000. The default folder
clears a Central battle (120-180 HP) in one or two turns and an Undernet
battle (about 400) in four. Over the story a player collects HPMemory and
chips area by area, which is what the rising virus HP is balanced against.

## What other roguelikes do

The games closest to this problem, and the part of each that transfers:

- **Into the Breach** lets the player pick any island first, and the first
  island is always the easiest. Difficulty follows how many islands are
  done, not which island. This is the direct answer to a random first area.
- **One Step From Eden**, made after Battle Network, shuffles its worlds
  and gives enemies tier 1-4 (like V1-V3/SP) from progress through the run.
- **Dead Cells** sets the enemy tier from the player's power (scrolls
  collected) and the stage, whichever is higher, fixed on entering a biome.
- **Slay the Spire** draws the first three fights of a run from an easy
  pool and keeps elites off the first five floors. A fixed rest before each
  boss, rewards after every boss, and a pity counter for rare cards keep the
  run readable. Its balance came from logging where players died.
- **Hades** shows each door's reward before you choose, marks harder rooms,
  and heals after each boss. Harder settings are opt-in after a first clear.
- **Monster Train and Risk of Rain 2** offer optional trials and shrines
  that make a fight harder for more reward.
- **Balatro** shows the next boss in advance so the player can prepare.
- **BN4** already tiered viruses by playthrough (V1 first, then V2, V3),
  and BN players' main complaint about BN difficulty is a boss that comes
  before the chips needed to beat it.

Sources: [StS map generation](https://slaythespire.wiki.gg/wiki/Map_Generation),
[StS monsters](https://slaythespire.wiki.gg/wiki/Monsters),
[StS metrics talk](https://www.gdcvault.com/play/1025731/-Slay-the-Spire-Metrics),
[Into the Breach difficulty](https://intothebreach.fandom.com/wiki/Difficulty),
[OSFE zones](https://onestepfromeden.fandom.com/wiki/Zones),
[OSFE enemies](https://onestepfromeden.fandom.com/wiki/Enemies),
[Dead Cells scaling](https://steamcommunity.com/app/588650/discussions/0/1741094390472524548/),
[Hades Pact](https://hades.fandom.com/wiki/Pact_of_Punishment),
[Monster Train trials](https://monstertrain2.miraheze.org/wiki/Trials),
[Balatro blinds](https://balatrowiki.org/w/Blinds_and_Antes),
[BN4 tiers](https://lparchive.org/Mega-Man-Battle-Network-4-6/Update%2029/).

## The structure

### Principles

1. Depth decides how hard a battle is. The area decides which viruses and
   which Navi, within that limit.
2. Every limit is measured against the max HP MegaMan is expected to have
   at that depth.
3. The player sees a threat before committing to it, and extra risk is a
   choice that pays.
4. Everything is decided from the run and layer seeds, before the game's
   RAM is involved, so layers stay reproducible.

### The act

Every act keeps three layers and gets a fixed rhythm:

| Layer | Battles | Services |
| --- | --- | --- |
| First | The first two battles from the easier half of the act's band | Mystery Data, a shop at 50% |
| Second | The full band; a marked Server challenge may appear | Net Dealer and Recovery Mr. Prog, always |
| Third | The full band | Heal and Net Dealer before the arena, then the guardian |

Beating the guardian heals MegaMan fully and gives the reward (see
[Growth](#growth)). The act card names the area and its guardian, so the
player can prepare the folder for it.

### Targets per act

For the first cycle. MegaMan's HP assumes the growth below and no HPMemory
bought; the value before this structure is in brackets.

| Act | Layers | MegaMan max HP | HP per battle | Hardest hit | Versions | Guardian |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 1-3 | 100 (100) | 100-200 | 50 | V1 | 400-600 V1 |
| 2 | 4-6 | 200 (160) | 150-280 | 80 | V1 | 600-800 V1 |
| 3 | 7-9 | 300 (220) | 200-360 | 120 | V1-V2 | 800-1000 V1 |
| 4 | 10-12 | 400 (280) | 280-420 | 160 | V2 | 1100-1300 (Colonel V1, EX of the 800s) |
| 5 | 13-15 | 500 (340) | 350-500 | 200 | V2-V3 | 1100-1500 EX (the Undernet) |
| 6 | 16-18 | 600 (400) | 400-580 | 240 | V3 | 1200-2000 EX (the Graveyard) |
| Nest | 19 | 700 (460) | 450-650 | 280 | V3, SP | SP |

"Hardest hit" is the damage value of the strongest virus in a formation,
kept to about 40% of MegaMan's HP, so no single hit takes more than half.
At act 1 that keeps Puffy, Trumpy, WindBox and ScarCrow (80-100 damage)
out until MegaMan has grown.

### Choosing battles

For each formation the area offers, the engine sums its viruses' HP and
takes the highest damage value, reading both from the ROM's enemy table for
each version. It then picks the highest version, up to the act's, at which
the formation fits the act's HP band and damage cap. A formation that does
not fit even at V1 is left out on that layer. If an area leaves nothing,
the band widens by one step before any formation is allowed.

Two more rules from Slay the Spire: the first two battles of a run, and the
first battle after each guardian, come from the lower half of the band, and
the same formation does not come twice in a row.

A rare virus (one, the others unchanged) may come from act 3 on, 3% of
battles and 2% more each later cycle, and only when the battle still fits
the band with it. BN6's own rare battles (byte 7 of a BattleSettings record)
are left out: the game holds them back until a family has been deleted 16
or 32 times.

### Choosing areas

Areas fall into three tiers by their own battles:

| Tier | Areas |
| --- | --- |
| Opening | Central, Robot Control Comp, Aquarium Comp, Sky HP, the first home computers (0x8C) |
| Middle | Seaside, Judge Tree Comp, Green, Green HP, the homepages (0x88, Aquarium HP's battles among them), the second home computers (0x8D) |
| Late | Sky, Mr. Weather Comp, ACDC HP, CopyBot's comp |

Act 1 draws from the opening tier, act 2 from opening or middle, act 3 from
middle or late, act 4 from late. Every run still varies, and the areas
whose viruses are late in BN6 come late in the run. The battle budget above
keeps any area safe even when a tier runs short.

Acts 5 and 6 changed places: the Undernet's battles are lighter than the
Graveyard's, and BN6 keeps the Graveyard for after the story. Beast Out now
comes from the last guardian before the Nest. Runs saved before keep their
order.

### Choosing guardians

Each area keeps its pool of Navis. A guardian is drawn from the Navis of
the area's pool whose HP at the act's version falls in the act's band;
when none does, from all Navis in the band; only then the pool's nearest.
Acts 1-3 fight V1, acts 4-6 EX where it fits and V1 otherwise. Act 1 meets
BlastMan, DiveMan or SpoutMan, and Colonel appears from act 4 on. Over 500
drawn runs every guardian lies in its act's band (`build.py pacing`).

### Growth

MegaMan reaches about 100 HP more per act:

- A guardian gives 5 HPMemory (+100) in place of 3, and heals MegaMan fully.
- A guardian also gives its own Navi chip at the version fought (V1, EX,
  SP). Beating a Navi and getting its chip is Battle Network's own reward.
- From act 2 on, 15% of the rich Mystery Data (the best of three
  qualities) hold an HPMemory, blue as the game keeps them.
- The Net Dealer's HPMemory costs 1200 zenny in act 1 and 600 more each
  act, not more each layer.

A start gift adds a choice before the first battle: on layer 1 a Mr. Prog
offers one of three (two HPMemory, a ★3 chip, a NaviCust program). If the
last run ended before the first guardian, he gives an HPMemory first
(`Profile.last_depth`, which took the place of an unused field).

### Risk the player chooses

- The Server challenge never appears on layer 1 and fights the next act's
  band one version up, at the layer's own depth (not three layers deeper).
  A win pays a chip from the best Mystery Data's roll on top of the game's
  own reward. From act 4 it may hold the area's SP Navi, who keeps his own
  HP.
- The dark flame into the Undernet stays a side route with tougher viruses
  and better Mystery Data.
- Busting Level stays the game's: a clean, fast battle already earns better
  drops.

### Later cycles

After the Nest the band and the damage cap rise by a quarter per cycle and
versions by one, capped at SP; guardians are SP. Two rules change besides
the numbers: rare viruses come 2% more often each cycle, and from the third
cycle the middle layer's heal is no longer certain.

## Status

Built as described above, with these differences from the first proposal:

- SP Navis in Server challenges wait until act 4, not act 3: at act 3
  MegaMan has about 300 HP against their 1400-1700.
- The guardian bands of acts 5 and 6 are 1100-1500 and 1200-2000, so the
  Undernet's and the Graveyard's own Navis (SlashMan, HeatMan, EraseMan,
  DustMan at EX) can hold them.
- A run lost early earns an extra HPMemory before the gift, not a fourth
  choice.

Not built: a second guardian in the Nest on later cycles, and difficulty
levels to choose on the title screen.

Fixed on the way:

- The four random-battle tables are event variants, not story stages
  (`docs/ROM_DATA.md`, `formations.c`).
- Colonel was fought as navi 17 of the enemy table, an unnamed navi with
  4000 HP at V1; he is navi 18 (1200 HP). Saved runs and his rival record
  move over on loading.
- A guardian battle's `no_escape` was never used and is gone; a guardian
  MegaMan runs from waits to fight again, as before.
- A Server challenge on layers 16-19 drew from the next cycle's depth.

## Measuring it

- `python3 build.py pacing` rolls every area's battles in every act and
  draws 500 runs' guardians, and marks anything past its band
  (docs/DEVTOOLS.md). It reports 0 now. Median battle HP by act: about
  140, 200, 270, 340, 400 and 500, and 510 in the Nest.
- `tests/test_core.c` checks the acts, bands, versions, area tiers, guardian
  choice and heals without a ROM.
- `runlog.txt` in the data folder records every battle (area, foes, their
  HP, MegaMan's HP before and after) and where each run ended.

## Where it is decided

| What | Where |
| --- | --- |
| Acts, bands, versions, area tiers, guardian bands, heals | `src/core/pacing.c` |
| Area order, guardian pools | `src/core/run.c` (`run_new`) |
| Act and guardian layers | `src/net/net_gen.c` (`biome_for_depth`, `is_boss_depth`) |
| Formation pick, versions inside the band | `src/core/loot.c` (`make_encounter`) |
| Guardian version | `src/core/loot.c` (`make_boss`) |
| Chip rarity and prices | `src/core/loot.c` (`roll_chip`, prices) |
| Services and Mystery Data per layer | `src/net/net_gen.c`, `src/layer/layer_objs.c` |
| Opening battles, challenge and its reward | `src/director/director.c` |
| Guardian rewards, the start gift | `src/layer/scripts.c`, `src/layer/guardian_objs.c`, `src/director/powers.c` |
