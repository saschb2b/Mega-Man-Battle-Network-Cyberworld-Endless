/* Grid battle. Coordinates: columns 0-5 left to right, rows 0-2 back to
 * front. Columns 0-2 start as MegaMan's (red) side, 3-5 as the enemy's. */
#include "battle.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "data.h"
#include "game.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"
#include "run.h"

#define COLS 6
#define ROWS 3
#define PANEL_W 40
#define PANEL_H 24
#define MAX_ENTS 10
#define MAX_SPELLS 48
#define HAND_MAX 5
#define OFFER_MAX 10
#define FOOT_Y 12 /* sprite origin below the panel top edge */
#define UI (R.layout->ui)

enum { PT_BROKEN = 1, PT_NORMAL = 2, PT_CRACKED = 3, PT_POISON = 4, PT_HOLY = 5, PT_GRASS = 6, PT_ICE = 7, PT_SWAMP = 8 };
enum { SIDE_PLAYER = 0, SIDE_ENEMY = 1 };
enum { K_PLAYER, K_VIRUS, K_NAVI, K_ROCK };

enum {
	HF_FLINCH = 1 << 0,
	HF_STUN = 1 << 1,
	HF_PUSH = 1 << 2,
	HF_NOINV = 1 << 3,   /* multi-hit: no invulnerability window */
	HF_BREAK = 1 << 4,   /* breaks barriers outright */
	HF_CRACK = 1 << 5,
	HF_PIERCE = 1 << 6,
};

typedef struct {
	uint8_t type, side;
	int restore;   /* frames until a broken panel repairs */
	int steal;     /* frames until a stolen panel returns */
	uint8_t home;  /* original side */
	int warn;      /* frames of danger highlight */
} Panel;

typedef struct {
	bool on;
	int kind, side, col, row;
	int hp, maxhp, elem, atk, shown_hp;
	int fam, ver, enemy_id;
	const VirusDef *vd;
	const NaviDef *nd;
	Sprite *spr;
	Anim anim;
	int pal;
	int state, timer, sub, tcol, trow, count;
	int ticket;    /* Mettaurs: turn order, the lowest holds the turn */
	int home_col, home_row;
	int flash, invuln, stun, flinch;
	int barrier;
	int invis;
	int ox, oy;
	bool dying;
	int die_timer;
	int appear;    /* materialising: 0 hidden .. 16 fully there */
	char name[24];
} Ent;

enum { SP_FX, SP_WAVE, SP_PROJ, SP_LOB, SP_TARGET, SP_BEAM, SP_THUNDER, SP_TORNADO, SP_BOOMER, SP_METEOR, SP_ROLL, SP_BOMB };

typedef struct {
	bool on;
	int type, side, col, row;
	float x, y, vx, vy;
	int timer, life, step, dmg, elem, flags, param;
	int tcol, trow;
	Sprite *spr;
	Anim anim;
	bool flip, loop;
	int owner;
	uint16_t hits; /* entities already hit */
	SDL_Color tint;
} Spell;

static struct {
	Encounter enc;
	void (*done)(const BattleResult *);
	Panel field[ROWS][COLS];
	Ent ent[MAX_ENTS];
	Spell sp[MAX_SPELLS];
	int state, state_timer;
	int custom;       /* gauge 0..CUSTOM_FULL */
	int turn;
	FolderChip deck[FOLDER_MAX];
	int deck_n, deck_pos;
	FolderChip offer[OFFER_MAX];
	int offer_n;
	bool picked[OFFER_MAX];
	int pick_order[HAND_MAX];
	int npicked;
	int cursor;       /* 0..offer_n-1, OFFER_MAX = OK */
	FolderChip hand[HAND_MAX];
	int hand_n;
	int atk_plus;
	/* player action */
	int act, act_timer, act_chip_kind, act_param, act_power, act_elem, act_id;
	int charge;
	int move_dc, move_dr;
	int hits_taken;
	int frames;
	int shake;
	char banner[32];
	int banner_timer;
	char chip_label[32];
	int chip_label_timer;
	bool undershirt_used;
	int scroll;
	int cross;        /* active style: navi index, 99 = Gregar beast, 0 = none */
	int cross_pick;   /* chosen in the Custom screen */
	bool crossed;     /* one transformation per battle */
	int beast_turns;
	int pa_flash;
	char pa_name[24];
	int shift;        /* the field slides down while the Custom window is open */
	int slide;        /* Custom window position, 0 hidden .. 10 open (12 px a step) */
	RewardOption reward[4];
	int reward_n, reward_sel;
	int result_busting;
	/* flow timing, measured from the original (docs/BATTLE_FLOW.md) */
	int t;            /* frames in the current state */
	int spawn_i;      /* enemy materialising in BS_SPAWN */
	bool foe_hp_on;   /* enemy HP shown under the enemies */
	bool gauge_on, icons_on;
	int banner_t;     /* frames into the BATTLE START / ENEMY DELETED banner, -1 none */
	int banner_which;
	int fight_t;      /* frames since the fight (re)started */
	int custom_req;   /* frames since L/R asked for the Custom window */
	bool paused;      /* START in a fight: everything stops under PAUSE */
	int full_t;       /* frames the gauge has been full (its animation runs on in a pause) */
	Anim charge_fx;   /* charge lines, then the charged glow */
	bool shot_charged; /* the buster shot waiting to leave */
	int tickets;       /* Mettaur turn tickets handed out */
	int bust_t, bust_row, bust_dmg; /* a shot on its way: it lands after bust_t frames */
	bool bust_charged;
	int emblem_t;     /* frames since a chip was picked (the emblem spins) */
	bool gauge_was_full;
	int hp_hurt;      /* frames the HP box stays in its damage colour */
	int desc_t;       /* chip description box: lines shown (0 closed) */
	int desc_close;   /* frame of the closing line */
	int result_x;     /* RESULT window slide */
	int reveal_t;     /* frames since the reward reveal started, -1 not yet */
	uint8_t reveal_order[42];
} B;

static RewardMaker reward_maker;
void battle_set_rewards(RewardMaker make) { reward_maker = make; }

bool battle_debug_one_hp; /* test hook: foes start at 1 HP */
int battle_debug_bg = -1; /* test hook: force a background id */

#define BEAST 99

static void revert_style(const char *why);
const char *cross_name(int cross);

/* BS_LOAD white screen then fade in, BS_SPAWN enemies materialise one by
 * one, BS_CUSTOM window open (sliding in), BS_CUSTOM_OUT sliding out,
 * BS_START waiting for and showing BATTLE START, BS_FIGHT, BS_WIN ENEMY
 * DELETED and the pause after it, BS_RESULT the window, BS_EXIT fade out. */
enum { BS_LOAD, BS_SPAWN, BS_CUSTOM, BS_CUSTOM_OUT, BS_START, BS_FIGHT, BS_WIN, BS_RESULT, BS_EXIT, BS_LOSE, BS_DONE };
enum { ACT_NONE, ACT_MOVE_OUT, ACT_MOVE_IN, ACT_BUSTER, ACT_CHARGED, ACT_CHIP, ACT_HIT, ACT_SHOT_WAIT };

#define CUSTOM_FULL 512  /* one per frame: the gauge fills in 512 frames */
#define PLAYER (&B.ent[0])

/* ---------------------------------------------------------------- */
/* Geometry */

static int field_x(void) { return P.core_x; }
static int field_y(void) { return P.core_y + 72 + B.shift; }
static int foot_x(int col) { return field_x() + col * PANEL_W + PANEL_W / 2; }
static int foot_y(int row) { return field_y() + row * PANEL_H + FOOT_Y; }
static bool in_field(int c, int r) { return c >= 0 && c < COLS && r >= 0 && r < ROWS; }

static Ent *ent_at(int c, int r) {
	for (int i = 0; i < MAX_ENTS; ++i) {
		Ent *e = &B.ent[i];
		if (e->on && !e->dying && e->col == c && e->row == r) return e;
	}
	return NULL;
}

static bool walkable(int c, int r, int side, bool air) {
	if (!in_field(c, r)) return false;
	Panel *p = &B.field[r][c];
	if (p->side != side) return false;
	if (p->type == PT_BROKEN && !air) return false;
	return ent_at(c, r) == NULL;
}

/* ---------------------------------------------------------------- */
/* Spells */

static Spell *spell_new(int type, int side, int col, int row) {
	for (int i = 0; i < MAX_SPELLS; ++i) {
		if (!B.sp[i].on) {
			Spell *s = &B.sp[i];
			memset(s, 0, sizeof *s);
			s->on = true;
			s->type = type;
			s->side = side;
			s->col = col;
			s->row = row;
			s->x = (float)foot_x(col);
			s->y = (float)foot_y(row);
			s->owner = -1;
			s->tint = WHITE;
			return s;
		}
	}
	return NULL;
}

static void spell_sprite(Spell *s, int cat, int idx, int anim, bool loop) {
	if (cat < 0 || idx < 0) return;
	s->spr = sprite_get(cat, idx);
	if (s->spr) anim_start(&s->anim, s->spr, anim < sprite_anim_count(s->spr) ? anim : 0);
	s->loop = loop;
}

static void fx(int cat, int idx, int anim, int x, int y) {
	Spell *s = spell_new(SP_FX, 0, 0, 0);
	if (!s) return;
	s->x = (float)x;
	s->y = (float)y;
	spell_sprite(s, cat, idx, anim, false);
	s->life = 90;
	if (!s->spr) s->on = false;
}

/* ---------------------------------------------------------------- */
/* Damage */

static bool beats(int atk, int def) {
	return (atk == ELEM_FIRE && def == ELEM_WOOD) || (atk == ELEM_WOOD && def == ELEM_ELEC) ||
	       (atk == ELEM_ELEC && def == ELEM_AQUA) || (atk == ELEM_AQUA && def == ELEM_FIRE);
}

static void kill_ent(Ent *e);

static void set_banner(const char *t, int frames) {
	snprintf(B.banner, sizeof B.banner, "%s", t);
	B.banner_timer = frames;
}

static void player_flinch(void);

static void roll_hp(Ent *e);

static int damage_ent(Ent *e, int dmg, int elem, int flags) {
	if (!e->on || e->dying) return 0;
	if (e->kind == K_PLAYER && (e->invuln > 0 || e->invis > 0) && !(flags & HF_BREAK)) return 0;
	if (e->barrier > 0) {
		if (flags & HF_BREAK) e->barrier = 0;
		else {
			e->barrier -= dmg;
			if (e->barrier < 0) e->barrier = 0;
			audio_sfx(SFX_GUARD);
			return 0;
		}
	}
	if (beats(elem, e->elem)) {
		dmg *= 2;
		if (e->kind == K_PLAYER && B.cross && B.cross != BEAST) revert_style("CROSS BROKEN!");
	}
	Panel *p = &B.field[e->row][e->col];
	if (p->type == PT_GRASS && elem == ELEM_FIRE) dmg *= 2;
	if (p->type == PT_HOLY) dmg /= 2;
	if (dmg < 1) dmg = 1;
	if (e->kind == K_PLAYER && (run.perks & PERK_UNDERSHIRT) && !B.undershirt_used && e->hp > 1 && dmg >= e->hp) {
		dmg = e->hp - 1;
		B.undershirt_used = true;
	}
	e->hp -= dmg;
	e->flash = 1; /* white for the frame of the hit only, as recorded */
	roll_hp(e);   /* the counter starts rolling on the hit frame */
	if (flags & HF_STUN) e->stun = 90;
	if (e->kind == K_PLAYER) {
		B.hits_taken++;
		B.hp_hurt = 17;
		audio_sfx(SFX_HURT);
		if (!(flags & HF_NOINV) || e->hp <= 0) {
			if (!(run.perks & PERK_SUPER_ARMOR) || e->hp <= 0) player_flinch();
			e->invuln = 120;
		}
	} else {
		audio_sfx(SFX_HIT);
		if ((flags & HF_FLINCH) && e->kind == K_NAVI && e->invuln <= 0) {
			e->flinch = 20;
			e->invuln = 40;
		}
	}
	if (e->hp <= 0) {
		e->hp = 0;
		kill_ent(e);
	}
	return dmg;
}

/* Attack one panel on behalf of `side`. Returns the entity hit, if any. */
static Ent *strike(int side, int c, int r, int dmg, int elem, int flags) {
	if (!in_field(c, r)) return NULL;
	if (flags & HF_CRACK) {
		Panel *p = &B.field[r][c];
		if (p->type == PT_CRACKED && !ent_at(c, r)) { p->type = PT_BROKEN; p->restore = 600; }
		else if (p->type != PT_BROKEN) p->type = PT_CRACKED;
	}
	Ent *e = ent_at(c, r);
	if (!e) return NULL;
	if (e->side == side && e->kind != K_ROCK) return NULL;
	damage_ent(e, dmg, elem, flags);
	if ((flags & HF_PUSH) && e->on && !e->dying && e->kind != K_ROCK) {
		int nc = c + (side == SIDE_PLAYER ? 1 : -1);
		if (walkable(nc, r, e->side, false)) e->col = nc;
	}
	return e;
}

/* First entity in the row, scanning away from the attacker. */
static Ent *first_in_row(int side, int col, int row, int *hit_col) {
	int dir = side == SIDE_PLAYER ? 1 : -1;
	for (int c = col + dir; c >= 0 && c < COLS; c += dir) {
		Ent *e = ent_at(c, row);
		if (e && (e->side != side || e->kind == K_ROCK)) {
			if (hit_col) *hit_col = c;
			return e;
		}
	}
	return NULL;
}

static int count_foes(void) {
	int n = 0;
	for (int i = 1; i < MAX_ENTS; ++i)
		if (B.ent[i].on && (B.ent[i].kind == K_VIRUS || B.ent[i].kind == K_NAVI)) ++n;
	return n;
}

/* Deletion as in the original: the enemy flashes for 33 frames while two
 * explosions go off, the second 16 frames later and up to the left. */
#define DIE_FRAMES 33

static void explosion(Ent *e, int dx, int dy) {
	Spell *s = spell_new(SP_FX, 0, 0, 0);
	if (!s) return;
	spell_sprite(s, SPR_GUI, (int)R.layout->ui.delete_sprite, 0, false);
	if (!s->spr) { s->on = false; return; }
	SDL_Rect b = sprite_frame_bounds(s->spr, 0, 0, 0);
	s->x = (float)(foot_x(e->col) + e->ox + dx - (b.x + b.w / 2));
	s->y = (float)(foot_y(e->row) - 18 + dy - (b.y + b.h / 2));
	s->life = 60;
	audio_sfx(SFX_DELETE);
}

static void kill_ent(Ent *e) {
	e->dying = true;
	e->die_timer = DIE_FRAMES;
	if (e->kind == K_VIRUS || e->kind == K_NAVI) run.viruses_deleted++;
}

static void update_dying(Ent *e) {
	int t = DIE_FRAMES - e->die_timer;
	if (e->kind != K_PLAYER && e->kind != K_ROCK) {
		if (t == 2) explosion(e, 0, 0);
		if (t == 18) explosion(e, -12, -7);
	} else if (e->kind == K_ROCK && t == 0) {
		fx(SPR_HIT, 0, 0, foot_x(e->col), foot_y(e->row) - 12);
	}
	if (--e->die_timer <= 0) e->on = false;
}

/* ---------------------------------------------------------------- */
/* Field setup */

