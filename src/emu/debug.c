#include "debug.h"

#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "emu.h"
#include "game.h"
#include "run.h"

bool emu_debug_on(void) { return getenv("CYBERWORLD_EMU_DEBUG") != NULL; }

/* CYBERWORLD_WATCH=ADDR:LEN,...: bytes of the game's memory (hex
 * addresses), printed with the frame's count each time they change; *ADDR
 * reads the pointer at ADDR first (*020093E4:16, the PET's submenu): a menu's
 * cursor found by moving it */
#define WATCH_MAX 8
#define WATCH_LEN 64
static struct {
	uint32_t addr;
	bool ptr, seen;
	int len;
	uint8_t last[WATCH_LEN];
} watch[WATCH_MAX];
static int nwatch = -1;

static void watch_parse(void) {
	nwatch = 0;
	for (const char *s = getenv("CYBERWORLD_WATCH"); s && *s && nwatch < WATCH_MAX; s = strchr(s, ',') ? strchr(s, ',') + 1 : NULL) {
		bool ptr = *s == '*';
		char *end;
		uint32_t a = (uint32_t)strtoul(s + ptr, &end, 16);
		int len = *end == ':' ? atoi(end + 1) : 4;
		watch[nwatch].addr = a;
		watch[nwatch].ptr = ptr;
		watch[nwatch++].len = len < 1 ? 1 : len > WATCH_LEN ? WATCH_LEN : len;
	}
}

static void watch_frame(int t) {
	if (nwatch < 0) watch_parse();
	for (int i = 0; i < nwatch; ++i) {
		uint32_t base = watch[i].ptr ? emu_read32(watch[i].addr) : watch[i].addr;
		bool same = watch[i].seen;
		for (int k = 0; k < watch[i].len; ++k) {
			uint8_t b = emu_read8(base + (uint32_t)k);
			same = same && watch[i].last[k] == b;
			watch[i].last[k] = b;
		}
		if (same) continue;
		watch[i].seen = true;
		fprintf(stderr, "watch t%d %08x:", t, (unsigned)base);
		for (int k = 0; k < watch[i].len; ++k) fprintf(stderr, " %02x", watch[i].last[k]);
		fputc('\n', stderr);
	}
}

static bool dump_part(const char *prefix, char part, uint32_t addr, uint32_t len) {
	char path[600];
	snprintf(path, sizeof path, "%s%c.bin", prefix, part);
	FILE *f = fopen(path, "wb");
	if (!f) return false;
	for (uint32_t a = 0; a < len; ++a) fputc(emu_read8(addr + a), f);
	return fclose(f) == 0;
}

bool emu_debug_dump(const char *prefix) {
	bool ok = dump_part(prefix, 'v', 0x06000000, 0x18000);
	ok = dump_part(prefix, 'p', 0x05000000, 0x400) && ok;
	ok = dump_part(prefix, 'i', 0x04000000, 0x60) && ok;
	return dump_part(prefix, 'o', 0x07000000, 0x400) && ok;
}

FILE *emu_debug_file(const char *name) {
	char path[600];
	snprintf(path, sizeof path, "%s/%s", g_data_dir, name);
	return fopen(path, "wb");
}

void emu_debug_frame(void) {
	static int t;
	static int watching = -1;
	if (watching < 0) watching = getenv("CYBERWORLD_WATCH") != NULL;
	if (watching) watch_frame(t + 1);
	if (!emu_debug_on()) {
		t += watching;
		return;
	}
	++t;
	if (t % 30 == 0)
		fprintf(stderr, "t%d depth %d mode %02x sub %02x chat %d pos %d %d z %d map %02x:%02x hooks %u\n", t, run.depth,
			emu_read8(emu_read32(BN6_TOOLKIT)), emu_read8(BN6_GAMESTATE), emu_read8(BN6_CHATBOX),
			bn6_player_x(), bn6_player_y(), bn6_player_z(),
			emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER), (unsigned)hook_hits);
	/* (a hook's event lost to a full queue, or our BKPT where no hook is: neither should happen) */
	static uint32_t dropped, strays;
	if (hook_dropped != dropped || hook_strays != strays) {
		dropped = hook_dropped;
		strays = hook_strays;
		fprintf(stderr, "hooks: %u events dropped, %u strays\n", (unsigned)dropped, (unsigned)strays);
	}
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
