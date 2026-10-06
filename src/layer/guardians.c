#include "guardians.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "net.h"
#include "rivals.h"
#include "rom.h"
#include "run.h"
#include "text.h"

/* Mugshots share their index with the Navi's overworld sprite (list 6);
 * Gregar has none for Falzar's Navis. Pose 24 is the signature move the
 * Gregar Navis' overworld sprites carry. */
static const Guardian guardians[] = {
	[1] = { "HeatMan", "Flame of the Expo", 0x47, 24, 248, 120, 40 },
	[2] = { "ElecMan", "Master of Current", 0x49, 24, 248, 224, 64 },
	[3] = { "SlashMan", "The Swift Claw", 0x4B, 24, 120, 200, 96 },
	[4] = { "EraseMan", "The Deleter", 0x50, 24, 176, 96, 224 },
	[5] = { "ChargeMan", "Engine of Ruin", 0x4F, 24, 232, 72, 56 },
	[6] = { "SpoutMan", "Keeper of Tides", GUARDIAN_NO_MUGSHOT, -1, 96, 176, 248 },
	[7] = { "TomahawkMan", "Warrior of Green", GUARDIAN_NO_MUGSHOT, -1, 216, 168, 72 },
	[8] = { "TenguMan", "Lord of the Gale", GUARDIAN_NO_MUGSHOT, -1, 224, 88, 88 },
	[9] = { "GroundMan", "Driller of Depths", GUARDIAN_NO_MUGSHOT, -1, 232, 176, 48 },
	[10] = { "DustMan", "The Scrap Heap", GUARDIAN_NO_MUGSHOT, -1, 168, 168, 136 },
	[11] = { "ProtoMan", "Blade of Justice", 0x3B, -1, 232, 56, 72 },
	[12] = { "BlastMan", "The Living Blast", 0x51, -1, 248, 136, 48 },
	[13] = { "DiveMan", "Terror of the Deep", 0x52, -1, 72, 136, 232 },
	[14] = { "CircusMan", "Ringmaster of Fear", 0x54, -1, 232, 64, 96 },
	[15] = { "JudgeMan", "Voice of Verdict", 0x55, -1, 88, 104, 216 },
	[16] = { "ElementMan", "Lord of Elements", 0x56, -1, 176, 136, 232 },
	[18] = { "Colonel", "The Iron Strategist", 0x53, -1, 120, 168, 136 },
	/* the super bosses (docs/BOSSES.md, Super bosses): Bass, his own face
	 * and cloaked sprite, and the Cybeast, its face and its beast on the
	 * map; their poses super_body's */
	[19] = { "Bass", "The Strongest Navi", 0x5B, -1, 232, 176, 40 },
	[20] = { "Gregar", "It copies everything", 0x58, -1, 248, 128, 40 },
	/* BN5's (docs/BOSSES.md, BN5's Navis): their faces and sprites copied in
	 * from its ROM as they are met (guardian_set_face), their pose its own
	 * animation 24, as Gregar's Navis' */
	[24] = { "Colonel", "Commander of the Old Net", GUARDIAN_NO_MUGSHOT, 24, 96, 184, 176 },
	[25] = { "ShadowMan", "Blade of the Shadows", GUARDIAN_NO_MUGSHOT, 24, 160, 96, 224 },
	[26] = { "NumberMan", "Master of the Odds", GUARDIAN_NO_MUGSHOT, 24, 176, 216, 80 },
	[27] = { "TomahawkMan", "Spirit of the Totem", GUARDIAN_NO_MUGSHOT, 24, 232, 88, 72 },
	[28] = { "KnightMan", "The Iron Fortress", GUARDIAN_NO_MUGSHOT, 24, 152, 136, 200 },
	[29] = { "ToadMan", "Maestro of the Marsh", GUARDIAN_NO_MUGSHOT, 24, 104, 200, 104 },
};
#define NGUARDIANS ((int)(sizeof guardians / sizeof *guardians))

bool guardian_known(int navi) {
	const Rival *rv = rival(navi);
	return rv->megaman_won + rv->navi_won > 0;
}

