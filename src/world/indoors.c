/* indoors.h. A place of home is BN6's own map as it stands: its tiles,
 * walls, furniture (its map objects), checks and doors; the run's people
 * in it and none of BN6's, no map scripts (they run the story), its own
 * warp list in which BN6's doors that stay open are copied and every other
 * entry leads back to where Lan comes in. */
#include "indoors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "bn6.h"
#include "emu.h"
#include "flags.h"
#include "npc.h"
#include "place_lines.h"
#include "text.h"

/* BN6's lists, as read before the maps were taken over */
#define LISTS 24
static struct { int group, number; uint32_t list; } bn6[LISTS];
static int nbn6;

uint32_t indoors_bn6_warps(int group, int number) {
	for (int i = 0; i < nbn6; ++i) if (bn6[i].group == group && bn6[i].number == number) return bn6[i].list;
	uint32_t list = mapslot_warps(group, number);
	if (!list || nbn6 >= LISTS) return list;
	bn6[nbn6].group = group;
	bn6[nbn6].number = number;
	bn6[nbn6++].list = list;
	return list;
}

void indoors_dest(uint32_t list, int entry, int *x, int *y, int *face) {
	uint32_t e = list + 16u * (uint32_t)(entry - 1);
	*x = (int32_t)emu_read32(e + 4) >> 16;
	*y = (int32_t)emu_read32(e + 8) >> 16;
	*face = emu_read8(e + 3);
}

/* (CYBERWORLD_TOWN_DEBUG: each trigger value's cells' bounds) */
static void debug_triggers(int group, int number) {
	AreaSrc a;
	if (!getenv("CYBERWORLD_TOWN_DEBUG") || !area_src_load(group, number, &a)) return;
	for (int v = 1; v < 256; ++v) {
		int n = 0, b[4] = { 1 << 20, 1 << 20, -(1 << 20), -(1 << 20) }, z = 0, h = 0, ty = 0;
		for (int i = 0; i < a.nsec[3]; ++i) {
			const CoordCell *c = &a.sec[3][i];
			if (c->value != v) continue;
			if (!n) { z = c->z; h = c->height; ty = c->type; }
			++n;
			if (c->x < b[0]) b[0] = c->x;
			if (c->y < b[1]) b[1] = c->y;
			if (c->x > b[2]) b[2] = c->x;
			if (c->y > b[3]) b[3] = c->y;
		}
		if (n) fprintf(stderr, "indoors %02x:%02x trigger %02x: %d cells %d,%d to %d,%d (z %d h %d type %x)\n", group, number, v, n, b[0], b[1], b[2], b[3], z, h, ty);
	}
	area_src_free(&a);
}

/* the people of map (group, number) (place_lines.c) into `npcs`, their
 * words in a text archive of their own, written where allocation is */
static void people(int group, int number, NpcList *npcs) {
	static TextArchive text;
	ta_begin(&text);
	int who[MAX_FOLK], n = 0;
	/* (the game's people of list 5 have their sprite's face less 0x20) */
	for (int i = 0; i < place_nfolk && n < MAX_FOLK; ++i)
		if (place_folk[i].group == group && place_folk[i].number == number) {
			who[n++] = i;
			ta_talk(&text, place_folk[i].folk.words, place_folk[i].folk.sprite - 0x20);
		}
	uint32_t words = n ? ta_commit(&text) : 0;
	for (int k = 0; k < n && words && npcs->n < 16; ++k) {
		const PlaceFolk *p = &place_folk[who[k]];
		const Folk *f = &p->folk;
		if (!npc_need_sprite(npcs, f->cat, f->sprite)) continue;
		npcs->script[npcs->n++] = p->counter ? npc_counter_talker(f->cat, f->sprite, f->x, f->y, 0, f->face, words, k, 0, p->counter)
			: npc_talker(f->cat, f->sprite, f->x, f->y, 0, f->face, words, k, -1, false);
	}
}

uint32_t indoors_take_over(int group, int number, int x, int y, unsigned keep, int song_k, int song) {
	debug_triggers(group, number);
	uint32_t from = indoors_bn6_warps(group, number);
	NpcList npcs;
	memset(&npcs, 0, sizeof npcs);
	people(group, number, &npcs);
	npcs.objects = mapslot_objects(group, number);
	npc_objects_sprites(&npcs);
	if (!from || !mapslot_install(group, number, &npcs, NULL, 0)) return 0;
	uint32_t list = mapslot_own_warps(group, number, x, y, 1);
	if (!list) return 0;
	for (int e = 1; e <= 16; ++e) if (keep >> e & 1) mapslot_copy_warp(list, e, from, e);
	return mapslot_music_home(song_k, group, number, song) ? list : 0;
}

void indoors_door_to(uint32_t list, int entry, int x, int y) {
	if (!list || entry < 1 || entry > 16) return;
	emu_write32(list + 16u * (uint32_t)(entry - 1) + 4, (uint32_t)x << 16);
	emu_write32(list + 16u * (uint32_t)(entry - 1) + 8, (uint32_t)y << 16);
}

bool indoors_spot(int group, int number, int value, int *x, int *y, int *box) {
	AreaSrc a;
	if (!area_src_load(group, number, &a)) return false;
	long sx = 0, sy = 0;
	int n = 0, b[4] = { 1 << 20, 1 << 20, -(1 << 20), -(1 << 20) };
	for (int i = 0; i < a.nsec[3]; ++i) {
		const CoordCell *c = &a.sec[3][i];
		if (c->value != value) continue;
		sx += c->x + 4; sy += c->y + 4; ++n;
		if (c->x < b[0]) b[0] = c->x;
		if (c->y < b[1]) b[1] = c->y;
		if (c->x + 8 > b[2]) b[2] = c->x + 8;
		if (c->y + 8 > b[3]) b[3] = c->y + 8;
	}
	area_src_free(&a);
	if (!n) return false;
	*x = (int)(sx / n);
	*y = (int)(sy / n);
	if (box) memcpy(box, b, sizeof b);
	return true;
}

void indoors_shut(unsigned keep) {
	for (int e = 1; e <= 15; ++e) if (!(keep >> e & 1)) flag_set(BN6_FLAG_WARP_OFF + e);
}
