# The blueprint: one series of trailers

The owner (15 November 2026): "We need to find a good consens to have
stable trailers. like a blueprint. So that not every trailer looks
completetly different. Yet has as much freedom to showcase its changes to
its fullest", with 0.17.0 as the reference: "I really like the music from
0.17.0 and its pace. Especially when there is much to showcase", and a
narrator: "Maybe lans father? Like he comes on after the trailer starts,
introduces himself, very short explanation on what endless cyberworld is
and slides into the new changes ... With a nice outro thanking players".

Until then every trailer had a look and a piece of its own (the rule of
13 October 2026, "never the last trailer with new screens"), and ten
trailers read as ten projects. From 0.22.0 a trailer has two halves:

| Fixed, the series' | Free, the release's |
| --- | --- |
| The slots and their order, the narrator's two among them | Which moments fill them: the cold open, each segment's shots, the recap's tiles |
| 150 beats a minute, a bar 48 video frames, a shot a bar | How many segments (2 to 5) and how long each is (3 to 6 bars) |
| The frame: 0.17.0's battle net, the version in the HP box, the section's label in the Custom gauge, the panel of the segment's shots, the picture at 3x, the caption on the plate; Silkscreen; the PET's colours | The accent (the gauge's fill, the numbers); the field (what moves behind the frame); the cut (how a shot arrives); a section's icon; a panel, a card or a whole segment drawn its own way |
| The narrator: Dad, in BN6's own chat box, hello after the title, thanks before the stinger | His lines; the footage behind his box |
| The house band: 0.17.0's (BN5's "Virus Busting" as habits), F minor, FluidR3 GM, what it plays on each slot | The tune: the hook, its answers, the cadence, the climb, the pickup |
| The title card, the stinger, the end card and its thanks | The release's name; the systems' line when one joins |
| The thumbnail's layout (the youtube-publish skill's `thumbnail.md`) | Its frame, its region, its words, its pointer |

