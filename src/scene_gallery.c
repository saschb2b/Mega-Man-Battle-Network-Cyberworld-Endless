/* Developer gallery: browse every ROM sprite animation.
 * L/R category, Left/Right sprite, Up/Down animation, A cycle palette. */
#include <stdio.h>

#include "game.h"
#include "gfx.h"
#include "platform.h"

static int cat = 1, idx = 1, anim, pal;
static Anim a;

static void reload(void) {
	Sprite *s = sprite_get(cat, idx);
	int n = sprite_anim_count(s);
	if (anim >= n) anim = 0;
	if (anim < 0) anim = n > 0 ? n - 1 : 0;
	if (s) anim_play(&a, s, anim);
	else a.spr = NULL;
}

static void enter(void) { reload(); }

static void update(void) {
	bool change = true;
	if (btn_repeat(BTN_RIGHT)) ++idx;
	else if (btn_repeat(BTN_LEFT)) --idx;
	else if (btn_repeat(BTN_UP)) --anim;
	else if (btn_repeat(BTN_DOWN)) ++anim;
	else if (btn_pressed(BTN_R)) { cat = (cat + 1) % 10; idx = 0; }
	else if (btn_pressed(BTN_L)) { cat = (cat + 9) % 10; idx = 0; }
	else if (btn_pressed(BTN_A)) pal = (pal + 1) % 6;
	else if (btn_pressed(BTN_B)) scene_set(&scene_title);
	else change = false;
	if (idx < 0) idx = 0;
	if (idx > 255) idx = 255;
	if (change) reload();
	if (a.spr) {
		anim_update(&a);
		if (a.done) anim_play(&a, a.spr, a.anim);
	}
}

static void draw(void) {
	battle_bg_draw(0x07, (int)P.frame);
	for (int x = 0; x < 6; ++x)
		for (int y = 0; y < 3; ++y)
			panel_draw(2, y, x >= 3, P.core_x + x * 40, P.core_y + 72 + y * 24);
	anim_draw(&a, P.core_x + 120, P.core_y + 120, false, pal, 0);
	text_drawf(4, 2, WHITE, TEXT_LEFT, "cat %d  sprite %d  anim %d/%d  pal %d", cat, idx, anim,
		sprite_anim_count(a.spr), pal);
}

const Scene scene_gallery = { "gallery", enter, update, draw, NULL };
