---
type: Code Map
title: "The game's own updates"
description: "What `src/update/` holds: the look at the release's update.json and the download's thread, the manifest's reading, SHA-256, each system's way to put a new version in place, the Switch's and the 3DS's swap of their own file, and the updater's words."
tags: [code, updates]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T05:00:00Z }
---

# The game's own updates

How it works, for players and for the code, is [The game's own
updates](../engine/updates.md); this is where each part lives.

| Path | Contents |
| --- | --- |
| `src/update/` | The updater (`update.h`): `update.c` the state the title and Options read, the switch (`settings.ini`'s `updates`), the look at the start and a press's download, each on a thread of its own through the statistics' HTTP client (`src/analytics/analytics_http.c`'s GET, [The anonymous statistics](analytics.md)), and the exit handler that starts the new version; `update_manifest.c` update.json read and versions compared; `update_sha256.c` SHA-256 fed as a download comes in; `update_install.c` each system's way (the AppImage over itself, the Windows installer silently, Android's installer, the Switch's `.nro` and the 3DS's `.3dsx`, the itch app's, the download page, the releases page, the PS5's store) and where its download is written and placed; `update_swap.c` the Switch's and the 3DS's file put in its own place as the game ends (the path from the loader's `argv[0]`, the new one beside it, set aside, renamed, put back on a failure; plain C, which the unit tests run on the host); `update_android.c` Android's look and download, done by the app's Java (below) and polled from the game's frames; `update_text.c` its words (Options' lines and notes, the title's note, settings.ini's comment); `update_pictures.c` the notes' pictures fetched (their own thread, checked whole) and one unpacked for the box |
| `src/core/options_news.c` | The update box: the release's notes from update.json's `changes`, laid out once into pages (a cover, then an entry each) and drawn in BN6's list window with the PET's pieces, an entry's picture at half scale under its heading; Left and Right turn, Up and Down scroll, SELECT a picture whole; **Update now** and **Later** as SAVES' buttons with their keys; the title's offer once a version ([The core](core.md)) |
| `src/scenes/title_version.c`, `title_menu.c` | The title's line over the copyright: the build, `New vX.Y.Z` beside it once one is found until the menu shows, the best depth; the key hints' `SELECT: Update` on top of SAVES' once it does; and the updater's tick and the box's offer on the title's frames ([The engine's screens](screens.md)) |
| `android/.../Updater.java`, `UpdateReceiver.java` | Android's update: update.json and the APK fetched with `HttpURLConnection`, the APK checked against its SHA-256 and committed to a `PackageInstaller` session (without a tap from Android 12 on), Android's prompt shown where it asks; the receiver that opens the new version after `MY_PACKAGE_REPLACED` (up to Android 9: later ones refuse it) ([The platforms and their packaging](platforms.md), `android/`) |
| `port/update.py`, `port/Cyberworld Endless.sh` | The PortMaster port's update put in place by its launcher once the game ends, and an update cut short undone ([The platforms and their packaging](platforms.md), `port/`) |
| `tools/update_manifest.py` | A release's update.json from its files and its notes (the players' notes, else the CHANGELOG section, as the box's paragraphs), and `update-pictures.bin` from the notes' pictures (palette, LZ77), written by the release workflow ([The platforms and their packaging](platforms.md), `.github/`) |
| `src/analytics/analytics_roots.c` | The roots the consoles' requests carry: Let's Encrypt's, and for the updater's GETs GitHub's with them ([The anonymous statistics](analytics.md)) |
| `tests/test_update.c`, `tests/test_port_update.py`, `tests/test_update_manifest.py` | The manifest as the tool writes it, its changes' escapes, versions, SHA-256's known answers, the Switch's and the 3DS's swap on a made-up SD card; the port's update on made-up ports folders; the tool's notes from made-up notes and a made-up CHANGELOG, its pictures' scale, colours and LZ77 against the game's decoder ([The tests](tests.md)) |

The unit stands over `analytics` in [the code's layers](layers.md): it
uses the statistics' HTTP client, and `core.options` and `scenes` use it.
