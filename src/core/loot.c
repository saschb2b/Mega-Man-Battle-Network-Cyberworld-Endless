#include "loot.h"

#include <string.h>

#include "data.h"
#include "chip_pool.h"
#include "formations.h"
#include "game.h"
#include "pacing.h"
#include "run.h"
#include "rom.h"

#define NAVI_CHALLENGE 40   /* % of challenges from the fourth act on against one of the area's SP navis */

/* When a virus family first meets MegaMan on the net: 0 the surface
 * areas, 1 the Undernet, 2 the Graveyard and the Underground (3 never). The
 * comps and homepages hold families of every stage and keep to the ones the
 * run has reached. */
static int family_stage(int family) {
	static int8_t stage[32];
	static bool known;
	if (!known) {
		known = true;
		for (int f = 0; f < 32; ++f) stage[f] = 3;
		static const struct { int biome, stage; } areas[] = {
			{ BIOME_CENTRAL, 0 }, { BIOME_SEASIDE, 0 }, { BIOME_SKY, 0 }, { BIOME_GREEN, 0 },
			{ BIOME_UNDERNET, 1 }, { BIOME_GRAVEYARD, 2 }, { BIOME_NEST, 2 },
		};
		for (unsigned a = 0; a < sizeof areas / sizeof *areas; ++a) {
			const Formation *list;
			int n = formations_of(areas[a].biome, &list);
			for (int i = 0; i < n; ++i)
				for (int k = 0; k < list[i].n; ++k) {
					const uint8_t *row = R.data + R.layout->enemy_ids + list[i].ent[k].id * 3;
					if (row[1] == 0 && row[2] < 32 && stage[row[2]] > areas[a].stage) stage[row[2]] = (int8_t)areas[a].stage;
				}
		}
	}
	return family >= 0 && family < 32 ? stage[family] : 3;
}

/* How many viruses a formation sets out (not its rocks and cubes). */
static int viruses_in(const Formation *f) {
	int n = 0;
	for (int k = 0; k < f->n; ++k) {
		const uint8_t *row = R.data + R.layout->enemy_ids + f->ent[k].id * 3;
		if (row[1] == 0 && row[2] >= 1 && row[2] <= 29) ++n;
	}
	return n;
}

static bool reached(const Formation *f, int allowed) {
	for (int k = 0; k < f->n; ++k) {
		const uint8_t *row = R.data + R.layout->enemy_ids + f->ent[k].id * 3;
		if (row[1] == 0 && row[2] >= 1 && row[2] <= 29 && family_stage(row[2]) > allowed) return false;
	}
	return true;
}

/* enemy_id for viruses, remembered (families 1-29, versions 0-5) */
static int virus_id(int family, int version) {
	static int16_t ids[30][6];
	static bool known;
	if (!known) {
		known = true;
		for (int f = 0; f < 30; ++f)
			for (int v = 0; v < 6; ++v) ids[f][v] = (int16_t)(f ? enemy_id(0, f, v) : -1);
	}
	return family >= 1 && family <= 29 && version >= 0 && version <= 5 ? ids[family][version] : -1;
}

/* A formation's foes with its viruses at `want` (the originals are paced
 * for the story; the late story's heavy families, the dragons and
 * Nightmare, met in packs, stay at their first version on the first
 * cycle), one of them rare with `rare`; their HP together and the
 * strongest one's damage. */
#define GEM_KEEP 35   /* % of the battles whose original holds a Mystery Data that keep it */
static bool gem_kept;    /* the battle being built keeps its Mystery Data */

