/* xchips_pas.c: BN5's Program Advances of the chip registry (XREG_PA), their recipes as BN5's list has them (0x08028020),
 * in BN6's chip ids, written by tools/xchiprecipe.py from BN5's ROM and the registry (src/core/xchips_registry.c), for BN6's
 * list grown with them (src/emu/xchips_code.c). Do not edit: write it again. Numbers alone. */
#include "xchips_pas.h"

const XChipPa xchips_pas[] = {
	{ 0x08027E8C, 3, 0, 356, { 204, 0, 0, 0 } },   /* SuprSpr1: 3 WideSht1 of codes in a row */
	{ 0x08027E92, 3, 0, 357, { 205, 0, 0, 0 } },   /* SuprSpr2: 3 WideSht2 of codes in a row */
	{ 0x08027E98, 3, 0, 358, { 206, 0, 0, 0 } },   /* SuprSpr3: 3 WideSht3 of codes in a row */
	{ 0x08027E9E, 3, 0, 350, { 5, 0, 0, 0 } },   /* InfVulc1: 3 Vulcan1 of codes in a row */
	{ 0x08027EA4, 3, 0, 351, { 6, 0, 0, 0 } },   /* InfVulc2: 3 Vulcan2 of codes in a row; never formed: three Vulcan2 of codes in a row, BN6's Vulcan2 D, F and L (BN6's record), none three in a row */
	{ 0x08027EAA, 3, 0, 352, { 7, 0, 0, 0 } },   /* InfVulc3: 3 Vulcan3 of codes in a row; never formed: three Vulcan3, BN6's A, G and R, none three in a row */
	{ 0x08027EB0, 3, 0, 353, { 107, 0, 0, 0 } },   /* Boxer1: 3 FireHit1 of codes in a row; never formed: three FireHit1 in a row, BN6's own FlmHook1's recipe, first in BN6's list (0x0802BB0E), which wins */
	{ 0x08027EB6, 3, 0, 354, { 108, 0, 0, 0 } },   /* Boxer2: 3 FireHit2 of codes in a row; never formed: three FireHit2, BN6's FlmHook2's recipe, first */
	{ 0x08027EBC, 3, 0, 355, { 109, 0, 0, 0 } },   /* Boxer3: 3 FireHit3 of codes in a row; never formed: three FireHit3, BN6's FlmHook3's recipe, first */
	{ 0x08027EC2, 3, 0, 359, { 417, 0, 0, 0 } },   /* ShakPar1: 3 SpShake1 of codes in a row */
	{ 0x08027EC8, 3, 0, 360, { 418, 0, 0, 0 } },   /* ShakPar2: 3 SpShake2 of codes in a row */
	{ 0x08027ECE, 3, 0, 361, { 419, 0, 0, 0 } },   /* ShakPar3: 3 SpShake3 of codes in a row */
	{ 0x08027ED4, 3, 0, 362, { 483, 0, 0, 0 } },   /* CacDanc1: 3 CactBal1 of codes in a row */
	{ 0x08027EDA, 3, 0, 363, { 484, 0, 0, 0 } },   /* CacDanc2: 3 CactBal2 of codes in a row */
	{ 0x08027EE0, 3, 0, 364, { 485, 0, 0, 0 } },   /* CacDanc3: 3 CactBal3 of codes in a row */
};
const int xchips_pas_count = (int)(sizeof xchips_pas / sizeof *xchips_pas);
