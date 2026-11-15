/* RAM kept patched (emu_keep.h): a few patches of the work RAM the engine
 * keeps its own across states, for both cores. */
#include "emu_keep.h"

#include <string.h>

#define KEEP_MAX 8
#define KEEP_LEN 64
static struct { uint32_t addr, len; uint8_t was[KEEP_LEN], now[KEEP_LEN]; } kept[KEEP_MAX];
static int nkept;

bool emu_keep_add(uint32_t addr, const void *was, const void *now, size_t len) {
	if (nkept >= KEEP_MAX || !len || len > KEEP_LEN) return false;
	kept[nkept].addr = addr;
	kept[nkept].len = (uint32_t)len;
	memcpy(kept[nkept].was, was, len);
	memcpy(kept[nkept].now, now, len);
	++nkept;
	return true;
}

void emu_keep_put(bool own, EmuRawRead rd, EmuRawWrite wr) {
	for (int i = 0; i < nkept; ++i) {
		const uint8_t *to = own ? kept[i].was : kept[i].now;
		bool there = true;
		for (uint32_t k = 0; own && there && k < kept[i].len; ++k) there = rd(kept[i].addr + k) == kept[i].now[k];
		for (uint32_t k = 0; there && k < kept[i].len; ++k)
			if (rd(kept[i].addr + k) != to[k]) wr(kept[i].addr + k, to[k]);
	}
}
