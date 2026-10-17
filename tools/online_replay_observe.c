#include "online_replay_observe.h"

#include <stdlib.h>
#include <string.h>
#include <mgba/internal/gba/gba.h>

#include "online_frame_pair_internal.h"
#include "online_link_script.h"
#include "sha256.h"

static void write_blob(FILE *out, const void *data, size_t size) {
	uint8_t hash[32];
	cw_sha256(data, size, hash);
	fprintf(out, "{\"bytes\":%zu,\"sha256\":\"", size);
	for (unsigned i = 0; i < sizeof hash; ++i) fprintf(out, "%02x", hash[i]);
	fputs("\"}", out);
}

static bool write_cores(OnlineFramePair *p, FILE *out) {
	fputs("\"cores\":[", out);
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *core = p->core[side];
		memset(p->state[side], 0, p->state_bytes[side]);
		if (!core->saveState(core, p->state[side])) return false;
		if (side) fputc(',', out);
		fputs("{\"state\":", out);
		write_blob(out, p->state[side], p->state_bytes[side]);
		void *flash = NULL;
		size_t bytes = core->savedataClone(core, &flash);
		if (bytes && !flash) return false;
		fputs(",\"flash\":", out);
		write_blob(out, flash, bytes);
		free(flash);
		fputc('}', out);
	}
	fputs("],", out);
	return true;
}

static const char *boolean(bool value) { return value ? "true" : "false"; }

static void write_nodes(OnlineFramePair *p, FILE *out) {
	fputs("\"nodes\":[", out);
	for (unsigned i = 0; i < 2; ++i) {
		struct GBASIOLockstepNode *n = &p->node[i];
		struct GBA *gba = p->core[i]->board;
		if (i) fputc(',', out);
		fprintf(out, "{\"credit\":%d,\"next_event\":%d,\"event_diff\":%d,\"mode\":%u,"
			"\"normal_so\":%s,\"transfer_finished\":%s,\"siocnt\":%u,\"rcnt\":%u,"
			"\"scheduled\":%s,\"event_delay\":%d,\"event_priority\":%u}",
			p->credit[i], n->nextEvent, n->eventDiff, (unsigned)n->mode,
			boolean(n->normalSO), boolean(n->transferFinished), n->d.p->siocnt, n->d.p->rcnt,
			boolean(mTimingIsScheduled(&gba->timing, &n->event)),
			mTimingUntil(&gba->timing, &n->event), n->event.priority);
	}
	fputs("]}", out);
}

static void write_cable(OnlineFramePair *p, FILE *out) {
	fprintf(out, "\"cable\":{\"attached\":%d,\"phase\":%u,\"cycles\":%d,\"attached_multi\":%d,"
		"\"attached_normal\":%d,\"waiting\":%u,\"transfers\":%u,\"running\":%d,\"multi_recv\":[",
		p->cable.d.attached, (unsigned)p->cable.d.transferActive, p->cable.d.transferCycles,
		p->cable.attachedMulti, p->cable.attachedNormal, p->waiting, p->transfers, p->running);
	for (unsigned i = 0; i < 4; ++i) fprintf(out, "%s%u", i ? "," : "", p->cable.multiRecv[i]);
	fputs("],\"normal_recv\":[", out);
	for (unsigned i = 0; i < 4; ++i) fprintf(out, "%s%u", i ? "," : "", p->cable.normalRecv[i]);
	fputs("],", out);
	write_nodes(p, out);
}

static void write_samples(OnlineFramePair *p, FILE *out) {
	fputs(",\"samples\":[", out);
	for (unsigned side = 0; side < 2; ++side) {
		uint32_t sample[8];
		online_link_battle_sample(p->core[side], side, sample, NULL);
		fputs(side ? ",[" : "[", out);
		for (unsigned i = 0; i < 8; ++i) fprintf(out, "%s%u", i ? "," : "", sample[i]);
		fputc(']', out);
	}
	fputs("],\"inputs\":[", out);
	for (unsigned ahead = 0; ahead < ONLINE_FRAME_LOOKAHEAD; ++ahead)
		fprintf(out, "%s[%u,%u]", ahead ? "," : "", online_frame_script_key(p->frame + ahead, 0),
			online_frame_script_key(p->frame + ahead, 1));
	fputc(']', out);
}

bool online_replay_observe(OnlineFramePair *p, FILE *out) {
	fprintf(out, "{\"frame\":%u,\"hash_algorithm\":\"sha256\",", p->frame);
	if (!write_cores(p, out)) return false;
	write_cable(p, out);
	fprintf(out, ",\"pair\":{\"frame\":%u,\"start_frames\":[%u,%u],\"traced_frames\":[%u,%u],"
		"\"entry_step\":%u,\"entry_frame\":%u}", p->frame, p->start_frame[0], p->start_frame[1],
		p->traced[0], p->traced[1], p->entry_step, p->entry_frame);
	write_samples(p, out);
	fputs("}\n", out);
	return !ferror(out);
}
