/* The super bosses' staging, after BN6's own scenes for Bass and the
 * Cybeast (bn6f's cutscenes before their battles; docs/BOSSES.md, Super
 * bosses): the music fades as the scene begins, the screen shakes with
 * the rumble every 32 frames, the music stops and the screen fades to
 * white with its sound, the boss stands there out of the white, and the
 * prelude starts after it. Bass's stone cracks before he wakes; the
 * Cybeast rears and roars. A rematch skips the first rumbles: the long
 * entrance is for the first meeting (the transitions' budget: rare, so it
 * may take its time). */
#include "boss_grand.h"

#include <stdio.h>

#include "bn6.h"
#include "cinema.h"
#include "emu.h"
#include "flags.h"
#include "gamecall.h"
#include "gfx.h"
#include "guardians.h"
#include "run.h"
#include "super_lines.h"
#include "talk.h"

#define RUMBLE_EVERY  32    /* BN6's own: its cutscenes shake and rumble every 0x20 frames */
#define QUAKE_EVERY   150   /* the approach's tremors before the Cybeast */
#define REMATCH_SKIP  80    /* the frames of the entrance a rematch skips: its first two rumbles */
#define CARD_FRAMES   260

/* The entrance's beats, at frames of the first meeting's */
enum {
	T_RUMBLE = 20,     /* the first rumble; then every RUMBLE_EVERY, five in all */
	T_SEAL = 100,      /* Bass's stone shakes and cracks */
	T_WHITE = 160,     /* the fade to white, with its sound: in 12 frames, held 40, out 24 */
	T_APPEAR = 172,    /* in the white: he stands there, the stone in pieces */
	T_PRELUDE = 212,   /* the prelude, as the white lifts */
	T_POSE = 236,      /* the white gone, his pose: Bass's cloak, the beast's roar */
	T_CARD = 276,      /* his title card */
};

static struct {
	GuardianStage g;
	uint32_t archive;
	bool rematch;      /* battled in any run before: the entrance cut short */
	bool faded;        /* the theme faded on the approach */
	int quake;         /* frames to the next tremor */
} G;

void grand_begin(uint32_t archive, const GuardianStage *g) {
	G.g = *g;
	G.archive = archive;
	G.rematch = guardian_known(g->navi);
	G.faded = false;
	G.quake = 0;
}

static void play(int script) {
	if (script >= 0) game_call(BN6_CHAT_RUN_SCRIPT, G.archive, (uint32_t)script);
}

void grand_approach(bool near) {
	/* (never over a chat: a script run now would take its place) */
	if (emu_read8(BN6_CHATBOX) || talk_busy()) return;
	if (near && !G.faded) {
		play(G.g.fade);
		G.faded = true;
		G.quake = 90;
	} else if (!near && G.faded) {
		play(G.g.theme);
		G.faded = false;
	}
	if (near && G.g.navi == SUPER_CYBEAST && --G.quake <= 0) {
		cinema_shake(14, 1);
		play(G.g.rumble);
		G.quake = QUAKE_EVERY;
	}
}

/* His title card: the top line and the line under his name his own
 * (super_lines.c), his name with his form */
static void grand_card(void) {
	const Guardian *gd = guardian(G.g.navi);
	const char *top, *sub;
	char name[40];
	super_card(G.g.navi, G.g.version, run.depth, &top, &sub);
	snprintf(name, sizeof name, "%s%s", gd->name, super_suffix(G.g.navi, G.g.version));
	cinema_title_grand(top, name, sub, gd->epithet, rgba(gd->r, gd->g, gd->b, 255), CARD_FRAMES);
}

bool grand_enter(int t) {
	int k = t + (G.rematch ? REMATCH_SKIP : 0);
	/* the rumble, harder each time */
	if (k >= T_RUMBLE && k < T_WHITE && (k - T_RUMBLE) % RUMBLE_EVERY == 0) {
		cinema_shake(16, 1 + (k - T_RUMBLE) / (2 * RUMBLE_EVERY));
		play(G.g.rumble);
	}
	if (k == T_SEAL && G.g.navi == SUPER_BASS) flag_set(LAYER_SUPER_SEAL_FLAG);
	if (k == T_WHITE) {
		cinema_whiteout(T_APPEAR - T_WHITE, T_PRELUDE - T_APPEAR, T_POSE - T_PRELUDE);
		play(G.g.reveal);
	}
	if (k == T_APPEAR) flag_set(LAYER_BOSS_APPEAR_FLAG);
	if (k == T_PRELUDE) play(G.g.prelude);
	if (k == T_POSE) {
		flag_set(LAYER_SUPER_POSE_FLAG);
		cinema_shake(30, 4);
		if (G.g.navi == SUPER_CYBEAST) play(G.g.rumble);
	}
	if (k < T_CARD) return false;
	grand_card();
	return true;
}

bool grand_fall(int t) {
	if (G.g.navi == SUPER_BASS) {
		/* he goes as BN6's Bass goes: its sound, a white */
		if (t == 1) {
			cinema_whiteout(8, 24, 30);
			play(G.g.depart);
			flag_set(LAYER_BOSS_GONE_FLAG);
		}
		return t >= 70;
	}
	/* the beast's last roar shakes the floor through a long white */
	if (t == 1) {
		cinema_whiteout(24, 40, 40);
		flag_set(LAYER_BOSS_GONE_FLAG);
	}
	if (t < 90 && t % 20 == 1) {
		cinema_shake(18, 3 - t / 40);
		play(G.g.rumble);
	}
	return t >= 120;
}

bool grand_theme_after(void) { return G.g.navi != SUPER_CYBEAST; }
