/* trade_order.h. BN6's Order Service screen as Trade Central's chip
 * picker speaks the desk's words, not AsterLand's ordering (docs/ONLINE.md;
 * the Order Service's text archive in docs/ROM_DATA.md): its archive is
 * read from the player's ROM, its prompts (scripts 0 and 1), its question (5),
 * its supplier's call (6), MegaMan's word after (7), the way out (2, 3)
 * written again round BN6's own commands (the chip's name and code, the
 * Yes and No), and the shop's descriptor pointed at the copy while
 * MegaMan is in the Square; put back as he leaves. */
#include "trade_order.h"

#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "emu.h"
#include "lz.h"
#include "mapslot.h"
#include "rom.h"
#include "shop.h"
#include "text.h"
#include "trade_words.h"

#define DESC_TEXT (BN6_SHOP_DESCS + SHOP_ORDER * 16u + 4u)   /* the Order Service's text archive, u32 */
#define ROOM 4096

static uint32_t original;

/* a script's head: the box opened with `face`'s picture (fc 06 f5 00 m e8 02 f1 00 00) */
static int head(uint8_t *o, int face) {
	const uint8_t h[] = { 0xFC, 0x06, 0xF5, 0x00, (uint8_t)face, 0xE8, 0x02, 0xF1, 0x00, 0x00 };
	memcpy(o, h, sizeof h);
	return (int)sizeof h;
}

static int text(uint8_t *o, const char *s) { return ta_encode(s, o, 200); }

/* the bytes of `script` of the original from its first byte equal to
 * `from` on (its commands after the words), -1 none */
static int tail(const uint8_t *s, int len, uint8_t from, int skip, uint8_t *o) {
	for (int i = skip; i < len; ++i)
		if (s[i] == from) { memcpy(o, s + i, (size_t)(len - i)); return len - i; }
	return -1;
}

/* script `i`'s words of ours round its commands; 0 to keep BN6's */
static int script(int i, const uint8_t *s, int len, uint8_t *o) {
	int n = 0, t;
	switch (i) {
	case 0: case 1:   /* (the prompt, and again after a chip is picked) */
		n = head(o, FACE_PROG); n += text(o + n, i ? trade_order_after() : trade_order_prompt());
		o[n++] = 0xEE; o[n++] = 0xFF; o[n++] = 0; o[n++] = 0; return n;
	case 3: n = head(o, FACE_PROG); n += text(o + n, trade_order_back()); o[n++] = 0xEE; o[n++] = 0xFF; o[n++] = 0; o[n++] = 0; return n;
	case 10: case 28: case 29:   /* (all offered, or one offered already) */
		n = head(o, FACE_PROG); n += text(o + n, i == 10 ? trade_order_all() : trade_order_once());
		o[n++] = 0xE7; o[n++] = 0; o[n++] = 0xEE; o[n++] = 0xFF; o[n++] = 0; o[n++] = 0; return n;
	case 6: n = head(o, FACE_PROG); n += text(o + n, trade_order_ready());
		o[n++] = 0xE7; o[n++] = 0; o[n++] = 0xEE; o[n++] = 0xFF; o[n++] = 0; o[n++] = 0; return n;
	case 2: case 5: case 7: {
		/* (its jumps first, BN6's own checks; then our words, then its
		 * commands from the chip's name or the line before its Yes and No
		 * on) */
		int j = 0;
		while (j + 1 < len && !(s[j] == 0xFC && s[j + 1] == 0x06)) ++j;
		memcpy(o, s, (size_t)j);
		n = j + head(o + j, i == 5 ? FACE_PROG : FACE_MEGAMAN);
		n += text(o + n, i == 2 ? trade_order_leave() : i == 5 ? trade_order_offer() : trade_order_said());
		t = tail(s, len, i == 2 ? 0xE9 : 0xAA, j + 10, o + n);
		return t < 0 ? 0 : n + t;
	}
	default: return 0;
	}
}

bool trade_order_words(bool on) {
	if (!on) {
		if (original) emu_write32(DESC_TEXT, original);
		original = 0;
		return true;
	}
	uint32_t at = original ? original : rom_u32(DESC_TEXT - 0x08000000u);
	if (at < 0x08000000u || at >= 0x08000000u + ROM_SIZE) return false;
	size_t len;
	uint8_t *blob = lz77_decompress(R.data + (at - 0x08000000u), ROM_SIZE - (at - 0x08000000u), &len);
	if (!blob || len < 8) { free(blob); return false; }
	/* (an archive after a 4-byte head: u16 offsets, then the scripts) */
	const uint8_t *arc = blob + 4;
	int alen = (int)len - 4, count = (arc[0] | arc[1] << 8) / 2;
	static uint8_t out[ROOM], packed[ROOM + ROOM / 8 + 16];
	int n = 4 + 2 * count;
	bool ok = count > 7 && n < ROOM;
	for (int i = 0; ok && i < count; ++i) {
		int a = arc[2 * i] | arc[2 * i + 1] << 8, b = i + 1 < count ? (arc[2 * i + 2] | arc[2 * i + 3] << 8) : alen;
		if (a > b || b > alen || n + 300 + (b - a) > ROOM) { ok = false; break; }
		put16(out + 4 + 2 * i, (uint16_t)(n - 4));
		int k = script(i, arc + a, b - a, out + n);
		if (!k) { memcpy(out + n, arc + a, (size_t)(b - a)); k = b - a; }
		n += k;
	}
	free(blob);
	if (!ok) return false;
	put32(out, (uint32_t)(n << 8));
	size_t size = lz_literal(out, (size_t)n, packed);
	uint32_t copy = mapslot_alloc(packed, (int)size);
	if (!copy) return false;
	if (!original) original = at;
	emu_write32(DESC_TEXT, copy);
	return true;
}
