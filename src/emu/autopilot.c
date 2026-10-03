/* The autopilot finds a path of floor panels from MegaMan's panel to the
 * exit (breadth first on the layer grid), by the layer's heal when he is
 * hurt, and holds the pad toward the next panel's centre. In battles it
 * reads the fight: the chips that go together on the Custom screen, each
 * used from a panel where it reaches an enemy, attacks stepped away from,
 * the buster charged in between. In text it taps A. Testing only. */
#include "autopilot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "data.h"
#include "director.h"
#include "emu.h"
#include "net.h"
#include "netmap.h"
#include "run.h"
#include "town.h"

bool autopilot_on(void) { return getenv("CYBERWORLD_AUTOPILOT") != NULL; }

/* a solid object (a talker) stands on the panel */
static bool blocked(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].solid && (int)layer.obj[i].x == x && (int)layer.obj[i].y == y) return true;
	return false;
}

static int search(int sx, int sy, int tx, int ty, bool avoid, int *nx, int *ny) {
	static int16_t prev[MAP_H][MAP_W];
	static int16_t q[MAP_W * MAP_H];
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) prev[y][x] = -1;
	int h = 0, t = 0;
	q[t++] = (int16_t)(sy * MAP_W + sx);
	prev[sy][sx] = (int16_t)(sy * MAP_W + sx);
	while (h < t) {
		int c = q[h++], x = c % MAP_W, y = c / MAP_W;
		if (x == tx && y == ty) break;
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int ax = x + d[k][0], ay = y + d[k][1];
			if (ax < 0 || ay < 0 || ax >= MAP_W || ay >= MAP_H || prev[ay][ax] >= 0 || layer.cell[ay][ax] != C_PATH || (avoid && blocked(ax, ay) && (ax != tx || ay != ty)) ||
				!layer_step_ok(x, y, ax, ay))
				continue;
			prev[ay][ax] = (int16_t)c;
			q[t++] = (int16_t)(ay * MAP_W + ax);
		}
	}
	if (prev[ty][tx] < 0) return 0;
	int c = ty * MAP_W + tx;
	while (prev[c / MAP_W][c % MAP_W] != sy * MAP_W + sx && c != sy * MAP_W + sx) c = prev[c / MAP_W][c % MAP_W];
	*nx = c % MAP_W;
	*ny = c / MAP_W;
	return 1;
}

/* The next panel towards (tx, ty): around talkers when possible, else past
 * them (MegaMan fits beside one on a panel). */
static int next_panel(int sx, int sy, int tx, int ty, int *nx, int *ny) {
	return search(sx, sy, tx, ty, true, nx, ny) || search(sx, sy, tx, ty, false, nx, ny);
}

/* CYBERWORLD_AUTOPILOT=weak: enemies keep 1 HP, so every battle is won and
 * what follows a win (a guardian's reward, the exit opening) can be tested. */
#define T1_OBJECTS BN6_T1_OBJECTS
#define T1_SIZE    BN6_T1_SIZE
#define T1_COUNT   BN6_T1_COUNT

static bool weak(void) {
	const char *how = getenv("CYBERWORLD_AUTOPILOT");
	return how && !strcmp(how, "weak");
}

/* A battle object that is in play: flag bit 0 of its header, HP left. */
static bool alive(uint32_t o) { return (emu_read8(o) & 1) && emu_read16(o + BN6_T1_HP) > 0; }

/* Up or down to MegaMan's row towards the nearest enemy's, 0 when aligned
 * (PanelY at +0x13, Alliance at +0x16). */
static uint32_t toward_enemy_row(void) {
	int mine = -1, theirs = -1;
	for (uint32_t i = 0; i < T1_COUNT; ++i) {
		uint32_t o = T1_OBJECTS + i * T1_SIZE;
		if (!alive(o)) continue;
		int row = emu_read8(o + BN6_T1_PANEL_Y);
		if (emu_read8(o + BN6_T1_ALLIANCE) == 0) { if (mine < 0) mine = row; }
		else if (theirs < 0 || abs(row - mine) < abs(theirs - mine)) theirs = row;
	}
	if (mine < 0 || theirs < 0 || mine == theirs) return 0;
	return theirs < mine ? KEY_UP : KEY_DOWN;
}

