/* MP2K's mixer (mp2k_state.h): a frame of the DirectSound channels into the
 * ring, as BN6's SoundMainRAM does it (bn6f asm/libs.s; docs/ROM_DATA.md,
 * BN6's sound engine): the reverb from the ring's next part, each channel's
 * envelope stepped once a frame, its samples added into 8-bit bytes that
 * wrap and never clip, at the mix's rate. */
#include "mp2k_state.h"

#include <string.h>

/* the ring's part this frame writes: right and left */
static int8_t *part(int side, int slice) { return &M.ring[side][slice * M.per_frame]; }

/* the part's start: the reverb of the ring's next part over it, or silence */
static void part_start(void) {
	int n = M.per_frame;
	int8_t *r = part(0, M.slice), *l = part(1, M.slice);
	if (!M.reverb) {
		memset(r, 0, (size_t)n);
		memset(l, 0, (size_t)n);
		return;
	}
	int src = M.dma_counter == 2 ? 0 : M.slice + 1;
	const int8_t *sr = part(0, src), *sl = part(1, src);
	for (int i = 0; i < n; ++i) {
		int v = ((r[i] + l[i] + sr[i] + sl[i]) * M.reverb) >> 9;
		if (v & 0x80) ++v;
		r[i] = l[i] = (int8_t)v;
	}
}

/* ---- a channel's envelope ---- */

/* the release done: the pseudo-echo's volume held, or the channel off;
 * false where it went off */
static bool to_echo(Mp2kChan *c, int *env) {
	*env = c->echo_volume;
	if (!*env) { c->status = 0; return false; }
	c->status |= CHN_ECHO;
	return true;
}

/* a channel begun: its sample from the top, the attack's first step */
static void env_begin(Mp2kChan *c, int *env) {
	c->status = 3;
	c->cur = c->wav + 0x10;
	c->count = mp2k_u32(c->wav + 0xC);
	c->env = 0;
	c->fw = 0;
	if (mp2k_u8(c->wav + 3) & 0xC0) c->status |= CHN_LOOP;
	*env = c->attack;
	if (*env >= 0xFF) { *env = 0xFF; --c->status; }
}

/* the envelope's step; false where the channel went off */
static bool env_step(Mp2kChan *c) {
	int env = c->env;
	if (c->status & CHN_START) {
		if (c->status & CHN_STOP) { c->status = 0; return false; }
		env_begin(c, &env);
	} else if (c->status & CHN_ECHO) {
		if (--c->echo_length == 0 || c->echo_length == 0xFF) { c->status = 0; return false; }
	} else if (c->status & CHN_STOP) {
		env = env * c->release >> 8;
		if (env <= c->echo_volume && !to_echo(c, &env)) return false;
	} else if ((c->status & 3) == 2) {
		env = env * c->decay >> 8;
		if (env <= c->sustain) {
			env = c->sustain;
			if (!env) { if (!to_echo(c, &env)) return false; }
			else --c->status;
		}
	} else if ((c->status & 3) == 3) {
		env += c->attack;
		if (env >= 0xFF) { env = 0xFF; --c->status; }
	}
	c->env = (uint8_t)env;
	int m = (M.master_volume + 1) * env >> 4;
	c->env_right = (uint8_t)(c->right * m >> 8);
	c->env_left = (uint8_t)(c->left * m >> 8);
	return true;
}

/* ---- a channel's samples ---- */

static int8_t sample(uint32_t addr) { return (int8_t)mp2k_u8(addr); }

/* (v times the side's volume, its high byte added into the byte, wrapping) */
static void add(int8_t *b, int v, int vol) { *b = (int8_t)(uint8_t)((uint8_t)*b + (uint8_t)((v * vol) >> 8)); }

/* a fixed-pitch sample: one sample a step; false where it ended (its
 * position and count left as the frame found them, as BN6's are) */
static bool mix_fixed(Mp2kChan *c, int8_t *r, int8_t *l, uint32_t loop_start, uint32_t loop_len) {
	uint32_t at = c->cur, count = c->count;
	for (int i = 0; i < M.per_frame; ++i) {
		int v = sample(at++);
		add(&r[i], v, c->env_right);
		add(&l[i], v, c->env_left);
		if (--count == 0) {
			if (!loop_len) { c->status = 0; return false; }
			at = loop_start;
			count = loop_len;
		}
	}
	c->cur = at;
	c->count = count;
	return true;
}

/* past the sample's end: back into the loop by the overshoot; false where
 * it has none */
static bool wrap(Mp2kChan *c, int32_t *count, uint32_t *at, uint32_t loop_start, uint32_t loop_len) {
	if (!loop_len) { c->status = 0; return false; }
	int32_t over = -*count;
	for (;;) {
		*count += (int32_t)loop_len;
		if (*count > 0) break;
		over -= (int32_t)loop_len;
	}
	*at = loop_start + (uint32_t)over;
	return true;
}

/* a pitched sample: interpolated between neighbours, 23 bits of fraction */
static bool mix_pitched(Mp2kChan *c, int8_t *r, int8_t *l, uint32_t loop_start, uint32_t loop_len) {
	uint32_t step = M.div_freq * c->frequency, fw = c->fw, at = c->cur;
	int32_t count = (int32_t)c->count;
	int s0 = sample(at), d = sample(at + 1) - s0;
	for (int i = 0; i < M.per_frame; ++i) {
		int v = s0 + (int)(((int64_t)(int32_t)fw * d) >> 23);
		add(&r[i], v, c->env_right);
		add(&l[i], v, c->env_left);
		fw += step;
		uint32_t n = fw >> 23;
		if (!n) continue;
		fw &= ~0x3F800000u;
		count -= (int32_t)n;
		if (count <= 0) {
			if (!wrap(c, &count, &at, loop_start, loop_len)) return false;
		} else at += n;
		s0 = sample(at);
		d = sample(at + 1) - s0;
	}
	c->cur = at;
	c->fw = fw;
	c->count = (uint32_t)count;
	return true;
}

void mp2k_mix_frame(void) {
	part_start();
	int8_t *r = part(0, M.slice), *l = part(1, M.slice);
	for (int i = 0; i < M.max_chans; ++i) {
		Mp2kChan *c = &M.chans[i];
		if (!(c->status & CHN_ON)) continue;
		mp2k_read(c->rom);
		if (!env_step(c)) continue;
		uint32_t loop_start = 0, loop_len = 0;
		if (c->status & CHN_LOOP) {
			loop_start = c->wav + 0x10 + mp2k_u32(c->wav + 8);
			loop_len = mp2k_u32(c->wav + 0xC) - mp2k_u32(c->wav + 8);
		}
		if (c->type & 8) mix_fixed(c, r, l, loop_start, loop_len);
		else mix_pitched(c, r, l, loop_start, loop_len);
	}
}
