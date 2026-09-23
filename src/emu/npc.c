/* NPC script builders. Commands (bn6f npc_script.inc): 0x00 end,
 * 0x03 free and end, 0x08 active and visible, 0x14 set coords (x, y, z
 * hwords), 0x16 set animation, 0x0A collision radius, 0x0E no interaction,
 * 0x1F disable collision, 0x25 sprite with
 * category, 0x29 init Mystery Data (flag), 0x45 wait until taken (flag),
 * 0x10 pause (frames), 0x02 jump (address), 0x44 text script (index,
 * archive). */
#include "npc.h"

#include "emu.h"
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
		0x0A, 0x00,   /* no collision radius */
		0x1F,
		0x0E,         /* nothing to talk to */
		0x00,
	};
	return mapslot_alloc(s, sizeof s);
}

uint32_t npc_talker(int category, int index, int x, int y, int z, int anim, uint32_t archive, int script) {
	uint8_t s[] = {
		0x08,
		0x25, (uint8_t)index, (uint8_t)(category * 4),
		0x14, (uint8_t)x, (uint8_t)(x >> 8), (uint8_t)y, (uint8_t)(y >> 8), (uint8_t)z, (uint8_t)(z >> 8),
		0x16, (uint8_t)anim,
		0x44, (uint8_t)script, (uint8_t)archive, (uint8_t)(archive >> 8), (uint8_t)(archive >> 16), (uint8_t)(archive >> 24),
		/* idle: pause a frame, then jump back */
		0x10, 0x01,
		0x02, 0, 0, 0, 0,
	};
	uint32_t at = mapslot_alloc(s, sizeof s);
	if (!at) return 0;
	uint32_t loop = at + (uint32_t)sizeof s - 7;
	uint8_t jump[4] = { (uint8_t)loop, (uint8_t)(loop >> 8), (uint8_t)(loop >> 16), (uint8_t)(loop >> 24) };
	emu_write(at + (uint32_t)sizeof s - 4, jump, 4);
	return at;
}
