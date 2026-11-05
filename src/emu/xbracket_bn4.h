/* BN4 Red Sun's bracket screen's recipe (xbracket_bn4.c, written by
 * tools/xrecipe_bracket.py; xbracket.h installs it). */
#ifndef CW_XBRACKET_BN4_H
#define CW_XBRACKET_BN4_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* A read of eToolkit +0x3C (mov rN,r10; ldr rN,[rN,#0x3C]), BN4's
 * tournaments where BN6 keeps its game state: its address in BN4, its
 * register */
typedef struct { uint32_t at; uint8_t reg; } XbrSite;

/* A call the entry makes: its r0, the routine (BN6's, Thumb; or BN4's moved with the screen, `own`) */
typedef struct { uint32_t arg, fn; bool own; } XbrCall;

extern const XVirusFamily xbracket_bn4;
extern const XbrCall xbracket_leave[];
extern const int xbracket_leave_count;
extern const XbrSite xbracket_sites[];
extern const int xbracket_site_count;

#endif
