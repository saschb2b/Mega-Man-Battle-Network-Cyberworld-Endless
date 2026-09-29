/* Scripts for the layers' services and choices, on the game's own text
 * commands (bn6f text_script_commands.inc). Who speaks each box is its
 * face: Dr. Hikari's Mr. Progs (in capitals, as the game's own talk),
 * the Net Dealers, MegaMan asking Lan; what MegaMan gets is told the
 * game's way, without a face. */
#include "scripts.h"

#include <stdio.h>

#include "bn6.h"
#include "data.h"
#include "emu.h"
#include "rom.h"

void chat_marks_install(void) {
	static const char pairs[5][2] = { { 'R', 'V' }, { 'B', 'X' }, { 'E', 'X' }, { 'S', 'P' }, { 'F', 'Z' } };
	uint32_t font = BN6_CHAT_FONT - 0x08000000u, widths = BN6_CHAT_FONT_WIDTHS - 0x08000000u;
	/* (the font where it should be: an 'E' of 8 pixels, its top row full) */
	if (R.data[widths + 0x0F] != 8 || R.data[font + 0x0F * 0x60] != 0x11) return;
	for (int m = 0; m < 5; ++m) {
		int a = 0x0B + pairs[m][0] - 'A', b = 0x0B + pairs[m][1] - 'A';
		int wa = R.data[widths + (uint32_t)a], wb = R.data[widths + (uint32_t)b];
		uint8_t g[0x60] = { 0 };
		for (int y = 0; y < 12; ++y)
			for (int x = 0; x < 16; ++x) {
				int code = x < wa ? a : b, sx = x < wa ? x : x - wa;
				if (sx >= (x < wa ? wa : wb)) continue;
				uint8_t v = R.data[font + (uint32_t)code * 0x60 + (uint32_t)(y * 8 + sx / 2)];
				int ci = sx & 1 ? v >> 4 : v & 15;
				g[y * 8 + x / 2] |= (uint8_t)(x & 1 ? ci << 4 : ci);
			}
		emu_write(BN6_CHAT_FONT + (uint32_t)(0x40 + m) * 0x60, g, sizeof g);
		emu_write8(BN6_CHAT_FONT_WIDTHS + (uint32_t)(0x40 + m), (uint8_t)(wa + wb > 16 ? 16 : wa + wb));
	}
}

/* A Yes/No choice after `question`, asked with `face` as the game's
 * shopkeepers ask it: two options, then select (Yes continues, No and B
 * jump to `no`); in the box a page before it opened, with `after`. A risky
 * one (a fight, a warp away) starts on No: a playtester's A pressed through
 * a strong virus signal's words took it on at Yes, and cost him 150 HP. */
static void ask_in(TextArchive *t, int face, const char *question, int no, bool after, bool risky) {
	static const uint8_t horizontal[] = { 0xF7, 0x07, 0x0B };        /* ts_position_option_horizontal */
	static const uint8_t yes_opt[] = { 0xEB, 0x00, 0x11, 0x00 };     /* ts_option: left/right to No */
	static const uint8_t no_opt[] = { 0xEB, 0x00, 0x00, 0x11 };
	static const uint8_t space[] = { 0xEC, 0x00, 0x01 };
	uint8_t select[] = { 0xED, 0x06, (uint8_t)(risky ? 1 : 0), 0xFF, (uint8_t)no, 0xFF };   /* (ts_select: its length, the option it starts on) */
	if (face >= 0) ta_mugshot(t, face);
	if (after) ta_clear(t); else ta_open(t);
	ta_text(t, question);
	ta_bytes(t, horizontal, sizeof horizontal);
	ta_bytes(t, yes_opt, sizeof yes_opt);
	ta_bytes(t, space, sizeof space);
	ta_text(t, " Yes ");
	ta_bytes(t, no_opt, sizeof no_opt);
	ta_bytes(t, space, sizeof space);
	ta_text(t, " No ");
	ta_bytes(t, select, sizeof select);
}

static void ask(TextArchive *t, int face, const char *question, int no) { ask_in(t, face, question, no, false, false); }

static void flag_set(TextArchive *t, int flag) {
	uint8_t b[] = { 0xEA, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8) };
	ta_bytes(t, b, sizeof b);
}

