/* The run's MegaMan for the Arena (arena_carry.h). */
#include "arena_carry.h"

#include <stdio.h>
#include <stdlib.h>

#include <mgba/core/core.h>

#include "arena_pair.h"
#include "rom.h"
#include "run.h"
#include "save.h"
#include "souls.h"

#define STATE_MOST (4ul << 20)   /* a checkpoint's state is under 1 MB: past this it is not one */

static uint8_t core_read8(void *context, uint32_t address) {
	struct mCore *c = context;
	return (uint8_t)c->rawRead8(c, address, -1);
}

static uint8_t *read_file(const char *path, size_t *size) {
	*size = 0;
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	uint8_t *bytes = NULL;
	long n = fseek(f, 0, SEEK_END) == 0 ? ftell(f) : -1;
	if (n > 0 && (unsigned long)n <= STATE_MOST && fseek(f, 0, SEEK_SET) == 0) bytes = malloc((size_t)n);
	if (bytes && fread(bytes, 1, (size_t)n, f) != (size_t)n) {
		free(bytes);
		bytes = NULL;
	}
	fclose(f);
	if (bytes) *size = (size_t)n;
	return bytes;
}

uint8_t *arena_carry_state(size_t *size) {
	*size = 0;
	Run saved;
	if (!R.data || !peek_run(&saved)) return NULL;
	char path[600];
	save_state_path(path, sizeof path);
	return read_file(path, size);
}

bool arena_carry_read(OnlineLoadout *out) {
	Run saved;
	if (!R.data || !peek_run(&saved)) return false;
	size_t size;
	uint8_t *state = arena_carry_state(&size);
	if (!state) return false;
	static uint32_t video[ARENA_W * ARENA_H];
	struct mCore *c = arena_core_new(R.data, ROM_SIZE, video, false);
	bool ok = c && arena_core_load(c, state, size);
	free(state);
	if (ok) {
		/* (the run's Souls are its own record, beside the state) */
		souls_load(saved.seed);
		const OnlineMemory m = { core_read8, c };
		ok = online_snapshot_read_from(&m, out, souls_held());
	}
	arena_core_free(c);
	return ok;
}
