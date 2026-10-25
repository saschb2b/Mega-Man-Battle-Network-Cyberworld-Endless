/* The Arena's pair (arena_pair.h): two BN6 cores on mGBA's cable.
 *
 * The cable is mGBA's local lockstep, its scheduling in our hands as mGBA's
 * own frontend has it with a thread a core (issue #128, docs/online/arena/rollback/overview.md,
 * Rollback): each core runs whole run loops (its instructions up to its next
 * event), the first posting the cycles it ran to the second, which runs
 * until it has used them and then sleeps until the first posts more; the
 * first waits while a transfer needs the second. So the two keep within a
 * few thousand cycles of each other (the research harness, docs/
 * docs/online/link.md, stepped them an instruction at a time in turn, and a
 * core could run two frames ahead of its partner). Both halt at BN6's wait
 * for a frame as the game's own core does (idle.h), which skips the cycles
 * the game would have spun through. A frame of the pair ends once both
 * cores have crossed into their next frame; a core that crossed first runs
 * on with the keys of its own next frame, which is why a frame needs the
 * keys of the one after it. */
#ifndef NDEBUG
/* (the lockstep structures as the Release mGBA every target links lays
 * them out: their debug fields are left out under NDEBUG) */
#define NDEBUG
#endif
#include "arena_pair.h"

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
#include "arena_team.h"
#include "arena_seats.h"
#include "arena_team_custom.h"
#include "devtools.h"
#include "emu.h"
#include "hook.h"
#include "idle.h"
#include "soulunite.h"
#include "xinstall_log.h"

/* run loops a frame of the pair may take, both cores' together: far past
 * a frame's (2,514 at most through a way in and a battle), short of a hang */
#define STEP_LIMIT 100000u
#define STATE_FLAGS (SAVESTATE_SAVEDATA | SAVESTATE_RTC)
/* the sound's fields in mGBA's state: the PSG's and the FIFOs', and the samples rendered, which its lazy PSG leaves
 * otherwise after a load than run straight (docs/online/arena/rollback/overview.md) */
#define SOUND_AT    offsetof(struct GBASerializedState, audio)
#define SOUND_END   (SOUND_AT + sizeof ((struct GBASerializedState *)0)->audio)
#define SAMPLES_AT  offsetof(struct GBASerializedState, samples)
#define SAMPLES_END (offsetof(struct GBASerializedState, currentSamples) + sizeof ((struct GBASerializedState *)0)->currentSamples)

struct ArenaPair {
	struct GBASIOLockstep cable;
	struct GBASIOLockstepNode node[2];
	struct mCore *core[2];
	uint32_t video[2][ARENA_W * ARENA_H];
	int32_t credit[2];
	unsigned waiting, asleep, transfers;   /* the first waits for these; these sleep, their cycles used */
	int running;
	unsigned drawn;                /* the sides whose picture is drawn (arena_pair_draw) */
	unsigned start[2], relative;   /* each core's frame at the anchor; the pair's frames since */
	uint8_t *scratch;              /* a core's raw state, for the hash */
	size_t scratch_size;
	ArenaSoulCarried carried[2];   /* (the Souls carried across, arena_souls.h) */
	/* --dev arena2v2 (arena_team.h): each side's partner's keys for the
	 * pair's frame and the one after it, and the frame of each core whose
	 * partner's keys were last sampled */
	bool team;
	uint16_t partner[ARENA_LOOKAHEAD][2];
	unsigned sampled[2];
};

/* A core's ROM copy: its bytes, BN5's installed in them where `bn5` */
static uint8_t *rom_copy(const uint8_t *rom, size_t size, bool bn5) {
	uint8_t *copy = malloc(EMU_ROM_SIZE);
	if (!copy) return NULL;
	memcpy(copy, rom, size);
	memset(copy + size, 0xFF, EMU_ROM_SIZE - size);
	if (bn5) xlog_apply(copy, EMU_ROM_SIZE);
	return copy;
}

