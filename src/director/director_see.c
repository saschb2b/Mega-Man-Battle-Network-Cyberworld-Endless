/* What the drawing reads of the game, taken after each frame (seen), and
 * what is drawn over the picture from it: CircusMan's tent, the Mystery Data
 * counters' state, the NaviCust's bug note and the duel's clock. */
#include "director_see.h"

#include <stdio.h>
#include <string.h>

#include "blockers.h"
#include "bn6_fields.h"
#include "boss.h"
#include "director.h"
#include "director_board.h"
#include "director_folder.h"
#include "director_state.h"
#include "gfx.h"
#include "net_shapes.h"
#include "platform.h"
#include "talk.h"

Seen seen;

/* CircusMan's tent: as his object's action turns to it (+0x09, 0x0C), BN6
 * lights the panel MegaMan stands on for a few frames, under his feet,
 * and drops the tent there; a step off it begun within 32 frames clears
 * it, and 36 frames in he is held (measured in god mode, a step at each
 * delay: the fade the briefing named came 6 frames before the hold, and a
 * playtester lost to him five times). The panel is marked over the picture
 * for those 36 frames of the battle's clock, which holds on the Custom
 * screen: not while its window covers the field (it was drawn over the
 * chips, issue #52), and again as SELECT hides the window to see it. */
#define TENT_ACTION 0x0C
#define TENT_FRAMES 36
static void see_tent(void) {
	static bool on;
	static uint32_t start;
	static int tx, ty;
	seen.tent_x = seen.tent_y = 0;
	if (!seen.battle || !boss_fighting() || D.objs.guardian.navi != 14 /* (CircusMan) */) { on = false; return; }
	int act = -1, mx = 0, my = 0, most = 0;
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if (!(emu_read8(o) & 1)) continue;
		/* (MegaMan's panel, and the side's biggest: CircusMan, not his lion) */
		if (emu_read8(o + BN6_T1_ALLIANCE) == 0 && emu_read8(o + BN6_T1_PANEL_X)) { mx = emu_read8(o + BN6_T1_PANEL_X); my = emu_read8(o + BN6_T1_PANEL_Y); }
		else if (emu_read8(o + BN6_T1_ALLIANCE) == 1 && emu_read16(o + BN6_T1_MAX_HP) > most) { most = emu_read16(o + BN6_T1_MAX_HP); act = emu_read8(o + BN6_T1_ACTION); }
	}
	if (act != TENT_ACTION) { on = false; return; }
	if (!on) { on = true; start = seen.timer; tx = mx; ty = my; }
	if (seen.timer - start < TENT_FRAMES) { seen.tent_x = tx; seen.tent_y = ty; }
}

/* Whether Mystery Data k stands on a panel the map shows. */
bool md_on_map(int k) {
	const NetObj *o = &layer.obj[D.objs.md_obj[k]];
	int x = (int)o->x, y = (int)o->y;
	return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && D.seen[y][x];
}

/* Whether MegaMan knows of the layer's Mystery Data k: taken, on a panel
 * the map shows (4 around where he has been: the picture shows no farther,
 * a panel 64 pixels across), or a set piece's he senses (L names them: a purple's lock,
 * a Rush gap's island, a teleport's, a lock's pocket); the invisible
 * path's pad not until seen, a secret whose navi's hint is its cue. A
 * count of all the layer holds had announced that secret, and made every
 * layer a checklist (the owner's, reasoned with the game-design skill). */
bool md_known(int k, bool taken) {
	const NetObj *o = &layer.obj[D.objs.md_obj[k]];
	int x = (int)o->x, y = (int)o->y;
	if (taken || md_on_map(k) || o->param == MD_PURPLE) return true;
	for (int p = 0; o->prize && p < layer.npaths; ++p) {
		const NetGap *g = &layer.path[p];
		if (x == g->x + dir_dx[g->dir] * (g->len + 2) && y == g->y + dir_dy[g->dir] * (g->len + 2)) return false;
	}
	return o->prize;
}

/* The layer's Mystery Data MegaMan knows of, taken and known, by colour:
 * the counters on the map's header, and over the picture a few seconds
 * after L (the owner's: a collector's count of what is left before the
 * one-way exit, where a playtester's sensed Rush gap and its prize were
 * lost with the layer, session 60). They hold through MegaMan's words,
 * then count down. */
