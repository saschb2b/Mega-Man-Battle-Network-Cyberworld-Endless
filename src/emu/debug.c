#include "debug.h"

#include <stdlib.h>

#include "bn6.h"
#include "emu.h"
#include "game.h"
#include "run.h"

bool emu_debug_on(void) { return getenv("CYBERWORLD_EMU_DEBUG") != NULL; }

FILE *emu_debug_file(const char *name) {
	char path[600];
	snprintf(path, sizeof path, "%s/%s", g_data_dir, name);
	return fopen(path, "wb");
}

void emu_debug_frame(void) {
	static int t;
	if (!emu_debug_on()) return;
	++t;
	if (t % 30 == 0)
		fprintf(stderr, "t%d depth %d mode %02x sub %02x chat %d pos %d %d z %d map %02x:%02x\n", t, run.depth,
			emu_read8(emu_read32(BN6_TOOLKIT)), emu_read8(BN6_GAMESTATE), emu_read8(BN6_CHATBOX),
			(int)emu_read32(BN6_PLAYER + 0x1C) >> 16, (int)emu_read32(BN6_PLAYER + 0x20) >> 16, (int)emu_read32(BN6_PLAYER + 0x24) >> 16,
			emu_read8(BN6_GAMESTATE + 4), emu_read8(BN6_GAMESTATE + 5));
	if (t % 60 == 0)
		fprintf(stderr, "music t%d song %08x status %08x\n", t, emu_read32(BN6_MUSIC_PLAYER), emu_read32(BN6_MUSIC_PLAYER + 4));
	if (t == 150) {
		/* VRAM, palettes and IO registers, for tools/romlab/labtrace.py */
		FILE *f = emu_debug_file("vram.bin");
		if (!f) return;
		for (uint32_t a = 0; a < 0x18000; ++a) fputc(emu_read8(0x06000000 + a), f);
		for (uint32_t a = 0; a < 0x400; ++a) fputc(emu_read8(0x05000000 + a), f);
		for (uint32_t a = 0; a < 0x60; ++a) fputc(emu_read8(0x04000000 + a), f);
		fclose(f);
	}
}
