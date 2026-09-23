# Running the game's own logic

Battles and the net run on BN6's own code, executed from the player's ROM by
an embedded [mGBA](https://mgba.io) core (MPL-2.0). The engine is the
director around it: it prepares game state in memory, feeds the game maps
generated in the game's own formats, and reads results back. Nothing from
the ROM is shipped; code, graphics and sound are all taken from the player's
copy at runtime, as before.

## Layers

| Layer | Owner |
| --- | --- |
| CPU, video, sound, input | mGBA core (`src/emu.*`), presented through SDL |
| Battles, net movement, menus, messages | The game's code |
| Run structure, map generation, rewards, saves | Cyberworld Endless |

## Phases

1. **Core.** Link a minimal static mGBA (no frontends, scripting or debugger)
   built from a pinned release in the build image; show the running ROM in
   the window with sound and the port's controls.
2. **Boot.** From power-on, reach a playable state without the story by
   calling the game's own new-game setup, then cache that state in the data
   directory (made on the device, never shipped).
3. **Battles.** Start encounters by writing battle settings, enemies, folder
   and HP (the forced encounter `tools/romlab` uses), run them on the game,
   and read the outcome, HP and reward back.
4. **Net.** Generate layers as the game's map data (tile sets from the
   areas, tile maps, collision, warps and objects), place them in free ROM
   space of the in-memory copy and point the map tables at them.
5. **Menus.** PET, folder, shops and traders through the game's own screens.

The pure-C engine stays in the repository until each phase replaces its
part.
