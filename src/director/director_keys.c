/* The pad on its way to the game (director_keys): A turned to what it
 * means and walked up to it, the guards that hold an A off as a screen
 * changes, the Custom screen's kept L and R, SELECT's map, R in the town, L's
 * briefing; and MegaMan freed from inside an NPC. */
#include "director_keys.h"

#include <math.h>

#include "autopilot.h"
#include "bn6_fields.h"
#include "boss.h"
#include "briefing_words.h"
#include "cinema.h"
#include "director.h"
#include "director_map.h"
#include "director_see.h"
#include "director_state.h"
#include "director_way.h"
#include "lan_house.h"
#include "lesson_words.h"
#include "talk.h"
#include "town.h"

#define CARD_SKIP    30   /* frames a card shows before A ends it, where MegaMan is held for it */
#define PORT_STEP    16   /* world units from a jack-in cell that R steps onto it */
#define WALK_UP      45   /* frames the walk up to a navi out of reach may take */

/* A on the map: MegaMan turns to face what he means to talk to, so the
 * game's own check finds it (walking into a navi slides him round it, and a
 * tap of the pad can leave him facing past it): the navi or Mystery Data
 * clearly before him if there is one, out to two and a half panels, else
 * the nearest within 52 units (the probes' reach) on the side he faces,
 * else the nearest behind (a vendor a step behind turned him round from
 * Mystery Data ahead). One out of reach he
 * walks up to (a step or two short of a navi, the press did nothing). */
static void probe_vectors(int vx[8], int vy[8]) {
	for (int k = 0; k < 8; ++k) {
		vx[k] = (int32_t)emu_read32(BN6_TALK_PROBES + (uint32_t)k * BN6_TALK_PROBE_SIZE) >> 16;
		vy[k] = (int32_t)emu_read32(BN6_TALK_PROBES + (uint32_t)k * BN6_TALK_PROBE_SIZE + BN6_TALK_PROBE_Y) >> 16;
	}
}

/* Where the game measures NPC object o from for talking and collision:
 * its place and its centre's shift (bn6f OverworldNPCObject +0x11-0x13: a
 * navi behind a counter is spoken to across it; a floor sprite stands
 * further back than it shows). */
static void npc_centre(uint32_t o, int *x, int *y) {
	*x = (int16_t)emu_read16(o + BN6_NPC_X16) + (int8_t)emu_read8(o + BN6_NPC_CENTER_X);
	*y = (int16_t)emu_read16(o + BN6_NPC_Y16) + (int8_t)emu_read8(o + BN6_NPC_CENTER_Y);
}

/* The NPC slot A means, or -1. */
static int talk_target(void) {
	int px = bn6_player_x(), py = bn6_player_y();
	int face = emu_read8(BN6_PLAYER_FACING) & 7, vx[8], vy[8];
	probe_vectors(vx, vy);
	double fl = sqrt((double)vx[face] * vx[face] + vy[face] * vy[face]);
	if (fl < 1) return -1;
	int front = -1, ahead = -1, near = -1, fd = 1 << 30, ad = 52 * 52 + 1, nd = 52 * 52 + 1;
	double fc = 0;
	for (int i = 0; i < 16; ++i) {
		uint32_t o = BN6_NPC_OBJECTS + (uint32_t)i * BN6_NPC_SIZE;   /* the game's NPC objects (director_describe) */
		if (!(emu_read8(o) & 1) || !emu_read8(o + BN6_NPC_RADIUS)) continue;
		int cx, cy;
		npc_centre(o, &cx, &cy);
		int dx = cx - px, dy = cy - py, d = dx * dx + dy * dy;
		/* before him: within 30 degrees of his facing, the one most straight
		 * ahead (the nearest in that cone took a bystander a little off his
		 * line over the Mystery Data he faced); one he faces out of reach is
		 * the one he means: no turn to another beside him */
		double c = d > 0 ? (vx[face] * dx + vy[face] * dy) / (fl * sqrt((double)d)) : 1;
		if (d <= 80 * 80 && d > 0 && c >= 0.866 && (c > fc + 0.02 || (c > fc - 0.02 && d < fd))) { fd = d; fc = c; front = i; }
		if (d <= 52 * 52 && c >= 0.2 && d < ad) { ad = d; ahead = i; }
		if (d <= 52 * 52 && d < nd) { nd = d; near = i; }
	}
	/* (but one he touches beats one before him out of reach) */
	if (front >= 0 && !(near >= 0 && nd <= 24 * 24 && fd > 52 * 52)) return front;
	return ahead >= 0 ? ahead : near;
}