/* Enemies at 1 HP, MegaMan topped up when low (HP +0x24, MaxHP +0x26, Alliance
 * +0x16), and enemies it has not hit after WEAK_FRAMES at none: what
 * follows a battle gets tested even where the autopilot fights badly
 * (planes out of its row, StarFish under water, Nightmares). */
#define WEAK_FRAMES 1200

static uint32_t battle_frames;   /* off the map in a row */

static void weaken_enemies(bool finish) {
	for (uint32_t i = 0; i < T1_COUNT; ++i) {
		uint32_t o = T1_OBJECTS + i * T1_SIZE;
		if (!(emu_read8(o) & 1)) continue;
		int alliance = emu_read8(o + BN6_T1_ALLIANCE), hp = emu_read16(o + BN6_T1_HP), max = emu_read16(o + BN6_T1_MAX_HP);
		uint8_t v[2] = { 1, 0 };
		if (alliance == 0 && hp > 0 && hp < 60) {
			int full = max > hp ? max : 100;
			v[0] = (uint8_t)full; v[1] = (uint8_t)(full >> 8);
			emu_write(o + BN6_T1_HP, v, 2);
		}
		if (alliance == 1 && finish) { v[0] = 0; emu_write(o + BN6_T1_HP, v, 2); }
		else if (alliance == 1 && hp > 1) emu_write(o + BN6_T1_HP, v, 2);
	}
}

/* weak's battles and screens: a 480-frame rhythm of picking chips, OK and
 * the buster, blind to the screen (the docs' pictures and clips are timed
 * by it) */
static uint32_t rhythm_keys(uint32_t frame) {
	uint32_t t = frame % 480;
	if (t < 48) return (t % 12) < 4 ? KEY_A : 0;          /* pick chips */
	if (t < 60) return t < 52 ? KEY_START : 0;            /* to OK */
	if (t < 72) return t < 64 ? KEY_A : 0;                /* send */
	if (t % 40 < 6) return KEY_A;                          /* use a chip / advance text */
	uint32_t row = toward_enemy_row();                     /* line up, then the buster */
	if (row) return (t % 8) < 2 ? row : 0;
	return (t % 12) < 4 ? KEY_B : 0;
}

/* The fight otherwise, read from the battle's state (docs/ROM_DATA.md, the
 * autopilot's fight). The rhythm lost the first battles of every run: its
 * A took the hand's first chip alone (an Atk+10, say) and its B on the
 * Custom screen took that back, its START paused the fight 480 frames at a
 * time, and with no R the Custom screen never opened again, so MegaMan
 * fought on with the buster's 1 while the viruses hit him. */
#define PRESS         4     /* frames a button is held in the fight */
#define CUSTOM_PRESS  3     /* frames a press on the Custom screen is held, */
#define CUSTOM_GAP    6     /* ... and from one to the next */
#define CUSTOM_SLIDE  16    /* frames the Custom screen takes to slide in */
#define CHIP_PATIENCE 90    /* frames a chip waits for an enemy in reach before it braves attacks, and twice that before it is used where he stands */
#define CHARGE        120   /* frames B is held for a charged shot (10 times the buster's 1; it charges in 101-110 at Charge 1) */
#define SETTLED       20    /* frames an enemy stands on its panel before MegaMan goes to where he hits it */
#define CYCLE_WORTH   10    /* a chip picked: the one drawn in its place next time */
#define NAVI_HP       300   /* an enemy this strong is a navi (act 1's guardians have 400 and more, its viruses 200 at most) */

/* MegaMan's battle object, 0 if none is in play */
static uint32_t megaman(void) {
	for (uint32_t i = 0; i < T1_COUNT; ++i) {
		uint32_t o = T1_OBJECTS + i * T1_SIZE;
		if (alive(o) && emu_read8(o + BN6_T1_ALLIANCE) == 0) return o;
	}
	return 0;
}

static uint32_t panel(int x, int y) { return BN6_FIELD_PANELS + (uint32_t)(y * 8 + x) * BN6_PANEL_SIZE; }

