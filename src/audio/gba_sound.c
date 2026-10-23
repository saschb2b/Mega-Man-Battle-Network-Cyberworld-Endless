/* The GBA's sound after MP2K (gba_sound.h), as mGBA 0.10.5 plays it: its
 * src/gba/io.c's register writes (each byte its register's, the values kept
 * masked as the game reads them back), src/gba/audio.c's GBAAudioSample and
 * GBAAudioSampleFIFO (a 65536 Hz slot sees every change made before its
 * end; the FIFOs' samples held from one timer 0 overflow to the next; the
 * PSG shifted by its ratio, a FIFO's sample times 4, the bias, 10 bits,
 * times 48), its blip_buf at the GBA's clock. The PSG is mGBA's own. */
#include "gba_sound.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/blip_buf.h>
#include <mgba/core/timing.h>
#include <mgba/internal/gb/audio.h>

#define CLOCK 16777216
#define FRAME 280896        /* cycles a GBA frame */
#define SLOT 256            /* an output sample: SOUNDBIAS's resolution 1, 65536 Hz */
#define TIMER 1596          /* timer 0's period: a DirectSound sample, 10512 Hz */
#define SEQUENCER 32768     /* the PSG's frame sequencer, 512 Hz (FRAME_CYCLES times the GBA's factor 4) */
#define BLIP_CHUNK 0x800    /* mGBA's CLOCKS_PER_FRAME */
#define OUT_RATE 48000
#define DS_MAX 704          /* a frame's DirectSound samples at most */

static struct {
	bool ready;
	struct mTiming timing;
	int32_t relative, next;
	struct GBAudio psg;
	uint16_t io[0x28];      /* 0x60-0xAF, by halfwords, as the game reads them */
	bool enable;
	int psg_shift;          /* 4 - SOUNDCNT_H's PSG ratio */
	bool a_full, b_full, a_right, a_left, b_right, b_left;
	int bias;
	uint32_t frame_start, slot_end;
	blip_t *left, *right;
	int blip_clock;
	int16_t last_left, last_right;
	int mute;
	int8_t held_right, held_left;   /* the last frame's last sample, till the first overflow */
	uint32_t phase;   /* timer 0's first overflow after a frame's start (the differential's) */
} G;

static uint16_t *reg16(int reg) { return &G.io[(reg - 0x60) >> 1]; }

/* the frame sequencer, as the GBA's _updateFrame keeps it */
static void sequencer(struct mTiming *timing, void *user, uint32_t late) {
	(void)user;
	mTimingSchedule(timing, &G.psg.frameEvent, SEQUENCER - (int32_t)late);
	GBAudioUpdateFrame(&G.psg);
}

/* ---- the registers ---- */

static void control(uint16_t v) {
	G.psg_shift = 4 - (v & 3);
	G.a_full = v >> 2 & 1;
	G.b_full = v >> 3 & 1;
	G.a_right = v >> 8 & 1;
	G.a_left = v >> 9 & 1;
	G.b_right = v >> 12 & 1;
	G.b_left = v >> 13 & 1;
}

/* GBAIOWrite's sound registers: the register's call, its value kept */
static void write16(int reg, uint16_t v) {
	struct GBAudio *p = &G.psg;
	uint16_t keep = v;
	switch (reg) {
	case 0x60: GBAudioWriteNR10(p, (uint8_t)v); keep = v & 0x007F; break;
	case 0x62: GBAudioWriteNR11(p, (uint8_t)v); GBAudioWriteNR12(p, (uint8_t)(v >> 8)); keep = v & 0xFFC0; break;
	case 0x64: GBAudioWriteNR13(p, (uint8_t)v); GBAudioWriteNR14(p, (uint8_t)(v >> 8)); keep = v & 0x4000; break;
	case 0x68: GBAudioWriteNR21(p, (uint8_t)v); GBAudioWriteNR22(p, (uint8_t)(v >> 8)); keep = v & 0xFFC0; break;
	case 0x6C: GBAudioWriteNR23(p, (uint8_t)v); GBAudioWriteNR24(p, (uint8_t)(v >> 8)); keep = v & 0x4000; break;
	case 0x70:
		p->ch3.size = GBAudioRegisterBankGetSize((uint8_t)v);
		p->ch3.bank = GBAudioRegisterBankGetBank((uint8_t)v);
		GBAudioWriteNR30(p, (uint8_t)v);
		keep = v & 0x00E0;
		break;
	case 0x72: GBAudioWriteNR31(p, (uint8_t)v); p->ch3.volume = GBAudioRegisterBankVolumeGetVolumeGBA((uint8_t)(v >> 8)); keep = v & 0xE000; break;
	case 0x74: GBAudioWriteNR33(p, (uint8_t)v); GBAudioWriteNR34(p, (uint8_t)(v >> 8)); keep = v & 0x4000; break;
	case 0x78: GBAudioWriteNR41(p, (uint8_t)v); GBAudioWriteNR42(p, (uint8_t)(v >> 8)); keep = v & 0xFF00; break;
	case 0x7C: GBAudioWriteNR43(p, (uint8_t)v); GBAudioWriteNR44(p, (uint8_t)(v >> 8)); keep = v & 0x40FF; break;
	case 0x80: GBAudioWriteNR50(p, (uint8_t)v); GBAudioWriteNR51(p, (uint8_t)(v >> 8)); keep = v & 0xFF77; break;
	case 0x82: control(v); keep = v & 0x770F; break;
	case 0x84:
		G.enable = v >> 7 & 1;
		GBAudioWriteNR52(p, (uint8_t)v);
		keep = (uint16_t)((v & 0x80) | (*reg16(0x84) & 0xF));
		break;
	case 0x88: keep = v & 0xC3FE; G.bias = keep & 0x3FE; break;
	default: break;
	}
	*reg16(reg) = keep;
}

