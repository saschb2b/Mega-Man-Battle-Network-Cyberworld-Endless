/* xchips_moved.c: the chips whose code moved from BN5's (src/emu/xchips_bn5.c, issue #82's C3), written by
 * tools/xchiprecipe.py from the two ROMs and the registry (src/core/xchips_registry.c): each its id in BN6's tables and the
 * family and subfamily its record takes in BN6's engine (a moved attack family's number, BN5's subfamily under it; or the
 * support or Navi family with the moved routine's entry). Do not edit: write it again. Numbers alone. */
#include "xchips_moved.h"

#include <stddef.h>

const XChipMoved xchips_moved[] = {
	{ 208, 0x9F, 0x00 },   /* DrilArm1 */
	{ 209, 0x9F, 0x00 },   /* DrilArm2 */
	{ 210, 0x9F, 0x00 },   /* DrilArm3 */
	{ 291, 0x15, 0x3D },   /* Jealousy */
	{ 294, 0x84, 0x00 },   /* Z Saver */
	{ 298, 0x6B, 0x00 },   /* CrakOut */
	{ 299, 0x6B, 0x01 },   /* DublCrak */
	{ 300, 0x6B, 0x02 },   /* TripCrak */
	{ 411, 0x9E, 0x00 },   /* MrkCan1 */
	{ 412, 0x9E, 0x01 },   /* MrkCan2 */
	{ 413, 0x9E, 0x02 },   /* MrkCan3 */
	{ 414, 0xA2, 0x00 },   /* Pulsar1 */
	{ 415, 0xA2, 0x00 },   /* Pulsar2 */
	{ 416, 0xA2, 0x00 },   /* Pulsar3 */
	{ 417, 0xA1, 0x00 },   /* SpShake1 */
	{ 418, 0xA1, 0x00 },   /* SpShake2 */
	{ 419, 0xA1, 0x00 },   /* SpShake3 */
	{ 435, 0xA4, 0x00 },   /* Katana1 */
	{ 436, 0xA4, 0x00 },   /* Katana2 */
	{ 437, 0xA4, 0x00 },   /* Katana3 */
	{ 441, 0xA5, 0x00 },   /* Skully1 */
	{ 442, 0xA5, 0x00 },   /* Skully2 */
	{ 443, 0xA5, 0x00 },   /* Skully3 */
	{ 450, 0x15, 0x30 },   /* MetaGel */
	{ 454, 0x15, 0x67 },   /* RainyDay */
	{ 456, 0x15, 0x32 },   /* Blinder */
	{ 457, 0x15, 0x3F },   /* NrthWind */
	{ 462, 0xA8, 0x00 },   /* HotBody1 */
	{ 463, 0xA8, 0x00 },   /* HotBody2 */
	{ 464, 0xA8, 0x00 },   /* HotBody3 */
	{ 465, 0x97, 0x00 },   /* AqWhirl1 */
	{ 466, 0x97, 0x00 },   /* AqWhirl2 */
	{ 467, 0x97, 0x00 },   /* AqWhirl3 */
	{ 468, 0x6C, 0x00 },   /* SideBub1 */
	{ 469, 0x6C, 0x00 },   /* SideBub2 */
	{ 470, 0x6C, 0x00 },   /* SideBub3 */
	{ 471, 0x73, 0x00 },   /* ElcReel1 */
	{ 472, 0x73, 0x00 },   /* ElcReel2 */
	{ 473, 0x73, 0x00 },   /* ElcReel3 */
	{ 483, 0x98, 0x00 },   /* CactBal1 */
	{ 484, 0x98, 0x00 },   /* CactBal2 */
	{ 485, 0x98, 0x00 },   /* CactBal3 */
	{ 486, 0x78, 0x00 },   /* WoodNos1 */
	{ 487, 0x78, 0x00 },   /* WoodNos2 */
	{ 488, 0x78, 0x00 },   /* WoodNos3 */
};
const int xchips_moved_count = (int)(sizeof xchips_moved / sizeof *xchips_moved);

const XChipMoved *xchips_moved_of(int bn6) {
	for (int i = 0; i < xchips_moved_count; ++i)
		if (xchips_moved[i].bn6 == bn6) return &xchips_moved[i];
	return NULL;
}