const Guardian *guardian(int navi) {
	static const Guardian unknown = { "???", "Guardian", GUARDIAN_NO_MUGSHOT, -1, 200, 200, 200 };
	return navi > 0 && navi < NGUARDIANS && guardians[navi].name ? &guardians[navi] : &unknown;
}

/* Falzar's Navis, whose faces Gregar lacks, speak with the portrait set
 * for them where the layer made one, else without a face: the HeelNavi's
 * is the bystanders' (a playtester read SpoutMan's lines as a
 * bystander's); BN5's with their own, copied in (xnavi_guardian) */
static int set_face[NGUARDIANS];   /* the face + 1, 0 for none */

int guardian_sprite(int navi) {
	static const struct { uint8_t navi, sprite; } sprites[] = {
		{ 1, 0x47 }, { 2, 0x49 }, { 3, 0x4B }, { 4, 0x50 }, { 5, 0x4F },   /* Heat, Elec, Slash, Erase, Charge */
		{ 11, 0x3B }, { 12, 0x51 }, { 13, 0x52 }, { 14, 0x54 }, { 15, 0x55 }, /* Proto, Blast, Dive, Circus, Judge */
		{ 16, 0x56 }, { 18, 0x53 },                                          /* Element, Colonel */
		{ 19, 0x5B }, { 20, 0x58 },                                          /* Bass, the Cybeast's beast */
	};
	for (unsigned i = 0; i < sizeof sprites / sizeof *sprites; ++i)
		if (sprites[i].navi == navi) return sprites[i].sprite;
	/* (BN5's: his own, copied in at the face's number, guardian_set_face) */
	if (guardian_older(navi) && set_face[navi]) return set_face[navi] - 1;
	return GUARDIAN_HEEL_SPRITE;
}

int guardian_older_sprite(int navi) {
	/* (BN5's lists 6 and 8: docs/ROM_DATA.md, BN5's Navis) */
	static const uint8_t numbers[GUARDIAN_OLDER_LAST - GUARDIAN_OLDER_FIRST + 1] = { 69, 75, 71, 72, 73, 74 };
	return guardian_older(navi) ? numbers[navi - GUARDIAN_OLDER_FIRST] : 0;
}

/* (BlastMan's, ElementMan's and Bass's sprites, drawn only towards the
 * camera, 2 to 5: an eighth turned the nearest way they show) */
static int toward_camera(int face) {
	static const int8_t drawn[8] = { 3, 3, 2, 3, 4, 5, 5, 5 };
	return drawn[face & 7];
}

/* Every guardian in his own shape (they had stood in a HeelNavi's body,
 * seven of seventeen, the copies "that didn't come out right"; the owner
 * asked for their personality back):
 *   Gregar's own overworld sprite where it has one, which faces every way;
 *   BlastMan's and ElementMan's, which Gregar draws only facing right
 *   through down-left (its story never turns them away from the camera):
 *   turned down-right or down-left where they would face into the screen
 *   (as they stood invisible there) or to the left (toward_camera);
 *   Falzar's Navis, whom Gregar has no overworld sprite of, in their
 *   battle sprites (list 0, 0x2E + the navi, each uncompressed), which face
 *   left as they fight, mirrored to face right, standing in animation 0
 *   and logging in by BN6's warp-in, their animation 3 (docs/ROM_DATA.md). */
/* How a Navi's overworld sprite logs in and out on the net, as BN6's
 * scenes play them beside the beam (docs/ROM_DATA.md, Guardians): its
 * animation 0x19 and 0x21 plus its facing, in Gregar's own Navis and BN5's;
 * BlastMan and DiveMan log out by their own and have no log-in, JudgeMan
 * has his own pair, ElementMan neither (he shows and goes with the beam) */
static void log_anims(NpcBody *b) {
	int dir = b->anim & 7;
	b->log_in = 0x19 + dir;
	b->log_out = 0x21 + dir;
	if (b->index == 0x51) { b->log_in = -1; b->log_out = b->anim <= 3 ? 0x1B : 0x1C; }
	else if (b->index == 0x52) { b->log_in = -1; b->log_out = 0x1C; }
	else if (b->index == 0x55) { b->log_in = 0x1B; b->log_out = 0x1A; }
	else if (b->index == 0x56) b->log_in = b->log_out = -1;
}

