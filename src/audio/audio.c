/* A player for the GBA "MP2K" sound engine data in the ROM.
 *
 * Songs are a header (track count, voicegroup, track pointers) and byte-coded
 * tracks: waits, notes, and controller commands with running status. Voices
 * are DirectSound samples or the four GBA PSG channels. The sequencer ticks
 * at the GBA frame rate inside the audio callback; the mixer resamples to the
 * host rate. */
#include "audio.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "rom.h"

#define OUT_RATE 48000
#define GBA_FPS 59.7275
#define MAX_TRACKS 16
#define MAX_CHANS 32
#define NUM_PLAYERS 5 /* 0 = music, 1-4 = effects */
#define SONG_COUNT 474

static const uint8_t len_table[49] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	28, 30, 32, 36, 40, 42, 44, 48, 52, 54, 56, 60, 64, 66, 68, 72, 76, 78, 80, 84, 88, 90, 92, 96,
};

typedef struct {
	bool on;
	uint32_t pc;
	uint32_t ret[3];
	int depth;
	int wait;
	uint8_t running;
	int voice, vol, pan, bend, bendr, keysh, tune;
	int mod, lfos, lfodl, modt, lfo_phase;
	int key, vel, gate;
	int rept_count;
	int prio;
} Track;

typedef struct {
	bool on;
	int song;
	uint32_t voicegroup;
	Track tr[MAX_TRACKS];
	int ntracks;
	int tempo; /* bpm */
	int tempo_acc;
	int vol;   /* 0-256, for fades */
	int fade;  /* per-frame change */
	int prio;
} Player;

enum { V_DS, V_SQ1, V_SQ2, V_WAVE, V_NOISE };

typedef struct {
	bool on;
	int kind;
	int player, track;
	int key, midi_key;
	int vel;
	int gate;          /* ticks left, -1 = tie */
	bool released;
	int env, atk, dec, sus, rel;
	int phase;         /* 0 attack, 1 decay, 2 sustain, 3 release */
	int cgb_timer;
	const int8_t *data;
	uint32_t len, loop_start;
	bool loop;
	double pos, step;
	uint32_t sample_rate;
	bool fixed;
	int duty;
	const uint8_t *wave;
	uint32_t lfsr;
	bool short_noise;
	double phase_acc;
	int voice_pan;
	float lv, rv;
	int age;
} Chan;

static SDL_AudioDeviceID dev;
static Player players[NUM_PLAYERS];
static Chan chans[MAX_CHANS];
static double frame_acc;
static int music_volume = 8, sfx_volume = 8;
static int current_music = -1;
static int sfx_rr;

static inline uint32_t rd32(uint32_t off) { return rom_u32(off); }
static inline uint8_t rd8(uint32_t off) { return R.data[off]; }
static bool valid(uint32_t ptr) { return rom_is_ptr(ptr); }

/* ------------------------------------------------------------------ */
/* Voices and channels */

static void chan_release(Chan *c) {
	if (c->on && !c->released) { c->released = true; c->phase = 3; c->cgb_timer = 0; }
}

static uint32_t resolve_voice(uint32_t vg, int voice, int *key) {
	uint32_t v = rom_off(vg) + (uint32_t)voice * 12;
	uint8_t type = rd8(v);
	if (type == 0x40) { /* key split */
		uint32_t sub = rd32(v + 4), table = rd32(v + 8);
		if (!valid(sub) || !valid(table)) return 0;
		int idx = rd8(rom_off(table) + (uint32_t)*key);
		return rom_off(sub) + (uint32_t)idx * 12;
	}
	if (type == 0x80) { /* drum kit: each key is its own voice at a fixed pitch */
		uint32_t sub = rd32(v + 4);
		if (!valid(sub)) return 0;
		uint32_t d = rom_off(sub) + (uint32_t)*key * 12;
		*key = rd8(d + 1);
		return d;
	}
	return v;
}

