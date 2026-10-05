/* The layers' service and choice scripts, in the game's text language. */
#ifndef CW_SCRIPTS_H
#define CW_SCRIPTS_H

#include <stdbool.h>

#include "text.h"

/* The ScrtData key item: three open the Secret Area's gate. */
#define SCRIPTS_SECRET_DATA 0x31
/* HPMemory, and how many a beaten guardian leaves */
#define SCRIPTS_HP_MEMORY 0x70
#define SCRIPTS_REG_UP1   0x72   /* RegUP1-3: +1, +2, +3 MB of Reg memory (bn6f sub_803CFB0: 4 + RegUP1 + 2 RegUP2 + 3 RegUP3, at most 99) */
#define SCRIPTS_TAG_CHIP  0x0B   /* the TagChip system's key item (issue #51) */
#define SCRIPTS_BOSS_HP_MEMORIES 5
#define SCRIPTS_EXP_MEMORY 0x71   /* ExpMemry, the NaviCust's board: 4x4, 5x4, 5x5 */

/* The chat box's version marks ([RV] [BX] [EX] [SP] [FZ], codes 0x40-0x44)
 * drawn as their two letters in the font's own capitals, in the core's
 * ROM copy: its marks are two letters stacked in one cell, which read as
 * a kanji ("ProtoMn" and one came out of a Mystery Data). */
void chat_marks_install(void);

/* Service NPCs on the game's own commands: a recovery Mr. Prog heals
 * (`variant` picks his words) once on his layer, `amount` HP (0: to full),
 * then says his patch is spent; `amount` < 0, the Heals helper's: to full,
 * as often as asked (issue #71). `told_flag` marks it given. (Chip Traders
 * speak the game's own lines, see trader.h.) */
int ta_heal(TextArchive *t, int variant, int told_flag, int amount);
/* Words that set `flag` as they are said (a bystander's news the director
 * keeps: docs/META.md, what MegaMan knows). */
int ta_say_flag(TextArchive *t, int face, const char *s, int flag);
/* The rival's duel (docs/RIVAL.md): ProtoMan's terms (boxes apart by '|'),
 * then "Take the duel?", starting on No; Yes sets `flag`. */
int ta_duel(TextArchive *t, int flag, int face, const char *terms);
/* A shopkeeper with `face`: `greeting` (ta_talk's boxes), then shop
 * `shop`'s screen; with `again` and a flag, the greeting the first time
 * (the flag set) and `again` after; `sold_out` instead of both, and no
 * screen, once nothing is left to buy. */
int ta_shop(TextArchive *t, int shop, int face, const char *greeting, const char *again, const char *sold_out, int told_flag);

/* Choices: Yes sets event flag `flag`, which the director acts on. A
 * challenge answers only once; the gate first wants three ScrtData. */
int ta_challenge(TextArchive *t, int flag, const char *prize, const char *navi);
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
	int fit_flag;   /* event flag + k set where program k fits the board's free space as it stands; 0 none */
} ScriptsDraft;

/* What a guardian's Guardian Data gives (ta_guardian_reward). */
typedef struct {
	const char *name;        /* the navi */
	const char *power;       /* what a Cross or BeastOut brings, ta_talk's boxes; NULL for none */
	int chip, code;          /* his navi chip, 0 for none; its code A=0 .. *=26 */
	const char *chip_name;
	bool last;               /* the run's last guardian: only what carries over */
	int taken_flag;          /* event flag set on every branch of the draft */
	int hp_memories;         /* HPMemory through the game's own item, +20 max HP each */
	const ScriptsDraft *draft;   /* the NaviCust's draft, NULL for none */
} ScriptsReward;

/* A guardian's Guardian Data, checked: his power, then the HPMemory, his
 * navi chip, a full heal, the NaviCust's draft, then the taken flag, and
 * last the way on. The run's last guardian gives only what carries over:
 * his chip, for the Library. */
int ta_guardian_reward(TextArchive *t, const ScriptsReward *r);
/* A flame of darkness (docs/META.md, DarkChips in BN5 territory): MegaMan
 * names its DarkChip `chip` and its price (all of it the `first` time), and
 * where BN6 keeps that kind too (`ours`), that our net's battles play it
 * from the folder for a BugFrag; then asks, starting on Yes; Yes sets event
 * flag `flag`, which it leaves by. */
