/* The net overworld: walk the generated layer, meet viruses, find data. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "battle.h"
#include "data.h"
#include "game.h"
#include "gfx.h"
#include "loot.h"
#include "net.h"
#include "platform.h"
#include "run.h"
#include "save.h"
#include "ui.h"

#define TILE_W 32
#define TILE_H 16
#define SLAB 6
#define MM_SPRITE 55    /* overworld MegaMan in the NPC sprite list */

static struct {
	bool ready;
	float px, py;
	int dir;
	bool moving;
	Anim mm;
	float walked;
	int since_battle;
	int flash;         /* encounter: 16 frames of mosaic and white fade */
	int return_fade;   /* back from a battle: 16 frames in from black */
	bool to_battle;    /* the battle starts next frame: stay white */
	int fade;          /* layer transition */
	int banner;
	char banner_text[40];
	int pending;       /* action once the flash finishes */
	Encounter enc;
	int active_obj;
	int map_open;
	uint32_t tick;
	int trade_pick[3];
	int trade_n;
	bool boss_fight;
	bool challenge_fight;
} N;

enum { PEND_NONE, PEND_BATTLE };

static SDL_Texture *tile_tex[BIOME_COUNT][2];

/* ------------------------------------------------------------------ */
/* Biome look */

typedef struct { uint8_t face[3], edge[3], line[3], left[3], right[3], bg0[3], bg1[3], grid[3]; } BiomeLook;

static const BiomeLook looks[BIOME_COUNT] = {
	/* Central: green circuitry over blue */
	{ { 72, 200, 96 }, { 180, 255, 180 }, { 40, 150, 70 }, { 24, 96, 48 }, { 36, 128, 60 }, { 8, 24, 72 }, { 16, 64, 128 }, { 40, 96, 180 } },
	/* Seaside: aqua */
	{ { 64, 176, 232 }, { 200, 240, 255 }, { 32, 120, 200 }, { 24, 72, 144 }, { 32, 100, 180 }, { 0, 32, 64 }, { 0, 88, 120 }, { 40, 160, 200 } },
	/* Sky: white and lavender */
	{ { 216, 224, 248 }, { 255, 255, 255 }, { 160, 168, 224 }, { 120, 128, 184 }, { 150, 160, 210 }, { 72, 120, 200 }, { 160, 200, 248 }, { 220, 240, 255 } },
	/* Green: leaves and wood */
	{ { 136, 200, 64 }, { 220, 250, 150 }, { 90, 150, 40 }, { 96, 72, 32 }, { 128, 96, 48 }, { 16, 48, 16 }, { 48, 96, 32 }, { 90, 160, 60 } },
	/* Graveyard: ash and violet */
	{ { 120, 112, 140 }, { 190, 180, 220 }, { 80, 72, 110 }, { 48, 40, 64 }, { 64, 56, 88 }, { 16, 8, 24 }, { 40, 24, 56 }, { 90, 60, 120 } },
	/* Undernet: blood red on black */
	{ { 150, 40, 56 }, { 240, 100, 120 }, { 100, 20, 40 }, { 56, 8, 24 }, { 80, 16, 32 }, { 4, 0, 8 }, { 32, 4, 20 }, { 120, 20, 60 } },
	/* Secret Area: gold and black */
	{ { 216, 176, 64 }, { 255, 240, 160 }, { 160, 120, 32 }, { 96, 64, 16 }, { 128, 96, 24 }, { 8, 8, 8 }, { 40, 32, 8 }, { 200, 160, 40 } },
	/* Cybeast Nest: magenta and dark teal */
	{ { 176, 64, 176 }, { 255, 160, 255 }, { 120, 32, 120 }, { 40, 24, 64 }, { 64, 32, 96 }, { 0, 16, 24 }, { 24, 48, 64 }, { 160, 60, 200 } },
};

static uint32_t rgb3(const uint8_t c[3]) { return 0xFF000000u | (uint32_t)c[0] << 16 | (uint32_t)c[1] << 8 | c[2]; }
static uint32_t mix(uint32_t a, uint32_t b) {
	return 0xFF000000u | ((((a >> 16) & 255) + ((b >> 16) & 255)) / 2) << 16 | ((((a >> 8) & 255) + ((b >> 8) & 255)) / 2) << 8 | (((a & 255) + (b & 255)) / 2);
}

static SDL_Texture *build_tile(int biome, bool corrupt) {
	const BiomeLook *L = &looks[biome];
	uint32_t px[TILE_W * (TILE_H + SLAB)] = { 0 };
	uint32_t face = rgb3(L->face), edge = rgb3(L->edge), line = rgb3(L->line), left = rgb3(L->left), right = rgb3(L->right);
	if (corrupt) {
		face = mix(face, 0xFF901838u);
		line = 0xFF400818u;
		edge = 0xFFFF6080u;
	}
	for (int y = 0; y < TILE_H; ++y) {
		int hw = y < TILE_H / 2 ? (y + 1) * 2 : (TILE_H - y) * 2;
		for (int x = TILE_W / 2 - hw; x < TILE_W / 2 + hw; ++x) {
			uint32_t c = face;
			bool border = x == TILE_W / 2 - hw || x == TILE_W / 2 + hw - 1;
			if (border) c = y < TILE_H / 2 ? edge : line;
			/* inset diamond: BN net paths have a panel pattern */
			int dx = abs(x * 2 + 1 - TILE_W) / 2, dy = abs(y * 2 + 1 - TILE_H);
			int d = dx + dy;
			if (d == 9 || d == 10) c = line;
			if (corrupt && ((x * 7 + y * 13) % 11 == 0)) c = 0xFF200010u;
			px[y * TILE_W + x] = c;
		}
	}
	for (int x = 0; x < TILE_W; ++x) {
		int yb = x < TILE_W / 2 ? TILE_H / 2 + x / 2 : TILE_H / 2 + (TILE_W - 1 - x) / 2;
		for (int k = 1; k <= SLAB; ++k) {
			int y = yb + k;
			if (y >= TILE_H + SLAB) break;
			uint32_t c = x < TILE_W / 2 ? left : right;
			if (k == SLAB) c = 0xFF000000u | ((c & 0xFEFEFE) >> 1);
			if (corrupt && k == 2) c = 0xFFFF4060u;
			px[y * TILE_W + x] = c;
		}
	}
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(px, TILE_W, TILE_H + SLAB, 32, TILE_W * 4, SDL_PIXELFORMAT_ARGB8888);
	SDL_Texture *t = SDL_CreateTextureFromSurface(P.renderer, s);
	SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(s);
	return t;
}

/* ------------------------------------------------------------------ */
/* Projection: cell (x, y) -> screen, x down-right, y down-left. */

static float cam_x, cam_y;

static void iso(float x, float y, int *sx, int *sy) {
	*sx = (int)floorf((x - y) * (TILE_W / 2) - cam_x) + P.w / 2;
	*sy = (int)floorf((x + y) * (TILE_H / 2) - cam_y) + P.h / 2;
}

