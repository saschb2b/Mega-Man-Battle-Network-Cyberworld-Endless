#ifndef CW_ONLINE_FRAME_PAIR_H
#define CW_ONLINE_FRAME_PAIR_H

#include <stdbool.h>
#include <stdint.h>

typedef struct OnlineFramePair OnlineFramePair;
#define ONLINE_FRAME_LOOKAHEAD 4

OnlineFramePair *online_frame_pair_open(const char *rom, const char *state, const char *prefix);
void online_frame_pair_close(OnlineFramePair *pair);
bool online_frame_pair_step(OnlineFramePair *pair, const uint16_t keys[ONLINE_FRAME_LOOKAHEAD][2]);
bool online_frame_pair_sample(OnlineFramePair *pair, uint32_t samples[2][8], uint64_t *hash);
bool online_frame_pair_shot(OnlineFramePair *pair, const char *prefix, unsigned frame);
unsigned online_frame_pair_transfers(const OnlineFramePair *pair);
unsigned online_frame_script_key(unsigned frame, unsigned side);

#endif
