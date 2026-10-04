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
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "rumors.h"
#include "rush.h"
#include "blockers.h"
#include "save.h"
#include "scripts.h"
#include "shop.h"
#include "trader.h"
#include "xguardian.h"
#include "xnavi.h"

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
bool layer_objs_dealer_again, layer_objs_dealer_named;
int layer_objs_duel_frames, layer_objs_duel_rung, layer_objs_duel_foes;
int layer_objs_official_level;
bool layer_objs_duel_later;

int layer_objs_bystander = LAYER_BYSTANDER, layer_objs_bystander2 = LAYER_BYSTANDER;   /* (EvilNavi) */
unsigned layer_objs_xlooks;
int layer_objs_dark_flame = -1;
const char *layer_objs_dark_chip = "";
const char *layer_objs_server_navi = "";
bool layer_objs_dark_first;

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
	int bonus = -1;   /* a chip's bonus tier (roll_chip), -1 for none */
	if (o->param == 0) {
		if (roll < 50 || (chips_only && roll < 85)) bonus = 0;
		else if (roll < 85) value = (100 + rng_range(0, 8) * 50) * (1 + run.depth / 6);
		else { kind = 5; value = rng_range(3, 8); }
	} else if (o->param == 1) {
		if (roll < 60 || chips_only) bonus = 1;
		else value = 800 + run.depth * 60;
	} else bonus = 3;
	if (bonus >= 0) {
		kind = 1;
		value = roll_chip(run.depth, bonus, &code);
		/* half the time a chip that comes in the folder's codes, in one of
		 * them: rolled again, eight times at most, for one (smart loot, as
		 * Diablo 3 leans a drop to its finder); the other half as it rolls.
		 * Leaning only the code, where the chip had one, a Blade folder's
		 * (S, L) finds came in its codes two times in seven (docs/META.md) */
		if (run.codes[0] && rng_range(0, 1)) {
			for (int t = 0; t < 8 && !loot_folder_code(value, false); ++t) value = roll_chip(run.depth, bonus, &code);
			char c = loot_folder_code(value, false);
			if (c) code = c;
		}
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

/* Whether MegaMan has program `program` now, in any colour (the PET's
 * program items, which count those on the board too). */
static bool program_had(int program) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	if (items < BN6_EWRAM || items >= BN6_EWRAM_END) return false;
	for (int c = 0; c < 4; ++c)
		if (emu_read8(items + BN6_PROGRAM_ITEMS + (uint32_t)(program * 4 + c))) return true;
	return false;
}

/* The run's Spin, a key item as a ScrtData is: the NaviCust turns programs
 * of colour c with key item 0x4F + c (bn6f sub_8136364). */
