---
type: Engine Mechanism
title: "Fast forward"
description: "Fast forward's frame batches in `scene_emu.c`: where BN6's state allows it (`fast_forward_game.c`, never the Custom screen or a fight), the 10 ms budget, the music at its own pace on the engine's MP2K and handed back, the effects kept at pitch by crossfaded grains, run time in game frames."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-24T23:07:49Z }
---

# Fast forward

Fast forward ([The player's guide](../../GUIDE.md#fast-forward), Fast forward) plays several of the game's
frames to one shown: `scene_emu.c`'s update runs a batch, each frame with
the update's keys and the run's logic after it, as the dev menu's speed
has always done, and draws the last. Where it may run is read from BN6
before each frame of the batch (`src/emu/fast_forward_game.c`, its rules
in `src/core/fast_forward.c`): the run's own game on the screen, not
online, the Jukebox or the dev menu; `BN6_TOOLKIT`'s mode the game's (not
the PET's screens, a shop, a trader, mail); `BN6_GAMESTATE` not the PET's
menu; and in a battle, `BN6_BATTLE_PHASE` neither the Custom screen's
(`BN6_PHASE_CUSTOM`) nor the fight's (`BN6_PHASE_FIGHT`, the pause and
the time a chip stops with it). A frame that opens the first Custom
screen, the PET or a shop ends the batch, so the next frame is played at
BN6's pace. A trace of seed 1's first battle, frame by frame, shows what
that leaves: `BN6_SUB_BATTLE_INIT` for 16 frames, phases 0x00 and 0x04
for about 125 as the enemies appear, 0x08 from the first Custom screen,
0x0C for the fight, then `BN6_PHASE_RESULTS` from the last deletion (the
result byte 1) for about 155 frames of ENEMY DELETED! and the RESULT
screen, 0x14, and the map. MegaMan's deletion goes from 0x0C to 0x14.
Run on the autopilot, those openings and closes are about five and a
half seconds a battle at the GBA's pace.

A batch keeps to 10 ms of the update (`BUDGET_MS`): another frame is
played only where the batch's time and the last frame's still fit, so a
machine that cannot run 3x runs what it can, and the loop's own pace is
kept (headless, it runs at the speed set). Every frame but the last is
played unshown, mGBA's `frameskipCounter` set so the core draws no
picture for it, as for a frame the loop catches up with. On the core's
own thread (the New 3DS) each frame of the batch waits for the one before
it, so the budget lets fewer through: not measured on a 3DS yet.

**The music** plays at its own pace (`src/audio/music_fast.c`, issue
#839, the suggestion of agergo100): while the game runs fast, its song
is played on the engine's own MP2K ([The engine's own sound](engine-sound.md))
from where the game had it, a frame of the engine's to each update, and
the game's music player runs on in the core unheard (`sound_quiet.c`'s
music away: its tracks' volumes given none, as Options' Music at 0
gives them). As the game runs at BN6's pace again, the song goes back
into the game where the engine has brought it, the same hand-over as
under the engine's screens (`mp2k_carry.c`), and the game plays on
from there. The engine follows what the game's player does meanwhile
(`mp2k_carry_follow`, read once an update): a song begun there (another
header, or its clock from 0 again: a battle's theme, the win's) is begun
here from its start, from the game's ROM copy, so another game's song
in the grown table plays as BN6's do; a fade begun there is begun here
with the game's speed, and runs at the engine's pace; a stop
(m4aMPlayStop, its tracks kept) stops, a continue continues, and the
tempo's factor is taken (the Glitch layer's drag). A song's own end in
the game, or its fade's, is left to the engine's pace: the engine plays
a jingle to its end unless the game begins the next song first. The
engine's sound goes through the core's ring, added to each frame of the
core's heard (`emu_sound_mixer`, `audio_engine_add`), so the music and
the effects keep one timeline: a GBA frame of the engine's to each,
paced to as many samples as the core's frame gave (`gba_sound_pace`:
the core's ring runs at the loop's 60 frames a second, not the GBA's
59.73, and unpaced the engine's song ran 216 samples a second ahead).
The core's DirectSound plays a mix from about line 175 of the GBA's 228,
where BN6's sound interrupt starts its DMA again, 616 of a frame's 803
samples after the frame's edge; the engine's starts it at the edge. So
through the ring the engine's DirectSound is played as late, its first
part the last mix's end, and as it takes the song the game's last mix is
handed to it (BN6's ring's part the DMA's count names), its first part
left to the core, which plays it itself: the two meet on that line. Its
PSG keeps the frame's edge, where the notes change hands both ways: as
the song is taken the game's PSG notes of its music are cut at once on
the game's registers (`mp2k_carry_hush`), which BN6's envelope would
otherwise fade over some frames under the engine's, and as it is given
back the game's registers take them as under the engine's screens. The
engine's PSG so leads its DirectSound by those 616 samples (13 ms) while
fast. After the hand-back the engine is heard two frames more, its last
mix. `settings.ini`'s `fast_music = off` keeps the music in the core,
played faster as below; there is no Options line (the game-design pass of 24 November 2026: a
choice few would make is not worth a line of the screen, and the mark,
▶▶, already says fast forward runs; the effects stay grains, in step
with what is on screen).

Measured on 23-24 November 2026, the sound dumped (`CYBERWORLD_AUDIO_DUMP`,
`CYBERWORLD_AUDIO_OFFLINE`) in Lan's net at depth 2, seed 2, MegaMan
standing, fast forward at 3x from update 60 or 300:

- **The song's length**, its loop found by the sound's envelope against
  itself over 233 s: 40.57 s at 1x, 40.57 s fast forwarded, 13.52 s
  fast forwarded the old way (`fast_music = off`).
- **Its place**: fast forwarded, the sound lines up with the 1x game's
  at the same update within 3 samples (the engine's DirectSound before
  it was delayed: 616 samples early, constant), at the same level
  (0.95-1.07 of it); its waveform apart by the PSG's phase, as the
  engine's is against the game's anywhere ([The engine's own sound](engine-sound.md)).
- **The hand-back**: fast forwarded from update 300 to 1304, the game's
  music player, its eight tracks and its eight channels are then the 1x
  game's at the same update byte for byte; of SoundInfo's first 0x50
  bytes the DMA's count and the PSG's 15-frame count differ, the game's
  own frames' clocks.
- **The switches**: the DirectSound alone (the PSG muted on both sides)
  goes over within 0.3 dB of the 1x game's in 2.5 ms windows both ways;
  the PSG alone has 10-20 ms of the GBA's filter settling after a note
  is cut and another begun, as BN6's own note cuts have. Altogether
  the switches stay within the engine's own spread against the game
  while fast (2.5 ms windows: +6.7 and -6.4 dB at the switches, the
  steady 99th percentile +7.6), and the largest jump between samples
  (the second difference) was 5206 at the switch on and 4276 at the
  switch off, the 1x game's own music reaching 6639 a few frames after.
- **A song begun while fast** (`0:battle`, the encounter's theme from
  the grown table, BN6's area and BN5's Area x0): the theme lines up
  with the 1x game's at the same moment (the envelopes at r 0.65-0.81),
  the old way's nowhere (r 0.34 at most).
- **The game's state**: fast forwarded through the battle's opening,
  the work RAM 1450 updates in is the same with the music on the engine
  as with it in the core, outside the sound engine's own (its tracks,
  players, SoundInfo with its ring, the PSG's channels), but for 8 bytes
  of the stack at `0x03007D60` in BN5's area.
- **Its cost**: the engine's frame took 53 µs on the desktop it was
  measured on (300 s of a song rendered in 0.92 s), against about
  650 µs of one of the game's frames: on the New 3DS, whose core takes
  11 of a frame's 16.7 ms on its own thread, where the engine's frame
  runs, about one more. Not measured on a 3DS or a PSP; where one runs
  short, `fast_music = off` takes it away.

**The sound** keeps its pitch. Only the first frame of a batch is heard
(`EMU_FRAME_UNHEARD` on the rest, `emu.c`): the ring takes one frame's
sound an update, as at the GBA's pace, so it neither starves nor floods,
and the effects play faster at their own pitch, as a granular time
stretch does, each frame's sound a grain (the music too where
`fast_music` is off). The grains are joined by a crossfade:
the first 5 ms of the frame after a heard one, which carries on where it
stopped, faded out under the next heard frame's first 5 ms. On the core's
sound dumped over the same 760 frames of seed 1 with fast forward on, the
largest jump between samples (the second difference) was 16404 with the
grains butted and 9288 crossfaded, and those over 6000 went from 440 to
272; the 272 lie in one burst of a battle's noise, spread evenly over the
frame's 800 samples, not at the joins.

**The run's time** counts the game's frames (`profile_played_frame`, from
`director_update`), not the updates: the statistics' minutes and the
week's DeepNet's tie-break ([Online: the Net Square](../../online/overview.md)) are the game's own time, the
same with fast forward or without, and at the GBA's pace the same as
before.
