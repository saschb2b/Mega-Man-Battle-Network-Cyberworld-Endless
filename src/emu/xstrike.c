/* BN5's obstacle strike in BN6's engine (xstrike.h). */
#include "xstrike.h"

#include <stdint.h>
#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "bn6_forms.h"
#include "bn6_navis.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "soul_room.h"
#include "soulcode.h"
#include "thumb_reloc.h"

#define BUS 0x08000000u
#define FREE_HALF 0xFFFFu          /* the free space as emu_init pads it */
#define THUMB_BX_LR 0x4770u
#define PANEL_AT 0x0800BD15u       /* the word BN5's panel getter leaves in r2 (0x0800AFC4 ldr r2: its routine), which the strike's spawn stores */
#define SITES 4
#define STRIKE_SIZE 0x58u          /* BN5's block as the Thumb at SOUL_COLONEL_STRIKE has it */

static bool done, broken;
static uint32_t wrapper;           /* BN5's strike's spawn (BN5_COLONEL_STRIKE_SPAWN), moved with the Souls' code */

/* the side's bit of BN6_COLONEL_SIDES, either Colonel's */
static bool armed(int side) { return (hook_read8(BN6_COLONEL_SIDES) & (BN6_COLONEL_SIDE0 << side)) != 0; }

/* (BN5's Colonel's side's flag set, 0x080CAD06, the stub ColonelSoul's ready calls: r5 the object) the side's bit, where a
 * Navi Colonel's spawn sets his (XN_STUB_SIDES) */
static HookAct side_set(HookRegs *r, void *user) {
	(void)user;
	int side = hook_read8(r->r[5] + BN6_T1_ALLIANCE) & 1;
	hook_write8(BN6_COLONEL_SIDES, (uint8_t)(hook_read8(BN6_COLONEL_SIDES) | BN6_COLONEL_SIDE0 << side));
	return HOOK_RETURN;
}

static uint32_t panel_flags(int x, int y) {
	if ((unsigned)(x - 1) >= 6 || (unsigned)(y - 1) >= 3) return 0;
	return hook_read32(BN6_FIELD_PANELS + (uint32_t)(y * BN6_FIELD_COLUMNS + x) * BN6_PANEL_SIZE + BN6_PANEL_FLAGS);
}

/* (BN5's test of an obstacle at r0, r1, 0x080CABEA, the stub) where the panel's other side is armed: an object of the
 * panel's side within two panels before the obstacle (0x080CAC42) its strike 1, else one beyond it (0x080CAC98) its
 * strike 2; r1 the striking side, r2 as BN5's searches leave it (its spawn keeps its low byte) */
static HookAct strike_test(HookRegs *r, void *user) {
	(void)user;
	int x = (int)r->r[0], y = (int)r->r[1];
	r->r[0] = 0;
	uint32_t f = panel_flags(x, y);
	if (!(f & BN6_PANEL_FIELD) || broken) return HOOK_RETURN;
	int alliance = hook_read8(BN6_FIELD_PANELS + (uint32_t)(y * BN6_FIELD_COLUMNS + x) * BN6_PANEL_SIZE + BN6_PANEL_ALLIANCE);
	int side = (alliance & 1) ^ 1;
	if (!armed(side)) return HOOK_RETURN;
	uint32_t mask = side ? BN6_PANEL_ALLIES : BN6_PANEL_ENEMIES;
	int dir = side ? -1 : 1;
	r->r[1] = (uint32_t)side;
	for (int k = 0, at = x - dir; k < 2; ++k, at -= dir) {
		uint32_t g = panel_flags(at, y);
		if (!g) break;
		if (g & mask) {
			r->r[0] = 1;
			r->r[2] = PANEL_AT;
			return HOOK_RETURN;
		}
	}
	for (int at = x + dir; (unsigned)(at - 1) < 6; at += dir)
		if ((panel_flags(at, y) & mask) == mask) {
			r->r[0] = 2;
			r->r[2] = mask;
			return HOOK_RETURN;
		}
	return HOOK_RETURN;
}

/* (an obstacle's states, at their collision flags' read where BN5's block stands) on to the Thumb of BN5's block where a
 * side is armed, r3 the way back */
static HookAct obstacle(HookRegs *r, void *user) {
	if (!done || !(armed(0) || armed(1))) return HOOK_CONTINUE;
	r->r[3] = ((uint32_t)(uintptr_t)user + 4) | 1;
	r->r[12] = SOUL_COLONEL_STRIKE | 1;
	return HOOK_JUMP;
}

/* BN5's block (SOUL_COLONEL_STRIKE; 0x08018002-0x0801802E): r3 the way back, then the collision flags' read it stood
 * before */
