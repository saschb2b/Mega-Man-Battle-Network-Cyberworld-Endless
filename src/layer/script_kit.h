/* The parts the layer objects' chats are built of (scripts.c), for the files that build them. */
#ifndef CW_SCRIPT_KIT_H
#define CW_SCRIPT_KIT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "scripts.h"

void ta_ask_in(TextArchive *t, int face, const char *question, int no, bool after, bool risky);
void ta_ask_costly(TextArchive *t, int face, const char *question, int no);
void ta_ask(TextArchive *t, int face, const char *question, int no);
void ta_flag_set(TextArchive *t, int flag);
/* ts_check_flag: on to script `to` while `flag` is set */
void ta_if_flag(TextArchive *t, int flag, int to);
/* A choice's later talk once its words are said (ScriptsChoice): on to
 * script `later` where `c.told` is set (none where it is -1) */
void ta_if_told(TextArchive *t, ScriptsChoice c, int later);
/* ... and its words said, as its first talk reaches the question */
void ta_told(TextArchive *t, ScriptsChoice c);
int ta_closing(TextArchive *t);
extern const uint8_t ta_full_hp[4];   /* ts_call_set_full_h_p */
void ta_give_chip(TextArchive *t, int id, int code, int count);
void ta_give_hp_memory(TextArchive *t, int count);
void ta_give_zenny(TextArchive *t, int zenny);
void ta_take_zenny(TextArchive *t, int zenny, int paid, int broke);
void ta_got(TextArchive *t, const char *thing, bool *first);
void ta_got_hp(TextArchive *t, int count, bool *first);
void ta_got_chip(TextArchive *t, const char *chip, int code, bool *first);
void ta_jump(TextArchive *t, int to);
void ta_options(TextArchive *t, const char *const *labels, int n);
void ta_choose(TextArchive *t, const int *scripts, int n, int b);
void ta_end_or(TextArchive *t, int next);
void ta_program_name(TextArchive *t, int program);
void ta_flag_clear(TextArchive *t, int flag);

#endif
