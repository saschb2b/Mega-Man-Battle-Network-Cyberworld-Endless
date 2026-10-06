/* Layer objects on the game's own NPCs, Mystery Data and text scripts. */
#include "layer_objs.h"

#include <stdio.h>
#include <string.h>

#include "blockers.h"
#include "bytes.h"
#include "chip_pool.h"
#include "data.h"
#include "debug.h"
#include "emu.h"
#include "flags.h"
#include "game.h"
#include "guardians.h"
#include "layer_words.h"
#include "loot.h"
#include "meta.h"
#include "navicust.h"
#include "netmap.h"
#include "npc.h"
#include "npc_lines.h"
#include "rivals.h"
#include "rom.h"
#include "rumor_lines.h"
#include "rush.h"
#include "save.h"
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
const ScriptsDark6 *layer_objs_dark6;
/* (a layer with a flame: its first bystander free of a part to play runs
 * BN6's own words on DarkChips, a rumor before the find) */
static bool dark_rumor_due;
const char *layer_objs_server_navi = "";
bool layer_objs_dark_first, layer_objs_dark_ours;

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
	/* (none on a trip back: the run's HPMemory lie where it goes on,
	 * docs/HOME.md) */
	if (o->param == 2 && run.depth >= 4 && run.side_kind != LAYER_BACK && rng_range(0, 99) < HP_MEMORY_CHANCE) {
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
				loaded &= npc_need_sprite(npcs, e[0] / 4, e[1]);
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

/* What the layer's Recovery Mr. Prog gives, once (issue #71, the owner's
 * call on a player's proposal): half of MegaMan's max HP, as the layer is
 * built, away from the arena, so a detour's HP carries on; to full before
 * the arena (0), where the guardian is met as his band assumes; the Heals
 * helper's, to full as often as asked (-1). */
static int heal_amount(void) {
	if (run.helpers & HELP_HEALS) return -1;
	if (layer.boss_layer) return 0;
	int half = emu_read16(BN6_NAVI_MAX_HP) / 2;
	return half > 0 ? half : 1;
}

/* The obstacles' sprites, first: one that could not load would stand as
 * noise in its pocket's mouth. */
static void blocker_sprites(NpcList *npcs) {
	for (int b = 0; b < layer.nblocks; ++b) {
		int cat, idx;
		blocker_sprite(b, &cat, &idx);
		npc_need_sprite(npcs, cat, idx);
	}
}

/* The layer's text, whole: the obstacles' talks added, the archive the
 * talkers read, and the obstacles readied with the map's checks taking
 * their talks from a copy of it; the archive's address, 0 none. */
static uint32_t commit_text(TextArchive *text, int group, int number) {
	int block_talk[2];
	blockers_talks(text, block_talk);
	uint32_t archive = text->n ? ta_commit(text) : 0;
	if (emu_debug_on()) fprintf(stderr, "layer text hash %08x\n", ta_hash(text));
	blockers_install();
	blockers_checks(group, number, text, block_talk);
	return archive;
}

static bool skull_here(void) {
	for (int k = 0; k < layer.nblocks; ++k)
		if (layer.block[k].kind == BLOCK_SKULL) return true;
	return false;
}

/* The first bystander on an act's first layer passes on the net's word
 * about its guardian, where MegaMan has never battled him: who he is and a
 * rumor, no moves (docs/META.md, what MegaMan knows; MegaMan himself had
 * said it, and how could he know?). His script, -1 none. */
static int rumor_talk(TextArchive *text, LayerObjs *out, int face) {
	int navi = run_layer_guardian();
	/* (a list-6 navi's face has its sprite's number) */
	if (run.side_kind != LAYER_NORMAL || layer_in_act(run.depth) != 0 || !navi || guardian_known(navi) || !guardian_rumor(navi) ||
		out->nchoices >= LAYER_MAX_CHOICES)
		return -1;
	/* (heard, MegaMan names him for the rest of the act: the director
	 * watches the flag as a choice) */
	int flag = LAYER_FLAG_BASE + out->nchoices;
	out->choice[out->nchoices].type = OBJ_NPC;
	out->choice[out->nchoices++].flag = flag;
	flag_clear(flag);
	return ta_say_flag(text, face, rumor_words(navi), flag);
}

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
	tk->script = out->dark_flame = layer_objs_dark6 ? ta_dark_flame6(text, LAYER_DARK_TAKEN_FLAG, layer_objs_dark6)
		: ta_dark_flame(text, LAYER_DARK_TAKEN_FLAG, layer_objs_dark_chip, layer_objs_dark_first, layer_objs_dark_ours);
	tk->gone_flag = LAYER_DARK_TAKEN_FLAG;
	out->dark_flame_obj = i;
	if (emu_debug_on()) fprintf(stderr, "dark: the flame of darkness at %d %d\n", tk->x, tk->y);
}

