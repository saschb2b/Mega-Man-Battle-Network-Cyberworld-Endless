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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/serialize.h>
#include <mgba/internal/gba/sio/lockstep.h>
#include <mgba-util/vfs.h>

#include "arena_souls.h"
#include "emu.h"
#include "hook.h"
#include "soulunite.h"
#include "xinstall_log.h"

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
	bool bn5;                      /* full rules: BN5's installs in both cores (arena_pair.h) */
	ArenaSoulCarried carried[2];   /* (full rules: the Souls carried across, arena_souls.h) */
};

struct mCore *arena_core_new(const uint8_t *rom, size_t size, uint32_t *video, bool bn5) {
	if (!rom || !size || size > EMU_ROM_SIZE) return NULL;
	emu_log_quiet();
	uint8_t *copy = malloc(EMU_ROM_SIZE);
	if (!copy) return NULL;
	memcpy(copy, rom, size);
	memset(copy + size, 0xFF, EMU_ROM_SIZE - size);
	if (bn5) xlog_apply(copy, EMU_ROM_SIZE);
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

/* full rules: BN5's hooks on the core's own table, after its state (a hook in RAM stands on what the state put there), and
 * its player's Souls for UNITE */
static bool installs(struct mCore *core, unsigned souls) {
	return xlog_hook_core(core) && soulunite_held_core(core, souls);
}

ArenaPair *arena_pair_open(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2], bool bn5,
	const unsigned souls[2]) {
	ArenaPair *p = calloc(1, sizeof *p);
	if (!p) return NULL;
	p->bn5 = bn5;
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
		p->core[side] = arena_core_new(rom, size, p->video[side], bn5);
		ok = p->core[side] && arena_core_load(p->core[side], state[side], state_size[side]) &&
			(!bn5 || installs(p->core[side], souls ? souls[side] : 0)) && attach(p, side);
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
		if (p->bn5) {
			soulunite_held_core(p->core[side], 0);
			hook_detach(p->core[side]);
		}
		p->core[side]->deinit(p->core[side]);
	}
	free(p->scratch);
	free(p);
}

void arena_pair_anchor(ArenaPair *p) {
	for (unsigned side = 0; side < 2; ++side) p->start[side] = p->core[side]->frameCounter(p->core[side]);
	p->relative = 0;
}

void arena_pair_begin(ArenaPair *p) {
	if (p->bn5) arena_souls_lists(p->core);
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
		if (both_crossed(p, target)) {
			++p->relative;
			if (p->bn5) arena_souls_carry(p->core, p->carried);
			return true;
		}
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
	/* (full rules: the installs both cores were made with, which their states do not hold) */
	if (p->bn5) {
		uint64_t made = xlog_hash();
		hash_word(&hash, (uint32_t)made);
		hash_word(&hash, (uint32_t)(made >> 32));
	}
	return hash ? hash : 1;
}

uint64_t arena_pair_record(const ArenaPair *p) { return p->bn5 ? xlog_hash() : 0; }

/* ---- the snapshot (arena_pair.h): what a rollback puts back ---- */

typedef struct {
	int32_t next_event, event_diff, until;
	int mode;
	bool normal_so, finished, scheduled;
} NodeSnap;

struct ArenaPairSnap {
	uint8_t *core[2];
	size_t core_size;
	uint8_t *kept[2];      /* full rules: what BN5's hooks keep each core between calls (hook_keep_slots) */
	size_t kept_size;
	/* the cable's own (mLockstep and GBASIOLockstep), which no core's state holds */
	int attached, multi, normal;
	enum mLockstepPhase active;
	int32_t transfer_cycles;
	uint16_t multi_recv[MAX_GBAS];
	uint32_t normal_recv[MAX_GBAS];
	NodeSnap node[2];
	/* the pair's scheduling, frame clock and Souls carried */
	int32_t credit[2];
	unsigned waiting, transfers, start[2], relative;
	int running;
	ArenaSoulCarried carried[2];
};

ArenaPairSnap *arena_pair_snap_new(ArenaPair *p) {
	size_t core_size = p->core[0]->stateSize(p->core[0]), kept_size = p->bn5 ? hook_kept_size() : 0;
	ArenaPairSnap *s = calloc(1, sizeof *s + 2 * (core_size + kept_size));
	if (!s) return NULL;
	s->core_size = core_size;
	s->kept_size = kept_size;
	s->core[0] = (uint8_t *)(s + 1);
	s->core[1] = s->core[0] + core_size;
	s->kept[0] = s->core[1] + core_size;
	s->kept[1] = s->kept[0] + kept_size;
	return s;
}

