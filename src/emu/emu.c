/* mGBA core host: runs the player's ROM, exposes its frame, sound and memory. */
#include "emu.h"
#include "audio_pace.h"
#include "claims.h"
#include "compat.h"
#include "platform.h"
#include "save_queue.h"
#include "sound_quiet.h"
#include "xinstall_log.h"

#include <stdio.h>
#include <string.h>

#include <SDL.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba/core/serialize.h>
#include <mgba/feature/video-logger.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/overrides.h>
#include <mgba-util/vfs.h>
#ifdef __3DS__
#include <3ds/types.h>
#include <3ds/thread.h>
#endif

#define RING 16384 /* stereo frames of buffered sound */
/* The GBA runs at 59.73 Hz and frames are paced at 60, so the core makes a
 * little more sound than is played: keep the buffer near LAT_TARGET frames
 * (about 32 ms) by nudging the output rate, and drop sound past LAT_MAX. */
#ifdef CW_DESKTOP
#define LAT_TARGET 1024   /* (a desktop keeps up: about 21 ms) */
#else
#define LAT_TARGET 1536
#endif
#define LAT_MAX    6144

static struct mCore *core;
static bool layout_ok;   /* the core's struct GBA reads as expected (emu_init) */
static uint32_t video[EMU_W * EMU_H];
static int out_rate = 48000;

static int16_t ring[RING * 2];
static int ring_r, ring_w;
static SDL_mutex *ring_lock;
/* the ring's place on the sound's timeline (emu_audio_mark): samples written
 * and played since the start, and those of a rollback's sound still to be
 * skipped, as they were played already (issue #167) */
static uint64_t ring_written, ring_played, ring_debt;
static AudioPace pace;   /* (the ring's averaged fill: the output rate's nudge) */

static void quiet(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a) {
	(void)l; (void)c; (void)lv; (void)f; (void)a;
}
static struct mLogger logger = { .log = quiet };

/* (once, before any core) */
void emu_log_quiet(void) {
	static bool set;
	if (!set) mLogSetDefaultLogger(&logger);
	set = true;
}

static void start_worker(void);

bool emu_init(const uint8_t *rom, size_t len) {
	if (core) return true;
	emu_log_quiet();
	/* the copy is padded to EMU_ROM_SIZE: the space past the game is free for
	 * the engine's hooks and generated data (EMU_FREE) */
	if (len > EMU_ROM_SIZE) return false;
	uint8_t *copy = malloc(EMU_ROM_SIZE);
	if (!copy) return false;
	memcpy(copy, rom, len);
	memset(copy + len, 0xFF, EMU_ROM_SIZE - len);
#ifdef __3DS__
	/* (on the 3DS the core maps this copy as it is and patches it in place
	 * (GBALoadROM maps a 16 MB ROM), so it lives as long as the core: a
	 * chunk made from it would be a second 16 MB on a 3DS's heap. mGBA's
	 * own ROM buffer, FIXED_ROM_BUFFER's, serves loaders the game never
	 * calls, and start_3ds.c gives it back) */
	struct VFile *vf = VFileFromMemory(copy, EMU_ROM_SIZE);
	if (!vf) { free(copy); return false; }
#else
	struct VFile *vf = VFileMemChunk(copy, EMU_ROM_SIZE);
	free(copy);
	if (!vf) return false;
#endif
	core = mCoreFindVF(vf);
	if (!core || !core->init(core)) { vf->close(vf); core = NULL; return false; }
	hook_attach(core);
	mCoreInitConfig(core, NULL);
	core->setVideoBuffer(core, (color_t *)video, EMU_W);
	if (!core->loadROM(core, vf)) { core->deinit(core); core = NULL; return false; }
	/* (emu_frame reaches into the core's own struct, so only where its
	 * fields read back what was just loaded and the library's reset value) */
	struct GBA *gba = core->board;
	layout_ok = gba->romVf == vf && gba->pristineRomSize == EMU_ROM_SIZE && gba->idleLoop == IDLE_LOOP_NONE;
	if (!layout_ok) fprintf(stderr, "emu: the core's layout was not the one expected; every frame draws its picture\n");
	/* the game's flash save lives in memory; runs keep their own saves */
	core->loadSave(core, VFileMemChunk(NULL, 0));
	core->setAudioBufferSize(core, 1024);
	emu_audio_rate(out_rate);
	emu_audio_open();
	start_worker();
#ifndef MINIMAL_CORE
	/* (with the core on a thread of its own, its picture is drawn on one
	 * more, mGBA's threaded video, which runs beside the emulation: on the
	 * 3DS on the main core, idle while the core runs) */
	if (emu_threaded()) mCoreConfigSetIntValue(&core->config, "threadedVideo", 1);
#endif
	core->reset(core);
	return true;
}

