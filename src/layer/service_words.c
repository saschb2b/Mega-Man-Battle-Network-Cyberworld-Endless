/* The services' chats (docs/VOICE.md): a Recovery Mr.Prog, a shop, a
 * Server's challenge and its prize, a flame of darkness and the rumor of
 * one, ProtoMan's duel, the Undernet's dark warp, the Secret Area's gate. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "darkchips.h"
#include "rom.h"
#include "script_kit.h"

int ta_heal(TextArchive *t, int variant, int told_flag, int amount, bool vow) {
	static const char *const hello[] = {
		"HELLO,MEGAMAN!\nLET ME PATCH YOU UP!",
		"DR.HIKARI SENT ME!\nHOLD STILL,NOW!",
		"RECOVERY PROGRAM,GO!\nTHIS WON'T HURT!",
	};
	/* once he has patched MegaMan on this layer (flag set): one patch a
	 * layer (issue #71: a heal as often as asked refunded every detour's
	 * HP), so his word that it is spent and where MiniEnrg is; the Heals
	 * helper's (amount < 0) heals again, in one box (an A too many after
	 * "fully restored" talked to him again, two boxes) */
	int back = ta_script(t);
	if (amount < 0) {
		ta_bytes(t, ta_full_hp, sizeof ta_full_hp);
		ta_page(t, FACE_PROG, "ALL PATCHED UP! COME BACK ANYTIME!", true);
	} else ta_page(t, FACE_PROG, "OH NO,MY PATCH IS\nUSED UP HERE!\nTHE NET DEALER SELLS\nMINIENRG!", true);
	ta_end(t);
	/* (a vow held, a request's (jobs.h): MegaMan names it, and the patch
	 * is on Yes, which starts on No; a playtester under one walked past a
	 * heal he could not tell what it would do to it, session 69) */
	int keep = vow ? ta_say(t, FACE_MEGAMAN, "Right! We keep our vow!") : -1;
	int i = ta_script(t);
	uint8_t check[] = { 0xEF, 0x00, (uint8_t)told_flag, (uint8_t)(told_flag >> 8), (uint8_t)back, 0xFF };  /* ts_check_flag */
	ta_bytes(t, check, sizeof check);
	if (vow) {
		ta_page(t, FACE_MEGAMAN, "Lan... A patch-up breaks our vow!", true);
		ta_ask_in(t, FACE_MEGAMAN, "Patch up anyway?\n", keep, true, true);
	}
	ta_page(t, FACE_PROG, hello[(unsigned)variant % 3], !vow);
	if (amount > 0) {
		/* (half of max HP, away from the arena: ts_start_heal, BN6's own,
		 * no higher than his max) */
		uint8_t heal[] = { 0xFB, 0x08, (uint8_t)amount, (uint8_t)(amount >> 8) };
		ta_bytes(t, heal, sizeof heal);
		char got[48];
		snprintf(got, sizeof got, "MegaMan recovered\nup to %d HP!", amount);
		ta_page(t, FACE_NONE, got, false);
	} else {
		ta_bytes(t, ta_full_hp, sizeof ta_full_hp);
		ta_page(t, FACE_NONE, "MegaMan's HP was fully restored!", false);
	}
	if (amount >= 0) ta_page(t, FACE_PROG, "ONE PATCH A LAYER!\nMAKE IT COUNT!", false);
	ta_flag_set(t, told_flag);
	ta_end(t);
	return i;
}

