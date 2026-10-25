---
type: Feature
title: "4. Central Town, home"
description: "Central Town as every run's home (issue #94): its original width, Lan's house and room taken over (`lan_house.c`), his PC the run's jack-in to Lan's HP, and his room's words recalling the last run."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-21T00:02:02Z }
---

# 4. Central Town, home

Every run's town is Central Town: Lan's house and his room join it as
BN6 has them, and his PC is the run's jack-in. The other towns' plans
stay for later uses (a job's trip by the Metroline), not as a run's
home. A home is the same place every run, so the people who remember can
be noticed.

As built (issue #94): the town is Central Town at its original width,
the same plan every run (`CYBERWORLD_TOWN_STYLE` and `_VARIANT` still
pick another for a test). A run begins in Lan's room, at the top of its
stairs where BN6 sets him down; the run's words play there, and R at his
PC jacks MegaMan in to Lan's HP (`lan_house.c`). Those words remember how
the last run went (`town_lines.c`): one left unfinished ("We never
finished that last dive..."), whether a saved run was started over or a
run was quit before its first save (the profile's `run_open`, set as a
run begins and cleared as it ends; session 74 heard "Dad's backup got me
home safe last time." after a plain quit), and Dad's backup only after a
deletion. BN6's own jack-in from
the PC (destination 1) runs a line of its story's that turns Lan back
("Lan,let's check out the town first!"), so the room's table goes
through the town's destination with Lan's plain "Jack in!". The house
and the room are BN6's maps, their furniture, checks and song (`0x04`)
kept, no people or story scripts; the room's stairs and the house's
front door and stairs are BN6's doors, the bathroom's left out, and the
front door lets Lan out where the planned town has it. The town keeps
its houses, people and checks, and Lan's front door (its warp 1, carried
with his house) is its door home (AsterLand's and the Academy's open
too, piece 10); it has no jack-in of its own. R in the
town, the house, or the room off the PC has MegaMan say where the PC is
("Home's right here,Lan! The PC's up in your room!"). BN6's jack-out from
Lan's HP sets Lan down at his PC. In the house the arrow leads from the
front door straight up past the sofa's end before it turns to the
stairs: the straight line runs into the shoe cabinet and the sofa
(sessions 72 and 73, `lan_house_way`).
