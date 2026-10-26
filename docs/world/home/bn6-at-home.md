---
type: Research
title: "What BN6 has at home"
description: "What BN6 itself has at home, verified in bn6f and Gregar: Central Town's maps, Lan's PC jacking in to Lan's HP (0x88:00), jack-in points, the Request BBS, AsterLand's Order Service, no day or night."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-05T20:36:44Z }
---

# What BN6 has at home

Verified in bn6f (Falzar, its names; `~/.cache/mmbn-ref/bn6f`) and in
Gregar's ROM unless said otherwise, 5 October 2026:

- **Lan lives in Central Town**: its group (`0x01`) holds the town
  (`0x00`), Lan's house (`0x01`), his room (`0x02`), the bathroom (`0x03`)
  and AsterLand (`0x04`) (bn6f `constants/enums/GameAreas.inc`).
- **Lan's PC jacks in to Lan's HP**: the jack-in table of Lan's room
  (Gregar `0x0804F874`, read through the literal its group's map-enter
  routine stores to GameState `+0x64`) sends point `0x40` to destination
  1, which is Lan's HP (`0x88:00`, at -178, -14). Lan's HP is a small
  room in cyan frames with a dark circuit floor and two warp pads, blue
  and pink (drawn from the ROM with `--atlas DIR:g88`); no area of ours
  takes that map (the layers take ACDC, Aquarium, Green and Sky HPs,
  `0x88:01`, `03`, `05`, `06`).
- **The towns' other points lead to their objects' comps**: Central
  Town's two (`0x40`, `0x41`) to destination 0, the RoboDog Comp
  (`0x8C:00`); ACDC Town's squirrel statue to ACDC HP (`0x88:01`) and its
  doghouse to the doghouse comp (`0x8D:0E`); the bathroom to the
  bathroom comp (`0x8C:06`). BN6 has a comp per jack-in object: the fish
  stick shop's, the water machine's, the symbol's, the popcorn shop's
  and more (groups `0x8C` and `0x8D`).
- **The Request BBS** is in the real world, a board in AsterLand
  (Central Town): 35 requests in the US text, posted by students,
  shopkeepers, a film crew, the NetPolice, Mr. Famous and others; each
  names a meeting place; "You can only pick one request at a time."
  (`CompText87EFE14`). Ranks C to Master by points. Rewards: Virus
  Deletion pays 10 BugFrags (`CompText876C2A8`); others (RegUP2,
  HPMemory) are the community's word, not verified.
- **AsterLand's counter** (Central Town, its clerk Tab) has In-Stock Chips
  (shop 4) and the Order Service (shop `0x12`): a paged list built from
  per-chip flags, the Library's among them (`sub_8048DF8`); for a chip
  picked the clerk names one code and a price ("I've got some of that
  chip in code X! It will be N Z"), or says it is not in stock; the chip
  is given at once (`CompText87F2094`). Higsby's in ACDC Town is closed
  all game ("off in Netopia... again").
- **A SubChip seller** stands in each town (shops `0xB`-`0xF`: ACDC,
  Central, Seaside, Green, Sky).
- **Jack-in points in objects**: R on trigger cell `0x40` + n jacks in
  unless flag `0x16D0` + n is set (`sub_8034CB6`), each to the
  destination its map's table names (destination table `byte_80984C8`,
  Gregar `0x08099A00`, 43 of 20 bytes).
- **Jacking out**: R on a net map asks "MegaMan, jack out?"
  (`CutsceneScriptAskJackOut_80988E4`); Lan stands again where he jacked
  in, and MegaMan's HP is made full at both the jack-in (`sub_8033FDC`)
  and the jack-out (`navi_80340F6`). The jack-in also rolls every net
  map's green Mystery Data again ([The net's maps and their pieces](../../rom/net-maps.md), Map takeover).
- **No day or night**: one palette per map, chosen by its group and
  number (`initMapTilesState_803037c`); nights are captions ("That
  night..."). BN6 does have story weather in its towns (rain, snow, wind)
  and a looping palette dim in Green and Sky Town (events `0xA9B`,
  `0xA9C`).
- **Its people change with the story**: four stage archives per map,
  picked by GameState `+0x08` (`chatbox_selectCompTextByMap_8040730`),
  and map scripts set out other people per chapter. The living room's TV
  is a check; news comes in cutscenes.