/* Panel (x, y) for MegaMan: 0 not his side's or a hole, 1 his, 2 his and
 * poison, which drains him while he stands on it */
static int panel_of(int x, int y) {
	if (x < 1 || x > 6 || y < 1 || y > 3 || emu_read8(panel(x, y) + BN6_PANEL_ALLIANCE) != 0) return 0;
	int type = emu_read8(panel(x, y) + BN6_PANEL_TYPE);
	return type <= BN6_PANEL_HOLE ? 0 : type == BN6_PANEL_POISON ? 2 : 1;
}

/* A chip's reach by its kind (src/core/data.c): an enemy lo to hi columns
 * ahead, `rows` rows either side; false for one that hits no one panel (a
 * recovery, a grab, a Navi), used where MegaMan stands. The buster is a
 * shot. */
static bool chip_reach(int kind, int *lo, int *hi, int *rows) {
	*lo = 1; *hi = 5; *rows = 0;
	switch (kind) {
	case CK_SWORD: *hi = 1; return true;
	case CK_WIDESWORD: *hi = 1; *rows = 1; return true;
	case CK_LONGSWORD: *hi = 2; return true;
	case CK_TORNADO: *lo = *hi = 2; return true;
	case CK_BOMB: *lo = *hi = 3; return true;
	case CK_FLAME: *hi = 3; return true;
	case CK_CANNON: case CK_AIRSHOT: case CK_VULCAN: case CK_SPREADER: case CK_WAVE: case CK_CROSSGUN: return true;
	default: return false;
	}
}

/* The frames each battle object has stood on its panel */
static uint16_t foe_still[T1_COUNT];
static uint8_t foe_at[T1_COUNT];

static void watch_still(void) {
	for (uint32_t i = 0; i < T1_COUNT; ++i) {
		uint32_t o = T1_OBJECTS + i * T1_SIZE;
		uint8_t at = (uint8_t)(emu_read8(o + BN6_T1_PANEL_Y) << 4 | emu_read8(o + BN6_T1_PANEL_X));
		foe_still[i] = at == foe_at[i] && alive(o) ? (uint16_t)(foe_still[i] < 0xFFFF ? foe_still[i] + 1 : foe_still[i]) : 0;
		foe_at[i] = at;
	}
}

/* Whether a chip with that reach hits an enemy from (x, y), counting only
 * those settled on their panel where `settled` (DiveMan, changing rows,
 * was gone before MegaMan got there); *in_row, whether one stands in row y */
static bool reaches(int x, int y, int lo, int hi, int rows, bool settled, bool *in_row) {
	bool hit = false;
	*in_row = false;
	for (uint32_t i = 0; i < T1_COUNT; ++i) {
		uint32_t o = T1_OBJECTS + i * T1_SIZE;
		if (!alive(o) || emu_read8(o + BN6_T1_ALLIANCE) == 0) continue;
		int dx = emu_read8(o + BN6_T1_PANEL_X) - x, dy = emu_read8(o + BN6_T1_PANEL_Y) - y;
		if (dy == 0) *in_row = true;
		if (dx >= lo && dx <= hi && abs(dy) <= rows && (!settled || foe_still[i] >= SETTLED)) hit = true;
	}
	return hit;
}

/* What standing on panel (x, y) risks: an enemy's attack on it (the panel
 * BN6 lights), or a thrown one coming down on it (a mine), DANGER_ON; one
 * coming at it down the row (a wave, a torpedo) two panels off or nearer,
 * or beside it and not past it (a bubble turns onto him), DANGER_NEAR; one
 * further off, DANGER_FAR. Attacks past the panel, moving off, do not
 * count. */
#define DANGER_ON   600
#define DANGER_NEAR 150
#define DANGER_FAR  20

