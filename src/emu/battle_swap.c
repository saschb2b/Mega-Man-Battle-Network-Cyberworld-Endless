/* A Navi swapped for another inside one BN6 battle (battle_swap.h).
 *
 * BN6 counts the enemies a battle began with (BattleState +5, the win at
 * its 0) and lowers the count at an enemy's destroy, after its deletion's
 * flashing; an enemy spawned mid-fight by the spawn's second entry
 * (AIData +2 1) is counted by none of it and freed at its destroy
 * (docs/rom/navis.md, A Navi swapped mid-battle). So the form deleted is
 * made such an arrival as its dying begins (BN6_NAVI_DIE): neither of its
 * counts lowered, its explosion and flashing passed by (BN6_NAVI_DIE_FLASH)
 * after some frames of mosaic, off its panel and the alive list by BN6's
 * own removal. The next form is spawned on that panel by BN6's mid-fight
 * spawn (BN6_SPAWN_ENEMY_1), called from the battle's frame
 * (BN6_BATTLE_MAIN) and back through a hook in free space, as a game call
 * comes back (gamecall.c); it arrives as BN6 brings a Navi into a fight
 * under way, its HP listed, in a mosaic that clears. As its arrival ends
 * it takes the first form's count (AIData +2 0, Param2 0), so that the last
 * form's deletion lowers the count to 0 and BN6 wins the battle as ever.
 * The count is the first form's from the start to the end: the battle never
 * stands at 0 between forms.
 *
 * Every hook here reads `S.armed` first and does nothing without it; they
 * are set at the first arm and taken off by battle_swap_disarm. The plan is
 * the director's, set between frames; the rest of `S` is the hooks'. */
#include "battle_swap.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bn6_battle_loop.h"
#include "bn6_families.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"

#define MOSAIC_FRAMES 24   /* a form's glitch out, and the next's in: the mosaic's frames */
#define SPAWN_TRIES   30   /* frames a spawn is retried for room before the battle is let end */
#define SIDE_ENEMY    1    /* the enemies' alliance */

typedef enum {
	SW_FIND,       /* the first form not yet found on the field */
	SW_FIGHT,      /* a form fighting */
	SW_OUT,        /* its dying begun, uncounted: its mosaic, then BN6's removal */
	SW_GONE,       /* off the alive list: the next form's spawn due at the battle's next frame */
	SW_ARRIVE,     /* the next form's arrival under way, its count not yet taken */
	SW_LAST,       /* the last form's deletion, BN6's own */
} Phase;

static struct {
	bool armed;
	int n;
	SwapForm form[SWAP_FORMS];
	/* (the hooks') */
	Phase phase;
	int cur, gone, fails, result;
	unsigned frames;           /* (the battle's frames seen, for the trace) */
	uint32_t obj, leaving;     /* the form fighting; the one being taken off */
	uint8_t px, py;            /* the panel it stood on */
	int out_left, in_left;     /* frames of mosaic left on the one going, and on the one come */
	bool in_call;              /* the battle's frame entered again from the spawn's return */
	uint32_t kept[4], kept_r5, kept_lr;   /* r0-r3, r5 and lr as the battle's frame met them */
} S;

static bool hooked;

static uint32_t bstate(uint32_t off) { return BN6_BATTLE_STATE + off; }

/* whether `o` is one of side 1's alive list */
static bool alive(uint32_t o) {
	for (uint32_t k = 0; k < 4; ++k)
		if (hook_read32(bstate(BN6_STATE_ALIVE + BN6_STATE_SIDE_STEP * SIDE_ENEMY + 4 * k)) == o) return true;
	return false;
}

