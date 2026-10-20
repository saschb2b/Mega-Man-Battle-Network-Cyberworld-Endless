/* The economy report (docs/ECONOMY.md, docs/DEVTOOLS.md, issue #154): what
 * a run hands out, layer by layer and act by act, over many runs' seeds,
 * from the ROM without the game, under the flat plan (the rolls before
 * the issue) and the opening one in play (economy.h). Two kinds of
 * number, said apart:
 *
 * - the engine's, simulated with its own code: each layer as
 *   layer_generate makes it (its Mystery Data, dealers, Servers, traders,
 *   gates), each Mystery Data's roll (loot_mystery_roll), each dealer's
 *   stock (shop_dealer_stock), the vendor's, the guardians' Guardian
 *   Data, the requests' pay (jobs_offers);
 * - the game's, read from the ROM: each random battle (loot_random_battle)
 *   pays as BN6 picks a reward, from its enemies' reward rows by its
 *   busting table (bn6f sub_80AA910, sub_80AAA98; docs/ROM_DATA.md,
 *   Battle rewards), at an assumed mix of busting levels and battles a
 *   layer (econ_bust_weight, econ_battles_roll), leaned to the folder's
 *   codes as encounter.c's hook leans them.
 *
 * The run: the endless net's first cycle (the short net's are its first
 * nine layers), BN6's areas alone (where BN5's dress them, half its chips
 * would be BN5's of the same tiers), threat 0, the standard folder, no
 * helpers, every Mystery Data opened and every battle won. Each act's
 * numbers against its plan's bands (econ_band), flagged where they leave
 * them; the plan in play's count is the report's verdict. */
#include "economy_report.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip_pool.h"
#include "chips.h"
#include "data.h"
#include "economy.h"
#include "emu.h"
#include "game.h"
#include "guardians.h"
#include "jobs.h"
#include "loot.h"
#include "net.h"
#include "netmap.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "shop.h"
#include "xguardian.h"

#define RUNS_MAX 1000
#define LAYERS CYCLE_LAYERS
#define START_FOLDER 0x0213ACu   /* BN6's starting folder (docs/ROM_DATA.md, Starting state) */
#define BUSTING_TABLE 0x020B9Cu  /* BN6's busting table, 12 rows of 16 pairs' offsets (docs/ROM_DATA.md, Battle rewards) */
#define LOW_HP_SHARE 10          /* % of battles won at 3/8 of MegaMan's HP or less: the rows' second half (assumed) */
#define GEM_KEPT 50              /* % of a battle's Mystery Data still there at its end (assumed) */
#define CODES 27                 /* A-Z and * */
#define PLANS 2

/* What a run gains or is offered on one layer (summed over the runs) */
typedef struct {
	double md[4], md_chip, md_zenny, md_frags, md_hpmem, md_keys;
	double battles, bt_chip, bt_zenny, bt_hp, bt_frags;
	double gem, gem_chip, gem_zenny;
	double g_chip, g_zenny, g_hpmem;
	double dealers, dealer_chips, dealer_cost, hpmem_cost, vendors, vendor_cost;
	double servers, server_good, traders, bugtraders, vaults, officials, gates;
	double job_zenny, job_frags, job_chips, job_hpmem;
	double tier[CHIP_TIERS], cls[3], chips, good, strong, fit;
} LayerTally;

/* Where a reward lands in the tally */
enum { FROM_BATTLE, FROM_GEM, FROM_GUARDIAN };

/* One plan's measure: each layer's tally; per run and layer the codes
 * held (the most of one letter and the *s together), the zenny earned so
 * far and what the act's dealer asks for its answer and HPMemory so far */
typedef struct {
	LayerTally t[LAYERS];
	int together[RUNS_MAX][LAYERS], stars[RUNS_MAX][LAYERS], earned[RUNS_MAX][LAYERS], wanted[RUNS_MAX][LAYERS];
} Measure;

