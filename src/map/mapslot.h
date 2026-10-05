/* A game map taken over by a generated layer: its NPC list, map scripts,
 * object spawns and Mystery Data point at the engine's own data. */
#ifndef CW_MAPSLOT_H
#define CW_MAPSLOT_H

#include <stdbool.h>
#include <stdint.h>

#define MAPSLOT_SPRITES 12
#define MAPSLOT_SPRITE_BYTES 0x8800

/* NPC scripts the layer runs (bus addresses of script bytecode), up to 32. */
typedef struct {
	uint32_t script[32];
	int n;
	/* compressed sprites the NPCs and objects use (list byte offset, index),
	 * loaded with the map: the game takes 12 and 0x8800 bytes decompressed */
	uint8_t sprite_cat[MAPSLOT_SPRITES], sprite_idx[MAPSLOT_SPRITES];
	int nsprites;
	uint32_t sprite_bytes;
	/* the map's object spawns (20-byte records ending 0xFF, bus address), or 0 */
	uint32_t objects;
} NpcList;

/* One Mystery Data: flag 0x1400 + index, its color, where, and what it
 * holds (the game's 8-byte content record). */
typedef struct {
	int x, y, z;
	int type;
	uint8_t content[8];
} MysteryData;

#define MYSTERY_BLUE   1
#define MYSTERY_PURPLE 3   /* locked: the game's text asks for an Unlocker (item 0x85) and takes it */
#define MYSTERY_GREEN  5

/* Clears map (group, number) of the original's NPCs, scripts, objects and
 * Mystery Data, and installs the layer's (or the town's, in the real world). */
bool mapslot_install(int group, int number, const NpcList *npcs, const MysteryData *md, int nmd);

/* Where the layer's exit pad (warp 1) leads: world (x, y) of map (group,
 * number), facing `facing`. */
void mapslot_exit_to(int group, int number, int x, int y, int facing);
/* Warp entry `entry` (2-15, its pads' trigger value) of the layer's map: a
 * teleport within map (group, number) to world (x, y), as BN6's gem pads
 * warp (departure 12, the camera scrolling along: issue #44). */
void mapslot_teleport(int entry, int group, int number, int x, int y, int facing);

/* The map's theme: every chapter's map music list plays `song` there. The
 * lists hold one real-world map (the town) and one internet map (the
 * layer) at a time. */
bool mapslot_music(int group, int number, int song);
void mapslot_music_forget_town(void);

/* Jacking in from real-world map (group, number) at trigger 0x40 takes
 * MegaMan to world (x, y) of (to_group, to_number), with the game's own
 * jack-in (one of its 20-byte destinations is taken over). */
bool mapslot_jack_in(int group, int number, int to_group, int to_number, int x, int y, int facing);
/* ... and where its jack-in leads now, mapslot_jack_in's table kept (a
 * town's port taking another way, docs/HOME.md) */
void mapslot_jack_to(int to_group, int to_number, int x, int y, int facing);

/* The town's warp list: every entry leads back to its own world (x, y),
 * so no trigger left in it can take Lan anywhere else. */
bool mapslot_town_warps(int group, int number, int x, int y, int facing);

/* What the map's checks say (section-3 triggers 0xF0 + n, answered by A):
 * script[n] of `archive` (a text archive's bytes; 0xFF none). The archive
 * becomes the map's own, which the game decompresses when it enters. */
bool mapslot_checks(int group, int number, const uint8_t script[16], const uint8_t *archive, int len);

/* Allocation from the town's own space (on) or the layers' (off): the
 * layers' halves are reused while the town still runs. */
void mapslot_town(bool on);

/* Space in the free ROM for NPC scripts; returns the bus address. A reset
 * starts the next layer in the other half of the space. */
uint32_t mapslot_alloc(const void *bytes, int len);
void mapslot_reset(void);

/* The first Mystery Data flag (index 0). */
#define MAPSLOT_MD_FLAG 0x1400

#endif
