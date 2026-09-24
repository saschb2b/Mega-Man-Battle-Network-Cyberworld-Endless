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
int enemy_id(int actor_type, int family, int version) {
	for (int id = 0; id < 0x200; ++id) {
		const uint8_t *e = R.data + R.layout->enemy_ids + id * 3;
		if (e[1] > 2) break;
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

bool enemy_stats(int id, int *hp, int *damage) {
	if (!R.data || id < 0 || id >= 0x200 || !R.layout->enemy_stats) return false;
	const uint8_t *e = R.data + R.layout->enemy_ids + id * 3;
	if (e[1] > 2) return false;
	/* a table per actor type, a pointer per ai, a record per version:
	 * u16 element << 12 | HP, version, flags, u16 element << 12 | damage */
	uint32_t types = rom_u32(R.layout->enemy_stats + (uint32_t)e[1] * 4);
	if (!rom_is_ptr(types)) return false;
	uint32_t recs = rom_u32(rom_off(types) + (uint32_t)e[2] * 4);
	if (!rom_is_ptr(recs)) return false;
	uint32_t r = rom_off(recs) + (uint32_t)e[0] * 6;
	if (r + 6 > ROM_SIZE) return false;
	*hp = rom_u16(r) & 0xFFF;
	*damage = rom_u16(r + 4) & 0xFFF;
	return true;
}

