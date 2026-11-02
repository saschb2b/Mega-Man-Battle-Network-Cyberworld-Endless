# Running BN6

* [The GBA's screen: colour correction, frame blending, lcd3x](lcd.md) - Options' Display can show the game as a GBA's own screen did (issue #265): mGBA's libretro core's Color Correction and Interframe Blending on BN6's picture, step for step (`lcd_frame.c`), the correction on what the engine draws over it too (`lcd_canvas.c`), and RetroArch's lcd3x shader over the screen (`lcd_grid.c`); what each does, where it runs, and how it was checked against mGBA's own code.
* [Portable saves](saves.md) - Portable saves (issue #114): the SAVES screen's three views (`src/saves/`), the `.cwsave` carrier and manifest (`src/core/backup*.c`), what travels, import checks, UNDO IMPORT and recovery, and the automatic copy (`mirror.c`).
* [The PET's screens: Battle Network's rules](ui.md) - BN6's PET screens' rules measured from Gregar (band, windows, information box, cursor, motion, keys, letters, colours) and what the engine's own screens (the Jukebox, Options, Trade Central) take from them through `pet_look.c`.

# Subdirectories

* [Running the game's own logic](emulation/) - How BN6's own code runs from the player's ROM on an embedded mGBA core, the engine directing it: why an emulator, the memory it writes, hooks, layers, boot, threads, time, sound and testing.