/* Where NPC slot i stands from MegaMan, the facing whose probe points at
 * it best, and whether that probe reaches it. */
static bool talk_reach(int i, int *face) {
	int px = bn6_player_x(), py = bn6_player_y(), vx[8], vy[8];
	uint32_t o = BN6_NPC_OBJECTS + (uint32_t)i * BN6_NPC_SIZE;
	int cx, cy;
	npc_centre(o, &cx, &cy);
	int tx = cx - px, ty = cy - py;
	probe_vectors(vx, vy);
	int k = emu_read8(BN6_PLAYER_FACING) & 7;
	double top = -2;
	for (int f = 0; f < 8; ++f) {
		double l = sqrt((double)vx[f] * vx[f] + vy[f] * vy[f]) * sqrt((double)tx * tx + ty * ty);
		double c = l > 0 ? (vx[f] * tx + vy[f] * ty) / l : -2;
		if (c > top) { top = c; k = f; }
	}
	*face = k;
	/* (the probe's circle and the NPC's meet, with a little to spare) */
	int r = emu_read8(BN6_TALK_PROBES + (uint32_t)k * BN6_TALK_PROBE_SIZE + BN6_TALK_PROBE_RADIUS) + emu_read8(o + BN6_NPC_RADIUS) - 3;
	int ex = tx - vx[k], ey = ty - vy[k];
	return ex * ex + ey * ey <= r * r;
}

static void talk_turn(int k) {
	emu_write8(BN6_PLAYER_FACING, (uint8_t)k);
	emu_write8(BN6_PLAYER_ANIM, (uint8_t)k);
}

#define PAD_KEYS (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)
/* the pad for each facing, in the probes' order */
static const uint32_t face_pad[8] = {
	KEY_UP, KEY_UP | KEY_RIGHT, KEY_RIGHT, KEY_DOWN | KEY_RIGHT, KEY_DOWN, KEY_DOWN | KEY_LEFT, KEY_LEFT, KEY_UP | KEY_LEFT,
};

/* Two navis side by side: the game's own check takes the first whose ring
 * its probe touches (every A went to the Recovery Mr. Prog beside the Net
 * Dealer Kai faced), so while the press goes through, the others near have
 * no ring. */
static struct { int t; uint8_t r[16]; } excl;

static void talk_only(int i) {
	int px = bn6_player_x(), py = bn6_player_y();
	for (int j = 0; j < 16; ++j) {
		uint32_t o = BN6_NPC_OBJECTS + (uint32_t)j * BN6_NPC_SIZE;
		uint8_t r = emu_read8(o + BN6_NPC_RADIUS);
		if (j == i || !(emu_read8(o) & 1) || !r || excl.r[j]) continue;
		int cx, cy;
		npc_centre(o, &cx, &cy);
		int dx = cx - px, dy = cy - py;
		if (dx * dx + dy * dy > 96 * 96) continue;
		excl.r[j] = r;
		emu_write8(o + BN6_NPC_RADIUS, 0);
	}
	excl.t = 8;   /* (the game takes a press two frames on) */
}

static void talk_only_update(void) {
	if (!excl.t || --excl.t) return;
	for (int j = 0; j < 16; ++j) {
		if (excl.r[j]) emu_write8(BN6_NPC_OBJECTS + (uint32_t)j * BN6_NPC_SIZE + BN6_NPC_RADIUS, excl.r[j]);
		excl.r[j] = 0;
	}
}

