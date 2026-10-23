/* --mp2k-trace (mp2k_trace.h): the engine's MP2K frame by frame, laid out
 * as BN6 keeps it (docs/ROM_DATA.md, BN6's sound engine). */
#include "mp2k_trace.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "gba_sound.h"
#include "mp2k.h"
#include "mp2k_state.h"

#define SI_SIZE 0xFB0
#define RECORD (SI_SIZE + 0x100 + 0x40 + 0x280)
#define MUSIC 31   /* the music player, BN6_MUSIC_PLAYER */

static void put32(uint8_t *b, uint32_t v) {
	for (int i = 0; i < 4; ++i) b[i] = (uint8_t)(v >> (8 * i));
}

static uint32_t chan_addr(int c) {
	if (c == MP2K_NONE) return 0;
	return c < MP2K_DS_MAX ? BN6_SOUND_CHANS + (uint32_t)c * 0x40 : BN6_SOUND_CGB_CHANS + (uint32_t)(c - MP2K_DS_MAX) * 0x40;
}

static uint32_t track_addr(int t) { return t == MP2K_NONE ? 0 : M.tracks[t].addr; }

static void put_chan(uint8_t *b, const Mp2kChan *c, bool cgb) {
	const uint8_t head[] = { c->status, c->type, c->right, c->left, c->attack, c->decay, c->sustain, c->release, c->key, c->env,
		c->env_right, c->env_left, c->echo_volume, c->echo_length };
	memcpy(b, head, sizeof head);
	b[0x10] = c->gate;
	b[0x11] = c->midi_key;
	b[0x12] = c->velocity;
	b[0x13] = c->priority;
	b[0x14] = (uint8_t)c->rhythm_pan;
	if (cgb) {
		const uint8_t tail[] = { c->sustain_goal, c->n4, c->pan, c->pan_mask, c->modify, c->length, c->sweep };
		memcpy(b + 0x19, tail, sizeof tail);
	} else {
		put32(b + 0x18, c->count);
		put32(b + 0x1C, c->fw);
	}
	put32(b + 0x20, c->frequency);
	put32(b + 0x24, c->wav);
	put32(b + 0x28, c->cur);
	put32(b + 0x2C, track_addr(c->track));
	put32(b + 0x30, chan_addr(c->prev));
	put32(b + 0x34, chan_addr(c->next));
}

static void put_track(uint8_t *b, const Mp2kTrack *t) {
	const uint8_t head[] = { t->flags, t->wait, t->pattern_level, t->repeat, t->gate, t->key, t->velocity, t->running,
		(uint8_t)t->key_m, t->pit_m, (uint8_t)t->key_shift, (uint8_t)t->key_shift_x, (uint8_t)t->tune, t->pit_x, (uint8_t)t->bend,
		t->bend_range, t->vol_mr, t->vol_ml, t->vol, t->vol_x, (uint8_t)t->pan, (uint8_t)t->pan_x, (uint8_t)t->mod_m, t->mod,
		t->mod_t, t->lfo_speed, t->lfo_speed_c, t->lfo_delay, t->lfo_delay_c, t->priority, t->echo_volume, t->echo_length };
	memcpy(b, head, sizeof head);
	put32(b + 0x20, chan_addr(t->chan));
	memcpy(b + 0x24, t->tone, 12);
	put32(b + 0x40, t->cmd);
	for (int i = 0; i < 3; ++i) put32(b + 0x44 + 4 * i, t->stack[i]);
}

static void put_player(uint8_t *b, const Mp2kPlayer *pl) {
	put32(b, pl->song);
	put32(b + 4, pl->status);
	b[8] = pl->tracks;
	b[9] = pl->priority;
	b[0xB] = pl->check_priority;
	put32(b + 0xC, pl->clock);
	const uint16_t h[] = { pl->tempo_d, pl->tempo_u, pl->tempo_i, pl->tempo_c, pl->fade_oi, pl->fade_oc, pl->fade_ov };
	for (int i = 0; i < 7; ++i) { b[0x1C + 2 * i] = (uint8_t)h[i]; b[0x1D + 2 * i] = (uint8_t)(h[i] >> 8); }
	put32(b + 0x30, pl->tone);
}

