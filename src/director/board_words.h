/* MegaMan on the NaviCust (board_words.c), for the director's other parts. */
#ifndef CW_BOARD_WORDS_H
#define CW_BOARD_WORDS_H

const char *off_board_words(void);
/* what Reg memory MegaMan speaks of (reg_words) */
enum { REG_LESSON, REG_UP, REG_TAGCHIP };
const char *reg_words(int kind, int reg);
const char *compressed_words(const char *about);
const char *clean_words(void);

#endif