/* The walk up: towards the NPC until its probe reaches, then A for him.
 * The pad or B takes over; a wall ends it with the A all the same. */
static uint32_t talk_walk(uint32_t keys) {
	static int last_x, last_y, still;
	int px = bn6_player_x(), py = bn6_player_y();
	uint32_t o = BN6_NPC_OBJECTS + (uint32_t)D.walk_to * BN6_NPC_SIZE;
	if ((keys & (PAD_KEYS | KEY_B)) || !(emu_read8(o) & 1) || !emu_read8(o + BN6_NPC_RADIUS) || emu_read8(BN6_CHATBOX) || talk_busy()) {
		D.walk_t = 0;
		return keys;
	}
	if (D.walk_t == WALK_UP) still = 0;
	else if (px == last_x && py == last_y) ++still;
	else still = 0;
	last_x = px; last_y = py;
	int k;
	bool reach = talk_reach(D.walk_to, &k);
	if (reach || still >= 6 || --D.walk_t <= 0) {
		D.walk_t = 0;
		talk_turn(k);
		talk_only(D.walk_to);
		return (keys & ~PAD_KEYS) | KEY_A;
	}
	return (keys & ~(PAD_KEYS | KEY_A)) | face_pad[k];
}

/* The guards below hold off an A for a fifth of a second as a screen
 * changes under it: a press that soon cannot be an answer to what just
 * appeared (a reaction takes about that long), so it was pressed at what
 * went before. One pressed after a look always passes (they had held A
 * off for half a second and more, which read as input lag). */
#define A_GUARD 12

static uint32_t shop_guard(uint32_t keys) {
	static int chat_recent, guard, last_mode = -1;
	int mode = main_mode();
	chat_recent = emu_read8(BN6_CHATBOX) ? 30 : chat_recent > 0 ? chat_recent - 1 : 0;
	if (last_mode == BN6_MODE_GAME && mode != BN6_MODE_GAME && mode != BN6_MODE_GAME_OVER && chat_recent) guard = A_GUARD;
	last_mode = mode;
	if (guard > 0) { --guard; keys &= ~KEY_A; }
	return keys;
}

/* A choice, BN6's or ours: a playtester's A's, pressed through a Net
 * Dealer's words, landed twice on the shop's "Are you sure? > Yes" (BN6's
 * default) and bought what he had not chosen; an A in the question's first
 * fifth of a second, pressed at what went before, is let go. B still
 * answers No. Every question in a shop starts as BN6's does, on Yes (the
 * owner's call: a visit's first had started on No and the rest on Yes, and
 * two playtesters' LEFT, A, learned on the first, answered No to the next,
 * session 64). */
static uint32_t choice_guard(uint32_t keys) {
	static int age;
	bool choice = emu_read8(BN6_CHATBOX) && emu_read8(BN6_CHATBOX_OPTIONS) >= 2;
	age = choice ? age + 1 : 0;
	if (choice && age <= A_GUARD) keys &= ~KEY_A;
	return keys;
}

/* In battle an L or R pressed as the Custom gauge was all but full was
 * lost (the game takes them at a full gauge only; Kai re-pressed in every
 * fight, and at 50 frames still: the gauge's last tenth takes about a
 * second and looks full), and so was one the game let pass while MegaMan
 * fired or flinched: a press is kept two and a half seconds (at a second
 * and a half, one Kai pressed 126 frames before the gauge filled was
 * dropped: the last of it looks full) and given as
 * the gauge fills, one frame let go first so the game sees a press, then
 * again every CUSTOM_RETRY frames until the Custom screen opens (the
 * gauge empties as it does) or CUSTOM_TRIES frames have passed. While
 * the d-pad is held, and CUSTOM_CALM frames after, it waits, a second at
 * most: the step comes first (a kept R opened the Custom screen over the
 * UP that was stepping MegaMan off a lit panel, and the bomb burst as the
 * battle went on; dropping the press at a d-pad press instead lost a
 * playtester's R pressed before a dodge in a guardian's fight, where both
 * are needed). Once a kept press has opened the Custom screen, L and R are
 * held off CUSTOM_HUSH frames: a second R, pressed as the screen was slow
 * to slide in, opened a chip's description on it. */
