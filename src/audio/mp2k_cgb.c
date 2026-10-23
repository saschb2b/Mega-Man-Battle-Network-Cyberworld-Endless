/* MP2K's driver of the GBA's PSG (mp2k_state.h): the four channels'
 * envelopes stepped in software once a frame and their registers written,
 * as BN6's CgbSound does it (bn6f asm/libs.s; docs/ROM_DATA.md, BN6's
 * sound engine). The PSG's own envelope runs between the frames from what
 * it writes into NRx2, and every change of volume retriggers the channel:
 * gba_sound.c plays it on mGBA's PSG. */
#include "mp2k_state.h"

#include "gba_sound.h"

/* a channel's registers, from 0x04000060 (ch 1 to 4) */
static const uint8_t NR0[4] = { 0x60, 0x61, 0x70, 0x71 }, NR1[4] = { 0x62, 0x68, 0x72, 0x78 }, NR2[4] = { 0x63, 0x69, 0x73, 0x79 },
	NR3[4] = { 0x64, 0x6C, 0x74, 0x7C }, NR4[4] = { 0x65, 0x6D, 0x75, 0x7D };
#define NR51 0x81
#define NR52 0x84
#define BIAS_HI 0x89
#define WAVE_RAM 0x90

uint32_t mp2k_cgb_key_to_freq(int type, int key, int fine) {
	unsigned k = (uint8_t)key, f = (uint8_t)fine;
	if (type == 4) {
		k = k <= 20 ? 0 : k - 21;
		if (k > 59) k = 59;
		return mp2k_u8(M.noise_table + k);
	}
	if (k <= 35) { f = 0; k = 0; }
	else k -= 36;
	if (k > 130) { k = 130; f = 0xFF; }
	uint8_t s1 = mp2k_u8(M.cgb_scale_table + k), s2 = mp2k_u8(M.cgb_scale_table + k + 1);
	int v1 = (int16_t)(mp2k_u8(M.cgb_freq_table + 2u * (s1 & 0xF)) | mp2k_u8(M.cgb_freq_table + 2u * (s1 & 0xF) + 1) << 8) >> (s1 >> 4);
	int v2 = (int16_t)(mp2k_u8(M.cgb_freq_table + 2u * (s2 & 0xF)) | mp2k_u8(M.cgb_freq_table + 2u * (s2 & 0xF) + 1) << 8) >> (s2 >> 4);
	return (uint32_t)(v1 + (((int)f * (v2 - v1)) >> 8) + 2048);
}

/* CgbOscOff: the channel silenced */
void mp2k_cgb_off(int type) {
	switch (type) {
	case 1: gba_sound_write8(0x63, 8); gba_sound_write8(0x65, 0x80); break;
	case 2: gba_sound_write8(0x69, 8); gba_sound_write8(0x6D, 0x80); break;
	case 3: gba_sound_write8(0x70, 0); break;
	default: gba_sound_write8(0x79, 8); gba_sound_write8(0x7D, 0x80); break;
	}
}

/* CgbModVol: the envelope's goal from the volumes, the side it sounds on */
static void mod_vol(Mp2kChan *c) {
	unsigned r = c->right, l = c->left, goal = (r + l) >> 4;
	if ((r >= l && (r >> 1) >= l) || (r < l && (l >> 1) >= r)) {
		c->pan = r >= l ? 0x0F : 0xF0;
		if (goal > 15) goal = 15;
	} else c->pan = 0xFF;
	c->env_right = (uint8_t)goal;
	c->sustain_goal = (uint8_t)((goal * c->sustain + 15) >> 4);
	c->pan &= c->pan_mask;
}

/* the wave channel's 32 samples loaded where they changed */
static void wave_load(Mp2kChan *c) {
	if (c->wav == c->cur) return;
	gba_sound_write8(0x70, 0x40);
	for (uint32_t i = 0; i < 16; i += 4) gba_sound_write32(WAVE_RAM + (int)i, mp2k_u32(c->wav + i));
	c->cur = c->wav;
}

/* a note begun: its registers set, its attack begun; `env` (the NRx2's
 * low bits) the attack's */
static void start(Mp2kChan *c, int ch, uint8_t *env) {
	c->status = 3;
	c->modify = 3;
	mod_vol(c);
	if (ch == 2) {
		wave_load(c);
		gba_sound_write8(0x70, 0);
		gba_sound_write8(NR1[2], c->length);
		c->n4 = c->length ? 0xC0 : 0x80;
	} else {
		if (ch == 0) gba_sound_write8(NR0[0], c->sweep);
		if (ch == 3) {
			gba_sound_write8(NR1[3], c->length);
			gba_sound_write8(NR3[3], (uint8_t)(c->wav << 3));
		} else gba_sound_write8(NR1[ch], (uint8_t)((c->wav << 6) + c->length));
		*env = (uint8_t)(c->attack + 8);
		c->n4 = c->length ? 0x40 : 0;
	}
	c->env_left = c->attack;
}

/* the release's end: the pseudo-echo's volume, or off; false where off */
static bool echo(Mp2kChan *c, int ch, uint8_t *env) {
	c->env = (uint8_t)((c->env_right * c->echo_volume + 0xFF) >> 8);
	if (!c->env) return false;
	c->status |= CHN_ECHO;
	c->modify |= 1;
	if (ch != 2) *env = 8;
	return true;
}

