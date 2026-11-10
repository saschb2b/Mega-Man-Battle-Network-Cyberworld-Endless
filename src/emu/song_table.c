/* BN6's song table grown (song_table.h). */
#include "song_table.h"

#include <stdio.h>

#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "xvirus_moved.h"

#define BUS 0x08000000u

static const uint32_t refs[] = { BN6_SONG_TABLE_REF_START, BN6_SONG_TABLE_REF_CHANGE, BN6_SONG_TABLE_REF_CONTINUE, BN6_SONG_TABLE_REF_STOP,
	BN6_SONG_TABLE_REF_RESUME };
#define NREFS (sizeof refs / sizeof *refs)

/* (a core not started reads 0: BN6's own table) */
static bool grown(void) { return emu_read32(refs[0]) == FREE_SONG_TABLE; }

/* the live table's bus address: the grown copy once the engine's words name it, else BN6's own; 0 where no ROM is read */
static uint32_t song_table_bus(void) {
	if (!R.layout || !R.layout->song_table) return 0;
	return grown() ? FREE_SONG_TABLE : BUS + R.layout->song_table;
}

bool song_slot_ok(int slot) { return slot >= 0 && slot < (grown() ? (int)SONG_GROWN_END : (int)BN6_SONG_COUNT) && song_table_bus(); }

uint32_t song_entry_bus(int slot) { return song_slot_ok(slot) ? song_table_bus() + (uint32_t)slot * BN6_SONG_ROW : 0; }

void song_entry_empty(int slot, uint32_t out[2]) {
	out[0] = out[1] = 0;
	if (!R.data || !R.layout || !R.layout->song_table || slot < 0) return;
	const uint8_t *e = R.data + R.layout->song_table + (uint32_t)(slot < (int)BN6_SONG_COUNT ? slot : BN6_SONGS_EMPTY_SLOT) * BN6_SONG_ROW;
	out[0] = get32(e);
	out[1] = get32(e + BN6_SONG_PLAYER);
}

bool song_table_grow(void) {
	if (!R.data || !R.layout || !R.layout->song_table) return false;
	if (grown()) return true;
	uint32_t table = BUS + R.layout->song_table, empty[2];
	for (unsigned k = 0; k < NREFS; ++k)
		if (emu_read32(refs[k]) != table || get32(R.data + (refs[k] - BUS)) != table) {
			fprintf(stderr, "song_table: the word at %08x does not name BN6's song table, not grown\n", (unsigned)refs[k]);
			return false;
		}
	song_entry_empty((int)SONG_GROWN_FIRST, empty);
	/* (the copy outside any claim: each entry stays its writer's, as in BN6's own table: xsong.c's, the families', the pool's) */
	for (uint32_t n = 0; n < SONG_GROWN_END; ++n) {
		uint32_t to = FREE_SONG_TABLE + n * BN6_SONG_ROW;
		emu_write32(to, n < BN6_SONG_COUNT ? emu_read32(table + n * BN6_SONG_ROW) : empty[0]);
		emu_write32(to + BN6_SONG_PLAYER, n < BN6_SONG_COUNT ? emu_read32(table + n * BN6_SONG_ROW + BN6_SONG_PLAYER) : empty[1]);
	}
	claims_begin(SONG_TABLE_CLAIMS);
	for (unsigned k = 0; k < NREFS; ++k) emu_write32(refs[k], FREE_SONG_TABLE);
	claims_end();
	/* (for a trace's reader, and the differential's pokes of BN6's table: at its copy) */
	xv_moved_add(table, FREE_SONG_TABLE, BN6_SONG_COUNT * BN6_SONG_ROW);
	printf("song_table: BN6's song table grown to %u songs\n", (unsigned)SONG_GROWN_END);
	return true;
}
