/* jukebox_text.h. Each song is named for where it plays: its maps' music
 * lists, its battles' records, the code that starts it (docs/PET.md, The
 * Jukebox, says which are verified and how); BN5's songs the two games
 * number alike carry BN6's names, and those of its own whose place is not
 * known yet carry their number. */
#include "jukebox_text.h"

#include <stdio.h>

const JukeSong jukebox_songs[] = {
	/* Battle Network 6 */
	{ 0, 0x03, JUKE_TOWNS, "Central Town" },
	{ 0, 0x04, JUKE_TOWNS, "Indoors" },
	{ 0, 0x05, JUKE_TOWNS, "Cyber Academy" },
	{ 0, 0x06, JUKE_TOWNS, "Seaside Town" },
	{ 0, 0x07, JUKE_TOWNS, "Sky Town" },
	{ 0, 0x08, JUKE_TOWNS, "Green Town" },
	{ 0, 0x24, JUKE_TOWNS, "ACDC Town" },
	{ 0, 0x25, JUKE_TOWNS, "The Expo" },
	{ 0, 0x13, JUKE_NET, "The Net" },
	{ 0, 0x10, JUKE_NET, "Robot Control" },
	{ 0, 0x11, JUKE_NET, "Aquarium Comp" },
	{ 0, 0x12, JUKE_NET, "Judge Tree Comp" },
	{ 0, 0x0A, JUKE_NET, "Mr.Weather Comp" },
	{ 0, 0x18, JUKE_NET, "Pavilion" },
	{ 0, 0x20, JUKE_NET, "Pavilion Comp" },
	{ 0, 0x09, JUKE_NET, "Graveyard" },
	{ 0, 0x14, JUKE_NET, "Undernet" },
	{ 0, 0x21, JUKE_NET, "Cybeasts" },
	{ 0, 0x15, JUKE_BATTLES, "Virus Busting" },
	{ 0, 0x1C, JUKE_BATTLES, "Before the Boss" },
	{ 0, 0x16, JUKE_BATTLES, "Boss Battle" },
	{ 0, 0x22, JUKE_BATTLES, "Showdown" },
	{ 0, 0x17, JUKE_BATTLES, "Final Battle" },
	{ 0, 0x23, JUKE_BATTLES, "The Chase" },
	{ 0, 0x02, JUKE_STORY, "Villains" },
	{ 0, 0x0B, JUKE_STORY, "Something's Up" },
	{ 0, 0x0C, JUKE_STORY, "Crisis" },
	{ 0, 0x0D, JUKE_STORY, "Sorrow" },
	{ 0, 0x0E, JUKE_STORY, "Good Times" },
	{ 0, 0x0F, JUKE_STORY, "Transmission" },
	{ 0, 0x01, JUKE_SCREENS, "Title" },
	{ 0, 0x1E, JUKE_SCREENS, "NaviCust" },
	{ 0, 0x19, JUKE_SCREENS, "Victory" },
	{ 0, 0x1F, JUKE_SCREENS, "Fanfare" },
	{ 0, 0x1A, JUKE_SCREENS, "Defeat" },
	{ 0, 0x1B, JUKE_SCREENS, "Game Over" },
	{ 0, 0x1D, JUKE_SCREENS, "Credits" },
	/* Battle Network 5 Team Colonel */
	{ 1, 0x03, JUKE_TOWNS, "ACDC Town" },
	{ 1, 0x04, JUKE_TOWNS, "Indoors" },
	{ 1, 0x05, JUKE_TOWNS, "School" },
	{ 1, 0x13, JUKE_NET, "The Net" },
	{ 1, 0x10, JUKE_NET, "Comp" },
	{ 1, 0x14, JUKE_NET, "Undernet" },
	{ 1, 0x15, JUKE_BATTLES, "Virus Busting" },
	{ 1, 0x1C, JUKE_BATTLES, "Before the Boss" },
	{ 1, 0x16, JUKE_BATTLES, "Boss Battle" },
	{ 1, 0x17, JUKE_BATTLES, "Final Battle" },
	{ 1, 0x02, JUKE_STORY, "Nebula" },
	{ 1, 0x0B, JUKE_STORY, "Something's Up" },
	{ 1, 0x0C, JUKE_STORY, "Crisis" },
	{ 1, 0x0D, JUKE_STORY, "Sorrow" },
	{ 1, 0x0E, JUKE_STORY, "Good Times" },
	{ 1, 0x0F, JUKE_STORY, "Transmission" },
	{ 1, 0x01, JUKE_SCREENS, "Title" },
	{ 1, 0x1E, JUKE_SCREENS, "NaviCust" },
	{ 1, 0x19, JUKE_SCREENS, "Victory" },
	{ 1, 0x1F, JUKE_SCREENS, "Fanfare" },
	{ 1, 0x1A, JUKE_SCREENS, "Defeat" },
	{ 1, 0x1B, JUKE_SCREENS, "Game Over" },
	{ 1, 0x1D, JUKE_SCREENS, "Credits" },
	{ 1, 0x06, JUKE_MORE, "Song 06" },
	{ 1, 0x07, JUKE_MORE, "Song 07" },
	{ 1, 0x08, JUKE_MORE, "Song 08" },
	{ 1, 0x09, JUKE_MORE, "Song 09" },
	{ 1, 0x11, JUKE_MORE, "Song 11" },
	{ 1, 0x12, JUKE_MORE, "Song 12" },
	{ 1, 0x18, JUKE_MORE, "Song 18" },
	{ 1, 0x20, JUKE_MORE, "Song 20" },
	{ 1, 0x21, JUKE_MORE, "Song 21" },
	{ 1, 0x22, JUKE_MORE, "Song 22" },
	{ 1, 0x23, JUKE_MORE, "Song 23" },
	{ 1, 0x24, JUKE_MORE, "Song 24" },
};
const int jukebox_song_count = (int)(sizeof jukebox_songs / sizeof *jukebox_songs);

const char *const jukebox_kind_names[JUKE_KINDS] = { "Towns", "The Net", "Battles", "Story", "Screens", "More" };
const char *const jukebox_game_tags[2] = { "BN6", "BN5" };
const char *const jukebox_mode_names[3] = { "Repeat", "In order", "Shuffle" };

const char jukebox_title[] = "Jukebox";
const char jukebox_now_label[] = "NOW PLAYING";
const char jukebox_idle_label[] = "READY";
const char jukebox_pick_hint[] = "Pick a song and press A.";
const char jukebox_key_play[] = "Play";
const char jukebox_key_back[] = "Back";
const char jukebox_key_section[] = "Section";

void jukebox_number_words(char *out, int n, int song) { snprintf(out, (size_t)n, "No. %02X", song); }

void jukebox_loops_words(char *out, int n, int loops) {
	if (loops <= 0) snprintf(out, (size_t)n, "%s", "");
	else if (loops == 1) snprintf(out, (size_t)n, "%s", "Round once");
	else snprintf(out, (size_t)n, "Round %d times", loops);
}
