/* The song a won battle plays (docs/multirom/music.md): BN6's win picks it
 * (BN6_WIN_SONG), and a hook answers it with another game's where the
 * layer's area is that game's, or BN5's One-Turn Win in a liberation. */
#ifndef CW_WIN_SONG_H
#define CW_WIN_SONG_H

/* The layer's area's game (XRomId; -1 BN6's): its battles' win songs, where
 * its layout names them, copied in and played in place of BN6's of the same
 * number. Set as each battle's record is written (encounter.c). */
void win_song_area(int xrom);
/* BN6's slot of a song a battle won in its first turn plays over the rest,
 * -1 for none (liberation_cues.c, between battles). */
void win_song_one_turn(int slot);

#endif
