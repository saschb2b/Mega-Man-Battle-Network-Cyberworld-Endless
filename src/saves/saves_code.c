/* saves_code.h. The link does the sending (src/online/transfer.c); SAVES
 * keeps what the player sees: the code, the digits, the comparison after.
 * Nothing here changes the saves: a code's saves go through the
 * comparison like a file's, USE THE FILE'S and UNDO IMPORT as ever. */
#include "saves_code.h"

#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "backup.h"
#include "rom.h"
#include "saves_cloud.h"
#include "saves_state.h"
#include "text_input.h"

/* (the native keyboard where one is likely at hand: the desktops' and the
 * browser's; a phone's covers the keys on the screen, which it taps) */
#if defined(CW_DESKTOP) || defined(__EMSCRIPTEN__)
#define KEYBOARD 1
#endif

bool saves_code_here(void) { return transfer_link() != NULL; }

/* The link's state now (the browser's moved on by it: the update's call,
 * never a draw's, which reads SV.transfer) */
static void saves_code_state(TransferState *out) {
	const TransferLink *link = transfer_link();
	if (link) link->state(out);
	else memset(out, 0, sizeof *out);
}

/* (a code SEND showed still held for these saves: its time not up) */
static bool code_holds(void) {
	TransferState s;
	saves_code_state(&s);
	return s.phase == TRANSFER_SENT && (int32_t)(s.until - SDL_GetTicks()) > 0 && SV.sent_hash == SV.here.hash;
}

static void saves_code_send(void) {
	const TransferLink *link = transfer_link();
	if (!link || !SV.have) return;
	SV.view = SAVES_SEND_CODE;
	SV.focus = SAVES_DONE;
	if (SV.code_link) transfer_link()->cancel();   /* (a link's code shown last: these saves' own now) */
	SV.code_link = false;
	if (code_holds()) { saves_code_state(&SV.transfer); return; }
	size_t n = 0;
	uint8_t *bytes = backup_pack(g_data_dir, 0, &n);
	SV.sent_hash = SV.here.hash;
	link->send(bytes, n);
	saves_code_state(&SV.transfer);
}

static void typing(bool on) {
#ifdef KEYBOARD
	if (on && !SV.typing) text_input_begin(SV.code, sizeof SV.code);
	if (!on && SV.typing) text_input_end();
	SV.typing = on;
#else
	(void)on;
#endif
}

static void saves_code_receive(void) {
	if (!transfer_link()) return;
	SV.view = SAVES_RECEIVE_CODE;
	SV.linking = false;
	SV.focus = SAVES_DIGIT + 1;
	SV.code[0] = 0;
	transfer_link()->cancel();
	saves_code_state(&SV.transfer);
	typing(true);
}

/* (RECEIVE's digits held while their code is asked for) */
static bool asking(void) {
	TransferState s;
	saves_code_state(&s);
	return s.phase == TRANSFER_WORKING;
}

static void saves_code_digit(int digit) {
	if (asking()) return;
	size_t n = strlen(SV.code);
	if (n >= sizeof SV.code - 1 || digit < 0 || digit > 9) return;
	SV.code[n] = (char)('0' + digit);
	SV.code[n + 1] = 0;
	/* (the tenth digit in: the cursor on OK) */
	if (n + 1 == sizeof SV.code - 1) SV.focus = SAVES_GET;
}

static void saves_code_erase(void) {
	if (asking()) return;
	size_t n = strlen(SV.code);
	if (n) SV.code[n - 1] = 0;
}

static void saves_code_get(void) {
	const TransferLink *link = transfer_link();
	if (link && strlen(SV.code) == sizeof SV.code - 1) link->receive(SV.code);
}

bool saves_code_press(int action) {
	if (action >= SAVES_DIGIT && action < SAVES_DIGIT + 10) saves_code_digit(action - SAVES_DIGIT);
	else if (action == SAVES_SEND) saves_code_send();
	else if (action == SAVES_RECEIVE) saves_code_receive();
	else if (action == SAVES_ERASE) saves_code_erase();
	else if (action == SAVES_GET) saves_code_get();
	else return false;
	return true;
}

/* (what the keyboard typed: its digits kept, the rest dropped; true for
 * Escape) */
static bool keyboard(void) {
	size_t k = 0;
	for (const char *p = SV.code; *p; ++p)
		if (*p >= '0' && *p <= '9') SV.code[k++] = *p;
	SV.code[k] = 0;
	int typed = text_input_result();
	if (typed == TEXT_INPUT_DONE) saves_code_get();
	return typed == TEXT_INPUT_CANCEL;
}

bool saves_code_tick(void) {
	if (SV.view != SAVES_SEND_CODE && SV.view != SAVES_RECEIVE_CODE) return false;
	/* (the browser's link moves on here, in the update, never in a draw) */
	saves_code_state(&SV.transfer);
	if (SV.view != SAVES_RECEIVE_CODE) return false;
	if (SV.typing && keyboard()) return true;
	if (SV.transfer.phase != TRANSFER_RECEIVED) return false;
	size_t n = 0;
	uint8_t *bytes = transfer_link()->take(&n);
	typing(false);
	SV.code[0] = 0;
	/* (a code's bytes are saves, or another system's cloud link) */
	if (bytes && saves_cloud_join(bytes, n)) free(bytes);
	else {
		saves_compare(bytes, n, false);
		SV.from_code = SV.bytes != NULL;
	}
	return false;
}

void saves_code_send_link(uint8_t *bytes, size_t n) {
	const TransferLink *link = transfer_link();
	if (!link || !bytes) {
		free(bytes);
		return;
	}
	SV.view = SAVES_SEND_CODE;
	SV.focus = SAVES_DONE;
	SV.sent_hash = 0;
	SV.code_link = true;
	link->cancel();
	link->send(bytes, n);
	saves_code_state(&SV.transfer);
}

void saves_code_leave(void) {
	typing(false);
	const TransferLink *link = transfer_link();
	TransferState s;
	saves_code_state(&s);
	if (link && s.phase == TRANSFER_WORKING) link->cancel();
}
