/* Made-up profile for the no-run visit proof. No ROM or player files. */
#include <stdio.h>
#include <stdlib.h>

#include "meta.h"
#include "save.h"
#include "save_format.h"
#include "save_layout.h"

int main(int argc, char **argv) {
	if (argc != 8) return 2;
	Profile sample = { .music_volume = 9, .sfx_volume = 9, .seen_intro = true,
		.last_folder = (uint8_t)atoi(argv[2]), .last_cross = (uint8_t)atoi(argv[3]),
		.last_helpers = (uint8_t)atoi(argv[4]), .folders_open = (uint16_t)atoi(argv[5]),
		.crosses_open = (uint8_t)atoi(argv[6]), .marks = (uint16_t)atoi(argv[7]) };
	const uint8_t *bytes = (const uint8_t *)&sample;
	uint32_t hash = 2166136261u;
	for (size_t i = 0; i < sizeof sample; ++i) hash = (hash ^ bytes[i]) * 16777619u;
	uint32_t header[3] = { PROFILE_MAGIC, sizeof sample, hash };
	FILE *file = fopen(argv[1], "wb");
	if (!file) return 1;
	bool ok = fwrite(header, sizeof header, 1, file) == 1 && fwrite(&sample, sizeof sample, 1, file) == 1;
	if (fclose(file)) ok = false;
	/* Expected project-authored folder, independently read before the
	 * game boots and chooses whether this profile may bring it. */
	const uint16_t *folder = meta_folder_chips(sample.last_folder);
	fputs("[", stdout);
	for (unsigned i = 0; folder && i < 30; ++i) printf("%s%u", i ? "," : "", folder[i]);
	puts("]");
	return ok ? 0 : 1;
}
