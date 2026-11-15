/* MP2K's state as BN6 keeps it (mp2k_layout.h). */
#include "mp2k_layout.h"

#include <string.h>

#include "bn6.h"
#include "bytes.h"

uint32_t mp2k_chan_addr(int c) {
	if (c < 0 || c >= MP2K_CHANS) return 0;
	return c < MP2K_DS_MAX ? BN6_SOUND_CHANS + (uint32_t)c * MP2K_CHAN_BYTES
	                       : BN6_SOUND_CGB_CHANS + (uint32_t)(c - MP2K_DS_MAX) * MP2K_CHAN_BYTES;
}

uint32_t mp2k_track_addr(int t) { return t < 0 || t >= MP2K_TRACKS ? 0 : M.tracks[t].addr; }

int mp2k_chan_at(uint32_t addr) {
	for (int c = 0; c < MP2K_CHANS; ++c)
		if (c < M.max_chans || c >= MP2K_DS_MAX)
			if (addr && mp2k_chan_addr(c) == addr) return c;
	return MP2K_NONE;
}

int mp2k_track_at(uint32_t addr) {
	for (int t = 0; addr && t < MP2K_TRACKS; ++t)
		if (M.tracks[t].addr == addr) return t;
	return MP2K_NONE;
}

void mp2k_put_chan(uint8_t *b, const Mp2kChan *c, bool cgb) {
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
	put32(b + MP2K_CHAN_TRACK, mp2k_track_addr(c->track));
	put32(b + MP2K_CHAN_PREV, mp2k_chan_addr(c->prev));
	put32(b + MP2K_CHAN_NEXT, mp2k_chan_addr(c->next));
}

void mp2k_get_chan(Mp2kChan *c, const uint8_t *b, bool cgb) {
	uint8_t *head[] = { &c->status, &c->type, &c->right, &c->left, &c->attack, &c->decay, &c->sustain, &c->release, &c->key, &c->env,
		&c->env_right, &c->env_left, &c->echo_volume, &c->echo_length };
	for (size_t i = 0; i < sizeof head / sizeof head[0]; ++i) *head[i] = b[i];
	c->gate = b[0x10];
	c->midi_key = b[0x11];
	c->velocity = b[0x12];
	c->priority = b[0x13];
	c->rhythm_pan = (int8_t)b[0x14];
	if (cgb) {
		uint8_t *tail[] = { &c->sustain_goal, &c->n4, &c->pan, &c->pan_mask, &c->modify, &c->length, &c->sweep };
		for (size_t i = 0; i < sizeof tail / sizeof tail[0]; ++i) *tail[i] = b[0x19 + i];
	} else {
		c->count = get32(b + 0x18);
		c->fw = get32(b + 0x1C);
	}
	c->frequency = get32(b + 0x20);
	c->wav = get32(b + 0x24);
	c->cur = get32(b + 0x28);
}

/* a track's bytes 0x00-0x1F, its fields in order */
#define TRACK_HEAD(t, CAST) { CAST(t->flags), CAST(t->wait), CAST(t->pattern_level), CAST(t->repeat), CAST(t->gate), CAST(t->key), \
	CAST(t->velocity), CAST(t->running), CAST(t->key_m), CAST(t->pit_m), CAST(t->key_shift), CAST(t->key_shift_x), CAST(t->tune), \
	CAST(t->pit_x), CAST(t->bend), CAST(t->bend_range), CAST(t->vol_mr), CAST(t->vol_ml), CAST(t->vol), CAST(t->vol_x), CAST(t->pan), \
	CAST(t->pan_x), CAST(t->mod_m), CAST(t->mod), CAST(t->mod_t), CAST(t->lfo_speed), CAST(t->lfo_speed_c), CAST(t->lfo_delay), \
	CAST(t->lfo_delay_c), CAST(t->priority), CAST(t->echo_volume), CAST(t->echo_length) }
#define AS_BYTE(f) ((uint8_t)(f))
#define AS_PLACE(f) ((uint8_t *)&(f))

void mp2k_put_track(uint8_t *b, const Mp2kTrack *t) {
	const uint8_t head[] = TRACK_HEAD(t, AS_BYTE);
	memcpy(b, head, sizeof head);
	put32(b + MP2K_TRACK_CHANS, mp2k_chan_addr(t->chan));
	memcpy(b + 0x24, t->tone, 12);
	put32(b + 0x40, t->cmd);
	for (int i = 0; i < 3; ++i) put32(b + 0x44 + 4 * i, t->stack[i]);
}

void mp2k_get_track(Mp2kTrack *t, const uint8_t *b) {
	uint8_t *head[] = TRACK_HEAD(t, AS_PLACE);
	for (size_t i = 0; i < sizeof head / sizeof head[0]; ++i) *head[i] = b[i];
	memcpy(t->tone, b + 0x24, 12);
	t->cmd = get32(b + 0x40);
	for (int i = 0; i < 3; ++i) t->stack[i] = get32(b + 0x44 + 4 * i);
}

void mp2k_put_player(uint8_t *b, const Mp2kPlayer *pl) {
	put32(b, pl->song);
	put32(b + 4, pl->status);
	b[8] = pl->tracks;
	b[9] = pl->priority;
	b[0xB] = pl->check_priority;
	put32(b + 0xC, pl->clock);
	const uint16_t h[] = { pl->tempo_d, pl->tempo_u, pl->tempo_i, pl->tempo_c, pl->fade_oi, pl->fade_oc, pl->fade_ov };
	for (int i = 0; i < 7; ++i) put16(b + 0x1C + 2 * i, h[i]);
	put32(b + 0x30, pl->tone);
}

void mp2k_get_player(Mp2kPlayer *pl, const uint8_t *b) {
	pl->song = get32(b);
	pl->status = get32(b + 4);
	pl->priority = b[9];
	pl->check_priority = b[0xB];
	pl->clock = get32(b + 0xC);
	uint16_t *h[] = { &pl->tempo_d, &pl->tempo_u, &pl->tempo_i, &pl->tempo_c, &pl->fade_oi, &pl->fade_oc, &pl->fade_ov };
	for (int i = 0; i < 7; ++i) *h[i] = get16(b + 0x1C + 2 * i);
	pl->tone = get32(b + 0x30);
}
