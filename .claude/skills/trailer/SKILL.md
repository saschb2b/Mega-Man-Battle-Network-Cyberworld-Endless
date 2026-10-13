---
name: trailer
description: Make Cyberworld Endless's trailer for a release - a new look and an original piece of music in the manner of the game the release shows (its habits read from the ROM's song, never its notes), footage from the release pictures' scripted runs, built in tools/trailers/, reviewed frame by frame and by numbers, then shown everywhere the project shows its face (the site's hero, every page's preview, the README, the notes, the 3DS banner's sound). Use when asked for a trailer, a release video or a new cut, when a minor release (0.N.0) is being prepared, or when the trailer's music, look or placement is to change.
---

# The trailer

A release that changes how the game looks or plays gets a trailer of its
own, and it replaces the last one everywhere (AGENTS.md, Trailers). The
project's face changes with the game instead of piling up. Read
`lessons.md` first: the trailers made so far and what the owner said of
each.

The one rule the owner gave in so many words: **never the last trailer
with new screens.** Each trailer has its own look and its own music, in
the manner of what the release is about (0.12.0, BN5's net: BN5's title
theme for the music, BN5's Liberation Missions for the look).

## The parts

| Part | Where |
| --- | --- |
| A cut: footage, look, `VERSION`, `POSTER`, `PLAY_LABEL`, `scenes()`, `compose(only)` | `tools/trailers/vX_Y_Z.py` |
| Its music: `arrangement()`, `instruments()`, `BPM`, `BARS`, `TAIL`, `MIX`, `DS_ENV`, `DS_GAIN`, `DS_LEVEL`, `REVERB`, `BANNER` | `tools/trailers/vX_Y_Z_music.py` (standard library only: the 3DS build renders its banner) |
| Which is current | `CURRENT` and `VERSIONS` in `tools/trailers/__init__.py` |
| The shared kit: capture, `from_shots`, `from_clips`, `shot`, encode, the play poster | `tools/trailer.py` |
| The Game Boy Advance's sound: squares, wave, noise, Direct Sound, MP2K's echo | `tools/trailer_music.py` |
| Helpers for this skill | `scripts/song_habits.py`, `scripts/music_check.py`, `scripts/sheets.py` |

`python3 tools/trailer.py [VERSION] [--keep] [--footage] [--frames A,B]`
captures, renders the music, draws and encodes; only the current one
writes `docs/clips/trailer.*`. The earlier cuts stay renderable (into
`.build/trailer-VERSION/out`), and their videos stay at their release's
tag.

## 1. The brief (before any code)

Write down, in a few lines:
- **The release's headline**, from the CHANGELOG section: what a player
  would tell a friend. The trailer shows that, not the whole game.
- **The vibe**: which game's or which feature's spirit it carries, and the
  visual concept that comes from its fiction. BN5 gave Liberation
  Missions: dark panels freed shot by shot. Name the devices the last
  trailer used (`lessons.md`) and choose others: its transitions, its
  type, its overlays, its colours.
- **The arc in bars**: an intro (4 bars), a main section of short shots
  with a headline each (8), a drop or breakdown for the release's darker
  or stranger part (2), a climax (4), the logo and the end card (2). About
  20 bars, 30 to 40 seconds.
- **The claims**: every headline and caption checked against the CHANGELOG
  and docs (0.12.0: "one battle in four from the second act", "six
  areas", the Dark Soul rising after a missed Chaos Unison). A trailer
  that oversells is a bug report waiting.

If the owner gave a reference (a recording, a video), the vibe follows it;
ask only when the release's headline leaves the vibe open.

## 2. The music

Original, always: the habits of a source song, never its melody, chords in
its order, or samples. The repository must never hold Capcom's sound; a
recording the owner hands over stays in `.build/`.

1. **Read the source song's habits** from the ROM:
   `python3 .claude/skills/trailer/scripts/song_habits.py bn5 0x01 --bars`
   gives the tempo, each track's role (voice kind, envelope, duty, sample
   size and loop, note lengths, range), the key's pitch classes, the drums'
   places, and notes a bar per track (the form: where the intro, the
   breakdown and the main section are). A song on a pickup counts its bars
   late: the script guesses the bar line (`--offset N`, kick and crash on
   one, snare on two and four); rerun with it before reading the drums.
2. **Read the recording** if there is one: `scripts/music_check.py
   --ref FILE` prints its balance of lows, mids and highs to match.
3. **Write the habits down** as the piece's docstring, as
   `v0_3_0_music.py` and `v0_12_0_music.py` do: tempo, key and mode, each
   channel's job, the kit's pattern, the form, then what this piece does
   with them. That docstring is the proof the piece is in the manner of
   the source and not a copy.
4. **Pick a tempo whose bar is whole video frames** at 30 fps: 150 is 48
   frames, 144 is 50 (BN5's 142 became 144). Cuts then land on frames.
5. **Compose in `vX_Y_Z_music.py`**: a `Score` of PSG notes (`sq1`, `sq2`,
   `wave`, `noise`, `fx`) and Direct Sound hits; a theme of two four-bar
   phrases (the second climbing where the first fell), a rhythmic cell
   repeated so it sticks, pickups into phrases, a fill into every section,
   a drop that thins the band (one square alone, the bass one note), and
   an ending cadence that rings. Borrow instruments from an earlier piece
   (`first.instruments()`), add your own.
6. **Check it without ears**: `scripts/music_check.py VERSION --ref FILE`:
   no bar near clipping, loudness that dips at the drop and rises after,
   a balance close to the reference (0.12.0: 47/48/5 against BN5's
   47/47/6), and `roll.png`, the score drawn, for the shape. You cannot
   hear it: say so, and ask the owner to listen.
7. **Set `BANNER`** (from, seconds: about 2.9): the 3DS HOME Menu plays it.
   The piece's most recognisable moment, usually its last cadence.

A change to `tools/trailer_music.py` must leave every piece's render byte
for byte the same: hash `music.wav` of each version before and after.

## 3. The footage

- Take scenes from the release pictures' runs: `from_shots(NAME, first,
  last)` (a `build.py` SCREENSHOTS entry, its `--input` and environment)
  and `from_clips(NAME)`. Add your own runs where none shows a moment.
  BN6-only scenes go in `BN6_ONLY`: they run with BN6's ROM alone, as most
  players have it.
- `--footage` captures eight runs at once (each run's environment goes to
  its container, so autopilot and plain runs mix freely). `--keep`
  reuses what is there; delete one scene's folder to retake it.
- `python3 .claude/skills/trailer/scripts/sheets.py scenes VERSION`, then
  look at every sheet and pick frames. Avoid the Custom screen's CHIP DATA
  TRANSMISSION, the BATTLE START banner and a chat mid-typing unless they
  are the point; a guardian's card, a Soul's flash, a chip landing are.
- A shot plays its scene from its first frame for its length; a scene
  shorter than that freezes on its last frame. Extend the run's `last`
  and retake it (0.12.0's BN6 battle needed 1000 game frames, not 800).

## 4. The look

- The game's picture stays whole and sharp: 4x nearest (960x640) in the
  1280x720 frame, or 2x for a mosaic of four. The 160-pixel margins are
  free for the cut's own readouts.
- Draw everything else in the cut (PIL), never from the ROM: the project's
  logo (`linux/steam/logo.png`) is the one picture taken in.
- Keep a timeline of `Shot`s and `Words` in bars and beats (`F(bar,
  beat)`), so the picture follows the music. Transitions on the cut's
  beat; a quick cut inside a bar gets a faster one.
- Words: a headline, a kicker over it, a caption under it. Fit them: a
  caption shrinks until it fits its band, a title stays inside the
  monitor. Put them where they hide nothing the shot is about (a chat box
  at the bottom: words at the top).
- Japanese: the home page's Japanese text uses only names its pages
  already use or Capcom's Japanese ones you are sure of; leave out what
  you cannot name.

## 5. Review

- `--frames A,B,...` draws only those frames (seconds); `sheets.py pick`
  shows them large. Iterate there.
- A whole render takes about 80 seconds. Then `sheets.py frames VERSION`
  (every 12 frames) and look at all of it: overflowing text, a shot
  frozen or caught on a banner, words over the subject, a cut that reads
  as a glitch, a dead stretch.
- `ffprobe` the MP4 (`--entrypoint ffprobe` on the ffmpeg image): 1280x720,
  AAC, the length the plan says.
- Send the MP4 to the owner (SendUserFile, `display: render`) with what
  you could not check: the music by ear.

## 6. Make it current and show it

1. `CURRENT` and `VERSIONS` in `tools/trailers/__init__.py`;
   `python3 tools/trailer.py --keep` writes `docs/clips/trailer.*`. Delete
   any versioned copies: the folder holds the current trailer only.
2. Rewrite what describes it: the hero's `aria-label` and the
   `VideoObject` (name, description, `uploadDate`, `duration`) in
   `web/index.html` and `web/ja/index.html`; the README's alt text. Run
   `python3 tests/test_site.py`.
3. The release's notes open with it: `<a href="../clips/trailer.mp4"><img
   src="../clips/trailer-play.png" ...></a>`, which `tools/release_notes.py`
   pins at the tag. Make the trailer before the tag; one made after it
   goes up as a release asset, with the owner's go-ahead.
4. The 3DS banner follows `BANNER` by itself; `build.py 3ds` checks it
   packs. Everything else (every page's preview picture, the AltStore
   header) follows the file name.
5. Commit on dev. The site changes when dev reaches main, which is the
   owner's call, as editing a published release is.
6. Add the trailer to `lessons.md`: its concept, its devices, the owner's
   words on it.

## Pitfalls met

- The linuxserver ffmpeg image refuses `-u` without `--entrypoint ffmpeg`
  (or `ffprobe`) for anything but the kit's own encode call. Name every
  container, kill only your own, mount nothing from `/tmp`.
- A heredoc that holds markdown or code must be quoted (`<<'EOF'`), or
  the shell runs its backticks.
- `tools/trailer.py` run as a script imports itself as `trailer`, so the
  cuts and the entry share one `WORK`. Keep cut modules free of import-time
  state.
- After moving or refactoring the kit, draw an old cut's poster frame and
  compare it with its published poster: 0.3.0's came out pixel-identical.