static void build_foes(const Formation *f, int depth, int want, bool rare, Encounter *e, int *hp, int *dmg) {
	e->nfoes = 0;
	e->field = f->battlefield;
	e->player = f->player;
	e->nobj = 0;
	for (int i = 0; i < f->nobj && i < MAX_FIELD_OBJS; ++i) {
		if (f->obj[i].kind >> 4 == FIELD_GEM && !gem_kept) continue;
		e->obj[e->nobj].kind = f->obj[i].kind;
		e->obj[e->nobj].panel = f->obj[i].panel;
		e->obj[e->nobj++].arg = f->obj[i].id;
	}
	*hp = *dmg = 0;
	for (int i = 0; i < f->n && e->nfoes < MAX_FOES; ++i) {
		const uint8_t *row = R.data + R.layout->enemy_ids + f->ent[i].id * 3;
		Foe *o = &e->foes[e->nfoes++];
		o->version = row[0];
		o->family = row[2];
		o->col = (f->ent[i].panel & 15) - 1;
		o->row = (f->ent[i].panel >> 4) - 1;
		o->id = f->ent[i].id;
		/* a virus is the first entry of its family and version; the rest of
		 * the table's type 0 are rocks, cubes and other objects */
		if (row[1] == 1) o->kind = FOE_NAVI;
		else if (row[1] == 0 && o->family >= 1 && o->family <= 29 && virus_id(o->family, o->version) == o->id) o->kind = FOE_VIRUS;
		else { o->kind = FOE_ROCK; continue; }
		if (o->kind == FOE_VIRUS && o->version <= 3) {
			int v = depth <= CYCLE_LAYERS && family_stage(o->family) >= 2 ? 0 : want;
			if (rare) {
				/* the first virus with a rare form takes it */
				int r = virus_id(o->family, pacing_act(depth) < 4 ? 4 : 5);
				if (r >= 0) { v = pacing_act(depth) < 4 ? 4 : 5; rare = false; }
			}
			int id = virus_id(o->family, v);
			if (id >= 0) { o->version = v; o->id = id; }
		}
		int h, d;
		if (enemy_stats(o->id, &h, &d)) {
			*hp += h;
			if (d > *dmg) *dmg = d;
		}
	}
}

/* (a rare find, rarer than a blue Mystery Data's: three chips a tier above
 * its, each in the folder's codes where one comes in them, and zenny twice
 * its; threat 6's chips only, a fourth chip for the zenny. A playtester's
 * gem paid 980 zenny, a blue Mystery Data's on the same layer, and read as
 * no find at all) */
void loot_gem_rewards(int depth, uint16_t out[4]) {
	for (int k = 0; k < 4; ++k) {
		if (k == 3 && run.threat < 6) break;
		char code = '*';
		int id = roll_chip(depth, 2, &code);
		for (int t = 0; t < 12 && run.codes[0] && !loot_folder_code(id, false); ++t) id = roll_chip(depth, 2, &code);
		char c = loot_folder_code(id, true);
		if (c) code = c;
		out[k] = (uint16_t)((id & 0x1FF) | (code == '*' ? 26 : code - 'A') << 9);
	}
	int zenny = 1600 + depth * 120;
	if (run.threat < 6) out[3] = (uint16_t)(0x4000 | (zenny > 0x3FFF ? 0x3FFF : zenny));
}

void loot_add_gem(Encounter *e) {
	for (int i = 0; i < e->nobj; ++i) if (e->obj[i].kind >> 4 == FIELD_GEM) return;
	if (e->nobj >= MAX_FIELD_OBJS) return;
	static const uint8_t panels[] = { 0x26, 0x16, 0x36, 0x25, 0x15, 0x35 };
	for (unsigned k = 0; k < sizeof panels; ++k) {
		int col = (panels[k] & 15) - 1, row = (panels[k] >> 4) - 1;
		bool taken = false;
		for (int i = 0; i < e->nfoes; ++i) taken |= e->foes[i].col == col && e->foes[i].row == row;
		for (int i = 0; i < e->nobj; ++i) taken |= e->obj[i].panel == panels[k];
		if (taken) continue;
		e->obj[e->nobj].kind = FIELD_GEM << 4;
		e->obj[e->nobj].panel = panels[k];
		e->obj[e->nobj++].arg = 0;
		return;
	}
}

/* The element that answers an act: the one strong against its guardian,
 * or with a guardian of none, against the element half or more of the
 * kinds of virus the area's battles can hold at `depth` have, two kinds at
 * least (Fire beats Wood, Aqua Fire, Elec Aqua, Wood Elec); 0 for none. A
 * playtester told "the viruses around here can't stand Aqua chips" met
 * Piranhas, bees, crows and planes: the area's most common element over
 * all its battles had been three kinds of Fire among nine elemental. */
