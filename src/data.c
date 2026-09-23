#include "data.h"

#include <stdio.h>
#include <string.h>

#include "rom.h"

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

/* Three chips picked in order become one Program Advance. mode 0: the same
 * chip with consecutive codes (A, B, C); mode 1: the listed chips sharing a
 * code (wildcards allowed). */
const ProgramAdvance program_advances[] = {
	{ 320, 0, { 1, 1, 1 } },
	{ 321, 0, { 2, 2, 2 } },
	{ 322, 0, { 3, 3, 3 } },
	{ 339, 1, { 71, 72, 73 } },
	{ 323, 1, { 20, 21, 22 } },
	{ 338, 1, { 9, 10, 11 } },
	{ 329, 1, { 40, 41, 42 } },
};
const int program_advance_count = sizeof program_advances / sizeof *program_advances;

const ChipDef *chip_def(int rom_id) {
	for (int i = 0; i < chip_def_count; ++i)
		if (chip_defs[i].rom_id == rom_id) return &chip_defs[i];
	return &chip_defs[0];
}

void chip_info(int rom_id, ChipInfo *out) {
	memset(out, 0, sizeof *out);
	uint32_t rec = R.layout->chip_data + rom_id * 0x2C;
	if (rom_id < 256) rom_text(R.layout->chip_names[0], rom_id, out->name, sizeof out->name);
	else rom_text(R.layout->chip_names[1], rom_id - 256, out->name, sizeof out->name);
	out->power = rom_u16(rec + 0x1A);
	out->element = R.data[rec + 4];
	out->chip_element = R.data[rec + 6];
	for (int i = 0; i < 4; ++i) {
		uint8_t c = R.data[rec + i];
		if (c == 0xFF) continue;
		out->codes[out->ncodes++] = c == 26 ? '*' : (char)('A' + c);
	}
	out->codes[out->ncodes] = 0;
	const ChipDef *d = chip_def(rom_id);
	if (d->kind == CK_NAVI) out->power = out->power > 1000 ? out->power % 1000 * 10 : out->power;
	if (d->kind == CK_RECOVER) out->power = d->param;
}

void chip_desc(int rom_id, char *out, int len) {
	out[0] = 0;
	if (rom_id < 256 && R.layout->chip_desc[0]) rom_script_text(R.layout->chip_desc[0], rom_id, out, (size_t)len);
	else if (rom_id >= 256 && R.layout->chip_desc[1]) rom_script_text(R.layout->chip_desc[1], rom_id - 256, out, (size_t)len);
	if (!out[0] && chip_def(rom_id)->kind == CK_NAVI) {
		ChipInfo ci;
		chip_info(rom_id, &ci);
		snprintf(out, (size_t)len, "Summons\n%s\nto attack!", ci.name);
	}
}

/* ai index, behaviour, idle/move/attack/hit anims, effect sprite, biomes, first depth.
 * Biomes: bit 0 Central, 1 Seaside, 2 Sky, 3 Green, 4 Graveyard, 5 Undernet, 6 Secret, 7 Nest. */
const VirusDef virus_defs[] = {
	{ 1, AI_METTAUR, 0, 4, 1, -1, -1, -1, -1, 0x2B, 0 },   /* Mettaur: wave drawn procedurally */
	{ 2, AI_SHOOTER, 0, 1, 3, -1, 1, 2, 7, 0x62, 0 },      /* Piranha: torpedo */
	{ 4, AI_SWORDY, 0, -1, 3, -1, 1, 4, 10, 0xF5, 1 },     /* Swordy */
	{ 5, AI_BEAM, 0, -1, 1, -1, -1, -1, -1, 0xB6, 2 },     /* KillerEye */
	{ 8, AI_PUNCHER, 0, -1, 2, -1, -1, -1, -1, 0xA9, 2 },  /* Champy */
	{ 14, AI_SHOOTER, 0, 1, 3, -1, 3, 0x20, 0, 0x26, 1 },  /* Puffy: bubble */
	{ 20, AI_LOBBER, 0, -1, 4, -1, 3, 0x24, 0, 0x78, 1 },  /* BombCorn */
	{ 23, AI_GUNNER, 0, -1, 2, -1, 1, 2, 8, 0xF5, 2 },     /* Gunner */
	{ 27, AI_ROLLER, 0, 1, 3, -1, -1, -1, -1, 0xF8, 3 },   /* Armadill */
};
const int virus_def_count = sizeof virus_defs / sizeof *virus_defs;