/* What a layer's objects are installed with (layer_objs_install): its map
 * and where they go, its text, the map's NPCs, the talkers and the Mystery
 * Data, the Net Dealer's stock and what the data hold beyond their rolls */
typedef struct {
	int group, number;
	LayerObjs *out;
	TextArchive *text;
	NpcList npcs;
	Talker talkers[16];
	int ntalk;
	MysteryData md[16];
	int nmd;
	int counter;       /* the element that answers this act */
	int said;          /* bystanders so far: each says another line */
	bool spin_first;   /* the run's Spin is the profile's first */
	ShopItem stock[SHOP_MAX_ITEMS];   /* the Net Dealer's */
	int nstock;
	MysteryPlan plan;
} Install;

/* A bystander's talker (Normal Navis and pink navis, or the two Navis
 * another game's area lends, taking turns), or the flame of darkness in
 * the last one's place */
static void bystander(Install *in, Talker *tk, int i) {
	static int base;
	TextArchive *text = in->text;
	LayerObjs *out = in->out;
	int *said = &in->said;
	if (layer_objs_dark_flame >= 0 && i == flame_host()) {
		flame_stand(text, out, tk, i);
		return;
	}
	/* (the next four of the pool each layer, from where the run's seed
	 * starts it: a bystander's random pick had a playtester hear the same
	 * line on an act's first and third layers) */
	if (!*said) {
		base = (int)(run.seed % 97u) + (run.depth - 1) * 4;
		dark_rumor_due = layer_objs_dark_flame >= 0;
	}
	/* (the first of them from the layer's seed) */
	tk->sprite = ((run.layer_seed * 2654435761u >> 28) + (unsigned)*said) & 1 ? layer_objs_bystander2 : layer_objs_bystander;
	if (emu_debug_on()) fprintf(stderr, "bystander %d: list 6 %d at %d %d\n", *said, tk->sprite, tk->x, tk->y);
	tk->script = bystander_talk(text, out, i, *said ? -1 : base + *said, *said, tk->sprite);
	if (tk->script < 0) tk->script = ta_say(text, tk->sprite, npc_line(run.depth, base + *said));
	++*said;
}

/* A bystander's talk with a part to play: the first's rumor of the act's
 * guardian (`first` >= 0), a P-Code's teller's, an invisible path's
 * hint (issue #46), the second's whisper (`said`, rumor_lines.c); -1 for his
 * own line. */
static int bystander_talk(TextArchive *text, LayerObjs *out, int i, int first, int said, int who) {
	int rumor = first >= 0 ? rumor_talk(text, out, who) : -1;
	if (rumor >= 0) return rumor;
	if (i == layer.teller - 1) return ta_pcode_teller(text, who, blockers_pcode(), LAYER_PCODE_FLAG);
	if (i == layer.hinter - 1)
		return ta_say(text, who, hinter_words());
	int dark = dark_rumor_due ? ta_dark_rumor(text, who) : -1;
	dark_rumor_due = false;
	if (dark >= 0 && emu_debug_on()) fprintf(stderr, "rumor: BN6's own words on DarkChips, bystander %d\n", said);
	if (dark >= 0) return dark;
	const char *whisper = said == 1 ? rumors_line() : NULL;
	if (whisper && emu_debug_on()) fprintf(stderr, "rumor: %s\n", whisper);
	return whisper ? ta_say(text, who, whisper) : -1;
}