static void setup_field(int preset) {
	for (int r = 0; r < ROWS; ++r) {
		for (int c = 0; c < COLS; ++c) {
			Panel *p = &B.field[r][c];
			memset(p, 0, sizeof *p);
			p->side = p->home = c < 3 ? SIDE_PLAYER : SIDE_ENEMY;
			p->type = PT_NORMAL;
			int roll = rng_range(0, 99);
			switch (preset) {
			case 1: if (roll < 30) p->type = PT_CRACKED; break;
			case 2: if (roll < 45) p->type = PT_GRASS; break;
			case 3: if (roll < 25 && c != 1) p->type = PT_POISON; break;
			case 4: if (roll < 15) p->type = PT_HOLY; break;
			case 5: if (roll < 20 && c != 1) p->type = PT_BROKEN; break;
			default: break;
			}
		}
	}
	B.field[1][1].type = PT_NORMAL;
}

static Ent *spawn(int kind, int side, int col, int row) {
	for (int i = kind == K_PLAYER ? 0 : 1; i < MAX_ENTS; ++i) {
		if (!B.ent[i].on) {
			Ent *e = &B.ent[i];
			memset(e, 0, sizeof *e);
			e->on = true;
			e->kind = kind;
			e->side = side;
			e->col = e->home_col = col;
			e->row = e->home_row = row;
			e->appear = 16;
			return e;
		}
	}
	return NULL;
}

static void ent_anim(Ent *e, int anim) {
	if (!e->spr || anim < 0) return;
	if (anim >= sprite_anim_count(e->spr)) anim = 0;
	/* entity animations update before the logic, so a new one keeps its first frame whole */
	anim_play(&e->anim, e->spr, anim);
}

static void spawn_foe(const Foe *f) {
	if (f->kind == FOE_ROCK) {
		Ent *e = spawn(K_ROCK, SIDE_ENEMY, f->col, f->row);
		if (!e) return;
		e->hp = e->maxhp = 200;
		e->spr = sprite_get(SPR_EFFECT, 8);
		ent_anim(e, 0);
		snprintf(e->name, sizeof e->name, "Rock");
		return;
	}
	int kind = f->kind == FOE_NAVI ? K_NAVI : K_VIRUS;
	Ent *e = spawn(kind, SIDE_ENEMY, f->col, f->row);
	if (!e) return;
	e->fam = f->family;
	e->ticket = ++B.tickets;
	e->ver = f->version;
	e->enemy_id = enemy_id(kind == K_NAVI ? 1 : 0, f->family, f->version);
	if (e->enemy_id < 0) e->enemy_id = enemy_id(kind == K_NAVI ? 1 : 0, f->family, 0);
	e->hp = e->maxhp = e->shown_hp = e->enemy_id >= 0 ? enemy_hp(e->enemy_id) : 100;
	e->elem = e->enemy_id >= 0 ? enemy_element(e->enemy_id) : 0;
	e->atk = e->enemy_id >= 0 ? enemy_attack(e->enemy_id) : 10;
	if (e->atk < 5) e->atk = 10 + 10 * f->version;
	e->pal = kind == K_VIRUS ? f->version : 0;
	if (kind == K_VIRUS) {
		for (int i = 0; i < virus_def_count; ++i)
			if (virus_defs[i].ai_index == f->family) e->vd = &virus_defs[i];
		e->spr = sprite_get(SPR_VIRUS, f->family);
		enemy_name(e->enemy_id, e->name, sizeof e->name);
		ent_anim(e, e->vd ? e->vd->anim_idle : 0);
	} else {
		e->nd = navi_def(f->family);
		e->spr = sprite_get(SPR_NAVI, f->family);
		/* The navi chip names carry the game's own abbreviations: BlastMn[EX]. */
		ChipInfo nci;
		chip_info(e->nd->chip_reward + (f->version > 2 ? 2 : f->version), &nci);
		snprintf(e->name, sizeof e->name, "%s", nci.name);
		ent_anim(e, 0);
		/* Early guardians are gentler; later ones keep getting tougher. */
		int act = ((run.depth - 1) % CYCLE_LAYERS) / 3, loop = (run.depth - 1) / CYCLE_LAYERS;
		static const int pct[7] = { 45, 60, 75, 90, 100, 110, 125 };
		int hp = e->maxhp * pct[act > 6 ? 6 : act] / 100 + loop * 400;
		e->hp = e->maxhp = e->shown_hp = hp < 200 ? 200 : hp;
	}
	e->timer = rng_range(30, 90);
	e->appear = 0;
	if (battle_debug_one_hp) e->hp = e->shown_hp = 1;
}

static void shuffle_deck(void) {
	B.deck_n = run.folder_n;
	memcpy(B.deck, run.folder, sizeof(FolderChip) * (size_t)run.folder_n);
	for (int i = B.deck_n - 1; i > 0; --i) {
		int j = rng_range(0, i);
		FolderChip t = B.deck[i]; B.deck[i] = B.deck[j]; B.deck[j] = t;
	}
	B.deck_pos = 0;
	B.offer_n = 0;
}

static void open_custom(void) {
	if (B.cross == BEAST && --B.beast_turns <= 0) revert_style("BEAST OUT ENDS");
	/* Keep unchosen chips, draw new ones up to the custom size. */
	int keep = 0;
	for (int i = 0; i < B.offer_n; ++i) if (!B.picked[i]) B.offer[keep++] = B.offer[i];
	B.offer_n = keep;
	int size = run.custom_size;
	if (size > OFFER_MAX) size = OFFER_MAX;
	while (B.offer_n < size && B.deck_pos < B.deck_n) B.offer[B.offer_n++] = B.deck[B.deck_pos++];
	memset(B.picked, 0, sizeof B.picked);
	B.npicked = 0;
	B.cursor = 0;
	B.state = BS_CUSTOM;
	B.state_timer = 0;
	B.slide = 1; /* first frame already 12 pixels in */
	B.emblem_t = 100;
	B.shift = 2;
	B.desc_t = 0;
	B.gauge_on = false;
	audio_sfx(SFX_CUSTOM_OPEN);
}

void battle_begin(const Encounter *e, void (*done)(const BattleResult *r)) {
	memset(&B, 0, sizeof B);
	B.enc = *e;
	B.done = done;
	setup_field(e->field);
	Ent *p = spawn(K_PLAYER, SIDE_PLAYER, 1, 1);
	p->hp = p->shown_hp = run.hp;
	p->maxhp = run.max_hp;
	p->spr = sprite_get(SPR_BATTLE, 0);
	snprintf(p->name, sizeof p->name, "MegaMan");
	ent_anim(p, 0);
	if (run.perks & PERK_FIRST_BARRIER) p->barrier = 10;
	for (int i = 0; i < e->nfoes; ++i) {
		spawn_foe(&e->foes[i]);
		Panel *pp = &B.field[e->foes[i].row][e->foes[i].col];
		if (pp->type == PT_BROKEN) pp->type = PT_NORMAL;
	}
	shuffle_deck();
	/* The screen arrives white from the encounter flash; music starts with the fade-in. */
	B.state = BS_LOAD;
	B.t = 0;
	B.banner_t = -1;
	B.reveal_t = -1;
	audio_music(MUS_NONE);
	scene_set(&scene_battle);
}

/* ---------------------------------------------------------------- */
/* Player */

static void player_flinch(void) {
	Ent *p = PLAYER;
	B.act = ACT_HIT;
	B.act_timer = 20;
	B.charge = 0;
	ent_anim(p, 1);
}

static bool code_ok(char code, int ignore) {
	/* Selection rule: all picked chips share a code (or '*'), or all share a name. */
	char need = 0;
	int first_id = -1;
	bool same_name = true;
	for (int i = 0; i < B.offer_n; ++i) {
		if (!B.picked[i] || i == ignore) continue;
		if (first_id < 0) first_id = B.offer[i].id;
		else if (B.offer[i].id != first_id) same_name = false;
		if (B.offer[i].code != '*') {
			if (need && need != B.offer[i].code) return false;
			need = B.offer[i].code;
		}
	}
	(void)same_name;
	return !need || code == '*' || code == need;
}

static bool can_pick(int i) {
	if (i < 0 || i >= B.offer_n || B.picked[i] || B.npicked >= HAND_MAX) return false;
	/* Same chip always combines. */
	bool all_same = true;
	for (int k = 0; k < B.offer_n; ++k)
		if (B.picked[k] && B.offer[k].id != B.offer[i].id) all_same = false;
	if (B.npicked > 0 && all_same) return true;
	return code_ok(B.offer[i].code, -1);
}

static void start_chip(void);
static void player_shoot_normal(bool charged);

/* ---------------------------------------------------------------- */
/* Program Advances and styles */

static bool pa_match(const ProgramAdvance *pa, const FolderChip *c) {
	/* Any order: sort the three picks by id then code. */
	FolderChip s[3] = { c[0], c[1], c[2] };
	for (int a = 0; a < 3; ++a)
		for (int b = a + 1; b < 3; ++b)
			if (s[b].id < s[a].id || (s[b].id == s[a].id && s[b].code < s[a].code)) { FolderChip t = s[a]; s[a] = s[b]; s[b] = t; }
	if (pa->mode == 0) {
		if (s[0].id != pa->parts[0] || s[1].id != pa->parts[0] || s[2].id != pa->parts[0]) return false;
		return s[1].code == s[0].code + 1 && s[2].code == s[1].code + 1;
	}
	char code = 0;
	for (int i = 0; i < 3; ++i) {
		if (s[i].id != pa->parts[i]) return false;
		if (s[i].code != '*') {
			if (code && code != s[i].code) return false;
			code = s[i].code;
		}
	}
	return true;
}

static void form_program_advance(void) {
	for (int start = 0; start + 3 <= B.hand_n; ++start) {
		for (int k = 0; k < program_advance_count; ++k) {
			const ProgramAdvance *pa = &program_advances[k];
			if (!pa_match(pa, &B.hand[start])) continue;
			FolderChip result = { pa->result, B.hand[start].code };
			B.hand[start] = result;
			memmove(&B.hand[start + 1], &B.hand[start + 3], sizeof(FolderChip) * (size_t)(B.hand_n - start - 3));
			B.hand_n -= 2;
			ChipInfo ci;
			chip_info(pa->result, &ci);
			snprintf(B.pa_name, sizeof B.pa_name, "%s", ci.name);
			B.pa_flash = 90;
			audio_sfx(SFX_CHARGED);
			return;
		}
	}
}

static int cross_sprite(int cross) {
	switch (cross) {
	case 1: return 1; /* Heat */
	case 2: return 2; /* Elec */
	case 3: return 3; /* Slash */
	case 4: return 4; /* Erase */
	case 5: return 5; /* Charge */
	case BEAST: return 11;
	default: return 0;
	}
}

static int cross_element(int cross) {
	switch (cross) {
	case 1: return ELEM_FIRE;
	case 2: return ELEM_ELEC;
	default: return ELEM_NULL;
	}
}

const char *cross_name(int cross) {
	switch (cross) {
	case 1: return "HeatCross";
	case 2: return "ElecCross";
	case 3: return "SlashCross";
	case 4: return "EraseCross";
	case 5: return "ChargeCross";
	case BEAST: return "Gregar Beast";
	default: return "None";
	}
}

static void transform(int cross) {
	Ent *p = PLAYER;
	B.cross = cross;
	B.crossed = true;
	p->spr = sprite_get(SPR_BATTLE, cross_sprite(cross));
	p->elem = cross_element(cross);
	ent_anim(p, 0);
	if (cross == BEAST) B.beast_turns = 3;
	fx(SPR_HIT, 0, 0, foot_x(p->col), foot_y(p->row) - 20);
	set_banner(cross == BEAST ? "BEAST OUT!" : cross_name(cross), 50);
	audio_sfx(SFX_CHARGED);
}

static void revert_style(const char *why) {
	Ent *p = PLAYER;
	B.cross = 0;
	p->spr = sprite_get(SPR_BATTLE, 0);
	p->elem = ELEM_NULL;
	ent_anim(p, 0);
	set_banner(why, 40);
}

static void cross_charge_shot(void) {
	Ent *p = PLAYER;
	int col, base = run.atk * 10 + 20;
	if (run.perks & PERK_ATTACK_MAX) base = base * 3 / 2;
	Ent *e;
	switch (B.cross) {
	case 1: /* Heat: flamethrower three panels ahead */
		for (int i = 1; i <= 3; ++i) {
			strike(SIDE_PLAYER, p->col + i, p->row, base, ELEM_FIRE, HF_FLINCH);
			if (in_field(p->col + i, p->row)) fx(SPR_EFFECT, 0x02, 0, foot_x(p->col + i), foot_y(p->row));
		}
		audio_sfx(SFX_FLAME);
		break;
	case 2: /* Elec: stunning bolt on the first enemy */
		e = first_in_row(SIDE_PLAYER, p->col, p->row, &col);
		if (e) { damage_ent(e, base, ELEM_ELEC, HF_FLINCH | HF_STUN); fx(SPR_EFFECT, 0x32, 0, foot_x(col), foot_y(p->row)); }
		audio_sfx(SFX_THUNDER);
		break;
	case 3: /* Slash: wide cut in front */
		for (int dy = -1; dy <= 1; ++dy) strike(SIDE_PLAYER, p->col + 1, p->row + dy, base + 20, ELEM_NULL, HF_FLINCH);
		fx(SPR_ATTACK, 0x14, 0, foot_x(p->col + 1) + 2, foot_y(p->row) - 11);
		audio_sfx(SFX_SWORD);
		break;
	case 4: /* Erase: finishes off weakened enemies */
		e = first_in_row(SIDE_PLAYER, p->col, p->row, &col);
		if (e) {
			int dmg = (e->hp <= 120 && e->kind != K_NAVI) ? e->hp : base;
			damage_ent(e, dmg, ELEM_NULL, HF_FLINCH);
			fx(SPR_HIT, 7, 0, foot_x(col), foot_y(p->row) - 16);
		}
		audio_sfx(SFX_SWORD);
		break;
	case 5: /* Charge: ramming blow on the first enemy */
		e = first_in_row(SIDE_PLAYER, p->col, p->row, &col);
		if (e) { damage_ent(e, base * 3 / 2, ELEM_NULL, HF_FLINCH | HF_BREAK); fx(SPR_HIT, 0, 0, foot_x(col), foot_y(p->row) - 16); }
		audio_sfx(SFX_DASH);
		break;
	default: /* Gregar Beast: lock-on claw flurry */
		for (int i = 1; i < MAX_ENTS; ++i) {
			Ent *o = &B.ent[i];
			if (!o->on || o->dying || o->side == SIDE_PLAYER) continue;
			damage_ent(o, run.atk * 8 + 10, ELEM_NULL, HF_NOINV);
			fx(SPR_HIT, 5, 0, foot_x(o->col), foot_y(o->row) - 16);
			break;
		}
		audio_sfx(SFX_SWORD);
		break;
	}
}

static void player_shoot(bool charged) {
	if (charged && B.cross) {
		cross_charge_shot();
		ent_anim(PLAYER, B.cross == 3 ? 6 : 9);
		B.act = ACT_CHARGED;
		B.act_timer = 16;
		return;
	}
	player_shoot_normal(charged);
}

/* The muzzle flash (attack sprite 6) leaves the buster with the sound; the
 * shot lands 3 frames later, as recorded. */
