/* Layer objects on the game's own NPCs, Mystery Data and text scripts. */
#include "layer_objs.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "chip_pool.h"
#include "data.h"
#include "debug.h"
#include "emu.h"
#include "flags.h"
#include "game.h"
#include "guardians.h"
#include "loot.h"
#include "rivals.h"
#include "mapslot.h"
#include "meta.h"
#include "net.h"
#include "navicust.h"
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
/* List 6's generic navis are three: GreenNavi (62-66, 87: one sprite), EvilNavi
 * (67, 68) and GirlNavi (69, 70), each role its own (Kai took bystanders for
 * the dealer while they shared GreenNavi); 56-58 are Roll, GutsMan and Glyde,
 * compressed, 59 ProtoMan, 71-86 the guardians (bn6f npcSpritePtrs) */
#define SPR_DEALER      62   /* GreenNavi, the Net Dealer's keeper in the game */
#define SPR_TECH        69   /* GirlNavi, the NaviCust vendor */
bool layer_objs_dealer_again;

#define SPR_BYSTANDER   67   /* EvilNavi */

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
	/* (threat 6, docs/META.md: a chip where zenny would be) */
	bool chips_only = run.threat >= 6;
	if (o->param == 0) {
		if (roll < 50 || (chips_only && roll < 85)) { kind = 1; value = roll_chip(run.depth, 0, &code); }
		else if (roll < 85) value = (100 + rng_range(0, 8) * 50) * (1 + run.depth / 6);
		else { kind = 5; value = rng_range(3, 8); }
	} else if (o->param == 1) {
		if (roll < 60 || chips_only) { kind = 1; value = roll_chip(run.depth, 1, &code); }
		else value = 800 + run.depth * 60;
	} else {
		kind = 1;
		value = roll_chip(run.depth, 3, &code);
	}
	/* (half the time in the folder's codes, the dealers' always) */
	if (kind == 1) code = loot_fit_code(value, code, false);
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

/* Compressed sprites only draw once the map loads them: false where the
 * map's list has no room left for one (12 of them, 0x8800 bytes
 * decompressed, the game's loader), which then must not be shown. */
static bool need_sprite(NpcList *npcs, int category, int index) {
	uint32_t list = emu_read32(0x08000000u + R.layout->sprite_lists + (uint32_t)category * 4);
	uint32_t ptr = emu_read32(list + (uint32_t)index * 4);
	if (!(ptr & 0x80000000u)) return true;
	for (int i = 0; i < npcs->nsprites; ++i)
		if (npcs->sprite_idx[i] == index && npcs->sprite_cat[i] == category * 4) return true;
	uint32_t bytes = emu_read32(ptr & 0x7FFFFFFFu) >> 8;   /* (its LZ77 header) */
	if (npcs->nsprites >= MAPSLOT_SPRITES || npcs->sprite_bytes + bytes > MAPSLOT_SPRITE_BYTES) return false;
	npcs->sprite_cat[npcs->nsprites] = (uint8_t)(category * 4);
	npcs->sprite_idx[npcs->nsprites++] = (uint8_t)index;
	npcs->sprite_bytes += bytes;
	return true;
}

/* The sprite props' map objects (docs/LEVEL_DESIGN.md, Props): the
 * originals' objects by their OverworldMapObjects ids, each at its panel's
 * spot +(14, 18) from the corner, as the originals set theirs, a set
 * piece's pieces where the original has them. */
#define OW_MAP_OBJECTS 0x0A4F24u
static const struct { uint8_t look; uint8_t id; int8_t dx, dy; int16_t dz; } prop_pieces[] = {
	{ LOOK_TREE, 0x3D, 14, 18, 0 },            /* a cybertree (Green Area 2, ACDC Area) */
	{ LOOK_GIANT_TREE, 0x7F, 14, 18, 0 },      /* the giant cybertree's body, */
	{ LOOK_GIANT_TREE, 0x80, 38, -6, -24 },    /* its base */
	{ LOOK_GIANT_TREE, 0x81, -10, 42, 154 },   /* and its crown (Green Area 2) */
	{ LOOK_STATUE, 0x9C, 14, 18, -12 },        /* the Undernet's statue */
	{ LOOK_BRAZIER, 0x9F, 14, 18, 0 },         /* a brazier (Undernet 1 and Zero) */
	{ LOOK_MONUMENT, 0xD3, 16, 16, 0 },        /* the Graveyard's monument */
	{ LOOK_GRAVE, 0xCA, 14, 18, 0 },           /* a gravestone (the Graveyard's rows) */
	{ LOOK_SIGN, 0x3E, 14, 18, 0 },            /* the NetCafe's WELCOME sign (Central Area 1) */
	{ LOOK_BBS, 0x47, 14, 18, -24 },           /* a BBS (Seaside Area 1) */
};

