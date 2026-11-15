/* The services' chats (docs/voice/rules.md): a Recovery Mr.Prog, a shop, a
 * Server's challenge (its pick in reward_words.c), a flame of darkness and
 * the rumor of one, ProtoMan's duel, the Undernet's dark warp, the Secret
 * Area's gate. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "darkchips.h"
#include "guardians.h"
#include "rom.h"
#include "run.h"
#include "script_kit.h"

/* (ts_check_flag: on to script `to` while `flag` is set) */
static void heal_check(TextArchive *t, int flag, int to) {
	uint8_t check[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)to, 0xFF };
	ta_bytes(t, check, sizeof check);
}

/* The patch's end: under a vow, the Mr.Prog's word that it was the
 * layer's one; the patch marked given. */
static void heal_said(TextArchive *t, int told_flag, bool vow) {
	if (vow) ta_page(t, FACE_PROG, "ONE PATCH A LAYER!\nMAKE IT COUNT!", false);
	ta_flag_set(t, told_flag);
	ta_end(t);
}

/* What MegaMan got from the patch, `got`, then its end. */
static void heal_page(TextArchive *t, const char *got, int told_flag, bool vow) {
	ta_page(t, FACE_NONE, got, false);
	heal_said(t, told_flag, vow);
}

/* The HP the patch gave, as said: the box's two lines into `s`, the
 * amount 1-9999; the second line's start, the amount's, returned. */
static const char *heal_line(char s[48], int amount) {
	snprintf(s, 48, "MegaMan recovered\n%dHP!", amount < 1 ? 1 : amount > 9999 ? 9999 : amount);
	const char *got = strchr(s, '\n');
	return got ? got + 1 : s;
}

void ta_heal_bytes(int amount, uint8_t a[2], uint8_t got[HEAL_GOT_LEN]) {
	amount = amount < 1 ? 1 : amount > 9999 ? 9999 : amount;
	a[0] = (uint8_t)amount;
	a[1] = (uint8_t)(amount >> 8);
	char s[48];
	memset(got, 0x00, HEAL_GOT_LEN);   /* (spaces, the charmap's 0x00: after "HP!", unseen) */
	ta_encode(heal_line(s, amount), got, HEAL_GOT_LEN);
}

/* ... said in a box of its own, the second line padded to HEAL_GOT_LEN
 * with spaces; where that line lies in the script's bytes. */
static int heal_got(TextArchive *t, int amount) {
	static const uint8_t hide[] = { 0xF5, 0x01 };   /* (no face, as ta_page's FACE_NONE) */
	static const uint8_t pad[HEAL_GOT_LEN] = { 0 };
	char s[48];
	int got = (int)(heal_line(s, amount) - s), len = (int)strlen(s);
	ta_bytes(t, hide, sizeof hide);
	ta_clear(t);
	int at = t->len + got;
	ta_text(t, s);
	if (len - got < HEAL_GOT_LEN) ta_bytes(t, pad, HEAL_GOT_LEN - (len - got));
	ta_wait(t);
	return at;
}