#ifdef __3DS__
/* (on the 3DS the core maps its ROM as it is and patches it in place,
 * mGBA's FIXED_ROM_BUFFER (emu.c): the Arena's cores, whose ROMs are the
 * same bytes with the same hooks at the same places, share one copy, 16 MB
 * of a 96 MB heap where two would be 32 (docs/online/arena/rollback/new-3ds.md, The New
 * 3DS). The boot core takes it first and lets it go before the pair's are
 * made, so its own hooks and writes leave with it.) */
static struct {
	uint8_t *bytes;
	const uint8_t *from;
	bool bn5;
	unsigned users;
	struct mCore *core[HOOK_CORES];
} shared;

static struct VFile *rom_file(const uint8_t *rom, size_t size, bool bn5) {
	if (!shared.users) {
		free(shared.bytes);
		if (!(shared.bytes = rom_copy(rom, size, bn5))) return NULL;
		shared.from = rom;
		shared.bn5 = bn5;
	} else if (shared.from != rom || shared.bn5 != bn5) return NULL;
	return VFileFromMemory(shared.bytes, EMU_ROM_SIZE);
}

/* a core that maps the copy, or one that no longer does: the copy let go
 * with its last (core NULL: one never made) */
static void rom_user(struct mCore *core, bool joins) {
	for (unsigned i = 0; core && i < HOOK_CORES; ++i) {
		if (shared.core[i] != (joins ? NULL : core)) continue;
		shared.core[i] = joins ? core : NULL;
		shared.users += joins ? 1 : -1u;
		break;
	}
	if (!shared.users) { free(shared.bytes); shared.bytes = NULL; }
}
#else
static struct VFile *rom_file(const uint8_t *rom, size_t size, bool bn5) {
	uint8_t *copy = rom_copy(rom, size, bn5);
	if (!copy) return NULL;
	struct VFile *vf = VFileMemChunk(copy, EMU_ROM_SIZE);
	free(copy);
	return vf;
}
static void rom_user(struct mCore *core, bool joins) { (void)core; (void)joins; }
#endif

struct mCore *arena_core_new(const uint8_t *rom, size_t size, uint32_t *video, bool bn5) {
	if (!rom || !size || size > EMU_ROM_SIZE) return NULL;
	emu_log_quiet();
	struct VFile *vf = rom_file(rom, size, bn5);
	if (!vf) { rom_user(NULL, false); return NULL; }
	struct mCore *c = mCoreFindVF(vf);
	if (!c || !c->init(c)) { vf->close(vf); rom_user(NULL, false); return NULL; }
	rom_user(c, true);
	mCoreInitConfig(c, NULL);
	c->setVideoBuffer(c, (color_t *)video, ARENA_W);
	if (!c->loadROM(c, vf)) { arena_core_free(c); return NULL; }
	/* (the game's save prompts write here only: never a player's file) */
	c->loadSave(c, VFileMemChunk(NULL, 0));
	c->setAudioBufferSize(c, 1024);
	blip_set_rates(c->getAudioChannel(c, 0), c->frequency(c), emu_audio_out_rate());
	blip_set_rates(c->getAudioChannel(c, 1), c->frequency(c), emu_audio_out_rate());
	c->reset(c);
	return c;
}

void arena_core_free(struct mCore *core) {
	if (!core) return;
	core->deinit(core);
	rom_user(core, false);
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
/* (the first posting cycles wakes the second, which sleeps once it has
 * used them: mGBA's own frontend's way, src/platform/qt/MultiplayerController.cpp) */
static void add_cycles(struct mLockstep *lock, int id, int32_t cycles) {
	ArenaPair *p = lock->context;
	p->credit[id == 0 ? 1 : id] += cycles;
	if (id == 0) p->asleep &= ~2u;
}
static int32_t use_cycles(struct mLockstep *lock, int id, int32_t cycles) {
	ArenaPair *p = lock->context;
	p->credit[id] -= cycles;
	if (p->credit[id] <= 0) p->asleep |= 1u << (unsigned)id;
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
 * its player's Souls for UNITE; --dev arena2v2's hooks (arena_team.h) */
static bool installs(struct mCore *core, unsigned side, unsigned souls) {
	return xlog_hook_core(core) && soulunite_held_core(core, souls) && (!arena_seats_on() || arena_team_hooks(core, side));
}

/* (plain: no hook and no install's hooks on its cores, no idle halt: arena_pair_open_plain) */
static ArenaPair *open_pair(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2], const unsigned souls[2],
	bool plain) {
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
		p->core[side] = arena_core_new(rom, size, p->video[side], true);
		ok = p->core[side] && arena_core_load(p->core[side], state[side], state_size[side]) &&
			(plain || (installs(p->core[side], side, souls ? souls[side] : 0) && idle_on(p->core[side]))) && attach(p, side);
	}
	if (ok) {
		p->scratch_size = p->core[0]->stateSize(p->core[0]);
		ok = (p->scratch = malloc(p->scratch_size)) != NULL;
	}
	if (!ok) { arena_pair_close(p); return NULL; }
	p->drawn = 3;
	p->team = !plain && arena_seats_on();
	arena_pair_anchor(p);
	return p;
}

