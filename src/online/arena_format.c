/* The MegaMan a NetBattle carries (arena_format.h). */
#include "arena_format.h"

#include <string.h>

#include "bn6.h"

#define CHIP_ID(e) ((unsigned)(e) & 0x1FFu)   /* a folder entry: chip | code << 9 */

bool arena_folder_holds(const OnlineLoadout *l, int k) {
	if (!l || k < 0 || k >= ONLINE_FOLDERS) return false;
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		uint16_t e = l->folders[k][i];
		if (e != 0xFFFF && CHIP_ID(e)) return true;
	}
	return false;
}

unsigned arena_folders_fitting(const OnlineLoadout *l) {
	unsigned fits = 0;
	for (int k = 0; k < ONLINE_FOLDERS; ++k)
		if (arena_folder_holds(l, k)) fits |= 1u << k;
	return fits;
}

void arena_format_carry(const OnlineLoadout *all, int k, OnlineLoadout *out) {
	*out = *all;
	if (k < 0 || k >= ONLINE_FOLDERS) k = 0;
	memcpy(out->folder, all->folders[k], sizeof out->folder);
	out->regular = all->regulars[k];
	out->tag[0] = all->tags[k][0];
	out->tag[1] = all->tags[k][1];
}
