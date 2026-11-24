/* looks_preview.h. The field's walking animations are 8-15, one a
 * direction going round (8 his back, 12 his front); the battle body's 0
 * stands, 9 fires the buster, 12 throws, 8 fires a charged shot (read from
 * the player's ROM, sprite lists 6's 0x37 and 0's 0). */
#include "looks_preview.h"

#include <string.h>

#include "look_apply.h"

#define FIELD_SPRITE 0x37
#define WALK 8          /* the field's first walking animation */
#define TURN 40         /* frames each direction is walked */
#define STAND 0         /* the battle body's animations */
#define ONCE -1         /* a step held until its animation ends */

static const struct { bool battle; int anim, frames; } loop[] = {
	{ false, WALK + 4, TURN }, { false, WALK + 5, TURN }, { false, WALK + 6, TURN }, { false, WALK + 7, TURN },
	{ false, WALK + 0, TURN }, { false, WALK + 1, TURN }, { false, WALK + 2, TURN }, { false, WALK + 3, TURN },
	{ false, WALK + 4, TURN },
	{ true, STAND, 40 }, { true, 9, ONCE }, { true, STAND, 24 }, { true, 12, ONCE }, { true, STAND, 24 }, { true, 8, ONCE }, { true, STAND, 40 },
};
#define STEPS (int)(sizeof loop / sizeof *loop)

static void play(LooksPreview *p) {
	p->battle = loop[p->step].battle;
	p->held = 0;
	Sprite *s = p->battle ? sprite_get(SPR_BATTLE, 0) : sprite_get(SPR_NPC, FIELD_SPRITE);
	if (s) anim_play(&p->anim, s, loop[p->step].anim);
	else p->anim.spr = NULL;
}

void looks_preview_start(LooksPreview *p) {
	p->step = 0;
	play(p);
}

void looks_preview_colours(LooksPreview *p, const Look *look) {
	uint16_t pal[16];
	static const int which[2] = { LOOK_SPRITE_FIELD, LOOK_SPRITE_BATTLE };
	for (int k = 0; k < 2; ++k) {
		p->have[k] = look_palette_of(look, which[k], pal);
		for (int i = 0; p->have[k] && i < 16; ++i) {
			p->pal[k][2 * i] = (uint8_t)pal[i];
			p->pal[k][2 * i + 1] = (uint8_t)(pal[i] >> 8);
		}
	}
}

void looks_preview_step(LooksPreview *p) {
	if (!p->anim.spr) return;
	anim_update(&p->anim);
	++p->held;
	int frames = loop[p->step].frames;
	if (frames == ONCE ? !p->anim.done : p->held < frames) return;
	p->step = (p->step + 1) % STEPS;
	play(p);
}

void looks_preview_draw(const LooksPreview *p, int x, int y) {
	if (!p->anim.spr) return;
	int k = p->battle;
	x += k ? 3 : 0;   /* (the battle body's buster reaches left of his feet) */
	if (p->have[k]) sprite_draw_palette(p->anim.spr, p->anim.anim, p->anim.frame, x, y, false, p->pal[k]);
	else sprite_draw_frame(p->anim.spr, p->anim.anim, p->anim.frame, x, y, false, 0, 0);
}
