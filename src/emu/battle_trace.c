#include "battle_trace.h"

#include <stdio.h>
#include <stdlib.h>

#include "bn6.h"
#include "bytes.h"
#include "debug.h"
#include "emu.h"
#include "xdiff_events.h"
#include "xvirus.h"

#define IWRAM      0x03000000u   /* (the GBA's) */
#define IWRAM_SIZE 0x8000u
#define EWRAM_SIZE (BN6_EWRAM_END - BN6_EWRAM)
#define IMAGE      (EWRAM_SIZE + IWRAM_SIZE)
#define GAP        16            /* equal bytes a run of changes spans before it ends */

static FILE *out;
static uint8_t *was, *now;
static uint32_t frames;

static void u32_out(uint32_t v) {
	uint8_t b[4];
	put32(b, v);
	fwrite(b, 1, 4, out);
}

static bool open_trace(void) {
	if (out) return true;
	was = calloc(1, IMAGE);
	now = malloc(IMAGE);
	out = was && now ? emu_debug_file("btrace.bin") : NULL;
	if (!out) { free(was); free(now); was = now = NULL; return false; }
	static XvMoved m[XV_MOVED_MAX];
	int n = xvirus_moved(m, XV_MOVED_MAX);
	fwrite("BTRC", 1, 4, out);
	u32_out(1);
	u32_out((uint32_t)n);
	for (int i = 0; i < n; ++i) { u32_out(m[i].from); u32_out(m[i].to); u32_out(m[i].len); }
	return true;
}

static void read_image(void) {
	for (uint32_t a = 0; a < EWRAM_SIZE; a += 4) put32(now + a, emu_read32(BN6_EWRAM + a));
	for (uint32_t a = 0; a < IWRAM_SIZE; a += 4) put32(now + EWRAM_SIZE + a, emu_read32(IWRAM + a));
}

/* the runs of bytes that differ from the frame before: written where `write`, counted either way */
static uint32_t runs(bool write) {
	uint32_t n = 0;
	for (uint32_t a = 0; a < IMAGE;) {
		if (now[a] == was[a]) { ++a; continue; }
		uint32_t end = a + 1, same = 0;
		for (uint32_t b = end; b < IMAGE && same < GAP; ++b) {
			if (now[b] != was[b]) { end = b + 1; same = 0; }
			else ++same;
		}
		if (write) { u32_out(a); u32_out(end - a); fwrite(now + a, 1, end - a, out); }
		++n;
		a = end;
	}
	return n;
}

void battle_trace_frame(void) {
	static int on = -1;
	static const char *draws;
	if (on < 0) {
		on = getenv("CYBERWORLD_BTRACE") != NULL;
		draws = getenv("CYBERWORLD_XDRAWS");
	}
	if (draws && *draws) xd_control_frame(draws);
	if (!on) return;
	uint8_t sub = emu_read8(BN6_GAMESTATE);
	if (sub != BN6_SUB_BATTLE_INIT && sub != BN6_SUB_BATTLE) return;
	if (!open_trace()) return;
	read_image();
	fwrite("FRAM", 1, 4, out);
	u32_out(frames++);
	u32_out(emu_read32(BN6_BATTLE_TIMER));
	u32_out(runs(false));
	runs(true);
	fflush(out);
	uint8_t *t = was;
	was = now;
	now = t;
}
