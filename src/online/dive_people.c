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
#include "dive_comings.h"
#include "dive_crew.h"
#include "dive_dots.h"
#include "dive_seats.h"
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
#define ARRIVAL_MS 2000   /* MegaMan's own marked so long after an arrival */
#define STACKED 16        /* ... while another diver stands this near him (world units) */
#define TAG_BOX 100       /* ... and the row BN6's chat box begins under, while one is open */
#define RING 14           /* world units: the ring under a crewmate's feet (#716) */
#define CREW_OTHERS (ONLINE_PARTY_MOST - 1)   /* the puppets a full crew needs, before the traces' (#755) */

/* One of the others in the layer's room this frame, as the names and
 * rings read him: his puppet, or none where the layer had no room left
 * (issue #755: his name and ring stand where he stands all the same) */
typedef struct {
	uint32_t id;
	int x, y, z;
	char name[ONLINE_NAME_BYTES + 1];
	bool drawn;
} Who;

static struct {
	uint32_t script[DIVE_PEOPLE];
	uint32_t id[DIVE_PEOPLE];          /* the diver puppet k shows, 0 none */
	int x[DIVE_PEOPLE], y[DIVE_PEOPLE], moving[DIVE_PEOPLE];
	int made;
	Who who[DIVE_PEOPLE];
	int nwho, undrawn;
	uint32_t seed;            /* the layer they were made on */
	uint32_t arrived;         /* ... and when (SDL_GetTicks) */
	int depth;
} P;

static int dev_room;   /* diveroom=N: N puppets at most, 0 as the layer leaves */

void dive_people_dev_room(int most) { dev_room = most; }

static void make_puppets(NpcList *npcs, int most) {
	if (dev_room > 0 && most > dev_room) most = dev_room;
	while (P.made < most && npcs->n < BN6_NPC_COUNT) {
		uint32_t at = puppet_make(6, PUPPET_MEGAMAN);
		if (!at) break;
		npcs->script[npcs->n++] = at;
		P.script[P.made++] = at;
	}
}

/* (layer_objs_more: after the layer's own, as many as its 16 leave) */
static void more(NpcList *npcs) {
	memset(&P, 0, sizeof P);
	P.seed = run.layer_seed;
	P.arrived = SDL_GetTicks();
	P.depth = run.depth;
	if (npcs->n >= BN6_NPC_COUNT || !npc_need_sprite(npcs, 6, PUPPET_MEGAMAN)) return;
	/* (a full crew's MegaMen first, then the week's traces, a slot or two
	 * where the layer leaves them: dive_traces.h; then the others: a layer
	 * with its guardian had given the traces its last slots, and a crew
	 * of four went half undrawn, issue #755) */
	make_puppets(npcs, CREW_OTHERS);
	int traces = dive_traces_more(npcs, BN6_NPC_COUNT - npcs->n);
	make_puppets(npcs, DIVE_PEOPLE);
	fprintf(stderr, "dive: room on this layer for %d other divers, %d traces' objects\n", P.made, traces);
}

static bool here(void) { return director_on_layer() && P.seed == run.layer_seed && P.depth == run.depth; }

void dive_people_on(bool on) {
	layer_objs_more = on ? more : NULL;
	if (!on) memset(&P, 0, sizeof P);
	/* (and their dots on the layer's map, issue #728) */
	dive_dots_on(on);
}

static void place(int k, const OnlinePeer *peer) {
	uint32_t at = P.script[k];
	puppet_show(at, peer != NULL);
	if (!peer) return;
	if (P.x[k] != peer->x || P.y[k] != peer->y) P.moving[k] = MOVING_FRAMES;
	else if (P.moving[k]) --P.moving[k];
	puppet_anim(at, peer->facing + (P.moving[k] ? PUPPET_WALK : 0));
	puppet_place(at, peer->x, peer->y, peer->z);
	P.x[k] = peer->x;
	P.y[k] = peer->y;
}

/* A diver's name over his MegaMan (issue #712), as the Square names its
 * players (map_label.h). `more`: divers under it whose names found no row. */
typedef struct { int x, y, w, more; bool crew; const char *name; } Tag;

/* What a diver is to MegaMan's crew (issue #716): one of it, about to be
 * (within the radius it forms in, a place in it left; the server's second
 * not yet passed), or neither. */
enum { SIGN_NONE, SIGN_CREW, SIGN_FORMING };

static int sign_of(const Who *w) {
	if (dive_crew_has(w->id)) return SIGN_CREW;
	/* (about to be a crew only where one can take him: MegaMan's has a place, and his is not full, issue #758) */
	if (dive_crew_count() >= ONLINE_PARTY_MOST || dive_comings_full_has(w->id)) return SIGN_NONE;
	long dx = w->x - bn6_player_x(), dy = w->y - bn6_player_y();
	return dx * dx + dy * dy <= (long)DIVE_CREW_NEAR * DIVE_CREW_NEAR ? SIGN_FORMING : SIGN_NONE;
}

/* the tag at (x, y) that `t` would lie on (NULL none) */
static Tag *tag_under(Tag *tags, int n, const Tag *t) {
	for (int j = 0; j < n; ++j)
		if (abs(tags[j].y - t->y) < MAP_LABEL_ROW && abs(tags[j].x - t->x) < (tags[j].w + t->w) / 2 + 4) return &tags[j];
	return NULL;
}

