/* The parts the layer objects' chats are built of (scripts.c), for the files that build them. */
#ifndef CW_SCRIPT_KIT_H
#define CW_SCRIPT_KIT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "scripts.h"

void ta_ask_in(TextArchive *t, int face, const char *question, int no, bool after, bool risky);
void ta_ask(TextArchive *t, int face, const char *question, int no);
void ta_flag_set(TextArchive *t, int flag);
int ta_closing(TextArchive *t);
extern const uint8_t ta_full_hp[4];   /* ts_call_set_full_h_p */
void ta_give_chip(TextArchive *t, int id, int code, int count);
void ta_give_hp_memory(TextArchive *t, int count);
void ta_got(TextArchive *t, const char *thing, bool *first);
void ta_got_hp(TextArchive *t, int count, bool *first);
void ta_got_chip(TextArchive *t, const char *chip, int code, bool *first);
void ta_jump(TextArchive *t, int to);
void ta_choose(TextArchive *t, const int *scripts, int n, int b);
void ta_end_or(TextArchive *t, int next);
void chip_desc_line(char *out, size_t n, const char *name, int code, const char *desc);
void ta_program_name(TextArchive *t, int program);
void ta_flag_clear(TextArchive *t, int flag);

#endif
