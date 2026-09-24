/* Layer objects on the game's own NPCs, Mystery Data and text scripts. */
#include "layer_objs.h"

#include <stdio.h>

#include "bn6.h"
#include "chip_pool.h"
#include "data.h"
#include "debug.h"
#include "emu.h"
#include "flags.h"
#include "game.h"
#include "loot.h"
#include "mapslot.h"
#include "net.h"
#include "netmap.h"
#include "stage_npc.h"
#include "npc.h"
#include "npc_lines.h"
#include "rom.h"
#include "run.h"
#include "save.h"
#include "scripts.h"
#include "shop.h"
#include "trader.h"

/* Overworld objects (sprite list 7) */
#define SPR_EXIT_PAD    0x22
#define SPR_RETURN_PAD  0x81
#define SPR_GATE        0x36
#define SPR_DARK_WARP   0x44
#define SPR_SERVER      0x9B
#define SPR_CHIP_TRADER 0x5C
/* Mr. Progs and navis (sprite list 6) */
#define SPR_PROG        60
#define SPR_PROG_BLUE   93

#define FRAGMENT_CHANCE 35   /* % a deep layer hides a ScrtData */
#define SPECIAL_FROM    9    /* place in the cycle from which a Chip Trader may be a Special */
#define SPECIAL_CHANCE  40   /* % of those */

#define HP_MEMORY_CHANCE 15   /* % of the rich Mystery Data from the second act on */

/* The game's 8-byte Mystery Data content: kind 1 chip (code, id), 3 zenny,
 * 4 item, 5 BugFrags (tested in the game; see docs/ROM_DATA.md). True for
 * an HPMemory, which the game keeps in blue Mystery Data. */
static bool mystery_content(const NetObj *o, uint8_t out[8]) {
	int roll = rng_range(0, 99);
	char code = '*';
	int kind = 3, value = 100;
	if (o->param == 2 && run.depth >= 4 && rng_range(0, 99) < HP_MEMORY_CHANCE) {
		const uint8_t c[8] = { 4, 0x20, 0xFF, 0xFF, SCRIPTS_HP_MEMORY, 0, 0, 0 };
		for (int i = 0; i < 8; ++i) out[i] = c[i];
		return true;
	}
	if (o->param == 0) {
		if (roll < 50) { kind = 1; value = roll_chip(run.depth, 0, &code); }
		else if (roll < 85) value = (100 + rng_range(0, 8) * 50) * (1 + run.depth / 6);
		else { kind = 5; value = rng_range(3, 8); }
	} else if (o->param == 1) {
		if (roll < 60) { kind = 1; value = roll_chip(run.depth, 1, &code); }
		else value = 800 + run.depth * 60;
	} else {
		kind = 1;
		value = roll_chip(run.depth, 3, &code);
	}
	out[0] = (uint8_t)kind;
	out[1] = 0x20;
	out[2] = 0xFF;
	out[3] = (uint8_t)(kind == 1 ? (code == '*' ? 26 : code - 'A') : 0xFF);
	out[4] = (uint8_t)value;
	out[5] = (uint8_t)(value >> 8);
	out[6] = out[7] = 0;
	return false;
}

/* A ScrtData, as the game's key item Mystery Data hold one. */
static void fragment_content(uint8_t out[8]) {
	const uint8_t c[8] = { 4, 0x20, 0xFF, 0xFF, SCRIPTS_SECRET_DATA, 0, 0, 0 };
	for (int i = 0; i < 8; ++i) out[i] = c[i];
}

/* Compressed sprites only draw once the map loads them. */
static void need_sprite(NpcList *npcs, int category, int index) {
	uint32_t list = emu_read32(0x08000000u + R.layout->sprite_lists + (uint32_t)category * 4);
	if (!(emu_read32(list + (uint32_t)index * 4) & 0x80000000u)) return;
	for (int i = 0; i < npcs->nsprites; ++i)
		if (npcs->sprite_idx[i] == index && npcs->sprite_cat[i] == category * 4) return;
	if (npcs->nsprites >= 8) return;
	npcs->sprite_cat[npcs->nsprites] = (uint8_t)(category * 4);
	npcs->sprite_idx[npcs->nsprites++] = (uint8_t)index;
}

typedef struct {
	int x, y, z, cat, sprite, script, gone_flag;
	bool floor;
	uint32_t archive;   /* its text archive; 0: the layer's */
} Talker;

