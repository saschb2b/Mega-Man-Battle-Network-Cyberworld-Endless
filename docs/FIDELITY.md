# Fidelity audit

Everything the engine shows or plays, and where it comes from. **Original**
means taken from the ROM and matched against recordings of the game (see
`tools/romlab`); **adapted** means the original has nothing to copy because
the roguelike needs something BN6 does not have; **engine-made** means it is
still an approximation that should be replaced by the original.

## Screens

| Element | Status | Notes |
| --- | --- | --- |
| Title screen | Original | Picture, logo glow, copyright line, PRESS START, NEW GAME / CONTINUE, dimming, sounds (`docs/ROM_DATA.md`) |
| Net floors | Original | Each biome's panels learned from one of the game's maps |
| Net backgrounds | Original | The areas' own BGAnim records, palette animation and scroll |
| Net HUD | Original | HP box and area name |
| Net movement and collision | Original | Screen-direction pad, 1 px a frame (2 running), 4-unit edge margin, clipping and sliding measured in Central Area 1 |
| Warp pads, Mystery Data, NPCs | Original sprites | Warp pads draw with the floor |
| Changing layers | Engine-made | A 30-frame white fade; the original's warp-out needs recording |
| Area map | Adapted | Reached from the PET (SubChip) and SELECT; BN6 has no area map, and it is drawn with rectangles |
| PET menu | Original graphics, adapted items | Library, MegaMan, E-Mail and Comm show run information and options |
| Messages | Original | Chat box, font and mugshots |
| Shops, traders, program vendor, reward cards, confirmations | Engine-made | Drawn with boxes and text; BN6's shop, Chip Trader and Number Trader screens are to be used |
| Encounter transition | Original | Mosaic and white fade, sound 0x78 |
| GAME OVER | Original | Followed by the run summary (adapted, drawn text) |

## Battle

| Element | Status | Notes |
| --- | --- | --- |
| Battle flow | Original | `docs/BATTLE_FLOW.md` |
| Buster, charge shot | Original | Pose, arm buster, muzzle flash, hit timing |
| Cannon, MiniBomb, swords | Original | Sprites, arcs and timing recorded |
| Other chips | Engine-made where noted in `src/data.c` | Each needs its recorded animation and timing |
| Mettaur | Original behaviour | Turn-taking and step delays from the disassembly (`ForMettaur_8109EF4`) |
| Other viruses | Engine-made | Behaviour approximated; the disassembly (`asm31.s`) has each AI |
| Navis | Engine-made | Attack patterns invented around their sprites |
| KillerEye beam, row and column blasts, fallback projectiles | Engine-made | Drawn as rectangles |
| Crosses and Beast Out | Engine-made | A text banner and a hit effect; the original transformation needs recording |
| Program Advance | Engine-made | A text banner |
| Cross choice in the Custom screen | Engine-made | Text under the window |
| Reward choice in the RESULT window | Adapted | Several rewards to choose from, shown in the original window |
| Wide screens | Adapted | Flat side borders beyond the 240-pixel field |

## Sounds

Recorded from the game (the effect played at that moment):
encounter 0x78, materialise 0x94, Custom open 0x79, chip cursor 0x7F, pick
0x81, OK 0x82, description 0x9C/0x9E, gauge full 0x8F, buster 0x6A, hit
0x6D, MegaMan hit 0x6B, charge 0x71/0x72, deletion 0x6F, reward 0x7E/0x95,
low HP 0x84, pause 0x9F, cannon 0xAE, sword 0xB0, throw 0xB2, bomb 0x70,
Mettaur pickaxe 0x183, shockwave 0xA6, title START 0x67, NEW GAME 0x9D,
CONTINUE 0x9C.

Named in the disassembly but not yet tied to a recorded moment: jack-in
0x77 (`SOUND_LOG_IN`), menu cursor 0x66, select 0x67, cancel 0x68.

Guessed, to be checked against recordings: error 0x69 (the disassembly calls
it `SOUND_CANT_JACK_IN`), guard 0xBA, recover 0xC7, wave 0x97, flame 0xE5,
thunder 0xF8, wind 0xFC, grab 0x10E, dash 0xE6, item 0x73, step 0x98 and a
virus warping 0x76 (`SOUND_LOG_OUT`).
