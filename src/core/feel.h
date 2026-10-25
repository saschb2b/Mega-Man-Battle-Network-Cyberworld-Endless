/* The controller's feel (docs/GUIDE.md, Rumble and triggers): rumble on
 * the beats of a fight the screen and the sound already tell (MegaMan hit,
 * by the damage; the charged shot ready and fired; a counter's Full
 * Synchro; a virus or a Navi deleted; Beast Out and a Cross; MegaMan's
 * defeat) and on a jack-in, and a DualSense's adaptive triggers: the one
 * that is L or R clicks while the Custom gauge is full, FAST's where fast
 * forward can run. Never the only cue, and nothing while fast forward
 * runs. The rules alone, tested without SDL (tests/test_core.c): what a
 * frame's change of BN6's state raises (feel_game.c reads it), each beat's
 * shape and their mix, Options' strength, the triggers' bytes;
 * feel_out.c sends them to the pad. */
#ifndef CW_FEEL_H
#define CW_FEEL_H

#include <stdbool.h>
#include <stdint.h>

/* settings.ini's `vibration` (Options' Audio, PS5's own names): how
 * strong, or none */
enum { FEEL_OFF, FEEL_WEAK, FEEL_MEDIUM, FEEL_STRONG, FEEL_LEVELS };
#define FEEL_DEFAULT FEEL_MEDIUM
/* "off", "weak" ... and back (FEEL_DEFAULT for a word it does not know) */
const char *feel_level_word(int level);
int feel_level_read(const char *word);
/* Options' Left and Right: off, weak, medium, strong, and round */
int feel_level_next(int level, int dir);

/* A beat felt */
typedef enum {
	FEEL_HIT,           /* MegaMan's HP lowered (amount: how hard, by the damage) */
	FEEL_CHARGED,       /* his buster's charge full */
	FEEL_SHOT,          /* ... and the charged shot fired */
	FEEL_COUNTER,       /* Full Synchro: a counter hit */
	FEEL_DELETE,        /* a virus deleted (amount: by how many at once) */
	FEEL_NAVI_DELETE,   /* an enemy Navi deleted */
	FEEL_DEFEAT,        /* MegaMan deleted */
	FEEL_FORM,          /* a Cross or Beast Out taken */
	FEEL_JACK_IN,       /* from the real world into the net */
	FEEL_CUES
} FeelCue;
const char *feel_cue_name(FeelCue cue);

/* What a frame of BN6 shows that the feel follows (feel_game.c) */
typedef struct {
	bool fight;            /* in battle: the fight or its Custom screen */
	uint16_t hp, max_hp;   /* MegaMan's */
	uint8_t charge;        /* his buster: 0, 1 charging, 2 full (BN6_AI_CHARGE) */
	bool synchro;          /* Full Synchro */
	uint8_t form;          /* BN6_BATTLE_FORM: 0, a Cross, Beast Out */
	uint8_t viruses;       /* enemies with HP left: viruses, */
	uint8_t navis;         /* ... and Navis */
	bool real;             /* Lan in the real world (the town, home's places) */
	bool net;              /* MegaMan in the net (a layer, Lan's HP) */
} FeelGame;

typedef struct { FeelCue cue; uint8_t amount; } FeelBeat;
#define FEEL_BEATS_MOST 6
/* The beats from one frame's state to the next's, at most
 * FEEL_BEATS_MOST: how many */
int feel_beats(const FeelGame *was, const FeelGame *now, FeelBeat out[FEEL_BEATS_MOST]);

/* The mix: the beats playing, each its shape over frames, the stronger of
 * them at each motor (the low one heavy, the high one light); a voice of
 * amount 0 is free (a zeroed mix plays nothing) */
#define FEEL_VOICES 6
typedef struct {
	struct { int8_t cue; uint8_t amount, step, t; } voice[FEEL_VOICES];
} FeelMix;
void feel_mix_start(FeelMix *m, FeelCue cue, uint8_t amount);
/* A frame of it: the motors' 0-65535 at the strength `level`, each beat
 * one frame on; true while any plays */
bool feel_mix_frame(FeelMix *m, int level, uint16_t *low, uint16_t *high);

/* A DualSense trigger's effect: none, the click where the Custom gauge
 * is full (firm), the click where fast forward can run (light) */
typedef enum { TRIGGER_OFF, TRIGGER_GAUGE, TRIGGER_FAST } FeelTrigger;
#define TRIGGER_BYTES 11
#define TRIGGER_REPORT 47
/* its 11 bytes as the controller takes them: a mode byte, then its
 * numbers (Nielk1's "DualSense trigger effects", gist.github.com/Nielk1/
 * 6d54cc2c00d2201ccb8c2720ad7538db; docs/game/fidelity.md) */
void feel_trigger_bytes(FeelTrigger t, uint8_t out[TRIGGER_BYTES]);
/* the effects' report SDL_GameControllerSendEffect takes for a
 * DualSense, both triggers set (SDL_hidapi_ps5.c's DS5EffectsState_t:
 * byte 0's 0x04 the right trigger's, 0x08 the left's, at 10 and 21) */
void feel_trigger_report(FeelTrigger left, FeelTrigger right, uint8_t out[TRIGGER_REPORT]);

#endif