int ta_shop(TextArchive *t, int shop, int face, const char *greeting, const char *again, const char *sold_out, int told_flag) {
	/* (ts_wait a moment first: an A mashed through his words had opened
	 * the list on its first chip, "Are you sure? > Yes") */
	uint8_t open[] = { 0xEE, 0x00, 24, 0, 0xFB, 0x05, (uint8_t)shop };   /* ts_wait, ts_start_shop */
	bool first = true;
	/* bought out: his word for it, and no list (an empty one after "More
	 * programs? Take a look!" read as a broken shop, issue #17) */
	int gone = sold_out ? ta_say(t, face, sold_out) : -1;
	/* once his words are said (flag set), a line and the list: Kai sat
	 * through six boxes each time he came back */
	int back = -1;
	if (again && told_flag >= 0) {
		back = ta_script(t);
		ta_pages(t, again, face, &first);
		ta_bytes(t, open, sizeof open);
		ta_end(t);
		first = true;
	}
	int i = ta_script(t);
	if (gone >= 0) {
		uint8_t stock[] = { 0xEF, 0x22, (uint8_t)shop, 0xFF, (uint8_t)gone };   /* ts_check_shop_stock */
		ta_bytes(t, stock, sizeof stock);
	}
	if (back >= 0) {
		uint8_t check[] = { 0xEF, 0x00, (uint8_t)told_flag, (uint8_t)(told_flag >> 8), (uint8_t)back, 0xFF };  /* ts_check_flag */
		ta_bytes(t, check, sizeof check);
	}
	ta_pages(t, greeting, face, &first);
	if (back >= 0) ta_flag_set(t, told_flag);
	ta_bytes(t, open, sizeof open);
	ta_end(t);
	return i;
}

int ta_counter(TextArchive *t, int shop, int face, const char *greeting, int closed_flag, const char *closed) {
	uint8_t open[] = { 0xEE, 0x00, 24, 0, 0xFB, 0x05, (uint8_t)shop };   /* ts_wait, ts_start_shop */
	int shut = closed_flag >= 0 && closed ? ta_say(t, face, closed) : -1;
	int i = ta_script(t);
	if (shut >= 0) {
		uint8_t check[] = { 0xEF, 0x00, (uint8_t)closed_flag, (uint8_t)(closed_flag >> 8), (uint8_t)shut, 0xFF };  /* ts_check_flag */
		ta_bytes(t, check, sizeof check);
	}
	bool first = true;
	ta_pages(t, greeting, face, &first);
	ta_bytes(t, open, sizeof open);
	ta_end(t);
	return i;
}

int ta_challenge(TextArchive *t, int flag, const char *prize, const char *navi) {
	int quiet = ta_say(t, FACE_MEGAMAN, "The signal's quiet now,Lan.");
	int no = ta_closing(t);
	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)quiet, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	/* how strong, before the choice (a playtester took one on at 100 HP,
	 * not knowing, and came out at 10) */
	bool first = true;
	/* (a Navi's signal named as one: a playtester's "strong virus signal"
	 * held ElementMan SP, session 65) */
	char signal[160];
	if (navi && *navi) snprintf(signal, sizeof signal, "Lan,careful! A strong Navi's signal!|%s|He outclasses anything here!", navi);
	else snprintf(signal, sizeof signal, "Lan,careful! A strong virus signal!|Its viruses outclass anything here!");
	ta_pages(t, signal, FACE_MEGAMAN, &first);
	/* (and what it pays: "a good chip" left a playtester guessing whether
	 * the risk was worth it) */
	char ask[64];
	snprintf(ask, sizeof ask, "It pays %s.\nTake it on?\n", prize && *prize ? prize : "a good chip");
	ta_ask_in(t, FACE_MEGAMAN, ask, no, true, true);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_dark_flame(TextArchive *t, int flag, const char *chip, bool first, bool ours) {
	int no = ta_say(t, FACE_MEGAMAN, "Let's leave it be,Lan.");
	int i = ta_script(t);
	/* (the price named before the bargain, the whole of it a profile's
	 * first time: docs/META.md, issue #65) */
	/* (when it comes, said every time: a returning playtester heard only
	 * "when I'm hurt badly", session 68; and what BN5 does when MegaMan
	 * falls after using one, seen there: he rose at 1 HP and the darkness
	 * fought with his body a while) */
	/* (and a kind BN6 keeps too, `ours`: in our net's battles a chip of our
	 * folder that burns a BugFrag a use, docs/META.md, BN6's own DarkChips) */
	char words[800];
	if (first)
		snprintf(words, sizeof words, "L-Lan... That flame is dark data!|There's a DarkChip in it. %s!|%s.|But only when I'm worried!|"
			"Down to a quarter HP,or hit again and again.|You'll see it on my face.|A COUNTER calms me down.|%s"
			"It's real power... But it eats at me.|Each battle I use one in,I lose %d max HP.|For the rest of this dive!|"
			"And if I fall after using one...|The darkness might get me up,and fight in my body.|...That's not our way,Lan.", chip,
			ours ? "In the old Net's battles,it comes to me" : "It comes only in the old Net's battles",
			ours ? "Our Net knows this one,too!|In our battles,it's a Folder chip.|Each use burns a BugFrag.|" : "", DARK_PRICE);
	else snprintf(words, sizeof words, "A flame of darkness,Lan!|A DarkChip's inside. %s!|It comes in the old Net's battles,when I'm worried.|"
		"At a quarter HP,or hit again and again.%s|Each battle I use one in costs %d max HP.", chip,
		ours ? "|In ours,it's a Folder chip.|Each use burns a BugFrag." : "", DARK_PRICE);
	bool open = true;
	ta_pages(t, words, FACE_MEGAMAN, &open);
	/* (on Yes: holding one costs nothing till it's used, and a paging A
	 * that said No twice, sessions 67 and 68, sent a playtester through
	 * the whole chat again) */
	ta_ask_in(t, FACE_MEGAMAN, "Take the DarkChip?\n", no, true, false);
	ta_flag_set(t, flag);
	char got[48];
	snprintf(got, sizeof got, "MegaMan got:\n\"%s\"!!", chip);
	ta_page(t, FACE_NONE, got, false);
	ta_page(t, FACE_MEGAMAN, ours ? "It's ours now,Lan.\nAnd in our Pack,too.\nEither way...\nit takes its price."
		: "It's ours now,Lan...\nIt'll come when I'm\nworried. At a price.", false);
	ta_end(t);
	return i;
}