/* Overworld sprites (list 6) of the Navis Gregar has on the net, by navi
 * index; the others (Falzar's Navis are placeholders here) take the shape of
 * a HeelNavi. */
#define SPR_HEEL_NAVI 0x43

static int navi_sprite(int navi) {
	static const struct { uint8_t navi, sprite; } sprites[] = {
		{ 1, 0x47 }, { 2, 0x49 }, { 3, 0x4B }, { 4, 0x50 }, { 5, 0x4F },   /* Heat, Elec, Slash, Erase, Charge */
		{ 11, 0x3B }, { 13, 0x52 }, { 14, 0x54 }, { 15, 0x55 },            /* Proto, Dive, Circus, Judge */
		{ 18, 0x53 },                                                        /* Colonel */
	};
	for (unsigned i = 0; i < sizeof sprites / sizeof *sprites; ++i)
		if (sprites[i].navi == navi) return sprites[i].sprite;
	return SPR_HEEL_NAVI;
}

bool layer_objs_install(int group, int number, LayerObjs *out) {
	mapslot_reset();
	NpcList npcs = { { 0 }, 0, { 0 }, { 0 }, 0 };
	static TextArchive text;
	ta_begin(&text);
	Talker talkers[16];
	int ntalk = 0;
	MysteryData md[16];
	int nmd = 0;
	out->nchoices = 0;
	out->guardian.navi = 0;
	out->challenge_reward = -1;
	/* ScrtData lie in deep layers until three are out there */
	bool fragment = !run.secret_cleared && run.fragments < 3 &&
		(run.side_kind == LAYER_UNDERNET || run.depth >= 4) && rng_range(0, 99) < FRAGMENT_CHANCE;
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int wx, wy;
		netmap_world((int)o->x, (int)o->y, &wx, &wy);
		/* in a raised room, on its floor */
		int wz = layer.level[(int)o->y][(int)o->x] ? layer.rise : 0;
		Talker tk = { wx, wy, wz, 6, SPR_PROG, -1, -1, false, 0 };
		bool asks = false;   /* a Yes/No the director acts on */
		switch (o->type) {
		case OBJ_WARP_IN:
			out->start_x = wx; out->start_y = wy;
			break;
		case OBJ_EXIT:
		case OBJ_RETURN: {
			int pad = o->type == OBJ_EXIT ? SPR_EXIT_PAD : SPR_RETURN_PAD;
			out->exit_x = wx; out->exit_y = wy;
			/* the game's own warp pad: trigger cells taking warp 1 */
			CoordPad exit = { wx, wy, 1 };
			netmap_set_pads(&exit, 1);
			need_sprite(&npcs, 7, pad);
			/* a guardian's exit shows once the guardian is beaten */
			if (npcs.n < 32)
				npcs.script[npcs.n++] = layer.boss_layer ? npc_sealed_pad(7, pad, wx, wy, wz, LAYER_EXIT_OPEN_FLAG)
					: npc_prop(7, pad, wx, wy, wz, 0);
			break;
		}
		case OBJ_MYSTERY:
			if (nmd < 16 && npcs.n < 32) {
				md[nmd].x = wx;
				md[nmd].y = wy;
				md[nmd].z = wz;
				md[nmd].type = MYSTERY_GREEN;
				if (fragment) {
					fragment = false;
					md[nmd].type = MYSTERY_BLUE;
					fragment_content(md[nmd].content);
				} else if (mystery_content(o, md[nmd].content)) {
					md[nmd].type = MYSTERY_BLUE;
				}
				npcs.script[npcs.n++] = npc_mystery(nmd);
				++nmd;
			}
			break;
		case OBJ_NPC: {
			/* Normal Navis and pink navis */
			static const int navis[6] = { 62, 64, 65, 66, 69, 87 };
			tk.sprite = navis[o->param % 6];
			tk.script = ta_say(&text, -1, npc_line(o->npc_line));
			break;
		}
		case OBJ_HEAL: tk.script = ta_heal(&text); break;
		case OBJ_TRADER:
		case OBJ_BUGTRADER: {
			/* the game's own machine and lines; deeper, some are Specials */
			TraderKind kind = o->type == OBJ_BUGTRADER ? TRADER_BUGFRAG
				: (run.depth - 1) % CYCLE_LAYERS >= SPECIAL_FROM && run.layer_seed % 100 < SPECIAL_CHANCE ? TRADER_SPECIAL : TRADER_CHIPS;
			trader_install(group, number, kind, run.depth);
			tk.cat = 7; tk.sprite = SPR_CHIP_TRADER;
			tk.archive = BN6_TRADER_TEXT;
			tk.script = kind;
			break;
		}
		case OBJ_SHOP: tk.script = ta_shop(&text, SHOP_DEALER, "Welcome to the\nNet Dealer!"); break;
		case OBJ_PROGRAMS:
			tk.sprite = SPR_PROG_BLUE;
			tk.script = ta_shop(&text, SHOP_PROGRAMS, "NaviCust programs\nfor sale!");
			break;
		case OBJ_CHALLENGE: {
			asks = true; tk.cat = 7; tk.sprite = SPR_SERVER;
			/* a win pays with a chip a tier better than Mystery Data */
			char code = '*';
			ChipInfo ci;
			int chip = roll_chip(run.depth, 3, &code);
			chip_info(chip, &ci);
			out->challenge_reward = ta_challenge_reward(&text, chip, ci.name, code == '*' ? 26 : code - 'A');
			break;
		}
		case OBJ_GIFT: {
			char code = '*';
			ChipInfo ci;
			int chip = chip_pool_pick(2);
			if (chip <= 0) chip = roll_chip(run.depth, 3, &code);
			chip_info(chip, &ci);
			code = ci.ncodes ? ci.codes[rng_range(0, ci.ncodes - 1)] : '*';
			ShopItem program = { 3, 1, 0, 0, 0 };
			shop_pick_program(&program);
			/* a run lost before its first guardian earns a little more */
			bool comfort = profile.last_depth >= 1 && profile.last_depth <= 3;
			tk.script = ta_gift(&text, LAYER_GIFT_FLAG, comfort, chip, ci.name, code == '*' ? 26 : code - 'A', program.id, program.code);
			flag_clear(LAYER_GIFT_FLAG);
			break;
		}
		case OBJ_UNDERNET: asks = true; tk.cat = 7; tk.sprite = SPR_DARK_WARP; break;
		case OBJ_SECRET_GATE: asks = true; tk.cat = 7; tk.sprite = SPR_GATE; tk.floor = true; break;
		case OBJ_BOSS:
			/* the guardian's own actors and scripts (guardian_objs.c) */
			guardian_scripts(&text, o, wx, wy, wz, &out->guardian);
			break;
		default:
			break;
		}
		if (asks && out->nchoices < LAYER_MAX_CHOICES) {
			int flag = LAYER_FLAG_BASE + out->nchoices;
			out->choice[out->nchoices].type = o->type;
			out->choice[out->nchoices++].flag = flag;
			tk.script = o->type == OBJ_CHALLENGE ? ta_challenge(&text, flag)
				: o->type == OBJ_UNDERNET ? ta_undernet(&text, flag)
				: ta_secret_gate(&text, flag);
			/* not chosen yet */
			flag_clear(flag);
		}
		if (tk.script >= 0 && ntalk < 16) talkers[ntalk++] = tk;
	}
	/* the shops' stock, in the game's shop data */
	ShopItem stock[SHOP_MAX_ITEMS];
	int nstock = shop_dealer_stock(run.depth, stock);
	if (emu_debug_on())
		for (int i = 0; i < nstock; ++i)
			if (stock[i].kind == 2) {
				ChipInfo ci;
				chip_info(stock[i].id, &ci);
				fprintf(stderr, "shop chip %s %c %dz\n", ci.name, stock[i].code == 26 ? '*' : 'A' + stock[i].code, stock[i].price * 100);
			}
	shop_install(SHOP_DEALER, stock, nstock);
	shop_install(SHOP_PROGRAMS, stock, shop_program_stock(run.depth, stock));
	for (int i = 0; i < ntalk; ++i)
		if (talkers[i].cat == 7) need_sprite(&npcs, 7, talkers[i].sprite);
	uint32_t archive = text.n ? ta_commit(&text) : 0;
	out->archive = archive;
	if (out->guardian.navi) guardian_actors(&npcs, archive, navi_sprite(out->guardian.navi), &out->guardian);
	for (int i = 0; i < ntalk && npcs.n < 32; ++i)
		npcs.script[npcs.n++] = npc_talker(talkers[i].cat, talkers[i].sprite, talkers[i].x, talkers[i].y, talkers[i].z,
			talkers[i].cat == 7 ? 0 : 4, talkers[i].archive ? talkers[i].archive : archive, talkers[i].script,
			talkers[i].gone_flag, talkers[i].floor);
	return mapslot_install(group, number, &npcs, md, nmd);
}