int counter_element(int depth, int biome, int navi) {
	static const int beats[5] = { 0, ELEM_AQUA, ELEM_ELEC, ELEM_WOOD, ELEM_FIRE };
	int e = navi > 0 ? enemy_element(enemy_id(1, navi, 0)) : -1;
	if (e > 0 && e <= 4) return beats[e];
	uint32_t fams = loot_families_here(depth, biome);
	int count[5] = { 0 }, total = 0;
	for (int f = 1; f < 30; ++f) {
		if (!(fams >> f & 1)) continue;
		int v = enemy_element(virus_id(f, 0));
		if (v > 0 && v <= 4) { ++count[v]; ++total; }
	}
	int best = 0;
	for (int v = 1; v <= 4; ++v) if (count[v] > count[best]) best = v;
	return best && count[best] >= 2 && count[best] * 2 >= total ? beats[best] : 0;
}

/* the last battle fought, not the last rolled: the next random battle is
 * rolled again every few seconds, and those rolls had pushed a Server's
 * battle out of mind before the fight after it (loot_battle_fought) */
static int last_biome = -1, last_pick = -1;   /* no formation twice in a row */
static uint32_t last_families;                  /* the virus families of the last battle (a bit each) */
static uint32_t last_viruses;                   /* ... and its viruses (viruses_of) */
static uint32_t before_families;                /* the battle's before it, its families */

void loot_battle_fought(const Encounter *e) {
	if (e->from_pick < 0) return;
	before_families = last_families;
	last_biome = e->from_biome;
	last_pick = e->from_pick;
	last_families = e->from_families;
	last_viruses = e->from_viruses;
}

void loot_memory(LootMemory *out) {
	*out = (LootMemory){ last_biome, last_pick, last_families, last_viruses, before_families };
}

void loot_memory_set(const LootMemory *m) {
	last_biome = m->biome;
	last_pick = m->pick;
	last_families = m->families;
	last_viruses = m->viruses;
	before_families = m->families_before;
}

/* Formation `f`'s viruses, whatever their panels: the same few viruses
 * stand in several of an area's records. */
static uint32_t viruses_of(const Formation *f) {
	uint16_t id[FORMATION_MAX_ENTS];
	int n = f->n < FORMATION_MAX_ENTS ? f->n : FORMATION_MAX_ENTS;
	for (int k = 0; k < n; ++k) id[k] = f->ent[k].id;
	for (int i = 1; i < n; ++i)
		for (int j = i; j > 0 && id[j - 1] > id[j]; --j) { uint16_t t = id[j]; id[j] = id[j - 1]; id[j - 1] = t; }
	uint32_t h = 2166136261u;
	for (int k = 0; k < n; ++k) h = (h ^ id[k]) * 16777619u;
	return h;
}

/* The family of formation `f`'s first virus (0 none): the one a run's
 * first battle keeps. */
#define FAMILY_PIRANHA 2
static int first_family(const Formation *f) {
	for (int k = 0; k < f->n; ++k) {
		const uint8_t *row = R.data + R.layout->enemy_ids + f->ent[k].id * 3;
		if (row[1] == 0 && row[2] >= 1 && row[2] <= 29) return row[2];
	}
	return 0;
}

/* The virus families formation `f` brings, a bit each. */
static uint32_t families_of(const Formation *f) {
	uint32_t in = 0;
	for (int k = 0; k < f->n; ++k) {
		const uint8_t *row = R.data + R.layout->enemy_ids + f->ent[k].id * 3;
		if (row[1] == 0 && row[2] > 0 && row[2] < 32) in |= 1u << row[2];
	}
	return in;
}

/* How many of formation `f`'s viruses are of `family`. */
static int family_count(const Formation *f, int family) {
	int n = 0;
	for (int k = 0; k < f->n; ++k) {
		const uint8_t *row = R.data + R.layout->enemy_ids + f->ent[k].id * 3;
		n += row[1] == 0 && row[2] == family;
	}
	return n;
}

/* DarkMechs teleport beside MegaMan and slash, for more than the damage the
 * pacing reads (a playtester took 60-110 a slash): two in one battle took
 * one from 480 HP to 20 in act 3, and neither could be kept out of reach.
 * Not two in a battle before the Undernet, their own area (act 5); CopyBot's
 * comps hold them only in pairs, so they wait for it. */
