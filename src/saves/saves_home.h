/* SAVES' home (docs/engine/saves-screens.md): its pages as Options' tabs,
 * each a short list that wraps, a line remembered per page; a value line
 * (Auto-copy) turned by Left and Right as Options' are; START's UNDO
 * IMPORT and the lines that change what is kept elsewhere (Turn on,
 * Unlink, Delete) asked first in the information box, Yes and No, No
 * chosen. */
#ifndef CW_SAVES_HOME_H
#define CW_SAVES_HOME_H

#include <stdbool.h>

/* Home's keys this frame (the view is home): L and R the page, Up and
 * Down the line, Left and Right a value, A the line or the answer, START
 * the undo's question, B a question's No; `turn` turns the page */
void saves_home_keys(void (*turn)(int step));
/* The question asked in the box: its line (SAVES_UNDO, SAVES_TURN_ON,
 * SAVES_UNLINK, SAVES_DELETE), -1 none; whether Yes is chosen */
int saves_home_asking(void);
bool saves_home_yes(void);
/* A line chosen (a tap, A): asked first where it asks, else done */
void saves_home_choose(int action);
/* The question answered (a tap on Yes or No, A): Yes does its line */
void saves_home_answer(bool yes);
/* B: a question let go, true where there was one */
bool saves_home_back(void);

#endif