static bool walkable(float x, float y) {
	int cx = (int)floorf(x), cy = (int)floorf(y);
	if (cx < 0 || cy < 0 || cx >= MAP_W || cy >= MAP_H) return false;
	return layer.cell[cy][cx] == C_PATH;
}

static bool blocked(float x, float y) {
	const float r = 0.28f;
	if (!walkable(x - r, y - r) || !walkable(x + r, y - r) || !walkable(x - r, y + r) || !walkable(x + r, y + r)) return true;
	for (int i = 0; i < layer.nobj; ++i) {
		NetObj *o = &layer.obj[i];
		if (!o->solid || (o->type == OBJ_MYSTERY && o->used)) continue;
		if (o->type == OBJ_BOSS && layer.boss_beaten) continue;
		float dx = o->x - x, dy = o->y - y;
		if (dx * dx + dy * dy < 0.45f * 0.45f) return true;
	}
	return false;
}

/* ------------------------------------------------------------------ */
/* Layer flow */

static void on_battle(const BattleResult *r);

/* Percent chance per cell walked. A quiet stretch after each battle, then the
 * odds climb; infested rooms are always busy. */
static int encounter_chance(bool corrupt) {
	int grace = corrupt ? 4 : 10;
	int chance = N.since_battle < grace ? 0 : (N.since_battle - grace) * 2;
	if (chance > 18) chance = 18;
	if (corrupt) chance = chance * 2 + 5;
	return chance;
}

static int act_layer(void) { return (run.depth - 1) % 3 + 1; }

static bool resuming;

void net_resume(void) { resuming = true; N.ready = false; }

/* Keep per-layer progress in the run so a reload restores it. */
static void sync_layer(void) {
	run.layer_used = 0;
	for (int i = 0; i < layer.nobj && i < 64; ++i)
		if (layer.obj[i].used) run.layer_used |= 1ull << i;
	run.layer_boss_beaten = layer.boss_beaten;
}

static void checkpoint(void) {
	sync_layer();
	save_run();
}

static void start_layer(void) {
	int biome = run.side_kind == LAYER_UNDERNET ? BIOME_UNDERNET : run.side_kind == LAYER_SECRET ? BIOME_SECRET : biome_for_depth(run.depth);
	run.biome = biome;
	run.layer_seed = run.seed ^ (uint32_t)(run.depth * 2654435761u) ^ (uint32_t)(run.side_kind * 40503u);
	layer_generate(run.layer_seed, run.depth, biome, run.side_kind);
	if (resuming) {
		for (int i = 0; i < layer.nobj && i < 64; ++i) layer.obj[i].used = (run.layer_used >> i) & 1;
		layer.boss_beaten = run.layer_boss_beaten;
		resuming = false;
	} else {
		run.layer_used = 0;
		run.layer_boss_beaten = false;
	}
	rng_seed(run.layer_seed ^ 0xA5A5A5A5u ^ (uint32_t)run.frames);
	NetObj *w = &layer.obj[0];
	N.px = w->x;
	N.py = w->y + 0.01f;
	N.dir = 4;
	N.since_battle = 0;
	N.walked = 0;
	N.fade = 30;
	N.banner = 150;
	if (run.side_kind == LAYER_UNDERNET) snprintf(N.banner_text, sizeof N.banner_text, "Undernet");
	else if (run.side_kind == LAYER_SECRET) snprintf(N.banner_text, sizeof N.banner_text, "Secret Area");
	else if (biome == BIOME_NEST) snprintf(N.banner_text, sizeof N.banner_text, "%s", biome_name(biome));
	else snprintf(N.banner_text, sizeof N.banner_text, "%s %d", biome_name(biome), act_layer());
	audio_music_id(biome_song(biome));
	audio_sfx(SFX_JACK_IN);
	checkpoint();
	if (run.depth == 1 && run.side_kind == LAYER_NORMAL && !profile.seen_intro) {
		profile.seen_intro = true;
		profile_save();
		ui_message("Jack-in complete! Find the green warp pad to go deeper. Every third area, a Navi guards it.", -1);
		ui_message("D-pad walks (Up goes up-right). Hold B to run. A talks and opens Mystery Data. Start opens the menu, Select the map.", -1);
		ui_message("In battle: A uses a chip, B fires the buster (hold to charge). When the Custom gauge fills, press L or R for new chips.", -1);
	}
}

static void next_layer(void) {
	run.side_kind = LAYER_NORMAL;
	run.depth++;
	start_layer();
}

static void make_rewards(int busting, RewardOption *opts, int *n);

static void begin_battle(const Encounter *e) {
	battle_set_rewards(make_rewards);
	N.enc = *e;
	N.pending = PEND_BATTLE;
	N.flash = 16;
	audio_music(MUS_NONE);
	audio_sfx(SFX_ENCOUNTER);
}

/* ------------------------------------------------------------------ */
/* Rewards */

static int replace_chip, replace_code;

static uint16_t fold_ids[FOLDER_MAX];
static char fold_codes[FOLDER_MAX];

static void folder_arrays(void) {
	for (int i = 0; i < run.folder_n; ++i) { fold_ids[i] = run.folder[i].id; fold_codes[i] = run.folder[i].code; }
}

static void replace_done(int choice) {
	if (choice < 0) { ui_message("You left the chip behind.", -1); return; }
	run.folder[choice].id = (uint16_t)replace_chip;
	run.folder[choice].code = (char)replace_code;
	ChipInfo ci;
	chip_info(replace_chip, &ci);
	ui_messagef(-1, "%s %c joined the folder.", ci.name, replace_code);
}

static void give_chip(int id, char code) {
	ChipInfo ci;
	chip_info(id, &ci);
	if (run.folder_n < FOLDER_MAX) {
		folder_add(id, code);
		ui_messagef(-1, "Got a chip: %s %c!", ci.name, code);
		audio_sfx(SFX_ITEM);
		return;
	}
	/* Folder full: choose a chip to replace, in the folder screen. */
	replace_chip = id;
	replace_code = code;
	ui_messagef(-1, "The folder is full. Swap a chip for %s %c?", ci.name, code);
	folder_arrays();
	ui_folder("Replace?", fold_ids, fold_codes, run.folder_n, NULL, true, replace_done);
}

/* GET DATA options for the RESULT window: chips, then zenny. */
static void make_rewards(int busting, RewardOption *opts, int *n) {
	*n = 0;
	if (N.boss_fight) {
		const NaviDef *d = navi_def(layer.boss_navi);
		ChipInfo ci;
		chip_info(d->chip_reward, &ci);
		opts[(*n)++] = (RewardOption){ d->chip_reward, ci.ncodes ? ci.codes[0] : '*', 0 };
		return;
	}
	int bonus = (N.challenge_fight ? 2 : 0) + (run.side_kind == LAYER_UNDERNET) + (busting >= 9);
	int chips = 3 + ((run.perks & PERK_COLLECT) ? 1 : 0) - 1;
	for (int i = 0; i < chips; ++i) {
		char code;
		int id = roll_chip(run.depth, bonus, &code);
		opts[(*n)++] = (RewardOption){ id, code, 0 };
	}
	int z = (60 + rng_range(0, 40) * run.depth / 2 + busting * 20) * ((run.perks & PERK_HUMOR) ? 3 : 2) / 2;
	opts[(*n)++] = (RewardOption){ -1, 0, z };
}