static uint32_t props_objects(NpcList *npcs) {
	static uint8_t recs[(MAX_PROPS * 3 + 1) * 20];
	int n = 0;
	for (int i = 0; i < layer.nprops; ++i) {
		const NetProp *p = &layer.props[i];
		if (p->kind != PROP_SPRITE) continue;
		/* (its sprites first: a piece without its sprite would draw noise) */
		bool loaded = true;
		for (size_t k = 0; k < sizeof prop_pieces / sizeof *prop_pieces; ++k)
			if (prop_pieces[k].look == p->look) {
				const uint8_t *e = R.data + OW_MAP_OBJECTS + prop_pieces[k].id * 16u;
				loaded &= need_sprite(npcs, e[0] / 4, e[1]);
			}
		if (emu_debug_on()) fprintf(stderr, "prop look %d at %d,%d%s%s\n", p->look, p->x, p->y, layer.cell[p->y][p->x] ? " (hole)" : "", loaded ? "" : " - no room for its sprite");
		if (!loaded) continue;
		int cx, cy;
		netmap_world(p->x, p->y, &cx, &cy);   /* (the panel's centre, 16 in from its corner) */
		int cz = layer.level[p->y][p->x] ? layer.rise : 0;
		for (size_t k = 0; k < sizeof prop_pieces / sizeof *prop_pieces && n < MAX_PROPS * 3; ++k) {
			if (prop_pieces[k].look != p->look) continue;
			uint8_t *r = recs + n++ * 20;
			memset(r, 0, 20);
			r[0] = 5;   /* a map object, its handler looked up by its id */
			put32(r + 4, (uint32_t)((int32_t)(cx - 16 + prop_pieces[k].dx) * 65536));
			put32(r + 8, (uint32_t)((int32_t)(cy - 16 + prop_pieces[k].dy) * 65536));
			put32(r + 12, (uint32_t)((int32_t)(cz + prop_pieces[k].dz) * 65536));
			put32(r + 16, prop_pieces[k].id);
		}
	}
	if (!n) return 0;
	memset(recs + n * 20, 0, 4);
	recs[n * 20] = 0xFF;
	return mapslot_alloc(recs, n * 20 + 4);
}

