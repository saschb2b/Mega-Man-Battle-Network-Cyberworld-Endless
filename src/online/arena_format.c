/* The rules a NetBattle is fought by (arena_format.h). */
#include "arena_format.h"

#include <string.h>

#include "arena_loadout.h"
#include "bn6.h"
#include "protocol.h"

#define CHIP_ID(e) ((unsigned)(e) & 0x1FFu)   /* a folder entry: chip | code << 9 */

unsigned arena_format_for(uint8_t mine, uint8_t peer) {
	bool both = (mine & peer & ONLINE_ROMS_BN5) != 0;
	return ARENA_FULL_RULES && both ? ONLINE_FORMAT_FULL : ONLINE_FORMAT_BN6;
}

bool arena_format_ok(unsigned format, uint8_t mine) {
	return format == ONLINE_FORMAT_BN6 || (ARENA_FULL_RULES && format == ONLINE_FORMAT_FULL && (mine & ONLINE_ROMS_BN5));
}

bool arena_souls_home(const OnlineLoadout *l, unsigned format) {
	/* (under either rules until full ones carry them: ARENA_FULL_SOULS) */
	return l && l->souls && (format != ONLINE_FORMAT_FULL || !ARENA_FULL_SOULS);
}

/* (one of BN6's own chips: arena_loadout_check's rule) */
static bool bn6_chip(unsigned id) { return id <= ARENA_BN6_CHIP_LAST && !BN6_CHIP_BLANK(id); }

ArenaFolderFit arena_folder_fit(const OnlineLoadout *l, unsigned format, int k) {
	if (!l || k < 0 || k >= ONLINE_FOLDERS) return ARENA_FOLDER_EMPTY;
	bool any = false, bn5 = false;
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		uint16_t e = l->folders[k][i];
		if (e == 0xFFFF || !CHIP_ID(e)) continue;
		any = true;
		bn5 |= !bn6_chip(CHIP_ID(e));
	}
	if (!any) return ARENA_FOLDER_EMPTY;
	return bn5 && format != ONLINE_FORMAT_FULL ? ARENA_FOLDER_BN5 : ARENA_FOLDER_FITS;
}

unsigned arena_folders_fitting(const OnlineLoadout *l, unsigned format) {
	unsigned fits = 0;
	for (int k = 0; k < ONLINE_FOLDERS; ++k)
		if (arena_folder_fit(l, format, k) == ARENA_FOLDER_FITS) fits |= 1u << k;
	return fits;
}

void arena_format_carry(const OnlineLoadout *all, unsigned format, int k, OnlineLoadout *out) {
	*out = *all;
	if (k < 0 || k >= ONLINE_FOLDERS) k = 0;
	memcpy(out->folder, all->folders[k], sizeof out->folder);
	out->regular = all->regulars[k];
	out->tag[0] = all->tags[k][0];
	out->tag[1] = all->tags[k][1];
	if (arena_souls_home(all, format)) out->souls = 0;
	if (format == ONLINE_FORMAT_FULL) return;
	/* (BN6's rules: what plain BN6 has not stays home) */
	for (unsigned id = 1; id < ONLINE_PACK_IDS; ++id)
		if (!bn6_chip(id)) memset(out->pack[id], 0, sizeof out->pack[id]);
}