#define BUST_FLASH_X 40
#define BUST_FLASH_Y (-27)
static void player_shoot_normal(bool charged) {
	Ent *p = PLAYER;
	int dmg = charged ? run.atk * 10 : run.atk;
	if (charged && (run.perks & PERK_ATTACK_MAX)) dmg = dmg * 3 / 2;
	B.bust_t = 4; /* counted down this frame too: lands 3 frames on */
	B.bust_row = p->row;
	B.bust_dmg = dmg;
	B.bust_charged = charged;
	fx(SPR_ATTACK, 6, 0, foot_x(p->col) + BUST_FLASH_X, foot_y(p->row) + BUST_FLASH_Y);
	audio_sfx(charged ? SFX_CHARGE_SHOT : SFX_BUSTER);
	B.act = charged ? ACT_CHARGED : ACT_BUSTER;
	B.act_timer = charged ? 16 : 10 - run.rapid;
}

static void update_buster_shot(void) {
	if (B.bust_t <= 0 || --B.bust_t > 0) return;
	Ent *p = PLAYER;
	int col;
	Ent *e = first_in_row(SIDE_PLAYER, p->col, B.bust_row, &col);
	if (!e) return;
	damage_ent(e, B.bust_dmg, ELEM_NULL, B.bust_charged ? HF_FLINCH : HF_NOINV);
	/* the spark starts the frame after the hit's white flash */
	Spell *s = spell_new(SP_FX, 0, 0, 0);
	if (!s) return;
	s->x = (float)foot_x(col);
	s->y = (float)(foot_y(B.bust_row) - 16);
	spell_sprite(s, SPR_HIT, B.bust_charged ? 4 : 5, 0, false);
	s->life = 90;
	s->timer = -1;
}

/* Charge 1 takes 101 frames (the original's first level); the charge
 * lines and sound start at 11. */
#define CHARGE_SHOWN 11
static int charge_time(void) { return 101 - (run.charge - 1) * 12; }

static void update_player(void) {
	Ent *p = PLAYER;
	if (p->dying) return;
	if (p->invuln > 0) --p->invuln;
	if (p->invis > 0) --p->invis;
	if (p->stun > 0) { --p->stun; return; }
	/* Panel effects */
	Panel *pp = &B.field[p->row][p->col];
	if (pp->type == PT_POISON && !(run.perks & PERK_FLOAT_SHOES) && B.frames % 8 == 0 && p->hp > 1) p->hp--;
	if (pp->type == PT_GRASS && p->elem == ELEM_WOOD && B.frames % 30 == 0 && p->hp < p->maxhp) p->hp++;

	if (B.act != ACT_NONE) {
		if (--B.act_timer > 0) {
			if (B.act == ACT_CHIP) start_chip();
			/* MegaMan raises the buster the frame before the shot */
			if (B.act == ACT_SHOT_WAIT && B.act_timer == 1) ent_anim(p, 9);
			return;
		}
		switch (B.act) {
		case ACT_SHOT_WAIT:
			player_shoot(B.shot_charged);
			return;
		case ACT_MOVE_OUT: {
			Panel *from = &B.field[p->row][p->col];
			int oc = p->col, orr = p->row;
			p->col += B.move_dc;
			p->row += B.move_dr;
			if (from->type == PT_CRACKED && !(run.perks & PERK_FLOAT_SHOES)) {
				B.field[orr][oc].type = PT_BROKEN;
				B.field[orr][oc].restore = 600;
			}
			ent_anim(p, 3);
			B.act = ACT_MOVE_IN;
			B.act_timer = 4;
			return;
		}
		case ACT_CHIP:
			start_chip();
			if (B.act_timer > 0) return;
			break;
		default:
			break;
		}
		B.act = ACT_NONE;
		ent_anim(p, 0);
	}

	/* Movement */
	int dc = 0, dr = 0;
	if (btn_held(BTN_LEFT)) dc = -1;
	else if (btn_held(BTN_RIGHT)) dc = 1;
	else if (btn_held(BTN_UP)) dr = -1;
	else if (btn_held(BTN_DOWN)) dr = 1;
	if ((dc || dr) && walkable(p->col + dc, p->row + dr, SIDE_PLAYER, run.perks & PERK_AIR_SHOES)) {
		B.move_dc = dc;
		B.move_dr = dr;
		B.act = ACT_MOVE_OUT;
		B.act_timer = 4;
		ent_anim(p, 4);
		return;
	}
	/* Chips */
	if (btn_pressed(BTN_A) && B.hand_n > 0) {
		B.act = ACT_CHIP;
		B.act_timer = 0;
		B.act_id = B.hand[0].id;
		const ChipDef *d = chip_def(B.act_id);
		ChipInfo ci;
		chip_info(B.act_id, &ci);
		B.act_chip_kind = d->kind;
		B.act_param = d->param;
		B.act_power = ci.power + (d->kind == CK_ATKPLUS ? 0 : B.atk_plus);
		if (B.cross == BEAST && d->kind != CK_RECOVER) B.act_power = B.act_power * 3 / 2;
		if (d->kind != CK_ATKPLUS && d->kind != CK_RECOVER) B.atk_plus = 0;
		B.act_elem = ci.element;
		snprintf(B.chip_label, sizeof B.chip_label, "%s", ci.name);
		B.chip_label_timer = 60;
		memmove(B.hand, B.hand + 1, sizeof(FolderChip) * (size_t)(B.hand_n - 1));
		B.hand_n--;
		B.act_timer = -1; /* start_chip sets the timeline */
		B.charge = 0;
		start_chip();
		return;
	}
	/* Buster */
	if (btn_held(BTN_B)) {
		B.charge++;
		Sprite *cs = sprite_get(SPR_GUI, (int)UI.charge_sprite);
		if (B.charge == CHARGE_SHOWN) { audio_sfx(SFX_CHARGE_START); if (cs) anim_play(&B.charge_fx, cs, 0); }
		if (B.charge == charge_time()) { audio_sfx(SFX_CHARGED); if (cs) anim_play(&B.charge_fx, cs, 2); }
		if (B.charge > CHARGE_SHOWN && B.charge_fx.spr) {
			anim_update(&B.charge_fx);
			if (B.charge_fx.done) anim_play(&B.charge_fx, B.charge_fx.spr, B.charge_fx.anim);
		}
	}
	/* The shot leaves 4 frames after B is let go, a charged one 8 (with
	 * MegaMan raising the buster at 7), as recorded */
	if (btn_released(BTN_B)) {
		B.shot_charged = B.charge >= charge_time();
		B.act = ACT_SHOT_WAIT;
		B.act_timer = B.shot_charged ? 8 : 4;
		B.charge = 0;
	}
}

/* ---------------------------------------------------------------- */
/* Chips. start_chip is called every frame of ACT_CHIP; act_timer counts
 * down and the chip fires at its trigger frame. */

static int chip_elapsed;

static void lob(int side, int from_c, int from_r, int tc, int tr, int dmg, int elem, int area, int cat, int idx) {
	Spell *s = spell_new(SP_LOB, side, from_c, from_r);
	if (!s) return;
	s->tcol = tc;
	s->trow = tr;
	s->dmg = dmg;
	s->elem = elem;
	s->param = area;
	s->life = 36;
	s->x = (float)foot_x(from_c);
	s->y = (float)foot_y(from_r) - 20;
	spell_sprite(s, cat, idx, 0, true);
	if (in_field(tc, tr)) B.field[tr][tc].warn = 36;
}

static void chip_fire(void) {
	Ent *p = PLAYER;
	int k = B.act_chip_kind, dmg = B.act_power, el = B.act_elem, c = p->col, r = p->row;
	int hc;
	Ent *e;
	switch (k) {
	case CK_CANNON:
		e = first_in_row(SIDE_PLAYER, c, r, &hc);
		if (e) { damage_ent(e, dmg, el, HF_FLINCH); fx(SPR_HIT, 1, 0, foot_x(hc), foot_y(r) - 16); }
		break;
	case CK_AIRSHOT:
		e = first_in_row(SIDE_PLAYER, c, r, &hc);
		if (e) { strike(SIDE_PLAYER, hc, r, dmg, el, HF_PUSH | HF_FLINCH); fx(SPR_HIT, 5, 0, foot_x(hc), foot_y(r) - 16); }
		audio_sfx(SFX_BUSTER);
		break;
	case CK_SPREADER:
		e = first_in_row(SIDE_PLAYER, c, r, &hc);
		for (int rep = 0; e && rep < (B.act_param ? B.act_param : 1); ++rep) {
			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx) {
					strike(SIDE_PLAYER, hc + dx, r + dy, dmg, el, HF_FLINCH);
					if (in_field(hc + dx, r + dy)) fx(SPR_HIT, 1, 0, foot_x(hc + dx), foot_y(r + dy) - 12);
				}
		}
		audio_sfx(SFX_CANNON);
		break;
	/* Slashes are attack sprite 20: anim 2 Sword, 0 WideSwrd, 1 LongSwrd
	 * (21 is the electric set), placed as the original's */
	case CK_SWORD:
		strike(SIDE_PLAYER, c + 1, r, dmg, el, HF_FLINCH);
		fx(SPR_ATTACK, el == ELEM_ELEC ? 0x15 : 0x14, 2, foot_x(c + 1) + 2, foot_y(r) - 11);
		break;
	case CK_WIDESWORD:
		for (int dx = 1; dx <= (B.act_param ? B.act_param : 1); ++dx) {
			for (int dy = -1; dy <= 1; ++dy) strike(SIDE_PLAYER, c + dx, r + dy, dmg, el, HF_FLINCH);
			fx(SPR_ATTACK, el == ELEM_ELEC ? 0x15 : 0x14, 0, foot_x(c + dx) + 2, foot_y(r) - 11);
		}
		break;
	case CK_LONGSWORD:
		strike(SIDE_PLAYER, c + 1, r, dmg, el, HF_FLINCH);
		strike(SIDE_PLAYER, c + 2, r, dmg, el, HF_FLINCH);
		if (el == ELEM_ELEC) fx(SPR_ATTACK, 0x12, 0, foot_x(c + 1) + 20, foot_y(r) - 14); /* ElecPulse */
		else fx(SPR_ATTACK, 0x14, 1, foot_x(c + 1) + 2, foot_y(r) - 11);
		break;
	case CK_WAVE: {
		Spell *s = spell_new(SP_WAVE, SIDE_PLAYER, c + 1, r);
		if (s) {
			s->dmg = dmg; s->elem = el; s->param = B.act_param ? 6 : 10; s->timer = 0;
			if (B.act_param) { s->flags = HF_CRACK; spell_sprite(s, SPR_EFFECT, 0x26, 0, true); }
			else spell_sprite(s, SPR_EFFECT, 0x34, 0, true);
		}
		audio_sfx(SFX_WAVE);
		break;
	}
	case CK_BOMB: {
		/* The original MiniBomb: a 37-frame arc to the panel three ahead,
		 * the bomb and its ground shadow drawn from attack sprite 2. */
		Spell *s = spell_new(SP_BOMB, SIDE_PLAYER, c, r);
		if (s) {
			s->tcol = c + 3; s->trow = r; s->dmg = dmg; s->elem = el; s->param = B.act_param; s->life = 37;
			s->spr = sprite_get(SPR_ATTACK, 2);
		}
		break;
	}
	case CK_RECOVER:
		p->hp += B.act_param;
		if (p->hp > p->maxhp) p->hp = p->maxhp;
		fx(SPR_HIT, 0x15, 0, foot_x(c), foot_y(r) - 16);
		audio_sfx(SFX_RECOVER);
		break;
	case CK_AREAGRAB:
	case CK_PANELGRAB: {
		/* Steal the enemy's front column (or one panel in MegaMan's row). */
		int front = -1;
		for (int cc = 0; cc < COLS && front < 0; ++cc)
			for (int rr = 0; rr < ROWS; ++rr)
				if (B.field[rr][cc].side == SIDE_ENEMY) { front = cc; break; }
		if (front < 0) break;
		for (int rr = 0; rr < ROWS; ++rr) {
			if (k == CK_PANELGRAB && rr != r) continue;
			Panel *pp = &B.field[rr][front];
			if (pp->side != SIDE_ENEMY) continue;
			fx(SPR_ATTACK, 0x7, 0, foot_x(front), foot_y(rr) - 8);
			Ent *o = ent_at(front, rr);
			if (o) { damage_ent(o, 10, ELEM_NULL, 0); continue; }
			pp->side = SIDE_PLAYER;
			pp->steal = 900;
		}
		audio_sfx(SFX_GRAB);
		break;
	}
	case CK_BARRIER:
		p->barrier = B.act_param;
		audio_sfx(SFX_GUARD);
		break;
	case CK_INVIS:
		p->invis = B.act_param;
		audio_sfx(SFX_GUARD);
		break;
	case CK_FLAME:
		for (int rr = (B.act_param == 3 ? 0 : r); rr <= (B.act_param == 3 ? ROWS - 1 : r); ++rr)
			for (int i = 1; i <= 3; ++i) {
				strike(SIDE_PLAYER, c + i, rr, dmg, ELEM_FIRE, HF_FLINCH);
				if (in_field(c + i, rr)) fx(SPR_EFFECT, 0x02, 0, foot_x(c + i), foot_y(rr));
			}
		audio_sfx(SFX_FLAME);
		break;
	case CK_THUNDER: {
		Spell *s = spell_new(SP_THUNDER, SIDE_PLAYER, c + 1, r);
		if (s) { s->dmg = dmg; s->elem = ELEM_ELEC; s->life = 300; s->flags = HF_STUN; spell_sprite(s, SPR_ATTACK, 0x13, 0, true); }
		audio_sfx(SFX_THUNDER);
		break;
	}
	case CK_TORNADO: {
		Spell *s = spell_new(SP_TORNADO, SIDE_PLAYER, c + 2, r);
		if (s) { s->dmg = dmg; s->elem = el; s->param = B.act_param; s->life = 8 * 6; spell_sprite(s, SPR_ATTACK, 0x17, 0, true); }
		audio_sfx(SFX_WIND);
		break;
	}
	case CK_CROSSGUN:
		e = first_in_row(SIDE_PLAYER, c, r, &hc);
		if (e) {
			static const int d[5][2] = { { 0, 0 }, { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 } };
			for (int i = 0; i < 5; ++i) {
				strike(SIDE_PLAYER, hc + d[i][0], r + d[i][1], dmg, el, HF_FLINCH);
				if (in_field(hc + d[i][0], r + d[i][1])) fx(SPR_HIT, 1, 0, foot_x(hc + d[i][0]), foot_y(r + d[i][1]) - 12);
			}
		}
		audio_sfx(SFX_CANNON);
		break;
	case CK_BOOMER: {
		Spell *s = spell_new(SP_BOOMER, SIDE_PLAYER, 0, 2);
		if (s) { s->dmg = dmg; s->elem = ELEM_WOOD; s->param = 0; spell_sprite(s, SPR_ATTACK, 0x2F, 0, true); s->x = (float)foot_x(0); s->y = (float)foot_y(2); }
		audio_sfx(SFX_THROW);
		break;
	}
	case CK_GEDDON:
		for (int rr = 0; rr < ROWS; ++rr)
			for (int cc = 0; cc < COLS; ++cc)
				if (B.field[rr][cc].side == SIDE_ENEMY && B.field[rr][cc].type != PT_BROKEN)
					B.field[rr][cc].type = ent_at(cc, rr) ? PT_CRACKED : PT_BROKEN, B.field[rr][cc].restore = 900;
		B.shake = 30;
		audio_sfx(SFX_EXPLODE);
		break;
	case CK_HOLYPANEL:
		B.field[r][c].type = PT_HOLY;
		audio_sfx(SFX_RECOVER);
		break;
	case CK_METEORS:
		for (int i = 0; i < B.act_param; ++i) {
			Spell *s = spell_new(SP_METEOR, SIDE_PLAYER, 0, 0);
			if (!s) break;
			s->timer = i * 12;
			s->dmg = dmg;
			s->elem = ELEM_FIRE;
			s->life = 30;
			spell_sprite(s, SPR_ATTACK, 0x31, 0, true);
		}
		audio_sfx(SFX_EXPLODE);
		break;
	case CK_ATKPLUS:
		B.atk_plus += B.act_param;
		audio_sfx(SFX_RECOVER);
		break;
	case CK_ROCKCUBE:
		if (walkable(c + 1, r, SIDE_PLAYER, false)) {
			Ent *rock = spawn(K_ROCK, SIDE_PLAYER, c + 1, r);
			if (rock) { rock->hp = rock->maxhp = 200; rock->spr = sprite_get(SPR_EFFECT, 8); ent_anim(rock, 0); }
		}
		break;
	case CK_NAVI:
		/* Navi strike: every enemy takes the chip's power. */
		for (int i = 1; i < MAX_ENTS; ++i) {
			Ent *o = &B.ent[i];
			if (o->on && !o->dying && o->side == SIDE_ENEMY) {
				damage_ent(o, dmg, el, HF_FLINCH | HF_BREAK);
				fx(SPR_HIT, 0, 0, foot_x(o->col), foot_y(o->row) - 12);
			}
		}
		B.shake = 20;
		break;
	default:
		break;
	}
}

