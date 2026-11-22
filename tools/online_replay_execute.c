#include "online_replay_execute.h"

#include <string.h>
#include "online_frame_pair_internal.h"
#include "online_frame_wire.h"
#include "online_link_script.h"

void online_replay_executor_init(OnlineReplayExecutor *e, OnlineFramePair *pair) {
	memset(e, 0, sizeof *e);
	e->pair = pair;
}

static void begin_frame(OnlineReplayExecutor *e) {
	OnlineFramePair *p = e->pair;
	e->battle = p->frame >= online_link_entry_frames();
	e->relative = e->battle ? p->frame - online_link_entry_frames() : p->entry_frame;
	e->round = 0;
	for (unsigned side = 0; side < 2; ++side) {
		e->target[side] = p->start_frame[side] + e->relative + 1;
		p->core[side]->setKeys(p->core[side], online_frame_script_key(p->frame, side));
	}
	e->in_frame = true;
}

static void trace_core(OnlineFramePair *p, unsigned side, unsigned frame) {
	if (frame == p->traced[side]) return;
	uint32_t sample[8];
	online_link_battle_sample(p->core[side], side, sample, NULL);
	fprintf(p->trace, "%u,%u,%u,%u,%u,%u,%u,%u,%u\n", side, frame, sample[2], sample[0],
		sample[3] & 65535, sample[3] >> 16, sample[4], sample[6], sample[7]);
	p->traced[side] = frame;
}

static bool run_round(OnlineReplayExecutor *e) {
	OnlineFramePair *p = e->pair;
	for (unsigned side = 0; side < 2; ++side) {
		if (!side && p->waiting) continue;
		p->running = (int)side;
		enum mLockstepPhase phase = p->cable.d.transferActive;
		unsigned frame = p->core[side]->frameCounter(p->core[side]);
		if (e->battle) {
			unsigned ahead = frame - p->start_frame[side] - e->relative;
			if (ahead >= ONLINE_FRAME_LOOKAHEAD) return false;
			p->core[side]->setKeys(p->core[side], online_frame_script_key(p->frame + ahead, side));
		}
		p->core[side]->step(p->core[side]);
		if (phase != TRANSFER_IDLE && p->cable.d.transferActive == TRANSFER_IDLE) ++p->transfers;
		trace_core(p, side, frame);
	}
	++e->round;
	e->at_check = true;
	return true;
}

static void finish_frame(OnlineReplayExecutor *e) {
	OnlineFramePair *p = e->pair;
	if (!e->battle) {
		unsigned count;
		const OnlineLinkInput *entry = online_link_entry_steps(&count);
		if (++p->entry_frame == entry[p->entry_step].frames) {
			p->entry_frame = 0;
			++p->entry_step;
			for (unsigned side = 0; side < 2; ++side) p->start_frame[side] = p->core[side]->frameCounter(p->core[side]);
		}
	}
	++p->frame;
	e->in_frame = e->at_check = false;
	e->round = 0;
}

bool online_replay_execute_frame(OnlineReplayExecutor *e, OnlineReplayStop stop, void *context) {
	OnlineFramePair *p = e->pair;
	if (!e->in_frame) begin_frame(e);
	uint64_t deadline = online_frame_now_ms() + 5000;
	for (;;) {
		/* A restored boundary resumes here, before any key setup or step. */
		if (e->at_check) {
			if (p->core[0]->frameCounter(p->core[0]) >= e->target[0] &&
				p->core[1]->frameCounter(p->core[1]) >= e->target[1]) { finish_frame(e); return true; }
			e->at_check = false;
		}
		if (e->round >= 4000000 || (!(e->round & 0x3FFFu) && online_frame_now_ms() >= deadline)) return false;
		if (!run_round(e)) return false;
		if (stop && stop(e, context)) return true;
	}
}
