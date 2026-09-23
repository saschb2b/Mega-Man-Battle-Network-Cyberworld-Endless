/* Scripts for the layers' services and choices, on the game's own text
 * commands (bn6f text_script_commands.inc). */
#include "scripts.h"

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

int ta_chip_trader(TextArchive *t) {
	int refuse = ta_say(t, -1, "You need 3 chips\nto trade.");
	int i = ta_script(t);
	uint8_t trade[] = { 0xFB, 0x06, 0x03, (uint8_t)refuse };      /* ts_start_chip_trader 3 */
	ta_open(t);
	ta_text(t, "It's a Chip Trader.\n3 chips for 1!");
	ta_wait(t);
	ta_bytes(t, trade, sizeof trade);
	ta_end(t);
	return i;
}

int ta_bug_trader(TextArchive *t) {
	int refuse = ta_say(t, -1, "Come back with\nmore BugFrags!");
	int i = ta_script(t);
	uint8_t trade[] = { 0xFB, 0x06, 0x01, (uint8_t)refuse };      /* ts_start_bug_frag_trader */
	ta_open(t);
	ta_text(t, "BugFrags for chips.\nInterested?");
	ta_wait(t);
	ta_bytes(t, trade, sizeof trade);
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
