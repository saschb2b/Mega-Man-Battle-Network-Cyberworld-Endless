---
type: Feature
title: "8. Time of day by act"
description: "Time of day by act (issue #90, `home_hour`): morning to night before the Nest, tinted onto Central Town's map and sprites as `scene_emu.c` draws it, since BN6 has no time of day."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-06T09:44:33Z }
---

# 8. Time of day by act

Morning at the start, afternoon after act 1, evening after act 2, night
before the Nest: the run's progress on the town's sky. BN6 has no time of
day, so this is the engine's, and it leans on BN6's own effects first:
its towns' palette dim (Green and Sky Town's), its rain, snow and wind,
then a tint of the town's palettes where BN6 has none. Judged by captures
at every hour before it ships; [Fidelity](../../game/fidelity.md) names it.

As built (issue #90; the owner looked at the captures of every hour and approved them as they are, 6 October 2026):
BN6's own dim is a palette animation of Green and Sky Town's (their map
scripts load it while event `0xA9C` is set), which Central Town has none
of, so the hour is the engine's. Its picture is tinted as it is drawn
(`scene_emu.c`; draws change nothing in the game): each colour channel of
the map's layers and of the sprites over it times the hour's, a touch
warm in the afternoon, orange in the evening, a dim blue at night. mGBA's
picture names each pixel's layer (its top byte), so the chat's text
(BG0) keeps its colours, and with a chat box open so do the sprites in
its rows (its frame and the speaker's face). Only the town's own map,
through its doors' fades too (the game is off the map while one fades,
and a door first flashed daylight), not under the PET's menu: Lan's
house, AsterLand, the Academy and Lan's HP keep BN6's light. The
hour is the run's (`home_hour`): morning at its start, then afternoon,
evening and night before the Nest (the endless net two acts an hour, a
new morning with each cycle). BN6's rain, snow and wind are left for
later.