/* a form's HP and MaxHP, and its HP's plate where it is listed */
static void give_hp(uint32_t o, int hp) {
	if (hp <= 0) return;
	hook_write16(o + BN6_T1_HP, (uint16_t)hp);
	hook_write16(o + BN6_T1_MAX_HP, (uint16_t)hp);
	for (uint32_t k = 0; k < BN6_HP_PLATE_COUNT; ++k) {
		uint32_t p = BN6_HP_PLATES + k * BN6_HP_PLATE_SIZE;
		if ((hook_read8(p) & 1) && hook_read32(p + BN6_HP_PLATE_OBJECT) == o) hook_write16(p + BN6_HP_PLATE_SHOWN, (uint16_t)hp);
	}
}

/* an object's sprite in BN6's own mosaic, `size` 0-15 its blocks (0: none) */
static void mosaic(uint32_t o, int size) {
	uint32_t spr = o + ((uint32_t)(hook_read8(o + BN6_OBJ_SPRITE_AT) >> 4) << 4) + BN6_SPRITE_MOSAIC_AT;
	uint8_t a = hook_read8(spr);
	hook_write8(spr, (uint8_t)(size ? a | BN6_SPRITE_MOSAIC : a & ~BN6_SPRITE_MOSAIC));
	uint32_t render = hook_read32(BN6_TOOLKIT + BN6_TOOLKIT_RENDER);
	if (render < BN6_EWRAM || render >= BN6_EWRAM_END) return;
	uint16_t m = hook_read16(render + BN6_RENDER_MOSAIC);
	hook_write16(render + BN6_RENDER_MOSAIC, (uint16_t)((m & 0x00FF) | (uint32_t)(size & 15) * 0x1100u));
}

/* the battle over: the hooks idle from here */
static bool battle_over(void) {
	uint8_t phase = hook_read8(BN6_LOOP);
	if (phase != BN6_LOOP_WON && phase != BN6_LOOP_LOST && phase != BN6_LOOP_DRAW && phase != BN6_LOOP_TIME_DRAW) return false;
	S.result = phase;
	S.armed = false;
	if (S.obj) mosaic(S.obj, 0);
	if (emu_debug_on()) fprintf(stderr, "swap: the battle over (loop phase %02x), form %d of %d, %d gone\n", phase, S.cur + 1, S.n, S.gone);
	return true;
}

/* whether the next form may stand on panel (x, y): the enemies', no hole, no one on it */
static bool free_panel(int x, int y) {
	uint32_t p = BN6_FIELD_PANELS + (uint32_t)(y * BN6_FIELD_COLUMNS + x) * BN6_PANEL_SIZE;
	return x >= 1 && x <= 6 && y >= 1 && y <= 3 && hook_read8(p + BN6_PANEL_ALLIANCE) == SIDE_ENEMY &&
		hook_read8(p + BN6_PANEL_TYPE) != BN6_PANEL_HOLE && !hook_read32(p + BN6_PANEL_RESERVER);
}

/* where the next form arrives: the panel the last stood on, where it was
 * the enemies' own (a Navi struck mid-attack may stand on MegaMan's), else
 * the first free of theirs from the middle back */
static void arrival_panel(void) {
	static const uint8_t near[][2] = { { 5, 2 }, { 5, 1 }, { 5, 3 }, { 6, 2 }, { 6, 1 }, { 6, 3 }, { 4, 2 }, { 4, 1 }, { 4, 3 } };
	if (free_panel(S.px, S.py)) return;
	for (size_t i = 0; i < sizeof near / sizeof *near; ++i)
		if (free_panel(near[i][0], near[i][1])) {
			S.px = near[i][0];
			S.py = near[i][1];
			return;
		}
}

/* the next form's spawn, from the battle's frame: BN6's mid-fight entry
 * with the panel the last stood on, back through FREE_SWAP_RETURN */
static HookAct spawn_next(HookRegs *r) {
	arrival_panel();
	memcpy(S.kept, r->r, sizeof S.kept);
	S.kept_r5 = r->r[5];
	S.kept_lr = r->lr;
	r->r[0] = (uint32_t)S.form[S.cur + 1].id;
	r->r[1] = S.px;
	r->r[2] = S.py;
	r->r[3] = SIDE_ENEMY;
	r->r[5] = 0;   /* (AIData +0x54: no summoner whose deletion takes it along, 0x0801B9BC) */
	r->lr = FREE_SWAP_RETURN | 1;
	r->r[12] = BN6_SPAWN_ENEMY_1 | 1;
	return HOOK_JUMP;
}

