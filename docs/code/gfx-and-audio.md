---
type: Code Map
title: "Graphics, sound and the Jukebox"
description: "What `src/gfx/` (sprites, ROM tiles, fonts, BN6's PET look, the QR code, BN5's tiles), `src/audio/` (the engine's MP2K, the device, the sound filter) and `src/jukebox/` hold, file by file."
tags: [code]
generated: { by: claude-code/unrecorded, at: 2026-10-23T17:29:34Z }
---

# Graphics, sound and the Jukebox

| Path | Contents |
| --- | --- |
| `src/gfx/` | Sprite decoding and animation, ROM tiles, the font, a word set from the ROM's own letters (`letters.c`: the title's OPTIONS, the PET's Options), the engine's own letters for screens before the game's font is read (`pixfont.c`), BN6's PET screens' look for the screens the engine draws (`pet_look.c`: the band, rails, windows, information box, cursor, the choice's triangle, page arrows, hints and their letters, the lists' in BN6's chat font; [The PET's screens: Battle Network's rules](../engine/ui.md), [Fidelity](../game/fidelity.md)), the QR code (`qr.c`, as the site's `qr.js`), another game's 4bpp tiles drawn from its ROM's bytes (`xtiles.c`: BN5's Liberation pieces) |
| `src/audio/` | BN6's sound engine, MP2K, as the engine runs it where the core does not (the title and the screens around it; [The engine's own sound](../engine/emulation/engine-sound.md)): `mp2k.c` its start, songs and frame, `mp2k_seq.c` the sequencer, `mp2k_note.c` the notes, `mp2k_mix.c` the mixer, `mp2k_cgb.c` the PSG's driver, `gba_sound.c` the GBA's sound after it (mGBA's PSG, the FIFOs, the mix into blip_buf); `audio.c` the device, the songs by name, and the start's chime (`intro_chime.c`, a Pixabay sound `tools/intro_chime.py` converts); the core's sound during play; everything heard shaped by settings.ini's `sound` (`sound_filter.c`: gba, or soft's low-pass) |
| `src/jukebox/` | The Jukebox ([The PET](../game/pet.md#the-jukebox), The Jukebox): a key item's PET screen over the held game that plays every song of the games' music players on the engine's MP2K (`src/audio/mp2k.h`, BN5's through `mp2k_song_start_in`): `jukebox.c` its course (sections, modes, the next song, `jukebox.ini`), `jukebox_draw.c` its picture and where a tap lands, `jukebox_text.c` the songs by game and section, their names and the screen's words; opened from the PET's key item in `src/director/pet.c`, the core held under it by `pet_holds` |