/* GBAIOWrite8: the PSG's bytes each their own register, the rest a
 * halfword with its other byte as it is */
void gba_sound_write8(int reg, uint8_t v) {
	struct GBAudio *p = &G.psg;
	uint16_t *r = reg16(reg & ~1);
	switch (reg) {
	case 0x62: GBAudioWriteNR11(p, v); *r = (uint16_t)((*r & 0xFF00) | (v & 0xC0)); return;
	case 0x63: GBAudioWriteNR12(p, v); *r = (uint16_t)((*r & 0x00C0) | v << 8); return;
	case 0x64: GBAudioWriteNR13(p, v); return;
	case 0x65: GBAudioWriteNR14(p, v); *r = (uint16_t)((v & 0x40) << 8); return;
	case 0x68: GBAudioWriteNR21(p, v); *r = (uint16_t)((*r & 0xFF00) | (v & 0xC0)); return;
	case 0x69: GBAudioWriteNR22(p, v); *r = (uint16_t)((*r & 0x00C0) | v << 8); return;
	case 0x6C: GBAudioWriteNR23(p, v); return;
	case 0x6D: GBAudioWriteNR24(p, v); *r = (uint16_t)((v & 0x40) << 8); return;
	case 0x72: GBAudioWriteNR31(p, v); return;
	case 0x73: p->ch3.volume = GBAudioRegisterBankVolumeGetVolumeGBA(v); *r = (uint16_t)((v & 0xE0) << 8); return;
	case 0x74: GBAudioWriteNR33(p, v); return;
	case 0x75: GBAudioWriteNR34(p, v); *r = (uint16_t)((v & 0x40) << 8); return;
	case 0x78: GBAudioWriteNR41(p, v); return;
	case 0x79: GBAudioWriteNR42(p, v); *r = (uint16_t)(v << 8); return;
	case 0x7C: GBAudioWriteNR43(p, v); *r = (uint16_t)((*r & 0x4000) | v); return;
	case 0x7D: GBAudioWriteNR44(p, v); *r = (uint16_t)((*r & 0x00FF) | (v & 0x40) << 8); return;
	default: break;
	}
	int shift = 8 * (reg & 1);
	write16(reg & ~1, (uint16_t)((v << shift) | (*r & ~(0xFF << shift))));
}

/* GBAAudioWriteWaveRAM: into the bank not playing */
void gba_sound_write32(int reg, uint32_t v) {
	int bank = G.enable ? !G.psg.ch3.bank : 1;
	GBAudioRun(&G.psg, mTimingCurrentTime(&G.timing), 0x4);
	G.psg.ch3.wavedata32[((reg - 0x90) >> 2) | (bank * 4)] = v;
}

uint8_t gba_sound_read8(int reg) { return (uint8_t)(*reg16(reg & ~1) >> (8 * (reg & 1))); }