The fixed half is code, so it cannot drift: `tools/trailers/house.py`
(the look) and `tools/trailers/house_music.py` (the band and the slots).
A cut is what is left: `vX_Y_Z.py` (about 150 lines: the name, the
accent, the field, the segments, the narrator's lines) and
`vX_Y_Z_music.py` (the tune and the segments' bars). SKILL.md's gates
all still hold; what the blueprint changes in them is said under each
heading below.

## The slots

| Slot | Bars | What it holds | The band |
| --- | --- | --- | --- |
| Cold open | 2 | The release's two most striking moments, the picture at 4x, no words | The stab alone under rising hats, then the band without its lead |
| Title card | 2 | The logo, the release's name, `VERSION · OUT NOW` | The hook and its answer |
| Hello | 4 | Dad: who he is, what Cyberworld Endless is in one box, "here's what VERSION brings" | Stepped back: no lead, no stab, no snare; its last bar lifts |
| A segment, 2 to 5 of them | 3 to 6 each | A section card of a bar (its number, its name, the panel's rows arriving), then a shot a bar, one concrete thing each | Four kinds in turn (hook into the climb, hook on a new chord, the break, the climax); the last segment is the climax |
| Recap | 2 | Four tiles at 2x, two a bar; the caption counts the fixes | The climb, to the major V |
| Thanks | 3 | Dad: that was it, thank you, a father's last word | As hello, a box a bar, lifting into the stinger |
| Stinger | 1 | A flash, the logo, the version | F minor with its ninth, ringing |
| End card | 3, held | The logo, THANKS FOR DIVING IN, `VERSION · OUT NOW`, the systems, the site, the ROM's lines | The backing alone, the strings' last chord |

Four segments of five bars make 36 bars, 58 seconds; 0.22.0's five
(5, 6, 4, 4, 6) make 42, 69 seconds. A cut is 45 to 75
seconds: with much to show it grows by segments and bars, never by
shortening a shot under a bar or a narrator's box under its reading time.
A release with one headline takes two long segments, not four thin ones.

Order the segments by what a player would tell a friend, but put the one
with the best pictures last: the climax's bars are the band's fullest.

## The frame

`house.py` draws 0.17.0's frame, at 427 x 240 scaled 3x: the battle's net,
the version in the HP box top left, the Custom gauge across the top with
`NN · THE SECTION'S LABEL` filling through the segment and flashing full
on its last beat, the panel on the left (a row for each shot of the
segment, its tag of two or three words; the PET's orange cursor on the
one shown, a green mark on those shown), the game's picture at 3x on the
right, the caption on the plate under both (40 letters at most, a count
in gold at its right where the CHANGELOG gives one). The cold open, the
title, the narrator and the stinger use one big window, the picture at
4x. SKILL.md's label grammar is this frame's: nothing moves between shots
but the words.

What a cut gives a `house.Trailer`:

- `accent`: one colour of the PET's (the site's palette), for the
  gauge's fill and the numbers. 0.22.0: the RUN! button's green.
- `field(fr)`: the 427 x 240 picture behind the frame, the net by
  default; the release's motif lives here (0.22.0: packets climbing the
  net's lines, saves on their way up). It stays dark and quiet: the
  footage is what moves.
- `cut`: `'open'` (0.17.0's: the shot opens out of its row), `'sweep'`
  (drawn in from the left under a bright line), `'hard'`, or a function
  `(small, old, new, t)` of its own, five frames long.
- A `Segment`'s `icon` (one of the site's, `web/assets/ui/`) for its
  card, and its `draw` to replace the panel where a feature wants a
  readout of its own (0.19.0's spinner, 0.21.0's tags would be this).
- Where even that is too tight (several games on screen at once, a
  board), a segment may take the big window for its bars; the header, the
  plate and the type stay.
- `house.Mosaic(scenes, offsets, widen)`: two or four games of one take
  in the big window at once, each whole at 2x, where a feed names a scene
  (the cold open, a narrator's backdrop); with `widen` the first game
  starts alone at 4x and pulls back into its corner as the others are
  uncovered (0.24.0: one player's Square, then four players' at once).

A scene whose footage is not taken yet draws as a card marked FOOTAGE TO
COME, so a cut is laid on stand-ins and each is swapped as its run lands
(0.17.0's way; delete a scene's folder under `.build/trailer-VERSION/`
to retake it).

## The narrator

Dad (BN6's face 0x05, `FACE_DAD`), because he is who explains things to
Lan and MegaMan in the run (his calls and mails, docs/voice/rules.md), and
BN has no voices: the chat box is its voice. Nothing is redrawn: the game
itself says his lines. `--talk narrate:FRAME` has Dad say the boxes of the
environment's `CYBERWORLD_NARRATE` in BN6's own chat box
(`src/director/director_dev.c`), and `house.Narration(scene, boxes,
backdrop)` writes that run, pages it, and plays each box from its beat,
the typing at twice the game's pace (a letter every two frames is slow
for a trailer).

His lines:

- docs/voice/rules.md's Dad: kind, calm, plain; a comma with no space
  after it; about seven words a box; "Now," and ",OK?" are his. No
  "!!".
- A box fits BN6's (three lines of 20 letters; `Narration` refuses one
  that does not) and stays up at least a bar; three words a second is
  the most a viewer reads.
- Hello is three boxes on four bars: who he is ("Hi,everyone! It's Lan's
  dad."), the game in one breath ("Cyberworld Endless is a new Net on
  every dive."), the hand-over ("Now,here's what 0.22.0 brings!"). The
  first two stay the same from trailer to trailer, as a host's greeting
  does; the third names the version.
- Thanks is three boxes on three bars: that was it, the thanks ("Thanks
  for diving in,everyone!"), a father's last word ("Be careful down
  there,OK?").
- He speaks to the players, and only in those two slots. The segments'
  captions are the frame's, not his: a chat box on every shot would halve
  the pace the owner asked to keep.

**Never Dad over a still picture.** The owner, of the first take (15
November 2026): "having dad speak while megaman just sits there is
boring. find a better way to fill the backdrop with something related."
A chat stops the game under it, so the scene he is captured on stands
still. `Narration`'s `backdrop` is a feed of other footage his box is
laid over: the box's rows of his frame over the backdrop's frame, both
whole frames of the game running, no part of either kept as a file. What
plays there is what the box says:

- "It's Lan's dad": a dive under way, a layer walked.
- "a new Net on every dive": three areas in three cuts.
- "here's what VERSION brings": a beat of each segment's best shot, the
  trailer's own contents page.
- The thanks: the release's battles again, then MegaMan on his way.

## The music

The band is fixed, the tune is the release's. `house_music.py` is
0.17.0's piece taken apart: its stab on 3+3+2, its kit, its bass, its
backing thrown left and right, its chords (`sketch_bust_lib.py`, in the
manner of BN5's song 0x15, whose habits are in that file's docstring),
laid on the slots by `arrangement(FORM, TUNE)`.

- **Gate 2** (one Battle Network song, heard in it) is met by the band:
  0x15, "Virus Busting", the song every layer of a run plays. A release
  whose news is another game's may take the shape of its tune from that
  game's theme (its contour and rhythm, read with `song_habits.py`,
  never its notes); the band stays.
- **Gate 3** (every melodic part on the soundfont). 0.17.0's piece
  played its lead, backing and bass on the squares and the wave alone
  and would fail today's `music_check.py`. The house band keeps its notes
  and doubles each of the three in unison on FluidR3 GM (81 Saw Wave, 13
  Xylophone, 34 Picked Bass: 0.20.0's programs), the GBA's channels
  under them. Against 0.17.0's piece, by `music_check.py`: bands
  46/33/19/2 against 49/33/17/1, punch 7.9 dB against 7.8, highs above
  3 kHz 5.5% against 5.9%.
- **Gate 4** (sketches for the owner's ear) is now three tunes, nine bars
  each on the band (`house_music.sketch(TUNE)`; `sketch_house_a.py`), and
  the piece names the one grown (`from trailers import sketch_house_a as
  SKETCH`), so his pick is one line and a render.
- A tune is seven lines of a bar each, in ticks (a bar is 96): `hook`
  (over Bbm7 Eb6 or Dbmaj7 Eb6), `answer` and `answer2` (Abmaj7 Dbmaj7),
  `cadence` (the Lydian Bmaj7, then Csus2), `climb` (Dbmaj7 Ebsus2),
  `climb2` (Fm7, then C major: end on its third or root), `pickup` (three
  sixteenths into the hook), and `top`, the stinger's long note.
  music_rules.md's melody rules hold; `music_check.py --piece NAME
  --source bn5 0x15 --offset 12` must read no rub, "busier than the
  source: none" and "every part on FluidR3 GM".
- The piece gives the cut its bars: `SEGMENTS = (5, 5, 4, 4)` (each
  segment's bars, its card the first), `FORM = house_music.form(SEGMENTS)`.
  The cut reads `music.FORM` and refuses a segment whose shots do not
  fill its bars.

## Making a cut on it

0. **What the release holds, from the merged pull requests**: `git log
   --merges vLAST..origin/dev` and `gh pr list --state merged`, every
   title read, before the CHANGELOG. The CHANGELOG and the notes are
   written behind the work: 0.22.0's first cut was laid from the
   Unreleased section and missed the shared dive and the layers'
   heights, both merged and in neither. The owner: "are you sure you
   have everything showcased in its fullest glory? go over the merged
   prs". A feature merged and playable is in the trailer, said as far as
   it goes; what is merged and missing from the CHANGELOG is also told
   to whoever writes the notes.
1. The brief (SKILL.md, step 1), with the blueprint's questions in place
   of the look's: the name, the accent and why, the field's motif, the
   cut, the segments in order with each shot's tag, caption and count,
   Dad's third hello box, what plays behind each of his boxes.
2. `vX_Y_Z_music.py` and three tune sketches; check them; render them to
   `.build/trailer-sketches/` for the owner.
3. `vX_Y_Z.py` on stand-ins, `--frames` to look; the footage's runs in
   `vX_Y_Z_footage.py` (`SCENES`, `MARKS`: what each sample shows), found
   by an agent of its own in a time box while the cut is laid.
4. `THUMBNAILS` and `YOUTUBE_HOOK` in the cut (the youtube-publish
   skill's `thumbnail.md`); `python3 tools/trailer_thumbnail.py VERSION`.
5. Review, the gate report, the owner (SKILL.md, step 5). The cut is in
   `trailers.VERSIONS` from the start and renders into `.build/`; it
   becomes `CURRENT` (step 6) once the owner has seen it.

## Leaving the blueprint

Only on the owner's word, for one trailer, said in its lessons entry: a
release whose news cannot be shown in this frame at all. A new device
that works (a readout, a cut, a field) goes into `house.py` as an option
for the next cut, so the series grows instead of forking.
