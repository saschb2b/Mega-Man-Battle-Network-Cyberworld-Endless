/* MegaMan's look onto his palettes in the ROM copy (look_apply.h;
 * docs/game/looks.md, How a pick colours a ramp). Each of his arts lists its
 * own indexes per channel (docs/rom/megaman-colours.md). BN6's own colours
 * are read from the player's ROM image (R.data, never written), so a look
 * is computed afresh each time and LOOK_NONE is BN6's bytes again.
 *
 * A pick sets a channel's average: the ramp's mean hue, saturation and
 * value (of BN6's plain palette) become the pick's, and each shade keeps
 * its place around that mean (its hue offset, its saturation and value
 * levelled: below the mean scaled toward 0, above it toward full). So a
 * blue pick near BN6's own gives BN6's ramp back, a pale one a pale ramp,
 * and a grey, white or black pick (no saturation) a shaded grey, white or
 * black: its saturation and value come from the pick, its shading from
 * BN6. The arithmetic is integer, so both players of a NetBattle compute
 * the same bytes on any system. */
#include "look_apply.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "rom.h"

#define ONE 4096                 /* fixed point: saturation and value 0 .. ONE, hue 0 .. 6 * ONE */
#define HUES (6 * ONE)
#define GBA_ROM 0x08000000u

typedef struct { uint8_t first, last; } Span;   /* a ramp's indexes, {0, 0} none (index 0 is clear) */
typedef struct { int h, s, v; } Hsv;

/* Each art's indexes by channel (armour, suit, accent), and the palette its
 * means are taken from (battle palettes 1 and 4 follow palette 0's) */
static const struct {
	Span ramp[LOOK_CHANNELS];
} arts[LOOK_SPRITE_COUNT] = {
	[LOOK_SPRITE_BATTLE] = { { { BN6_LOOK_ARMOUR_FIRST, BN6_LOOK_ARMOUR_LAST }, { BN6_LOOK_SUIT_FIRST, BN6_LOOK_SUIT_LAST }, { BN6_LOOK_GOLD, BN6_LOOK_GOLD } } },
	[LOOK_SPRITE_BATTLE_PALE] = { { { BN6_LOOK_ARMOUR_FIRST, BN6_LOOK_ARMOUR_LAST }, { BN6_LOOK_SUIT_FIRST, BN6_LOOK_SUIT_LAST }, { BN6_LOOK_GOLD, BN6_LOOK_GOLD } } },
	[LOOK_SPRITE_BATTLE_SYNCHRO] = { { { BN6_LOOK_ARMOUR_FIRST, BN6_LOOK_ARMOUR_LAST }, { BN6_LOOK_SUIT_FIRST, BN6_LOOK_SUIT_LAST }, { BN6_LOOK_GOLD, BN6_LOOK_GOLD } } },
	[LOOK_SPRITE_FIELD] = { { { BN6_LOOK_FIELD_BLUE_FIRST, BN6_LOOK_FIELD_SUIT_FIRST - 1 }, { BN6_LOOK_FIELD_SUIT_FIRST, BN6_LOOK_FIELD_BLUE_LAST }, { BN6_LOOK_FIELD_GOLD, BN6_LOOK_FIELD_GOLD } } },
	[LOOK_SPRITE_MUGSHOT] = { { { BN6_LOOK_MUGSHOT_BLUE_FIRST, BN6_LOOK_MUGSHOT_BLUE_LAST }, { 0, 0 }, { BN6_LOOK_MUGSHOT_GOLD, BN6_LOOK_MUGSHOT_GOLD } } },
	[LOOK_SPRITE_FACE] = { { { BN6_LOOK_FACE_BLUE_FIRST, BN6_LOOK_FACE_BLUE_LAST }, { 0, 0 }, { 0, 0 } } },
};

/* ---- colour arithmetic, integer ---- */

static int up(int c5) { return c5 * ONE / 31; }
static int down(int x) { return (x * 31 + ONE / 2) / ONE; }

static Hsv hsv_of(uint16_t c) {
	int r = up(c & 31), g = up(c >> 5 & 31), b = up(c >> 10 & 31);
	int mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b), d = mx - mn;
	Hsv o = { 0, mx ? d * ONE / mx : 0, mx };
	if (!d) return o;
	if (mx == r) o.h = (g - b) * ONE / d;
	else if (mx == g) o.h = 2 * ONE + (b - r) * ONE / d;
	else o.h = 4 * ONE + (r - g) * ONE / d;
	if (o.h < 0) o.h += HUES;
	return o;
}

static uint16_t rgb_of(Hsv c) {
	int h = ((c.h % HUES) + HUES) % HUES, i = h / ONE, f = h % ONE, v = c.v, s = c.s;
	int p = v * (ONE - s) / ONE, q = v * (ONE - s * f / ONE) / ONE, t = v * (ONE - s * (ONE - f) / ONE) / ONE;
	int rgb[6][3] = { { v, t, p }, { q, v, p }, { p, v, t }, { p, q, v }, { t, p, v }, { v, p, q } };
	return (uint16_t)(down(rgb[i][0]) | down(rgb[i][1]) << 5 | down(rgb[i][2]) << 10);
}

/* A hue's offset from `from`, wrapped into half a turn either way */
static int hue_off(int h, int from) {
	int d = h - from;
	while (d >= HUES / 2) d -= HUES;
	while (d < -HUES / 2) d += HUES;
	return d;
}

/* x placed around the mean m as t places the mean (0 and ONE stay) */
static int level(int x, int m, int t) {
	if (m <= 0) return t;
	if (m >= ONE) return x * t / ONE;
	return x <= m ? x * t / m : t + (x - m) * (ONE - t) / (ONE - m);
}

/* ---- BN6's own palettes ---- */