static int danger(int x, int y) {
	if (emu_read32(panel(x, y) + BN6_PANEL_FLAGS) & BN6_PANEL_STRUCK) return DANGER_ON;
	int d = 0;
	for (int ax = x + 1; ax <= 6; ++ax)
		if (emu_read32(panel(ax, y) + BN6_PANEL_FLAGS) & BN6_PANEL_STRUCK) { d = ax - x <= 2 ? DANGER_NEAR : DANGER_FAR; break; }
	for (uint32_t i = 0; i < BN6_T3_COUNT; ++i) {
		uint32_t o = BN6_T3_OBJECTS + i * T1_SIZE;
		int ox = emu_read8(o + BN6_T1_PANEL_X), oy = emu_read8(o + BN6_T1_PANEL_Y);
		if (!(emu_read8(o) & 1) || emu_read8(o + BN6_T1_ALLIANCE) == 0 || ox < 1 || ox > 6) continue;
		if ((ox == x && oy == y) || (emu_read8(o + BN6_T1_FUTURE_X) == x && emu_read8(o + BN6_T1_FUTURE_Y) == y)) return DANGER_ON;
		int near = ox < x ? 0 : oy == y ? (ox - x <= 2 ? DANGER_NEAR : DANGER_FAR) : abs(oy - y) == 1 && ox - x <= 1 ? DANGER_NEAR : 0;
		if (near > d) d = near;
	}
	return d;
}

/* A step's risk, never taken from a panel that has less: poison (1), an
 * attack on the panel (2) */
static int risk(int x, int y) { return (panel_of(x, y) == 2 ? 1 : 0) + (danger(x, y) >= DANGER_ON ? 2 : 0); }

/* Where MegaMan goes with the chip of `kind` he holds (-1 none): his own
 * panel where it hits from there, else the panel of his side nearest him
 * from which it hits an enemy settled on its panel; where none does (a
 * sword's enemy in the back row) or he holds none, the nearest panel out
 * of the enemies' rows, the further back the better. Never poison where he
 * has a choice, a panel that is not plain (a conveyor carries him off it)
 * only where it pays, and away from attacks unless `bold` (a chip that has
 * waited long enough). True when he can strike from (*tx, *ty). */
static bool spot(int kind, int col, int row, bool bold, int *tx, int *ty) {
	int lo = 0, hi = 0, rows = 0, best = -1;
	bool aims = kind >= 0 && chip_reach(kind, &lo, &hi, &rows), strike = false;
	*tx = col; *ty = row;
	if (kind >= 0 && !aims) return true;
	for (int y = 1; y <= 3; ++y)
		for (int x = 1; x <= 6; ++x) {
			int p = panel_of(x, y);
			if (!p) continue;
			bool in_row, hit = aims && reaches(x, y, lo, hi, rows, x != col || y != row, &in_row);
			if (!aims) reaches(x, y, 1, 5, 0, false, &in_row);
			int d = danger(x, y), cost = abs(x - col) + abs(y - row) + (bold && hit && d < DANGER_ON ? 0 : d);
			cost += p == 2 ? 1000 : emu_read8(panel(x, y) + BN6_PANEL_TYPE) != BN6_PANEL_PLAIN ? 200 : 0;
			if (!hit) cost += 100 + (in_row ? 40 : 0) + x * 2;
			if (best < 0 || cost < best) { best = cost; *tx = x; *ty = y; strike = hit; }
		}
	return strike;
}

/* A chip's kind (src/core/data.c's table); one it lacks is a shot where
 * it does damage, else used where MegaMan stands (BusterUp, a capsule) */
static int chip_kind(int id) {
	const ChipDef *d = chip_def(id);
	ChipInfo ci;
	if (d->rom_id == id) return d->kind;
	chip_info(id, &ci);
	return ci.power > 0 ? CK_CANNON : CK_COUNT;
}

/* What a chip is worth to the autopilot: its damage where it hits from any
 * column; half that where it needs a column of its own (a sword, a bomb)
 * and an enemy stands where it reaches from his side, a quarter while none
 * does; a recovery its HP when MegaMan is down that much; a grab a little,
 * anything else 1 */