static const char *perk_name(uint32_t p) {
	switch (p) {
	case PERK_SUPER_ARMOR: return "SuperArmr";
	case PERK_UNDERSHIRT: return "UnderSht";
	case PERK_FLOAT_SHOES: return "FloatShoe";
	case PERK_AIR_SHOES: return "AirShoes";
	case PERK_FIRST_BARRIER: return "1stBarrier";
	case PERK_COLLECT: return "Collect";
	case PERK_RAPID_CUSTOM: return "FastGauge";
	case PERK_REFLECT: return "Reflect";
	case PERK_HUMOR: return "Humor";
	case PERK_ATTACK_MAX: return "AttackMAX";
	default: return "?";
	}
}

static const char *perk_desc(uint32_t p) {
	switch (p) {
	case PERK_SUPER_ARMOR: return "Hits no longer knock MegaMan back.";
	case PERK_UNDERSHIRT: return "Once per battle, survive a lethal hit with 1 HP.";
	case PERK_FLOAT_SHOES: return "Ignore poison, and cracked panels hold.";
	case PERK_AIR_SHOES: return "Walk over broken panels.";
	case PERK_FIRST_BARRIER: return "Start every battle with a Barrier.";
	case PERK_COLLECT: return "One more chip to choose from after battles.";
	case PERK_RAPID_CUSTOM: return "The Custom gauge fills 50% faster.";
	case PERK_HUMOR: return "Viruses drop half again as much zenny.";
	case PERK_ATTACK_MAX: return "Charged shots deal 50% more.";
	default: return "";
	}
}

static uint32_t random_perk(void) {
	uint32_t pool[PERK_COUNT_BITS];
	int n = 0;
	for (int i = 0; i < PERK_COUNT_BITS; ++i) {
		uint32_t p = 1u << i;
		if (p == PERK_REFLECT || (run.perks & p)) continue;
		pool[n++] = p;
	}
	return n ? pool[rng_range(0, n - 1)] : 0;
}

static void give_perk(uint32_t p) {
	if (!p) { run.zenny += 1000; ui_message("A program you already have... converted to 1000 zenny.", -1); return; }
	run.perks |= p;
	ui_messagef(-1, "Installed program: %s! %s", perk_name(p), perk_desc(p));
	audio_sfx(SFX_ITEM);
}

/* ------------------------------------------------------------------ */
/* Mystery data */

static void open_mystery(NetObj *o) {
	if (o->param == 2) {
		if (run.unlockers <= 0) {
			ui_message("Purple Mystery Data. It's locked tight... An Unlocker would open it.", -1);
			return;
		}
		run.unlockers--;
		ui_message("Used an Unlocker.", -1);
	}
	o->used = true;
	audio_sfx(SFX_ITEM);
	int roll = rng_range(0, 99);
	char code;
	int id;
	switch (o->param) {
	case 0:
		if (roll < 50) { id = roll_chip(run.depth, 0, &code); give_chip(id, code); }
		else if (roll < 85) { int z = (100 + rng_range(0, 8) * 50) * (1 + run.depth / 6); run.zenny += z; ui_messagef(-1, "Found %d zenny.", z); }
		else { int b = rng_range(3, 8); run.bugfrags += b; ui_messagef(-1, "Found %d BugFrags.", b); }
		break;
	case 1:
		if (roll < 35) { run.max_hp += 20; run.hp += 20; ui_message("Got an HP Memory! Max HP +20.", -1); }
		else if (roll < 65) { id = roll_chip(run.depth, 1, &code); give_chip(id, code); }
		else if (roll < 80) { run.unlockers++; ui_message("Got an Unlocker! It opens purple Mystery Data.", -1); }
		else if (roll < 92) { int z = 800 + run.depth * 60; run.zenny += z; ui_messagef(-1, "Found %d zenny.", z); }
		else give_perk(random_perk());
		break;
	default:
		if (run.fragments < 3 && !run.secret_cleared && roll < 45) {
			run.fragments++;
			ui_messagef(-1, "Found a Secret Data fragment (%d/3). Something in the Undernet reacts...", run.fragments);
		} else if (roll < 75) { id = roll_chip(run.depth, 3, &code); give_chip(id, code); }
		else give_perk(random_perk());
		break;
	}
}

/* ------------------------------------------------------------------ */
/* Shops and traders */

static UiItem shop_items[8];
static int shop_ids[8];
static char shop_codes[8];
static int shop_prices[8];
static int shop_n;
enum { SHOP_CHIP, SHOP_HPMEM, SHOP_POWER, SHOP_RAPID, SHOP_CHARGE, SHOP_LEAVE };
static int shop_kind[8];

static void shop_open(void);

static void shop_done(int choice) {
	if (choice < 0 || shop_kind[choice] == SHOP_LEAVE) { ui_message("Mr. Prog: Come again!", -1); return; }
	if (run.zenny < shop_prices[choice]) { ui_message("Mr. Prog: Not enough zenny!", -1); return; }
	run.zenny -= shop_prices[choice];
	switch (shop_kind[choice]) {
	case SHOP_CHIP: give_chip(shop_ids[choice], shop_codes[choice]); shop_ids[choice] = -1; break;
	case SHOP_HPMEM: run.max_hp += 20; run.hp += 20; ui_message("Max HP +20!", -1); break;
	case SHOP_POWER: run.atk++; ui_messagef(-1, "Buster Attack is now %d.", run.atk); break;
	case SHOP_RAPID: run.rapid++; ui_messagef(-1, "Buster Rapid is now %d.", run.rapid); break;
	case SHOP_CHARGE: run.charge++; ui_messagef(-1, "Buster Charge is now %d.", run.charge); break;
	default: break;
	}
	audio_sfx(SFX_ITEM);
	shop_prices[choice] = -1;
}

static void shop_stock(NetObj *o) {
	uint32_t saved = rng_state();
	rng_seed(run.layer_seed ^ (uint32_t)(o - layer.obj) * 7919u);
	shop_n = 0;
	for (int i = 0; i < 4; ++i) {
		shop_kind[shop_n] = SHOP_CHIP;
		shop_ids[shop_n] = roll_chip(run.depth + 2, 0, &shop_codes[shop_n]);
		shop_prices[shop_n] = chip_price(shop_ids[shop_n]);
		++shop_n;
	}
	shop_kind[shop_n] = SHOP_HPMEM;
	shop_prices[shop_n++] = 1000 + run.max_hp * 5;
	int up = rng_range(0, 2);
	shop_kind[shop_n] = SHOP_POWER + up;
	shop_prices[shop_n++] = 2000 + 1500 * ((up == 0 ? run.atk : up == 1 ? run.rapid : run.charge) - 1);
	shop_kind[shop_n] = SHOP_LEAVE;
	shop_prices[shop_n++] = 0;
	rng_seed(saved);
}