/* the arrival ended (its state on from the spawn's): the first form's count its own */
static void take_count(uint32_t o) {
	hook_write8(hook_read32(o + BN6_T1_AI_DATA) + BN6_AI_ARRIVAL, 0);
	hook_write8(o + BN6_T1_PARAM2, 0);
	/* (the foe finders' list as the battle began, its first form's place) */
	for (uint32_t k = 0; k < 4; ++k) {
		uint32_t at = bstate(BN6_STATE_ROSTER + BN6_STATE_SIDE_STEP * SIDE_ENEMY + 4 * k);
		if (hook_read32(at) == S.leaving) hook_write32(at, o);
	}
}

/* (CYBERWORLD_EMU_DEBUG) the form fighting: its state, HP, panel, and the counts */
static void trace(void) {
	uint32_t o = S.obj, plate = 0;
	for (uint32_t k = 0; k < BN6_HP_PLATE_COUNT; ++k)
		plate = plate << 8 | hook_read8(BN6_HP_PLATES + k * BN6_HP_PLATE_SIZE);
	fprintf(stderr, "swap: t%u form %d phase %d obj %08X state %02X/%02X/%02X hp %u panel %u,%u flags %02X coll %08X | loop %02X left %u counted %u plates %08X\n",
		(unsigned)hook_read32(BN6_BATTLE_TIMER), S.cur + 1, (int)S.phase, (unsigned)o, hook_read8(o + BN6_T1_STATE), hook_read8(o + BN6_T1_ACTION),
		hook_read8(o + BN6_T1_PHASE), hook_read16(o + BN6_T1_HP), hook_read8(o + BN6_T1_PANEL_X), hook_read8(o + BN6_T1_PANEL_Y), hook_read8(o),
		(unsigned)hook_read32(hook_read32(o + BN6_T1_COLLISION) + BN6_COLL_FLAGS), hook_read8(BN6_LOOP), hook_read8(bstate(BN6_STATE_SIDE1_LEFT)),
		hook_read8(bstate(BN6_STATE_COUNT + SIDE_ENEMY)), (unsigned)plate);
}

/* The battle's frame, every phase of it */
static HookAct frame(HookRegs *r, void *user) {
	(void)user;
	if (!S.armed) return HOOK_CONTINUE;
	if (S.in_call) {   /* (entered again from the spawn's return: its own frame already seen) */
		S.in_call = false;
		return HOOK_CONTINUE;
	}
	if (S.phase != SW_FIND && battle_over()) return HOOK_CONTINUE;
	if (emu_debug_on() && S.obj && ++S.frames % 30 == 0) trace();
	switch (S.phase) {
	case SW_FIND:
		for (uint32_t k = 0; k < 4 && !S.obj; ++k) {
			uint32_t o = hook_read32(bstate(BN6_STATE_ALIVE + BN6_STATE_SIDE_STEP * SIDE_ENEMY + 4 * k));
			if (o && hook_read16(o + BN6_T1_NAME_ID) == S.form[0].id) S.obj = o;
		}
		if (!S.obj) break;
		give_hp(S.obj, S.form[0].hp);
		S.cur = 0;
		S.phase = SW_FIGHT;
		if (emu_debug_on()) fprintf(stderr, "swap: form 1 %03X found at %08X, HP %u\n", S.form[0].id, (unsigned)S.obj, hook_read16(S.obj + BN6_T1_HP));
		break;
	case SW_OUT:
		if (S.out_left > 0) mosaic(S.leaving, 1 + (MOSAIC_FRAMES - S.out_left) * 15 / MOSAIC_FRAMES);
		if (alive(S.leaving)) break;
		S.phase = SW_GONE;   /* (and spawned in the frame it went) */
		return spawn_next(r);
	case SW_GONE:
		return spawn_next(r);
	case SW_ARRIVE:
		if (S.in_left > 0) mosaic(S.obj, --S.in_left * 15 / MOSAIC_FRAMES);
		if (hook_read8(S.obj + BN6_T1_ACTION) != 0 && S.in_left <= 0) {
			take_count(S.obj);
			S.phase = S.cur == S.n - 1 ? SW_LAST : SW_FIGHT;
			if (emu_debug_on()) fprintf(stderr, "swap: form %d arrived, its count taken (side 1 %u left, %u counted)\n", S.cur + 1,
				hook_read8(bstate(BN6_STATE_SIDE1_LEFT)), hook_read8(bstate(BN6_STATE_COUNT + SIDE_ENEMY)));
		}
		break;
	default:
		break;
	}
	return HOOK_CONTINUE;
}

