/* What the game's code tells the director through hooks (hook.h; docs/
 * EMULATION.md, Hooks): the kinds of event the hooks queue, which
 * director_update takes up after each frame. */
#ifndef CW_EVENTS_H
#define CW_EVENTS_H

enum {
	EV_BATTLE_START = 1,   /* StartBattle, as any battle begins: r0 its record */
	EV_MEGAMAN_HIT,        /* MegaMan's HP lowered, in a watched battle (emu_battle_watch) */
	EV_MAP_ENTER,          /* EnterMap: a map entered, its flags cleared */
	EV_CHOICE,             /* SetEventFlag on a layer's choice flag: r0 the flag */
	EV_ITEM_GIVEN,         /* GiveItem: r0 the key item, r1 how many */
	EV_GUEST_BATTLE,       /* the encounter roll gave a battle on a layer whose battles are the guest's (encounter_guest) */
	EV_DARK_RAN,           /* a DarkChip's dark power ran for MegaMan, a BugFrag paid (darkbn6.h): r0 the chip */
	EV_DARK_BASE,          /* ... or, with no BugFrag, its base chip ran: r0 its DarkChipID */
	EV_JOB,                /* SetEventFlag on one of the jobs' flags at home (job_words.h): r0 the flag */
};

/* The hooks for the map, flag and key item events, once the core is up
 * (the battle's: emu_encounters_install). */
void events_install(void);

#endif
