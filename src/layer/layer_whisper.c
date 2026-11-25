/* The layer's second bystander's whisper (layer_objs.h): a rumor of the
 * net's (rumor_lines.c), and a line heard that the director keeps as a
 * choice's flag: the act's guardian named, a starting folder's milestone
 * told (docs/game/meta/shelf.md). */
#include "layer_objs.h"

#include <stdio.h>

#include "debug.h"
#include "flags.h"
#include "rumor_lines.h"

int layer_heard_flag(LayerObjs *out, int type) {
	int flag = LAYER_FLAG_BASE + out->nchoices;
	out->choice[out->nchoices].type = type;
	out->choice[out->nchoices++].flag = flag;
	flag_clear(flag);
	return flag;
}

int layer_whisper_talk(TextArchive *text, LayerObjs *out, int who) {
	int folder = 0;
	const char *whisper = rumors_line(&folder);
	if (!whisper) return -1;
	if (emu_debug_on()) fprintf(stderr, "rumor: %s\n", whisper);
	out->folder_told = folder && out->nchoices < LAYER_MAX_CHOICES ? folder : 0;
	out->whisper = out->folder_told ? ta_say_flag(text, who, whisper, layer_heard_flag(out, CHOICE_WHISPER)) : ta_say(text, who, whisper);
	return out->whisper;
}