/* navi, idle/move/hit, three attacks (anim, kind, effect sprite), reward chip, cross name */
const NaviDef navi_defs[] = {
	{ 1, 0, 4, 1, { 8, 5, 14 }, { NA_ROWBLAST, NA_TARGET, NA_SHOT }, { 4, 3, 3 }, { 0x02, 0x24, 0x0E }, { 0, 0, 0 }, 227, "Heat" },
	{ 2, 0, 4, 1, { 18, 14, 19 }, { NA_COLUMN, NA_SHOT, NA_TARGET }, { 4, 3, 4 }, { 0x32, 0x13, 0x32 }, { 0, 0, 0 }, 230, "Elec" },
	{ 3, 0, 4, 1, { 17, 6, 19 }, { NA_DASH, NA_COLUMN, NA_DASH }, { -1, 3, -1 }, { -1, 0x14, -1 }, { -1, 0, -1 }, 233, "Slash" },
	{ 5, 0, 4, 1, { 6, 5, 8 }, { NA_DASH, NA_TARGET, NA_SHOT }, { -1, 3, 3 }, { -1, 0x24, 0x0E }, { -1, 0, 0 }, 239, "Charge" },
	{ 7, 0, 4, 1, { 6, 5, 8 }, { NA_WAVE, NA_TARGET, NA_DASH }, { -1, 3, -1 }, { -1, 0x24, -1 }, { -1, 0, -1 }, 245, NULL },
	{ 12, 0, 4, 1, { 5, 7, 8 }, { NA_TARGET, NA_DASH, NA_SHOT }, { 2, -1, 2 }, { 12, -1, 12 }, { 18, -1, 17 }, 257, NULL },
	{ 13, 0, 4, 1, { 8, 5, 14 }, { NA_WAVE, NA_TARGET, NA_SHOT }, { -1, 3, 3 }, { -1, 0x20, 0x20 }, { -1, 0, 0 }, 260, NULL },
	{ 15, 0, 4, 1, { 5, 8, 14 }, { NA_TARGET, NA_ROWBLAST, NA_SHOT }, { 5, 5, 3 }, { 0x14, 0x14, 0x13 }, { 0, 0, 0 }, 266, NULL },
	{ 16, 0, 4, 1, { 5, 8, 14 }, { NA_TARGET, NA_COLUMN, NA_SHOT }, { 3, 4, 3 }, { 0x24, 0x09, 0x0E }, { 0, 0, 0 }, 269, NULL },
	{ 11, 0, 4, 1, { 6, 6, 9 }, { NA_DASH, NA_COLUMN, NA_SHOT }, { -1, 3, 3 }, { -1, 0x15, 0x02 }, { -1, 0, 0 }, 224, NULL },
	{ 4, 0, 4, 1, { 6, 5, 14 }, { NA_COLUMN, NA_TARGET, NA_SHOT }, { 3, 3, 3 }, { 0x14, 0x24, 0x13 }, { 0, 0, 0 }, 236, "Erase" },
	{ 9, 0, 4, 1, { 8, 5, 6 }, { NA_WAVE, NA_TARGET, NA_DASH }, { -1, 3, -1 }, { -1, 0x24, -1 }, { -1, 0, -1 }, 251, NULL },
	{ 10, 0, 4, 1, { 8, 5, 14 }, { NA_ROWBLAST, NA_TARGET, NA_SHOT }, { 4, 3, 3 }, { 0x2E, 0x24, 0x0E }, { 0, 0, 0 }, 254, NULL },
};
const int navi_def_count = sizeof navi_defs / sizeof *navi_defs;

const NaviDef *navi_def(int ai_index) {
	for (int i = 0; i < navi_def_count; ++i)
		if (navi_defs[i].ai_index == ai_index) return &navi_defs[i];
	return &navi_defs[0];
}

/* ---- ROM enemy table ---- */
int enemy_id(int actor_type, int family, int version) {
	for (int id = 0; id < 0x200; ++id) {
		const uint8_t *e = R.data + R.layout->enemy_ids + id * 3;
		if (e[1] > 2) break;
		if (e[0] == version && e[1] == actor_type && e[2] == family) return id;
	}
	return -1;
}

static const uint8_t *enemy_stats(int id) {
	const uint8_t *e = R.data + R.layout->enemy_ids + id * 3;
	uint32_t by_type = rom_off(rom_u32(R.layout->enemy_stats + e[1] * 4));
	uint32_t by_ai = rom_off(rom_u32(by_type + e[2] * 4));
	return R.data + by_ai + e[0] * 6;
}

int enemy_hp(int id) {
	const uint8_t *s = enemy_stats(id);
	return (s[0] | s[1] << 8) & 0xFFF;
}

int enemy_element(int id) {
	const uint8_t *s = enemy_stats(id);
	return ((s[0] | s[1] << 8) >> 12) & 7;
}

int enemy_attack(int id) {
	const uint8_t *s = enemy_stats(id);
	return (s[4] | s[5] << 8) & 0xFFF;
}

void enemy_name(int id, char *out, int len) {
	if (id >= 0 && id < 235) rom_text(R.layout->enemy_names, id, out, (size_t)len);
	else snprintf(out, (size_t)len, "???");
}

void navi_name(int navi, char *out, int len) {
	rom_text(R.layout->navi_names, navi, out, (size_t)len);
}
