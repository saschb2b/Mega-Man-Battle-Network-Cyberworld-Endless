/* soulcode_bn5.c: the code BN5's Team Colonel Souls run, moved into BN6's battle engine, written by tools/xsoul.py from
 * the two ROMs (python3 tools/xsoul.py; --check says whether this is what it writes). Do not edit: change the generator
 * and write it again. Numbers alone (docs/multirom/souls.md): addresses in BN5 Team Colonel (USA) and BN6
 * Cybeast Gregar (USA), counts and the CRC32s of the ranges src/emu/soulcode.c copies from the player's ROM. */
#include "soulcode.h"

#include <stddef.h>

#include "bn6.h"
#include "soul_room.h"
#include "xchips_code.h"
#include "xnavi_recipe.h"
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
	{ 0x08016B96, 0x08016BBC, SOULS_CODE + 0x4E2, 0xA53A3BEFu },   /* shadow */
	{ 0x080236BC, 0x08023736, SOULS_CODE + 0x510, 0x10F05CF6u },   /* colonel */
	{ 0x080254F8, 0x08025524, SOULS_CODE + 0x590, 0x9749B90Du },   /* colonel */
	{ 0x080CA91C, 0x080CAD48, SOULS_CODE + 0x5C0, 0xD263FB4Fu },   /* colonel */
	{ 0x080EBDC6, 0x080EC234, SOULS_CODE + 0x9F2, 0x51FA1AC5u },   /* shadow */
	{ 0x080EE864, 0x080EE93C, SOULS_CODE + 0xE60, 0xE0262102u },   /* number */
	{ 0x080EEADC, 0x080EEB0C, SOULS_CODE + 0xF40, 0x560AFA29u },   /* shadow */
	{ 0x080EEBDE, 0x080EEC86, SOULS_CODE + 0xF72, 0x60A893B6u },   /* knight */
	{ 0x080EED92, 0x080EEE82, SOULS_CODE + 0x1022, 0xD56BB4A7u },   /* colonel */
	{ 0x080EEF6C, 0x080EF032, SOULS_CODE + 0x1120, 0x90EBB3BDu },   /* tomahawk */
	{ 0x080EF68A, 0x080EF732, SOULS_CODE + 0x11F2, 0x8EE8BB09u },   /* toad */
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
	{ 0x080CA930, 0x080CA934u, SOULS_CODE + 0x5D8u },   /* moved with the code */
	{ 0x080CA934, 0x080CA941u, SOULS_CODE + 0x5E5u },   /* a jump table's entry, moved */
	{ 0x080CA938, 0x080CA97Fu, SOULS_CODE + 0x623u },   /* a jump table's entry, moved */
	{ 0x080CA93C, 0x080138EBu, 0x08016C8Bu },   /* a jump table's entry, BN6's twin (project:cangard) */
	{ 0x080CA9D4, 0x080CA9D8u, SOULS_CODE + 0x67Cu },   /* moved with the code */
	{ 0x080CA9D8, 0x080CA9E5u, SOULS_CODE + 0x689u },   /* a jump table's entry, moved */
	{ 0x080CA9DC, 0x080CAAA9u, SOULS_CODE + 0x74Du },   /* a jump table's entry, moved */
	{ 0x080CA9E0, 0x080CAB71u, SOULS_CODE + 0x815u },   /* a jump table's entry, moved */
	{ 0x080CA9F4, 0x080CA9F8u, SOULS_CODE + 0x69Cu },   /* moved with the code */
	{ 0x080CA9F8, 0x080CAA01u, SOULS_CODE + 0x6A5u },   /* a jump table's entry, moved */
	{ 0x080CA9FC, 0x080CAA3Bu, SOULS_CODE + 0x6DFu },   /* a jump table's entry, moved */
	{ 0x080CAAB8, 0x080CAABCu, SOULS_CODE + 0x760u },   /* moved with the code */
	{ 0x080CAABC, 0x080CAAC5u, SOULS_CODE + 0x769u },   /* a jump table's entry, moved */
	{ 0x080CAAC0, 0x080CAAF9u, SOULS_CODE + 0x79Du },   /* a jump table's entry, moved */
	{ 0x080CAB68, 0x080CAB6Cu, SOULS_CODE + 0x810u },   /* data moved with the code */
	{ 0x080CAD2C, 0x01001020u, 0x01000057u },   /* its sprite: list 4 0x20 in BN5, list 0 0x57 in BN6 (a packed word) */
	{ 0x080CAD30, 0x01031020u, 0x01030057u },   /* its sprite: list 4 0x20 in BN5, list 0 0x57 in BN6 (a packed word) */
	{ 0x080CAD34, 0x13140300u, 0x13140300u },   /* a number */
	{ 0x080CAD38, 0x0705FF02u, 0x0705FF02u },   /* I/O or video, the same */
	{ 0x080CAD3C, 0x02034000u, 0x0203FF60u },   /* BN5's per-side words of ColonelSoul's obstacle strike (0x080CACE0, 8 bytes a side): the RAM plan's RAM_COLONEL (free_space.h), which BN6 leaves alone */
	{ 0x080CAD40, 0x02034000u, 0x0203FF60u },   /* BN5's per-side words of ColonelSoul's obstacle strike (0x080CACE0, 8 bytes a side): the RAM plan's RAM_COLONEL (free_space.h), which BN6 leaves alone */
	{ 0x080CAD44, 0x02034000u, 0x0203FF60u },   /* BN5's per-side words of ColonelSoul's obstacle strike (0x080CACE0, 8 bytes a side): the RAM plan's RAM_COLONEL (free_space.h), which BN6 leaves alone */
	{ 0x080EBDD4, 0x080EBDD8u, SOULS_CODE + 0xA04u },   /* moved with the code */
	{ 0x080EBDD8, 0x080EBDEDu, SOULS_CODE + 0xA19u },   /* a jump table's entry, moved */
	{ 0x080EBDDC, 0x080EBEADu, 0x080ECB8Du },   /* a jump table's entry, BN6's twin (search, by place) */
	{ 0x080EBDE0, 0x080EBEC3u, SOULS_CODE + 0xAEFu },   /* a jump table's entry, moved */
	{ 0x080EBDE4, 0x080EC1CDu, SOULS_CODE + 0xDF9u },   /* a jump table's entry, moved */
	{ 0x080EBDE8, 0x080EC209u, 0x080ECF23u },   /* a jump table's entry, BN6's twin (search, by place) */
	{ 0x080EBEA4, 0x0000FFE0u, 0x0000FFE0u },   /* a number */
	{ 0x080EBEA8, 0x01010014u, 0x01010014u },   /* a number */
	{ 0x080EC050, 0x080EC074u, SOULS_CODE + 0xCA0u },   /* data moved with the code */
	{ 0x080EC054, 0x080EC110u, SOULS_CODE + 0xD3Cu },   /* data moved with the code */
	{ 0x080EC058, 0x080EC05Cu, SOULS_CODE + 0xC88u },   /* data moved with the code */
	{ 0x080EC05C, 0x080EC0A8u, SOULS_CODE + 0xCD4u },   /* a pointer in a table of its data, moved */
	{ 0x080EC060, 0x080EC0DCu, SOULS_CODE + 0xD08u },   /* a pointer in a table of its data, moved */
	{ 0x080EC064, 0x0705FF01u, 0x0705FF01u },   /* I/O or video, the same */
	{ 0x080EC068, 0x0000FFE0u, 0x0000FFE0u },   /* a number */
	{ 0x080EC06C, 0x0101001Eu, 0x0101001Eu },   /* a number */
	{ 0x080EC070, 0x0000001Eu, 0x0000001Eu },   /* a number */
	{ 0x080EE874, 0x080EE878u, SOULS_CODE + 0xE74u },   /* moved with the code */
	{ 0x080EE878, 0x080EE881u, SOULS_CODE + 0xE7Du },   /* a jump table's entry, moved */
	{ 0x080EE87C, 0x080EE919u, SOULS_CODE + 0xF15u },   /* a jump table's entry, moved */
	{ 0x080EE904, 0x080EE908u, SOULS_CODE + 0xF04u },   /* data moved with the code */
	{ 0x080EEB00, 0x080EEB08u, SOULS_CODE + 0xF6Cu },   /* moved with the code */
	{ 0x080EEB04, 0x00000600u, 0x00000600u },   /* a number */
	{ 0x080EEB08, 0x080EEB0Du, 0x080F11EDu },   /* a jump table's entry, BN6's twin (map:bytes) */
	{ 0x080EEBEC, 0x080EEBF0u, SOULS_CODE + 0xF84u },   /* moved with the code */
	{ 0x080EEBF0, 0x080EEBFDu, SOULS_CODE + 0xF91u },   /* a jump table's entry, moved */
	{ 0x080EEBF4, 0x080EEC45u, SOULS_CODE + 0xFD9u },   /* a jump table's entry, moved */
	{ 0x080EEBF8, 0x080EEC63u, SOULS_CODE + 0xFF7u },   /* a jump table's entry, moved */
	{ 0x080EEDA0, 0x080EEDA4u, SOULS_CODE + 0x1034u },   /* moved with the code */
	{ 0x080EEDA4, 0x080EEDADu, SOULS_CODE + 0x103Du },   /* a jump table's entry, moved */
	{ 0x080EEDA8, 0x080EEE5Du, SOULS_CODE + 0x10EDu },   /* a jump table's entry, moved */
	{ 0x080EEE44, 0x080EEE4Cu, SOULS_CODE + 0x10DCu },   /* data moved with the code */
	{ 0x080EEF7C, 0x080EEF80u, SOULS_CODE + 0x1134u },   /* moved with the code */
	{ 0x080EEF80, 0x080EEF89u, SOULS_CODE + 0x113Du },   /* a jump table's entry, moved */
	{ 0x080EEF84, 0x080EF00Du, SOULS_CODE + 0x11C1u },   /* a jump table's entry, moved */
	{ 0x080EF008, 0x0705FF11u, 0x0705FF11u },   /* I/O or video, the same */
	{ 0x080EF698, 0x080EF69Cu, SOULS_CODE + 0x1204u },   /* moved with the code */
	{ 0x080EF69C, 0x080EF6A5u, SOULS_CODE + 0x120Du },   /* a jump table's entry, moved */
	{ 0x080EF6A0, 0x080EF705u, SOULS_CODE + 0x126Du },   /* a jump table's entry, moved */
};
/* every call out of its block: its site, BN5's routine, BN6's in its place */
static const XvNumber calls[] = {
	{ 0x0800F078, 0x080C34A6u, XN_CODE + 0x87FF },   /* the navis' copy of a type's module (TYPES) */
	{ 0x0800F0DA, 0x080C34D2u, XN_CODE + 0x882B },   /* the navis' copy of a type's module (TYPES) */
	{ 0x0800F670, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F7A4, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F7D6, 0x0800FE70u, 0x080126EAu },   /* soul:the chip writer's second entry: push {r4, DIFF (the Souls' audit) */
	{ 0x0800F7E2, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F7F0, 0x080BA634u, XC_CODE + 0xA8CD },   /* the chips' copy of a type's module (TYPES) */
	{ 0x0800F832, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F876, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800F9F8, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x0800FA0A, 0x080BCA14u, XC_CODE + 0xB3B5 },   /* the chips' copy of a type's module (TYPES) */
	{ 0x08011C42, 0x080CAD06u, SOUL_STUB_STRIKE },   /* answered by the Souls' C */
	{ 0x08011C4E, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x08011C5E, 0x0800FE56u, 0x08012642u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x08011C72, 0x080CACE0u, SOULS_CODE + 0x985u },   /* moved */
	{ 0x08011C7C, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x08011C80, 0x0800E510u, 0x080107C0u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x08011C88, 0x08016B96u, SOULS_CODE + 0x4E3u },   /* moved */
	{ 0x08011C94, 0x080E22C4u, 0x080E33A8u },   /* map:bytes, SAME */
	{ 0x08011CAE, 0x0800E096u, 0x08010312u },   /* map:bytes, SAME */
	{ 0x08011D2E, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x08011D32, 0x0800E510u, 0x080107C0u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x08011D3A, 0x08016B96u, SOULS_CODE + 0x4E3u },   /* moved */
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
	{ 0x08016BA4, 0x08016BBCu, 0x0801A0BAu },   /* search, SAME */
	{ 0x08016BAE, 0x08016BBCu, 0x0801A0BAu },   /* search, SAME */
	{ 0x08016BB6, 0x080165E4u, 0x08019F44u },   /* project:families, DIFF (audited) */
	{ 0x080236C6, 0x0801DF0Cu, 0x08021AA4u },   /* map:bytes, SAME */
	{ 0x080236D0, 0x080009E8u, 0x08000AC8u },   /* map:bytes, SAME */
	{ 0x080236FC, 0x080254F8u, SOULS_CODE + 0x591u },   /* moved */
	{ 0x0802370A, 0x08024004u, 0x080281D4u },   /* map:bytes, DIFF (the Souls' audit) */
	{ 0x08023714, 0x080258E8u, 0x08029CD4u },   /* soul:a pick's slot drawn on the Custom screen, DIFF (the Souls' audit) */
	{ 0x08023728, 0x08024A2Au, 0x08028E32u },   /* map:calls, DIFF (the Souls' audit) */
	{ 0x080254FC, 0x08023BE0u, 0x08027E1Cu },   /* map:bytes, SAME */
	{ 0x08025514, 0x080089D8u, 0x08009FF8u },   /* map:bytes, SAME */
	{ 0x080CA928, 0x080186A8u, 0x0801BBACu },   /* project:cangard, SAME */
	{ 0x080CA94E, 0x0800F198u, 0x08011504u },   /* map:bytes, SAME */
	{ 0x080CA956, 0x0800325Cu, 0x08003458u },   /* project:cangard, SAME */
	{ 0x080CA95A, 0x0800C69Cu, 0x0800E29Cu },   /* project:cangard, SAME */
	{ 0x080CA980, 0x080167B0u, 0x0801A00Eu },   /* project:cangard, DIFF (audited) */
	{ 0x080CA98E, 0x0800CDECu, 0x0800EB6Cu },   /* map:bytes, SAME */
	{ 0x080CA9A6, 0x080C5386u, XN_CODE + 0x8C73 },   /* the navis' copy of a type's module (TYPES) */
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
	{ 0x080CAB44, 0x080C6E0Eu, XC_CODE + 0x9187 },   /* the chips' copy of a type's module (TYPES) */
	{ 0x080CAB4A, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080CAB56, 0x08016B88u, 0x0801A074u },   /* project:cangard, SAME */
	{ 0x080CAB96, 0x0800CDECu, 0x0800EB6Cu },   /* map:bytes, SAME */
	{ 0x080CABD0, 0x08003178u, XV_SPAWN_3 },   /* the second-level dispatch's spawner */
	{ 0x080CABDA, 0x0800F1EEu, 0x0801155Au },   /* map:bytes, SAME */
	{ 0x080EBDF8, 0x0800CA7Cu, 0x0800E5FCu },   /* map:bytes, SAME */
	{ 0x080EBE02, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EBE0E, 0x08018654u, 0x0801BB1Cu },   /* map:calls, DIFF (audited) */
	{ 0x080EBE14, 0x08010DEEu, SOUL_STUB_FORM },   /* MegaMan's form read: the Soul's number, as BN5 keeps it */
	{ 0x080EBE24, 0x080125CCu, 0x08015B00u },   /* map:bytes, SAME */
	{ 0x080EBE32, 0x08012530u, SOULS_CODE + 0x301u },   /* moved */
	{ 0x080EBE3C, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080EBE46, 0x0800D1C6u, 0x0800F2AAu },   /* map:bytes, SAME */
	{ 0x080EBE4E, 0x0800DA6Au, 0x0800FCE4u },   /* map:bytes, SAME */
	{ 0x080EBE62, 0x080E36DCu, XC_CODE + 0x35A5 },   /* the chips' copy of a type's module (TYPES) */
	{ 0x080EBE6C, 0x0800C69Cu, 0x0800E29Cu },   /* project:cangard, SAME */
	{ 0x080EBE70, 0x08016B60u, 0x0801A04Cu },   /* project:cangard, SAME */
	{ 0x080EBE82, 0x0800D1E2u, 0x0800F2C6u },   /* map:bytes, SAME */
	{ 0x080EBE86, 0x0801A514u, 0x0801DBD4u },   /* map:bytes, SAME */
	{ 0x080EBE92, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EBED0, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EBED4, 0x0800F114u, 0x08011450u },   /* map:bytes, SAME */
	{ 0x080EBED8, 0x0800DB3Cu, 0x0800FDB6u },   /* map:calls+bytes, SAME */
	{ 0x080EBEEE, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080EBF0E, 0x080B9CAEu, XN_OBJECTS + 0x253 },   /* the navis' copy of a type's module (TYPES) */
	{ 0x080EBF18, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EBF20, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080EBF34, 0x080EA0BAu, SOUL_STUB_NONE },   /* answered by the Souls' C */
	{ 0x080EBF4A, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080EBF54, 0x0800D1C6u, 0x0800F2AAu },   /* map:bytes, SAME */
	{ 0x080EBF5C, 0x0800DA6Au, 0x0800FCE4u },   /* map:bytes, SAME */
	{ 0x080EBF70, 0x080E36DCu, XC_CODE + 0x35A5 },   /* the chips' copy of a type's module (TYPES) */
	{ 0x080EBF78, 0x0800C88Eu, 0x0800E456u },   /* project:cangard, SAME */
	{ 0x080EBF8C, 0x080B9CE0u, XN_OBJECTS + 0x285 },   /* the navis' copy of a type's module (TYPES) */
	{ 0x080EBFA0, 0x080E36DCu, XC_CODE + 0x35A5 },   /* the chips' copy of a type's module (TYPES) */
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
	{ 0x080EE8EC, 0x080C5346u, XN_CODE + 0x8C33 },   /* the navis' copy of a type's module (TYPES) */
	{ 0x080EE932, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EE936, 0x0800F29Eu, 0x08011714u },   /* project:cangard, DIFF (audited) */
	{ 0x080EEADE, 0x0800DD5Eu, 0x0800FFFEu },   /* map:calls+bytes, SAME */
	{ 0x080EEAEE, 0x0800E338u, SOUL_STUB_COUNTER },   /* answered by the Souls' C */
	{ 0x080EEC06, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EEC0A, 0x0800DB3Cu, 0x0800FDB6u },   /* map:calls+bytes, SAME */
	{ 0x080EEC12, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EEC28, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080EEC30, 0x080C7772u, XN_CODE + 0x10523 },   /* the navis' copy of a type's module (TYPES) */
	{ 0x080EEC6C, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EEC80, 0x0800F29Eu, 0x08011714u },   /* project:cangard, DIFF (audited) */
	{ 0x080EEDB8, 0x08016C42u, 0x0801A152u },   /* project:cangard, SAME */
	{ 0x080EEDBE, 0x0800D1D2u, 0x0800F2B6u },   /* map:bytes, SAME */
	{ 0x080EEDC4, 0x080EC16Au, SOULS_CODE + 0xD97u },   /* moved */
	{ 0x080EEDCA, 0x080EC120u, SOULS_CODE + 0xD4Du },   /* moved */
	{ 0x080EEDD6, 0x080B9CAEu, XN_OBJECTS + 0x253 },   /* the navis' copy of a type's module (TYPES) */
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
	{ 0x080EF6CA, 0x080B9CAEu, XN_OBJECTS + 0x253 },   /* the navis' copy of a type's module (TYPES) */
	{ 0x080EF6D4, 0x08000598u, 0x080005CCu },   /* project:cangard, SAME */
	{ 0x080EF6D8, 0x0800C6CAu, 0x0800E2CAu },   /* project:cangard, SAME */
	{ 0x080EF6EA, 0x080D688Eu, XN_CODE + 0x14207 },   /* the navis' copy of a type's module (TYPES) */
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
	{ 0x08012066, THUMB_MOVS, 4, 0, 0x3, 0xC0 },   /* its effect: 0x3 in BN5, 0xc0 in BN6 */
	{ 0x080CAA7E, THUMB_ADDS, 4, 0, 0x17, 0xA3 },   /* its effect: 0x17 in BN5, 0xa3 in BN6 */
	{ 0x080EEE18, THUMB_ADDS, 0, 0, 0x48, 0xA8 },   /* its effects 0x48-0x4a in BN5, 0xa8-0xaa in BN6 */
	{ 0x080EEFDC, THUMB_ADDS, 4, 0, 0x45, 0xA6 },   /* its effect: 0x45 in BN5, 0xa6 in BN6 */
	{ 0x080EF6D0, THUMB_MOVS, 0, 0, 0x18, 0x5E },   /* its sound: song 0x117 in BN5 (movs, adds 0xFF), BN6's slot 0x5e */
	{ 0x080EF6D2, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* ... its adds made none (soulpower.c copies it) */
};
/* the object types its code spawns through the second-level dispatch: BN6's twin, moved (its routine BN5's), or another
 * track's copy, installed before (shared: tools/bn5_twins.py's TYPES) */
static const SoulType types[] = {
	{ 3, 0x30, XV_TYPE_MOVED, 0x080CA91Du },
};
/* BN5's routines and data its code runs in other tracks' copies (those of the types it shares), each's place there: what
 * src/emu/soulcode.c's soulcode_moved answers for them */
static const SoulRoute routes[] = {
	{ 0x080B9CAEu, XN_OBJECTS + 0x252 },
	{ 0x080B9CE0u, XN_OBJECTS + 0x284 },
	{ 0x080BA634u, XC_CODE + 0xA8CC },
	{ 0x080BCA14u, XC_CODE + 0xB3B4 },
	{ 0x080C34A6u, XN_CODE + 0x87FE },
	{ 0x080C34D2u, XN_CODE + 0x882A },
	{ 0x080C5346u, XN_CODE + 0x8C32 },
	{ 0x080C5386u, XN_CODE + 0x8C72 },
	{ 0x080C6E0Eu, XC_CODE + 0x9186 },
	{ 0x080C7772u, XN_CODE + 0x10522 },
	{ 0x080D688Eu, XN_CODE + 0x14206 },
	{ 0x080E36DCu, XC_CODE + 0x35A4 },
};
/* the effects it spawns of another track's install, one BN6 number a BN5 effect: present before the Souls' install */
static const uint8_t shared_effects[] = {
	0xA0, 0xA2, 0xA3, 0xA6, 0xA8, 0xA9, 0xAA,
};
/* BN5's sprites its code draws that BN6 has none of by their bytes: copied into SOULS_ART (past it FREE_SOULS_ART), listed at a number of a grown list
 * (list 0 the Souls' own; another's list a number of the track tools/bn5_objects.py names, whose copy serves where it stands first) */
static const XvSprite sprites[] = {
	{ 5, 0x03, 0, 0x54, SOULS_ART + 0x0, 0x3000 },
	{ 4, 0x20, 0, 0x57, SOULS_ART + 0x3000, 0x1D00 },
	{ 5, 0x1C, 0, 0x58, SOULS_ART + 0x4D00, 0x1900 },
	{ 3, 0x1F, 0, 0x4E, SOULS_ART + 0x6600, 0x400 },
};
/* BN5's effects its code spawns that BN6 has none of: at an id of the grown table, BN5's entry with its sprite as listed above */
static const SoulEffect effects[] = {
	{ 0x03, 0xC0, 0, 0x54, 0x00, 0x00 },
	{ 0x20, 0xC1, 5, 0xFC, 0x00, 0x00 },
};
/* bytes of its raw tables BN6 numbers otherwise */
static const XvByte bytes[] = {
	{ 0x080EC112, 0x17, 0xA3 },   /* its effect for ShadowSoul's charged shot (sub-family 2): 0x17 in BN5, 0xa3 in BN6 */
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
	{ 0x117, 0x5E },
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
	blocks, N(blocks), data, N(data), words, N(words), calls, N(calls), imms, N(imms), types, N(types), sprites, N(sprites), effects, N(effects), bytes, N(bytes), fields, N(fields), powers, N(powers), songs, N(songs), families, N(families), routes, N(routes), shared_effects, N(shared_effects),
};