static bool too_many_slashers(const Formation *f, int depth) {
	return pacing_act(depth) < 4 && family_count(f, FAMILY_DARKMECH) > 1;
}

#define MAX_FIT 160

/* Each formation's version inside the band (fit[i], -1 for none), and their
 * weight together: the ones that reach the band's aim if any do, else the
 * lighter ones. `skip` is left out. */
static int weigh(const Formation *list, int n, int depth, int target, PacingBand band, int allowed, int skip, int most,
                 int8_t fit[MAX_FIT]) {
	static uint8_t inside[MAX_FIT];
	int in_total = 0, any_total = 0;
	for (int i = 0; i < n && i < MAX_FIT; ++i) {
		fit[i] = -1;
		inside[i] = 0;
		if (i == skip || list[i].navi || !reached(&list[i], allowed) || viruses_in(&list[i]) > most || too_many_slashers(&list[i], depth)) continue;
		for (int v = target; v >= 0; --v) {
			Encounter t;
			int hp, dmg;
			build_foes(&list[i], depth, v, false, &t, &hp, &dmg);
			if (hp > band.hi || dmg > band.cap) continue;
			fit[i] = (int8_t)v;
			inside[i] = hp >= band.lo;
			break;
		}
		if (fit[i] < 0) continue;
		any_total += list[i].weight;
		if (inside[i]) in_total += list[i].weight;
	}
	if (!in_total) return any_total;
	for (int i = 0; i < n && i < MAX_FIT; ++i) if (!inside[i]) fit[i] = -1;
	return in_total;
}

/* A formation from the area's original random battles, at the highest
 * version up to the act's that keeps it inside the act's band
 * (docs/PROGRESSION.md). False when the ROM has none for the area. */
/* The area whose battles an area with few of its own shares, and in how
 * many thirds of them: the Sky and Green homepages' random battles are one
 * formation each (every act 2 battle on Sky HP was Gunner and FgtrPlne),
 * and each stands in an area of its own world with ten kinds of virus, two
 * in three; the Aquarium Comp's two kinds of virus take most of its battles
 * (seven in a row were Piranhas and Quakers), one in three from their
 * town's other area; the ACDC HP's three kinds two in three too (from
 * act 3, where its Catacks alone fill the band: three of a playtester's
 * four battles there were the same Catack pair). -1 for none. */
static int shares_with(int biome, int *thirds) {
	switch (biome) {
	case BIOME_SKY_HP: *thirds = 2; return BIOME_SKY;
	case BIOME_GREEN_HP: *thirds = 2; return BIOME_GREEN;
	case BIOME_AQUARIUM_COMP: *thirds = 1; return BIOME_HOMEPAGE;
	case BIOME_ACDC_HP: *thirds = 2; return BIOME_CENTRAL;
	default: return -1;
	}
}

static bool from_formations(int depth, int biome, int kind, Encounter *e);

uint32_t loot_families_here(int depth, int biome) {
	const Formation *list;
	int n = formations_of(biome, &list);
	uint32_t in = 0;
	int p = (depth - 1) % CYCLE_LAYERS, allowed = depth > CYCLE_LAYERS ? 2 : p < 6 ? 0 : p < 12 ? 1 : 2;
	static int8_t fit[MAX_FIT];
	if (n && weigh(list, n, depth, pacing_virus_version(depth, false), pacing_band(depth, false, false), allowed, -1, 9, fit))
		for (int i = 0; i < n && i < MAX_FIT; ++i)
			if (fit[i] >= 0) in |= families_of(&list[i]);
	int thirds, other = shares_with(biome, &thirds);
	if (other >= 0) in |= loot_families_here(depth, other);
	return in;
}

static bool original_encounter(int depth, int biome, int kind, Encounter *e) {
	/* its thirds of the battles from the shared area, where one fits the act */
	int thirds = 0, other = shares_with(biome, &thirds);
	if (other >= 0 && rng_range(0, 2) < thirds && from_formations(depth, other, kind, e)) return true;
	return from_formations(depth, biome, kind, e);
}

