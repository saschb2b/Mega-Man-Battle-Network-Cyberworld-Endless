/* Each original native video boundary is observed once, without emulation writes. */
#include "online_carried_observe.h"

#include <stdio.h>
#include <stdlib.h>
#include <mgba/internal/gba/gba.h>
#include "bn6.h"
#include "online_frame_pair_internal.h"

typedef struct {
	unsigned frame, ordinal, input, main, menu, state, timer, result, side;
	unsigned object[2][6], custom[4], picked, picks[6], deck[30];
	unsigned hand_at, ids[6], powers[6], bonuses[6], rng[2];
} Observation;
typedef struct {
	struct mCore *core;
	unsigned total, queued;
	bool overflow;
	Observation queue[16];
} PlayerObserver;
struct OnlineCarriedObserver { PlayerObserver players[2]; FILE *out; };

static void words(struct mCore *core, unsigned *out, unsigned count, uint32_t at, bool bytes) {
	for (unsigned i = 0; i < count; ++i)
		out[i] = bytes ? core->rawRead8(core, at + i, -1) : core->rawRead16(core, at + 2 * i, -1);
}

static void frame_started(void *context) {
	PlayerObserver *p = context;
	if (p->queued == sizeof p->queue / sizeof *p->queue) { p->overflow = true; return; }
	Observation *r = &p->queue[p->queued++];
	struct mCore *c = p->core;
	r->frame = c->frameCounter(c); r->ordinal = p->total++;
	r->input = ((struct GBA *)c->board)->keysActive;
	r->main = c->rawRead8(c, c->rawRead32(c, BN6_TOOLKIT, -1), -1);
	r->menu = c->rawRead32(c, c->rawRead32(c, BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU, -1), -1);
	r->state = c->rawRead32(c, BN6_BATTLE_STATE, -1); r->timer = c->rawRead32(c, BN6_BATTLE_TIMER, -1);
	r->result = c->rawRead8(c, BN6_BATTLE_RESULT, -1); r->side = c->rawRead8(c, BN6_BATTLE_SIDE, -1);
	for (unsigned i = 0; i < 2; ++i) {
		uint32_t at = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		r->object[i][0] = c->rawRead16(c, at + BN6_T1_NAME_ID, -1);
		r->object[i][1] = c->rawRead16(c, at + BN6_T1_HP, -1);
		r->object[i][2] = c->rawRead8(c, at + BN6_T1_ALLIANCE, -1);
		r->object[i][3] = c->rawRead8(c, at + BN6_T1_PANEL_X, -1);
		r->object[i][4] = c->rawRead8(c, at + BN6_T1_PANEL_Y, -1);
		r->object[i][5] = c->rawRead16(c, at + BN6_T1_CHIP, -1);
	}
	r->custom[0] = c->rawRead8(c, BN6_CUSTOM_MODE, -1); r->custom[1] = c->rawRead8(c, BN6_CUSTOM_CURSOR, -1);
	r->custom[2] = c->rawRead8(c, BN6_CUSTOM_HAND, -1); r->custom[3] = c->rawRead8(c, BN6_CUSTOM_WINDOW, -1);
	r->picked = c->rawRead8(c, BN6_CUSTOM_PICKED, -1);
	words(c, r->picks, 6, BN6_CUSTOM_PICKS, true); words(c, r->deck, 30, BN6_BATTLE_DECK, false);
	r->hand_at = c->rawRead16(c, BN6_BATTLE_HAND + BN6_HAND_AT, -1);
	words(c, r->ids, 6, BN6_BATTLE_HAND + BN6_HAND_CHIPS, false);
	words(c, r->powers, 6, BN6_BATTLE_HAND + BN6_HAND_POWERS, false);
	words(c, r->bonuses, 6, BN6_BATTLE_HAND + BN6_HAND_BONUS, false);
	r->rng[0] = c->rawRead32(c, BN6_RNG_PRIMARY, -1); r->rng[1] = c->rawRead32(c, BN6_RNG_SECONDARY, -1);
}

static void array(FILE *out, const unsigned *values, unsigned count) {
	fputc('[', out);
	for (unsigned i = 0; i < count; ++i) fprintf(out, "%s%u", i ? "," : "", values[i]);
	fputc(']', out);
}

static void record(FILE *out, unsigned side, const Observation *r) {
	fprintf(out, "{\"core\":%u,\"frame\":%u,\"ordinal\":%u,\"input\":%u,\"main\":%u,\"menu\":%u,"
		"\"state\":%u,\"timer\":%u,\"result\":%u,\"side\":%u,\"objects\":[", side, r->frame, r->ordinal,
		r->input, r->main, r->menu, r->state, r->timer, r->result, r->side);
	for (unsigned i = 0; i < 2; ++i) {
		const unsigned *o = r->object[i];
		fprintf(out, "%s{\"id\":%u,\"hp\":%u,\"alliance\":%u,\"x\":%u,\"y\":%u,\"chip\":%u}",
			i ? "," : "", o[0], o[1], o[2], o[3], o[4], o[5]);
	}
	fprintf(out, "],\"custom\":{\"mode\":%u,\"cursor\":%u,\"size\":%u,\"window\":%u,\"picked\":%u,\"picks\":",
		r->custom[0], r->custom[1], r->custom[2], r->custom[3], r->picked);
	array(out, r->picks, 6); fputs(",\"deck\":", out); array(out, r->deck, 30);
	fprintf(out, "},\"hand\":{\"at\":%u,\"ids\":", r->hand_at); array(out, r->ids, 6);
	fputs(",\"powers\":", out); array(out, r->powers, 6);
	fputs(",\"bonuses\":", out); array(out, r->bonuses, 6);
	fputs("},\"rng\":", out); array(out, r->rng, 2); fputs("}\n", out);
}

OnlineCarriedObserver *online_carried_observer_open(OnlineFramePair *pair, const char *prefix) {
	char path[1024];
	if (snprintf(path, sizeof path, "%s.jsonl", prefix) >= (int)sizeof path) return NULL;
	OnlineCarriedObserver *out = calloc(1, sizeof *out);
	if (!out) return NULL;
	out->out = fopen(path, "w");
	if (!out->out) { free(out); return NULL; }
	for (unsigned i = 0; i < 2; ++i) {
		out->players[i].core = pair->core[i];
		struct mCoreCallbacks callbacks = { .context = &out->players[i], .videoFrameStarted = frame_started };
		pair->core[i]->addCoreCallbacks(pair->core[i], &callbacks);
	}
	return out;
}

bool online_carried_observer_flush(OnlineCarriedObserver *observer) {
	for (unsigned i = 0; i < 2; ++i) {
		PlayerObserver *p = &observer->players[i];
		if (p->overflow) return false;
		for (unsigned j = 0; j < p->queued; ++j) record(observer->out, i, &p->queue[j]);
		p->queued = 0;
	}
	return !ferror(observer->out);
}

void online_carried_observer_close(OnlineCarriedObserver *observer) {
	if (!observer) return;
	for (unsigned i = 0; i < 2; ++i) observer->players[i].core->clearCoreCallbacks(observer->players[i].core);
	fclose(observer->out);
	free(observer);
}
