/* puppet.h. Moved out of online_square.c (the shared dive, issue #589). */
#include "puppet.h"

#include "bytes.h"
#include "emu.h"
#include "mapslot.h"

#define SCRIPT_LOOP 9
#define SCRIPT_VISIBLE SCRIPT_LOOP
#define SCRIPT_ANIM (SCRIPT_LOOP + 2)
#define SCRIPT_POSITION (SCRIPT_LOOP + 4)
#define SCRIPT_JUMP (SCRIPT_LOOP + 13)
#define SCRIPT_SOUND (SCRIPT_LOOP + 17)

/* the mutable loop uses existing NPC bytecode (visible or hidden,
 * animation, position, a frame's pause, a jump), and a jump through a sound
 * (0x28) once when one is asked for. ns_set_animation only reloads a
 * changed animation, so a walker keeps its cadence (bn6f npc.s). */
uint32_t puppet_make(int category, int index) {
	uint8_t script[] = {
		0x09, 0x25, (uint8_t)index, (uint8_t)(category * 4), 0x0A, 0, 0x1F, 0x0E, 0x13,
		0x09, 0x16, 0, 0x14, 0, 0, 0, 0, 0, 0,
		0x10, 1, 0x02, 0, 0, 0, 0,
		0x28, 0, 0, 0x02, 0, 0, 0, 0,
	};
	_Static_assert(sizeof script == PUPPET_BYTES, "a puppet's bytes (puppet.h)");
	uint32_t at = mapslot_alloc(script, sizeof script);
	if (!at) return 0;
	put32(script + SCRIPT_JUMP, at + SCRIPT_LOOP);
	put32(script + sizeof script - 4, at + SCRIPT_LOOP);
	emu_write(at, script, sizeof script);
	return at;
}

void puppet_show(uint32_t at, bool visible) { emu_write8(at + SCRIPT_VISIBLE, visible ? 0x08 : 0x09); }

void puppet_anim(uint32_t at, int anim) { emu_write8(at + SCRIPT_ANIM, (uint8_t)anim); }

void puppet_place(uint32_t at, int x, int y, int z) {
	uint8_t p[6];
	put16(p, (uint16_t)x);
	put16(p + 2, (uint16_t)y);
	put16(p + 4, (uint16_t)z);
	emu_write(at + SCRIPT_POSITION, p, sizeof p);
}

static void jump(uint32_t at, uint32_t to) {
	uint8_t b[4];
	put32(b, to);
	emu_write(at + SCRIPT_JUMP, b, sizeof b);
}

void puppet_sound(uint32_t at, int sound) {
	emu_write8(at + SCRIPT_SOUND + 1, (uint8_t)sound);
	emu_write8(at + SCRIPT_SOUND + 2, (uint8_t)(sound >> 8));
	jump(at, at + SCRIPT_SOUND);
}

void puppet_loop(uint32_t at) { jump(at, at + SCRIPT_LOOP); }
