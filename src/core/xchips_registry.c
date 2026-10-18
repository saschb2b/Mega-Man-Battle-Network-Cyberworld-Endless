/* The chip registry (xchips_registry.h): Team Colonel's chips a run can
 * meet (docs/MULTIROM.md, the finer calls: its standard chips, Megas and
 * DarkChips, Team Colonel's Gigas, the Navi chips of the six Navis who
 * guard runs, V1, SP and DS, and its Program Advances whose parts are
 * registered; not Team ProtoMan's version chips, the Darkloids' and
 * Django's Navi chips, BugCharg, Phoenix or DethPhnx), in BN5's order.
 *
 * Paired by name with BN6's chips (BN5's names in BN6's text codes, rom.h's
 * xrom_text_bn6): one chip per name, BN6's, but where BN6's of the name is
 * a pseudo-chip of its battles (Z Saver, 381), which no folder holds; and
 * the three BN6's US release named otherwise (Attck+10 and Attck+30, BN6's
 * Atk+10 and Atk+30, which BN6's hand merges into the chip before them by
 * their ids; PitHoky, PitHocky). The rest take ids of their own: 203-207
 * and 211-213 the eight BN6's code plays now (over AirHocky's, WideSht's,
 * Spreadr1's and Geddon's families: BN6's Geddon cracks, breaks or poisons
 * by its first parameter as BN5's three do), 208-210 DrilArm1-3 (BN5's own
 * drill, moved as Z Saver is), 214-220 the seven DarkChips,
 * 291-294 the Megas, 295-300 the first chips to be moved (Guard1-3,
 * CrakOut, DublCrak, TripCrak), 314 HolyDrem beside BN6's Gigas, 350-364
 * the Program Advances beside BN6's, 411 on the rest, the Navi chips last.
 * Generated once from both ROMs, paired by their names (the ROMs are told
 * by their SHA-1s, so the pairs hold for every player's); xreg_check
 * holds its rules in the unit tests, which have no ROM to read names in. */
#include "xchips_registry.h"

#include <stdio.h>
#include <string.h>

#include "bn6_chips.h"

