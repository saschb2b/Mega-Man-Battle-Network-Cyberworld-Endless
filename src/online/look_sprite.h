/* Other players' MegaMen in their own looks (issue #854; the Net Square's
 * peers, docs/online/square/looks.md, and the shared dive's divers, issue
 * #878): a copy of BN6's field MegaMan (list 6's 0x37) in the map's room,
 * listed at list 6's BN6_LOOK_FIELD_SPARE, whose frames name a palette
 * block of the copy's own: palette 0 BN6's own colours, then one a puppet.
 * A puppet's NPC object is set to its palette each frame (BN6 sets it into
 * the sprite), and the palette given the colours of the look it is to show;
 * BN6 copies a sprite's palette into an OBJ slot by content, so a look
 * shows the next frame and two puppets in one look share a slot
 * (docs/rom/megaman-colours.md, The OBJ palette slots). */
#ifndef CW_LOOK_SPRITE_H
#define CW_LOOK_SPRITE_H

#include <stdbool.h>
#include <stdint.h>

#include "looks.h"

#define LOOK_SPRITE_PUPPETS 7   /* puppets a copy colours at most (a room's eight, MegaMan among them) */

typedef struct {
	uint32_t block;                                       /* the copy's palettes: the first's address (0 none) */
	int puppets;                                          /* the palettes past BN6's own */
	uint16_t colours[1 + LOOK_SPRITE_PUPPETS][16];        /* as written */
} LookSprite;

/* The copy made in the map's room (mapslot_alloc, inside the caller's
 * claims) with `puppets` palettes past BN6's own: the list-6 number a
 * puppet takes, PUPPET_MEGAMAN where BN6's MegaMan is not as measured or
 * the map had no room (s->block 0 then). `who` names the caller in the log. */
int look_sprite_install(LookSprite *s, int puppets, const char *who);

/* Puppet `k`'s palette (0 first) given look `l`'s field colours, or BN6's
 * own where `l` is NULL or has none (inside the caller's claims). */
void look_sprite_colour(LookSprite *s, int k, const Look *l);

/* The NPC object a puppet's script runs (0 none), and whether BN6 drew its
 * sprite past the slots it had, in a stale slot's colours. */
uint32_t look_sprite_object(uint32_t script);
bool look_sprite_stale(uint32_t object);

/* Object `object` (look_sprite_object) drawn in puppet `k`'s palette. */
void look_sprite_paint(uint32_t object, int k);

/* A peer's look as its wire bytes say it: BN6's own where they are no look. */
Look look_sprite_wire(const uint8_t bytes[2 * LOOK_CHANNELS]);

#endif
