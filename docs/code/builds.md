---
type: Engine Mechanism
title: "The builds: desktop and browser"
description: "How the desktop builds differ (CW_DESKTOP: a window, saves and the ROM in the user's data folder) and the browser build's limits (no threads, one frame per 1/60 s, IndexedDB through `platform_persist()`, BN5 read as data)."
tags: [code]
generated: { by: claude-code/unrecorded, at: 2026-10-12T21:51:29Z }
---

# The builds: desktop and browser

The host, Linux, Windows and macOS builds are desktop builds
(`CW_DESKTOP`): a resizable window, saves in
`~/.local/share/cyberworld-endless` (Windows:
`%LOCALAPPDATA%\cyberworld-endless`, macOS: `~/Library/Application
Support/cyberworld-endless`), the ROM looked for there, beside the binary
and in `./rom`. The ROCKNIX build fills the screen
and takes both folders from its launcher. `build.py run` is the quickest way
to play a change; the headless `shot` stays the way to capture one.

The browser build (`__EMSCRIPTEN__`) has no threads: the page drives one
game frame per 1/60 s (`emscripten_set_main_loop`), and writes reach
IndexedDB through `platform_persist()`, which saves and states call. Keep
the frame loop free of blocking waits. Every battle is BN6's core's on
every build, BN5's areas', Navis' and chips' too ([One net: the design](../multirom/one-net.md), One
net): BN5's ROM is read as data, and its own core (`src/emu/guest.c`) is
the differential's reference alone, never started in a run. `build.py serve` also serves the
developer's ROMs, told by their SHA-1, for tests in a local browser:
BN6's at `/.dev/rom.gba`, BN5's at `/.dev/bn5.gba`; the page itself only
takes the ROMs the player chooses.
