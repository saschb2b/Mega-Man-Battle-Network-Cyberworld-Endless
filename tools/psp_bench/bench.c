/* A GBA core's cost on BN6, frame by frame, for docs/engine/emulation/psp-and-ds.md:
 * the ROM loaded, an mGBA state put in or a scripted input played from
 * power-on, each frame's time taken by the system's clock (on a PSP
 * sceKernelGetSystemTimeWide: in PPSSPP, emulated time).
 * BACKEND_MGBA: mGBA 0.10.5's library as the game drives it, its ROM the
 * padded 16 MB copy, with the engine's idle hook (src/emu/idle.c).
 * BACKEND_GPSP: libretro gpSP (its MIPS dynarec on a PSP); an mGBA state
 * converted into gpSP's own (backend_state).
 * Built and run by tools/psp_bench.py; not part of the game. */
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSP
#include <malloc.h>
#include <pspkernel.h>
#include <psppower.h>
#include <psputils.h>
PSP_MODULE_INFO("cwbench", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-1024);
static int64_t now_us(void) { return (int64_t)sceKernelGetSystemTimeWide(); }
#else
#include <time.h>
static int64_t now_us(void) {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (int64_t)t.tv_sec * 1000000 + t.tv_nsec / 1000;
}
#endif

enum { K_A = 1, K_B = 2, K_SEL = 4, K_START = 8, K_R = 16, K_L = 32, K_U = 64, K_D = 128, K_RB = 256, K_LB = 512 };

/* the script: hold `keys` for `frames` frames, in turn */
typedef struct { int frames; unsigned keys; } Step;
static Step script[512];
static int nsteps;

static unsigned keys_at(int frame) {
	int f = 0;
	for (int i = 0; i < nsteps; ++i) {
		if (frame < f + script[i].frames) return script[i].keys;
		f += script[i].frames;
	}
	return 0;
}

/* "40:,4:A,6:,30:UP+RIGHT" as tools/play.py's input */
static void parse_script(const char *s) {
	nsteps = 0;
	while (*s && nsteps < 512) {
		int n = atoi(s);
		const char *c = strchr(s, ':');
		if (!c) break;
		++c;
		unsigned k = 0;
		while (*c && *c != ',') {
			const char *e = c;
			while (*e && *e != ',' && *e != '+') ++e;
			size_t len = (size_t)(e - c);
#define IS(w) (len == strlen(w) && !strncmp(c, w, len))
			if (IS("A")) k |= K_A; else if (IS("B")) k |= K_B; else if (IS("SELECT")) k |= K_SEL;
			else if (IS("START")) k |= K_START; else if (IS("RIGHT")) k |= K_R; else if (IS("LEFT")) k |= K_L;
			else if (IS("UP")) k |= K_U; else if (IS("DOWN")) k |= K_D; else if (IS("R")) k |= K_RB;
			else if (IS("L")) k |= K_LB;
#undef IS
			c = *e == '+' ? e + 1 : e;
		}
		script[nsteps].frames = n;
		script[nsteps].keys = k;
		++nsteps;
		s = *c == ',' ? c + 1 : c;
	}
}

static uint8_t *slurp(const char *path, size_t *len) {
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long n = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t *b = malloc((size_t)n);
	if (b && fread(b, 1, (size_t)n, f) != (size_t)n) { free(b); b = NULL; }
	fclose(f);
	*len = (size_t)n;
	return b;
}

static uint32_t frame32[240 * 160];
static void dump_ppm(const char *dir, int frame) {
	char path[256];
	snprintf(path, sizeof path, "%s/f%05d.ppm", dir, frame);
	FILE *f = fopen(path, "wb");
	if (!f) return;
	fprintf(f, "P6\n240 160\n255\n");
	static uint8_t row[240 * 3];
	for (int y = 0; y < 160; ++y) {
		for (int x = 0; x < 240; ++x) {
			uint32_t p = frame32[y * 240 + x];
			row[x * 3] = (uint8_t)p; row[x * 3 + 1] = (uint8_t)(p >> 8); row[x * 3 + 2] = (uint8_t)(p >> 16);
		}
		fwrite(row, 1, sizeof row, f);
	}
	fclose(f);
}

