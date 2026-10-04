/* Rush's gaps on a layer (issue #14; docs/ROM_DATA.md, Rush).
 *
 * BN6 runs Rush itself: a check each frame (bn6f sub_809C968) answers A at
 * a gap's trigger strip (section-3 values 0x30-0x33) with the gap's
 * record, from a table by map group and number; with RushFood held, as
 * many as the gap's panels, its cutscene calls Rush, sets the gap's
 * deployed flag and takes one. Rush's map object (handler 0x25) then lies
 * in each panel and sets the flag that switches off the walls across the
 * gap's mouths (coords.c), so MegaMan walks over. The table counts groups
 * from 0x90, the internet's; three literals pointed at a table of the
 * engine's own, from 0x80, reach every map a layer takes over. */
#include "rush.h"

#include <string.h>

#include "bytes.h"
#include "emu.h"
#include "flags.h"
#include "net.h"
#include "net_shapes.h"
#include "netmap.h"

#define RUSH_AT (EMU_FREE + 0x0400)      /* the engine's tables (docs/EMULATION.md) */
#define RUSH_GROUPS     (RUSH_AT)         /* a per-map array for each group 0x80-0x96 */
#define RUSH_NO_MAPS    (RUSH_AT + 0x60)  /* every map of a group: none */
#define RUSH_HOST_MAPS  (RUSH_AT + 0xA0)  /* the layer's group's maps */
#define RUSH_RECORDS    (RUSH_AT + 0xE0)  /* the gaps' records, two a gap (near, far) */
#define RUSH_OBJECTS    (RUSH_AT + 0x140) /* handler 0x25's records: BN6's 13, then the layer's */

/* the check, "deployed?" and the RushFood count's text, each reading the
 * table at (group - 0x90) * 4 from its literal (Gregar; bn6f sub_809C968,
 * sub_809CA40, sub_809CA84) */
static const uint32_t table_literals[3] = { 0x0809DF70u, 0x0809DFB8u, 0x0809DFF0u };
#define RUSH_OBJ_LITERAL 0x080AB42Cu      /* handler 0x25's records' literal */
#define RUSH_OBJ_TABLE  0x080AB334u       /* ... BN6's 13 */
#define BONES_D6_PRIORITY 0x080A5C87u     /* OverworldMapObjects 0xD6's priority (2; 0xD5's is 3) */

#define RUSH_OWN 13                       /* the layer's first record in handler 0x25's list */
#define FLAG_RUSH_KNOWN 0x224
#define FLAG_DEPLOYED   0x12A             /* + the gap's index: Rush lies there */
#define FLAG_BONES      0x18E             /* + the gap's index: its bones show */
#define FLAG_NO_COIL    0x1BB             /* (set, the cutscene says "No Rush Coil!" and stops) */
#define FLAG_WALLS      0x16BF            /* - the gap's index: its mouths open (flag byte 0xFF - g) */
#define BONES_ID        0xD5              /* + the gap's index: OverworldMapObjects' bone markers */

/* A gap's world geometry: where MegaMan stands to call Rush, the stand
 * panel's middle on the lane's axis, the unit step across, and BN6's
 * direction (0 +X, 1 +Y, 2 -X, 3 -Y). Rush lies 32 on from it for each
 * panel, so each piece of him in the middle of its void panel, his paws on
 * both floors' edges, as the originals have him within 4 units of it (their
 * stands lie 8 and 16 in from the edge: from 8, he hung clear of the
 * stand's floor). */
typedef struct { int rx, ry, ux, uy, dir; } GapWorld;

static GapWorld gap_world(const NetGap *g) {
	int ex, ey, fx, fy;
	netmap_world(g->x, g->y, &ex, &ey);
	netmap_world(g->x + dir_dx[g->dir], g->y + dir_dy[g->dir], &fx, &fy);
	GapWorld w = { ex, ey, (fx - ex) / 32, (fy - ey) / 32, 0 };
	w.dir = w.ux > 0 ? 0 : w.uy > 0 ? 1 : w.ux < 0 ? 2 : 3;
	return w;
}

static void put_record(uint8_t *r, int x, int y, int dir, int len, int g) {
	memset(r, 0, 24);
	put32(r, (uint32_t)x << 16);
	put32(r + 4, (uint32_t)y << 16);
	r[12] = (uint8_t)dir;
	r[13] = (uint8_t)(len - 1);
	put32(r + 16, (uint32_t)(FLAG_DEPLOYED + g));
	put32(r + 20, (uint32_t)(FLAG_BONES + g));
}

