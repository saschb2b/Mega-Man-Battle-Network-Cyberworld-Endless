#include "online_replay_snapshot.h"

#include <stdlib.h>
#include <string.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/gba/gba.h>

#include "online_frame_pair_internal.h"

typedef struct {
	int32_t next_event, event_diff, event_delay;
	unsigned event_priority;
	enum GBASIOMode mode;
	enum GBASIOMode sio_mode;
	bool normal_so, transfer_finished, scheduled, driver_active;
	uint16_t siocnt, rcnt;
	uint32_t keys;
	int32_t cpu_next_event;
	/* These channel structs contain values only, with no pointer wiring. */
	struct GBAudioSquareChannel square[2];
	struct GBAudioWaveChannel wave;
	struct GBAudioNoiseChannel noise;
	uint16_t io[SIZE_IO / sizeof(uint16_t)];
} NodeState;

struct OnlineReplaySnapshot {
	void *core[2], *flash[2];
	size_t core_bytes[2], flash_bytes[2];
	NodeState node[2];
	int attached, attached_multi, attached_normal;
	enum mLockstepPhase phase;
	int32_t transfer_cycles, credit[2];
	uint16_t multi_recv[4];
	uint32_t normal_recv[4];
	unsigned waiting, transfers;
	int running;
	unsigned traced[2], start_frame[2], frame, entry_step, entry_frame;
	unsigned target[2], relative, round;
	bool battle, in_frame, at_check;
};

void online_replay_snapshot_free(OnlineReplaySnapshot *s) {
	if (!s) return;
	for (unsigned i = 0; i < 2; ++i) { free(s->core[i]); free(s->flash[i]); }
	free(s);
}

static void take_node(OnlineReplaySnapshot *s, OnlineFramePair *p, unsigned side) {
	struct GBASIOLockstepNode *node = &p->node[side];
	struct GBA *gba = p->core[side]->board;
	s->node[side] = (NodeState){.next_event = node->nextEvent, .event_diff = node->eventDiff,
		.event_delay = mTimingUntil(&gba->timing, &node->event), .event_priority = node->event.priority,
		.mode = node->mode, .normal_so = node->normalSO, .transfer_finished = node->transferFinished,
		.sio_mode = gba->sio.mode,
		.scheduled = mTimingIsScheduled(&gba->timing, &node->event),
		.driver_active = gba->sio.activeDriver == &node->d, .siocnt = gba->sio.siocnt, .rcnt = gba->sio.rcnt,
		.keys = gba->keysActive, .cpu_next_event = gba->cpu->nextEvent,
		.square = {gba->audio.psg.ch1, gba->audio.psg.ch2}, .wave = gba->audio.psg.ch3, .noise = gba->audio.psg.ch4};
	memcpy(s->node[side].io, gba->memory.io, sizeof s->node[side].io);
}

static void take_bookkeeping(OnlineReplaySnapshot *s, OnlineReplayExecutor *e) {
	OnlineFramePair *p = e->pair;
	s->attached = p->cable.d.attached;
	s->phase = p->cable.d.transferActive;
	s->transfer_cycles = p->cable.d.transferCycles;
	s->attached_multi = p->cable.attachedMulti;
	s->attached_normal = p->cable.attachedNormal;
	memcpy(s->multi_recv, p->cable.multiRecv, sizeof s->multi_recv);
	memcpy(s->normal_recv, p->cable.normalRecv, sizeof s->normal_recv);
	memcpy(s->credit, p->credit, sizeof s->credit);
	s->waiting = p->waiting;
	s->transfers = p->transfers;
	s->running = p->running;
	memcpy(s->traced, p->traced, sizeof s->traced);
	memcpy(s->start_frame, p->start_frame, sizeof s->start_frame);
	s->frame = p->frame;
	s->entry_step = p->entry_step;
	s->entry_frame = p->entry_frame;
	memcpy(s->target, e->target, sizeof s->target);
	s->relative = e->relative;
	s->round = e->round;
	s->battle = e->battle;
	s->in_frame = e->in_frame;
	s->at_check = e->at_check;
}

OnlineReplaySnapshot *online_replay_snapshot_take(OnlineReplayExecutor *e) {
	OnlineFramePair *p = e->pair;
	OnlineReplaySnapshot *s = calloc(1, sizeof *s);
	if (!s) return NULL;
	for (unsigned i = 0; i < 2; ++i) {
		struct mCore *core = p->core[i];
		s->core_bytes[i] = core->stateSize(core);
		s->core[i] = calloc(1, s->core_bytes[i]);
		if (!s->core[i] || !core->saveState(core, s->core[i])) { online_replay_snapshot_free(s); return NULL; }
		s->flash_bytes[i] = core->savedataClone(core, &s->flash[i]);
		if (s->flash_bytes[i] && !s->flash[i]) { online_replay_snapshot_free(s); return NULL; }
		take_node(s, p, i);
	}
	take_bookkeeping(s, e);
	return s;
}

