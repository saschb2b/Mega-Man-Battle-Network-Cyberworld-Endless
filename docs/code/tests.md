---
type: Code Map
title: "The tests"
description: "What the ROM-free unit tests cover (`tests/test_core.c`, `test_backup.c`, `test_saves.c`, the transfer, discovery and map-slot tests, the Android saves and Steam scripts, the voice catalog) and the lint baselines."
tags: [code]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-10T21:45:00Z }
---

# The tests

| Path | Contents |
| --- | --- |
| `tests/test_core.c` | ROM-free unit tests (with `tests/test_backup.c`, the `.cwsave` carrier and its install); `tests/test_saves.c` the SAVES screen, `tests/test_transfer.c` and `tests/test_discovery.c` the automatic copy and its folders, with made-up progress; `tests/test_mapslot.c` the maps' Mystery Data lists emptied as layers are installed, in a made-up ROM's tables; `tests/test_android_saves.py` `RomLook`'s saves against document-provider doubles on a JVM; `tests/test_emu.c` the hooks on mGBA, and two cores side by side as BN6's and the differential's reference run (each one's hooks, RAM, sound and state its own), with ROMs of its own bytes, and with `tests/test_emu_pack.c` the chip pack's routines run there as BN6's code calls them (an entry for every id, a chip given and its order stamp in BN6's pack, the extension and the cart's save RAM, the clear with and without its hook, the entries through a state, a state from before them and a raw snapshot); `tests/test_add_to_steam.py` runs `linux/steam/add-to-steam.py` against a made-up Steam folder; `tests/test_voice_catalog.py` the voice catalog's reading of C on a words file of its own; `tests/lint/` the baselines `build.py lint` checks against |
| `tests/layer_make_hash.txt` | The layers' make and `layer_generate`'s hash over the unit tests' seeds as `dev` last recorded them (`test_layer_make` compares; `tools/layer_make.py` writes it on `dev`, never a branch) |
