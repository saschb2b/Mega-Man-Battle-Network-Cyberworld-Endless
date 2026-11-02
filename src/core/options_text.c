/* options_text.h. Menu words, short and plain: what each line opens or
 * sets, and the choices as docs/GUIDE.md's Screen and Sound say them. */
#include "options_text.h"


/* (in OptionsWord's order, its name beside each) */
static const char *const words[] = {
	"Options",   /* OW_TITLE */
	"Controls",   /* OW_CONTROLS */
	"Which buttons and keys are A, B, L, R, START and SELECT",   /* OW_CONTROLS_NOTE */
	"Audio",   /* OW_AUDIO */
	"Music and sounds, their mix, and the controller's rumble",   /* OW_AUDIO_NOTE */
	"Sound",   /* OW_SOUND */
	"GBA",   /* OW_GBA */
	"As a GBA plays it: BN6's own mix, crisp and bright",   /* OW_GBA_NOTE */
	"Soft",   /* OW_SOFT */
	"The Legacy Collection's balance: softer highs, less deep bass",   /* OW_SOFT_NOTE */
	"Display",   /* OW_DISPLAY */
	"How the picture fills the screen, and a GBA screen's look",   /* OW_DISPLAY_NOTE */
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
	"Fast forward",   /* OW_FAST */
	"FAST's button speeds up walks, talk and fades. Every fight stays at BN6's pace",   /* OW_FAST_ON_NOTE */
	"The game always at the GBA's pace: FAST's button does nothing",   /* OW_FAST_OFF_NOTE */
	"Vibration",   /* OW_VIBRATION */
	"Weak",   /* OW_WEAK */
	"Medium",   /* OW_MEDIUM */
	"Strong",   /* OW_STRONG */
	"The controller rumbles on hits, your attacks, counters and deletes",   /* OW_VIBRATION_ON_NOTE */
	"No rumble: the controller stays still",   /* OW_VIBRATION_OFF_NOTE */
	"Trigger effects",   /* OW_TRIGGERS */
	"L's trigger clicks at a full Custom gauge, FAST's where it can run",   /* OW_TRIGGERS_ON_NOTE */
	"Both triggers pull free, as on any controller",   /* OW_TRIGGERS_OFF_NOTE */
	"Colour correction",   /* OW_COLOUR */
	"mGBA's GBA colours: darker and softer, as a GBA's own screen shows them",   /* OW_COLOUR_GBA_NOTE */
	"The colours as the game stores them, bright and full",   /* OW_COLOUR_OFF_NOTE */
	"VBA",   /* OW_VBA */
	"DS",   /* OW_NDS */
	"PSP",   /* OW_PSP */
	"VisualBoyAdvance's GBA colours: darker still, and softer",   /* OW_COLOUR_VBA_NOTE */
	"The first DS's screen: dimmer whites, softer colours",   /* OW_COLOUR_NDS_NOTE */
	"A PSP 1000's or 2000's screen: bright, its colours warmer",   /* OW_COLOUR_PSP_NOTE */
	"Frame blending",   /* OW_BLEND */
	"Simple",   /* OW_SIMPLE */
	"Smart",   /* OW_SMART */
	"LCD",   /* OW_LCD */
	"LCD fast",   /* OW_LCD_FAST */
	"Every frame on its own, sharp",   /* OW_BLEND_OFF_NOTE */
	"mGBA's Simple: each frame mixed half and half with the last",   /* OW_SIMPLE_NOTE */
	"mGBA's Smart: frames mixed only where a pixel flickers",   /* OW_SMART_NOTE */
	"mGBA's LCD ghosting: each frame fades out over the next four, as on a GBA",   /* OW_LCD_NOTE */
	"mGBA's fast LCD ghosting: a longer trail, lighter to run",   /* OW_LCD_FAST_NOTE */
	"LCD 3x",   /* OW_LCD3X */
	"The lcd3x shader: each pixel striped red, green and blue, a dark line through each row",   /* OW_LCD3X_ON_NOTE */
	"Plain pixels, no stripes or lines",   /* OW_LCD3X_OFF_NOTE */
	"mGBA",   /* OW_MGBA */
	"mGBA's Audio Filter, as in RetroArch: its low-pass softens the highs",   /* OW_MGBA_NOTE */
	"Output",   /* OW_OUTPUT */
	"Speakers",   /* OW_SPEAKERS */
	"Headphones",   /* OW_HEADPHONES */
	"Mono",   /* OW_MONO */
	"The game's stereo, left and right as it plays them",   /* OW_SPEAKERS_NOTE */
	"Crossfeed: each ear hears a little of the other side, as from speakers",   /* OW_HEADPHONES_NOTE */
	"Both sides the same: for one speaker, one earbud or one ear",   /* OW_MONO_NOTE */
	"The game's music, quieter: each step down is 3 dB",   /* OW_MUSIC_LOW_NOTE */
	"Menus, chips and battles quieter: each step down is 3 dB",   /* OW_EFFECTS_LOW_NOTE */
};
_Static_assert(sizeof words / sizeof *words == OW_COUNT, "a word for each OptionsWord");

