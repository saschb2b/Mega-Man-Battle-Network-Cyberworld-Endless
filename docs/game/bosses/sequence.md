---
type: Feature
title: "The sequence (src/director/boss.c)"
description: "The guardian fight's states in `src/director/boss.c` (enter, title card, talk, battle, last word, log-out, Guardian Data, open exit; a tournament's bracket, rounds and prizes between), its event flags, BN6's no-song 0x63 for silence, and MegaMan stepping up beside the guardian (`guardian_stand`)."
tags: [game, bosses]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T15:11:58Z }
---

# The sequence (`src/director/boss.c`)

| State | What happens |
| --- | --- |
| Wait | The guardian's NPC waits hidden in the arena's middle |
| Enter | MegaMan steps into the arena: bars slide in, the music stops, MegaMan steps up to his place beside the guardian's and turns to it; the boss prelude starts, the guardian logs in facing him as BN6's Navis do (BN6's beam rising, its sound 0x76, and his sprite's own log-in, `0x19` + his facing) and the screen shakes; the card waits for MegaMan to be there |
| Title | The title card, while Gregar's own Navis strike their signature pose (animation 24) |
| Talk | The intro lines; input is A and B only. A B held as the talk opens counts once let go, so a player running in reads the lines; one pressed while the staging held MegaMan hurries them (a CONTINUE from the arena's door, MegaMan back on the step in, begins the staging at once: a playtester's B pressed under the title card waited on the first box, session 77) |
| Fight | The game's navi battle, forced at once (the random battle record is not re-rolled over it) |
| After | Back on the map: bars, silence. From here the map (SELECT) leaves out his red Boss mark and its key's entry: two playtesters read it on the arena after he was gone (session 78) |
| Last word | Its defeat line |
| Log out | His sprite's own log-out (`0x21` + his facing: a frame in his lighter colours, then gone) beside BN6's beam and its sound, about a quarter of a second, as BN6's Navis log out on the net; no flash and no fading by alpha, which BN6 never does (the owner saw the old fade as a "blup"); then a beat, 45 frames from its start |
| Reward | The Guardian Data shows (sound 0x94), BN6's Mystery Data crystal turning (its sprite's animation 0); the area's theme returns and MegaMan walks to it. For its first second (`REWARD_SEEN`, 60 frames) no A reaches it, and an A held from then counts only once let go: two playtesters' A's, pressed in step to page the last word, came 5 and 14 frames after it showed, and A's reach turned MegaMan and walked him to it from a panel and a half away (session 78). Taken, it gives in BN6's own "MegaMan got:" boxes, MegaMan's one word on a Cross and on a first meeting's battle data, the heal, and the draft: one box and its menu. Its chat clears the shown flag as it opens, and the crystal goes as the boxes close, before the draft's screen: BN6 runs no NPC's script while its chat is open, as its own Mystery Data goes when its box closes; it had waited for the taken flag at the chat's end, through the pick |
| Open | Taken: the exit pad appears and its warp works. The pad draws over the map's second layer (layer priority 2), still under MegaMan: BN4's Town Area lays its pale floor there, and the pad under it was unseen (session 78), as BN5's looks hid a dark layer's |

Where the tournament's bracket stands in the guardian's place
([the tournament](../tournament.md), As built), Enter shows BN4's bracket
screen first (Bracket, off the map), and each round comes back to it:
Round in (a round's Navi straight to his card; the final's log-in as
above), the title card, the battle, Prize (a round's Navi chip) and the
bracket again, until the final's battle, after which the bracket shows
MegaMan the winner before After. A round has no talk and no last word.

Event flags `0x1448`-`0x144C` drive the NPCs: the guardian leaves, logs in,
the Guardian Data shows, it was taken, the exit shows. A CONTINUE enters
the map again, which starts the NPCs anew: the guardian and his beam,
gone already, end at once rather than play his log-in again (issue
#614: BN4's GutsMan, waiting on a log-in animation his sprite lacks,
stood unseen over his data, and no A reached it). While the exit is
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

A slide ends at the arena's door (issue #697). SlipRunr on the NaviCust
slides MegaMan on B until a wall or an object stops him, and a slide
reads no pad, so the staging's steps never reached him: a player slid
in, stopped on the guardian's own spot, met him there and stood on his
Guardian Data. BN6's own scenes never begin mid-slide (its triggers wait
for its end, bn6f `sub_8034C36`); this one ends it as a wall does
(`sub_809DE30`: flags `0x173D` and `0x173E` cleared, `0x173F` set until
B is let go, [the game's state](../../rom/game-state.md)), on the frame
he steps in and before the door's save, so he stops on the step in and
walks up to his place as a walker does. The state's `met beside` or
`met astray` says where the steps left him, from the card to the
battle; `tests/scenarios/battle_guardian_slide.py` slides in
(`--dev sliprun`, the `by arena` step: [Scenarios](../../devtools/scenarios.md)).
