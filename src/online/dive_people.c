/* dive_people.h. The Square's peers' puppets (online_square.c), on a
 * layer of the run. */
#include "dive_people.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "emu.h"
#include "claims.h"
#include "crew_look.h"
#include "director.h"
#include "dive_crew.h"
#include "dive_traces.h"
#include "floor_ring.h"
#include "gfx.h"
#include "layer_objs.h"
#include "map_label.h"
#include "npc.h"
#include "pixfont.h"
#include "puppet.h"
#include "run.h"

#define WRITER "dive people"
#define MOVING_FRAMES 8   /* a step seen keeps the walk this long */
#define TAG_TOP 4         /* the picture's top row a name piled on others may reach */
#define TAG_BOX 100       /* ... and the row BN6's chat box begins under, while one is open */
#define RING 14           /* world units: the ring under a crewmate's feet (#716) */

static struct {
	uint32_t script[DIVE_PEOPLE];
	uint32_t id[DIVE_PEOPLE];
	int x[DIVE_PEOPLE], y[DIVE_PEOPLE], z[DIVE_PEOPLE], moving[DIVE_PEOPLE];
	char name[DIVE_PEOPLE][ONLINE_NAME_BYTES + 1];
	int made;
	uint32_t seed;            /* the layer they were made on */
	int depth;
} P;

/* (layer_objs_more: after the layer's own, as many as its 16 leave) */
static void more(NpcList *npcs) {
	memset(&P, 0, sizeof P);
	P.seed = run.layer_seed;
	P.depth = run.depth;
	if (npcs->n >= BN6_NPC_COUNT || !npc_need_sprite(npcs, 6, PUPPET_MEGAMAN)) return;
	/* (the week's traces first, a slot or two where the layer leaves room: dive_traces.h) */
	dive_traces_more(npcs, BN6_NPC_COUNT - npcs->n);
	while (P.made < DIVE_PEOPLE && npcs->n < BN6_NPC_COUNT) {
		uint32_t at = puppet_make(6, PUPPET_MEGAMAN);
		if (!at) break;
		npcs->script[npcs->n++] = at;
		P.script[P.made++] = at;
	}
	fprintf(stderr, "dive: room on this layer for %d other divers\n", P.made);
}

static bool here(void) { return director_on_layer() && P.seed == run.layer_seed && P.depth == run.depth; }

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
	P.z[k] = peer->z;
	memcpy(P.name[k], peer->name, sizeof P.name[k]);
}

/* A diver's name over his MegaMan (issue #712), as the Square names its
 * players (map_label.h). `more`: divers under it whose names found no row. */
typedef struct { int x, y, w, more; bool crew; const char *name; } Tag;

/* What a diver is to MegaMan's crew (issue #716): one of it, about to be
 * (within the radius it forms in, a place in it left; the server's second
 * not yet passed), or neither. */
enum { SIGN_NONE, SIGN_CREW, SIGN_FORMING };

static int sign_of(int k) {
	if (dive_crew_has(P.id[k])) return SIGN_CREW;
	if (dive_crew_count() >= ONLINE_PARTY_MOST) return SIGN_NONE;
	long dx = P.x[k] - bn6_player_x(), dy = P.y[k] - bn6_player_y();
	return dx * dx + dy * dy <= (long)DIVE_CREW_NEAR * DIVE_CREW_NEAR ? SIGN_FORMING : SIGN_NONE;
}

/* the tag at (x, y) that `t` would lie on (NULL none) */
static Tag *tag_under(Tag *tags, int n, const Tag *t) {
	for (int j = 0; j < n; ++j)
		if (abs(tags[j].y - t->y) < MAP_LABEL_ROW && abs(tags[j].x - t->x) < (tags[j].w + t->w) / 2 + 4) return &tags[j];
	return NULL;
}

/* The names that show now, as the puppets stand: each over its diver's
 * head, and where divers stand on one spot (a layer's arrival) each next
 * name a row over the last, those past the picture's top counted on the
 * topmost. Reads the game's camera; changes nothing. */
