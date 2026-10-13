#include "data.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "chips.h"
#include "rom.h"
#include "run.h"

/* rom id, behaviour, parameter, tier (0 common - 4 legendary), price (x100 z) */
const ChipDef chip_defs[] = {
	{ 1, CK_CANNON, 0, 0, 5 },      /* Cannon */
	{ 2, CK_CANNON, 0, 1, 12 },     /* HiCannon */
	{ 3, CK_CANNON, 0, 2, 25 },     /* M-Cannon */
	{ 4, CK_AIRSHOT, 0, 0, 4 },     /* AirShot */
	{ 5, CK_VULCAN, 3, 0, 5 },      /* Vulcan1 */
	{ 6, CK_VULCAN, 4, 1, 12 },     /* Vulcan2 */
	{ 7, CK_VULCAN, 6, 2, 22 },     /* Vulcan3 */
	{ 8, CK_VULCAN, 10, 3, 45 },    /* SuprVulc */
	{ 9, CK_SPREADER, 0, 0, 6 },    /* Spreadr1 */
	{ 10, CK_SPREADER, 0, 1, 14 },  /* Spreadr2 */
	{ 11, CK_SPREADER, 0, 2, 26 },  /* Spreadr3 */
	{ 20, CK_FLAME, 0, 0, 7 },      /* FireBrn1 */
	{ 21, CK_FLAME, 0, 1, 15 },     /* FireBrn2 */
	{ 22, CK_FLAME, 0, 2, 26 },     /* FireBrn3 */
	{ 30, CK_THUNDER, 0, 1, 10 },   /* Thunder */
	{ 34, CK_LONGSWORD, 0, 0, 8 },  /* ElcPuls1 */
	{ 35, CK_LONGSWORD, 0, 1, 16 }, /* ElcPuls2 */
	{ 36, CK_LONGSWORD, 0, 2, 26 }, /* ElcPuls3 */
	{ 40, CK_WAVE, 0, 0, 7 },       /* RlngLog1 */
	{ 41, CK_WAVE, 0, 1, 15 },      /* RlngLog2 */
	{ 42, CK_WAVE, 0, 2, 25 },      /* RlngLog3 */
	{ 52, CK_TORNADO, 8, 2, 24 },   /* Tornado */
	{ 54, CK_BOMB, 0, 0, 5 },       /* MiniBomb */
	{ 55, CK_BOMB, 2, 1, 12 },      /* EnergBom */
	{ 56, CK_BOMB, 2, 2, 22 },      /* MegEnBom */
	{ 60, CK_BOMB, 0, 3, 40 },      /* BlkBomb */
	{ 202, CK_BOMB, 1, 3, 38 },     /* BigBomb */
	{ 71, CK_SWORD, 0, 0, 6 },      /* Sword */
	{ 72, CK_WIDESWORD, 0, 1, 12 }, /* WideSwrd */
	{ 73, CK_LONGSWORD, 0, 1, 14 }, /* LongSwrd */
	{ 74, CK_WIDESWORD, 0, 3, 36 }, /* WideBlde */
	{ 75, CK_LONGSWORD, 0, 3, 36 }, /* LongBlde */
	{ 76, CK_WIDESWORD, 0, 1, 18 }, /* FireSwrd */
	{ 77, CK_WIDESWORD, 0, 2, 24 }, /* AquaSwrd */
	{ 78, CK_WIDESWORD, 0, 2, 22 }, /* ElecSwrd */
	{ 79, CK_WIDESWORD, 0, 2, 24 }, /* BambSwrd */
	{ 116, CK_BOOMER, 0, 1, 16 },   /* Boomer */
	{ 117, CK_BOOMER, 0, 2, 26 },   /* HiBoomer */
	{ 118, CK_BOOMER, 0, 3, 38 },   /* M-Boomer */
	{ 142, CK_CROSSGUN, 0, 2, 28 }, /* CircGun */
	{ 139, CK_METEORS, 12, 4, 80 }, /* Meteors */
	{ 143, CK_ROCKCUBE, 0, 0, 4 },  /* RockCube */
	{ 154, CK_RECOVER, 10, 0, 2 },  /* Recov10 */
	{ 155, CK_RECOVER, 30, 0, 5 },  /* Recov30 */
	{ 156, CK_RECOVER, 50, 0, 8 },  /* Recov50 */
	{ 157, CK_RECOVER, 80, 1, 12 }, /* Recov80 */
	{ 158, CK_RECOVER, 120, 1, 18 },/* Recov120 */
	{ 159, CK_RECOVER, 150, 2, 24 },/* Recov150 */
	{ 160, CK_RECOVER, 200, 2, 32 },/* Recov200 */
	{ 161, CK_RECOVER, 250, 3, 45 },/* Recov300 (capped for the run's balance) */
	{ 162, CK_PANELGRAB, 0, 0, 6 }, /* PanlGrab */
	{ 163, CK_AREAGRAB, 0, 1, 14 }, /* AreaGrab */
	{ 167, CK_GEDDON, 0, 2, 20 },   /* Geddon */
	{ 168, CK_HOLYPANEL, 0, 1, 14 },/* HolyPanl */
	{ 177, CK_INVIS, 240, 2, 22 },  /* Invisibl */
	{ 178, CK_BARRIER, 10, 1, 10 }, /* Barrier */
	{ 179, CK_BARRIER, 100, 2, 24 },/* Barr100 */
	{ 180, CK_BARRIER, 200, 3, 40 },/* Barr200 */
	{ 192, CK_ATKPLUS, 10, 1, 10 }, /* Atk+10 */
	{ 195, CK_ATKPLUS, 30, 3, 35 }, /* Atk+30 */
	/* Navi chips: earned from bosses, never sold */
	{ 224, CK_NAVI, 11, 4, 0 },     /* ProtoMan */
	{ 227, CK_NAVI, 1, 4, 0 },      /* HeatMan */
	{ 230, CK_NAVI, 2, 4, 0 },      /* ElecMan */
	{ 233, CK_NAVI, 3, 4, 0 },      /* SlashMan */
	{ 236, CK_NAVI, 4, 4, 0 },      /* EraseMan */
	{ 239, CK_NAVI, 5, 4, 0 },      /* ChrgeMan */
	{ 242, CK_NAVI, 6, 4, 0 },      /* SpoutMan */
	{ 245, CK_NAVI, 7, 4, 0 },      /* TmhkMan */
	{ 248, CK_NAVI, 8, 4, 0 },      /* TenguMan */
	{ 251, CK_NAVI, 9, 4, 0 },      /* GrndMan */
	{ 254, CK_NAVI, 10, 4, 0 },     /* DustMan */
	{ 257, CK_NAVI, 12, 4, 0 },     /* BlastMan */
	{ 260, CK_NAVI, 13, 4, 0 },     /* DiveMan */
	{ 263, CK_NAVI, 14, 4, 0 },     /* CrcusMan */
	{ 266, CK_NAVI, 15, 4, 0 },     /* JudgeMan */
	{ 269, CK_NAVI, 16, 4, 0 },     /* ElmntMan */
	{ 272, CK_NAVI, 18, 4, 0 },     /* Colonel */
	/* Program Advances: formed in the Custom screen, never dropped (tier 5) */
	{ 320, CK_CANNON, 0, 5, 0 },    /* GigaCan1 */
	{ 321, CK_CANNON, 0, 5, 0 },    /* GigaCan2 */
	{ 322, CK_CANNON, 0, 5, 0 },    /* GigaCan3 */
	{ 323, CK_FLAME, 3, 5, 0 },     /* WideBrn1: three rows */
	{ 329, CK_WAVE, 1, 5, 0 },      /* PwrWave1 */
	{ 338, CK_SPREADER, 5, 5, 0 },  /* H-Burst: five blasts */
	{ 339, CK_WIDESWORD, 2, 5, 0 }, /* LifeSrd: two columns */
};
const int chip_def_count = sizeof chip_defs / sizeof *chip_defs;

