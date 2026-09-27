/* BN6's chat box for the engine's own screens (the title's question): its
 * frame drawn to the game's measure, its text in the game's chat font from
 * the ROM. */
#ifndef CW_CHATBOX_H
#define CW_CHATBOX_H

/* Where the game puts it on its 240x160 screen: the box, the face in it
 * (its centre, a mugshot's origin; 40x48 from 5,104) and the first of its
 * three lines, 14 apart. */
enum {
	CHATBOX_X = 1, CHATBOX_Y = 100, CHATBOX_W = 238, CHATBOX_H = 56,
	CHATBOX_FACE_X = 25, CHATBOX_FACE_Y = 128,
	CHATBOX_TEXT_X = 51, CHATBOX_TEXT_Y = 108, CHATBOX_LINE = 14,
};

/* The box at (x, y), w by h: a dark blue line and a light blue band around
 * the page, the corners rounded, and the tab on its top right (4 rows
 * above y). */
void chatbox_frame(int x, int y, int w, int h);
/* Text in the chat font, its top-left at (x, y), the first `n` characters
 * (-1 all); the pixels it took across. */
int chatbox_text(int x, int y, const char *s, int n);

#endif