#define COUNTS_AFTER 180

static void see_counts(void) {
	memset(seen.md_taken, 0, sizeof seen.md_taken);
	memset(seen.md_known, 0, sizeof seen.md_known);
	seen.md_marked = 0;
	/* (an Unlocker held: its lock on the map, as the dealer who sold it
	 * said where it lies; a playtester bought one and never found the
	 * purple data, session 61, the owner's call) */
	bool key = D.active && !D.town && key_item(SUB_UNLOCKER) > 0;
	for (int k = 0; D.active && !D.town && k < D.objs.nmd; ++k) {
		bool taken = flag_get(MAPSLOT_MD_FLAG + k);
		if (!md_known(k, taken)) continue;
		if (!taken && (md_on_map(k) || (key && D.objs.md_colour[k] == MYSTERY_PURPLE))) seen.md_marked |= (uint16_t)(1u << k);
		int c = D.objs.md_colour[k] == MYSTERY_GREEN ? 0 : D.objs.md_colour[k] == MYSTERY_BLUE ? 1 : 2;
		seen.md_known[c]++;
		seen.md_taken[c] += taken;
	}
	if (D.counts_t > 0 && !talk_busy()) --D.counts_t;
	bool shown = D.counts_t > 0 && D.active && !D.town && seen.on_map && !seen.battle && !D.map_shown;
	seen.counts_a = !shown ? 0 : D.counts_t < 16 ? D.counts_t * 255 / 16 : 255;
}

/* (any Mystery Data MegaMan knows of on the layer: the counters show) */
bool counts_any(void) { return seen.md_known[0] + seen.md_known[1] + seen.md_known[2] > 0; }

/* Whether the layer's lock k (a Link Navi obstacle, a security cube, a
 * skull or number door: layer.block[k]) still stands: its present flag,
 * cleared as it opens (blockers.c). */
bool lock_shut(int k) { return D.active && !D.town && k < layer.nblocks && flag_get(BLOCK_PRESENT_FLAG + k); }

static void see_locks(void) {
	seen.locks = 0;
	for (int k = 0; k < layer.nblocks && k < 8; ++k)
		if (lock_shut(k)) seen.locks |= (uint8_t)(1u << k);
}

/* L heard on the map: a press kept is spent, and the counters show */
void l_taken(void) {
	D.l_kept = 0;
	D.counts_t = COUNTS_AFTER;
}
#define NOTE_CALM 10    /* (as bug_watch's BUG_CALM) */

static void see_bug_note(void) {
	static uint8_t at_open[NAVICUST_BUGS], last[NAVICUST_BUGS];
	static bool open;
	static int calm;
	if (seen.bug_note_t > 0) --seen.bug_note_t;
	if (!D.active || D.town || main_mode() != PET_MODE) { open = false; seen.bug_note_t = 0; return; }
	uint8_t now[NAVICUST_BUGS];
	for (int t = 0; t < NAVICUST_BUGS; ++t) now[t] = emu_read8(BN6_NAVICUST_BUGS + (uint32_t)t);
	if (!open) { open = true; memcpy(at_open, now, sizeof now); memcpy(last, now, sizeof now); calm = 0; return; }
	calm = memcmp(now, last, sizeof now) ? 0 : calm + 1;
	memcpy(last, now, sizeof now);
	if (calm != NOTE_CALM || !memcmp(now, at_open, sizeof now)) return;
	memcpy(at_open, now, sizeof now);
	bool bug = false;
	for (int t = 1; t < NAVICUST_BUGS; ++t) bug |= now[t] != 0;
	const char *cause = bug ? bug_cause() : NULL;
	if (!bug) return;
	snprintf(seen.bug_note, sizeof seen.bug_note, "A bug! %s", cause ? cause : "Two programs of one colour touch, or one breaks the board's rules.");
	seen.bug_note_t = 300;
}

