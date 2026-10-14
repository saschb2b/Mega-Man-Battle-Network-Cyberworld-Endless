/* An editor's native keyboard, handled only by platform_poll. */
#ifndef CW_TEXT_INPUT_H
#define CW_TEXT_INPUT_H

#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>

enum { TEXT_INPUT_NONE, TEXT_INPUT_DONE, TEXT_INPUT_CANCEL };
void text_input_begin(char *buffer, size_t size);
void text_input_end(void);
bool text_input_event(const SDL_Event *event);
int text_input_result(void);
void text_input_add(const char *text);
void text_input_erase(void);

#endif
