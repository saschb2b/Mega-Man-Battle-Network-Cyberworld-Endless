/* The layers' service and choice scripts, in the game's text language. */
#ifndef CW_SCRIPTS_H
#define CW_SCRIPTS_H

#include <stdbool.h>

#include "text.h"

/* The ScrtData key item: three open the Secret Area's gate. */
#define SCRIPTS_SECRET_DATA 0x31
/* HPMemory, and how many a beaten guardian leaves */
#define SCRIPTS_HP_MEMORY 0x70
#define SCRIPTS_BOSS_HP_MEMORIES 5
#define SCRIPTS_EXP_MEMORY 0x71   /* ExpMemry, the NaviCust's board: 4x4, 5x4, 5x5 */

/* The chat box's version marks ([RV] [BX] [EX] [SP] [FZ], codes 0x40-0x44)
 * drawn as their two letters in the font's own capitals, in the core's
 * ROM copy: its marks are two letters stacked in one cell, which read as
 * a kanji ("ProtoMn" and one came out of a Mystery Data). */
void chat_marks_install(void);

/* Service NPCs on the game's own commands: a recovery Mr. Prog heals to
 * full HP (`variant` picks his words). (Chip Traders speak the game's own
 * lines, see trader.h.) */
int ta_heal(TextArchive *t, int variant, int told_flag);
/* A shopkeeper with `face`: `greeting` (ta_talk's boxes), then shop
 * `shop`'s screen; with `again` and a flag, the greeting the first time
 * (the flag set) and `again` after. */
int ta_shop(TextArchive *t, int shop, int face, const char *greeting, const char *again, int told_flag);

/* Choices: Yes sets event flag `flag`, which the director acts on. A
 * challenge answers only once; the gate first wants three ScrtData. */
int ta_challenge(TextArchive *t, int flag);
int ta_undernet(TextArchive *t, int flag, bool deeper);
int ta_secret_gate(TextArchive *t, int flag);
/* Plays song `song` (0xFF stops the music, SCRIPTS_AREA_MUSIC the map's
 * own) without opening the chat box. */
#define SCRIPTS_AREA_MUSIC -1
int ta_music(TextArchive *t, int song);
/* What a Guardian Data adds for the NaviCust (docs/NAVICUST.md): an
 * ExpMemry, then a draft of `n` programs (give ids and colours, with
 * MegaMan's words for each) or none for `skip_frags` BugFrags; `teach` adds
 * a word on the board's rules (the run's first draft). */
typedef struct {
	int expmemry;         /* the board it grows to: 0 none, 1 5x4, 2 5x5 */
	int n;
	uint8_t program[3], color[3];
	const char *about[3];
	int skip_frags;
	bool teach;
} ScriptsDraft;

/* A guardian's Guardian Data, checked: `power` (what a Cross or BeastOut
 * brings, ta_talk's boxes; NULL for none), then HPMemory through the game's
 * own item (+20 max HP each), its navi chip (`chip` 0 for none; `code`
 * A=0 .. *=26), a full heal, the NaviCust's `draft` (NULL for none), then
 * event flag `taken_flag` (on every branch of the draft). */
int ta_guardian_reward(TextArchive *t, const char *name, const char *power, int chip, const char *chip_name, int code,
                       int taken_flag, const ScriptsDraft *draft);
/* A won challenge's own reward: a chip. */
int ta_challenge_reward(TextArchive *t, int chip, const char *chip_name, int code);
/* The run's first layer: Dr. Hikari's dive support Mr. Prog offers one of
 * two HPMemory, a chip (`power`: what it hits for) or a NaviCust program
 * (`program` in `color`, named by the game; `about`: what it does, or
 * NULL), once (event flag `flag`); with `comfort`, after a run lost
 * early, an HPMemory more first. */
int ta_gift(TextArchive *t, int flag, bool comfort, bool brief, bool head_start, int chip, const char *chip_name, int power, int code, int program, int color,
            const char *about);

#endif