const ChipDef *chip_def(int rom_id) {
	for (int i = 0; i < chip_def_count; ++i)
		if (chip_defs[i].rom_id == rom_id) return &chip_defs[i];
	return &chip_defs[0];
}

bool chip_direct(int rom_id) {
	const uint8_t *c = chips_record(rom_id);
	if (!c) return false;
	if (c[BN6_CHIP_LOCK_ON] == 1) return true;
	/* (and the families that strike outright without the lock-on: the Navi
	 * chips, MachGun, AquaNdl, WaveArm, AirSpin, the Dragons) */
	switch (c[BN6_CHIP_FAMILY]) {
	case 27: case 41: case 49: case 50: case 56: case 81: return true;
	default: return false;
	}
}

int chip_family(int rom_id) {
	const uint8_t *c = chips_record(rom_id);
	return c ? c[BN6_CHIP_FAMILY] : 0;
}

bool chip_melee(int rom_id) {
	int family = chip_family(rom_id);
	return family == 19 || family == 63 || family == 64;
}

bool chip_standard(int rom_id) {
	const uint8_t *c = chips_record(rom_id);
	return c && c[BN6_CHIP_LIBRARY_TYPE] == 0;
}

/* Whether chip `rom_id` is a Navi's, in any of its three versions. */
static bool navi_chip_of(int rom_id) {
	for (int i = 0; i < chip_def_count; ++i)
		if (chip_defs[i].kind == CK_NAVI && rom_id >= chip_defs[i].rom_id && rom_id < chip_defs[i].rom_id + 3) return true;
	return false;
}

