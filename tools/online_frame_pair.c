#include "online_frame_pair.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/blip_buf.h>
#include <mgba/core/config.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/sio/lockstep.h>
#include <mgba-util/vfs.h>

#include "online_frame_wire.h"
#include "online_link_script.h"

struct OnlineFramePair {
	struct GBASIOLockstep cable;
	struct GBASIOLockstepNode node[2];
	struct mCore *core[2];
	color_t video[2][240 * 160];
	uint8_t *state[2];
	size_t state_bytes[2];
	int32_t credit[2];
	unsigned waiting, transfers;
	int running;
	unsigned traced[2];
	unsigned start_frame[2], frame;
	unsigned entry_step, entry_frame;
	FILE *trace;
};

static bool signal_peer(struct mLockstep *lock, unsigned mask) {
	OnlineFramePair *p = lock->context;
	if (p->running != 0) p->waiting &= ~mask;
	return true;
}
static bool wait_peer(struct mLockstep *lock, unsigned mask) {
	((OnlineFramePair *)lock->context)->waiting = mask;
	return true;
}
static void add_cycles(struct mLockstep *lock, int id, int32_t cycles) {
	((OnlineFramePair *)lock->context)->credit[id == 0 ? 1 : id] += cycles;
}
static int32_t use_cycles(struct mLockstep *lock, int id, int32_t cycles) {
	OnlineFramePair *p = lock->context;
	p->credit[id] -= cycles;
	return p->credit[id];
}
static int32_t unused_cycles(struct mLockstep *lock, int id) {
	return ((OnlineFramePair *)lock->context)->credit[id];
}
static void unload_peer(struct mLockstep *lock, int id) {
	((OnlineFramePair *)lock->context)->waiting &= ~(1u << (unsigned)id);
}
static void quiet_log(struct mLogger *logger, int category, enum mLogLevel level, const char *format, va_list args) {
	(void)logger; (void)category; (void)level; (void)format; (void)args;
}
static struct mLogger quiet = { .log = quiet_log };

