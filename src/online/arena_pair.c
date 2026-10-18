/* The Arena's pair (arena_pair.h): two BN6 cores on mGBA's cable.
 *
 * The cable is mGBA's local lockstep, its scheduling in our hands as the
 * research harness had it (tools/online_link_bn6.c, docs/ONLINE_LINK.md):
 * the cores step an instruction at a time in turn, the first held while it
 * waits for its partner, each side's cycles credited to the other. A frame
 * of the pair ends once both cores have crossed into their next frame; a
 * core that crossed first runs on with the keys of its own next frame,
 * which is why a frame needs the keys of the three after it. */
#ifndef NDEBUG
/* (the lockstep structures as the Release mGBA every target links lays
 * them out: their debug fields are left out under NDEBUG) */
#define NDEBUG
#endif
#include "arena_pair.h"

#if CW_ARENA

#include <stdlib.h>
#include <string.h>

#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/sio/lockstep.h>
#include <mgba-util/vfs.h>

#include "emu.h"

/* instructions a frame of the pair may take, both cores' together: far
 * past a frame's (about 280,000 cycles each), short of a hang */
#define STEP_LIMIT 4000000u
#define STATE_FLAGS (SAVESTATE_SAVEDATA | SAVESTATE_RTC)

struct ArenaPair {
	struct GBASIOLockstep cable;
	struct GBASIOLockstepNode node[2];
	struct mCore *core[2];
	uint32_t video[2][ARENA_W * ARENA_H];
	int32_t credit[2];
	unsigned waiting, transfers;
	int running;
	unsigned start[2], relative;   /* each core's frame at the anchor; the pair's frames since */
	uint8_t *scratch;              /* a core's raw state, for the hash */
	size_t scratch_size;
};

struct mCore *arena_core_new(const uint8_t *rom, size_t size, uint32_t *video) {
	if (!rom || !size || size > EMU_ROM_SIZE) return NULL;
	emu_log_quiet();
	uint8_t *copy = malloc(EMU_ROM_SIZE);
	if (!copy) return NULL;
	memcpy(copy, rom, size);
	memset(copy + size, 0xFF, EMU_ROM_SIZE - size);
	struct VFile *vf = VFileMemChunk(copy, EMU_ROM_SIZE);
	free(copy);
	if (!vf) return NULL;
	struct mCore *c = mCoreFindVF(vf);
	if (!c || !c->init(c)) { vf->close(vf); return NULL; }
	mCoreInitConfig(c, NULL);
	c->setVideoBuffer(c, (color_t *)video, ARENA_W);
	if (!c->loadROM(c, vf)) { c->deinit(c); return NULL; }
	/* (the game's save prompts write here only: never a player's file) */
	c->loadSave(c, VFileMemChunk(NULL, 0));
	c->setAudioBufferSize(c, 1024);
	blip_set_rates(c->getAudioChannel(c, 0), c->frequency(c), emu_audio_out_rate());
	blip_set_rates(c->getAudioChannel(c, 1), c->frequency(c), emu_audio_out_rate());
	c->reset(c);
	return c;
}

void arena_core_free(struct mCore *core) {
	if (core) core->deinit(core);
}

uint8_t *arena_core_save(struct mCore *core, size_t *size) {
	*size = 0;
	struct VFile *vf = VFileMemChunk(NULL, 0);
	if (!vf) return NULL;
	uint8_t *out = NULL;
	ssize_t n = mCoreSaveStateNamed(core, vf, STATE_FLAGS) ? vf->size(vf) : 0;
	if (n > 0 && (out = malloc((size_t)n)) != NULL) {
		vf->seek(vf, 0, SEEK_SET);
		if (vf->read(vf, out, (size_t)n) == n) *size = (size_t)n;
		else { free(out); out = NULL; }
	}
	vf->close(vf);
	return out;
}

bool arena_core_load(struct mCore *core, const uint8_t *state, size_t size) {
	struct VFile *vf = VFileFromConstMemory(state, size);
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, STATE_FLAGS);
	vf->close(vf);
	return ok;
}

/* ---- the cable's scheduling, ours (mLockstep's callbacks) ---- */

