/* npc_anims.h: each NPC in another game's sprite, the animations our code
 * tells it to play against the sprite as copied into the core (npc.c
 * reads its table: how many animations, whether one's first frame draws
 * anything). One it stands or poses in must draw; one it logs in, out or
 * strikes by must be there (a log-in's first frames are blank). */
#include "npc_anims.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "emu.h"
#include "guardians.h"
#include "liberation.h"
#include "liberation_board.h"
#include "npc.h"
#include "rom.h"
#include "stage_npc.h"
#include "xguardian.h"
#include "xnavi.h"

static FILE *out;
static int fails, checked;

/* One NPC: who, its sprite, and what it lacks so far */
typedef struct {
	char who[48];
	int list, index;
	char bad[240];
	int nbad;
} Check;

/* Animation `anim` (none where -1) told to sprite c's NPC, for `what`: there, and drawn where `drawn` */
static void need(Check *c, int anim, bool drawn, const char *what) {
	if (anim < 0) return;
	bool there = npc_sprite_has(c->list, c->index, anim);
	if (there && (!drawn || npc_sprite_shows(c->list, c->index, anim))) return;
	if (strstr(c->bad, what) && c->nbad > 6) return;
	size_t len = strlen(c->bad);
	snprintf(c->bad + len, sizeof c->bad - len, "%s%s %d %s", len ? ", " : "", what, anim, there ? "blank" : "past its animations");
	++c->nbad;
}

static void done(Check *c) {
	++checked;
	fprintf(out, "%s\t%s\tlist %d sprite %d, %d animations\t%s\n", c->nbad ? "FAIL" : "ok", c->who, c->list, c->index,
		npc_sprite_anims(c->list, c->index), c->bad);
	if (c->nbad) {
		fprintf(stderr, "npc-anims: FAIL %s (list %d sprite %d): %s\n", c->who, c->list, c->index, c->bad);
		++fails;
	}
}

/* A guardian of another game's on a layer (guardian_body: standing every
 * way, his pose, his log-in and log-out), as a Glitch copy (a talker
 * facing down-left, layer_objs.c, turned to MegaMan where his sprite turns
 * by npc_sprite_turns' own check), and a Darkloid on a dark layer's board
 * (standing, his pose his strike; never logged in or out) */
static void guardian_check(int navi) {
	int slot = xguardian_slot(navi);
	if (slot < 0) {
		fprintf(out, "skip\t%s (%d)\tno sprite of his own game's copied in: BN6's HeelNavi\n", guardian(navi)->name, navi);
		return;
	}
	guardian_set_face(navi, slot);
	Check c = { .list = 6, .index = slot };
	snprintf(c.who, sizeof c.who, "%s (guardian %d)", guardian(navi)->name, navi);
	for (int face = 0; face < 8; ++face) {
		NpcBody b = guardian_body(navi, face);
		c.list = b.list;
		c.index = b.index;
		need(&c, b.anim, true, "stands");
		need(&c, b.pose, true, "poses");
		if (guardian_darkloid(navi)) continue;
		need(&c, b.log_in, false, "logs in");
		need(&c, b.log_out, false, "logs out");
		if (face == 5) need(&c, npc_shown_anim(b.list, b.index, b.anim), true, "copy stands");
	}
	done(&c);
}

/* The bystanders of another game's areas (NetAreaDef.xnavi, xnavi2):
 * talkers facing down (layer_objs.c), turned to MegaMan where their
 * sprite turns */
static void bystanders_check(void) {
	for (int x = 0; x < XROM_COUNT; ++x) {
		if (!XR[x].data || !XR[x].layout) continue;
		int seen[8] = { 0 }, nseen = 0;
		for (int i = 0; i < XR[x].layout->nareas; ++i) {
			const NetAreaDef *a = &XR[x].layout->areas[i];
			int navis[2] = { a->xnavi, a->xnavi2 };
			for (int k = 0; k < 2; ++k) {
				bool again = !navis[k];
				for (int j = 0; j < nseen; ++j) again |= seen[j] == navis[k];
				if (again || nseen == 8) continue;
				seen[nseen++] = navis[k];
				int slot = xnavi_slot(x, navis[k], -1);
				if (slot < 0) continue;
				Check c = { .list = 6, .index = slot };
				snprintf(c.who, sizeof c.who, "%s's bystander %d", XR[x].layout->tag, navis[k]);
				need(&c, npc_shown_anim(6, slot, 4), true, "stands");
				done(&c);
			}
		}
	}
}