typedef struct {
	int x, y, z, cat, sprite, script, gone_flag;
	bool floor;
	uint32_t archive;   /* its text archive; 0: the layer's */
	/* behind a counter: facing its front (animation), spoken to across it
	 * at (sx, sy) from where it stands */
	bool behind;
	int anim, sx, sy;
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
	out->fragment_found = -1;
	for (int i = 0; i <= OBJ_VAULT; ++i) out->script_of[i] = -1;
	out->gate_navi = 0;
	out->gate_reward = -1;
	/* the element that answers this act: its guardian's weakness, else its
	 * viruses' (the Net Dealer stocks a chip of it and says so) */
	int counter = counter_element(run.depth, run.biome, run.boss_order[run.biome]);
	/* ScrtData lie in deep layers until three are out there */
	int said = 0;   /* bystanders so far: each says another line */
	bool fragment = !run.secret_cleared && run.fragments < 3 &&
		(run.side_kind == LAYER_UNDERNET || run.depth >= 4) && rng_range(0, 99) < FRAGMENT_CHANCE, fragment_placed = false;
	/* the Net Dealer's stock, before his words (they say how many of his
	 * answer he brought) */
	ShopItem stock[SHOP_MAX_ITEMS];
	int navi_of_act = run.boss_order[run.biome], ge_of_act = navi_of_act > 0 ? enemy_element(enemy_id(1, navi_of_act, 0)) : 0;
	bool elementless = navi_of_act > 0 && !(ge_of_act > 0 && ge_of_act <= 4);
	int nstock = shop_dealer_stock(run.depth, elementless ? -1 : counter, elementless ? counter : 0, stock);
	/* (whether his list has a chip of the viruses' element, for his word) */
	bool virus_chip = false;
	for (int i = 0; i < nstock; ++i) {
		ChipInfo ci;
		if (stock[i].kind == 2 && counter > 0 && (chip_info(stock[i].id, &ci), ci.element == counter)) virus_chip = true;
	}
	/* the program vendor's, before his words too (he names the programs
	 * MegaMan has had in earlier runs, which lead his list) */
	out->nprograms = shop_program_stock(run.depth, out->programs);
	const char *brought = nstock && stock[0].stock == 1 ? "It's the only one I've got, so make it count!" : "I brought two, and they go fast!";
	/* (how his pick lands, where it is not straight ahead: AquaNdl2 missed
	 * a hopping BlastMan two times in three) */
	const char *lands = nstock && stock[0].kind == 2 && chip_family(stock[0].id) == 50
		? "|Its needles drop where he's standing a moment later, so fire when he stops!" : "";
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int wx, wy;
		netmap_world((int)o->x, (int)o->y, &wx, &wy);
		/* in a raised room, on its floor */
		int wz = layer.level[(int)o->y][(int)o->x] ? layer.rise : 0;
		Talker tk = { wx, wy, wz, 6, SPR_PROG, -1, -1, false, 0, false, 0, 0, 0 };
		/* a navi behind a counter stands in its aisle, facing its front
		 * (screen down-right or down-left: the game's facings 3 and 5) */
		if (o->prop >= 0 && netmap_prop_navi(o->prop, &tk.x, &tk.y, &tk.sx, &tk.sy)) {
			tk.behind = true;
			tk.anim = layer.props[o->prop].faces == FACES_X ? 3 : 5;
		}
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
					fragment_placed = true;
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
			static int base;
			if (!said) base = o->npc_line;
			tk.sprite = SPR_BYSTANDER;
			/* (a list-6 navi's face has its sprite's number) */
			tk.script = ta_say(&text, tk.sprite, npc_line(run.depth, base + said++));
			break;
		}
		case OBJ_HEAL: tk.script = ta_heal(&text, o->npc_line + run.depth, LAYER_HEAL_TOLD_FLAG); break;
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
				snprintf(word, sizeof word, "|Word is, %s can't stand %s chips.|My pick for the job's first on the list. %s%s",
					guardian(navi)->name, elem[counter], brought, lands);
			else if (navi > 0 && counter > 0 && virus_chip)
				/* (no element to answer the guardian with: the hardest hit on
				 * the list, and the viruses' weakness besides, of which the
				 * list has one) */
				snprintf(word, sizeof word, "|Word is, %s has no weak element. Hit hard: my pick for the job's first on the list. %s|"
					"The viruses around here can't stand %s chips, though. I've got one of those too!", guardian(navi)->name, brought,
					elem[counter]);
			else if (navi > 0)
				snprintf(word, sizeof word, "|Word is, %s has no weak element. Hit hard: my pick for the job's first on the list. %s",
					guardian(navi)->name, brought);
			/* (and that his chips come in the folder's codes, loot_fit_code) */
			snprintf(hello, sizeof hello, "%s%s", run.depth <= 3
				? run.codes[0] ? "Welcome to the Net Dealer! Divers need chips, and I've got 'em, in your folder's codes when I can!"
					: "Welcome to the Net Dealer! Divers need chips, and I've got 'em!"
				: "Still diving, MegaMan? Stock up. It only gets tougher from here!", word);
			/* (met in this act already: the pick, in a line) */
			if (layer_objs_dealer_again && navi > 0)
				snprintf(hello, sizeof hello, "Back again, MegaMan! My pick for %s is first on the list. %s", guardian(navi)->name, brought);
			tk.sprite = SPR_DEALER;
			tk.script = ta_shop(&text, SHOP_DEALER, FACE_NAVI, hello, "Back for more? Take a look!", LAYER_DEALER_TOLD_FLAG);
			break;
		}
		case OBJ_PROGRAMS: {
			/* (programs from earlier runs, by name: the reason they lead) */
			char names[2][16], hello[300], again[140] = "";
			int nnames = 0;
			for (int k = 0; k < out->nprograms && nnames < 2; ++k) {
				const char *about = navicust_about(out->programs[k].id / 4);
				if (!about || !shop_program_found(out->programs[k].id / 4)) continue;
				snprintf(names[nnames++], sizeof names[0], "%.*s", (int)strcspn(about, ":"), about);
			}
			if (nnames == 1) snprintf(again, sizeof again, "|I hear MegaMan's used %s before. I brought it along!", names[0]);
			else if (nnames == 2)
				snprintf(again, sizeof again, "|I hear MegaMan's used %s and %s before. I brought them along!", names[0], names[1]);
			snprintf(hello, sizeof hello, "NaviCust programs, fresh from my workbench!%s|Install them in your PET: MegaMan, then NaviCust.",
				again);
			tk.sprite = SPR_TECH;
			tk.script = ta_shop(&text, SHOP_PROGRAMS, FACE_TECH, hello, "More programs? Take a look!", LAYER_VENDOR_TOLD_FLAG);
			break;
		}
		case OBJ_CHALLENGE: {
			asks = true; tk.cat = 7; tk.sprite = SPR_SERVER;
			/* a win pays with a chip well past Mystery Data, the hardest
			 * hitting of a dozen from where the Net Dealer finds his picks,
			 * two layers deeper (a WhiCapsl, a FireBrn1 for 120 HP, then an
			 * M-Cannon beside his DolThdr2 were poor prizes for the risk) */
			char code = '*';
			ChipInfo ci;
			int chip = roll_chip(run.depth + 2, 4, &code), best_power = -1;
			for (int tries = 0; tries < 12; ++tries) {
				char c = '*';
				int id = tries ? roll_chip(run.depth + 2, 4, &c) : chip;
				chip_info(id, &ci);
				int power = chip_direct(id) ? ci.power : 0;   /* (a MchnSwrd's 200 needs a paralysed enemy) */
				if (power > best_power) { best_power = power; chip = id; if (tries) code = c; }
			}
			chip_info(chip, &ci);
			/* (in its * code where it has one, for any folder: an M-Cannon R
			 * went with nothing a playtester carried) */
			if (strchr(ci.codes, '*')) code = '*';
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
				/* (not a recovery chip, whose power is what it heals: Recov150
				 * was offered as hitting for 150) */
				if (ci.power >= 60 && ci.ncodes && chip_def(c)->kind != CK_RECOVER) break;
			}
			if (chip <= 0) chip = roll_chip(run.depth, 3, &code);
			chip_info(chip, &ci);
			code = loot_fit_code(chip, ci.ncodes ? ci.codes[rng_range(0, ci.ncodes - 1)] : '*', true);
			ShopItem program = { 3, 1, 0, 0, 0 };
			const char *about = shop_pick_gift_program(&program);
			/* a run lost before its first guardian earns a little more */
			bool comfort = profile.last_depth >= 1 && profile.last_depth <= 3;
			if (emu_debug_on()) fprintf(stderr, "gift: chip %d \"%s\" %c, program %d color %d\n", chip, ci.name, code, program.id, program.code);
			tk.script = ta_gift(&text, LAYER_GIFT_FLAG, comfort, profile.runs >= 2, (run.helpers & HELP_HEAD_START) != 0, chip, ci.name, ci.power, code == '*' ? 26 : code - 'A', program.id,
				program.code, about);
			/* (and logs out once it is taken: he stood beside the arrival,
			 * and a playtester's A pressed through the last box talked to
			 * him again, two sessions running) */
			tk.gone_flag = LAYER_GIFT_FLAG;
			flag_clear(LAYER_GIFT_FLAG);
			break;
		}
		case OBJ_UNDERNET: asks = true; tk.cat = 7; tk.sprite = SPR_DARK_WARP; break;
		case OBJ_SECRET_GATE: asks = true; tk.cat = 7; tk.sprite = SPR_GATE; tk.floor = true; break;
		case OBJ_NAVI_GATE: {
			/* sealed until his code is earned (docs/META.md, gates): its
			 * talk then says how, else it asks for his SP and pays his SP
			 * chip */
			tk.cat = 7; tk.sprite = SPR_GATE; tk.floor = true;
			int navi = o->param, won = rival(navi)->megaman_won;
			const char *name = guardian(navi)->name;
			asks = won >= GATE_CODE;
			out->gate_navi = navi;
			if (!asks) tk.script = ta_navi_gate(&text, -1, name, won, GATE_CODE);
			else {
				int chip = navi_chip(navi, 2), code = 26;
				ChipInfo ci = { 0 };
				if (chip > 0) {
					chip_info(chip, &ci);
					if (ci.ncodes) code = ci.codes[0] == '*' ? 26 : ci.codes[0] - 'A';
				}
				out->gate_reward = chip > 0 ? ta_gate_reward(&text, name, chip, ci.name, code) : -1;
			}
			break;
		}
		case OBJ_VAULT: {
			/* a collector's vault (docs/META.md, gates): sealed until the
			 * profile's Library holds enough chips, its talk then says how
			 * many; open, three rare chips, one to take, in the folder's
			 * codes where they come in them */
			tk.cat = 7; tk.sprite = SPR_GATE; tk.floor = true;
			int need = meta_vault_need(run.depth), have = meta_library_count(-1);
			ScriptsVault v = { 0 };
			for (int k = 0; k < 3; ++k) {
				char code = '*';
				int id = 0;
				for (int tries = 0; tries < 12; ++tries) {
					id = roll_chip(run.depth + 2, 4, &code);
					bool again = false;
					for (int j = 0; j < k; ++j) again |= v.chip[j] == id;
					if (!again) break;
				}
				ChipInfo ci;
				chip_info(id, &ci);
				code = loot_fit_code(id, code, true);
				v.chip[k] = id;
				v.code[k] = code == '*' ? 26 : code - 'A';
				snprintf(v.name[k], sizeof v.name[k], "%s", ci.name);
			}
			tk.script = ta_vault(&text, LAYER_VAULT_FLAG, need, have, &v);
			break;
		}
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
				: o->type == OBJ_NAVI_GATE ? ta_navi_gate(&text, flag, guardian(o->param)->name, GATE_CODE, GATE_CODE)
				: ta_secret_gate(&text, flag);
			/* not chosen yet */
			flag_clear(flag);
		}
		if (tk.script >= 0 && !tk.archive && out->script_of[o->type] < 0) out->script_of[o->type] = tk.script;
		if (tk.script >= 0 && ntalk < 16) talkers[ntalk++] = tk;
	}
	/* the shops' stock, in the game's shop data */
	if (emu_debug_on())
		for (int i = 0; i < nstock; ++i)
			if (stock[i].kind == 2) {
				ChipInfo ci;
				chip_info(stock[i].id, &ci);
				fprintf(stderr, "shop chip %s %c %dz\n", ci.name, stock[i].code == 26 ? '*' : 'A' + stock[i].code, stock[i].price * 100);
			}
	for (int i = 0; i < nstock; ++i) out->dealer[i] = stock[i];
	out->ndealer = nstock;
	layer_objs_shops(out);
	for (int i = 0; i < ntalk; ++i)
		if (talkers[i].cat == 7) need_sprite(&npcs, 7, talkers[i].sprite);
	npcs.objects = props_objects(&npcs);
	if (text.full || emu_debug_on())
		fprintf(stderr, "layer text: %d scripts, %d bytes%s\n", text.n, text.len, text.full ? " - FULL, lines left out" : "");
	/* (what a ScrtData is for, said as it is picked up: a playtester was
	 * told a layer later, then on every layer after) */
	if (fragment_placed) {
		static const char *const found[3] = {
			"A ScrtData, Lan!|Three of these open the golden gate to the Secret Area. Let's find two more!",
			"Our second ScrtData!|One more, and the golden gate to the Secret Area opens!",
			"That's three ScrtData, Lan!|The golden gate to the Secret Area will open for us now!",
		};
		out->fragment_found = ta_say(&text, FACE_MEGAMAN, found[run.fragments < 3 ? run.fragments : 2]);
	}
	uint32_t archive = text.n ? ta_commit(&text) : 0;
	out->archive = archive;
	if (out->guardian.navi) guardian_actors(&npcs, archive, guardian_sprite(out->guardian.navi), &out->guardian);
	for (int i = 0; i < ntalk && npcs.n < 32; ++i) {
		const Talker *t = &talkers[i];
		uint32_t a = t->archive ? t->archive : archive;
		npcs.script[npcs.n++] = t->behind ? npc_counter_talker(t->cat, t->sprite, t->x, t->y, t->z, t->anim, a, t->script, t->sx, t->sy)
			: npc_talker(t->cat, t->sprite, t->x, t->y, t->z, t->cat == 7 ? 0 : 4, a, t->script, t->gone_flag, t->floor);
	}
	return mapslot_install(group, number, &npcs, md, nmd);
}