static int tags_now(Tag tags[DIVE_PEOPLE]) {
	int n = 0, camera[3];
	if (!here() || !director_on_map() || !map_label_camera(camera)) return 0;
	for (int k = 0; k < P.made; ++k) {
		if (!P.id[k] || !P.name[k][0]) continue;
		Tag t = { .name = P.name[k], .w = pixfont_width(P.name[k], 1), .crew = sign_of(k) == SIGN_CREW };
		map_label_at(P.x[k], P.y[k], P.z[k], camera, &t.x, &t.y);
		if (!map_label_shown(t.x, t.y)) continue;
		t.x = map_label_x(t.x, t.name);
		Tag *under;
		while ((under = tag_under(tags, n, &t)) != NULL && under->y - MAP_LABEL_ROW >= TAG_TOP) t.y = under->y - MAP_LABEL_ROW;
		if (under) ++under->more;
		else tags[n++] = t;
	}
	return n;
}

/* the divers whose feet show, each with his sign: their count */
static int signs_now(int k_of[DIVE_PEOPLE], int sign[DIVE_PEOPLE], int camera[3]) {
	int n = 0;
	if (!here() || !director_on_map() || !map_label_camera(camera)) return 0;
	for (int k = 0; k < P.made; ++k) {
		int px, py;
		if (!P.id[k]) continue;
		map_label_floor(P.x[k], P.y[k], P.z[k], camera, &px, &py);
		if (px < 0 || px >= EMU_W || py < 0 || py >= EMU_H) continue;
		k_of[n] = k;
		sign[n++] = sign_of(k);
	}
	return n;
}

/* the ring under each crewmate, and a dotted one under a diver about to be */
static void rings_draw(bool chat) {
	int k_of[DIVE_PEOPLE], sign[DIVE_PEOPLE], camera[3];
	int n = signs_now(k_of, sign, camera);
	for (int i = 0; i < n; ++i) {
		if (sign[i] == SIGN_NONE) continue;
		int k = k_of[i];
		FloorRing how = { .colour = CREW_COLOUR, .dotted = sign[i] == SIGN_FORMING, .around = true, .bottom = chat ? TAG_BOX : EMU_H };
		floor_ring(P.x[k], P.y[k], P.z[k], RING, camera, &how);
	}
}

void dive_people_draw(bool chat) {
	rings_draw(chat);
	Tag tags[DIVE_PEOPLE];
	int n = tags_now(tags);
	for (int i = 0; i < n; ++i) {
		/* (never over BN6's chat box: the names above it stay) */
		if (chat && tags[i].y + MAP_LABEL_ROW > TAG_BOX) continue;
		char text[ONLINE_NAME_BYTES + 8];
		if (tags[i].more) snprintf(text, sizeof text, "%s +%d", tags[i].name, tags[i].more);
		else snprintf(text, sizeof text, "%s", tags[i].name);
		map_label_plate(tags[i].x, tags[i].y, text, tags[i].crew ? CREW_COLOUR : WHITE);
	}
}

void dive_people_describe(FILE *f) {
	Tag tags[DIVE_PEOPLE];
	int n = tags_now(tags);
	fprintf(f, "tags %d", n);
	for (int i = 0; i < n; ++i) {
		fprintf(f, "%s%s", i ? " " : ": ", tags[i].name);
		if (tags[i].more) fprintf(f, "+%d", tags[i].more);
		if (tags[i].crew) fputc('*', f);
	}
	fputc('\n', f);
	int k_of[DIVE_PEOPLE], sign[DIVE_PEOPLE], camera[3], shown = 0;
	int n2 = signs_now(k_of, sign, camera);
	for (int i = 0; i < n2; ++i) shown += sign[i] != SIGN_NONE;
	fprintf(f, "rings %d", shown);
	for (int i = 0, j = 0; i < n2; ++i)
		if (sign[i] != SIGN_NONE) fprintf(f, "%s%s%s", j++ ? " " : ": ", P.name[k_of[i]], sign[i] == SIGN_FORMING ? "(forming)" : "");
	fputc('\n', f);
}

void dive_people_frame(const OnlineSnapshot *s) {
	/* (only on the layer they were made on: another map took the slots' memory since) */
	if (!P.made || !here()) return;
	claims_begin(WRITER);
	for (int k = 0; k < P.made; ++k) place(k, s->connected && (size_t)k < s->peer_count ? &s->peers[k] : NULL);
	claims_end();
}
