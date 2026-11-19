---
type: Code Map
title: "Discord's presence"
description: "What `src/discord/` holds: the setting and the connection's course, the pipe to the Discord app on each desktop, the frames and JSON of the wire, the words friends read, and the part that reads the game for them."
tags: [code, discord]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-18T16:00:00Z }
---

# Discord's presence

How it works, for players and for the code, is [Discord's
presence](../engine/discord.md); this is where each part lives. The
folder is two units of [the code's layers](layers.md): `discord`, low
enough for Options to switch it, and `discord.game`, above the scenes,
which reads the game.

| Path | Contents |
| --- | --- |
| `src/discord/discord.c` | The session (`discord.h`): `settings.ini`'s `discord` and `discord_app` lines read at the start (`discord_start`, from `src/core/start_net.c`), Options' switch kept (`discord_set_on`), the connection's course (away, hello, ready, refused) and its pace: the activity asked for (`discord_show`) sent when it differs, once in five seconds at most, Discord looked for every 20 seconds, its frames read and a ping answered (`discord_frame`), the activity cleared at the end (`discord_quit`). The application's id, the picture's and the button's addresses are named once in `discord.h` |
| `src/discord/discord_ipc.c` | The pipe to the Discord app (`discord_ipc.h`): Windows' named pipe, a Unix socket on Linux and macOS (the runtime and temporary folders, the Flatpak's, the Snap's and Vesktop's), opened on the first of ten that answers, read without waiting; the other builds' stand-ins (`DISCORD_IPC` says which have one) |
| `src/discord/discord_activity.c` | The wire, numbers and text only: a frame's header written and a whole frame taken from what came, the handshake's and `SET_ACTIVITY`'s JSON (escaped, a line under two bytes left out, NULL a clear), two activities compared |
| `src/discord/discord_text.c` | The words friends read (`discord_text.h`), composed from `DiscordFacts` (where, what, the layer, the area, the guardian, home, the best), and `settings.ini`'s note on the `discord` line; the voice catalog's `Discord` domain, a screen's words. Options' line has its words in `src/core/options_text.c` |
| `src/discord/discord_game.c` | The game read twice a second into those facts (`discord_game.h`): the scene (`game.h`'s `scene_current`), the run's layer and area (`run.h`, `guardians.h`), the director's battle, guardian, duel and home (`director.h`'s `director_in_battle`, `director_dueling`), the Battle Lab (`lab_battle.h`), the Net Square's visit (`online_visit.h`), the title's setup and summary (`title_setup.h`, `title_summary.h`'s `title_summary_shown`), the profile's best (`save.h`); its frame and the game's end handed to `main.c` through `game.h`'s `frame_watch` and `quit_watch`, so the entry point includes nothing of it |

Its tests are `tests/test_discord.c` in `test_core` ([The ROM-free
tests](tests.md)): the frames, the JSON and the words, without a Discord.
