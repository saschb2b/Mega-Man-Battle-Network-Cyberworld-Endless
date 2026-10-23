/* --mp2k-trace SONG:FRAMES:PREFIX[:C15:COUNTER]: the engine's MP2K (src/audio/mp2k.h)
 * playing a song of BN6's from silence, each frame's state written as
 * romlab's `sound` writes the game's (PREFIX.si: SoundInfo with its
 * channels and its mix's ring, the PSG's channels, the music player and its
 * tracks, at BN6's offsets and addresses) and its sound (PREFIX.wav, 48
 * kHz), for tools/mp2k_diff.py to lay beside the game's. */
#ifndef CW_MP2K_TRACE_H
#define CW_MP2K_TRACE_H

int mp2k_trace_run(const char *spec);

#endif
