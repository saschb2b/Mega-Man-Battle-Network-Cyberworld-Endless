/* The courier in Lan's HP, the director's part (director_courier.c,
 * docs/HOME.md piece 2, issue #108). */
#ifndef CW_DIRECTOR_COURIER_H
#define CW_DIRECTOR_COURIER_H

/* As home's places are installed, the askers' flags set: what the town
 * holds for this visit, from the run's state (the same at a CONTINUE).
 * Then each frame at home: whether any of it still waits, his flag and
 * his words as it stands. */
void home_courier_visit(void);
void home_courier_frame(void);

#endif
