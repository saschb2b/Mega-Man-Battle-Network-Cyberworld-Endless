/* Another game's battles on a second core beside BN6's (docs/MULTIROM.md,
 * Guest battles): BN5's, on the layers BN5's areas dress, in BN5's own
 * engine. Desktop, Android, iOS and PortMaster builds; the 3DS keeps one
 * ROM, and the browser reads no other. */
#ifndef CW_GUEST_H
#define CW_GUEST_H

#include <stdbool.h>
#include <stdint.h>

enum { GUEST_WON, GUEST_LOST, GUEST_ESCAPED };
#define GUEST_DARK_KINDS 12   /* BN5's DarkChips (darkchips.h) */

/* Makes the guest core for extra ROM `xrom` (rom.h, XR) where the build
 * and the ROMs allow it, and boots it to a playable state (its title, NEW
 * GAME and intro, pressed through once and kept as a state in the data
 * directory); true once it is ready. */
bool guest_start(int xrom);

/* MegaMan as the run has him, into a guest battle */
typedef struct {
	int hp, max_hp;
	const uint16_t *folder;           /* the run's (30 BN6 entries, chip | code << 9), each chip as its game's of the same name; NULL: the guest's own */
	uint8_t dark[GUEST_DARK_KINDS];   /* his DarkChips, counts by kind */
	uint8_t buster[3];                /* his buster's Attack, Speed and Charge as BN6's NaviCust makes them (BN6_NAVI_ATTACK: 0-4, levels 1-5) */
	bool star;                        /* the All * helper: every chip in *, the rewards too */
} GuestMegaMan;

/* Begins a battle from BattleSettings record `record`, an address in the
 * guest's ROM (its own game's records: BN5_BATTLE_TABLES), with MegaMan as
 * `mm` has him: from the next guest frame the guest runs and BN6's core
 * waits. */
bool guest_battle(uint32_t record, const GuestMegaMan *mm);

/* The chips of `folder` (30 BN6 entries) that sit out of its battles, its
 * game having none of their names: each once, in the folder's order, the
 * first `max` into `out` (BN6 ids); how many there are, 0 before the guest
 * is ready. */
int guest_sitting_out(const uint16_t *folder, uint16_t *out, int max);

/* A guest battle runs: the scene shows and steers the guest. */
bool guest_active(void);
/* ... and its battle is on the screen: MegaMan's HP and max HP in it (a
 * state's); false before and after. */
bool guest_fight_hp(int *hp, int *max);
/* ... and its Custom gauge (0-0x4000) while it is fought, -1 otherwise */
int guest_custom_gauge(void);
/* ... and its Custom screen is up (the autopilot's chips and OK) */
bool guest_custom_screen(void);
/* ... and its battle is on its screen (its game state's battle, not the
 * room its boot left it in, which the battle's opening showed) */
bool guest_on_screen(void);
/* One frame of the guest with these keys held (GBA key bits). */
void guest_frame(uint32_t keys);
/* ... unseen and unheard: a frame run ahead while its battle opens behind
 * the white */
void guest_frame_quiet(uint32_t keys);
/* The guest's last frame, 240x160, as emu_video's. */
const uint32_t *guest_video(void);

typedef struct {
	int outcome;   /* GUEST_WON, GUEST_LOST (MegaMan deleted) or GUEST_ESCAPED */
	int frames;    /* how long it ran */
	int hp;        /* MegaMan's HP at its end */
	int chip;      /* (won) the reward: BN6's chip of its name, and its code (A=0, *=26); 0 none */
	int code;
	int zenny;     /* ... or zenny */
	bool dark_used;   /* a DarkChip was used in it */
	uint8_t dark[GUEST_DARK_KINDS];   /* the run's DarkChips left after it, by kind */
	int recoded;      /* the folder's chips that fought with another code (its game's chip lacked theirs) */
	int recode_chip, recode_from, recode_to;   /* the first of them: its BN6 id, its code, the code it fought with */
	int reward_from;  /* the code the reward chip had there, where it came back with another; -1 none */
} GuestResult;
/* Once a battle has ended (guest_active false again): its result, once. */
bool guest_take_result(GuestResult *out);

/* The background its battles stand in front of, by its own game's number
 * (NetAreaDef.xbg: the dressed area's), where a record leaves it to the
 * map (0xFF), which would be the map its boot state stands on; -1 the
 * map's. */
extern int guest_backdrop;

/* (dev: MegaMan worried through every guest battle, for captures of a
 * DarkChip offered: --dev worried) */
extern bool guest_dev_worried;

/* DarkChip `k`'s name in BN5 (0-11, ids 187-198), "" without BN5 */
const char *guest_dark_name(int k);

/* A record's strength, to hold its battles to the run's pacing: its
 * viruses' HP together and the strongest one's damage, from its game's
 * enemy tables (the ROM file, no core needed); how many viruses it sets
 * (a win counts them as the run's deleted), 0 for one the run never
 * draws: a battle its game keeps for a story's condition (its byte 7), a
 * Navi's, or none at all. */
int guest_record_strength(int xrom, uint32_t record, int *hp, int *damage);

/* Its game's battle records for net map (group, number): how many, and the
 * i-th's address; read from the ROM file, no core needed. */
int guest_records(int xrom, int group, int number);
uint32_t guest_record(int xrom, int group, int number, int i);

#endif