static Measure M[PLANS], S[PLANS];   /* the endless net's, the short net's */
static const EconPlan *const plans[PLANS] = { &econ_flat, &econ_opening };
static int runs;
static int codes_held[CODES];
static int zenny_run, wanted_run;
static int bust_fixed;   /* a busting level for every battle (the skill table), 0: the mix */
static int net_layers = LAYERS;   /* the net measured: the endless net's first cycle, or the short net (SHORT_LAYERS) */

static int cmp_int(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

static int median_at(int (*rows)[LAYERS], int col) {
	static int v[RUNS_MAX];
	for (int r = 0; r < runs; ++r) v[r] = rows[r][col];
	qsort(v, (size_t)runs, sizeof *v, cmp_int);
	return v[runs / 2];
}

/* A chip gained: its tier, class, a good one's share, its code */
static void gain_chip(LayerTally *t, int id, char code) {
	int tier = chip_pool_tier(id), cls = chip_pool_class(id);
	if (tier < 0) tier = 0;
	t->chips += 1;
	t->tier[tier] += 1;
	if (cls >= 0) t->cls[cls] += 1;
	t->good += econ_chip_good(tier);
	ChipInfo ci;
	chip_info(id, &ci);
	t->strong += chip_direct(id) && ci.power >= ECON_STRONG_POWER;
	int c = code == '*' ? 26 : code >= 'A' && code <= 'Z' ? code - 'A' : -1;
	if (c >= 0) ++codes_held[c];
	bool fits = code == '*';
	for (int k = 0; k < 3 && run.codes[k]; ++k) fits |= code == 'A' + run.codes[k] - 1;
	t->fit += fits;
}

/* BN6's reward pick as a battle ends (bn6f sub_80AA910, sub_80AAA98): one
 * of its enemies at random, then an entry of its row: one of the 16 pairs'
 * offsets of the busting level's row of the table, a coin within the pair,
 * the row's second half with MegaMan at 3/8 of his HP or less; the second
 * entry of a pair in the folder's codes by the plan's lean, and in a
 * guardian's own battle his Navi chip's entries zenny (encounter.c,
 * lean_row) */
static uint16_t reward_pick(const Encounter *e, int bust, bool guardian, int depth) {
	int ids[MAX_FOES], n = 0;
	/* (a guardian's by his family and version, as emu_encounter_set
	 * names him) */
	for (int i = 0; i < e->nfoes; ++i) {
		const Foe *f = &e->foes[i];
		if (f->kind != FOE_ROCK) ids[n++] = f->id >= 0 ? f->id : enemy_id(f->kind == FOE_NAVI ? 1 : 0, f->family, f->version);
	}
	if (!n) return 0xFFFF;
	int id = ids[rng_range(0, n - 1)];
	int pair = R.data[BUSTING_TABLE + (uint32_t)bust * 16u + (uint32_t)rng_range(0, 15)] / 4;
	int k = pair * 2 + rng_range(0, 1) + (rng_range(0, 99) < LOW_HP_SHARE ? 10 : 0);
	uint16_t v = enemy_drop(id, k);
	if (v == 0xFFFF || v >> 14) return v;
	int chip = v & 0x1FF, code = v >> 9 & 0x1F;
	if (guardian && (chip_family(chip) == CHIP_FAMILY_NAVI || xguardian_navi_chip(chip))) return (uint16_t)(1 << 14 | loot_guardian_zenny(depth));
	char c = code >= 26 ? '*' : (char)('A' + code);
	if ((k & 1) && econ_keep_by(ECON_LEVER_LEAN, depth, run.layer_seed ^ (uint32_t)id * 2654435761u ^ (uint32_t)k * 40503u))
		c = loot_fit_code(chip, c, true);
	return (uint16_t)(chip | (c == '*' ? 26 : c - 'A') << 9);
}

/* A busting level by the assumed mix (econ_bust_weight), or the fixed one */
static int bust_roll(void) {
	if (bust_fixed) return bust_fixed;
	int total = 0;
	for (int b = 1; b <= ECON_BUST_S; ++b) total += econ_bust_weight(b);
	int roll = rng_range(0, total - 1);
	for (int b = 1; b <= ECON_BUST_S; ++b) {
		if (roll < econ_bust_weight(b)) return b;
		roll -= econ_bust_weight(b);
	}
	return ECON_BUST_S;
}

/* A reward of BN6's encoding into the layer's tally */
static void gain_reward(LayerTally *t, uint16_t v, int from) {
	if (v == 0xFFFF) return;
	int kind = v >> 14, amount = v & 0x3FFF;
	double *chips = from == FROM_GEM ? &t->gem_chip : from == FROM_GUARDIAN ? &t->g_chip : &t->bt_chip;
	double *zenny = from == FROM_GEM ? &t->gem_zenny : from == FROM_GUARDIAN ? &t->g_zenny : &t->bt_zenny;
	if (kind == 0) {
		gain_chip(t, v & 0x1FF, (v >> 9 & 0x1F) >= 26 ? '*' : (char)('A' + (v >> 9 & 0x1F)));
		*chips += 1;
	} else if (kind == 1) {
		*zenny += amount;
		zenny_run += amount;
	} else if (kind == 2) t->bt_hp += 1;
	else t->bt_frags += amount;
}

/* The layer's random battles, each won: their rewards, and a Mystery Data
 * on the field where one is kept to the end */
static void layer_battles(LayerTally *t, int depth, int biome) {
	int n = econ_battles_roll();
	for (int k = 0; k < n; ++k) {
		bool opening = layer_in_act(depth) == 0 && (depth == 1 ? k < 2 : true);
		int kind = depth == 1 && k == 0 ? ENC_FIRST : opening ? ENC_EASY : ENC_NORMAL;
		Encounter e = loot_random_battle(depth, biome, kind, run.layer_seed, k);
		loot_battle_fought(&e);
		t->battles += 1;
		gain_reward(t, reward_pick(&e, bust_roll(), false, depth), FROM_BATTLE);
		for (int i = 0; i < e.nobj; ++i)
			if (e.obj[i].kind >> 4 == FIELD_GEM && rng_range(0, 99) < GEM_KEPT) {
				uint16_t r[4];
				loot_gem_rewards(depth, r);
				t->gem += 1;
				gain_reward(t, r[rng_range(0, 3)], FROM_GEM);
			}
	}
}

/* The guardian's battle and his Guardian Data: the game's reward (his Navi
 * chip's entries zenny), his chip and five HPMemory (docs/PROGRESSION.md,
 * Growth) */
static void layer_guardian(LayerTally *t, int depth, int biome) {
	int navi = run_guardian(biome);
	if (navi <= 0 || guardian_older(navi)) return;
	Encounter e = make_boss(depth, biome, navi);
	gain_reward(t, reward_pick(&e, bust_roll(), true, depth), FROM_GUARDIAN);
	int chip = e.nfoes ? navi_chip(navi, e.foes[0].version) : 0;
	if (chip > 0) {
		ChipInfo ci;
		chip_info(chip, &ci);
		gain_chip(t, chip, ci.ncodes ? ci.codes[0] : '*');
		t->g_chip += 1;
	}
	t->g_hpmem += 5;
}

/* A purple data's chip, as layer_objs.c's purple_content rolls it (no
 * dealer's stock to leave out here) */
static void purple_find(LayerTally *t, int depth) {
	char code = '*';
	int id = roll_chip(depth, 4, &code);
	for (int k = 0; k < 16 && (code == '*' || chip_pool_tier(id) < 2); ++k) id = roll_chip(depth, 4, &code);
	gain_chip(t, id, code);
	t->md_chip += 1;
}

/* The layer's Mystery Data, every one opened: what each holds, but the
 * one a RegUp or a ScrtData takes (layer_objs.c, fill_mystery) */
static void layer_mystery(LayerTally *t, int depth, int *fragments) {
	bool reg = layer_regup(depth, LAYER_NORMAL) > 0;
	bool frag = *fragments < 3 && depth >= 4 && rng_range(0, 99) < 35;
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		if (o->type != OBJ_MYSTERY) continue;
		t->md[o->param == MD_PURPLE ? 3 : o->param] += 1;
		if (o->param == MD_PURPLE) { purple_find(t, depth); continue; }
		if (reg && o->param >= 1) { reg = false; t->md_keys += 1; continue; }
		if (frag) { frag = false; ++*fragments; t->md_keys += 1; continue; }
		LootFind f = loot_mystery_roll(o->param, depth);
		if (f.kind == LOOT_FIND_CHIP) { gain_chip(t, f.value, f.code); t->md_chip += 1; }
		else if (f.kind == LOOT_FIND_ZENNY) { t->md_zenny += f.value; zenny_run += f.value; }
		else if (f.kind == LOOT_FIND_BUGFRAGS) t->md_frags += f.value;
		else t->md_hpmem += 1;
	}
}

