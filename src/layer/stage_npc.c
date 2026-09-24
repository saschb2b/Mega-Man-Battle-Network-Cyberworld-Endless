/* The boss sequence's actors, in the game's NPC bytecode (see npc.c for
 * the commands): each waits hidden for an event flag the director sets,
 * then plays its part. Commands used here beyond npc.c's: 0x04 jump if
 * flag set, 0x09 active but invisible, 0x0D/0x0E talking on/off, 0x1F/0x20
 * collision off/on, 0x28 play sound, 0x2A wait for the animation's end
 * (0xC0), 0x31 alpha (0x10 opaque .. 0x02, 0 off). */
#include "stage_npc.h"

#include <string.h>

#include "bytes.h"
#include "emu.h"
#include "mapslot.h"
#include "npc.h"

#define SOUND_LOG_IN   0x77
#define SOUND_LOG_OUT  0x76
#define SOUND_APPEAR   0x94
#define ANIM_LOG_IN    25   /* every Navi's overworld sprite: materializing */

/* A script under construction, with jumps to labels resolved once its
 * address is known. */
typedef struct {
	uint8_t b[160];
	int n;
	int label[8];
	struct { int at, label; } fix[12];
	int nfix;
} Script;

static void op(Script *s, const uint8_t *b, int n) { memcpy(s->b + s->n, b, (size_t)n); s->n += n; }
#define OP(s, ...) do { const uint8_t b_[] = { __VA_ARGS__ }; op((s), b_, (int)sizeof b_); } while (0)
static void mark(Script *s, int l) { s->label[l] = s->n; }
static void addr(Script *s, int l) { s->fix[s->nfix].at = s->n; s->fix[s->nfix++].label = l; s->n += 4; }
static void jump_if(Script *s, int flag, int l) { OP(s, 0x04, (uint8_t)flag, (uint8_t)(flag >> 8)); addr(s, l); }
static void sound(Script *s, int id) { OP(s, 0x28, (uint8_t)id, (uint8_t)(id >> 8)); }
static void coords(Script *s, int x, int y, int z) {
	OP(s, 0x14, (uint8_t)x, (uint8_t)(x >> 8), (uint8_t)y, (uint8_t)(y >> 8), (uint8_t)z, (uint8_t)(z >> 8));
}
/* Waits a frame at a time until `flag` is set, then goes on at `l`. */
static void wait_flag(Script *s, int flag, int l) {
	int loop = s->n;
	jump_if(s, flag, l);
	OP(s, 0x10, 0x01);
	OP(s, 0x02);
	s->fix[s->nfix].at = s->n; s->fix[s->nfix++].label = -1 - loop; s->n += 4;
}

static uint32_t commit(Script *s) {
	uint32_t at = mapslot_alloc(s->b, s->n);
	if (!at) return 0;
	for (int i = 0; i < s->nfix; ++i) {
		int l = s->fix[i].label;
		put32(s->b + s->fix[i].at, at + (uint32_t)(l >= 0 ? s->label[l] : -1 - l));
	}
	emu_write(at, s->b, (size_t)s->n);
	return at;
}

enum { L_SHOW, L_IDLE, L_LEAVE };

uint32_t npc_guardian(int sprite, int x, int y, int z, int face, int pose, bool logs_in, const StageFlags *f) {
	Script s = { .n = 0 };
	OP(&s, 0x09, 0x25, (uint8_t)sprite, 6 * 4, 0x16, (uint8_t)face, 0x0E, 0x13);
	coords(&s, x, y, z);
	wait_flag(&s, f->appear, L_SHOW);
	mark(&s, L_SHOW);
	/* logs in where it stands, then strikes its pose for the title card */
	sound(&s, SOUND_LOG_IN);
	if (logs_in) OP(&s, 0x16, ANIM_LOG_IN, 0x08, 0x2A, 0xC0);
	else OP(&s, 0x08);
	if (pose >= 0) OP(&s, 0x16, (uint8_t)pose, 0x10, 90);
	OP(&s, 0x16, (uint8_t)face);
	mark(&s, L_IDLE);
	wait_flag(&s, f->gone, L_LEAVE);
	mark(&s, L_LEAVE);
	/* deleted: logs out, fading away */
	sound(&s, SOUND_LOG_OUT);
	for (int a = 0x10; a >= 0x02; a -= 2) OP(&s, 0x31, (uint8_t)a, 0x10, 0x04);
	OP(&s, 0x09, 0x31, 0x00, 0x03);
	return commit(&s);
}

uint32_t npc_guardian_data(int x, int y, int z, int anim, uint32_t archive, int script, const StageFlags *f) {
	Script s = { .n = 0 };
	/* the game's Mystery Data sprite, hidden and out of the way until shown */
	OP(&s, 0x09, 0x25, 0x02, 7 * 4, 0x16, (uint8_t)anim, 0x0E, 0x1F, 0x13);
	OP(&s, 0x44, (uint8_t)script, (uint8_t)archive, (uint8_t)(archive >> 8), (uint8_t)(archive >> 16), (uint8_t)(archive >> 24));
	coords(&s, x, y, z);
	wait_flag(&s, f->reward, L_SHOW);
	mark(&s, L_SHOW);
	sound(&s, SOUND_APPEAR);
	OP(&s, 0x08, 0x0D, 0x20);
	mark(&s, L_IDLE);
	wait_flag(&s, f->taken, L_LEAVE);
	mark(&s, L_LEAVE);
	OP(&s, 0x03);
	return commit(&s);
}

uint32_t npc_sealed_pad(int category, int index, int x, int y, int z, int open_flag) {
	Script s = { .n = 0 };
	/* as npc_prop's pads: a floor sprite under MegaMan (npc.c) */
	OP(&s, 0x09, 0x25, (uint8_t)index, (uint8_t)(category * 4), 0x16, 0x00, 0x0A, 0x00, 0x1F, 0x0E);
	x += NPC_FLOOR_BACK; y -= NPC_FLOOR_BACK; z -= NPC_FLOOR_BACK;
	coords(&s, x, y, z);
	OP(&s, 0x0C, (uint8_t)-NPC_FLOOR_BACK, (uint8_t)NPC_FLOOR_BACK, (uint8_t)NPC_FLOOR_BACK, 0x1B);
	wait_flag(&s, open_flag, L_SHOW);
	mark(&s, L_SHOW);
	sound(&s, SOUND_APPEAR);
	OP(&s, 0x08, 0x00);
	return commit(&s);
}