void arena_pair_snap_free(ArenaPairSnap *s) { free(s); }
size_t arena_pair_snap_bytes(const ArenaPairSnap *s) {
	return 2 * (s->core_size + s->kept_size) + sizeof *s - offsetof(ArenaPairSnap, attached);
}
size_t arena_pair_snap_core_bytes(const ArenaPairSnap *s) { return s->core_size; }

bool arena_pair_save(ArenaPair *p, ArenaPairSnap *s) {
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *c = p->core[side];
		if (c->stateSize(c) != s->core_size || !c->saveState(c, s->core[side])) return false;
		struct GBASIOLockstepNode *n = &p->node[side];
		struct mTiming *timing = &((struct GBA *)c->board)->timing;
		bool scheduled = mTimingIsScheduled(timing, &n->event);
		s->node[side] = (NodeSnap){ n->nextEvent, n->eventDiff, scheduled ? mTimingUntil(timing, &n->event) : 0, (int)n->mode,
			n->normalSO, n->transferFinished, scheduled };
		if (s->kept_size) hook_kept_save(hook_slot_of(c), s->kept[side]);
	}
	s->attached = p->cable.d.attached;
	s->active = p->cable.d.transferActive;
	s->transfer_cycles = p->cable.d.transferCycles;
	s->multi = p->cable.attachedMulti;
	s->normal = p->cable.attachedNormal;
	memcpy(s->multi_recv, p->cable.multiRecv, sizeof s->multi_recv);
	memcpy(s->normal_recv, p->cable.normalRecv, sizeof s->normal_recv);
	memcpy(s->credit, p->credit, sizeof s->credit);
	s->waiting = p->waiting;
	s->transfers = p->transfers;
	s->running = p->running;
	memcpy(s->start, p->start, sizeof s->start);
	s->relative = p->relative;
	memcpy(s->carried, p->carried, sizeof s->carried);
	return true;
}

/* (a core's state loaded clears its timing, the cable's event with it, and
 * writes its SIO registers through the cable, which may start a transfer:
 * so both cores first, then the cable's own and its events put back) */
bool arena_pair_load(ArenaPair *p, const ArenaPairSnap *s) {
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *c = p->core[side];
		if (c->stateSize(c) != s->core_size || !c->loadState(c, s->core[side])) return false;
		if (s->kept_size) hook_kept_load(hook_slot_of(c), s->kept[side]);
	}
	p->cable.d.attached = s->attached;
	p->cable.d.transferActive = s->active;
	p->cable.d.transferCycles = s->transfer_cycles;
	p->cable.attachedMulti = s->multi;
	p->cable.attachedNormal = s->normal;
	memcpy(p->cable.multiRecv, s->multi_recv, sizeof s->multi_recv);
	memcpy(p->cable.normalRecv, s->normal_recv, sizeof s->normal_recv);
	for (unsigned side = 0; side < 2; ++side) {
		struct GBASIOLockstepNode *n = &p->node[side];
		struct mTiming *timing = &((struct GBA *)p->core[side]->board)->timing;
		const NodeSnap *ns = &s->node[side];
		n->nextEvent = ns->next_event;
		n->eventDiff = ns->event_diff;
		n->mode = (enum GBASIOMode)ns->mode;
		n->normalSO = ns->normal_so;
		n->transferFinished = ns->finished;
		if (mTimingIsScheduled(timing, &n->event)) mTimingDeschedule(timing, &n->event);
		if (ns->scheduled) mTimingSchedule(timing, &n->event, ns->until);
	}
	memcpy(p->credit, s->credit, sizeof p->credit);
	p->waiting = s->waiting;
	p->transfers = s->transfers;
	p->running = s->running;
	memcpy(p->start, s->start, sizeof p->start);
	p->relative = s->relative;
	memcpy(p->carried, s->carried, sizeof p->carried);
	return true;
}

/* a byte of the sound's in mGBA's state: the PSG's and the FIFOs' fields and
 * the samples rendered, which its lazy PSG leaves otherwise after a load
 * than run straight (docs/ONLINE_ARENA.md, Rollback) */