/* The Net Dealer's stock, as layer_objs.c's install_begin asks for it;
 * the act's middle layer's answer (his first chip, all he has of it) and
 * HPMemory are what a run wants of him */
static void layer_dealer(LayerTally *t, int depth, int biome) {
	int navi = run_guardian(biome);
	int counter = counter_element(depth, biome, navi);
	bool weakless = navi > 0 && guardian_weakness(navi) <= 0;
	ShopItem stock[SHOP_MAX_ITEMS];
	int n = shop_dealer_stock(depth, weakless ? -1 : counter, weakless ? counter : 0, stock);
	t->dealers += 1;
	for (int i = 0; i < n; ++i) {
		int cost = stock[i].price * 100 * (stock[i].kind == 2 ? stock[i].stock : 1);
		bool wanted = layer_in_act(depth) == 1 && (i == 0 || (stock[i].kind == 1 && stock[i].id == 0x70));
		if (stock[i].kind == 2) { t->dealer_chips += stock[i].stock; t->dealer_cost += cost; }
		else if (stock[i].kind == 1 && stock[i].id == 0x70) t->hpmem_cost += cost;
		if (wanted) wanted_run += cost;
	}
}

/* A Server's prize: the hardest hitting of a dozen two layers deeper
 * (layer_objs.c, install_server, without its folder and stock checks) */
