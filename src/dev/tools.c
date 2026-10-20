/* tools.h: --atlas (atlas.c), --pacing (pacing_report.c), --economy
 * (economy_report.c), --render-song (a song into a WAV file), --sheet (sheets.c), --xdiff (xdiff.c) and
 * --xrecords (xrecords_report.c), each run in the game's place once the
 * ROM is read. */
#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atlas.h"
#include "audio.h"
#include "economy_report.h"
#include "net_layouts.h"
#include "pacing_report.h"
#include "sheets.h"
#include "xdiff.h"
#include "xrecords_report.h"

static const char *atlas_spec, *pacing_spec, *economy_spec, *render_spec, *sheet_spec, *xdiff_spec, *xrecords_spec;

bool tools_option(const char *a, const char *v) {
	if (!strcmp(a, "--render-song")) render_spec = v;
	else if (!strcmp(a, "--sheet")) sheet_spec = v;
	else if (!strcmp(a, "--atlas")) atlas_spec = v;
	else if (!strcmp(a, "--pacing")) pacing_spec = v;
	else if (!strcmp(a, "--economy")) economy_spec = v;
	else if (!strcmp(a, "--xdiff")) xdiff_spec = v;
	else if (!strcmp(a, "--xrecords")) xrecords_spec = v;
	else if (!strcmp(a, "--net-layout")) layout_forced = atoi(v);
	else return false;
	return true;
}

int tools_run(void) {
	if (atlas_spec) return atlas_run(atlas_spec);
	if (pacing_spec) return pacing_report_run(pacing_spec);
	if (economy_spec) return economy_report_run(economy_spec);
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
	if (xrecords_spec) return xrecords_report_run(xrecords_spec);
	return -1;
}
