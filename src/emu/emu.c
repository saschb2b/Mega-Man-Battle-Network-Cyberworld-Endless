/* mGBA core host: runs the player's ROM, exposes its frame, sound and memory. */
#include "emu.h"

/* (on mGBA; a build on gpSP has emu_gpsp.c instead: CW_GPSP) */
#ifndef CW_GPSP
#include "claims.h"
#include "compat.h"
#include "emu_keep.h"
#include "emu_sound.h"
#include "free_space_watch.h"
#include "platform.h"
#include "ram_survey.h"
#include "xchips_pack.h"
#include "save_queue.h"
#include "sound_quiet.h"
#include "xinstall_log.h"

#include <stdio.h>
#include <stdlib.h>
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


static bool surveying;   /* (ram_survey.h) */
static struct mCore *core;
static bool layout_ok;   /* the core's struct GBA reads as expected (emu_init) */
static uint32_t video[EMU_W * EMU_H];
static int out_rate = 48000;


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

/* (CYBERWORLD_FREE_SPACE=FILE: what each region of the free space holds as
 * the program ends, for tools/free_space.py --usage) */
static void usage_at_exit(void) {
	const char *path = getenv("CYBERWORLD_FREE_SPACE");
	if (core && path && !free_space_usage(path, emu_read8)) fprintf(stderr, "free space: %s not written\n", path);
}

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
#if defined(__3DS__) || defined(__PSP__)
	/* (on the 3DS and the PSP the core maps this copy as it is and patches
	 * it in place (GBALoadROM maps a 16 MB ROM), so it lives as long as the
	 * core: a chunk made from it would be a second 16 MB on their heaps.
	 * mGBA's own ROM buffer, FIXED_ROM_BUFFER's, serves loaders the game
	 * never calls: start_3ds.c gives the 3DS's back, the PSP's is none) */
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
	/* (the chip pack's entries in that save's RAM, which every state carries: xchips_pack.h) */
	xchips_pack_ready(core);
	core->setAudioBufferSize(core, 1024);
	emu_audio_rate(out_rate);
	emu_audio_open();
	start_worker();
	if (getenv("CYBERWORLD_FREE_SPACE")) { free_space_measure(); atexit(usage_at_exit); }
	surveying = ram_survey_open();
	ram_survey_watch(core);
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

/* Fast forward's sound (emu_frame_as): the frame running's EMU_FRAME_*;
 * the first samples of the first frame not heard after one heard, which
 * carry on where that one stopped, and the next heard frame's first faded
 * in over them */
#define XFADE 240   /* stereo samples: 5 ms at 48 kHz, a third of a frame's */
static unsigned frame_how;
static int16_t tail[XFADE * 2];
static int tail_n, tail_at;
static bool tail_wanted;

/* `got` samples of an unheard frame: the tail taken from them where one is wanted */
static void tail_take(const int16_t *tmp, int got) {
	if (!tail_wanted) return;
	int n = got < XFADE - tail_n ? got : XFADE - tail_n;
	memcpy(tail + tail_n * 2, tmp, (size_t)n * 4);
	tail_n += n;
	if (tail_n == XFADE) tail_wanted = false;
}

/* ... and of a heard one: the tail faded out under its first */
static void tail_join(int16_t *tmp, int got) {
	for (int i = 0; i < got && tail_at < tail_n; ++i, ++tail_at)
		for (int ch = 0; ch < 2; ++ch)
			tmp[i * 2 + ch] = (int16_t)((tail[tail_at * 2 + ch] * (tail_n - tail_at) + tmp[i * 2 + ch] * tail_at) / tail_n);
}

/* (a core's sound into the ring: BN6's, or the differential's second core's) */
static void pull_audio_from(struct mCore *c) {
	blip_t *l = c->getAudioChannel(c, 0), *r = c->getAudioChannel(c, 1);
	int16_t tmp[1024 * 2];
	int got;
	bool own = c == core, unheard = own && (frame_how & EMU_FRAME_UNHEARD);
	while ((got = blip_samples_avail(l)) > 0) {
		if (got > 1024) got = 1024;
		blip_read_samples(l, tmp, got, 1);
		blip_read_samples(r, tmp + 1, got, 1);
		if (unheard) { tail_take(tmp, got); continue; }
		if (own) tail_join(tmp, got);
		emu_sound_put(tmp, got);
	}
	/* (a frame heard: the next unheard one's first samples are its tail) */
	if (own && !unheard) {
		tail_n = tail_at = 0;
		tail_wanted = true;
	}
	double adj = emu_sound_settle();
	blip_set_rates(l, c->frequency(c), out_rate * adj);
	blip_set_rates(r, c->frequency(c), out_rate * adj);
}

