/* order_sort.h. The clerk's list at AsterLand holds every chip the Library
 * does, in BN6's ID order, five rows a page (docs/HOME.md, piece 6); BN6's
 * folder editor and the Chip Trader beside him sort theirs by seven keys
 * on START, the Order Service by none (issue #124). Here START on his list
 * steps through those keys in their order, ID, ABCDE, Code, Attack,
 * Element, No. and MB, each the way BN6 first sorts by it, and SELECT
 * turns it round, as BN6 does for a key picked twice; the cursor goes to
 * the top, BN6's sound plays and his box says the sort over the buttons.
 * No. is the copies of the entry, its chip in its code, MegaMan holds,
 * the folder's among them. The sort is the session's: his list opens in it
 * again.
 *
 * BN6 makes the list it shows from the entries it found as the screen
 * opened (BN6_SHOP_SOURCE), again at each BN6_SHOP_REDRAW: those are
 * sorted, between frames, by BN6's own keys (bn6f sub_811FE7C, the pack's
 * list), its ties by ID; then the screen is sent back to its list through
 * BN6's own way back (BN6_SHOP_AT_AGAIN), which runs his list prompt, and
 * a hook on the chat has ours said in its place, there and wherever the
 * screen asks for it (his scripts 0 and 1), in AsterLand alone, the one
 * place the engine opens BN6's order screen (Trade Central's picker is a
 * screen of its own since issue #145). */
#include "order_sort.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "aster_land.h"
#include "bn6.h"
#include "bytes.h"
#include "chips.h"
#include "director.h"
#include "emu.h"
#include "hook.h"
#include "place_lines.h"
#include "rom.h"
#include "shop.h"
#include "text.h"

#define ORDER_DESC (BN6_SHOP_DESCS + SHOP_ORDER * 16u)   /* the Order Service's descriptor, as BN6_SHOP_DESC names it */
#define CHAT_RUN (BN6_CHAT_RUN_SCRIPT & ~1u)
#define CHAT_RUN_PUSH 0xB570   /* its first instruction: push {r4-r6,lr} */
#define SOUND_SORTED 0x81      /* bn6f SOUND_MENU_SELECT: BN6's SORT window's, as a key is picked */
#define ROWS 0x140             /* the Order Service's entries at most (its descriptor's) */
#define PACK_MOST 1024         /* the pack's entries read at most (a chip in a code each) */

enum { SORT_ID, SORT_ABCDE, SORT_CODE, SORT_ATTACK, SORT_ELEMENT, SORT_NO, SORT_MB };

/* The session's sort, whether a key was picked since his list opened, and
 * his words: an archive of their own (script 0 the greeting, then each
 * key's two ways, each said plain and with BN6's sound), and the script
 * the hook says, set between frames */
static int key = SORT_ID;
static bool flipped, picked;
static uint32_t words;
static int say;

static int script_of(bool sound) {
	if (!picked && key == SORT_ID && !flipped) return 0;
	return 1 + (key * 2 + flipped) * 2 + sound;
}

/* The chat begun: the Order Service's list prompt at AsterLand's counter
 * (its keeper's scripts 0 and 1, as the screen opens and after each line)
 * is ours */
static HookAct chat_run(HookRegs *r, void *user) {
	(void)user;
	if (!words || r->r[1] > 1 || !aster_land_map(hook_read8(BN6_MAP_GROUP), hook_read8(BN6_MAP_NUMBER))) return HOOK_CONTINUE;
	if (hook_read8(hook_read32(BN6_TOOLKIT)) != BN6_MODE_SHOP) return HOOK_CONTINUE;
	if (hook_read32(BN6_SHOP_DESC) != ORDER_DESC || r->r[0] != hook_read32(BN6_SHOP_ARCHIVE)) return HOOK_CONTINUE;
	r->r[0] = words;
	r->r[1] = (uint32_t)say;
	return HOOK_CONTINUE;
}

void order_sort_install(void) {
	if (rom_u16(rom_off(CHAT_RUN)) == CHAT_RUN_PUSH) emu_hook(CHAT_RUN, chat_run, NULL);
}

/* a box as BN6's own list prompt is: his face, opened at once, its text
 * at once, held; with `sound`, BN6's sound first */