static void layer_server(LayerTally *t, int depth) {
	int best = -1, power = -1;
	for (int k = 0; k < 12; ++k) {
		char code;
		int id = roll_chip(depth + 2, 4, &code);
		ChipInfo ci;
		chip_info(id, &ci);
		int p = chip_direct(id) ? ci.power : 0;
		if (p > power) { power = p; best = id; }
	}
	t->servers += 1;
	t->server_good += best > 0 && econ_chip_good(chip_pool_tier(best));
}

/* What stands on the layer besides its data: dealers, vendors, Servers,
 * traders, gates */
static void layer_services(LayerTally *t, int depth, int biome) {
	for (int i = 0; i < layer.nobj; ++i) {
		switch (layer.obj[i].type) {
		case OBJ_SHOP: layer_dealer(t, depth, biome); break;
		case OBJ_PROGRAMS: {
			ShopItem p[SHOP_MAX_ITEMS];
			int n = shop_program_stock(depth, p);
			t->vendors += 1;
			for (int k = 0; k < n; ++k) t->vendor_cost += p[k].price * 100;
			break;
		}
		case OBJ_CHALLENGE: layer_server(t, depth); break;
		case OBJ_TRADER: t->traders += 1; break;
		case OBJ_BUGTRADER: t->bugtraders += 1; break;
		case OBJ_VAULT: t->vaults += 1; break;
		case OBJ_OFFICIAL: t->officials += 1; break;
		case OBJ_NAVI_GATE: t->gates += 1; break;
		default: break;
		}
	}
}

/* The requests home offers before an act (each of three, as offered) */
static void act_jobs(LayerTally *t, int depth) {
	Job j[JOB_ASKERS];
	jobs_offers(run.seed, depth, j);
	for (int k = 0; k < JOB_ASKERS; ++k) {
		if (j[k].pay_kind == PAY_ZENNY) t->job_zenny += j[k].pay;
		else if (j[k].pay_kind == PAY_BUGFRAGS) t->job_frags += j[k].pay;
		else if (j[k].pay_kind == PAY_CHIP) t->job_chips += 1;
		else t->job_hpmem += j[k].pay;
	}
}