static bool sound_byte(size_t i) {
	size_t audio = offsetof(struct GBASerializedState, audio), samples = offsetof(struct GBASerializedState, samples);
	size_t rendered = offsetof(struct GBASerializedState, currentSamples) + sizeof ((struct GBASerializedState *)0)->currentSamples;
	return (i >= audio && i < audio + sizeof ((struct GBASerializedState *)0)->audio) || (i >= samples && i < rendered);
}

unsigned arena_pair_snap_diff(const ArenaPairSnap *a, const ArenaPairSnap *b, unsigned frame, bool say) {
	unsigned game = 0;
	for (unsigned side = 0; side < 2; ++side) {
		/* (each stretch of bytes apart, as offsets in mGBA's state: its serialize.h names them) */
		const uint8_t *x = a->core[side], *y = b->core[side];
		for (size_t i = 0; i < a->core_size; ++i) {
			if (x[i] == y[i]) continue;
			size_t end = i;
			while (end + 2 < a->core_size && (x[end + 1] != y[end + 1] || x[end + 2] != y[end + 2])) ++end;
			bool sound = sound_byte(i) && sound_byte(end);
			if (!sound && game++ < 12 && say)
				printf("arena: frame %u: core %u's state apart at 0x%05zx-0x%05zx\n", frame, side, i, end);
			i = end;
		}
	}
	size_t own = offsetof(ArenaPairSnap, attached);
	if (memcmp((const uint8_t *)a + own, (const uint8_t *)b + own, sizeof *a - own)) {
		if (say) printf("arena: frame %u: the cable's or the pair's own apart\n", frame);
		++game;
	}
	for (unsigned side = 0; side < 2; ++side)
		if (a->kept_size && memcmp(a->kept[side], b->kept[side], a->kept_size)) {
			if (say) printf("arena: frame %u: what core %u's hooks keep apart\n", frame, side);
			++game;
		}
	return game;
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

struct mCore *arena_core_new(const uint8_t *rom, size_t size, uint32_t *video, bool bn5) {
	(void)rom; (void)size; (void)video; (void)bn5;
	return NULL;
}
void arena_core_free(struct mCore *core) { (void)core; }
uint8_t *arena_core_save(struct mCore *core, size_t *size) { (void)core; *size = 0; return NULL; }
bool arena_core_load(struct mCore *core, const uint8_t *state, size_t size) { (void)core; (void)state; (void)size; return false; }
ArenaPair *arena_pair_open(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2], bool bn5,
	const unsigned souls[2]) {
	(void)rom; (void)size; (void)state; (void)state_size; (void)bn5; (void)souls;
	return NULL;
}
void arena_pair_close(ArenaPair *pair) { (void)pair; }
void arena_pair_anchor(ArenaPair *pair) { (void)pair; }
void arena_pair_begin(ArenaPair *pair) { (void)pair; }
bool arena_pair_step(ArenaPair *pair, const uint16_t keys[ARENA_LOOKAHEAD][2], bool own_clock) { (void)pair; (void)keys; (void)own_clock; return false; }
uint64_t arena_pair_hash(ArenaPair *pair) { (void)pair; return 0; }
uint64_t arena_pair_record(const ArenaPair *pair) { (void)pair; return 0; }
struct mCore *arena_pair_core(ArenaPair *pair, unsigned side) { (void)pair; (void)side; return NULL; }
const uint32_t *arena_pair_video(const ArenaPair *pair, unsigned side) { (void)pair; (void)side; return NULL; }
unsigned arena_pair_transfers(const ArenaPair *pair) { (void)pair; return 0; }
void arena_pair_sound(ArenaPair *pair, unsigned side) { (void)pair; (void)side; }
void arena_pair_mute(ArenaPair *pair) { (void)pair; }
ArenaPairSnap *arena_pair_snap_new(ArenaPair *pair) { (void)pair; return NULL; }
void arena_pair_snap_free(ArenaPairSnap *snap) { (void)snap; }
size_t arena_pair_snap_bytes(const ArenaPairSnap *snap) { (void)snap; return 0; }
size_t arena_pair_snap_core_bytes(const ArenaPairSnap *snap) { (void)snap; return 0; }
bool arena_pair_save(ArenaPair *pair, ArenaPairSnap *snap) { (void)pair; (void)snap; return false; }
bool arena_pair_load(ArenaPair *pair, const ArenaPairSnap *snap) { (void)pair; (void)snap; return false; }
unsigned arena_pair_snap_diff(const ArenaPairSnap *a, const ArenaPairSnap *b, unsigned frame, bool say) { (void)a; (void)b; (void)frame; (void)say; return 0; }

#endif
