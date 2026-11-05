/* xnavi_bn4_sounds.h: BN4's Navis' sounds in the pool of BN6's empty song
 * slots they share. */
#include "xnavi_bn4_sounds.h"

#include "bytes.h"
#include "emu.h"
#include "rom.h"
#include "xsong.h"
#include "xvirus_tx.h"

#define BUS 0x08000000u

/* (BN6's empty song slots, each its shared header of no tracks in the player's ROM, that no other track takes: the themes'
 * 0x26-0x31 and the families' 0x34-0x3F below and above the first two, BN5's 0x40-0x62 left alone; the five to 0xFF a movs
 * or a byte names, then those past it; docs/rom/sound.md) */
const uint16_t xnavi_bn4_pool[XNAVI_BN4_POOL_SLOTS] = { 0x32, 0x33, 0x63, 0xD0, 0xE0, 0x104, 0x117, 0x148, 0x150, 0x155, 0x159, 0x15D,
	0x167, 0x16B, 0x16E, 0x172 };

static bool pool_slot(int slot) {
	for (int i = 0; i < XNAVI_BN4_POOL_SLOTS; ++i)
		if (xnavi_bn4_pool[i] == slot) return true;
	return false;
}

void xnavi_bn4_songs_set(const XVirusFamily *f) {
	const XRom *x = f->game >= 0 && f->game < XROM_COUNT ? &XR[f->game] : NULL;
	if (!x || !x->data || !x->layout || !x->layout->song_table || !R.layout || !R.layout->song_table) {
		if (f->nsongs) xv_fail("no song table to put its sounds in");
		return;
	}
	for (int i = 0; i < f->nsongs && !xv_failed(); ++i) {
		const XvSong *s = &f->songs[i];
		uint32_t slot = R.layout->song_table + 8u * s->slot, empty = slot + 8 <= ROM_SIZE ? get32(R.data + slot) : 0;
		/* (a pool slot still BN6's empty song, a header of no tracks, in the player's ROM and in the core's copy) */
		if (!pool_slot(s->slot) || empty < BUS || empty - BUS >= ROM_SIZE || R.data[empty - BUS] != 0 ||
			emu_read32(BUS + slot) != empty || (uint32_t)s->from * 8 + 8 > ROM_SIZE - x->layout->song_table) {
			xv_fail("its sound %#x's slot %#x, no free slot of the pool", s->from, s->slot);
			return;
		}
		uint8_t *p = xv_put(s->at, s->room, true);
		uint32_t header = p ? xsong_pack(f->game, s->from, s->at, p, s->room) : 0;
		if (!header) { xv_fail("its sound %#x (%u bytes of room)", s->from, (unsigned)s->room); return; }
		xv_word(BUS + slot, empty, header);
		xv_word(BUS + slot + 4, get32(R.data + slot + 4), get32(x->data + x->layout->song_table + 8u * s->from + 4));
	}
}