static Chan *alloc_chan(int player) {
	Chan *best = NULL;
	for (int i = 0; i < MAX_CHANS; ++i) if (!chans[i].on) return &chans[i];
	/* Steal the oldest released channel, else the oldest of a player that matters less. */
	for (int i = 0; i < MAX_CHANS; ++i)
		if (chans[i].released && (!best || chans[i].age > best->age)) best = &chans[i];
	if (best) return best;
	for (int i = 0; i < MAX_CHANS; ++i)
		if (players[chans[i].player].prio <= players[player].prio && (!best || chans[i].age > best->age)) best = &chans[i];
	return best;
}

static double note_freq(int key, int fine) {
	return 440.0 * pow(2.0, ((double)key - 69.0 + fine / 256.0) / 12.0);
}

static void chan_pitch(Chan *c, const Track *t) {
	int fine = t->tune * 4 + t->bend * t->bendr * 4;
	if (t->mod && t->modt == 0)
		fine += (int)(sin(t->lfo_phase * 2 * M_PI / 256.0) * t->mod * 4);
	int key = c->key + t->keysh;
	if (c->kind == V_DS) {
		if (c->fixed) c->step = (double)c->sample_rate / 1024.0 / OUT_RATE;
		else c->step = (double)c->sample_rate / 1024.0 * pow(2.0, ((double)key - 60.0 + fine / 256.0) / 12.0) / OUT_RATE;
	} else if (c->kind == V_NOISE) {
		c->step = note_freq(key, fine) * 8.0 / OUT_RATE;
	} else {
		c->step = note_freq(key, fine) / OUT_RATE; /* 1.0 = one period */
	}
}

static void chan_volume(Chan *c, const Player *p, const Track *t) {
	int pan = t->pan + c->voice_pan;
	if (pan < -64) pan = -64;
	if (pan > 63) pan = 63;
	double v = (t->vol / 127.0) * (c->vel / 127.0) * (p->vol / 256.0);
	c->lv = (float)(v * (63.0 - pan) / 127.0 * 2.0);
	c->rv = (float)(v * (pan + 64.0) / 127.0 * 2.0);
}

static void note_on(int pi, int ti, int key, int vel, int gate) {
	Player *p = &players[pi];
	Track *t = &p->tr[ti];
	int k = key;
	uint32_t v = resolve_voice(p->voicegroup, t->voice, &k);
	if (!v) return;
	uint8_t type = rd8(v);
	Chan *c = alloc_chan(pi);
	if (!c) return;
	memset(c, 0, sizeof *c);
	c->player = pi;
	c->track = ti;
	c->midi_key = key;
	c->key = k;
	c->vel = vel;
	c->gate = gate;
	c->on = true;
	uint8_t pan = rd8(v + 3);
	c->voice_pan = (pan & 0x80) ? (int)pan - 0xC0 : 0;
	c->atk = rd8(v + 8);
	c->dec = rd8(v + 9);
	c->sus = rd8(v + 10);
	c->rel = rd8(v + 11);
	switch (type & 7) {
	case 0: {
		uint32_t sp = rd32(v + 4);
		if (!valid(sp)) { c->on = false; return; }
		uint32_t s = rom_off(sp);
		c->kind = V_DS;
		c->loop = (rd32(s) & 0x40000000u) != 0;
		c->sample_rate = rd32(s + 4);
		c->loop_start = rd32(s + 8);
		c->len = rd32(s + 12);
		if (c->len > ROM_SIZE || s + 16 + c->len > ROM_SIZE) { c->on = false; return; }
		c->data = (const int8_t *)(R.data + s + 16);
		c->fixed = (type & 0x08) != 0;
		break;
	}
	case 1:
	case 2:
		c->kind = (type & 7) == 1 ? V_SQ1 : V_SQ2;
		c->duty = (int)(rd32(v + 4) & 3);
		break;
	case 3: {
		uint32_t wp = rd32(v + 4);
		if (!valid(wp)) { c->on = false; return; }
		c->kind = V_WAVE;
		c->wave = R.data + rom_off(wp);
		break;
	}
	case 4:
		c->kind = V_NOISE;
		c->short_noise = (rd32(v + 4) & 1) != 0;
		c->lfsr = 0x7FFF;
		break;
	default:
		c->on = false;
		return;
	}
	if (c->kind != V_DS) {
		/* PSG envelopes step in fifteenths; scale them to the 0-255 model. */
		c->env = c->atk == 0 ? 255 : 0;
		c->phase = c->atk == 0 ? 1 : 0;
		c->sus = c->sus * 17;
	}
	chan_pitch(c, t);
	chan_volume(c, p, t);
}

