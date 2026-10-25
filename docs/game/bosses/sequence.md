---
type: Feature
title: "The sequence (src/director/boss.c)"
description: "The guardian fight's states in `src/director/boss.c` (enter, title card, talk, battle, last word, log-out, Guardian Data, open exit), its event flags, BN6's no-song 0x63 for silence, and MegaMan stepping up beside the guardian (`guardian_stand`)."
tags: [game, bosses]
generated: { by: claude-code/unrecorded, at: 2026-10-06T18:14:19Z }
---

# The sequence (`src/director/boss.c`)

| State | What happens |
| --- | --- |
| Wait | The guardian's NPC waits hidden in the arena's middle |
| Enter | MegaMan steps into the arena: bars slide in, the music stops, MegaMan steps up to his place beside the guardian's and turns to it; the boss prelude starts, the guardian logs in facing him as BN6's Navis do (BN6's beam rising, its sound 0x76, and his sprite's own log-in, `0x19` + his facing) and the screen shakes; the card waits for MegaMan to be there |
| Title | The title card, while Gregar's own Navis strike their signature pose (animation 24) |
| Talk | The intro lines; input is A and B only |
| Fight | The game's navi battle, forced at once (the random battle record is not re-rolled over it) |
| After | Back on the map: bars, silence |
| Last word | Its defeat line |
| Log out | His sprite's own log-out (`0x21` + his facing: a frame in his lighter colours, then gone) beside BN6's beam and its sound, about a quarter of a second, as BN6's Navis log out on the net; no flash and no fading by alpha, which BN6 never does (the owner saw the old fade as a "blup"); then a beat, 45 frames from its start |
| Reward | The Guardian Data shows (sound 0x94), BN6's Mystery Data crystal turning (its sprite's animation 0); the area's theme returns and MegaMan walks to it. Taken, it gives in BN6's own "MegaMan got:" boxes, MegaMan's one word on a Cross and on a first meeting's battle data, the heal, and the draft: one box and its menu |
| Open | Taken: the exit pad appears and its warp works |

Event flags `0x1448`-`0x144C` drive the NPCs: the guardian leaves, logs in,
the Guardian Data shows, it was taken, the exit shows. While the exit is
shut, flag `0x16F1` keeps its warp from starting.

The silence is BN6's own "no song", 0x63, which its `PlayMusic` stops
every song for, as its scripts hush a scene. Until 6 October 2026 the
hush played song 0xFF, which stopped nothing: the area's theme played on
as MegaMan stepped in, and after the battle, where the game starts the
map's theme a frame after the battle anyway, under the guardian's last
word (the music's status read each frame, `BN6_MUSIC_STATUS`). The hush
after the battle now runs while a song plays, in the 40 frames before the
last word.

MegaMan meets the guardian beside him (the owner's call, 6 October 2026):
BN6's camera follows MegaMan and the chat box fills the picture's lower
third, so where the arena lay below its way in, a guardian he walked
straight at stood half behind the box through the talk. Now he steps up
to a place at the guardian's height on the screen, on the side the
bridge comes in from, and turns to him (`guardian_stand`: 24 world units
off on both axes, 48 pixels across the picture, a super boss 28, his
sprite being wider); the guardian logs in facing him (BlastMan, ElementMan
and Bass, drawn only towards the camera, turned down-left where he stands
on their left). The steps are the staging's own, as BN6's cutscenes walk
MegaMan (`cs_move_player_in_facing_direction`: as if the player walked,
the pad the cutscene's): the player's pad is held from the step in to
the battle, as before, and his own walking is BN6's everywhere else. The
place is worked out from the arena's way in, so it holds whichever side
a layer brings MegaMan in from; `tests/test_core.c` checks it on every
generated arena (floor under his feet, the bridge's side). A layer without
an arena keeps the old meeting, straight at the guardian.