int ta_say_flag(TextArchive *t, int face, const char *s, int flag) {
	int i = ta_script(t);
	flag_set(t, flag);
	bool first = true;
	ta_pages(t, s, face, &first);
	ta_end(t);
	return i;
}

/* closes the box: the answer No */
static int closing(TextArchive *t) {
	int i = ta_script(t);
	ta_end(t);
	return i;
}

static const uint8_t full_hp[] = { 0xFC, 0x03, 0x13, 0x00 };  /* ts_call_set_full_h_p */

int ta_heal(TextArchive *t, int variant, int told_flag) {
	static const char *const hello[] = {
		"I'M A RECOVERY PROGRAM FROM SCILAB! HOLD STILL, MEGAMAN...",
		"DR. HIKARI SENT ME TO PATCH YOU UP! HERE GOES...",
		"RECOVERY PROGRAM, RUNNING! THIS WON'T TAKE A SECOND!",
	};
	/* once he has spoken on this layer (flag set): the heal in one box (an
	 * A too many after "fully restored" talked to him again, two boxes) */
	int back = ta_script(t);
	ta_bytes(t, full_hp, sizeof full_hp);
	ta_page(t, FACE_PROG, "ALL PATCHED UP! COME BACK ANYTIME!", true);
	ta_end(t);
	int i = ta_script(t);
	uint8_t check[] = { 0xEF, 0x00, (uint8_t)told_flag, (uint8_t)(told_flag >> 8), (uint8_t)back, 0xFF };  /* ts_check_flag */
	ta_bytes(t, check, sizeof check);
	ta_page(t, FACE_PROG, hello[(unsigned)variant % 3], true);
	ta_bytes(t, full_hp, sizeof full_hp);
	ta_page(t, FACE_NONE, "MegaMan's HP was fully restored!", false);
	flag_set(t, told_flag);
	ta_end(t);
	return i;
}

int ta_shop(TextArchive *t, int shop, int face, const char *greeting, const char *again, int told_flag) {
	/* (ts_wait a moment first: an A mashed through his words had opened
	 * the list on its first chip, "Are you sure? > Yes") */
	uint8_t open[] = { 0xEE, 0x00, 24, 0, 0xFB, 0x05, (uint8_t)shop };   /* ts_wait, ts_start_shop */
	bool first = true;
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
	if (back >= 0) {
		uint8_t check[] = { 0xEF, 0x00, (uint8_t)told_flag, (uint8_t)(told_flag >> 8), (uint8_t)back, 0xFF };  /* ts_check_flag */
		ta_bytes(t, check, sizeof check);
	}
	ta_pages(t, greeting, face, &first);
	if (back >= 0) flag_set(t, told_flag);
	ta_bytes(t, open, sizeof open);
	ta_end(t);
	return i;
}

