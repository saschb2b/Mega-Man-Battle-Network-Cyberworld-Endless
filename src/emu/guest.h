/* Another game's battles on a second core beside BN6's (docs/MULTIROM.md,
 * Guest battles): BN5's, on the layers BN5's areas dress, in BN5's own
 * engine. Desktop, Android, iOS and PortMaster builds; the 3DS keeps one
 * ROM, and the browser reads no other. */
#ifndef CW_GUEST_H
#define CW_GUEST_H

#include <stdbool.h>
#include <stdint.h>

/* Makes the guest core for extra ROM `xrom` (rom.h, XR) where the build
 * and the ROMs allow it, and boots it to a playable state (its title, NEW
 * GAME and intro, pressed through once and kept as a state in the data
 * directory); true once it is ready. */
bool guest_start(int xrom);

/* Begins a battle from BattleSettings record `record`, an address in the
 * guest's ROM (its own game's records: BN5_BATTLE_TABLES), with MegaMan at
 * `hp` of `max_hp` and the run's `folder` (30 BN6 entries, chip | code <<
 * 9; NULL: the guest's own), each chip as its game's of the same name:
 * from the next guest frame the guest runs and BN6's core waits. */
bool guest_battle(uint32_t record, int hp, int max_hp, const uint16_t *folder);

/* A guest battle runs: the scene shows and steers the guest. */
bool guest_active(void);
/* ... and its Custom screen is up (the autopilot's chips and OK) */
bool guest_custom_screen(void);
/* One frame of the guest with these keys held (GBA key bits). */
void guest_frame(uint32_t keys);
/* The guest's last frame, 240x160, as emu_video's. */
const uint32_t *guest_video(void);

enum { GUEST_WON, GUEST_LOST, GUEST_ESCAPED };
typedef struct {
	int outcome;   /* GUEST_WON, GUEST_LOST (MegaMan deleted) or GUEST_ESCAPED */
	int frames;    /* how long it ran */
	int hp;        /* MegaMan's HP at its end */
	int chip;      /* (won) the reward: BN6's chip of its name, and its code (A=0, *=26); 0 none */
	int code;
	int zenny;     /* ... or zenny */
} GuestResult;
/* Once a battle has ended (guest_active false again): its result, once. */
bool guest_take_result(GuestResult *out);

/* Its game's battle records for net map (group, number): how many, and the
 * i-th's address; read from the ROM file, no core needed. */
int guest_records(int xrom, int group, int number);
uint32_t guest_record(int xrom, int group, int number, int i);

#endif
