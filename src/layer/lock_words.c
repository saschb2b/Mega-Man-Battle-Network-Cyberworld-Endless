/* The set pieces' locks' chats (docs/VOICE.md): a security cube's P-Code
 * or toll, a skull door, a number door, and the navi who tells the
 * P-Code. */

#include "lock_words.h"

#include <stdio.h>

#include "script_kit.h"

/* The cube's present flag cleared and half a second for it to go, the box
 * still up: the words after it say what just happened, and one A closes
 * them (the flag cleared after the last page left the box up for another
 * A, the cube gone under it). */
static void cube_opens(TextArchive *t, int present) {
	static const uint8_t pause[] = { 0xEE, 0x00, 30, 0 };   /* ts_wait */
	ta_flag_clear(t, present);
	ta_bytes(t, pause, sizeof pause);
}

int ta_cube_pcode(TextArchive *t, int present, int told, const char *code) {
	char s[160];
	bool first = true;
	int open = ta_script(t);
	snprintf(s, sizeof s, "MegaMan entered the P-Code: %s.", code);
	ta_page(t, FACE_NONE, s, true);
	cube_opens(t, present);
	ta_page(t, FACE_NONE, "The cube opens!", false);
	ta_end(t);
	int i = ta_script(t);
	uint8_t check[] = { 0xEF, 0x00, (uint8_t)told, (uint8_t)(told >> 8), (uint8_t)open, 0xFF };  /* ts_check_flag */
	ta_bytes(t, check, sizeof check);
	first = true;
	ta_pages(t, "A security cube,Lan.|It wants a P-Code...|Someone here must know it.|Let's ask around!", FACE_MEGAMAN, &first);
	ta_end(t);
	return i;
}

int ta_cube_toll(TextArchive *t, int present, int price) {
	int paid = ta_script(t);
	cube_opens(t, present);
	ta_page(t, FACE_NONE, "Paid. The cube opens!", false);
	ta_end(t);
	bool first = true;
	int broke = ta_script(t);
	ta_pages(t, "We don't have enough Zenny,Lan...", FACE_MEGAMAN, &first);
	ta_end(t);
	int no = ta_closing(t);
	int i = ta_script(t);
	char q[64];
	/* (a line at most 22 characters: TOLL: 200 ZENNY TO PASS. overran the
	 * box, its first letters drawn over) */
	snprintf(q, sizeof q, "Toll: %d Zenny.\nPay to pass?\n", price);
	ta_ask(t, FACE_NONE, q, no);
	uint8_t take[] = { 0xEF, 0x0F, (uint8_t)price, (uint8_t)(price >> 8), (uint8_t)(price >> 16), (uint8_t)(price >> 24),
		(uint8_t)paid, (uint8_t)broke, (uint8_t)broke };   /* ts_check_take_zenny */
	ta_bytes(t, take, sizeof take);
	ta_end(t);
	return i;
}

int ta_cube_skull(TextArchive *t, int present, int id) {
	int open = ta_script(t);
	ta_page(t, FACE_NONE, "MegaMan showed the WWW-ID.", true);
	cube_opens(t, present);
	ta_page(t, FACE_NONE, "The skull door opens!", false);
	ta_end(t);
	bool first = true;
	int shut = ta_script(t);
	ta_pages(t, "A skull door,Lan...|Only WWW members get through.|Without a WWW-ID,we're stuck!", FACE_MEGAMAN, &first);
	ta_end(t);
	int i = ta_script(t);
	uint8_t has[] = { 0xEF, 0x07, (uint8_t)id, 1, (uint8_t)open, (uint8_t)open, (uint8_t)shut };   /* ts_check_item07 */
	ta_bytes(t, has, sizeof has);
	ta_end(t);
	return i;
}

/* (the number door's three answers in a column: three numbers in a row,
 * the right one first, second or third as `seed` has it) */
