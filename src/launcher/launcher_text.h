/* The launcher's words (launcher_text.c): its labels, buttons and notes,
 * in the plain style of the game's menus (docs/VOICE.md), apart from the
 * logic that says when. */
#ifndef CW_LAUNCHER_TEXT_H
#define CW_LAUNCHER_TEXT_H

#include <stdbool.h>
#include <stddef.h>

/* The two cartridges */
enum { SLOT_BN6, SLOT_BN5, SLOTS };

/* What a note is about: its colour */
enum { NOTE_INFO, NOTE_GOOD, NOTE_BAD };

/* The words that stand alone */
typedef enum {
	W_TITLE, W_PLACE, W_PLACE_SHORT,
	W_NEEDED, W_OPTIONAL, W_READY,
	W_CHOOSE_FILE, W_CHOOSE_FOLDER, W_LOOK_AGAIN, W_WAITING,
	W_PLAY, W_DONE, W_FILES_INSTEAD,
	W_PLAY_LOCKED, W_FIVE_ADDS, W_CANCELLED,
	W_COUNT
} LauncherWord;
const char *launcher_word(LauncherWord w);

/* A cartridge's names: on its open spot ("BN6"), under it ("Cybeast
 * Gregar (USA)"), on its label (one line, or two on a large one) */
const char *slot_short(int slot);
const char *slot_caption(int slot);
const char *slot_label(int slot, int line, bool large);

/* The note for what the cursor is on. `kind` the pickers there are
 * (pick.h's bits; 0 none), `where` the place to put a ROM by hand */
void note_slot_empty(int slot, unsigned kind, const char *where, char *out, size_t n);
void note_slot_ready(int slot, const char *kept, char *out, size_t n);
void note_play(bool bn5, bool from_title, const char *title_button, char *out, size_t n);
void note_alt(unsigned kind, char *out, size_t n);

/* A chosen file's fate: taken ("Took BN6 Cybeast Gregar (USA) from x.gba,
 * and BN5 beside it"), or why not */
void note_took(int slot, const char *file, bool five_beside, char *out, size_t n);
void note_refused(const char *file, const char *why, char *out, size_t n);
/* why a ROM with header game code `code` is not taken (BN6's and BN5's
 * other versions named, as the phones' pages name them) */
const char *refuse_why(const char code[4], int slot_of_code, bool changed);
/* ... a file that was no ROM at all, or zipped */
const char *refuse_not_rom(bool zipped);
/* The ROM found where the game looked at the start */
void note_found(const char *where, char *out, size_t n);
/* ... gone since the last start */
void note_five_gone(char *out, size_t n);
/* A note that there is no file chooser here: where to put the file */
void note_no_picker(const char *where, char *out, size_t n);

/* The saves' line: where they are kept. A phone's is two lines, the first
 * one standing alone where there is room for no more: its folder's name
 * (NULL or "": none chosen), whether automatic copies are enabled, and
 * `refused` where the last copy could not be written there */
void saves_line_desktop(const char *where, char *out, size_t n);
void saves_line_phone(const char *folder, bool automatic, bool refused, char *out, size_t n);

/* "Press Esc again to quit", as main.c says it where there is a ROM */
const char *quit_words(bool pad);
/* The key hints: "J: choose  Return: play", from the device's words;
 * `brief` the last alone */
void hint_words(const char *a, const char *start, const char *b, bool from_title, bool brief, char *out, size_t n);

#endif