/* The run's folder: BN6's starting one, its codes as each layer reads them */
static void start_folder(void) {
	uint16_t folder[30];
	for (int i = 0; i < 30; ++i) folder[i] = rom_u16(START_FOLDER + 2u * (uint32_t)i);
	loot_folder_codes(folder, 30, run.codes);
	loot_folder_counts(folder, 30);
}

/* One layer of run `r`, made and emptied as a run would */
static void one_layer(Measure *m, int r, int depth, int *fragments) {
	LayerTally *t = &m->t[depth - 1];
	int biome = biome_for_depth(depth);
	run.depth = depth;
	run.biome = biome;
	run.side_kind = LAYER_NORMAL;
	run.layer_seed = run.seed ^ (uint32_t)(depth * 2654435761u);
	loot_layer_nets(-1, -1);
	LayerKit kit;
	netmap_kit(biome, &kit);
	layer_generate(run.layer_seed, depth, biome, LAYER_NORMAL, &kit);
	rng_seed(run.layer_seed ^ 0xEC0A0001u);
	if (layer_in_act(depth) == 0) act_jobs(t, depth);
	layer_services(t, depth, biome);
	layer_mystery(t, depth, fragments);
	layer_battles(t, depth, biome);
	if (is_boss_depth(depth)) layer_guardian(t, depth, biome);
	int top = 0;
	for (int c = 0; c < 26; ++c) if (codes_held[c] > top) top = codes_held[c];
	m->together[r][depth - 1] = top + codes_held[26];
	m->stars[r][depth - 1] = codes_held[26];
	m->earned[r][depth - 1] = zenny_run;
	m->wanted[r][depth - 1] = wanted_run;
}

static void one_run(Measure *m, int r) {
	run_new(0xEC0A0000u + (uint32_t)r * 7919u);
	run.mode = net_layers == SHORT_LAYERS ? RUN_SHORT : RUN_ENDLESS;
	start_folder();
	memset(codes_held, 0, sizeof codes_held);
	zenny_run = wanted_run = 0;
	int fragments = 0;
	for (int depth = 1; depth <= net_layers; ++depth) one_layer(m, r, depth, &fragments);
}

/* Every run of a net of `layers` under `plan` */
static void measure(Measure *m, const EconPlan *plan, int layers) {
	const EconPlan *was = econ_plan;
	econ_plan = plan;
	net_layers = layers;
	memset(m->t, 0, sizeof m->t);
	for (int r = 0; r < runs; ++r) one_run(m, r);
	econ_plan = was;
	net_layers = LAYERS;
}

/* (the Nest the net's last layer) */
static void act_range(int act, int *first, int *last) {
	*first = act == ECON_ACTS - 1 ? net_layers : act * 3 + 1;
	*last = act == ECON_ACTS - 1 ? net_layers : *first + 2;
}

/* An act's layers summed, per run */
static LayerTally act_sum(const Measure *m, int act) {
	int first, last;
	act_range(act, &first, &last);
	LayerTally s;
	memset(&s, 0, sizeof s);
	double *o = (double *)&s;
	for (int d = first; d <= last; ++d) {
		const double *v = (const double *)&m->t[d - 1];
		for (size_t k = 0; k < sizeof s / sizeof *o; ++k) o[k] += v[k] / runs;
	}
	return s;
}

static const char *const act_names[ECON_ACTS] = { "1", "2", "3", "4", "5", "6", "Nest" };

/* An act's five measured numbers, the bands' (econ_band) */
static void act_metrics(Measure *m, int act, double v[ECON_METRICS]) {
	int first, last;
	act_range(act, &first, &last);
	LayerTally s = act_sum(m, act);
	v[ECON_CHIPS] = s.chips;
	v[ECON_GOOD] = s.chips ? 100 * s.good / s.chips : 0;
	v[ECON_MD] = s.md[0] + s.md[1] + s.md[2] + s.md[3];
	v[ECON_ZENNY] = s.bt_zenny + s.md_zenny + s.gem_zenny + s.g_zenny;
	v[ECON_CODES] = median_at(m->together, last - 1);
}

