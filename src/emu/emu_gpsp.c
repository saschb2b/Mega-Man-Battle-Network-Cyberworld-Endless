/* The GBA core on gpSP (CW_GPSP: the PSP's build, and a computer's with
 * CYBERWORLD_CORE=gpsp to test it): emu.h as emu.c gives it on mGBA, over
 * libretro gpSP with the game's additions (docker/gpsp-cyberworld.patch,
 * docs/engine/emulation/psp-and-ds.md).
 *
 * The ROM is the player's bytes themselves (R.data), paged 32 KB at a time
 * (cw.h): a page the engine writes is copied first, and the free space past
 * the game is one shared page of 0xFF until a page of it is written, so the
 * ROM is held once where mGBA kept a 16 MB copy beside it. The hooks are a
 * table the translator reads (hook_gpsp.c): the ROM never holds a BKPT. Its
 * sound goes through the same ring and pace as emu.c's, its states are
 * gpSP's own with the cart's save RAM after them. One core, run on the
 * game's thread. */
#include "emu_gpsp.h"

#include "emu.h"

#ifdef CW_GPSP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <cw.h>
#include <libretro.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/log.h>

#include "claims.h"
#include "compat.h"
#include "emu_keep.h"
#include "emu_sound.h"
#include "free_space_watch.h"
#include "hook_gpsp.h"
#include "platform.h"
#include "save_queue.h"
#include "sound_quiet.h"
#include "xchips_pack.h"
#include "xinstall_log.h"

/* ---- gpSP's own memory, read and written in place (its gba_memory.h) ---- */
extern uint32_t reg[64];
extern uint8_t ewram[1024 * 256 * 2];   /* the work RAM, then its code's marks */
extern uint8_t iwram[1024 * 32 * 2];    /* the internal RAM's code marks, then the RAM */
extern uint8_t vram[1024 * 96];
extern uint8_t *memory_map_read[8 * 1024];
uint32_t read_memory8(uint32_t address);
uint32_t read_memory16(uint32_t address);
uint32_t read_memory32(uint32_t address);
uint8_t write_memory8(uint32_t address, uint8_t value);
uint8_t write_memory16(uint32_t address, uint16_t value);
uint8_t write_memory32(uint32_t address, uint32_t value);
void flush_translation_cache_ram(void);

#define PAGE 0x8000u

static bool ready;
static uint32_t video[EMU_W * EMU_H];   /* ABGR, as mGBA draws it (emu_picture.c) */
static uint32_t pictures;
static int out_rate = 48000;
static unsigned cur_keys;
static void (*before_frame)(void);

/* ---- the ROM: the player's bytes, a page copied where written ---- */
static uint8_t ff_page[PAGE];            /* the free space not yet written */
static uint8_t *owned[CW_PAGES];         /* the pages copied to be written */
static int nowned;
static bool rom_written, ram_code_written;

/* the page of ROM address a, copied first where it is the player's or the
 * free space's shared one (NULL where memory ran out) */
static uint8_t *rom_write_at(uint32_t a) {
	uint32_t i = (a & (EMU_ROM_SIZE - 1)) / PAGE;
	if (!owned[i]) {
		uint8_t *p = malloc(PAGE);
		if (!p) return NULL;
		memcpy(p, cw_rom_pages[i], PAGE);
		owned[i] = p;
		++nowned;
		cw_rom_page_set(i, p);
	}
	rom_written = true;
	return owned[i] + (a & (PAGE - 1));
}

/* ---- raw reads and writes, as mGBA's rawRead and rawWrite go: no bus
 * rules (a byte into VRAM is that byte), the I/O through its registers ---- */
static bool is_rom(uint32_t a) { return a >= 0x08000000u && a < 0x0E000000u; }

static uint8_t *ram_at(uint32_t a) {
	switch (a >> 24) {
	case 0x02: return ewram + (a & 0x3FFFFu);
	case 0x03: return iwram + 0x8000u + (a & 0x7FFFu);
	case 0x06: {
		uint32_t o = a & 0x1FFFFu;
		return vram + (o >= 0x18000u ? o - 0x8000u : o);
	}
	default: return NULL;
	}
}

/* (a write over translated code in the work RAM: its translations dropped
 * before the next frame, as gpSP's own stores do at once) */
static void ram_code_check(uint32_t a) {
	const uint8_t *mark = (a >> 24) == 0x02 ? ewram + 0x40000u + (a & 0x3FFFEu) : (a >> 24) == 0x03 ? iwram + (a & 0x7FFEu) : NULL;
	if (mark && (mark[0] | mark[1])) ram_code_written = true;
}