#define CUSTOM_EARLY 150
#define CUSTOM_TRIES 45
#define CUSTOM_RETRY 20
#define CUSTOM_CALM 12
#define CUSTOM_WAIT 60
#define CUSTOM_HUSH 30
static uint32_t custom_buffer(uint32_t keys, bool l_pressed, bool r_pressed) {
	static int kept, step, calm, wait, hush;
	static uint32_t which;
	if (main_mode() != BN6_MODE_GAME || emu_read8(BN6_GAMESTATE) != BN6_SUB_BATTLE) { kept = step = calm = hush = 0; return keys; }
	if (hush > 0) { --hush; return keys & ~(KEY_L | KEY_R); }
	if (keys & PAD_KEYS) calm = CUSTOM_CALM;
	else if (calm > 0) --calm;
	bool full = emu_read16(BN6_CUSTOM_GAUGE) >= 0x4000;
	if (l_pressed || r_pressed) {
		/* (pressed at a full gauge: the game has this press; again later
		 * only if it let it pass) */
		kept = full ? CUSTOM_TRIES : CUSTOM_EARLY;
		step = full ? 2 : 0;
		wait = CUSTOM_WAIT;
		which = l_pressed ? KEY_L : KEY_R;
		return keys;
	}
	if (kept <= 0) return keys;
	if (!full) {
		/* still filling, or emptied by the Custom screen taking a press */
		if (step) hush = CUSTOM_HUSH;
		kept = step ? 0 : kept - 1;
		return keys;
	}
	if (calm > 0) {
		if (--wait <= 0) kept = step = 0;
		return keys;
	}
	if (step == 0) kept = CUSTOM_TRIES;
	--kept;
	int s = step++ % CUSTOM_RETRY;
	if (s == 0) return keys & ~(KEY_L | KEY_R);
	if (s == 1) return keys | which;
	return keys;
}

/* R in Lan's house, or in his room off the PC (on it, BN6's own jack-in):
 * MegaMan says where the PC is */
static uint32_t house_r(uint32_t keys, int number) {
	int px = bn6_player_x(), py = bn6_player_y(), gx, gy, far;
	if ((number == LAN_ROOM && lan_room_on_pc(px, py)) || !lan_house_goal(number, &gx, &gy)) return keys;
	talk_start(port_words(number == LAN_ROOM ? PORT_ROOM : PORT_HOUSE, way_to(gx, gy, &far)), FACE_MEGAMAN);
	D.arrow_pending = true;
	cinema_arrow(way_last(), 600);
	return keys & ~KEY_R;
}

/* R in the town away from the port (home's: Lan's front door): a step
 * short of a port cell, the step taken; else MegaMan's word on where the
 * port is, and the arrow its way; in Lan's house and room, house_r. The
 * keys as they go on to the game. */
static uint32_t town_r(uint32_t keys) {
	int px = bn6_player_x(), py = bn6_player_y();
	int number = emu_read8(BN6_MAP_NUMBER);
	if (lan_house_map(emu_read8(BN6_MAP_GROUP), number)) return house_r(keys, number);
	int dx = town_info()->port_x - px, dy = town_info()->port_y - py, far, nx = 0, ny = 0;
	int near = town_port_near(px, py, &nx, &ny);
	/* (a step short of a jack-in cell: he takes it, and R jacks in; a
	 * playtester stood at the mermaid fountain's rim a step off its
	 * ring and pressed R five times) */
	if (near >= 0 && near <= PORT_STEP * PORT_STEP) {
		emu_write32(BN6_PLAYER_X, (uint32_t)nx << 16);
		emu_write32(BN6_PLAYER_Y, (uint32_t)ny << 16);
		cinema_arrow(0, 0);
		D.arrow_pending = false;
		return keys;
	}
	const char *way = town_way(&far);
	/* (close by: which way to its nearest cell, as the crow flies, where
	 * the walk to the front's middle wound round the basin and turned
	 * from "up and to the left" to "straight down" a step apart) */
	int how = near >= 0 && near < 128 * 128 ? PORT_ALMOST_CELL : dx * dx + dy * dy < 128 * 128 ? PORT_ALMOST : PORT_AWAY;
	talk_start(port_words(how, how == PORT_ALMOST_CELL ? way_to(nx, ny, &far) : way), FACE_MEGAMAN);
	D.arrow_pending = true;
	cinema_arrow(way_last(), 600);
	return keys & ~KEY_R;
}