ArenaPair *arena_pair_open(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2], const unsigned souls[2]) {
	return open_pair(rom, size, state, state_size, souls, false);
}

ArenaPair *arena_pair_open_plain(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2]) {
	return open_pair(rom, size, state, state_size, NULL, true);
}

ArenaPair *arena_pair_open_screen(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2], unsigned side,
	unsigned souls) {
	ArenaPair *p = open_pair(rom, size, state, state_size, NULL, true);
	struct mCore *c = p ? p->core[side & 1u] : NULL;
	if (c && !(xlog_hook_core(c) && soulunite_held_core(c, souls))) {
		arena_pair_close(p);
		return NULL;
	}
	return p;
}

void arena_pair_draw(ArenaPair *p, unsigned sides) { p->drawn = sides & 3u; }

void arena_pair_close(ArenaPair *p) {
	if (!p) return;
	for (unsigned side = 0; side < 2; ++side) {
		if (!p->core[side]) continue;
		GBASIOSetDriver(&((struct GBA *)p->core[side]->board)->sio, NULL, SIO_MULTI);
		if (p->node[side].p) GBASIOLockstepDetachNode(&p->cable, &p->node[side]);
		soulunite_held_core(p->core[side], 0);
		hook_detach(p->core[side]);
		arena_core_free(p->core[side]);
	}
	free(p->scratch);
	free(p);
}

void arena_pair_anchor(ArenaPair *p) {
	for (unsigned side = 0; side < 2; ++side) p->start[side] = p->core[side]->frameCounter(p->core[side]);
	p->relative = 0;
}

void arena_pair_begin(ArenaPair *p) {
	arena_souls_lists(p->core);
}

static bool both_crossed(ArenaPair *p, const unsigned target[2]) {
	return p->core[0]->frameCounter(p->core[0]) >= target[0] && p->core[1]->frameCounter(p->core[1]) >= target[1];
}

/* which core runs next (-1: neither, the cable stuck): the second while
 * it has the first's cycles to use, then the first unless it waits for
 * the second; a core as far ahead as the keys go waits for its partner,
 * but the second runs on where the first waits for it, on the last keys
 * it has */
static int next_side(const ArenaPair *p, const unsigned ahead[2], bool own_clock) {
	bool held[2] = { own_clock && ahead[0] >= ARENA_LOOKAHEAD, own_clock && ahead[1] >= ARENA_LOOKAHEAD && !p->waiting };
	if (!(p->asleep & 2u) && !held[1]) return 1;
	if (!p->waiting && !held[0]) return 0;
	return -1;
}

/* --dev arena2v2: a core in a frame of its own clock it has not yet run
 * in samples its side's partner's keys for that frame, as a GBA's pad is
 * read in each of its frames; queued in both cores by its battle's
 * iteration (arena_team_sample). A core already as far ahead as the keys
 * go samples once the next frame of the pair gives it its own. */
static void sample_partner(ArenaPair *p, unsigned side, unsigned ahead) {
	struct mCore *c = p->core[side];
	unsigned frame = c->frameCounter(c);
	if (!p->team || ahead >= ARENA_LOOKAHEAD || frame == p->sampled[side]) return;
	p->sampled[side] = frame;
	arena_team_sample(p->core, side, p->partner[ahead][side]);
}