static void shop_open(void) {
	for (int i = 0; i < shop_n; ++i) {
		UiItem *it = &shop_items[i];
		memset(it, 0, sizeof *it);
		it->chip = -1;
		switch (shop_kind[i]) {
		case SHOP_CHIP: {
			if (shop_ids[i] < 0) { snprintf(it->label, sizeof it->label, "SOLD OUT"); it->disabled = true; break; }
			ChipInfo ci;
			chip_info(shop_ids[i], &ci);
			snprintf(it->label, sizeof it->label, "%s", ci.name);
			it->chip = shop_ids[i];
			it->code = shop_codes[i];
			break;
		}
		case SHOP_HPMEM: snprintf(it->label, sizeof it->label, "HPMemory"); break;
		case SHOP_POWER: snprintf(it->label, sizeof it->label, "Attack+1"); it->disabled = run.atk >= 5; break;
		case SHOP_RAPID: snprintf(it->label, sizeof it->label, "Rapid+1"); it->disabled = run.rapid >= 5; break;
		case SHOP_CHARGE: snprintf(it->label, sizeof it->label, "Charge+1"); it->disabled = run.charge >= 5; break;
		case SHOP_LEAVE: snprintf(it->label, sizeof it->label, "Leave"); break;
		default: break;
		}
		if (shop_prices[i] < 0) { snprintf(it->detail, sizeof it->detail, "-"); it->disabled = true; }
		else if (shop_kind[i] != SHOP_LEAVE) snprintf(it->detail, sizeof it->detail, "%dz", shop_prices[i]);
	}
	char title[40];
	snprintf(title, sizeof title, "Net Dealer  %dz", run.zenny);
	ui_menu(title, shop_items, shop_n, true, shop_done);
}

/* Chip Trader: three chips in, one better chip out. */
static void trader_pick(int choice);

static void trader_menu(void) {
	static bool marked[FOLDER_MAX];
	memset(marked, 0, sizeof marked);
	for (int k = 0; k < N.trade_n; ++k) marked[N.trade_pick[k]] = true;
	folder_arrays();
	char title[24];
	snprintf(title, sizeof title, "Trade %d more", 3 - N.trade_n);
	ui_folder(title, fold_ids, fold_codes, run.folder_n, marked, true, trader_pick);
}

static void trader_pick(int choice) {
	if (choice < 0) { N.trade_n = 0; ui_message("Chip Trader: Maybe next time.", -1); return; }
	N.trade_pick[N.trade_n++] = choice;
	if (N.trade_n < 3) { trader_menu(); return; }
	/* Remove the three chips, highest index first, then pay out. */
	int idx[3] = { N.trade_pick[0], N.trade_pick[1], N.trade_pick[2] };
	for (int a = 0; a < 3; ++a)
		for (int b = a + 1; b < 3; ++b)
			if (idx[b] > idx[a]) { int t = idx[a]; idx[a] = idx[b]; idx[b] = t; }
	for (int k = 0; k < 3; ++k) {
		memmove(&run.folder[idx[k]], &run.folder[idx[k] + 1], sizeof(FolderChip) * (size_t)(run.folder_n - idx[k] - 1));
		run.folder_n--;
	}
	N.trade_n = 0;
	layer.obj[N.active_obj].used = true;
	char code;
	int id = roll_chip(run.depth + 4, 2, &code);
	ui_message("Chip Trader: *whirr*... *clunk*!", -1);
	give_chip(id, code);
}

static void bug_done(int choice) {
	static const int cost[3] = { 20, 8, 12 };
	if (choice < 0 || choice > 2) return;
	if (run.bugfrags < cost[choice]) { ui_message("Not enough BugFrags.", -1); return; }
	run.bugfrags -= cost[choice];
	char code;
	switch (choice) {
	case 0: { int id = roll_chip(run.depth + 6, 3, &code); give_chip(id, code); break; }
	case 1: run.unlockers++; ui_message("Got an Unlocker.", -1); break;
	default: run.max_hp += 20; run.hp += 20; ui_message("Max HP +20!", -1); break;
	}
}

static uint32_t prog_offer[3];

static void prog_done(int choice) {
	if (choice < 0 || choice > 2 || !prog_offer[choice]) return;
	int price = 3000 + run.depth * 100;
	if (run.zenny < price) { ui_message("Not enough zenny.", -1); return; }
	run.zenny -= price;
	give_perk(prog_offer[choice]);
	prog_offer[choice] = 0;
}

/* ------------------------------------------------------------------ */
/* Interaction */

static const char *npc_lines[] = {
	"Purple Mystery Data won't open without an Unlocker.",
	"They say the Graveyard hides warps to the Undernet...",
	"Viruses gather where the panels glow red. Dangerous, but the data there is worth it.",
	"Three fragments of Secret Data open a gate in the Undernet. Nobody knows what waits inside.",
	"Chips that share a code can be sent together. Stack them!",
	"Hold B to charge your buster. A charged shot can make a Navi flinch.",
	"Every third area, a Navi guards the exit. Beat them and they'll lend you their power.",
	"Chip Traders swap three chips for one. Great for clearing out junk.",
	"The Custom gauge is full when it flashes. Press L or R to pick new chips.",
	"Beyond the Undernet lies the Cybeast's nest. It feeds on the whole net...",
	"Programs from the NaviCust vendor stay installed for the whole run.",
	"AreaGrab steals the enemy's front column. More room to move, less room to hide!",
};

static void confirm_boss(int choice);
static void confirm_undernet(int choice);
static void confirm_secret(int choice);
static void confirm_challenge(int choice);

static const UiItem yes_no[2] = { { "Yes", "", -1, 0, false }, { "No", "", -1, 0, false } };

