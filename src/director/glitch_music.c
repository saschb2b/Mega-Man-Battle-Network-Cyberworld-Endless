/* A Glitch layer's music in step with its flips (glitch.h;
 * docs/game/glitch-layer.md, Its sound): BN6's own music player's tempo
 * dragged while a look is on the floor and stalled at a flip's edge from
 * stage 3, the sequence stopped while its notes sound on, then the tempo
 * the song set given back. Its tempo's factor alone is written (as
 * m4aMPlayTempoControl writes it), so the song is never restarted; a song
 * begun since (a battle's) has its own and is left alone. In the
 * director's update. */
#include "glitch.h"

#include <stdio.h>

#include "bn6.h"
#include "bn6_sound.h"
#include "debug.h"
#include "emu.h"

static struct {
	int u;           /* the factor written, 0x100 none */
	uint32_t song;   /* ... on the song whose header the player held then */
} S = { 0x100, 0 };

static void tempo(int u) {
	uint16_t d = emu_read16(BN6_MUSIC_PLAYER + BN6_MPLAY_TEMPO_D), f = (uint16_t)u, i = (uint16_t)((d * (uint32_t)u) >> 8);
	emu_write(BN6_MUSIC_PLAYER + BN6_MPLAY_TEMPO_U, &f, 2);
	emu_write(BN6_MUSIC_PLAYER + BN6_MPLAY_TEMPO_I, &i, 2);
}

void glitch_music(int u) {
	uint32_t song = emu_read32(BN6_MUSIC_PLAYER);
	if (S.u != 0x100 && song != S.song) S.u = 0x100;
	if (u == S.u || !song || (emu_read32(BN6_MUSIC_STATUS) & BN6_MUSIC_STOPPED)) return;
	tempo(u);
	S.u = u;
	S.song = song;
	/* (its clock's ticks between two lines, against the frames between
	 * them: the drag measured) */
	if (emu_debug_on()) fprintf(stderr, "glitch: the music's tempo factor %#x at its clock %u\n", (unsigned)u, (unsigned)emu_read32(BN6_MUSIC_PLAYER + BN6_MPLAY_CLOCK));
}

void glitch_music_whole(void) {
	S.u = 0x100;
	/* (a state saved while it dragged: nothing else in BN6 or the engine
	 * moves the factor, so anything but 0x100 is the glitch's) */
	if (emu_read16(BN6_MUSIC_PLAYER + BN6_MPLAY_TEMPO_U) != 0x100 && emu_read32(BN6_MUSIC_PLAYER)) tempo(0x100);
}