static void record(uint8_t *r) {
	memset(r, 0, RECORD);
	put32(r, 0x68736D53);
	const uint8_t info[] = { M.dma_counter, M.reverb, M.max_chans, M.master_volume, 3, 0, M.c15, M.dma_period };
	memcpy(r + 4, info, sizeof info);
	put32(r + 0x10, (uint32_t)M.per_frame);
	put32(r + 0x14, 10512);
	put32(r + 0x18, M.div_freq);
	for (int i = 0; i < 4; ++i) put_chan(r + 0x50 + 0x40 * i, &M.chans[i], false);
	int ring = M.dma_period * M.per_frame;
	memcpy(r + 0x350, M.ring[0], (size_t)ring);
	memcpy(r + 0x350 + 0x630, M.ring[1], (size_t)ring);
	for (int i = 0; i < MP2K_CGB; ++i) put_chan(r + SI_SIZE + 0x40 * i, CGB_CHAN(i), true);
	put_player(r + SI_SIZE + 0x100, &M.players[MUSIC]);
	for (int i = 0; i < 8; ++i) put_track(r + SI_SIZE + 0x140 + 0x50 * i, &M.tracks[M.players[MUSIC].first + i]);
}

int mp2k_trace_run(const char *spec) {
	int song = 1, frames = 600, c15 = -1, counter = -1;
	char prefix[256] = "trace";
	sscanf(spec, "%i:%d:%255[^:]:%d:%d", &song, &frames, prefix, &c15, &counter);
	if (!mp2k_init()) { fprintf(stderr, "mp2k-trace: no BN6 ROM\n"); return 1; }
	/* (the game's own phases where it is laid beside: its PSG envelope's
	 * 15-frame count and its ring's, as they stood the frame before) */
	if (c15 >= 0) M.c15 = (uint8_t)c15;
	if (counter > 0) M.dma_counter = (uint8_t)counter;
	char path[300];
	snprintf(path, sizeof path, "%s.si", prefix);
	FILE *si = fopen(path, "wb");
	snprintf(path, sizeof path, "%s.wav", prefix);
	FILE *wav = fopen(path, "wb");
	if (!si || !wav) {
		fprintf(stderr, "mp2k-trace: %s not written\n", prefix);
		if (si) fclose(si);
		if (wav) fclose(wav);
		return 1;
	}
	fwrite("RIFF\0\0\0\0WAVEfmt \x10\0\0\0\x01\0\x02\0\x80\xbb\0\0\0\xee\x02\0\x04\0\x10\0data\0\0\0\0", 1, 44, wav);
	mp2k_song_start(song);
	static uint8_t r[RECORD];
	static int8_t prev_r[704], prev_l[704];
	int16_t out[2048 * 2];
	uint32_t bytes = 0;
	for (int f = 0; f < frames; ++f) {
		mp2k_frame();
		int n = mp2k_per_frame();
		gba_sound_frame(prev_r, prev_l, n);
		memcpy(prev_r, mp2k_mix_right(), (size_t)n);
		memcpy(prev_l, mp2k_mix_left(), (size_t)n);
		int got = gba_sound_read(out, 2048);
		fwrite(out, 4, (size_t)got, wav);
		bytes += (uint32_t)got * 4;
		record(r);
		fwrite(r, 1, RECORD, si);
	}
	fseek(wav, 4, SEEK_SET);
	uint32_t v = 36 + bytes;
	fwrite(&v, 4, 1, wav);
	fseek(wav, 40, SEEK_SET);
	fwrite(&bytes, 4, 1, wav);
	fclose(wav);
	fclose(si);
	printf("mp2k-trace: song %#x, %d frames, %s.si and %s.wav\n", song, frames, prefix, prefix);
	return 0;
}
