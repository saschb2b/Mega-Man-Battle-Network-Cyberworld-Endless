#include "feel_out.h"

#include <SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "buttons.h"
#include "pads.h"
#include "platform.h"

/* (a rumble sent lasts this long at most, sent again sooner while it
 * plays: a frame the game misses never leaves the motors running) */
#define RUMBLE_MS 150
#define RUMBLE_AGAIN 6
/* (a trigger's effect changed once the game has asked the same this many
 * frames: a frame's fade between two places never twitches it) */
#define TRIGGER_STEADY 6

static struct {
	int level;
	bool triggers;
	bool trace;
	FeelMix mix;
	uint16_t low, high;          /* the rumble sent last */
	int since;                   /* frames since */
	bool gauge, fast;            /* this frame's triggers, as the game asked */
	int pad;                     /* the pad the triggers were sent to (pads_feel_pad's id) */
	FeelTrigger left, right;     /* ... and what */
	FeelTrigger ask_left, ask_right;   /* what the game asks, */
	int steady;                  /* ... for this many frames */
	bool sent;                   /* ... and that it took them */
	bool dualsense;
} F = { .level = FEEL_DEFAULT, .triggers = true, .left = TRIGGER_OFF, .right = TRIGGER_OFF };

/* (Android's log takes SDL_Log, not stdout) */
static void say(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void say(const char *fmt, ...) {
	if (!F.trace) return;
	char s[200];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(s, sizeof s, fmt, ap);
	va_end(ap);
#ifdef __ANDROID__
	SDL_Log("feel: %s", s);
#else
	printf("feel: %s\n", s);
#endif
}

void feel_trace(bool on) { F.trace = on; }

/* A Bluetooth DualSense (and DualShock 4) rumbles only once SDL has put
 * it in its full reports, which this asks (a pad on USB has them): SDL's
 * hint, taken when it changes, kept by the pad till it is turned off */
void feel_set_level(int level) {
	F.level = level >= 0 && level < FEEL_LEVELS ? level : FEEL_DEFAULT;
	if (F.level != FEEL_OFF) {
		SDL_SetHint("SDL_JOYSTICK_HIDAPI_PS4_RUMBLE", "1");
		SDL_SetHint("SDL_JOYSTICK_HIDAPI_PS5_RUMBLE", "1");
	} else memset(&F.mix, 0, sizeof F.mix);
}

int feel_level(void) { return F.level; }
void feel_set_triggers(bool on) { F.triggers = on; }
bool feel_triggers(void) { return F.triggers; }
bool feel_here(void) {
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__) || defined(__3DS__)
	return false;
#else
	return true;
#endif
}

bool feel_dualsense(void) {
#ifdef CW_IOS
	return false;
#else
	return F.dualsense;
#endif
}

void feel_beat(FeelCue cue, uint8_t amount) {
	if (F.level == FEEL_OFF) return;
	say("frame %llu: %s %u", (unsigned long long)P.frame, feel_cue_name(cue), amount);
	feel_mix_start(&F.mix, cue, amount);
}

void feel_want_triggers(bool gauge, bool fast) {
	F.gauge = gauge;
	F.fast = fast;
}

static void rumble_frame(void) {
	uint16_t low, high;
	feel_mix_frame(&F.mix, F.level, &low, &high);
	++F.since;
	bool same = low == F.low && high == F.high;
	if (same && (!(low | high) || F.since < RUMBLE_AGAIN)) return;
	if (!same) say("frame %llu: rumble %u %u", (unsigned long long)P.frame, low, high);
	pads_rumble(low, high, low || high ? RUMBLE_MS : 0);
	F.low = low;
	F.high = high;
	F.since = 0;
}

/* a trigger's effect by what the map makes it: L or R clicks while the
 * Custom gauge is full, FAST where fast forward can run */
static FeelTrigger trigger_of(uint32_t buttons) {
	if (!F.triggers) return TRIGGER_OFF;
	if (F.gauge && (buttons & (BTN_L | BTN_R))) return TRIGGER_GAUGE;
	if (F.fast && (buttons & BTN_FAST)) return TRIGGER_FAST;
	return TRIGGER_OFF;
}

static void triggers_frame(void) {
	uint32_t l2, r2;
	int pad = pads_feel_pad(&F.dualsense, &l2, &r2);
	bool ds = feel_dualsense();
	FeelTrigger left = ds ? trigger_of(l2) : TRIGGER_OFF, right = ds ? trigger_of(r2) : TRIGGER_OFF;
	F.gauge = F.fast = false;
	if (left != F.ask_left || right != F.ask_right) {
		F.ask_left = left;
		F.ask_right = right;
		F.steady = 0;
	}
	++F.steady;
	/* (a pad come new takes them at once, its last effects cleared) */
	if (!ds || (pad == F.pad && ((left == F.left && right == F.right) || F.steady < TRIGGER_STEADY))) return;
	uint8_t report[TRIGGER_REPORT];
	feel_trigger_report(left, right, report);
	F.sent = pads_send_effect(report, (int)sizeof report);
	static const char *const names[] = { "off", "gauge", "fast" };
	say("frame %llu: triggers L2 %s, R2 %s%s", (unsigned long long)P.frame, names[left], names[right], F.sent ? "" : " (not taken)");
	F.pad = pad;
	F.left = left;
	F.right = right;
}

void feel_frame(void) {
	rumble_frame();
	triggers_frame();
}

void feel_close(void) {
	memset(&F.mix, 0, sizeof F.mix);
	if (F.low || F.high) pads_rumble(0, 0, 0);
	F.low = F.high = 0;
	if (feel_dualsense() && (F.left != TRIGGER_OFF || F.right != TRIGGER_OFF)) {
		uint8_t report[TRIGGER_REPORT];
		feel_trigger_report(TRIGGER_OFF, TRIGGER_OFF, report);
		pads_send_effect(report, (int)sizeof report);
		say("triggers freed");
	}
	F.left = F.right = TRIGGER_OFF;
}
