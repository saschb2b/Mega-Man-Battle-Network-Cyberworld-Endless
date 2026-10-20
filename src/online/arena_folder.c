/* The folder a NetBattle's MegaMan carries, asked (arena_folder.h). */
#include "arena_folder.h"

#include "arena_words.h"
#include "bn6.h"
#include "emu.h"
#include "flags.h"
#include "online_square.h"
#include "script_kit.h"
#include "talk.h"
#include "text.h"

#define ACCEPT_FLAG 0x14A1   /* the Square's (online_chat.c's): Yes to a challenge */
#define FOLDER_FLAG 0x14A2   /* + k: folder k picked to carry, 0x14A2-0x14A4 */

bool arena_folder_challenge(const char *name) {
	flag_clear(ACCEPT_FLAG);
	return talk_ask(arena_challenged_words(name), FACE_MEGAMAN, arena_challenged_question(), ACCEPT_FLAG);
}

bool arena_folder_challenge_yes(void) {
	bool yes = flag_get(ACCEPT_FLAG);
	flag_clear(ACCEPT_FLAG);
	return yes;
}

bool arena_folder_again(const char *said, const char *name) {
	flag_clear(ACCEPT_FLAG);
	return talk_ask(said, FACE_MEGAMAN, arena_again_question(name), ACCEPT_FLAG);
}

int arena_folder_first(unsigned fits) {
	for (int k = 0; k < ONLINE_FOLDERS; ++k) if (fits >> k & 1) return k;
	return 0;
}

bool arena_folder_asked(unsigned fits) { return (fits & (fits - 1)) != 0; }

bool arena_folder_ask(unsigned fits) {
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	if (!fits) return false;
	int folders[ONLINE_FOLDERS], n = 0;
	for (int k = 0; k < ONLINE_FOLDERS; ++k) if (fits >> k & 1) folders[n++] = k;
	ta_begin(&t);
	ta_script(&t);
	bool first = true;
	ta_pages(&t, arena_folder_words(), FACE_MEGAMAN, &first);
	ta_mugshot(&t, FACE_MEGAMAN);
	ta_clear(&t);
	const char *labels[ONLINE_FOLDERS];
	int to[ONLINE_FOLDERS];
	for (int i = 0; i < n; ++i) { labels[i] = arena_folder_name(folders[i]); to[i] = 1 + i; }
	ta_options(&t, labels, n);
	ta_choose(&t, to, n, 1 + n);
	for (int i = 0; i < n; ++i) {
		ta_script(&t);
		ta_flag_set(&t, FOLDER_FLAG + folders[i]);
		ta_end(&t);
	}
	ta_closing(&t);
	int size = ta_build(&t, out);
	arena_folder_reset();
	if (size > SQUARE_TALK_ROOM) return false;
	emu_write(online_square_talk_room(), out, (size_t)size);
	return talk_script(online_square_talk_room(), 0);
}

int arena_folder_picked(void) {
	int k = -1;
	for (int i = 0; i < ONLINE_FOLDERS; ++i)
		if (k < 0 && flag_get(FOLDER_FLAG + i)) k = i;
	arena_folder_reset();
	return k;
}

void arena_folder_reset(void) {
	for (int k = 0; k < ONLINE_FOLDERS; ++k) flag_clear(FOLDER_FLAG + k);
	flag_clear(ACCEPT_FLAG);
}