/* ---- the backends: init, run a frame with keys, its picture into frame32 ---- */
#ifdef BACKEND_MGBA
#include <mgba/core/blip_buf.h>
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba/core/serialize.h>
#include <mgba/gba/core.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/internal/gba/gba.h>
#include <mgba-util/vfs.h>
static struct mCore *core;
static uint32_t video[240 * 160];
static void quiet(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a) { (void)l; (void)c; (void)lv; (void)f; (void)a; }
static struct mLogger logger = { .log = quiet };
static const char *backend_name = "mgba";
static void idle_hook(void);
#define AWAIT_LOOP 0x080003A6u   /* BN6_AWAIT_FRAME_LOOP (src/emu/bn6.h) */
static uint16_t await_original;
static int backend_init(uint8_t *rom, size_t len, uint8_t *sav, size_t savlen) {
	mLogSetDefaultLogger(&logger);
	/* the game's padded 16 MB copy, as emu.c makes it */
	uint8_t *copy = malloc(16u << 20);
	if (!copy) return 0;
	memcpy(copy, rom, len);
	memset(copy + len, 0xFF, (16u << 20) - len);
	free(rom);
	/* (the hook's BKPT written into the copy before the core maps it: a
	 * patch after would make mGBA copy the ROM, 32 MB, as on the 3DS) */
	if (!getenv("BENCH_NO_IDLE")) {
		uint32_t o = AWAIT_LOOP & 0xFFFFFF;
		await_original = (uint16_t)(copy[o] | copy[o + 1] << 8);
		copy[o] = 0xCE; copy[o + 1] = 0xBE;
	}
	struct VFile *vf = VFileFromMemory(copy, 16u << 20);
	core = GBACoreCreate();
	if (!core || !core->init(core)) return 0;
	mCoreInitConfig(core, NULL);
	core->setVideoBuffer(core, (color_t *)video, 240);
	if (!core->loadROM(core, vf)) return 0;
	struct VFile *sv = VFileMemChunk(sav, savlen);
	core->loadSave(core, sv);
	core->setAudioBufferSize(core, 1024);
	blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), 44100);
	blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), 44100);
	core->reset(core);
	idle_hook();
	return 1;
}
/* the engine's idle hook (src/emu/idle.c): a Thumb BKPT over BN6's
 * VBlank wait (BN6_AWAIT_FRAME_LOOP), halting the CPU there while VBlank's
 * interrupt is on, then the original instruction run in its place */
static void (*board_bkpt16)(struct ARMCore *, int);
static void on_bkpt16(struct ARMCore *cpu, int imm) {
	uint32_t at = (uint32_t)cpu->gprs[ARM_PC] - 4;
	if (imm != 0xCE || at != AWAIT_LOOP) { board_bkpt16(cpu, imm); return; }
	uint16_t stat = (uint16_t)core->rawRead16(core, 0x04000004, -1);
	if (!(stat & 1) && (stat & 8) && (core->rawRead16(core, 0x04000200, -1) & 1) && (core->rawRead16(core, 0x04000208, -1) & 1))
		GBAHalt((struct GBA *)core->board);
	ARMRunFake(cpu, await_original);
}
static void idle_hook(void) {
	if (!getenv("BENCH_NO_IDLE")) {
		struct ARMCore *cpu = core->cpu;
		if (cpu->irqh.bkpt16 != on_bkpt16) { board_bkpt16 = cpu->irqh.bkpt16; cpu->irqh.bkpt16 = on_bkpt16; }
	}
}
static int backend_state(const char *path) {
	struct VFile *vf = VFileOpen(path, O_RDONLY);
	if (!vf) return 0;
	int ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
	vf->close(vf);
	idle_hook();
	return ok;
}
static void backend_frame(unsigned keys, int want_picture) {
	core->setKeys(core, keys);
	core->runFrame(core);
	static int16_t buf[2048];
	blip_t *l = core->getAudioChannel(core, 0), *r = core->getAudioChannel(core, 1);
	int n;
	while ((n = blip_samples_avail(l)) > 0) {
		if (n > 1024) n = 1024;
		blip_read_samples(l, buf, n, 1);
		blip_read_samples(r, buf + 1, n, 1);
	}
	if (want_picture) memcpy(frame32, video, sizeof video);
}
#endif

