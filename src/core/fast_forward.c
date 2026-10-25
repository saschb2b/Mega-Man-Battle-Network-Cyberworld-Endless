/* fast_forward.h. Fast forward's rules: its speed, its button, where it
 * may run and how many frames a batch plays. */
#include "fast_forward.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"

static int speed = FAST_DEFAULT;

void fast_speed_set(int s) { speed = s < FAST_OFF ? FAST_OFF : s > FAST_MOST ? FAST_MOST : s; }
int fast_speed(void) { return speed; }

const char *fast_speed_word(int s) {
	static const char *const words[FAST_MOST + 1] = { "off", "off", "2x", "3x", "4x" };
	return words[s < FAST_OFF ? FAST_OFF : s > FAST_MOST ? FAST_MOST : s];
}

int fast_speed_read(const char *word) {
	for (int s = FAST_OFF; s <= FAST_MOST; ++s)
		if (!strcmp(word, fast_speed_word(s))) return s;
	if (!strcmp(word, "no") || !strcmp(word, "0") || !strcmp(word, "1")) return FAST_OFF;
	/* ("3" for "3x") */
	int n = 0;
	if (sscanf(word, "%d", &n) == 1 && n > FAST_OFF && n <= FAST_MOST) return n;
	return FAST_DEFAULT;
}

int fast_speed_next(int s, int dir) {
	int span = FAST_MOST - FAST_OFF + 1;
	return FAST_OFF + ((s - FAST_OFF + (dir < 0 ? span - 1 : 1)) % span);
}

void fast_button(FastButton *b, bool down) {
	if (down) {
		if (!b->held) b->on = !b->on;
		if (b->held < FAST_HOLD + 1) ++b->held;
		return;
	}
	/* (let go after a hold: back as it was before the press) */
	if (b->held > FAST_HOLD) b->on = !b->on;
	b->held = 0;
}

bool fast_scene_allows(const FastScene *s) {
	if (!s->run || s->apart) return false;
	/* (BN6's own menus, shops, traders and mail: a cursor's repeat at its pace) */
	if (s->toolkit != BN6_MODE_GAME || s->sub == BN6_SUB_PET) return false;
	/* (the fight, the Custom screen and the pause with it: BN6's clock; a
	 * Virus Battler's fight is the viruses' own, its VIRUS LIST a menu) */
	if (s->sub == BN6_SUB_BATTLE && s->mode == BN6_MODE_VIRUS_BATTLER) return s->phase != BN6_PHASE_CUSTOM;
	if (s->sub == BN6_SUB_BATTLE && (s->phase == BN6_PHASE_CUSTOM || s->phase == BN6_PHASE_FIGHT)) return false;
	return true;
}

bool fast_batch_more(int s, int played, double spent, double last, double budget) {
	return played < s && spent + last <= budget;
}
