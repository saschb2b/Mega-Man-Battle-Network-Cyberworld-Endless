/* Dev tools for testing a run quickly (devtools.c, docs/DEVTOOLS.md): a
 * menu over the game (hold SELECT, press R) and the same switches from the
 * command line (--dev). */
#ifndef CW_DEVTOOLS_H
#define CW_DEVTOOLS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	bool god;       /* MegaMan's HP stays full, in battle and out */
	bool onehit;    /* enemies keep 1 HP: one hit deletes them */
	bool quiet;     /* no random battles */
	bool fragile;   /* MegaMan keeps 1 HP in battle: the run ends at the first hit */
	int speed;      /* game frames per frame shown: 1, 2, 4, 8 */
	bool powers;    /* the five Crosses and BeastOut open (a capture's: tools/trailer.py) */
	bool gem;       /* every random battle with a Mystery Data on the field */
	bool visitors;  /* every random battle the visitors' rules allow with a visitor of the other net (visitors.h; issue #82) */
	bool veteran;   /* a profile that has met seven guardians, found two Spins and beaten the rival (captures: the PET's mail) */
	int duels;      /* duels=N: the rival's wins made N, for a capture of a rung or an official gate (docs/RIVAL.md); -1 left alone */
	int hp;         /* hp=N: MegaMan's max HP N, his HP with it each time the max moves (a later act's fight swept at a playtester's HP); 0 left alone */
	int hp_now;     /* hp=N/H: ... and his HP H through the run's first 600 frames (a battle begun hurt); 0 the max */
	bool worried;   /* MegaMan held worried by BN5's mood in BN6's battles, so its DarkChips come (soulmood.h; a capture of one offered) */
	bool mapall;    /* the layer's map whole, as if every panel were seen (a capture of the map: tools/before_after.py) */
	int folder;     /* folder=ID: the run's folder all chip ID in * (a capture of one chip's battles, the first hand all of it); 0 none */
	int folder_n;   /* folder=ID/N: ... its first N entries alone (one beside the run's own chips) */
	int pack;       /* pack=N: a copy of each of the first N chips in the pack, in its first code (the folder editor's pack, issue #75) */
	int programs;   /* programs=N: a copy of each of the first N NaviCust programs, in a colour the ROM draws (the NaviCustomizer, issue #76) */
	int clock;      /* clock=N: the Net's clock at N notches from the run's start (docs/HOME.md: its guardians' HP) */
	int job;        /* job=K: the run holding a request of kind K (jobs.h), taken; at --scene home one from the act before */
	int job_state;  /* jobstate=S: ... in state S (2 done, 3 failed) */
	int zenny;      /* zenny=N: N zenny given as the run starts (BN6's GiveZenny: home's shops, docs/HOME.md piece 6) */
	int side;       /* side=N: the run starts on side layer N at --run-depth, 1 the Undernet (its golden gate), 2 the Secret Area (Bass's once the title's S is held, --marks 08: docs/BOSSES.md) */
	int bass;       /* bass=N: MegaMan's wins over Bass made N (his form: Bass, SP from 1, BX from 2 with the Cybeast beaten) */
	int beast;      /* beast=N: ... and over the Cybeast (its rematch's words; Bass BX) */
	bool xcontrol;  /* the control family installed as the core starts: BN6's own Mettaur moved as AI 32, enemy ids 0x200-0x205
	                   (xvirus.h; its differential, tools/xcontrol.py) */
	bool nofamilies; /* no other game's family installed, BN5's ROM read or not (xvirus.h): BN6's battles as its own, to compare */
	bool xnavicontrol; /* the control Navi installed as the core starts: BN6's own HeatMan moved as Navi AI 25, ids 0x1C4-0x1C9
	                      (xnavi_install.h; its differential, tools/xcontrol.py --navi) */
	bool nonavis;   /* no other game's Navi installed, BN5's ROM read or not (xnavi_install.h) */
	bool chipai;    /* MegaMan under BN5's chip AI two seconds into each battle, for 600 frames (darkinvs.h: DarkInvs's machine) */
	bool soulproxy; /* form 8 made HeatCross's copy as the core starts (soulproxy.h: the Souls' proxy form; its differential, tools/xproxy.py) */
	int proxybreak; /* proxybreak=N: parts of it left out, the comparator's own test (soulproxy.h, SOULPROXY_BREAK_*) */
	int unitebreak; /* unitebreak=N: UNITE's rule broken, the differential's own test (soulunite.h, SOULUNITE_BREAK_*) */
	const char *online;     /* online=URL: Lan's HP's gate goes to this server, the keeper's questions skipped (docs/ONLINE.md) */
	const char *online_name; /* onlinename=NAME: ... as NAME */
	int soulbreak;  /* soulbreak=N: a Soul's look, abilities or passives broken, the differential's own test (soulunite.h, SOULBREAK_*) */
	bool rollbench; /* the start is the rollback's measurement: the game's core, then the Arena's pair (src/dev/rollbench.h, issue #128) */
	bool arena;     /* the start is the Arena's: a Practice NetBattle on two linked cores, the second player a script (arena.h, issue #120) */
	bool arena_busy; /* arenabusy: --dev arena's second player steps, shoots and OKs the Custom screen by the frame's number alone (the rollback's guesses tried) */
	bool arena_both; /* arenaboth: its first player a script too, the Custom screen's OK and charged shots; in the Net Square, Lan's Yes to a challenge at once and his keys the same script (a battle that plays itself, arena.h) */
	bool quit_end;  /* quitend: the game quits once --dev rollbench and --dev arena are done (a run sent to a device, 3dslink's netloader back) */
	int arena_delay; /* arenadelay=N: its input delay, N frames (1-60), and the delay a Net Square challenge or Yes asks for; 0 its own */
	int arena_window; /* arenawindow=N: the Arena rolls back N frames at most (0: the delay alone); -1 what a frame costs here affords (arena.h, issue #128) */
	int arena_lag;  /* arenalag=N: its local link's one way, N ticks; -1 its own */
	int arena_hp;   /* arenahp=N: the carried MegaMan's max HP made N there (the installer seen in the battle; a Net Square battle's too) */
	int arena_roll; /* arenaroll=N: --dev arena rolls back N frames every N, the game's state checked (arena_roll.h, issue #128); 0 the game's hash said alone; -1 none */
	int arena_desync; /* arenadesync=N: a Net Square battle's keys from the other player corrupted at frame N on this side, as a link that went wrong (the hashes' stop seen) */
} DevFlags;

