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
- **Height.** Dead-end rooms in Sky and Undernet layers can stand one level
  up, reached by a stair cut whole from the area's own map (tiles, ramp and
  side walls); the raised floor gets its heights and walls in the game's
  format, and whatever stands there is placed on it.
- **Layouts.** Layers follow the shape language of BN6's own net areas,
  measured from the ROM: eight layouts, several per area, varied within an
  act (docs/LEVEL_DESIGN.md).

## Next

- **Two materials.** The original areas draw their walkways in a second
  floor (Central's blue catwalks on green fields, Seaside's yellow
  boardwalks); the tile learner would need two styles and their joins.
- **More stairs.** Other areas' slopes are longer than two panels or not
  aligned to them (Seaside's rises 16 over five cells); taking those needs
  stairs of other lengths and a raised floor at their height.
