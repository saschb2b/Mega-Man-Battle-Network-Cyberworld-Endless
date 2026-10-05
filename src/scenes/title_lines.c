/* What the title's summary says of the run's end (scene_title.c,
 * docs/VOICE.md): how MegaMan came home, and Lan's word. */
#include "title_lines.h"

const char *summary_words(int which) {
	static const char *const said[SUMMARY_LINES] = {
		[SUMMARY_WON_HOW] = "MegaMan jacked out,victorious!",
		[SUMMARY_WON] = "We did it,MegaMan!!",
		[SUMMARY_LOST_HOW] = "Dad's backup got MegaMan home.",
		[SUMMARY_LEARNED] = "We've got his battle data now!",
		[SUMMARY_BEST] = "Our deepest dive yet,MegaMan!",
		[SUMMARY_ROUGH] = "That was rough... Let's try again!",
		[SUMMARY_FURTHER] = "We'll get further next time!",
	};
	return which >= 0 && which < SUMMARY_LINES ? said[which] : "";
}