static void spin_content(uint8_t out[8], int colour) {
	const uint8_t c[8] = { 4, 0x20, 0xFF, 0xFF, (uint8_t)(0x4F + colour), 0, 0, 0 };
	for (int i = 0; i < 8; ++i) out[i] = c[i];
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

#define RUSH_OBJECTS_MAX 12   /* Rush and its bones in each panel of two gaps */

/* BN5's dark hole on Nebula Area's layers (XLOOK_DARK_HOLE): a vortex in
 * the void past the back rim of the layer's best room, where its own maps
 * stand theirs past a platform's edge, the Graveyard's monument's place;
 * the layer itself unchanged (a handler-3 object of blockers.c's, as
 * BN6's map objects' table has no room for an id of the engine's) */
static int xprops_objects(uint8_t *recs, int n, int max) {
	int x, y, cx, cy;
	if (!(layer_objs_xlooks & XLOOK_DARK_HOLE) || xnavi_object(XOBJ_DARK_HOLE) < 0 || !layer_landmark_void(&x, &y)) return n;
	netmap_world(x, y, &cx, &cy);
	if (emu_debug_on()) fprintf(stderr, "prop: BN5's dark hole at %d,%d (world %d,%d)\n", x, y, cx, cy);
	return blockers_xprop(recs, n, max, cx, cy);
}

static uint32_t props_objects(NpcList *npcs) {
	static uint8_t recs[(MAX_PROPS * 3 + 1 + RUSH_OBJECTS_MAX + MAX_BLOCKS + 1) * 20];
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
	n = xprops_objects(recs, n, MAX_PROPS * 3 + 1);
	n = rush_objects(recs, n, MAX_PROPS * 3 + 1 + RUSH_OBJECTS_MAX);
	n = blockers_objects(recs, n, MAX_PROPS * 3 + 1 + RUSH_OBJECTS_MAX + MAX_BLOCKS);
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

void layer_objs_shops(const LayerObjs *o, bool saved) {
	shop_install(SHOP_DEALER, o->dealer, o->ndealer, saved);
	shop_install(SHOP_PROGRAMS, o->programs, o->nprograms, saved);
}

/* Chip `id` as pick k of a vault or an official gate, in code `code` (its
 * own, fitted to the folder's codes where it comes in them). */
static void vault_chip(ScriptsVault *v, int k, int id, char code) {
	ChipInfo ci;
	chip_info(id, &ci);
	code = loot_fit_code(id, code, true);
	v->chip[k] = id;
	v->code[k] = code == '*' ? 26 : code - 'A';
	v->power[k] = ci.power;
	snprintf(v->name[k], sizeof v->name[k], "%s", ci.name);
	chip_desc(id, v->desc[k], sizeof v->desc[k]);
}

/* An official gate's three chips (docs/RIVAL.md): at level 1 an official
 * Chip Order, standard chips the Library holds (held in any run), as BN6's
 * Chip Order orders them, in the folder's codes; at level 2 Mega chips. */
static void official_picks(int level, ScriptsVault *v) {
	int from[512], nfrom = 0;
	/* (an official order: the Library's uncommon and rare standard
	 * chips, the common ones only while it holds too few; a
	 * playtester's was IceSeed at 10, a chip he had won and one he
	 * had just cut) */
	for (int floor = level == 1 ? 1 : 0; floor >= 0 && nfrom < 3; --floor) {
		nfrom = 0;
		for (int id = 1; id < 512 && nfrom < 512; ++id)
			if (level == 1 ? chip_pool_class(id) == 0 && chip_pool_tier(id) >= floor && meta_library_has(id) : chip_pool_class(id) == 1)
				from[nfrom++] = id;
	}
	for (int k = 0; k < 3; ++k) {
		int id = 0;
		for (int tries = 0; tries < 16; ++tries) {
			id = nfrom >= 3 ? from[rng_range(0, nfrom - 1)] : chip_pool_pick(2);
			bool again = id <= 0;
			for (int j = 0; j < k; ++j) again |= v->chip[j] == id;
			if (!again) break;
		}
		ChipInfo ci;
		chip_info(id, &ci);
		vault_chip(v, k, id, ci.ncodes ? ci.codes[0] : '*');
	}
}

/* Whether an object of `type` stands on the layer. */
static bool layer_has(int type) {
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == type) return true;
	return false;
}

/* ProtoMan's words where his netbattle waits for a later act: where, as
 * the net goes ("the third act" was the game's word, not his). */
static const char *netbattle_later_words(void) {
	return pacing_act(run.depth) == 0
		? "Enough racing, MegaMan. Our next duel is a netbattle: you against me.|Not here. I'll be waiting past the next two guardians."
		: "Enough racing, MegaMan. Our next duel is a netbattle: you against me.|Not here. I'll be waiting past the next guardian.";
}

/* Whether the official gate is the prize of ProtoMan's duel beside it (it
 * opens to the winner); where the duel waits for a later act, it opens to
 * the clearance, as any other (it had stayed sealed, a duel's prize with
 * no duel). */
static bool official_duel_prize(int level) {
	return layer_has(OBJ_DUEL) && !layer_objs_duel_later && rival_clearance() >= level;
}

/* The layer's warp pads: the exit's (warp 1), and a teleport pair's (issue
 * #44), each warping to the other within the map (warps 2 and 3). */
static void set_pads(int group, int number, int ex, int ey) {
	CoordPad pads[3] = { { ex, ey, 1 } };
	int n = 1;
	for (int k = 0; k < 2 && layer.nteleports; ++k) {
		int x, y, px, py;
		netmap_world(layer.teleport_x[k], layer.teleport_y[k], &x, &y);
		netmap_world(layer.teleport_x[1 - k], layer.teleport_y[1 - k], &px, &py);
		pads[n++] = (CoordPad){ x, y, 2 + k };
		mapslot_teleport(2 + k, group, number, px, py, 4);
	}
	netmap_set_pads(pads, n);
}

/* The best of the layer's Mystery Data, the first of them (the farthest
 * detour's: net_gen.c places those first), among the 16 the map holds; -1
 * none. */
static int best_mystery(void) {
	int best = -1;
	for (int i = 0, k = 0; i < layer.nobj && k < 16; ++i)
		if (layer.obj[i].type == OBJ_MYSTERY) {
			if (best < 0 || layer.obj[i].param > layer.obj[best].param) best = i;
			++k;
		}
	return best;
}

/* Blue where a detour ends, as BN6 keeps its better data; green loose;
 * purple locked. */
static int mystery_colour(const NetObj *o) { return o->param == MD_PURPLE ? MYSTERY_PURPLE : o->param >= 1 ? MYSTERY_BLUE : MYSTERY_GREEN; }

/* The layer's RegUp's Mystery Data, where it has one (layer_regup): the
 * first a set piece keeps, behind a lock or on an island, else the best
 * blue data where a detour ends; never the Spin's or a purple one (its
 * rarest chip is its own); -1 none. */
static int regup_md(int spin_md) {
	int prize = -1, best = -1;
	for (int i = 0, k = 0; i < layer.nobj && k < 16; ++i) {
		const NetObj *o = &layer.obj[i];
		if (o->type != OBJ_MYSTERY) continue;
		++k;
		if (i == spin_md || o->param == MD_PURPLE) continue;
		if (o->prize && prize < 0) prize = i;
		if (o->param >= 1 && (best < 0 || o->param > layer.obj[best].param)) best = i;
	}
	return prize >= 0 ? prize : best;
}

static bool purple_here(void) {
	int best = best_mystery();
	return best >= 0 && layer.obj[best].param == MD_PURPLE;
}

/* A purple data's: what no dealer sells, a chip of the rarest tiers in
 * one of its letters (BN6's hold ElecSword E, Muramasa M, DreamAura U),
 * never one the layer's dealer lists (issue #41), and never a common or
 * uncommon one: its Unlocker costs about a layer's zenny, and a
 * playtester's opened on CrakShot G, which the dealer sells for less
 * (session 62). (With the All * helper every chip is in *, its too.) */
static void purple_content(uint8_t out[8], const ShopItem *stock, int nstock) {
	char code = '*';
	int id = roll_chip(run.depth, 4, &code);
	for (int t = 0; t < 16; ++t) {
		bool sold = false;
		for (int k = 0; k < nstock; ++k) sold |= stock[k].kind == 2 && stock[k].id == id;
		if ((code != '*' || run_all_star()) && !sold && chip_pool_tier(id) != 0 && chip_pool_tier(id) != 1) break;
		id = roll_chip(run.depth, 4, &code);
	}
	const uint8_t c[8] = { 1, 0x20, 0xFF, (uint8_t)(code == '*' ? 26 : code - 'A'), (uint8_t)id, (uint8_t)(id >> 8), 0, 0 };
	memcpy(out, c, 8);
}

/* What a layer's Mystery Data hold beyond their rolls: the run's Spin in
 * the best, the layer's RegUp in its prize (issue #51), a ScrtData in the
 * first other (never behind a lock), and where the layer's purple data has
 * its key on the layer, an Unlocker in a blue one on another branch. */
typedef struct {
	int spin_md, spin_colour;
	int reg_md, reg_mb;
	bool fragment, fragment_placed, key_here;
	const ShopItem *stock;
	int nstock;
} MysteryPlan;

/* The plan's RegUp: its MB and Mystery Data, where the layer has one
 * (planned as the first Mystery Data is filled: reg_mb -1 till then). */
static void plan_regup(MysteryPlan *p) {
	p->reg_mb = layer_regup(run.depth, run.side_kind);
	p->reg_md = p->reg_mb ? regup_md(p->spin_md) : -1;
}

static void fill_mystery(MysteryData *m, const NetObj *o, int i, MysteryPlan *p, LayerObjs *out) {
	if (p->reg_mb < 0) plan_regup(p);
	m->type = mystery_colour(o);
	if (i == p->spin_md) {
		if (m->type == MYSTERY_GREEN) m->type = MYSTERY_BLUE;
		spin_content(m->content, p->spin_colour);
		out->spin_colour = p->spin_colour;
		if (emu_debug_on()) fprintf(stderr, "spin: colour %d in the Mystery Data at %d %d\n", p->spin_colour, m->x, m->y);
	} else if (i == p->reg_md) {
		const uint8_t c[8] = { 4, 0x20, 0xFF, 0xFF, (uint8_t)(SCRIPTS_REG_UP1 + p->reg_mb - 1), 0, 0, 0 };
		memcpy(m->content, c, 8);
		m->type = MYSTERY_BLUE;
		if (emu_debug_on()) fprintf(stderr, "regup: +%d MB in the Mystery Data at %d %d%s\n", p->reg_mb, m->x, m->y, o->prize ? " (a set piece's)" : "");
	} else if (p->fragment && o->param != MD_PURPLE) {
		p->fragment = false;
		p->fragment_placed = true;
		if (m->type == MYSTERY_GREEN) m->type = MYSTERY_BLUE;
		fragment_content(m->content);
	} else if (o->param == MD_PURPLE) {
		purple_content(m->content, p->stock, p->nstock);
		if (emu_debug_on()) fprintf(stderr, "purple: chip %d code %d (tier %d) in the Mystery Data at %d %d\n", m->content[4] | m->content[5] << 8,
			m->content[3], chip_pool_tier(m->content[4] | m->content[5] << 8), m->x, m->y);
	} else if (p->key_here && o->param >= 1) {
		static const uint8_t key[8] = { 4, 0x20, 0xFF, 0xFF, SUB_UNLOCKER, 0, 0, 0 };
		p->key_here = false;
		memcpy(m->content, key, 8);
	} else if (mystery_content(o, m->content)) {
		m->type = MYSTERY_BLUE;
	}
	if (out->nmd < 16) {
		out->md_obj[out->nmd] = (uint8_t)i;
		out->md_colour[out->nmd++] = (uint8_t)m->type;
	}
}

/* The Net Dealer's word on the act's guardian, `navi`, by his list
 * (`stock`, n of them): the element he can't stand (of either wheel:
 * TenguMan's Sword, issue #39) and the pick of it, first on the list; for
 * one weak to none, the hardest hit, and the viruses' weakness besides
 * where the list has a chip of it (an element-less guardian's act is
 * answered for its viruses). ElementMan's element changes as he fights,
 * so none answers him for long. */
static void dealer_word(char *word, size_t n, int navi, int counter, const ShopItem *stock, int nstock, const char *brought,
                        const char *lands) {
	bool weak = counter > 0 && guardian_weakness(navi) > 0, listed = false;
	for (int i = 0; i < nstock; ++i)
		if (stock[i].kind == 2 && counter > 0 && chip_hits_with(stock[i].id) == counter && (i == 0 || !weak)) listed = true;
	if (weak && listed) {
		snprintf(word, n, "|Word is, %s can't stand %s chips.|My pick for the job's first on the list. %s%s", guardian(navi)->name,
			elem_name(counter), brought, lands);
		return;
	}
	/* (a weakness he found no chip of, as a few layers' rolls of Cursor
	 * chips came to: the hardest hit, shop_dealer_stock) */
	if (weak) {
		snprintf(word, n, "|Word is, %s can't stand %s chips, but I couldn't get my hands on any. Hit hard: my pick for the job's "
			"first on the list. %s", guardian(navi)->name, elem_name(counter), brought);
		return;
	}
	/* (KnightMan's armor turns every blow but while he swings or leaps: a
	 * playtester learned it over three Custom screens, session 68) */
	bool knight = guardian_older(navi) && guardian_older_ai(navi) == BN5_NAVI_KNIGHTMAN;
	int k = snprintf(word, n, "|Word is, %s %s. Hit hard%s: my pick for the job's first on the list. %s", guardian(navi)->name,
		navi == GUARDIAN_ELEMENTMAN ? "changes his element as he fights" :
		knight ? "has no weak element, and his armor turns every blow but while he swings or leaps" : "has no weak element",
		knight ? " then" : "", brought);
	if (listed && k > 0 && (size_t)k < n)
		snprintf(word + k, n - (size_t)k, "|The viruses around here can't stand %s chips, though. I've got one of those too!",
			elem_name(counter));
}

/* The obstacles' sprites, first: one that could not load would stand as
 * noise in its pocket's mouth. */
static void blocker_sprites(NpcList *npcs) {
	for (int b = 0; b < layer.nblocks; ++b) {
		int cat, idx;
		blocker_sprite(b, &cat, &idx);
		need_sprite(npcs, cat, idx);
	}
}

/* The layer's text, whole: the obstacles' talks added, the archive the
 * talkers read, and the obstacles readied with the map's checks taking
 * their talks from a copy of it; the archive's address, 0 none. */
static uint32_t commit_text(TextArchive *text, int group, int number) {
	int block_talk[2];
	blockers_talks(text, block_talk);
	uint32_t archive = text->n ? ta_commit(text) : 0;
	blockers_install();
	blockers_checks(group, number, text, block_talk);
	return archive;
}

static bool skull_here(void) {
	for (int k = 0; k < layer.nblocks; ++k)
		if (layer.block[k].kind == BLOCK_SKULL) return true;
	return false;
}

/* (Rush's gap, and how many RushFood call him there: he comes for as many
 * as its panels and eats one, which only MegaMan said, at its bones; a
 * playtester would have bought one of three, session 64) */
static void dealer_rush(char *s, size_t n) {
	int hold = layer.ngaps ? layer.gap[0].len : shop_rush_need(run.depth);
	const char *where = layer.ngaps ? "on this layer" : "deeper in this act";
	if (hold > 1)
		snprintf(s, n, "|And there's a gap %s that Rush can bridge. He only comes when you hold %d RushFood, and he eats one. It's on my "
			"list too!", where, hold);
	else snprintf(s, n, "|And there's a gap %s that Rush can bridge for one RushFood. It's on my list too!", where);
}

/* (the keys he stocks, said: what each opens and where, issues #41, #14, #47) */
static void dealer_keys(char *hello, size_t n, const ShopItem *stock, int nstock) {
	for (int i = 0; i < nstock; ++i) {
		size_t k = strlen(hello);
		if (stock[i].kind != 1 || k >= n) continue;
		if (stock[i].id == SUB_UNLOCKER)
			snprintf(hello + k, n - k, "|Word is, there's purple data locked %s. An Unlocker opens it, and it's on my list!",
				purple_here() ? "on this layer" : "deeper in this act");
		else if (stock[i].id == ITEM_RUSH_FOOD)
			dealer_rush(hello + k, n - k);
		else if (stock[i].id == ITEM_WWW_ID)
			snprintf(hello + k, n - k, "|The skull doors %s only open for WWW members. A WWW-ID gets you through every one!",
				skull_here() ? "on this layer" : "deeper in the Undernet");
	}
}

/* The first bystander on an act's first layer passes on the net's word
 * about its guardian, where MegaMan has never battled him: who he is and a
 * rumor, no moves (docs/META.md, what MegaMan knows; MegaMan himself had
 * said it, and how could he know?). His script, -1 none. */
static int rumor_talk(TextArchive *text, LayerObjs *out, int face) {
	static char rumor[200];
	int navi = run_layer_guardian();
	/* (a list-6 navi's face has its sprite's number) */
	if (run.side_kind != LAYER_NORMAL || layer_in_act(run.depth) != 0 || !navi || guardian_known(navi) || !guardian_rumor(navi) ||
		out->nchoices >= LAYER_MAX_CHOICES)
		return -1;
	snprintf(rumor, sizeof rumor, "They say a copy of %s guards the end of %s.|Word is, %s", guardian(navi)->name,
		guardian_area_in_text(run.biome, LAYER_NORMAL), guardian_rumor(navi));
	/* (heard, MegaMan names him for the rest of the act: the director
	 * watches the flag as a choice) */
	int flag = LAYER_FLAG_BASE + out->nchoices;
	out->choice[out->nchoices].type = OBJ_NPC;
	out->choice[out->nchoices++].flag = flag;
	flag_clear(flag);
	return ta_say_flag(text, face, rumor, flag);
}

/* A bystander's talk with a part to play: the first's rumor of the act's
 * guardian (`first` >= 0), a P-Code's teller's, an invisible path's
 * hint (issue #46), the second's whisper (`said`, rumors.c); -1 for his
 * own line. */
/* The layer's last bystander, whose place a flame of darkness takes (the
 * P-Code teller and the invisible path's hinter keep theirs): its object
 * index, -1 for none. */
static int last_bystander(void) {
	int last = -1;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_NPC && i != layer.teller - 1 && i != layer.hinter - 1) last = i;
	return last;
}