static uint8_t raw_read8(uint32_t a) {
	if (is_rom(a)) return memory_map_read[a >> 15][a & (PAGE - 1)];
	uint8_t *p = ram_at(a);
	if (p) return *p;
	if ((a >> 24) == 0x0E) {
		const uint8_t *s = retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
		size_t n = retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
		return s && n ? s[(a & 0xFFFFu) % n] : 0xFF;
	}
	return (uint8_t)read_memory8(a);
}

static void raw_write8(uint32_t a, uint8_t v) {
	if (is_rom(a)) {
		uint8_t *p = rom_write_at(a);
		if (p) *p = v;
		return;
	}
	uint8_t *p = ram_at(a);
	if (p) {
		*p = v;
		ram_code_check(a);
		return;
	}
	uint32_t z = a >> 24;
	if (z == 0x05 || z == 0x07) {
		/* (the palette and OAM by halfwords, through gpSP's stores: its
		 * converted palette and its OAM's sort follow them) */
		uint16_t h = (uint16_t)read_memory16(a & ~1u);
		h = (a & 1) ? (uint16_t)((h & 0xFF) | v << 8) : (uint16_t)((h & 0xFF00) | v);
		write_memory16(a & ~1u, h);
		return;
	}
	if (z == 0x0E) {
		uint8_t *s = retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
		size_t n = retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
		if (s && n) s[(a & 0xFFFFu) % n] = v;
		return;
	}
	write_memory8(a, v);
}

/* ---- sound: gpSP's samples through a resampler into the ring, as emu.c's
 * mGBA sound goes (emu_sound.h) ---- */
static blip_t *blip[2];
static int16_t last[2];
static unsigned clocked;   /* the frame's samples so far */
static double in_rate = 32768;
static unsigned frame_how;

static void sample(int16_t l, int16_t r) {
	if (!blip[0]) return;
	if (l != last[0]) { blip_add_delta(blip[0], clocked, l - last[0]); last[0] = l; }
	if (r != last[1]) { blip_add_delta(blip[1], clocked, r - last[1]); last[1] = r; }
	++clocked;
}
static void cb_sample(int16_t l, int16_t r) { sample(l, r); }
static size_t cb_samples(const int16_t *d, size_t n) {
	for (size_t i = 0; i < n; ++i) sample(d[i * 2], d[i * 2 + 1]);
	return n;
}

static void pull_audio(void) {
	if (!blip[0] || !emu_sound_ready()) return;
	blip_end_frame(blip[0], clocked);
	blip_end_frame(blip[1], clocked);
	clocked = 0;
	int16_t tmp[1024 * 2];
	int got;
	while ((got = blip_samples_avail(blip[0])) > 0) {
		if (got > 1024) got = 1024;
		blip_read_samples(blip[0], tmp, got, 1);
		blip_read_samples(blip[1], tmp + 1, got, 1);
		/* (a frame fast forward does not sound: its sound dropped) */
		if (!(frame_how & EMU_FRAME_UNHEARD)) emu_sound_put(tmp, got);
	}
	double adj = emu_sound_settle();
	blip_set_rates(blip[0], in_rate, out_rate * adj);
	blip_set_rates(blip[1], in_rate, out_rate * adj);
}

void emu_audio_rate(int rate) {
	out_rate = rate;
	for (int k = 0; k < 2; ++k)
		if (blip[k]) blip_set_rates(blip[k], in_rate, rate);
}

/* (the guest cores', which a build on gpSP never makes: emu.c pulls theirs) */
void emu_audio_from(void *core) { (void)core; }
void emu_audio_drop_from(void *core) { (void)core; }

int emu_audio_out_rate(void) { return out_rate; }

/* ---- libretro's calls ---- */
static void cb_video(const void *data, unsigned w, unsigned h, size_t pitch) {
	if (!data || w < EMU_W || h < EMU_H) return;
	for (int y = 0; y < EMU_H; ++y) {
		const uint16_t *row = (const uint16_t *)((const uint8_t *)data + (size_t)y * pitch);
		uint32_t *out = video + y * EMU_W;
		for (int x = 0; x < EMU_W; ++x) {
			uint16_t p = row[x];   /* RGB565 */
			uint32_t r = (uint32_t)(p >> 11) << 3, g = (uint32_t)(p >> 5 & 63) << 2, b = (uint32_t)(p & 31) << 3;
			out[x] = 0xFF000000u | (b | b >> 5) << 16 | (g | g >> 6) << 8 | (r | r >> 5);
		}
	}
}

