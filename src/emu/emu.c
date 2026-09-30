/* mGBA core host: runs the player's ROM, exposes its frame, sound and memory. */
#include "emu.h"
#include "compat.h"
#include "platform.h"

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

static void quiet(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a) {
	(void)l; (void)c; (void)lv; (void)f; (void)a;
}
static struct mLogger logger = { .log = quiet };

static void start_worker(void);

bool emu_init(const uint8_t *rom, size_t len) {
	if (core) return true;
	mLogSetDefaultLogger(&logger);
	/* the copy is padded to EMU_ROM_SIZE: the space past the game is free for
	 * the engine's hooks and generated data (EMU_FREE) */
	if (len > EMU_ROM_SIZE) return false;
	uint8_t *copy = malloc(EMU_ROM_SIZE);
	if (!copy) return false;
	memcpy(copy, rom, len);
	memset(copy + len, 0xFF, EMU_ROM_SIZE - len);
#ifdef __3DS__
	/* (on the 3DS the core copies the ROM into a buffer of its own as it
	 * loads, mGBA's FIXED_ROM_BUFFER: it reads this copy as it is, which
	 * lives as long as the core, as a chunk made from it would be a third
	 * 16 MB on a 3DS's heap) */
	struct VFile *vf = VFileFromMemory(copy, EMU_ROM_SIZE);
	if (!vf) { free(copy); return false; }
#else
	struct VFile *vf = VFileMemChunk(copy, EMU_ROM_SIZE);
	free(copy);
	if (!vf) return false;
#endif
	core = mCoreFindVF(vf);
	if (!core || !core->init(core)) { vf->close(vf); core = NULL; return false; }
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
	ring_lock = SDL_CreateMutex();
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
void emu_reset(void) { wait_frame(); if (core) core->reset(core); }

void emu_audio_rate(int rate) {
	wait_frame();
	out_rate = rate;
	if (!core) return;
	blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), rate);
	blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), rate);
}

static void pull_audio(void) {
	blip_t *l = core->getAudioChannel(core, 0), *r = core->getAudioChannel(core, 1);
	int16_t tmp[1024 * 2];
	int got;
	while ((got = blip_samples_avail(l)) > 0) {
		if (got > 1024) got = 1024;
		blip_read_samples(l, tmp, got, 1);
		blip_read_samples(r, tmp + 1, got, 1);
		SDL_LockMutex(ring_lock);
		for (int i = 0; i < got; ++i) {
			int next = (ring_w + 1) % RING;
			if (next == ring_r) break; /* full: drop the rest */
			ring[ring_w * 2] = tmp[i * 2];
			ring[ring_w * 2 + 1] = tmp[i * 2 + 1];
			ring_w = next;
		}
		SDL_UnlockMutex(ring_lock);
	}
	SDL_LockMutex(ring_lock);
	int fill = (ring_w - ring_r + RING) % RING;
	if (fill > LAT_MAX) { ring_r = (ring_w - LAT_TARGET + RING) % RING; fill = LAT_TARGET; }
	SDL_UnlockMutex(ring_lock);
	double adj = 1.0 - 0.02 * (fill - LAT_TARGET) / LAT_TARGET;
	if (adj < 0.97) adj = 0.97;
	if (adj > 1.03) adj = 1.03;
	blip_set_rates(l, core->frequency(core), out_rate * adj);
	blip_set_rates(r, core->frequency(core), out_rate * adj);
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

static void wait_frame(void) {
	if (!in_flight) return;
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
	if (ring_lock) SDL_UnlockMutex(ring_lock);
	memset(out + n * 2, 0, (size_t)(frames - n) * 4);
	return n;
}

const uint32_t *emu_video(void) { return threaded ? shown : video; }

uint8_t emu_read8(uint32_t a) { wait_frame(); return core ? (uint8_t)core->rawRead8(core, a, -1) : 0; }
uint16_t emu_read16(uint32_t a) { wait_frame(); return core ? (uint16_t)core->rawRead16(core, a, -1) : 0; }
uint32_t emu_read32(uint32_t a) { wait_frame(); return core ? core->rawRead32(core, a, -1) : 0; }
void emu_write8(uint32_t a, uint8_t v) { wait_frame(); if (core) core->rawWrite8(core, a, -1, v); }
void emu_write32(uint32_t a, uint32_t v) { wait_frame(); if (core) core->rawWrite32(core, a, -1, v); }
void emu_write(uint32_t a, const void *data, size_t len) {
	const uint8_t *p = data;
	for (size_t i = 0; i < len; ++i) emu_write8(a + (uint32_t)i, p[i]);
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
	bool ok = mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	vf->close(vf);
	if (ok) ok = cw_rename(tmp, path);
	else remove(tmp);
	platform_persist();
	return ok;
}

bool emu_load_state(const char *path) {
	wait_frame();
	if (!core) return false;
	struct VFile *vf = VFileOpen(vf_path(path), O_RDONLY);
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	vf->close(vf);
	return ok;
}
