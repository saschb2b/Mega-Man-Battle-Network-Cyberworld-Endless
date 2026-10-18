/* Stand-ins under BN6's rules (arena_standin.h). */
#include "arena_standin.h"

#include <stdlib.h>

#include "arena_loadout.h"
#include "bn6.h"
#include "bytes.h"
#include "chips.h"

#define CHIP_ID(e) ((unsigned)(e) & 0x1FFu)   /* a folder entry: chip | code << 9 */
#define CHIP_CODE(e) ((unsigned)(e) >> 9)
#define ENTRY(id, code) ((uint16_t)((id) | (unsigned)(code) << 9))
#define SAME_MOST 4                           /* BN6's folder: a standard chip's copies at most */
#define NO_CHIP 0xFFFFu
#define NONE 0xFF                             /* no Regular, no TagChip */

enum { KIND_STANDARD, KIND_MEGA, KIND_GIGA };

static bool bn6_chip(unsigned id) { return id >= 1 && id <= ARENA_BN6_CHIP_LAST && !BN6_CHIP_BLANK(id); }

static unsigned power(const uint8_t *rec) { return get16(rec + BN6_CHIP_ATTACK_POWER); }

/* copies of chip `id` in the folder: of its code `code`, or any where -1 */
static int copies(const OnlineLoadout *l, unsigned id, int code) {
	int n = 0;
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i)
		if (l->folder[i] != NO_CHIP && CHIP_ID(l->folder[i]) == id && (code < 0 || (int)CHIP_CODE(l->folder[i]) == code)) ++n;
	return n;
}

/* the folder's chips of BN6's of kind `kind` */
static int of_kind(const OnlineLoadout *l, unsigned kind) {
	int n = 0;
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		unsigned id = CHIP_ID(l->folder[i]);
		const uint8_t *rec = l->folder[i] != NO_CHIP && bn6_chip(id) ? chips_record((int)id) : NULL;
		if (rec && rec[BN6_CHIP_LIBRARY_TYPE] == kind) ++n;
	}
	return n;
}

/* whether one more of chip `id`'s code slot `s` may join the folder: a
 * copy in the Pack past those in it, and BN6's folder rules */
static bool may_join(const OnlineLoadout *l, unsigned id, const uint8_t *rec, int s) {
	int code = rec[BN6_CHIP_CODES + s];
	if (code == 0xFF || l->pack[id][s] <= copies(l, id, code)) return false;
	switch (rec[BN6_CHIP_LIBRARY_TYPE]) {
	case KIND_STANDARD: return copies(l, id, -1) < SAME_MOST;
	case KIND_MEGA: return !copies(l, id, -1) && of_kind(l, KIND_MEGA) < (l->mega_limit ? l->mega_limit : 5);
	case KIND_GIGA: return !copies(l, id, -1) && of_kind(l, KIND_GIGA) < (l->giga_limit ? l->giga_limit : 1);
	default: return false;
	}
}

/* how near `rec` is to `want`: its kind first, then its element, then its power */
static int nearness(const uint8_t *want, const uint8_t *rec) {
	if (!want) return 0;
	int d = abs((int)power(rec) - (int)power(want));
	return (rec[BN6_CHIP_LIBRARY_TYPE] == want[BN6_CHIP_LIBRARY_TYPE]) * 1000 + (rec[BN6_CHIP_ELEMENT] == want[BN6_CHIP_ELEMENT]) * 100
		+ (rec[BN6_CHIP_ATTACK_ELEMENT] == want[BN6_CHIP_ATTACK_ELEMENT]) * 30 - (d < 400 ? d : 400) / 4;
}

/* the chip of BN6's standing in for BN5's chip `gone` */
static uint16_t stand_in(const OnlineLoadout *l, unsigned gone) {
	const uint8_t *want = chips_record((int)gone);
	int best = 0;
	uint16_t pick = NO_CHIP;
	for (unsigned id = 1; id <= ARENA_BN6_CHIP_LAST; ++id) {
		const uint8_t *rec = bn6_chip(id) ? chips_record((int)id) : NULL;
		for (int s = 0; rec && s < 4; ++s) {
			int v = nearness(want, rec);
			if (may_join(l, id, rec, s) && (pick == NO_CHIP || v > best)) { best = v; pick = ENTRY(id, rec[BN6_CHIP_CODES + s]); }
		}
	}
	/* (no spare in the Pack: BN6's first standard chip the folder has room for) */
	for (unsigned id = 1; pick == NO_CHIP && id <= ARENA_BN6_CHIP_LAST; ++id) {
		const uint8_t *rec = bn6_chip(id) ? chips_record((int)id) : NULL;
		if (rec && rec[BN6_CHIP_LIBRARY_TYPE] == KIND_STANDARD && rec[BN6_CHIP_CODES] != 0xFF && copies(l, id, -1) < SAME_MOST)
			pick = ENTRY(id, rec[BN6_CHIP_CODES]);
	}
	return pick;
}

int arena_standins(OnlineLoadout *l, ArenaStandIn *list, int cap) {
	int n = 0;
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		uint16_t e = l->folder[i];
		if (e == NO_CHIP || !CHIP_ID(e) || bn6_chip(CHIP_ID(e))) continue;
		l->folder[i] = NO_CHIP;   /* (its place free while the stand-in is picked) */
		l->folder[i] = stand_in(l, CHIP_ID(e));
		if (l->regular == i) l->regular = NONE;
		for (int t = 0; t < 2; ++t) if (l->tag[t] == i) l->tag[0] = l->tag[1] = NONE;
		if (list && n < cap) list[n] = (ArenaStandIn){ e, l->folder[i] };
		++n;
	}
	return n;
}