static void cb_poll(void) {}

static int16_t cb_input(unsigned port, unsigned device, unsigned index, unsigned id) {
	(void)device; (void)index;
	if (port) return 0;
	static const unsigned key_of[16] = {
		[RETRO_DEVICE_ID_JOYPAD_A] = KEY_A, [RETRO_DEVICE_ID_JOYPAD_B] = KEY_B,
		[RETRO_DEVICE_ID_JOYPAD_SELECT] = KEY_SELECT, [RETRO_DEVICE_ID_JOYPAD_START] = KEY_START,
		[RETRO_DEVICE_ID_JOYPAD_RIGHT] = KEY_RIGHT, [RETRO_DEVICE_ID_JOYPAD_LEFT] = KEY_LEFT,
		[RETRO_DEVICE_ID_JOYPAD_UP] = KEY_UP, [RETRO_DEVICE_ID_JOYPAD_DOWN] = KEY_DOWN,
		[RETRO_DEVICE_ID_JOYPAD_R] = KEY_R, [RETRO_DEVICE_ID_JOYPAD_L] = KEY_L,
	};
	if (id == RETRO_DEVICE_ID_JOYPAD_MASK) {
		int16_t m = 0;
		for (unsigned i = 0; i < 16; ++i)
			if (key_of[i] && (cur_keys & key_of[i])) m = (int16_t)(m | 1 << i);
		return m;
	}
	return id < 16 && key_of[id] && (cur_keys & key_of[id]) ? 1 : 0;
}

static const char *system_dir = ".";
static bool cb_environment(unsigned cmd, void *data) {
	switch (cmd) {
	case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
	case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: *(const char **)data = system_dir; return true;
	case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: return *(const enum retro_pixel_format *)data == RETRO_PIXEL_FORMAT_RGB565;
	case RETRO_ENVIRONMENT_GET_VARIABLE: {
		/* (gpSP's own BIOS, the game booted as the player's cart starts it, the
		 * dynarec on, every frame drawn) */
		struct retro_variable *v = data;
		v->value = !strcmp(v->key, "gpsp_bios") ? "builtin" : !strcmp(v->key, "gpsp_boot_mode") ? "game" :
			!strcmp(v->key, "gpsp_drc") ? "enabled" : !strcmp(v->key, "gpsp_frameskip") ? "disabled" : NULL;
		return v->value != NULL;
	}
	case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: *(bool *)data = false; return true;
	case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS: return true;
	case RETRO_ENVIRONMENT_GET_CAN_DUPE: *(bool *)data = true; return true;
	default: return false;
	}
}

/* ---- the core ---- */
static void quiet(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a) {
	(void)l; (void)c; (void)lv; (void)f; (void)a;
}
static struct mLogger logger = { .log = quiet };
void emu_log_quiet(void) { mLogSetDefaultLogger(&logger); }

bool emu_init(const uint8_t *rom, size_t len) {
	if (ready) return true;
	if (len > EMU_ROM_SIZE || len % PAGE) return false;
	/* the player's pages as they are, read alone (gpSP never writes its ROM's
	 * pages), and the free space past them; the first megabyte one run of
	 * them as gpSP loads the game, which reads its header and looks for its
	 * save's kind there */
	memset(ff_page, 0xFF, sizeof ff_page);
	for (uint32_t i = 0; i < CW_PAGES; ++i) {
		union { const uint8_t *c; uint8_t *v; } p = { rom + (size_t)i * PAGE };
		cw_rom_pages[i] = (size_t)(i + 1) * PAGE <= len ? p.v : ff_page;
	}
	cw_mode = true;
	cw_hooked = hook_gpsp_hooked;
	retro_set_environment(cb_environment);
	retro_set_video_refresh(cb_video);
	retro_set_audio_sample(cb_sample);
	retro_set_audio_sample_batch(cb_samples);
	retro_set_input_poll(cb_poll);
	retro_set_input_state(cb_input);
	retro_init();
	struct retro_game_info game = { .path = "bn6.gba", .data = NULL, .size = 0, .meta = NULL };
	if (!retro_load_game(&game)) {
		fprintf(stderr, "emu: gpSP did not load the game\n");
		return false;
	}
	/* (the first page copied, as gpSP writes a cart's clock registers into
	 * it where the cart has one: the player's bytes stay theirs) */
	if (!rom_write_at(0x08000000u)) return false;
	rom_written = false;
	struct retro_system_av_info av;
	retro_get_system_av_info(&av);
	in_rate = av.timing.sample_rate > 0 ? av.timing.sample_rate : 32768;
	blip[0] = blip_new(4096);
	blip[1] = blip_new(4096);
	emu_audio_rate(out_rate);
	emu_audio_open();
	xchips_pack_ready(NULL);
	ready = true;
	printf("emu: the GBA core is gpSP (its dynarec), the ROM held once\n");
	return true;
}