static void vulcan_tick(void) {
	Ent *p = PLAYER;
	int hc;
	Ent *e = first_in_row(SIDE_PLAYER, p->col, p->row, &hc);
	if (e) {
		damage_ent(e, B.act_power, B.act_elem, HF_NOINV);
		fx(SPR_HIT, 5, 0, foot_x(hc), foot_y(p->row) - 16);
		/* The last shot splashes the panel behind. */
		if (B.act_param == 1) strike(SIDE_PLAYER, hc + 1, p->row, B.act_power, B.act_elem, 0);
	}
	audio_sfx(SFX_BUSTER);
}

static void start_chip(void) {
	Ent *p = PLAYER;
	if (B.act_timer == -1) {
		/* First frame: choose the animation and timeline. */
		int anim = 0, total = 20;
		chip_elapsed = 0;
		switch (B.act_chip_kind) {
		case CK_CANNON: case CK_SPREADER: case CK_CROSSGUN: anim = 8; total = 30; break;
		case CK_AIRSHOT: anim = 9; total = 14; break;
		case CK_VULCAN: anim = 11; total = 8 * B.act_param + 8; break;
		case CK_SWORD: case CK_WIDESWORD: case CK_LONGSWORD: anim = 6; total = 30; break;
		case CK_BOMB: anim = 5; total = 24; break;
		case CK_BOOMER: anim = 5; total = 22; break;
		case CK_WAVE: case CK_FLAME: case CK_THUNDER: case CK_TORNADO: anim = 12; total = 22; break;
		case CK_NAVI: anim = 0; total = 70; break;
		default: anim = 0; total = 12; break;
		}
		ent_anim(p, anim);
		B.act_timer = total;
		return;
	}
	++chip_elapsed;
	int k = B.act_chip_kind;
	if (k == CK_VULCAN) {
		if (chip_elapsed % 8 == 4) {
			int left = B.act_param - chip_elapsed / 8;
			int saved = B.act_param;
			B.act_param = left <= 1 ? 1 : 0;
			vulcan_tick();
			B.act_param = saved;
		}
		if (chip_elapsed % 8 == 0 && chip_elapsed < B.act_param * 8) ent_anim(p, 11);
		return;
	}
	int trigger = 8;
	if (k == CK_SWORD || k == CK_WIDESWORD || k == CK_LONGSWORD) {
		/* the draw sound at 5, the slash at 14 (recorded WideSwrd) */
		if (chip_elapsed == 5) audio_sfx(SFX_SWORD);
		trigger = 14;
	}
	if (k == CK_CANNON || k == CK_SPREADER || k == CK_CROSSGUN) {
		/* attack sprite 1: the barrel, its flash and blast; the shot lands
		 * two frames after the big flash (17 frames in) */
		if (chip_elapsed == 1) {
			Spell *s = spell_new(SP_FX, SIDE_PLAYER, p->col, p->row);
			if (s) { spell_sprite(s, SPR_ATTACK, 1, B.act_power >= 60 ? 1 : 0, false); s->life = 40; s->x = (float)(foot_x(p->col) + 24); s->y = (float)(foot_y(p->row) - 25); }
		}
		if (chip_elapsed == 15) audio_sfx(SFX_CANNON);
		trigger = 17;
	}
	if (k == CK_BOMB) {
		/* the bomb sits in MegaMan's hand for 9 frames, then flies from 13 */
		if (chip_elapsed == 2) {
			Spell *s = spell_new(SP_FX, SIDE_PLAYER, p->col, p->row);
			if (s) { spell_sprite(s, SPR_ATTACK, 2, 0, false); s->life = 9; s->x = (float)foot_x(p->col); s->y = (float)foot_y(p->row); }
			audio_sfx(SFX_THROW);
		}
		trigger = 13;
	}
	if (k == CK_NAVI) {
		if (chip_elapsed == 1) {
			const ChipDef *d = chip_def(B.act_id);
			Spell *s = spell_new(SP_FX, SIDE_PLAYER, p->col, p->row);
			if (s) {
				spell_sprite(s, SPR_NAVI, d->param, 3, false);
				s->x = (float)foot_x(p->col + 1 < COLS ? p->col + 1 : p->col);
				s->life = 60;
			}
		}
		trigger = 40;
	}
	if (chip_elapsed == trigger) chip_fire();
}

/* ---------------------------------------------------------------- */
/* Enemy AI */

static Ent *player_target(void) { return PLAYER->on && !PLAYER->dying ? PLAYER : NULL; }

static bool step_toward_row(Ent *e, int row) {
	int dr = row > e->row ? 1 : row < e->row ? -1 : 0;
	if (!dr) return false;
	if (walkable(e->col, e->row + dr, e->side, false)) {
		e->row += dr;
		return true;
	}
	return false;
}

static void random_move(Ent *e) {
	for (int tries = 0; tries < 12; ++tries) {
		int c = rng_range(0, COLS - 1), r = rng_range(0, ROWS - 1);
		if (walkable(c, r, e->side, false)) { e->col = c; e->row = r; return; }
	}
}

static Spell *enemy_projectile(Ent *e, int speed_px) {
	Spell *s = spell_new(SP_PROJ, e->side, e->col, e->row);
	if (!s) return NULL;
	s->dmg = e->atk;
	s->elem = e->elem;
	s->vx = (float)-speed_px;
	s->x = (float)foot_x(e->col) - 12;
	s->y = (float)foot_y(e->row) - 12;
	s->flags = HF_FLINCH;
	s->owner = (int)(e - B.ent);
	s->flip = false;
	return s;
}

static int speed_scale(Ent *e) {
	/* Higher versions act sooner. */
	return 100 - e->ver * 12;
}

/* Mettaurs take turns (ForMettaur_8109EF4 in the disassembly): only the one
 * holding the turn acts. It waits 30 frames, steps a row at a time toward
 * MegaMan (a step every 30, 24, 18, 12, 18, 12 frames by version), then
 * raises its pickaxe (sound 0x183) and sends the shockwave 50 frames later;
 * the turn then passes on. From V2 on the others follow MegaMan's row. */
static bool mettaur_turn(const Ent *e) {
	for (int i = 1; i < MAX_ENTS; ++i) {
		const Ent *o = &B.ent[i];
		if (o != e && o->on && !o->dying && o->vd && o->vd->ai == AI_METTAUR && o->side == e->side && o->ticket < e->ticket) return false;
	}
	return true;
}

static void ai_mettaur(Ent *e, Ent *pl, int sp) {
	static const int step_delay[6] = { 30, 24, 18, 12, 18, 12 };
	const VirusDef *d = e->vd;
	int v = e->ver < 0 ? 0 : e->ver > 5 ? 5 : e->ver;
	switch (e->state) {
	case 0: /* waiting for the turn */
		if (mettaur_turn(e)) { e->state = 1; e->timer = 30; break; }
		if (e->ver > 0 && e->row != pl->row) { e->state = 4; e->timer = step_delay[v] * sp / 100; }
		break;
	case 1: /* its turn: a pause, then row by row toward MegaMan */
		if (--e->timer > 0) break;
		if (e->row != pl->row) {
			if (step_toward_row(e, pl->row)) ent_anim(e, d->anim_move);
			e->timer = step_delay[v] * sp / 100;
			break;
		}
		e->state = 2;
		e->timer = 50 * sp / 100;
		ent_anim(e, d->anim_attack);
		audio_sfx(SFX_PICKAXE);
		break;
	case 2: /* pickaxe raised */
		if (--e->timer > 0) break;
		{
			/* The original's shockwave (effect sprite 3): 22 frames a panel
			 * for a Mettaur, with faster animations for higher versions. */
			static const int step[3] = { 22, 15, 10 };
			int w = e->ver > 2 ? 2 : e->ver;
			Spell *s = spell_new(SP_WAVE, e->side, e->col - 1, e->row);
			if (s) { s->dmg = e->atk; s->elem = e->elem; s->param = step[w]; s->flags = HF_FLINCH; spell_sprite(s, SPR_EFFECT, 3, w, true); }
			e->count = s ? (int)(s - B.sp) : -1;
		}
		e->state = 3;
		e->timer = 20;
		break;
	case 3: /* the turn passes on once its shockwave has run out */
		if (e->timer > 0 && --e->timer == 0) ent_anim(e, d->anim_idle);
		if (e->count >= 0 && B.sp[e->count].on && B.sp[e->count].type == SP_WAVE) break;
		if (e->timer > 0) break;
		e->ticket = ++B.tickets;
		e->state = 0;
		break;
	case 4: /* V2 and up without the turn: follow MegaMan's row */
		if (--e->timer > 0) break;
		if (step_toward_row(e, pl->row)) ent_anim(e, d->anim_move);
		e->state = 0;
		break;
	}
}

static void ai_virus(Ent *e) {
	const VirusDef *d = e->vd;
	Ent *pl = player_target();
	if (!d || !pl) return;
	if (e->stun > 0) { --e->stun; return; }
	int sp = speed_scale(e);
	switch (d->ai) {
	case AI_METTAUR:
		ai_mettaur(e, pl, sp);
		break;
	case AI_SHOOTER:
		/* state 0: wander toward the row; 1: wind up; 2: recover */
		if (e->state == 0) {
			if (--e->timer > 0) break;
			if (e->row != pl->row) {
				if (step_toward_row(e, pl->row)) ent_anim(e, d->anim_move);
				e->timer = 40 * sp / 100;
			} else {
				e->state = 1;
				e->timer = 24;
				ent_anim(e, d->anim_attack);
			}
		} else if (e->state == 1) {
			if (--e->timer > 0) break;
			Spell *s = enemy_projectile(e, 3 + e->ver);
			if (s) spell_sprite(s, d->fx_cat, d->fx_idx, d->fx_anim, true);
			audio_sfx(SFX_BUSTER);
			e->state = 2;
			e->timer = 50 * sp / 100;
		} else {
			if (--e->timer > 0) break;
			ent_anim(e, d->anim_idle);
			e->state = 0;
			e->timer = rng_range(40, 90) * sp / 100;
		}
		break;
	case AI_SWORDY:
	case AI_PUNCHER:
		if (e->state == 0) {
			if (--e->timer > 0) break;
			/* Warp next to MegaMan if the panel is ours. */
			int tc = pl->col + 1, tr = pl->row;
			if (walkable(tc, tr, e->side, false)) {
				e->col = tc; e->row = tr;
				e->state = 1;
				e->timer = 22 * sp / 100 + 8;
				ent_anim(e, d->anim_idle);
				audio_sfx(SFX_WARP);
			} else {
				if (step_toward_row(e, pl->row)) ent_anim(e, d->anim_idle);
				e->timer = 30;
			}
		} else if (e->state == 1) {
			if (--e->timer == 8) ent_anim(e, d->anim_attack);
			if (e->timer > 0) break;
			if (d->ai == AI_SWORDY) {
				int span = e->ver >= 1 ? 1 : 0;
				for (int dy = -span; dy <= span; ++dy) strike(e->side, e->col - 1, e->row + dy, e->atk, e->elem, HF_FLINCH);
				Spell *s = spell_new(SP_FX, e->side, e->col - 1, e->row);
				if (s) { spell_sprite(s, d->fx_cat, d->fx_idx, d->fx_anim, false); s->life = 30; s->x += 10; s->y -= 14; }
				audio_sfx(SFX_SWORD);
			} else {
				strike(e->side, e->col - 1, e->row, e->atk, ELEM_FIRE, HF_FLINCH);
				fx(SPR_HIT, 1, 0, foot_x(e->col - 1), foot_y(e->row) - 16);
				audio_sfx(SFX_HIT);
			}
			e->state = 2;
			e->timer = 30;
		} else {
			if (--e->timer > 0) break;
			if (walkable(e->home_col, e->home_row, e->side, false)) { e->col = e->home_col; e->row = e->home_row; }
			else random_move(e);
			ent_anim(e, d->anim_idle);
			e->state = 0;
			e->timer = rng_range(60, 110) * sp / 100;
		}
		break;
	case AI_BEAM:
		if (e->state == 0) {
			if (--e->timer > 0) break;
			if (e->row == pl->row) {
				e->state = 1;
				e->timer = 36 * sp / 100;
				ent_anim(e, d->anim_attack);
				for (int c = 0; c < e->col; ++c) B.field[e->row][c].warn = e->timer;
			} else {
				step_toward_row(e, pl->row);
				e->timer = 45;
			}
		} else if (e->state == 1) {
			if (--e->timer > 0) break;
			Spell *s = spell_new(SP_BEAM, e->side, e->col, e->row);
			if (s) { s->life = 16; s->tint = rgba(255, 255, 120, 255); }
			for (int c = e->col - 1; c >= 0; --c) strike(e->side, c, e->row, e->atk, ELEM_ELEC, HF_FLINCH | HF_STUN);
			audio_sfx(SFX_THUNDER);
			e->state = 2;
			e->timer = 70;
		} else {
			if (--e->timer > 0) break;
			ent_anim(e, d->anim_idle);
			e->state = 0;
			e->timer = rng_range(50, 100) * sp / 100;
		}
		break;
	case AI_LOBBER:
		if (--e->timer > 0) break;
		if (e->state == 0) {
			ent_anim(e, d->anim_attack);
			e->state = 1;
			e->timer = 20;
		} else {
			lob(e->side, e->col, e->row, pl->col, pl->row, e->atk, e->elem, 0, d->fx_cat, d->fx_idx);
			audio_sfx(SFX_THROW);
			ent_anim(e, d->anim_idle);
			e->state = 0;
			e->timer = rng_range(90, 140) * sp / 100;
		}
		break;
	case AI_GUNNER:
		if (e->state == 0) {
			if (--e->timer > 0) break;
			e->state = 1;
			e->tcol = 0;
			e->trow = pl->row;
			e->count = 0;
			e->timer = 6;
			ent_anim(e, 1);
		} else if (e->state == 1) {
			/* Crosshair sweeps along MegaMan's row, re-aiming each step. */
			if (--e->timer > 0) break;
			e->trow = pl->row;
			if (e->tcol == pl->col || ++e->count > 12) {
				e->state = 2;
				e->timer = 10;
				e->count = 0;
				ent_anim(e, d->anim_attack);
			} else {
				e->tcol += e->tcol < pl->col ? 1 : -1;
				e->timer = 12 * sp / 100;
			}
			B.field[e->trow][e->tcol].warn = 8;
		} else if (e->state == 2) {
			B.field[e->trow][e->tcol].warn = 2;
			if (--e->timer > 0) break;
			strike(e->side, e->tcol, e->trow, e->atk, ELEM_NULL, e->count == 2 ? HF_FLINCH : HF_NOINV);
			fx(SPR_HIT, 5, 0, foot_x(e->tcol), foot_y(e->trow) - 14);
			audio_sfx(SFX_BUSTER);
			if (++e->count >= 3) { e->state = 3; e->timer = 60; }
			else e->timer = 8;
		} else {
			if (--e->timer > 0) break;
			ent_anim(e, d->anim_idle);
			e->state = 0;
			e->timer = rng_range(60, 110) * sp / 100;
		}
		break;
	case AI_ROLLER:
		if (e->state == 0) {
			if (--e->timer > 0) break;
			if (e->row != pl->row) { step_toward_row(e, pl->row); e->timer = 40; break; }
			e->state = 1;
			e->timer = 20;
			ent_anim(e, d->anim_attack);
		} else if (e->state == 1) {
			if (--e->timer > 0) break;
			e->state = 2;
			e->ox = 0;
			e->count = 0;
		} else if (e->state == 2) {
			/* Roll left across the whole row, then reappear at home. */
			e->ox -= 4 + e->ver;
			int col = e->col + (e->ox - 20) / PANEL_W;
			if (col != e->count + 100) {
				e->count = col - 100;
				Ent *hit = ent_at(col, e->row);
				if (hit && hit->side != e->side) damage_ent(hit, e->atk, e->elem, HF_FLINCH);
			}
			if (foot_x(e->col) + e->ox < field_x() - 30) {
				e->ox = 0;
				e->state = 3;
				e->timer = 40;
				if (!walkable(e->col, e->row, e->side, false) && ent_at(e->col, e->row) != e) random_move(e);
				ent_anim(e, d->anim_idle);
			}
		} else {
			if (--e->timer > 0) break;
			e->state = 0;
			e->timer = rng_range(70, 120) * sp / 100;
		}
		break;
	default:
		break;
	}
}