/* The before-and-after table of a net (the endless net's six acts and
 * its Nest, `ms` its two plans' measures, or the short net's three and
 * its Nest): each act's numbers under both plans, each against its own
 * plan's bands (! outside); how many of the plan in play's are outside */
static int compare_table(FILE *out, Measure *ms, bool short_net) {
	static const char *const names[ECON_METRICS] = { "chips", "good%", "MD", "zenny", "codes" };
	static const int endless[] = { 0, 1, 2, 3, 4, 5, ECON_ACTS - 1 }, brief[] = { 0, 1, 2, ECON_ACTS - 1 };
	const int *acts = short_net ? brief : endless;
	int nacts = short_net ? 4 : 7, flagged = 0;
	net_layers = short_net ? SHORT_LAYERS : LAYERS;
	if (short_net) fprintf(out, "\nThe short net (a new run's: layers 1-%d, its Nest the last), before (flat) -> now (opening), as above:\nact  ", SHORT_LAYERS);
	else fprintf(out, "\nBy act, before (flat) -> now (opening) (per run, the act's layers together; codes: the most chips gained in one letter\n"
		"with the *s at the act's end, median run; ! outside its plan's band, economy.h):\nact  ");
	for (int k = 0; k < ECON_METRICS; ++k) fprintf(out, " %17s", names[k]);
	fprintf(out, "   earned/wanted so far (median)\n");
	for (int i = 0; i < nacts; ++i) {
		int first, last;
		act_range(acts[i], &first, &last);
		double v[PLANS][ECON_METRICS];
		for (int p = 0; p < PLANS; ++p) act_metrics(&ms[p], acts[i], v[p]);
		fprintf(out, "%-4s ", act_names[acts[i]]);
		for (int k = 0; k < ECON_METRICS; ++k) {
			bool out_of[PLANS];
			for (int p = 0; p < PLANS; ++p) {
				EconBand band = econ_band(plans[p], short_net, k, acts[i]);
				out_of[p] = v[p][k] < band.lo || v[p][k] > band.hi;
			}
			flagged += plans[0] == econ_plan ? out_of[0] : out_of[1];
			fprintf(out, " %7.1f%s->%7.1f%s", v[0][k], out_of[0] ? "!" : " ", v[1][k], out_of[1] ? "!" : " ");
		}
		fprintf(out, "   %6d/%-6d -> %6d/%d\n", median_at(ms[0].earned, last - 1), median_at(ms[0].wanted, last - 1),
			median_at(ms[1].earned, last - 1), median_at(ms[1].wanted, last - 1));
	}
	net_layers = LAYERS;
	return flagged;
}

/* Where the chips and zenny come from, by act */
static void sources_table(FILE *out, const Measure *m, const char *name) {
	fprintf(out, "\nWhere an act's chips and zenny come from, %s (per run): its battles (BN6's rows), the field's Mystery\n"
		"Data in them, the layers' Mystery Data, the guardian; and what is offered: dealers' chips, Servers' prizes (good%% of\n"
		"them), the requests' pay (all three offered), traders and gates\n"
		"act  battles chip/zenny/hp   gem chip/zenny   MD chip/zenny/frags/hpmem   guardian chip/zenny/hpmem   dealers chips $"
		"      servers good%%  requests zenny/frags/chips/hpmem   traders bug  vault official gate\n", name);
	for (int a = 0; a < ECON_ACTS; ++a) {
		LayerTally s = act_sum(m, a);
		fprintf(out, "%-4s %5.1f %4.1f/%5.0f/%4.2f  %4.2f/%5.0f   %4.1f/%5.0f/%4.1f/%4.2f    %4.2f/%5.0f/%4.1f       %4.2f %5.1f %6.0f  "
			"%4.2f %5.0f   %5.0f/%4.1f/%4.1f/%4.1f       %4.2f %4.2f  %4.2f %4.2f %4.2f\n",
			act_names[a], s.battles, s.bt_chip, s.bt_zenny, s.bt_hp, s.gem_chip, s.gem_zenny, s.md_chip, s.md_zenny, s.md_frags, s.md_hpmem,
			s.g_chip, s.g_zenny, s.g_hpmem, s.dealers, s.dealer_chips, s.dealer_cost + s.hpmem_cost, s.servers,
			s.servers ? 100 * s.server_good / s.servers : 0, s.job_zenny, s.job_frags, s.job_chips, s.job_hpmem, s.traders, s.bugtraders,
			s.vaults, s.officials, s.gates);
	}
}

