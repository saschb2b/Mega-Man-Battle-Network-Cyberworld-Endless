#include "online_capture.h"

#include <stdio.h>
#include <stdlib.h>

#include "run.h"
#include "sha256.h"

void online_capture_loadout(const OnlineLoadout *navi, bool checkpoint) {
	const char *path = getenv("CYBERWORLD_LOADOUT_TRACE");
	if (!path || !*path) return;
	FILE *out = fopen(path, "w");
	if (!out) { fprintf(stderr, "online: cannot write local loadout trace\n"); return; }
	fprintf(out, "{\"checkpoint\":%s,\"run_active\":%s,\"hp\":%u,\"max_hp\":%u,"
		"\"crosses\":%u,\"souls\":%u,\"beast\":%u,\"helpers\":%u,\"folder\":[",
		checkpoint ? "true" : "false", run.active ? "true" : "false", navi->hp, navi->max_hp,
		navi->crosses, navi->souls, navi->beast, run.helpers);
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) fprintf(out, "%s%u", i ? "," : "", navi->folder[i]);
	fputs("],\"pack_sha256\":\"", out);
	uint8_t hash[32];
	cw_sha256(navi->pack, sizeof navi->pack, hash);
	for (unsigned i = 0; i < sizeof hash; ++i) fprintf(out, "%02x", hash[i]);
	fputs("\"}\n", out);
	if (fclose(out)) fprintf(stderr, "online: local loadout trace could not be completed\n");
}