static void wait_frame(void);

bool emu_ready(void) { return core != NULL; }
void emu_reset(void) { wait_frame(); if (core) { core->reset(core); hook_reapply(); } }

bool emu_hook(uint32_t addr, EmuHook fn, void *user) {
	wait_frame();
	bool ok = core && hook_add(addr, fn, user);
	if (ok) claims_hook(addr);
	if (ok) xlog_hook(addr, fn, user);
	return ok;
}

bool emu_hook_event(uint32_t addr, int kind) {
	wait_frame();
	bool ok = core && hook_add_event(addr, kind);
	if (ok) claims_hook(addr);
	if (ok) xlog_event(addr);
	return ok;
}

void emu_unhook(uint32_t addr) { wait_frame(); if (core) hook_remove(addr); }
void emu_sync(void) { wait_frame(); }
int emu_hook_events(HookEvent *out, int max) { wait_frame(); return core ? hook_drain(out, max) : 0; }

void emu_audio_rate(int rate) {
	wait_frame();
	out_rate = rate;
	if (!core) return;
	blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), rate);
	blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), rate);
}

/* (a core's sound into the ring: BN6's, or the differential's second core's) */
static void pull_audio_from(struct mCore *c) {
	blip_t *l = c->getAudioChannel(c, 0), *r = c->getAudioChannel(c, 1);
	int16_t tmp[1024 * 2];
	int got;
	while ((got = blip_samples_avail(l)) > 0) {
		if (got > 1024) got = 1024;
		blip_read_samples(l, tmp, got, 1);
		blip_read_samples(r, tmp + 1, got, 1);
		SDL_LockMutex(ring_lock);
		for (int i = 0; i < got; ++i) {
			if (ring_debt) { --ring_debt; continue; }   /* (heard already, before a rollback: emu_audio_rewind) */
			int next = (ring_w + 1) % RING;
			if (next == ring_r) break; /* full: drop the rest */
			ring[ring_w * 2] = tmp[i * 2];
			ring[ring_w * 2 + 1] = tmp[i * 2 + 1];
			ring_w = next;
			++ring_written;
		}
		SDL_UnlockMutex(ring_lock);
	}
	SDL_LockMutex(ring_lock);
	int fill = (ring_w - ring_r + RING) % RING;
	if (fill > LAT_MAX) {
		ring_played += (uint64_t)(fill - LAT_TARGET);
		ring_r = (ring_w - LAT_TARGET + RING) % RING;
		fill = LAT_TARGET;
		audio_pace_reset(&pace, fill);
	}
	SDL_UnlockMutex(ring_lock);
	double adj = audio_pace(&pace, fill, LAT_TARGET);
	blip_set_rates(l, c->frequency(c), out_rate * adj);
	blip_set_rates(r, c->frequency(c), out_rate * adj);
}

static void pull_audio(void) { pull_audio_from(core); }

void emu_audio_from(void *other) {
	if (other && ring_lock) pull_audio_from(other);
}

uint64_t emu_audio_mark(void) {
	if (!ring_lock) return 0;
	SDL_LockMutex(ring_lock);
	uint64_t mark = ring_written;
	SDL_UnlockMutex(ring_lock);
	return mark;
}