/* A liberation's teammates in BN5's sprites (xnavi_mate: standing every
 * way on the board, and walking and running as the player in his body),
 * its guardians (standing, striking: liberation_look.c) and BN5's map
 * objects (their animations as blockers.c and director_dark.c play them) */
static void liberation_check(void) {
	for (int navi = GUARDIAN_OLDER_FIRST; navi <= GUARDIAN_OLDER_LAST; ++navi) {
		int slot = xnavi_mate(0, guardian_older_sprite(navi));
		if (slot < 0) continue;
		Check c = { .list = 6, .index = slot };
		snprintf(c.who, sizeof c.who, "%s (teammate)", guardian(navi)->name);
		for (int a = 0; a < 24; ++a) need(&c, a, a < 8, a < 8 ? "stands" : "walks");
		done(&c);
	}
	static const char *const units[] = { "BigBrute", "TinHawk", "Bladia" };
	for (int kind = UNIT_BRUTE; kind <= UNIT_BLADIA; ++kind) {
		int slot = xnavi_object(XOBJ_BRUTE + kind - UNIT_BRUTE);
		if (slot < 0) continue;
		Check c = { .list = 7, .index = slot };
		snprintf(c.who, sizeof c.who, "%s (liberation guardian)", units[kind - UNIT_BRUTE]);
		for (int face = 0; face < 8; ++face) {
			need(&c, lib_look_unit_anim(kind, face, false), true, "stands");
			need(&c, lib_look_unit_anim(kind, face, true), false, "strikes");
		}
		done(&c);
	}
	static const struct { int obj, last; const char *who; } objects[] = {
		{ XOBJ_DARK_FLAME, 0, "BN5's flame of darkness" }, { XOBJ_CUBE, 1, "BN5's Security Cube" },
		{ XOBJ_DARK_WALL, 3, "BN5's wall of dark flames" }, { XOBJ_DARK_HOLE, 0, "BN5's dark hole" },
	};
	for (unsigned k = 0; k < sizeof objects / sizeof *objects; ++k) {
		int slot = xnavi_object(objects[k].obj);
		if (slot < 0) continue;
		Check c = { .list = 7, .index = slot };
		snprintf(c.who, sizeof c.who, "%s", objects[k].who);
		for (int a = 0; a <= objects[k].last; ++a) need(&c, a, false, "plays");
		done(&c);
	}
	int flame = xnavi_bn6_flame();
	if (flame >= 0) {
		Check c = { .list = 7, .index = flame };
		snprintf(c.who, sizeof c.who, "BN6's flame of darkness, copied");
		need(&c, 0, false, "plays");
		done(&c);
	}
}

int npc_anims_run(const char *path) {
	if (!emu_init(R.data, ROM_SIZE)) return 1;
	out = fopen(path, "w");
	if (!out) return 1;
	bystanders_check();
	for (int navi = GUARDIAN_OLDER_FIRST; navi <= GUARDIAN_DARKLOID_LAST; ++navi) guardian_check(navi);
	for (int k = 0; k < xguardian_bn4_count(); ++k) guardian_check(xguardian_bn4_navi(k));
	for (int navi = GUARDIAN_NEBULA_FIRST; navi <= GUARDIAN_NEBULA_LAST; ++navi) guardian_check(navi);
	liberation_check();
	fprintf(out, "%d checked, %d failing\n", checked, fails);
	printf("npc-anims: %d NPCs in another game's sprite checked, %d failing (%s)\n", checked, fails, path);
	bool ok = !ferror(out);
	return fclose(out) == 0 && ok && !fails && checked ? 0 : 1;
}