void chip_info(int rom_id, ChipInfo *out) {
	memset(out, 0, sizeof *out);
	const uint8_t *rec = chips_record(rom_id);
	if (!rec) return;
	chips_name(rom_id, out->name, sizeof out->name);
	out->power = get16(rec + BN6_CHIP_ATTACK_POWER);
	out->mb = rec[BN6_CHIP_MB];
	out->element = rec[BN6_CHIP_ATTACK_ELEMENT];
	out->chip_element = rec[BN6_CHIP_ELEMENT];
	for (int i = 0; i < 4; ++i) {
		uint8_t c = rec[BN6_CHIP_CODES + i];
		if (c == 0xFF) continue;
		out->codes[out->ncodes++] = c == 26 ? '*' : (char)('A' + c);
	}
	/* (the All * helper: in * alone, as the game's record then has it, so
	 * every reward, stock and prize the engine rolls comes in *) */
	if (run_all_star() && out->ncodes) {
		out->codes[0] = '*';
		out->ncodes = 1;
	}
	out->codes[out->ncodes] = 0;
	const ChipDef *d = chip_def(rom_id);
	/* (a Navi chip's power is 1000 and a tenth of it, in its V1, EX and SP
	 * alike: a playtester's vault offered "DiveManSP D 1013") */
	if (navi_chip_of(rom_id) && out->power > 1000) out->power = out->power % 1000 * 10;
	if (d->kind == CK_RECOVER) out->power = d->param;
}

void chip_desc(int rom_id, char *out, size_t outlen) { chips_desc(rom_id, out, outlen); }

/* ---- enemy ids another game's family took (xvirus.c) ---- */
static struct {
	int n;
	struct { int id; EnemyMoved e; } at[ENEMIES_MOVED_MAX];
	int16_t slot[ENEMY_IDS_MAX];   /* each id's place in at[] plus one, 0 none (the readers below are asked often) */
} moved;

static const EnemyMoved *moved_of(int id) {
	return id >= 0 && id < ENEMY_IDS_MAX && moved.slot[id] ? &moved.at[moved.slot[id] - 1].e : NULL;
}

bool enemy_moved_set(int id, const EnemyMoved *e) {
	if (id < 0 || id >= ENEMY_IDS_MAX) return false;
	int i = moved.slot[id] - 1;
	if (i >= 0) {
		if (e) moved.at[i].e = *e;
		else {
			/* (the last one into its place) */
			moved.slot[id] = 0;
			if (i != --moved.n) {
				moved.at[i] = moved.at[moved.n];
				moved.slot[moved.at[i].id] = (int16_t)(i + 1);
			}
		}
		return true;
	}
	if (!e) return true;
	if (moved.n >= ENEMIES_MOVED_MAX) return false;
	moved.at[moved.n].id = id;
	moved.at[moved.n].e = *e;
	moved.slot[id] = (int16_t)++moved.n;
	return true;
}

uint16_t enemy_drop(int id, int k) {
	const EnemyMoved *m = moved_of(id);
	if (k < 0 || k >= ENEMY_DROPS) return 0xFFFF;
	if (m && m->drops_own) return m->drops[k];
	return R.data && id > 0 && id < BN6_DROP_ROW_COUNT ? rom_u16(rom_off(BN6_DROP_ROWS) + (uint32_t)id * 2u * ENEMY_DROPS + 2u * (uint32_t)k) : 0xFFFF;
}

