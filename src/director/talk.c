/* Conversations the director starts: Lan and MegaMan over the PET, Dad's
 * and Chaud's calls. Each is built when it is due into one slot of the
 * free ROM space and run by the game's chat box; the player's keys only
 * page it until it closes. */
#include "talk.h"

#include <stdio.h>

#include "bn6.h"
#include "cinema.h"
#include "debug.h"
#include "emu.h"
#include "flags.h"
#include "gamecall.h"
#include "text.h"

#define TALK_AT   (EMU_FREE + 0x150000)   /* one archive, rewritten for each conversation */
#define TALK_SIZE 0x2000

static struct {
	bool running, seen;
	int t;
} K;

/* The player held for the talk as the game holds him for a dialogue no NPC
 * starts (bn6f owPlayer_lockPlayerForNonNPCDialogue_809E0B0 and its unlock):
 * else the A that pages the text also talks to an NPC he faces, whose chat
 * takes over the box and leaves the NPC unable to talk again. */
static void hold(bool on) {
	emu_write8(BN6_DIALOGUE_LOCK, on ? 1 : 0);
	if (on) { flag_set(BN6_FLAG_DIALOGUE_1718); flag_clear(BN6_FLAG_PLAYER_CAN_MOVE); }
	else { flag_set(BN6_FLAG_PLAYER_CAN_MOVE); flag_clear(BN6_FLAG_DIALOGUE_1719); }
}

static void begin(uint32_t archive, int script) {
	hold(true);
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
	if (emu_debug_on()) fprintf(stderr, "talk: %d bytes%s: %s\n", n, a.full ? ", FULL" : "", boxes);
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
		hold(false);
		/* (unless a guardian's staging has taken the keys meanwhile) */
		if (cinema_input_mode() == CINEMA_TALK) cinema_input(CINEMA_FREE);
	}
}

bool talk_busy(void) { return K.running; }

void talk_reset(void) {
	if (K.running && cinema_input_mode() == CINEMA_TALK) cinema_input(CINEMA_FREE);
	if (K.running) hold(false);
	K.running = false;
}