#ifdef BACKEND_GPSP
#include "libretro.h"
static const char *backend_name = "gpsp";
static unsigned cur_keys;
static const void *last_fb;
static size_t last_pitch;
static void vid(const void *data, unsigned w, unsigned h, size_t pitch) { (void)w; (void)h; if (data) { last_fb = data; last_pitch = pitch; } }
static void aud(int16_t l, int16_t r) { (void)l; (void)r; }
static size_t audb(const int16_t *d, size_t n) { (void)d; return n; }
static void poll(void) {}
static int16_t inp(unsigned port, unsigned dev, unsigned idx, unsigned id) {
	(void)dev; (void)idx;
	if (port) return 0;
	static const unsigned map[16] = {
		[RETRO_DEVICE_ID_JOYPAD_A] = K_A, [RETRO_DEVICE_ID_JOYPAD_B] = K_B,
		[RETRO_DEVICE_ID_JOYPAD_SELECT] = K_SEL, [RETRO_DEVICE_ID_JOYPAD_START] = K_START,
		[RETRO_DEVICE_ID_JOYPAD_RIGHT] = K_R, [RETRO_DEVICE_ID_JOYPAD_LEFT] = K_L,
		[RETRO_DEVICE_ID_JOYPAD_UP] = K_U, [RETRO_DEVICE_ID_JOYPAD_DOWN] = K_D,
		[RETRO_DEVICE_ID_JOYPAD_R] = K_RB, [RETRO_DEVICE_ID_JOYPAD_L] = K_LB };
	if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
		int16_t m = 0;
		for (unsigned i = 0; i < 16; ++i) if (map[i] && (cur_keys & map[i])) m |= (int16_t)(1 << i);
		return m;
	}
	return id < 16 && map[id] && (cur_keys & map[id]) ? 1 : 0;
}
static const char *sysdir = ".";
static bool env(unsigned cmd, void *data) {
	switch (cmd) {
	case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
		*(const char **)data = sysdir; return true;
	case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: return *(const int *)data == RETRO_PIXEL_FORMAT_RGB565;
	case RETRO_ENVIRONMENT_GET_VARIABLE: {
		struct retro_variable *v = data;
		v->value = NULL;
		if (!strcmp(v->key, "gpsp_bios")) v->value = "builtin";
		else if (!strcmp(v->key, "gpsp_boot_mode")) v->value = "game";
		else if (!strcmp(v->key, "gpsp_drc")) v->value = "enabled";
		else if (!strcmp(v->key, "gpsp_frameskip")) v->value = "disabled";
		return v->value != NULL;
	}
	case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: *(bool *)data = false; return true;
	case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS: return true;
	case RETRO_ENVIRONMENT_GET_CAN_DUPE: *(bool *)data = true; return true;
	default: return false;
	}
}
static int backend_init(uint8_t *rom, size_t len, uint8_t *sav, size_t savlen) {
	retro_set_environment(env);
	retro_set_video_refresh(vid);
	retro_set_audio_sample(aud);
	retro_set_audio_sample_batch(audb);
	retro_set_input_poll(poll);
	retro_set_input_state(inp);
	retro_init();
	struct retro_game_info gi = { .path = "bn6.gba", .data = rom, .size = len, .meta = NULL };
	if (!retro_load_game(&gi)) return 0;
	void *sram = retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
	size_t ssz = retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
	if (sram && sav) memcpy(sram, sav, savlen < ssz ? savlen : ssz);
	retro_reset();
	return 1;
}
/* An mGBA state (its GBASerializedState, serialize.h's layout) put into
 * gpSP: its own state taken, mGBA's memory written over it, loaded (gpSP's
 * fixups: the palette, the dynarec's caches), then the CPU's registers set
 * and the timers', DMAs' and sound's control registers written again
 * through gpSP's own handlers, as the game wrote them */