/* A on the map: let go a fifth of a second after a chat closes, else
 * turned to what it would talk to, or walked up to it; the keys as they go
 * on to the game */
static uint32_t a_keys(uint32_t keys, bool a_pressed) {
	/* (no A a fifth of a second after a chat closes: a playtester's A
	 * pressed through a chat's last box talked to the gift Prog beside him
	 * again, twice a session) */
	bool chat_open = emu_read8(BN6_CHATBOX) || talk_busy();
	if (D.chat_was_open && !chat_open) D.a_quiet = A_GUARD;
	D.chat_was_open = chat_open;
	if (D.a_quiet > 0) { --D.a_quiet; if (!chat_open) { keys &= ~KEY_A; a_pressed = false; } }
	/* (turned to what A would talk to, the pad left alone for that frame so
	 * the game does not turn him back; not in the town, where A also reads
	 * the doors and signs Lan faces) */
	if (D.walk_t > 0) keys = talk_walk(keys);
	else if (a_pressed && !D.town && !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && !D.warping) {
		int i = talk_target(), k;
		if (i >= 0 && talk_reach(i, &k)) {
			if (k != (emu_read8(BN6_PLAYER_FACING) & 7)) { talk_turn(k); keys &= ~PAD_KEYS; }
			talk_only(i);
		} else if (i >= 0 && !(keys & PAD_KEYS) && !autopilot_on()) {
			/* (not while he walks: the pad is his) */
			D.walk_to = i;
			D.walk_t = WALK_UP;
			keys = talk_walk(keys & ~KEY_A);
		}
	}
	return keys;
}

/* L on the map (`pressed` this frame): MegaMan's briefing, kept a while
 * where it cannot be said yet */
static void l_keys(bool pressed) {
	/* (not while a warp or the jack-in departs, nor through a guardian's
	 * staging or the battle it has armed; an L pressed as a chat closes is
	 * kept half a second, as the first press after one went unheard) */
	bool can_l = !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && !D.warping && emu_read8(BN6_WARP_PENDING) == 0 &&
		(D.town || (!boss_cinematic() && !boss_fighting()));
	if (pressed && !can_l) D.l_kept = 30;
	else if (!pressed && D.l_kept > 0) {
		/* (kept through the engine's own cards and words, and given after
		 * the arrival's, which wait for the card too: an L pressed as the
		 * act card faded went to the act's arrival words and was gone) */
		if (can_l && !D.beat[0]) { pressed = true; D.l_kept = 0; }
		else if (!talk_busy() && !cinema_busy() && !D.beat[0]) --D.l_kept;
	}
	if (pressed && can_l) {
		l_taken();
		/* (the arrow shows through the words and a few seconds after) */
		if (talk_start(status_words(), FACE_MEGAMAN) && (D.town || !D.objs.guardian.navi || !boss_beaten() || boss_done())) {
			D.arrow_pending = true;
			cinema_arrow(way_last(), 600);
		}
	}
}

/* R on the port: the jack-in, which the arrow does not follow into its
 * flash and tunnel (a playtester saw it drawn over them) */
static void port_r(void) {
	cinema_arrow(0, 0);
	D.arrow_pending = false;
}

