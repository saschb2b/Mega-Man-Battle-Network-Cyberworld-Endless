#include "online_capture.h"

#include <stdio.h>
#include <stdlib.h>

#include "online_trade_panel.h"
#include "online_trade_text.h"
#include "pixfont.h"
#include "platform.h"
#include "run.h"
#include "second_frame.h"
#include "sha256.h"

static bool preview;
static OnlineTradePanel panel;

void online_capture_start(void) {
	preview = false;
	const char *value = getenv("CYBERWORLD_TRADE_PREVIEW");
	if (!value || !*value) return;
	char *end;
	long phase = strtol(value, &end, 10);
	if (*end || phase < 0 || phase >= TRADE_PANEL_PHASES) return;
	/* Native fixture entries verified by the routine-level trade observer. */
	panel = (OnlineTradePanel){ .give = 2 | 11 << 9, .receive = 3 | 17 << 9,
		.peer = "NaviTraveler", .phase = (OnlineTradePanelPhase)phase };
	preview = true;
}

bool online_capture_preview(void) { return preview; }

void online_capture_draw(void) {
	if (!preview) return;
	online_trade_panel_draw(&panel);
	pixfont_draw(P.core_x + 8, P.core_y + 20, trade_panel_preview, PET_GOLD, PET_EXIT, PIXFONT_LEFT, 1);
}

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
