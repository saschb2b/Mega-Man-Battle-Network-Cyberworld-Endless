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
#include "guardians.h"
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
#define SPR_DEALER      62   /* a Normal Navi, the Net Dealer's keeper in the game */
#define SPR_TECH        66   /* the orange technician navi (shop 3's keeper) */

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

void layer_objs_shops(const LayerObjs *o) {
	shop_install(SHOP_DEALER, o->dealer, o->ndealer);
	shop_install(SHOP_PROGRAMS, o->programs, o->nprograms);
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
	for (int i = 0; i <= OBJ_GIFT; ++i) out->script_of[i] = -1;
	/* the element that answers this act: its guardian's weakness, else its
	 * viruses' (the Net Dealer stocks a chip of it and says so) */
	int counter = counter_element(run.biome, run.boss_order[run.biome]);
	/* ScrtData lie in deep layers until three are out there */
	int said = 0;   /* bystanders so far: each says another line */
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
			static int base;
			if (!said) base = o->npc_line;
			tk.sprite = navis[o->param % 6];
			/* (a list-6 navi's face has its sprite's number) */
			tk.script = ta_say(&text, tk.sprite, npc_line(run.depth, base + said++));
			break;
		}
		case OBJ_HEAL: tk.script = ta_heal(&text, o->npc_line + run.depth); break;
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
		case OBJ_SHOP: {
			/* (and a word on the element that answers this act, which the
			 * stock carries a chip of) */
			static const char *const elem[5] = { "", "Fire", "Aqua", "Elec", "Wood" };
			char hello[400], word[280] = "";
			int navi = run.boss_order[run.biome], ge = navi > 0 ? enemy_element(enemy_id(1, navi, 0)) : 0;
			/* (the guardian's weakness by name; an element-less guardian's
			 * act is answered for its viruses) */
			if (counter > 0 && ge > 0 && ge <= 4)
				snprintf(word, sizeof word, "|Word is, %s can't stand %s chips.|My best %s chip's first on the list, and I've got two!",
					guardian(navi)->name, elem[counter], elem[counter]);
			else if (navi > 0 && counter > 0)
				/* (no element to answer the guardian with: the hardest hit on
				 * the list, and the viruses' weakness besides) */
				snprintf(word, sizeof word, "|Word is, %s has no weak element. Hit hard: my hardest hitter's first on the list, and I've got two!|"
					"The viruses around here can't stand %s chips, though.", guardian(navi)->name, elem[counter]);
			else if (navi > 0)
				snprintf(word, sizeof word, "|Word is, %s has no weak element. Hit hard: my hardest hitter's first on the list, and I've got two!",
					guardian(navi)->name);
			snprintf(hello, sizeof hello, "%s%s", run.depth <= 3
				? "Welcome to the Net Dealer! Divers need chips, and I've got 'em!"
				: "Still diving, MegaMan? Stock up. It only gets tougher from here!", word);
			tk.sprite = SPR_DEALER;
			tk.script = ta_shop(&text, SHOP_DEALER, FACE_NAVI, hello);
			break;
		}
		case OBJ_PROGRAMS:
			tk.sprite = SPR_TECH;
			tk.script = ta_shop(&text, SHOP_PROGRAMS, FACE_TECH,
				"NaviCust programs, fresh from my workbench!|Install them in your PET: MegaMan, then NaviCust.");
			break;
		case OBJ_CHALLENGE: {
			asks = true; tk.cat = 7; tk.sprite = SPR_SERVER;
			/* a win pays with a chip a tier better than Mystery Data, the
			 * hardest hitting of a few (a WhiCapsl was a poor prize for
			 * the risk) */
			char code = '*';
			ChipInfo ci;
			int chip = roll_chip(run.depth, 3, &code), best_power = -1;
			for (int tries = 0; tries < 6; ++tries) {
				char c = '*';
				int id = tries ? roll_chip(run.depth, 3, &c) : chip;
				chip_info(id, &ci);
				if (ci.power > best_power) { best_power = ci.power; chip = id; if (tries) code = c; }
			}
			chip_info(chip, &ci);
			out->challenge_reward = ta_challenge_reward(&text, chip, ci.name, code == '*' ? 26 : code - 'A');
			break;
		}
		case OBJ_GIFT: {
			char code = '*';
			ChipInfo ci;
			/* a chip that hits hard (the tier's traps and supports are
			 * little help to a starting folder) */
			int chip = -1;
			for (int tries = 0; tries < 24; ++tries) {
				int c = chip_pool_pick(2);
				if (c <= 0) break;
				chip_info(c, &ci);
				chip = c;
				if (ci.power >= 60 && ci.ncodes) break;
			}
			if (chip <= 0) chip = roll_chip(run.depth, 3, &code);
			chip_info(chip, &ci);
			code = ci.ncodes ? ci.codes[rng_range(0, ci.ncodes - 1)] : '*';
			ShopItem program = { 3, 1, 0, 0, 0 };
			const char *about = shop_pick_gift_program(&program);
			/* a run lost before its first guardian earns a little more */
			bool comfort = profile.last_depth >= 1 && profile.last_depth <= 3;
			if (emu_debug_on()) fprintf(stderr, "gift: chip %d \"%s\" %c, program %d color %d\n", chip, ci.name, code, program.id, program.code);
			tk.script = ta_gift(&text, LAYER_GIFT_FLAG, comfort, chip, ci.name, ci.power, code == '*' ? 26 : code - 'A', program.id,
				program.code, about);
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
				: o->type == OBJ_UNDERNET ? ta_undernet(&text, flag, run.biome == BIOME_UNDERNET)
				: ta_secret_gate(&text, flag);
			/* not chosen yet */
			flag_clear(flag);
		}
		if (tk.script >= 0 && !tk.archive && out->script_of[o->type] < 0) out->script_of[o->type] = tk.script;
		if (tk.script >= 0 && ntalk < 16) talkers[ntalk++] = tk;
	}
	/* the shops' stock, in the game's shop data */
	ShopItem stock[SHOP_MAX_ITEMS];
	int navi_of_act = run.boss_order[run.biome], ge_of_act = navi_of_act > 0 ? enemy_element(enemy_id(1, navi_of_act, 0)) : 0;
	int nstock = shop_dealer_stock(run.depth, navi_of_act > 0 && !(ge_of_act > 0 && ge_of_act <= 4) ? -1 : counter, stock);
	if (emu_debug_on())
		for (int i = 0; i < nstock; ++i)
			if (stock[i].kind == 2) {
				ChipInfo ci;
				chip_info(stock[i].id, &ci);
				fprintf(stderr, "shop chip %s %c %dz\n", ci.name, stock[i].code == 26 ? '*' : 'A' + stock[i].code, stock[i].price * 100);
			}
	for (int i = 0; i < nstock; ++i) out->dealer[i] = stock[i];
	out->ndealer = nstock;
	out->nprograms = shop_program_stock(run.depth, out->programs);
	layer_objs_shops(out);
	for (int i = 0; i < ntalk; ++i)
		if (talkers[i].cat == 7) need_sprite(&npcs, 7, talkers[i].sprite);
	if (text.full || emu_debug_on())
		fprintf(stderr, "layer text: %d scripts, %d bytes%s\n", text.n, text.len, text.full ? " - FULL, lines left out" : "");
	uint32_t archive = text.n ? ta_commit(&text) : 0;
	out->archive = archive;
	if (out->guardian.navi) guardian_actors(&npcs, archive, guardian_sprite(out->guardian.navi), &out->guardian);
	for (int i = 0; i < ntalk && npcs.n < 32; ++i)
		npcs.script[npcs.n++] = npc_talker(talkers[i].cat, talkers[i].sprite, talkers[i].x, talkers[i].y, talkers[i].z,
			talkers[i].cat == 7 ? 0 : 4, talkers[i].archive ? talkers[i].archive : archive, talkers[i].script,
			talkers[i].gone_flag, talkers[i].floor);
	return mapslot_install(group, number, &npcs, md, nmd);
}
