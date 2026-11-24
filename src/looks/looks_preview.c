/* looks_preview.h. The field's walking animations are 8-15, one a
 * direction going round (8 his back, 12 his front); the battle body's 0
 * stands, 9 fires the buster, 12 throws, 8 fires a charged shot (read from
 * the player's ROM, sprite lists 6's 0x37 and 0's 0); his face is list 8's
 * 0x37. A channel's indexes in each art are docs/rom/megaman-colours.md's. */
#include "looks_preview.h"

#include <string.h>

#include "bn6.h"
#include "look_apply.h"
#include "rom.h"

#define MEGAMAN 0x37    /* his field sprite's and his face's number in their lists */
#define WALK 8          /* the field's first walking animation */
#define TURN 40         /* frames each direction is walked */
#define STAND 0         /* the battle body's animations */
#define BUSTER 9
#define ONCE -1         /* a step held until its animation ends */
#define FOREVER 0       /* ... a step held until the course changes */
#define MARK 32         /* a channel's flash: frames, on and off by 4 */

typedef struct { bool battle; int anim, frames; } Step;

static const Step loop[] = {
	{ false, WALK + 4, TURN }, { false, WALK + 5, TURN }, { false, WALK + 6, TURN }, { false, WALK + 7, TURN },
	{ false, WALK + 0, TURN }, { false, WALK + 1, TURN }, { false, WALK + 2, TURN }, { false, WALK + 3, TURN },
	{ false, WALK + 4, TURN },
	{ true, STAND, 40 }, { true, BUSTER, ONCE }, { true, STAND, 24 }, { true, 12, ONCE }, { true, STAND, 24 }, { true, 8, ONCE }, { true, STAND, 40 },
};
static const Step hold[] = { { true, STAND, 8 }, { true, BUSTER, ONCE }, { true, STAND, FOREVER } };

/* each channel's indexes in each art, first to last (0 0: none) */
static const uint8_t ramps[LOOKS_ARTS][LOOK_CHANNELS][2] = {
	[LOOKS_ART_FIELD] = { { 2, 5 }, { 6, 8 }, { 10, 10 } },
	[LOOKS_ART_BATTLE] = { { BN6_LOOK_ARMOUR_FIRST, BN6_LOOK_ARMOUR_LAST }, { BN6_LOOK_SUIT_FIRST, BN6_LOOK_SUIT_LAST }, { BN6_LOOK_GOLD, BN6_LOOK_GOLD } },
	[LOOKS_ART_FACE] = { { BN6_LOOK_MUGSHOT_BLUE_FIRST, BN6_LOOK_MUGSHOT_BLUE_LAST }, { 0, 0 }, { 11, 11 } },
};

static const Step *course(const LooksPreview *p, int *n) {
	*n = p->hold ? (int)(sizeof hold / sizeof *hold) : (int)(sizeof loop / sizeof *loop);
	return p->hold ? hold : loop;
}

static void play(LooksPreview *p) {
	int n;
	const Step *c = course(p, &n);
	p->battle = c[p->step].battle;
	p->held = 0;
	Sprite *s = p->battle ? sprite_get(SPR_BATTLE, 0) : sprite_get(SPR_NPC, MEGAMAN);
	if (s) anim_play(&p->anim, s, c[p->step].anim);
	else p->anim.spr = NULL;
}

void looks_preview_loop(LooksPreview *p) {
	p->hold = false;
	p->step = 0;
	play(p);
}

void looks_preview_hold(LooksPreview *p) {
	p->hold = true;
	p->step = 0;
	play(p);
}

void looks_preview_front(LooksPreview *p) {
	if (p->hold) looks_preview_hold(p);
	else looks_preview_loop(p);
}

void looks_preview_mark(LooksPreview *p, int c) {
	p->mark = c;
	p->mark_t = MARK;
}

void looks_preview_colours(LooksPreview *p, const Look *look) {
	static const int which[LOOKS_ARTS] = { LOOK_SPRITE_FIELD, LOOK_SPRITE_BATTLE, LOOK_SPRITE_MUGSHOT };
	static const uint32_t own[LOOKS_ARTS] = { BN6_LOOK_FIELD_PALS, BN6_LOOK_BATTLE_PALS, BN6_LOOK_MUGSHOT_PAL };
	for (int k = 0; k < LOOKS_ARTS; ++k) {
		uint16_t pal[16];
		p->have[k] = look_palette_of(look, which[k], pal);
		/* (none from the core: BN6's own, from the player's ROM) */
		if (!p->have[k] && R.data) {
			memcpy(p->pal[k], R.data + (own[k] - 0x08000000u), 32);
			p->have[k] = true;
			continue;
		}
		for (int i = 0; p->have[k] && i < 16; ++i) {
			p->pal[k][2 * i] = (uint8_t)pal[i];
			p->pal[k][2 * i + 1] = (uint8_t)(pal[i] >> 8);
		}
	}
}

void looks_preview_step(LooksPreview *p) {
	if (p->mark_t > 0) --p->mark_t;
	if (!p->anim.spr) return;
	anim_update(&p->anim);
	++p->held;
	int n;
	const Step *c = course(p, &n);
	int frames = c[p->step].frames;
	if (frames == FOREVER || (frames == ONCE ? !p->anim.done : p->held < frames)) return;
	p->step = (p->step + 1) % n;
	play(p);
}

/* sprite `s` in art k's palette, the channel marked white while it flashes */
static void sprite(const LooksPreview *p, Sprite *s, int anim, int frame, int k, int x, int y) {
	if (!s) return;
	if (!p->have[k]) {
		sprite_draw_frame(s, anim, frame, x, y, false, 0, 0);
		return;
	}
	uint8_t pal[32];
	memcpy(pal, p->pal[k], 32);
	const uint8_t *r = p->mark >= 0 && p->mark < LOOK_CHANNELS ? ramps[k][p->mark] : NULL;
	if (r && r[0] && p->mark_t > 0 && p->mark_t / 4 % 2)
		for (int i = r[0]; i <= r[1]; ++i) pal[2 * i] = 0xFF, pal[2 * i + 1] = 0x7F;
	sprite_draw_palette(s, anim, frame, x, y, false, pal);
}

void looks_preview_draw(const LooksPreview *p, int x, int y, int w, int h) {
	/* (his face as a chat box draws it, its origin its middle: 40 x 48) */
	sprite(p, sprite_get(SPR_MUGSHOT, MEGAMAN), 0, 0, LOOKS_ART_FACE, x + w / 2, y + 24);
	if (p->anim.spr)
		sprite(p, p->anim.spr, p->anim.anim, p->anim.frame, p->battle ? LOOKS_ART_BATTLE : LOOKS_ART_FIELD, x + w / 2 + 1, y + h - 3);
}