static void note_off(int pi, int ti, int key) {
	for (int i = 0; i < MAX_CHANS; ++i) {
		Chan *c = &chans[i];
		if (c->on && c->player == pi && c->track == ti && c->midi_key == key && !c->released) chan_release(c);
	}
}

/* Once per GBA frame. */
static void chan_envelope(Chan *c) {
	c->age++;
	if (c->kind == V_DS) {
		switch (c->phase) {
		case 0: c->env += c->atk; if (c->env >= 255) { c->env = 255; c->phase = 1; } break;
		case 1: c->env = c->env * c->dec >> 8; if (c->env <= c->sus) { c->env = c->sus; c->phase = 2; } break;
		case 2: break;
		default: c->env = c->env * c->rel >> 8; if (c->env <= 1) c->on = false; break;
		}
		return;
	}
	int rate = c->phase == 0 ? c->atk : c->phase == 1 ? c->dec : c->phase == 3 ? c->rel : 0;
	if (c->phase == 2) return;
	if (rate == 0) {
		if (c->phase == 0) { c->env = 255; c->phase = 1; }
		else if (c->phase == 1) { c->env = c->sus; c->phase = 2; }
		else c->on = false;
		return;
	}
	if (++c->cgb_timer < rate) return;
	c->cgb_timer = 0;
	switch (c->phase) {
	case 0: c->env += 17; if (c->env >= 255) { c->env = 255; c->phase = 1; } break;
	case 1: c->env -= 17; if (c->env <= c->sus) { c->env = c->sus; c->phase = 2; } break;
	default: c->env -= 17; if (c->env <= 0) c->on = false; break;
	}
}

/* ------------------------------------------------------------------ */
/* Sequencer */