static bool from_formations(int depth, int biome, int kind, Encounter *e) {
	const Formation *list;
	int n = formations_of(biome, &list);
	if (!n) return false;
	bool challenge = kind == ENC_CHALLENGE, easy = kind == ENC_EASY || kind == ENC_FIRST || (kind == ENC_NORMAL && (run.helpers & HELP_GENTLE));
	int p = (depth - 1) % CYCLE_LAYERS, allowed = depth > CYCLE_LAYERS ? 2 : p < 6 ? 0 : p < 12 ? 1 : 2;
	int target = pacing_virus_version(depth, challenge);
	bool rare = pacing_rare(depth, rng_range(0, 99));
	/* from the fourth act a challenge may meet one of the area's SP navis,
	 * who keeps his own strength */
	/* (threat 5, docs/META.md: from the first act) */
	if (challenge && (pacing_act(depth) >= 3 || run.threat >= 5) && rng_range(0, 99) < NAVI_CHALLENGE) {
		int total = 0;
		for (int i = 0; i < n; ++i) total += list[i].navi ? list[i].weight : 0;
		if (total) {
			int roll = rng_range(0, total - 1), pick = 0, hp, dmg;
			for (int i = 0; i < n; ++i) {
				if (!list[i].navi) continue;
				if (roll < list[i].weight) { pick = i; break; }
				roll -= list[i].weight;
			}
			build_foes(&list[pick], depth, target, false, e, &hp, &dmg);
			return e->nfoes > 0;
		}
	}
	static int8_t fit[MAX_FIT];
	PacingBand band = pacing_band(depth, challenge, easy);
	int total = 0;
	/* an opening battle sets out two viruses at most, the run's first one,
	 * where the area has such battles */
	static const int8_t tries[3][3] = { { 9, 0, 0 }, { 2, 9, 0 }, { 1, 2, 9 } };
	const int8_t *most = tries[kind == ENC_FIRST ? 2 : easy ? 1 : 0];
	for (int t = 0; t < 3 && most[t] && !total; ++t) {
		PacingBand b = band;
		for (int widen = 0; widen < 4 && !total; ++widen) {
			int allow = widen == 3 ? 3 : allowed;   /* an area of late viruses only */
			/* not the last battle again, unless nothing else fits */
			total = weigh(list, n, depth, target, b, allow, biome == last_biome ? last_pick : -1, most[t], fit);
			if (!total) total = weigh(list, n, depth, target, b, allow, -1, most[t], fit);
			b = pacing_band_wider(b);
		}
	}
	if (!total) return false;
	/* none of the last two battles' virus families where another battle
	 * fits (a Server's pair came back two fights after it, and planes flew
	 * in three of a playtester's five Sky HP fights), then none of the
	 * last one's (six of seven act 1 battles held Gunners; a third as
	 * likely still let two repeats through in five), then a family it
	 * brought a third as likely but never its very viruses (Piranha and
	 * Puffy twice in a row, from two of the area's records), then anything:
	 * the area's own viruses, not the same ones fight after fight */
	int weight[MAX_FIT];
	total = 0;
	for (int again = 0; again < 4 && !total; ++again)
		for (int i = 0; i < n && i < MAX_FIT; ++i) {
			uint32_t fam = families_of(&list[i]);
			bool shares = fam & last_families, before = fam & before_families;
			bool same = biome == last_biome && viruses_of(&list[i]) == last_viruses;
			weight[i] = fit[i] < 0 || (again == 0 && (shares || before)) || (again == 1 && shares) || (again == 2 && same)
				? 0 : list[i].weight * (shares ? 1 : 3);
			total += weight[i];
		}
	/* the run's first battle, the formation's first virus alone: not a
	 * Piranha where another fits, which dives out of the starting folder's
	 * reach and on ice freezes MegaMan (a playtester's first fight, 40 HP) */
	if (kind == ENC_FIRST) {
		int kept = 0;
		for (int i = 0; i < n && i < MAX_FIT; ++i) kept += first_family(&list[i]) == FAMILY_PIRANHA ? 0 : weight[i];
		if (kept) {
			for (int i = 0; i < n && i < MAX_FIT; ++i) if (first_family(&list[i]) == FAMILY_PIRANHA) weight[i] = 0;
			total = kept;
		}
	}
	int roll = rng_range(0, total - 1), pick = -1;
	for (int i = 0; i < n && i < MAX_FIT; ++i) {
		if (!weight[i]) continue;
		if (roll < weight[i]) { pick = i; break; }
		roll -= weight[i];
	}
	if (pick < 0) return false;
	e->from_biome = biome;
	e->from_pick = pick;
	/* (a Mystery Data on the field about one battle in twenty, a run's two
	 * or so: every one the areas' battles hold came in one of seven, to
	 * one of four on Sky HP, each worth a blue Mystery Data) */
	gem_kept = rng_range(0, 99) < GEM_KEEP;
	e->from_families = families_of(&list[pick]);
	e->from_viruses = viruses_of(&list[pick]);
	int hp, dmg;
	build_foes(&list[pick], depth, fit[pick], false, e, &hp, &dmg);
	band = pacing_band(depth, challenge, easy);
	if (rare) {
		/* a rare virus, if the battle still fits with it */
		Encounter r = *e;
		int rhp, rdmg;
		build_foes(&list[pick], depth, fit[pick], true, &r, &rhp, &rdmg);
		if (rhp <= band.hi && rdmg <= band.cap) *e = r;
	}
	return e->nfoes > 0;
}