static void interact(NetObj *o) {
	N.active_obj = (int)(o - layer.obj);
	switch (o->type) {
	case OBJ_MYSTERY:
		if (!o->used) open_mystery(o);
		break;
	case OBJ_SHOP:
		if (!o->used) { shop_stock(o); o->used = true; }
		ui_message("Mr. Prog: Welcome to the Net Dealer!", -1);
		shop_open();
		break;
	case OBJ_HEAL:
		if (o->used) { ui_message("The recovery program is spent.", -1); break; }
		o->used = true;
		run.hp = run.max_hp;
		ui_message("Recovery program ran. MegaMan's HP is full!", -1);
		audio_sfx(SFX_RECOVER);
		break;
	case OBJ_TRADER:
		if (o->used) { ui_message("The Chip Trader is out of order.", -1); break; }
		if (run.folder_n < 12) { ui_message("Chip Trader: You need at least 12 chips to trade.", -1); break; }
		N.trade_n = 0;
		trader_menu();
		break;
	case OBJ_BUGTRADER: {
		static UiItem items[3];
		memset(items, 0, sizeof items);
		snprintf(items[0].label, sizeof items[0].label, "Rare chip");
		snprintf(items[0].detail, sizeof items[0].detail, "20 BF");
		snprintf(items[1].label, sizeof items[1].label, "Unlocker");
		snprintf(items[1].detail, sizeof items[1].detail, "8 BF");
		snprintf(items[2].label, sizeof items[2].label, "HPMemory");
		snprintf(items[2].detail, sizeof items[2].detail, "12 BF");
		for (int i = 0; i < 3; ++i) items[i].chip = -1;
		char title[40];
		snprintf(title, sizeof title, "BugFrag Trader  %d BF", run.bugfrags);
		ui_menu(title, items, 3, true, bug_done);
		break;
	}
	case OBJ_PROGRAMS: {
		static UiItem items[3];
		if (!o->used) {
			o->used = true;
			for (int i = 0; i < 3; ++i) {
				prog_offer[i] = random_perk();
				for (int k = 0; k < i; ++k) if (prog_offer[k] == prog_offer[i]) prog_offer[i] = 0;
			}
		}
		for (int i = 0; i < 3; ++i) {
			memset(&items[i], 0, sizeof items[i]);
			items[i].chip = -1;
			if (prog_offer[i]) {
				snprintf(items[i].label, sizeof items[i].label, "%s", perk_name(prog_offer[i]));
				snprintf(items[i].detail, sizeof items[i].detail, "%dz", 3000 + run.depth * 100);
			} else {
				snprintf(items[i].label, sizeof items[i].label, "---");
				items[i].disabled = true;
			}
		}
		ui_message("NaviCust vendor: Programs stay installed for the whole run.", -1);
		ui_menu("Programs", items, 3, true, prog_done);
		break;
	}
	case OBJ_BOSS: {
		if (layer.boss_beaten) break;
		char name[24];
		navi_name(o->param, name, sizeof name);
		ui_messagef(-1, "%s: So you made it this far. The way forward is through me!", name);
		ui_menu("Fight?", yes_no, 2, true, confirm_boss);
		break;
	}
	case OBJ_EXIT:
	case OBJ_RETURN:
		if (layer.boss_layer && !layer.boss_beaten) {
			ui_message(o->type == OBJ_EXIT ? "The warp is sealed. Its guardian must be deleted first." : "A presence blocks the way out...", -1);
			break;
		}
		N.fade = -30;
		audio_sfx(SFX_WARP);
		break;
	case OBJ_UNDERNET:
		ui_message("A dark warp hums here. The Undernet lies beyond. Its exit leads one area deeper.", -1);
		ui_menu("Enter the Undernet?", yes_no, 2, true, confirm_undernet);
		break;
	case OBJ_SECRET_GATE:
		if (run.fragments < 3) { ui_messagef(-1, "A sealed gate. %d of 3 Secret Data fragments.", run.fragments); break; }
		ui_message("The three fragments resonate. The gate opens...", -1);
		ui_menu("Enter the Secret Area?", yes_no, 2, true, confirm_secret);
		break;
	case OBJ_CHALLENGE:
		if (o->used) { ui_message("The signal has gone quiet.", -1); break; }
		ui_message("A powerful virus signal. Take it on for a rare reward?", -1);
		ui_menu("Challenge?", yes_no, 2, true, confirm_challenge);
		break;
	case OBJ_NPC:
		ui_message(npc_lines[o->npc_line % (int)(sizeof npc_lines / sizeof *npc_lines)], -1);
		break;
	default:
		break;
	}
}

static void confirm_boss(int choice) {
	if (choice != 0) return;
	N.boss_fight = true;
	Encounter e = make_boss(run.depth, run.biome, layer.boss_navi);
	begin_battle(&e);
}

static bool side_pending;

static void confirm_undernet(int choice) {
	if (choice != 0) return;
	run.side_kind = LAYER_UNDERNET;
	side_pending = true;
	N.fade = -30;
}

static void confirm_secret(int choice) {
	if (choice != 0) return;
	run.side_kind = LAYER_SECRET;
	side_pending = true;
	N.fade = -30;
}

static void confirm_challenge(int choice) {
	if (choice != 0) return;
	N.challenge_fight = true;
	layer.obj[N.active_obj].used = true;
	Encounter e = make_encounter(run.depth + 3, run.biome, true, true);
	begin_battle(&e);
}

/* ------------------------------------------------------------------ */
/* Battle results */

static void on_battle(const BattleResult *r) {
	scene_set(&scene_net);
	N.since_battle = 0;
	if (r->outcome == BATTLE_LOSE) {
		save_delete();
		profile_record_run();
		N.ready = false;
		scene_set(&scene_gameover);
		return;
	}
	audio_music_id(biome_song(run.biome));
	N.return_fade = 16;
	N.to_battle = false;
	if (N.boss_fight) {
		N.boss_fight = false;
		layer.boss_beaten = true;
		run.bosses_beaten++;
		const NaviDef *d = navi_def(layer.boss_navi);
		char name[24];
		navi_name(layer.boss_navi, name, sizeof name);
		ui_messagef(-1, "%s deleted! The warp is open.", name);
		if (r->took_reward) give_chip(r->reward.chip, r->reward.code);
		run.max_hp += 40;
		run.hp = run.hp + 40 > run.max_hp ? run.max_hp : run.hp + 40;
		ui_message("Max HP +40.", -1);
		if (d->cross && !(run.crosses & (1u << layer.boss_navi))) {
			run.crosses |= 1u << layer.boss_navi;
			ui_messagef(-1, "%s's power flows into MegaMan: %sCross! Press SELECT in the Custom screen to change style.", name, d->cross);
		}
		if (run.biome == BIOME_GRAVEYARD && !run.beast_out) {
			run.beast_out = true;
			ui_message("Something ancient stirs in MegaMan's data... Beast Out unlocked! Pick it with SELECT in the Custom screen.", -1);
		}
		if (run.side_kind == LAYER_SECRET) {
			run.secret_cleared = true;
			give_perk(random_perk());
		}
		if (run.biome == BIOME_NEST)
			ui_message("The Cybeast's nest falls silent... but the net goes on forever. How deep can you go?", -1);
		checkpoint();
		return;
	}
	int bonus = 0;
	if (N.challenge_fight) {
		N.challenge_fight = false;
		bonus = 2;
		if (run.fragments < 3 && !run.secret_cleared && rng_range(0, 99) < 40) {
			run.fragments++;
			ui_messagef(-1, "The virus left a Secret Data fragment (%d/3)!", run.fragments);
		}
	}
	(void)bonus;
	if (rng_range(0, 99) < 20) run.bugfrags += rng_range(1, 3);
	if (r->took_reward) {
		if (r->reward.chip >= 0) give_chip(r->reward.chip, r->reward.code);
		else { run.zenny += r->reward.zenny; ui_messagef(-1, "Got %d zenny.", r->reward.zenny); }
	}
}

/* ------------------------------------------------------------------ */
/* Pause menu */