bool arena_pair_step(ArenaPair *p, const uint16_t keys[ARENA_LOOKAHEAD][2], bool own_clock) {
	unsigned target[2];
	for (unsigned side = 0; side < 2; ++side) {
		target[side] = p->start[side] + p->relative + 1;
		p->core[side]->setKeys(p->core[side], keys[0][side]);
		/* (a picture not drawn: mGBA's frame skip for this frame, from its
		 * start to its VBlank; the game's state the same either way) */
		((struct GBA *)p->core[side]->board)->video.frameskipCounter = (p->drawn >> side) & 1u ? 0 : 1;
	}
	for (unsigned limit = 0; limit < STEP_LIMIT; ++limit) {
		unsigned ahead[2];
		for (unsigned side = 0; side < 2; ++side) ahead[side] = p->core[side]->frameCounter(p->core[side]) - p->start[side] - p->relative;
		int side = next_side(p, ahead, own_clock);
		if (side < 0) return false;
		struct mCore *c = p->core[side];
		/* (a core past the pair's frame takes its own frame's keys: a frame begins in an event, where a run loop ends) */
		if (own_clock) c->setKeys(c, keys[ahead[side] < ARENA_LOOKAHEAD ? ahead[side] : ARENA_LOOKAHEAD - 1][side]);
		if (own_clock) sample_partner(p, (unsigned)side, ahead[side]);
		p->running = side;
		enum mLockstepPhase phase = p->cable.d.transferActive;
		c->runLoop(c);
		if (phase != TRANSFER_IDLE && p->cable.d.transferActive == TRANSFER_IDLE) ++p->transfers;
		if (both_crossed(p, target)) {
			++p->relative;
			arena_souls_carry(p->core, p->carried);
			return true;
		}
	}
	return false;
}

bool arena_pair_step_all(ArenaPair *p, const uint16_t keys[ARENA_LOOKAHEAD][ARENA_PLAYERS]) {
	uint16_t pads[ARENA_LOOKAHEAD][2];
	for (unsigned k = 0; k < ARENA_LOOKAHEAD; ++k)
		for (unsigned side = 0; side < 2; ++side) {
			pads[k][side] = keys[k][side];
			p->partner[k][side] = keys[k][2 + side];
		}
	return arena_pair_step(p, (const uint16_t (*)[2])pads, true);
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
		p->waiting, p->asleep, p->transfers, (uint32_t)p->running };
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

static uint64_t pair_hash(ArenaPair *p, bool sound) {
	uint64_t hash = UINT64_C(14695981039346656037);
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *c = p->core[side];
		memset(p->scratch, 0, p->scratch_size);
		if (c->stateSize(c) != p->scratch_size || !c->saveState(c, p->scratch)) return 0;
		if (!sound && p->scratch_size >= SAMPLES_END) {
			memset(p->scratch + SOUND_AT, 0, SOUND_END - SOUND_AT);
			memset(p->scratch + SAMPLES_AT, 0, SAMPLES_END - SAMPLES_AT);
		}
		hash_bytes(&hash, p->scratch, p->scratch_size);
		void *flash = NULL;
		size_t size = c->savedataClone(c, &flash);
		hash_word(&hash, (uint32_t)size);
		if (flash) hash_bytes(&hash, flash, size);
		free(flash);
	}
	hash_cable(p, &hash);
	/* (the installs both cores were made with, which their states do not hold) */
	uint64_t made = xlog_hash();
	hash_word(&hash, (uint32_t)made);
	hash_word(&hash, (uint32_t)(made >> 32));
	/* (--dev arena2v2: the partners' folders, which their decks are drawn from and no core's state holds) */
	if (p->team) {
		uint64_t folders = arena_team_custom_hash(UINT64_C(14695981039346656037));
		hash_word(&hash, (uint32_t)folders);
		hash_word(&hash, (uint32_t)(folders >> 32));
	}
	return hash ? hash : 1;
}

uint64_t arena_pair_hash(ArenaPair *p) { return pair_hash(p, false); }
uint64_t arena_pair_hash_whole(ArenaPair *p) { return pair_hash(p, true); }

uint64_t arena_pair_record(const ArenaPair *p) { (void)p; return xlog_hash(); }