int ta_dark_flame6(TextArchive *t, int flag, const ScriptsDark6 *d) {
	int no = ta_say(t, FACE_MEGAMAN, "Let's leave it be,Lan.");
	int i = ta_script(t);
	/* (all of it a profile's first BN6 flame: what it does, the BugFrag it
	 * burns, the NaviCust's bug for the battle, its base chip, the max HP;
	 * the rule every time after, as BN5's flame says its own) */
	char words[720];
	const char *an = strchr("AEIOU", d->base[0]) && d->base[0] ? "an" : "a";
	if (d->first)
		snprintf(words, sizeof words, "L-Lan... That flame is dark data!|There's a DarkChip in it. %s!|Its darkness feeds on bugs!|"
			"Each use burns one of our BugFrags...|...for %s!|Then it bugs me,like a NaviCust bug.|For the rest of that battle!|"
			"No BugFrags? Then it's only %s %s.|And each battle it runs in costs %d max HP.|For the rest of this dive...",
			d->chip, d->does, an, d->base, DARK_PRICE);
	else snprintf(words, sizeof words, "A flame of darkness,Lan!|A DarkChip's inside. %s!|Each use burns a BugFrag...|...for %s.|"
		"Then it bugs me for the battle.|No BugFrags? Then it's %s %s.|Each battle it runs in costs %d max HP.", d->chip, d->does, an, d->base,
		DARK_PRICE);
	bool open = true;
	ta_pages(t, words, FACE_MEGAMAN, &open);
	/* (on Yes, as BN5's flame asks: holding one costs nothing till it is
	 * used) */
	ta_ask_in(t, FACE_MEGAMAN, "Take the DarkChip?\n", no, true, false);
	ta_flag_set(t, flag);
	char got[48];
	snprintf(got, sizeof got, "MegaMan got:\n\"%s\"!!", d->chip);
	ta_page(t, FACE_NONE, got, false);
	ta_page(t, FACE_MEGAMAN, "It's in our Pack!\nTo use it,pick PET,\nFolder,then EDIT.\nThree DarkChips max\nin a Folder,though.", false);
	ta_end(t);
	return i;
}

int ta_dark_rumor(TextArchive *t, int face) {
	static uint8_t line[320];
	static int len = -1;
	if (len < 0) {
		len = 0;
		uint32_t at = BN6_BBS_ARCHIVES - 0x08000000u + 4u * BN6_BBS_DARK_ARCHIVE, off = R.data && at + 4 <= ROM_SIZE ? rom_u32(at) - 0x08000000u : ROM_SIZE;
		size_t n = 0;
		/* (an archive of the ROM's, LZ77, its scripts past the four bytes
		 * BN6's unpacking skips) */
		uint8_t *a = off < ROM_SIZE && R.data[off] == 0x10 ? lz77_decompress(R.data + off, ROM_SIZE - off, &n) : NULL;
		if (a && n > 4) len = ta_rom_pages(a + 4, (int)n - 4, BN6_BBS_DARK_SCRIPT, line, (int)sizeof line);
		free(a);
	}
	if (!len) return -1;
	int i = ta_script(t);
	if (face >= 0) ta_mugshot(t, face);
	ta_open(t);
	ta_bytes(t, line, len);
	ta_end(t);
	return i;
}