static void pause_done(int choice);

static void folder_view(void) {
	folder_arrays();
	ui_folder("Folder", fold_ids, fold_codes, run.folder_n, NULL, true, NULL);
}

static void options_done(int choice);

static void options_open(void) {
	static UiItem items[3];
	memset(items, 0, sizeof items);
	snprintf(items[0].label, sizeof items[0].label, "Music");
	snprintf(items[0].detail, sizeof items[0].detail, "%d", profile.music_volume - 1);
	snprintf(items[1].label, sizeof items[1].label, "Effects");
	snprintf(items[1].detail, sizeof items[1].detail, "%d", profile.sfx_volume - 1);
	snprintf(items[2].label, sizeof items[2].label, "Done");
	for (int i = 0; i < 3; ++i) items[i].chip = -1;
	ui_menu("Options: A raises, wraps at 10", items, 3, true, options_done);
}

static void options_done(int choice) {
	if (choice == 0) profile.music_volume = profile.music_volume % 11 + 1;
	else if (choice == 1) profile.sfx_volume = profile.sfx_volume % 11 + 1;
	else { profile_save(); return; }
	audio_set_volume(profile.music_volume - 1, profile.sfx_volume - 1);
	audio_sfx(SFX_ITEM);
	options_open();
}

static int pause_item;

static void pause_open(void) {
	PetInfo info = { run.hp, run.max_hp, run.zenny, run.bugfrags, "" };
	snprintf(info.place, sizeof info.place, "%.31s", N.banner_text);
	ui_pet(&info, pause_item, pause_done);
}

static void save_quit_done(int choice) {
	if (choice != 0) { pause_open(); return; }
	checkpoint();
	N.ready = false;
	scene_set(&scene_title);
}

/* PET items: ChipFolder, SubChip (map), Library, MegaMan, E-Mail (tips),
 * KeyItem, Comm (options), Save. */
static void pause_done(int choice) {
	if (choice < 0) return;
	pause_item = choice;
	switch (choice) {
	case 0: folder_view(); break;
	case 1: N.map_open = 1; break;
	case 2: ui_message("The Library has no signal this deep in the Cyberworld.", -1); break;
	case 3: {
		char perks[160] = "";
		for (int i = 0; i < PERK_COUNT_BITS; ++i)
			if (run.perks & (1u << i)) {
				strncat(perks, perk_name(1u << i), sizeof perks - strlen(perks) - 1);
				strncat(perks, " ", sizeof perks - strlen(perks) - 1);
			}
		ui_messagef(-1, "MegaMan: HP %d/%d. Buster: Attack %d, Rapid %d, Charge %d.", run.hp, run.max_hp, run.atk, run.rapid, run.charge);
		ui_messagef(-1, "Programs: %s", perks[0] ? perks : "none");
		break;
	}
	case 4: {
		/* E-Mail: two tips from the net's gossip */
		int a = rng_range(0, (int)(sizeof npc_lines / sizeof *npc_lines) - 1);
		ui_messagef(-1, "E-Mail: %s", npc_lines[a]);
		ui_messagef(-1, "E-Mail: %s", npc_lines[(a + 5) % (int)(sizeof npc_lines / sizeof *npc_lines)]);
		break;
	}
	case 5:
		ui_messagef(-1, "KeyItems: Unlocker x%d. Secret Data %d/3.%s", run.unlockers, run.fragments, run.beast_out ? " Gregar's power slumbers within." : "");
		break;
	case 6: options_open(); break;
	case 7: {
		static const UiItem yes_no2[2] = { { "Save & quit", "", -1, 0, false }, { "Back", "", -1, 0, false } };
		ui_message("The run is saved at every warp. Save now and return to the title?", -1);
		ui_menu("Save", yes_no2, 2, true, save_quit_done);
		break;
	}
	default: break;
	}
}

/* ------------------------------------------------------------------ */
/* Update */

static int dir_from(float dx, float dy) {
	/* World delta -> the sprite's 8 screen directions (0 = up, clockwise). */
	float sx = dx - dy, sy = (dx + dy) * 0.5f;
	float a = atan2f(sy, sx);
	int oct = (int)floorf((a + (float)M_PI / 8) / ((float)M_PI / 4));
	oct = ((oct % 8) + 8) % 8; /* 0 = right, 2 = down */
	static const int map[8] = { 2, 3, 4, 5, 6, 7, 0, 1 };
	return map[oct];
}

static NetObj *nearby_object(void) {
	NetObj *best = NULL;
	float bd = 1.15f;
	for (int i = 0; i < layer.nobj; ++i) {
		NetObj *o = &layer.obj[i];
		if (o->type == OBJ_WARP_IN) continue;
		if (o->type == OBJ_MYSTERY && o->used) continue;
		if (o->type == OBJ_BOSS && layer.boss_beaten) continue;
		float dx = o->x - N.px, dy = o->y - N.py;
		float d = sqrtf(dx * dx + dy * dy);
		if (d < bd) { bd = d; best = o; }
	}
	return best;
}

void net_reset(void) { N.ready = false; }

static void enter(void) {
	if (!N.ready) {
		memset(&N, 0, sizeof N);
		N.ready = true;
		anim_play(&N.mm, sprite_get(SPR_NPC, MM_SPRITE), 4);
		ui_clear();
		start_layer();
	}
}

static void update(void) {
	++N.tick;
	if (N.banner > 0) --N.banner;
	if (N.return_fade > 0) --N.return_fade;
	if (N.fade > 0) --N.fade;
	if (N.fade < 0) {
		if (++N.fade == 0) {
			if (side_pending) { side_pending = false; start_layer(); }
			else next_layer();
		}
		return;
	}
	if (N.flash > 0) {
		if (--N.flash == 0 && N.pending == PEND_BATTLE) {
			N.pending = PEND_NONE;
			N.to_battle = true;
			battle_begin(&N.enc, on_battle);
		}
		return;
	}
	if (N.map_open) {
		if (btn_pressed(BTN_A) || btn_pressed(BTN_B) || btn_pressed(BTN_START) || btn_pressed(BTN_SELECT)) N.map_open = 0;
		return;
	}
	if (ui_active()) { ui_update(); return; }
	if (btn_pressed(BTN_START)) { pause_open(); return; }
	if (btn_pressed(BTN_SELECT)) { N.map_open = 1; return; }

	/* The pad follows the isometric axes like the original games:
	 * Up = up-right, Right = down-right; diagonals move along the screen. */
	float ix = 0, iy = 0;
	if (btn_held(BTN_UP)) iy -= 1;
	if (btn_held(BTN_DOWN)) iy += 1;
	if (btn_held(BTN_LEFT)) ix -= 1;
	if (btn_held(BTN_RIGHT)) ix += 1;
	bool running = btn_held(BTN_B);
	N.moving = ix != 0 || iy != 0;
	if (N.moving) {
		float len = sqrtf(ix * ix + iy * iy);
		float spd = running ? 0.105f : 0.07f;
		float dx = ix / len * spd, dy = iy / len * spd;
		N.dir = dir_from(dx, dy);
		float ox = N.px, oy = N.py;
		if (!blocked(N.px + dx, N.py)) N.px += dx;
		if (!blocked(N.px, N.py + dy)) N.py += dy;
		N.walked += fabsf(N.px - ox) + fabsf(N.py - oy);
		int want = (running ? 16 : 8) + N.dir;
		if (N.mm.anim != want) anim_play(&N.mm, N.mm.spr, want);
		/* Random encounters, like the original's step counter. */
		if (N.walked >= 1.0f) {
			N.walked -= 1.0f;
			N.since_battle++;
			int cx = (int)N.px, cy = (int)N.py;
			bool corrupt = layer.corrupt[cy][cx];
			int chance = encounter_chance(corrupt);
			if (rng_range(0, 99) < chance) {
				Encounter e = make_encounter(run.depth, run.biome, corrupt, false);
				begin_battle(&e);
			}
		}
	} else if (N.mm.anim != N.dir) anim_play(&N.mm, N.mm.spr, N.dir);
	anim_update(&N.mm);
	if (N.mm.done) anim_play(&N.mm, N.mm.spr, N.mm.anim);

	for (int y = (int)N.py - 5; y <= (int)N.py + 5; ++y)
		for (int x = (int)N.px - 5; x <= (int)N.px + 5; ++x)
			if (x >= 0 && y >= 0 && x < MAP_W && y < MAP_H) layer.seen[y][x] = 1;

	if (btn_pressed(BTN_A)) {
		NetObj *o = nearby_object();
		if (o) interact(o);
	}
}

