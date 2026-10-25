---
type: Overview
title: "Running the game's own logic"
description: "How BN6's own code runs from the player's ROM on an embedded mGBA core, the engine directing it: why an emulator, the memory it writes, hooks, layers, boot, threads, time, sound and testing."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-09T23:33:26Z }
---

# Running the game's own logic

Battles, the net, menus, shops and messages run on BN6's own code, executed
from the player's ROM by an embedded [mGBA](https://mgba.io) core (0.10.5,
MPL-2.0, built by `docker/mgba.sh`). Cyberworld Endless is the director
around it: it generates each layer in the game's own formats, patches the
map tables of its in-memory ROM copy to point at them, and follows the game
to move the run along: hooks on its code tell it what happens as it happens
([Hooks](hooks.md)), and it reads the game's memory between frames for the rest.
Nothing from the ROM is shipped.

| Layer | Owner |
| --- | --- |
| CPU, video, sound, input | mGBA core (`src/emu/emu.c`), presented through SDL |
| Battles, net movement, menus, messages, shops, traders | The game's code |
| Run structure, layer generation, loot, saves, the title | Cyberworld Endless |

The player's A+B+Select+Start returns to Cyberworld Endless's title
before the core sees it (`scene_emu.c`, issue #113): BN6's own reset would
start its title on the map tables the run has taken over. The director
stops without saving, so CONTINUE takes the run up at its last checkpoint,
as after a quit in a battle. The four buttons held together are no
controller's Select+Start quit either.

## Contents

- [Why an emulator](why-an-emulator.md)
- [Memory the engine writes](memory.md)
- [A layer](layer.md)
- [A new run and boot](boot.md)
- [The core on its own thread](thread.md)
- [The browser's memory](browser-memory.md)
- [The 3DS's memory](3ds-memory.md)
- [The core's time](time.md)
- [The core's sound](core-sound.md)
- [Fast forward](fast-forward.md)
- [The engine's own sound](engine-sound.md)
- [Hooks](hooks.md)
- [Testing](testing.md)
