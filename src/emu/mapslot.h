/* A game map taken over by a generated layer: its NPC list, map scripts,
 * object spawns and Mystery Data point at the engine's own data. */
#ifndef CW_MAPSLOT_H
#define CW_MAPSLOT_H

#include <stdbool.h>
#include <stdint.h>

/* NPC scripts the layer runs (bus addresses of script bytecode), up to 32. */
typedef struct {
	uint32_t script[32];
	int n;
	/* compressed sprites the NPCs use (list byte offset, index), loaded with the map */
	uint8_t sprite_cat[8], sprite_idx[8];
	int nsprites;
} NpcList;

/* One Mystery Data: flag 0x1400 + index, its color, where, and what it
 * holds (the game's 8-byte content record). */
typedef struct {
	int x, y;
	int type;
	uint8_t content[8];
} MysteryData;

#define MYSTERY_BLUE  1
#define MYSTERY_GREEN 5

/* Clears map (group, number) of the original's NPCs, scripts, objects and
 * Mystery Data, and installs the layer's. */
bool mapslot_install(int group, int number, const NpcList *npcs, const MysteryData *md, int nmd);

/* Space in the free ROM for NPC scripts; returns the bus address. */
uint32_t mapslot_alloc(const void *bytes, int len);
void mapslot_reset(void);

/* The first Mystery Data flag (index 0). */
#define MAPSLOT_MD_FLAG 0x1400

#endif