int ta_heal(TextArchive *t, int variant, int told_flag, int full_flag, int amount, bool vow, HealSpot *spot) {
	static const char full[] = "MegaMan's HP was fully restored!";
	static const char *const hello[] = {
		"HELLO,MEGAMAN!\nLET ME PATCH YOU UP!",
		"DR.HIKARI SENT ME!\nOUCH,YOU'RE HURT!",
		/* (ready, not "GO!": the patch waits on his question, kai5) */
		"RECOVERY PROGRAM,\nREADY TO GO!",
	};
	/* once he has patched MegaMan on this layer (flag set): one patch a
	 * layer (issue #71: a heal as often as asked refunded every detour's
	 * HP), so his word that it is spent and where MiniEnrg is; the Heals
	 * helper's (amount < 0) heals again, in one box (an A too many after
	 * "fully restored" talked to him again, two boxes) */
	if (spot) *spot = (HealSpot){ -1, -1 };
	int back = ta_script(t);
	if (amount < 0) {
		ta_bytes(t, ta_full_hp, sizeof ta_full_hp);
		ta_page(t, FACE_PROG, "ALL PATCHED UP! COME BACK ANYTIME!", true);
	} else ta_page(t, FACE_PROG, "OH NO,MY PATCH IS\nUSED UP HERE!\nTHE NET DEALER SELLS\nMINIENRG!", true);
	ta_end(t);
	/* (at full HP, the director's flag: he keeps his patch, where he had
	 * patched an unhurt MegaMan unasked and spent the layer's one, session
	 * 75) */
	int fine = ta_say(t, FACE_PROG, "YOU'RE IN TOP SHAPE!\nCOME BACK WHEN\nYOU'RE HURT!");
	/* (a vow held, a request's (jobs.h): MegaMan names it, and the patch
	 * is on Yes, which starts on No; a playtester under one walked past a
	 * heal he could not tell what it would do to it, session 69; the
	 * layer's one patch is on Yes too, his No keeping it) */
	int keep = vow ? ta_say(t, FACE_MEGAMAN, "Right! We keep our vow!") : -1;
	/* (half his max HP that reached it: said as a heal to full, where "up
	 * to 70 HP" at 80 of 140 read as his HP now, kai8, 15 November 2026) */
	int topped = -1;
	if (amount > 0) {
		topped = ta_script(t);
		heal_page(t, full, told_flag, vow);
	}
	bool asks = amount >= 0 && !vow;
	int later = asks ? ta_say(t, FACE_PROG, "OK! I'LL BE RIGHT\nHERE,MEGAMAN!") : -1;
	int i = ta_script(t);
	heal_check(t, told_flag, back);
	heal_check(t, full_flag, fine);
	if (vow) {
		ta_page(t, FACE_MEGAMAN, "Lan... A patch-up breaks our vow!", true);
		ta_ask_in(t, FACE_MEGAMAN, "Patch up anyway?\n", keep, true, true);
	}
	ta_page(t, FACE_PROG, hello[(unsigned)variant % 3], !vow);
	if (asks) ta_ask_in(t, FACE_PROG, "ONE PATCH A LAYER!\nUSE IT NOW?\n", later, true, false);
	if (amount > 0) {
		/* (half of max HP, away from the arena: ts_start_heal, BN6's own,
		 * no higher than his max, at once (bn6f chatbox_804252C); then
		 * ts_wait, a frame for the director's full flag to see it, and
		 * the HP he got, in BN6's own words for MiniEnrg's "recovers
		 * 50HP!", or "fully restored" where the half reached his max;
		 * the amount's bytes in places of their own, `spot`, which a
		 * CONTINUE sets again once its state holds his max HP, session 77) */
		uint8_t a[2], got[HEAL_GOT_LEN];
		ta_heal_bytes(amount, a, got);
		uint8_t heal[] = { 0xFB, 0x08, a[0], a[1], 0xEE, 0x00, 2, 0 };
		int amount_at = t->len + 2;
		ta_bytes(t, heal, sizeof heal);
		heal_check(t, full_flag, topped);
		int got_at = heal_got(t, amount);
		heal_said(t, told_flag, vow);
		/* (an archive that filled may have left them out) */
		if (spot && !t->full) *spot = (HealSpot){ amount_at, got_at };
	} else {
		ta_bytes(t, ta_full_hp, sizeof ta_full_hp);
		heal_page(t, full, told_flag, vow && amount == 0);
	}
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

int ta_challenge(TextArchive *t, int flag, int won_flag, int pick, const ScriptsVault *v, const char *navi) {
	int quiet = ta_say(t, FACE_MEGAMAN, "The signal's quiet now,Lan.");
	int no = ta_closing(t);
	int i = ta_script(t);
	/* (won, its pick waits: B left the three) */
	uint8_t won[] = { 0xEF, 0x00, (uint8_t)won_flag, (uint8_t)(won_flag >> 8), (uint8_t)pick, 0xFF };  /* ts_check_flag */
	ta_bytes(t, won, sizeof won);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)quiet, 0xFF };
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
	/* (and what it pays, the three by name: "a good chip" left a
	 * playtester guessing whether the risk was worth it) */
	char held[96];
	snprintf(held, sizeof held, "It holds %s %c,%s %c and %s %c!", v->name[0], v->code[0] == 26 ? '*' : 'A' + v->code[0],
		v->name[1], v->code[1] == 26 ? '*' : 'A' + v->code[1], v->name[2], v->code[2] == 26 ? '*' : 'A' + v->code[2]);
	ta_pages(t, held, FACE_MEGAMAN, &first);
	ta_ask_in(t, FACE_MEGAMAN, "Win,and one's ours!\nTake it on?\n", no, true, true);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_dark_flame(TextArchive *t, int flag, const char *chip, const char *does, bool first) {
	int no = ta_say(t, FACE_MEGAMAN, "Let's leave it be,Lan.");
	int i = ta_script(t);
	/* (the price named before the bargain, the whole of it a profile's
	 * first time: docs/game/meta/, issue #65; when it comes, said every time:
	 * a returning playtester heard only "when I'm hurt badly", session 68;
	 * BN5's own worried rule and its dark battle unsaid, and the Pack's
	 * EDIT after the take: the voice sweep, docs/voice/sweep/net.md.
	 * One by BN5's rule: BN5's worried rule deals it into his hand in our
	 * battles too, and one used makes him dark for that battle, as BN5's
	 * do: docs/multirom/one-net.md. DarkPlus and DarkInvs say what they
	 * do, as the owner's calls of 13 October 2026 ask: `does`, pages apart
	 * by '|', "" none) */
	const char *bar = does && *does ? "|" : "";
	if (!does) does = "";
	char words[720];
	if (first)
		snprintf(words, sizeof words, "L-Lan... That flame is dark data!|There's a DarkChip in it. %s!%s%s|It only comes when I'm worried.|"
			"It's real power... But it eats at me.|"
			"And each battle I use one in...|...takes %d max HP,for this whole dive!", chip, bar, does, DARK_PRICE);
	else snprintf(words, sizeof words, "A flame of darkness,Lan!|A DarkChip's inside. %s!%s%s|"
		"Each battle I use one in takes %d max HP,for the dive.", chip, bar, does, DARK_PRICE);
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
	ta_end(t);
	return i;
}

