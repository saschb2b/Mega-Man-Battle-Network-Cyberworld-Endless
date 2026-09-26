/* Conversations the director starts: Lan and MegaMan over the PET, Dad's
 * and Chaud's calls. Each is built when it is due into one slot of the
 * free ROM space and run by the game's chat box; the player's keys only
 * page it until it closes. */
#include "talk.h"

#include "bn6.h"
#include "cinema.h"
#include "emu.h"
#include "gamecall.h"
#include "text.h"

#define TALK_AT   (EMU_FREE + 0x150000)   /* one archive, rewritten for each conversation */
#define TALK_SIZE 0x2000

static struct {
	bool running, seen;
	int t;
} K;

static void begin(uint32_t archive, int script) {
	game_call(BN6_CHAT_RUN_SCRIPT, archive, (uint32_t)script);
	cinema_input(CINEMA_TALK);
	K.running = true;
	K.seen = false;
	K.t = 0;
}

bool talk_start(const char *boxes, int face) {
	if (K.running || emu_read8(BN6_CHATBOX)) return false;
	static TextArchive a;
	static uint8_t bytes[TEXT_ARCHIVE_MAX];
	ta_begin(&a);
	ta_talk(&a, boxes, face);
	int n = ta_build(&a, bytes);
	if (n > TALK_SIZE) return false;
	emu_write(TALK_AT, bytes, (size_t)n);
	begin(TALK_AT, 0);
	return true;
}

bool talk_script(uint32_t archive, int script) {
	if (K.running || emu_read8(BN6_CHATBOX) || !archive || script < 0) return false;
	begin(archive, script);
	return true;
}

void talk_update(void) {
	if (!K.running) return;
	bool open = emu_read8(BN6_CHATBOX) != 0;
	if (open) K.seen = true;
	/* read to its end (or never shown: a minute on, the keys come back) */
	if ((K.seen && !open) || ++K.t > 60 * 60) {
		K.running = false;
		/* (unless a guardian's staging has taken the keys meanwhile) */
		if (cinema_input_mode() == CINEMA_TALK) cinema_input(CINEMA_FREE);
	}
}

bool talk_busy(void) { return K.running; }

void talk_reset(void) {
	if (K.running && cinema_input_mode() == CINEMA_TALK) cinema_input(CINEMA_FREE);
	K.running = false;
}
