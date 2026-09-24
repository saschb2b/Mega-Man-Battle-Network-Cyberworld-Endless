/* Text archives in the game's text script language (bn6f
 * text_script_commands.inc), written into the free ROM space. */
#ifndef CW_TEXT_H
#define CW_TEXT_H

#include <stdint.h>

#define TEXT_MAX_SCRIPTS 32
#define TEXT_MAX_BYTES 4096

typedef struct {
	uint8_t buf[TEXT_MAX_BYTES];
	uint16_t off[TEXT_MAX_SCRIPTS];
	int len, n;
} TextArchive;

void ta_begin(TextArchive *t);
/* Starts the next script; returns its index. */
int ta_script(TextArchive *t);
void ta_bytes(TextArchive *t, const uint8_t *b, int n);
/* ASCII text; '\n' breaks the line. */
void ta_text(TextArchive *t, const char *s);
void ta_open(TextArchive *t);            /* E8 00: open the chat box */
void ta_wait(TextArchive *t);            /* E7 00: wait for A */
void ta_clear(TextArchive *t);           /* F2 */
void ta_end(TextArchive *t);             /* E6 */
void ta_mugshot(TextArchive *t, int m);  /* F5 00 m */
/* A whole message: open, text, wait, end. */
int ta_say(TextArchive *t, int mugshot, const char *s);
/* A conversation: chat boxes split by '|', each shown with the next of
 * `mugshots` (-1: none), its words wrapped anew (a '\n' counts as a space)
 * and waiting for A. */
int ta_talk(TextArchive *t, const char *boxes, const int *mugshots);
/* Writes the archive (u16 offsets, then the scripts); bus address or 0. */
uint32_t ta_commit(TextArchive *t);

#endif