int ta_challenge(TextArchive *t, int flag) {
	int quiet = ta_say(t, FACE_MEGAMAN, "The virus signal's gone quiet, Lan.");
	int no = closing(t);
	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)quiet, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	/* how strong, before the choice (a playtester took one on at 100 HP,
	 * not knowing, and came out at 10) */
	bool first = true;
	ta_pages(t, "Lan, a strong virus\nsignal! Its viruses\noutclass this layer.", FACE_MEGAMAN, &first);
	ask_in(t, FACE_MEGAMAN, "It pays a good chip.\nTake it on?\n", no, true, true);
	flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_undernet(TextArchive *t, int flag, bool deeper) {
	int no = closing(t);
	int i = ta_script(t);
	/* what is in there, where it is chosen (a playtester kept off one, not
	 * knowing what it was for): the Undernet's viruses, its richer Mystery
	 * Data and BugFrag Trader, and where its exit leads */
	bool first = true;
	ta_pages(t, deeper ? "Tougher viruses still, and richer data!|Its exit leads back to the main path."
		: "The Undernet: tougher viruses and richer data!|A BugFrag Trader too. Its exit leads to the next layer.",
		FACE_MEGAMAN, &first);
	ask_in(t, FACE_MEGAMAN, deeper ? "Even deeper into the\nUndernet... Go in?\n" : "A warp into the\nUndernet! Go in?\n", no, true, true);
	flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_secret_gate(TextArchive *t, int flag) {
	/* how many ScrtData MegaMan holds, said at a closed gate */
	int none = ta_say(t, FACE_MEGAMAN, "It's sealed tight, Lan.|Maybe that strange ScrtData could open it...");
	int one = ta_say(t, FACE_MEGAMAN, "It's sealed tight. Our ScrtData is glowing a little...|I bet we need more of it!");
	int two = ta_say(t, FACE_MEGAMAN, "Our two ScrtData are glowing brighter, Lan!|One more should open this gate!");
	int no = closing(t);
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
	ask(t, FACE_MEGAMAN, "The ScrtData glows!\nOpen the gate?\n", no);
	flag_set(t, flag);
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

/* ts_item_give_chip: `count` of chip `id` with code index `code` (A=0, *=26) */
static void give_chip(TextArchive *t, int id, int code, int count) {
	uint8_t b[] = { 0xF4, 0x10, (uint8_t)id, (uint8_t)(id >> 8), (uint8_t)code, (uint8_t)count };
	ta_bytes(t, b, sizeof b);
}

/* ts_item_give HPMemory, with its jingle */
static void give_hp_memory(TextArchive *t, int count) {
	uint8_t b[] = { 0xF4, 0x00, SCRIPTS_HP_MEMORY, (uint8_t)count };
	ta_bytes(t, b, sizeof b);
}

/* What MegaMan got, as the game says it: `MegaMan got:` then the thing in
 * quotes. */
static void got(TextArchive *t, const char *thing, bool *first) {
	char line[64];
	snprintf(line, sizeof line, "MegaMan got:\n\"%s\"!!", thing);
	ta_page(t, FACE_NONE, line, *first);
	*first = false;
}

static void got_hp(TextArchive *t, int count, bool *first) {
	char thing[32];
	if (count > 1) snprintf(thing, sizeof thing, "%d HPMemory", count);
	else snprintf(thing, sizeof thing, "HPMemory");
	got(t, thing, first);
}

static void got_chip(TextArchive *t, const char *chip, int code, bool *first) {
	char thing[40];
	snprintf(thing, sizeof thing, "%s %c", chip, code == 26 ? '*' : 'A' + code);
	got(t, thing, first);
}

static void program_name(TextArchive *t, int program);

/* ts_jump: on in script `to` (the way on, after a branch of the draft) */
static void jump(TextArchive *t, int to) {
	uint8_t b[] = { 0xF0, 0x00, (uint8_t)to };
	ta_bytes(t, b, sizeof b);
}

/* A script's end: the way on's question next, where there is one (`next`
 * its script), else the chat's end. */
static void end_or(TextArchive *t, int next) {
	if (next >= 0) jump(t, next);
	else ta_end(t);
}

/* The way on: the question, the two ways in a column, a branch each (the
 * second sets the flag; B takes the first); the question's script. */
static int route_scripts(TextArchive *t, const ScriptsRoute *r) {
	int way[3] = { 0 }, n = r->n == 3 ? 3 : 2;
	for (int k = 0; k < n; ++k) {
		way[k] = ta_script(t);
		if (k) flag_set(t, k == 1 ? r->flag : r->dark_flag);
		ta_page(t, FACE_MEGAMAN, r->then[k], true);
		ta_end(t);
	}
	int q = ta_script(t);
	bool first = false;   /* (the chat box is open: the Guardian Data's) */
	ta_pages(t, r->question, FACE_MEGAMAN, &first);
	ta_mugshot(t, FACE_MEGAMAN);
	ta_clear(t);
	/* (in a column, as the draft's: up and down move, left and right too
	 * where two) */
	static const uint8_t opt2[2][4] = { { 0xEB, 0x00, 0x00, 0x11 }, { 0xEB, 0x00, 0x11, 0x00 } };
	static const uint8_t opt3[3][4] = { { 0xEB, 0x00, 0x00, 0x21 }, { 0xEB, 0x00, 0x11, 0x02 }, { 0xEB, 0x00, 0x22, 0x10 } };
	static const uint8_t space[] = { 0xEC, 0x00, 0x01 };
	for (int k = 0; k < n; ++k) {
		ta_bytes(t, n == 3 ? opt3[k] : opt2[k], 4);
		ta_bytes(t, space, sizeof space);
		ta_text(t, r->option[k]);
		if (k + 1 < n) ta_text(t, "\n");
	}
	if (n == 3) {
		uint8_t select[] = { 0xED, 0x07, 0xA0, (uint8_t)way[0], (uint8_t)way[1], (uint8_t)way[2], (uint8_t)way[0] };
		ta_bytes(t, select, sizeof select);
	} else {
		uint8_t select[] = { 0xED, 0x06, 0xA0, (uint8_t)way[0], (uint8_t)way[1], (uint8_t)way[0] };
		ta_bytes(t, select, sizeof select);
	}
	ta_end(t);
	return q;
}

/* the draft's branches: a program given, or BugFrags for none; each sets
 * `taken_flag` and ends, or goes on to the way on (`next`) */
static int draft_take(TextArchive *t, int program, int color, bool teach, int taken_flag, int next) {
	int s = ta_script(t);
	uint8_t give[] = { 0xEF, 0x1B, (uint8_t)program, 1, (uint8_t)color };   /* ts_item_give_navi_cust_program */
	ta_bytes(t, give, sizeof give);
	static const uint8_t no_face[] = { 0xF5, 0x01 };   /* (the options kept MegaMan's) */
	ta_bytes(t, no_face, sizeof no_face);
	ta_open(t);
	ta_text(t, "MegaMan got:\n\"");
	program_name(t, program);
	ta_text(t, "\"!!");
	ta_wait(t);
	/* (every time: a returning playtester, told once runs before, left
	 * one off the board for two acts) */
	ta_page(t, FACE_MEGAMAN, "Let's install it, Lan! In the PET: MegaMan, then NaviCust.", false);
	(void)teach;
	flag_set(t, taken_flag);
	end_or(t, next);
	return s;
}

static int draft_skip(TextArchive *t, int frags, int taken_flag, int next) {
	int s = ta_script(t);
	uint8_t give[] = { 0xEF, 0x12, (uint8_t)frags, (uint8_t)(frags >> 8), 0, 0, 0xFF, 0xFF, 0xFF };   /* ts_check_give_bug_frags */
	ta_bytes(t, give, sizeof give);
	char line[96];
	snprintf(line, sizeof line, "We'll travel light, Lan. The program data broke down into %d BugFrags!", frags);
	ta_page(t, FACE_MEGAMAN, line, false);
	flag_set(t, taken_flag);
	end_or(t, next);
	return s;
}

int ta_guardian_reward(TextArchive *t, const char *name, const char *power, int chip, const char *chip_name, int code,
                       int taken_flag, int hp_memories, const ScriptsDraft *draft, const ScriptsRoute *route) {
	/* (the way on, then the draft's branches first: the choices jump to
	 * them) */
	int next = route ? route_scripts(t, route) : -1;
	int take[3] = { 0 }, skip = 0, n = draft ? draft->n : 0;
	for (int k = 0; k < n; ++k) take[k] = draft_take(t, draft->program[k], draft->color[k], draft->teach, taken_flag, next);
	if (n) skip = draft_skip(t, draft->skip_frags, taken_flag, next);
	int i = ta_script(t);
	char head[64];
	bool first = true;
	snprintf(head, sizeof head, "MegaMan downloaded %s's Guardian Data!", name);
	ta_page(t, FACE_NONE, head, first);
	first = false;
	if (power) ta_pages(t, power, FACE_NONE, &first);
	give_hp_memory(t, hp_memories);
	got_hp(t, hp_memories, &first);
	if (chip > 0) {
		/* the navi's own chip, as Battle Network gives it (to the pack) */
		give_chip(t, chip, code, 1);
		got_chip(t, chip_name, code, &first);
		ta_page(t, FACE_MEGAMAN, "It's in our pack, Lan. Let's put it in our folder from the PET!", false);
	}
	ta_bytes(t, full_hp, sizeof full_hp);
	ta_page(t, FACE_NONE, "MegaMan's HP was fully restored!", false);
	if (draft && draft->expmemry) {
		/* BN6's own ExpMemry (key item 0x71): the game grows the board and
		 * runs the NaviCust again as it gives it */
		uint8_t give[] = { 0xF4, 0x00, SCRIPTS_EXP_MEMORY, 1 };   /* ts_item_give */
		ta_bytes(t, give, sizeof give);
		got(t, "ExpMemry", &first);
		/* (the board it grows to is the game's count, not the act's: a run
		 * that passed act 2 on an older build gets its first here) */
		ta_page(t, FACE_MEGAMAN, "Our NaviCust's board just grew, Lan! More room for programs.", false);
	}
	if (!n) {
		flag_set(t, taken_flag);
		end_or(t, next);
		return i;
	}
	/* the draft: what each program does, then the choice (B: none) */
	ta_page(t, FACE_MEGAMAN, "Program data too, Lan! Pick one for our NaviCust:", false);
	for (int k = 0; k < n; ++k) if (draft->about[k]) ta_page(t, FACE_MEGAMAN, draft->about[k], false);
	if (draft->teach)
		ta_page(t, FACE_MEGAMAN, "Big programs need a block on the command line; plus parts go anywhere else. "
			"Same colors touching or a block off the edge: a bug. L and R turn a program as we place it from the list.", false);
	char none[96];
	snprintf(none, sizeof none, "Or B takes none: the data breaks down into %d BugFrags.", draft->skip_frags);
	ta_page(t, FACE_MEGAMAN, none, false);
	ta_mugshot(t, FACE_MEGAMAN);
	ta_clear(t);
	/* three in a column (ts_option), as the gift's; ts_select: clear, B its
	 * own choice (0xA0), a script per option and one for B */
	static const uint8_t opt[3][4] = { { 0xEB, 0x00, 0x00, 0x21 }, { 0xEB, 0x00, 0x11, 0x02 }, { 0xEB, 0x00, 0x22, 0x10 } };
	static const uint8_t space[] = { 0xEC, 0x00, 0x01 };
	for (int k = 0; k < n; ++k) {
		ta_bytes(t, opt[k], 4);
		ta_bytes(t, space, sizeof space);
		program_name(t, draft->program[k]);
		if (k + 1 < n) ta_text(t, "\n");
	}
	uint8_t select[] = { 0xED, 0x07, 0xA0, (uint8_t)take[0], (uint8_t)take[1], (uint8_t)take[2], (uint8_t)skip };
	ta_bytes(t, select, sizeof select);
	ta_end(t);
	return i;
}

int ta_navi_gate(TextArchive *t, int flag, const char *navi, int beaten, int needed) {
	char s[240];
	if (beaten < needed) {
		/* (the telegraph first: which code, and how far along) */
		snprintf(s, sizeof s, "@M It's sealed with %s's code, Lan.|@M Deleting %s %s as a guardian, in any dive, would crack it. %s", navi, navi,
			needed == 2 ? "twice" : "again", beaten >= 1 ? "Once more!" : "We haven't yet.");
		return ta_say(t, FACE_MEGAMAN, s);
	}
	int quiet = ta_say(t, FACE_MEGAMAN, "The gate stands open, Lan. Nothing's left inside.");
	int no = closing(t);
	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)quiet, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	bool first = true;
	snprintf(s, sizeof s, "%s's code opens the gate! %s SP waits inside, the hardest fight on this layer.", navi, navi);
	ta_pages(t, s, FACE_MEGAMAN, &first);
	ask_in(t, FACE_MEGAMAN, "His SP chip is the prize.\nTake him on?\n", no, true, true);
	flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_gate_reward(TextArchive *t, const char *navi, int chip, const char *chip_name, int code) {
	int i = ta_script(t);
	bool first = true;
	char s[96];
	snprintf(s, sizeof s, "%s SP's data is ours, Lan!", navi);
	ta_page(t, FACE_MEGAMAN, s, true);
	first = false;
	give_chip(t, chip, code, 1);
	got_chip(t, chip_name, code, &first);
	ta_page(t, FACE_MEGAMAN, "It's in our pack. Let's put it in our folder, Lan!", false);
	ta_end(t);
	return i;
}

int ta_vault(TextArchive *t, int flag, int need, int have, const ScriptsVault *v) {
	char s[240];
	if (have < need) {
		/* (the telegraph first: the count it wants, and ours) */
		snprintf(s, sizeof s, "@M A vault, Lan, with a collector's lock. It opens for a Library of %d chips, and ours holds %d.|"
			"@M Every chip we hold, in any dive, goes in the Library!", need, have);
		return ta_say(t, FACE_MEGAMAN, s);
	}
	int empty = ta_say(t, FACE_MEGAMAN, "The vault stands open, Lan. We took our pick.");
	int leave = ta_say(t, FACE_MEGAMAN, "We'll leave them for now. The vault keeps.");
	int take[3];
	for (int k = 0; k < 3; ++k) {
		take[k] = ta_script(t);
		bool first = true;
		give_chip(t, v->chip[k], v->code[k], 1);
		got_chip(t, v->name[k], v->code[k], &first);
		ta_page(t, FACE_MEGAMAN, "It's in our pack. Let's put it in our folder, Lan!", false);
		flag_set(t, flag);
		ta_end(t);
	}
	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)empty, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	bool first = true;
	snprintf(s, sizeof s, "Our Library of %d opens the collector's lock!|Three rare chips inside, Lan. We can take one.", have);
	ta_pages(t, s, FACE_MEGAMAN, &first);
	ta_mugshot(t, FACE_MEGAMAN);
	ta_clear(t);
	/* three in a column, as the draft's; B leaves them */
	static const uint8_t opt[3][4] = { { 0xEB, 0x00, 0x00, 0x21 }, { 0xEB, 0x00, 0x11, 0x02 }, { 0xEB, 0x00, 0x22, 0x10 } };
	static const uint8_t space[] = { 0xEC, 0x00, 0x01 };
	for (int k = 0; k < 3; ++k) {
		ta_bytes(t, opt[k], 4);
		ta_bytes(t, space, sizeof space);
		snprintf(s, sizeof s, "%s %c%s", v->name[k], v->code[k] == 26 ? '*' : 'A' + v->code[k], k < 2 ? "\n" : "");
		ta_text(t, s);
	}
	uint8_t select[] = { 0xED, 0x07, 0xA0, (uint8_t)take[0], (uint8_t)take[1], (uint8_t)take[2], (uint8_t)leave };
	ta_bytes(t, select, sizeof select);
	ta_end(t);
	return i;
}

int ta_challenge_reward(TextArchive *t, int chip, const char *chip_name, int code) {
	int i = ta_script(t);
	bool first = true;
	ta_page(t, FACE_MEGAMAN, "The virus signal left a chip behind, Lan!", true);
	first = false;
	give_chip(t, chip, code, 1);
	got_chip(t, chip_name, code, &first);
	ta_page(t, FACE_MEGAMAN, "It's in our pack. Let's put it in our folder, Lan!", false);
	ta_end(t);
	return i;
}

/* ts_print_navi_cust_program5: the program's name, printed by the game */
/* A NaviCust program's name: the shop data's (and EF 1B's) program id
 * counts in fours (the game adds 0x90 to make an item id), its name
 * archive by one; a direct FA takes the name's index (bn6f chatbox_8041FB4:
 * a buffered value goes (value - 0x90) >> 2). The id itself read past the
 * 75 names for most programs and printed garbage into the gift's menu. */
static void program_name(TextArchive *t, int program) {
	uint8_t b[] = { 0xFA, 0x00, (uint8_t)(program >> 2), 0x05 };
	ta_bytes(t, b, sizeof b);
}

int ta_gift(TextArchive *t, int flag, bool comfort, bool brief, bool head_start, int chip, const char *chip_name, int power, int code, int program, int color,
            const char *about) {
	/* what each choice gives, then the flag that it was chosen */
	char line[64];
	int thanks = ta_say(t, FACE_PROG, "GOOD LUCK DOWN THERE, MEGAMAN! DIVE AS DEEP AS YOU CAN!");
	int hp = ta_script(t);
	bool first = true;
	give_hp_memory(t, 2);
	got_hp(t, 2, &first);
	flag_set(t, flag);
	ta_end(t);
	int chipped = ta_script(t);
	first = true;
	give_chip(t, chip, code, 1);
	got_chip(t, chip_name, code, &first);
	/* (a new chip goes to the pack, as in BN6: the folder is the player's) */
	ta_page(t, FACE_PROG, "IT'S IN YOUR PACK! PUT IT IN YOUR FOLDER FROM THE PET'S CHIPFOLDER!", false);
	flag_set(t, flag);
	ta_end(t);
	int programmed = ta_script(t);
	uint8_t give_program[] = { 0xEF, 0x1B, (uint8_t)program, 1, (uint8_t)color };   /* ts_item_give_navi_cust_program */
	ta_bytes(t, give_program, sizeof give_program);
	static const uint8_t no_face[] = { 0xF5, 0x01 };   /* (the options kept Mr. Prog's) */
	ta_bytes(t, no_face, sizeof no_face);
	ta_open(t);
	ta_text(t, "MegaMan got:\n\"");
	program_name(t, program);
	ta_text(t, "\"!!");
	ta_wait(t);
	ta_page(t, FACE_PROG, "INSTALL IT IN YOUR PET: MEGAMAN, THEN NAVICUST!", false);
	flag_set(t, flag);
	ta_end(t);

	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)thanks, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	first = true;
	/* (a returning player: one page, not three; a playtester called the
	 * chats of a restart after a long run a chore) */
	if (brief) ta_pages(t, "WELCOME BACK, MEGAMAN! PICK YOUR GIFT: HPMEMORY X2, A CHIP OR A PROGRAM!", FACE_PROG, &first);
	else ta_pages(t, "HELLO, MEGAMAN! I'M DR. HIKARI'S DIVE SUPPORT PROGRAM!|"
		"EVERY DIVE STARTS FROM SCRATCH, SO HERE'S A GIFT!|"
		"PICK ONE: HPMEMORY X2, A CHIP OR A NAVICUST PROGRAM!", FACE_PROG, &first);
	/* what the chip and the program do, before the choice */
	{
		char said[160];
		if (power > 0 && chip_def(chip)->kind == CK_RECOVER) snprintf(said, sizeof said, "THE CHIP IS %s: IT RESTORES %d HP!", chip_name, power);
		else if (power > 0) snprintf(said, sizeof said, "THE CHIP IS %s: IT HITS FOR %d!", chip_name, power);
		else snprintf(said, sizeof said, "THE CHIP IS %s!", chip_name);
		for (char *c = said; *c; ++c) if (*c >= 'a' && *c <= 'z') *c = (char)(*c - 'a' + 'A');
		ta_page(t, FACE_PROG, said, false);
		if (about) ta_page(t, FACE_PROG, about, false);
	}
	if (head_start) {
		/* the head-start helper (docs/META.md) */
		ta_page(t, FACE_PROG, "YOU ASKED FOR A HEAD START, SO TAKE THESE TOO!", false);
		give_hp_memory(t, 2);
		got_hp(t, 2, &first);
	}
	if (comfort) {
		/* the last dive ended early: a little more help */
		ta_page(t, FACE_PROG, "YOUR LAST DIVE ENDED EARLY, SO TAKE THIS TOO!", false);
		give_hp_memory(t, 1);
		got_hp(t, 1, &first);
	}
	ta_mugshot(t, FACE_PROG);
	ta_clear(t);
	/* half a second before the options (an A pressed through the last page
	 * chose the first of them, unread) */
	static const uint8_t pause[] = { 0xEE, 0x00, 30, 0 };   /* ts_wait */
	ta_bytes(t, pause, sizeof pause);
	/* three options in a column (ts_option: left/right stay, up/down move) */
	static const uint8_t opt[3][4] = { { 0xEB, 0x00, 0x00, 0x21 }, { 0xEB, 0x00, 0x11, 0x02 }, { 0xEB, 0x00, 0x22, 0x10 } };
	static const uint8_t space[] = { 0xEC, 0x00, 0x01 };
	ta_bytes(t, opt[0], 4);
	ta_bytes(t, space, sizeof space);
	ta_text(t, "HPMemory x2\n");
	ta_bytes(t, opt[1], 4);
	ta_bytes(t, space, sizeof space);
	snprintf(line, sizeof line, "%s %c\n", chip_name, code == 26 ? '*' : 'A' + code);
	ta_text(t, line);
	ta_bytes(t, opt[2], 4);
	ta_bytes(t, space, sizeof space);
	program_name(t, program);
	/* ts_select: clear, B does nothing, a script per option */
	uint8_t select[] = { 0xED, 0x07, 0xC0, (uint8_t)hp, (uint8_t)chipped, (uint8_t)programmed, 0xFF };
	ta_bytes(t, select, sizeof select);
	ta_end(t);
	return i;
}