static bool open_core(OnlineFramePair *p, unsigned side, const char *rom, const char *state) {
	struct mCore *c = mCoreFind(rom);
	if (!c || !c->init(c)) return false;
	p->core[side] = c;
	mCoreInitConfig(c, NULL);
	c->setVideoBuffer(c, p->video[side], 240);
	if (!mCoreLoadFile(c, rom)) return false;
	/* The game's save prompt writes here only, never to a player's file. */
	c->loadSave(c, VFileMemChunk(NULL, 0));
	c->setAudioBufferSize(c, 1024);
	blip_set_rates(c->getAudioChannel(c, 0), c->frequency(c), 48000);
	blip_set_rates(c->getAudioChannel(c, 1), c->frequency(c), 48000);
	c->reset(c);
	struct VFile *vf = VFileOpen(state, O_RDONLY);
	bool ok = vf && mCoreLoadStateNamed(c, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	if (vf) vf->close(vf);
	if (!ok) return false;
	GBASIOLockstepNodeCreate(&p->node[side]);
	if (!GBASIOLockstepAttachNode(&p->cable, &p->node[side])) return false;
	GBASIOSetDriver(&((struct GBA *)c->board)->sio, &p->node[side].d, SIO_MULTI);
	p->state_bytes[side] = c->stateSize(c);
	p->state[side] = calloc(1, p->state_bytes[side]);
	p->start_frame[side] = c->frameCounter(c);
	return p->state[side] != NULL;
}

OnlineFramePair *online_frame_pair_open(const char *rom, const char *state, const char *prefix) {
	mLogSetDefaultLogger(&quiet);
	OnlineFramePair *p = calloc(1, sizeof *p);
	if (!p) return NULL;
	mLockstepInit(&p->cable.d);
	GBASIOLockstepInit(&p->cable);
	p->cable.d.context = p;
	p->cable.d.signal = signal_peer;
	p->cable.d.wait = wait_peer;
	p->cable.d.addCycles = add_cycles;
	p->cable.d.useCycles = use_cycles;
	p->cable.d.unusedCycles = unused_cycles;
	p->cable.d.unload = unload_peer;
	char path[1024];
	if (snprintf(path, sizeof path, "%s.csv", prefix) < (int)sizeof path) p->trace = fopen(path, "w");
	if (p->trace) fputs("core,frame,battle_timer,result,hp0,hp1,positions,menu,battle_state\n", p->trace);
	if (p->trace && open_core(p, 0, rom, state) && open_core(p, 1, rom, state)) return p;
	online_frame_pair_close(p);
	return NULL;
}

void online_frame_pair_close(OnlineFramePair *p) {
	if (!p) return;
	if (p->trace) fclose(p->trace);
	for (unsigned side = 0; side < 2; ++side) {
		if (p->core[side]) {
			GBASIOSetDriver(&((struct GBA *)p->core[side]->board)->sio, NULL, SIO_MULTI);
			if (p->node[side].p) GBASIOLockstepDetachNode(&p->cable, &p->node[side]);
			p->core[side]->deinit(p->core[side]);
		}
		free(p->state[side]);
	}
	free(p);
}

static void trace_frame(OnlineFramePair *p, unsigned side, unsigned frame) {
	if (frame == p->traced[side]) return;
	uint32_t sample[8];
	online_link_battle_sample(p->core[side], side, sample, NULL);
	fprintf(p->trace, "%u,%u,%u,%u,%u,%u,%u,%u,%u\n", side, frame, sample[2], sample[0],
		sample[3] & 65535, sample[3] >> 16, sample[4], sample[6], sample[7]);
	p->traced[side] = frame;
}

static void finish_frame(OnlineFramePair *p) {
	if (p->frame < online_link_entry_frames()) {
		unsigned count;
		const OnlineLinkInput *entry = online_link_entry_steps(&count);
		if (++p->entry_frame == entry[p->entry_step].frames) {
			p->entry_frame = 0;
			++p->entry_step;
			for (unsigned side = 0; side < 2; ++side) p->start_frame[side] = p->core[side]->frameCounter(p->core[side]);
		}
	}
	++p->frame;
}

bool online_frame_pair_step(OnlineFramePair *p, const uint16_t keys[ONLINE_FRAME_LOOKAHEAD][2]) {
	unsigned target[2];
	bool battle = p->frame >= online_link_entry_frames();
	unsigned relative = battle ? p->frame - online_link_entry_frames() : p->entry_frame;
	for (unsigned i = 0; i < 2; ++i) {
		target[i] = p->start_frame[i] + relative + 1;
		p->core[i]->setKeys(p->core[i], keys[0][i]);
	}
	uint64_t deadline = online_frame_now_ms() + 5000;
	for (unsigned limit = 0; limit < 4000000; ++limit) {
		if (!(limit & 0x3FFFu) && online_frame_now_ms() >= deadline) return false;
		for (unsigned side = 0; side < 2; ++side) {
			if (!side && p->waiting) continue;
			p->running = (int)side;
			enum mLockstepPhase phase = p->cable.d.transferActive;
			unsigned frame = p->core[side]->frameCounter(p->core[side]);
			/* Match the original oracle's per-core frame sampling: one core
			 * can cross VBlank while its cable peer is still finishing a frame.
			 * The next delivered frame is buffered before either core runs. */
			if (battle) {
				unsigned ahead = frame - p->start_frame[side] - relative;
				if (ahead >= ONLINE_FRAME_LOOKAHEAD) {
					fprintf(stderr, "cable frame lead: side=%u epoch=%u frame=%u target=%u\n", side, p->frame, frame, target[side]);
					return false;
				}
				p->core[side]->setKeys(p->core[side], keys[ahead][side]);
			}
			p->core[side]->step(p->core[side]);
			if (phase != TRANSFER_IDLE && p->cable.d.transferActive == TRANSFER_IDLE) ++p->transfers;
			trace_frame(p, side, frame);
		}
		if (p->core[0]->frameCounter(p->core[0]) >= target[0] &&
			p->core[1]->frameCounter(p->core[1]) >= target[1]) { finish_frame(p); return true; }
	}
	return false;
}

static void hash_bytes(uint64_t *hash, const void *data, size_t size) {
	const uint8_t *bytes = data;
	for (size_t i = 0; i < size; ++i) {
		*hash ^= bytes[i];
		*hash *= UINT64_C(1099511628211);
	}
}

static void hash_word(uint64_t *hash, uint32_t word) {
	uint8_t bytes[4];
	for (unsigned i = 0; i < 4; ++i) bytes[i] = (uint8_t)(word >> (i * 8));
	hash_bytes(hash, bytes, sizeof bytes);
}

static void hash_cable(OnlineFramePair *p, uint64_t *hash) {
	const uint32_t fields[] = {(uint32_t)p->cable.d.attached, (uint32_t)p->cable.d.transferActive,
		(uint32_t)p->cable.d.transferCycles, (uint32_t)p->cable.attachedMulti,
		(uint32_t)p->cable.attachedNormal, p->waiting, p->transfers, (uint32_t)p->running};
	for (unsigned i = 0; i < sizeof fields / sizeof *fields; ++i) hash_word(hash, fields[i]);
	for (unsigned i = 0; i < 4; ++i) {
		hash_word(hash, p->cable.multiRecv[i]);
		hash_word(hash, p->cable.normalRecv[i]);
	}
	for (unsigned i = 0; i < 2; ++i) {
		struct GBASIOLockstepNode *n = &p->node[i];
		struct GBA *gba = p->core[i]->board;
		const uint32_t node[] = {(uint32_t)p->credit[i], (uint32_t)n->nextEvent, (uint32_t)n->eventDiff,
			(uint32_t)n->mode, n->normalSO, n->transferFinished, n->d.p->siocnt, n->d.p->rcnt,
			mTimingIsScheduled(&gba->timing, &n->event), (uint32_t)mTimingUntil(&gba->timing, &n->event)};
		for (unsigned j = 0; j < sizeof node / sizeof *node; ++j) hash_word(hash, node[j]);
	}
}

bool online_frame_pair_sample(OnlineFramePair *p, uint32_t samples[2][8], uint64_t *hash) {
	*hash = UINT64_C(14695981039346656037);
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *c = p->core[side];
		online_link_battle_sample(c, side, samples[side], NULL);
		memset(p->state[side], 0, p->state_bytes[side]);
		if (!c->saveState(c, p->state[side])) return false;
		hash_bytes(hash, p->state[side], p->state_bytes[side]);
		void *flash = NULL;
		size_t size = c->savedataClone(c, &flash);
		if (size && !flash) return false;
		hash_word(hash, (uint32_t)size);
		hash_bytes(hash, flash, size);
		free(flash);
	}
	hash_cable(p, hash);
	return true;
}

