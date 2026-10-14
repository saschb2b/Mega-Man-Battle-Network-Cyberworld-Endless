#ifndef CW_ONLINE_LINK_SCRIPT_H
#define CW_ONLINE_LINK_SCRIPT_H

#include <stdbool.h>
#include <stdint.h>

struct mCore;
typedef struct {
	unsigned frames;
	uint32_t keys[2];
} OnlineLinkInput;

const OnlineLinkInput *online_link_entry_steps(unsigned *count);
const OnlineLinkInput *online_link_battle_steps(unsigned *count);
unsigned online_link_battle_frames(void);
unsigned online_link_entry_frames(void);
unsigned online_link_full_frames(void);
void online_link_battle_input(struct mCore *core, unsigned side, unsigned frame, void *context);
void online_link_full_input(struct mCore *core, unsigned side, unsigned frame, void *context);
void online_link_battle_sample(struct mCore *core, unsigned side, uint32_t out[8], void *context);
int online_link_battle_agrees(const uint32_t left[8], const uint32_t right[8]);
bool online_link_battle_active(const uint32_t left[8], const uint32_t right[8]);

#endif
