# Fidelity

What the player sees, and whose it is. **Original** is the game's own code
and data running from the ROM; **generated** is built by the engine in the
game's formats; **adapted** is the engine's own, where BN6 has nothing to
use.

| Element | Status | Notes |
| --- | --- | --- |
| Title screen | Original data, engine-drawn | Picture, logo glow, copyright, PRESS START, NEW GAME / CONTINUE, timings and sounds recorded from the game (`docs/ROM_DATA.md`) |
| Run summary | Adapted | Drawn text over the dimmed title picture after the game's GAME OVER |
| Net movement, collision, camera, HUD | Original | The game's overworld code on generated maps |
| Net floors | Generated | Tiles learned from one original map per area; rare edge shapes may take tiles from décor |
| Net walls | Generated | Wall cells in the game's coordinate-data format, shapes as the original maps use them |
| NPCs, Mystery Data, exit pads | Original sprites and scripts | Placed by the engine in the game's NPC bytecode |
| Dialogue | Original text engine | Lines written by the engine in the game's text script language |
| Shops, Chip Trader, BugFrag Trader, healing | Original | The game's screens and commands; stock chosen by the engine |
| Battles, rewards, Busting Level | Original | Encounters chosen by the engine (enemies, background, music) |
| Boss navis | Original sprites and battles | HeatMan, ElecMan, SlashMan, EraseMan, ChargeMan, ProtoMan, DiveMan and JudgeMan wait before the exit and ask to fight; Navis Gregar has no overworld sprite for appear as a HeelNavi |
| Choices (challenge, Undernet, Secret Area) | Original text and flags | Yes sets an event flag the engine acts on |
| Layer changes | Original warp pads | The game's jack-out and jack-in, with the next layer built while MegaMan jacks out |
| Screen | Adapted | 240x160 at a whole-number scale with black borders |

## Known gaps

- Area names show the game's padding as `___` before some names.
- Generated layers do not use the game's layer-priority or Z-modifier
  sections, so there are no ramps or raised platforms.
- Talking to a pad (the Secret Area gate) needs a press of A beside it;
  pads cannot be stepped on.
