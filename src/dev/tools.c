/* tools.h: --atlas (atlas.c), --pacing (pacing_report.c), --render-song
 * (a song into a WAV file), --sheet (sheets.c) and --xdiff (xdiff.c), each
 * run in the game's place once the ROM is read. */
#include "tools.h"

#include <stdio.h>
#include <string.h>

#include "atlas.h"
#include "audio.h"
#include "pacing_report.h"
#include "sheets.h"
#include "xdiff.h"

static const char *atlas_spec, *pacing_spec, *render_spec, *sheet_spec, *xdiff_spec;

bool tools_option(const char *a, const char *v) {
	if (!strcmp(a, "--render-song")) render_spec = v;
	else if (!strcmp(a, "--sheet")) sheet_spec = v;
	else if (!strcmp(a, "--atlas")) atlas_spec = v;
	else if (!strcmp(a, "--pacing")) pacing_spec = v;
	else if (!strcmp(a, "--xdiff")) xdiff_spec = v;
	else return false;
	return true;
}

int tools_run(void) {
	if (atlas_spec) return atlas_run(atlas_spec);
	if (pacing_spec) return pacing_report_run(pacing_spec);
	if (render_spec) {
		int song = 0, secs = 20;
		char out[256] = "song.wav";
		sscanf(render_spec, "%i:%d:%255s", &song, &secs, out);
		bool ok = audio_render_wav(song, secs, out);
		printf("rendered song %d (%ds) to %s: %s\n", song, secs, out, ok ? "ok" : "failed");
		return ok ? 0 : 1;
	}
	if (sheet_spec) return sheet_run(sheet_spec);
	if (xdiff_spec) return xdiff_run(xdiff_spec);
	return -1;
}