extern uint32_t reg[64];
extern uint32_t spsr[6];
extern uint32_t reg_mode[7][7];
extern uint16_t io_registers[512];
extern uint8_t write_memory16(uint32_t address, uint16_t value);
static int patch_bytes(uint8_t *doc, size_t len, const char *key, const uint8_t *src, uint32_t n) {
	size_t kl = strlen(key);
	for (size_t i = 0; i + kl + 7 + n <= len; ++i) {
		if (doc[i] == 0x05 && !memcmp(doc + i + 1, key, kl) && doc[i + 1 + kl] == 0) {
			uint8_t *p = doc + i + 2 + kl;
			uint32_t sz = (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
			if (sz != n) { printf("bench: %s is %u bytes in gpSP, %u in mGBA\n", key, (unsigned)sz, (unsigned)n); return 0; }
			memcpy(p + 5, src, n);
			return 1;
		}
	}
	printf("bench: no %s in gpSP's state\n", key);
	return 0;
}
static uint32_t rd32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }
/* mGBA's bank (arm.h) for a CPSR mode; gpSP's reg_mode/spsr row for it */
static int mgba_bank(uint32_t m) {
	switch (m & 0x1F) { case 0x11: return 1; case 0x12: return 2; case 0x13: return 3; case 0x17: return 4; case 0x1B: return 5; default: return 0; }
}
static int gpsp_mode(uint32_t m) {
	switch (m & 0x1F) { case 0x10: return 0x00; case 0x1F: return 0x10; case 0x12: return 0x11; case 0x11: return 0x12;
	case 0x13: return 0x13; case 0x17: return 0x14; case 0x1B: return 0x15; default: return 0x10; }
}
static int backend_state(const char *path) {
	size_t n = 0;
	uint8_t *st = slurp(path, &n);
	if (!st || n < 0x61000 || rd32(st) != 0x01000007) { printf("bench: not an mGBA state (%08x)\n", st ? (unsigned)rd32(st) : 0u); return 0; }
	size_t sz = retro_serialize_size();
	uint8_t *doc = malloc(sz);
	if (!retro_serialize(doc, sz)) return 0;
	uint16_t vcount = io_registers[3], dispstat = io_registers[2];
	if (!(patch_bytes(doc, sz, "iwram", st + 0x19000, 0x8000) && patch_bytes(doc, sz, "ewram", st + 0x21000, 0x40000) &&
	      patch_bytes(doc, sz, "vram", st + 0x1000, 0x18000) && patch_bytes(doc, sz, "oamram", st + 0xC00, 0x400) &&
	      patch_bytes(doc, sz, "palram", st + 0x800, 0x400) && patch_bytes(doc, sz, "ioregs", st + 0x400, 0x400)))
		return 0;
	if (!retro_unserialize(doc, sz)) { printf("bench: gpSP refused the state\n"); return 0; }
	/* the picture's position stays gpSP's own (both at the frame's end) */
	io_registers[3] = vcount;
	io_registers[2] = (uint16_t)((io_registers[2] & ~7u) | (dispstat & 7u));
	/* the CPU: mGBA's PC is one instruction ahead (its prefetch) */
	uint32_t cpsr = rd32(st + 0x60), m = cpsr & 0x1F;
	for (int i = 0; i < 16; ++i) reg[i] = rd32(st + 0x20 + i * 4);
	reg[15] -= (cpsr & 0x20) ? 2 : 4;
	reg[16] = cpsr;
	reg[17] = (uint32_t)gpsp_mode(m);
	reg[18] = 0;   /* active */
	const uint8_t *banked = st + 0x70, *bspsr = st + 0x118;
	static const uint32_t modes[] = { 0x10, 0x11, 0x12, 0x13, 0x17, 0x1B };
	for (int k = 0; k < 6; ++k) {
		int b = mgba_bank(modes[k]), row = gpsp_mode(modes[k]) & 0xF;
		reg_mode[row][5] = rd32(banked + (b * 7 + 0) * 4);
		reg_mode[row][6] = rd32(banked + (b * 7 + 1) * 4);
		if (b == 1) for (int r = 0; r < 5; ++r) reg_mode[row][r] = rd32(banked + (7 + 2 + r) * 4);
		spsr[row] = rd32(bspsr + b * 4);
	}
	int cur = gpsp_mode(m) & 0xF;
	spsr[cur] = rd32(st + 0x64);
	for (int i = 0; i < 7; ++i) reg_mode[cur][i] = reg[8 + i];
	/* timers: the reload, then the control written again */
	for (int t = 0; t < 4; ++t) {
		uint16_t ctl = rd16(st + 0x400 + 0x102 + t * 4);
		write_memory16(0x04000102 + t * 4, 0);
		write_memory16(0x04000100 + t * 4, rd16(st + 0x200 + t * 0x14));
		write_memory16(0x04000102 + t * 4, ctl);
	}
	/* sound's control (the FIFOs' timers) and the DMAs that wait on a timing */
	write_memory16(0x04000082, rd16(st + 0x400 + 0x82));
	write_memory16(0x04000084, rd16(st + 0x400 + 0x84));
	for (int d = 0; d < 4; ++d) {
		uint32_t base = 0xB0 + d * 12;
		uint16_t ctl = rd16(st + 0x400 + base + 10);
		write_memory16(0x04000000 + base + 10, 0);
		if (!(ctl & 0x8000) || ((ctl >> 12) & 3) == 0) continue;
		uint32_t src = rd32(st + 0x250 + d * 16), dst = rd32(st + 0x254 + d * 16);
		write_memory16(0x04000000 + base, (uint16_t)src); write_memory16(0x04000000 + base + 2, (uint16_t)(src >> 16));
		write_memory16(0x04000000 + base + 4, (uint16_t)dst); write_memory16(0x04000000 + base + 6, (uint16_t)(dst >> 16));
		write_memory16(0x04000000 + base + 8, rd16(st + 0x400 + base + 8));
		write_memory16(0x04000000 + base + 10, ctl);
	}
	printf("bench: state at pc %08x cpsr %08x\n", (unsigned)reg[15], (unsigned)cpsr);
	free(doc);
	free(st);
	return 1;
}
static void backend_frame(unsigned keys, int want_picture) {
	cur_keys = keys;
	retro_run();
	if (want_picture && last_fb) {
		for (int y = 0; y < 160; ++y) {
			const uint16_t *r = (const uint16_t *)((const uint8_t *)last_fb + (size_t)y * last_pitch);
			for (int x = 0; x < 240; ++x) {
				uint16_t p = r[x];   /* RGB565 */
				uint32_t R = (p >> 11) << 3, G = ((p >> 5) & 63) << 2, B = (p & 31) << 3;
				frame32[y * 240 + x] = R | G << 8 | B << 16;
			}
		}
	}
}
#endif

