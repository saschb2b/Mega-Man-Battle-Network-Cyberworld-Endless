# Running BN6

* [The GBA's screen: colour correction, frame blending, lcd3x](lcd.md) - Options' Display can show the game as a GBA's own screen did (issue #265): mGBA's libretro core's Color Correction and Interframe Blending on BN6's picture, step for step (`lcd_frame.c`), libretro's handheld colour shaders (VBA, DS, PSP) over that picture as RetroArch runs them, and RetroArch's lcd3x shader over the screen (`lcd_grid.c`); what each does, where it runs, and how it was checked against mGBA's own code and the shaders' math.
* [Portable saves](saves.md) - Portable saves (issue #114): the SAVES screen's three views (`src/saves/`), the `.cwsave` carrier and manifest (`src/core/backup*.c`), what travels, import checks, UNDO IMPORT and recovery, and the automatic copy (`mirror.c`).
* [Settings screens elsewhere: what Options can take](settings-screens.md) - How well-regarded games, the accessibility guidelines, emulator frontends and retro collections show a setting's effect while it is chosen (live preview, a still frame behind the menu, a picture per setting, a one-press look), describe, confirm, reset and lay out their settings, researched on 2 November 2026 for Options' Display, what of it fits a PET panel at 240x160, and what Options was rebuilt with: pages on L and R, START's defaults, and BN6's own battle looping beside Display's lines.
* [The PET's screens: Battle Network's rules](ui.md) - BN6's PET screens' rules measured from Gregar (band, windows, information box, cursor, motion, keys, letters, colours) and what the engine's own screens (the Jukebox, Options and its pages and scene, Trade Central) take from them through `pet_look.c`.

# Subdirectories

* [Running the game's own logic](emulation/) - How BN6's own code runs from the player's ROM on an embedded mGBA core, the engine directing it: why an emulator, the memory it writes, hooks, layers, boot, threads, time, sound and testing.