void rush_install(int group, int number) {
	/* the per-group table, from group 0x80, every group's maps none but
	 * the layer's own */
	for (int k = 0; k < 23; ++k) emu_write32(RUSH_GROUPS + (uint32_t)k * 4, group == 0x80 + k ? RUSH_HOST_MAPS : RUSH_NO_MAPS);
	for (int m = 0; m < 16; ++m) {
		emu_write32(RUSH_NO_MAPS + (uint32_t)m * 4, 0);
		emu_write32(RUSH_HOST_MAPS + (uint32_t)m * 4, m == number && layer.ngaps ? RUSH_RECORDS : 0);
	}
	for (int i = 0; i < 3; ++i) emu_write32(table_literals[i], RUSH_GROUPS + 0x40);
	/* each gap's records: the near side's (its trigger strip's), and the
	 * far side's as BN6 sets both */
	uint8_t recs[4 * 24];
	memset(recs, 0, sizeof recs);
	for (int g = 0; g < layer.ngaps && g < 2; ++g) {
		const NetGap *p = &layer.gap[g];
		GapWorld w = gap_world(p);
		put_record(recs + 48 * g, w.rx, w.ry, w.dir, p->len, g);
		put_record(recs + 48 * g + 24, w.rx + 32 * (p->len + 1) * w.ux, w.ry + 32 * (p->len + 1) * w.uy, (w.dir + 2) & 3, p->len, g);
	}
	emu_write(RUSH_RECORDS, recs, sizeof recs);
	/* Rush lying in the gap: BN6's records, then one a gap (its flags,
	 * sprite 7:0x9E's animation 5, under MegaMan, turned for a gap along
	 * world Y) */
	uint8_t obj[(RUSH_OWN + 2) * 16];
	for (int i = 0; i < RUSH_OWN * 16; ++i) obj[i] = emu_read8(RUSH_OBJ_TABLE + (uint32_t)i);
	for (int g = 0; g < 2; ++g) {
		uint8_t *o = obj + (RUSH_OWN + g) * 16;
		memset(o, 0, 16);
		put16(o, (uint32_t)(FLAG_DEPLOYED + g));
		put16(o + 2, (uint32_t)(FLAG_WALLS - g));
		put16(o + 4, (uint32_t)(FLAG_BONES + g));
		o[6] = 0x1C;
		o[7] = 0x9E;
		o[8] = 5;
		o[12] = 3;
		o[13] = g < layer.ngaps && (gap_world(&layer.gap[g]).dir & 1);
	}
	emu_write(RUSH_OBJECTS, obj, sizeof obj);
	emu_write32(RUSH_OBJ_LITERAL, RUSH_OBJECTS);
	/* (the second gap's bones under MegaMan too, as the first's) */
	emu_write8(BONES_D6_PRIORITY, 3);
	/* Rush known (BN6's seller sets it); each gap open, its bones shown */
	flag_set(FLAG_RUSH_KNOWN);
	flag_clear(FLAG_NO_COIL);
	for (int g = 0; g < 2; ++g) {
		flag_clear(FLAG_DEPLOYED + g);
		flag_set(FLAG_BONES + g);
	}
}

int rush_objects(uint8_t *recs, int n, int max) {
	for (int g = 0; g < layer.ngaps && g < 2; ++g) {
		const NetGap *p = &layer.gap[g];
		GapWorld w = gap_world(p);
		/* in each panel, at its spot +(64, -64, -64) as the game keeps
		 * floor sprites: Rush (handler 0x25), and the bones (handler 0) */
		for (int k = 0; k < p->len; ++k)
			for (int b = 0; b < 2 && n < max; ++b) {
				uint8_t *r = recs + n++ * 20;
				memset(r, 0, 20);
				r[0] = 5;
				r[1] = b ? 0 : 0x25;
				put32(r + 4, (uint32_t)((int32_t)(w.rx + 32 * (k + 1) * w.ux + 64) * 65536));
				put32(r + 8, (uint32_t)((int32_t)(w.ry + 32 * (k + 1) * w.uy - 64) * 65536));
				put32(r + 12, (uint32_t)(-64 * 65536));
				put32(r + 16, b ? (uint32_t)(BONES_ID + g) : (uint32_t)(RUSH_OWN + g));
			}
	}
	return n;
}
