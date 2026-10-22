# Trailers made, and what they taught

## 0.3.0: the first (2026-09-29), `tools/trailers/v0_3_0.py`

21 seconds, 13 bars at 150. A 2003 game advert: chrome words that slam
in on the beat with speed lines, white flashes and a shake on every cut,
a punch-in zoom, scanlines, the net's blue diagonal grid behind a framed
picture. The music: an E minor piece in the manner of BN6's battle themes
(songs 0x15 and 0x16), 3-3-2 stabs, sixteenth bass jumping octaves,
orchestra hits on bVI, bVII, i. It stayed the site's hero until 0.12.0.

Devices now used up: chrome type, speed lines, white-flash cuts, shake,
orchestra hits, the 3-3-2 theme.

## 0.12.0: one Battle Network (2026-10-13), `tools/trailers/v0_12_0.py`

The first cut was 0.3.0's advert with BN5's footage and 0.3.0's music.
The owner: "you've used the old music again... Don't make the old
trailer just with new screens. bn5 is a completely different vibe", with
BN5's title theme as the inspiration (a recording, kept in `.build/`).

What was made: 36 seconds, 20 bars at 144 (a bar 50 frames). A
Liberation Mission: the net's panels dark purple around a field monitor,
freed to blue shot by shot, taken back by darkness for the DarkChips,
all freed at the logo. Typed briefing words with a kicker and caption,
panel-flip cuts, violet glitch on dark shots, readouts in the margins
(share freed, which net a shot is from, a radar). An intro of BN5 feeds,
"TWO NETS" with BN6's and BN5's side by side, then one picture split down
the middle: "ONE ENGINE", "ONE BATTLE NETWORK". The music: B major with
Lydian and Mixolydian colour, plucked square arpeggios over three
octaves, eighth-note wave bass, five-tick stabs, pads, a busy kit, a pad
intro over a pedal, a breakdown of one square alone on the borrowed iv.

What helped: reading BN5's song 0x01 for habits (tempo 142, six tracks'
roles, the drums' places once the pickup was found, the form from notes
a bar) and the recording's band balance; contact sheets of every scene
before picking; drawing single frames with `--frames` while iterating.

What went wrong on the way: a BN6 battle scene too short for its shot
(froze on its last frame); a title wider than the monitor; captions
running past their band; words covering the flame's chat box, the very
thing the shot was about; a montage of fast flips that read as glitches
(hard cuts with a punch-in instead); a visitor shot that opened on the
Custom screen.

Also decided with this trailer (owner, 2026-10-13): a trailer for every
release that earns one, replacing the last everywhere, "so that everything
feels it transforms with updates rather than just grows over time".

Devices now used up: the liberation field, typed briefing words,
panel-flip cuts, the monitor with side readouts, violet glitch.

## 0.13.0: Cyberworld Endless connects (2026-10-20), `tools/trailers/v0_13_0.py`

The owner: "a cool new trailer... The trailer should surely focus on the
online part. Cyberworld Endless connects", the way online shown first,
then the NetPass, then each thing to do there, with BN6's "Surge of
Power!" (a recording, kept in `.build/`) as the music's inspiration and
MMO launch trailers as the model (issue #132's brief: a hook readable
without sound in five seconds, the place full of people, escalation, a
call to action).

What was made: 39 seconds, 23 bars at 150 (a bar 48 frames). Linking:
several players' games on screen at once, each a window in its
operator's colour with his name, channel and ping, joined by lines of
light whose packets carry the cuts (a window opens as a packet from the
last one reaches it, traces its frame, scans its picture in; it closes
into a line). The words are the Square's own manner: a step's pill
("01 · GO ONLINE" to "07 · DIVE"), its headline in a white speech bubble
whose tail points at the game, the phrases as chat bubbles with the
speaker's name. A HUD over the top (ONLINE or CONNECTING, the channel,
eight operator pips, a ping line) and the Square's log at the bottom,
MMO chat. The arc: two games logging in and saying hello under
CYBERWORLD ENDLESS / CONNECTS; the gate, CONNECTING, the log-in beam; the
NetPass's stamps on the music's three stamp beats; meet, trade, a
NetBattle from both sides, the stands and the referee; the week's DeepNet
on the Board with the field gone dark; two, four, eight games; the logo,
BRING A FRIEND., YOU and A FRIEND linked, the address. The music: BN6's
song 0x16 (matched to the recording by tempo and chroma), its habits in F
minor at 150, the two squares as two players calling and answering, the
idle one running the 32nd-note arpeggio, joined in octaves at the climax.

New footage: the release pictures' runs are single games, so
`tools/trailer_online.py` runs a local server and up to eight headless
Linux builds at once against it, each with its own data folder, name and
script; a cut's `online_scenes()` lists them and `footage(keep)` takes
them (the kit calls it after `scenes()`). Scripts go by the wall clock
(`Visit.at(seconds)`), since a visit plays at the clock's pace and games
start seconds apart. Trade Central's screens came from the `trade` dev
step, the Board's entries from a seeded `weeks.jsonl`.

What went wrong on the way: BN6's chat types slowly and a press mid-typing
only finishes the line, so the phrase menu needs its question paged,
then Say something, then the phrase, about 7.5 seconds a phrase; a
`place` while a chat is open is ignored (MegaMen stayed at the arrival);
`--dev online` skips MegaMan's gate question, so the gate's shot needs a
`network.ini` instead; the hook's first footage frame was the gate's
flat yellow flash (start where the Square is drawn); captions under a
headline ran into the chat bubbles below them; an overlay caption on the
games needs a plate. Which player wins a scripted NetBattle changes with
the run, so the words name no winner.

Devices now used up: windows joined by links with packets carrying the
cuts, speech-bubble headlines, the numbered step pills, the operator
HUD and chat log, the multiplying wall of games.

## 0.13.0's music heard again (2026-10-22), `tools/trailers/sketch_*.py`

The owner: "Its current music feels very ai-ish. Doesn't truly feel like
a MMBN theme." The piece had been written from "Surge of Power!"'s
habits, so the gap was not the source but how the habits became music.
Against his recording, by numbers (`music_rules.md`, Why 0.13.0's piece
read as "AI-ish"): half the game's swing inside a beat (3.4 dB against
7.1), a third of its noisiness, the spectrum 30% filled against 54%,
nothing between 120 and 500 Hz, static pans where every BN6 song throws
a track left and right note by note, no bends, one rhythm cell in 15 of
23 bars, a major V at the cadences. Its own notes rendered through the
new kit measured nearly the same: the writing was most of it.

What changed: `song_habits.py --detail --theory` (articulation, the drum
kit's keys, harmony and melody in aggregates); fifteen BN6 and BN5 songs
read into `music_rules.md` with the composers' own words; the kit's
sampled instruments of our own (clipped kick and clap, chord samples, a
distorted stab, a looped lead, slap bass, bell, brass, hand drums, an
impact), `bend` on any note and `HARDWARE = 32768` for unsmoothed squares,
every earlier piece byte for byte the same; `music_check.py` with
`--piece`, punch, four bands and the lead's rubs. Three eight-bar sketches
for the owner's ear before any piece: a battle drive (A, nearest the
reference: 7.4 dB of punch against 7.8, bands 54/31/13/1 against
57/28/13/1), the Square's bounce (B), a heroic opening (C). Not yet heard
by anyone; the owner chooses.

Devices used in the sketches (not yet in a trailer): ping-pong 32nds, a
power chord diving, triad samples over other basses, a skank on the
off-beats, the square and a bell talking in thirds, a Lydian lift.
