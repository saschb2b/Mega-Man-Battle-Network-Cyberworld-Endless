/* State of the current run and the permanent profile. */
#ifndef RUN_H
#define RUN_H

#include <stdbool.h>
#include <stdint.h>

#define FOLDER_MAX 30
#define PACK_MAX 60

typedef struct {
	uint16_t id;
	char code;
} FolderChip;

/* Programs: NaviCust parts found during a run. */
enum {
	PERK_SUPER_ARMOR = 1 << 0, /* no flinch */
	PERK_UNDERSHIRT = 1 << 1,  /* survive one lethal hit per battle at 1 HP */
	PERK_FLOAT_SHOES = 1 << 2, /* ignore panel effects */
	PERK_AIR_SHOES = 1 << 3,   /* stand on broken panels */
	PERK_FIRST_BARRIER = 1 << 4,
	PERK_COLLECT = 1 << 5,     /* extra chip choice after battles */
	PERK_RAPID_CUSTOM = 1 << 6,/* custom gauge fills faster */
	PERK_REFLECT = 1 << 7,     /* shield on L */
	PERK_HUMOR = 1 << 8,       /* better zenny */
	PERK_ATTACK_MAX = 1 << 9,  /* charge shot power x1.5 */
	PERK_COUNT_BITS = 10,
};

enum { BIOME_CENTRAL, BIOME_SEASIDE, BIOME_SKY, BIOME_GREEN, BIOME_GRAVEYARD, BIOME_UNDERNET, BIOME_SECRET, BIOME_NEST, BIOME_COUNT };

typedef struct {
	bool active;
	uint32_t seed;
	int hp, max_hp;
	int zenny, bugfrags;
	int atk, rapid, charge;  /* buster levels 1-5 */
	int custom_size;         /* chips offered in the custom screen */
	FolderChip folder[FOLDER_MAX];
	int folder_n;
	uint32_t perks;
	uint32_t crosses;        /* bit per navi index whose cross was earned */
	int depth;               /* 1-based layer */
	int biome;
	int bosses_beaten;
	int viruses_deleted;
	uint32_t frames;
	int score;
	bool beast_out;          /* Gregar beast form unlocked this run */
	uint8_t biome_order[6];  /* act -> biome for this run */
	uint8_t boss_order[8];   /* biome -> navi for this run */
	int fragments;           /* secret data fragments (3 open the Secret Area) */
	int unlockers;           /* open purple mystery data */
	int side_kind;           /* LAYER_* while in an Undernet or Secret layer */
	uint32_t layer_seed;
	bool secret_cleared;
	uint64_t layer_used;     /* objects used on the current layer */
	bool layer_boss_beaten;
} Run;

#define CYCLE_LAYERS 19      /* 6 acts of 3 layers, then the Cybeast Nest */

extern Run run;

void run_new(uint32_t seed);
void folder_add(int id, char code);
const char *biome_name(int b);
int biome_bg(int b);
int biome_song(int b);

#endif