static int cmp_i(const void *a, const void *b) { int x = *(const int *)a, y = *(const int *)b; return (x > y) - (x < y); }

int main(int argc, char **argv) {
	(void)argc; (void)argv;
#ifdef PSP
	scePowerSetClockFrequency(333, 333, 166);
	printf("bench: cpu %d MHz, bus %d MHz\n", scePowerGetCpuClockFrequency(), scePowerGetBusClockFrequency());
#endif
	const char *dir = getenv("BENCH_DIR");
	if (!dir) dir = ".";
	char path[256];
	/* the run's settings, one per line: frames N, dump F (a picture each F frames), script ... */
	int frames = 3600, dump_every = 0;
	static char state[256] = "";
	static char scr[8192] = "";
	snprintf(path, sizeof path, "%s/bench.txt", dir);
	FILE *cf = fopen(path, "r");
	if (cf) {
		static char line[8192];
		while (fgets(line, sizeof line, cf)) {
			line[strcspn(line, "\r\n")] = 0;
			if (!strncmp(line, "frames ", 7)) frames = atoi(line + 7);
			else if (!strncmp(line, "dump ", 5)) dump_every = atoi(line + 5);
			else if (!strncmp(line, "state ", 6)) snprintf(state, sizeof state, "%.120s/%.120s", dir, line + 6);
			else if (!strncmp(line, "script ", 7)) snprintf(scr, sizeof scr, "%s", line + 7);
		}
		fclose(cf);
	}
	parse_script(scr);
	size_t len = 0, savlen = 0;
	snprintf(path, sizeof path, "%s/bn6.gba", dir);
	uint8_t *rom = slurp(path, &len);
	snprintf(path, sizeof path, "%s/bn6.sav", dir);
	uint8_t *sav = slurp(path, &savlen);
	if (!rom) { printf("bench: no ROM at %s/bn6.gba\n", dir); return 1; }
	printf("bench: %s, ROM %u bytes, save %u bytes, %d frames, %d steps\n", backend_name, (unsigned)len, (unsigned)savlen, frames, nsteps);
	int64_t t0 = now_us();
	if (!backend_init(rom, len, sav, savlen)) { printf("bench: init failed\n"); return 1; }
	printf("bench: init %lld us\n", (long long)(now_us() - t0));
	if (state[0]) printf("bench: state %s %s\n", state, backend_state(state) ? "loaded" : "NOT loaded");
#ifdef PSP
	{
		struct mallinfo mi = mallinfo();
		printf("bench: heap in use %d KB of an arena of %d KB\n", mi.uordblks / 1024, mi.arena / 1024);
	}
#endif
	int *us = malloc(sizeof(int) * (size_t)frames);
	int64_t start = now_us(), sec_start = start;
	for (int f = 0; f < frames; ++f) {
		int want = dump_every && (f % dump_every == 0);
		int64_t a = now_us();
		backend_frame(keys_at(f), want);
		int64_t b = now_us();
		us[f] = (int)(b - a);
		if (want) dump_ppm(dir, f);
		if ((f + 1) % 300 == 0) {
			int64_t t = now_us();
			int mx = 0; long sum = 0;
			for (int i = f - 299; i <= f; ++i) { sum += us[i]; if (us[i] > mx) mx = us[i]; }
			printf("bench: frames %5d-%5d mean %6.2f ms max %6.2f ms (wall %lld ms)\n", f - 299, f, sum / 300.0 / 1000.0, mx / 1000.0, (long long)((t - sec_start) / 1000));
			sec_start = t;
		}
	}
	int64_t total = now_us() - start;
	qsort(us, (size_t)frames, sizeof(int), cmp_i);
	printf("bench: %d frames in %lld ms: mean %.2f ms, median %.2f, p90 %.2f, p99 %.2f, max %.2f (a frame's budget 16.74 ms)\n",
	 frames, (long long)(total / 1000), total / 1000.0 / frames, us[frames / 2] / 1000.0, us[frames * 9 / 10] / 1000.0,
	 us[frames * 99 / 100] / 1000.0, us[frames - 1] / 1000.0);
#ifdef PSP
	sceKernelExitGame();
#endif
	return 0;
}
#ifdef PSP
/* (mGBA's file layer asks for it; the harness never truncates) */
#include <sys/types.h>
int ftruncate(int fd, off_t len) { (void)fd; (void)len; return -1; }
#endif