static bool rom_pal(uint32_t at, uint16_t out[16]) {
	if (!R.data || at < GBA_ROM || at + 32 > GBA_ROM + ROM_SIZE) return false;
	const uint8_t *p = R.data + (at - GBA_ROM);
	for (int i = 0; i < 16; ++i) out[i] = (uint16_t)(p[2 * i] | p[2 * i + 1] << 8);
	return true;
}

/* Sprite `which`'s palette in BN6's own ROM, and where the core's copy keeps it */
static uint32_t pal_at(int which, bool in_core) {
	switch (which) {
	case LOOK_SPRITE_BATTLE: return BN6_LOOK_BATTLE_PALS + BN6_LOOK_PAL_PLAIN * BN6_LOOK_PAL_SIZE;
	case LOOK_SPRITE_BATTLE_PALE: return BN6_LOOK_BATTLE_PALS + BN6_LOOK_PAL_PALE * BN6_LOOK_PAL_SIZE;
	case LOOK_SPRITE_BATTLE_SYNCHRO: return BN6_LOOK_BATTLE_PALS + BN6_LOOK_PAL_SYNCHRO * BN6_LOOK_PAL_SIZE;
	case LOOK_SPRITE_FIELD: return BN6_LOOK_FIELD_PALS;
	case LOOK_SPRITE_MUGSHOT: return BN6_LOOK_MUGSHOT_PAL;
	/* (face 0's: soullook.c's copy in the core, the word repointed) */
	case LOOK_SPRITE_FACE: return in_core ? emu_read32(BN6_FACE_PALS_REF) : BN6_FACE_PALS;
	default: return 0;
	}
}

/* The palette a sprite's means are taken from: battle's plain one for its three */
static int base_of(int which) {
	return which == LOOK_SPRITE_BATTLE_PALE || which == LOOK_SPRITE_BATTLE_SYNCHRO ? LOOK_SPRITE_BATTLE : which;
}

/* One channel's ramp in `pal` coloured by pick `c`, its means from `base` */
static void colour_ramp(uint16_t pal[16], const uint16_t base[16], Span sp, uint16_t c) {
	int n = sp.last - sp.first + 1, first_h = hsv_of(base[sp.first]).h, dh = 0, s = 0, v = 0;
	for (int i = sp.first; i <= sp.last; ++i) {
		Hsv o = hsv_of(base[i]);
		dh += hue_off(o.h, first_h);
		s += o.s;
		v += o.v;
	}
	Hsv mean = { first_h + dh / n, s / n, v / n }, pick = hsv_of(c);
	for (int i = sp.first; i <= sp.last; ++i) {
		Hsv o = hsv_of(pal[i]);
		Hsv out = { pick.h + hue_off(o.h, mean.h), level(o.s, mean.s, pick.s), level(o.v, mean.v, pick.v) };
		pal[i] = rgb_of(out);
	}
}

bool look_palette_of(const Look *look, int which, uint16_t out16[16]) {
	uint16_t pal[16], base[16];
	if (which < 0 || which >= LOOK_SPRITE_COUNT || !rom_pal(pal_at(which, false), pal) || !rom_pal(pal_at(base_of(which), false), base))
		return false;
	for (int ch = 0; look && look_valid(look) && ch < LOOK_CHANNELS; ++ch) {
		Span sp = arts[which].ramp[ch];
		if (!sp.first || look->palette[ch] == LOOK_NONE) continue;
		colour_ramp(pal, base, sp, looks_colour(look->palette[ch], look->colour[ch]));
	}
	memcpy(out16, pal, sizeof pal);
	return true;
}

/* ---- the core's copy ---- */

static Look applied;
static bool applied_on;

/* Whether the core's OBJ work palette holds sprite `which`'s palette for the
 * look applied: a slot given this frame, or a fixed one */
static bool seen(int which) {
	uint16_t want[16];
	if (!look_palette_of(&applied, which, want)) return false;
	for (int slot = 0; slot < 16; ++slot) {
		bool same = true;
		for (int i = 1; i < 16 && same; ++i) same = emu_read16(BN6_OBJ_PAL_WORK + 32u * (unsigned)slot + 2u * (unsigned)i) == want[i];
		if (same) return true;
	}
	return false;
}

static void describe_seen(FILE *f) {
	static const char *const names[LOOK_SPRITE_COUNT] = { "battle", "field", "mugshot", "face", "pale", "synchro" };
	fprintf(f, "lookseen");
	int n = 0;
	for (int w = 0; applied_on && w < LOOK_SPRITE_COUNT; ++w)
		if (seen(w)) fprintf(f, " %s", names[w]), ++n;
	fprintf(f, "%s\n", n ? "" : " none");
}

void look_apply(const Look *look) {
	Look plain = look_plain();
	if (!look || !look_valid(look)) look = &plain;
	if (!emu_ready()) return;
	/* (no claim: a recolour of BN6's own bytes and of soullook.c's copy of
	 * the faces, rewritten whenever the look changes, not an install) */
	for (int w = 0; w < LOOK_SPRITE_COUNT; ++w) {
		uint16_t pal[16];
		uint32_t at = pal_at(w, true);
		if (!at || !look_palette_of(look, w, pal)) continue;
		uint8_t bytes[32];
		for (int i = 0; i < 16; ++i) bytes[2 * i] = (uint8_t)pal[i], bytes[2 * i + 1] = (uint8_t)(pal[i] >> 8);
		emu_write(at, bytes, sizeof bytes);
	}
	applied = *look;
	applied_on = R.data != NULL;
	looks_describe_with(describe_seen);
}

static void apply_worn(void) {
	Look w = looks_worn();
	look_apply(&w);
}

void look_apply_worn(void) {
	looks_on_change(apply_worn);
	apply_worn();
}
