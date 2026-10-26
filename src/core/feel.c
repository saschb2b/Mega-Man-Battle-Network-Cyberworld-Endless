#include "feel.h"

#include <string.h>

/* ---- Options' strength ---- */

static const char *const level_words[FEEL_LEVELS] = { "off", "weak", "medium", "strong" };

const char *feel_level_word(int level) { return level >= 0 && level < FEEL_LEVELS ? level_words[level] : level_words[FEEL_DEFAULT]; }

int feel_level_read(const char *word) {
	for (int i = 0; i < FEEL_LEVELS; ++i)
		if (!strcmp(word, level_words[i])) return i;
	/* (as settings.ini's other switches read theirs) */
	if (!strcmp(word, "no") || !strcmp(word, "0")) return FEEL_OFF;
	return FEEL_DEFAULT;
}

int feel_level_next(int level, int dir) { return ((level + (dir < 0 ? -1 : 1)) % FEEL_LEVELS + FEEL_LEVELS) % FEEL_LEVELS; }

/* ---- the beats ---- */

const char *feel_cue_name(FeelCue cue) {
	static const char *const names[FEEL_CUES] = { "hit", "charged", "shot", "counter", "delete", "navi-delete", "defeat", "form", "jack-in" };
	return (unsigned)cue < FEEL_CUES ? names[cue] : "?";
}

/* a hit's strength by its share of MegaMan's HP: a third of the full
 * shape for a scratch, all of it from a quarter of his HP */
static uint8_t hit_amount(int damage, int max_hp) {
	int share = max_hp > 0 ? damage * 256 / max_hp : 256;
	int full = share * 4 > 256 ? 256 : share * 4;
	return (uint8_t)(85 + full * 170 / 256);
}

static int put(FeelBeat out[FEEL_BEATS_MOST], int n, FeelCue cue, int amount) {
	if (n < FEEL_BEATS_MOST) out[n++] = (FeelBeat){ cue, (uint8_t)(amount > 255 ? 255 : amount) };
	return n;
}

/* (the fight's beats only between two frames of it: a battle's first
 * frame, its last and the results read what the last one left) */
static int fight_beats(const FeelGame *was, const FeelGame *now, FeelBeat out[FEEL_BEATS_MOST], int n) {
	if (now->hp == 0 && was->hp > 0) n = put(out, n, FEEL_DEFEAT, 255);
	else if (now->hp < was->hp) n = put(out, n, FEEL_HIT, hit_amount(was->hp - now->hp, now->max_hp));
	if (was->charge == 1 && now->charge == 2) n = put(out, n, FEEL_CHARGED, 255);
	if (was->charge == 2 && now->charge == 0) n = put(out, n, FEEL_SHOT, 255);
	if (!was->synchro && now->synchro) n = put(out, n, FEEL_COUNTER, 255);
	if (now->navis < was->navis) n = put(out, n, FEEL_NAVI_DELETE, 255);
	else if (now->viruses < was->viruses) n = put(out, n, FEEL_DELETE, 160 + 48 * (was->viruses - now->viruses));
	if (now->form != was->form && now->form) n = put(out, n, FEEL_FORM, 255);
	return n;
}

int feel_beats(const FeelGame *was, const FeelGame *now, FeelBeat out[FEEL_BEATS_MOST]) {
	int n = 0;
	if (was->fight && now->fight) n = fight_beats(was, now, out, n);
	if (was->real && now->net) n = put(out, n, FEEL_JACK_IN, 255);
	return n;
}

/* ---- their shapes and the mix ---- */

/* A shape's step: its frames, each motor's 0-255 from its start to its
 * end (the low motor heavy, the high one light); frames 0 ends it */
typedef struct { uint8_t frames, low0, high0, low1, high1; } Step;

static const Step hit[] = { { 3, 255, 150, 255, 150 }, { 7, 210, 30, 30, 0 }, { 0, 0, 0, 0, 0 } };
/* (the charge full: a crisp click over its hum; the charged shot: a recoil, heavier and longer than a buster's tick) */
static const Step charged[] = { { 2, 90, 230, 90, 230 }, { 3, 30, 80, 0, 0 }, { 0, 0, 0, 0, 0 } };
static const Step shot[] = { { 4, 255, 210, 255, 210 }, { 12, 210, 90, 0, 0 }, { 0, 0, 0, 0, 0 } };
static const Step counter[] = { { 4, 140, 230, 140, 230 }, { 4, 0, 0, 0, 0 }, { 6, 210, 255, 210, 255 }, { 5, 140, 120, 0, 0 }, { 0, 0, 0, 0, 0 } };
static const Step deleted[] = { { 4, 190, 110, 190, 110 }, { 9, 150, 40, 0, 0 }, { 0, 0, 0, 0, 0 } };
static const Step navi_delete[] = { { 10, 255, 180, 255, 180 }, { 48, 255, 120, 0, 0 }, { 0, 0, 0, 0, 0 } };
static const Step defeat[] = { { 14, 255, 60, 255, 60 }, { 60, 220, 0, 0, 0 }, { 0, 0, 0, 0, 0 } };
static const Step form[] = { { 22, 30, 20, 230, 150 }, { 5, 230, 150, 230, 150 }, { 9, 200, 100, 0, 0 }, { 0, 0, 0, 0, 0 } };
static const Step jack_in[] = { { 20, 0, 30, 60, 150 }, { 10, 60, 150, 0, 0 }, { 0, 0, 0, 0, 0 } };
static const Step *const shapes[FEEL_CUES] = { hit, charged, shot, counter, deleted, navi_delete, defeat, form, jack_in };

