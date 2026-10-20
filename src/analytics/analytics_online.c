/* analytics_online.h. Online play's events, written as analytics.c writes
 * its own (analytics_payload.h) and sent the same way: only where the
 * session sends, nothing but numbers and the words below. */
#include "analytics_online.h"

#include <stddef.h>

#include "analytics.h"
#include "analytics_payload.h"
#include "online_visit.h"

/* The visit's end as the statistics name it */
static const char *visit_end(int ended) {
	switch (ended) {
	case VISIT_END_LEFT: return "left";
	case VISIT_END_CANCELLED: return "called off";
	case VISIT_END_UNREACHABLE: return "no answer";
	case VISIT_END_VERSION: return "another version";
	case VISIT_END_ROM: return "another ROM";
	case VISIT_END_NAME: return "name refused";
	case VISIT_END_FULL: return "full";
	case VISIT_END_BARRED: return "barred";
	case VISIT_END_REMOVED: return "removed";
	case VISIT_END_LOST: return "lost";
	case VISIT_END_BROKEN: return "not built";
	case VISIT_END_DIVE: return "dive";
	default: return "closed";
	}
}

/* (seconds as whole minutes, rounded) */
static int minutes(int seconds) { return (seconds + 30) / 60; }

void analytics_visit(int ended, bool full, int seconds, bool official) {
	if (!analytics_sends()) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	analytics_event_begin(&j, out, sizeof out, "online-visit");
	json_member(&j, "system", analytics_system());
	json_member(&j, "pass", full ? "full" : "square");
	json_member(&j, "ended", visit_end(ended));
	if (seconds >= 0) json_int(&j, "minutes", minutes(seconds));
	json_bool(&j, "official", official);
	analytics_event_send(&j);
}

static const char *battle_end(int how) {
	static const char *const words[] = { "won", "lost", "over", "disconnected", "apart", "stopped", "left" };
	return how >= 0 && how < (int)(sizeof words / sizeof *words) ? words[how] : "stopped";
}

void analytics_netbattle(const AnalyticsBattle *b) {
	if (!analytics_sends() || !b) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	analytics_event_begin(&j, out, sizeof out, "netbattle");
	json_member(&j, "system", analytics_system());
	json_member(&j, "result", battle_end(b->how));
	json_int(&j, "side", b->side);
	json_int(&j, "delay", b->delay);
	json_int(&j, "window", b->window);
	json_int(&j, "rollbacks", b->rollbacks);
	json_int(&j, "deepest", b->deepest);
	json_int(&j, "stalls", b->stalls);
	json_int(&j, "worst_tick_ms", b->worst_tick_ms > 0 ? (int)(b->worst_tick_ms + 0.5) : 0);
	if (b->frames > 0) json_int(&j, "seconds", (b->frames + 30) / 60);
	analytics_event_send(&j);
}

void analytics_trade(bool board) {
	if (!analytics_sends()) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	analytics_event_begin(&j, out, sizeof out, "online-trade");
	json_bool(&j, "board", board);
	analytics_event_send(&j);
}

void analytics_watch(int how, int seconds) {
	static const char *const words[] = { "end", "left", "too slow", "stopped", "lost" };
	if (!analytics_sends()) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	analytics_event_begin(&j, out, sizeof out, "online-watch");
	json_member(&j, "ended", how >= 0 && how < (int)(sizeof words / sizeof *words) ? words[how] : "stopped");
	if (seconds >= 0) json_int(&j, "seconds", seconds);
	analytics_event_send(&j);
}

void analytics_dive_post(int layer, int act, bool won) {
	if (!analytics_sends()) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	analytics_event_begin(&j, out, sizeof out, "deepnet-post");
	json_int(&j, "layer", layer);
	json_int(&j, "act", act);
	json_bool(&j, "won", won);
	analytics_event_send(&j);
}
