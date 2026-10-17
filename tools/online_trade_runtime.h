#ifndef CW_ONLINE_TRADE_RUNTIME_H
#define CW_ONLINE_TRADE_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "online_trade_capture.h"

struct mCore;
extern struct mCore *trade_probe_core;
void trade_probe_directory(const char *path);
FILE *trade_probe_output(const char *name);
bool trade_probe_saved_state(const char *name, bool loading);
unsigned trade_probe_program_counter(void);
unsigned trade_probe_chip_code(unsigned id, unsigned slot);
bool trade_probe_editor(const uint32_t regs[16], int32_t cpsr);
bool trade_probe_start(const char *rom, const char *state);

#endif