static int chip_worth(int id, int hurt) {
	int kind = chip_kind(id), lo, hi, rows;
	ChipInfo ci;
	chip_info(id, &ci);
	if (kind == CK_RECOVER) return hurt >= chip_def(id)->param ? chip_def(id)->param : 1;
	if (kind == CK_AREAGRAB) return 10;
	if (kind == CK_ATKPLUS) return 0;
	if (ci.power <= 0) return 1;
	if (!chip_reach(kind, &lo, &hi, &rows) || hi >= 5) return ci.power;
	for (int y = 1; y <= 3; ++y)
		for (int x = 1; x <= 6; ++x) {
			bool in_row;
			if (panel_of(x, y) == 1 && reaches(x, y, lo, hi, rows, false, &in_row)) return ci.power / 2;
		}
	return ci.power / 4;
}

/* The hand's slots to pick, in order: of the groups the game takes
 * together (chips of one code, * going with any, or copies of one chip),
 * the one worth the most, each chip picked worth CYCLE_WORTH more (one
 * left on the Custom screen stays in the hand: a hand of swords the
 * enemies never came near for stood a whole battle); a grab first, then
 * the attacks (those that hit from anywhere before those that need a
 * column), the Atk+ after them, a recovery last */
static int plan_picks(int *slots) {
	int n = emu_read8(BN6_CUSTOM_HAND), hurt = 0, best = 0, nbest = 0;
	uint32_t me = megaman();
	if (n > 5) n = 5;   /* (the top row) */
	if (me) hurt = emu_read16(me + BN6_T1_MAX_HP) - emu_read16(me + BN6_T1_HP);
	for (int g = 0; g < 26 + n; ++g) {
		int bucket[5][5], k[5] = { 0 }, value = 0, plus = 0, lo, hi, rows;
		int name = g < 26 ? -1 : emu_read16(BN6_BATTLE_DECK + 2 * (uint32_t)(g - 26)) & 0x1FF;
		for (int i = 0; i < n; ++i) {
			int c = emu_read16(BN6_BATTLE_DECK + 2 * (uint32_t)i), id = c & 0x1FF, code = c >> 9, kind = chip_kind(id), at;
			if (c == 0xFFFF || (g < 26 ? code != g && code != 26 : id != name)) continue;
			if (kind == CK_ATKPLUS) { plus += chip_def(id)->param; at = 3; }
			else if (kind == CK_AREAGRAB) at = 0;
			else if (kind == CK_RECOVER) at = 4;
			else at = chip_reach(kind, &lo, &hi, &rows) && hi < 5 ? 2 : 1;
			value += chip_worth(id, hurt) + CYCLE_WORTH;
			bucket[at][k[at]++] = i;
		}
		if (k[1] + k[2]) value += plus;
		else k[3] = 0;
		if (value <= best) continue;
		best = value;
		nbest = 0;
		for (int a = 0; a < 5; ++a)
			for (int j = 0; j < k[a]; ++j) slots[nbest++] = bucket[a][j];
	}
	return nbest;
}

/* On the Custom screen: the planned chips, the cursor read to each (left
 * and right along the top row), then OK. A pick the game refuses twice is
 * left. */
static uint32_t custom_keys(uint32_t n) {
	static int plan[5], nplan, next, tries, picked;
	static uint32_t key;
	if (n < CUSTOM_SLIDE) return 0;
	if ((n - CUSTOM_SLIDE) % CUSTOM_GAP) return (n - CUSTOM_SLIDE) % CUSTOM_GAP < CUSTOM_PRESS ? key : 0;
	if (n == CUSTOM_SLIDE) {
		nplan = plan_picks(plan);
		next = tries = 0;
		picked = emu_read8(BN6_CUSTOM_PICKED);
	}
	int now = emu_read8(BN6_CUSTOM_PICKED);
	if (now > picked || tries >= 2) { ++next; tries = 0; }
	picked = now;
	int target = next < nplan ? plan[next] : BN6_CUSTOM_OK, cursor = emu_read8(BN6_CUSTOM_CURSOR);
	if (cursor == target) { key = KEY_A; ++tries; }
	else key = cursor > target ? KEY_LEFT : KEY_RIGHT;
	return key;
}

/* A navi in the fight: the charged shot's 10 is not worth a step into his
 * row, where his attacks go */