int ta_dark_flame6(TextArchive *t, int flag, const ScriptsDark6 *d) {
	int no = ta_say(t, FACE_MEGAMAN, "Let's leave it be,Lan.");
	int i = ta_script(t);
	/* (all of it a profile's first BN6 flame: what it does, the BugFrag it
	 * burns, the NaviCust's bug for the battle, its base chip, the max HP;
	 * the price every time after, as BN5's flame says its own: the rule
	 * once, docs/voice/sweep/net.md) */
	char words[720];
	const char *an = strchr("AEIOU", d->base[0]) && d->base[0] ? "an" : "a";
	if (d->first)
		snprintf(words, sizeof words, "L-Lan... That flame is dark data!|There's a DarkChip in it. %s!|Its darkness feeds on bugs!|"
			"Each use burns one of our BugFrags...|...for %s!|Then it bugs me,like a NaviCust bug.|For the rest of that battle!|"
			"No BugFrags? Then it's only %s %s.|And each battle it runs in...|...takes %d max HP,for this whole dive!",
			d->chip, d->does, an, d->base, DARK_PRICE);
	else snprintf(words, sizeof words, "A flame of darkness,Lan!|A DarkChip's inside. %s!|Each battle it runs in takes %d max HP,for the dive.", d->chip,
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
	/* (his own two boxes first, on the purple flame he walked past: the
	 * post is a reply on BN6's BBS, and alone its "A dar...DarkChip!?"
	 * came out of nowhere, a playtester's word, session 76; so he is
	 * the one it dawns on, and MegaMan's find at the flame stays his) */
	bool open = true;
	ta_pages(t, "Hey... Did you see that purple flame?|Something dark's burning in it...", face, &open);
	ta_clear(t);
	ta_bytes(t, line, len);
	ta_end(t);
	return i;
}

int ta_duel(TextArchive *t, int flag, int face, const char *terms, bool race) {
	int no = ta_closing(t);
	int i = ta_script(t);
	/* ProtoMan's terms, then the choice, which starts on No (a duel costs
	 * HP; docs/game/rival.md); a race's asked by MegaMan, as the layer's
	 * other risks are, after his own words on it (ProtoMan's "Take the
	 * duel?" read as a fight with him, where L and Chaud had said a race
	 * against his time, session 76) */
	bool first = true;
	ta_pages(t, terms, face, &first);
	if (race) ta_ask_in(t, FACE_MEGAMAN, "Race his time?\n", no, true, true);
	else ta_ask_in(t, face, "Take the duel?\n", no, true, true);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_undernet(TextArchive *t, int flag, bool deeper, bool server) {
	int no = ta_closing(t);
	int i = ta_script(t);
	/* what is in there, where it is chosen (a playtester kept off one, not
	 * knowing what it was for): the Undernet's viruses, its richer Mystery
	 * Data and BugFrag Trader, and where its exit leads */
	bool first = true;
	ta_pages(t, deeper ? "Even tougher viruses down there!|And richer data,too!" : "The Undernet,Lan!|Tougher viruses,and richer data!|A BugFrag Trader,too!",
		FACE_MEGAMAN, &first);
	/* (what the Undernet behind it holds, read from its seed: a Server's
	 * signal, as at the exit pad beside it, docs/world/level-design/fork-read.md) */
	if (server) ta_pages(t, "And a strong virus signal!", FACE_MEGAMAN, &first);
	ta_pages(t, deeper ? "Its exit leads back to the main path." : "Its exit leads to the next layer.", FACE_MEGAMAN, &first);
	ta_ask_in(t, FACE_MEGAMAN, deeper ? "Even deeper into the\nUndernet... Go in?\n" : "A warp into the\nUndernet! Go in?\n", no, true, true);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

/* What MegaMan senses behind the golden gate where Bass waits (docs/
 * docs/game/bosses/super-bosses.md): a dark signal, unnamed until they have met,
 * then Bass and his form; NULL for ProtoMan's copy */
static const char *gate_beyond(void) {
	if (run.boss_order[BIOME_SECRET] != SUPER_BASS) return NULL;
	if (!guardian_known(SUPER_BASS)) return "Lan... Something's behind this gate.|Its signal is so dark...";
	int form = super_form(SUPER_BASS, run.depth);
	return form == SUPER_BASS_BX ? "Bass is in there,Lan...|And the Cybeast's power is with him!"
		: form == SUPER_BASS_SP ? "Bass is in there,Lan.|He's even stronger than last time..." : "Bass is in there,Lan. I can feel him.";
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
	/* (Bass behind it: said first, and the question starts on No, as a
	 * risky choice's does) */
	const char *beyond = gate_beyond();
	bool first = true;
	if (beyond) ta_pages(t, beyond, FACE_MEGAMAN, &first);
	ta_ask_in(t, FACE_MEGAMAN, "The ScrtData glows!\nOpen the gate?\n", no, beyond != NULL, beyond != NULL);
	ta_flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_music(TextArchive *t, int song) {
	int i = ta_script(t);
	uint8_t play[] = { 0xFD, 0x01, (uint8_t)song, (uint8_t)(song >> 8) };   /* ts_sound_play_bgm (bn6f's PlayMusic; 0x63 stops) */
	static const uint8_t area[] = { 0xFD, 0x0A };                            /* ts_sound_play_area_bgm */
	if (song == SCRIPTS_AREA_MUSIC) ta_bytes(t, area, sizeof area);
	else ta_bytes(t, play, sizeof play);
	ta_end(t);
	return i;
}

int ta_lib_find_chip(TextArchive *t, int chip, const char *chip_name, int code) {
	int i = ta_script(t);
	bool first = true;
	ta_page(t, FACE_MEGAMAN, "Lan! There was a chip in that panel!", true);
	first = false;
	ta_give_chip(t, chip, code, 1);
	ta_got_chip(t, chip_name, code, &first);
	ta_end(t);
	return i;
}

int ta_lib_prize(TextArchive *t, int chip, const char *chip_name, int code) {
	int i = ta_script(t);
	bool first = true;
	ta_give_chip(t, chip, code, 1);
	ta_got_chip(t, chip_name, code, &first);
	ta_end(t);
	return i;
}

int ta_lib_find_zenny(TextArchive *t, int zenny) {
	int i = ta_script(t);
	bool first = false;
	char what[24];
	snprintf(what, sizeof what, "%d Zenny", zenny);
	ta_page(t, FACE_MEGAMAN, "Lan! Zenny in that panel!", true);
	ta_give_zenny(t, zenny);
	ta_got(t, what, &first);
	ta_end(t);
	return i;
}

/* (over par BN5's "You got:" and the Zennys, straight after the clear's
 * tally, as ta_lib_prize gives a chip) */
int ta_lib_prize_zenny(TextArchive *t, int zenny) {
	int i = ta_script(t);
	bool first = true;
	char what[24];
	snprintf(what, sizeof what, "%d Zenny", zenny);
	ta_give_zenny(t, zenny);
	ta_got(t, what, &first);
	ta_end(t);
	return i;
}