static void pull_audio(void) { pull_audio_from(core); }

void emu_audio_from(void *other) {
	if (other && emu_sound_ready()) pull_audio_from(other);
}

void emu_audio_drop_from(void *other) {
	struct mCore *c = other;
	if (!c) return;
	blip_clear(c->getAudioChannel(c, 0));
	blip_clear(c->getAudioChannel(c, 1));
}

int emu_audio_out_rate(void) { return out_rate; }

uint64_t emu_core_ticks, emu_core_unshown_ticks;
int emu_core_unshown;

static void run_frame(bool unshown) {
	uint64_t t0 = SDL_GetPerformanceCounter();
	core->runFrame(core);
	/* (a dev tool's look at the work RAM, off without its variables: ram_survey.h) */
	if (surveying) ram_survey_frame((uint8_t *)((struct GBA *)core->board)->memory.wram, (uint8_t *)((struct GBA *)core->board)->memory.iwram);
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
static uint32_t pictures;   /* (emu_pictures) */
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
	++pictures;
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
	bool want = e && *e && strcmp(e, "0");
#ifdef __vita__
	/* (the Vita's 444 MHz cores: the GBA's on one of its own, its drawing
	 * beside it, as on the New 3DS) */
	want = !e || strcmp(e, "0");
#endif
	if (want) threaded = (worker = SDL_CreateThread(worker_main, "emu", NULL)) != NULL;
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

void emu_frame(uint32_t keys) { emu_frame_as(keys, 0); }

static void (*before_frame)(void);
void emu_before_frame(void (*step)(void)) { before_frame = step; }

void emu_frame_as(uint32_t keys, unsigned how) {
	if (!core) return;
	wait_frame();
	frame_how = how;
	/* (the music or effects turned off or on: Options' Audio, sound_quiet.h) */
	sound_quiet_frame();
	/* (an install's own step before the frame: the pool of BN4's Navis' sounds pointed at the battle's Navi's,
	 * xnavi_bn4_sounds.h) */
	if (before_frame) before_frame();
	core->setKeys(core, keys);
	if (threaded) {
		in_flight = true;
		SDL_SemPost(go);
		return;
	}
	/* (a frame the loop plays to catch up is not shown: the core draws no
	 * picture for it, a fifth of its time on a 3DS. Smooth motion mixes
	 * every frame shown; of fast forward's batch only its last is.) */
	bool unshown = ((P.skip_present && !P.blend) || (how & EMU_FRAME_UNSHOWN)) && layout_ok;
	if (unshown) ((struct GBA *)core->board)->video.frameskipCounter = 1;
	run_frame(unshown);
	if (!unshown) ++pictures;
}

const uint32_t *emu_video(void) { return threaded ? shown : video; }

uint32_t emu_pictures(void) { return pictures; }

/* The cartridge's bytes as mGBA holds them, read and written in place
 * rather than a byte at a time through its bus: the installs' reads and
 * writes so were a fifth of a start with BN5 and BN4
 * (docs/devtools/profiling.md). A read is GBALoad8's for a cartridge
 * address within the ROM's size (0x08-0x0D, mirrors of one image); a write
 * leaves what rawWrite8 byte by byte leaves, its last byte written that way
 * (the copy made from the player's ROM before the first, the size grown
 * to cover the write), the rest copied in where those writes put them. */
#define CART_MASK 0x01FFFFFFu

static int cart_read(uint32_t a) {
	if (!core || (a >> 24) < 0x08 || (a >> 24) > 0x0D) return -1;
	const struct GBA *gba = core->board;
	uint32_t off = a & CART_MASK;
	return off < gba->memory.romSize ? ((const uint8_t *)gba->memory.rom)[off] : -1;
}

static bool cart_write(uint32_t a, const uint8_t *p, size_t len) {
	if (!core || !len || len > CART_MASK || a < 0x08000000u || a > 0x0A000000u - len) return false;
	core->rawWrite8(core, a + (uint32_t)(len - 1), -1, p[len - 1]);
	struct GBA *gba = core->board;
	memcpy((uint8_t *)gba->memory.rom + (a & CART_MASK), p, len - 1);
	return true;
}

uint8_t emu_read8(uint32_t a) {
	wait_frame();
	int v = cart_read(a);
	return v >= 0 ? (uint8_t)v : core ? (uint8_t)core->rawRead8(core, a, -1) : 0;
}
uint16_t emu_read16(uint32_t a) { wait_frame(); return core ? (uint16_t)core->rawRead16(core, a, -1) : 0; }
uint32_t emu_read32(uint32_t a) { wait_frame(); return core ? core->rawRead32(core, a, -1) : 0; }
void emu_read_bytes(uint32_t a, uint8_t *out, uint32_t len) {
	wait_frame();
	for (uint32_t i = 0; i < len; ++i) {
		int v = cart_read(a + i);
		out[i] = v >= 0 ? (uint8_t)v : core ? (uint8_t)core->rawRead8(core, a + i, -1) : 0;
	}
}

void emu_write8(uint32_t a, uint8_t v) {
	wait_frame();
	claims_note(a, 1);
	free_space_write(a, 1);
	xlog_write(a, &v, 1);
	if (core) core->rawWrite8(core, a, -1, v);
}

void emu_write32(uint32_t a, uint32_t v) {
	wait_frame();
	claims_note(a, 4);
	free_space_write(a, 4);
	const uint8_t b[4] = { (uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24) };
	xlog_write(a, b, sizeof b);
	if (core) core->rawWrite32(core, a, -1, v);
}

void emu_write(uint32_t a, const void *data, size_t len) {
	const uint8_t *p = data;
	wait_frame();
	claims_note(a, (uint32_t)len);
	free_space_write(a, (uint32_t)len);
	xlog_write(a, data, len);
	if (cart_write(a, p, len)) return;
	for (size_t i = 0; core && i < len; ++i) core->rawWrite8(core, a + (uint32_t)i, -1, p[i]);
}

/* ---- RAM kept patched (emu_keep, emu_keep.h) ---- */

static uint8_t raw_read8(uint32_t a) { return (uint8_t)core->rawRead8(core, a, -1); }
static void raw_write8(uint32_t a, uint8_t v) { core->rawWrite8(core, a, -1, v); }
static void keep_put(bool own) { emu_keep_put(own, raw_read8, raw_write8); }

bool emu_keep(uint32_t addr, const void *was, const void *now, size_t len) {
	wait_frame();
	if (!core || !emu_keep_add(addr, was, now, len)) return false;
	claims_note(addr, (uint32_t)len);
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
	/* (with the game's own bytes where the engine keeps RAM patched, as
	 * emu_save_state saves them: a checkpoint is the state another build
	 * loads) */
	keep_put(true);
	bool ok = vf && mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	keep_put(false);
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
		xchips_pack_ready(core);   /* (a state from before the pack's entries in the save RAM: xchips_pack.h) */
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

/* The core's whole state in memory with the cart's flash save in it, as a
 * checkpoint's file holds it: a test battle's snapshot (lab_battle.h),
 * which must put back the pack's entries in the save RAM too */
void *emu_snapshot(size_t *size) {
	wait_frame();
	if (!core) return NULL;
	size_t want = core->stateSize(core) + STATE_EXTRA;
	uint8_t *bytes = calloc(1, want);
	struct VFile *vf = bytes ? VFileFromMemory(bytes, want) : NULL;
	bool ok = vf && mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	off_t n = ok ? vf->seek(vf, 0, SEEK_CUR) : 0;
	if (vf) vf->close(vf);
	if (!ok || n <= 0 || (size_t)n >= want) { free(bytes); return NULL; }
	*size = (size_t)n;
	return bytes;
}

bool emu_snapshot_load(void *bytes, size_t size) {
	wait_frame();
	struct VFile *vf = core && bytes ? VFileFromMemory(bytes, size) : NULL;
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	vf->close(vf);
	if (ok) {
		hook_reapply();
		keep_put(false);
		sound_quiet_loaded();
	}
	return ok;
}

#endif
