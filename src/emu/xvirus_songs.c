/* xvirus_songs.h: a moved family's sounds BN6 has none of, copied into
 * BN6's empty song slots at its install. */
#include "xvirus_songs.h"

#include <stdio.h>

#include "bytes.h"
#include "emu.h"
#include "rom.h"
#include "xsong.h"
#include "xvirus_tx.h"

#define BUS 0x08000000u

static bool in_rom(uint32_t a, uint32_t n) { return a >= BUS && a - BUS <= ROM_SIZE && n <= ROM_SIZE - (a - BUS); }

/* whether the song header at bus address `at` in the core's copy is its game's song `song`'s (its tracks, blocks, priority and
 * reverb), as another install of the shared allocations' slot copied it */
static bool song_there(int game, int song, uint32_t at) {
	const XRom *x = game >= 0 && game < XROM_COUNT ? &XR[game] : NULL;
	if (!x || !x->data || !x->layout || !x->layout->song_table || !in_rom(at, 4) || (uint32_t)song * 8 + 8 > ROM_SIZE - x->layout->song_table)
		return false;
	uint32_t h = get32(x->data + x->layout->song_table + 8u * (uint32_t)song);
	return in_rom(h, 4) && emu_read32(at) == get32(x->data + (h - BUS)) && emu_read8(at) != 0;
}

void xvirus_songs_set(const XVirusFamily *f) {
	const XRom *x = f->game >= 0 && f->game < XROM_COUNT ? &XR[f->game] : NULL;
	for (int i = 0; i < f->nsongs && !xv_failed(); ++i) {
		const XvSong *s = &f->songs[i];
		uint32_t table = R.layout->song_table, slot = table + 8u * s->slot;
		/* (the slot BN6's empty song, a header of no tracks, in the player's ROM: copied where the core's copy still
		 * holds it; where another install of the shared slot (docs/multirom/) copied its game's same song there first,
		 * found there) */
		uint32_t empty = table && slot + 8 <= ROM_SIZE ? get32(R.data + slot) : 0;
		if (!in_rom(empty, 1) || R.data[empty - BUS] != 0) { xv_fail("its song's slot %#x", s->slot); return; }
		if (s->slot != xsong_sound_slot(f->game, s->from) || !x || !x->data || !x->layout || !x->layout->song_table ||
			(uint32_t)s->from * 8 + 8 > ROM_SIZE - x->layout->song_table) {
			xv_fail("its sound %#x's slot %#x is not the one every track copies it to", s->from, s->slot);
			return;
		}
		uint32_t now = emu_read32(BUS + slot);
		if (now != empty) {
			/* (its game's same song there: a family's packed copy before it, or xsong.c's copy of another track's install, the
			 * Navis' or the chips', made before the families') */
			int game = -1;
			if (!song_there(f->game, s->from, now) && !(xsong_copy_of(s->slot, &game) == s->from && game == f->game))
				xv_fail("its song's slot %#x holds another song", s->slot);
			continue;
		}
		uint8_t *p = xv_put(s->at, s->room, true);
		uint32_t header = p ? xsong_pack(f->game, s->from, s->at, p, s->room) : 0;
		if (!header) { xv_fail("its sound %#x (%u bytes of room)", s->from, (unsigned)s->room); return; }
		xv_word(BUS + slot, empty, header);
		xv_word(BUS + slot + 4, get32(R.data + slot + 4), get32(x->data + x->layout->song_table + 8u * s->from + 4));
	}
}

void xvirus_songs_noted(const XVirusFamily *f) {
	for (int i = 0; i < f->nsongs; ++i)
		if (!xsong_found(f->game, f->songs[i].from, f->songs[i].slot))
			fprintf(stderr, "xvirus: %s's sound %#x at slot %#x not noted\n", f->name, f->songs[i].from, f->songs[i].slot);
}
