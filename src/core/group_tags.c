/* groups.h: the families' measured tags, a row each: its net (0 BN6's, 1 BN5's, 2 BN4's), BN6's AI index, its
 * pressure (the HP a standing MegaMan loses in 20 seconds to its V1 alone), its tricks and its cost (the HP the test
 * autopilot loses deleting its V1 alone with the starting folder, 50 seconds at most; -1 not fought), and of two and
 * of three of it together their pressure and cost as measured, each one more than the number (the multiples' pass;
 * 0 not fought).
 * Written by tools/encounter_tags.py engine from tools/encounter_data/ (docs/devtools/encounter-tags.md); do not
 * edit: measure again and write it again (--check says whether this is what it writes). */
#include "groups.h"

const GroupTag group_tags[] = {
	{ 0, 1, 70, 0, 0, { { 61, 1 }, { 71, 1 } } },
	{ 0, 2, 20, 0, 0, { { 101, 1 }, { 121, 51 } } },
	{ 0, 3, 240, 0, 20, { { 241, 191 }, { 241, 311 } } },
	{ 0, 4, 180, GROUP_SHAPES, 0, { { 211, 1 }, { 211, 1 } } },
	{ 0, 5, 100, 0, 0, { { 101, 1 }, { 101, 51 } } },
	{ 0, 6, 120, 0, 0, { { 121, 51 }, { 121, 111 } } },
	{ 0, 7, 210, GROUP_SHAPES, 35, { { 156, 96 }, { 156, 236 } } },
	{ 0, 8, 160, 0, 20, { { 161, 31 }, { 161, 231 } } },
	{ 0, 9, 0, 0, 0, { { 1, 1 }, { 1, 1 } } },
	{ 0, 10, 0, 0, 0, { { 1, 1 }, { 1, 1 } } },
	{ 0, 11, 60, 0, 20, { { 61, 21 }, { 81, 21 } } },
	{ 0, 12, 200, 0, 100, { { 201, 391 }, { 201, 391 } } },
	{ 0, 13, 0, GROUP_TOUGH, 0, { { 1, 1 }, { 1, 1 } } },
	{ 0, 14, 50, 0, 0, { { 71, 21 }, { 81, 51 } } },
	{ 0, 15, 320, 0, 100, { { 341, 291 }, { 341, 411 } } },
	{ 0, 16, 300, GROUP_TOUGH, 390, { { 401, 791 }, { 501, 791 } } },
	{ 0, 17, 90, 0, 0, { { 91, 51 }, { 91, 141 } } },
	{ 0, 18, 120, GROUP_TOUGH, 0, { { 161, 291 }, { 201, 211 } } },
	{ 0, 19, 0, 0, 0, { { 1, 51 }, { 1, 31 } } },
	{ 0, 20, 180, 0, 50, { { 181, 141 }, { 211, 261 } } },
	{ 0, 21, 0, GROUP_SHAPES, 0, { { 1, 1 }, { 1, 61 } } },
	{ 0, 22, 0, 0, 215, { { 1, 191 }, { 1, 246 } } },
	{ 0, 23, 80, 0, 0, { { 81, 1 }, { 81, 1 } } },
	{ 0, 24, 100, 0, 20, { { 121, 71 }, { 141, 131 } } },
	{ 0, 25, 440, GROUP_TOUGH, 550, { { 581, 811 }, { 691, 931 } } },
	{ 0, 26, 180, 0, 60, { { 181, 171 }, { 181, 171 } } },
	{ 0, 27, 200, 0, 30, { { 181, 91 }, { 181, 51 } } },
	{ 0, 28, 150, GROUP_BREAKS, 140, { { 301, 341 }, { 451, 641 } } },
	{ 0, 29, 80, GROUP_TOUGH, 140, { { 0, 0 }, { 0, 0 } } },
	{ 1, 32, 60, 0, 0, { { 61, 1 }, { 61, 1 } } },
	{ 1, 33, 150, 0, 190, { { 151, 291 }, { 151, 291 } } },
	{ 1, 34, 40, GROUP_BREAKS, 0, { { 81, 1 }, { 81, 81 } } },
	{ 1, 35, 150, 0, 60, { { 181, 131 }, { 181, 201 } } },
	{ 1, 36, 540, GROUP_TOUGH, 0, { { 421, 111 }, { 421, 231 } } },
	{ 1, 37, 150, 0, 0, { { 181, 171 }, { 331, 201 } } },
	{ 1, 38, 200, 0, 110, { { 241, 431 }, { 281, 471 } } },
	{ 1, 39, 90, GROUP_TOUGH, 190, { { 91, 231 }, { 91, 201 } } },
	{ 1, 40, 60, 0, 30, { { 121, 51 }, { 81, 131 } } },
	{ 1, 41, 60, 0, 30, { { 121, 111 }, { 1, 171 } } },
	{ 1, 42, 240, GROUP_SHAPES, 30, { { 241, 81 }, { 241, 141 } } },
	{ 1, 43, 150, 0, 60, { { 211, 291 }, { 211, 351 } } },
	{ 1, 44, 100, 0, 20, { { 101, 51 }, { 101, 51 } } },
	{ 1, 45, 160, GROUP_TOUGH, 30, { { 281, 121 }, { 321, 111 } } },
	{ 1, 46, 210, GROUP_TOUGH, 170, { { 271, 201 }, { 271, 231 } } },
	{ 1, 47, 60, 0, 50, { { 61, 51 }, { 101, 131 } } },
	{ 1, 48, 90, GROUP_TOUGH, 80, { { 0, 0 }, { 0, 0 } } },
	{ 1, 49, 90, 0, 0, { { 91, 61 }, { 91, 111 } } },
	{ 1, 50, 90, 0, 30, { { 121, 141 }, { 121, 201 } } },
	{ 1, 51, 60, 0, 0, { { 61, 21 }, { 61, 21 } } },
	{ 1, 52, 60, GROUP_SUMMONS, 0, { { 121, 71 }, { 181, 91 } } },
	{ 1, 53, 30, GROUP_BREAKS | GROUP_TOUGH, 110, { { 91, 61 }, { 121, 41 } } },
	{ 1, 54, 120, GROUP_TOUGH, 80, { { 181, 81 }, { 121, 81 } } },
	{ 1, 55, 300, GROUP_TOUGH, 110, { { 421, 141 }, { 481, 171 } } },
	{ 1, 56, 150, GROUP_BREAKS | GROUP_TOUGH, 190, { { 301, 491 }, { 451, 741 } } },
	{ 2, 64, 30, 0, 15, { { 61, 51 }, { 91, 121 } } },
	{ 2, 65, 90, 0, 30, { { 91, 31 }, { 151, 111 } } },
	{ 2, 66, 90, 0, 50, { { 61, 141 }, { 121, 81 } } },
	{ 2, 67, 140, 0, 0, { { 141, 51 }, { 141, 231 } } },
	{ 2, 68, 210, 0, 0, { { 211, 91 }, { 151, 51 } } },
	{ 2, 69, 210, GROUP_TOUGH, 80, { { 211, 291 }, { 271, 291 } } },
	{ 2, 70, 80, 0, 0, { { 0, 0 }, { 0, 0 } } },
	{ 2, 71, 45, 0, 15, { { 0, 0 }, { 0, 0 } } },
	{ 2, 72, 80, GROUP_TOUGH, 70, { { 121, 101 }, { 121, 91 } } },
	{ 2, 73, 120, 0, 70, { { 211, 351 }, { 241, 361 } } },
	{ 2, 74, 120, 0, 0, { { 91, 1 }, { 151, 31 } } },
};
const int group_tags_count = (int)(sizeof group_tags / sizeof *group_tags);