static void track_tick(int pi, int ti) {
	Player *p = &players[pi];
	Track *t = &p->tr[ti];
	if (!t->on) return;
	for (int i = 0; i < MAX_CHANS; ++i) {
		Chan *c = &chans[i];
		if (c->on && c->player == pi && c->track == ti && c->gate > 0 && !c->released && --c->gate == 0) chan_release(c);
	}
	if (t->wait > 0 && --t->wait > 0) goto refresh;
	for (int guard = 0; t->on && t->wait == 0 && guard < 512; ++guard) {
		uint8_t cmd = rd8(t->pc);
		if (cmd < 0x80) {
			/* running status: repeat the previous command, this byte is its argument */
			cmd = t->running;
			if (!cmd) { t->on = false; break; }
		} else {
			t->pc++;
			if (cmd >= 0xBD) t->running = cmd;
		}
		if (cmd <= 0xB0) { t->wait = len_table[cmd - 0x80]; continue; }
		if (cmd >= 0xCF) {
			/* Note: optional key, velocity and gate extension */
			if (rd8(t->pc) < 0x80) {
				t->key = rd8(t->pc++);
				if (rd8(t->pc) < 0x80) {
					t->vel = rd8(t->pc++);
					if (cmd != 0xCF && rd8(t->pc) < 0x80) t->gate = rd8(t->pc++);
					else t->gate = 0;
				}
			}
			note_on(pi, ti, t->key, t->vel, cmd == 0xCF ? -1 : len_table[cmd - 0xCF] + t->gate);
			continue;
		}
		switch (cmd) {
		case 0xB1: t->on = false; break;
		case 0xB2: t->pc = rom_off(rd32(t->pc)); break;
		case 0xB3:
			if (t->depth < 3) { t->ret[t->depth++] = t->pc + 4; t->pc = rom_off(rd32(t->pc)); }
			else t->pc += 4;
			break;
		case 0xB4: if (t->depth > 0) t->pc = t->ret[--t->depth]; break;
		case 0xB5: {
			uint8_t count = rd8(t->pc);
			uint32_t dst = rd32(t->pc + 1);
			if (count == 0 || ++t->rept_count < count) t->pc = rom_off(dst);
			else { t->rept_count = 0; t->pc += 5; }
			break;
		}
		case 0xB9: t->pc += 3; break;
		case 0xBA: t->prio = rd8(t->pc++); break;
		case 0xBB: p->tempo = rd8(t->pc++) * 2; break;
		case 0xBC: t->keysh = (int8_t)rd8(t->pc++); break;
		case 0xBD: t->voice = rd8(t->pc++); break;
		case 0xBE: t->vol = rd8(t->pc++); break;
		case 0xBF: t->pan = rd8(t->pc++) - 64; break;
		case 0xC0: t->bend = rd8(t->pc++) - 64; break;
		case 0xC1: t->bendr = rd8(t->pc++); break;
		case 0xC2: t->lfos = rd8(t->pc++); break;
		case 0xC3: t->lfodl = rd8(t->pc++); break;
		case 0xC4: t->mod = rd8(t->pc++); break;
		case 0xC5: t->modt = rd8(t->pc++); break;
		case 0xC8: t->tune = rd8(t->pc++) - 64; break;
		case 0xCD: t->pc += 2; break;
		case 0xCE:
			if (rd8(t->pc) < 0x80) note_off(pi, ti, rd8(t->pc++));
			else note_off(pi, ti, t->key);
			break;
		default: break;
		}
		if (t->pc >= ROM_SIZE) t->on = false;
	}
refresh:
	t->lfo_phase = (t->lfo_phase + t->lfos) & 255;
	for (int i = 0; i < MAX_CHANS; ++i) {
		Chan *c = &chans[i];
		if (c->on && c->player == pi && c->track == ti) { chan_pitch(c, t); chan_volume(c, p, t); }
	}
}

static void player_frame(int pi) {
	Player *p = &players[pi];
	if (!p->on) return;
	if (p->fade) {
		p->vol += p->fade;
		if (p->vol <= 0) {
			p->on = false;
			for (int i = 0; i < MAX_CHANS; ++i) if (chans[i].player == pi) chans[i].on = false;
			return;
		}
		if (p->vol >= 256) { p->vol = 256; p->fade = 0; }
	}
	p->tempo_acc += p->tempo;
	while (p->tempo_acc >= 150) {
		p->tempo_acc -= 150;
		bool any = false;
		for (int t = 0; t < p->ntracks; ++t) { track_tick(pi, t); any |= p->tr[t].on; }
		if (!any) { p->on = false; break; }
	}
}

static void player_start(int pi, int song) {
	if (song < 0 || song >= SONG_COUNT) return;
	uint32_t hdr_ptr = rd32(R.layout->song_table + (uint32_t)song * 8);
	if (!valid(hdr_ptr)) return;
	uint32_t h = rom_off(hdr_ptr);
	Player *p = &players[pi];
	for (int i = 0; i < MAX_CHANS; ++i) if (chans[i].on && chans[i].player == pi) chans[i].on = false;
	memset(p, 0, sizeof *p);
	p->ntracks = rd8(h);
	if (p->ntracks > MAX_TRACKS) p->ntracks = MAX_TRACKS;
	p->prio = rd8(h + 2) + (pi > 0 ? 256 : 0);
	p->voicegroup = rd32(h + 4);
	if (!valid(p->voicegroup)) return;
	p->tempo = 150;
	p->vol = 256;
	p->song = song;
	for (int t = 0; t < p->ntracks; ++t) {
		uint32_t tp = rd32(h + 8 + (uint32_t)t * 4);
		if (!valid(tp)) continue;
		Track *tr = &p->tr[t];
		tr->on = true;
		tr->pc = rom_off(tp);
		tr->vol = 100;
		tr->vel = 127;
		tr->key = 60;
		tr->bendr = 2;
	}
	p->on = true;
}