/* (BN6's names: the viruses' to 0xFF, the Navis' past it by the id's low
 * byte; a moved id's its own, from 0x200 those of the families alone) */
void enemy_name(int name_id, char *out, size_t outlen) {
	const EnemyMoved *m = moved_of(name_id);
	if (m) snprintf(out, outlen, "%s", m->name);
	else if (name_id >= 0 && name_id < 0x200) rom_text(R.layout->enemy_names[name_id > 0xFF], name_id & 0xFF, out, outlen);
	else if (outlen) *out = 0;
}

void item_name(int id, char *out, size_t outlen) {
	if (R.layout->item_names) rom_text(R.layout->item_names, id, out, outlen);
	else if (outlen) *out = 0;
}

void program_name(int program, char *out, size_t outlen) {
	if (R.layout->navicust_names) rom_text(R.layout->navicust_names, program, out, outlen);
	else if (outlen) *out = 0;
}

uint16_t chip_entry_star(uint16_t entry) {
	return entry == 0xFFFF || !(entry & 0x1FF) ? entry : (uint16_t)((entry & 0x1FF) | CHIP_CODE_STAR << 9);
}

void chip_pack_star(uint8_t e[CHIP_PACK_ENTRY]) {
	int n = e[0];
	for (int k = 1; k < 4; ++k) {
		/* (its order stamp, where the first code had none: the stamps
		 * after the counts, BN6's pack sorts by them) */
		if (e[k] && !n) { e[4] = e[4 + 2 * k]; e[5] = e[5 + 2 * k]; }
		n += e[k];
		e[k] = 0;
		e[4 + 2 * k] = e[5 + 2 * k] = 0;
	}
	e[0] = (uint8_t)(n > 99 ? 99 : n);
}

/* family, biomes, first depth. Biomes: bit 0 Central, 1 Seaside, 2 Sky,
 * 3 Green, 4 Graveyard, 5 Undernet, 6 Secret, 7 Nest. */
const VirusDef virus_defs[] = {
	{ 1, 0x2B, 0 },    /* Mettaur */
	{ 2, 0x62, 0 },    /* Piranha */
	{ 4, 0xF5, 1 },    /* Swordy */
	{ 5, 0xB6, 2 },    /* KillerEye */
	{ 8, 0xA9, 2 },    /* Champy */
	{ 14, 0x26, 1 },   /* Puffy */
	{ 20, 0x78, 1 },   /* BombCorn */
	{ 23, 0xF5, 2 },   /* Gunner */
	{ 27, 0xF8, 3 },   /* Armadill */
};
const int virus_def_count = sizeof virus_defs / sizeof *virus_defs;

/* ---- ROM enemy table ---- */
const uint8_t *enemy_triple(int id) {
	const EnemyMoved *m = moved_of(id);
	if (m) return m->triple;
	return R.data && id >= 0 && id < BN6_ENEMY_IDS_COUNT ? R.data + R.layout->enemy_ids + id * 3 : NULL;
}

int enemy_id(int actor_type, int family, int version) {
	for (int i = 0; i < moved.n; ++i) {
		const uint8_t *e = moved.at[i].e.triple;
		if (e[0] == version && e[1] == actor_type && e[2] == family) return moved.at[i].id;
	}
	for (int id = 0; id < BN6_ENEMY_IDS_COUNT; ++id) {
		const uint8_t *e = enemy_triple(id);
		if (!e || e[1] > 2) break;
		if (e[0] == version && e[1] == actor_type && e[2] == family) return id;
	}
	return -1;
}

int navi_chip(int navi, int version) {
	/* each navi's chips come in threes: V1, EX, SP */
	for (int i = 0; i < chip_def_count; ++i)
		if (chip_defs[i].kind == CK_NAVI && chip_defs[i].param == navi) return chip_defs[i].rom_id + (version < 0 ? 0 : version > 2 ? 2 : version);
	return 0;
}

/* The record of enemy `id`'s stats, NULL when not found: a table per actor
 * type, a pointer per ai, a record per version of u16 element << 12 | HP,
 * version, flags, u16 element << 12 | damage. */
