#include "debug.h"

#include <stdlib.h>

#include "bn6.h"
#include "bn6_fields.h"
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
			bn6_player_x(), bn6_player_y(), bn6_player_z(),
			emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER));
	if (t % 60 == 0)
		fprintf(stderr, "music t%d song %08x status %08x\n", t, emu_read32(BN6_MUSIC_PLAYER), emu_read32(BN6_MUSIC_STATUS));
	/* (the NaviCust's bug counts, when any is set) */
	if (t % 30 == 0) {
		char b[16 * 3 + 1];
		int any = 0;
		for (uint32_t i = 0; i < 16; ++i) { any |= emu_read8(BN6_NAVICUST_BUGS + i); snprintf(b + i * 3, 4, "%02x ", emu_read8(BN6_NAVICUST_BUGS + i)); }
		if (any) fprintf(stderr, "bugs t%d %s\n", t, b);
	}
	/* (an open chat box: its script state, open state, jump table offset,
	 * cursor into the script and flags, bn6f chatbox_struct) */
	if (t % 30 == 0 && emu_read8(BN6_CHATBOX))
		fprintf(stderr, "chatbox t%d state %02x open %02x jump %02x at %08x flags %04x\n", t, emu_read8(BN6_CHATBOX_STATE),
			emu_read8(BN6_CHATBOX_OPEN), emu_read8(BN6_CHATBOX_JUMP), emu_read32(BN6_CHATBOX_SCRIPT_AT), emu_read16(BN6_CHATBOX_BOX_FLAGS));
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
