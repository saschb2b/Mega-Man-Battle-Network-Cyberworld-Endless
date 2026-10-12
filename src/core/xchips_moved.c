/* xchips_moved.c: the chips whose code moved from BN5's (src/emu/xchips_bn5.c, issue #82's C3), written by
 * tools/xchiprecipe.py from the two ROMs and the registry (src/core/xchips_registry.c): each its id in BN6's tables and the
 * family and subfamily its record takes in BN6's engine (a moved attack family's number, BN5's subfamily under it; or the
 * support or Navi family with the moved routine's entry). Do not edit: write it again. Numbers alone. */
#include "xchips_moved.h"

#include <stddef.h>

const XChipMoved xchips_moved[] = {
	{ 291, 0x15, 0x3D },   /* Jealousy */
	{ 298, 0x6B, 0x00 },   /* CrakOut */
	{ 299, 0x6B, 0x01 },   /* DublCrak */
	{ 300, 0x6B, 0x02 },   /* TripCrak */
	{ 457, 0x15, 0x3F },   /* NrthWind */
	{ 462, 0xA8, 0x00 },   /* HotBody1 */
	{ 463, 0xA8, 0x00 },   /* HotBody2 */
	{ 464, 0xA8, 0x00 },   /* HotBody3 */
};
const int xchips_moved_count = (int)(sizeof xchips_moved / sizeof *xchips_moved);

const XChipMoved *xchips_moved_of(int bn6) {
	for (int i = 0; i < xchips_moved_count; ++i)
		if (xchips_moved[i].bn6 == bn6) return &xchips_moved[i];
	return NULL;
}
