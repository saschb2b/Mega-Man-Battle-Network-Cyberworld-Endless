/* Production reader, attached only to a private original-game research core. */
#include "online_carried_read.h"

#include <mgba/core/core.h>
#include "emu.h"
#include "sha256.h"

static struct mCore *observed;
uint8_t emu_read8(uint32_t address) { return observed->rawRead8(observed, address, -1); }
uint16_t emu_read16(uint32_t address) { return observed->rawRead16(observed, address, -1); }
uint32_t emu_read32(uint32_t address) { return observed->rawRead32(observed, address, -1); }

bool online_carried_read(struct mCore *core, OnlineLoadout *out) {
	observed = core;
	bool ok = online_snapshot_read(out, 0);
	observed = NULL;
	return ok;
}

void online_carried_blob(FILE *out, const void *data, size_t bytes) {
	uint8_t hash[32];
	cw_sha256(data, bytes, hash);
	fprintf(out, "{\"bytes\":%zu,\"sha256\":\"", bytes);
	for (unsigned i = 0; i < sizeof hash; ++i) fprintf(out, "%02x", hash[i]);
	fputs("\"}", out);
}

bool online_carried_write(FILE *out, const OnlineLoadout *n) {
	fputs("{\"folder\":[", out);
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) fprintf(out, "%s%u", i ? "," : "", n->folder[i]);
	fputs("],\"pack\":", out); online_carried_blob(out, n->pack, sizeof n->pack);
	fputs(",\"board\":", out); online_carried_blob(out, n->board, sizeof n->board);
	fputs(",\"grid\":", out); online_carried_blob(out, n->grid, sizeof n->grid);
	fputs(",\"record\":", out); online_carried_blob(out, n, sizeof *n);
	fprintf(out, ",\"hp\":%u,\"max_hp\":%u,\"base_hp\":%u,\"attack\":%u,\"speed\":%u,\"charge\":%u,"
		"\"regular_mb\":%u,\"mega_limit\":%u,\"giga_limit\":%u,\"regular\":%u,\"tag\":[%u,%u],"
		"\"crosses\":%u,\"souls\":%u,\"beast\":%u}\n", n->hp, n->max_hp, n->base_hp,
		n->attack, n->speed, n->charge, n->regular_mb, n->mega_limit, n->giga_limit, n->regular,
		n->tag[0], n->tag[1], n->crosses, n->souls, n->beast);
	return !ferror(out);
}