/* Layer object i, a Mystery Data: the map's next one (16 at most), or
 * the flame of darkness standing in its place (flame_host). */
static void mystery_or_flame(Install *in, Talker *tk, int i) {
	if (layer_objs_dark_flame >= 0 && i == flame_host()) { flame_stand(in->text, in->out, tk, i); return; }
	if (in->nmd >= 16 || in->npcs.n >= 32) return;
	MysteryData *m = &in->md[in->nmd];
	m->x = tk->x;
	m->y = tk->y;
	m->z = tk->z;
	fill_mystery(m, &layer.obj[i], i, &in->plan, in->out);
	in->npcs.script[in->npcs.n++] = npc_mystery(in->nmd);
	++in->nmd;
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

/* The layer's objects begun: no choices or scripts yet, the act's answer,
 * whether a ScrtData and the run's Spin lie here, and the Net Dealer's and
 * the program vendor's stock, each from rolls of its own */
static void install_begin(Install *in) {
	LayerObjs *out = in->out;
	out->nchoices = 0;
	layer_objs_dealer_named = false;
	/* (no guardian, and no scripts of his: a layer without one kept the
	 * last guardian layer's numbers, which --talk intro ran in this
	 * layer's archive) */
	out->guardian = (GuardianStage){ .intro = -1, .defeat = -1, .reward = -1 };
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
	in->counter = counter_element(run.depth, run.biome, run_layer_guardian());
	/* ScrtData lie in deep layers until three are out there (none on a
	 * trip back, docs/HOME.md) */
	rolls_of(0x100);
	bool fragment = !run.secret_cleared && run.fragments < 3 && run.side_kind != LAYER_BACK &&
		(run.side_kind == LAYER_UNDERNET || run.depth >= 4) && rng_range(0, 99) < FRAGMENT_CHANCE;
	/* the run's Spin, in the best Mystery Data of one layer of 4-8 (docs/
	 * META.md: one a run, a colour the profile lacks, kept for good) */
	int spin_colour = meta_spin_here() ? meta_spin_colour() : 0;
	in->plan = (MysteryPlan){ spin_colour ? best_mystery() : -1, spin_colour, -1, -1, fragment, false, false, NULL, 0 };
	in->spin_first = meta_spins() == 0;
	/* the Net Dealer's stock, before his words (they say how many of his
	 * answer he brought) */
	int navi_of_act = run_layer_guardian();
	bool weakless = navi_of_act > 0 && guardian_weakness(navi_of_act) <= 0;
	rolls_of(0x101);
	in->nstock = shop_dealer_stock(run.depth, weakless ? -1 : in->counter, weakless ? in->counter : 0, in->stock);
	in->plan.stock = in->stock;
	in->plan.nstock = in->nstock;
	/* (a purple data's key in a blue one, three layers in ten: a lock and
	 * its key on one layer) */
	rolls_of(0x102);
	in->plan.key_here = purple_here() && rng_range(0, 99) < 30;
	/* the program vendor's, before his words too (he names the programs
	 * MegaMan has had in earlier runs, which lead his list) */
	rolls_of(0x103);
	out->nprograms = shop_program_stock(run.depth, out->programs);
}

/* The exit (or return) pad: the game's own warp pad, trigger cells taking
 * warp 1, and a teleport pair's, warps 2 and 3 to each other; a guardian's
 * exit shows once the guardian is beaten */
static void install_exit(Install *in, const NetObj *o, int wx, int wy, int wz) {
	int pad = o->type == OBJ_EXIT ? SPR_EXIT_PAD : SPR_RETURN_PAD;
	in->out->exit_x = wx; in->out->exit_y = wy;
	set_pads(in->group, in->number, wx, wy);
	npc_need_sprite(&in->npcs, 7, pad);
	if (in->npcs.n < 32)
		in->npcs.script[in->npcs.n++] = layer.boss_layer ? npc_sealed_pad(7, pad, wx, wy, wz, LAYER_EXIT_OPEN_FLAG)
			: npc_prop(7, pad, wx, wy, wz, 0);
}

/* A Chip or BugFrag Trader: the game's own machine and lines; deeper, some
 * are Specials */
static void install_trader(Install *in, const NetObj *o, Talker *tk) {
	TraderKind kind = o->type == OBJ_BUGTRADER ? TRADER_BUGFRAG
		: (run.depth - 1) % CYCLE_LAYERS >= SPECIAL_FROM && run.layer_seed % 100 < SPECIAL_CHANCE ? TRADER_SPECIAL : TRADER_CHIPS;
	trader_install(in->group, in->number, kind, run.depth);
	in->out->trader_kind = kind;
	tk->cat = 7; tk->sprite = SPR_CHIP_TRADER;
	tk->archive = BN6_TRADER_TEXT;
	tk->script = kind;
}

/* The Net Dealer, and his word on the element that answers this act, which
 * his stock carries a chip of (layer_words.c) */
static void install_dealer(Install *in, Talker *tk) {
	int navi = run_layer_guardian();
	/* (the net's word comes back from an act's second layer: its
	 * first keeps the mystery of a guardian never battled, but for
	 * a bystander's rumor, sought out; a playtester's dealer named
	 * one two minutes into the act) */
	bool tells = navi > 0 && (guardian_known(navi) || layer_in_act(run.depth) > 0);
	layer_objs_dealer_named = tells;
	DealerTalk talk = { navi, tells, layer_objs_dealer_again, in->counter, in->stock, in->nstock, purple_here(), skull_here() };
	ShopWords w = dealer_words(&talk);
	tk->sprite = SPR_DEALER;
	tk->script = ta_shop(in->text, SHOP_DEALER, FACE_NAVI, w.hello, w.again, w.sold_out, LAYER_DEALER_TOLD_FLAG);
}

/* The NaviCust vendor: programs from earlier runs, by name, the reason
 * they lead; not one MegaMan has on him now, which the vendor's "brought it
 * along" offered a playtester who had SuperArmor installed */
static void install_vendor(Install *in, Talker *tk) {
	const LayerObjs *out = in->out;
	char names[2][16];
	int nnames = 0;
	for (int k = 0; k < out->nprograms && nnames < 2; ++k) {
		const char *about = navicust_about(out->programs[k].id / 4);
		if (!about || !shop_program_found(out->programs[k].id / 4) || program_had(out->programs[k].id / 4)) continue;
		snprintf(names[nnames++], sizeof names[0], "%.*s", (int)strcspn(about, ":"), about);
	}
	ShopWords w = vendor_words(names, nnames);
	tk->sprite = SPR_TECH;
	tk->script = ta_shop(in->text, SHOP_PROGRAMS, FACE_TECH, w.hello, w.again, w.sold_out, LAYER_VENDOR_TOLD_FLAG);
}

/* A Server: a win pays with a chip well past Mystery Data, the hardest
 * hitting of a dozen from where the Net Dealer finds his picks, two layers
 * deeper (a WhiCapsl, a FireBrn1 for 120 HP, then an M-Cannon beside his
 * DolThdr2 were poor prizes for the risk); the chip, named in its offer,
 * into `prize` */
static void install_server(Install *in, Talker *tk, char *prize, size_t n) {
	tk->cat = 7; tk->sprite = SPR_SERVER;
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
		for (int s = 0; s < in->nstock; ++s) fresh &= !(in->stock[s].kind == 2 && in->stock[s].id == id);
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
	in->out->challenge_reward = ta_challenge_reward(in->text, chip, ci.name, code == '*' ? 26 : code - 'A');
	snprintf(prize, n, "%s %c", ci.name, code);
}

/* The first layer's gift: a chip that hits hard (the tier's traps and
 * supports are little help to a starting folder) and a program */
static void install_gift(Install *in, Talker *tk) {
	char code = '*';
	ChipInfo ci;
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
	ScriptsGift gift = { .flag = LAYER_GIFT_FLAG, .comfort = comfort, .brief = profile.runs >= 2, .head_start = (run.helpers & HELP_HEAD_START) != 0,
		.chip = chip, .code = code == '*' ? 26 : code - 'A', .power = ci.power, .chip_name = ci.name, .program = program.id, .color = program.code,
		.about = about };
	tk->script = ta_gift(in->text, &gift);
	/* (and logs out once it is taken: he stood beside the arrival,
	 * and a playtester's A pressed through the last box talked to
	 * him again, two sessions running) */
	tk->gone_flag = LAYER_GIFT_FLAG;
	flag_clear(LAYER_GIFT_FLAG);
}

/* The real ProtoMan (docs/RIVAL.md), in his own body: the Nest's copies
 * are the guardians; a netbattle not yet due, he names where it will be.
 * True where his duel is a choice. */
static bool install_duel(Install *in, Talker *tk, int wx, int wy) {
	tk->cat = 6;
	tk->sprite = guardian_sprite(11);
	npc_need_sprite(&in->npcs, 6, tk->sprite);
	if (layer_objs_duel_later) tk->script = ta_say(in->text, guardian_face(11), netbattle_later_words());
	if (emu_debug_on()) fprintf(stderr, "duel: ProtoMan at %d %d, his time %d frames\n", wx, wy, layer_objs_duel_frames);
	return !layer_objs_duel_later;
}

/* A Navi gate, sealed until his code is earned (docs/META.md, gates): its
 * talk then says how, else it asks for his SP and pays his SP chip. True
 * where it asks. */
static bool install_navi_gate(Install *in, const NetObj *o, Talker *tk) {
	tk->cat = 7; tk->sprite = SPR_GATE; tk->floor = true;
	int navi = o->param, won = rival(navi)->megaman_won;
	const char *name = guardian(navi)->name;
	bool asks = won >= GATE_CODE;
	in->out->gate_navi = navi;
	if (!asks) tk->script = ta_navi_gate(in->text, -1, name, won, GATE_CODE);
	else {
		int chip = navi_chip(navi, 2), code = 26;
		ChipInfo ci = { 0 };
		if (chip > 0) {
			chip_info(chip, &ci);
			if (ci.ncodes) code = ci.codes[0] == '*' ? 26 : ci.codes[0] - 'A';
		}
		in->out->gate_reward = chip > 0 ? ta_gate_reward(in->text, name, chip, ci.name, code) : -1;
	}
	return asks;
}

/* An official gate (docs/RIVAL.md): sealed until Chaud's clearance reaches
 * its level; open, three chips to take one: at level 1 an official Chip
 * Order, standard chips the Library holds (held in any run), as BN6's Chip
 * Order orders them; at level 2 Mega chips */
static void install_official(Install *in, const NetObj *o, Talker *tk) {
	tk->cat = 7; tk->sprite = SPR_GATE; tk->floor = true;
	int level = o->param >= 2 ? 2 : 1;
	layer_objs_official_level = level;
	ScriptsVault v = { 0 };
	official_picks(level, &v);
	tk->script = ta_official(in->text, LAYER_OFFICIAL_FLAG, LAYER_CLEARED_FLAG, level, profile.duel_won, official_duel_prize(level), &v);
}

/* A collector's vault (docs/META.md, gates): sealed until the profile's
 * Library holds enough chips, its talk then says how many; open, three
 * rare chips, one to take, in the folder's codes where they come in them */
static void install_vault(Install *in, Talker *tk) {
	tk->cat = 7; tk->sprite = SPR_GATE; tk->floor = true;
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
	tk->script = ta_vault(in->text, LAYER_VAULT_FLAG, need, have, &v);
}

/* A Yes/No the director acts on (out->choice): its flag, not chosen yet,
 * and its script; ProtoMan's terms said before his */
static void install_choice(Install *in, const NetObj *o, Talker *tk, const char *prize) {
	LayerObjs *out = in->out;
	int flag = LAYER_FLAG_BASE + out->nchoices;
	out->choice[out->nchoices].type = o->type;
	out->choice[out->nchoices++].flag = flag;
	static char terms[GUARDIAN_TERMS_MAX];
	if (o->type == OBJ_DUEL) {
		/* his terms: the time to beat and the rung's rule (layer_words.c);
		 * the netbattle's, his own */
		if (layer_objs_duel_rung == 2) guardian_netbattle_terms(terms, sizeof terms);
		else duel_terms(terms, sizeof terms, profile.duel_won + profile.duel_lost, layer_objs_duel_foes, layer_objs_duel_frames,
			layer_objs_duel_rung);
		/* (he logs out as the duel begins: one a layer) */
		tk->gone_flag = flag;
	}
	tk->script = o->type == OBJ_DUEL ? ta_duel(in->text, flag, guardian_face(11), terms)
		: o->type == OBJ_CHALLENGE ? ta_challenge(in->text, flag, prize, layer_objs_server_navi)
		: o->type == OBJ_UNDERNET ? ta_undernet(in->text, flag, run.biome == BIOME_UNDERNET)
		: o->type == OBJ_NAVI_GATE ? ta_navi_gate(in->text, flag, guardian(o->param)->name, GATE_CODE, GATE_CODE)
		: ta_secret_gate(in->text, flag);
	/* not chosen yet */
	flag_clear(flag);
}

/* Layer object i on the map: its talker and script, and its choice */
/* What stands on the layer without a chat: where MegaMan comes in, the
 * ways out, the guardian (its own actors and scripts: guardian_objs.c). */
static bool install_place(Install *in, const NetObj *o, int wx, int wy, int wz) {
	switch (o->type) {
	case OBJ_WARP_IN:
		in->out->start_x = wx; in->out->start_y = wy;
		return true;
	case OBJ_EXIT:
	case OBJ_RETURN: install_exit(in, o, wx, wy, wz); return true;
	case OBJ_BOSS: guardian_scripts(in->text, o, wx, wy, wz, &in->out->guardian); return true;
	default: return false;
	}
}

/* Who speaks for an object, where it stands: a navi behind a counter in its
 * aisle, facing its front (screen down-right or down-left: the game's
 * facings 3 and 5). */
static Talker talker_at(const NetObj *o, int wx, int wy, int wz) {
	Talker tk = { wx, wy, wz, 6, SPR_PROG, -1, -1, false, 0, false, 0, 0, 0 };
	if (o->prop >= 0 && netmap_prop_navi(o->prop, &tk.x, &tk.y, &tk.sx, &tk.sy)) {
		tk.behind = true;
		tk.anim = layer.props[o->prop].faces == FACES_X ? 3 : 5;
	}
	return tk;
}

/* The object's chat; true where it ends on a Yes/No the director acts on
 * (a Server names its chip in `prize`). */
static bool install_talk(Install *in, const NetObj *o, Talker *tk, int i, char *prize, size_t size) {
	switch (o->type) {
	case OBJ_MYSTERY: mystery_or_flame(in, tk, i); return false;
	case OBJ_NPC: bystander(in, tk, i); return false;
	case OBJ_HEAL: tk->script = ta_heal(in->text, o->npc_line + run.depth, LAYER_HEAL_TOLD_FLAG, heal_amount()); return false;
	case OBJ_TRADER:
	case OBJ_BUGTRADER: install_trader(in, o, tk); return false;
	case OBJ_SHOP: install_dealer(in, tk); return false;
	case OBJ_PROGRAMS: install_vendor(in, tk); return false;
	case OBJ_CHALLENGE: install_server(in, tk, prize, size); return true;
	case OBJ_GIFT: install_gift(in, tk); return false;
	case OBJ_UNDERNET: tk->cat = 7; tk->sprite = SPR_DARK_WARP; return true;
	case OBJ_DUEL: return install_duel(in, tk, tk->x, tk->y);
	case OBJ_SECRET_GATE: tk->cat = 7; tk->sprite = SPR_GATE; tk->floor = true; return true;
	case OBJ_NAVI_GATE: return install_navi_gate(in, o, tk);
	case OBJ_OFFICIAL: install_official(in, o, tk); return false;
	case OBJ_VAULT: install_vault(in, tk); return false;
	default: return false;
	}
}

static void install_object(Install *in, int i) {
	const NetObj *o = &layer.obj[i];
	rolls_of((uint32_t)i);
	int wx, wy;
	netmap_world((int)o->x, (int)o->y, &wx, &wy);
	/* in a raised room, on its floor */
	int wz = layer.level[(int)o->y][(int)o->x] ? layer.rise : 0;
	if (install_place(in, o, wx, wy, wz)) return;
	Talker tk = talker_at(o, wx, wy, wz);
	char prize[24] = "";
	bool asks = install_talk(in, o, &tk, i, prize, sizeof prize);
	if (asks && in->out->nchoices < LAYER_MAX_CHOICES) install_choice(in, o, &tk, prize);
	if (tk.script >= 0 && !tk.archive && in->out->script_of[o->type] < 0) in->out->script_of[o->type] = tk.script;
	if (tk.script >= 0 && in->ntalk < 16) in->talkers[in->ntalk++] = tk;
}

bool layer_objs_install(int group, int number, LayerObjs *out) {
	mapslot_reset();
	static TextArchive text;
	ta_begin(&text);
	static Install in;
	memset(&in, 0, sizeof in);
	in.group = group;
	in.number = number;
	in.out = out;
	in.text = &text;
	install_begin(&in);
	for (int i = 0; i < layer.nobj; ++i) install_object(&in, i);
	rolls_of(0x1FF);
	/* the shops' stock, in the game's shop data */
	if (emu_debug_on())
		for (int i = 0; i < in.nstock; ++i)
			if (in.stock[i].kind == 2) {
				ChipInfo ci;
				chip_info(in.stock[i].id, &ci);
				fprintf(stderr, "shop chip %s %c %dz\n", ci.name, in.stock[i].code == 26 ? '*' : 'A' + in.stock[i].code, in.stock[i].price * 100);
			}
	for (int i = 0; i < in.nstock; ++i) out->dealer[i] = in.stock[i];
	out->ndealer = in.nstock;
	layer_objs_shops(out, false);
	NpcList *npcs = &in.npcs;
	blocker_sprites(npcs);
	for (int i = 0; i < in.ntalk; ++i)
		if (in.talkers[i].cat == 7) npc_need_sprite(npcs, 7, in.talkers[i].sprite);
	npcs->objects = props_objects(npcs);
	if (text.full || emu_debug_on())
		fprintf(stderr, "layer text: %d scripts, %d bytes%s\n", text.n, text.len, text.full ? " - FULL, lines left out" : "");
	/* (what a ScrtData is for, said as it is picked up: layer_words.c) */
	if (in.plan.fragment_placed) out->fragment_found = ta_say(&text, FACE_MEGAMAN, fragment_words(run.fragments));
	if (out->spin_colour) out->spin_found = spin_words(&text, out->spin_colour, in.spin_first);
	uint32_t archive = commit_text(&text, group, number);
	out->archive = archive;
	if (out->guardian.navi) guardian_actors(npcs, archive, &out->guardian);
	for (int i = 0; i < in.ntalk && npcs->n < 32; ++i) {
		const Talker *t = &in.talkers[i];
		uint32_t a = t->archive ? t->archive : archive;
		npcs->script[npcs->n++] = t->behind ? npc_counter_talker(t->cat, t->sprite, t->x, t->y, t->z, t->anim, a, t->script, t->sx, t->sy)
			: npc_talker(t->cat, t->sprite, t->x, t->y, t->z, t->cat == 7 ? 0 : 4, a, t->script, t->gone_flag, t->floor);
	}
	rush_install(group, number);
	return mapslot_install(group, number, npcs, in.md, in.nmd);
}
