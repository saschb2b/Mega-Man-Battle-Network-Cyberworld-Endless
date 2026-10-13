/* A sprite by its bytes, for the differential (issue #82; docs/DEVTOOLS.md,
 * The differential): the two games number their sprites otherwise (a moved
 * attack's copy in BN6's grown lists, a twin at another number of BN6's),
 * so an object's sprite is compared by what it draws, never by its number.
 * Read in a hook on sprite_load (xdiff_events.c), from the core it runs on. */
#ifndef CW_XDIFF_SPRITE_H
#define CW_XDIFF_SPRITE_H

#include <stdint.h>

/* The CRC32 of the sprite game `game` (5 the guest's core, 6 BN6's) loads
 * by its list's offset and number (sprite_load's r1 and r2), decompressed
 * as BN6 draws it (its size word and its data, xnavi_sprite_bytes'); 0
 * where it cannot be read. Its lists as the core has them (BN6's grown). */
uint32_t xd_sprite_crc(int game, uint32_t list_offset, uint32_t number);

#endif