/* ---- the snapshot (arena_pair.h): what a rollback puts back ---- */

typedef struct {
	int32_t next_event, event_diff, until;
	int mode;
	bool normal_so, finished, scheduled;
} NodeSnap;

/* what a core's GBA keeps outside its state, which its next instructions
 * read: mGBA's idle loop detection, the open bus, a DMA under way, an
 * interrupt sprung (a state loaded leaves them as they were before it) */
typedef struct {
	uint32_t bus, idle_loop, last_jump;
	int performing_dma, spring_irq, idle_step, idle_failures;
	bool halt_pending, cpu_blocked, early_exit;
	int32_t cached[16];
	bool tainted[16];
	uint16_t io[512];          /* the registers as they read: a state keeps what a read gives and loads it through a write */
	uint16_t sio_rcnt, sio_siocnt;
} GbaSnap;

static void gba_save(const struct GBA *g, GbaSnap *s) {
	*s = (GbaSnap){ g->bus, g->idleLoop, g->lastJump, g->performingDMA, g->springIRQ, g->idleDetectionStep, g->idleDetectionFailures,
		g->haltPending, g->cpuBlocked, g->earlyExit, { 0 }, { 0 } };
	memcpy(s->cached, g->cachedRegisters, sizeof s->cached);
	memcpy(s->tainted, g->taintedRegisters, sizeof s->tainted);
	memcpy(s->io, g->memory.io, sizeof s->io);
	s->sio_rcnt = g->sio.rcnt;
	s->sio_siocnt = g->sio.siocnt;
}

static void gba_load(struct GBA *g, const GbaSnap *s) {
	g->bus = s->bus;
	g->idleLoop = s->idle_loop;
	g->lastJump = s->last_jump;
	g->performingDMA = s->performing_dma;
	g->springIRQ = s->spring_irq;
	g->idleDetectionStep = s->idle_step;
	g->idleDetectionFailures = s->idle_failures;
	g->haltPending = s->halt_pending;
	g->cpuBlocked = s->cpu_blocked;
	g->earlyExit = s->early_exit;
	memcpy(g->cachedRegisters, s->cached, sizeof s->cached);
	memcpy(g->taintedRegisters, s->tainted, sizeof s->tainted);
	memcpy(g->memory.io, s->io, sizeof s->io);
	g->sio.rcnt = s->sio_rcnt;
	g->sio.siocnt = s->sio_siocnt;
}

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
	GbaSnap gba[2];
	/* the pair's scheduling, frame clock and Souls carried */
	int32_t credit[2];
	unsigned waiting, asleep, transfers, start[2], relative;
	int running;
	ArenaSoulCarried carried[2];
	unsigned sampled[2];   /* (--dev arena2v2's partners' sampling) */
};

ArenaPairSnap *arena_pair_snap_new(ArenaPair *p) {
	size_t core_size = p->core[0]->stateSize(p->core[0]), kept_size = hook_kept_size();
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
		gba_save(c->board, &s->gba[side]);
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
	s->asleep = p->asleep;
	s->transfers = p->transfers;
	s->running = p->running;
	memcpy(s->start, p->start, sizeof s->start);
	s->relative = p->relative;
	memcpy(s->carried, p->carried, sizeof s->carried);
	memcpy(s->sampled, p->sampled, sizeof s->sampled);
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
		gba_load(c->board, &s->gba[side]);
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
	p->asleep = s->asleep;
	p->transfers = s->transfers;
	p->running = s->running;
	memcpy(p->start, s->start, sizeof p->start);
	p->relative = s->relative;
	memcpy(p->carried, s->carried, sizeof p->carried);
	memcpy(p->sampled, s->sampled, sizeof p->sampled);
	return true;
}

/* a byte of the sound's in mGBA's state (SOUND_AT) */
static bool sound_byte(size_t i) { return (i >= SOUND_AT && i < SOUND_END) || (i >= SAMPLES_AT && i < SAMPLES_END); }

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
				printf("arena: frame %u: core %u's state apart at 0x%05x-0x%05x\n", frame, side, (unsigned)i, (unsigned)end);
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