static int bystander_talk(TextArchive *text, LayerObjs *out, int i, int first, int said, int who);

/* Where the flame of darkness stands (docs/META.md): in the last
 * bystander's place, else, where the services and data took all the map's
 * talkers, in its last green Mystery Data's (never the best, the Spin's): a
 * playtester walked a layer of ACDC Area whose flame had nowhere to stand
 * (session 66). -1 none. */
static int flame_host(void) {
	int b = last_bystander(), best = best_mystery();
	for (int i = layer.nobj - 1; b < 0 && i >= 0; --i)
		if (layer.obj[i].type == OBJ_MYSTERY && layer.obj[i].param == 0 && i != best) b = i;
	return b;
}

static void flame_stand(TextArchive *text, LayerObjs *out, Talker *tk, int i) {
	tk->cat = 7;
	tk->sprite = layer_objs_dark_flame;
	tk->script = out->dark_flame = ta_dark_flame(text, LAYER_DARK_TAKEN_FLAG, layer_objs_dark_chip, layer_objs_dark_first);
	tk->gone_flag = LAYER_DARK_TAKEN_FLAG;
	out->dark_flame_obj = i;
	if (emu_debug_on()) fprintf(stderr, "dark: the flame of darkness at %d %d\n", tk->x, tk->y);
}

