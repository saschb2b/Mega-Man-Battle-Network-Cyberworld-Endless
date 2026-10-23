/* What MegaMan says on the run's summary, in its NAVIGATOR box
 * (title_summary.c, docs/VOICE.md): how he came home, and his word. */
#include "title_lines.h"

const char *summary_words(int which) {
	static const char *const said[SUMMARY_LINES] = {
		[SUMMARY_WON_HOW] = "Home,safe and sound!",
		[SUMMARY_WON] = "Lan,we did it!!",
		[SUMMARY_LOST_HOW] = "Dad's backup worked!",
		[SUMMARY_LEARNED] = "We've got his battle data now!",
		[SUMMARY_BEST] = "Our deepest dive yet,Lan!",
		[SUMMARY_ROUGH] = "That was rough... Let's try again!",
		[SUMMARY_FURTHER] = "We'll get further next time!",
	};
	return which >= 0 && which < SUMMARY_LINES ? said[which] : "";
}