NpcBody guardian_body(int navi, int face) {
	NpcBody b = { 6, guardian_sprite(navi), face, false, -1, guardian(navi)->pose, -1 };
	if (navi == 12 || navi == 16) b.anim = toward_camera(face);
	log_anims(&b);
	/* (Falzar's in their battle sprites: BN6's warp-in, their animation 3,
	 * and its reverse, 4) */
	if (navi >= 6 && navi <= 10) b = (NpcBody){ 0, 0x2E + navi, 0, face >= 1 && face <= 3, 3, -1, 4 };
	return b;
}

/* The super bosses as they stand on the net (docs/BOSSES.md, Super
 * bosses), in BN6's own sprites and animations as its scenes use them:
 * Bass cloaked, drawn only towards the camera as BlastMan is (2 to 5),
 * throwing his cloak open (26) to stand without it (25), as he fights;
 * the Cybeast's beast crouching (28), rearing with a roar (29), its sprite
 * drawn facing left, mirrored to face right. */
SuperBody super_body(int navi, int face) {
	if (navi == SUPER_BASS)
		return (SuperBody){ { 6, guardian_sprite(navi), toward_camera(face), false, -1, 26, -1 }, 16, 25, false };
	return (SuperBody){ { 6, guardian_sprite(navi), 28, face >= 1 && face <= 3, -1, 29, -1 }, 60, 28, true };
}

int guardian_stand(int arena_dir, int gx, int gy, int spread, int *sx, int *sy) {
	/* (screen right is world +X +Y: grid x runs down-right as world +Y, grid
	 * y down-left as world -X; a bridge along grid +y or -x comes in from
	 * the guardian's right, one along +x or -y from his left) */
	bool right = (arena_dir & 3) == 1 || (arena_dir & 3) == 2;
	*sx = gx + (right ? spread : -spread);
	*sy = gy + (right ? spread : -spread);
	return right ? GUARDIAN_FACE_LEFT : GUARDIAN_FACE_RIGHT;
}

void guardian_set_face(int navi, int face) {
	if (navi > 0 && navi < NGUARDIANS) set_face[navi] = face < 0 ? 0 : face + 1;
}

int guardian_face(int navi) {
	const Guardian *g = guardian(navi);
	if (g->mugshot != GUARDIAN_NO_MUGSHOT) return g->mugshot;
	return navi > 0 && navi < NGUARDIANS && set_face[navi] ? set_face[navi] - 1 : FACE_NONE;
}

/* Another game's area where it draws `biome` in this run (run_dress). */
static const NetAreaDef *dressed(int biome) {
	int a = run_dress(biome);
	return a >= NET_AREAS ? net_area_def(a) : NULL;
}

bool guardian_area_older(int biome) { return dressed(biome) != NULL; }

/* BN6's own name for the area */
static const char *own_name(int biome) {
	static const char *const names[BIOME_COUNT] = {
		[BIOME_CENTRAL] = "Central Area", [BIOME_SEASIDE] = "Seaside Area", [BIOME_SKY] = "Sky Area",
		[BIOME_GREEN] = "Green Area", [BIOME_GRAVEYARD] = "Graveyard", [BIOME_UNDERNET] = "Undernet",
		[BIOME_SECRET] = "Secret Area", [BIOME_NEST] = "Cybeast Nest", [BIOME_COMP] = "RoboDog Comp",
		[BIOME_HOMEPAGE] = "Aquarium HP", [BIOME_COMP_B] = "Lab Comps",
		[BIOME_ROBOT_COMP] = "Robot Control Comp", [BIOME_AQUARIUM_COMP] = "Aquarium Comp",
		[BIOME_JUDGE_COMP] = "Judge Tree Comp", [BIOME_WEATHER_COMP] = "Mr. Weather Comp",
		[BIOME_COPYBOT_COMP] = "CopyBot's Comp", [BIOME_ACDC_HP] = "ACDC HP", [BIOME_GREEN_HP] = "Green HP",
		[BIOME_SKY_HP] = "Sky HP",
	};
	return biome >= 0 && biome < BIOME_COUNT && names[biome] ? names[biome] : "the Net";
}