/* A bystander's talker (Normal Navis and pink navis, or the two Navis
 * another game's area lends, taking turns), or the flame of darkness in
 * the last one's place */
static void bystander(TextArchive *text, LayerObjs *out, Talker *tk, int i, int *said) {
	static int base;
	if (layer_objs_dark_flame >= 0 && i == flame_host()) {
		flame_stand(text, out, tk, i);
		return;
	}
	/* (the next four of the pool each layer, from where the run's seed
	 * starts it: a bystander's random pick had a playtester hear the same
	 * line on an act's first and third layers) */
	if (!*said) base = (int)(run.seed % 97u) + (run.depth - 1) * 4;
	/* (the first of them from the layer's seed) */
	tk->sprite = ((run.layer_seed * 2654435761u >> 28) + (unsigned)*said) & 1 ? layer_objs_bystander2 : layer_objs_bystander;
	if (emu_debug_on()) fprintf(stderr, "bystander %d: list 6 %d at %d %d\n", *said, tk->sprite, tk->x, tk->y);
	tk->script = bystander_talk(text, out, i, *said ? -1 : base + *said, *said, tk->sprite);
	if (tk->script < 0) tk->script = ta_say(text, tk->sprite, npc_line(run.depth, base + *said));
	++*said;
}

static int bystander_talk(TextArchive *text, LayerObjs *out, int i, int first, int said, int who) {
	int rumor = first >= 0 ? rumor_talk(text, out, who) : -1;
	if (rumor >= 0) return rumor;
	if (i == layer.teller - 1) return ta_pcode_teller(text, who, blockers_pcode(), LAYER_PCODE_FLAG);
	if (i == layer.hinter - 1)
		return ta_say(text, who, "See that little pad out in the void, all by itself?|I saw a Navi walk out to it. Right over nothing!");
	const char *whisper = said == 1 ? rumors_line() : NULL;
	if (whisper && emu_debug_on()) fprintf(stderr, "rumor: %s\n", whisper);
	return whisper ? ta_say(text, who, whisper) : -1;
}

