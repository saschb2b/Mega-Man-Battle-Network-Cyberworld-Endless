/* dive_looks.h. A layer's room for looks is not one number, as a part of
 * the Square's is: each layer has its own Mr.Progs, Mystery Data, traces
 * and bystanders. So it is read off BN6's renderer each frame the field
 * shows: the OBJ palette slots it gave (BN6_OBJ_PAL_COUNT), less those
 * holding a diver's colours, are the layer's own; the mode's limit less
 * those is the room the divers' looks share with MegaMan's own
 * (docs/rom/megaman-colours.md, The OBJ palette slots). The least seen
 * since the last pick is the next pick's, so a sprite that comes and goes
 * (an emote, a blinking find) is counted while it shows; one BN6 still
 * draws stale takes a look off for ten seconds, as in the Square. */
#include "dive_looks.h"

#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "dive_people.h"
#include "emu.h"
#include "look_apply.h"
#include "look_sprite.h"
#include "square_looks.h"
#include "square_looks_pick.h"

#define STALE_FRAMES 600        /* how long the room is held smaller after a diver drew in stale colours */
#define CREW_FIRST (1 << 20)    /* world units a diver out of the crew counts beyond his distance: the crew picked first */
#define SLOTS 16

_Static_assert(DIVE_PEOPLE <= LOOK_SPRITE_PUPPETS, "a palette a diver's puppet (look_sprite.h)");

static struct {
	LookSprite sprite;
	bool made;                   /* the copy stands in this layer's room */
	bool shown[DIVE_PEOPLE];     /* drawn in his own look */
	uint32_t ids[DIVE_PEOPLE];   /* the divers the pick was made for, by puppet */
	Look own;                    /* MegaMan's own, as the pick saw it */
	int held;                    /* frames the pick has held */
	int room, room_least;        /* the room the pick was made with (-1 none measured), and the least seen since */
	int cap_down, cap_down_for;  /* looks taken off the room where a diver showed stale colours, and how long */
	int stale;
} D;

void dive_looks_reset(void) {
	memset(&D, 0, sizeof D);
	D.room = D.room_least = -1;
}

int dive_looks_install(int puppets) {
	dive_looks_reset();
	int sprite = look_sprite_install(&D.sprite, puppets, "dive");
	D.made = D.sprite.block != 0;
	return sprite;
}

/* the slots BN6 gave last frame holding none of the divers' colours (MegaMan's own look aside, which is his) */
static int layer_slots(const uint16_t own[16]) {
	uint32_t given = emu_read32(BN6_OBJ_PAL_COUNT);
	if (given > SLOTS) given = SLOTS;
	int ours = 0;
	for (uint32_t j = 0; j < given; ++j) {
		uint16_t slot[16];
		for (int i = 0; i < 16; ++i) slot[i] = emu_read16(BN6_OBJ_PAL_WORK + j * 32 + (uint32_t)i * 2);
		if (!memcmp(slot, own, sizeof slot)) continue;
		for (int k = 0; k < D.sprite.puppets; ++k)
			if (!memcmp(slot, D.sprite.colours[1 + k], sizeof slot)) { ++ours; break; }
	}
	return (int)given - ours;
}

/* this frame's room, where the field shows as itself (no card or fade, whose shades change every slot's colours) */
static void measure(const Look *own) {
	int cover = director_cover();
	if (cover != COVER_NONE && cover != COVER_CHAT) return;
	uint16_t mine[16];
	if (!look_palette_of(own, LOOK_SPRITE_FIELD, mine)) memcpy(mine, D.sprite.colours[0], sizeof mine);
	int room = (int)emu_read8(BN6_OBJ_PAL_LIMIT) - layer_slots(mine);
	if (room < 0) room = 0;
	if (D.room_least < 0 || room < D.room_least) D.room_least = room;
}

static void pick(const DiveLookPuppet *p, int n, const Look *own) {
	LookPeer peers[DIVE_PEOPLE];
	int k_of[DIVE_PEOPLE], m = 0;
	for (int k = 0; k < n; ++k) {
		if (!p[k].id) continue;
		bool held = D.ids[k] == p[k].id && D.shown[k];
		peers[m] = (LookPeer){ p[k].look, p[k].distance + (p[k].crew ? 0 : CREW_FIRST), held };
		k_of[m++] = k;
	}
	/* (none measured since the last pick, a card or a fade over it all along: the last measure's) */
	if (D.room_least >= 0) D.room = D.room_least;
	int cap = D.room - D.cap_down;
	bool shown[DIVE_PEOPLE] = { false };
	square_looks_pick(peers, m, own, cap < 0 ? 0 : cap, SQUARE_LOOKS_KEEP, shown);
	memset(D.shown, 0, sizeof D.shown);
	for (int i = 0; i < m; ++i) D.shown[k_of[i]] = shown[i];
	for (int k = 0; k < DIVE_PEOPLE; ++k) D.ids[k] = k < n ? p[k].id : 0;
	D.own = *own;
	D.held = 0;
	D.room_least = -1;
}

void dive_looks_frame(const DiveLookPuppet *p, int n) {
	if (!D.made) return;
	if (n > D.sprite.puppets) n = D.sprite.puppets;
	Look own = looks_worn();
	measure(&own);
	/* (made again as the first measure comes, MegaMan's look changes, a diver comes or goes, and every 90 frames) */
	bool again = (D.room < 0 && D.room_least >= 0) || !square_looks_same(&own, &D.own) || ++D.held >= SQUARE_LOOKS_HOLD;
	for (int k = 0; k < DIVE_PEOPLE; ++k) again |= D.ids[k] != (k < n ? p[k].id : 0u);
	uint32_t objects[DIVE_PEOPLE] = { 0 };
	D.stale = 0;
	for (int k = 0; k < n; ++k) D.stale += p[k].id && (objects[k] = look_sprite_object(p[k].script)) && look_sprite_stale(objects[k]);
	/* (BN6 ran out of slots after all, a sprite more than measured: one look fewer a while) */
	if (D.stale && D.room >= 0 && D.cap_down < D.room) {
		++D.cap_down;
		D.cap_down_for = STALE_FRAMES;
		again = true;
		fprintf(stderr, "dive looks: a diver drawn in stale colours; %d looks fewer a while\n", D.cap_down);
	} else if (D.cap_down && --D.cap_down_for <= 0) {
		--D.cap_down;
		D.cap_down_for = STALE_FRAMES;
	}
	/* (until the layer has shown once, no room measured: every diver in MegaMan's look or BN6's own) */
	if (again) pick(p, n, &own);
	for (int k = 0; k < n; ++k) {
		if (!p[k].id) continue;
		Look l = D.shown[k] ? p[k].look : look_plain();
		look_sprite_colour(&D.sprite, k, &l);
		look_sprite_paint(objects[k] ? objects[k] : look_sprite_object(p[k].script), k);
	}
}

int dive_looks_shown(int k) {
	if (!D.made || k < 0 || k >= DIVE_PEOPLE || !D.ids[k]) return -1;
	return D.shown[k];
}

int dive_looks_room(void) { return D.made && D.room >= 0 ? D.room - D.cap_down : -1; }

int dive_looks_stale(void) { return D.stale; }