int ta_duel(TextArchive *t, int flag, int face, const char *terms) {
	int no = ta_closing(t);
	int i = ta_script(t);
	/* ProtoMan's terms, then the choice, which starts on No (a duel costs
	 * HP; docs/RIVAL.md) */
	bool first = true;
	ta_pages(t, terms, face, &first);
	ta_ask_in(t, face, "Take the duel?\n", no, true, true);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_undernet(TextArchive *t, int flag, bool deeper) {
	int no = ta_closing(t);
	int i = ta_script(t);
	/* what is in there, where it is chosen (a playtester kept off one, not
	 * knowing what it was for): the Undernet's viruses, its richer Mystery
	 * Data and BugFrag Trader, and where its exit leads */
	bool first = true;
	ta_pages(t, deeper ? "Even tougher viruses down there!|And richer data,too!|Its exit leads back to the main path."
		: "The Undernet,Lan!|Tougher viruses,and richer data!|A BugFrag Trader,too!|Its exit leads to the next layer.",
		FACE_MEGAMAN, &first);
	ta_ask_in(t, FACE_MEGAMAN, deeper ? "Even deeper into the\nUndernet... Go in?\n" : "A warp into the\nUndernet! Go in?\n", no, true, true);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_secret_gate(TextArchive *t, int flag) {
	/* how many ScrtData MegaMan holds, said at a closed gate */
	int none = ta_say(t, FACE_MEGAMAN, "It's sealed tight,Lan.|Maybe ScrtData could open it...");
	int one = ta_say(t, FACE_MEGAMAN, "Our ScrtData's glowing a little...|I bet we need more!");
	int two = ta_say(t, FACE_MEGAMAN, "Whoa! Our two ScrtData are glowing!|One more should open it!");
	int no = ta_closing(t);
	/* ts_check_item07: item, amount, then the scripts for equal, more and
	 * fewer (0xFF: on); fewer than three says how many */
	int fewer = ta_script(t);
	uint8_t has2[] = { 0xEF, 0x07, SCRIPTS_SECRET_DATA, 2, (uint8_t)two, 0xFF, 0xFF };
	uint8_t has1[] = { 0xEF, 0x07, SCRIPTS_SECRET_DATA, 1, (uint8_t)one, 0xFF, (uint8_t)none };
	ta_bytes(t, has2, sizeof has2);
	ta_bytes(t, has1, sizeof has1);
	ta_end(t);
	int i = ta_script(t);
	uint8_t has3[] = { 0xEF, 0x07, SCRIPTS_SECRET_DATA, 3, 0xFF, 0xFF, (uint8_t)fewer };
	ta_bytes(t, has3, sizeof has3);
	ta_ask(t, FACE_MEGAMAN, "The ScrtData glows!\nOpen the gate?\n", no);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_music(TextArchive *t, int song) {
	int i = ta_script(t);
	uint8_t play[] = { 0xFD, 0x01, (uint8_t)song, (uint8_t)(song >> 8) };   /* ts_sound_play_bgm; 0xFF stops */
	static const uint8_t area[] = { 0xFD, 0x0A };                            /* ts_sound_play_area_bgm */
	if (song == SCRIPTS_AREA_MUSIC) ta_bytes(t, area, sizeof area);
	else ta_bytes(t, play, sizeof play);
	ta_end(t);
	return i;
}

int ta_challenge_reward(TextArchive *t, int chip, const char *chip_name, int code) {
	int i = ta_script(t);
	bool first = true;
	ta_page(t, FACE_MEGAMAN, "Look,Lan! The signal left a chip!", true);
	first = false;
	ta_give_chip(t, chip, code, 1);
	ta_got_chip(t, chip_name, code, &first);
	ta_page(t, FACE_MEGAMAN, "It's in our Pack!\nLet's add it to our\nFolder,Lan!", false);
	ta_end(t);
	return i;
}