/* Rarity: the chips gained by tier, and how many fit the folder's codes */
static void rarity_table(FILE *out) {
	fprintf(out, "\nThe chips gained by tier (chip_pool.h: 0 common, 1 uncommon, 2 rare standard, 3 rarer or Mega, 4 Mega 5-star\n"
		"or Giga), %% of the act's, before (flat) -> now (opening); good = tier %d or more; strong = a straight hit of %d or more; fit = in\n"
		"one of the folder's codes or *\n"
		"act   tier0      tier1      tier2      tier3      tier4      strong%%    fit%%       mega        giga\n", ECON_GOOD_TIER,
		ECON_STRONG_POWER);
	for (int a = 0; a < ECON_ACTS; ++a) {
		LayerTally s[PLANS];
		for (int p = 0; p < PLANS; ++p) s[p] = act_sum(&M[p], a);
		fprintf(out, "%-4s ", act_names[a]);
		for (int k = 0; k < CHIP_TIERS; ++k)
			fprintf(out, " %3.0f -> %3.0f", 100 * s[0].tier[k] / (s[0].chips ? s[0].chips : 1), 100 * s[1].tier[k] / (s[1].chips ? s[1].chips : 1));
		fprintf(out, " %3.0f -> %3.0f %3.0f -> %3.0f %4.2f -> %4.2f %4.2f -> %4.2f\n", 100 * s[0].strong / (s[0].chips ? s[0].chips : 1),
			100 * s[1].strong / (s[1].chips ? s[1].chips : 1), 100 * s[0].fit / (s[0].chips ? s[0].chips : 1),
			100 * s[1].fit / (s[1].chips ? s[1].chips : 1), s[0].cls[1], s[1].cls[1], s[0].cls[2], s[1].cls[2]);
	}
}

/* By layer: the Mystery Data laid (the rich among the blue), the chips
 * gained, the codes held and the *s among them, the zenny earned so far
 * (median run), both plans */
static void layers_table(FILE *out) {
	fprintf(out, "\nBy layer (per run), before (flat) -> now (opening): Mystery Data laid (green / blue, rich of them / purple), chips gained, the\n"
		"codes held (the most in one letter with the *s) and the *s, the zenny earned so far (median run)\n"
		"layer   green          blue           rich           purple      chips           codes      *s         zenny so far\n");
	for (int d = 1; d <= LAYERS; ++d) {
		const LayerTally *t = &M[0].t[d - 1], *u = &M[1].t[d - 1];
		fprintf(out, "%5d   %4.2f -> %4.2f   %4.2f -> %4.2f   %4.2f -> %4.2f   %4.2f -> %4.2f  %5.2f -> %5.2f   %2d -> %2d   %2d -> %2d   %6d -> %6d\n",
			d, t->md[0] / runs, u->md[0] / runs, (t->md[1] + t->md[2]) / runs, (u->md[1] + u->md[2]) / runs, t->md[2] / runs, u->md[2] / runs,
			t->md[3] / runs, u->md[3] / runs, t->chips / runs, u->chips / runs, median_at(M[0].together, d - 1), median_at(M[1].together, d - 1),
			median_at(M[0].stars, d - 1), median_at(M[1].stars, d - 1), median_at(M[0].earned, d - 1), median_at(M[1].earned, d - 1));
	}
}

/* The opening plan's curves: each lever's share by act, on the endless net
 * and the short one */