/* What a Spin does, and that it stays: the first also how they are
 * found, one deeper in each dive. */
static int spin_words(TextArchive *text, int colour, bool first) {
	static char words[400];
	const char *c = meta_spin_name(colour);
	int held = 0;
	for (int k = 0; k < 6; ++k) held += meta_spins() >> k & 1;
	if (first)
		snprintf(words, sizeof words, "A Spin for %s programs, Lan! Now we can turn %s programs on the NaviCust's board: "
			"hold one and press L or R.|And it stays with us, in every dive from now on. Every dive hides one more deeper in, "
			"a color we don't have yet!", c, c);
	else if (held >= 5)
		snprintf(words, sizeof words, "A Spin for %s programs, Lan! That's all six: every program on our board turns now, "
			"in every dive!", c);
	else
		snprintf(words, sizeof words, "A Spin for %s programs, Lan! %c%s programs turn with L and R on the NaviCust's board now, "
			"in every dive from here on.", c, c[0] - 'a' + 'A', c + 1);
	return ta_say(text, FACE_MEGAMAN, words);
}

/* Layer object i, a Mystery Data: the map's next one (16 at most), or
 * the flame of darkness standing in its place (flame_host). */
static void mystery_or_flame(TextArchive *text, LayerObjs *out, Talker *tk, int i, MysteryData *md, int *nmd, NpcList *npcs, MysteryPlan *plan) {
	if (layer_objs_dark_flame >= 0 && i == flame_host()) { flame_stand(text, out, tk, i); return; }
	if (*nmd >= 16 || npcs->n >= 32) return;
	md[*nmd].x = tk->x;
	md[*nmd].y = tk->y;
	md[*nmd].z = tk->z;
	fill_mystery(&md[*nmd], &layer.obj[i], i, plan, out);
	npcs->script[npcs->n++] = npc_mystery(*nmd);
	++*nmd;
}

/* Each of the layer's rolls from a stream of its own, the layer seed's
 * and `salt`'s, so that what one object draws never moves another's. A
 * Chip Trader's prize new to the Library, the program list the profile's
 * programs lead and an official order of the Library's chips read the
 * profile, which play changes after the layer's save: their draws moved
 * every roll after them, and a CONTINUE found other chips in the same
 * Mystery Data (a playtester's MachGun2 S, Vulcan3 A the session before;
 * session 65). */