static void restore_bookkeeping(OnlineReplayExecutor *e, const OnlineReplaySnapshot *s) {
	OnlineFramePair *p = e->pair;
	memcpy(p->traced, s->traced, sizeof p->traced);
	memcpy(p->start_frame, s->start_frame, sizeof p->start_frame);
	p->frame = s->frame;
	p->entry_step = s->entry_step;
	p->entry_frame = s->entry_frame;
	memcpy(e->target, s->target, sizeof e->target);
	e->relative = s->relative;
	e->round = s->round;
	e->battle = s->battle;
	e->in_frame = s->in_frame;
	e->at_check = s->at_check;
}

static void restore_node(OnlineFramePair *p, const OnlineReplaySnapshot *s, unsigned side, OnlineReplayFault fault) {
	struct GBASIOLockstepNode *node = &p->node[side];
	struct GBA *gba = p->core[side]->board;
	const NodeState *saved = &s->node[side];
	mTimingDeschedule(&gba->timing, &node->event);
	/* Restore numeric state; all wiring refers to this live pair. The pinned
	 * mGBA driver's callback remains installed, never stored in the snapshot. */
	node->p = &p->cable;
	node->id = (int)side;
	node->d.p = &gba->sio;
	node->nextEvent = saved->next_event;
	node->eventDiff = saved->event_diff;
	node->mode = saved->mode;
	node->normalSO = saved->normal_so;
	node->transferFinished = saved->transfer_finished;
	node->event.context = node;
	node->event.priority = saved->event_priority;
	int32_t delay = saved->event_delay + (fault == REPLAY_BAD_EVENT && !side ? 1 : 0);
	/* Negative values mean already due, and must not be clamped to zero. */
	node->event.when = (uint32_t)mTimingCurrentTime(&gba->timing) + (uint32_t)delay;
	if (saved->scheduled) mTimingSchedule(&gba->timing, &node->event, delay);
	else node->event.next = NULL;
	gba->sio.siocnt = saved->siocnt;
	gba->sio.rcnt = saved->rcnt;
	gba->sio.mode = saved->sio_mode;
	gba->sio.drivers.multiplayer = &node->d;
	gba->sio.activeDriver = saved->driver_active ? &node->d : NULL;
}

static void restore_cable(OnlineFramePair *p, const OnlineReplaySnapshot *s, OnlineReplayFault fault) {
	p->cable.d.attached = s->attached;
	p->cable.d.transferActive = s->phase;
	p->cable.d.transferCycles = s->transfer_cycles;
	p->cable.attachedMulti = s->attached_multi;
	p->cable.attachedNormal = s->attached_normal;
	memcpy(p->cable.multiRecv, s->multi_recv, sizeof p->cable.multiRecv);
	memcpy(p->cable.normalRecv, s->normal_recv, sizeof p->cable.normalRecv);
	memcpy(p->credit, s->credit, sizeof p->credit);
	p->waiting = s->waiting;
	p->transfers = s->transfers;
	p->running = s->running;
	for (unsigned i = 0; i < 4; ++i) p->cable.players[i] = i < 2 ? &p->node[i] : NULL;
	for (unsigned i = 0; i < 2; ++i) restore_node(p, s, i, fault);
	if (fault == REPLAY_BAD_CREDIT) ++p->credit[1];
	if (fault == REPLAY_BAD_RECEIVE) p->cable.multiRecv[0] ^= 1;
}

bool online_replay_snapshot_restore(OnlineReplayExecutor *e, const OnlineReplaySnapshot *s, OnlineReplayFault fault) {
	if (!s || fault > REPLAY_BAD_EVENT) return false;
	OnlineFramePair *p = e->pair;
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *core = p->core[side];
		if (core->stateSize(core) != s->core_bytes[side]) return false;
		/* The core's backing save file was created in memory by pair_open. */
		if (fault != REPLAY_OMIT_FLASH && !core->savedataRestore(core, s->flash[side], s->flash_bytes[side], true)) return false;
		if (!core->loadState(core, s->core[side])) return false;
	}
	restore_bookkeeping(e, s);
	if (fault != REPLAY_OMIT_CABLE) restore_cable(p, s, fault);
	for (unsigned side = 0; side < 2; ++side) {
		struct GBA *gba = p->core[side]->board;
		/* mGBA's raw state omits the external controller latch. setKeys would
		 * test keypad IRQ again, so restore the captured latch directly.
		 * loadState/scheduling interrupts the CPU event countdown; restore its
		 * captured value only after rebuilding the complete timing queue. */
		gba->keysActive = s->node[side].keys;
		gba->cpu->nextEvent = s->node[side].cpu_next_event;
		/* GBAAudioDeserialize writes channel registers after deserialization,
		 * running the PSG again. Preserve the captured numeric channel state. */
		gba->audio.psg.ch1 = s->node[side].square[0];
		gba->audio.psg.ch2 = s->node[side].square[1];
		gba->audio.psg.ch3 = s->node[side].wave;
		gba->audio.psg.ch4 = s->node[side].noise;
		/* GBAIOSerialize skips some latches, including SIOMLT_SEND. The
		 * cable reads that live register when starting its next transfer. */
		memcpy(gba->memory.io, s->node[side].io, sizeof s->node[side].io);
	}
	return true;
}

size_t online_replay_snapshot_bytes(const OnlineReplaySnapshot *s) {
	if (!s) return 0;
	return sizeof *s + s->core_bytes[0] + s->core_bytes[1] + s->flash_bytes[0] + s->flash_bytes[1];
}
