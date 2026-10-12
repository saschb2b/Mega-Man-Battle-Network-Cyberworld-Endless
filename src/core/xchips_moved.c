/* xchips_moved.c: the chips whose code moved from BN5's (src/emu/xchips_bn5.c, issue #82's C3), written by
 * tools/xchiprecipe.py from the two ROMs and the registry (src/core/xchips_registry.c): each its id in BN6's tables and the
 * family and subfamily its record takes in BN6's engine (a moved attack family's number, BN5's subfamily under it; or the
 * support or Navi family with the moved routine's entry); and the computed powers whose routines moved (BN5's table's
 * entry, BN6's grown table's). Do not edit: write it again. Numbers alone. */
#include "xchips_moved.h"

#include <stddef.h>

const XChipMoved xchips_moved[] = {
	{ 208, 0x9F, 0x00 },   /* DrilArm1 */
	{ 209, 0x9F, 0x00 },   /* DrilArm2 */
	{ 210, 0x9F, 0x00 },   /* DrilArm3 */
	{ 214, 0x15, 0x54 },   /* DarkCirc */
	{ 216, 0x75, 0x02 },   /* DarkWide */
	{ 217, 0x6A, 0x11 },   /* DarkMetr */
	{ 220, 0x15, 0x45 },   /* DrkSonic */
	{ 291, 0x15, 0x3D },   /* Jealousy */
	{ 294, 0x84, 0x00 },   /* Z Saver */
	{ 295, 0x6F, 0x00 },   /* Guard1 */
	{ 296, 0x6F, 0x01 },   /* Guard2 */
	{ 297, 0x6F, 0x02 },   /* Guard3 */
	{ 298, 0x6B, 0x00 },   /* CrakOut */
	{ 299, 0x6B, 0x01 },   /* DublCrak */
	{ 300, 0x6B, 0x02 },   /* TripCrak */
	{ 350, 0x69, 0x04 },   /* InfVulc1 */
	{ 351, 0x69, 0x05 },   /* InfVulc2 */
	{ 352, 0x69, 0x06 },   /* InfVulc3 */
	{ 353, 0x6A, 0x15 },   /* Boxer1 */
	{ 354, 0x6A, 0x15 },   /* Boxer2 */
	{ 355, 0x6A, 0x15 },   /* Boxer3 */
	{ 356, 0x75, 0x01 },   /* SuprSpr1 */
	{ 357, 0x75, 0x01 },   /* SuprSpr2 */
	{ 358, 0x75, 0x01 },   /* SuprSpr3 */
	{ 359, 0x6A, 0x16 },   /* ShakPar1 */
	{ 360, 0x6A, 0x16 },   /* ShakPar2 */
	{ 361, 0x6A, 0x16 },   /* ShakPar3 */
	{ 362, 0x6A, 0x17 },   /* CacDanc1 */
	{ 363, 0x6A, 0x17 },   /* CacDanc2 */
	{ 364, 0x6A, 0x17 },   /* CacDanc3 */
	{ 411, 0x9E, 0x00 },   /* MrkCan1 */
	{ 412, 0x9E, 0x01 },   /* MrkCan2 */
	{ 413, 0x9E, 0x02 },   /* MrkCan3 */
	{ 414, 0xA2, 0x00 },   /* Pulsar1 */
	{ 415, 0xA2, 0x00 },   /* Pulsar2 */
	{ 416, 0xA2, 0x00 },   /* Pulsar3 */
	{ 417, 0xA1, 0x00 },   /* SpShake1 */
	{ 418, 0xA1, 0x00 },   /* SpShake2 */
	{ 419, 0xA1, 0x00 },   /* SpShake3 */
	{ 420, 0x62, 0x0B },   /* Quake1 */
	{ 421, 0x62, 0x0B },   /* Quake2 */
	{ 422, 0x62, 0x0B },   /* Quake3 */
	{ 423, 0x62, 0x0A },   /* CrakBom */
	{ 424, 0x62, 0x0A },   /* ParaBom */
	{ 425, 0x62, 0x0A },   /* ResetBom */
	{ 426, 0x62, 0x0E },   /* LavaSeed */
	{ 427, 0x62, 0x04 },   /* CannBall */
	{ 428, 0x62, 0x05 },   /* Geyser */
	{ 429, 0x62, 0x0C },   /* SeaSeed */
	{ 430, 0x63, 0x07 },   /* CustSwrd */
	{ 431, 0x79, 0x00 },   /* Slasher */
	{ 432, 0xA3, 0x00 },   /* MoonBld1 */
	{ 433, 0xA3, 0x00 },   /* MoonBld2 */
	{ 434, 0xA3, 0x00 },   /* MoonBld3 */
	{ 435, 0xA4, 0x00 },   /* Katana1 */
	{ 436, 0xA4, 0x00 },   /* Katana2 */
	{ 437, 0xA4, 0x00 },   /* Katana3 */
	{ 438, 0x6A, 0x0F },   /* RedFrut1 */
	{ 439, 0x6A, 0x0F },   /* RedFrut2 */
	{ 440, 0x6A, 0x0F },   /* RedFrut3 */
	{ 441, 0xA5, 0x00 },   /* Skully1 */
	{ 442, 0xA5, 0x00 },   /* Skully2 */
	{ 443, 0xA5, 0x00 },   /* Skully3 */
	{ 444, 0x6A, 0x10 },   /* Voltz1 */
	{ 445, 0x6A, 0x10 },   /* Voltz2 */
	{ 446, 0x6A, 0x10 },   /* Voltz3 */
	{ 447, 0x15, 0x64 },   /* BoyBomb1 */
	{ 448, 0x15, 0x64 },   /* BoyBomb2 */
	{ 449, 0x15, 0x64 },   /* BoyBomb3 */
	{ 450, 0x15, 0x30 },   /* MetaGel */
	{ 451, 0x6A, 0x11 },   /* Astroid1 */
	{ 452, 0x6A, 0x11 },   /* Astroid2 */
	{ 453, 0x6A, 0x11 },   /* Astroid3 */
	{ 454, 0x15, 0x67 },   /* RainyDay */
	{ 455, 0xA6, 0x00 },   /* ElemRage */
	{ 456, 0x15, 0x32 },   /* Blinder */
	{ 457, 0x15, 0x3F },   /* NrthWind */
	{ 458, 0x15, 0x36 },   /* AntiFire */
	{ 459, 0x15, 0x36 },   /* AntiWatr */
	{ 460, 0x15, 0x36 },   /* AntiElec */
	{ 461, 0x15, 0x36 },   /* AntiWood */
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
	{ 474, 0x77, 0x00 },   /* CusVolt1 */
	{ 475, 0x77, 0x00 },   /* CusVolt2 */
	{ 476, 0x77, 0x00 },   /* CusVolt3 */
	{ 477, 0x6F, 0x05 },   /* CrsShld1 */
	{ 478, 0x6F, 0x06 },   /* CrsShld2 */
	{ 479, 0x6F, 0x07 },   /* CrsShld3 */
	{ 480, 0x6A, 0x13 },   /* WavePit */
	{ 481, 0x6A, 0x13 },   /* RedWave */
	{ 482, 0x6A, 0x13 },   /* MudWave */
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

const XChipPower xchips_powers[] = {
	{ 8, 0x88 },   /* PowerSP: BN5 0800E8D6 */
	{ 9, 0x89 },   /* PowerSP: BN5 0800E8D6 */
	{ 10, 0x8A },   /* PowerSP: BN5 0800E8D6 */
	{ 11, 0x8B },   /* PowerSP: BN5 0800E8D6 */
	{ 12, 0x8C },   /* PowerSP: BN5 0800E8D6 */
	{ 13, 0x8D },   /* PowerSP: BN5 0800E8D6 */
	{ 30, 0x9E },   /* PowerDS: BN5 0800E9A8 */
	{ 31, 0x9F },   /* PowerDS: BN5 0800E9A8 */
	{ 32, 0xA0 },   /* PowerDS: BN5 0800E9A8 */
	{ 33, 0xA1 },   /* PowerDS: BN5 0800E9A8 */
	{ 34, 0xA2 },   /* PowerDS: BN5 0800E9A8 */
	{ 35, 0xA3 },   /* PowerDS: BN5 0800E9A8 */
	{ 45, 0xAD },   /* PowerCustSwrd: BN5 0800EA18 */
	{ 73, 0xC9 },   /* PowerCusVolt: BN5 0800EB06 */
	{ 74, 0xCA },   /* PowerCusVolt: BN5 0800EB0C */
	{ 75, 0xCB },   /* PowerCusVolt: BN5 0800EB12 */
};
const int xchips_powers_count = (int)(sizeof xchips_powers / sizeof *xchips_powers);

int xchips_power_of(int bn5_power) {
	for (int i = 0; i < xchips_powers_count; ++i)
		if (1000 + xchips_powers[i].bn5 == bn5_power) return 1000 + xchips_powers[i].bn6;
	return 0;
}
