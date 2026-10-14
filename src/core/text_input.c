#include "text_input.h"

#include <string.h>

static char *text;
static size_t capacity;
static int result;
static bool selected;

void text_input_begin(char *buffer, size_t size) {
	text = buffer;
	capacity = size;
	result = TEXT_INPUT_NONE;
	selected = false;
	SDL_StartTextInput();
}

void text_input_end(void) {
	SDL_StopTextInput();
	text = NULL;
	capacity = 0;
	result = TEXT_INPUT_NONE;
}

void text_input_add(const char *input) {
	if (!text || !capacity) return;
	if (selected) text[0] = 0;
	selected = false;
	size_t n = strlen(text);
	for (const unsigned char *p = (const unsigned char *)input; *p && n + 1 < capacity; ++p)
		if (*p >= 32 && *p < 127) text[n++] = (char)*p;
	text[n] = 0;
}

void text_input_erase(void) {
	if (!text) return;
	size_t n = strlen(text);
	if (selected) text[0] = 0;
	else if (n) text[n - 1] = 0;
	selected = false;
}

int text_input_result(void) {
	int got = result;
	result = TEXT_INPUT_NONE;
	return got;
}

bool text_input_event(const SDL_Event *event) {
	if (!text) return false;
	if (event->type == SDL_TEXTINPUT) { text_input_add(event->text.text); return true; }
	if (event->type == SDL_KEYUP) return event->key.keysym.scancode != SDL_SCANCODE_F11;
	if (event->type != SDL_KEYDOWN) return false;
	SDL_Scancode key = event->key.keysym.scancode;
	if (key == SDL_SCANCODE_F11) return false;
	if (key == SDL_SCANCODE_BACKSPACE || key == SDL_SCANCODE_DELETE) text_input_erase();
	else if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER) result = TEXT_INPUT_DONE;
	else if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_AC_BACK) result = TEXT_INPUT_CANCEL;
	else if ((event->key.keysym.mod & KMOD_CTRL) && key == SDL_SCANCODE_A) selected = true;
	else if ((event->key.keysym.mod & KMOD_CTRL) && key == SDL_SCANCODE_V) {
		char *clipboard = SDL_GetClipboardText();
		if (clipboard) { text_input_add(clipboard); SDL_free(clipboard); }
	}
	return true;
}
