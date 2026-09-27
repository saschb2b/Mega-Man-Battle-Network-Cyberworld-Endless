/* Text archives in the game's text script language (bn6f
 * text_script_commands.inc), written into the free ROM space. */
#ifndef CW_TEXT_H
#define CW_TEXT_H

#include <stdbool.h>
#include <stdint.h>

#define TEXT_MAX_SCRIPTS 32
#define TEXT_MAX_BYTES 4096

typedef struct {
	uint8_t buf[TEXT_MAX_BYTES];
	uint16_t off[TEXT_MAX_SCRIPTS];
	int len, n;
	bool full;   /* something did not fit (and was left out) */
} TextArchive;

void ta_begin(TextArchive *t);
/* `s` in the game's charmap, at most `max` bytes; returns how many. */
int ta_encode(const char *s, uint8_t *out, int max);
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

/* The speakers' faces (the game's mugshots; docs/ROM_DATA.md). Townsfolk of
 * sprite list 5 have the face of their sprite less 0x20, Navis of list 6
 * that of their sprite. */
#define FACE_NONE    -1
#define FACE_LAN     0x00
#define FACE_MAYL    0x01
#define FACE_CHAUD   0x04
#define FACE_DAD     0x05
#define FACE_MEGAMAN 0x37
#define FACE_PROG    0x3C   /* Mr. Prog */
#define FACE_NAVI    0x3E   /* a Normal Navi, the Net Dealers' own */
#define FACE_TECH    0x39   /* a heavy engineer Navi, the NaviCust vendor (shop 3's keeper, 0x42, is the Net Dealer's green navi on the map) */
#define FACE_HEEL    0x43   /* a HeelNavi */
#define FACE_BEAST   0x58   /* MegaMan in Gregar's BeastOut */

/* A page of a chat: `face`, then the box opened (first) or cleared, `s`
 * word-wrapped to the box (20 characters a line; '\n' breaks the line, a
 * new box every three lines), waiting for A. */
void ta_page(TextArchive *t, int face, const char *s, bool first);
/* Pages of a conversation into the script under way (ta_talk's boxes);
 * `first` opens the chat box, and is cleared. */
void ta_pages(TextArchive *t, const char *boxes, int face, bool *first);
/* A conversation: chat boxes split by '|', each with `face` or the one its
 * speaker mark names ("@L Leave it to us!": L Lan, M MegaMan, D Dad, P Mr.
 * Prog, B MegaMan's beast, C Chaud, Y Mayl, H HeelNavi, N no face). */
int ta_talk(TextArchive *t, const char *boxes, int face);
/* The same, one box (or as many as `s` fills). */
int ta_say(TextArchive *t, int face, const char *s);
/* The archive's bytes (u16 offsets, then the scripts) into `out`
 * (TEXT_ARCHIVE_MAX bytes); returns their length. */
#define TEXT_ARCHIVE_MAX (TEXT_MAX_SCRIPTS * 2 + TEXT_MAX_BYTES)
int ta_build(const TextArchive *t, uint8_t *out);
/* Writes the archive; bus address or 0. */
uint32_t ta_commit(TextArchive *t);

#endif