static void navi_attack_begin(Ent *e) {
	const NaviDef *d = e->nd;
	int pick = rng_range(0, 2);
	if (d->attack_kind[pick] == NA_SHOT || d->attack_kind[pick] == NA_ROWBLAST || d->attack_kind[pick] == NA_WAVE || d->attack_kind[pick] == NA_DASH) {
		/* These need MegaMan's row. */
		Ent *pl = player_target();
		if (pl && e->row != pl->row) {
			int c = e->col;
			if (walkable(c, pl->row, e->side, false)) e->row = pl->row;
			else {
				for (int cc = 3; cc < COLS; ++cc)
					if (walkable(cc, pl->row, e->side, false)) { e->col = cc; e->row = pl->row; break; }
			}
			ent_anim(e, 3);
		}
	}
	e->sub = pick;
	e->state = 2;
	e->timer = 24;
	ent_anim(e, d->attack_anim[pick]);
	Ent *pl = player_target();
	if (pl) { e->tcol = pl->col; e->trow = pl->row; }
	if (d->attack_kind[pick] == NA_COLUMN && pl)
		for (int r = 0; r < ROWS; ++r) B.field[r][pl->col].warn = 24;
	if (d->attack_kind[pick] == NA_TARGET && pl) B.field[pl->row][pl->col].warn = 24;
}

static void navi_attack_fire(Ent *e) {
	const NaviDef *d = e->nd;
	int k = d->attack_kind[e->sub];
	int cat = d->fx_cat[e->sub], idx = d->fx_idx[e->sub], an = d->fx_anim[e->sub];
	if (cat == 2) idx = e->fam; /* navi's own sprite effects */
	switch (k) {
	case NA_SHOT: {
		Spell *s = enemy_projectile(e, 4);
		if (s) { spell_sprite(s, cat, idx, an, true); s->dmg = e->atk; }
		audio_sfx(SFX_CANNON);
		break;
	}
	case NA_TARGET:
		lob(e->side, e->col, e->row, e->tcol, e->trow, e->atk, e->elem, 1, cat >= 0 ? cat : SPR_ATTACK, idx >= 0 ? idx : 0x24);
		audio_sfx(SFX_THROW);
		break;
	case NA_COLUMN:
		for (int r = 0; r < ROWS; ++r) {
			strike(e->side, e->tcol, r, e->atk, e->elem, HF_FLINCH);
			if (cat >= 0) {
				Spell *s = spell_new(SP_FX, e->side, e->tcol, r);
				if (s) { spell_sprite(s, cat, idx, an, false); s->life = 30; }
			}
		}
		audio_sfx(e->elem == ELEM_ELEC ? SFX_THUNDER : SFX_SWORD);
		break;
	case NA_WAVE: {
		Spell *s = spell_new(SP_WAVE, e->side, e->col - 1, e->row);
		if (s) { s->dmg = e->atk; s->elem = e->elem; s->param = 8; s->flags = HF_FLINCH | HF_CRACK; spell_sprite(s, SPR_EFFECT, 0x26, 0, true); }
		audio_sfx(SFX_WAVE);
		break;
	}
	case NA_ROWBLAST:
		for (int c = e->col - 1; c >= e->col - 3 && c >= 0; --c) {
			strike(e->side, c, e->row, e->atk, e->elem, HF_FLINCH);
			if (cat >= 0) {
				Spell *s = spell_new(SP_FX, e->side, c, e->row);
				if (s) { spell_sprite(s, cat, idx, an, false); s->life = 30; s->flip = true; }
			}
		}
		audio_sfx(SFX_FLAME);
		break;
	case NA_DASH:
		e->state = 4;
		e->ox = 0;
		e->count = -1;
		audio_sfx(SFX_DASH);
		return;
	default:
		break;
	}
	e->state = 3;
	e->timer = 30;
}

static void ai_navi(Ent *e) {
	Ent *pl = player_target();
	if (!pl || !e->nd) return;
	if (e->stun > 0) { --e->stun; return; }
	if (e->invuln > 0) --e->invuln;
	if (e->flinch > 0) {
		if (--e->flinch == 19) ent_anim(e, e->nd->anim_hit);
		if (e->flinch == 0) { ent_anim(e, 0); e->state = 0; e->timer = 20; e->ox = 0; }
		return;
	}
	bool angry = e->hp * 2 < e->maxhp;
	int pace = angry ? 70 : 100;
	pace -= e->ver * 10;
	switch (e->state) {
	case 0: /* idle, then vanish */
		if (--e->timer > 0) break;
		ent_anim(e, 4);
		e->state = 1;
		e->timer = 5;
		break;
	case 1: /* reappear elsewhere; attack every few moves */
		if (--e->timer > 0) break;
		random_move(e);
		ent_anim(e, 3);
		if (++e->count >= (angry ? 2 : 3)) {
			e->count = 0;
			navi_attack_begin(e);
		} else {
			e->state = 0;
			e->timer = rng_range(30, 60) * pace / 100;
		}
		break;
	case 2: /* wind-up */
		if (--e->timer > 0) break;
		navi_attack_fire(e);
		break;
	case 3: /* recover */
		if (--e->timer > 0) break;
		ent_anim(e, 0);
		e->state = 0;
		e->timer = rng_range(30, 60) * pace / 100;
		break;
	case 4: { /* dash left along the row */
		e->ox -= 6;
		int col = e->col + (e->ox - 20) / PANEL_W;
		if (col != e->count) {
			e->count = col;
			Ent *hit = ent_at(col, e->row);
			if (hit && hit->side != e->side) damage_ent(hit, e->atk, e->elem, HF_FLINCH);
		}
		if (foot_x(e->col) + e->ox < field_x() - 40) {
			e->ox = 0;
			e->count = 0;
			random_move(e);
			ent_anim(e, 3);
			e->state = 3;
			e->timer = 30;
		}
		break;
	}
	default:
		e->state = 0;
		break;
	}
}

/* ---------------------------------------------------------------- */
/* Spell updates */

static void spell_hit_row_entity(Spell *s, int c, int r) {
	Ent *e = ent_at(c, r);
	if (!e || (e->side == s->side && e->kind != K_ROCK)) return;
	int idx = (int)(e - B.ent);
	if (s->hits & (1 << idx)) return;
	s->hits |= (uint16_t)(1 << idx);
	damage_ent(e, s->dmg, s->elem, s->flags);
}