static void number_options(TextArchive *t, int answer, unsigned seed, int right, int wrong, int no) {
	static const uint8_t opt[3][4] = { { 0xEB, 0x00, 0x00, 0x21 }, { 0xEB, 0x00, 0x11, 0x02 }, { 0xEB, 0x00, 0x22, 0x10 } };
	static const uint8_t space[] = { 0xEC, 0x00, 0x01 };
	int low = answer - (int)(seed % 3);
	if (low < 1) low = 1;
	char line[16];
	for (int k = 0; k < 3; ++k) {
		ta_bytes(t, opt[k], 4);
		ta_bytes(t, space, sizeof space);
		snprintf(line, sizeof line, k < 2 ? "%d\n" : "%d", low + k);   /* (a line break after the last asked for a fourth line) */
		ta_text(t, line);
	}
	uint8_t select[] = { 0xED, 0x07, 0x80, (uint8_t)(low == answer ? right : wrong), (uint8_t)(low + 1 == answer ? right : wrong),
		(uint8_t)(low + 2 == answer ? right : wrong), (uint8_t)no };   /* (ts_select: clear, a script per option, B) */
	ta_bytes(t, select, sizeof select);
}

int ta_cube_number(TextArchive *t, int present, int sealed, int answer, unsigned seed) {
	bool first = true;
	int right = ta_script(t);
	ta_page(t, FACE_NONE, "The number is right.", true);
	cube_opens(t, present);
	ta_page(t, FACE_NONE, "The door opens!", false);
	ta_end(t);
	int wrong = ta_script(t);
	ta_flag_set(t, sealed);
	ta_pages(t, "ACCESS DENIED.|The door seals itself!", FACE_NONE, &first);
	ta_end(t);
	first = true;
	int closed = ta_script(t);
	ta_pages(t, "The door sealed itself,Lan...|We got the number wrong.", FACE_MEGAMAN, &first);
	ta_end(t);
	int no = ta_closing(t);
	int i = ta_script(t);
	uint8_t check[] = { 0xEF, 0x00, (uint8_t)sealed, (uint8_t)(sealed >> 8), (uint8_t)closed, 0xFF };   /* ts_check_flag */
	ta_bytes(t, check, sizeof check);
	first = true;
	ta_pages(t, "NUMBER DOOR.|How many flames of hatred burn on this layer?|"
		"@M The braziers' flames,Lan!|@M Let's count carefully. A wrong number seals it!", FACE_NONE, &first);
	static const uint8_t hide[] = { 0xF5, 0x01 };   /* (no face over the answers) */
	ta_bytes(t, hide, sizeof hide);
	ta_clear(t);
	number_options(t, answer, seed, right, wrong, no);
	ta_end(t);
	return i;
}

int ta_pcode_teller(TextArchive *t, int face, const char *code, int told) {
	char s[160];
	bool first = true;
	int i = ta_script(t);
	snprintf(s, sizeof s, "Psst! MegaMan!|That security cube here?|Its P-Code is %s.|Don't tell anyone I told you!", code);
	ta_pages(t, s, face, &first);
	ta_flag_set(t, told);
	ta_end(t);
	return i;
}

/* MegaMan at a Link Navi's obstacle (issue #42), `what`: the two whose
 * Cross could clear it, which the run lacks (the hint for the next run's
 * Cross, said as it is) */
const char *obstacle_stuck_words(const char *what, const char *a, const char *b) {
	static char s[240];
	snprintf(s, sizeof s, "%s blocks the way,Lan.|%s's or %s's Cross could clear it...|But we don't have either.", what, a, b);
	return s;
}

/* ... and with `cross`'s Cross data, who is asked to `deed` it */
const char *obstacle_cross_words(const char *what, const char *cross, const char *deed) {
	static char s[240];
	snprintf(s, sizeof s, "%s blocks the way!|We've got %s's Cross data!|Let's ask him to %s it!", what, cross, deed);
	return s;
}

/* ... and his answer */
const char *obstacle_answer_words(void) { return "Leave it to me!"; }
