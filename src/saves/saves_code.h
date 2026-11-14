/* SAVES' saves sent by a code (docs/engine/saves.md, Sending by code):
 * SEND puts this device's saves on the server under a new code and shows
 * it; RECEIVE takes a code's saves and hands them to the comparison, as
 * IMPORT hands a file's. Through transfer_link.h, none of it where the
 * system has no link. */
#ifndef CW_SAVES_CODE_H
#define CW_SAVES_CODE_H

#include <stdbool.h>

#include "transfer_link.h"

/* Whether this system can send and receive */
bool saves_code_here(void);
/* A button of theirs chosen: home's SEND (the saves sent, or where these
 * very saves' code still holds, that code shown again) and RECEIVE (the
 * code's digits asked for), RECEIVE's digits, ERASE and OK; false for
 * any other button */
bool saves_code_press(int action);
/* Each frame of SAVES: the link's state taken into SV.transfer (a code's
 * saves come to the comparison), the keyboard's digits; true where the
 * keyboard's Escape asks to go back */
bool saves_code_tick(void);
/* SEND's or RECEIVE's view left: what is under way given up (a code shown
 * stays with the server till its time is up) */
void saves_code_leave(void);

#endif