bool emu_ready(void) { return ready; }
void emu_reset(void) {
	if (ready) retro_reset();
}
bool emu_threaded(void) { return false; }
void emu_sync(void) {}
void emu_quit(void) {}
void emu_before_frame(void (*step)(void)) { before_frame = step; }
bool emu_drawing;
int emu_draw_waits;
uint64_t emu_core_ticks, emu_core_unshown_ticks;
int emu_core_unshown;

bool emu_hook(uint32_t addr, EmuHook fn, void *user) {
	bool ok = ready && hook_add(addr, fn, user);
	if (ok) claims_hook(addr);
	if (ok) xlog_hook(addr, fn, user);
	return ok;
}

bool emu_hook_event(uint32_t addr, int kind) {
	bool ok = ready && hook_add_event(addr, kind);
	if (ok) claims_hook(addr);
	if (ok) xlog_event(addr);
	return ok;
}

void emu_unhook(uint32_t addr) {
	if (ready) hook_remove(addr);
}
int emu_hook_events(HookEvent *out, int max) { return ready ? hook_drain(out, max) : 0; }

void emu_frame(uint32_t keys) { emu_frame_as(keys, 0); }

void emu_frame_as(uint32_t keys, unsigned how) {
	if (!ready) return;
	frame_how = how;
	sound_quiet_frame();
	if (before_frame) before_frame();
	/* (what the engine changed since the last frame, the ROM's code or the
	 * hooks or code in the work RAM: its translations made afresh) */
	if (rom_written || hook_gpsp_changed()) cw_flush();
	else if (ram_code_written) flush_translation_cache_ram();
	rom_written = ram_code_written = false;
	cur_keys = keys;
	uint64_t t0 = SDL_GetPerformanceCounter();
	retro_run();
	uint64_t dt = SDL_GetPerformanceCounter() - t0;
	emu_core_ticks += dt;
	pull_audio();
	++pictures;
}

const uint32_t *emu_video(void) { return video; }
uint32_t emu_pictures(void) { return pictures; }

uint8_t emu_read8(uint32_t a) { return ready ? raw_read8(a) : 0; }
uint16_t emu_read16(uint32_t a) { return (uint16_t)(emu_read8(a) | emu_read8(a + 1) << 8); }
uint32_t emu_read32(uint32_t a) { return (uint32_t)emu_read16(a) | (uint32_t)emu_read16(a + 2) << 16; }
void emu_read_bytes(uint32_t a, uint8_t *out, uint32_t len) {
	for (uint32_t i = 0; i < len; ++i) out[i] = emu_read8(a + i);
}

void emu_write8(uint32_t a, uint8_t v) {
	claims_note(a, 1);
	free_space_write(a, 1);
	xlog_write(a, &v, 1);
	if (ready) raw_write8(a, v);
}

void emu_write32(uint32_t a, uint32_t v) {
	claims_note(a, 4);
	free_space_write(a, 4);
	const uint8_t b[4] = { (uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24) };
	xlog_write(a, b, sizeof b);
	/* (the I/O as one word, as the game writes it: a byte at a time would
	 * act on each half) */
	if (ready && (a >> 24) == 0x04) { write_memory32(a, v); return; }
	for (int i = 0; ready && i < 4; ++i) raw_write8(a + (uint32_t)i, b[i]);
}

void emu_write(uint32_t a, const void *data, size_t len) {
	const uint8_t *p = data;
	claims_note(a, (uint32_t)len);
	free_space_write(a, (uint32_t)len);
	xlog_write(a, data, len);
	for (size_t i = 0; ready && i < len; ++i) raw_write8(a + (uint32_t)i, p[i]);
}

/* (the pages the engine copied to write, a line of frame_log's: the ROM's
 * memory beyond the player's own bytes) */