/* ------------------------------------------------------------------ */
/* Mixer */

static void mix(float *out, int frames) {
	float mv = music_volume / 10.0f * 0.3f, sv = sfx_volume / 10.0f * 0.75f;
	for (int i = 0; i < MAX_CHANS; ++i) {
		Chan *c = &chans[i];
		if (!c->on) continue;
		float g = (c->player == 0 ? mv : sv) * (float)c->env / 255.0f;
		float lv = c->lv * g, rv = c->rv * g;
		for (int f = 0; f < frames && c->on; ++f) {
			float s;
			switch (c->kind) {
			case V_DS: {
				uint32_t ip = (uint32_t)c->pos;
				if (ip >= c->len) {
					if (c->loop && c->len > c->loop_start) {
						while (c->pos >= c->len) c->pos -= (double)(c->len - c->loop_start);
						ip = (uint32_t)c->pos;
					} else { c->on = false; s = 0; continue; }
				}
				double fr = c->pos - ip;
				int a = c->data[ip];
				int b = ip + 1 < c->len ? c->data[ip + 1] : (c->loop ? c->data[c->loop_start] : 0);
				s = (float)(a + (b - a) * fr) / 128.0f;
				c->pos += c->step;
				break;
			}
			case V_SQ1:
			case V_SQ2: {
				static const float duty[4] = { 0.125f, 0.25f, 0.5f, 0.75f };
				s = c->phase_acc < duty[c->duty] ? 0.35f : -0.35f;
				c->phase_acc += c->step;
				if (c->phase_acc >= 1.0) c->phase_acc -= floor(c->phase_acc);
				break;
			}
			case V_WAVE: {
				int idx = (int)(c->phase_acc * 32.0) & 31;
				uint8_t b = c->wave[idx / 2];
				int nib = (idx & 1) ? (b & 15) : (b >> 4);
				s = (nib - 7.5f) / 21.0f;
				c->phase_acc += c->step;
				if (c->phase_acc >= 1.0) c->phase_acc -= floor(c->phase_acc);
				break;
			}
			default: {
				c->phase_acc += c->step;
				while (c->phase_acc >= 1.0) {
					c->phase_acc -= 1.0;
					uint32_t bit = (c->lfsr ^ (c->lfsr >> 1)) & 1;
					c->lfsr = (c->lfsr >> 1) | (bit << 14);
					if (c->short_noise) c->lfsr = (c->lfsr & ~0x40u) | (bit << 6);
				}
				s = (c->lfsr & 1) ? 0.3f : -0.3f;
				break;
			}
			}
			out[f * 2] += s * lv;
			out[f * 2 + 1] += s * rv;
		}
	}
}

static void render(int16_t *o, int frames) {
	static float buf[4096 * 2];
	const double per_frame = OUT_RATE / GBA_FPS;
	int done = 0;
	while (done < frames) {
		if (frame_acc < 1.0) {
			for (int p = 0; p < NUM_PLAYERS; ++p) player_frame(p);
			for (int i = 0; i < MAX_CHANS; ++i) if (chans[i].on) chan_envelope(&chans[i]);
			frame_acc += per_frame;
		}
		int n = (int)frame_acc;
		if (n > frames - done) n = frames - done;
		if (n > 4096) n = 4096;
		memset(buf, 0, sizeof(float) * (size_t)n * 2);
		mix(buf, n);
		for (int i = 0; i < n * 2; ++i) {
			float v = buf[i];
			/* soft clip keeps loud passages from crackling */
			v = v / (1.0f + fabsf(v) * 0.25f);
			if (v > 1.0f) v = 1.0f;
			if (v < -1.0f) v = -1.0f;
			o[done * 2 + i] = (int16_t)(v * 32000.0f);
		}
		done += n;
		frame_acc -= n;
	}
}

