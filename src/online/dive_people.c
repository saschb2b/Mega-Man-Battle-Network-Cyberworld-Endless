/* dive_people.h. The Square's peers' puppets (online_square.c), on a
 * layer of the run. */
#include "dive_people.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "claims.h"
#include "director.h"
#include "dive_traces.h"
#include "layer_objs.h"
#include "npc.h"
#include "puppet.h"
#include "run.h"

#define WRITER "dive people"
#define MOVING_FRAMES 8   /* a step seen keeps the walk this long */

static struct {
	uint32_t script[DIVE_PEOPLE];
	uint32_t id[DIVE_PEOPLE];
	int x[DIVE_PEOPLE], y[DIVE_PEOPLE], moving[DIVE_PEOPLE];
	int made;
	uint32_t seed;            /* the layer they were made on */
	int depth;
} P;

/* (layer_objs_more: after the layer's own, as many as its 16 leave) */
static void more(NpcList *npcs) {
	memset(&P, 0, sizeof P);
	if (npcs->n >= BN6_NPC_COUNT || !npc_need_sprite(npcs, 6, PUPPET_MEGAMAN)) return;
	/* (the week's traces first, a slot or two where the layer leaves room: dive_traces.h) */
	dive_traces_more(npcs, BN6_NPC_COUNT - npcs->n);
	while (P.made < DIVE_PEOPLE && npcs->n < BN6_NPC_COUNT) {
		uint32_t at = puppet_make(6, PUPPET_MEGAMAN);
		if (!at) break;
		npcs->script[npcs->n++] = at;
		P.script[P.made++] = at;
	}
	P.seed = run.layer_seed;
	P.depth = run.depth;
	fprintf(stderr, "dive: room on this layer for %d other divers\n", P.made);
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
	if (!P.made || !director_on_layer() || P.seed != run.layer_seed || P.depth != run.depth) return;
	claims_begin(WRITER);
	for (int k = 0; k < P.made; ++k) place(k, s->connected && (size_t)k < s->peer_count ? &s->peers[k] : NULL);
	claims_end();
}