/* R at home in the real world: on the town's port the jack-in, the arrow
 * off; away from it (and in Lan's house and room, off his PC) MegaMan says
 * where to jack in, the game itself doing nothing there: true then, `keys`
 * as they go on */
static bool home_r(uint32_t *keys) {
	bool house = lan_house_map(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER));
	bool on_port = !house && town_on_port(bn6_player_x(), bn6_player_y());
	if (on_port) port_r();
	if (on_port || talk_busy() || emu_read8(BN6_CHATBOX) || cinema_busy() || emu_read8(BN6_WARP_PENDING) != 0) return false;
	*keys = town_r(*keys);
	return true;
}

uint32_t director_keys(uint32_t keys) {
	talk_only_update();
	if (D.active) keys = choice_guard(shop_guard(keys));
	bool l = (keys & KEY_L) != 0, pressed = l && !D.l_held;
	bool r = (keys & KEY_R) != 0, r_pressed = r && !D.r_held;
	bool a = (keys & KEY_A) != 0, a_pressed = a && !D.a_held;
	D.l_held = l;
	D.r_held = r;
	D.a_held = a;
	D.dir_held = (keys & PAD_KEYS) != 0;
	D.map_shown = false;
	if (D.active && !autopilot_on()) keys = custom_buffer(keys, pressed, r_pressed);
	if (!D.active || !on_map()) return keys;   /* (in battle L opens the Custom screen) */
	/* (A ends the arrival's card early: its words wait on it) */
	if (D.arrival_hold && a_pressed && cinema_card_age() >= CARD_SKIP) cinema_card_yield();
	keys = a_keys(keys, a_pressed);
	/* SELECT on a layer: the map, while it is held */
	if (!D.town && (keys & KEY_SELECT)) { D.map_shown = !emu_read8(BN6_CHATBOX); map_note_held(D.map_shown); keys &= ~KEY_SELECT; }
	if (director_in_town() && r_pressed && home_r(&keys)) return keys;
	/* on the map L is MegaMan's word on where they are: the game's own
	 * has no lines for this story */
	keys &= ~KEY_L;
	l_keys(pressed);
	return keys;
}

/* MegaMan wedged inside an NPC (a walker came at him, or a wall's push-out
 * on a walkway moved him in): every one of his movement probes meets it, so
 * no direction moves him. After a second of pushing he is put back where
 * he last stood clear of every NPC (the game's NPC objects, bn6f
 * eOverworldNPCObjects: 16 of 0xD8 bytes). */
void unwedge(void) {
	int px = bn6_player_x(), py = bn6_player_y();
	bool inside = false;
	for (int i = 0; i < 16 && !inside; ++i) {
		uint32_t o = BN6_NPC_OBJECTS + (uint32_t)i * BN6_NPC_SIZE;
		int r = emu_read8(o + BN6_NPC_RADIUS);
		if (!(emu_read8(o) & 1) || !r) continue;
		int cx, cy;
		npc_centre(o, &cx, &cy);
		int dx = cx - px, dy = cy - py;
		inside = dx * dx + dy * dy < (r + 2) * (r + 2);
	}
	bool moved = px != D.last_x || py != D.last_y;
	D.last_x = px; D.last_y = py;
	if (!inside) { D.free_x = px; D.free_y = py; D.wedged = 0; return; }
	if (moved || !D.dir_held || emu_read8(BN6_CHATBOX) || talk_busy()) { D.wedged = 0; return; }
	if (++D.wedged < 60 || (D.free_x == px && D.free_y == py)) return;
	D.wedged = 0;
	emu_write32(BN6_PLAYER_X, (uint32_t)D.free_x << 16);
	emu_write32(BN6_PLAYER_Y, (uint32_t)D.free_y << 16);
	emu_write32(BN6_PLAYER_NEXT_X, (uint32_t)D.free_x << 16);
	emu_write32(BN6_PLAYER_NEXT_Y, (uint32_t)D.free_y << 16);
}