const char *options_word(OptionsWord w) { return (unsigned)w < OW_COUNT ? words[w] : ""; }

const char *options_level_word(int level) {
	static const char *const levels[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
	return level <= 0 ? words[OW_OFF] : levels[level > 10 ? 9 : level - 1];
}

const char *options_sound_note(void) {
	return "\n# sound: gba plays the game's sound as a GBA makes it, a bright hiss above\n"
		"# the music and all; soft gives it the Legacy Collection's balance, as\n"
		"# measured from it: the highs and the deepest bass taken down; mgba puts\n"
		"# it through mGBA's Audio Filter, the low-pass RetroArch's mGBA has, at\n"
		"# low_pass's level. Options (the title's menu, or the PET's in a run)\n"
		"# changes it too.\n";
}

const char *options_low_pass_note(void) {
	return "\n# low_pass: mGBA's Audio Filter Level, 5 to 95 (%), when sound is mgba:\n"
		"# the higher, the more of the highs it takes. 60 is mGBA's own default.\n";
}

const char *options_output_note(void) {
	return "\n# output: speakers plays the game's stereo as it is; headphones lets each\n"
		"# ear hear a little of the other side, lower and later, as from speakers\n"
		"# in a room (bs2b's crossfeed); mono plays both sides together on each,\n"
		"# for one speaker, one earbud or hearing on one side. Options' Audio\n"
		"# changes it too.\n";
}

const char *options_music_note(void) {
	return "\n# music: how loud the game's music plays, 10 (its own level) down to 1,\n"
		"# each step 3 dB quieter, or 0 (off) for none, so your own can play over\n"
		"# it; its sounds play on, as effects says. Options' Audio changes it too.\n";
}

const char *options_effects_note(void) {
	return "\n# effects: how loud the game's sounds play, its menus', chips' and\n"
		"# battles': 10 (their own level) down to 1, each step 3 dB quieter, or 0\n"
		"# (off) for none. With music at 0 as well, the game makes no sound.\n"
		"# Options' Audio changes it too.\n";
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

const char *options_fast_note(void) {
	return "\n# fast_forward: how much faster the game runs while fast forward is on:\n"
		"# 2x, 3x or 4x, or off. It runs on the net and in town, in chats, fades\n"
		"# and a battle's opening and close; never in the fight, on BN6's menus or\n"
		"# online. Its button, FAST in keys.ini and pad.ini (Tab or Space, a\n"
		"# controller's right trigger), turns it on and off; held, it is the other\n"
		"# way while held. Options changes it too.\n";
}

const char *options_vibration_note(void) {
	return "\n# vibration: how strongly the controller rumbles on a fight's beats (a hit,\n"
		"# by its damage; the charged shot ready and fired; a counter; a deletion;\n"
		"# Beast Out and a Cross; MegaMan's defeat), on his attacks as each sounds\n"
		"# and on a jack-in: strong, medium, weak or off. Never while fast forward\n"
		"# runs. Options' Audio changes it too.\n";
}

const char *options_triggers_note(void) {
	return "\n# trigger_effects: on, a DualSense's triggers click: the one that is L or R\n"
		"# once the Custom gauge is full, FAST's where fast forward can run (as\n"
		"# pad.ini maps them); off, both pull free. Options' Audio changes it too.\n";
}

const char *options_colour_note(void) {
	return "\n# colour_correction: gba shows the game's colours as a GBA's own screen\n"
		"# does, darker and softer, by mGBA's Color Correction (its libretro core's,\n"
		"# step for step); vba, nds and psp run libretro's handheld colour shaders\n"
		"# over the picture as RetroArch does: VisualBoyAdvance's GBA colours, the\n"
		"# first DS's screen, a PSP 1000's or 2000's; off as the game stores them.\n"
		"# Options' Display changes it too.\n";
}

const char *options_blend_note(void) {
	return "\n# interframe_blending: each frame mixed with the ones before, by mGBA's\n"
		"# Interframe Blending: simple (half and half with the one before), smart\n"
		"# (that only where a pixel flickers), lcd_ghosting (a GBA screen's fade\n"
		"# over the next four frames) or lcd_ghosting_fast (a running mix that\n"
		"# costs less); off shows every frame on its own. Options' Display changes\n"
		"# it too.\n";
}

const char *options_lcd3x_note(void) {
	return "\n# lcd3x: on draws RetroArch's lcd3x shader over the picture at the\n"
		"# screen's own pixels: each of the game's pixels striped red, green and\n"
		"# blue, a dark line through each row, the picture darker for it. Not on\n"
		"# the 3DS. Options' Display changes it too.\n";
}

const char *options_fullscreen_note(void) {
	return "\n# fullscreen: on starts the game borderless over the whole screen, off in\n"
		"# a window. Alt+Enter or F11 switch it while it runs, as Options' Display\n"
		"# does, and each keeps it here; --fullscreen or --window come first.\n";
}
