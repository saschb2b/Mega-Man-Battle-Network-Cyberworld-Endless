/* The folder a NetBattle's MegaMan carries, asked (arena_folder.h). */
#include "arena_folder.h"

#include <stdio.h>

#include "arena_standin.h"
#include "arena_words.h"
#include "bn6.h"
#include "chips.h"
#include "emu.h"
#include "flags.h"
#include "online_square.h"
#include "script_kit.h"
#include "talk.h"
#include "text.h"

#define ACCEPT_FLAG 0x14A1   /* the Square's (online_chat.c's): Yes to a challenge */
#define FOLDER_FLAG 0x14A2   /* + k: folder k picked to carry, 0x14A2-0x14A4 */

bool arena_folder_challenge(const char *name, unsigned format) {
	flag_clear(ACCEPT_FLAG);
	return talk_ask(arena_challenged_words(name, format), FACE_MEGAMAN, arena_challenged_question(), ACCEPT_FLAG);
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

/* the folders whose BN5 chips sit out under `format`, a bit each */
static unsigned left_home(const OnlineLoadout *l, unsigned format) {
	unsigned bn5 = 0;
	for (int k = 0; k < ONLINE_FOLDERS; ++k)
		if (arena_folder_fit(l, format, k) == ARENA_FOLDER_BN5) bn5 |= 1u << k;
	return bn5;
}

void arena_folder_carry(const OnlineLoadout *all, unsigned format, int k, OnlineLoadout *out, bool say) {
	arena_format_carry(all, format, k, out);
	if (format != ONLINE_FORMAT_BN6) return;
	ArenaStandIn in[BN6_FOLDER_ENTRIES];
	int n = arena_standins(out, in, BN6_FOLDER_ENTRIES);
	for (int i = 0; say && i < n && i < BN6_FOLDER_ENTRIES; ++i)
		printf("arena: chip %u code %u sits out, chip %u code %u stands in\n", in[i].out & 0x1FFu, in[i].out >> 9u, in[i].in & 0x1FFu,
			in[i].in >> 9u);
}

/* a folder entry as BN6 names it: the chip, then its code */
static void entry_name(uint16_t e, char *out, size_t n) {
	char name[24];
	chips_name((int)(e & 0x1FFu), name, sizeof name);
	unsigned code = e >> 9u;
	snprintf(out, n, "%s %c", name, code == 26 ? '*' : (char)('A' + code));
}

/* MegaMan's words on the stand-ins of each folder that holds BN5's chips */
static const char *standins_said(const OnlineLoadout *l, unsigned format) {
	static char s[400];
	int len = 0;
	s[0] = 0;
	for (int k = 0; format == ONLINE_FORMAT_BN6 && k < ONLINE_FOLDERS; ++k) {
		if (arena_folder_fit(l, format, k) != ARENA_FOLDER_BN5) continue;
		OnlineLoadout carried;
		ArenaStandIn first = { 0, 0 };
		arena_format_carry(l, format, k, &carried);
		int n = arena_standins(&carried, &first, 1);
		if (n <= 0 || len >= (int)sizeof s) continue;
		char gone[32], in[32];
		entry_name(first.out, gone, sizeof gone);
		entry_name(first.in, in, sizeof in);
		len += snprintf(s + len, sizeof s - (size_t)len, "%s", arena_standin_words(arena_folder_name(k), n, gone, in));
	}
	return s;
}

bool arena_folder_asked(const OnlineLoadout *l, unsigned format, unsigned fits) {
	return l && ((fits & (fits - 1)) || left_home(l, format));
}

bool arena_folder_ask(const OnlineLoadout *l, unsigned format, unsigned fits) {
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	if (!l || !fits) return false;
	int folders[ONLINE_FOLDERS], n = 0;
	for (int k = 0; k < ONLINE_FOLDERS; ++k) if (fits >> k & 1) folders[n++] = k;
	ta_begin(&t);
	ta_script(&t);
	bool first = true;
	ta_pages(&t, arena_folder_words(format, standins_said(l, format), arena_souls_home(l, format)), FACE_MEGAMAN, &first);
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
