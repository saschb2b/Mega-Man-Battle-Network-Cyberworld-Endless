/* Scripts for the layers' services and choices, on the game's own text
 * commands (bn6f text_script_commands.inc). */
#include "scripts.h"

#include <stdio.h>

/* A Yes/No choice after `question`, as the game's shopkeepers ask it:
 * two options, then select (Yes continues, No and B jump to `no`). */
static void ask(TextArchive *t, const char *question, int no) {
	static const uint8_t horizontal[] = { 0xF7, 0x07, 0x0B };        /* ts_position_option_horizontal */
	static const uint8_t yes_opt[] = { 0xEB, 0x00, 0x11, 0x00 };     /* ts_option: left/right to No */
	static const uint8_t no_opt[] = { 0xEB, 0x00, 0x00, 0x11 };
	static const uint8_t space[] = { 0xEC, 0x00, 0x01 };
	uint8_t select[] = { 0xED, 0x06, 0x00, 0xFF, (uint8_t)no, 0xFF };
	ta_open(t);
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

static void flag_set(TextArchive *t, int flag) {
	uint8_t b[] = { 0xEA, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8) };
	ta_bytes(t, b, sizeof b);
}

/* closes the box: the answer No */
static int closing(TextArchive *t) {
	int i = ta_script(t);
	ta_end(t);
	return i;
}

int ta_heal(TextArchive *t) {
	int i = ta_script(t);
	static const uint8_t full_hp[] = { 0xFC, 0x03, 0x13, 0x00 };  /* ts_call_set_full_h_p */
	ta_open(t);
	ta_text(t, "Recovery program\nrunning...");
	ta_bytes(t, full_hp, sizeof full_hp);
	ta_wait(t);
	ta_clear(t);
	ta_text(t, "MegaMan's HP\nis full!");
	ta_wait(t);
	ta_end(t);
	return i;
}

int ta_shop(TextArchive *t, int shop, const char *greeting) {
	int i = ta_script(t);
	uint8_t open[] = { 0xFB, 0x05, (uint8_t)shop };               /* ts_start_shop */
	ta_open(t);
	ta_text(t, greeting);
	ta_wait(t);
	ta_bytes(t, open, sizeof open);
	ta_end(t);
	return i;
}

int ta_challenge(TextArchive *t, int flag) {
	int quiet = ta_say(t, -1, "The signal has\ngone quiet.");
	int no = closing(t);
	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)quiet, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	ask(t, "A strong virus\nsignal. Fight?\n", no);
	flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_undernet(TextArchive *t, int flag) {
	int no = closing(t);
	int i = ta_script(t);
	ask(t, "A dark warp hums.\nEnter Undernet?\n", no);
	flag_set(t, flag);
	ta_end(t);
	return i;
}

