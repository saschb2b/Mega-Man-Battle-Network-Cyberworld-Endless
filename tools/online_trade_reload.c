/* Independent unmodified-source reload control. Never links a candidate. */
#include "online_trade_runtime.h"

#include <stdio.h>
#include <mgba/core/core.h>
#include <mgba/internal/gba/serialize.h>

#include "bytes.h"

static bool source_at_boundary(const char *path) {
	FILE *file = fopen(path, "rb");
	if (!file) return false;
	uint8_t pc[4];
	bool ok = !fseek(file, (long)offsetof(struct GBASerializedState, cpu.gprs[15]), SEEK_SET) &&
		fread(pc, 1, sizeof pc, file) == sizeof pc && get32(pc) == BN6_OW_HOOK + 2;
	return !fclose(file) && ok;
}

int main(int argc, char **argv) {
	if (argc != 4 || !source_at_boundary(argv[2]) || !trade_probe_start(argv[1], argv[2])) return 2;
	trade_probe_directory(argv[3]);
	bool ok = trade_probe_saved_state("reload-control", false);
	trade_probe_core->deinit(trade_probe_core);
	return ok ? 0 : 2;
}
