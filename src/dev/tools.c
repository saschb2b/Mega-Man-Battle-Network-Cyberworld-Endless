/* tools.h: --atlas (atlas.c), --pacing (pacing_report.c), --economy
 * (economy_report.c), --render-song (a song into a WAV file), --sheet (sheets.c), --xdiff (xdiff.c),
 * --xrecords (xrecords_report.c), --mp2k-trace (mp2k_trace.c), --xsong-pack (another game's song as
 * xsong.c copies it), --pick-fit (pick_fit.c) and --encounter-tags (encounter_tags.c), each run in the game's place once the
 * ROM is read. */
#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atlas.h"
#include "audio.h"
#include "economy.h"
#include "economy_report.h"
#include "encounter_tags.h"
#include "free_space.h"
#include "mp2k_trace.h"
#include "net_layouts.h"
#include "pacing_report.h"
#include "pick_fit.h"
#include "rom.h"
#include "sheets.h"
#include "xdiff.h"
#include "xrecords_report.h"
#include "xsong.h"

static const char *atlas_spec, *pacing_spec, *economy_spec, *render_spec, *sheet_spec, *xdiff_spec, *xrecords_spec, *mp2k_spec,
	*pack_spec, *fit_spec, *tags_spec;

bool tools_option(const char *a, const char *v) {
	/* (another game's song, xN/: the games read for research alone read too, BN4's) */
	if ((!strcmp(a, "--render-song") || !strcmp(a, "--xsong-pack")) && v[0] == 'x') xrom_research = true;
	if (!strcmp(a, "--render-song")) render_spec = v;
	else if (!strcmp(a, "--xsong-pack")) pack_spec = v;
	else if (!strcmp(a, "--sheet")) sheet_spec = v;
	else if (!strcmp(a, "--atlas")) atlas_spec = v;
	else if (!strcmp(a, "--pacing")) pacing_spec = v;
	else if (!strcmp(a, "--economy")) economy_spec = v;
	else if (!strcmp(a, "--xdiff")) xdiff_spec = v;
	else if (!strcmp(a, "--xrecords")) xrecords_spec = v;
	else if (!strcmp(a, "--mp2k-trace")) mp2k_spec = v;
	else if (!strcmp(a, "--pick-fit")) fit_spec = v;
	else if (!strcmp(a, "--encounter-tags")) tags_spec = v;
	else if (!strcmp(a, "--net-layout")) layout_forced = atoi(v);
	/* (a game under the flat plan, the rolls before issue #154, beside
	 * the opening one in play: docs/game/economy/) */
	else if (!strcmp(a, "--economy-plan")) econ_plan = !strcmp(v, "flat") ? &econ_flat : &econ_opening;
	else return false;
	return true;
}

/* --xsong-pack xN/SONG:PATH: extra ROM N's song as xsong.c copies it into
 * BN6, laid out from FREE_SONGS, its bytes into PATH and its header's
 * address printed: what tools/mp2k_verify.py plays in BN6 beside the song
 * in its own game (docs/devtools/sound-check.md) */
static int pack_run(const char *spec) {
	int rom = -1, song = -1;
	char out[256] = "";
	if (sscanf(spec, "x%d/%i:%255s", &rom, &song, out) != 3) {
		fprintf(stderr, "xsong-pack: xN/SONG:PATH\n");
		return 1;
	}
	/* (the songs' free space, 640 KB, from the heap for the tool alone) */
	const uint32_t size = FREE_SONGS_END - FREE_SONGS;
	uint8_t *room = malloc(size);
	if (!room) return 1;
	uint32_t header = xsong_pack(rom, song, FREE_SONGS, room, size);
	/* (its sequence and header last: the header's tracks' pointers end it) */
	uint32_t len = header ? header - FREE_SONGS + 8 + 4u * room[header - FREE_SONGS] : 0;
	FILE *f = header ? fopen(out, "wb") : NULL;
	bool ok = f && fwrite(room, 1, len, f) == len;
	if (f) fclose(f);
	free(room);
	if (!ok) {
		fprintf(stderr, "xsong-pack: song %#x of extra ROM %d not copied\n", song, rom);
		return 1;
	}
	printf("xsong-pack: song %#x of extra ROM %d, header %08x, %u bytes from %08x into %s\n", song, rom, (unsigned)header, (unsigned)len,
		(unsigned)FREE_SONGS, out);
	return 0;
}

int tools_run(void) {
	if (atlas_spec) return atlas_run(atlas_spec);
	if (pacing_spec) return pacing_report_run(pacing_spec);
	if (economy_spec) return economy_report_run(economy_spec);
	if (render_spec) {
		int song = 0, secs = 20, rom = 0;
		char out[256] = "song.wav";
		/* (xN/ first: extra ROM N's song, docs/multirom/) */
		const char *spec = render_spec;
		if (spec[0] == 'x' && sscanf(spec, "x%d/", &rom) == 1) { spec = strchr(spec, '/') + 1; ++rom; }
		sscanf(spec, "%i:%d:%255s", &song, &secs, out);
		bool ok = audio_render_wav(rom, song, secs, out);
		printf("rendered song %d (%ds) to %s: %s\n", song, secs, out, ok ? "ok" : "failed");
		return ok ? 0 : 1;
	}
	if (sheet_spec) return sheet_run(sheet_spec);
	if (xdiff_spec) return xdiff_run(xdiff_spec);
	if (xrecords_spec) return xrecords_report_run(xrecords_spec);
	if (mp2k_spec) return mp2k_trace_run(mp2k_spec);
	if (pack_spec) return pack_run(pack_spec);
	if (fit_spec) return pick_fit_run(fit_spec);
	if (tags_spec) return encounter_tags_run(tags_spec);
	return -1;
}