/* The note over the PET (see_bug_note), wrapped in a box at the top */
void director_draw_bug_note(void) {
	if (seen.bug_note_t <= 0) return;
	char lines[4][64];
	int n = 0, x0 = P.core_x + 4, y0 = P.core_y + 4, w = 232;
	const char *s = seen.bug_note;
	while (*s && n < 4) {
		int len = (int)strlen(s), cut = len;
		char buf[64];
		/* (as many words as fit the box's width) */
		for (int k = 1; k <= len && k < 63; ++k) {
			snprintf(buf, sizeof buf, "%.*s", k, s);
			if (text_width(buf) > w - 8) { cut = k - 1; break; }
		}
		if (cut < len) { int b = cut; while (b > 0 && s[b] != ' ') --b; if (b > 0) cut = b; }
		snprintf(lines[n++], sizeof lines[0], "%.*s", cut, s);
		s += cut;
		while (*s == ' ') ++s;
	}
	int a = seen.bug_note_t < 16 ? seen.bug_note_t * 255 / 16 : 255, h = n * TEXT_H + 6;
	fill_rect(x0, y0, w, h, rgba(40, 0, 16, (Uint8)(220 * a / 255)));
	fill_rect(x0, y0 + h - 1, w, 1, rgba(255, 110, 110, (Uint8)(220 * a / 255)));
	for (int i = 0; i < n; ++i) text_draw(x0 + 4, y0 + 3 + i * TEXT_H, lines[i], rgba(255, 230, 230, (Uint8)a), TEXT_LEFT);
}

void director_see(void) {
	seen.px = bn6_player_x();
	seen.py = bn6_player_y();
	seen.on_map = on_map();
	seen.battle = emu_read8(BN6_GAMESTATE) == BN6_SUB_BATTLE;
	seen.timer = emu_read32(BN6_BATTLE_TIMER);
	seen.custom = seen.battle && emu_read8(BN6_CUSTOM_WINDOW);
	see_tent();
	see_counts();
	see_locks();
	see_bug_note();
}

/* The panel BN6 lights under MegaMan's feet, marked over his sprite: a
 * panel of the field (40 by 24, its first row 72 down the picture) in the
 * game's warning yellow, steady. */
void director_draw_tent(void) {
	if (!seen.tent_x || seen.custom) return;
	int x = P.core_x + (seen.tent_x - 1) * 40, y = P.core_y + 72 + (seen.tent_y - 1) * 24;
	SDL_Color c = rgba(255, 232, 0, 255);
	fill_rect(x + 2, y + 2, 36, 20, rgba(255, 232, 0, 96));
	fill_rect(x, y, 40, 2, c);
	fill_rect(x, y + 22, 40, 2, c);
	fill_rect(x, y + 2, 2, 20, c);
	fill_rect(x + 38, y + 2, 2, 20, c);
}

/* The duel's clock (docs/RIVAL.md): while its battle runs, the time so far
 * against ProtoMan's, as BN6's results screen counts them, in the picture's
 * top right, under the Custom gauge; on the second rung whether MegaMan has
 * been hit. Hidden while the clock holds (BATTLE START!, the Custom screen,
 * the pause): a playtester raced a time he could not see. */
void director_draw_duel(void) {
	if (!D.active || !D.duel || layer_objs_duel_rung == 2 || seen.on_map || !seen.battle) return;
	static uint32_t last;
	static int still;
	uint32_t t = seen.timer;
	still = t == last ? still + 1 : 0;
	last = t;
	if (still > 2 || !t) return;
	int his = layer_objs_duel_frames;
	char mine[16], theirs[16];
	snprintf(mine, sizeof mine, "%u:%02u.%02u", t / 3600, t / 60 % 60, t % 60 * 100 / 60);
	snprintf(theirs, sizeof theirs, "%d:%02d.%02d", his / 3600, his / 60 % 60, his % 60 * 100 / 60);
	char vs[32];
	snprintf(vs, sizeof vs, "ProtoMan %s", theirs);
	int lines = layer_objs_duel_rung == 1 ? 3 : 2, x = P.core_x + 236, y = P.core_y + 19, w = text_width(vs) + 6;
	fill_rect(x - w + 2, y - 2, w, lines * 10 + 3, rgba(0, 16, 40, 170));
	text_drawf(x, y, (int)t < his ? WHITE : rgba(255, 120, 120, 255), TEXT_RIGHT, "%s", mine);
	text_draw(x, y + 10, vs, rgba(170, 200, 255, 255), TEXT_RIGHT);
	if (lines == 3) text_draw(x, y + 20, D.duel_hit ? "Hit!" : "No hits", D.duel_hit ? rgba(255, 120, 120, 255) : rgba(140, 255, 170, 255), TEXT_RIGHT);
}