void gba_sound_reset(void) {
	if (G.ready) {
		GBAudioDeinit(&G.psg);
		blip_delete(G.left);
		blip_delete(G.right);
	}
	memset(&G, 0, sizeof G);
	mTimingInit(&G.timing, &G.relative, &G.next);
	G.next = 0x7FFFFFFF;
	GBAudioInit(&G.psg, 0, (uint8_t *)reg16(0x84), GB_AUDIO_GBA);
	G.psg.timing = &G.timing;
	G.psg.clockRate = CLOCK;
	G.psg.frameEvent.callback = sequencer;
	G.psg.frameEvent.context = NULL;
	GBAudioReset(&G.psg);
	mTimingSchedule(&G.timing, &G.psg.frameEvent, 0);
	G.left = blip_new(0x4000);
	G.right = blip_new(0x4000);
	blip_set_rates(G.left, CLOCK, OUT_RATE);
	blip_set_rates(G.right, CLOCK, OUT_RATE);
	G.psg_shift = 4;
	G.slot_end = SLOT;
	/* (CYBERWORLD_SOUND_MUTE: channels left out, as romlab's mute: the
	 * differential's parts heard alone) */
	const char *mute = getenv("CYBERWORLD_SOUND_MUTE");
	G.mute = mute ? (int)strtol(mute, NULL, 16) : 0;
	const char *phase = getenv("CYBERWORLD_SOUND_PHASE");
	G.phase = phase ? (uint32_t)atoi(phase) % TIMER : 0;
	for (int i = 0; i < 4; ++i) G.psg.forceDisableCh[i] = G.mute >> (2 + i) & 1;
	G.ready = true;
	/* (SoundInit: the sound on, DirectSound A right and B left at 100%, the
	 * PSG at 100%, the bias at 0x200 and 65536 Hz) */
	write16(0x84, 0x008F);
	write16(0x82, 0xA90E);
	write16(0x88, 0x4200);
	/* (MPlayExtender) */
	write16(0x84, 0x008F);
	write16(0x80, 0);
	gba_sound_write8(0x63, 8);
	gba_sound_write8(0x69, 8);
	gba_sound_write8(0x79, 8);
	gba_sound_write8(0x65, 0x80);
	gba_sound_write8(0x6D, 0x80);
	gba_sound_write8(0x7D, 0x80);
	gba_sound_write8(0x70, 0);
	gba_sound_write8(0x80, 0x77);
}

/* ---- the output ---- */

/* _applyBias: the bias, 10 bits, the master volume (0x100) times 3, >> 4 */
static int16_t biased(int s) {
	s += G.bias;
	if (s >= 0x400) s = 0x3FF;
	else if (s < 0) s = 0;
	return (int16_t)(((s - G.bias) * 0x100 * 3) >> 4);
}

static void slot(int ds_right, int ds_left) {
	int16_t pl = 0, pr = 0;
	GBAudioRun(&G.psg, (int32_t)(G.slot_end - SLOT), 0xF);
	GBAudioSamplePSG(&G.psg, &pl, &pr);
	int l = pl >> G.psg_shift, r = pr >> G.psg_shift;
	int a = G.mute & 1 ? 0 : (ds_right * 4) >> !G.a_full, b = G.mute & 2 ? 0 : (ds_left * 4) >> !G.b_full;
	if (G.a_left) l += a;
	if (G.a_right) r += a;
	if (G.b_left) l += b;
	if (G.b_right) r += b;
	int16_t ol = biased(l), or_ = biased(r);
	blip_add_delta(G.left, (unsigned)G.blip_clock, ol - G.last_left);
	blip_add_delta(G.right, (unsigned)G.blip_clock, or_ - G.last_right);
	G.last_left = ol;
	G.last_right = or_;
	G.blip_clock += SLOT;
	if (G.blip_clock >= BLIP_CHUNK) {
		blip_end_frame(G.left, BLIP_CHUNK);
		blip_end_frame(G.right, BLIP_CHUNK);
		G.blip_clock -= BLIP_CHUNK;
	}
}

/* the timing moved on to `to` (what is due before it done) */
static void tick_to(uint32_t to) {
	int32_t by = (int32_t)(to - (uint32_t)mTimingCurrentTime(&G.timing));
	if (by > 0) mTimingTick(&G.timing, by);
}

void gba_sound_frame(const int8_t *right, const int8_t *left, int n) {
	if (!G.ready) return;
	uint32_t end = G.frame_start + FRAME;
	if (n > DS_MAX) n = DS_MAX;
	while ((int32_t)(G.slot_end - end) <= 0) {
		tick_to(G.slot_end - 1);
		/* (the FIFOs' sample: the last timer 0 overflow before the slot's end) */
		int32_t since = (int32_t)(G.slot_end - 1 - G.frame_start - G.phase);
		int j = since < 0 ? -1 : since / TIMER;
		if (j >= n) j = n - 1;
		slot(j >= 0 ? right[j] : G.held_right, j >= 0 ? left[j] : G.held_left);
		if (j == n - 1) { G.held_right = right[j]; G.held_left = left[j]; }
		G.slot_end += SLOT;
	}
	tick_to(end);
	G.frame_start = end;
}

int gba_sound_avail(void) { return G.ready ? blip_samples_avail(G.left) : 0; }

int gba_sound_read(int16_t *out, int frames) {
	if (!G.ready) return 0;
	int got = blip_read_samples(G.left, out, frames, 1);
	blip_read_samples(G.right, out + 1, got, 1);
	return got;
}