/* The names that show now, as the divers stand: each over his
 * head, and where divers stand on one spot (a layer's arrival) each next
 * name a row over the last, those past the picture's top counted on the
 * topmost. Reads the game's camera; changes nothing. */
static int tags_now(Tag tags[DIVE_PEOPLE]) {
	int n = 0, camera[3];
	if (!here() || !director_on_map() || !map_label_camera(camera)) return 0;
	for (int i = 0; i < P.nwho; ++i) {
		const Who *w = &P.who[i];
		if (!w->name[0]) continue;
		Tag t = { .name = w->name, .w = pixfont_width(w->name, 1), .crew = sign_of(w) == SIGN_CREW };
		map_label_at(w->x, w->y, w->z, camera, &t.x, &t.y);
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
	for (int i = 0; i < P.nwho; ++i) {
		int px, py;
		map_label_floor(P.who[i].x, P.who[i].y, P.who[i].z, camera, &px, &py);
		if (px < 0 || px >= EMU_W || py < 0 || py >= EMU_H) continue;
		k_of[n] = i;
		sign[n++] = sign_of(&P.who[i]);
	}
	return n;
}

/* the ring under each crewmate, and a dotted one under a diver about to be */
static void rings_draw(bool chat) {
	int k_of[DIVE_PEOPLE], sign[DIVE_PEOPLE], camera[3];
	int n = signs_now(k_of, sign, camera);
	for (int i = 0; i < n; ++i) {
		if (sign[i] == SIGN_NONE) continue;
		const Who *w = &P.who[k_of[i]];
		FloorRing how = { .colour = CREW_COLOUR, .dotted = sign[i] == SIGN_FORMING, .around = true, .bottom = chat ? TAG_BOX : EMU_H };
		floor_ring(w->x, w->y, w->z, RING, camera, &how);
	}
}

/* MegaMan's own marked for a moment at an arrival, where another diver stands on his spot (every diver arrives on
 * one): the PET's cursor at his head (issue #758) */
static bool arrival_marked(void) {
	if (!here() || !P.arrived || SDL_GetTicks() - P.arrived > ARRIVAL_MS) return false;
	for (int i = 0; i < P.nwho; ++i) {
		long dx = P.who[i].x - bn6_player_x(), dy = P.who[i].y - bn6_player_y();
		if (dx * dx + dy * dy <= (long)STACKED * STACKED) return true;
	}
	return false;
}

void dive_people_draw(bool chat) {
	rings_draw(chat);
	int camera[3], x, y;
	if (!chat && arrival_marked() && director_on_map() && map_label_camera(camera)) {
		map_label_at(bn6_player_x(), bn6_player_y(), bn6_player_z(), camera, &x, &y);
		map_label_cursor(x, y);
	}
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
	fprintf(f, "arrival %s\n", arrival_marked() ? "marked" : "none");
	fprintf(f, "rings %d", shown);
	for (int i = 0, j = 0; i < n2; ++i)
		if (sign[i] != SIGN_NONE) fprintf(f, "%s%s%s", j++ ? " " : ": ", P.who[k_of[i]].name, sign[i] == SIGN_FORMING ? "(forming)" : "");
	fputc('\n', f);
	/* (the divers in the room with no MegaMan drawn: the layer's slots were taken) */
	fprintf(f, "undrawn %d", here() ? P.undrawn : 0);
	for (int i = 0, j = 0; here() && i < P.nwho; ++i)
		if (!P.who[i].drawn) fprintf(f, "%s%s%s", j++ ? " " : ": ", P.who[i].name, dive_crew_has(P.who[i].id) ? "*" : "");
	fputc('\n', f);
	dive_dots_describe(f);
}

void dive_people_frame(const OnlineSnapshot *s) {
	dive_dots_frame(s);
	/* (only on the layer they were made on: another map took the slots' memory since) */
	if (!here()) return;
	int n = s->connected ? (int)(s->peer_count < DIVE_PEOPLE ? s->peer_count : DIVE_PEOPLE) : 0;
	SeatDiver d[DIVE_PEOPLE];
	for (int i = 0; i < n; ++i) d[i] = (SeatDiver){ s->peers[i].id, (int)s->peers[i].x, (int)s->peers[i].y, dive_crew_has(s->peers[i].id) };
	/* (the crew first, then the nearest: dive_seats.h) */
	P.undrawn = dive_seats(P.id, P.made, d, n, bn6_player_x(), bn6_player_y());
	P.nwho = n;
	for (int i = 0; i < n; ++i) {
		const OnlinePeer *p = &s->peers[i];
		Who *w = &P.who[i];
		*w = (Who){ .id = p->id, .x = (int)p->x, .y = (int)p->y, .z = (int)p->z };
		memcpy(w->name, p->name, sizeof w->name);
		for (int k = 0; k < P.made; ++k) w->drawn |= P.id[k] == p->id;
	}
	claims_begin(WRITER);
	for (int k = 0; k < P.made; ++k) {
		const OnlinePeer *peer = NULL;
		for (int i = 0; i < n && P.id[k]; ++i)
			if (s->peers[i].id == P.id[k]) peer = &s->peers[i];
		place(k, peer);
	}
	claims_end();
}