void emu_audio_rewind(uint64_t mark) {
	if (!ring_lock) return;
	SDL_LockMutex(ring_lock);
	if (mark >= ring_played && mark < ring_written) {
		/* (none of it heard yet: cut, the next written in its place) */
		ring_w = (int)(((uint64_t)ring_w + RING - (ring_written - mark)) % RING);
		ring_written = mark;
	} else if (mark < ring_played) {
		/* (some heard: the rest cut, and as much of the next written skipped) */
		ring_debt += ring_played - mark;
		ring_w = ring_r;
		ring_written = ring_played;
	}
	SDL_UnlockMutex(ring_lock);
}

void emu_audio_drop_from(void *other) {
	struct mCore *c = other;
	if (!c) return;
	blip_clear(c->getAudioChannel(c, 0));
	blip_clear(c->getAudioChannel(c, 1));
}

int emu_audio_out_rate(void) { return out_rate; }

void emu_audio_open(void) {
	if (!ring_lock) ring_lock = SDL_CreateMutex();
}

uint64_t emu_core_ticks, emu_core_unshown_ticks;
int emu_core_unshown;

static void run_frame(bool unshown) {
	uint64_t t0 = SDL_GetPerformanceCounter();
	core->runFrame(core);
	uint64_t dt = SDL_GetPerformanceCounter() - t0;
	emu_core_ticks += dt;
	if (unshown) { emu_core_unshown_ticks += dt; ++emu_core_unshown; }
	pull_audio();
}

/* ---- the core on a thread of its own: the New 3DS's third core (or a
 * computer's, with CYBERWORLD_EMU_THREAD set, to test it) ----
 * emu_frame starts the frame and returns, and whatever else touches the
 * core waits for that frame first (wait_frame): the director's logic takes in
 * each frame whole, as before, and the frame done is drawn while the next
 * runs (scene_emu.c's update takes the frame done in, then starts the
 * next). The frame done's picture is copied aside for the drawing. */
static bool threaded;
static volatile bool in_flight, stopping;
static SDL_sem *go, *done;
static uint32_t shown[EMU_W * EMU_H];
#ifdef __3DS__
static Thread worker;
#else
static SDL_Thread *worker;
#endif

static int worker_main(void *arg) {
	(void)arg;
	for (;;) {
		SDL_SemWait(go);
		if (stopping) return 0;
		run_frame(false);
		/* (the picture drawn on a thread of its own: its frame whole) */
		if (core->videoLogger && core->videoLogger->wait) core->videoLogger->wait(core->videoLogger);
		SDL_SemPost(done);
	}
}
#ifdef __3DS__
static void worker_entry(void *arg) { worker_main(arg); }
#endif

/* (reads of the game while a frame is drawn, which waited for the next
 * one on the core's own thread: the frame log counts them, as each costs
 * the frame its present; the drawing takes what it reads from the update) */
bool emu_drawing;
int emu_draw_waits;

static void wait_frame(void) {
	if (!in_flight) return;
	emu_draw_waits += emu_drawing;
	SDL_SemWait(done);
	in_flight = false;
	memcpy(shown, video, sizeof shown);
}

bool emu_threaded(void) { return threaded; }

static void start_worker(void) {
	if (!(go = SDL_CreateSemaphore(0)) || !(done = SDL_CreateSemaphore(0))) return;
#ifdef __3DS__
	/* (none where it is refused: the old 3DS, or a title without it) */
	worker = threadCreate(worker_entry, NULL, 0x10000, 0x30, 2, false);
	threaded = worker != NULL;
#else
	const char *e = getenv("CYBERWORLD_EMU_THREAD");
	if (e && *e && strcmp(e, "0")) threaded = (worker = SDL_CreateThread(worker_main, "emu", NULL)) != NULL;
#endif
	printf("emu: the GBA core runs %s\n", threaded ? "on a core of its own" : "between the frames' drawing");
}