static FILE *dump; /* CYBERWORLD_AUDIO_DUMP: raw 48 kHz s16 stereo of everything played */
static bool offline; /* no device: audio_frame() renders the dump a frame at a time */

static AudioSource external; /* the embedded game, when it plays */

void audio_external(AudioSource src) { external = src; }

static void mix_out(int16_t *out, int frames) {
	if (external) external(out, frames);
	else render(out, frames);
}

static void callback(void *ud, Uint8 *stream, int len) {
	(void)ud;
	mix_out((int16_t *)stream, len / 4);
	if (dump) fwrite(stream, 1, (size_t)len, dump);
}

/* ------------------------------------------------------------------ */
/* Public API */

static const int sfx_ids[SFX_COUNT] = {
	[SFX_CURSOR] = 0x66, [SFX_SELECT] = 0x67, [SFX_CONFIRM] = 0x81, [SFX_CANCEL] = 0x68, [SFX_ERROR] = 0x69,
	[SFX_BUSTER] = 0x6A, [SFX_CHARGED] = 0x72, [SFX_CHARGE_SHOT] = 0x6A, [SFX_CANNON] = 0xAE, [SFX_SWORD] = 0xB0,
	[SFX_THROW] = 0xB2, [SFX_EXPLODE] = 0x6C, [SFX_HIT] = 0x6D, [SFX_HURT] = 0x6B, [SFX_GUARD] = 0xBA,
	[SFX_RECOVER] = 0xC7, [SFX_WAVE] = 0x97, [SFX_FLAME] = 0xE5, [SFX_THUNDER] = 0xF8, [SFX_WIND] = 0xFC,
	[SFX_GRAB] = 0x10E, [SFX_WARP] = 0x76, [SFX_DASH] = 0xE6, [SFX_CUSTOM_OPEN] = 0x79,
	[SFX_ITEM] = 0x73, [SFX_JACK_IN] = 0x77, [SFX_STEP] = 0x98,
	[SFX_ENCOUNTER] = 0x78, [SFX_MATERIALIZE] = 0x94, [SFX_CHIP_CURSOR] = 0x7F, [SFX_CHIP_SELECT] = 0x81,
	[SFX_CHIP_OK] = 0x82, [SFX_DESC_OPEN] = 0x9C, [SFX_DESC_CLOSE] = 0x9E, [SFX_GAUGE_FULL] = 0x8F,
	[SFX_DELETE] = 0x6F, [SFX_REVEAL] = 0x7E, [SFX_GOT] = 0x95, [SFX_CHARGE_START] = 0x71,
	[SFX_SHOCKWAVE] = 0xA6, [SFX_PICKAXE] = 0x183, [SFX_LOW_HP] = 0x84, [SFX_PAUSE] = 0x9F, [SFX_BOMB] = 0x70,
};

static const int music_ids[MUS_COUNT] = {
	[MUS_NONE] = 0, [MUS_TITLE] = 0x01, [MUS_NET] = 0x13, [MUS_BATTLE] = 0x15, [MUS_BOSS] = 0x16,
	[MUS_WIN] = 0x19, [MUS_UNDERNET] = 0x14, [MUS_SHOP] = 0x1E, [MUS_GAMEOVER] = 0x1B,
};