/* (the motors' share at each strength, in 256ths) */
static const int level_scale[FEEL_LEVELS] = { 0, 100, 180, 256 };

void feel_mix_start(FeelMix *m, FeelCue cue, uint8_t amount) {
	if ((unsigned)cue >= FEEL_CUES) return;
	/* (a free voice, else the one nearest its end: the newest beat is felt) */
	int at = 0, most = -1;
	for (int i = 0; i < FEEL_VOICES; ++i) {
		if (!m->voice[i].amount) { at = i; break; }
		if (m->voice[i].step > most) { most = m->voice[i].step; at = i; }
	}
	m->voice[at].cue = (int8_t)cue;
	m->voice[at].amount = amount;
	m->voice[at].step = m->voice[at].t = 0;
}

static int lerp(int a, int b, int t, int n) { return n > 1 ? a + (b - a) * t / (n - 1) : a; }

bool feel_mix_frame(FeelMix *m, int level, uint16_t *low, uint16_t *high) {
	int lo = 0, hi = 0;
	bool any = false;
	for (int i = 0; i < FEEL_VOICES; ++i) {
		if (!m->voice[i].amount) continue;
		const Step *s = &shapes[m->voice[i].cue][m->voice[i].step];
		int a = m->voice[i].amount, t = m->voice[i].t;
		int l = lerp(s->low0, s->low1, t, s->frames) * a / 255, h = lerp(s->high0, s->high1, t, s->frames) * a / 255;
		if (l > lo) lo = l;
		if (h > hi) hi = h;
		any = true;
		if (++m->voice[i].t >= s->frames) {
			m->voice[i].t = 0;
			if (!shapes[m->voice[i].cue][++m->voice[i].step].frames) m->voice[i].amount = 0;
		}
	}
	*low = feel_motor(level, lo);
	*high = feel_motor(level, hi);
	return any;
}

uint8_t feel_charge_hum(int frames) {
	if (frames < 0) return 0;
	int t = frames > 90 ? 90 : frames;
	return (uint8_t)(26 + t * 60 / 90);
}

uint8_t feel_charged_throb(int frames) {
	if (frames < 0) return 0;
	/* (a quick rise and a slower fall each beat, over a floor that never lets go) */
	int t = frames % FEEL_THROB_FRAMES, swell = t < 4 ? t * 25 : (FEEL_THROB_FRAMES - t) * 100 / (FEEL_THROB_FRAMES - 4);
	return (uint8_t)(50 + (swell > 100 ? 100 : swell) * 60 / 100);
}

uint16_t feel_motor(int level, int v) {
	int scale = level >= 0 && level < FEEL_LEVELS ? level_scale[level] : 0;
	v = v < 0 ? 0 : v > 255 ? 255 : v;
	return (uint16_t)(v * scale / 256 * 257);
}

void feel_mix_sound(int level, uint8_t sound_low, uint8_t sound_high, uint16_t *low, uint16_t *high) {
	uint16_t l = feel_motor(level, sound_low * FEEL_SOUND_SHARE / 256), h = feel_motor(level, sound_high * FEEL_SOUND_SHARE / 256);
	if (l > *low) *low = l;
	if (h > *high) *high = h;
}

/* ---- the triggers ---- */

/* Weapon (mode 0x25): resistance from zone `start` of the pull's ten to
 * `end`, where it gives way, a click: bytes 1-2 the two zones' bits, 3
 * the strength less one (1-8). The give at zone 5, half the pull, where
 * the press counts (pads.c's PUSHED, 16000 of 32767). */
static void weapon(uint8_t out[TRIGGER_BYTES], int start, int end, int strength) {
	uint16_t zones = (uint16_t)(1u << start | 1u << end);
	out[0] = 0x25;
	out[1] = (uint8_t)(zones & 0xFF);
	out[2] = (uint8_t)(zones >> 8);
	out[3] = (uint8_t)(strength - 1);
}

void feel_trigger_bytes(FeelTrigger t, uint8_t out[TRIGGER_BYTES]) {
	memset(out, 0, TRIGGER_BYTES);
	if (t == TRIGGER_GAUGE) weapon(out, 2, 5, 6);
	else if (t == TRIGGER_FAST) weapon(out, 2, 5, 3);
	/* (Off: mode 0x05, the trigger free) */
	else out[0] = 0x05;
}

void feel_trigger_report(FeelTrigger left, FeelTrigger right, uint8_t out[TRIGGER_REPORT]) {
	memset(out, 0, TRIGGER_REPORT);
	out[0] = 0x04 | 0x08;
	feel_trigger_bytes(right, out + 10);
	feel_trigger_bytes(left, out + 21);
}
