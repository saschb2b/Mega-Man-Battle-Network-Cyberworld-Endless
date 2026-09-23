# Changelog

## Unreleased

First playable version.

- Engine in C11 and SDL2 that reads sprites, chip art and descriptions, the
  battle font, panels, enemy data and music from the player's BN6 Cybeast
  Gregar (USA) ROM.
- Grid battles with the Custom screen, 60 battle chips, 16 navi chips, nine virus families in six
  versions and 13 navi bosses, Program Advances, five Crosses and Beast Out.
- Generated net layers across eight areas, with Mystery Data, shops,
  traders, NaviCust programs, virus-infested rooms, the Undernet and the
  Secret Area.
- MP2K sound engine playing the game's music and effects.
- Battles follow the original frame by frame, measured from recordings of the
  game: the encounter mosaic and white flash, enemies materialising one by
  one, the Custom window's slide, chip descriptions, OK's transmission panel,
  the BATTLE START and ENEMY DELETED banners every turn, the gauge's speed and
  full animation, rolling HP counters, warning panels, the buster charge,
  deletion explosions, the RESULT window and reward reveal, the pause, MegaMan's
  deletion, the GAME OVER screen, and the sounds for each.
- Cannon, MiniBomb, swords and the charged buster use the original sprites,
  arcs and timings.
- Each area fights on one of the game's own animated battle backgrounds.
- The net looks and moves like the original: floors built from each area's
  own 64x32 panels, learned from the ROM's maps at load, the area's animated
  background, MegaMan walking a pixel a frame along the screen (two running),
  and the HP box and area name as the only HUD.
- Checkpoint saves at every layer and a profile with best depth.
- PortMaster launcher for ROCKNIX; tested on the Retroid Nova and the
  Retroid Pocket Flip 2.
