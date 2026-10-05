/* Text scripts of the layer objects' chats: the parts every chat is built
 * of (a question, a choice, a flag, a jump, a chip given and its "got"),
 * on the text archives of text.c, and the speaker marks the chat box draws.
 * The chats themselves: service_words.c (the services), reward_words.c (the
 * rewards, gates and gifts), lock_words.c (the set pieces' locks). */
#include "scripts.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "rom.h"
#include "script_kit.h"

/* Scripts for the layers' services and choices, on the game's own text
 * commands (bn6f text_script_commands.inc). Who speaks each box is its
 * face: Dr. Hikari's Mr. Progs (in capitals, as the game's own talk),
 * the Net Dealers, MegaMan asking Lan; what MegaMan gets is told the
 * game's way, without a face. */

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
void ta_ask_in(TextArchive *t, int face, const char *question, int no, bool after, bool risky) {
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

void ta_ask(TextArchive *t, int face, const char *question, int no) { ta_ask_in(t, face, question, no, false, false); }

void ta_flag_set(TextArchive *t, int flag) {
	uint8_t b[] = { 0xEA, 0x00, (uint8_t)flag, (uint8_t)(flag >> 8) };
	ta_bytes(t, b, sizeof b);
}

int ta_say_flag(TextArchive *t, int face, const char *s, int flag) {
	int i = ta_script(t);
	ta_flag_set(t, flag);
	bool first = true;
	ta_pages(t, s, face, &first);
	ta_end(t);
	return i;
}

/* closes the box: the answer No */
int ta_closing(TextArchive *t) {
	int i = ta_script(t);
	ta_end(t);
	return i;
}

const uint8_t ta_full_hp[] = { 0xFC, 0x03, 0x13, 0x00 };  /* ts_call_set_full_h_p */

/* ts_item_give_chip: `count` of chip `id` with code index `code` (A=0, *=26) */
void ta_give_chip(TextArchive *t, int id, int code, int count) {
	uint8_t b[] = { 0xF4, 0x10, (uint8_t)id, (uint8_t)(id >> 8), (uint8_t)code, (uint8_t)count };
	ta_bytes(t, b, sizeof b);
}

/* ts_item_give HPMemory, with its jingle */
void ta_give_hp_memory(TextArchive *t, int count) {
	uint8_t b[] = { 0xF4, 0x00, SCRIPTS_HP_MEMORY, (uint8_t)count };
	ta_bytes(t, b, sizeof b);
}

/* What MegaMan got, as the game says it: `MegaMan got:` then the thing in
 * quotes. */
void ta_got(TextArchive *t, const char *thing, bool *first) {
	char line[64];
	snprintf(line, sizeof line, "MegaMan got:\n\"%s\"!!", thing);
	ta_page(t, FACE_NONE, line, *first);
	*first = false;
}

void ta_got_hp(TextArchive *t, int count, bool *first) {
	char thing[32];
	if (count > 1) snprintf(thing, sizeof thing, "%d HPMemory", count);
	else snprintf(thing, sizeof thing, "HPMemory");
	ta_got(t, thing, first);
}

void ta_got_chip(TextArchive *t, const char *chip, int code, bool *first) {
	char thing[40];
	snprintf(thing, sizeof thing, "%s %c", chip, code == 26 ? '*' : 'A' + code);
	ta_got(t, thing, first);
}

/* ts_jump: on in script `to` (the way on, after a branch of the draft) */
void ta_jump(TextArchive *t, int to) {
	uint8_t b[] = { 0xF0, 0x00, (uint8_t)to };
	ta_bytes(t, b, sizeof b);
}

/* BN6's choice (ts_select) among the `n` options just written in a
 * column: the box cleared, a script for each option, and B its own
 * (0xA0), which reads none of them but goes on past the choice (bn6f
 * chatbox_ED_select): here to `b`, the script B means. (Once written with
 * B's script among the options', its press ran past the choice to the
 * talk's end: a Guardian Data's B gave no BugFrags nor marked its reward
 * taken, and it gave its HPMemory and chip again at every talk, issue 16) */
void ta_choose(TextArchive *t, const int *scripts, int n, int b) {
	uint8_t select[6] = { 0xED, (uint8_t)(3 + n), 0xA0 };
	for (int k = 0; k < n; ++k) select[3 + k] = (uint8_t)scripts[k];
	ta_bytes(t, select, 3 + n);
	ta_jump(t, b);
}

/* A script's end: the way on's question next, where there is one (`next`
 * its script), else the chat's end. */
void ta_end_or(TextArchive *t, int next) {
	if (next >= 0) ta_jump(t, next);
	else ta_end(t);
}

/* A chip's description as a sentence after its name: BN6's own, with a
 * period where it ends without one ("A piercng thunder attack!" had gone
 * on into "!.", session 64); "" where it is blank (a DarkChip's is, in
 * Gregar). Its spaces are the wrap's to tidy. */
void chip_desc_line(char *out, size_t n, const char *name, int code, const char *desc) {
	size_t len = strlen(desc);
	while (len && desc[len - 1] == ' ') --len;
	if (!len) { if (n) out[0] = 0; return; }
	bool ends = strchr(".!?", desc[len - 1]) != NULL;
	snprintf(out, n, "%s %c: %.*s%s", name, code == 26 ? '*' : 'A' + code, (int)len, desc, ends ? "" : ".");
}

/* ts_print_navi_cust_program5: the program's name, printed by the game */
/* A NaviCust program's name: the shop data's (and EF 1B's) program id
 * counts in fours (the game adds 0x90 to make an item id), its name
 * archive by one; a direct FA takes the name's index (bn6f chatbox_8041FB4:
 * a buffered value goes (value - 0x90) >> 2). The id itself read past the
 * 75 names for most programs and printed garbage into the gift's menu. */
void ta_program_name(TextArchive *t, int program) {
	uint8_t b[] = { 0xFA, 0x00, (uint8_t)(program >> 2), 0x05 };
	ta_bytes(t, b, sizeof b);
}

void ta_flag_clear(TextArchive *t, int flag) {
	uint8_t b[] = { 0xEA, 0x01, (uint8_t)flag, (uint8_t)(flag >> 8) };
	ta_bytes(t, b, sizeof b);
}
