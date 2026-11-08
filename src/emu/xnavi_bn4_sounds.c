/* xnavi_bn4_sounds.h: BN4's Navis' sounds in the pool of BN6's empty song
 * slots they share, pointed at the battle's Navi's. */
#include "xnavi_bn4_sounds.h"

#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "xnavi_room.h"
#include "xsong.h"
#include "xvirus_tx.h"

#define BUS 0x08000000u
#define NAVIS_MAX 32   /* (BN4's Navis' seats, xnavi_recipe.h XN_BN4_SEATS, and room to spare) */

/* (BN6's empty song slots, each its shared header of no tracks in the player's ROM, that no other track takes: the themes'
 * 0x26-0x31 and the families' 0x34-0x3F below and above the first two, BN5's 0x40-0x62 left alone; the five to 0xFF a movs
 * or a byte names, then those past it; docs/rom/sound.md) */
const uint16_t xnavi_bn4_pool[XNAVI_BN4_POOL_SLOTS] = { 0x32, 0x33, 0x63, 0xD0, 0xE0, 0x104, 0x117, 0x148, 0x150, 0x155, 0x159, 0x15D,
	0x167, 0x16B, 0x16E, 0x172 };

/* a Navi's songs in the pool: his ids, and each song's slot and its entry (its packed header, its player's word) */
typedef struct {
	int first_id, versions, n;
	uint16_t slot[XNAVI_BN4_POOL_SLOTS];
	uint32_t entry[XNAVI_BN4_POOL_SLOTS][2];
} Navi;
static Navi navis[NAVIS_MAX], pending;
static int nnavis, pointed = -1;

static bool pool_slot(int slot) {
	for (int i = 0; i < XNAVI_BN4_POOL_SLOTS; ++i)
		if (xnavi_bn4_pool[i] == slot) return true;
	return false;
}

/* whether `header` is one of the pool's Navis' songs at `slot` */
static bool pool_song(uint32_t slot, uint32_t header) {
	for (int i = 0; i < nnavis; ++i)
		for (int k = 0; k < navis[i].n; ++k)
			if (navis[i].slot[k] == slot && navis[i].entry[k][0] == header) return true;
	return false;
}

void xnavi_bn4_songs_set(const XVirusFamily *f, int versions) {
	const XRom *x = f->game >= 0 && f->game < XROM_COUNT ? &XR[f->game] : NULL;
	memset(&pending, 0, sizeof pending);
	if (!x || !x->data || !x->layout || !x->layout->song_table || !R.layout || !R.layout->song_table) {
		if (f->nsongs) xv_fail("no song table to put its sounds in");
		return;
	}
	pending = (Navi){ f->first_id, versions, 0, { 0 }, { { 0 } } };
	for (int i = 0; i < f->nsongs && i < XNAVI_BN4_POOL_SLOTS && !xv_failed(); ++i) {
		const XvSong *s = &f->songs[i];
		uint32_t slot = R.layout->song_table + 8u * s->slot, empty = slot + 8 <= ROM_SIZE ? get32(R.data + slot) : 0, now = emu_read32(BUS + slot);
		/* (a pool slot whose entry, in the player's ROM, is BN6's empty song, a header of no tracks; in the core's copy still
		 * that, or another BN4 Navi's song of the pool) */
		if (!pool_slot(s->slot) || empty < BUS || empty - BUS >= ROM_SIZE || R.data[empty - BUS] != 0 ||
			(now != empty && !pool_song(s->slot, now)) || (uint32_t)s->from * 8 + 8 > ROM_SIZE - x->layout->song_table) {
			xv_fail("its sound %#x's slot %#x, no free slot of the pool", s->from, s->slot);
			return;
		}
		/* (packed here to check it fits and name its header, not written: the art's room holds it in his battles alone,
		 * xnavi_bn4_art.h) */
		uint8_t *p = malloc(s->room);
		uint32_t header = p ? xsong_pack(f->game, s->from, s->at, p, s->room) : 0;
		free(p);
		if (!header) { xv_fail("its sound %#x (%u bytes of room)", s->from, (unsigned)s->room); return; }
		uint32_t player = get32(x->data + x->layout->song_table + 8u * s->from + 4);
		if (s->player) player = s->player | (uint32_t)s->player << 16;   /* (BN6's player for it: its ms and me, tools/xnavirecipe_bn4.py song_player) */
		xv_word(BUS + slot, now, header);
		xv_word(BUS + slot + 4, emu_read32(BUS + slot + 4), player);
		pending.slot[pending.n] = s->slot;
		pending.entry[pending.n][0] = header;
		pending.entry[pending.n][1] = player;
		++pending.n;
	}
}

void xnavi_bn4_songs_noted(void) {
	if (!pending.n || nnavis >= NAVIS_MAX) return;
	navis[nnavis] = pending;
	pointed = nnavis++;
	memset(&pending, 0, sizeof pending);
}

/* the pool's Navi whose ids hold `id`, -1 none */
static int navi_of(uint32_t id) {
	for (int i = 0; i < nnavis; ++i)
		if (id >= (uint32_t)navis[i].first_id && id < (uint32_t)(navis[i].first_id + navis[i].versions)) return i;
	return -1;
}

void xnavi_bn4_sounds_frame(void) {
	if (!nnavis || !R.layout || !R.layout->song_table) return;
	int which = -1;
	for (uint32_t k = 0; k < BN6_T1_COUNT && which < 0; ++k) {
		uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
		if (emu_read8(o) & 1) which = navi_of(emu_read16(o + BN6_T1_NAME_ID));
	}
	if (which < 0 || which == pointed) return;
	/* (the battle holds another BN4 Navi than the pool's: his songs at its slots, the core's copy of the song table only) */
	claims_begin(XN_CLAIMS);
	for (int k = 0; k < navis[which].n; ++k) {
		uint32_t slot = BUS + R.layout->song_table + 8u * navis[which].slot[k];
		emu_write32(slot, navis[which].entry[k][0]);
		emu_write32(slot + 4, navis[which].entry[k][1]);
	}
	claims_end();
	pointed = which;
}