static bool navi_fight(void) {
	for (uint32_t i = 0; i < T1_COUNT; ++i) {
		uint32_t o = T1_OBJECTS + i * T1_SIZE;
		if (alive(o) && emu_read8(o + BN6_T1_ALLIANCE) != 0 && emu_read16(o + BN6_T1_MAX_HP) >= NAVI_HP) return true;
	}
	return false;
}

/* A step from (col, row) toward (tx, ty), the row first, onto a panel no
 * riskier than his own */
static uint32_t step_toward(int col, int row, int tx, int ty) {
	int dy = ty > row ? 1 : ty < row ? -1 : 0, dx = tx > col ? 1 : tx < col ? -1 : 0, here = risk(col, row);
	if (dy && panel_of(col, row + dy) && risk(col, row + dy) <= here) return dy > 0 ? KEY_DOWN : KEY_UP;
	if (dx && panel_of(col + dx, row) && risk(col + dx, row) <= here) return dx > 0 ? KEY_RIGHT : KEY_LEFT;
	return 0;
}

/* A frame of the fight, MegaMan's object `me`: B held without a chip, the
 * charged shot let go where a shot hits */
static uint32_t charge;   /* frames B has been held */

static uint32_t fight_frame(uint32_t me, uint32_t frame) {
	static uint32_t held_for;
	static int held = -1, hp;
	watch_still();
	int chip = emu_read16(me + BN6_T1_CHIP), col = emu_read8(me + BN6_T1_PANEL_X), row = emu_read8(me + BN6_T1_PANEL_Y), tx, ty;
	if (chip != held) { held = chip; held_for = 0; }
	if (emu_read16(me + BN6_T1_HP) < hp) charge = 0;   /* (a hit ends the charge) */
	hp = emu_read16(me + BN6_T1_HP);
	/* (A and R once he can act: an A pressed in a step was taken as the
	 * step ended, wherever that left him) */
	bool has = chip != 0xFFFF, ready = emu_read8(me + BN6_T1_ACTION) == BN6_MEGAMAN_READY, pulse = frame % (2 * PRESS) < PRESS;
	if (!has && emu_read16(BN6_CUSTOM_GAUGE) >= 0x4000) { charge = 0; return ready && pulse ? KEY_R : 0; }
	bool waited = has && ++held_for > CHIP_PATIENCE;
	int kind = has ? chip_kind(chip) : charge >= CHARGE && !navi_fight() ? CK_CANNON : -1;
	bool strike = spot(kind, col, row, waited, &tx, &ty);
	if (has && held_for > 2 * CHIP_PATIENCE && !strike) { strike = true; tx = col; ty = row; }
	uint32_t buster = has ? 0 : KEY_B;
	charge = buster ? charge + 1 : 0;
	if (tx != col || ty != row) return (pulse ? step_toward(col, row, tx, ty) : 0) | buster;
	if (has) return strike && ready && pulse ? KEY_A : 0;
	if (strike) { charge = 0; return 0; }
	return buster;
}

static uint32_t fight_keys(uint32_t frame) {
	static uint32_t custom, clock;
	if (emu_read8(BN6_GAMESTATE) != BN6_SUB_BATTLE) { custom = 0; return frame % 20 < PRESS ? KEY_A : 0; }
	if (emu_read8(BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM) { charge = 0; return custom_keys(custom++); }
	custom = 0;
	/* (the clock holds through BATTLE START!, the results and a chat: A) */
	uint32_t was = clock, me = megaman();
	clock = emu_read32(BN6_BATTLE_TIMER);
	if (clock == was || !me) { charge = 0; return frame % 20 < PRESS ? KEY_A : 0; }
	return fight_frame(me, frame);
}

/* The layer's heal (it restores every HP each time it is talked to) when
 * MegaMan is hurt; once he has been healed on a layer, only when he is
 * down to half his HP or the guardian is next: walked back to after every
 * scratch, a heal far from the exit met new battles both ways (seed 12
 * fought eight on its second layer) */
static bool heal_panel(bool guardian, int *x, int *y) {
	static int healed_on = -1, going = -1;
	int hp = emu_read16(BN6_NAVI_HP), max = emu_read16(BN6_NAVI_MAX_HP);
	if (hp >= max) {
		if (going == run.depth) healed_on = run.depth;
		going = -1;
		return false;
	}
	if (healed_on == run.depth && hp * 2 > max && !guardian) return false;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_HEAL) {
			*x = (int)layer.obj[i].x;
			*y = (int)layer.obj[i].y;
			going = run.depth;
			return true;
		}
	return false;
}