Encounter make_encounter(int depth, int biome, int kind) {
	bool challenge = kind == ENC_CHALLENGE;
	Encounter e = { 0 };
	e.biome = biome;
	e.from_pick = -1;
	for (int i = 0; i < MAX_FOES; ++i) e.foes[i].id = -1;
	if (original_encounter(depth, biome, kind, &e)) {
		/* the run's first battle: the formation's first virus alone (the
		 * areas' own battles all bring two or more) */
		if (kind == ENC_FIRST) {
			int n = 0;
			bool kept = false;
			for (int i = 0; i < e.nfoes; ++i) {
				if (e.foes[i].kind == FOE_VIRUS && kept) continue;
				kept |= e.foes[i].kind == FOE_VIRUS;
				e.foes[n++] = e.foes[i];
			}
			for (int i = n; i < e.nfoes; ++i) e.foes[i].id = -1;
			e.nfoes = n;
		}
		return e;
	}
	e.nfoes = 0;
	for (int i = 0; i < MAX_FOES; ++i) e.foes[i].id = -1;
	int pool[16], n = 0;
	int p = (depth - 1) % CYCLE_LAYERS + (depth > CYCLE_LAYERS ? 12 : 0);
	for (int i = 0; i < virus_def_count; ++i)
		if ((virus_defs[i].biome_mask >> biome) & 1 && virus_defs[i].first_depth <= p) pool[n++] = virus_defs[i].family;
	if (!n) pool[n++] = 1;
	int count = depth <= 1 ? rng_range(1, 2) : rng_range(2, 3);
	if (challenge) ++count;
	if (count > 4) count = 4;
	bool used[3][3] = { { false } };
	for (int i = 0; i < count; ++i) {
		Foe *f = &e.foes[e.nfoes];
		f->kind = FOE_VIRUS;
		f->family = pool[rng_range(0, n - 1)];
		f->version = pacing_virus_version(depth, challenge);
		for (int tries = 0; tries < 20; ++tries) {
			int c = rng_range(0, 2), r = rng_range(0, 2);
			if (used[r][c]) continue;
			used[r][c] = true;
			f->col = 3 + c;
			f->row = r;
			e.nfoes++;
			break;
		}
	}
	return e;
}

Encounter make_boss(int depth, int biome, int navi) {
	Encounter e = { 0 };
	e.biome = biome;
	e.from_pick = -1;
	e.boss = true;
	e.nfoes = 1;
	e.foes[0].id = -1;
	e.foes[0].kind = FOE_NAVI;
	e.foes[0].family = navi;
	/* the version whose HP suits the act (docs/PROGRESSION.md) */
	e.foes[0].version = pacing_guardian_version(navi, pacing_act(depth), pacing_loop(depth),
		(biome == BIOME_NEST && !run_short_nest(depth)) || biome == BIOME_SECRET, navi_hp);
	/* (threat 4, docs/META.md: EX from act 2) */
	if (run.threat >= 4 && pacing_act(depth) >= 1 && e.foes[0].version < 1) e.foes[0].version = 1;
	e.foes[0].col = 4;
	e.foes[0].row = 1;
	return e;
}