static void prompt(TextArchive *t, const char *line, bool sound) {
	static const uint8_t brighten_off[] = { 0xFC, 0x06 };   /* ts_call_disable_mugshot_brighten */
	static const uint8_t quick[] = { 0xE8, 0x02, 0xF1, 0x00, 0x00 };   /* ts_msg_open_quick, ts_text_speed 0 */
	static const uint8_t hold[] = { 0xEE, 0xFF, 0x00, 0x00 };   /* ts_wait_hold */
	static const uint8_t chime[] = { 0xFD, 0x00, SOUND_SORTED, 0x00 };   /* ts_sound_play00 */
	char text[64];
	snprintf(text, sizeof text, "%s\n%s", line, place_order_buttons);
	ta_script(t);
	if (sound) ta_bytes(t, chime, sizeof chime);
	ta_bytes(t, brighten_off, sizeof brighten_off);
	ta_mugshot(t, FACE_CLERK);
	ta_bytes(t, quick, sizeof quick);
	ta_text(t, text);
	ta_bytes(t, hold, sizeof hold);
}

void order_sort_words(void) {
	static TextArchive t;
	ta_begin(&t);
	prompt(&t, place_order_hello, false);
	for (int k = 0; k < PLACE_SORT_KEYS; ++k)
		for (int way = 0; way < 2; ++way)
			for (int sound = 0; sound < 2; ++sound) prompt(&t, place_order_sorted[k][way], sound);
	words = t.full ? 0 : ta_commit(&t);
	say = script_of(false);
}

/* ---- the keys (bn6f sub_811FE7C's, the pack's list) ---- */

/* frames as BN6 times them, h:m:s and hundredths in BCD (bn6f
 * memory_bcd_8000D84; a u16 never reaches its cap) */
static uint32_t bcd2(unsigned v) { return (v / 10 % 10) << 4 | v % 10; }
static uint32_t time_bcd(unsigned frames) {
	return bcd2(frames / 216000) << 24 | bcd2(frames % 216000 / 3600) << 16 | bcd2(frames % 3600 / 60) << 8 | bcd2(frames % 60 * 100 / 60);
}

/* a power BN6 computes, a Navi chip's, by MegaMan's best time against the
 * Navi (bn6f sub_8010C76) */
static int computed_power(int power) {
	if (power < BN6_NAVI_POWER_FIRST) return power;
	uint32_t n = (uint32_t)(power - BN6_NAVI_POWER_FIRST), t = time_bcd(emu_read16(BN6_NAVI_TIMES + 2 * n)), step = 0;
	while (step < BN6_NAVI_TIME_STEP_COUNT && t > emu_read32(BN6_NAVI_TIME_STEPS + 4 * step)) ++step;
	return emu_read16(BN6_NAVI_POWERS + BN6_NAVI_POWER_STEPS * 2 * n + 2 * step);
}

/* the Attack sort's (bn6f sub_811FF80) */
static uint32_t attack(const uint8_t *rec) {
	uint8_t flags = rec[BN6_CHIP_EFFECT_FLAGS];
	if (flags & BN6_SORT_ODD_FLAG) return BN6_SORT_POWER_ODD;
	if (!(flags & BN6_CHIP_SHOWS_POWER)) return BN6_SORT_NO_POWER;
	int power = get16(rec + BN6_CHIP_ATTACK_POWER);
	return (uint32_t)(power > BN6_SORT_COMPUTED ? computed_power(power) : power);
}

/* MegaMan's copies of `entry` (chip | code << 9): the pack's count, which
 * outside the folder editor holds the folder's copies too (second_read.c) */
static uint16_t pack[PACK_MOST];
static uint8_t pack_count[PACK_MOST];
static int npack;
static uint32_t held(uint16_t entry) {
	for (int i = 0; i < npack; ++i)
		if (pack[i] == entry) return pack_count[i];
	return 0;
}

typedef struct {
	uint8_t e[BN6_SHOP_ENTRY];
	uint32_t key, tie;
} Row;