static bool signal_peer(struct mLockstep *lock, unsigned mask) {
	ArenaPair *p = lock->context;
	if (p->running != 0) p->waiting &= ~mask;
	return true;
}
static bool wait_peer(struct mLockstep *lock, unsigned mask) {
	((ArenaPair *)lock->context)->waiting = mask;
	return true;
}
static void add_cycles(struct mLockstep *lock, int id, int32_t cycles) {
	((ArenaPair *)lock->context)->credit[id == 0 ? 1 : id] += cycles;
}
static int32_t use_cycles(struct mLockstep *lock, int id, int32_t cycles) {
	ArenaPair *p = lock->context;
	p->credit[id] -= cycles;
	return p->credit[id];
}
static int32_t unused_cycles(struct mLockstep *lock, int id) {
	return ((ArenaPair *)lock->context)->credit[id];
}
static void unload_peer(struct mLockstep *lock, int id) {
	((ArenaPair *)lock->context)->waiting &= ~(1u << (unsigned)id);
}

static bool attach(ArenaPair *p, unsigned side) {
	GBASIOLockstepNodeCreate(&p->node[side]);
	if (!GBASIOLockstepAttachNode(&p->cable, &p->node[side])) return false;
	GBASIOSetDriver(&((struct GBA *)p->core[side]->board)->sio, &p->node[side].d, SIO_MULTI);
	return true;
}

ArenaPair *arena_pair_open(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2]) {
	ArenaPair *p = calloc(1, sizeof *p);
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
	bool ok = true;
	for (unsigned side = 0; ok && side < 2; ++side) {
		p->core[side] = arena_core_new(rom, size, p->video[side]);
		ok = p->core[side] && arena_core_load(p->core[side], state[side], state_size[side]) && attach(p, side);
	}
	if (ok) {
		p->scratch_size = p->core[0]->stateSize(p->core[0]);
		ok = (p->scratch = malloc(p->scratch_size)) != NULL;
	}
	if (!ok) { arena_pair_close(p); return NULL; }
	arena_pair_anchor(p);
	return p;
}

void arena_pair_close(ArenaPair *p) {
	if (!p) return;
	for (unsigned side = 0; side < 2; ++side) {
		if (!p->core[side]) continue;
		GBASIOSetDriver(&((struct GBA *)p->core[side]->board)->sio, NULL, SIO_MULTI);
		if (p->node[side].p) GBASIOLockstepDetachNode(&p->cable, &p->node[side]);
		p->core[side]->deinit(p->core[side]);
	}
	free(p->scratch);
	free(p);
}

void arena_pair_anchor(ArenaPair *p) {
	for (unsigned side = 0; side < 2; ++side) p->start[side] = p->core[side]->frameCounter(p->core[side]);
	p->relative = 0;
}

static bool both_crossed(ArenaPair *p, const unsigned target[2]) {
	return p->core[0]->frameCounter(p->core[0]) >= target[0] && p->core[1]->frameCounter(p->core[1]) >= target[1];
}

