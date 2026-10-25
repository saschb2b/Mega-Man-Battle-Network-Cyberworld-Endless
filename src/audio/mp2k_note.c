/* MP2K's notes (mp2k_state.h): a note given a channel by priority (ply_note),
 * a tie ended (ply_endtie), a track's and a channel's volumes and pitch
 * (TrkVolPitSet, ChnVolSetAsm, MidiKeyToFreq), as BN6's code does them
 * (bn6f asm/libs.s; docs/rom/sound.md, BN6's sound engine). */
#include "mp2k_state.h"

#include <string.h>

/* umul3232H32: the high word of a 32 x 32 product */
static uint32_t mul_high(uint32_t a, uint32_t b) { return (uint32_t)(((uint64_t)a * b) >> 32); }

uint32_t mp2k_key_to_freq(uint32_t wav, int key, int fine) {
	uint32_t k = (uint8_t)key, f = (uint32_t)(uint8_t)fine << 24;
	if (k > 178) { k = 178; f = 0xFFu << 24; }
	uint8_t s1 = mp2k_lib_u8(M.scale_table + k), s2 = mp2k_lib_u8(M.scale_table + k + 1);
	uint32_t v1 = mp2k_lib_u32(M.freq_table + 4u * (s1 & 0xF)) >> (s1 >> 4);
	uint32_t v2 = mp2k_lib_u32(M.freq_table + 4u * (s2 & 0xF)) >> (s2 >> 4);
	return mul_high(mp2k_u32(wav + 4), v1 + mul_high(v2 - v1, f));
}

void mp2k_track_vol_pit(int player, Mp2kTrack *t) {
	/* (a player silenced: its volume none and none left to work out, the
	 * pitch's part as BN6 works it; mp2k_quiet_set) */
	if ((M.quiet >> player & 1) && (t->flags & TRK_VOLSET)) {
		t->vol_mr = t->vol_ml = 0;
		t->flags &= (uint8_t)~TRK_VOLSET;
	}
	if (t->flags & TRK_VOLSET) {
		uint32_t x = (uint32_t)(t->vol * t->vol_x) >> 5;
		if (t->mod_t == 1) x = (x * (uint32_t)(t->mod_m + 128)) >> 7;
		int y = 2 * t->pan + t->pan_x;
		if (t->mod_t == 2) y += t->mod_m;
		if (y < -128) y = -128;
		if (y > 127) y = 127;
		t->vol_mr = (uint8_t)(((uint32_t)(y + 128) * x) >> 8);
		t->vol_ml = (uint8_t)(((uint32_t)(127 - y) * x) >> 8);
	}
	if (t->flags & TRK_PITSET) {
		int x = (t->bend * t->bend_range + t->tune) * 4 + (t->key_shift << 8) + (t->key_shift_x << 8) + t->pit_x;
		if (t->mod_t == 0) x += t->mod_m * 16;
		t->key_m = (int8_t)(x >> 8);
		t->pit_m = (uint8_t)x;
	}
	t->flags &= 0xFA;
}

void mp2k_chan_volume(Mp2kChan *c, const Mp2kTrack *t) {
	uint32_t r = (uint32_t)((128 + c->rhythm_pan) * c->velocity) * t->vol_mr >> 14;
	uint32_t l = (uint32_t)((127 - c->rhythm_pan) * c->velocity) * t->vol_ml >> 14;
	c->right = (uint8_t)(r > 0xFF ? 0xFF : r);
	c->left = (uint8_t)(l > 0xFF ? 0xFF : l);
}

/* ---- ply_note ---- */

/* the note's own bytes after the command: key, velocity, extra gate */
static void note_bytes(Mp2kTrack *t) {
	uint32_t p = t->cmd;
	if (mp2k_u8(p) >= 0x80) return;
	t->key = mp2k_u8(p++);
	if (mp2k_u8(p) < 0x80) {
		t->velocity = mp2k_u8(p++);
		if (mp2k_u8(p) < 0x80) t->gate = (uint8_t)(t->gate + mp2k_u8(p++));
	}
	t->cmd = p;
}

/* the tone the note plays (a key split's or a drum kit's by its key) into
 * `tone`; its key and pan; false where it plays none */
static bool note_tone(const Mp2kTrack *t, uint8_t tone[12], int *key, int *pan) {
	uint8_t type = t->tone[0];
	*key = t->key;
	*pan = 0;
	if (!(type & 0xC0)) { memcpy(tone, t->tone, 12); return true; }
	uint32_t split = (uint32_t)t->tone[8] | (uint32_t)t->tone[9] << 8 | (uint32_t)t->tone[10] << 16 | (uint32_t)t->tone[11] << 24;
	uint32_t subs = (uint32_t)t->tone[4] | (uint32_t)t->tone[5] << 8 | (uint32_t)t->tone[6] << 16 | (uint32_t)t->tone[7] << 24;
	int idx = type & 0x40 ? mp2k_u8(split + (uint32_t)t->key) : t->key;
	uint32_t sub = subs + (uint32_t)idx * 12;
	for (int i = 0; i < 12; ++i) tone[i] = mp2k_u8(sub + (uint32_t)i);
	if (tone[0] & 0xC0) return false;
	if (type & 0x80) {
		if (tone[3] & 0x80) *pan = (tone[3] - 0xC0) * 2;
		*key = tone[1];
	}
	return true;
}

