/* Chaos Unison's Thumb in the Souls' room (issue #82, the Souls track's
 * S6; soulchaos.c installs it at SOULS_CHAOS): the six chaos power attacks
 * (BN5's 0x31-0x3A, each a DarkChip through the chip writer), the
 * requests' ends (BN5's 0x080F043C and 0x080F0470), the chaos attack's
 * dispatch (0x0800F330) and the failure's family (BN5's 0x39). Offsets
 * into it of what soulchaos.c names or fills: */
#ifndef CW_SOULCHAOS_THUMB_H
#define CW_SOULCHAOS_THUMB_H

#include <stdint.h>

#define CHAOS_THUMB_SIZE         0x2F2u
#define CHAOS_POWER_COLONEL      0x000u
#define CHAOS_POWER_SHADOW       0x006u
#define CHAOS_POWER_NUMBER       0x00Cu
#define CHAOS_POWER_TOMAHAWK     0x012u
#define CHAOS_POWER_KNIGHT       0x018u
#define CHAOS_POWER_TOAD         0x01Eu
#define CHAOS_CHIP_COLONEL       0x030u
#define CHAOS_CHIP_SHADOW        0x034u
#define CHAOS_CHIP_NUMBER        0x038u
#define CHAOS_CHIP_TOMAHAWK      0x03Cu
#define CHAOS_CHIP_KNIGHT        0x040u
#define CHAOS_CHIP_TOAD          0x044u
#define CHAOS_CHAOS_HIT          0x070u
#define CHAOS_CHAOS_MISS         0x09Au
#define CHAOS_FAIL_FAMILY        0x0F0u
#define CHAOS_FAIL_0             0x0FEu
#define CHAOS_FAIL_4             0x144u
#define CHAOS_FAIL_8             0x1E0u
#define CHAOS_W_PHASES           0x1ACu
#define CHAOS_FAIL_PHASES        0x1B0u
#define CHAOS_W_OUT              0x1D0u
#define CHAOS_W_PANEL            0x1D4u
#define CHAOS_W_DARK             0x1D8u
#define CHAOS_OUT_STUB           0x2ECu
#define CHAOS_ZERO               0x2EEu

/* its bytes, the words soulchaos.c fills 0 */
void soulchaos_thumb(uint8_t out[CHAOS_THUMB_SIZE]);

#endif
