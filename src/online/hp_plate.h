/* A MegaMan's HP under him as BN6 writes an enemy's (docs/rom/graphics.md,
 * The HP under an enemy): BN6's own battle digits, white, orange or green,
 * read from the player's ROM and drawn over the picture, the number
 * counted toward his HP a frame at a time as BN6 counts an enemy's
 * (BN6_HP_PLATE_STEP). For the MegaMen BN6 gives no such number: a
 * partner's on a party's field (party_marks.h) and the ally's in the
 * Arena's two a side (arena_team_view.h). Reads the core, changes
 * nothing of it: the count is the drawing's own. */
#ifndef CW_HP_PLATE_H
#define CW_HP_PLATE_H

#include <stdbool.h>
#include <stdint.h>

struct mCore;

/* one MegaMan's number as last drawn: what it shows, its colour (a
 * BN6_HP_FONT_*), the frames before it turns white again, and the
 * battle's frame (BN6_BATTLE_TIMER) it was counted to */
typedef struct {
	uint16_t shown;
	uint8_t font, timer;
	uint32_t at;
	bool on;
} HpPlate;

/* the number of `object` (a MegaMan of the battle on `core`) centred on
 * canvas x, its cells' top at y as BN6 places its OBJ (the lit rows
 * BN6_HP_DIGIT_TOP under it, BN6_HP_DIGIT_H of them): counted a step for
 * each frame the battle has run since it was last drawn, or begun afresh
 * at his HP, as BN6's is when it lists a foe, where that is not a few
 * frames on (a new battle, a rollback; the timer holds on the Custom
 * screen, so the count goes on after it) */
void hp_plate_draw(HpPlate *plate, struct mCore *core, uint32_t object, int x, int y);

#endif
