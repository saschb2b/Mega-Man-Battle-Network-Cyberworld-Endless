# Roadmap

What remains between the current build and a run that plays like the
original from start to end. Each step ends with headless captures, an
autopilot run and, when the Nova is reachable, `tools/device_run.py`.

## Done

- **MegaMan stays in the run.** R no longer jacks out (flag `0x1727`), the
  PET's Comm and Save are off (`0x1706`), and MegaMan found on another map
  is warped back to the layer.
- **Progress the game understands.** A beaten Navi's Cross (`0xE2`-`0xE6`)
  and, in the Graveyard, Beast Out (`0xE0`) switch on through the story's
  own flags, and the game's chat box says so.
- **One source of truth.** `Run` keeps only the engine's decisions (format
  CWE2, older saves converted); MegaMan himself lives in the game's state.
- **Warps.** Exits are the game's own warp pads; the next layer is built
  while MegaMan jacks out. Side layers start the same departure.
- **Guardians on the net.** Every guardian Navi waits before the sealed exit
  pad and asks to fight (HeelNavi for those without an overworld sprite),
  and leaves once beaten.
- **Testing.** `CYBERWORLD_AUTOPILOT=weak` clears a whole cycle (depth 2 to
  19) headless; `tools/device_run.py` runs builds on the Nova from `/tmp`.

## Next

- **Height.** Ramps and raised platforms from the Z-modifier and
  layer-priority sections (coordinate data sections 1 and 2), with tiles
  learned from the source maps' own slopes. Both sections use the walls'
  key/shape lists. In Sky Area 2, section 1 raises whole cells (type 0x11)
  to z 32, 64 or 120 and slopes them with type 0x14 cells (value 1, height
  32 down to 20 in steps of 4); section 2 marks cells of each level (type
  0x11, height 8) for sprite priority. The generator would need room levels
  and ramps, and the tile classes a level per panel.
- **Area names.** Some show the game's padding as `___`; check against a
  recording of the original whether the name box should hide it.
