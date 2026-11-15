/* dive_people.h. The Square's peers' puppets (online_square.c), on a
 * layer of the run. */
#include "dive_people.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "emu.h"
#include "mapslot.h"
#include "claims.h"
#include "director.h"
#include "layer_objs.h"
#include "npc.h"
#include "puppet.h"
#include "run.h"

#define WRITER "dive people"
#define MOVING_FRAMES 8   /* a step seen keeps the walk this long */
#define TALK_ROOM 1024    /* the dive's chats' words, one archive at a time */
#define REACH 24          /* world units from MegaMan's probe a diver is spoken to */

static struct {
	uint32_t script[DIVE_PEOPLE];
	uint32_t id[DIVE_PEOPLE];
	int x[DIVE_PEOPLE], y[DIVE_PEOPLE], moving[DIVE_PEOPLE];
	int made;
	uint32_t room;            /* the chats' words' room */
	uint32_t seed;            /* the layer they were made on */
	int depth;
} P;

/* (layer_objs_more: after the layer's own, as many as its 16 leave) */
static void more(NpcList *npcs) {
	memset(&P, 0, sizeof P);
	static const uint8_t empty[] = { 2, 0, 0xE6 };
	P.room = mapslot_alloc(empty, TALK_ROOM);
	P.seed = run.layer_seed;
	P.depth = run.depth;
	if (npcs->n >= BN6_NPC_COUNT || !npc_need_sprite(npcs, 6, PUPPET_MEGAMAN)) return;
	while (P.made < DIVE_PEOPLE && npcs->n < BN6_NPC_COUNT) {
		uint32_t at = puppet_make(6, PUPPET_MEGAMAN);
		if (!at) break;
		npcs->script[npcs->n++] = at;
		P.script[P.made++] = at;
	}
	fprintf(stderr, "dive: room on this layer for %d other divers\n", P.made);
}

static bool here(void) { return director_on_layer() && P.seed == run.layer_seed && P.depth == run.depth; }

uint32_t dive_people_talk_room(void) { return here() ? P.room : 0; }

uint32_t dive_people_ahead(void) {
	if (!here()) return 0;
	int dx, dy, x = bn6_player_x(), y = bn6_player_y();
	npc_probe(emu_read8(BN6_PLAYER_FACING) & 7, &dx, &dy);
	uint32_t best = 0;
	int best_d = REACH * REACH;
	for (int k = 0; k < P.made; ++k) {
		if (!P.id[k]) continue;
		int ex = P.x[k] - (x + dx), ey = P.y[k] - (y + dy), d = ex * ex + ey * ey;
		if (d < best_d) { best = P.id[k]; best_d = d; }
	}
	return best;
}

void dive_people_on(bool on) {
	layer_objs_more = on ? more : NULL;
	if (!on) memset(&P, 0, sizeof P);
}

static void place(int k, const OnlinePeer *peer) {
	uint32_t at = P.script[k];
	puppet_show(at, peer != NULL);
	if (!peer) { P.id[k] = 0; return; }
	if (P.id[k] == peer->id && (P.x[k] != peer->x || P.y[k] != peer->y)) P.moving[k] = MOVING_FRAMES;
	else if (P.moving[k]) --P.moving[k];
	puppet_anim(at, peer->facing + (P.moving[k] ? PUPPET_WALK : 0));
	puppet_place(at, peer->x, peer->y, peer->z);
	P.id[k] = peer->id;
	P.x[k] = peer->x;
	P.y[k] = peer->y;
}

void dive_people_frame(const OnlineSnapshot *s) {
	/* (only on the layer they were made on: another map took the slots' memory since) */
	if (!P.made || !here()) return;
	claims_begin(WRITER);
	for (int k = 0; k < P.made; ++k) place(k, s->connected && (size_t)k < s->peer_count ? &s->peers[k] : NULL);
	claims_end();
}