static void rolls_of(uint32_t salt) { rng_seed(run.layer_seed ^ (salt + 1u) * 0x9E3779B9u); }

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
	layer_objs_dealer_named = false;
	out->guardian.navi = 0;
	out->challenge_reward = -1;
	out->fragment_found = -1;
	out->spin_found = -1;
	out->spin_colour = out->nmd = 0;
	for (int i = 0; i <= OBJ_OFFICIAL; ++i) out->script_of[i] = -1;
	out->gate_navi = 0;
	out->gate_reward = -1;
	out->trader_kind = -1;
	out->dark_flame = out->dark_flame_obj = -1;
	layer_objs_official_level = 0;
	/* the element that answers this act: its guardian's weakness, else its
	 * viruses' (the Net Dealer stocks a chip of it and says so) */
	int counter = counter_element(run.depth, run.biome, run_layer_guardian());
	/* ScrtData lie in deep layers until three are out there */
	int said = 0;   /* bystanders so far: each says another line */
	rolls_of(0x100);
	bool fragment = !run.secret_cleared && run.fragments < 3 &&
		(run.side_kind == LAYER_UNDERNET || run.depth >= 4) && rng_range(0, 99) < FRAGMENT_CHANCE;
	/* the run's Spin, in the best Mystery Data of one layer of 4-8 (docs/
	 * META.md: one a run, a colour the profile lacks, kept for good) */
	int spin_colour = meta_spin_here() ? meta_spin_colour() : 0;
	MysteryPlan plan = { spin_colour ? best_mystery() : -1, spin_colour, -1, -1, fragment, false, false, NULL, 0 };
	bool spin_first = meta_spins() == 0;
	/* the Net Dealer's stock, before his words (they say how many of his
	 * answer he brought) */
	ShopItem stock[SHOP_MAX_ITEMS];
	int navi_of_act = run_layer_guardian();
	bool weakless = navi_of_act > 0 && guardian_weakness(navi_of_act) <= 0;
	rolls_of(0x101);
	int nstock = shop_dealer_stock(run.depth, weakless ? -1 : counter, weakless ? counter : 0, stock);
	plan.stock = stock;
	plan.nstock = nstock;
	/* (a purple data's key in a blue one, three layers in ten: a lock and
	 * its key on one layer) */
	rolls_of(0x102);
	plan.key_here = purple_here() && rng_range(0, 99) < 30;
	/* the program vendor's, before his words too (he names the programs
	 * MegaMan has had in earlier runs, which lead his list) */
	rolls_of(0x103);
	out->nprograms = shop_program_stock(run.depth, out->programs);
	const char *brought = nstock && stock[0].stock == 1 ? "It's the only one I've got, so make it count!" : "I brought two, and they go fast!";
	/* (how his pick lands, where it is not straight ahead: AquaNdl2 missed
	 * a hopping BlastMan two times in three) */
	const char *lands = nstock && stock[0].kind == 2 && chip_family(stock[0].id) == 50
		? "|Its needles drop where he's standing a moment later, so fire when he stops!" : "";
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		rolls_of((uint32_t)i);
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
		char prize[24] = "";   /* (a Server's chip, named in its offer) */
		switch (o->type) {
		case OBJ_WARP_IN:
			out->start_x = wx; out->start_y = wy;
			break;
		case OBJ_EXIT:
		case OBJ_RETURN: {
			int pad = o->type == OBJ_EXIT ? SPR_EXIT_PAD : SPR_RETURN_PAD;
			out->exit_x = wx; out->exit_y = wy;
			/* the game's own warp pad: trigger cells taking warp 1; and a
			 * teleport pair's, warps 2 and 3 to each other */
			set_pads(group, number, wx, wy);
			need_sprite(&npcs, 7, pad);
			/* a guardian's exit shows once the guardian is beaten */
			if (npcs.n < 32)
				npcs.script[npcs.n++] = layer.boss_layer ? npc_sealed_pad(7, pad, wx, wy, wz, LAYER_EXIT_OPEN_FLAG)
					: npc_prop(7, pad, wx, wy, wz, 0);
			break;
		}
		case OBJ_MYSTERY: mystery_or_flame(&text, out, &tk, i, md, &nmd, &npcs, &plan); break;
		case OBJ_NPC: bystander(&text, out, &tk, i, &said); break;
		case OBJ_HEAL: tk.script = ta_heal(&text, o->npc_line + run.depth, LAYER_HEAL_TOLD_FLAG); break;
		case OBJ_TRADER:
		case OBJ_BUGTRADER: {
			/* the game's own machine and lines; deeper, some are Specials */
			TraderKind kind = o->type == OBJ_BUGTRADER ? TRADER_BUGFRAG
				: (run.depth - 1) % CYCLE_LAYERS >= SPECIAL_FROM && run.layer_seed % 100 < SPECIAL_CHANCE ? TRADER_SPECIAL : TRADER_CHIPS;
			trader_install(group, number, kind, run.depth);
			out->trader_kind = kind;
			tk.cat = 7; tk.sprite = SPR_CHIP_TRADER;
			tk.archive = BN6_TRADER_TEXT;
			tk.script = kind;
			break;
		}
		case OBJ_SHOP: {
			/* (and a word on the element that answers this act, which the
			 * stock carries a chip of) */
			char hello[720], word[280] = "";
			int navi = run_layer_guardian();
			/* (the net's word comes back from an act's second layer: its
			 * first keeps the mystery of a guardian never battled, but for
			 * a bystander's rumor, sought out; a playtester's dealer named
			 * one two minutes into the act) */
			bool tells = navi > 0 && (guardian_known(navi) || layer_in_act(run.depth) > 0);
			layer_objs_dealer_named = tells;
			/* (nor does he deny the bystanders' rumor: a playtester heard it
			 * two platforms before his "No word yet") */
			if (navi > 0 && !tells)
				snprintf(word, sizeof word, "|Nobody's come back from the end of %s to tell what guards it. There's talk on the net, "
					"but I don't sell on talk. Ask me again deeper in!", guardian_area_in_text(run.biome, LAYER_NORMAL));
			else if (navi > 0)
				dealer_word(word, sizeof word, navi, counter, stock, nstock, brought, lands);
			/* (and that his chips come in the folder's codes, loot_fit_code) */
			snprintf(hello, sizeof hello, "%s%s", run.depth <= 3
				? run.codes[0] ? "Welcome to the Net Dealer! Divers need chips, and I've got 'em, in your folder's codes when I can!"
					: "Welcome to the Net Dealer! Divers need chips, and I've got 'em!"
				: run.side_kind == LAYER_NORMAL && run.mode == RUN_SHORT && run_short_last(run.depth)
					/* (the run's last layer has no "from here": a playtester heard it there) */
					? "The bottom of the net, MegaMan! Stock up. This is my last stop, and yours!"
					: "Still diving, MegaMan? Stock up. It only gets tougher from here!", word);
			/* (met in this act already: the pick, in a line) */
			if (layer_objs_dealer_again && tells)
				snprintf(hello, sizeof hello, "Back again, MegaMan! My pick for %s is first on the list. %s", guardian(navi)->name, brought);
			dealer_keys(hello, sizeof hello, stock, nstock);
			tk.sprite = SPR_DEALER;
			tk.script = ta_shop(&text, SHOP_DEALER, FACE_NAVI, hello, "Back for more? Take a look!",
				"Sold out, MegaMan! You bought every chip I brought.", LAYER_DEALER_TOLD_FLAG);
			break;
		}
		case OBJ_PROGRAMS: {
			/* (programs from earlier runs, by name: the reason they lead;
			 * not one MegaMan has on him now, which the vendor's "brought
			 * it along" offered a playtester who had SuperArmor installed) */
			char names[2][16], hello[300], again[140] = "";
			int nnames = 0;
			for (int k = 0; k < out->nprograms && nnames < 2; ++k) {
				const char *about = navicust_about(out->programs[k].id / 4);
				if (!about || !shop_program_found(out->programs[k].id / 4) || program_had(out->programs[k].id / 4)) continue;
				snprintf(names[nnames++], sizeof names[0], "%.*s", (int)strcspn(about, ":"), about);
			}
			if (nnames == 1) snprintf(again, sizeof again, "|I hear MegaMan's used %s before. I brought it along!", names[0]);
			else if (nnames == 2)
				snprintf(again, sizeof again, "|I hear MegaMan's used %s and %s before. I brought them along!", names[0], names[1]);
			snprintf(hello, sizeof hello, "NaviCust programs, fresh from my workbench!%s|Install them in your PET: MegaMan, then NaviCust.",
				again);
			tk.sprite = SPR_TECH;
			tk.script = ta_shop(&text, SHOP_PROGRAMS, FACE_TECH, hello, "More programs? Take a look!",
				"Sold out! Every program I brought is yours now.", LAYER_VENDOR_TOLD_FLAG);
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
			bool best_fits = false, best_new = false;
			for (int tries = 0; tries < 12; ++tries) {
				char c = '*';
				int id = tries ? roll_chip(run.depth + 2, 4, &c) : chip;
				chip_info(id, &ci);
				int power = chip_direct(id) ? ci.power : 0;   /* (a MchnSwrd's 200 needs a paralysed enemy) */
				/* (one the folder can play first, in its codes or *: a Blade
				 * folder's won HeatManEX H; and first of all one the layer's
				 * Net Dealer doesn't sell, from the same pool: a playtester
				 * won a third MoonBld A beside the two he had just bought) */
				bool fits = loot_folder_code(id, true) != 0, fresh = true;
				for (int s = 0; s < nstock; ++s) fresh &= !(stock[s].kind == 2 && stock[s].id == id);
				if ((fresh && !best_new) || (fresh == best_new && ((fits && !best_fits) || (fits == best_fits && power > best_power)))) {
					best_power = power; best_fits = fits; best_new = fresh; chip = id; if (tries) code = c;
				}
			}
			chip_info(chip, &ci);
			/* (in the folder's code, or its * where it has none of them, for
			 * any folder: an M-Cannon R went with nothing a playtester
			 * carried) */
			char fit = loot_folder_code(chip, true);
			if (fit) code = fit;
			else if (strchr(ci.codes, '*')) code = '*';
			out->challenge_reward = ta_challenge_reward(&text, chip, ci.name, code == '*' ? 26 : code - 'A');
			snprintf(prize, sizeof prize, "%s %c", ci.name, code);
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
		case OBJ_DUEL:
			/* the real ProtoMan (docs/RIVAL.md), in his own body: the Nest's
			 * copies are the guardians; a netbattle not yet due, he names
			 * where it will be */
			asks = !layer_objs_duel_later;
			tk.cat = 6;
			tk.sprite = guardian_sprite(11);
			need_sprite(&npcs, 6, tk.sprite);
			if (layer_objs_duel_later) tk.script = ta_say(&text, guardian_face(11), netbattle_later_words());
			if (emu_debug_on()) fprintf(stderr, "duel: ProtoMan at %d %d, his time %d frames\n", wx, wy, layer_objs_duel_frames);
			break;
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
		case OBJ_OFFICIAL: {
			/* an official gate (docs/RIVAL.md): sealed until Chaud's
			 * clearance reaches its level; open, three chips to take one:
			 * at level 1 an official Chip Order, standard chips the
			 * Library holds (held in any run), as BN6's Chip Order orders
			 * them; at level 2 Mega chips */
			tk.cat = 7; tk.sprite = SPR_GATE; tk.floor = true;
			int level = o->param >= 2 ? 2 : 1;
			layer_objs_official_level = level;
			ScriptsVault v = { 0 };
			official_picks(level, &v);
			tk.script = ta_official(&text, LAYER_OFFICIAL_FLAG, LAYER_CLEARED_FLAG, level, profile.duel_won, official_duel_prize(level), &v);
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
				vault_chip(&v, k, id, code);
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
			static char terms[GUARDIAN_TERMS_MAX];
			if (o->type == OBJ_DUEL) {
				/* his terms: the time to beat, as the results screen shows a
				 * DeleteTime (seconds and hundredths, cut), and the rung's
				 * rule; the first duel says who he is */
				int f = layer_objs_duel_frames, sec = f / 60, cs = (f % 60) * 100 / 60;
				int met = profile.duel_won + profile.duel_lost;
				if (layer_objs_duel_rung == 2) guardian_netbattle_terms(terms, sizeof terms);
				else
				{
					/* (the squad and the stake said before the choice, as
					 * docs/RIVAL.md promises: a playtester took it at 100 HP,
					 * nothing saying it was a real fight; and the two are old
					 * rivals, not strangers) */
					static const char *const count[] = { "", "a lone virus", "a pair of viruses", "three viruses", "four viruses" };
					int nf = layer_objs_duel_foes >= 1 && layer_objs_duel_foes <= 4 ? layer_objs_duel_foes : 3;
					snprintf(terms, sizeof terms, "%sI busted %s here in %d:%02d.%02d. Beat that%s, MegaMan.|"
						"@M Real viruses, Lan. If they delete us, the dive's over, so let's heal up first if we're hurt.",
						met ? "Back again, MegaMan? Chaud's watching.|" :
						"MegaMan. So it's you diving the Endless Net. The Nest copies Navis, they say. I'm no copy.|Chaud wants to see what you've got.|",
						count[nf], sec / 60, sec % 60, cs, layer_objs_duel_rung == 1 ? ", without taking a hit" : "");
				}
				/* (he logs out as the duel begins: one a layer) */
				tk.gone_flag = flag;
			}
			tk.script = o->type == OBJ_DUEL ? ta_duel(&text, flag, guardian_face(11), terms)
				: o->type == OBJ_CHALLENGE ? ta_challenge(&text, flag, prize, layer_objs_server_navi)
				: o->type == OBJ_UNDERNET ? ta_undernet(&text, flag, run.biome == BIOME_UNDERNET)
				: o->type == OBJ_NAVI_GATE ? ta_navi_gate(&text, flag, guardian(o->param)->name, GATE_CODE, GATE_CODE)
				: ta_secret_gate(&text, flag);
			/* not chosen yet */
			flag_clear(flag);
		}
		if (tk.script >= 0 && !tk.archive && out->script_of[o->type] < 0) out->script_of[o->type] = tk.script;
		if (tk.script >= 0 && ntalk < 16) talkers[ntalk++] = tk;
	}
	rolls_of(0x1FF);
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
	layer_objs_shops(out, false);
	blocker_sprites(&npcs);
	for (int i = 0; i < ntalk; ++i)
		if (talkers[i].cat == 7) need_sprite(&npcs, 7, talkers[i].sprite);
	npcs.objects = props_objects(&npcs);
	if (text.full || emu_debug_on())
		fprintf(stderr, "layer text: %d scripts, %d bytes%s\n", text.n, text.len, text.full ? " - FULL, lines left out" : "");
	/* (what a ScrtData is for, said as it is picked up: a playtester was
	 * told a layer later, then on every layer after; and where its gate
	 * stands, which a playtester holding three asked) */
	if (plan.fragment_placed) {
		static const char *const found[3] = {
			"A ScrtData, Lan!|Three of these open the golden gate to the Secret Area, in the Undernet's copy a dark warp leads to. Let's find two more!",
			"Our second ScrtData!|One more, and the golden gate to the Secret Area opens. It stands in the Undernet's copy!",
			"That's three ScrtData, Lan!|The golden gate to the Secret Area will open for us now. It stands in the Undernet's copy: the next dark warp leads there!",
		};
		out->fragment_found = ta_say(&text, FACE_MEGAMAN, found[run.fragments < 3 ? run.fragments : 2]);
	}
	if (out->spin_colour) out->spin_found = spin_words(&text, out->spin_colour, spin_first);
	uint32_t archive = commit_text(&text, group, number);
	out->archive = archive;
	if (out->guardian.navi) guardian_actors(&npcs, archive, &out->guardian);
	for (int i = 0; i < ntalk && npcs.n < 32; ++i) {
		const Talker *t = &talkers[i];
		uint32_t a = t->archive ? t->archive : archive;
		npcs.script[npcs.n++] = t->behind ? npc_counter_talker(t->cat, t->sprite, t->x, t->y, t->z, t->anim, a, t->script, t->sx, t->sy)
			: npc_talker(t->cat, t->sprite, t->x, t->y, t->z, t->cat == 7 ? 0 : 4, a, t->script, t->gone_flag, t->floor);
	}
	rush_install(group, number);
	return mapslot_install(group, number, &npcs, md, nmd);
}
