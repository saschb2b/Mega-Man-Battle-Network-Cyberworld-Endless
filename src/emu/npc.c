/* NPC script builders. Commands (bn6f npc_script.inc): 0x00 end,
 * 0x03 free and end, 0x08 active and visible, 0x14 set coords (x, y, z
 * hwords), 0x16 set animation, 0x1F disable collision, 0x25 sprite with
 * category, 0x29 init Mystery Data (flag), 0x45 wait until taken (flag). */
#include "npc.h"

#include "mapslot.h"

uint32_t npc_mystery(int index) {
	uint16_t flag = (uint16_t)(MAPSLOT_MD_FLAG + index);
	uint8_t s[] = {
		0x08,
		0x25, 0x02, 0x1C,
		0x29, (uint8_t)flag, (uint8_t)(flag >> 8),
		0x45, (uint8_t)flag, (uint8_t)(flag >> 8),
		0x03,
	};
	return mapslot_alloc(s, sizeof s);
}

uint32_t npc_prop(int category, int index, int x, int y, int z, int anim) {
	uint8_t s[] = {
		0x08,
		0x25, (uint8_t)index, (uint8_t)(category * 4),
		0x14, (uint8_t)x, (uint8_t)(x >> 8), (uint8_t)y, (uint8_t)(y >> 8), (uint8_t)z, (uint8_t)(z >> 8),
		0x16, (uint8_t)anim,
		0x1F,
		0x00,
	};
	return mapslot_alloc(s, sizeof s);
}
