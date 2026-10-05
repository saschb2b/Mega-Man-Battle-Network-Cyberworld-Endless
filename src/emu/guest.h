/* Another game's battles on a second core beside BN6's (docs/MULTIROM.md,
 * Guest battles): BN5's, on the layers BN5's areas dress, in BN5's own
 * engine. Desktop, Android, iOS, PortMaster and browser builds; the 3DS
 * keeps one ROM. */
#ifndef CW_GUEST_H
#define CW_GUEST_H

#include <stdbool.h>
#include <stdint.h>

#include "rom.h"

enum { GUEST_WON, GUEST_LOST, GUEST_ESCAPED };
#define GUEST_DARK_KINDS 12   /* BN5's DarkChips (darkchips.h) */

/* Makes the guest core for extra ROM `xrom` (rom.h, XR) where the build
 * and the ROMs allow it, and boots it to a playable state (its title, NEW
 * GAME and intro, pressed through once and kept as a state in the data
 * directory; that state loaded later) in the background, never in a
 * frame's way: on a thread of its own on native builds, else a slice of
 * each frame's spare time (guest_tick: the browser's). True once it is
 * ready or as its boot begins: a layer's battles are its game's either way
 * (what a layer makes never hangs on the boot), and a battle asked before
 * the boot is done waits for it (guest_boot_waiting). False where it
 * cannot be. */
bool guest_start(int xrom);
/* The frames before a run (the boot screen, the title): BN5's boot begun
 * as early as it can be, where BN5's ROM is there (in the browser only
 * where its state is not kept: a layer loads that quickly), so a battle
 * seldom waits for it. */
void guest_warm(void);
/* Each frame, after its drawing: the boot run on for about `ms`
 * milliseconds (the frame's spare time) where it runs a slice a frame, or
 * its end taken in where its thread is done. At its end the guest is
 * ready, and a battle that waited for it begins. */
void guest_tick(int ms);
/* Its boot's thread stopped and waited for, as the game ends */
void guest_quit(void);
/* How far its boot is, 0-99 (percent), while one runs; -1 none */
int guest_boot_progress(void);
/* ... and about how long it has left at the pace it has kept, in
 * milliseconds; 0 while that pace is not known yet, -1 none */
int guest_boot_left_ms(void);
/* A battle waits for its boot (guest_battle asked before it was done) */
bool guest_boot_waiting(void);
/* (dev: its boot at this many of its frames a frame, on the main thread,
 * the same frames every run, so a battle waits for it behind its screen:
 * --dev slowboot=N) */
extern int guest_dev_slowboot;

/* A battle's scaling to the act (docs/PROGRESSION.md, BN5's battles): its
 * record's viruses `up` versions up, each to version `vcap` at most (0 V1,
 * 3 its Omega) and never below its own, as BN6's battles take the act's
 * version (pacing.c): within a family (BN5's ids table, version and AI
 * index) the formation stays, its viruses grow. */
typedef struct { int up, vcap; } GuestScale;

/* MegaMan as the run has him, into a guest battle */
typedef struct {
	int hp, max_hp;
	const uint16_t *folder;           /* the run's (30 BN6 entries, chip | code << 9), each chip as its game's of the same name; NULL: the guest's own */
	uint8_t dark[GUEST_DARK_KINDS];   /* his DarkChips, counts by kind */
	uint8_t buster[3];                /* his buster's Attack, Speed and Charge as BN6's NaviCust makes them (BN6_NAVI_ATTACK: 0-4, levels 1-5) */
	bool star;                        /* the All * helper: every chip in *, the rewards too */
	uint8_t codes[3];                 /* the folder's codes (run.codes: 1 + the letter, 0 none): half of BN5's chip rewards come in one, as BN6's do */
	uint8_t souls;                    /* the run's Souls (souls.h), bit k Team Colonel's Soul 7 + k: offered on its Custom screen as its own rule
	                                   * offers them (UNITE), Chaos Unison with a DarkChip of their kind (docs/META.md) */
} GuestMegaMan;

/* Begins a battle from BattleSettings record `record`, an address in the
 * guest's ROM (its own game's records: BN5_BATTLE_TABLES), with MegaMan as
 * `mm` has him: from the next guest frame the guest runs and BN6's core
 * waits (first for the guest's boot, where it still runs:
 * guest_boot_waiting; a headless run's waits for it at once). */
bool guest_battle(uint32_t record, GuestScale sc, const GuestMegaMan *mm);

/* A territory's guardian from its game (docs/BOSSES.md, BN5's Navis): Navi
 * `ai` (his ids table's AI index, bn5.h BN5_NAVI_*) at `version` (0 V1 ..
 * 3 SP), his HP `hp_cap` at most as he spawns (0: his own), his results
 * screen paying `zenny` for each chip of his reward rows, as BN6's
 * guardians' battles pay where their row holds their chip */
typedef struct { int ai, version, hp_cap, zenny; } GuestBoss;
/* Begins his battle as guest_battle begins one, from his game's own record
 * for him at that version (guest_navi_record): no running, its results
 * screen on; false where it cannot be. */