uint32_t autopilot_keys(void) {
	static uint32_t frame;
	++frame;
	int mode = emu_read8(emu_read32(BN6_TOOLKIT));
	if (mode != BN6_MODE_GAME || emu_read8(BN6_GAMESTATE) != BN6_SUB_MAP) {
		if (!weak()) return fight_keys(frame);
		weaken_enemies(++battle_frames > WEAK_FRAMES);
		return rhythm_keys(frame);
	}
	battle_frames = 0;
	if (emu_read8(BN6_CHATBOX)) return (frame / 4) & 1 ? KEY_A : 0;
	int px = bn6_player_x(), py = bn6_player_y();
	int cx, cy, ex, ey, nx, ny;
	bool talk = false;
	int wx, wy;
	if (director_in_town()) {
		/* in the town: along the streets to the jack-in, then R */
		const TownInfo *ti = town_info();
		if (abs(px - ti->port_x) + abs(py - ti->port_y) < 6) return frame % 16 < 2 ? KEY_R : 0;
		if (!town_route(px, py, &wx, &wy)) { wx = ti->port_x; wy = ti->port_y; }
	} else {
		if (!netmap_panel(px, py, &cx, &cy) || !director_goal_panel(&ex, &ey, &talk)) return 0;
		/* (weak walks as its pictures were timed, hurt or not) */
		if (!weak() && heal_panel(talk, &ex, &ey)) talk = true;
		if (cx < 0 || cy < 0 || cx >= MAP_W || cy >= MAP_H || !next_panel(cx, cy, ex, ey, &nx, &ny)) { nx = ex; ny = ey; }
		netmap_world(nx, ny, &wx, &wy);
	}
	/* the pad direction whose world motion best follows the path (UP moves
	 * +X -Y, RIGHT +X +Y, DOWN -X +Y, LEFT -X -Y; diagonals one axis) */
	static const struct { int x, y; uint32_t k; } dirs[8] = {
		{ 7, -7, KEY_UP }, { 10, 0, KEY_UP | KEY_RIGHT }, { 7, 7, KEY_RIGHT }, { 0, 10, KEY_DOWN | KEY_RIGHT },
		{ -7, 7, KEY_DOWN }, { -10, 0, KEY_DOWN | KEY_LEFT }, { -7, -7, KEY_LEFT }, { 0, -10, KEY_UP | KEY_LEFT },
	};
	/* beside the guardian or the heal: face him and talk (A also answers Yes) */
	if (talk) {
		int gx, gy;
		netmap_world(ex, ey, &gx, &gy);
		if (abs(gx - px) + abs(gy - py) < 40) {
			if (frame % 8 < 2) return KEY_A;
			wx = gx; wy = gy;   /* walk into him: MegaMan faces him */
		}
	}
	/* stuck on something (a Mystery Data, an NPC, a corner): take it, then
	 * try each direction in turn */
	static int last_x, last_y, still, unstick, tries;
	still = px == last_x && py == last_y ? still + 1 : 0;
	last_x = px; last_y = py;
	if (still > 45) { still = 0; unstick = 30; ++tries; }
	if (unstick > 0) {
		--unstick;
		if (unstick > 24) return unstick & 1 ? KEY_A : 0;
		return dirs[tries % 8].k;
	}
	int dx = wx - px, dy = wy - py;
	if (abs(dx) + abs(dy) < 3) return 0;
	uint32_t k = 0;
	long best = -1000000;
	for (int i = 0; i < 8; ++i) {
		long d = (long)dirs[i].x * dx + (long)dirs[i].y * dy;
		if (d > best) { best = d; k = dirs[i].k; }
	}
	return k;
}
