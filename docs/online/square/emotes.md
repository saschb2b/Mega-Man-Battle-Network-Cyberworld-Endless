---
type: Feature
title: "Emotes"
description: "Emotes in the Square (issue #176, protocol v14): twelve emotes in BN6's own poses and sounds (`square_emote.c`), favourites on L and a direction, the SELECT strip and the palette (issue #178), aimed waves, the beat."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-22T20:25:41Z }
---

# Emotes

Issue #176, the owner, 22 October 2026: the Square's emotes were
captions ("o/", "^_^" over a head), "really barebone, not on the level
of modern online games". The research (modern games' social expression;
what BN6 has to show a mood, `src/emu/bn6_expression.h` and
[ROM data](../../rom/overview.md)'s Expression rows) and the game-design skill's lens:

- **Experience**: being seen and being answered among up to eight
  players. **Loop**: the Square's session (arrive, meet, do one thing
  together, go home); nothing in it makes a run stronger.
- **One verb deepened**: the emote, and the wave first of all: performed
  by the body, one press away, aimed at someone, answered, joined.
- **Telegraph honestly**: an emote is MegaMan's own pose from BN6's
  sprite and a sound of BN6's, the same on every screen; a caption only
  where a pose cannot carry the meaning, in BN6's chat box's look. BN6
  has no emotion-bubble set (no "?", heart, note or sweat over a head:
  [Online: the Square, the Arena and the link battle](../../rom/online.md), Expression), so none of ours is drawn in its place.
- **Frequency budget**: a burst of three, then one a second, on the game
  and the server alike (Monster Hunter Rise's limit, too tight at first,
  was loosened; the research's pitfall); the Square's answer once a size
  in its ten-second rest; a sound a frame at most, only from a MegaMan in
  view, one's own, or one aimed at one.
- **Subtract**: no wheel; the four favourites on L and a direction,
  the rest on SELECT's strip (One press, below; issue #178 replaced L's
  pages of two). **Positive by design**: only
  the warm emotes (Wave, Cheer, Bow) can be aimed, so no Laugh lands on a
  player who just lost; an aimed emote repeated past its cap is shown to
  its sender alone; the quiet hides everyone else's. An emote never
  moves a MegaMan (AGENTS.md: he walks as BN6 walks him).

