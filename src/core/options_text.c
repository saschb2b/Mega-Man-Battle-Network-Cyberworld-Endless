/* options_text.h. Menu words, short and plain: what each line opens or
 * sets, and the choices as docs/GUIDE.md's Screen and Sound say them. */
#include "options_text.h"


/* (in OptionsWord's order, its name beside each) */
static const char *const words[] = {
	"Options",   /* OW_TITLE */
	"Controls",   /* OW_CONTROLS */
	"Which buttons and keys are A, B, L, R, START and SELECT",   /* OW_CONTROLS_NOTE */
	"Audio",   /* OW_AUDIO */
	"Music and sounds, on or off, and their mix",   /* OW_AUDIO_NOTE */
	"Sound",   /* OW_SOUND */
	"GBA",   /* OW_GBA */
	"As a GBA plays it: BN6's own mix, crisp and bright",   /* OW_GBA_NOTE */
	"Soft",   /* OW_SOFT */
	"The Legacy Collection's balance: softer highs, less deep bass",   /* OW_SOFT_NOTE */
	"Display",   /* OW_DISPLAY */
	"How the picture fills the screen",   /* OW_DISPLAY_NOTE */
	"Fullscreen",   /* OW_FULLSCREEN */
	"Borderless, over the whole screen (Alt+Enter or F11)",   /* OW_FULLSCREEN_ON_NOTE */
	"In a window you can size (Alt+Enter or F11)",   /* OW_FULLSCREEN_OFF_NOTE */
	"Scale",   /* OW_SCALE */
	"Auto",   /* OW_AUTO */
	"Integer",   /* OW_WHOLE */
	"Fill",   /* OW_FILL */
	"Stretch",   /* OW_STRETCH */
	"Whole pixels, but filling where they leave it much smaller",   /* OW_AUTO_NOTE */
	"Whole-number scale: every pixel the same size",   /* OW_WHOLE_NOTE */
	"As big as the screen holds, its shape kept",   /* OW_FILL_NOTE */
	"The whole screen, no border: the picture out of shape",   /* OW_STRETCH_NOTE */
	"Smooth motion",   /* OW_SMOOTH */
	"Frames mixed at each refresh: even on 90 or 144 Hz",   /* OW_SMOOTH_ON_NOTE */
	"The game's own frames, sharp: best on 60 or 120 Hz",   /* OW_SMOOTH_OFF_NOTE */
	"On",   /* OW_ON */
	"Off",   /* OW_OFF */
	"INFORMATION",   /* OW_INFORMATION */
	"Music",   /* OW_MUSIC */
	"The game's music plays",   /* OW_MUSIC_ON_NOTE */
	"No music: play your own over the game",   /* OW_MUSIC_OFF_NOTE */
	"Effects",   /* OW_EFFECTS */
	"The sounds of menus, chips and battles play",   /* OW_EFFECTS_ON_NOTE */
	"No sounds from menus, chips or battles",   /* OW_EFFECTS_OFF_NOTE */
};
_Static_assert(sizeof words / sizeof *words == OW_COUNT, "a word for each OptionsWord");

const char *options_word(OptionsWord w) { return (unsigned)w < OW_COUNT ? words[w] : ""; }

const char *options_sound_note(void) {
	return "\n# sound: gba plays the game's sound as a GBA makes it, a bright hiss above\n"
		"# the music and all; soft gives it the Legacy Collection's balance, as\n"
		"# measured from it: the highs and the deepest bass taken down. Options\n"
		"# (the title's menu, or the PET's in a run) changes it too.\n";
}

const char *options_music_note(void) {
	return "\n# music: off silences the game's music, so your own can play over it; its\n"
		"# sounds play on, as effects says. Options' Audio changes it too.\n";
}

const char *options_effects_note(void) {
	return "\n# effects: off silences the game's sounds, its menus', chips' and\n"
		"# battles'. With music off as well, the game makes no sound. Options'\n"
		"# Audio changes it too.\n";
}

const char *options_screen_note(void) {
	return "\n# screen: whole (or integer) draws the game's picture at the largest whole\n"
		"# scale, every pixel the same size; fill makes it as big as the screen\n"
		"# holds, each pixel's edge a little soft; auto fills where a whole scale\n"
		"# would leave it a quarter smaller or more (a 640 x 480 screen), else\n"
		"# whole; stretch covers the whole screen, the picture out of its shape.\n"
		"# Options' Display changes it too.\n";
}

const char *options_smooth_note(void) {
	return "\n# smooth_motion: the game runs at the GBA's 60 frames a second. A 90, 144\n"
		"# or 165 Hz screen shows each frame for one refresh or two (or two or\n"
		"# three) in turn, which reads as judder. on mixes the two latest frames at\n"
		"# every refresh, so motion is even, a little blurred and a frame later;\n"
		"# off shows the game's own frames, sharp (best on 60 and 120 Hz). Options'\n"
		"# Display changes it too.\n";
}

const char *options_fullscreen_note(void) {
	return "\n# fullscreen: on starts the game borderless over the whole screen, off in\n"
		"# a window. Alt+Enter or F11 switch it while it runs, as Options' Display\n"
		"# does, and each keeps it here; --fullscreen or --window come first.\n";
}
