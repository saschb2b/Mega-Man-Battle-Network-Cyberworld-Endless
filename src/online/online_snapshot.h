/* The carried MegaMan's data, read without changing his run (issue #120).
 * A local experiment record; it is deliberately not a network format. */
#ifndef CW_ONLINE_SNAPSHOT_H
#define CW_ONLINE_SNAPSHOT_H

#include <stdbool.h>
#include <stdint.h>

#include "bn6.h"

#define ONLINE_PACK_IDS 512
#define ONLINE_BOARD_BYTES (BN6_NAVICUST_SLOTS * 8)
#define ONLINE_BOARD_CELLS 49

typedef struct {
	uint16_t folder[BN6_FOLDER_ENTRIES];
	uint8_t pack[ONLINE_PACK_IDS][BN6_PACK_ENTRY];
	uint8_t board[ONLINE_BOARD_BYTES];
	uint8_t grid[ONLINE_BOARD_CELLS];
	uint16_t hp, max_hp, base_hp;
	uint8_t attack, speed, charge;
	uint8_t regular_mb, mega_limit, giga_limit, regular, tag[2];
	uint8_t crosses, souls, beast;
} OnlineLoadout;

/* Between emulator frames. The caller supplies souls_held(), which is a
 * run-side record rather than BN6 RAM. False before the folder/pack exist. */
bool online_snapshot_read(OnlineLoadout *out, unsigned souls);

/* ... from another core's memory, a byte at a time (the Arena's reading
 * of a checkpoint's state: arena_carry.c) */
typedef struct {
	uint8_t (*read8)(void *context, uint32_t address);
	void *context;
} OnlineMemory;
bool online_snapshot_read_from(const OnlineMemory *m, OnlineLoadout *out, unsigned souls);

#endif