/* ------------------------------------------------------------------ */
/* Drawing */

static void draw_background(void) {
	const BiomeLook *L = &looks[layer.biome];
	for (int y = 0; y < P.h; y += 4) {
		int t = y * 255 / P.h;
		fill_rect(0, y, P.w, 4, rgba((L->bg0[0] * (255 - t) + L->bg1[0] * t) / 255, (L->bg0[1] * (255 - t) + L->bg1[1] * t) / 255,
			(L->bg0[2] * (255 - t) + L->bg1[2] * t) / 255, 255));
	}
	SDL_Color g = rgba(L->grid[0], L->grid[1], L->grid[2], 50);
	int off = (int)(N.tick / 2) % 32;
	int ox = ((int)cam_x / 3) % 32, oy = ((int)cam_y / 3) % 32;
	for (int x = -32 + off - ox; x < P.w + 32; x += 32) fill_rect(x, 0, 1, P.h, g);
	for (int y = -32 + off / 2 - oy; y < P.h + 32; y += 32) fill_rect(0, y, P.w, 1, g);
	for (int i = 0; i < 24; ++i) {
		int x = (int)((uint32_t)(i * 97) + N.tick / 3 * (uint32_t)(1 + i % 3)) % (P.w + 20) - 10;
		int y = (i * 53 + i * i * 7) % P.h;
		fill_rect(x, y, 2, 2, rgba(L->grid[0], L->grid[1], L->grid[2], 120));
	}
}

typedef struct { int y, kind, idx; } Drawable;

static void draw_obj(NetObj *o) {
	int sx, sy;
	iso(o->x, o->y, &sx, &sy);
	if (sx < -60 || sx > P.w + 60 || sy < -80 || sy > P.h + 60) return;
	Sprite *s = NULL;
	int pal = 0, anim = 0;
	switch (o->type) {
	case OBJ_WARP_IN: s = sprite_get(SPR_OBJECT, 0x06); break;
	case OBJ_EXIT: s = sprite_get(SPR_OBJECT, 0x22); break;
	case OBJ_RETURN: s = sprite_get(SPR_OBJECT, 0x81); break;
	case OBJ_UNDERNET: s = sprite_get(SPR_OBJECT, 0x44); break;
	case OBJ_SECRET_GATE: s = sprite_get(SPR_OBJECT, 0x36); break;
	case OBJ_MYSTERY: if (o->used) return; s = sprite_get(SPR_OBJECT, 0x5F); pal = o->param; break;
	case OBJ_SHOP: s = sprite_get(SPR_NPC, 0x3C); anim = 4; break;
	case OBJ_PROGRAMS: s = sprite_get(SPR_NPC, 0x5D); anim = 4; break;
	case OBJ_HEAL: s = sprite_get(SPR_OBJECT, 0x73); break;
	case OBJ_TRADER: s = sprite_get(SPR_OBJECT, 0x5C); break;
	case OBJ_BUGTRADER: s = sprite_get(SPR_NPC, 0x43); anim = 4; break;
	case OBJ_CHALLENGE: s = sprite_get(SPR_OBJECT, 0x9B); break;
	case OBJ_NPC: {
		static const int npcs[6] = { 0x2A, 0x2B, 0x2E, 0x3D, 0x3E, 0x40 };
		s = sprite_get(SPR_NPC, npcs[o->param % 6]);
		anim = 4;
		break;
	}
	case OBJ_BOSS:
		if (layer.boss_beaten) return;
		s = sprite_get(SPR_NAVI, o->param);
		break;
	default: break;
	}
	if (!s) return;
	if (anim >= sprite_anim_count(s)) anim = 0;
	int frames = sprite_frame_count(s, anim);
	int f = frames > 1 ? (int)(N.tick / 8) % frames : 0;
	int fx = (o->type == OBJ_EXIT && layer.boss_layer && !layer.boss_beaten) ? FX_DARK : 0;
	sprite_draw_frame(s, anim, f, sx, sy + 8, o->type == OBJ_BOSS, pal, fx);
}

