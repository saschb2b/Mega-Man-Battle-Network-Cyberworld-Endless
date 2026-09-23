# ROM data

All offsets are for Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1
`89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`, and live in `RomLayout` in
`src/rom.c`. The [bn6f disassembly](https://github.com/dism-exe/bn6f)
targets Cybeast Falzar; most code-region tables sit at the same addresses in
Gregar, and the rest were found by searching the Gregar ROM for the
structures the disassembly describes. `tools/romlab` runs the ROM in libmgba
to confirm findings against real frames and VRAM.

| Data | Offset | How it was found / verified |
| --- | --- | --- |
| Sprite pointer lists | `0x031CC4` | Ten pointers, the first pointing just past the list (`SpritePointersList`). Every category renders correctly. |
| Chip records (0x2C bytes) | `0x021DA8` | Same address as Falzar's `ChipDataArr`. Cannon = 40 power, codes A B C *. |
| Chip names | `0x6E88D0`, `0x6E92D8` | Text archives found by encoding "Cannon" with the game's character table; pointers in code reference both. |
| Chip descriptions (ids 0-255) | `0x6E983C` | Archive whose offset table ends where the first description begins. |
| Chip icon palette | `0x6E3880` | Matched against the Custom screen's OBJ palette in a VRAM capture. |
| Enemy names | `0x6EE400` | Archive starting "TestVirs", "Mettaur". |
| Navi names | `0x7F1D8C` | Archive starting "MegaMan", "HeatMan". |
| Enemy id table | `0x0182C4` | `GetVerActorTyAndAIIdx`: (version, actor type, AI index) triples. |
| Enemy stats | `0x00F260` | `enemy_getStruct2` tables: HP and element in one halfword, attack after. |
| Battle panel tiles | `0x6DBB24` (LZ77) | Traced from BG2 tiles in a battle VRAM capture. |
| Panel palettes | `0x6DE45C`-`0x6DE95C` | Matched from the same capture. |
| Panel tile layouts | `src/panel_layout.inc` | Recorded by writing each panel type into `ePanelData` in a running battle and reading back the tilemap. |
| Battle font | `0x6B5A2C` | 8x16 glyphs indexed by character code; traced from the enemy-name layer. |
| Battle backgrounds | `0x082058` (records), `0x0822E0` (animations) | One pointer per background id 0x00-0x15 (the BattleSettings background byte). A record is the disassembly's `BGAnimData`: tiles (word count, then an offset to LZ77 data), a 32x32 tilemap (LZ77 at +12), a palette (after a 4-byte size). The animation list holds `GFXAnimData` scripts: tile copies (source tiles, VRAM destination, tile count, then index lists with delays) and palette copies. Found by setting the background byte of a forced battle to every value and matching VRAM and palette RAM; scroll speeds were measured from BG1's offset registers. |
| Net area backgrounds | `RomLayout.net_area[].bg`, `.bg_anims` | Each area's `LoadBGAnim` passes a `BGAnimData` record in the battle format, loaded into BG3 with tiles at 0x06008000; its `LoadGFXAnims` list animates the palette alongside the floor (floor entries point elsewhere in VRAM and are skipped). The records were located in the Falzar disassembly's area loaders and matched in Gregar by their fixed fields (0x06008020, 0x1800, 0x03001960, 0x20); each list sits at the same distance from its record and has the same entries. Scroll comes from the loaders' callbacks: `BGScrollCB_BG3Diagonal3to2Scroll` (right 1/2, down 1/4 a frame), `BGScrollCB_BG3SlowRightScroll` (offset +1/16, the Graveyard) or none. |
| MP2K song table | `0x159F48` | Longest run of valid song headers (474 entries). Song and effect ids follow the disassembly's `SoundOffsets.inc`. |
| MP2K player table | `0x159DC8` | 32 entries of (`MusicPlayerInfo`, tracks, count) ending at the song table; player 31 is music. Used to log sounds in recordings. |
| Battle-flow UI | `RomLayout.ui` | Traced from VRAM/OAM captures of a recorded battle with `tools/romlab/labtrace.py`; every offset and how it appears is listed in [BATTLE_FLOW.md](BATTLE_FLOW.md). |
| HP box damage palette | `0x6DFC3C` | BG palette of the HP box while MegaMan's HP rolls down. |
| Warning panel palette | `0x6DE5BC` | The game replaces a threatened panel's tiles with one solid tile in its colour 14. |
| Custom cursor, second frame | `0x6E3560` | OAM of the pulsing cursor, 8 frames each with `0x6E3540`. |
| OK panel art | `0x7204F0`, palette `0x723730` | 7x6 tiles in the art slot while the cursor is on OK. |
| PRESS A BUTTON | `0x730AF0` | 10 tiles on the RESULT window, result palette. |
| Zenny reward art | `0x730D90`, palette `0x7312D0` | 7x6 tiles in the RESULT window's art slot. |
| Text box tiles | `0x6BCB4C` corner, `0x6BCAAC` edge, `0x6BCB6C` side, `0x6BCBAC` side by the arrow, palette `0x6BCBCC` | The chip description box; the arrow is `0x6A270C` in the chat palette. |
| Enemy name tab | `0x6E4020` | End cap then fill, two tiles each, HP palette. |
| PAUSE | `0x6E40A0` | 4x2 tiles then a 1x2 column, enemy HP palette. |
| Charge glow palette | `0x3AB1B0` | The charge lines (GUI sprite 162) once charged. |
| GAME OVER | LZ77 `0x6C211C`, palette `0x6C25AC` | Both BG layers traced from a capture of the screen into `hud_layout.inc`. |
| Deletion explosion | GUI sprite 155 | Matched to OAM tiles of a deleted Mettaur and rendered with `--sheet`. |