bool online_frame_pair_shot(OnlineFramePair *p, const char *prefix, unsigned frame) {
	for (unsigned side = 0; side < 2; ++side) {
		char path[1024];
		if (snprintf(path, sizeof path, "%s-f%u-side%u.ppm", prefix, frame, side) >= (int)sizeof path) return false;
		FILE *file = fopen(path, "wb");
		if (!file) return false;
		fprintf(file, "P6\n240 160\n255\n");
		bool ok = true;
		for (unsigned i = 0; i < 240 * 160; ++i) {
			uint32_t pixel = p->video[side][i];
			uint8_t rgb[3] = {pixel & 255, (pixel >> 8) & 255, (pixel >> 16) & 255};
			if (fwrite(rgb, 1, sizeof rgb, file) != sizeof rgb) ok = false;
		}
		if (fclose(file) || !ok) return false;
	}
	return true;
}

unsigned online_frame_pair_transfers(const OnlineFramePair *p) { return p->transfers; }

unsigned online_frame_script_key(unsigned frame, unsigned side) {
	unsigned count;
	const OnlineLinkInput *steps;
	if (frame < online_link_entry_frames()) steps = online_link_entry_steps(&count);
	else { frame -= online_link_entry_frames(); steps = online_link_battle_steps(&count); }
	for (unsigned i = 0; i < count; ++i) {
		if (frame < steps[i].frames) return steps[i].keys[side];
		frame -= steps[i].frames;
	}
	return 0;
}