static uint32_t track_addr(int track) { return track == MP2K_NONE ? 0 : M.tracks[track].addr; }

/* the PSG's channel of the type, where the note may take it */
static int note_cgb(int type, uint8_t prio, const Mp2kTrack *t) {
	Mp2kChan *c = CGB_CHAN(type - 1);
	if ((c->status & CHN_ON) && !(c->status & CHN_STOP)) {
		if (c->priority > prio) return MP2K_NONE;
		if (c->priority == prio && track_addr(c->track) < t->addr) return MP2K_NONE;
	}
	return MP2K_DS_MAX + type - 1;
}

/* a DirectSound channel: a free one, else the stopping one, else the one
 * of the lowest priority (a later track's on a tie) */
static int note_ds(uint8_t prio, const Mp2kTrack *t) {
	int best = MP2K_NONE;
	unsigned best_prio = prio;
	uint32_t best_track = t->addr;
	bool stopping = false;
	for (int i = 0; i < M.max_chans; ++i) {
		const Mp2kChan *c = &M.chans[i];
		if (!(c->status & CHN_ON)) return i;
		uint32_t ta = track_addr(c->track);
		if ((c->status & CHN_STOP) && !stopping) {
			stopping = true;
			best_prio = c->priority;
			best_track = ta;
			best = i;
			continue;
		}
		if (stopping && !(c->status & CHN_STOP)) continue;
		if (c->priority < best_prio || (c->priority == best_prio && ta >= best_track)) {
			best_prio = c->priority;
			best_track = ta;
			best = i;
		}
	}
	return best;
}

/* the channel joined to the track's list, its fields from the note */
static void note_take(int player, int ti, int chan, const uint8_t tone[12], int key, int pan, uint8_t prio) {
	Mp2kTrack *t = &M.tracks[ti];
	Mp2kChan *c = &M.chans[chan];
	mp2k_clear_chain(chan);
	c->prev = MP2K_NONE;
	c->next = t->chan;
	if (t->chan != MP2K_NONE) M.chans[t->chan].prev = chan;
	t->chan = chan;
	c->track = ti;
	c->rom = M.players[player].rom;
	t->lfo_delay_c = t->lfo_delay;
	if (t->lfo_delay) {
		t->mod_m = 0;
		t->lfo_speed_c = 0;
		t->flags |= t->mod_t == 0 ? TRK_PITCHG : TRK_VOLCHG;
	}
	mp2k_track_vol_pit(player, t);
	c->gate = t->gate;
	c->midi_key = t->key;
	c->velocity = t->velocity;
	c->priority = prio;
	c->key = (uint8_t)key;
	c->rhythm_pan = (int8_t)pan;
	c->type = tone[0];
	c->wav = (uint32_t)tone[4] | (uint32_t)tone[5] << 8 | (uint32_t)tone[6] << 16 | (uint32_t)tone[7] << 24;
	c->attack = tone[8];
	c->decay = tone[9];
	c->sustain = tone[10];
	c->release = tone[11];
	c->echo_volume = t->echo_volume;
	c->echo_length = t->echo_length;
	mp2k_chan_volume(c, t);
}

void mp2k_note(int player, int ti, int clock_index) {
	Mp2kTrack *t = &M.tracks[ti];
	t->gate = mp2k_lib_u8(M.clock_table + (uint32_t)clock_index);
	note_bytes(t);
	uint8_t tone[12];
	int key, pan;
	if (!note_tone(t, tone, &key, &pan)) return;
	unsigned p = M.players[player].priority + t->priority;
	uint8_t prio = (uint8_t)(p > 0xFF ? 0xFF : p);
	int type = tone[0] & 7;
	int chan = type ? note_cgb(type, prio, t) : note_ds(prio, t);
	if (chan == MP2K_NONE) return;
	note_take(player, ti, chan, tone, key, pan, prio);
	Mp2kChan *c = &M.chans[chan];
	int k = c->key + t->key_m;
	if (k < 0) k = 0;
	if (type) {
		c->length = tone[2];
		c->sweep = !(tone[3] & 0x80) && (tone[3] & 0x70) ? tone[3] : 8;
		c->frequency = mp2k_cgb_key_to_freq(type, k, t->pit_m);
	} else c->frequency = mp2k_key_to_freq(c->wav, k, t->pit_m);
	c->status = CHN_START;
	t->flags &= 0xF0;
}

/* ply_endtie: the first of the track's notes on the key, still held, let go */
void mp2k_end_tie(Mp2kTrack *t) {
	uint8_t key = mp2k_u8(t->cmd);
	if (key < 0x80) {
		t->key = key;
		++t->cmd;
	} else key = t->key;
	for (int c = t->chan; c != MP2K_NONE; c = M.chans[c].next == c ? MP2K_NONE : M.chans[c].next) {
		Mp2kChan *ch = &M.chans[c];
		if ((ch->status & 0x83) && !(ch->status & CHN_STOP) && ch->midi_key == key) {
			ch->status |= CHN_STOP;
			return;
		}
	}
}