/* The pairs that press harder together than their own pressures add up to (the pairs' pass: tools/encounter_danger.py
 * pairs), 40 HP in 20 seconds or more at V1, or cost the test autopilot as much more than their own costs
 * add up to (pairs --modes fight): each family's net and BN6's AI index, the pressure's excess and the cost's. */
const GroupPair group_pairs[] = {
	{ 0, 1, 0, 21, 60, 0 },
	{ 0, 2, 0, 12, 40, 0 },
	{ 0, 2, 0, 21, 120, 0 },
	{ 0, 5, 0, 21, 120, 0 },
	{ 0, 6, 0, 21, 40, 0 },
	{ 0, 12, 0, 21, 80, 0 },
	{ 0, 15, 0, 18, 60, 0 },
	{ 0, 16, 0, 18, 40, 0 },
	{ 0, 16, 0, 21, 40, 0 },
	{ 0, 17, 0, 21, 120, 0 },
	{ 0, 18, 0, 25, 100, 0 },
	{ 0, 19, 0, 20, 60, 0 },
	{ 0, 19, 0, 21, 60, 0 },
	{ 0, 19, 0, 25, 90, 0 },
	{ 0, 20, 0, 21, 60, 0 },
	{ 0, 21, 0, 28, 80, 0 },
	{ 1, 34, 0, 2, 40, 0 },
	{ 1, 35, 0, 9, 60, 0 },
	{ 1, 35, 0, 13, 60, 0 },
	{ 1, 35, 0, 22, 60, 0 },
	{ 1, 36, 0, 5, 60, 0 },
	{ 1, 38, 0, 15, 60, 0 },
	{ 1, 43, 0, 19, 90, 0 },
	{ 1, 49, 0, 15, 70, 0 },
	{ 1, 49, 0, 25, 90, 0 },
	{ 1, 53, 0, 21, 60, 0 },
	{ 1, 54, 0, 18, 40, 0 },
};
const int group_pairs_count = (int)(sizeof group_pairs / sizeof *group_pairs);

/* BN6's battlefields a group may stand on, a bit each by the field's number (the fields' pass: a pair harmless on the
 * plain field fought on each, fair where the test autopilot wins every fight and loses 20 HP at most). */
const uint8_t group_fair_fields[32] = {
	0x6F, 0x01, 0xFD, 0x1F, 0x43, 0x6A, 0xF0, 0xFF, 0xAE, 0x3F, 0x0B, 0x0E, 0x01, 0x08, 0x22, 0xE1, 0x0F, 0x00, 0x04, 0x1E, 0xC5, 0x88, 0xCC, 0x60, 0x00, 0x3E, 0x10, 0xA2, 0xD2, 0x0B, 0x00, 0x00,
};

/* The caps by act on a group's pressure and on its cost: the three-quarter marks of the acts' areas' own battles as
 * these tags predict them (acts 1 to 6, the Nest). */
const int16_t group_caps[2][PACING_ACTS] = {
	{ 180, 320, 530, 620, 880, 1040, 935 },
	{ 20, 60, 175, 140, 180, 678, 447 },
};
