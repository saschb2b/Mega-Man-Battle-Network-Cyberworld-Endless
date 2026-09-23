#include "audio.h"
#include "game.h"
#include "gfx.h"
#include "platform.h"
#include "run.h"
#include "save.h"

static int t;
bool gameover_summary_only; /* the game already played its own GAME OVER */

/* The original GAME OVER screen runs 224 frames (fading in over 10, its
 * music from 6, the streaked grid racing right 16 pixels a frame, fading
 * out from 180); the run summary follows it. */
#define GO_FRAMES 224

static void enter(void) {
	t = gameover_summary_only ? GO_FRAMES : 0;
	gameover_summary_only = false;
	run.active = false;
}

static void update(void) {
	++t;
	if (t == 6) audio_music(MUS_GAMEOVER);
	if (t > GO_FRAMES + 60 && (btn_pressed(BTN_A) || btn_pressed(BTN_START))) scene_set(&scene_title);
}

static void draw_game_over(void) {
	fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 255));
	int shift = (t * 16) % 256;
	for (int x = P.core_x - 256 + shift - 256; x < P.w; x += 256)
		for (int y = P.core_y - 160; y < P.h; y += 160) hud_window(HUD_GAMEOVER_BG, x, y);
	hud_window(HUD_GAMEOVER_TEXT, P.core_x, P.core_y);
	if (t < 10) { P.fx_fade = 16 - (t * 16) / 10; P.fx_fade_color = BLACK; }
	if (t >= 180) { P.fx_fade = (t - 180) / 2 > 16 ? 16 : (t - 180) / 2; P.fx_fade_color = BLACK; } /* 32-frame fade out */
}

static void draw(void) {
	if (t < GO_FRAMES) { draw_game_over(); return; }
	fill_rect(0, 0, P.w, P.h, rgba(8, 4, 24, 255));
	int x = P.w / 2, y = P.core_y + 20;
	text_draw(x, y, "MEGAMAN DELETED", rgba(255, 90, 90, 255), TEXT_CENTER);
	text_drawf(x, y + 28, WHITE, TEXT_CENTER, "Reached B%d", run.depth);
	text_drawf(x, y + 44, WHITE, TEXT_CENTER, "Viruses deleted: %d", run.viruses_deleted);
	text_drawf(x, y + 60, WHITE, TEXT_CENTER, "Navis deleted: %d", run.bosses_beaten);
	text_drawf(x, y + 84, rgba(255, 230, 90, 255), TEXT_CENTER, "Best: B%d", profile.best_depth);
	if (t > GO_FRAMES + 60 && (t / 20) & 1) text_draw(x, y + 112, "Press A", rgba(200, 210, 255, 255), TEXT_CENTER);
	if (t < GO_FRAMES + 16) { P.fx_fade = GO_FRAMES + 16 - t; P.fx_fade_color = BLACK; }
}

const Scene scene_gameover = { "gameover", enter, update, draw, NULL };