int ta_dark_flame(TextArchive *t, int flag, const char *chip, bool first, bool ours);
/* A flame of darkness of BN6's own (docs/META.md, BN6's own DarkChips): its
 * DarkChip, its base chip BN6 plays it as without a BugFrag, what its dark
 * power does in MegaMan's words, and whether he says all of its price (a
 * profile's first). */
typedef struct {
	const char *chip, *base, *does;
	bool first;
} ScriptsDark6;
/* MegaMan names it, the BugFrag a use burns, the NaviCust's bug for the
 * battle, the base chip, the max HP (all of it the `first` time, the rule
 * after), then asks, starting on Yes; Yes sets event flag `flag`, which it
 * leaves by. */
int ta_dark_flame6(TextArchive *t, int flag, const ScriptsDark6 *d);
/* BN6's own words on DarkChips, a post of its BBS run from the player's
 * ROM (docs/ROM_DATA.md), said by a bystander with `face` on a flame's
 * layer; -1 where the ROM's text is not as expected. */
int ta_dark_rumor(TextArchive *t, int face);
/* A gate sealed with `navi`'s code: while `beaten` (his deletions as a
 * guardian, any runs) is short of `needed`, its words say so (flag -1:
 * no choice); else it asks for his SP, flag set on Yes, and says the gate
 * is quiet once it has been. */
int ta_navi_gate(TextArchive *t, int flag, const char *navi, int beaten, int needed);
/* A security cube (issue #45). One asking a P-Code: once a navi of the
 * layer has told it (flag `told`), A opens it (clears flag `present`); else
 * MegaMan says to ask around. One taking a toll: `price` zenny to pass,
 * asked, and paid, it opens. And the navi who tells the P-Code (sets
 * `told`). */
int ta_cube_pcode(TextArchive *t, int present, int told, const char *code);
int ta_cube_toll(TextArchive *t, int present, int price);
int ta_pcode_teller(TextArchive *t, int face, const char *code, int told);
/* The Undernet's doors (issue #47): a skull door opens for the WWW-ID held
 * (key item `id`); a number door asks how many braziers burn on the layer,
 * `answer`, as a choice of three (`seed` orders them), and a wrong answer
 * seals it for the layer (flag `sealed`). */
int ta_cube_skull(TextArchive *t, int present, int id);
int ta_cube_number(TextArchive *t, int present, int sealed, int answer, unsigned seed);
/* A Navi gate's win: his SP chip. */
int ta_gate_reward(TextArchive *t, const char *navi, int chip, const char *chip_name, int code);
/* A collector's vault (docs/META.md, gates): its three chips (`code`
 * A=0 .. *=26), named, and what each hits for (0: none to say). */
typedef struct {
	int chip[3], code[3], power[3];
	char name[3][20], desc[3][64];   /* desc: its description (chip_desc) */
} ScriptsVault;
/* The vault's talk: while the Library's `have` is short of `need`, its
 * words say so; else the three chips, one to take (event flag `flag` set
 * as it is, after which the vault stands empty; B leaves them). */
int ta_vault(TextArchive *t, int flag, int need, int have, const ScriptsVault *v);
/* An official gate (docs/RIVAL.md): sealed while the rival's duels `won`
 * are short of `need`, its words say for whom it opens; else its three
 * chips, one to take (a Chip Order from the Library at level 1, Mega
 * chips at level 2). `duel_prize`: the one beside ProtoMan, the clearance
 * held, waiting on his duel's winner. */
int ta_official(TextArchive *t, int flag, int open_flag, int level, int won, bool duel_prize, const ScriptsVault *v);
/* A won challenge's own reward: a chip. */
int ta_challenge_reward(TextArchive *t, int chip, const char *chip_name, int code);
/* What Dr. Hikari's dive support Mr. Prog offers on the run's first layer
 * (ta_gift). */
typedef struct {
	int flag;                /* event flag: taken */
	bool comfort;            /* after a run lost early: an HPMemory more first */
	bool brief;              /* a returning player: one page, not three */
	bool head_start;         /* the head-start helper's two HPMemory first */
	int chip, code, power;   /* the chip, its code A=0 .. *=26, what it hits for */
	const char *chip_name;
	int program, color;      /* the NaviCust program, named by the game */
	const char *about;       /* what the program does, or NULL */
} ScriptsGift;

/* The run's first layer: the dive support Mr. Prog offers one of two
 * HPMemory, the chip or the NaviCust program, once. */
int ta_gift(TextArchive *t, const ScriptsGift *g);

#endif
