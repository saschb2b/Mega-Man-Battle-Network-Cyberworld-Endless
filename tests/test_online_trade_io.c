#include "test_online_trade_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "emu.h"
#include "online_trade_files.h"
#include "online_trade_gate.h"
#include "save_queue.h"

static int failures;
#define CHECK(value) do { if (!(value)) { fprintf(stderr, "trade I/O FAIL line %d: %s\n", __LINE__, #value); ++failures; } } while (0)

static void remove_at(const char *directory, const char *name) {
	char path[TRADE_PATH_MAX];
	CHECK(trade_files_path(path, directory, name) && trade_files_remove(path));
}

int test_online_trade_io(void) {
	failures = 0;
	char root[] = "/tmp/cw-trade-io-XXXXXX", current[TRADE_PATH_MAX], path[TRADE_PATH_MAX], boot[TRADE_PATH_MAX];
	CHECK(mkdtemp(root));
	CHECK(trade_files_path(current, root, "savedata") && trade_files_mkdir(current));
	CHECK(trade_files_path(path, current, "run.state") && trade_files_path(boot, root, "boot-3.state"));
	save_queue_wait();
	online_trade_gate_start(root);
	CHECK(online_trade_gate_state() == TRADE_GATE_CLEAR);
	emu_write32(0x02000000, 0x12345678);
	CHECK(emu_save_state(path));
	uint8_t before[32], after[32];
	CHECK(trade_files_hash(current, false, before));
	CHECK(online_trade_gate_new_run(19));
	CHECK(!online_trade_gate_recover() && online_trade_gate_state() == TRADE_GATE_UNBOUND);
	unsigned sequence = save_write_failures();
	uint8_t bytes[4] = { 1, 2, 3, 4 };
	CHECK(!save_file_write(path, bytes, sizeof bytes));
	CHECK(save_write_failures() != sequence);
	uint8_t *owned = malloc(sizeof bytes);
	CHECK(owned);
	if (owned) {
		memcpy(owned, bytes, sizeof bytes);
		bool accepted = save_queue_file(path, owned, sizeof bytes);
		CHECK(!accepted);
		if (!accepted) free(owned);
	}
	CHECK(!emu_save_state(path) && !emu_save_state_queued(path));
	save_queue_wait();
	CHECK(trade_files_hash(current, false, after) && !memcmp(before, after, sizeof before));
	emu_write32(0x02000000, 0xABCDEF01);
	CHECK(!emu_load_state(path) && emu_read32(0x02000000) == 0xABCDEF01);
	/* The boot cache carries no run and can still be rebuilt at startup. */
	CHECK(emu_save_state(boot));
	emu_write32(0x02000000, 0);
	CHECK(emu_load_state(boot) && emu_read32(0x02000000) == 0xABCDEF01);
	CHECK(online_trade_gate_discard() && online_trade_gate_state() == TRADE_GATE_CLEAR);
	CHECK(trade_files_kind(path) == 0);
	remove_at(root, "boot-3.state");
	remove_at(root, "trade.history");
	remove_at(root, "trade.lock");
	CHECK(rmdir(current) == 0 && rmdir(root) == 0);
	return failures;
}
