# Fidelity

What the player sees, and whose it is. **Original** is the game's own code
and data running from the ROM; **generated** is built by the engine in the
game's formats; **adapted** is the engine's own, where BN6 has nothing to
use.

| Element | Status | Notes |
| --- | --- | --- |
| Title screen | Original logo and backgrounds, generated mark and subtitle | The logo (emblem, MEGAMAN, BATTLE NETWORK) cut from the game's title picture, copyright, PRESS START, NEW GAME / CONTINUE, cursor and sounds are the game's (`docs/ROM_DATA.md`). The infinity mark in the 6's place and the CYBERWORLD ENDLESS subtitle are drawn by the engine in the 6's and the plate's palette colours; behind them the battle backgrounds of the run's areas take turns, with the game's animations and scroll. The best depth and the saved run's depth are drawn text |
| Start gift | Original items, engine script | A Mr. Prog on the first layer offers two HPMemory, a chip or a NaviCust program through the game's own give commands and option menu |
| Run summary | Adapted | Drawn text over the darkened battle background of the area where MegaMan was deleted, after the game's GAME OVER |
| Net movement, collision, camera, HUD | Original | The game's overworld code on generated maps |
| Net floors | Generated | Tiles learned from one original map per area, platforms and walkways in its two floors, each checked against where the floor and its side faces are drawn and, inside the floor, against the area's usual panels; a few inner corners the original never shows still take the nearest shape |
| Net walls | Generated | Wall cells in the game's coordinate-data format, shapes as the original maps use them |
| Raised rooms | Original stairs, generated floor | A stair's tiles, ramp, and side walls cut whole from Sky Area 2 or Undernet 1; the raised floor's heights and walls in the same format |
| NPCs, Mystery Data, exit pads | Original sprites and scripts | Placed by the engine in the game's NPC bytecode |
| Dialogue | Original text engine | Lines written by the engine in the game's text script language |
| Shops, Chip Trader, BugFrag Trader, healing | Original | The game's screens and commands; stock chosen by the engine. Chip Traders are the game's machine and dialogue, with the original prize pools picked by depth; deeper layers can hold a Chip Trader Special |
| Battles, rewards, Busting Level | Original | The area's original random battles (viruses, their places, rocks and cubes, the battlefield's panels), each at the highest version that keeps it inside its act's HP and damage limits (docs/PROGRESSION.md); challenges from act 4 can meet the area's SP navi |
| Boss navis | Original sprites and battles | HeatMan, ElecMan, SlashMan, EraseMan, ChargeMan, ProtoMan, DiveMan, CircusMan, JudgeMan and Colonel wait before the exit and ask to fight; Navis Gregar has no overworld sprite for appear as a HeelNavi. Chosen by HP for their act; each leaves five HPMemory and its own Navi chip (the game's items) and heals MegaMan |
| Choices (challenge, Undernet, Secret Area) | Original text and flags | Yes sets an event flag the engine acts on |
| Layer changes | Original warp pads | The game's jack-out and jack-in, with the next layer built while MegaMan jacks out |
| Screen | Adapted | 240x160 at a whole-number scale with black borders |

## Known gaps

- Area names show the padding the ROM stores before shorter names (`0xB2`,
  drawn as `_`); the game's own code draws them.
- Only Sky and Undernet layers raise rooms: their maps are the only ones
  with a stair two panels square. Raised rooms are one level high and
  never overlap other floor on screen, so layer priorities (section 2) are
  not generated.
- Talking to a pad (the Secret Area gate) needs a press of A beside it;
  pads cannot be stepped on.