const XChipReg xreg_rows[] = {
	{   1,   1, XREG_SAME },   /* Cannon */
	{   2,   2, XREG_SAME },   /* HiCannon */
	{   3,   3, XREG_SAME },   /* M-Cannon */
	{   4,   4, XREG_SAME },   /* AirShot */
	{   5, 203, XREG_RECORD,  50, 0xFF, { 0x10, 0x11, 0x12, 0xFF }, 0, 40 },   /* AirHoc */
	{   6, 116, XREG_SAME },   /* Boomer */
	{   7, 149, XREG_SAME },   /* Silence */
	{   8,  52, XREG_SAME },   /* Tornado */
	{   9, 204, XREG_RECORD,  23, 0xFF, { 0x0B, 0x0C, 0x0D, 0xFF }, 0, 10 },   /* WideSht1 */
	{  10, 205, XREG_RECORD,  23, 0xFF, { 0x04, 0x05, 0x06, 0xFF }, 0, 30 },   /* WideSht2 */
	{  11, 206, XREG_RECORD,  23, 0xFF, { 0x12, 0x13, 0x14, 0xFF }, 0, 50 },   /* WideSht3 */
	{  12, 411, XREG_MOVED,     0, 0xFF, { 0x0C, 0x0A, 0x12, 0xFF }, 0, 18 },   /* MrkCan1 */
	{  13, 412, XREG_MOVED,     0, 0xFF, { 0x03, 0x05, 0x19, 0xFF }, 0, 27 },   /* MrkCan2 */
	{  14, 413, XREG_MOVED,     0, 0xFF, { 0x04, 0x11, 0x18, 0xFF }, 0, 36 },   /* MrkCan3 */
	{  15,   5, XREG_SAME },   /* Vulcan1 */
	{  16,   6, XREG_SAME },   /* Vulcan2 */
	{  17,   7, XREG_SAME },   /* Vulcan3 */
	{  18, 207, XREG_RECORD,   9, 0xFF, { 0x02, 0x03, 0x04, 0x1A }, 0, 10 },   /* Spreader */
	{  19,  30, XREG_SAME },   /* Thunder */
	{  20,  69, XREG_SAME },   /* IceSeed */
	{  21, 414, XREG_MOVED,     0, 0xFF, { 0x05, 0x0F, 0x13, 0xFF }, 0, 14 },   /* Pulsar1 */
	{  22, 415, XREG_MOVED,     0, 0xFF, { 0x04, 0x06, 0x0C, 0xFF }, 0, 26 },   /* Pulsar2 */
	{  23, 416, XREG_MOVED,     0, 0xFF, { 0x02, 0x11, 0x12, 0xFF }, 0, 38 },   /* Pulsar3 */
	{  24, 417, XREG_MOVED,     0, 0xFF, { 0x06, 0x07, 0x08, 0xFF }, 0, 30 },   /* SpShake1 */
	{  25, 418, XREG_MOVED,     0, 0xFF, { 0x01, 0x02, 0x03, 0xFF }, 0, 40 },   /* SpShake2 */
	{  26, 419, XREG_MOVED,     0, 0xFF, { 0x12, 0x13, 0x14, 0xFF }, 0, 50 },   /* SpShake3 */
	{  27,  53, XREG_SAME },   /* Static */
	{  28, 191, XREG_SAME },   /* LifeSync */
	{  29,  54, XREG_SAME },   /* MiniBomb */
	{  30,  55, XREG_SAME },   /* EnergBom */
	{  31,  56, XREG_SAME },   /* MegEnBom */
	{  32,  15, XREG_SAME },   /* GunDelS1 */
	{  33,  16, XREG_SAME },   /* GunDelS2 */
	{  34,  17, XREG_SAME },   /* GunDelS3 */
	{  35, 420, XREG_MOVED,     0, 0xFF, { 0x00, 0x0A, 0x0F, 0xFF }, 0, 10 },   /* Quake1 */
	{  36, 421, XREG_MOVED,     0, 0xFF, { 0x01, 0x0B, 0x18, 0xFF }, 0, 25 },   /* Quake2 */
	{  37, 422, XREG_MOVED,     0, 0xFF, { 0x05, 0x07, 0x0D, 0xFF }, 0, 40 },   /* Quake3 */
	{  38, 423, XREG_MOVED,     0, 0xFF, { 0x02, 0x11, 0x15, 0x1A }, 0, 18 },   /* CrakBom */
	{  39, 424, XREG_MOVED,     0, 0xFF, { 0x07, 0x0D, 0x0F, 0xFF }, 0, 34 },   /* ParaBom */
	{  40, 425, XREG_MOVED,     0, 0xFF, { 0x03, 0x08, 0x0E, 0xFF }, 0, 50 },   /* ResetBom */
	{  41,  67, XREG_SAME },   /* BugBomb */
	{  42,  68, XREG_SAME },   /* GrasSeed */
	{  43, 426, XREG_MOVED,     0, 0xFF, { 0x07, 0x0A, 0x0F, 0xFF }, 0, 60 },   /* LavaSeed */
	{  44, 427, XREG_MOVED,     0, 0xFF, { 0x02, 0x13, 0x19, 0x1A }, 0, 30 },   /* CannBall */
	{  45, 428, XREG_MOVED,     0, 0xFF, { 0x06, 0x0C, 0x12, 0xFF }, 0, 38 },   /* Geyser */
	{  46,  60, XREG_SAME },   /* BlkBomb */
	{  47, 429, XREG_MOVED,     0, 0xFF, { 0x03, 0x09, 0x0C, 0x1A }, 0, 28 },   /* SeaSeed */
	{  48,  71, XREG_SAME },   /* Sword */
	{  49,  72, XREG_SAME },   /* WideSwrd */
	{  50,  73, XREG_SAME },   /* LongSwrd */
	{  51,  74, XREG_SAME },   /* WideBlde */
	{  52,  75, XREG_SAME },   /* LongBlde */
	{  53,  80, XREG_SAME },   /* WindRack */
	{  54, 430, XREG_MOVED,     0, 0xFF, { 0x04, 0x0B, 0x11, 0xFF }, 0, 20 },   /* CustSwrd */
	{  55,  82, XREG_SAME },   /* VarSwrd */
	{  56, 431, XREG_MOVED,     0, 0xFF, { 0x01, 0x08, 0x11, 0xFF }, 0, 18 },   /* Slasher */
	{  57, 432, XREG_MOVED,     0, 0xFF, { 0x04, 0x0D, 0x19, 0xFF }, 0, 23 },   /* MoonBld1 */
	{  58, 433, XREG_MOVED,     0, 0xFF, { 0x06, 0x0E, 0x15, 0xFF }, 0, 36 },   /* MoonBld2 */
	{  59, 434, XREG_MOVED,     0, 0xFF, { 0x02, 0x0B, 0x13, 0xFF }, 0, 49 },   /* MoonBld3 */
	{  60, 435, XREG_MOVED,     0, 0xFF, { 0x03, 0x07, 0x12, 0xFF }, 0, 25 },   /* Katana1 */
	{  61, 436, XREG_MOVED,     0, 0xFF, { 0x01, 0x11, 0x16, 0xFF }, 0, 37 },   /* Katana2 */
	{  62, 437, XREG_MOVED,     0, 0xFF, { 0x00, 0x09, 0x19, 0xFF }, 0, 49 },   /* Katana3 */
	{  63,  12, XREG_SAME },   /* TankCan1 */
	{  64,  13, XREG_SAME },   /* TankCan2 */
	{  65,  14, XREG_SAME },   /* TankCan3 */
	{  66, 438, XREG_MOVED,     0, 0xFF, { 0x04, 0x0A, 0x19, 0xFF }, 0, 24 },   /* RedFrut1 */
	{  67, 439, XREG_MOVED,     0, 0xFF, { 0x05, 0x13, 0x16, 0xFF }, 0, 48 },   /* RedFrut2 */
	{  68, 440, XREG_MOVED,     0, 0xFF, { 0x07, 0x0E, 0x14, 0xFF }, 0, 62 },   /* RedFrut3 */
	{  69, 441, XREG_MOVED,     0, 0xFF, { 0x02, 0x0A, 0x12, 0xFF }, 0, 34 },   /* Skully1 */
	{  70, 442, XREG_MOVED,     0, 0xFF, { 0x00, 0x07, 0x15, 0xFF }, 0, 50 },   /* Skully2 */
	{  71, 443, XREG_MOVED,     0, 0xFF, { 0x06, 0x0D, 0x14, 0xFF }, 0, 76 },   /* Skully3 */
	{  72, 208, XREG_MOVED,     0, 0xFF, { 0x00, 0x09, 0x16, 0xFF }, 0, 38 },   /* DrilArm1 */
	{  73, 209, XREG_MOVED,     0, 0xFF, { 0x03, 0x0B, 0x12, 0xFF }, 0, 51 },   /* DrilArm2 */
	{  74, 210, XREG_MOVED,     0, 0xFF, { 0x08, 0x0F, 0x19, 0xFF }, 0, 64 },   /* DrilArm3 */
	{  75, 144, XREG_SAME },   /* TimeBom1 */
	{  76, 200, XREG_SAME },   /* TimeBom2 */
	{  77, 201, XREG_SAME },   /* TimeBom3 */
	{  78, 444, XREG_MOVED,     0, 0xFF, { 0x0A, 0x0E, 0x18, 0xFF }, 0, 30 },   /* Voltz1 */
	{  79, 445, XREG_MOVED,     0, 0xFF, { 0x09, 0x14, 0x19, 0xFF }, 0, 48 },   /* Voltz2 */
	{  80, 446, XREG_MOVED,     0, 0xFF, { 0x00, 0x07, 0x15, 0xFF }, 0, 66 },   /* Voltz3 */
	{  81, 119, XREG_SAME },   /* Lance */
	{  82,  19, XREG_SAME },   /* YoYo */
	{  83, 129, XREG_SAME },   /* Wind */
	{  84, 130, XREG_SAME },   /* Fan */
	{  85, 447, XREG_MOVED,     0, 0xFF, { 0x02, 0x05, 0x12, 0xFF }, 0, 30 },   /* BoyBomb1 */
	{  86, 448, XREG_MOVED,     0, 0xFF, { 0x07, 0x0F, 0x16, 0xFF }, 0, 40 },   /* BoyBomb2 */
	{  87, 449, XREG_MOVED,     0, 0xFF, { 0x01, 0x10, 0x18, 0xFF }, 0, 50 },   /* BoyBomb3 */
	{  88, 295, XREG_MOVED,     0, 0xFF, { 0x00, 0x03, 0x0A, 0x1A }, 0,  4 },   /* Guard1 */
	{  89, 296, XREG_MOVED,     0, 0xFF, { 0x01, 0x0B, 0x10, 0x1A }, 0, 10 },   /* Guard2 */
	{  90, 297, XREG_MOVED,     0, 0xFF, { 0x02, 0x0C, 0x12, 0x1A }, 0, 16 },   /* Guard3 */
	{  91, 298, XREG_MOVED,     0, 0xFF, { 0x1A, 0xFF, 0xFF, 0xFF }, 0,  4 },   /* CrakOut */
	{  92, 299, XREG_MOVED,     0, 0xFF, { 0x1A, 0xFF, 0xFF, 0xFF }, 0,  7 },   /* DublCrak */
	{  93, 300, XREG_MOVED,     0, 0xFF, { 0x1A, 0xFF, 0xFF, 0xFF }, 0, 10 },   /* TripCrak */
	{  94, 141, XREG_SAME },   /* Magnum */
	{  95, 450, XREG_MOVED,     0, 0xFF, { 0x05, 0x0D, 0x13, 0xFF }, 0, 52 },   /* MetaGel */
	{  96, 134, XREG_SAME },   /* Snake */
	{  97, 142, XREG_SAME },   /* CircGun */
	{  98, 145, XREG_SAME },   /* Mine */
	{  99, 143, XREG_SAME },   /* RockCube */
	{ 100, 146, XREG_SAME },   /* Fanfare */
	{ 101, 147, XREG_SAME },   /* Discord */
	{ 102, 148, XREG_SAME },   /* Timpani */
	{ 103, 150, XREG_SAME },   /* VDoll */
	{ 104, 451, XREG_MOVED,     0, 0xFF, { 0x00, 0x10, 0x13, 0xFF }, 0, 26 },   /* Astroid1 */
	{ 105, 452, XREG_MOVED,     0, 0xFF, { 0x04, 0x0C, 0x14, 0xFF }, 0, 42 },   /* Astroid2 */
	{ 106, 453, XREG_MOVED,     0, 0xFF, { 0x02, 0x0F, 0x18, 0xFF }, 0, 56 },   /* Astroid3 */
	{ 107, 154, XREG_SAME },   /* Recov10 */
	{ 108, 155, XREG_SAME },   /* Recov30 */
	{ 109, 156, XREG_SAME },   /* Recov50 */
	{ 110, 157, XREG_SAME },   /* Recov80 */
	{ 111, 158, XREG_SAME },   /* Recov120 */
	{ 112, 159, XREG_SAME },   /* Recov150 */
	{ 113, 160, XREG_SAME },   /* Recov200 */
	{ 114, 161, XREG_SAME },   /* Recov300 */
	{ 115, 175, XREG_SAME },   /* BusterUp */
	{ 116, 162, XREG_SAME },   /* PanlGrab */
	{ 117, 163, XREG_SAME },   /* AreaGrab */
	{ 118, 165, XREG_SAME },   /* GrabRvng */
	{ 119, 164, XREG_SAME },   /* GrabBnsh */
	{ 120, 172, XREG_SAME },   /* SloGauge */
	{ 121, 173, XREG_SAME },   /* FstGauge */
	{ 122, 166, XREG_SAME },   /* PnlRetrn */
	{ 123, 211, XREG_RECORD, 167, 0xFF, { 0x0C, 0x0F, 0x19, 0x1A }, 0, 34 },   /* Geddon1 */
	{ 124, 212, XREG_RECORD, 167, 0xFF, { 0x07, 0x13, 0x16, 0xFF }, 0, 47 },   /* Geddon2 */
	{ 125, 213, XREG_RECORD, 167, 0xFF, { 0x00, 0x0E, 0x15, 0xFF }, 0, 62 },   /* Geddon3 */
	{ 126, 454, XREG_MOVED,     0, 0xFF, { 0x01, 0x06, 0x0B, 0xFF }, 0, 47 },   /* RainyDay */
	{ 127, 194, XREG_SAME },   /* ColorPt */
	{ 128, 455, XREG_MOVED,     0, 0xFF, { 0x04, 0x0A, 0x16, 0xFF }, 0, 39 },   /* ElemRage */
	{ 129, 456, XREG_MOVED,     0, 0xFF, { 0x00, 0x03, 0x07, 0x1A }, 0,  9 },   /* Blinder */
	{ 130, 126, XREG_SAME },   /* AirSpin1 */
	{ 131, 127, XREG_SAME },   /* AirSpin2 */
	{ 132, 128, XREG_SAME },   /* AirSpin3 */
	{ 133, 177, XREG_SAME },   /* Invisibl */
	{ 134, 181, XREG_SAME },   /* BblWrap */
	{ 135, 178, XREG_SAME },   /* Barrier */
	{ 136, 179, XREG_SAME },   /* Barr100 */
	{ 137, 180, XREG_SAME },   /* Barr200 */
	{ 138, 457, XREG_MOVED,     0, 0xFF, { 0x03, 0x10, 0x15, 0x1A }, 0, 33 },   /* NrthWind */
	{ 139, 168, XREG_SAME },   /* HolyPanl */
	{ 140, 458, XREG_MOVED,     0, 0xFF, { 0x04, 0x0A, 0x13, 0x1A }, 0, 58 },   /* AntiFire */
	{ 141, 459, XREG_MOVED,     0, 0xFF, { 0x03, 0x06, 0x10, 0x1A }, 0, 54 },   /* AntiWatr */
	{ 142, 460, XREG_MOVED,     0, 0xFF, { 0x00, 0x0D, 0x18, 0x1A }, 0, 59 },   /* AntiElec */
	{ 143, 461, XREG_MOVED,     0, 0xFF, { 0x0C, 0x12, 0x19, 0x1A }, 0, 55 },   /* AntiWood */
	{ 144, 186, XREG_SAME },   /* AntiNavi */
	{ 145, 187, XREG_SAME },   /* AntiDmg */
	{ 146, 188, XREG_SAME },   /* AntiSwrd */
	{ 147, 189, XREG_SAME },   /* AntiRecv */
	{ 148, 190, XREG_SAME },   /* CopyDmg */
	{ 149, 192, XREG_ALIAS },   /* Attck+10 */
	{ 150, 193, XREG_SAME },   /* Navi+20 */
	{ 151, 107, XREG_SAME },   /* FireHit1 */
	{ 152, 108, XREG_SAME },   /* FireHit2 */
	{ 153, 109, XREG_SAME },   /* FireHit3 */
	{ 154, 462, XREG_MOVED,     0, 0xFF, { 0x00, 0x05, 0x0F, 0xFF }, 0, 28 },   /* HotBody1 */
	{ 155, 463, XREG_MOVED,     0, 0xFF, { 0x02, 0x11, 0x16, 0xFF }, 0, 40 },   /* HotBody2 */
	{ 156, 464, XREG_MOVED,     0, 0xFF, { 0x09, 0x0C, 0x12, 0xFF }, 0, 52 },   /* HotBody3 */
	{ 157, 465, XREG_MOVED,     0, 0xFF, { 0x01, 0x05, 0x0D, 0xFF }, 0, 30 },   /* AqWhirl1 */
	{ 158, 466, XREG_MOVED,     0, 0xFF, { 0x00, 0x07, 0x0E, 0xFF }, 0, 42 },   /* AqWhirl2 */
	{ 159, 467, XREG_MOVED,     0, 0xFF, { 0x03, 0x08, 0x11, 0xFF }, 0, 54 },   /* AqWhirl3 */
	{ 160, 468, XREG_MOVED,     0, 0xFF, { 0x03, 0x10, 0x12, 0xFF }, 0, 25 },   /* SideBub1 */
	{ 161, 469, XREG_MOVED,     0, 0xFF, { 0x07, 0x0B, 0x16, 0xFF }, 0, 35 },   /* SideBub2 */
	{ 162, 470, XREG_MOVED,     0, 0xFF, { 0x04, 0x0D, 0x0F, 0xFF }, 0, 45 },   /* SideBub3 */
	{ 163, 471, XREG_MOVED,     0, 0xFF, { 0x02, 0x15, 0x19, 0xFF }, 0, 32 },   /* ElcReel1 */
	{ 164, 472, XREG_MOVED,     0, 0xFF, { 0x01, 0x10, 0x18, 0xFF }, 0, 46 },   /* ElcReel2 */
	{ 165, 473, XREG_MOVED,     0, 0xFF, { 0x04, 0x0B, 0x11, 0xFF }, 0, 60 },   /* ElcReel3 */
	{ 166, 474, XREG_MOVED,     0, 0xFF, { 0x00, 0x15, 0x18, 0x1A }, 0, 40 },   /* CusVolt1 */
	{ 167, 475, XREG_MOVED,     0, 0xFF, { 0x02, 0x04, 0x06, 0xFF }, 0, 55 },   /* CusVolt2 */
	{ 168, 476, XREG_MOVED,     0, 0xFF, { 0x06, 0x0C, 0x0F, 0xFF }, 0, 70 },   /* CusVolt3 */
	{ 169, 477, XREG_MOVED,     0, 0xFF, { 0x01, 0x05, 0x15, 0xFF }, 0, 28 },   /* CrsShld1 */
	{ 170, 478, XREG_MOVED,     0, 0xFF, { 0x02, 0x0C, 0x11, 0xFF }, 0, 30 },   /* CrsShld2 */
	{ 171, 479, XREG_MOVED,     0, 0xFF, { 0x03, 0x0E, 0x19, 0xFF }, 0, 52 },   /* CrsShld3 */
	{ 172, 480, XREG_MOVED,     0, 0xFF, { 0x01, 0x04, 0x19, 0xFF }, 0, 42 },   /* WavePit */
	{ 173, 481, XREG_MOVED,     0, 0xFF, { 0x00, 0x05, 0x07, 0xFF }, 0, 48 },   /* RedWave */
	{ 174, 482, XREG_MOVED,     0, 0xFF, { 0x03, 0x0D, 0x16, 0xFF }, 0, 54 },   /* MudWave */
	{ 175, 483, XREG_MOVED,     0, 0xFF, { 0x07, 0x08, 0x09, 0xFF }, 0, 10 },   /* CactBal1 */
	{ 176, 484, XREG_MOVED,     0, 0xFF, { 0x10, 0x11, 0x12, 0xFF }, 0, 30 },   /* CactBal2 */
	{ 177, 485, XREG_MOVED,     0, 0xFF, { 0x0B, 0x0C, 0x0D, 0xFF }, 0, 50 },   /* CactBal3 */
	{ 178, 486, XREG_MOVED,     0, 0xFF, { 0x05, 0x11, 0x16, 0xFF }, 0, 27 },   /* WoodNos1 */
	{ 179, 487, XREG_MOVED,     0, 0xFF, { 0x09, 0x13, 0x19, 0xFF }, 0, 40 },   /* WoodNos2 */
	{ 180, 488, XREG_MOVED,     0, 0xFF, { 0x08, 0x12, 0x15, 0xFF }, 0, 53 },   /* WoodNos3 */
	{ 187, 214, XREG_DARK, 142, 0xFF, { 0x11, 0xFF, 0xFF, 0xFF }, 0, 99 },   /* DarkCirc */
	{ 188, 286, XREG_SAME },   /* DrkSword */
	{ 189, 289, XREG_SAME },   /* DarkInvs */
	{ 190, 290, XREG_SAME },   /* DarkPlus */
	{ 191, 215, XREG_DARK, 119, 0xFF, { 0x16, 0xFF, 0xFF, 0xFF }, 0, 99 },   /* DrkLance */
	{ 192, 216, XREG_DARK,  23, 0x02, { 0x13, 0xFF, 0xFF, 0xFF }, 0, 99 },   /* DarkWide */
	{ 193, 287, XREG_SAME },   /* DarkThnd */
	{ 194, 288, XREG_SAME },   /* DrkRecov */
	{ 195, 217, XREG_DARK,   0, 0xFF, { 0x05, 0xFF, 0xFF, 0xFF }, 0, 99 },   /* DarkMetr */
	{ 196, 218, XREG_DARK,  51, 0x01, { 0x03, 0xFF, 0xFF, 0xFF }, 0, 99 },   /* DarkDril */
	{ 197, 219, XREG_DARK,  53, 0x03, { 0x13, 0xFF, 0xFF, 0xFF }, 0, 99 },   /* DarkTorn */
	{ 198, 220, XREG_DARK, 146, 0xFF, { 0x04, 0x1A, 0xFF, 0xFF }, 0, 99 },   /* DrkSonic */
	{ 201, 182, XREG_SAME },   /* LifeAur */
	{ 202,  85, XREG_SAME },   /* Muramasa */
	{ 203, 151, XREG_SAME },   /* Guardian */
	{ 204, 152, XREG_SAME },   /* Anubis */
	{ 205, 195, XREG_ALIAS },   /* Attck+30 */
	{ 206, 176, XREG_SAME },   /* BugFix */
	{ 207, 196, XREG_SAME },   /* DblPoint */
	{ 208, 169, XREG_SAME },   /* Snctuary */
	{ 209, 174, XREG_SAME },   /* FullCust */
	{ 210, 139, XREG_SAME },   /* Meteors */
	{ 211, 138, XREG_SAME },   /* NumbrBl */
	{ 212, 291, XREG_MOVED,     0, 0xFF, { 0x09, 0xFF, 0xFF, 0xFF }, 1, 35 },   /* Jealousy */
	{ 213, 292, XREG_MOVED,     0, 0xFF, { 0x0F, 0xFF, 0xFF, 0xFF }, 1, 40 },   /* Poltrgst */
	{ 214, 293, XREG_MOVED,     0, 0xFF, { 0x16, 0xFF, 0xFF, 0xFF }, 1, 58 },   /* BlakWing */
	{ 216, 140, XREG_SAME },   /* JustcOne */
	{ 217,  83, XREG_SAME },   /* NeoVari */
	{ 218, 294, XREG_MOVED,     0, 0xFF, { 0x19, 0xFF, 0xFF, 0xFF }, 1, 80 },   /* Z Saver */
	{ 220,   8, XREG_SAME },   /* SuprVulc */
	{ 242, 489, XREG_MOVED,     0, 0xFF, { 0x0D, 0xFF, 0xFF, 0xFF }, 1, 33 },   /* NumbrMan */
	{ 243, 490, XREG_MOVED,     0, 0xFF, { 0x0D, 0xFF, 0xFF, 0xFF }, 1, 66 },   /* NumbrMn[SP] */
	{ 244, 491, XREG_MOVED,     0, 0xFF, { 0x0D, 0xFF, 0xFF, 0xFF }, 1, 66 },   /* NumbrMnDS */
	{ 245, 272, XREG_SAME },   /* Colonel */
	{ 246, 274, XREG_SAME },   /* Colonel[SP] */
	{ 247, 492, XREG_MOVED,     0, 0xFF, { 0x02, 0xFF, 0xFF, 0xFF }, 1, 75 },   /* ColonelDS */
	{ 248, 493, XREG_MOVED,     0, 0xFF, { 0x12, 0xFF, 0xFF, 0xFF }, 1, 50 },   /* ShadoMan */
	{ 249, 494, XREG_MOVED,     0, 0xFF, { 0x12, 0xFF, 0xFF, 0xFF }, 1, 80 },   /* ShadoMn[SP] */
	{ 250, 495, XREG_MOVED,     0, 0xFF, { 0x12, 0xFF, 0xFF, 0xFF }, 1, 80 },   /* ShadoMnDS */
	{ 251, 496, XREG_MOVED,     0, 0xFF, { 0x13, 0xFF, 0xFF, 0xFF }, 1, 43 },   /* TmhwkMan */
	{ 252, 497, XREG_MOVED,     0, 0xFF, { 0x13, 0xFF, 0xFF, 0xFF }, 1, 81 },   /* TmhwkMn[SP] */
	{ 253, 498, XREG_MOVED,     0, 0xFF, { 0x13, 0xFF, 0xFF, 0xFF }, 1, 81 },   /* TmhwkMnDS */
	{ 254, 499, XREG_MOVED,     0, 0xFF, { 0x0A, 0xFF, 0xFF, 0xFF }, 1, 50 },   /* KnightMn */
	{ 255, 500, XREG_MOVED,     0, 0xFF, { 0x0A, 0xFF, 0xFF, 0xFF }, 1, 90 },   /* KnigtMn[SP] */
	{ 256, 501, XREG_MOVED,     0, 0xFF, { 0x0A, 0xFF, 0xFF, 0xFF }, 1, 90 },   /* KnigtMnDS */
	{ 257, 502, XREG_MOVED,     0, 0xFF, { 0x13, 0xFF, 0xFF, 0xFF }, 1, 53 },   /* ToadMan */
	{ 258, 503, XREG_MOVED,     0, 0xFF, { 0x13, 0xFF, 0xFF, 0xFF }, 1, 69 },   /* ToadMan[SP] */
	{ 259, 504, XREG_MOVED,     0, 0xFF, { 0x13, 0xFF, 0xFF, 0xFF }, 1, 69 },   /* ToadManDS */
	{ 306, 306, XREG_SAME },   /* BassAnly */
	{ 307, 314, XREG_MOVED,     0, 0xFF, { 0x07, 0xFF, 0xFF, 0xFF }, 2, 92 },   /* HolyDrem */
	{ 308, 302, XREG_SAME },   /* BigHook */
	{ 309, 308, XREG_SAME },   /* CrossDiv */
	{ 320, 320, XREG_SAME },   /* GigaCan1 */
	{ 321, 321, XREG_SAME },   /* GigaCan2 */
	{ 322, 322, XREG_SAME },   /* GigaCan3 */
	{ 323, 350, XREG_PA,   0, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* InfVulc1 */
	{ 324, 351, XREG_PA,   0, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* InfVulc2 */
	{ 325, 352, XREG_PA,   0, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* InfVulc3 */
	{ 326, 353, XREG_PA,   0, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* Boxer1 */
	{ 327, 354, XREG_PA,   0, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* Boxer2 */
	{ 328, 355, XREG_PA,   0, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* Boxer3 */
	{ 329, 356, XREG_PA, 337, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* SuprSpr1 */
	{ 330, 357, XREG_PA, 337, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* SuprSpr2 */
	{ 331, 358, XREG_PA, 337, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* SuprSpr3 */
	{ 332, 359, XREG_PA,   0, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* ShakPar1 */
	{ 333, 360, XREG_PA,   0, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* ShakPar2 */
	{ 334, 361, XREG_PA,   0, 0xFF, { 0x00, 0x01, 0x02, 0x1A }, 4,  5 },   /* ShakPar3 */
	{ 335, 362, XREG_PA,   0, 0xFF, { 0xFF, 0xFF, 0xFF, 0xFF }, 4,  5 },   /* CacDanc1 */
	{ 336, 363, XREG_PA,   0, 0xFF, { 0xFF, 0xFF, 0xFF, 0xFF }, 4,  5 },   /* CacDanc2 */
	{ 337, 364, XREG_PA,   0, 0xFF, { 0xFF, 0xFF, 0xFF, 0xFF }, 4,  5 },   /* CacDanc3 */
	{ 338, 338, XREG_SAME },   /* H-Burst */
	{ 339, 339, XREG_SAME },   /* LifeSrd */
	{ 340, 340, XREG_SAME },   /* GreatYo */
	{ 341, 341, XREG_ALIAS },   /* PitHoky */
	{ 342, 342, XREG_SAME },   /* PoisPhar */
	{ 347, 335, XREG_SAME },   /* TimeBom+ */
	{ 348, 343, XREG_SAME },   /* BodyGrd */
};
const int xreg_count = (int)(sizeof xreg_rows / sizeof *xreg_rows);

bool xreg_own(const XChipReg *r) { return r && r->how != XREG_SAME && r->how != XREG_ALIAS; }

#define BN5_IDS 424   /* BN5's chip records */

/* each BN5 id's row and each own id's, plus one (0: none), made once */
static struct { bool made; int16_t of_bn5[BN5_IDS], at[XREG_IDS], named[XREG_IDS]; } idx;

static void index_make(void) {
	if (idx.made) return;
	idx.made = true;
	for (int i = 0; i < xreg_count; ++i) {
		const XChipReg *r = &xreg_rows[i];
		if (r->bn5 < BN5_IDS) idx.of_bn5[r->bn5] = (int16_t)(i + 1);
		if (xreg_own(r) && r->bn6 < XREG_IDS) idx.at[r->bn6] = (int16_t)(i + 1);
		if (!xreg_own(r) && r->bn6 < XREG_IDS && !idx.named[r->bn6]) idx.named[r->bn6] = (int16_t)(i + 1);
	}
}

const XChipReg *xreg_of_bn5(int bn5) {
	index_make();
	return bn5 > 0 && bn5 < BN5_IDS && idx.of_bn5[bn5] ? &xreg_rows[idx.of_bn5[bn5] - 1] : NULL;
}

const XChipReg *xreg_at(int bn6) {
	index_make();
	return bn6 > 0 && bn6 < XREG_IDS && idx.at[bn6] ? &xreg_rows[idx.at[bn6] - 1] : NULL;
}

const XChipReg *xreg_named(int bn6) {
	index_make();
	return bn6 > 0 && bn6 < XREG_IDS && idx.named[bn6] ? &xreg_rows[idx.named[bn6] - 1] : NULL;
}

bool xreg_free_id(int id) {
	/* (BN6's blank records: docs/ROM_DATA.md, The chip registry; 365-367
	 * too, the end of 350-368, whose 368 is BN6's pseudo-chips' bound) */
	return BN6_CHIP_BLANK(id) || (id >= 350 && id <= 367) || (id >= 411 && id < XREG_IDS);
}

static int bad(int i, char *why, size_t len, const char *what) {
	snprintf(why, len, "row %d: %s", i, what);
	return i + 1;
}

int xreg_check(const XChipReg *rows, int n, char *why, size_t len) {
	static uint8_t bn5_seen[BN5_IDS], own_seen[XREG_IDS];
	memset(bn5_seen, 0, sizeof bn5_seen);
	memset(own_seen, 0, sizeof own_seen);
	for (int i = 0; i < n; ++i) {
		const XChipReg *r = &rows[i];
		if (!r->bn5 || r->bn5 >= BN5_IDS || bn5_seen[r->bn5]++) return bad(i, why, len, "BN5's id is none, past its records, or twice");
		if (r->how > XREG_MOVED) return bad(i, why, len, "no such way in");
		if (!xreg_own(r)) {
			if (!r->bn6 || (r->bn6 > 313 && (r->bn6 < 320 || r->bn6 > 349))) return bad(i, why, len, "BN6's chip of its name is no folder chip");
			continue;
		}
		if (!xreg_free_id(r->bn6) || own_seen[r->bn6]++) return bad(i, why, len, "its own id is not free, or taken twice");
		if (r->twin > 410 || (r->how == XREG_RECORD && !r->twin)) return bad(i, why, len, "its twin is none of BN6's records");
		if ((r->type == 4) != (r->how == XREG_PA)) return bad(i, why, len, "a Program Advance's library type and its way in disagree");
	}
	return 0;
}
