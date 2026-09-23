/* Layer objects on the game's own NPCs, Mystery Data and text scripts. */
#include "layer_objs.h"

#include "bn6.h"
#include "emu.h"
#include "game.h"
#include "loot.h"
#include "mapslot.h"
#include "net.h"
#include "netmap.h"
#include "npc.h"
#include "npc_lines.h"
#include "rom.h"
#include "run.h"
#include "scripts.h"
#include "shop.h"

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

/* The game's 8-byte Mystery Data content: kind 1 chip (code, id), 3 zenny,
 * 4 key item, 5 BugFrags (tested in the game; see docs/ROM_DATA.md). */
static void mystery_content(const NetObj *o, uint8_t out[8]) {
	int roll = rng_range(0, 99);
	char code = '*';
	int kind = 3, value = 100;
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

typedef struct { int x, y, cat, sprite, script; } Talker;

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
	/* ScrtData lie in deep layers until three are out there */
	bool fragment = !run.secret_cleared && run.fragments < 3 &&
		(run.side_kind == LAYER_UNDERNET || run.depth >= 4) && rng_range(0, 99) < FRAGMENT_CHANCE;
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int wx, wy;
		netmap_world((int)o->x, (int)o->y, &wx, &wy);
		Talker tk = { wx, wy, 6, SPR_PROG, -1 };
		int choice = -1;
		switch (o->type) {
		case OBJ_WARP_IN:
			out->start_x = wx; out->start_y = wy;
			break;
		case OBJ_EXIT:
		case OBJ_RETURN: {
			int pad = o->type == OBJ_EXIT ? SPR_EXIT_PAD : SPR_RETURN_PAD;
			out->exit_x = wx; out->exit_y = wy;
			need_sprite(&npcs, 7, pad);
			if (npcs.n < 32) npcs.script[npcs.n++] = npc_prop(7, pad, wx, wy, 0, 0);
			break;
		}
		case OBJ_MYSTERY:
			if (nmd < 16 && npcs.n < 32) {
				md[nmd].x = wx;
				md[nmd].y = wy;
				md[nmd].type = MYSTERY_GREEN;
				if (fragment) {
					fragment = false;
					md[nmd].type = MYSTERY_BLUE;
					fragment_content(md[nmd].content);
				} else {
					mystery_content(o, md[nmd].content);
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
		case OBJ_BUGTRADER: tk.script = ta_bug_trader(&text); break;
		case OBJ_TRADER:
			tk.cat = 7; tk.sprite = SPR_CHIP_TRADER;
			tk.script = ta_chip_trader(&text);
			break;
		case OBJ_SHOP: tk.script = ta_shop(&text, SHOP_DEALER, "Welcome to the\nNet Dealer!"); break;
		case OBJ_PROGRAMS:
			tk.sprite = SPR_PROG_BLUE;
			tk.script = ta_shop(&text, SHOP_PROGRAMS, "NaviCust programs\nfor sale!");
			break;
		case OBJ_CHALLENGE: choice = 1; tk.cat = 7; tk.sprite = SPR_SERVER; break;
		case OBJ_UNDERNET: choice = 1; tk.cat = 7; tk.sprite = SPR_DARK_WARP; break;
		case OBJ_SECRET_GATE: choice = 1; tk.cat = 7; tk.sprite = SPR_GATE; break;
		default:
			break;
		}
		if (choice >= 0 && out->nchoices < LAYER_MAX_CHOICES) {
			int flag = LAYER_FLAG_BASE + out->nchoices;
			out->choice[out->nchoices].type = o->type;
			out->choice[out->nchoices++].flag = flag;
			tk.script = o->type == OBJ_CHALLENGE ? ta_challenge(&text, flag)
				: o->type == OBJ_UNDERNET ? ta_undernet(&text, flag) : ta_secret_gate(&text, flag);
			/* not chosen yet */
			uint32_t fb = BN6_EVENT_FLAGS + (uint32_t)flag / 8u;
			emu_write8(fb, (uint8_t)(emu_read8(fb) & ~(0x80u >> (flag & 7))));
		}
		if (tk.script >= 0 && ntalk < 16) talkers[ntalk++] = tk;
	}
	/* the shops' stock, in the game's shop data */
	ShopItem stock[SHOP_MAX_ITEMS];
	shop_install(SHOP_DEALER, stock, shop_dealer_stock(run.depth, stock));
	shop_install(SHOP_PROGRAMS, stock, shop_program_stock(run.depth, stock));
	for (int i = 0; i < ntalk; ++i)
		if (talkers[i].cat == 7) need_sprite(&npcs, 7, talkers[i].sprite);
	uint32_t archive = text.n ? ta_commit(&text) : 0;
	for (int i = 0; i < ntalk && npcs.n < 32; ++i)
		npcs.script[npcs.n++] = npc_talker(talkers[i].cat, talkers[i].sprite, talkers[i].x, talkers[i].y, 0,
			talkers[i].cat == 7 ? 0 : 4, archive, talkers[i].script);
	return mapslot_install(group, number, &npcs, md, nmd);
}