void emu_quit(void) {
	if (!threaded) return;
	wait_frame();
	stopping = true;
	SDL_SemPost(go);
#ifdef __3DS__
	threadJoin(worker, U64_MAX);
	threadFree(worker);
#else
	SDL_WaitThread(worker, NULL);
#endif
	threaded = false;
	/* (and the picture's thread with the core: a 3DS app ending with a
	 * thread alive takes the HOME Menu down with it) */
	core->deinit(core);
	core = NULL;
}

void emu_frame(uint32_t keys) {
	if (!core) return;
	wait_frame();
	/* (the music or effects turned off or on: Options' Audio, sound_quiet.h) */
	sound_quiet_frame();
	core->setKeys(core, keys);
	if (threaded) {
		in_flight = true;
		SDL_SemPost(go);
		return;
	}
	/* (a frame the loop plays to catch up is not shown: the core draws no
	 * picture for it, a fifth of its time on a 3DS. Smooth motion mixes
	 * every frame.) */
	bool unshown = P.skip_present && !P.blend && layout_ok;
	if (unshown) ((struct GBA *)core->board)->video.frameskipCounter = 1;
	run_frame(unshown);
}

int emu_audio_read(int16_t *out, int frames) {
	int n = 0;
	if (ring_lock) SDL_LockMutex(ring_lock);
	while (n < frames && ring_r != ring_w) {
		out[n * 2] = ring[ring_r * 2];
		out[n * 2 + 1] = ring[ring_r * 2 + 1];
		ring_r = (ring_r + 1) % RING;
		++n;
	}
	ring_played += (uint64_t)n;
	if (ring_lock) SDL_UnlockMutex(ring_lock);
	memset(out + n * 2, 0, (size_t)(frames - n) * 4);
	return n;
}

const uint32_t *emu_video(void) { return threaded ? shown : video; }

uint8_t emu_read8(uint32_t a) { wait_frame(); return core ? (uint8_t)core->rawRead8(core, a, -1) : 0; }
uint16_t emu_read16(uint32_t a) { wait_frame(); return core ? (uint16_t)core->rawRead16(core, a, -1) : 0; }
uint32_t emu_read32(uint32_t a) { wait_frame(); return core ? core->rawRead32(core, a, -1) : 0; }
void emu_write8(uint32_t a, uint8_t v) {
	wait_frame();
	claims_note(a, 1);
	xlog_write(a, &v, 1);
	if (core) core->rawWrite8(core, a, -1, v);
}

void emu_write32(uint32_t a, uint32_t v) {
	wait_frame();
	claims_note(a, 4);
	const uint8_t b[4] = { (uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24) };
	xlog_write(a, b, sizeof b);
	if (core) core->rawWrite32(core, a, -1, v);
}

void emu_write(uint32_t a, const void *data, size_t len) {
	const uint8_t *p = data;
	wait_frame();
	claims_note(a, (uint32_t)len);
	xlog_write(a, data, len);
	for (size_t i = 0; core && i < len; ++i) core->rawWrite8(core, a + (uint32_t)i, -1, p[i]);
}

/* ---- RAM kept patched (emu_keep) ---- */

#define KEEP_MAX 8
#define KEEP_LEN 64
static struct { uint32_t addr, len; uint8_t was[KEEP_LEN], now[KEEP_LEN]; } kept[KEEP_MAX];
static int nkept;

/* each patch's bytes made the engine's (or, `own`, the game's again) where
 * they read the other; bytes that read neither are left as they are */
static void keep_put(bool own) {
	for (int i = 0; i < nkept; ++i) {
		const uint8_t *from = own ? kept[i].now : kept[i].was, *to = own ? kept[i].was : kept[i].now;
		bool there = true;
		for (uint32_t k = 0; there && k < kept[i].len; ++k) there = (uint8_t)core->rawRead8(core, kept[i].addr + k, -1) == from[k];
		for (uint32_t k = 0; there && k < kept[i].len; ++k) core->rawWrite8(core, kept[i].addr + k, -1, to[k]);
	}
}