static void curves_table(FILE *out) {
	static const char *const levers[ECON_LEVERS] = { "green MD", "MD chips", "good weight", "zenny", "lean" };
	fprintf(out, "\nThe opening plan's curves, in play (economy.c: floor + (1 - floor) ln(1 + bend x) / ln(1 + bend), x the act over the net's\n"
		"last before its Nest): each lever's share of the flat plan's by act, the endless net | the short net\nlever        floor bend ");
	for (int a = 0; a < 6; ++a) fprintf(out, "  act %d", a + 1);
	fprintf(out, " |");
	for (int a = 0; a < 3; ++a) fprintf(out, "  act %d", a + 1);
	fprintf(out, "\n");
	for (int l = 0; l < ECON_LEVERS; ++l) {
		EconCurve c = econ_opening.lever[l];
		fprintf(out, "%-12s %4.2f %4.1f ", levers[l], c.floor, c.bend);
		for (int a = 0; a < 6; ++a) fprintf(out, "  %5.2f", econ_curve(c, a / 5.0));
		fprintf(out, " |");
		for (int a = 0; a < 3; ++a) fprintf(out, "  %5.2f", econ_curve(c, a / 2.0));
		fprintf(out, "\n");
	}
}

/* BN6's own skill table: a battle's reward at each busting level, the
 * plan in play, acts 1 and 4 (the rows' chip share and zenny a battle) */
static void skill_table(FILE *out) {
	static const int levels[] = { 4, 6, 8, 10, ECON_BUST_S };
	fprintf(out, "\nBusting level pays (BN6's table, the plan in play): a battle's chip %% and zenny, every battle at one level\nact ");
	for (unsigned k = 0; k < sizeof levels / sizeof *levels; ++k) fprintf(out, "   level %-2s     ", levels[k] == ECON_BUST_S ? "S" : "");
	fprintf(out, "\n");
	int keep = runs;
	runs = runs < 100 ? runs : 100;
	static Measure probe;
	for (int a = 0; a < 6; a += 3) {
		fprintf(out, "%-4d", a + 1);
		for (unsigned k = 0; k < sizeof levels / sizeof *levels; ++k) {
			bust_fixed = levels[k];
			measure(&probe, econ_plan, LAYERS);
			LayerTally s = act_sum(&probe, a);
			fprintf(out, "  %3d: %3.0f%% %4.0f", levels[k], s.battles ? 100 * s.bt_chip / s.battles : 0, s.battles ? s.bt_zenny / s.battles : 0);
		}
		fprintf(out, "\n");
	}
	bust_fixed = 0;
	runs = keep;
}

int economy_report_run(const char *spec) {
	char path[256] = "economy.txt";
	runs = 300;
	sscanf(spec, "%255[^:]:%d", path, &runs);
	if (runs < 1) runs = 1;
	if (runs > RUNS_MAX) runs = RUNS_MAX;
	if (!emu_init(R.data, ROM_SIZE)) return 1;
	FILE *out = fopen(path, "w");
	if (!out) return 1;
	for (int p = 0; p < PLANS; ++p) {
		measure(&M[p], plans[p], LAYERS);
		measure(&S[p], plans[p], SHORT_LAYERS);
	}
	start_folder();
	fprintf(out, "Economy report (docs/ECONOMY.md): %d runs of the endless net's first cycle, BN6's areas, threat 0, BN6's starting\n"
		"folder (codes %c %c %c: the finds lean to them), every Mystery Data opened, %d-%d random battles a layer won at busting\n"
		"levels by an assumed mix (econ_bust_weight), %d%% of them at low HP, a battle's Mystery Data kept %d%% of the time.\n"
		"The plan in play: %s.\n", runs, run.codes[0] ? 'A' + run.codes[0] - 1 : '-', run.codes[1] ? 'A' + run.codes[1] - 1 : '-',
		run.codes[2] ? 'A' + run.codes[2] - 1 : '-', ECON_BATTLES_LEAST, ECON_BATTLES_MOST, LOW_HP_SHARE, GEM_KEPT, econ_plan->name);
	int flagged = compare_table(out, M, false) + compare_table(out, S, true);
	sources_table(out, &M[0], "before (flat)");
	sources_table(out, &M[1], "now (opening)");
	rarity_table(out);
	layers_table(out);
	curves_table(out);
	skill_table(out);
	fclose(out);
	printf("economy report in %s: %d numbers of the plan in play (%s) outside their band\n", path, flagged, econ_plan->name);
	return 0;
}