bool arena_pair_step(ArenaPair *p, const uint16_t keys[ARENA_LOOKAHEAD][2], bool own_clock) {
	unsigned target[2];
	for (unsigned side = 0; side < 2; ++side) {
		target[side] = p->start[side] + p->relative + 1;
		p->core[side]->setKeys(p->core[side], keys[0][side]);
	}
	for (unsigned limit = 0; limit < STEP_LIMIT; ++limit) {
		for (unsigned side = 0; side < 2; ++side) {
			struct mCore *c = p->core[side];
			if (!side && p->waiting) continue;
			p->running = (int)side;
			enum mLockstepPhase phase = p->cable.d.transferActive;
			if (own_clock) {
				/* (a core past the pair's frame takes its own frame's keys;
				 * one as far ahead as the keys go waits for its partner, as
				 * it can once the cable is idle (the menus after a battle):
				 * the second core alone runs on where the first waits for it,
				 * on the last keys it has) */
				unsigned ahead = c->frameCounter(c) - p->start[side] - p->relative;
				if (ahead >= ARENA_LOOKAHEAD && !(side && p->waiting)) continue;
				c->setKeys(c, keys[ahead < ARENA_LOOKAHEAD ? ahead : ARENA_LOOKAHEAD - 1][side]);
			}
			c->step(c);
			if (phase != TRANSFER_IDLE && p->cable.d.transferActive == TRANSFER_IDLE) ++p->transfers;
		}
		if (both_crossed(p, target)) { ++p->relative; return true; }
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
	uint8_t bytes[4] = { (uint8_t)word, (uint8_t)(word >> 8), (uint8_t)(word >> 16), (uint8_t)(word >> 24) };
	hash_bytes(hash, bytes, sizeof bytes);
}

/* the cable's own state, which no core's holds */
static void hash_cable(ArenaPair *p, uint64_t *hash) {
	const uint32_t fields[] = { (uint32_t)p->cable.d.attached, (uint32_t)p->cable.d.transferActive,
		(uint32_t)p->cable.d.transferCycles, (uint32_t)p->cable.attachedMulti, (uint32_t)p->cable.attachedNormal,
		p->waiting, p->transfers, (uint32_t)p->running };
	for (size_t i = 0; i < sizeof fields / sizeof *fields; ++i) hash_word(hash, fields[i]);
	for (unsigned i = 0; i < 4; ++i) {
		hash_word(hash, p->cable.multiRecv[i]);
		hash_word(hash, p->cable.normalRecv[i]);
	}
	for (unsigned side = 0; side < 2; ++side) {
		struct GBASIOLockstepNode *n = &p->node[side];
		struct GBA *gba = p->core[side]->board;
		const uint32_t node[] = { (uint32_t)p->credit[side], (uint32_t)n->nextEvent, (uint32_t)n->eventDiff,
			(uint32_t)n->mode, n->normalSO, n->transferFinished, mTimingIsScheduled(&gba->timing, &n->event),
			(uint32_t)mTimingUntil(&gba->timing, &n->event) };
		for (size_t j = 0; j < sizeof node / sizeof *node; ++j) hash_word(hash, node[j]);
	}
}

uint64_t arena_pair_hash(ArenaPair *p) {
	uint64_t hash = UINT64_C(14695981039346656037);
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *c = p->core[side];
		memset(p->scratch, 0, p->scratch_size);
		if (c->stateSize(c) != p->scratch_size || !c->saveState(c, p->scratch)) return 0;
		hash_bytes(&hash, p->scratch, p->scratch_size);
		void *flash = NULL;
		size_t size = c->savedataClone(c, &flash);
		hash_word(&hash, (uint32_t)size);
		if (flash) hash_bytes(&hash, flash, size);
		free(flash);
	}
	hash_cable(p, &hash);
	return hash ? hash : 1;
}

struct mCore *arena_pair_core(ArenaPair *p, unsigned side) { return p->core[side & 1]; }
const uint32_t *arena_pair_video(const ArenaPair *p, unsigned side) { return p->video[side & 1]; }
unsigned arena_pair_transfers(const ArenaPair *p) { return p->transfers; }

void arena_pair_sound(ArenaPair *p, unsigned side) {
	emu_audio_open();
	emu_audio_from(p->core[side & 1]);
	emu_audio_drop_from(p->core[(side & 1) ^ 1]);
}

void arena_pair_mute(ArenaPair *p) {
	emu_audio_drop_from(p->core[0]);
	emu_audio_drop_from(p->core[1]);
}

#else   /* no Arena in this build (arena_pair.h, CW_ARENA) */

struct mCore *arena_core_new(const uint8_t *rom, size_t size, uint32_t *video) { (void)rom; (void)size; (void)video; return NULL; }
void arena_core_free(struct mCore *core) { (void)core; }
uint8_t *arena_core_save(struct mCore *core, size_t *size) { (void)core; *size = 0; return NULL; }
bool arena_core_load(struct mCore *core, const uint8_t *state, size_t size) { (void)core; (void)state; (void)size; return false; }
ArenaPair *arena_pair_open(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2]) {
	(void)rom; (void)size; (void)state; (void)state_size;
	return NULL;
}
void arena_pair_close(ArenaPair *pair) { (void)pair; }
void arena_pair_anchor(ArenaPair *pair) { (void)pair; }
bool arena_pair_step(ArenaPair *pair, const uint16_t keys[ARENA_LOOKAHEAD][2], bool own_clock) { (void)pair; (void)keys; (void)own_clock; return false; }
uint64_t arena_pair_hash(ArenaPair *pair) { (void)pair; return 0; }
struct mCore *arena_pair_core(ArenaPair *pair, unsigned side) { (void)pair; (void)side; return NULL; }
const uint32_t *arena_pair_video(const ArenaPair *pair, unsigned side) { (void)pair; (void)side; return NULL; }
unsigned arena_pair_transfers(const ArenaPair *pair) { (void)pair; return 0; }
void arena_pair_sound(ArenaPair *pair, unsigned side) { (void)pair; (void)side; }
void arena_pair_mute(ArenaPair *pair) { (void)pair; }

#endif