bool emu_keep(uint32_t addr, const void *was, const void *now, size_t len) {
	wait_frame();
	if (!core || nkept >= KEEP_MAX || !len || len > KEEP_LEN) return false;
	claims_note(addr, (uint32_t)len);
	kept[nkept].addr = addr;
	kept[nkept].len = (uint32_t)len;
	memcpy(kept[nkept].was, was, len);
	memcpy(kept[nkept].now, now, len);
	++nkept;
	keep_put(false);
	return true;
}

/* A path for mGBA's own files: on the 3DS they go to the SD card's file
 * system itself, which takes a path from its root, without the C
 * library's "sdmc:" (a state was never written there) */
static const char *vf_path(const char *path) {
#ifdef __3DS__
	if (!strncmp(path, "sdmc:", 5)) return path + 5;
#endif
	return path;
}

bool emu_save_state(const char *path) {
	wait_frame();
	if (!core) return false;
	/* written beside, then renamed over: a power cut keeps the old state */
	char tmp[640];
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	struct VFile *vf = VFileOpen(vf_path(tmp), O_CREAT | O_TRUNC | O_RDWR);
	if (!vf) return false;
	/* (with the game's own bytes where the engine keeps RAM patched: a
	 * state holds the game's RAM, whatever build loads it) */
	keep_put(true);
	bool ok = mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	keep_put(false);
	vf->close(vf);
	if (ok) ok = cw_rename(tmp, path);
	else remove(tmp);
	platform_persist();
	return ok;
}

/* The state's bytes into memory, for the save queue to write (a
 * checkpoint's, which the game does not wait on): the memory sized from
 * the last state and some, a state that fills it taken as cut short and
 * written at once instead, as emu_save_state writes */
#define STATE_EXTRA (160u * 1024u)   /* (past the core's state: its extdata, the game's flash save the largest) */
bool emu_save_state_queued(const char *path) {
	static size_t room;
	wait_frame();
	if (!core) return false;
	size_t want = room ? room : core->stateSize(core) + STATE_EXTRA;
	/* (zeroed, as a new file is: mGBA writes the state into the file's
	 * memory, and leaves the fields it does not use as it found them) */
	uint8_t *bytes = calloc(1, want);
	struct VFile *vf = bytes ? VFileFromMemory(bytes, want) : NULL;
	bool ok = vf && mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	off_t n = ok ? vf->seek(vf, 0, SEEK_CUR) : 0;
	if (vf) vf->close(vf);
	if (ok && n > 0 && (size_t)n < want) {
		room = (size_t)n + 4096;
		if (save_queue_file(path, bytes, (size_t)n)) { platform_persist(); return true; }
	} else if (ok) {
		room = want * 2;
	}
	free(bytes);
	save_queue_wait();   /* (after those queued before it) */
	return emu_save_state(path);
}

bool emu_load_state(const char *path) {
	save_queue_wait();
	wait_frame();
	if (!core) return false;
	struct VFile *vf = VFileOpen(vf_path(path), O_RDONLY);
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	vf->close(vf);
	/* (a state holds RAM, not the ROM copy: hooks in RAM are written again,
	 * and the patches the engine keeps there) */
	if (ok) {
		hook_reapply();
		keep_put(false);
		sound_quiet_loaded();
	}
	return ok;
}

/* The core's raw state in memory, no file and no flash save: what a
 * rollback would keep a frame (--dev rollbench, src/dev/rollbench.c) */
size_t emu_state_size(void) {
	wait_frame();
	return core ? core->stateSize(core) : 0;
}

bool emu_state_save(void *state) {
	wait_frame();
	return core && core->saveState(core, state);
}

bool emu_state_load(const void *state) {
	wait_frame();
	if (!core || !core->loadState(core, state)) return false;
	hook_reapply();
	keep_put(false);
	sound_quiet_loaded();
	return true;
}
