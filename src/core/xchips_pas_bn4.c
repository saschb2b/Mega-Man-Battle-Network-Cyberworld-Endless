/* xchips_pas_bn4.c: BN4 Red Sun's Program Advances of the chip registry (XREG_PA, xchips_registry_bn4.c) whose code is
 * moved (tools/xreg_bn4.py PAS), their recipes as BN4's list has them (BN4_PA_LIST), in BN6's chip ids and BN6's kinds
 * (BN4's listed kind BN6's 4), every part a chip that plays; written by tools/xreg_bn4.py from BN4's ROM and its rows, for
 * BN6's list grown with them after BN5's (src/emu/xchips_code.c). Do not edit: write it again. Numbers alone. */
#include "xchips_pas.h"

const XChipPa xchips_pas4[] = {
	{ 0x080211D8, 3, 4, 606, { 588, 589, 590, 0 } },   /* HeatSprd: HeatShot + Heat-V + HeatSide */
	{ 0x08021200, 3, 0, 606, { 588, 0, 0, 0 } },   /* HeatSprd: 3 HeatShot of codes in a row */
	{ 0x08021206, 3, 0, 607, { 591, 0, 0, 0 } },   /* BubSprd: 3 Bubbler of codes in a row */
	{ 0x0802120C, 3, 0, 606, { 589, 0, 0, 0 } },   /* HeatSprd: 3 Heat-V of codes in a row */
	{ 0x08021212, 3, 0, 607, { 592, 0, 0, 0 } },   /* BubSprd: 3 Bub-V of codes in a row */
	{ 0x08021218, 3, 0, 606, { 590, 0, 0, 0 } },   /* HeatSprd: 3 HeatSide of codes in a row */
	{ 0x0802121E, 3, 0, 607, { 593, 0, 0, 0 } },   /* BubSprd: 3 BublSide of codes in a row */
	{ 0x08021248, 3, 0, 608, { 595, 0, 0, 0 } },   /* FlmCros1: 3 FlmLine1 of codes in a row */
	{ 0x0802124E, 3, 0, 609, { 596, 0, 0, 0 } },   /* FlmCros2: 3 FlmLine2 of codes in a row */
	{ 0x08021254, 3, 0, 610, { 597, 0, 0, 0 } },   /* FlmCros3: 3 FlmLine3 of codes in a row */
	{ 0x0802125A, 3, 0, 611, { 584, 0, 0, 0 } },   /* PitRng1: 3 Boomer1 of codes in a row */
	{ 0x08021260, 3, 0, 612, { 585, 0, 0, 0 } },   /* PitRng2: 3 Boomer2 of codes in a row */
	{ 0x08021266, 3, 0, 613, { 586, 0, 0, 0 } },   /* PitRng3: 3 Boomer3 of codes in a row */
	{ 0x0802126C, 3, 0, 614, { 656, 0, 0, 0 } },   /* BstFang1: 3 TwnFng1 of codes in a row */
	{ 0x08021272, 3, 0, 615, { 657, 0, 0, 0 } },   /* BstFang2: 3 TwnFng2 of codes in a row */
	{ 0x08021278, 3, 0, 616, { 658, 0, 0, 0 } },   /* BstFang3: 3 TwnFng3 of codes in a row */
	{ 0x0802127E, 3, 0, 617, { 650, 0, 0, 0 } },   /* PitHoky1: 3 AirHoc1 of codes in a row */
	{ 0x08021284, 3, 0, 618, { 651, 0, 0, 0 } },   /* PitHoky2: 3 AirHoc2 of codes in a row */
	{ 0x0802128A, 3, 0, 619, { 652, 0, 0, 0 } },   /* PitHoky3: 3 AirHoc3 of codes in a row */
	{ 0x08021290, 3, 0, 620, { 642, 0, 0, 0 } },   /* MagShok1: 3 MagBolt1 of codes in a row */
	{ 0x08021296, 3, 0, 621, { 643, 0, 0, 0 } },   /* MagShok2: 3 MagBolt2 of codes in a row */
	{ 0x0802129C, 3, 0, 622, { 644, 0, 0, 0 } },   /* MagShok3: 3 MagBolt3 of codes in a row */
};
const int xchips_pas4_count = (int)(sizeof xchips_pas4 / sizeof *xchips_pas4);
