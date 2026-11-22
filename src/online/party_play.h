/* The guardian's fight on the party's core (the shared dive, issue #589;
 * docs/online/shared-dive.md, As built): the run's staging hands the fight
 * over at its start (boss_party_fight), the run's core holds still
 * (online_visit_held), and the party's core (party_core.h) fights it,
 * every seat's keys on it. Over the server (ARENA_STATE START with format
 * ONLINE_ARENA_PARTY) each seat's MegaMan, keys and hashes go to the
 * others through ARENA_SEAT as a 2v2's do (arena_link_net_party), each
 * frame run once every seat's keys for it have come; a seat gone leaves
 * the field and the hashes stop (each game then finishes its own battle,
 * the owner's call). Over, BN6's results on the party's core, then won or
 * lost and this seat's HP back into the run (boss_party_over).
 *
 * --dev partycore=N: every guardian's fight on the party's core with N
 * MegaMen, the partners scripted (partytwin=1: a second core beside it
 * from the same keys, the hashes compared every frame; partyroll=W:
 * rolled back W frames every W, the hash checked against the frames run
 * straight). */
#ifndef CW_PARTY_PLAY_H
#define CW_PARTY_PLAY_H

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "director_crew.h"
#include "foes.h"
#include "party_field.h"

/* The fight taken over from the staging (dev's partycore, or a party's
 * START): on or off */
void party_play_on(bool on);
/* --dev partycore=N (1-4, one the dev's solo on the same field and bot; partytwin, partyroll, partyend): every guardian's
 * fight on the party's core, the partners scripted; partybot=1 my keys in
 * a party's fight a bot's, as the Arena's arenaboth (the Custom screen's OK,
 * then charged shots); partyroll=W over the server the rollback window,
 * partydelay=N the input delay, partylead=1 the enemies' aim on the lead
 * alone as BN6 has it (every game of the party alike), partyview=N the
 * field drawn as seat N sees it (his marks and HP box: party_marks.h);
 * partydel=MASK with partyend those seats' MegaMen deleted then (a crew's
 * pull-back's test, issue #718); partyhp=N under partycore the stood-in
 * partners' HP going in (a partner's own HP told from his lead's, issue
 * #757); partyscale=1 every guardian scaled for N as a party's door scales
 * him, partyhpper=P and partysteps=S other rules for it (boss_party_dev,
 * tools/party_guardian.py) */
/* (join_wait: joinwait=N, a crew's battle's join said N frames late, a test's cut at the join, dive_join.c; issue #751) */
typedef struct { int n, twin, roll, end, bot, delay, lead, view, del, hp, scale, hp_per, steps, reward, join_wait; } PartyDev;
void party_play_dev(const PartyDev *d);
/* ... its joinwait in ms (0 none) */
unsigned party_play_join_wait_ms(void);
/* The dive's party battle START: its battle id, my seat, the party's
 * size, the input delay and the seats' sessions (each MegaMan's name on
 * the field: party_marks.h); the next guardian's fight its */
void party_play_start(uint32_t battle, unsigned seat, unsigned n, unsigned delay, const uint32_t seats[PARTY_MOST]);
/* ... a crew's random battle its (after party_play_start, at once: the
 * shared dive's join-in battles, dive_join.h), each game making `e` alike
 * (director_crew_battle) and the run's next battle already
 * (director_crew_record); false where it could not be taken over. Whether
 * one is under way; and its end, once, where the field fought it to one:
 * won, the reward BN6's results on the field picked (BN6_BATTLE_REWARD)
 * and every seat's MegaMan as it ended (director_crew.h's CrewOver) */
bool party_play_crew(const Encounter *e);
bool party_play_crew_fighting(void);
bool party_play_crew_end(CrewOver *out);
/* ... a seat of it gone (ARENA_STATE seat_gone) */
void party_play_gone(unsigned seat);
/* ARENA_SEAT for it: a seat's MegaMan, keys or hash; false where it is not */
bool party_play_mail(const uint8_t *m, size_t n);

/* While its fight runs: the run's core held, this its frame and picture */
bool party_play_held(void);
/* ... and still being made: its staging's still picture of the map shows, not yet the field */
bool party_play_staging(void);
/* When its field first showed with BN6's Custom screen closed (SDL ticks), 0 not yet or not fought */
uint32_t party_play_field_since(void);
/* Whether a crew battle's picture may have ours over it now (party_play_draw_more): its staging's still map, or the
 * field with BN6's Custom screen closed, never a partner's own screen (issue #781) */
bool party_play_drawn_over(void);
/* This game's MegaMan's HP and max HP on the party's field while one is fought (false none): his box's, where the run's
 * own core stands still under it */
bool party_play_hp(int *hp, int *max);
void party_play_frame(uint32_t pad);
void party_play_draw(void);
/* Drawn over a crew's battle while it is made (the staging's still picture
 * of the map, never the field: issue #781), set by the dive's band
 * (dive_band.c: whose battle MegaMan was taken into, issue #750); NULL
 * none. */
extern void (*party_play_draw_more)(void);
/* ... in the state's words: the field's size and the seat it is drawn
 * for, its marks (party_marks.h); over the server "party" its stage and
 * frame, the hashes agreed, and this game's own link through a drop (up,
 * down, alone; issue #180) */
void party_play_describe(FILE *f);

#endif
