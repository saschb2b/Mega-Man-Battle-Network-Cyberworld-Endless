/* Staging over the running game (docs/BOSSES.md): letterbox bars, title
 * and area cards, flashes and shakes drawn over the game's frame, and how
 * much of the player's input reaches MegaMan. */
#ifndef CW_CINEMA_H
#define CW_CINEMA_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

enum {
	CINEMA_FREE,   /* the player plays */
	CINEMA_TALK,   /* only A and B, to read a conversation */
	CINEMA_HOLD,   /* MegaMan holds still */
	CINEMA_WALK,   /* MegaMan walks where the staging leads him (cinema_walk) */
};

void cinema_reset(void);
void cinema_update(void);
/* Draws over the game's frame; (*dx, *dy) is where the frame itself goes
 * this frame, shaken. */
void cinema_offset(int *dx, int *dy);
void cinema_draw(void);
/* The GBA keys that reach the game. */
uint32_t cinema_keys(uint32_t keys);
/* Off the map (a battle, a menu) the player always plays. */
void cinema_on_map(bool on_map);

void cinema_input(int mode);
int cinema_input_mode(void);
/* The keys MegaMan walks by this frame, in CINEMA_WALK. */
void cinema_walk(uint32_t keys);
void cinema_letterbox(bool on);
void cinema_flash(int frames);
void cinema_shake(int frames, int amplitude);
/* An arrow pointing the way on for `frames` (screen direction 0 right,
 * then clockwise in eighths). */
void cinema_arrow(int dir, int frames);
/* The arrow shown on for at least `frames` more (none shown: nothing). */
void cinema_arrow_extend(int frames);
/* Whether the arrow shows, and a new way for it as it shows. */
bool cinema_arrow_on(void);
void cinema_arrow_turn(int dir);
/* The way the arrow shows (screen eighths). */
int cinema_arrow_dir(void);
/* How long the arrow has shown, in frames. */
int cinema_arrow_age(void);
/* A guardian's title card: `top` over its name, large, and `epithet`. */
void cinema_title(const char *top, const char *name, const char *epithet, SDL_Color accent, int frames);
/* A card between areas: `small` over `big`, then up to two lines. */
void cinema_card(const char *small, const char *big, const char *line1, const char *line2, SDL_Color accent, int frames);
/* A title or card is showing. */
bool cinema_busy(void);
/* An area's card fades out at once, where a chat has opened under it (an A
 * at the gift Prog beside the arrival drew its box under the card). */
void cinema_card_yield(void);

#endif