static void update_spell(Spell *s) {
	if (s->type == SP_FX && s->timer < 0) { ++s->timer; return; }
	if (s->spr) {
		anim_update(&s->anim);
		if (s->anim.done && s->loop) anim_play(&s->anim, s->spr, s->anim.anim);
	}
	switch (s->type) {
	case SP_FX:
		if ((s->spr && s->anim.done) || --s->life <= 0) s->on = false;
		break;
	case SP_WAVE: {
		int dir = s->side == SIDE_PLAYER ? 1 : -1;
		if (s->timer++ % s->param == 0) {
			if (s->timer > 1) s->col += dir;
			if (!in_field(s->col, s->row) || B.field[s->row][s->col].type == PT_BROKEN) { s->on = false; break; }
			B.field[s->row][s->col].warn = s->param; /* the panel under the wave turns yellow */
			if (s->spr && s->timer > 1) anim_play(&s->anim, s->spr, s->anim.anim);
			if (s->side == SIDE_ENEMY) audio_sfx(SFX_SHOCKWAVE);
			s->x = (float)foot_x(s->col);
			/* a panel where the wave hits someone does not stay lit */
			if (strike(s->side, s->col, s->row, s->dmg, s->elem, s->flags | HF_NOINV)) B.field[s->row][s->col].warn = 0;
			Ent *e = ent_at(s->col, s->row);
			if (e && e->kind == K_ROCK) { s->on = false; break; }
		}
		break;
	}
	case SP_PROJ: {
		s->x += s->vx;
		int c = (int)((s->x - field_x()) / PANEL_W);
		if (s->x < field_x() - 16 || s->x > field_x() + COLS * PANEL_W + 16) { s->on = false; break; }
		if (in_field(c, s->row)) {
			Ent *e = ent_at(c, s->row);
			if (e && (e->side != s->side || e->kind == K_ROCK)) {
				damage_ent(e, s->dmg, s->elem, s->flags);
				fx(SPR_HIT, 5, 0, (int)s->x, (int)s->y);
				s->on = false;
			}
		}
		break;
	}
	case SP_LOB: {
		++s->timer;
		float t = (float)s->timer / (float)s->life;
		float sx = (float)foot_x(s->col), sy = (float)foot_y(s->row) - 20;
		float tx = (float)foot_x(s->tcol), ty = (float)foot_y(s->trow) - 4;
		s->x = sx + (tx - sx) * t;
		s->y = sy + (ty - sy) * t - sinf(t * 3.14159f) * 40.0f;
		if (s->timer >= s->life) {
			static const int cross[5][2] = { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
			int n = s->param == 0 ? 1 : 5;
			for (int i = 0; i < n; ++i) {
				int c = s->tcol + cross[i][0], r = s->trow + cross[i][1];
				if (!in_field(c, r)) continue;
				strike(s->side, c, r, s->dmg, s->elem, HF_FLINCH);
				fx(SPR_HIT, 0, 0, foot_x(c), foot_y(r) - 8);
			}
			if (s->param == 2)
				for (int dy = -1; dy <= 1; dy += 2)
					for (int dx = -1; dx <= 1; dx += 2) {
						strike(s->side, s->tcol + dx, s->trow + dy, s->dmg, s->elem, HF_FLINCH);
						if (in_field(s->tcol + dx, s->trow + dy)) fx(SPR_HIT, 0, 0, foot_x(s->tcol + dx), foot_y(s->trow + dy) - 8);
					}
			audio_sfx(SFX_EXPLODE);
			s->on = false;
		}
		break;
	}
	case SP_BEAM:
		if (--s->life <= 0) s->on = false;
		break;
	case SP_BOMB:
		if (++s->timer >= s->life) {
			/* lands: the blast and the game's explosion on the panel */
			Ent *hit = strike(s->side, s->tcol, s->trow, s->dmg, s->elem, HF_FLINCH);
			(void)hit;
			if (in_field(s->tcol, s->trow)) {
				Spell *x = spell_new(SP_FX, 0, 0, 0);
				if (x) {
					spell_sprite(x, SPR_GUI, (int)UI.delete_sprite, 0, false);
					SDL_Rect b = x->spr ? sprite_frame_bounds(x->spr, 0, 0, 0) : (SDL_Rect){ 0, 0, 0, 0 };
					x->x = (float)(foot_x(s->tcol) - (b.x + b.w / 2));
					x->y = (float)(foot_y(s->trow) - 18 - (b.y + b.h / 2));
					x->life = 40;
				}
			}
			audio_sfx(SFX_BOMB);
			s->on = false;
		}
		break;
	case SP_THUNDER: {
		/* Drifts forward, bending toward the nearest enemy row. */
		if (--s->life <= 0) { s->on = false; break; }
		s->x += 1.2f;
		Ent *best = NULL;
		for (int i = 1; i < MAX_ENTS; ++i) {
			Ent *e = &B.ent[i];
			if (e->on && !e->dying && e->side != s->side && foot_x(e->col) > s->x) { best = e; break; }
		}
		if (best) {
			float ty = (float)foot_y(best->row) - 12;
			s->y += (ty > s->y ? 0.6f : ty < s->y ? -0.6f : 0);
		}
		int c = (int)((s->x - field_x()) / PANEL_W), r = (int)((s->y + 12 - field_y()) / PANEL_H);
		if (!in_field(c, r)) { if (s->x > field_x() + COLS * PANEL_W) s->on = false; break; }
		Ent *e = ent_at(c, r);
		if (e && e->side != s->side) {
			damage_ent(e, s->dmg, ELEM_ELEC, HF_STUN | HF_FLINCH);
			fx(SPR_HIT, 7, 0, (int)s->x, (int)s->y);
			s->on = false;
		}
		break;
	}
	case SP_TORNADO:
		if (s->timer++ % 6 == 0) {
			strike(s->side, s->col, s->row, s->dmg, s->elem, HF_NOINV);
			if (--s->param <= 0) s->on = false;
		}
		break;
	case SP_BOOMER: {
		/* Runs along the bottom row, up the far edge and back along the top. */
		float spd = 4.0f;
		float right = (float)foot_x(COLS - 1), left = (float)foot_x(0) - 30;
		if (s->param == 0) { s->x += spd; if (s->x >= right) { s->x = right; s->param = 1; } }
		else if (s->param == 1) { s->y -= spd; if (s->y <= foot_y(0)) { s->y = (float)foot_y(0); s->param = 2; } }
		else { s->x -= spd; if (s->x < left) s->on = false; }
		int c = (int)((s->x - field_x()) / PANEL_W), r = (int)((s->y - field_y()) / PANEL_H);
		if (in_field(c, r)) spell_hit_row_entity(s, c, r);
		break;
	}
	case SP_METEOR:
		if (s->timer > 0) { --s->timer; if (s->timer == 0) {
			/* pick a random enemy panel */
			for (int tries = 0; tries < 20; ++tries) {
				int c = rng_range(3, COLS - 1), r = rng_range(0, ROWS - 1);
				if (B.field[r][c].side == SIDE_ENEMY) { s->tcol = c; s->trow = r; break; }
			}
			s->life = 20;
			B.field[s->trow][s->tcol].warn = 20;
		} break; }
		s->x = (float)foot_x(s->tcol) + (float)s->life * 3;
		s->y = (float)foot_y(s->trow) - (float)s->life * 6;
		if (--s->life <= 0) {
			strike(s->side, s->tcol, s->trow, s->dmg, ELEM_FIRE, HF_FLINCH | HF_CRACK);
			fx(SPR_HIT, 0, 0, foot_x(s->tcol), foot_y(s->trow) - 8);
			s->on = false;
		}
		break;
	default:
		s->on = false;
		break;
	}
}

/* ---------------------------------------------------------------- */
/* Results */

static int busting_level(void) {
	int lvl = 10;
	int sec = B.frames / 60;
	if (sec > 5) lvl--;
	if (sec > 12) lvl--;
	if (sec > 20) lvl--;
	if (sec > 35) lvl--;
	lvl -= B.hits_taken;
	if (lvl < 1) lvl = 1;
	return lvl;
}

static void finish(int outcome) {
	BattleResult r = { outcome, busting_level(), B.frames, PLAYER->hp, false, { -1, 0, 0 } };
	if (outcome == BATTLE_WIN && B.reward_n > 0) { r.took_reward = true; r.reward = B.reward[B.reward_sel]; r.busting = B.result_busting; }
	run.hp = PLAYER->hp > 0 ? PLAYER->hp : 0;
	B.state = BS_DONE;
	if (B.done) B.done(&r);
}

/* ---------------------------------------------------------------- */
/* Scene */

static void update_panels(void) {
	for (int r = 0; r < ROWS; ++r)
		for (int c = 0; c < COLS; ++c) {
			Panel *p = &B.field[r][c];
			if (p->warn > 0) --p->warn;
			if (p->type == PT_BROKEN && p->restore > 0 && --p->restore == 0) p->type = PT_NORMAL;
			if (p->steal > 0 && --p->steal == 0 && !ent_at(c, r)) p->side = p->home;
		}
}

static void update_custom_screen(void) {
	++B.state_timer;
	if (B.emblem_t < 100) ++B.emblem_t;
	if (B.slide < 10) {
		/* Slides in 12 pixels a frame while the field sinks 1.5 */
		++B.slide;
		B.shift = (B.slide * 3 + 1) / 2;
		return;
	}
	/* R shows the chip description in a text box, a line a frame from the
	 * frame after the press; R, A or B folds it away over four frames and
	 * the cursor returns two frames later. */
	if (B.desc_close > 0) {
		if (++B.desc_close > 7) B.desc_close = B.desc_t = 0;
		return;
	}
	if (B.desc_t > 0) {
		if (B.desc_t < 4) ++B.desc_t;
		if (btn_pressed(BTN_R) || btn_pressed(BTN_A) || btn_pressed(BTN_B)) {
			B.desc_close = 1;
			audio_sfx(SFX_DESC_CLOSE);
		}
		return;
	}
	if (btn_pressed(BTN_R) && B.cursor < B.offer_n) {
		B.desc_t = 1;
		audio_sfx(SFX_DESC_OPEN);
		return;
	}
	if (btn_repeat(BTN_LEFT)) {
		if (B.cursor == OFFER_MAX) B.cursor = B.offer_n > 0 ? (B.offer_n - 1 < 4 ? B.offer_n - 1 : 4) : 0;
		else if (B.cursor % 5 > 0) --B.cursor;
		audio_sfx(SFX_CHIP_CURSOR);
	} else if (btn_repeat(BTN_RIGHT)) {
		if (B.cursor != OFFER_MAX) {
			if (B.cursor % 5 == 4 || B.cursor + 1 >= B.offer_n) B.cursor = OFFER_MAX;
			else ++B.cursor;
		}
		audio_sfx(SFX_CHIP_CURSOR);
	} else if (btn_repeat(BTN_DOWN)) {
		if (B.cursor != OFFER_MAX && B.cursor + 5 < B.offer_n) B.cursor += 5;
		audio_sfx(SFX_CHIP_CURSOR);
	} else if (btn_repeat(BTN_UP)) {
		if (B.cursor != OFFER_MAX && B.cursor >= 5) B.cursor -= 5;
		audio_sfx(SFX_CHIP_CURSOR);
	}
	if (btn_pressed(BTN_START) && B.cursor != OFFER_MAX) { B.cursor = OFFER_MAX; audio_sfx(SFX_CHIP_CURSOR); }
	if (btn_pressed(BTN_SELECT) && !B.crossed) {
		/* Cycle through the styles earned this run. */
		int opts[8] = { 0 }, n = 1;
		for (int k = 1; k <= 5; ++k) if (run.crosses & (1u << k)) opts[n++] = k;
		if (run.beast_out) opts[n++] = BEAST;
		int cur = 0;
		for (int k = 0; k < n; ++k) if (opts[k] == B.cross_pick) cur = k;
		B.cross_pick = opts[(cur + 1) % n];
		audio_sfx(n > 1 ? SFX_CHIP_CURSOR : SFX_ERROR);
	}
	if (btn_pressed(BTN_A)) {
		if (B.cursor == OFFER_MAX) {
			/* Confirm: selected chips become the hand, in pick order. */
			B.hand_n = 0;
			for (int i = 0; i < B.npicked; ++i) B.hand[B.hand_n++] = B.offer[B.pick_order[i]];
			form_program_advance();
			if (B.cross_pick && !B.crossed) transform(B.cross_pick);
			B.state = BS_CUSTOM_OUT;
			B.t = 0;
			B.custom = 0;
			B.turn++;
			audio_sfx(SFX_CHIP_OK);
		} else if (can_pick(B.cursor)) {
			B.picked[B.cursor] = true;
			B.pick_order[B.npicked++] = B.cursor;
			B.emblem_t = 0;
			audio_sfx(SFX_CHIP_SELECT);
		} else audio_sfx(SFX_ERROR);
	}
	if (btn_pressed(BTN_B) && B.npicked > 0) {
		B.picked[B.pick_order[--B.npicked]] = false;
		audio_sfx(SFX_CANCEL);
	}
}

/* Displayed HP rolls toward the real value: MegaMan's by an eighth of the
 * gap plus 4 a frame, enemies' plus 2 (both measured from the original). */
static void roll_hp(Ent *e) {
	int gap = e->shown_hp - e->hp;
	if (gap > 0) {
		int step = (gap >> 3) + (e->kind == K_PLAYER ? 4 : 2);
		e->shown_hp -= step > gap ? gap : step;
	} else if (gap < 0) {
		int step = (-gap >> 3) + 4;
		e->shown_hp += step > -gap ? -gap : step;
	}
}

static void setup_reveal(void) {
	for (int i = 0; i < 42; ++i) B.reveal_order[i] = (uint8_t)i;
	for (int i = 41; i > 0; --i) {
		int j = rng_range(0, i);
		uint8_t t = B.reveal_order[i]; B.reveal_order[i] = B.reveal_order[j]; B.reveal_order[j] = t;
	}
}

/* Frames of the result flow after the A press (original: a tile a frame
 * from 5 to 46, a tick every 4 frames from 6, the name at 76). */
#define REVEAL_START 4
#define REVEAL_NAME 76

static int foe_count_alive(void) {
	int n = 0;
	for (int i = 1; i < MAX_ENTS; ++i)
		if (B.ent[i].on && (B.ent[i].kind == K_VIRUS || B.ent[i].kind == K_NAVI)) ++n;
	return n;
}

/* The next enemy still to materialise, or NULL. */
static Ent *next_to_spawn(void) {
	for (int i = 1; i < MAX_ENTS; ++i)
		if (B.ent[i].on && B.ent[i].appear < 16) return &B.ent[i];
	return NULL;
}

/* Banners: 58 frames each, scaled open over 8 and closed over 6. */
#define BANNER_FRAMES 58
static int banner_pd(int t) {
	static const int open[8] = { 768, 640, 512, 384, 256, 208, 224, 256 };
	static const int close[6] = { 320, 384, 512, 640, 768, 896 };
	if (t < 0 || t >= BANNER_FRAMES) return 0;
	if (t < 8) return open[t];
	if (t >= BANNER_FRAMES - 6) return close[t - (BANNER_FRAMES - 6)];
	return 256;
}

static void update_sprites(void) {
	for (int i = 0; i < MAX_ENTS; ++i) {
		Ent *e = &B.ent[i];
		if (!e->on || !e->spr) continue;
		anim_update(&e->anim);
		if (e->anim.done && (i != 0 || B.act == ACT_NONE)) {
			/* idle anims loop; one-shots hold until the AI changes them */
			if (e->anim.anim == 0) anim_play(&e->anim, e->spr, 0);
		}
		roll_hp(e);
		if (e->flash > 0) --e->flash;
	}
	if (B.hp_hurt > 0) --B.hp_hurt;
}

static void update_fx_only(void) {
	for (int i = 0; i < MAX_SPELLS; ++i) if (B.sp[i].on && B.sp[i].type == SP_FX) update_spell(&B.sp[i]);
	for (int i = 1; i < MAX_ENTS; ++i) if (B.ent[i].on && B.ent[i].dying) update_dying(&B.ent[i]);
}

static void update(void) {
	B.scroll++;
	if (B.state == BS_DONE) return;
	if (B.banner_t >= 0 && ++B.banner_t >= BANNER_FRAMES) B.banner_t = -1;
	if (B.banner_timer > 0) --B.banner_timer;
	if (B.pa_flash > 0) --B.pa_flash;
	if (B.chip_label_timer > 0) --B.chip_label_timer;
	if (B.shake > 0) --B.shake;
	update_sprites();

	switch (B.state) {
	case BS_LOAD:
		/* 73 white frames (the original's load), then the music and a
		 * 16-frame fade in */
		if (++B.t == 73) {
			if (B.enc.boss && (B.enc.biome == BIOME_NEST || B.enc.biome == BIOME_SECRET)) audio_music_id(0x17);
			else audio_music(B.enc.boss ? MUS_BOSS : MUS_BATTLE);
		}
		if (B.t >= 90) { B.state = BS_SPAWN; B.t = 0; }
		return;
	case BS_SPAWN: {
		/* Each enemy fades in over 32 frames under a shrinking mosaic; the
		 * HP numbers follow a frame after the last, the Custom window 5. */
		Ent *e = next_to_spawn();
		if (e && B.t == 32) {
			e->appear = 16;
			B.t = 0;
			e = next_to_spawn();
			if (!e) { B.spawn_i = 0; return; }
		}
		if (e) {
			if (B.t == 0) audio_sfx(SFX_MATERIALIZE);
			e->appear = B.t++ / 2;
			return;
		}
		++B.spawn_i;
		if (B.spawn_i == 1) B.foe_hp_on = true;
		if (B.spawn_i >= 5) open_custom();
		return;
	}
	case BS_CUSTOM:
		update_custom_screen();
		return;
	case BS_CUSTOM_OUT:
		/* A frame after the press it slides out 12 pixels a frame; the
		 * gauge returns once it is gone */
		if (B.t++ == 0) return;
		if (B.slide > 0) {
			--B.slide;
			B.shift = 15 - ((10 - B.slide) * 3) / 2;
			return;
		}
		B.gauge_on = true;
		B.t = 0;
		B.slide = -1;
		B.fight_t = 0;
		B.state = BS_START; /* every turn starts with BATTLE START */
		return;
	case BS_START:
		/* The chip icons appear at 51 frames, BATTLE START at 57 */
		++B.t;
		if (B.t == 51) B.icons_on = true;
		if (B.t == 57) { B.banner_which = 0; B.banner_t = 0; }
		if (B.t >= 58 + BANNER_FRAMES) { B.state = BS_FIGHT; B.t = 0; }
		return;
	case BS_WIN:
		update_fx_only();
		/* ENEMY DELETED, then 47 frames before the RESULT window */
		if (++B.t == 1) {
			B.banner_which = 1;
			B.banner_t = 0;
			B.gauge_on = B.icons_on = false;
			audio_music(MUS_WIN);
		}
		if (B.t >= BANNER_FRAMES + 48) {
			B.result_busting = busting_level();
			B.reward_n = 0;
			B.reward_sel = 0;
			if (reward_maker) reward_maker(B.result_busting, B.reward, &B.reward_n);
			setup_reveal();
			B.state = BS_RESULT;
			B.state_timer = 0;
			B.reveal_t = -1;
		}
		return;
	case BS_RESULT:
		++B.state_timer;
		if (B.reveal_t >= 0) {
			++B.reveal_t;
			if (B.reveal_t >= 6 && B.reveal_t <= 42 && (B.reveal_t - 6) % 4 == 0) audio_sfx(SFX_REVEAL);
			if (B.reveal_t == REVEAL_NAME && B.reward_n > 0) audio_sfx(SFX_GOT);
			if (B.reveal_t < REVEAL_NAME) return;
			if (B.reward_n > 1 && btn_repeat(BTN_RIGHT)) { B.reward_sel = (B.reward_sel + 1) % B.reward_n; audio_sfx(SFX_CHIP_CURSOR); }
			if (B.reward_n > 1 && btn_repeat(BTN_LEFT)) { B.reward_sel = (B.reward_sel + B.reward_n - 1) % B.reward_n; audio_sfx(SFX_CHIP_CURSOR); }
			if (btn_pressed(BTN_A)) { B.state = BS_EXIT; B.t = 0; }
		} else if (B.state_timer > 13 && btn_pressed(BTN_A)) {
			B.reveal_t = 0;
		}
		return;
	case BS_EXIT:
		/* 22 frames, a 16-frame fade to black, black until the net fades in */
		if (++B.t == 42) audio_music(MUS_NONE);
		if (B.t >= 61) finish(BATTLE_WIN);
		return;
	case BS_LOSE: {
		/* MegaMan: 22 frames as a white silhouette, then 32 fading out under
		 * a growing mosaic; MEGAMAN DELETED at 57 while the enemies carry on;
		 * a fade to black from 153, then the game over screen. */
		Ent *p = PLAYER;
		++B.t;
		if (B.t > 22) p->appear = 16 - (B.t - 22) / 2 > 0 ? 16 - (B.t - 22) / 2 : 0;
		if (B.t == 57) { B.banner_which = 2; B.banner_t = 0; B.gauge_on = B.icons_on = false; }
		if (B.t == 167) audio_music(MUS_NONE);
		if (B.t >= 177) { finish(BATTLE_LOSE); return; }
		update_panels();
		for (int i = 1; i < MAX_ENTS; ++i) {
			Ent *e = &B.ent[i];
			if (!e->on) continue;
			if (e->dying) { update_dying(e); continue; }
			if (e->kind == K_VIRUS) ai_virus(e);
			else if (e->kind == K_NAVI) ai_navi(e);
		}
		for (int i = 0; i < MAX_SPELLS; ++i) if (B.sp[i].on) update_spell(&B.sp[i]);
		return;
	}
	default:
		break;
	}

	/* Fighting; START pauses (the gauge animation and HP rolls carry on) */
	if (B.custom >= CUSTOM_FULL) ++B.full_t;
	if (btn_pressed(BTN_START) && !B.custom_req) { B.paused = !B.paused; audio_sfx(SFX_PAUSE); }
	if (B.paused) return;
	B.frames++;
	run.frames++;
	int gain = (run.perks & PERK_RAPID_CUSTOM) ? 2 : 1;
	if (B.custom < CUSTOM_FULL && ++B.fight_t > 1) B.custom += gain;
	if (B.custom >= CUSTOM_FULL) {
		B.custom = CUSTOM_FULL;
		if (!B.gauge_was_full) { B.gauge_was_full = true; audio_sfx(SFX_GAUGE_FULL); }
		/* L or R: the fight runs on for 8 frames, the gauge and chips go on
		 * the 9th and the window starts in on the 10th, as in the original */
		if (!B.custom_req && (btn_pressed(BTN_L) || btn_pressed(BTN_R))) B.custom_req = 1;
	}
	if (B.custom_req && ++B.custom_req >= 11) {
		if (B.custom_req == 11) { B.gauge_on = B.icons_on = false; }
		else {
			B.custom_req = 0;
			B.gauge_was_full = false;
			B.full_t = 0;
			open_custom();
			return;
		}
	}
	update_panels();
	update_player();
	update_buster_shot();
	for (int i = 1; i < MAX_ENTS; ++i) {
		Ent *e = &B.ent[i];
		if (!e->on) continue;
		if (e->dying) { update_dying(e); continue; }
		if (e->kind == K_VIRUS) ai_virus(e);
		else if (e->kind == K_NAVI) ai_navi(e);
		else if (e->kind == K_ROCK && e->hp <= 0) e->on = false;
	}
	for (int i = 0; i < MAX_SPELLS; ++i) if (B.sp[i].on) update_spell(&B.sp[i]);

	if (PLAYER->hp <= 0) {
		PLAYER->dying = true;
		PLAYER->die_timer = 9999;
		ent_anim(PLAYER, 1);
		B.state = BS_LOSE;
		B.t = 0;
		B.charge = 0;
		audio_sfx(SFX_EXPLODE);
		return;
	}
	/* Low HP: the warning beep every 45 frames */
	if (PLAYER->hp * 4 <= PLAYER->maxhp && B.frames % 45 == 0) audio_sfx(SFX_LOW_HP);
	/* The last enemy gone: ENEMY DELETED three frames later */
	if (foe_count_alive() == 0 && ++B.t >= 3) {
		B.state = BS_WIN;
		B.t = 0;
	}
	(void)count_foes;
}

/* ---------------------------------------------------------------- */
/* Drawing */

/* ---------------------------------------------------------------- */
/* Original UI, drawn from the ROM's own tiles (see docs/ROM_DATA.md). */


/* The Custom screen's icon palette: ten colours from one table, three from
 * the element colour table, as the game assembles it. */
static uint32_t icon_palette(void) {
	static uint32_t handle;
	if (!handle) {
		uint16_t c[16] = { 0 };
		for (int i = 0; i < 10; ++i) c[i] = rom_u16(UI.icon_pal + (uint32_t)i * 2);
		for (int i = 0; i < 3; ++i) c[10 + i] = rom_u16(UI.icon_elem_colors + (uint32_t)i * 2);
		handle = gfx_palette(c);
	}
	return handle;
}

static int code_index(char code) { return code == '*' ? 26 : code >= 'A' && code <= 'Z' ? code - 'A' : 27; }

/* 8x16 glyph: two tiles, the bottom one 0x20 after the top. */
static void tall_glyph(uint32_t base, int index, uint32_t pal, int x, int y) {
	uint32_t t = base + (uint32_t)index * 0x40;
	rom_tile(t, pal, x, y, 0);
	rom_tile(t + 0x20, pal, x, y + 8, 0);
}

/* Right-aligned number in `slots` 8-pixel cells; blank cells are skipped. */
static void tall_number(uint32_t base, uint32_t pal, int x, int y, int slots, int value) {
	char buf[12];
	snprintf(buf, sizeof buf, "%d", value < 0 ? 0 : value);
	int n = (int)strlen(buf);
	for (int i = 0; i < n && i < slots; ++i) tall_glyph(base, buf[n - 1 - i] - '0', pal, x + (slots - 1 - i) * 8, y);
}

static void draw_hp_box(int x, int y, int hp, bool hurt) {
	uint32_t pal = hurt ? UI.hp_hurt_pal : UI.hp_pal;
	tall_glyph(UI.hp_digits, 11, pal, x, y);
	for (int i = 0; i < 4; ++i) tall_glyph(UI.hp_digits, 10, pal, x + 8 + i * 8, y);
	tall_number(UI.hp_digits, pal, x + 8, y, 4, hp);
	rom_tile(UI.hp_digits + 11 * 0x40, pal, x + 40, y, 1);
	rom_tile(UI.hp_digits + 11 * 0x40 + 0x20, pal, x + 40, y + 8, 1);
}

void battle_hp_box(int x, int y, int hp) { draw_hp_box(x, y, hp, false); }

void battle_area_name(int x, int y, const char *s) { text_draw_cells(x, y, s, UI.hp_pal); }

static bool player_hp_hurt(void) { return B.hp_hurt > 0 || PLAYER->shown_hp != PLAYER->hp; }

static void draw_gauge(int x, int y, int value, int full, int full_t) {
	/* caps, a 16-tile body with the CUSTOM label, and 1-pixel fill steps
	 * (a pixel per 4 frames, 127 at most); when full the bar cycles four
	 * patterns every 7 frames and the "L or R" label swaps colour every 8. */
	uint32_t cap = UI.gauge + 0x120, body = UI.gauge + 0x160, label = UI.gauge + 0x180;
	rom_tile(cap, UI.gauge_pal, x, y, 0);
	rom_tile(cap + 0x20, UI.gauge_pal, x, y + 8, 0);
	int px = value >> 2;
	if (px > 127) px = 127;
	for (int i = 0; i < 16; ++i) {
		int tx = x + 8 + i * 8;
		rom_tile(i >= 6 && i <= 9 ? label + (uint32_t)(i - 6) * 0x20 : body, UI.gauge_pal, tx, y, 0);
		if (value >= full) {
			uint32_t t = i >= 6 && i <= 9 ? UI.gauge + ((full_t / 8) & 1 ? 0x300 : 0x280) + (uint32_t)(i - 6) * 0x20
			                            : UI.gauge + 0x200 + (uint32_t)((full_t / 7) % 4) * 0x20;
			rom_tile(t, UI.gauge_pal, tx, y + 8, 0);
			continue;
		}
		int k = px - i * 8;
		if (k < 0) k = 0;
		if (k > 8) k = 8;
		rom_tile(UI.gauge + (uint32_t)k * 0x20, UI.gauge_pal, tx, y + 8, 0);
	}
	rom_tile(cap, UI.gauge_pal, x + 136, y, 1);
	rom_tile(cap + 0x20, UI.gauge_pal, x + 136, y + 8, 1);
}

static void draw_emotion(int x, int y) {
	rom_tiles(UI.emotion, UI.emotion_pal, x, y, 4, 2, 0);
	rom_tiles(UI.emotion_face, UI.emotion_pal, x + 32, y, 2, 2, 0);
}

static void draw_icon(int chip, uint32_t pal, int x, int y) {
	uint32_t icon = rom_u32(R.layout->chip_data + (uint32_t)chip * 0x2C + 0x20);
	if (rom_is_ptr(icon)) rom_tiles(rom_off(icon), pal, x, y, 2, 2, 0);
}

/* The Custom cursor pulses: 8 frames with its corners out, 8 drawn in. */
static void draw_cursor(int x, int y, int w, int h, int t) {
	if ((t / 8) & 1) {
		rom_tile(UI.cursor_wide, UI.cursor_pal, x - 2, y - 2, 0);
		rom_tile(UI.cursor_wide, UI.cursor_pal, x + w - 7, y - 2, 1);
		rom_tile(UI.cursor_wide, UI.cursor_pal, x - 2, y + h - 7, 2);
		rom_tile(UI.cursor_wide, UI.cursor_pal, x + w - 7, y + h - 7, 3);
	} else {
		rom_tile(UI.cursor, UI.cursor_pal, x - 3, y - 3, 0);
		rom_tile(UI.cursor, UI.cursor_pal, x + w - 5, y - 3, 1);
		rom_tile(UI.cursor, UI.cursor_pal, x - 3, y + h - 5, 2);
		rom_tile(UI.cursor, UI.cursor_pal, x + w - 5, y + h - 5, 3);
	}
}

static void draw_entity(Ent *e) {
	if (!e->on || e->appear <= 0) return;
	int x = foot_x(e->col) + e->ox, y = foot_y(e->row) + e->oy;
	int fx_flags = 0, pal = e->pal;
	/* hit and deletion flashes are plain white silhouettes */
	if (e->dying && e->kind != K_PLAYER && !(((DIE_FRAMES - e->die_timer) / 2) & 1)) pal = SPRITE_WHITE;
	if (e->dying && e->kind == K_PLAYER) pal = SPRITE_WHITE;
	if (e->flash > 0) pal = SPRITE_WHITE;
	if (e->kind == K_PLAYER) {
		if (e->invuln > 0 && !e->dying && (B.scroll & 2)) return;
		if (e->invis > 0) fx_flags |= FX_GHOST;
		/* MegaMan's shadow is a separate animation. */
		if (!e->dying) sprite_draw_frame(e->spr, 22, 0, x, y, false, 0, 0);
		/* charge lines around MegaMan, pink once charged (its own palette) */
		if (B.charge >= CHARGE_SHOWN && B.charge_fx.spr) {
			bool full = B.charge >= charge_time();
			anim_draw(&B.charge_fx, x + 4, y - 2, false, full ? SPRITE_ROM_PAL + (int)UI.charge_full_pal : 0, 0);
		}
	}
	if (e->stun > 0) fx_flags |= (B.scroll & 4) ? FX_FLASH : 0;
	/* Battle sprites face right; the game mirrors everything on the enemy side. */
	bool flip = e->side == SIDE_ENEMY && e->kind != K_ROCK;
	if (e->appear < 16) {
		/* materialising: alpha e/16 under a 16..2 pixel mosaic */
		gfx_obj_alpha = e->appear * 16;
		gfx_obj_mosaic = 17 - e->appear;
	}
	anim_draw(&e->anim, x, y, flip, pal, fx_flags);
	/* the shooting pose (anim 9) wears the arm buster, a sprite of its own
	 * following the pose frame by frame */
	if (e->kind == K_PLAYER && !B.cross && e->anim.anim == 9) {
		Sprite *bs = sprite_get(SPR_GUI, (int)UI.buster_sprite);
		if (bs) sprite_draw_frame(bs, 0, e->anim.frame, x, y, false, pal, fx_flags);
	}
	gfx_obj_alpha = 255;
	gfx_obj_mosaic = 0;
	if (e->barrier > 0) {
		Sprite *bub = sprite_get(SPR_EFFECT, 0x25);
		if (bub) sprite_draw_frame(bub, 0, 0, x, y, false, 0, FX_GHOST);
	}
}

static void draw_field(void) {
	int sx = B.shake ? ((B.scroll & 2) ? 2 : -2) : 0;
	for (int r = 0; r < ROWS; ++r)
		for (int c = 0; c < COLS; ++c) {
			Panel *p = &B.field[r][c];
			int x = field_x() + c * PANEL_W + sx, y = field_y() + r * PANEL_H;
			if (p->warn > 0) panel_draw_warn(p->type, r, p->side, x, y);
			else panel_draw(p->type, r, p->side, x, y);
		}
	for (int c = 0; c < COLS; ++c) panel_edge_draw(B.field[ROWS - 1][c].side, field_x() + c * PANEL_W + sx, field_y() + ROWS * PANEL_H);
}

/* The text box the original opens over the Custom window for a chip's
 * description (27 x 8 tiles), lines shown one per frame. */
static void draw_desc_box(int x, int y, int chip, int lines, int rows, bool arrow) {
	const int cols = 27;
	y += (8 - rows) * 4; /* folding keeps the box centred */
	for (int r = 0; r < rows; ++r)
		for (int c = 0; c < cols; ++c) {
			bool top = r == 0, bot = r == rows - 1, left = c == 0, right = c == cols - 1;
			uint32_t t = UI.box_fill;
			int flip = 0;
			if ((top || bot) && (left || right)) { t = UI.box_corner; flip = (right ? 1 : 0) | (bot ? 2 : 0); }
			else if (top || bot) { t = UI.box_edge; flip = bot ? 2 : 0; }
			else if (left) { t = UI.box_side; flip = 2; }
			else if (right) { t = r == 1 ? UI.box_side_top : UI.box_side; flip = r == 1 ? 0 : 3; }
			rom_tile(t, UI.box_pal, x + c * 8, y + r * 8, flip);
		}
	if (chip < 0) return;
	char desc[128];
	chip_desc(chip, desc, sizeof desc);
	char *line = desc;
	for (int l = 0; l < 3 && l < lines && line; ++l) {
		char *nl = strchr(line, '\n');
		if (nl) *nl = 0;
		chat_draw(x + 51, y + 12 + l * 16, line, -1);
		line = nl ? nl + 1 : NULL;
	}
	if (arrow) rom_tiles(UI.box_arrow, UI.chat_pal, x + 202, y + 45, 2, 2, 0);
}

static void draw_custom_screen(void) {
	/* 12 pixels a step; the HP box and emotion window ride on its right edge */
	int x0 = P.core_x - (120 - 12 * B.slide), y0 = P.core_y;
	if (B.slide < 0) return;
	/* Wide screens: carry the window's background colour into the border. */
	if (P.core_x > 0) fill_rect(x0 - P.core_x, y0, P.core_x, CORE_H, rgba(128, 144, 176, 255));
	hud_window(HUD_CUSTOM, x0, y0);
	/* Picking a chip spins the emblem once, zooming, over 17 frames from the
	 * second frame after the press (the OAM affine values recorded) */
	static const struct { short ang; short scale; signed char dx, dy; } spin[17] = {
		{ 45, 106, -1, -1 }, { 90, 114, -1, -2 }, { 135, 114, 0, -2 }, { 180, 123, 1, -1 }, { 225, 123, 1, -1 },
		{ 248, 123, 1, 0 }, { 270, 123, 1, 0 }, { 292, 123, 1, 0 }, { 304, 115, 1, 0 }, { 315, 115, 1, 0 },
		{ 326, 115, 0, 0 }, { 332, 115, 0, 0 }, { 338, 107, 0, 0 }, { 343, 107, 0, 0 }, { 349, 107, 0, 0 },
		{ 353, 106, 0, 0 }, { 357, 100, 0, 0 },
	};
	int sk = B.emblem_t - 2;
	if (sk >= 0 && sk < 17) rom_tiles_affine(UI.emblem, UI.cursor_pal, x0 + 95 + spin[sk].dx, y0 + 4 + spin[sk].dy, 2, 2, spin[sk].ang, spin[sk].scale / 100.0);
	else rom_tiles(UI.emblem, UI.cursor_pal, x0 + 95, y0 + 4, 2, 2, 0);
	int sel = B.cursor < B.offer_n ? B.cursor : -1;
	if (B.cursor == OFFER_MAX) {
		/* OK shows the "CHIP DATA TRANSMISSION" panel in the art slot */
		rom_tiles(UI.ok_art, UI.ok_art_pal, x0 + 16, y0 + 24, 7, 6, 0);
	} else if (sel < 0) fill_rect(x0 + 16, y0 + 24, 56, 48, rgba(16, 24, 48, 255));
	if (sel >= 0) {
		ChipInfo ci;
		chip_info(B.offer[sel].id, &ci);
		text_draw(x0 + 16, y0 + 8, ci.name, WHITE, TEXT_LEFT);
		SDL_Texture *art = chip_image(B.offer[sel].id);
		if (art) { SDL_Rect d = { x0 + 16, y0 + 24, 56, 48 }; SDL_RenderCopy(P.renderer, art, NULL, &d); }
		tall_glyph(UI.code_letters, code_index(B.offer[sel].code), UI.window_pal, x0 + 16, y0 + 72);
		rom_tiles(UI.elem_icons + (uint32_t)ci.chip_element * 0x80, icon_palette(), x0 + 24, y0 + 72, 2, 2, 0);
		if (ci.power > 0 && chip_def(B.offer[sel].id)->kind != CK_RECOVER)
			tall_number(UI.power_digits, UI.window_pal, x0 + 40, y0 + 72, 4, ci.power);
	}
	/* Chips already picked, in the right-hand column */
	for (int i = 0; i < B.npicked; ++i) draw_icon(B.offer[B.pick_order[i]].id, icon_palette(), x0 + 96, y0 + 24 + i * 16);
	/* Offer grid: two rows of five; unused slots are covered with the window's fill.
	 * A picked chip leaves its empty slot and keeps its code letter. */
	for (int i = 0; i < 10; ++i) {
		int sx = x0 + 8 + (i % 5) * 16, sy = y0 + 104 + (i / 5) * 24;
		if (i >= B.offer_n) {
			if (i >= 5) for (int ty = 0; ty < 3; ++ty) for (int tx = 0; tx < 2; ++tx) rom_tile(0x02A6FC, UI.window_pal, sx + tx * 8, sy + ty * 8, 0);
			continue;
		}
		if (!B.picked[i]) draw_icon(B.offer[i].id, can_pick(i) ? icon_palette() : UI.icon_dim_pal, sx, sy);
		rom_tiles(UI.grid_letters + (uint32_t)code_index(B.offer[i].code) * 0x40, UI.window_pal, sx, sy + 16, 2, 1, 0);
	}
	if (B.state == BS_CUSTOM && B.slide >= 10 && B.desc_close == 0 && B.desc_t == 0) {
		if (B.cursor == OFFER_MAX) draw_cursor(x0 + 88, y0 + 104, 30, 22, B.state_timer);
		else draw_cursor(x0 + 8 + (B.cursor % 5) * 16, y0 + 104 + (B.cursor / 5) * 24, 16, 16, B.state_timer);
	}
	if (!B.crossed && (run.crosses || run.beast_out))
		text_drawf(x0 + 4, y0 + CORE_H + 1, B.cross_pick ? rgba(255, 200, 90, 255) : rgba(210, 220, 240, 255), TEXT_LEFT,
			"SELECT: %s", B.cross_pick ? cross_name(B.cross_pick) : "change style");
	draw_hp_box(x0 + 120, y0, PLAYER->shown_hp, player_hp_hurt());
	draw_emotion(x0 + 120, y0 + 18);
	/* Enemy names on dark tabs, top right, once the window is in */
	if (B.state == BS_CUSTOM && B.slide >= 10) {
		int ny = y0;
		for (int i = 1; i < MAX_ENTS; ++i) {
			Ent *e = &B.ent[i];
			if (!e->on || e->dying || e->kind == K_ROCK) continue;
			int n = (int)strlen(e->name), tx = P.core_x + CORE_W - 8 * n;
			rom_tile(UI.name_tab, UI.hp_pal, tx - 8, ny, 0);
			rom_tile(UI.name_tab + 0x20, UI.hp_pal, tx - 8, ny + 8, 0);
			for (int c = 0; c < n; ++c) {
				rom_tile(UI.name_tab + 0x40, UI.hp_pal, tx + c * 8, ny, 0);
				rom_tile(UI.name_tab + 0x60, UI.hp_pal, tx + c * 8, ny + 8, 0);
			}
			text_draw_cells(tx, ny, e->name, UI.hp_pal);
			ny += 16;
		}
	}
	int sel_id = sel >= 0 ? B.offer[sel].id : -1;
	if (B.desc_close == 1 || B.desc_close == 2) draw_desc_box(P.core_x, y0 + 96, sel_id, 3, 8, B.desc_close == 1);
	else if (B.desc_close >= 3 && B.desc_close <= 5) draw_desc_box(P.core_x, y0 + 96, -1, 0, 8 - 2 * (B.desc_close - 2), false);
	else if (B.desc_t >= 2 && !B.desc_close) draw_desc_box(P.core_x, y0 + 96, sel_id, B.desc_t - 1, 8, B.desc_t >= 4);
}

/* The reward art, revealed one 8x8 tile a frame in random order. */
static void draw_reward_art(SDL_Texture *art, int x, int y, int shown) {
	if (!art) return;
	for (int i = 0; i < shown && i < 42; ++i) {
		int t = B.reveal_order[i];
		SDL_Rect src = { (t % 7) * 8, (t / 7) * 8, 8, 8 }, dst = { x + src.x, y + src.y, 8, 8 };
		SDL_RenderCopy(P.renderer, art, &src, &dst);
	}
}

static void draw_result(int x, int y) {
	/* slides in from the left 16 pixels a frame, landing at 24 on frame 13 */
	int st = B.state == BS_RESULT ? B.state_timer : 13;
	int wx = x + (16 * st - 176 < 24 ? 16 * st - 176 : 24), wy = y + 16;
	hud_window(HUD_RESULT, wx, wy);
	rom_tiles(UI.emblem, UI.cursor_pal, wx + 13, wy + 5, 2, 2, 0);
	wx += 8; /* contents are laid out from the inner edge */
	/* DeleteTime m:ss:cc and Busting level (10 shows as S) */
	int cs = B.frames * 100 / 60;
	int m = cs / 6000 % 10, sec = cs / 100 % 60, c = cs % 100;
	tall_glyph(UI.result_digits, m, UI.result_pal, wx + 104, wy + 32);
	tall_glyph(UI.result_digits, sec / 10, UI.result_pal, wx + 120, wy + 32);
	tall_glyph(UI.result_digits, sec % 10, UI.result_pal, wx + 128, wy + 32);
	tall_glyph(UI.result_digits, c / 10, UI.result_pal, wx + 144, wy + 32);
	tall_glyph(UI.result_digits, c % 10, UI.result_pal, wx + 152, wy + 32);
	int lvl = B.result_busting;
	tall_glyph(UI.result_digits, lvl >= 10 ? 11 : lvl, UI.result_pal, wx + 152, wy + 48);
	/* PRESS A BUTTON blinks 8 on, 8 off from 6 frames after the window lands */
	if (B.reveal_t < 0 && st >= 19 && !(((st - 19) / 8) & 1)) rom_tiles(UI.press_a, UI.result_pal, wx + 8, wy + 112, 10, 1, 0);
	if (B.reward_n <= 0 || B.reveal_t < 0) return;
	RewardOption *o = &B.reward[B.reward_sel];
	int shown = B.reveal_t - REVEAL_START;
	if (shown < 0) shown = 0;
	SDL_Texture *art = o->chip >= 0 ? chip_image(o->chip) : zenny_image();
	if (B.reveal_t >= REVEAL_NAME) {
		if (art) { SDL_Rect d = { wx + 104, wy + 80, 56, 48 }; SDL_RenderCopy(P.renderer, art, NULL, &d); }
	} else draw_reward_art(art, wx + 104, wy + 80, shown);
	if (B.reveal_t < REVEAL_NAME) return;
	char name[32];
	if (o->chip >= 0) {
		ChipInfo ci;
		chip_info(o->chip, &ci);
		snprintf(name, sizeof name, "%s %c", ci.name, o->code);
		text_draw(wx + 10, wy + 95, name, WHITE, TEXT_LEFT);
	} else {
		snprintf(name, sizeof name, "%d z", o->zenny);
		text_draw(wx + 88, wy + 95, name, WHITE, TEXT_RIGHT);
	}
	if (B.reward_n > 1) {
		text_drawf(wx + 92, wy + 128, rgba(40, 60, 110, 255), TEXT_CENTER, "%d/%d", B.reward_sel + 1, B.reward_n);
		if ((B.scroll / 16) & 1) {
			text_draw(wx + 100, wy + 96, "<", rgba(255, 230, 90, 255), TEXT_RIGHT);
			text_draw(wx + 162, wy + 96, ">", rgba(255, 230, 90, 255), TEXT_LEFT);
		}
	}
}

static void draw_hud(void) {
	Ent *p = PLAYER;
	int x = P.core_x, y = P.core_y;
	bool custom = B.state == BS_CUSTOM || B.state == BS_CUSTOM_OUT;
	if (!custom) {
		draw_hp_box(x, y, p->shown_hp, player_hp_hurt());
		if (B.state != BS_WIN && B.state != BS_RESULT && B.state != BS_EXIT && !(B.state == BS_LOSE && B.t >= 57)) draw_emotion(x, y + 18);
	}
	if (B.gauge_on && !custom) draw_gauge(x + 48, y, B.custom, CUSTOM_FULL, B.full_t);
	if (B.paused && B.state == BS_FIGHT) {
		/* PAUSE: 5x2 tiles laid out as the game's two OBJs */
		for (int i = 0; i < 4; ++i) {
			rom_tile(UI.pause_text + (uint32_t)i * 0x20, UI.enemy_hp_pal, x + 100 + i * 8, y + 63, 0);
			rom_tile(UI.pause_text + 0x80 + (uint32_t)i * 0x20, UI.enemy_hp_pal, x + 100 + i * 8, y + 71, 0);
		}
		rom_tile(UI.pause_text + 0x100, UI.enemy_hp_pal, x + 132, y + 63, 0);
		rom_tile(UI.pause_text + 0x120, UI.enemy_hp_pal, x + 132, y + 71, 0);
	}
	/* Enemy HP in the game's small digits; red glyphs (10 on) while it drops */
	for (int i = 1; i < MAX_ENTS && B.foe_hp_on; ++i) {
		Ent *e = &B.ent[i];
		if (!e->on || e->dying || e->kind == K_ROCK || e->appear < 16) continue;
		char buf[8];
		snprintf(buf, sizeof buf, "%d", e->shown_hp);
		int n = (int)strlen(buf), red = e->shown_hp != e->hp ? 10 : 0;
		for (int k = 0; k < n && k < 4; ++k)
			tall_glyph(UI.enemy_hp_digits, buf[n - 1 - k] - '0' + red, UI.enemy_hp_pal, foot_x(e->col) + e->ox - 20 + (3 - k) * 8, foot_y(e->row));
	}
	/* Hand: icons stacked above MegaMan's head, and the next chip's name */
	if (B.icons_on && (B.state == BS_START || B.state == BS_FIGHT) && B.hand_n > 0 && !p->dying)
		for (int i = B.hand_n - 1; i >= 0; --i)
			draw_icon(B.hand[i].id, UI.icon_obj_pal, foot_x(p->col) - 9 + i * 2, foot_y(p->row) - 56 - i * 2);
	/* The next chip's name in 8x16 cells at the bottom left, power in the
	 * HP box's yellow digits (12 on), two frames into the fight */
	if (B.state == BS_FIGHT && B.fight_t >= 2 && !B.paused && B.hand_n > 0 && !p->dying) {
		ChipInfo ci;
		chip_info(B.hand[0].id, &ci);
		text_draw_cells(x, y + CORE_H - 16, ci.name, UI.hp_pal);
		if (ci.power > 0 && chip_def(B.hand[0].id)->kind != CK_RECOVER) {
			char buf[8];
			snprintf(buf, sizeof buf, "%d", ci.power + B.atk_plus);
			int nx = x + 8 * (int)strlen(ci.name);
			for (int k = 0; buf[k]; ++k) tall_glyph(UI.hp_digits, 12 + buf[k] - '0', UI.hp_pal, nx + k * 8, y + CORE_H - 16);
		}
	}
	if (B.pa_flash > 0) {
		fill_rect(x, y + 20, CORE_W, 34, rgba(40, 10, 80, 200));
		text_draw(x + CORE_W / 2, y + 22, "PROGRAM ADVANCE", ((B.scroll / 4) & 1) ? rgba(255, 120, 255, 255) : WHITE, TEXT_CENTER);
		text_draw(x + CORE_W / 2, y + 37, B.pa_name, rgba(255, 230, 90, 255), TEXT_CENTER);
	}
	/* The original banners for the start and the end of a battle */
	if (B.banner_t >= 0) hud_banner_scaled(B.banner_which, x, y + 64, banner_pd(B.banner_t));
	if (B.banner_timer > 0 && B.banner[0]) {
		fill_rect(x, y + 60, CORE_W, 20, rgba(20, 30, 80, 200));
		text_draw(x + CORE_W / 2, y + 62, B.banner, WHITE, TEXT_CENTER);
	}
	if (B.state == BS_RESULT || B.state == BS_EXIT) draw_result(x, y);
}

static void draw(void) {
	battle_bg_draw(battle_debug_bg >= 0 ? battle_debug_bg : biome_bg(B.enc.biome), B.scroll);
	draw_field();
	/* Entities back to front */
	for (int r = 0; r < ROWS; ++r)
		for (int i = MAX_ENTS - 1; i >= 0; --i)
			if (B.ent[i].on && B.ent[i].row == r) draw_entity(&B.ent[i]);
	for (int i = 0; i < MAX_SPELLS; ++i) {
		Spell *s = &B.sp[i];
		if (!s->on) continue;
		if (s->type == SP_METEOR && s->timer > 0) continue;
		if (s->type == SP_FX && s->timer < 0) continue; /* waiting to appear */
		if (s->type == SP_BOMB) {
			/* flight: x from 13 px ahead of MegaMan to 4 short of the target,
			 * height 58 + 16t/15 - t^2/15 above the row (measured) */
			if (!s->spr) continue;
			float t = (float)s->timer;
			float x0 = (float)foot_x(s->col) + 13, x1 = (float)foot_x(s->tcol) - 4;
			float x = x0 + (x1 - x0) * t / 36.0f, h = 58 + 16 * t / 15 - t * t / 15;
			int gy = foot_y(s->row);
			sprite_draw_part(s->spr, 1, 0, 0, (int)x, gy, false, 0);
			sprite_draw_part(s->spr, 1, 0, 1, (int)x, gy - (int)h, false, 0);
			continue;
		}
		if (s->type == SP_BEAM) {
			int y = foot_y(s->row) - 18;
			fill_rect(field_x(), y - 3, foot_x(s->col) - field_x(), 6, rgba(s->tint.r, s->tint.g, s->tint.b, 200));
			fill_rect(field_x(), y - 1, foot_x(s->col) - field_x(), 2, WHITE);
			continue;
		}
		if (s->spr) anim_draw(&s->anim, (int)s->x, (int)s->y + B.shift, s->flip || s->side == SIDE_ENEMY, 0, 0);
		else if (s->type == SP_WAVE) {
			int x = foot_x(s->col), y = foot_y(s->row);
			int hgt = 10 + (s->timer % s->param) * 2;
			fill_rect(x - 8, y - hgt, 16, hgt, rgba(s->tint.r, s->tint.g, s->tint.b, 220));
			fill_rect(x - 4, y - hgt - 4, 8, 4, WHITE);
		} else fill_rect((int)s->x - 3, (int)s->y - 3, 6, 6, s->tint);
	}
	draw_hud();
	if (B.state == BS_CUSTOM || B.state == BS_CUSTOM_OUT) draw_custom_screen();
	/* Screen fades: white in from the encounter, black out after the results */
	P.fx_fade_color = WHITE;
	if (B.state == BS_LOAD) P.fx_fade = B.t < 73 ? 16 : 16 - (B.t - 73);
	if (B.state == BS_EXIT && B.t > 22) { P.fx_fade_color = BLACK; P.fx_fade = B.t - 22 > 16 ? 16 : B.t - 22; }
	if (B.state == BS_LOSE && B.t > 152) { P.fx_fade_color = BLACK; P.fx_fade = B.t - 152 > 16 ? 16 : B.t - 152; }
}

static void enter(void) {}

const Scene scene_battle = { "battle", enter, update, draw, NULL };