int ta_secret_gate(TextArchive *t, int flag) {
	int sealed = ta_say(t, -1, "A sealed gate.\nIt wants three\nScrtData.");
	int no = closing(t);
	int i = ta_script(t);
	uint8_t check[] = { 0xEF, 0x07, SCRIPTS_SECRET_DATA, 3, 0xFF, 0xFF, (uint8_t)sealed };  /* ts_check_item07 */
	ta_bytes(t, check, sizeof check);
	ask(t, "ScrtData glows.\nOpen the gate?\n", no);
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

static void chip_text(char *out, size_t n, const char *lead, const char *chip, int code) {
	snprintf(out, n, "%s\n%s %c!", lead, chip, code == 26 ? '*' : 'A' + code);
}

int ta_guardian_reward(TextArchive *t, const char *name, const char *power, int chip, const char *chip_name, int code,
                       int taken_flag) {
	int i = ta_script(t);
	static const uint8_t full_hp[] = { 0xFC, 0x03, 0x13, 0x00 };  /* ts_call_set_full_h_p */
	char head[64], line[64];
	snprintf(head, sizeof head, "%s's\nGuardian Data!", name);
	ta_open(t);
	ta_text(t, head);
	ta_wait(t);
	ta_clear(t);
	if (power) {
		ta_text(t, power);
		ta_wait(t);
		ta_clear(t);
	}
	give_hp_memory(t, SCRIPTS_BOSS_HP_MEMORIES);
	snprintf(line, sizeof line, "MegaMan got\n%d HPMemory!", SCRIPTS_BOSS_HP_MEMORIES);
	ta_text(t, line);
	ta_wait(t);
	ta_clear(t);
	if (chip > 0) {
		/* the navi's own chip, as Battle Network gives it */
		give_chip(t, chip, code, 1);
		chip_text(line, sizeof line, "MegaMan got", chip_name, code);
		ta_text(t, line);
		ta_wait(t);
		ta_clear(t);
	}
	ta_bytes(t, full_hp, sizeof full_hp);
	ta_text(t, "MegaMan's HP\nis restored!");
	ta_wait(t);
	flag_set(t, taken_flag);
	ta_end(t);
	return i;
}

int ta_challenge_reward(TextArchive *t, int chip, const char *chip_name, int code) {
	int i = ta_script(t);
	char line[64];
	ta_open(t);
	ta_text(t, "The signal's core\nis left behind!");
	ta_wait(t);
	ta_clear(t);
	give_chip(t, chip, code, 1);
	chip_text(line, sizeof line, "MegaMan got", chip_name, code);
	ta_text(t, line);
	ta_wait(t);
	ta_end(t);
	return i;
}

int ta_gift(TextArchive *t, int flag, bool comfort, int chip, const char *chip_name, int code, int program, int color) {
	/* what each choice gives, then the flag that it was chosen */
	char line[64];
	int thanks = ta_say(t, -1, "Good luck out\nthere, MegaMan!");
	int hp = ta_script(t);
	ta_open(t);
	give_hp_memory(t, 2);
	ta_text(t, "MegaMan got\n2 HPMemory!");
	ta_wait(t);
	flag_set(t, flag);
	ta_end(t);
	int chipped = ta_script(t);
	ta_open(t);
	give_chip(t, chip, code, 1);
	chip_text(line, sizeof line, "MegaMan got", chip_name, code);
	ta_text(t, line);
	ta_wait(t);
	flag_set(t, flag);
	ta_end(t);
	int programmed = ta_script(t);
	ta_open(t);
	uint8_t give_program[] = { 0xEF, 0x1B, (uint8_t)program, 1, (uint8_t)color };   /* ts_item_give_navi_cust_program */
	ta_bytes(t, give_program, sizeof give_program);
	ta_text(t, "MegaMan got a\nNaviCust program!");
	ta_wait(t);
	flag_set(t, flag);
	ta_end(t);

	int i = ta_script(t);
	uint8_t done[] = { 0xEF, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8), (uint8_t)thanks, 0xFF };  /* ts_check_flag */
	ta_bytes(t, done, sizeof done);
	ta_open(t);
	if (comfort) {
		/* the last run ended early: a little more help */
		ta_text(t, "A rough last run?\nTake this too.");
		ta_wait(t);
		ta_clear(t);
		give_hp_memory(t, 1);
		ta_text(t, "MegaMan got\n1 HPMemory!");
		ta_wait(t);
		ta_clear(t);
	}
	ta_text(t, "Before you go,\nMegaMan: pick one\ngift for the road.");
	ta_wait(t);
	ta_clear(t);
	/* three options in a column (ts_option: left/right stay, up/down move) */
	static const uint8_t opt[3][4] = { { 0xEB, 0x00, 0x00, 0x21 }, { 0xEB, 0x00, 0x11, 0x02 }, { 0xEB, 0x00, 0x22, 0x10 } };
	static const uint8_t space[] = { 0xEC, 0x00, 0x01 };
	ta_bytes(t, opt[0], 4);
	ta_bytes(t, space, sizeof space);
	ta_text(t, "2 HPMemory\n");
	ta_bytes(t, opt[1], 4);
	ta_bytes(t, space, sizeof space);
	snprintf(line, sizeof line, "%s %c\n", chip_name, code == 26 ? '*' : 'A' + code);
	ta_text(t, line);
	ta_bytes(t, opt[2], 4);
	ta_bytes(t, space, sizeof space);
	ta_text(t, "NaviCust program");
	/* ts_select: clear, B does nothing, a script per option */
	uint8_t select[] = { 0xED, 0x07, 0xC0, (uint8_t)hp, (uint8_t)chipped, (uint8_t)programmed, 0xFF };
	ta_bytes(t, select, sizeof select);
	ta_end(t);
	return i;
}
