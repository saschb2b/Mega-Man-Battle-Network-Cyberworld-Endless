/* stopwatch.h: milliseconds by SDL's performance counter. */
#include "stopwatch.h"

#include <SDL.h>

double stopwatch_ms(void) { return (double)SDL_GetPerformanceCounter() * 1000.0 / (double)SDL_GetPerformanceFrequency(); }
