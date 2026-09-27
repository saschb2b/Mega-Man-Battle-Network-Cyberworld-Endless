# Fidelity

What the player sees, and whose it is. **Original** is the game's own code
and data running from the ROM; **generated** is built by the engine in the
game's formats; **adapted** is the engine's own, where BN6 has nothing to
use.

| Element | Status | Notes |
| --- | --- | --- |
| Title screen | Original logo and backgrounds, generated mark and subtitle | The logo (emblem, MEGAMAN, BATTLE NETWORK) cut from the game's title picture, copyright, PRESS START, NEW GAME / CONTINUE, cursor and sounds are the game's (`docs/ROM_DATA.md`). The infinity mark in the 6's place and the CYBERWORLD ENDLESS subtitle are drawn by the engine in the 6's and the plate's palette colours; behind them the battle backgrounds of the run's areas take turns, with the game's animations and scroll. The best depth and the saved run's depth are drawn text |
| Start gift | Original items, engine script | A Mr. Prog on the first layer offers two HPMemory, a chip or a NaviCust program through the game's own give commands and option menu |
| Run summary | Adapted, original face | Drawn text and Lan's face (the game's mugshot) over the darkened battle background of the area where MegaMan was deleted, after the game's GAME OVER: where and by whom, the layer reached, the counts and the best |
| Story | Adapted | The Endless Net, a premise written for the run: a net that opened under the town, copied from the real one and rebuilt stronger each time its Nest falls; Dad's call, Lan and MegaMan's words on arriving, the guardians as copies built from MegaMan's battles, Chaud's call. Told in the game's chat box with its mugshots |
| The town | Original pieces, arranged per run | Central Town's or ACDC Town's own map cut into pieces and set out again (docs/OVERWORLD.md): their buildings, trees, statues and Chip Trader copied whole, the ground between them stretched with their own tiles. The townsfolk are the game's generic NPC sprites, and they and the towns' checks say lines written by the engine; the music is the town's own |
| Jacking in | Original | The game's jack-in (Lan's line, the transmission, MegaMan's arrival) to one of its jack-in destinations, pointed at the first layer |
| Net movement, collision, camera, HUD | Original, with help at edges | The game's overworld code on generated maps. The engine steers the pad a little where the generated floors are narrow: a single direction held at a walkway's mouth lines MegaMan up with it and takes him in, a diagonal held into a corner slides him round it, MegaMan pushed into a navi for a second steps free, and A turns him to a navi or Mystery Data he stands at before the game's own check, or walks him up to one a step or two before him and presses A for him; with two side by side, the others near have no ring while the press goes through, so it reaches the one he means |
| Net floors | Generated | Tiles learned from one original map per area, platforms and walkways in its two floors, each checked against where the floor and its side faces are drawn and, inside the floor, against the area's usual panels (not at Robot Control's platform edges, whose two-band rims reach deeper; its pads look like its raised white platforms); a few inner corners the original never shows still take the nearest shape. The area's other maps fill in places its own never shows, but not with pieces in colours its own map's floors never show (Central Area 2's raised yellow plateau at Central's inner corners), except Seaside's yellow panels, the only fields of its second floor. What the originals set in front of their floors on the second tile layer (bridges, ornaments, spikes, overlapping corners) is left out. Where an original never joins two floors flush, each piece is drawn whole on its own and laid over the rest on the game's two tile layers: the Aquarium's glass pools, which its maps raise on legs above the water (a channel ends at a pool's rim, and the guardian's arena is a pool too), Green's grass and CopyBot's plateaus and red pads, which their walkways reach by stairs and ladders |
| Net walls | Generated | Wall cells in the game's coordinate-data format, shapes as the original maps use them |
| Raised rooms | Original stairs, generated floor | A stair's tiles, ramp, and side walls cut whole from Sky Area 2 or Undernet 1; the raised floor's heights and walls in the same format |
| NPCs, Mystery Data, exit pads | Original sprites and scripts | Placed by the engine in the game's NPC bytecode |
| Dialogue | Original text engine and faces | Lines written by the engine in the game's text script language, each with the speaker's own mugshot (Lan, MegaMan, Dad, Chaud, Mr. Prog, the townsfolk, the bystander and shop navis, the guardians; a HeelNavi's for the navis Gregar has no face of) |
| Shops, Chip Trader, BugFrag Trader, healing | Original | The game's screens and commands; stock chosen by the engine. The Net Dealer is the game's Normal Navi, the NaviCust vendor its technician navi (shop 3, whose screen shows that face). Chip Traders are the game's machine and dialogue, with the original prize pools picked by depth; deeper layers can hold a Chip Trader Special |
| Battles, rewards, Busting Level | Original | The area's original random battles (viruses, their places, rocks and cubes, the battlefield's panels), each at the highest version that keeps it inside its act's HP and damage limits (docs/PROGRESSION.md); challenges from act 4 can meet the area's SP navi |
| Boss navis | Original sprites and battles | HeatMan, ElecMan, SlashMan, EraseMan, ChargeMan, ProtoMan, DiveMan, CircusMan, JudgeMan and Colonel wait before the exit and ask to fight; Navis Gregar has no overworld sprite for appear as a HeelNavi. Chosen by HP for their act; each leaves five HPMemory and its own Navi chip (the game's items) and heals MegaMan |
| Choices (challenge, Undernet, Secret Area) | Original text and flags | Yes sets an event flag the engine acts on |
| Layer changes | Original warp pads | The game's jack-out and jack-in, with the next layer built while MegaMan jacks out |
| Screen | Adapted | 240x160 at a whole-number scale with black borders |

## Known gaps

- The town is Central Town in two widths or ACDC Town in two orders; its
  doors lead nowhere (a check says why), and most of its people stand
  still.

- The game's map-name label at the bottom right shows where the run is
  ("Layer 12", "Undernet", "ACDC Town") in the game's own font and box.
- Only Sky and Undernet layers raise rooms: their maps are the only ones
  with a stair two panels square. Raised rooms are one level high and
  never overlap other floor on screen, so layer priorities (section 2) are
  not generated.
- Talking to a pad (the Secret Area gate) needs a press of A beside it;
  pads cannot be stepped on.
