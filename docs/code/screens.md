---
type: Code Map
title: "The engine's screens: the ROMs screen, SAVES, the start and title, the second screen"
description: "What `src/launcher/` (the ROMs screen and pickers), `src/saves/` (SAVES), `src/scenes/` (the start, title and error screen) and `src/second/` (the 3DS's and Android's second screen) hold, file by file."
tags: [code]
generated: { by: claude-code/unrecorded, at: 2026-10-23T18:02:58Z }
---

# The engine's screens: the ROMs screen, SAVES, the start and title, the second screen

| Path | Contents |
| --- | --- |
| `src/launcher/` | The ROMs screen before the game on the desktops, Android and iOS (issue #97, [The player's guide](../GUIDE.md#the-roms-screen)'s The ROMs screen): `launcher.c` its course, `launcher_draw.c` its layout and picture, drawn on the game's own screen in the style of the second screen's PET panels (`second_frame.c`; a second display stays dark meanwhile), `launcher_art.c` the cartridges and open spots (the project's own drawing; a label's face drawn from the player's ROM as it opens), `launcher_roms.c` what a chosen file is and the last start's record (`launcher.ini`), its words in `launcher_text.c`; the system's pickers behind `pick.h` (`pick_desktop.c`, `pick_android.c`, `pick_ios.c`, `pick_none.c`; what they share for SAVES in `pick_saves.c`: `saves.ini`, the transfer place where it is a plain folder; their words in `pick_text.c`), and the automatic copy to the transfer folder, a phone's ROM folder or any chosen one (`mirror.c`) |
| `src/saves/` | The SAVES screen (issue #114, [Portable saves](../engine/saves.md)), over the title or the ROMs screen: `saves.c` its course, `saves_actions.c` its buttons' work, `saves_draw.c` its picture, `saves_layout.c` where its buttons are, `saves_text.c` its words; the state its parts share in `saves_state.h` |
| `src/scenes/` | The start (`scene_intro.c`: the developer's boot screen, `intro_logo.c`, and MegaMan's word on GitHub, before the title at a plain start), the title (`scene_title.c`; its menu `title_menu.c`, the run summary `title_summary.c`, its PET text `title_summary_text.c` and MegaMan's words `title_lines.c`), the screen a start without its ROM shows where there is no ROMs screen (`scene_error.c`: PortMaster, the 3DS) and the sprite gallery |
| `src/second/` | The second screen (issue #72): the 3DS's bottom screen, Android's second display. It follows BN6's screen as the DS games' PET followed theirs: `second.c` reads which screen the player is on once a frame (BN6's modes, [ROM data](../rom/overview.md)) into a state the draw reads (on the 3DS the draw runs while the core's thread runs the next frame), `second_read.c` reads each panel's facts, `second_frame.c` draws the PET's frame (in `tools/site_art.py`'s colours, measured from BN6's PET), each screen's panel has a file of its own (the Square's emote palette, a tap an emote or phrase, `second_palette.c`), and `second_text.c` holds the panels' words |
