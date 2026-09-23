# Roadmap

What remains between the current build and a run that plays like the
original from start to end. Each step ends with headless captures, an
autopilot run and, when the Nova is reachable, a test from `/tmp` there.

## 1. Keep MegaMan in the run

- **Jack-out.** Find how the net offers jacking out (R) and switch it off on
  layers the way the story does, or patch the check. Test: R on a layer does
  nothing.
- **PET Save.** The game's save writes only to the core's in-memory flash.
  Hide or disable the PET's Save, or make it run the engine's checkpoint.
- **Leaving the layer any other way** (story warps, Lan's room): the
  director notices MegaMan off the layer and warps him back.

## 2. Progress the game understands

- Find the event flags and state behind Crosses, Beast Out and the Link
  PET/NaviCust features, and set them as the run goes deeper: a Navi's
  Cross after its boss battle, Beast Out after the Graveyard's guardian.
- Check which chapter-gated content (shops, map music, Mystery Data kinds)
  depends on GameState+7, and pick the chapter deliberately.

## 3. One source of truth

- The game's RAM holds HP, folder, pack, zenny, BugFrags and key items. Trim
  `Run` to what the engine decides (seed, depth, side layer, biome order,
  bosses, counters) and read the rest from the game when the summary needs
  it. Bump `RUN_MAGIC`.
- Remove the generator's leftovers: the boss object, infested rooms and
  loot settings no longer used.

## 4. Maps closer to the original

- **Warps.** Exits as the game's own warp triggers (coordinate data section 3
  or map objects), so the game plays its warp-out; the director only reads
  where MegaMan arrived.
- **Boss Navis on the net.** Their overworld sprites standing at the exit;
  talking starts the battle.
- **Height.** Ramps and raised platforms using the Z-modifier and
  layer-priority sections, learned from the source maps.