/* the decay's end: sustained, or released at once where it has none */
static int sustain(Mp2kChan *c, int ch, uint8_t *env, bool *off) {
	if (!c->sustain) {
		c->status &= 0xFC;
		*off = !echo(c, ch, env);
		return -1;
	}
	--c->status;
	c->modify |= 1;
	if (ch != 2) *env = 8;
	c->env = c->sustain_goal;
	return 7;
}

/* the attack's end: the decay begun (or sustained where it has none) */
static int attack_done(Mp2kChan *c, int ch, uint8_t *env, bool *off) {
	--c->status;
	c->env_left = c->decay;
	if (!c->decay) return sustain(c, ch, env, off);
	c->modify |= 1;
	c->env = c->env_right;
	if (ch != 2) *env = c->decay;
	return -2;
}

/* one envelope step of a channel whose count ran out: the counter's next
 * value, -1 where the channel echoes or went off, -2 where the counter is
 * already set */
static int env_tick(Mp2kChan *c, int ch, uint8_t *env, bool *off) {
	if (ch == 2) c->modify |= 1;
	mod_vol(c);
	switch (c->status & 3) {
	case 0:
		if ((int8_t)--c->env > 0) return c->release;
		*off = !echo(c, ch, env);
		return -1;
	case 1:
		c->env = c->sustain_goal;
		return 7;
	case 2:
		if ((int8_t)--c->env > (int8_t)c->sustain_goal) return c->decay;
		return sustain(c, ch, env, off);
	default:
		if (++c->env < c->env_right) return c->attack;
		return attack_done(c, ch, env, off);
	}
}

/* the envelope's frame: its counter, a step where it ran out (two on the
 * c15 frame); false where the channel went off; *regs whether to write */
static bool envelope(Mp2kChan *c, int ch, uint8_t *env, bool extra) {
	for (;;) {
		bool off = false;
		if (c->env_left == 0) {
			int next = env_tick(c, ch, env, &off);
			if (off) return false;
			if (next == -1) return true;
			if (next >= 0) c->env_left = (uint8_t)next;
		}
		--c->env_left;
		if (!extra) return true;
		extra = false;
	}
}

/* the frame's changes into the registers: the frequency, the volume (a
 * retrigger) */
static void registers(Mp2kChan *c, int ch, uint8_t env) {
	if (c->modify & 2) {
		if (ch < 3 && (c->type & 8)) {
			uint8_t pwm = gba_sound_read8(BIAS_HI);
			if (pwm <= 0x3F) c->frequency = (c->frequency + 2) & 0x7FC;
			else if (pwm <= 0x7F) c->frequency = (c->frequency + 1) & 0x7FE;
		}
		uint32_t f = ch != 3 ? c->frequency : (gba_sound_read8(NR3[3]) & 8) | c->frequency;
		gba_sound_write8(NR3[ch], (uint8_t)f);
		c->n4 = (uint8_t)((c->n4 & 0xC0) + ((c->frequency & 0x3F00) >> 8));
		gba_sound_write8(NR4[ch], c->n4);
	}
	if (!(c->modify & 1)) return;
	gba_sound_write8(NR51, (uint8_t)((gba_sound_read8(NR51) & ~c->pan_mask) | c->pan));
	if (ch == 2) {
		gba_sound_write8(NR2[2], mp2k_u8(M.cgb3_vol_table + c->env));
		if (c->n4 & 0x80) {
			gba_sound_write8(0x70, 0x80);
			gba_sound_write8(NR4[2], c->n4);
			c->n4 &= 0x7F;
		}
		return;
	}
	gba_sound_write8(NR2[ch], (uint8_t)((c->env << 4) + (env & 0xF)));
	gba_sound_write8(NR4[ch], (uint8_t)(c->n4 | 0x80));
	if (ch == 0 && !(gba_sound_read8(NR0[0]) & 8)) gba_sound_write8(NR4[0], (uint8_t)(c->n4 | 0x80));
}

/* a channel's frame; false where it went off */
static bool channel(Mp2kChan *c, int ch, bool extra) {
	uint8_t env = gba_sound_read8(NR2[ch]);
	if (c->status & CHN_START) {
		if (c->status & CHN_STOP) return false;
		start(c, ch, &env);
		if (c->attack) c->env = 0;
		else {
			bool off = false;
			int next = attack_done(c, ch, &env, &off);
			if (off) return false;
			if (next == -1) { registers(c, ch, env); return true; }
			if (next >= 0) c->env_left = (uint8_t)next;
		}
		--c->env_left;
		if (extra && !envelope(c, ch, &env, false)) return false;
	} else if ((c->status & CHN_ECHO) || !(gba_sound_read8(NR52) >> ch & 1)) {
		if ((int8_t)--c->echo_length <= 0) return false;
	} else if ((c->status & CHN_STOP) && (c->status & 3)) {
		c->status &= 0xFC;
		c->env_left = c->release;
		if (c->release) {
			c->modify |= 1;
			if (ch != 2) env = c->release;
			--c->env_left;
			if (extra && !envelope(c, ch, &env, false)) return false;
		} else if (!echo(c, ch, &env)) return false;
	} else if (!envelope(c, ch, &env, extra)) return false;
	registers(c, ch, env);
	return true;
}

void mp2k_cgb_frame(void) {
	M.c15 = M.c15 ? (uint8_t)(M.c15 - 1) : 14;
	for (int ch = 0; ch < MP2K_CGB; ++ch) {
		Mp2kChan *c = CGB_CHAN(ch);
		if (!(c->status & CHN_ON)) continue;
		if (!channel(c, ch, M.c15 == 0)) {
			mp2k_cgb_off(ch + 1);
			c->status = 0;
		}
		c->modify = 0;
	}
}
