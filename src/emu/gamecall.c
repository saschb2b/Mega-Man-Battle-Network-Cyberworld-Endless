/* A stub in the free ROM space runs one of the game's routines from the
 * game-state update (cbGameState, run every frame of the game mode), which
 * the engine borrows for a frame; the stub marks BN6_ENGINE_MARK when it
 * has run. */
#include "gamecall.h"

#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "emu.h"

#define WARP_DATA (EMU_FREE + 0x000)  /* a warp record and a one-entry warp list */
#define CALL_STUB (EMU_FREE + 0x100)

bool game_call(uint32_t fn, uint32_t r0, uint32_t r1) {
	/* push {r4-r7,lr}; ldr r2,=mark; movs r3,#1; strb r3,[r2]; ldr r0,=a0;
	 * ldr r1,=a1; ldr r3,=fn; bl 1f; pop {r4-r7,pc}; 1: bx r3; then
	 * mark, a0, a1, fn */
	uint8_t stub[40] = {
		0xF0, 0xB5, 0x05, 0x4A, 0x01, 0x23, 0x13, 0x70, 0x04, 0x48, 0x05, 0x49,
		0x05, 0x4B, 0x00, 0xF0, 0x01, 0xF8, 0xF0, 0xBD, 0x18, 0x47, 0x00, 0x00,
	};
	put32(stub + 24, BN6_ENGINE_MARK);
	put32(stub + 28, r0);
	put32(stub + 32, r1);
	put32(stub + 36, fn);
	emu_write(CALL_STUB, stub, sizeof stub);
	/* the hook: ldr r0,[pc]; bx r0; .word stub+1 */
	uint8_t saved[8], jump[8] = { 0x00, 0x48, 0x00, 0x47 };
	put32(jump + 4, CALL_STUB + 1);
	for (int i = 0; i < 8; ++i) saved[i] = emu_read8(BN6_OW_HOOK + (uint32_t)i);
	emu_write8(BN6_ENGINE_MARK, 0);
	emu_write(BN6_OW_HOOK, jump, sizeof jump);
	for (int i = 0; i < 120 && !emu_read8(BN6_ENGINE_MARK); ++i) emu_frame(0);
	emu_write(BN6_OW_HOOK, saved, sizeof saved);
	return emu_read8(BN6_ENGINE_MARK) != 0;
}

/* the game starts a map's song only when it is not the one it last started:
 * after the intro, or a state whose song had stopped, forget it */
static void restart_music(void) {
	if (emu_read32(BN6_MUSIC_PLAYER + 4) & 0x80000000u) emu_write8(BN6_GAMESTATE + 0x0F, 0xFF);
}

void emu_warp(int group, int number, int x, int y, int facing) {
	restart_music();
	/* Warp2011bb0: the destination, warp index 1 of a list holding it */
	uint8_t data[32] = { (uint8_t)group, (uint8_t)number, 0, (uint8_t)facing };
	put32(data + 4, (uint32_t)x << 16);
	put32(data + 8, (uint32_t)y << 16);
	data[17] = 1;
	put32(data + 20, WARP_DATA);
	emu_write(WARP_DATA, data, sizeof data);
	emu_write(BN6_WARP, data, sizeof data);
	game_call(BN6_ENTER_MAP_ON_WARP, 0, 0);
}

void emu_warp_out(void) {
	restart_music();
	/* as the warp pad's trigger leaves it: under way, index 1, same group kind */
	emu_write8(BN6_WARP + 0x10, 1);
	emu_write8(BN6_WARP + 0x11, 1);
	emu_write8(BN6_WARP + 0x12, 0);
	game_call(BN6_WARP_DEPART_JACK_OUT, 0, 0);
}
