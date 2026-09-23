/* mGBA core host: runs the player's ROM, exposes its frame, sound and memory. */
#include "emu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>

#define RING 16384 /* stereo frames of buffered sound */

static struct mCore *core;
static uint32_t video[EMU_W * EMU_H];
static int out_rate = 48000;

static int16_t ring[RING * 2];
static int ring_r, ring_w;
static SDL_mutex *ring_lock;

static void quiet(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a) {
	(void)l; (void)c; (void)lv; (void)f; (void)a;
}
static struct mLogger logger = { .log = quiet };

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
	struct VFile *vf = VFileMemChunk(copy, EMU_ROM_SIZE);
	free(copy);
	if (!vf) return false;
	core = mCoreFindVF(vf);
	if (!core || !core->init(core)) { vf->close(vf); core = NULL; return false; }
	mCoreInitConfig(core, NULL);
	core->setVideoBuffer(core, (color_t *)video, EMU_W);
	if (!core->loadROM(core, vf)) { core->deinit(core); core = NULL; return false; }
	/* the game's flash save lives in memory; runs keep their own saves */
	core->loadSave(core, VFileMemChunk(NULL, 0));
	core->setAudioBufferSize(core, 1024);
	emu_audio_rate(out_rate);
	ring_lock = SDL_CreateMutex();
	core->reset(core);
	return true;
}

bool emu_ready(void) { return core != NULL; }
void emu_reset(void) { if (core) core->reset(core); }

void emu_audio_rate(int rate) {
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
}

void emu_frame(uint32_t keys) {
	if (!core) return;
	core->setKeys(core, keys);
	core->runFrame(core);
	pull_audio();
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

const uint32_t *emu_video(void) { return video; }

uint8_t emu_read8(uint32_t a) { return core ? (uint8_t)core->rawRead8(core, a, -1) : 0; }
uint16_t emu_read16(uint32_t a) { return core ? (uint16_t)core->rawRead16(core, a, -1) : 0; }
uint32_t emu_read32(uint32_t a) { return core ? core->rawRead32(core, a, -1) : 0; }
void emu_write8(uint32_t a, uint8_t v) { if (core) core->rawWrite8(core, a, -1, v); }
void emu_write32(uint32_t a, uint32_t v) { if (core) core->rawWrite32(core, a, -1, v); }
void emu_write(uint32_t a, const void *data, size_t len) {
	const uint8_t *p = data;
	for (size_t i = 0; i < len; ++i) emu_write8(a + (uint32_t)i, p[i]);
}

bool emu_save_state(const char *path) {
	if (!core) return false;
	/* written beside, then renamed over: a power cut keeps the old state */
	char tmp[640];
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	struct VFile *vf = VFileOpen(tmp, O_CREAT | O_TRUNC | O_RDWR);
	if (!vf) return false;
	bool ok = mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	vf->close(vf);
	if (ok) ok = rename(tmp, path) == 0;
	else remove(tmp);
	return ok;
}

bool emu_load_state(const char *path) {
	if (!core) return false;
	struct VFile *vf = VFileOpen(path, O_RDONLY);
	if (!vf) return false;
	bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	vf->close(vf);
	return ok;
}
