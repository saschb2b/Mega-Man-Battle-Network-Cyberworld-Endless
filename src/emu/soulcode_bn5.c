/* soulcode_bn5.c: the code BN5's Team Colonel Souls run, moved into BN6's battle engine, written by tools/xsoul.py from
 * the two ROMs (python3 tools/xsoul.py; --check says whether this is what it writes). Do not edit: change the generator
 * and write it again. Numbers alone (docs/MULTIROM.md, Transplanted Souls): addresses in BN5 Team Colonel (USA) and BN6
 * Cybeast Gregar (USA), counts and the CRC32s of the ranges src/emu/soulcode.c copies from the player's ROM. */
#include "soulcode.h"

#include <stddef.h>

#include "bn6.h"
#include "soul_room.h"
#include "xvirus_dispatch.h"

static const XvBlock blocks[] = {
	{ 0x0800F074, 0x0800F080, SOULS_CODE + 0x0, 0xDA1B2BE9u },   /* number */
	{ 0x0800F0D6, 0x0800F0E4, SOULS_CODE + 0x12, 0x13A7AB96u },   /* number */
	{ 0x0800F66A, 0x0800F68E, SOULS_CODE + 0x22, 0x6CF9E8A5u },   /* number */
	{ 0x0800F79E, 0x0800F840, SOULS_CODE + 0x52, 0xE82794A4u },   /* knight */
	{ 0x0800F864, 0x0800F880, SOULS_CODE + 0x100, 0x05186FCDu },   /* tomahawk */
	{ 0x0800F9E6, 0x0800FA14, SOULS_CODE + 0x122, 0x234C2284u },   /* toad */
	{ 0x0800FBE6, 0x0800FC04, SOULS_CODE + 0x152, 0x1B492291u },   /* shadow */
	{ 0x08011C3C, 0x08011D4C, SOULS_CODE + 0x170, 0xCB8CEE1Du },   /* colonel */
	{ 0x08012008, 0x08012082, SOULS_CODE + 0x280, 0xB112DCC9u },   /* unite */
	{ 0x08012530, 0x08012704, SOULS_CODE + 0x300, 0x2030EB14u },   /* shadow */
	{ 0x08016634, 0x0801663C, SOULS_CODE + 0x4E0, 0xB384251Bu },   /* colonel */
	{ 0x080236BC, 0x08023736, SOULS_CODE + 0x4F0, 0x10F05CF6u },   /* colonel */
	{ 0x080254F8, 0x08025524, SOULS_CODE + 0x570, 0x9749B90Du },   /* colonel */
	{ 0x080B9A5C, 0x080B9D10, SOULS_CODE + 0x5A0, 0xBA3CBC8Cu },   /* data */
	{ 0x080BA634, 0x080BA6A8, SOULS_CODE + 0x860, 0x34F7D768u },   /* colonel */
	{ 0x080BCA14, 0x080BCABE, SOULS_CODE + 0x8E0, 0x4058C9F5u },   /* toad */
	{ 0x080C33A8, 0x080C34E0, SOULS_CODE + 0x990, 0x217F5537u },   /* data */
	{ 0x080C5014, 0x080C544C, SOULS_CODE + 0xAD0, 0x76FE03D1u },   /* number */
	{ 0x080C6C08, 0x080C6E28, SOULS_CODE + 0xF10, 0xFFCD61DDu },   /* data */
	{ 0x080C7650, 0x080C77EC, SOULS_CODE + 0x1130, 0x597770AFu },   /* knight */
	{ 0x080CA91C, 0x080CAD48, SOULS_CODE + 0x12D0, 0xD263FB4Fu },   /* colonel */
	{ 0x080D6788, 0x080D68C0, SOULS_CODE + 0x1700, 0x1432C507u },   /* toad */
	{ 0x080E3618, 0x080E3638, SOULS_CODE + 0x1840, 0xD01AE090u },   /* shadow */
	{ 0x080E36DC, 0x080E36F8, SOULS_CODE + 0x1860, 0x5F51E73Au },   /* shadow */
	{ 0x080EBDC6, 0x080EC234, SOULS_CODE + 0x1882, 0x51FA1AC5u },   /* shadow */
	{ 0x080EE864, 0x080EE93C, SOULS_CODE + 0x1CF0, 0xE0262102u },   /* number */
	{ 0x080EEADC, 0x080EEB0C, SOULS_CODE + 0x1DD0, 0x560AFA29u },   /* shadow */
	{ 0x080EEBDE, 0x080EEC86, SOULS_CODE + 0x1E02, 0x60A893B6u },   /* knight */
	{ 0x080EED92, 0x080EEE82, SOULS_CODE + 0x1EB2, 0xD56BB4A7u },   /* colonel */
	{ 0x080EEF6C, 0x080EF032, SOULS_CODE + 0x1FB0, 0x90EBB3BDu },   /* tomahawk */
	{ 0x080EF68A, 0x080EF732, SOULS_CODE + 0x2082, 0x8EE8BB09u },   /* toad */
};
static const XvData data[] = {
	{ 0x0800F7CE, 0x0800F7D0, XV_RAW },
	{ 0x0800F7FC, 0x0800F818, XV_RAW },
	{ 0x08011C8E, 0x08011C90, XV_RAW },
	{ 0x08011CA2, 0x08011CA4, XV_RAW },
	{ 0x08011CA4, 0x08011CA8, XV_POOL },
	{ 0x08011CA8, 0x08011CAA, XV_RAW },
	{ 0x08011CB4, 0x08011D2A, XV_RAW },
	{ 0x08011D40, 0x08011D48, XV_RAW },
	{ 0x08011D48, 0x08011D4C, XV_POOL },
	{ 0x080125B0, 0x080125B8, XV_POOL },
	{ 0x080125B8, 0x080126FC, XV_RAW },
	{ 0x080126FC, 0x08012704, XV_POOL },
	{ 0x080236E2, 0x080236E4, XV_RAW },
	{ 0x080236E4, 0x080236E8, XV_POOL },
	{ 0x0802551A, 0x0802551C, XV_RAW },
	{ 0x0802551C, 0x08025524, XV_POOL },
	{ 0x080B9A5C, 0x080B9B80, XV_RAW },
	{ 0x080B9B8E, 0x080B9B90, XV_RAW },
	{ 0x080B9B90, 0x080B9BA0, XV_POOL },
	{ 0x080B9CD6, 0x080B9CE0, XV_RAW },
	{ 0x080B9CEE, 0x080B9D04, XV_RAW },
	{ 0x080B9D04, 0x080B9D10, XV_POOL },
	{ 0x080BA694, 0x080BA698, XV_POOL },
	{ 0x080BA698, 0x080BA6A8, XV_RAW },
	{ 0x080C33A8, 0x080C33AC, XV_RAW },
	{ 0x080C33BA, 0x080C33BC, XV_RAW },
	{ 0x080C33BC, 0x080C33CC, XV_POOL },
	{ 0x080C34A0, 0x080C34A4, XV_RAW },
	{ 0x080C34D2, 0x080C34DC, XV_RAW },
	{ 0x080C34DC, 0x080C34E0, XV_POOL },
	{ 0x080C5038, 0x080C5048, XV_POOL },
	{ 0x080C514A, 0x080C514C, XV_RAW },
	{ 0x080C514C, 0x080C5164, XV_POOL },
	{ 0x080C5164, 0x080C516C, XV_RAW },
	{ 0x080C53DE, 0x080C543C, XV_RAW },
	{ 0x080C543C, 0x080C544C, XV_POOL },
	{ 0x080C6C08, 0x080C6C80, XV_RAW },
	{ 0x080C6C8E, 0x080C6C90, XV_RAW },
	{ 0x080C6C90, 0x080C6CA0, XV_POOL },
	{ 0x080C6D24, 0x080C6D38, XV_RAW },
	{ 0x080C6D38, 0x080C6D44, XV_POOL },
	{ 0x080C6D44, 0x080C6D70, XV_RAW },
	{ 0x080C6D7E, 0x080C6D80, XV_RAW },
	{ 0x080C6D80, 0x080C6D8C, XV_POOL },
	{ 0x080C6DEC, 0x080C6DF8, XV_RAW },
	{ 0x080C6E24, 0x080C6E28, XV_POOL },
	{ 0x080C7668, 0x080C7678, XV_POOL },
	{ 0x080C7794, 0x080C77AC, XV_RAW },
	{ 0x080C77E2, 0x080C77E4, XV_RAW },
	{ 0x080C77E4, 0x080C77EC, XV_POOL },
	{ 0x080CA92E, 0x080CA930, XV_RAW },
	{ 0x080CA930, 0x080CA940, XV_POOL },
	{ 0x080CA9D4, 0x080CA9E4, XV_POOL },
	{ 0x080CA9F2, 0x080CA9F4, XV_RAW },
	{ 0x080CA9F4, 0x080CAA00, XV_POOL },
	{ 0x080CAAB6, 0x080CAAB8, XV_RAW },
	{ 0x080CAAB8, 0x080CAAC4, XV_POOL },
	{ 0x080CAB68, 0x080CAB6C, XV_POOL },
	{ 0x080CAB6C, 0x080CAB70, XV_RAW },
	{ 0x080CABEA, 0x080CACE0, XV_RAW },
	{ 0x080CAD06, 0x080CAD2C, XV_RAW },
	{ 0x080CAD2C, 0x080CAD48, XV_POOL },
	{ 0x080D679A, 0x080D679C, XV_RAW },
	{ 0x080D679C, 0x080D67AC, XV_POOL },
	{ 0x080D6840, 0x080D684C, XV_POOL },
	{ 0x080D68B6, 0x080D68B8, XV_RAW },
	{ 0x080D68B8, 0x080D68C0, XV_POOL },
	{ 0x080E3626, 0x080E3628, XV_RAW },
	{ 0x080E3628, 0x080E3638, XV_POOL },
	{ 0x080EBDD4, 0x080EBDEC, XV_POOL },
	{ 0x080EBE9C, 0x080EBEA4, XV_RAW },
	{ 0x080EBEA4, 0x080EBEAC, XV_POOL },
	{ 0x080EBEAC, 0x080EBEC2, XV_RAW },
	{ 0x080EC04E, 0x080EC050, XV_RAW },
	{ 0x080EC050, 0x080EC074, XV_POOL },
	{ 0x080EC074, 0x080EC120, XV_RAW },
	{ 0x080EC208, 0x080EC232, XV_RAW },
	{ 0x080EE872, 0x080EE874, XV_RAW },
	{ 0x080EE874, 0x080EE880, XV_POOL },
	{ 0x080EE902, 0x080EE904, XV_RAW },
	{ 0x080EE904, 0x080EE908, XV_POOL },
	{ 0x080EE908, 0x080EE918, XV_RAW },
	{ 0x080EEB00, 0x080EEB0C, XV_POOL },
	{ 0x080EEBEC, 0x080EEBFC, XV_POOL },
	{ 0x080EEDA0, 0x080EEDAC, XV_POOL },
	{ 0x080EEE44, 0x080EEE48, XV_POOL },
	{ 0x080EEE48, 0x080EEE5C, XV_RAW },
	{ 0x080EEF7A, 0x080EEF7C, XV_RAW },
	{ 0x080EEF7C, 0x080EEF88, XV_POOL },
	{ 0x080EF006, 0x080EF008, XV_RAW },
	{ 0x080EF008, 0x080EF00C, XV_POOL },
	{ 0x080EF698, 0x080EF6A4, XV_POOL },
};
/* every word of the pools and tables: BN5's, and BN6's in its place */
static const XvNumber words[] = {
	{ 0x08011CA4, 0x000A0006u, 0x000A0006u },   /* a number */
	{ 0x08011D48, 0x00004000u, 0x00004000u },   /* a number */
	{ 0x080125B0, 0x080125B8u, SOULS_CODE + 0x388u },   /* data moved with the code */
	{ 0x080125B4, 0x080125C0u, SOULS_CODE + 0x390u },   /* data moved with the code */
	{ 0x080126FC, 0x00010000u, 0x00010000u },   /* a number */
	{ 0x08012700, 0x0F800000u, 0x0F800000u },   /* a number */
	{ 0x080236E4, 0x06016EC0u, 0x06016EC0u },   /* I/O or video, the same */
	{ 0x0802551C, 0x40450084u, 0x40450084u },   /* a number */
	{ 0x08025520, 0x0000A776u, 0x0000A776u },   /* a number */
	{ 0x080B9B90, 0x080B9B94u, SOULS_CODE + 0x6D8u },   /* moved with the code */
	{ 0x080B9B94, 0x080B9BA1u, SOULS_CODE + 0x6E5u },   /* a jump table's entry, moved */
	{ 0x080B9B98, 0x080B9C27u, SOULS_CODE + 0x76Bu },   /* a jump table's entry, moved */
	{ 0x080B9B9C, 0x0800325Du, 0x08003459u },   /* a jump table's entry, BN6's twin (project:cangard) */
	{ 0x080B9D04, 0x080B9A5Cu, SOULS_CODE + 0x5A0u },   /* data moved with the code */
	{ 0x080B9D08, 0x00000180u, 0x00000180u },   /* a number */
	{ 0x080B9D0C, 0x080B9A5Cu, SOULS_CODE + 0x5A0u },   /* data moved with the code */
	{ 0x080BA694, 0x080BA698u, SOULS_CODE + 0x8C4u },   /* data moved with the code */
	{ 0x080C33BC, 0x080C33C0u, SOULS_CODE + 0x9A8u },   /* moved with the code */
	{ 0x080C33C0, 0x080C33CDu, SOULS_CODE + 0x9B5u },   /* a jump table's entry, moved */
	{ 0x080C33C4, 0x080C341Bu, SOULS_CODE + 0xA03u },   /* a jump table's entry, moved */
	{ 0x080C33C8, 0x0800325Du, 0x08003459u },   /* a jump table's entry, BN6's twin (project:cangard) */
	{ 0x080C34DC, 0x080C33A8u, SOULS_CODE + 0x990u },   /* data moved with the code */
	{ 0x080C5038, 0x080C503Cu, SOULS_CODE + 0xAF8u },   /* moved with the code */
	{ 0x080C503C, 0x080C5049u, SOULS_CODE + 0xB05u },   /* a jump table's entry, moved */
	{ 0x080C5040, 0x080C50C7u, SOULS_CODE + 0xB83u },   /* a jump table's entry, moved */
	{ 0x080C5044, 0x080138EBu, 0x08016C8Bu },   /* a jump table's entry, BN6's twin (project:cangard) */
	{ 0x080C514C, 0x080C5154u, SOULS_CODE + 0xC10u },   /* moved with the code */
	{ 0x080C5150, 0x080C5164u, SOULS_CODE + 0xC20u },   /* data moved with the code */
	{ 0x080C5154, 0x080C516Du, SOULS_CODE + 0xC29u },   /* a jump table's entry, moved */
	{ 0x080C5158, 0x080C523Du, SOULS_CODE + 0xCF9u },   /* a jump table's entry, moved */
	{ 0x080C515C, 0x080C5279u, SOULS_CODE + 0xD35u },   /* a jump table's entry, moved */
	{ 0x080C5160, 0x080C52CDu, SOULS_CODE + 0xD89u },   /* a jump table's entry, moved */
	{ 0x080C543C, 0x00040000u, 0x00040000u },   /* a number */
	{ 0x080C5440, 0xFFFFD000u, 0xFFFFD000u },   /* a number */
	{ 0x080C5444, 0x00030000u, 0x00030000u },   /* a number */
	{ 0x080C5448, 0x0405FF0Fu, 0x0405FF0Fu },   /* I/O or video, the same */
	{ 0x080C6C90, 0x080C6C94u, SOULS_CODE + 0xF9Cu },   /* moved with the code */
	{ 0x080C6C94, 0x080C6CA1u, SOULS_CODE + 0xFA9u },   /* a jump table's entry, moved */
	{ 0x080C6C98, 0x080C6CE9u, SOULS_CODE + 0xFF1u },   /* a jump table's entry, moved */
	{ 0x080C6C9C, 0x080138EBu, 0x08016C8Bu },   /* a jump table's entry, BN6's twin (project:cangard) */
	{ 0x080C6D38, 0x080C6D3Cu, SOULS_CODE + 0x1044u },   /* moved with the code */
	{ 0x080C6D3C, 0x080C6D45u, 0x080C8279u },   /* a jump table's entry, BN6's twin (search, by place) */
	{ 0x080C6D40, 0x080C6D71u, SOULS_CODE + 0x1079u },   /* a jump table's entry, moved */
	{ 0x080C6D80, 0x080C6D84u, SOULS_CODE + 0x108Cu },   /* moved with the code */
	{ 0x080C6D84, 0x080C6D8Du, SOULS_CODE + 0x1095u },   /* a jump table's entry, moved */
	{ 0x080C6D88, 0x080C6DEDu, 0x080C831Du },   /* a jump table's entry, BN6's twin (search, by place) */
	{ 0x080C6E24, 0x080C6C08u, SOULS_CODE + 0xF10u },   /* data moved with the code */
	{ 0x080C7668, 0x080C766Cu, SOULS_CODE + 0x114Cu },   /* moved with the code */
	{ 0x080C766C, 0x080C7679u, SOULS_CODE + 0x1159u },   /* a jump table's entry, moved */
	{ 0x080C7670, 0x080C7707u, SOULS_CODE + 0x11E7u },   /* a jump table's entry, moved */
	{ 0x080C7674, 0x080138EBu, 0x08016C8Bu },   /* a jump table's entry, BN6's twin (project:cangard) */
	{ 0x080C77E4, 0x08005CD0u, 0x080065E0u },   /* a sine table (its 0x80 bytes the same) */
	{ 0x080C77E8, 0x08005D50u, 0x08006660u },   /* a cosine table (its 0x80 bytes the same) */
	{ 0x080CA930, 0x080CA934u, SOULS_CODE + 0x12E8u },   /* moved with the code */
	{ 0x080CA934, 0x080CA941u, SOULS_CODE + 0x12F5u },   /* a jump table's entry, moved */
	{ 0x080CA938, 0x080CA97Fu, SOULS_CODE + 0x1333u },   /* a jump table's entry, moved */
	{ 0x080CA93C, 0x080138EBu, 0x08016C8Bu },   /* a jump table's entry, BN6's twin (project:cangard) */
	{ 0x080CA9D4, 0x080CA9D8u, SOULS_CODE + 0x138Cu },   /* moved with the code */
	{ 0x080CA9D8, 0x080CA9E5u, SOULS_CODE + 0x1399u },   /* a jump table's entry, moved */
	{ 0x080CA9DC, 0x080CAAA9u, SOULS_CODE + 0x145Du },   /* a jump table's entry, moved */
	{ 0x080CA9E0, 0x080CAB71u, SOULS_CODE + 0x1525u },   /* a jump table's entry, moved */
	{ 0x080CA9F4, 0x080CA9F8u, SOULS_CODE + 0x13ACu },   /* moved with the code */
	{ 0x080CA9F8, 0x080CAA01u, SOULS_CODE + 0x13B5u },   /* a jump table's entry, moved */
	{ 0x080CA9FC, 0x080CAA3Bu, SOULS_CODE + 0x13EFu },   /* a jump table's entry, moved */
	{ 0x080CAAB8, 0x080CAABCu, SOULS_CODE + 0x1470u },   /* moved with the code */
	{ 0x080CAABC, 0x080CAAC5u, SOULS_CODE + 0x1479u },   /* a jump table's entry, moved */
	{ 0x080CAAC0, 0x080CAAF9u, SOULS_CODE + 0x14ADu },   /* a jump table's entry, moved */
	{ 0x080CAB68, 0x080CAB6Cu, SOULS_CODE + 0x1520u },   /* data moved with the code */
	{ 0x080CAD2C, 0x01001020u, 0x01001020u },   /* a number */
	{ 0x080CAD30, 0x01031020u, 0x01031020u },   /* a number */
	{ 0x080CAD34, 0x13140300u, 0x13140300u },   /* a number */
	{ 0x080CAD38, 0x0705FF02u, 0x0705FF02u },   /* I/O or video, the same */
	{ 0x080CAD3C, 0x02034000u, 0x0203FF60u },   /* BN5's per-side words of ColonelSoul's obstacle strike (0x080CACE0, 8 bytes a side): the RAM plan's RAM_COLONEL (free_space.h), which BN6 leaves alone */
	{ 0x080CAD40, 0x02034000u, 0x0203FF60u },   /* BN5's per-side words of ColonelSoul's obstacle strike (0x080CACE0, 8 bytes a side): the RAM plan's RAM_COLONEL (free_space.h), which BN6 leaves alone */
	{ 0x080CAD44, 0x02034000u, 0x0203FF60u },   /* BN5's per-side words of ColonelSoul's obstacle strike (0x080CACE0, 8 bytes a side): the RAM plan's RAM_COLONEL (free_space.h), which BN6 leaves alone */
	{ 0x080D679C, 0x080D67A0u, SOULS_CODE + 0x1718u },   /* moved with the code */
	{ 0x080D67A0, 0x080D67ADu, SOULS_CODE + 0x1725u },   /* a jump table's entry, moved */
	{ 0x080D67A4, 0x080D67FDu, SOULS_CODE + 0x1775u },   /* a jump table's entry, moved */
	{ 0x080D67A8, 0x080138EBu, 0x08016C8Bu },   /* a jump table's entry, BN6's twin (project:cangard) */
	{ 0x080D6840, 0x080D6844u, SOULS_CODE + 0x17BCu },   /* moved with the code */
	{ 0x080D6844, 0x080D684Du, SOULS_CODE + 0x17C5u },   /* a jump table's entry, moved */
	{ 0x080D6848, 0x080D6861u, SOULS_CODE + 0x17D9u },   /* a jump table's entry, moved */
	{ 0x080D68B8, 0x00001044u, 0x00001044u },   /* a number */
	{ 0x080D68BC, 0x0A050000u, 0x0A050000u },   /* a number */
	{ 0x080E3628, 0x080E362Cu, SOULS_CODE + 0x1854u },   /* moved with the code */
	{ 0x080E362C, 0x080E3639u, 0x080E4619u },   /* a jump table's entry, BN6's twin (map:calls) */
	{ 0x080E3630, 0x080E369Bu, 0x080E46AFu },   /* a jump table's entry, BN6's twin (map:calls) */
	{ 0x080E3634, 0x080E36D5u, 0x080E4713u },   /* a jump table's entry, BN6's twin (map:calls) */
	{ 0x080EBDD4, 0x080EBDD8u, SOULS_CODE + 0x1894u },   /* moved with the code */
	{ 0x080EBDD8, 0x080EBDEDu, SOULS_CODE + 0x18A9u },   /* a jump table's entry, moved */
	{ 0x080EBDDC, 0x080EBEADu, 0x080ECB8Du },   /* a jump table's entry, BN6's twin (search, by place) */
	{ 0x080EBDE0, 0x080EBEC3u, SOULS_CODE + 0x197Fu },   /* a jump table's entry, moved */
	{ 0x080EBDE4, 0x080EC1CDu, SOULS_CODE + 0x1C89u },   /* a jump table's entry, moved */
	{ 0x080EBDE8, 0x080EC209u, 0x080ECF23u },   /* a jump table's entry, BN6's twin (search, by place) */
	{ 0x080EBEA4, 0x0000FFE0u, 0x0000FFE0u },   /* a number */
	{ 0x080EBEA8, 0x01010014u, 0x01010014u },   /* a number */
	{ 0x080EC050, 0x080EC074u, SOULS_CODE + 0x1B30u },   /* data moved with the code */
	{ 0x080EC054, 0x080EC110u, SOULS_CODE + 0x1BCCu },   /* data moved with the code */
	{ 0x080EC058, 0x080EC05Cu, SOULS_CODE + 0x1B18u },   /* data moved with the code */
	{ 0x080EC05C, 0x080EC0A8u, SOULS_CODE + 0x1B64u },   /* a pointer in a table of its data, moved */
	{ 0x080EC060, 0x080EC0DCu, SOULS_CODE + 0x1B98u },   /* a pointer in a table of its data, moved */
	{ 0x080EC064, 0x0705FF01u, 0x0705FF01u },   /* I/O or video, the same */
	{ 0x080EC068, 0x0000FFE0u, 0x0000FFE0u },   /* a number */
	{ 0x080EC06C, 0x0101001Eu, 0x0101001Eu },   /* a number */
	{ 0x080EC070, 0x0000001Eu, 0x0000001Eu },   /* a number */
	{ 0x080EE874, 0x080EE878u, SOULS_CODE + 0x1D04u },   /* moved with the code */
	{ 0x080EE878, 0x080EE881u, SOULS_CODE + 0x1D0Du },   /* a jump table's entry, moved */
	{ 0x080EE87C, 0x080EE919u, SOULS_CODE + 0x1DA5u },   /* a jump table's entry, moved */
	{ 0x080EE904, 0x080EE908u, SOULS_CODE + 0x1D94u },   /* data moved with the code */
	{ 0x080EEB00, 0x080EEB08u, SOULS_CODE + 0x1DFCu },   /* moved with the code */
	{ 0x080EEB04, 0x00000600u, 0x00000600u },   /* a number */
	{ 0x080EEB08, 0x080EEB0Du, 0x080F11EDu },   /* a jump table's entry, BN6's twin (map:bytes) */
	{ 0x080EEBEC, 0x080EEBF0u, SOULS_CODE + 0x1E14u },   /* moved with the code */
	{ 0x080EEBF0, 0x080EEBFDu, SOULS_CODE + 0x1E21u },   /* a jump table's entry, moved */
	{ 0x080EEBF4, 0x080EEC45u, SOULS_CODE + 0x1E69u },   /* a jump table's entry, moved */
	{ 0x080EEBF8, 0x080EEC63u, SOULS_CODE + 0x1E87u },   /* a jump table's entry, moved */
	{ 0x080EEDA0, 0x080EEDA4u, SOULS_CODE + 0x1EC4u },   /* moved with the code */
	{ 0x080EEDA4, 0x080EEDADu, SOULS_CODE + 0x1ECDu },   /* a jump table's entry, moved */
	{ 0x080EEDA8, 0x080EEE5Du, SOULS_CODE + 0x1F7Du },   /* a jump table's entry, moved */
	{ 0x080EEE44, 0x080EEE4Cu, SOULS_CODE + 0x1F6Cu },   /* data moved with the code */
	{ 0x080EEF7C, 0x080EEF80u, SOULS_CODE + 0x1FC4u },   /* moved with the code */
	{ 0x080EEF80, 0x080EEF89u, SOULS_CODE + 0x1FCDu },   /* a jump table's entry, moved */
	{ 0x080EEF84, 0x080EF00Du, SOULS_CODE + 0x2051u },   /* a jump table's entry, moved */
	{ 0x080EF008, 0x0705FF11u, 0x0705FF11u },   /* I/O or video, the same */
	{ 0x080EF698, 0x080EF69Cu, SOULS_CODE + 0x2094u },   /* moved with the code */
	{ 0x080EF69C, 0x080EF6A5u, SOULS_CODE + 0x209Du },   /* a jump table's entry, moved */
	{ 0x080EF6A0, 0x080EF705u, SOULS_CODE + 0x20FDu },   /* a jump table's entry, moved */
};
/* every call out of its block: its site, BN5's routine, BN6's in its place */
static const XvNumber calls[] = {
	{ 0x0800F078, 0x080C34A6u, SOULS_CODE + 0xA8Fu },   /* moved */
	{ 0x0800F0DA, 0x080C34D2u, 0x080C5A74u },   /* search, by place, SAME */
	{ 0x0800F670, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F7A4, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F7D6, 0x0800FE70u, 0x080126EAu },   /* soul:the chip writer's second entry: push {r4, DIFF (the Souls' audit) */
	{ 0x0800F7E2, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F7F0, 0x080BA634u, SOULS_CODE + 0x861u },   /* moved */
	{ 0x0800F832, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F876, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F9F8, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800FA0A, 0x080BCA14u, SOULS_CODE + 0x8E1u },   /* moved */
	{ 0x08011C42, 0x080CAD06u, SOUL_STUB_STRIKE },   /* answered by the Souls' C */
	{ 0x08011C4E, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x08011C5E, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x08011C72, 0x080CACE0u, SOULS_CODE + 0x1695u },   /* moved */
	{ 0x08011C7C, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x08011C80, 0x0800E510u, 0x080107C0u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x08011C88, 0x08016B96u, 0x0801A082u },   /* map:calls, DIFF (audited) */
	{ 0x08011C94, 0x080E22C4u, 0x080E33A8u },   /* map:bytes, SAME */
	{ 0x08011CAE, 0x0800E096u, 0x08010312u },   /* map:bytes, SAME */
	{ 0x08011D2E, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x08011D32, 0x0800E510u, 0x080107C0u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x08011D3A, 0x08016B96u, 0x0801A082u },   /* map:calls, DIFF (audited) */
	{ 0x08012012, 0x080123FAu, SOUL_STUB_PENDING },   /* answered by the Souls' C */
	{ 0x08012018, 0x080C060Cu, 0x080C1EACu },   /* map:bytes, SAME */
	{ 0x08012026, 0x080123FAu, SOUL_STUB_PENDING },   /* answered by the Souls' C */
	{ 0x0801202E, 0x080C0760u, 0x080C2000u },   /* map:bytes, SAME */
	{ 0x08012042, 0x080123FAu, SOUL_STUB_PENDING },   /* answered by the Souls' C */
	{ 0x0801204E, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x08012056, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x08012068, 0x080DFFD2u, 0x080E1932u },   /* project:cangard, SAME */
	{ 0x0801253C, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x08012550, 0x0800B334u, 0x0800CC72u },   /* map:bytes, SAME */
	{ 0x0801256A, 0x0800AFB0u, 0x0800C8F8u },   /* map:calls, LAYOUT */
	{ 0x08012584, 0x0800B348u, 0x0800CC86u },   /* map:bytes, SAME */
	{ 0x080236C6, 0x0801DF0Cu, 0x08021AA4u },   /* map:bytes, SAME */
	{ 0x080236D0, 0x080009E8u, 0x08000AC8u },   /* map:bytes, SAME */
	{ 0x080236FC, 0x080254F8u, SOULS_CODE + 0x571u },   /* moved */
	{ 0x0802370A, 0x08024004u, 0x080281D4u },   /* map:bytes, DIFF (the Souls' audit) */
	{ 0x08023714, 0x080258E8u, 0x08029CD4u },   /* soul:a pick's slot drawn on the Custom screen, DIFF (the Souls' audit) */
	{ 0x08023728, 0x08024A2Au, 0x08028E32u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x080254FC, 0x08023BE0u, 0x08027E1Cu },   /* map:bytes, SAME */
	{ 0x08025514, 0x080089D8u, 0x08009FF8u },   /* map:bytes, SAME */
	{ 0x080B9BB2, 0x08002650u, 0x080026E4u },   /* project:cangard, SAME */
	{ 0x080B9BB6, 0x08002610u, 0x080026A4u },   /* project:cangard, SAME */
	{ 0x080B9BBA, 0x08002DF4u, 0x08002F90u },   /* project:cangard, SAME */
	{ 0x080B9BCC, 0x08010D64u, 0x080136CCu },   /* map:calls+bytes, SAME */
	{ 0x080B9BE2, 0x08002C10u, 0x08002DA4u },   /* project:cangard, SAME */
	{ 0x080B9BE6, 0x08002610u, 0x080026A4u },   /* project:cangard, SAME */
	{ 0x080B9BF8, 0x08002BECu, 0x08002D80u },   /* project:cangard, SAME */
	{ 0x080B9C10, 0x08015140u, 0x08018810u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x080B9C5A, 0x08002D40u, 0x08002EDCu },   /* map:bytes, SAME */
	{ 0x080B9C5E, 0x08002D34u, 0x08002ED0u },   /* map:bytes, SAME */
	{ 0x080B9C64, 0x08002C34u, 0x08002DC8u },   /* map:bytes, SAME */
	{ 0x080B9C68, 0x08002C20u, 0x08002DB4u },   /* near, SAME */
	{ 0x080B9C6E, 0x08002B4Cu, 0x08002CE0u },   /* map:bytes, SAME */
	{ 0x080B9C72, 0x08002AEAu, 0x08002C7Eu },   /* map:bytes, SAME */
	{ 0x080B9C78, 0x08002DE2u, 0x08002F7Eu },   /* map:bytes, SAME */
	{ 0x080B9C7C, 0x08002DC0u, 0x08002F5Cu },   /* project:cangard, SAME */
	{ 0x080B9C8E, 0x08008A78u, 0x0800A098u },   /* project:cangard, SAME */
	{ 0x080B9C94, 0x08008A1Cu, 0x0800A03Cu },   /* project:cangard, LAYOUT */
	{ 0x080B9C9A, 0x080186A8u, 0x0801BBACu },   /* project:cangard, SAME */
	{ 0x080B9CB4, 0x08003140u, XV_SPAWN_1 },   /* the second-level dispatch's spawner */
	{ 0x080BA638, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080BA656, 0x0800B334u, 0x0800CC72u },   /* map:bytes, SAME */
	{ 0x080BA660, 0x0800AFB0u, 0x0800C8F8u },   /* map:calls, LAYOUT */
	{ 0x080BA682, 0x0800B334u, 0x0800CC72u },   /* map:bytes, SAME */
	{ 0x080BCA18, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080BCA96, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080C33CE, 0x0800C6ACu, 0x0800E2ACu },   /* project:cangard, SAME */
	{ 0x080C33E0, 0x08002650u, 0x080026E4u },   /* project:cangard, SAME */
	{ 0x080C33E4, 0x08002610u, 0x080026A4u },   /* project:cangard, SAME */
	{ 0x080C33E8, 0x08002DF4u, 0x08002F90u },   /* project:cangard, SAME */
	{ 0x080C33FA, 0x08002C10u, 0x08002DA4u },   /* project:cangard, SAME */
	{ 0x080C33FE, 0x08002610u, 0x080026A4u },   /* project:cangard, SAME */
	{ 0x080C3404, 0x08002BF8u, 0x08002D8Cu },   /* map:bytes, SAME */
	{ 0x080C3408, 0x08002BECu, 0x08002D80u },   /* project:cangard, SAME */
	{ 0x080C340C, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080C3410, 0x08002DC0u, 0x08002F5Cu },   /* project:cangard, SAME */
	{ 0x080C3440, 0x08002BF8u, 0x08002D8Cu },   /* map:bytes, SAME */
	{ 0x080C3444, 0x08002BECu, 0x08002D80u },   /* project:cangard, SAME */
	{ 0x080C344A, 0x08002D40u, 0x08002EDCu },   /* map:bytes, SAME */
	{ 0x080C344E, 0x08002D34u, 0x08002ED0u },   /* map:bytes, SAME */
	{ 0x080C3454, 0x08002C34u, 0x08002DC8u },   /* map:bytes, SAME */
	{ 0x080C3458, 0x08002C20u, 0x08002DB4u },   /* near, SAME */
	{ 0x080C345E, 0x08002DA2u, 0x08002F3Eu },   /* map:bytes, SAME */
	{ 0x080C3462, 0x08002D66u, 0x08002F02u },   /* near, SAME */
	{ 0x080C346C, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080C3470, 0x08002DC0u, 0x08002F5Cu },   /* project:cangard, SAME */
	{ 0x080C3476, 0x08002B4Cu, 0x08002CE0u },   /* map:bytes, SAME */
	{ 0x080C347A, 0x08002AEAu, 0x08002C7Eu },   /* map:bytes, SAME */
	{ 0x080C348E, 0x08008A78u, 0x0800A098u },   /* project:cangard, SAME */
	{ 0x080C3494, 0x08008A1Cu, 0x0800A03Cu },   /* project:cangard, LAYOUT */
	{ 0x080C349A, 0x080186A8u, 0x0801BBACu },   /* project:cangard, SAME */
	{ 0x080C34AC, 0x08003140u, XV_SPAWN_1 },   /* the second-level dispatch's spawner */
	{ 0x080C5026, 0x08008A78u, 0x0800A098u },   /* project:cangard, SAME */
	{ 0x080C502C, 0x080186A8u, 0x0801BBACu },   /* project:cangard, SAME */
	{ 0x080C5032, 0x080186F0u, 0x0801BBF4u },   /* map:bytes, SAME */
	{ 0x080C5050, 0x08002650u, 0x080026E4u },   /* project:cangard, SAME */
	{ 0x080C5054, 0x08002610u, 0x080026A4u },   /* project:cangard, SAME */
	{ 0x080C5058, 0x08002DF4u, 0x08002F90u },   /* project:cangard, SAME */
	{ 0x080C5078, 0x08002BECu, 0x08002D80u },   /* project:cangard, SAME */
	{ 0x080C507C, 0x08016126u, 0x08019892u },   /* project:cangard, DIFF (audited) */
	{ 0x080C508E, 0x0800325Cu, 0x08003458u },   /* project:cangard, SAME */
	{ 0x080C50A4, 0x08016760u, 0x08019FB4u },   /* project:cangard, DIFF (audited) */
	{ 0x080C50AE, 0x08017054u, 0x0801A258u },   /* map:bytes, SAME */
	{ 0x080C50B4, 0x08016C34u, 0x0801A140u },   /* project:cangard, SAME */
	{ 0x080C50B8, 0x08016B1Au, 0x0801A018u },   /* project:cangard, DIFF (audited) */
	{ 0x080C50C8, 0x080167B0u, 0x0801A00Eu },   /* project:cangard, DIFF (audited) */
	{ 0x080C50CC, 0x08016C04u, 0x0801A0D4u },   /* project:cangard, DIFF (audited) */
	{ 0x080C50D0, 0x08008B6Eu, 0x0800A18Eu },   /* project:cangard, SAME */
	{ 0x080C50F8, 0x08008A78u, 0x0800A098u },   /* project:cangard, SAME */
	{ 0x080C5108, 0x08016B1Au, 0x0801A018u },   /* project:cangard, DIFF (audited) */
	{ 0x080C5116, 0x080DFFD2u, 0x080E1932u },   /* project:cangard, SAME */
	{ 0x080C512C, 0x080DFFD2u, 0x080E1932u },   /* project:cangard, SAME */
	{ 0x080C5132, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080C5136, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080C5178, 0x0800C676u, 0x0800E276u },   /* project:cangard, SAME */
	{ 0x080C518A, 0x08001224u, 0x08001288u },   /* map:bytes, SAME */
	{ 0x080C519A, 0x080EA08Cu, SOUL_STUB_NONE },   /* answered by the Souls' C */
	{ 0x080C51C8, 0x08016B90u, 0x0801A07Cu },   /* project:cangard, SAME */
	{ 0x080C51CE, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080C51E6, 0x0800C69Cu, 0x0800E29Cu },   /* project:cangard, SAME */
	{ 0x080C51EA, 0x0800B368u, 0x0800CCA6u },   /* map:calls+bytes, SAME */
	{ 0x080C51F0, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080C51FC, 0x080DFFD2u, 0x080E1932u },   /* project:cangard, SAME */
	{ 0x080C521A, 0x0800C6ACu, 0x0800E2ACu },   /* project:cangard, SAME */
	{ 0x080C521E, 0x08016B60u, 0x0801A04Cu },   /* project:cangard, SAME */
	{ 0x080C5222, 0x0800B328u, 0x0800CC66u },   /* project:cangard, SAME */
	{ 0x080C5228, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080C5244, 0x080EA08Cu, SOUL_STUB_NONE },   /* answered by the Souls' C */
	{ 0x080C5256, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080C529A, 0x080EA0BAu, SOUL_STUB_NONE },   /* answered by the Souls' C */
	{ 0x080C52AA, 0x08002C44u, 0x08002DD8u },   /* map:bytes, SAME */
	{ 0x080C52B8, 0x08002C44u, 0x08002DD8u },   /* map:bytes, SAME */
	{ 0x080C52C6, 0x08002C1Cu, 0x08002DB0u },   /* map:bytes, SAME */
	{ 0x080C52CE, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080C5300, 0x080C4E72u, 0x080C6BDAu },   /* map:bytes, SAME */
	{ 0x080C5312, 0x080C4EAEu, 0x080C6C16u },   /* map:calls+bytes, SAME */
	{ 0x080C532A, 0x08018838u, 0x0801BD3Cu },   /* map:bytes, SAME */
	{ 0x080C5332, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080C534C, 0x08003178u, XV_SPAWN_3 },   /* the second-level dispatch's spawner */
	{ 0x080C5390, 0x0800CE1Eu, 0x0800EB9Eu },   /* map:calls, LAYOUT */
	{ 0x080C5394, 0x0800C7CCu, 0x0800E3BEu },   /* map:calls, DIFF (audited) */
	{ 0x080C53B0, 0x08002C44u, 0x08002DD8u },   /* map:bytes, SAME */
	{ 0x080C53BA, 0x08002C1Cu, 0x08002DB0u },   /* map:bytes, SAME */
	{ 0x080C53CA, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080C6CA6, 0x0800C676u, 0x0800E276u },   /* project:cangard, SAME */
	{ 0x080C6CBA, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080C6CC0, 0x08016126u, 0x08019892u },   /* project:cangard, DIFF (audited) */
	{ 0x080C6CC8, 0x0800325Cu, 0x08003458u },   /* project:cangard, SAME */
	{ 0x080C6CD4, 0x08016760u, 0x08019FB4u },   /* project:cangard, DIFF (audited) */
	{ 0x080C6CDE, 0x08016B1Au, 0x0801A018u },   /* project:cangard, DIFF (audited) */
	{ 0x080C6CEA, 0x080167B0u, 0x0801A00Eu },   /* project:cangard, DIFF (audited) */
	{ 0x080C6CEE, 0x08016C04u, 0x0801A0D4u },   /* project:cangard, DIFF (audited) */
	{ 0x080C6CFA, 0x08016C56u, 0x0801A166u },   /* project:cangard, SAME */
	{ 0x080C6D08, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080C6D1E, 0x08016B1Au, 0x0801A018u },   /* project:cangard, DIFF (audited) */
	{ 0x080C6DBE, 0x08018838u, 0x0801BD3Cu },   /* map:bytes, SAME */
	{ 0x080C6DC4, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080C6DD6, 0x080DFFD2u, 0x080E1932u },   /* project:cangard, SAME */
	{ 0x080C6DE2, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080C6E02, 0x08016B96u, 0x0801A082u },   /* map:calls, DIFF (audited) */
	{ 0x080C6E08, 0x08016634u, SOULS_CODE + 0x4E1u },   /* moved */
	{ 0x080C6E14, 0x08003178u, XV_SPAWN_3 },   /* the second-level dispatch's spawner */
	{ 0x080C6E1E, 0x0800F1EEu, 0x0801155Au },   /* map:bytes, SAME */
	{ 0x080C7662, 0x080187CCu, 0x0801BCD0u },   /* map:bytes, SAME */
	{ 0x080C768A, 0x08002650u, 0x080026E4u },   /* project:cangard, SAME */
	{ 0x080C768E, 0x08002610u, 0x080026A4u },   /* project:cangard, SAME */
	{ 0x080C7692, 0x08002DF4u, 0x08002F90u },   /* project:cangard, SAME */
	{ 0x080C76A6, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080C76AA, 0x08002DC0u, 0x08002F5Cu },   /* project:cangard, SAME */
	{ 0x080C76B6, 0x0800C6ACu, 0x0800E2ACu },   /* project:cangard, SAME */
	{ 0x080C76C8, 0x08016126u, 0x08019892u },   /* project:cangard, DIFF (audited) */
	{ 0x080C76D0, 0x0800325Cu, 0x08003458u },   /* project:cangard, SAME */
	{ 0x080C76DC, 0x08016760u, 0x08019FB4u },   /* project:cangard, DIFF (audited) */
	{ 0x080C76E2, 0x08016C34u, 0x0801A140u },   /* project:cangard, SAME */
	{ 0x080C76E6, 0x08016B1Au, 0x0801A018u },   /* project:cangard, DIFF (audited) */
	{ 0x080C76EC, 0x08017054u, 0x0801A258u },   /* map:bytes, SAME */
	{ 0x080C7708, 0x080167B0u, 0x0801A00Eu },   /* project:cangard, DIFF (audited) */
	{ 0x080C7714, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080C772C, 0x08016B90u, 0x0801A07Cu },   /* project:cangard, SAME */
	{ 0x080C7734, 0x0800B290u, 0x0800CBD8u },   /* map:bytes, SAME */
	{ 0x080C7738, 0x08016B1Au, 0x0801A018u },   /* project:cangard, DIFF (audited) */
	{ 0x080C775A, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080C7768, 0x0800C6ACu, 0x0800E2ACu },   /* project:cangard, SAME */
	{ 0x080C776C, 0x08016B60u, 0x0801A04Cu },   /* project:cangard, SAME */
	{ 0x080C7778, 0x08003178u, XV_SPAWN_3 },   /* the second-level dispatch's spawner */
	{ 0x080C77AE, 0x0800C6C0u, 0x0800E2C0u },   /* map:calls+bytes, SAME */
	{ 0x080CA928, 0x080186A8u, 0x0801BBACu },   /* project:cangard, SAME */
	{ 0x080CA94E, 0x0800F198u, 0x08011504u },   /* map:bytes, SAME */
	{ 0x080CA956, 0x0800325Cu, 0x08003458u },   /* project:cangard, SAME */
	{ 0x080CA95A, 0x0800C69Cu, 0x0800E29Cu },   /* project:cangard, SAME */
	{ 0x080CA980, 0x080167B0u, 0x0801A00Eu },   /* project:cangard, DIFF (audited) */
	{ 0x080CA98E, 0x0800CDECu, 0x0800EB6Cu },   /* map:bytes, SAME */
	{ 0x080CA9A6, 0x080C5386u, SOULS_CODE + 0xE43u },   /* moved */
	{ 0x080CA9BA, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080CA9BE, 0x08008A78u, 0x0800A098u },   /* project:cangard, SAME */
	{ 0x080CA9CE, 0x08016B1Au, 0x0801A018u },   /* project:cangard, DIFF (audited) */
	{ 0x080CAA12, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080CAA16, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080CAA24, 0x080EA0BAu, SOUL_STUB_NONE },   /* answered by the Souls' C */
	{ 0x080CAA5C, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080CAA70, 0x080C4E72u, 0x080C6BDAu },   /* map:bytes, SAME */
	{ 0x080CAA78, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080CAA82, 0x0800C676u, 0x0800E276u },   /* project:cangard, SAME */
	{ 0x080CAA8E, 0x080DFFD2u, 0x080E1932u },   /* project:cangard, SAME */
	{ 0x080CAA9A, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080CAAD4, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080CAAE2, 0x080EA0BAu, SOUL_STUB_NONE },   /* answered by the Souls' C */
	{ 0x080CAB24, 0x080014A6u, 0x08001532u },   /* project:handi, SAME */
	{ 0x080CAB34, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080CAB44, 0x080C6E0Eu, SOULS_CODE + 0x1117u },   /* moved */
	{ 0x080CAB4A, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080CAB56, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080CAB96, 0x0800CDECu, 0x0800EB6Cu },   /* map:bytes, SAME */
	{ 0x080CABD0, 0x08003178u, XV_SPAWN_3 },   /* the second-level dispatch's spawner */
	{ 0x080CABDA, 0x0800F1EEu, 0x0801155Au },   /* map:bytes, SAME */
	{ 0x080D6794, 0x08018720u, 0x0801BC24u },   /* map:bytes, SAME */
	{ 0x080D67B2, 0x0800F198u, 0x08011504u },   /* map:bytes, SAME */
	{ 0x080D67B6, 0x0800C69Cu, 0x0800E29Cu },   /* project:cangard, SAME */
	{ 0x080D67C6, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080D67E6, 0x08017054u, 0x0801A258u },   /* map:bytes, SAME */
	{ 0x080D67EA, 0x08016B1Au, 0x0801A018u },   /* project:cangard, DIFF (audited) */
	{ 0x080D67EE, 0x08002DBEu, 0x08002F5Au },   /* near, SAME */
	{ 0x080D67FE, 0x080167B0u, 0x0801A00Eu },   /* project:cangard, DIFF (audited) */
	{ 0x080D6802, 0x08016C04u, 0x0801A0D4u },   /* project:cangard, DIFF (audited) */
	{ 0x080D680E, 0x0800B328u, 0x0800CC66u },   /* project:cangard, SAME */
	{ 0x080D6816, 0x08008B6Eu, 0x0800A18Eu },   /* project:cangard, SAME */
	{ 0x080D6828, 0x08016B1Au, 0x0801A018u },   /* project:cangard, DIFF (audited) */
	{ 0x080D682E, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080D6884, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080D6894, 0x08003178u, XV_SPAWN_3 },   /* the second-level dispatch's spawner */
	{ 0x080D68A0, 0x0800F1EEu, 0x0801155Au },   /* map:bytes, SAME */
	{ 0x080E36E2, 0x080031CCu, XV_SPAWN_4 },   /* the second-level dispatch's spawner */
	{ 0x080EBDF8, 0x0800CA7Cu, 0x0800E5FCu },   /* map:bytes, SAME */
	{ 0x080EBE02, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EBE0E, 0x08018654u, 0x0801BB1Cu },   /* map:calls, DIFF (audited) */
	{ 0x080EBE14, 0x08010DEEu, SOUL_STUB_FORM },   /* MegaMan's form read: the Soul's number, as BN5 keeps it */
	{ 0x080EBE24, 0x080125CCu, 0x08015B00u },   /* map:bytes, SAME */
	{ 0x080EBE32, 0x08012530u, SOULS_CODE + 0x301u },   /* moved */
	{ 0x080EBE3C, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080EBE46, 0x0800D1C6u, 0x0800F2AAu },   /* map:bytes, SAME */
	{ 0x080EBE4E, 0x0800DA6Au, 0x0800FCE4u },   /* map:bytes, SAME */
	{ 0x080EBE62, 0x080E36DCu, SOULS_CODE + 0x1861u },   /* moved */
	{ 0x080EBE6C, 0x0800C69Cu, 0x0800E29Cu },   /* project:cangard, SAME */
	{ 0x080EBE70, 0x08016B60u, 0x0801A04Cu },   /* project:cangard, SAME */
	{ 0x080EBE82, 0x0800D1E2u, 0x0800F2C6u },   /* map:bytes, SAME */
	{ 0x080EBE86, 0x0801A514u, 0x0801DBD4u },   /* map:bytes, SAME */
	{ 0x080EBE92, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EBED0, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EBED4, 0x0800F114u, 0x08011450u },   /* map:bytes, SAME */
	{ 0x080EBED8, 0x0800DB3Cu, 0x0800FDB6u },   /* map:calls+bytes, SAME */
	{ 0x080EBEEE, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080EBF0E, 0x080B9CAEu, SOULS_CODE + 0x7F3u },   /* moved */
	{ 0x080EBF18, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EBF20, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080EBF34, 0x080EA0BAu, SOUL_STUB_NONE },   /* answered by the Souls' C */
	{ 0x080EBF4A, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080EBF54, 0x0800D1C6u, 0x0800F2AAu },   /* map:bytes, SAME */
	{ 0x080EBF5C, 0x0800DA6Au, 0x0800FCE4u },   /* map:bytes, SAME */
	{ 0x080EBF70, 0x080E36DCu, SOULS_CODE + 0x1861u },   /* moved */
	{ 0x080EBF78, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080EBF8C, 0x080B9CE0u, SOULS_CODE + 0x825u },   /* moved */
	{ 0x080EBFA0, 0x080E36DCu, SOULS_CODE + 0x1861u },   /* moved */
	{ 0x080EBFBA, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080EBFDE, 0x080C4E72u, 0x080C6BDAu },   /* map:bytes, SAME */
	{ 0x080EBFF4, 0x080C4E72u, 0x080C6BDAu },   /* map:bytes, SAME */
	{ 0x080EBFFC, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080EC00A, 0x0800C676u, 0x0800E276u },   /* project:cangard, SAME */
	{ 0x080EC016, 0x080DFFD2u, 0x080E1932u },   /* project:cangard, SAME */
	{ 0x080EC14E, 0x08010DEEu, SOUL_STUB_FORM },   /* MegaMan's form read: the Soul's number, as BN5 keeps it */
	{ 0x080EC196, 0x08010DEEu, SOUL_STUB_FORM },   /* MegaMan's form read: the Soul's number, as BN5 keeps it */
	{ 0x080EC1CE, 0x08002C56u, 0x08002DEAu },   /* project:cangard, SAME */
	{ 0x080EC1E6, 0x0800D1E2u, 0x0800F2C6u },   /* map:bytes, SAME */
	{ 0x080EC1EA, 0x0801A514u, 0x0801DBD4u },   /* map:bytes, SAME */
	{ 0x080EC1F6, 0x08018676u, 0x0801BB46u },   /* map:calls, LAYOUT */
	{ 0x080EC1FA, 0x08016B60u, 0x0801A04Cu },   /* project:cangard, SAME */
	{ 0x080EC1FE, 0x0800C69Cu, 0x0800E29Cu },   /* project:cangard, SAME */
	{ 0x080EE88A, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EE890, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080EE894, 0x0800DB3Cu, 0x0800FDB6u },   /* map:calls+bytes, SAME */
	{ 0x080EE89C, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EE8AE, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080EE8CA, 0x08001490u, 0x0800151Cu },   /* map:calls+bytes, SAME */
	{ 0x080EE8EC, 0x080C5346u, SOULS_CODE + 0xE03u },   /* moved */
	{ 0x080EE932, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EE936, 0x0800F29Eu, 0x08011714u },   /* project:cangard, DIFF (audited) */
	{ 0x080EEADE, 0x0800DD5Eu, 0x0800FFFEu },   /* map:calls+bytes, SAME */
	{ 0x080EEAEE, 0x0800E338u, SOUL_STUB_COUNTER },   /* answered by the Souls' C */
	{ 0x080EEC06, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EEC0A, 0x0800DB3Cu, 0x0800FDB6u },   /* map:calls+bytes, SAME */
	{ 0x080EEC12, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EEC28, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080EEC30, 0x080C7772u, SOULS_CODE + 0x1253u },   /* moved */
	{ 0x080EEC6C, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EEC80, 0x0800F29Eu, 0x08011714u },   /* project:cangard, DIFF (audited) */
	{ 0x080EEDB8, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EEDBE, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EEDC4, 0x080EC16Au, SOULS_CODE + 0x1C27u },   /* moved */
	{ 0x080EEDCA, 0x080EC120u, SOULS_CODE + 0x1BDDu },   /* moved */
	{ 0x080EEDD6, 0x080B9CAEu, SOULS_CODE + 0x7F3u },   /* moved */
	{ 0x080EEDDC, 0x0800DB3Cu, 0x0800FDB6u },   /* map:calls+bytes, SAME */
	{ 0x080EEDE2, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080EEE0A, 0x080C4E72u, 0x080C6BDAu },   /* map:bytes, SAME */
	{ 0x080EEE10, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080EEE20, 0x0800C676u, 0x0800E276u },   /* project:cangard, SAME */
	{ 0x080EEE2A, 0x080DFFD2u, 0x080E1932u },   /* project:cangard, SAME */
	{ 0x080EEE3A, 0x08016C4Cu, 0x0801A15Cu },   /* project:cangard, SAME */
	{ 0x080EEE7C, 0x0800F29Eu, 0x08011714u },   /* project:cangard, DIFF (audited) */
	{ 0x080EEF92, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EEF96, 0x0800DB3Cu, 0x0800FDB6u },   /* map:calls+bytes, SAME */
	{ 0x080EEF9E, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080EEFA6, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EEFBC, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080EEFCE, 0x080C4E72u, 0x080C6BDAu },   /* map:bytes, SAME */
	{ 0x080EEFD6, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080EEFE0, 0x0800C676u, 0x0800E276u },   /* project:cangard, SAME */
	{ 0x080EEFEC, 0x080DFFD2u, 0x080E1932u },   /* project:cangard, SAME */
	{ 0x080EF02C, 0x0800F29Eu, 0x08011714u },   /* project:cangard, DIFF (audited) */
	{ 0x080EF6AE, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EF6B2, 0x0800DB3Cu, 0x0800FDB6u },   /* map:calls+bytes, SAME */
	{ 0x080EF6BE, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EF6CA, 0x080B9CAEu, SOULS_CODE + 0x7F3u },   /* moved */
	{ 0x080EF6D4, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080EF6D8, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080EF6EA, 0x080D688Eu, SOULS_CODE + 0x1807u },   /* moved */
	{ 0x080EF720, 0x08016C4Cu, 0x0801A15Cu },   /* project:cangard, SAME */
	{ 0x080EF72C, 0x0800F29Eu, 0x08011714u },   /* project:cangard, DIFF (audited) */
};
static const XvImm imms[] = {
	{ 0x0800F68A, THUMB_MOVS, 0, 0, 0x3A, 0xC1 },   /* its family: CurAction 0x3a in BN5, the Souls' 0xc1 in BN6 (xattack_grow.h) */
	{ 0x0800F7CA, THUMB_MOVS, 0, 0, 0x40, 0xC2 },   /* its family: CurAction 0x40 in BN5, the Souls' 0xc2 in BN6 (xattack_grow.h) */
	{ 0x0800F7F8, THUMB_MOVS, 0, 0, 0x43, 0xC0 },   /* its family: CurAction 0x43 in BN5, the Souls' 0xc0 in BN6 (xattack_grow.h) */
	{ 0x0800F83C, THUMB_MOVS, 0, 0, 0x13, 0xC6 },   /* its family: CurAction 0x13 in BN5, the Souls' 0xc6 in BN6 (xattack_grow.h) */
	{ 0x0800F87C, THUMB_MOVS, 0, 0, 0x45, 0xC5 },   /* its family: CurAction 0x45 in BN5, the Souls' 0xc5 in BN6 (xattack_grow.h) */
	{ 0x0800FA10, THUMB_MOVS, 0, 0, 0x50, 0xC3 },   /* its family: CurAction 0x50 in BN5, the Souls' 0xc3 in BN6 (xattack_grow.h) */
	{ 0x0800FC00, THUMB_MOVS, 0, 0, 0x3D, 0xC4 },   /* its family: CurAction 0x3d in BN5, the Souls' 0xc4 in BN6 (xattack_grow.h) */
	{ 0x08012066, THUMB_MOVS, 4, 0, 0x3, 0xC5 },   /* its effect: 0x3 in BN5, 0xc5 in BN6 */
	{ 0x080C504A, THUMB_MOVS, 1, 0, 0xC, 0x0 },   /* its sprite's list: 3 in BN5, 0 in BN6 */
	{ 0x080C504C, THUMB_MOVS, 2, 0, 0xA, 0x51 },   /* its sprite: list 3 0xa in BN5, list 0 0x51 in BN6 */
	{ 0x080C50EC, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C5114, THUMB_MOVS, 4, 0, 0x14, 0xC6 },   /* its effect: 0x14 in BN5, 0xc6 in BN6 */
	{ 0x080C5130, THUMB_MOVS, 0, 0, 0x70, 0x5F },   /* its sound: song 0x70 in BN5, BN6's slot 0x5f (soulpower.c copies it) */
	{ 0x080C51FA, THUMB_MOVS, 4, 0, 0x2, 0xC7 },   /* its effect: 0x2 in BN5, 0xc7 in BN6 */
	{ 0x080C5252, THUMB_MOVS, 0, 0, 0x16, 0x5A },   /* its sound: song 0x115 in BN5 (movs, adds 0xFF), BN6's slot 0x5a */
	{ 0x080C5254, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* ... its adds made none (soulpower.c copies it) */
	{ 0x080C6CF4, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C7684, THUMB_MOVS, 1, 0, 0xC, 0x0 },   /* its sprite's list: 3 in BN5, 0 in BN6 */
	{ 0x080C7686, THUMB_MOVS, 2, 0, 0x5, 0x53 },   /* its sprite: list 3 0x5 in BN5, list 0 0x53 in BN6 */
	{ 0x080C770E, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CAA7E, THUMB_ADDS, 4, 0, 0x17, 0xC3 },   /* its effect: 0x17 in BN5, 0xc3 in BN6 */
	{ 0x080D6808, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080EEE18, THUMB_ADDS, 0, 0, 0x48, 0xC0 },   /* its effects 0x48-0x4a in BN5, 0xc0-0xc2 in BN6 */
	{ 0x080EEFDC, THUMB_ADDS, 4, 0, 0x45, 0xC4 },   /* its effect: 0x45 in BN5, 0xc4 in BN6 */
	{ 0x080EF6D0, THUMB_MOVS, 0, 0, 0x18, 0x5E },   /* its sound: song 0x117 in BN5 (movs, adds 0xFF), BN6's slot 0x5e */
	{ 0x080EF6D2, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* ... its adds made none (soulpower.c copies it) */
};
/* the object types its code spawns through the second-level dispatch: BN6's twin, or moved (its routine BN5's) */
static const SoulType types[] = {
	{ 1, 0x05, XV_TYPE_MOVED, 0x080B9B81u },
	{ 1, 0x54, XV_TYPE_MOVED, 0x080C33ADu },
	{ 3, 0x05, XV_TYPE_MOVED, 0x080C5015u },
	{ 3, 0x12, XV_TYPE_MOVED, 0x080C6C81u },
	{ 3, 0x17, XV_TYPE_MOVED, 0x080C7651u },
	{ 3, 0x30, XV_TYPE_MOVED, 0x080CA91Du },
	{ 3, 0x81, XV_TYPE_MOVED, 0x080D6789u },
	{ 4, 0x28, XV_TYPE_MOVED, 0x080E3619u },
};
/* BN5's sprites its code draws that BN6 has none of by their bytes: copied into SOULS_ART (past it FREE_SOULS_ART), listed at a number of a grown list
 * (list 0 the Souls' own; another's list a number of the track tools/bn5_objects.py names, whose copy serves where it stands first) */
static const XvSprite sprites[] = {
	{ 4, 0x1F, 0, 0x4B, SOULS_ART + 0x0, 0x1400 },
	{ 3, 0x1F, 0, 0x4E, SOULS_ART + 0x1400, 0x400 },
	{ 3, 0x14, 0, 0x4F, SOULS_ART + 0x1800, 0x2500 },
	{ 4, 0x01, 0, 0x50, SOULS_ART + 0x3D00, 0x500 },
	{ 3, 0x0A, 0, 0x51, SOULS_ART + 0x4200, 0x1700 },
	{ 3, 0x15, 0, 0x52, SOULS_ART + 0x5900, 0x1300 },
	{ 3, 0x05, 0, 0x53, SOULS_ART + 0x6C00, 0x500 },
	{ 5, 0x03, 0, 0x54, SOULS_ART + 0x7100, 0x3000 },
	{ 5, 0x0D, 0, 0x55, SOULS_ART + 0xA100, 0x1200 },
	{ 5, 0x02, 0, 0x56, SOULS_ART + 0xB300, 0x1300 },
	{ 5, 0x0F, 0, 0x57, SOULS_ART + 0xC600, 0x1300 },
	{ 3, 0x03, 5, 0xA6, FREE_SOULS_ART + 0x0, 0x2000 },
	{ 3, 0x00, 4, 0xB5, FREE_SOULS_ART + 0x2000, 0x5600 },
};
/* BN5's effects its code spawns that BN6 has none of: at an id of the grown table, BN5's entry with its sprite as listed above */
static const SoulEffect effects[] = {
	{ 0x48, 0xC0, 0, 0x4B, 0x00, 0x00 },
	{ 0x49, 0xC1, 0, 0x4B, 0x01, 0x00 },
	{ 0x4A, 0xC2, 0, 0x4B, 0x02, 0x00 },
	{ 0x17, 0xC3, 0, 0x4F, 0x01, 0x00 },
	{ 0x45, 0xC4, 0, 0x52, 0x01, 0x00 },
	{ 0x03, 0xC5, 0, 0x54, 0x00, 0x00 },
	{ 0x14, 0xC6, 0, 0x55, 0x02, 0x00 },
	{ 0x02, 0xC7, 0, 0x56, 0x01, 0x00 },
	{ 0x20, 0xC8, 0, 0x57, 0x00, 0x00 },
};
/* bytes of its raw tables BN6 numbers otherwise */
static const XvByte bytes[] = {
	{ 0x080B9A6B, 0x0C, 0x10 },   /* the arm's flash kind's sprite's list (3): 3 in BN5, 4 in BN6 */
	{ 0x080B9A6C, 0x00, 0xB5 },   /* the arm's flash kind's sprite (3): list 3 0x0 in BN5, list 4 0xb5 in BN6 */
	{ 0x080B9A8E, 0x0C, 0x10 },   /* the arm's flash kind's sprite's list (10): 3 in BN5, 4 in BN6 */
	{ 0x080B9A8F, 0x00, 0xB5 },   /* the arm's flash kind's sprite (10): list 3 0x0 in BN5, list 4 0xb5 in BN6 */
	{ 0x080B9B0B, 0x0C, 0x14 },   /* the arm's flash kind's sprite's list (35): 3 in BN5, 5 in BN6 */
	{ 0x080B9B0C, 0x03, 0xA6 },   /* the arm's flash kind's sprite (35): list 3 0x3 in BN5, list 5 0xa6 in BN6 */
	{ 0x080B9B51, 0x0C, 0x00 },   /* the arm's flash kind's sprite's list (49): 3 in BN5, 0 in BN6 */
	{ 0x080B9B52, 0x1F, 0x4E },   /* the arm's flash kind's sprite (49): list 3 0x1f in BN5, list 0 0x4e in BN6 */
	{ 0x080B9B56, 0x0C, 0x10 },   /* the arm's flash kind's sprite's list (50): 3 in BN5, 4 in BN6 */
	{ 0x080B9B57, 0x00, 0xB5 },   /* the arm's flash kind's sprite (50): list 3 0x0 in BN5, list 4 0xb5 in BN6 */
	{ 0x080B9B5B, 0x0C, 0x14 },   /* the arm's flash kind's sprite's list (51): 3 in BN5, 5 in BN6 */
	{ 0x080B9B5C, 0x03, 0xA6 },   /* the arm's flash kind's sprite (51): list 3 0x3 in BN5, list 5 0xa6 in BN6 */
	{ 0x080C33AA, 0x10, 0x00 },   /* NumberSoul's companion's sprite's list (1): 4 in BN5, 0 in BN6 */
	{ 0x080C33AB, 0x01, 0x50 },   /* NumberSoul's companion's sprite (1): list 4 0x1 in BN5, list 0 0x50 in BN6 */
	{ 0x080C6C52, 0x20, 0xC8 },   /* its effect for the obstacle strike's shot (its parameter 12, 0x080CAB40: the third kind's 0x12's table 0x080C6C08, 6 bytes an entry, +2 its effect): 0x20 in BN5, 0xc8 in BN6 */
	{ 0x080EC112, 0x17, 0xC3 },   /* its effect for ShadowSoul's charged shot (sub-family 2): 0x17 in BN5, 0xc3 in BN6 */
};
/* its reads of BN5's AI data's Soul fields, which BN6's AI data holds otherwise: the Souls' C answers from BN6_SOULS */
static const SoulField fields[] = {
	{ 0x0800F7D4, 0, 2, BN6_SOUL_CHIP },   /* AI data +0x32 */
	{ 0x080EC18E, 0, 1, BN6_SOUL_CHAOS },   /* AI data +0x12 */
};
/* the power attacks a Soul's loadout names, BN5's ids, each its entry in BN5 (moved) */
static const SoulPower powers[] = {
	{ 0x14, 0x0800F7DDu },   /* ColonelSoul's charged shot */
	{ 0x13, 0x0800F7D1u },   /* ColonelSoul's chip shot */
	{ 0x17, 0x0800F819u },   /* ShadowSoul's charged shot */
	{ 0x45, 0x0800FBE7u },   /* ShadowSoul's B+Left */
	{ 0x07, 0x0800F66Bu },   /* NumberSoul's charged shot */
	{ 0x1A, 0x0800F865u },   /* TomahawkSoul's charged shot */
	{ 0x11, 0x0800F79Fu },   /* KnightSoul's charged shot */
	{ 0x2D, 0x0800F9E7u },   /* ToadSoul's charged shot */
};
/* BN5's songs its code plays that Gregar has otherwise: copied into an empty slot (soulpower.c) */
static const SoulSong songs[] = {
	{ 0x70, 0x5F },
	{ 0x117, 0x5E },
	{ 0x115, 0x5A },
};
/* the families moved: BN5's CurAction, BN6's, BN5's entry */
static const SoulFamily families[] = {
	{ 0x43, 0xC0, 0x080EED93u },
	{ 0x3A, 0xC1, 0x080EE865u },
	{ 0x40, 0xC2, 0x080EEBDFu },
	{ 0x50, 0xC3, 0x080EF68Bu },
	{ 0x3D, 0xC4, 0x080EEADDu },
	{ 0x45, 0xC5, 0x080EEF6Du },
	{ 0x13, 0xC6, 0x080EBDC7u },
};

#define N(a) (int)(sizeof (a) / sizeof *(a))
const SoulCode soulcode_bn5 = {
	blocks, N(blocks), data, N(data), words, N(words), calls, N(calls), imms, N(imms), types, N(types), sprites, N(sprites), effects, N(effects), bytes, N(bytes), fields, N(fields), powers, N(powers), songs, N(songs), families, N(families),
};