static uint32_t key_of(const uint8_t *rec, int chip, int code) {
	switch (key) {
	case SORT_ABCDE: return get16(rec + BN6_CHIP_ALPHA_SORT);
	case SORT_CODE: return (uint32_t)code;
	case SORT_ATTACK: return attack(rec);
	case SORT_ELEMENT: {
		uint8_t e = rec[BN6_CHIP_ELEMENT];
		return e < BN6_SORT_ELEMENT_COUNT ? emu_read8(BN6_SORT_ELEMENTS + e) : e;
	}
	case SORT_NO: return held((uint16_t)(chip | code << 9));
	case SORT_MB: return rec[BN6_CHIP_MB];
	default: return get16(rec + BN6_CHIP_ID_SORT);
	}
}

static int by_key(const void *a, const void *b) {
	const Row *x = a, *y = b;
	if (x->key != y->key) return x->key < y->key ? -1 : 1;
	return x->tie < y->tie ? -1 : x->tie > y->tie;
}

/* BN6_SHOP_SOURCE in the session's sort, the list made again from it */
static void sort_list(void) {
	static Row rows[ROWS];
	int n = 0;
	for (; n < ROWS; ++n) {
		for (uint32_t b = 0; b < BN6_SHOP_ENTRY; ++b) rows[n].e[b] = emu_read8(BN6_SHOP_SOURCE + BN6_SHOP_ENTRY * (uint32_t)n + b);
		if (!rows[n].e[0]) break;
	}
	if (key == SORT_NO) npack = director_pack_now(pack, pack_count, PACK_MOST);
	/* (a key newly picked BN6 sorts the way of its byte XOR BN6_SORT_DOWN,
	 * SELECT the other; down, the key's values inverted as BN6 inverts
	 * them, its ties by ID up either way) */
	bool down = !(emu_read8(BN6_SORT_MODES + (uint32_t)key) & BN6_SORT_DOWN) != flipped;
	for (int i = 0; i < n; ++i) {
		int chip = get16(rows[i].e + 2), code = rows[i].e[4];
		const uint8_t *rec = chips_record(chip);
		uint32_t k = rec ? key_of(rec, chip, code) : 0;
		rows[i].key = down ? ~k : k;
		rows[i].tie = rec ? (uint32_t)get16(rec + BN6_CHIP_ID_SORT) << 7 | (uint32_t)code : 0;
	}
	qsort(rows, (size_t)n, sizeof *rows, by_key);
	for (int i = 0; i < n; ++i) emu_write(BN6_SHOP_SOURCE + BN6_SHOP_ENTRY * (uint32_t)i, rows[i].e, BN6_SHOP_ENTRY);
	emu_write8(BN6_SHOP_REDRAW, 0xFF);
}

/* a key on his list: the sort it asks for, the cursor at the top, and
 * BN6's way back to the list, which has his box say it */
static void resort(uint16_t keys) {
	if (keys & KEY_START) {
		key = (key + 1) % PLACE_SORT_KEYS;
		flipped = false;
	} else flipped = !flipped;
	picked = true;
	sort_list();
	static const uint8_t top[2] = { 0, 0 };
	emu_write(BN6_SHOP_ROW, top, sizeof top);
	emu_write(BN6_SHOP_SCROLL, top, sizeof top);
	emu_write8(BN6_SHOP_STATE, BN6_SHOP_AT_AGAIN);
	say = script_of(true);
}

void order_sort_frame(void) {
	static bool open;
	if (!words || emu_read8(emu_read32(BN6_TOOLKIT)) != BN6_MODE_SHOP || emu_read32(BN6_SHOP_DESC) != ORDER_DESC) {
		open = picked = false;
		say = script_of(false);
		return;
	}
	int at = emu_read8(BN6_SHOP_STATE);
	/* (as it opens, under its fade: in the session's sort, made again on
	 * the next frame) */
	if (!open && at) {
		open = true;
		if (key != SORT_ID || flipped) sort_list();
	}
	if (at != BN6_SHOP_AT_LIST) return;
	/* (back on the list: the sort said, his box quiet after) */
	say = script_of(false);
	uint16_t keys = emu_read16(BN6_KEYS_PRESSED);
	if (keys & (KEY_START | KEY_SELECT)) resort(keys);
}