const char *guardian_area_name(int biome) {
	const NetAreaDef *x = dressed(biome);
	return x ? x->name : own_name(biome);
}

const char *guardian_area_short(int biome) {
	const NetAreaDef *x = dressed(biome);
	if (x) return x->short_name;
	/* (nine letters at most: the PET's PLACE holds twelve, with the layer) */
	static const char *const names[BIOME_COUNT] = {
		[BIOME_CENTRAL] = "Central", [BIOME_SEASIDE] = "Seaside", [BIOME_SKY] = "Sky Area", [BIOME_GREEN] = "Green",
		[BIOME_GRAVEYARD] = "Graveyard", [BIOME_UNDERNET] = "Undernet", [BIOME_SECRET] = "Secret", [BIOME_NEST] = "Nest",
		[BIOME_COMP] = "RoboDog", [BIOME_HOMEPAGE] = "Aquarium", [BIOME_COMP_B] = "Lab Comp", [BIOME_ROBOT_COMP] = "RobotComp",
		[BIOME_AQUARIUM_COMP] = "Aquarium", [BIOME_JUDGE_COMP] = "JudgeTree", [BIOME_WEATHER_COMP] = "Weather",
		[BIOME_COPYBOT_COMP] = "CopyBot", [BIOME_ACDC_HP] = "ACDC HP", [BIOME_GREEN_HP] = "Green HP", [BIOME_SKY_HP] = "Sky HP",
	};
	return biome >= 0 && biome < BIOME_COUNT && names[biome] ? names[biome] : "Net";
}

const char *guardian_area_in_text(int biome, int side) {
	static char buf[32];
	if (side == LAYER_UNDERNET) biome = BIOME_UNDERNET;
	else if (side == LAYER_SECRET) biome = BIOME_SECRET;
	/* (another game's area in its place goes by its own name, "Nebula
	 * Area", but by BN6's as BN6 says it: BN5's Undernet is the Undernet) */
	const char *name = guardian_area_name(biome);
	bool the = !strcmp(name, own_name(biome)) && (biome == BIOME_GRAVEYARD || biome == BIOME_UNDERNET || biome == BIOME_SECRET || biome == BIOME_NEST);
	snprintf(buf, sizeof buf, "%s%s", the ? "the " : "", name);
	return buf;
}


const char *guardian_area_motto(int biome) {
	const NetAreaDef *x = dressed(biome);
	if (x) return x->motto;
	static const char *const mottos[BIOME_COUNT] = {
		[BIOME_CENTRAL] = "Where every Net path begins", [BIOME_SEASIDE] = "Currents of the aquarium Net",
		[BIOME_SKY] = "Above the clouds of data", [BIOME_GREEN] = "Wild data, overgrown",
		[BIOME_GRAVEYARD] = "Where deleted data rests", [BIOME_UNDERNET] = "The lawless depths",
		[BIOME_SECRET] = "Where the strongest wait", [BIOME_NEST] = "Lair of the Cybeasts",
		[BIOME_COMP] = "Circuits of a home comp", [BIOME_HOMEPAGE] = "The aquarium's own homepage",
		[BIOME_COMP_B] = "Deep in the lab's machines",
		[BIOME_ROBOT_COMP] = "The city's robots run here", [BIOME_AQUARIUM_COMP] = "Mazes of water and light",
		[BIOME_JUDGE_COMP] = "Roots of the great tree", [BIOME_WEATHER_COMP] = "Where the forecast is made",
		[BIOME_COPYBOT_COMP] = "A copy of a copy", [BIOME_ACDC_HP] = "Home of ACDC Town",
		[BIOME_GREEN_HP] = "Home of Green Town", [BIOME_SKY_HP] = "Home of Sky Town",
	};
	return biome >= 0 && biome < BIOME_COUNT && mottos[biome] ? mottos[biome] : "";
}