static void draw_world(void) {
	if (!tile_tex[layer.biome][0]) {
		tile_tex[layer.biome][0] = build_tile(layer.biome, false);
		tile_tex[layer.biome][1] = build_tile(layer.biome, true);
	}
	/* Tiles back to front along the diagonals. */
	for (int s = 0; s < MAP_W + MAP_H; ++s) {
		for (int y = 0; y < MAP_H; ++y) {
			int x = s - y;
			if (x < 0 || x >= MAP_W || layer.cell[y][x] != C_PATH) continue;
			int sx, sy;
			iso((float)x, (float)y, &sx, &sy);
			if (sx < -TILE_W || sx > P.w + TILE_W || sy < -TILE_H * 2 || sy > P.h + TILE_H) continue;
			/* iso() of a cell's origin is the diamond's top corner */
			SDL_Rect d = { sx - TILE_W / 2, sy, TILE_W, TILE_H + SLAB };
			SDL_Texture *t = tile_tex[layer.biome][layer.corrupt[y][x] ? 1 : 0];
			if (layer.corrupt[y][x]) {
				int pulse = 200 + (int)(55 * sinf((float)N.tick * 0.08f + (float)(x + y)));
				SDL_SetTextureColorMod(t, 255, (Uint8)pulse, (Uint8)pulse);
			}
			SDL_RenderCopy(P.renderer, t, NULL, &d);
		}
	}
	/* Sprites sorted by depth. */
	Drawable list[MAX_OBJS + 1];
	int n = 0;
	for (int i = 0; i < layer.nobj; ++i) list[n++] = (Drawable){ (int)((layer.obj[i].x + layer.obj[i].y) * 100), 0, i };
	list[n++] = (Drawable){ (int)((N.px + N.py) * 100), 1, 0 };
	for (int i = 1; i < n; ++i) {
		Drawable t = list[i];
		int j = i - 1;
		while (j >= 0 && list[j].y > t.y) { list[j + 1] = list[j]; --j; }
		list[j + 1] = t;
	}
	for (int i = 0; i < n; ++i) {
		if (list[i].kind == 0) draw_obj(&layer.obj[list[i].idx]);
		else {
			int sx, sy;
			iso(N.px, N.py, &sx, &sy);
			fill_rect(sx - 5, sy + 7, 10, 2, rgba(0, 0, 0, 90));
			anim_draw(&N.mm, sx, sy + 8, false, 0, 0);
		}
	}
	if (!ui_active()) {
		NetObj *o = nearby_object();
		if (o) {
			int sx, sy;
			iso(o->x, o->y, &sx, &sy);
			if ((N.tick / 16) & 1) text_draw(sx, sy - 40, "A", rgba(255, 240, 120, 255), TEXT_CENTER);
		}
	}
}

static void draw_map(void) {
	fill_rect(0, 0, P.w, P.h, rgba(0, 8, 24, 225));
	int scale = (P.h - 24) / MAP_H;
	if (scale < 2) scale = 2;
	int ox = (P.w - MAP_W * scale) / 2, oy = 16;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (layer.cell[y][x] != C_PATH || !layer.seen[y][x]) continue;
			fill_rect(ox + x * scale, oy + y * scale, scale, scale, layer.corrupt[y][x] ? rgba(200, 60, 80, 255) : rgba(80, 180, 255, 255));
		}
	for (int i = 0; i < layer.nobj; ++i) {
		NetObj *o = &layer.obj[i];
		if (!layer.seen[(int)o->y][(int)o->x]) continue;
		SDL_Color c = WHITE;
		switch (o->type) {
		case OBJ_EXIT: case OBJ_RETURN: c = rgba(80, 255, 120, 255); break;
		case OBJ_MYSTERY:
			if (o->used) continue;
			c = o->param == 0 ? rgba(80, 255, 80, 255) : o->param == 1 ? rgba(80, 160, 255, 255) : rgba(200, 80, 255, 255);
			break;
		case OBJ_BOSS: if (layer.boss_beaten) continue; c = rgba(255, 60, 60, 255); break;
		case OBJ_SHOP: case OBJ_PROGRAMS: case OBJ_TRADER: case OBJ_BUGTRADER: c = rgba(255, 220, 80, 255); break;
		case OBJ_UNDERNET: case OBJ_SECRET_GATE: c = rgba(200, 80, 255, 255); break;
		default: break;
		}
		fill_rect(ox + (int)o->x * scale - 1, oy + (int)o->y * scale - 1, scale + 2, scale + 2, c);
	}
	if ((N.tick / 8) & 1) fill_rect(ox + (int)N.px * scale - 1, oy + (int)N.py * scale - 1, scale + 2, scale + 2, WHITE);
	text_draw(P.w / 2, 0, N.banner_text, WHITE, TEXT_CENTER);
}

static void draw_hud(void) {
	int x = P.core_x, y = P.core_y;
	fill_rect(x + 2, y + 2, 62, 14, rgba(24, 40, 72, 230));
	draw_rect(x + 2, y + 2, 62, 14, rgba(160, 200, 255, 255));
	text_drawf(x + 5, y + 1, run.hp * 4 < run.max_hp ? rgba(255, 120, 90, 255) : WHITE, TEXT_LEFT, "%d/%d", run.hp, run.max_hp);
	char z[24];
	snprintf(z, sizeof z, "%dz", run.zenny);
	int zw = text_width(z) + 8;
	fill_rect(x + CORE_W - zw - 2, y + 2, zw, 14, rgba(24, 40, 72, 230));
	text_draw(x + CORE_W - 6, y + 1, z, rgba(255, 230, 90, 255), TEXT_RIGHT);
	/* Virus signal: how close the next encounter feels. */
	int cx = (int)N.px, cy = (int)N.py;
	int danger = encounter_chance(layer.corrupt[cy][cx]) * 2;
	if (danger > 60) danger = 60;
	SDL_Color dc = danger < 12 ? rgba(80, 220, 120, 255) : danger < 30 ? rgba(255, 210, 60, 255) : rgba(255, 70, 70, 255);
	fill_rect(x + 2, y + 17, 62, 3, rgba(0, 0, 0, 160));
	fill_rect(x + 2, y + 17, 2 + danger, 3, dc);
	text_drawf(x + 4, y + CORE_H - 14, rgba(200, 220, 255, 255), TEXT_LEFT, "%s", N.banner_text);
	char depth[16];
	snprintf(depth, sizeof depth, "B%d", run.depth);
	text_draw(x + CORE_W - 4, y + CORE_H - 14, depth, rgba(200, 220, 255, 255), TEXT_RIGHT);
	if (run.fragments > 0 && !run.secret_cleared) text_drawf(x + CORE_W - 4, y + 18, rgba(220, 150, 255, 255), TEXT_RIGHT, "Frag %d/3", run.fragments);
}

static void draw(void) {
	cam_x = (N.px - N.py) * (TILE_W / 2);
	cam_y = (N.px + N.py) * (TILE_H / 2) - 8;
	draw_background();
	draw_world();
	if (!ui_fullscreen()) draw_hud();
	if (N.banner > 0 && N.banner < 140 && !ui_fullscreen()) {
		int a = N.banner > 30 ? 255 : N.banner * 8;
		fill_rect(0, P.core_y + 44, P.w, 20, rgba(10, 20, 60, a * 3 / 4));
		text_draw(P.w / 2, P.core_y + 46, N.banner_text, rgba(255, 255, 255, a), TEXT_CENTER);
	}
	if (N.map_open) draw_map();
	ui_draw();
	if (N.flash > 0 && N.flash < 16) {
		/* The original's encounter: horizontal mosaic and fade to white, 16 frames */
		P.fx_mosaic = 17 - N.flash;
		P.fx_fade = 16 - N.flash;
		P.fx_fade_color = WHITE;
	}
	if (N.to_battle) { P.fx_fade = 16; P.fx_fade_color = WHITE; }
	if (N.return_fade > 0) { P.fx_fade = N.return_fade; P.fx_fade_color = BLACK; }
	if (N.fade != 0) {
		int a = N.fade > 0 ? N.fade * 255 / 30 : (30 + N.fade) * 255 / 30;
		fill_rect(0, 0, P.w, P.h, rgba(255, 255, 255, a));
	}
}

const Scene scene_net = { "net", enter, update, draw, NULL };
