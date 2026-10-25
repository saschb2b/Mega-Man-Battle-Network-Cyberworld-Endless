---
type: Research
title: "What BN6's data says"
description: "What BN6's ROM data says about difficulty: virus families' HP and damage by version, each area's battle HP from Central to the Graveyard, guardians' V1, EX and SP HP, and MegaMan's 100 HP start."
tags: [game, progression]
generated: { by: claude-code/unrecorded, at: 2026-09-27T21:34:54Z }
---

# What BN6's data says

All numbers come from the ROM (enemy stats at `0x00F260`, the random
battles at `0x020170`, the starting folder at `0x0213AC`, base navi stats at
`0x0210DD`), each with its note in `docs/rom/`.

## Viruses

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

## Areas

Total virus HP of one random battle in each area, with the viruses at the
versions the game gives them there (median, and the highest):

| Area | HP per battle | Main families |
| --- | --- | --- |
| Robot Control Comp | 140 / 200 | Mettaur, Gunner, Champy, OldStov |
| Central 1-3 | 120-200 / 260 | Mettaur, Gunner, Champy, OldStov, Swordy, Quaker |
| Aquarium HP | 160 / 220 | Piranha, Puffy |
| Aquarium Comp | 200 / 360 | Piranha, Puffy, Quaker, StarFish |
| Sky HP | 200 / 260 | FgtrPlne, Gunner (and the Sky's battles: see below) |
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

The four random-battle tables are not story stages, as `docs/rom/`
and `formations.c` said. They are identical except for Central and Seaside,
where event flags `0x67F`-`0x681` swap in other battles for story scenes.

## Guardians

V1 HP per Navi (EX / SP in brackets):

| HP | Navis |
| --- | --- |
| 400-600 | BlastMan 400 (800/1400), DiveMan 500 (1000/1500), SpoutMan 600 (1300/1700) |
| 700-800 | HeatMan 700, CircusMan 700, SlashMan, EraseMan, TenguMan, JudgeMan 800 |
| 900-1000 | ElecMan, DustMan, ElementMan 900; ChargeMan, TomahawkMan, GroundMan 1000 |
| 1200+ | Colonel 1200 (1600/2000), ProtoMan 1800 (2000/2000) |

## MegaMan

BN6 starts at 100 HP; each HPMemory adds 20, up to 1000. The default folder
clears a Central battle (120-180 HP) in one or two turns and an Undernet
battle (about 400) in four. Over the story a player collects HPMemory and
chips area by area, which is what the rising virus HP is balanced against.