bool guest_boss_battle(const GuestBoss *boss, const GuestMegaMan *mm);
/* Whether battles of game `xrom` can be fought here: the build runs a
 * second core, its ROM is read, and the guest has not failed to boot (one
 * not booted yet counts). */
bool guest_possible(int xrom);
/* BN6's standard chips whose namesake in the guest's game is of kind
 * `kind` (its chip records' BN5_CHIP_KIND), their ids into `out`, in BN6's
 * order; how many (the names paired at the first call with both ROMs read,
 * 0 without its ROM) */
int guest_kind_chips(int kind, uint16_t *out, int max);

/* The chips of `folder` (30 BN6 entries) that sit out of its battles, its
 * game having none of their names: each once, in the folder's order, the
 * first `max` into `out` (BN6 ids); how many there are, 0 without its ROM
 * (its boot may still run). */
int guest_sitting_out(const uint16_t *folder, uint16_t *out, int max);

/* A guest battle runs: the scene shows and steers the guest. */
bool guest_active(void);
/* ... and its battle is on the screen: MegaMan's HP and max HP in it (a
 * state's); false before and after. */
bool guest_fight_hp(int *hp, int *max);
/* ... and its Custom gauge (0-0x4000) while it is fought, -1 otherwise */
int guest_custom_gauge(void);
/* ... and its battle is on its screen (its game state's battle, not the
 * room its boot left it in, which the battle's opening showed) */
bool guest_on_screen(void);
/* ... its memory, bus addresses (its battle as bn5.h names it: the
 * autopilot's and the dev switches'); 0 without a guest */
uint8_t guest_read8(uint32_t addr);
uint16_t guest_read16(uint32_t addr);
uint32_t guest_read32(uint32_t addr);
void guest_write16(uint32_t addr, uint16_t v);
/* BN6's chip of the same name as its game's chip `id`, 0 for none (its
 * DarkChips) */
int guest_chip_bn6(int id);
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
	int heal;      /* ... or the HP its results screen restored (HP+50: hp counts it already) */
	bool dark_used;   /* a DarkChip was used in it */
	bool dark_rose;   /* ... and MegaMan fell in it and rose again (BN5's own: at 1 HP, the darkness fighting with his body a while) */
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

/* A record's strength with its viruses scaled (GuestScale; { 0, 0 } as
 * its game has it), to hold its battles to the run's pacing: its viruses'
 * HP together and the strongest one's damage, from its game's enemy tables
 * (the ROM file, no core needed); how many viruses it sets (a win counts
 * them as the run's deleted), 0 for one the run never draws: a battle its
 * game keeps for a story's condition (its byte 7), a Navi's, or none at
 * all. */
int guest_record_scaled(int xrom, uint32_t record, GuestScale sc, int *hp, int *damage);
/* ... and its enemies by its game's ids, so scaled, the first `max` into
 * `ids`; how many (a Navi, a story's battle among them) */
int guest_record_foes_scaled(int xrom, uint32_t record, GuestScale sc, int *ids, int max);

/* Its game's own battle record for Navi `ai` at `version` (0 V1 .. 3 SP):
 * its story's first that sets him alone, else its net maps' (his SP, where
 * he roams); 0 none. Read from the ROM file, no core needed. */
uint32_t guest_navi_record(int xrom, int ai, int version);
/* Navi `ai`'s HP at `version` in its game's stats, -1 none; his element
 * into *element where it is not NULL (BN6's numbering, ELEM_*: 0 none, 1
 * Fire, 2 Aqua, 3 Elec, 4 Wood) */
int guest_navi_hp(int xrom, int ai, int version, int *element);
/* The chip kind (BN5_CHIP_KIND) Navi `ai`'s Soul unites with in its game,
 * -1 none */
int guest_soul_kind(int xrom, int ai);

/* Its game's battle records for net map (group, number): how many, and the
 * i-th's address; read from the ROM file, no core needed. */
int guest_records(int xrom, int group, int number);
uint32_t guest_record(int xrom, int group, int number, int i);

/* The act's band for a guest battle (pacing.c): its viruses' HP together
 * from `lo` (the floor) to `hi` (the cap), their strongest hit `cap` at
 * most, their versions `vcap` at most */
typedef struct { int lo, hi, cap, vcap; } GuestBand;
/* The records a battle on net map (group, number) of `area` (an area of
 * game `xrom`) is picked from, each at the most versions up (ups[i], to
 * the band's vcap) that keep it under the band's cap: inside the band (HP
 * from its floor) the map's own, else the area's maps', else every area
 * its game lends; none inside anywhere, the same three under the floor;
 * else the map's own weakest alone (*fits false). Into out and ups, how
 * many (0: none at all). Read from the ROM file, no core needed: the
 * director's pick and the pacing report's. */
int guest_pool(int xrom, const NetAreaDef *area, int group, int number, const GuestBand *band, uint32_t *out, uint8_t *ups, int max, bool *fits);

#endif