**The poses** (`square_emote.c`, the mapping proposed for the owner to
adjust; contact sheets of each one's frames in the issue): a peer's is
his puppet's animation byte (online_square.c's `SCRIPT_ANIM`, over his
walk for the emote's frames), MegaMan's own BN6's text commands, as
BN6's item-get plays his 0x18: `F8 00` (hold him), `F8 01 n` (the pose),
`FD 00` (the sound), `EE 00` (a wait), `F8 04` and `F8 02` (his own
animation back, let go), run by the chat box's engine without opening a
box; the keys are held from BN6 meanwhile. The script is started by a
game call that goes on into the overworld's update in the same frame
(`game_call_then_update`): a plain game call takes that frame's update,
and with it every sprite BN6 draws, so each emote of one's own once
began with a frame of no MegaMan, no Square life and no HP box
(issue #187; [The game's structures, boot and events](../../rom/game-state.md), Boot and warps).

| Emote | Pose (BN6_OW_MEGAMAN's) | Sound (BN6's) | Over his head |
| --- | --- | --- | --- |
| Wave (aimed) | 0x18, a hand raised (the item-get pose) | 0x177, "*ding, dong*" | |
| Cheer (aimed) | 0x40, both arms up | 0x19A, "*pop,pop,pop!!*" | |
| Laugh | 0x39, hunched, shaking | 0x65, a letter typed | "Haha!" |
| Surprised | 0x2E + his way of four, a step back | 0xCD, "*BEEP! BEEP!*" | BN6's "!!" burst |
| Thinking | 0x44, crouched, looking about | 0x7F, a menu's cursor | "Hmm..." |
| Bow (aimed) | 0x41, on one knee | 0x14F, "*ding,dong, ding,doooong*" | |
| Got it! | 0x18, the item-get pose | 0x73, the item-get jingle | |
| Buster | 0x33 + his way of four, the buster held out | 0x71, the buster charging | |
| Phew... | 0x42, panting on one knee | 0x8E (assumed soft) | |
| I'm beat | 0x3F, standing, kneeling, down | 0x14B, "Waaa!!", "*thud*" | |
| Spin | his eight facings in turn, 6 frames each | 0x77, a log-in | |
| Zapped | 0x38, sparks beside him | 0x6B, a hit | |

Seen in BN6's own drawing: some frames of 0x40, 0x39 and 0x42 are drawn
in the log-in's light palette (a cyan flash), on a peer and on MegaMan
himself alike; 0x2D (the arrival's turn) was tried for Spin and dropped,
its first frames blank. The leap (0x43, one frame) waits on the NPC
hop (`22 11 60 DC 21`), not yet run on a puppet.

**One press**: L held and a direction sends a favourite: up Wave, right
Cheer, down Bow, left Laugh by default (`network.ini`'s
`favourites=0,1,5,2`: items, 0-11 the emotes, 12-19 the phrases, as the
strip keeps them); MegaMan stands while L is down, and L let go without a
direction opens L's chat: Look around and **Quiet please** (Let them talk
to undo it, kept in `network.ini`'s `quiet`): everyone else's phrases,
poses, sounds, captions and aims hidden; their names stay, and
per-player Mute stays on the card. The first time L's chat opens,
MegaMan names SELECT ("Lan,press SELECT and I'll strike a pose or say
hi!", `network.ini`'s `hinted` after): BN6 names a button once, at its
first use, through whoever needs it ([Voice](../../voice/rules.md)).

**The emote strip** (issue #178, the owner, 22 October 2026: "the emotes
are invoked via the text bubble still, which seems tedious"; the research
in `.build/research/social/invoke.md`, its Design C and D chosen). L's
pages cost 5 to 16 presses an emote (10.5 on average) through 3 to 8
chat boxes typed out first, a phrase 4 to 11 (7.5), and the one fast
path, the chord, had no signifier, could not be changed in the game and
was two fingers of one hand on a phone. The lens (the game-design skill,
interface): usability, not engageability (the emotes were worth sending,
they cost too much); the frequency budget upside down; the set-once
favourites out of reach; the chord a marking menu with its menu missing.
SELECT, free in the Square, now opens a PET screen of the project's own
(`emote_strip.c`, drawn in `emote_strip_draw.c`, its PET text in
`emote_strip_text.c`), laid out as the Custom screen lays out its chips
(BN6 has no screen for emotes; its Custom screen's art is not copied):

- **The tray** over the bottom of the picture: the twelve emotes as icons
  in two rows of six, each MegaMan's own pose (`BN6_OW_MEGAMAN`, drawn
  from the player's ROM as it is drawn, never kept; I'm beat his last
  frame, down; Got it! a sparkle beside Wave's pose, which it shares), L
  and R turning to the eight phrases in two columns and back, the four
  favourites in a cross at its right.
- **The window** at the top left: the item's name on its plate, MegaMan
  playing the pose as `square_emote.c` plays it (then standing a moment,
  again), a phrase in BN6's caption box over his head, and whom it goes
  to: "AT ALPHA" where an aimed emote would go to the MegaMan faced (or
  the one who aimed at him first), "TO EVERYONE" else.
- **A sends and closes.** The send is the moment: the Square's answer to
  it (the peer's pose, the "!!" of an answer, the phrase over his head)
  is the reward and is seen whole only with the tray gone; the burst
  (three, then one a second) makes strings pointless; and a repeat costs
  two presses, as the strip opens on the item last sent, its page too.
  B closes, and so does SELECT tapped; the Square goes on behind it and
  MegaMan stands, as BN6 holds him in a talk. A chat box that comes (the
  greeter, a challenge) closes it.
- **SELECT held with a direction** keeps the item under the cursor on L
  and that direction, the same chord that sends it (Animal Crossing's
  slot swapped in place, Elden Ring's gesture slots; the marking menu's
  rehearsal): the favourite's plate flashes, its icon's corner shows its
  arrow, `network.ini` keeps it. Phrases can be favourites too.
- **Touch**: SELECT's touch button opens it (the touch controls stay); a
  tap on an icon or a phrase sends it, on a tab turns the page, on a
  favourite's plate sends that one, on the window closes it
  (`platform_picture_taps`: a finger on the picture on none of the
  controls). Icons are 22 x 28 of the picture's pixels, 132 x 168 on a
  2340 x 1080 phone.

**The palette** (Design D): on the second screen (the 3DS's bottom one,
Android's second display) the Square's panel is every emote and phrase
as a PET button, one tap each (`emote_palette.c`, drawn in
`src/second/second_palette.c`): the emotes with their poses and names,
the phrases on plates, the favourites marked with their way of L, the
tapped one lit (dim where the burst was spent), "AT ALPHA" for the aimed
ones; its NETPASS tab shows the pass the panel showed before. The 3DS
reads its touch panel (a stylus let go is a tap where it last was,
`platform_second_poll`); Android's `SecondScreen` hands a finger let go on
its picture to the game (`nativeTap`); `stap X Y` taps it in a test
([Dev tools](../../devtools/overview.md)). Monster Hunter 4 Ultimate's touch-screen chat and
Phantasy Star 0's one-tap shortcuts are the pattern; the top screen stays
BN6's.

| Presses (a tap one) | Before (L's pages) | The strip | The palette |
| --- | --- | --- | --- |
| A favourite (four) | 1 chord | 1 chord | 1 tap |
| An emote | 5-16, mean 10.5 | 2-6, mean 4.0 | 1 tap |
| A phrase | 4-11, mean 7.5 | 3-6, mean 4.5 | 1 tap |
| The last one again | as its first | 2 | 1 tap |
| On touch | the same, the chord two fingers of one hand | 2 taps an emote, 3 a phrase | 1 tap |

(The strip's counts from the cursor on Wave and on Hello!, as it opens
the first time: SELECT, the steps on its wrapping grid, A; a phrase adds
R. No chat box is typed out on the way.)

**Wave and wave back**: an emote that can be aimed, sent facing another
MegaMan near enough to talk to (the card's Wave too), is aimed at him
(SOCIAL's target, protocol v14). He is told quietly: a raised hand
blinks over the sender's name for the answer's five seconds, no box and
no sound but the emote's; his next emote that can be aimed answers it,
whichever way he faces (one press, L and up). The server flags the
answer (EVENT's answered bit) when it comes within five seconds, and
every screen plays BN6's "!!" between the two and BN6's sparkle (0x94).
Two aimed emotes at one MegaMan unanswered in 30 seconds are the cap:
past it the server shows the sender's to him alone, so a wave cannot
be spammed at anyone; an answer starts both ways afresh.

**In step**: every emote starts on the server's beat, a quarter of a
second of its clock, at least the channel's farthest one-way trip (half
the round trip its PINGs say) and 60 ms ahead, 400 ms at most. EVENT
carries the milliseconds from its sending to that beat; each game starts
it that long after it came, less half its own round trip, so emotes sent
together start together on every screen, and a group's Cheer is one
gesture. Seen with five games on a local server: (below, What has been
shown).