static const uint8_t *enemy_record(int id) {
	const EnemyMoved *m = moved_of(id);
	if (m) return m->stats;
	if (!R.data || id < 0 || id >= BN6_ENEMY_IDS_COUNT || !R.layout->enemy_stats) return NULL;
	const uint8_t *e = R.data + R.layout->enemy_ids + id * 3;
	if (e[1] > 2) return NULL;
	uint32_t types = rom_u32(R.layout->enemy_stats + (uint32_t)e[1] * 4);
	if (!rom_is_ptr(types)) return NULL;
	uint32_t recs = rom_u32(rom_off(types) + (uint32_t)e[2] * 4);
	if (!rom_is_ptr(recs)) return NULL;
	uint32_t r = rom_off(recs) + (uint32_t)e[0] * 6;
	return r + 6 > ROM_SIZE ? NULL : R.data + r;
}

static int u16_at(const uint8_t *p) { return p[0] | p[1] << 8; }

const char *elem_name(int elem) {
	static const char *const names[ELEM_COUNT] = { "", "Fire", "Aqua", "Elec", "Wood", "Sword", "Wind", "Cursor", "Breaker" };
	return elem > 0 && elem < ELEM_COUNT ? names[elem] : "";
}

/* The record of enemy `id`'s traits, 0 when not found (bn6f
 * enemy_getStruct1): a table per actor type, a pointer per ai, 8 bytes:
 * its sprites, then at 5 its element of the second wheel and at 6 the
 * ones that hit it twice as hard, a bit each (0x80 Sword, 0x40 Cursor,
 * 0x20 Wind, 0x10 Breaker) */
static const uint8_t *enemy_traits(int id) {
	const EnemyMoved *m = moved_of(id);
	if (m) return m->traits;
	if (!R.data || id < 0 || id >= BN6_ENEMY_IDS_COUNT || !R.layout->enemy_traits) return NULL;
	const uint8_t *e = R.data + R.layout->enemy_ids + id * 3;
	if (e[1] > 2) return NULL;
	uint32_t types = rom_u32(R.layout->enemy_traits + (uint32_t)e[1] * 4);
	if (!rom_is_ptr(types)) return NULL;
	uint32_t rec = rom_u32(rom_off(types) + (uint32_t)e[2] * 4);
	if (!rom_is_ptr(rec)) return NULL;
	return rom_off(rec) + 8 > ROM_SIZE ? NULL : R.data + rom_off(rec);
}

static int second_wheel(uint8_t bits) {
	return bits & 0x80 ? ELEM_SWORD : bits & 0x40 ? ELEM_CURSOR : bits & 0x20 ? ELEM_WIND : bits & 0x10 ? ELEM_BREAK : 0;
}

int enemy_element(int id) {
	const uint8_t *r = enemy_record(id), *t = enemy_traits(id);
	if (!r) return -1;
	int e = u16_at(r) >> 12;
	return e > 0 && e <= ELEM_WOOD ? e : t ? second_wheel(t[5]) : 0;
}

int enemy_weakness(int id) {
	static const int beats[ELEM_WOOD + 1] = { 0, ELEM_AQUA, ELEM_ELEC, ELEM_WOOD, ELEM_FIRE };
	const uint8_t *r = enemy_record(id), *t = enemy_traits(id);
	if (!r) return -1;
	int e = u16_at(r) >> 12;
	return e > 0 && e <= ELEM_WOOD ? beats[e] : t ? second_wheel(t[6]) : 0;
}

int chip_hits_with(int rom_id) {
	ChipInfo ci;
	chip_info(rom_id, &ci);
	if (ci.element > 0 && ci.element <= ELEM_WOOD) return ci.element;
	/* (the icon's: 5 Sword, 6 Cursor, 8 Wind, 9 Break; bn6f CHIP_ELEM_*) */
	switch (ci.chip_element) {
	case 5: return ELEM_SWORD;
	case 6: return ELEM_CURSOR;
	case 8: return ELEM_WIND;
	case 9: return ELEM_BREAK;
	default: return 0;
	}
}

bool enemy_stats(int id, int *hp, int *damage) {
	const uint8_t *r = enemy_record(id);
	if (!r) return false;
	*hp = u16_at(r) & 0xFFF;
	*damage = u16_at(r + 4) & 0xFFF;
	return true;
}