/* The spawn's return: r0 the object it made, or none (no room) */
static HookAct returned(HookRegs *r, void *user) {
	(void)user;
	uint32_t o = r->r[0];
	int id = S.form[S.cur + 1].id;
	bool made = o >= BN6_T1_OBJECTS && o < BN6_T1_OBJECTS + BN6_T1_COUNT * BN6_T1_SIZE && (hook_read8(o + BN6_T1_IN_PLAY) & 1) &&
		hook_read16(o + BN6_T1_NAME_ID) == id;
	if (made) {
		S.cur++;
		S.gone++;
		S.obj = o;
		give_hp(o, S.form[S.cur].hp);
		S.in_left = MOSAIC_FRAMES;
		mosaic(o, 15);
		S.phase = SW_ARRIVE;
		if (emu_debug_on()) fprintf(stderr, "swap: form %d %03X spawned at %08X on %u,%u, HP %u\n", S.cur + 1, id, (unsigned)o, S.px, S.py, hook_read16(o + BN6_T1_HP));
	} else if (++S.fails >= SPAWN_TRIES) {
		/* (no room for it: the first form's count let go, and the battle won as BN6 wins it) */
		uint32_t left = bstate(BN6_STATE_SIDE1_LEFT), counted = bstate(BN6_STATE_COUNT + SIDE_ENEMY);
		if (hook_read8(left)) hook_write8(left, (uint8_t)(hook_read8(left) - 1));
		if (hook_read8(counted)) hook_write8(counted, (uint8_t)(hook_read8(counted) - 1));
		S.phase = SW_LAST;
		fprintf(stderr, "swap: form %d %03X found no room in %d frames: the battle let end\n", S.cur + 2, id, SPAWN_TRIES);
	}
	memcpy(r->r, S.kept, sizeof S.kept);
	r->r[5] = S.kept_r5;
	r->lr = S.kept_lr;
	S.in_call = true;
	r->r[12] = BN6_BATTLE_MAIN | 1;
	return HOOK_JUMP;
}

/* A Navi's dying begins (r5 its object): the form fighting, uncounted
 * where another follows; the last counted as the battle's first was */
static HookAct dying(HookRegs *r, void *user) {
	(void)user;
	uint32_t o = r->r[5];
	if (!S.armed || o != S.obj || S.phase == SW_FIND) return HOOK_CONTINUE;
	uint32_t ai = hook_read32(o + BN6_T1_AI_DATA);
	bool last = S.cur >= S.n - 1;
	hook_write8(ai + BN6_AI_ARRIVAL, last ? 0 : 1);
	hook_write8(o + BN6_T1_PARAM2, last ? 0 : 1);
	if (last) {
		S.phase = SW_LAST;
		if (emu_debug_on()) fprintf(stderr, "swap: the last form deleted, BN6's own\n");
		return HOOK_CONTINUE;
	}
	S.leaving = o;
	S.px = hook_read8(o + BN6_T1_PANEL_X);
	S.py = hook_read8(o + BN6_T1_PANEL_Y);
	S.out_left = MOSAIC_FRAMES;
	S.phase = SW_OUT;
	if (emu_debug_on()) fprintf(stderr, "swap: form %d taken off at %u,%u\n", S.cur + 1, S.px, S.py);
	return HOOK_CONTINUE;
}