int roll_chip(int depth, int bonus_tier, char *code) {
	int p = (depth - 1) % CYCLE_LAYERS + 3 * ((depth - 1) / CYCLE_LAYERS);
	/* tiers (chip_pool.h): common and uncommon standard chips early, rarer
	 * ones and Megas deeper, a Giga now and then past the middle */
	int w[CHIP_TIERS] = { 60 - p * 3, 30, 6 + p * 2, p > 5 ? p : 0, p > 12 ? (p - 12) / 2 : 0 };
	if (w[0] < 8) w[0] = 8;
	for (int b = 0; b < bonus_tier; ++b) { w[0] /= 2; w[3] += 6; w[2] += 6; w[4] += p > 8 ? 2 : 0; }
	int total = w[0] + w[1] + w[2] + w[3] + w[4];
	int roll = rng_range(0, total - 1), tier = 0;
	while (tier < CHIP_TIERS - 1 && roll >= w[tier]) roll -= w[tier++];
	int id = chip_pool_pick(tier);
	if (id <= 0) {
		/* without a ROM: the engine's own list */
		int pick[128], n = 0;
		if (tier > 3) tier = 3;
		for (int i = 0; i < chip_def_count; ++i)
			if (chip_defs[i].tier == tier && chip_defs[i].kind != CK_NAVI) pick[n++] = chip_defs[i].rom_id;
		if (!n) for (int i = 0; i < chip_def_count; ++i) if (chip_defs[i].tier <= 1) pick[n++] = chip_defs[i].rom_id;
		id = pick[rng_range(0, n - 1)];
	}
	ChipInfo ci;
	chip_info(id, &ci);
	*code = ci.ncodes ? ci.codes[rng_range(0, ci.ncodes - 1)] : '*';
	return id;
}

/* (the folder's codes: run.codes, read as each layer is made) */
char loot_fit_code(int id, char code, bool always) {
	if (code == '*' || !run.codes[0]) return code;
	for (int k = 0; k < 3 && run.codes[k]; ++k)
		if (code == 'A' + run.codes[k] - 1) return code;   /* (one of them already) */
	ChipInfo ci;
	chip_info(id, &ci);
	for (int k = 0; k < 3 && run.codes[k]; ++k) {
		char c = (char)('A' + run.codes[k] - 1);
		if (memchr(ci.codes, c, (size_t)ci.ncodes)) return always || rng_range(0, 1) ? c : code;
	}
	return code;
}

char loot_folder_code(int id, bool star) {
	ChipInfo ci;
	chip_info(id, &ci);
	for (int k = 0; k < 3 && run.codes[k]; ++k) {
		char c = (char)('A' + run.codes[k] - 1);
		if (memchr(ci.codes, c, (size_t)ci.ncodes)) return c;
	}
	return star && memchr(ci.codes, '*', (size_t)ci.ncodes) ? '*' : 0;
}

void loot_folder_codes(const uint16_t *folder, int n, uint8_t out[3]) {
	int count[26] = { 0 };
	for (int i = 0; i < n; ++i) {
		int c = folder[i] >> 9;
		if (c < 26) ++count[c];
	}
	/* (a code three chips hold: one a hand can be built on) */
	for (int k = 0; k < 3; ++k) {
		int best = -1;
		for (int c = 0; c < 26; ++c)
			if (count[c] >= 3 && (best < 0 || count[c] > count[best])) best = c;
		out[k] = (uint8_t)(best < 0 ? 0 : best + 1);
		if (best >= 0) count[best] = 0;
	}
}

int chip_price(int id) {
	/* zenny by tier, dearer on later cycles */
	static const int by_tier[CHIP_TIERS] = { 5, 10, 20, 40, 80 };
	int t = chip_pool_tier(id), loop = (run.depth - 1) / CYCLE_LAYERS;
	int base = t >= 0 ? by_tier[t] : chip_def(id)->price;
	return base * 100 * (2 + loop) / 2;
}
