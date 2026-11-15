/* The shared dive's crews (issue #589; docs/online/shared-dive.md): up to
 * four divers who team up on a layer and stay together across layers. A on
 * a diver's MegaMan asks him (BN6's own Yes/No, MegaMan's words as Team
 * NetBattles' are), an ask that comes in is answered the same way, and the
 * band says who teamed up or split; on a crewmate, A asks to split up. */
#ifndef CW_DIVE_CREW_H
#define CW_DIVE_CREW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_types.h"

void dive_crew_reset(void);
/* Each frame of the dive on its layer: a chat to open, or one closed read. */
void dive_crew_frame(const OnlineSnapshot *s);
/* The pad's keys: A on a diver's MegaMan. */
uint32_t dive_crew_keys(uint32_t keys);
/* CREW_STATE. */
void dive_crew_mail(const uint8_t *m, size_t n);
/* The crew at Lan's HP (home_way_hold): portal k taken holds while some
 * crewmate stands on no other (-1: stepped off); whether MegaMan waits. */
bool dive_crew_way_hold(int portal);
bool dive_crew_waiting(void);
/* A line for the dive's band, taken once (NULL none). */
const char *dive_crew_news(void);

#endif
