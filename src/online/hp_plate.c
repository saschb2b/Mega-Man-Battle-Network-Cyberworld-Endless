/* A MegaMan's HP under him as BN6 writes an enemy's (hp_plate.h). */
#include "hp_plate.h"

#include <stdbool.h>
#include <stdio.h>

#include <mgba/core/core.h>

#include "bn6.h"
#include "bn6_panels.h"
#include "bytes.h"
#include "platform.h"
#include "rom.h"
#include "xtiles.h"

#define GLYPH 0x40      /* a glyph's bytes: its 8x16 cell's two tiles */
#define CATCH_UP 30     /* the battle's frames a plate is counted on through at a draw, at most (more: begun afresh) */

/* the ROM's bytes at bus address `a`, `n` of them; NULL where they are not in it */
static const uint8_t *rom(uint32_t a, uint32_t n) {
	return R.data && rom_is_ptr(a) && rom_off(a) + n <= ROM_SIZE ? R.data + rom_off(a) : NULL;
}

/* glyph `g` (a digit, or BN6_HP_FONT_GLYPHS - 1 the blank) of colour `font`, through BN6's own tables (BN6_HP_FONTS) */
static const uint8_t *glyph(unsigned font, unsigned g) {
	const uint8_t *fonts = rom(BN6_HP_FONTS + 4u * font, 4);
	const uint8_t *table = fonts ? rom(get32(fonts) + 4u * g, 4) : NULL;
	return table ? rom(get32(table), GLYPH) : NULL;
}

/* a frame of BN6's count toward `hp` (BN6_HP_PLATE_STEP): toward it by the gap's eighth and two, orange down and a
 * frame after, green up and ten after; orange too while poison counts on him */
static void step(HpPlate *p, unsigned hp, bool poisoned) {
	if (p->shown != hp) {
		bool up = p->shown < hp;
		unsigned gap = up ? hp - p->shown : p->shown - hp, by = (gap >> BN6_HP_PLATE_ROLL) + BN6_HP_PLATE_ROLL_ADD;
		p->font = up ? BN6_HP_FONT_GREEN : BN6_HP_FONT_ORANGE;
		p->timer = up ? BN6_HP_PLATE_GREEN : 1;
		p->shown = (uint16_t)(by >= gap ? hp : up ? p->shown + by : p->shown - by);
		return;
	}
	if (poisoned) {
		p->font = BN6_HP_FONT_ORANGE;
		p->timer = BN6_HP_PLATE_POISON;
		return;
	}
	if (p->timer && !--p->timer) p->font = BN6_HP_FONT_WHITE;
}

void hp_plate_draw(HpPlate *p, struct mCore *c, uint32_t o, int x, int y) {
	if (!p || !c || o < BN6_EWRAM || o >= BN6_EWRAM_END) return;
	unsigned hp = (unsigned)c->rawRead16(c, o + BN6_T1_HP, -1) & 0xFFFFu;
	uint32_t coll = (uint32_t)c->rawRead32(c, o + BN6_T1_COLLISION, -1);
	bool poisoned = coll >= BN6_EWRAM && coll < BN6_EWRAM_END && (c->rawRead8(c, coll + BN6_COLL_POISON_TIMER, -1) & 0xFF);
	uint32_t now = (uint32_t)c->rawRead32(c, BN6_BATTLE_TIMER, -1);
	if (!p->on || now < p->at || now - p->at > CATCH_UP) *p = (HpPlate){ .shown = (uint16_t)hp, .at = now, .on = true };
	for (; p->at < now; ++p->at) step(p, hp, poisoned);
	const uint8_t *pal = rom(BN6_HP_DIGITS_PAL, 32);
	char s[8];
	int n = snprintf(s, sizeof s, "%u", p->shown > 9999 ? 9999u : p->shown);   /* (BN6's four places) */
	for (int i = 0; pal && i < n; ++i)
		xtiles_draw(glyph(p->font, (unsigned)(s[i] - '0')), pal, x - BN6_HP_DIGIT_STEP * n / 2 + BN6_HP_DIGIT_STEP * i, y, 1, 2, 0);
}
