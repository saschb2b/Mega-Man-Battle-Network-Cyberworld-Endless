---
type: Code Map
title: "The game's own updates"
description: "What `src/update/` holds: the look at the release's update.json and the download's thread, the manifest's reading, SHA-256, each system's way to put a new version in place, and the updater's words."
tags: [code, updates]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T03:30:00Z }
---

# The game's own updates

How it works, for players and for the code, is [The game's own
updates](../engine/updates.md); this is where each part lives.

| Path | Contents |
| --- | --- |
| `src/update/` | The updater (`update.h`): `update.c` the state the title and Options read, the switch (`settings.ini`'s `updates`), the look at the start and a press's download, each on a thread of its own through the statistics' HTTP client (`src/analytics/analytics_http.c`'s GET, [The anonymous statistics](analytics.md)), and the exit handler that starts the new version; `update_manifest.c` update.json read and versions compared; `update_sha256.c` SHA-256 fed as a download comes in; `update_install.c` each system's way (the AppImage over itself, the Windows installer silently, the itch app's, the download page, the releases page) and where its download is written and placed; `update_text.c` its words (Options' lines and notes, the title's note, settings.ini's comment) |
| `src/scenes/title_version.c` | The title's line over the copyright: the build, `New vX.Y.Z` beside it once one is found, the best depth; and the updater's tick on the title's frames ([The engine's screens](screens.md)) |
| `port/update.py`, `port/Cyberworld Endless.sh` | The PortMaster port's update put in place by its launcher once the game ends, and an update cut short undone ([The platforms and their packaging](platforms.md), `port/`) |
| `tools/update_manifest.py` | A release's update.json from its files, written by the release workflow ([The platforms and their packaging](platforms.md), `.github/`) |
| `tests/test_update.c`, `tests/test_port_update.py` | The manifest as the tool writes it, versions, SHA-256's known answers; the port's update on made-up ports folders ([The tests](tests.md)) |

The unit stands over `analytics` in [the code's layers](layers.md): it
uses the statistics' HTTP client, and `core.options` and `scenes` use it.