size_t emu_gpsp_rom_owned(void) { return (size_t)nowned * PAGE; }

uint8_t *emu_gpsp_save_ram(size_t *size) {
	*size = ready ? retro_get_memory_size(RETRO_MEMORY_SAVE_RAM) : 0;
	return ready ? retro_get_memory_data(RETRO_MEMORY_SAVE_RAM) : NULL;
}

/* ---- RAM kept patched (emu_keep, emu_keep.h) ---- */
static void keep_put(bool own) { emu_keep_put(own, raw_read8, raw_write8); }

bool emu_keep(uint32_t addr, const void *was, const void *now, size_t len) {
	if (!ready || !emu_keep_add(addr, was, now, len)) return false;
	claims_note(addr, (uint32_t)len);
	keep_put(false);
	return true;
}

/* ---- states: gpSP's own, the cart's save RAM after them ---- */
#define STATE_MAGIC 0x53475743u   /* "CWGS": a state of the game on gpSP */
typedef struct { uint32_t magic, state, save; } StateHead;

/* the whole state in memory (*size bytes): its head, gpSP's state, the save RAM */
static uint8_t *state_bytes(size_t *size) {
	size_t n = retro_serialize_size(), s = retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
	uint8_t *b = malloc(sizeof(StateHead) + n + s);
	if (!b) return NULL;
	StateHead head = { STATE_MAGIC, (uint32_t)n, (uint32_t)s };
	memcpy(b, &head, sizeof head);
	/* (with the game's own bytes where the engine keeps RAM patched: a
	 * state holds the game's RAM, whatever build loads it) */
	keep_put(true);
	bool ok = retro_serialize(b + sizeof head, n);
	keep_put(false);
	const uint8_t *save = retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
	if (!ok || (s && !save)) { free(b); return NULL; }
	if (s) memcpy(b + sizeof head + n, save, s);
	*size = sizeof head + n + s;
	return b;
}

static bool state_from(const uint8_t *b, size_t size) {
	StateHead head;
	if (size < sizeof head) return false;
	memcpy(&head, b, sizeof head);
	size_t s = retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
	/* (a state of mGBA's, another system's, is not one of gpSP's) */
	if (head.magic != STATE_MAGIC || head.state != retro_serialize_size() || head.save != s ||
		size < sizeof head + head.state + head.save) return false;
	if (!retro_unserialize(b + sizeof head, head.state)) return false;
	uint8_t *save = retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
	if (s && save) memcpy(save, b + sizeof head + head.state, s);
	keep_put(false);
	sound_quiet_loaded();
	xchips_pack_ready(NULL);
	return true;
}

bool emu_save_state(const char *path) {
	size_t n = 0;
	uint8_t *b = ready ? state_bytes(&n) : NULL;
	if (!b) return false;
	char tmp[640];
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *f = fopen(tmp, "wb");
	bool ok = f && fwrite(b, 1, n, f) == n;
	if (f && fclose(f)) ok = false;
	free(b);
	if (ok) ok = cw_rename(tmp, path);
	else remove(tmp);
	platform_persist();
	return ok;
}

bool emu_save_state_queued(const char *path) {
	size_t n = 0;
	uint8_t *b = ready ? state_bytes(&n) : NULL;
	if (!b) return false;
	if (save_queue_file(path, b, n)) { platform_persist(); return true; }
	free(b);
	save_queue_wait();
	return emu_save_state(path);
}

bool emu_load_state(const char *path) {
	save_queue_wait();
	if (!ready) return false;
	FILE *f = fopen(path, "rb");
	if (!f) return false;
	fseek(f, 0, SEEK_END);
	long n = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t *b = n > 0 ? malloc((size_t)n) : NULL;
	bool ok = b && fread(b, 1, (size_t)n, f) == (size_t)n;
	fclose(f);
	ok = ok && state_from(b, (size_t)n);
	free(b);
	return ok;
}

size_t emu_state_size(void) { return ready ? retro_serialize_size() : 0; }
bool emu_state_save(void *state) { return ready && retro_serialize(state, retro_serialize_size()); }
bool emu_state_load(const void *state) {
	if (!ready || !retro_unserialize(state, retro_serialize_size())) return false;
	keep_put(false);
	sound_quiet_loaded();
	return true;
}

void *emu_snapshot(size_t *size) { return ready ? state_bytes(size) : NULL; }
bool emu_snapshot_load(void *bytes, size_t size) { return ready && bytes && state_from(bytes, size); }

#endif