extern DevFlags dev;

/* "god,onehit,quiet,speed=4,powers" */
void devtools_parse(const char *spec);
/* With `veteran`, once the profile is loaded: its guardians' records and
 * Spins, where it has none yet; with `duels=N`, the rival's wins. */
void devtools_veteran(void);
/* The player's GBA keys: the menu takes them while it is open. */
uint32_t devtools_keys(uint32_t keys);
/* The menu is open: the game holds still. */
bool devtools_open(void);
/* 10000 zenny, the game's way (the menu's, and --talk zenny). */
void devtools_zenny(void);
/* After each game frame: the switches' effects. */
void devtools_update(void);
void devtools_draw(void);

/* A frame to save after the next draw (the tour sets it); "" for none. */
extern char devtools_shot[512];

/* Trade Central's screens opened in a run, out of the Net Square, for a
 * scripted capture (trade_preview.c, issue #145; docs/DEVTOOLS.md): the
 * dev step "trade list [WANT]" lists the run's spares (the `chip` step
 * gives some), "trade deal ID CODE ID CODE" shows the chip trade of the
 * two (MegaMan's first) and "trade swap" swaps its cards as a done trade
 * does. The other player is "Beta"; nothing is sent, and what is chosen is
 * printed. */
void trade_preview(const char *spec);
bool trade_preview_on(void);
/* While one is up: its frame, the game's keys none */
uint32_t trade_preview_keys(uint32_t keys);
void trade_preview_draw(void);

#endif