/* The dying's phase 4 (r5 its object): for the form taken off, its frames
 * of mosaic in place of the explosion and the flashing (the routine passed
 * by, before its push), then BN6's removal at once */
static HookAct flash(HookRegs *r, void *user) {
	(void)user;
	uint32_t o = r->r[5];
	if (!S.armed || o != S.leaving || S.phase != SW_OUT || hook_read8(o + BN6_T1_PHASE_INIT)) return HOOK_CONTINUE;
	if (--S.out_left > 0) {
		/* (back to the caller, which called it by mov lr,pc; bx r1: lr's
		 * Thumb bit clear, so not bx lr, HOOK_RETURN's) */
		r->r[12] = r->lr | 1;
		return HOOK_JUMP;
	}
	mosaic(o, 0);
	hook_write8(o + BN6_T1_PHASE_INIT, 4);
	hook_write16(o + BN6_T1_TIMER, 0);
	hook_write32(hook_read32(o + BN6_T1_AI_DATA) + BN6_AI_DIE_SPARKS, 0);
	return HOOK_CONTINUE;
}

/* whether a form's id is a Navi of BN6's whose state 2 is BN6's generic dying */
static bool swappable(int id) {
	uint32_t ids = emu_read32(BN6_ENEMY_IDS_REF);
	if (id <= 0 || id >= BN6_ENEMY_IDS_COUNT || ids < 0x08000000u) return false;
	uint32_t t = ids + (uint32_t)id * 3;
	if (emu_read8(t + 1) != 1) return false;   /* (actor type 1: a Navi) */
	uint32_t ai = emu_read8(t + 2), table = emu_read32(emu_read32(BN6_NAVI_REF_STATES) + ai * 4);
	uint32_t die = emu_read32(table + 8);
	return die == BN6_NAVI_DYING || die == BN6_NAVI_DYING_PROTO;
}

bool battle_swap_arm(const SwapForm *forms, int n) {
	if (n < 2 || n > SWAP_FORMS) return false;
	for (int i = 0; i < n; ++i)
		if (!swappable(forms[i].id) || forms[i].hp < 0 || forms[i].hp > 0xFFF) {
			fprintf(stderr, "swap: form %d (%03X) is no Navi of BN6's whose deletion is its generic one\n", i + 1, forms[i].id);
			return false;
		}
	if (!hooked) {
		hooked = emu_hook(BN6_BATTLE_MAIN, frame, NULL) && emu_hook(FREE_SWAP_RETURN, returned, NULL) &&
			emu_hook(BN6_NAVI_DIE, dying, NULL) && emu_hook(BN6_NAVI_DIE_FLASH, flash, NULL);
		if (!hooked) {
			fprintf(stderr, "swap: its hooks did not fit\n");
			battle_swap_disarm();
			return false;
		}
	}
	emu_sync();   /* (the hooks' frame over before the plan is written: the core's own thread, where it has one) */
	memset(&S, 0, sizeof S);
	S.n = n;
	for (int i = 0; i < n; ++i) S.form[i] = forms[i];
	S.cur = -1;
	S.phase = SW_FIND;
	S.armed = true;
	return true;
}

void battle_swap_disarm(void) {
	emu_sync();
	S.armed = false;
	emu_unhook(BN6_BATTLE_MAIN);
	emu_unhook(FREE_SWAP_RETURN);
	emu_unhook(BN6_NAVI_DIE);
	emu_unhook(BN6_NAVI_DIE_FLASH);
	hooked = false;
}

SwapState battle_swap_state(void) {
	emu_sync();
	return (SwapState){ S.armed, S.n, S.cur, S.gone, S.fails, S.result };
}
