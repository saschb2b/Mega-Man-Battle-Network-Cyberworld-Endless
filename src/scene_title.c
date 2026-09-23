#include "audio.h"
#include "game.h"
#include "gfx.h"
#include "platform.h"
#include "run.h"
#include "save.h"

static Anim mm;
static int cursor, t;
static bool has_save;

static void enter(void) {
	anim_play(&mm, sprite_get(SPR_BATTLE, 0), 0);
	audio_music(MUS_TITLE);
	has_save = save_exists();
	cursor = 0;
	t = 0;
}

static void update(void) {
	++t;
	anim_update(&mm);
	if (mm.done) anim_play(&mm, mm.spr, 0);
	int items = has_save ? 2 : 1;
	if (btn_repeat(BTN_UP) || btn_repeat(BTN_DOWN)) { cursor = (cursor + 1) % items; audio_sfx(SFX_CURSOR); }
	if (t > 20 && (btn_pressed(BTN_START) || btn_pressed(BTN_A))) {
		audio_sfx(SFX_CONFIRM);
		bool cont = has_save && cursor == 0;
		if (cont && load_run()) net_resume();
		else {
			run_new(rng_next() ^ (uint32_t)SDL_GetTicks());
			net_reset();
		}
		scene_set(&scene_net);
	}
	if (btn_pressed(BTN_SELECT)) scene_set(&scene_gallery);
}

static void draw(void) {
	battle_bg_draw(0x0E, t);
	text_draw(P.w / 2, P.core_y + 16, "MEGA MAN BATTLE NETWORK", WHITE, TEXT_CENTER);
	text_draw(P.w / 2, P.core_y + 32, "CYBERWORLD ENDLESS", rgba(255, 230, 90, 255), TEXT_CENTER);
	anim_draw(&mm, P.w / 2, P.core_y + 104, false, 0, 0);
	const char *labels[2] = { "CONTINUE", "NEW RUN" };
	int n = has_save ? 2 : 1;
	for (int i = 0; i < n; ++i) {
		const char *l = has_save ? labels[i] : labels[1];
		bool sel = i == cursor;
		int y = P.core_y + 112 + i * 16;
		if (sel) fill_rect(P.w / 2 - 44, y + 1, 88, 14, rgba(20, 40, 110, 200));
		text_draw(P.w / 2, y, l, sel ? rgba(255, 240, 120, 255) : WHITE, TEXT_CENTER);
	}
	if (profile.runs) text_drawf(P.w / 2, P.core_y + CORE_H - 14, rgba(200, 210, 255, 255), TEXT_CENTER, "Best B%d  Runs %d", profile.best_depth, profile.runs);
}

const Scene scene_title = { "title", enter, update, draw, NULL };