bool audio_init(void) {
	if (!R.data) return false;
	if (SDL_WasInit(SDL_INIT_AUDIO) == 0 && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return false;
	SDL_AudioSpec want = { 0 }, have;
	want.freq = OUT_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = 1024;
	want.callback = callback;
	const char *dp = getenv("CYBERWORLD_AUDIO_DUMP");
	if (dp) dump = fopen(dp, "wb");
	/* frame-exact dumps for comparisons: no device, one frame of sound per frame */
	if (dump && getenv("CYBERWORLD_AUDIO_OFFLINE")) { offline = true; return true; }
	dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (!dev) {
		fprintf(stderr, "audio: %s\n", SDL_GetError());
		offline = dump != NULL;
		return offline;
	}
	SDL_PauseAudioDevice(dev, 0);
	printf("audio %d Hz, %d channels, %d-sample buffer\n", have.freq, have.channels, have.samples);
	return true;
}

bool audio_offline(void) { return offline; }

void audio_frame(void) {
	if (!offline) return;
	static int16_t buf[OUT_RATE / 60 * 2];
	mix_out(buf, OUT_RATE / 60);
	fwrite(buf, sizeof buf, 1, dump);
}

static void lock(void) { if (dev) SDL_LockAudioDevice(dev); }
static void unlock(void) { if (dev) SDL_UnlockAudioDevice(dev); }

void audio_play_song(int id, bool music) {
	lock();
	if (music) {
		printf("music: song %#x\n", id);
		/* One song at a time: a music change also silences every effect player. */
		for (int p = 1; p < NUM_PLAYERS; ++p) players[p].on = false;
		for (int i = 0; i < MAX_CHANS; ++i) if (chans[i].player != 0) chans[i].on = false;
		player_start(0, id);
		current_music = id;
	} else {
		sfx_rr = sfx_rr % (NUM_PLAYERS - 1) + 1;
		/* Restart the same effect instead of stacking copies of it. */
		for (int p = 1; p < NUM_PLAYERS; ++p) if (players[p].on && players[p].song == id) sfx_rr = p;
		player_start(sfx_rr, id);
	}
	unlock();
}

void audio_sfx(Sfx s) {
	if (s < SFX_COUNT && getenv("CYBERWORLD_SFX_LOG")) {
		extern uint64_t audio_log_frame;
		fprintf(stderr, "sfx %llu %#x\n", (unsigned long long)audio_log_frame, sfx_ids[s]);
	}
	if ((!dev && !offline) || s >= SFX_COUNT || !sfx_ids[s]) return;
	audio_play_song(sfx_ids[s], false);
}

uint64_t audio_log_frame; /* frame number for CYBERWORLD_SFX_LOG, set by the main loop */

void audio_music_id(int id) {
	if (getenv("CYBERWORLD_SFX_LOG")) fprintf(stderr, "music %llu %#x\n", (unsigned long long)audio_log_frame, id);
	if (!dev && !offline) return;
	if (id == current_music && players[0].on) return;
	if (id == 0) {
		lock();
		players[0].fade = -12;
		current_music = 0;
		unlock();
		return;
	}
	audio_play_song(id, true);
}

void audio_music(Music m) {
	if (m >= MUS_COUNT) return;
	audio_music_id(music_ids[m]);
}

void audio_set_volume(int music, int sfx) {
	music_volume = music;
	sfx_volume = sfx;
}

/* Offline render for tests: writes a 16-bit stereo WAV. */
bool audio_render_wav(int song, int seconds, const char *path) {
	memset(players, 0, sizeof players);
	memset(chans, 0, sizeof chans);
	frame_acc = 0;
	player_start(0, song);
	FILE *f = fopen(path, "wb");
	if (!f) return false;
	uint32_t frames = (uint32_t)(OUT_RATE * seconds), bytes = frames * 4;
	uint8_t hdr[44] = { 'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ', 16, 0, 0, 0, 1, 0, 2, 0 };
	uint32_t v;
	v = 36 + bytes; memcpy(hdr + 4, &v, 4);
	v = OUT_RATE; memcpy(hdr + 24, &v, 4);
	v = OUT_RATE * 4; memcpy(hdr + 28, &v, 4);
	hdr[32] = 4; hdr[34] = 16;
	memcpy(hdr + 36, "data", 4);
	memcpy(hdr + 40, &bytes, 4);
	fwrite(hdr, 1, 44, f);
	int16_t buf[2048 * 2];
	for (uint32_t done = 0; done < frames;) {
		int n = frames - done > 2048 ? 2048 : (int)(frames - done);
		render(buf, n);
		fwrite(buf, 4, (size_t)n, f);
		done += (uint32_t)n;
	}
	fclose(f);
	return true;
}
