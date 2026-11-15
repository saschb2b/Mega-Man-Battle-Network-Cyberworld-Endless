---
type: Tool
title: "NPC animations: another game's sprites play what they draw"
description: "`python3 build.py npc-anims` (the host build's `--npc-anims`, the ROMs read) checks every NPC the game places in BN5's or BN4's sprite against the sprite copied into the core: each animation our code can tell it to play is there, and one it stands in draws something, failing where one does not (issue #614)."
tags: [devtools, sprites, multirom, npc]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T15:30:00Z }
---

# NPC animations: another game's sprites play what they draw

BN6 plays an NPC's animation by its number in the sprite's table and never
checks it against the sprite's count: past the last it reads stray bytes
as an animation, which draws stray tiles or nothing and never ends. Issue
\#614 was one: BN4's GutsMan, logging in again after a CONTINUE by BN6's
log-in animation (`0x19` on), which his sprite of 18 lacks, stood unseen
over his Guardian Data waiting for its end, and A reached nothing
([BN4's Navis as guardians](../game/bosses/bn4-navis.md)). The other games'
overworld sprites hold other counts and other layouts than BN6's
([BN4 Red Sun's tables](../rom/bn4.md), Sprite lists), and some draw
nothing at a facing BN6's would.

```bash
python3 build.py npc-anims   # builds the host binary, runs it with the ROMs; .build/npc-anims.txt
```

Local only: it needs BN6's ROM and BN5's and BN4's beside it
(`CYBERWORLD_ROM_DIR`, else `~/.cache/mmbn-ref/roms`). The host build's
`--npc-anims PATH` (`src/dev/npc_anims.c`) starts the core, copies in
every sprite as a run does (`src/layer/xnavi.c`) and reads each through
`src/layer/npc_sprite.c`, the engine's own reader of a sprite's table:
its count (the first offset over four) and whether an animation's first
frame draws anything (an object list that is not empty, tiles not all
colour 0: BN4's blank facings are one tile of nothing). A line per NPC:

| NPC | What its code tells it | Checked |
| --- | --- | --- |
| BN5's and BN4's guardians (`guardian_body`, `stage_npc.c`) | standing each of the eight ways, his pose, his log-in and log-out | standing and pose drawn, log-in and log-out there (their first frames are blank by design) |
| ... as a Glitch layer's copy (`layer_objs.c`) | a talker standing facing down-left | drawn |
| BN5's Darkloids on a dark layer's board ([Stage 4](../game/liberation/building/stage-4-darkloids.md)) | standing each way, his strike | drawn |
| The bystanders of another game's areas (`NetAreaDef.xnavi`) | a talker standing facing down | drawn |
| A liberation's teammates in BN5's sprites (`xnavi_mate`) | standing each way, walking and running as the player | standing drawn, the rest there |
| BigBrute, TinHawk and Bladia on the board (`liberation_look.c`) | standing each way, striking | standing drawn, strikes there |
| BN5's map objects (cube, wall of dark flames, dark hole, flame) and BN6's flame copied | the animations `blockers.c` and `director_dark.c` give them | there |

A talker turns to MegaMan when spoken to as BN6 turns one, to 1, 3, 5 or
7 (bn6f `npc_inChatbox_curAction_init_809EB20`), only where its sprite
draws all four (`npc_sprite_turns`); so that needs no line of its own.

## What it found (15 November 2026)

On `dev` before the fix, besides GutsMan's log-in (#617): WindMan's
standing facing left and up-left past his six animations, TopMan's and
SparkMan's facing up, up-right, left and up-left blank (BN4's PR #618
turned them to their sprites' own views as this check was written), and
BN5's CosmoMan blank facing up-right and up-left on a dark layer's
board: his sprite (BN5's list 6's 78) draws him only towards the camera,
2 to 5, as BN6's BlastMan's does. Any guardian whose sprite draws nothing
the way he faces now turns towards the camera (`guardian_body`), a log-in
or log-out his sprite lacks is the beam's (`npc_guardian`), and a talker
stands in a facing its sprite draws (`npc_sprite_stand`).

Beside it, in the same talker's code: a Glitch layer's copy
([the Glitch layer](../game/glitch-layer.md)) of a Falzar Navi, in his
battle sprite (list 0), stood in animation 4, every talker's standing
facing, which in a battle sprite is his warp-out, looping as streaks of
his colours. He now stands in his body's own animation (`guardian_body`,
0 for a battle sprite), and a battle sprite never turns to MegaMan
(its odd animations are its attacks). Seen on seed 1 at depth 7 with
`--dev glitch,side=4`; `CYBERWORLD_EMU_DEBUG` names the copy's form and
place (`copy: navi 6 at 412 -12`).