static void strike_code(uint8_t out[STRIKE_SIZE]) {
	static const uint16_t t[] = {
		0xB408, 0x4B11, 0, 0, 0xD117,     /* push {r3}; ldr r3,=paused; bl bx r3; bne the end */
		0x7A68, 0x2800, 0xD014,           /* ldrb r0,[r5,#9]; cmp r0,#0; beq the end */
		0x7CA8, 0x7CE9, 0x7DAA,           /* ldrb r0,[r5,#0x12]; ldrb r1,[r5,#0x13]; ldrb r2,[r5,#0x16] */
		0x4B0D, 0, 0, 0x4200, 0xD00C,     /* ldr r3,=the test; bl bx r3; tst r0,r0; beq the end */
		0x1E44, 0x1C0B, 0x7CA8, 0x7CE9,   /* subs r4,r0,#1; adds r3,r1,#0; ldrb r0,[r5,#0x12]; ldrb r1,[r5,#0x13] */
		0x2600, 0xB408, 0x4B08, 0x469C,   /* movs r6,#0; push {r3}; ldr r3,=the spawn; mov r12,r3 */
		0xBC08, 0, 0,                     /* pop {r3}; bl bx r12 */
		0x2000, 0x6268,                   /* movs r0,#0; str r0,[r5,#0x24] */
		0x4B06, 0, 0, 0xBC08, 0x4718,     /* the end: ldr r3,=the flags' read; bl bx r3; pop {r3}; bx r3 */
		0x4718, 0x4760,                   /* bx r3; bx r12 */
	};
	for (size_t i = 0; i < sizeof t / sizeof *t; ++i) put16(out + 2 * i, t[i]);
	thumb_bl_write(out, 0x04, SOUL_COLONEL_STRIKE, SOUL_COLONEL_STRIKE + 0x44);
	thumb_bl_write(out, 0x18, SOUL_COLONEL_STRIKE, SOUL_COLONEL_STRIKE + 0x44);
	thumb_bl_write(out, 0x32, SOUL_COLONEL_STRIKE, SOUL_COLONEL_STRIKE + 0x46);
	thumb_bl_write(out, 0x3C, SOUL_COLONEL_STRIKE, SOUL_COLONEL_STRIKE + 0x44);
	put32(out + 0x48, BN6_BATTLE_PAUSED | 1);
	put32(out + 0x4C, SOUL_STUB_STRIKE_CHECK | 1);
	put32(out + 0x50, wrapper | 1);
	put32(out + 0x54, BN6_OBJ_FLAGS | 1);
}

static const uint32_t sites[SITES] = { BN6_OBSTACLE_FLAGS_A, BN6_OBSTACLE_FLAGS_B, BN6_OBSTACLE_FLAGS_C, BN6_OBSTACLE_FLAGS_D };

/* every byte it stands on as Gregar has it, its room free */
static bool expected(void) {
	for (int i = 0; i < SITES; ++i) {
		uint32_t t;
		if (!thumb_bl_decode(sites[i], rom_u16(sites[i] - BUS), rom_u16(sites[i] + 2 - BUS), &t) || t != BN6_OBJ_FLAGS) {
			fprintf(stderr, "xstrike: %08x is not the obstacle's flags' read expected\n", (unsigned)sites[i]);
			return false;
		}
	}
	for (uint32_t a = SOUL_COLONEL_STRIKE; a < SOUL_COLONEL_STRIKE + STRIKE_SIZE; a += 2)
		if (emu_read16(a) != FREE_HALF) return false;
	return emu_read16(SOUL_STUB_STRIKE) == FREE_HALF && emu_read16(SOUL_STUB_STRIKE_CHECK) == FREE_HALF;
}

bool xstrike_install(void) {
	if (done) return true;
	if (!R.data || !emu_ready() || !soulcode_install(&soulcode_bn5)) return false;
	wrapper = soulcode_moved(BN5_COLONEL_STRIKE_SPAWN | 1);
	if (!wrapper || !expected()) {
		fprintf(stderr, "xstrike: the obstacle strike not placed (its soldier's spawn %08x)\n", (unsigned)wrapper);
		return false;
	}
	uint8_t st[STRIKE_SIZE];
	strike_code(st);
	const uint8_t bx[2] = { THUMB_BX_LR & 0xFF, THUMB_BX_LR >> 8 };
	claims_begin(XSTRIKE_CLAIMS);
	emu_write(SOUL_COLONEL_STRIKE, st, sizeof st);
	emu_write(SOUL_STUB_STRIKE, bx, sizeof bx);
	emu_write(SOUL_STUB_STRIKE_CHECK, bx, sizeof bx);
	bool ok = emu_hook(SOUL_STUB_STRIKE, side_set, NULL) && emu_hook(SOUL_STUB_STRIKE_CHECK, strike_test, NULL);
	for (int i = 0; ok && i < SITES; ++i) ok = emu_hook(sites[i], obstacle, (void *)(uintptr_t)sites[i]);
	claims_end();
	done = ok;
	printf("xstrike: BN5's obstacle strike%s\n", ok ? ", either Colonel's side armed through BN6_COLONEL_SIDES" : " NOT placed whole");
	return ok;
}

void xstrike_break(bool on) { broken = on; }
