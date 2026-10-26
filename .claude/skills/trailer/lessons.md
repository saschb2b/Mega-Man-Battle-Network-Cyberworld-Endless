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

The owner chose sketch A by ear (22 October 2026) and allowed FluidR3 GM
(MIT) in a Docker image. Sketch A through its samples (`sketch_a_sf.py`:
notes and chords cut into 10512 Hz 8-bit samples, played by the same MP2K
model) measured nearer the recording's tone than the kit's own (tonal
share 83% against 84%; the kit's 80%). It grew into 0.14.0's piece
(`v0_14_0_music.py`, 20 bars), which renders in either sound (`SOUND`);
with the soundfont it renders in `docker/Dockerfile.music`'s image and
keeps its 3DS banner as `3ds/banner-0.14.0.wav`.

## 0.14.0: Pick a side (2026-10-22), `tools/trailers/v0_14_0.py`

The brief, from the owner's headline: the Square's social side and its
wider NetBattles (emotes performed and the strip on SELECT, wave and wave
back, the Square cheering together; a NetBattle from the ask to the
result card and a rematch; Team NetBattles one by one; two MegaMen a
side). The music: sketch A grown into 20 bars, the battle drive in G
minor at 150, played by FluidR3 GM's samples; the owner: it "sounds
awesome".

What was made: 34 seconds, 20 bars at 150 (a bar 48 frames). Every piece
of the release's news is a pair (an emote and who sees it, a wave and its
answer, a challenge and its Yes, two Yeses for a rematch, a team of two,
two a side), so the cut is a versus screen: a slanted seam splits the
frame into a red side and a blue side, each side's stripes stepping
toward the seam on every beat; the seam sweeping across the frame in the
new shot's colour is the cut, and a cut inside a shot sweeps across the
game's picture behind the same slanted line. The words are a call and
its answer: the red side's headline runs in on a slanted ribbon, the
blue side's answer on the next beat ("WAVE" / "WAVE BACK", "BATTLE?" /
"TAKE IT, LAN?", "AGAIN?" / "AGAIN!"); where the two meet (the "!!", the
cheer, the rematch) the seam flashes gold and gold diamonds burst. A VS
badge on the seam for the NetBattle, SIDE · BY · SIDE across it for the
climax; each game framed in its side's colour with a slanted name plate;
the drop's stripes stopped and dimmed. Type: Ubuntu Sans, condensed heavy
italic (its variable axes through PIL's `set_variation_by_axes`). The
arc: wave and wave back, the title; the strip, an emote on two screens,
six emotes seen, the cheer; the ask, the way in, the result card, the
rematch; team up and the blind order, one by one with the rest watching;
side by side asked, the four screens at once, each partner's own Custom
screen with his Cross and Beast Out, the stands; the logo and PICK A
SIDE.

New footage: the NetBattles, the relay and the two a side wait on what
the games do (a bout's end, a card to answer, a team's match), so
`tools/trailer_online.py` grew `Remote` games played by remote play, the
scene's course reacting to each game's log, every frame kept as it comes
(a thread turns every second one into the scene's PNGs), those scenes
run at once after the ones by the clock. Their cards are read from the
picture (the chat box and its green cursor), not the clock: under load a
timed A landed on the wrong player's card.

What went wrong on the way: the owner, of the first render: "the pick a
side screens show megaman standing in the void". The players stood at
`place` spots of earlier harnesses, at the plaza's edge, the net's dark
and its rings filling the picture. They now stand by `square_plan.c`'s
panels (`at(gx, gy, facing)`: only within two panels of the landmark's
base does the floor fill the whole picture), and every Square shot was
looked over again on the sheets (the referee's spot on the Arena's walk
went the same way: the stands are Nova's watching instead). A `place`
while a chat box is open is ignored, and A with none open reads the
faced MegaMan's card: page with B before placing. A player's own emote
leaves his first frame blank (step over it); a feed that holds or slows
the same footage must not sweep. The stands' last frames say who won
("Mira's team won the NetBattle."): cut short of them. The owner:
"approved".

Devices now used up: the slanted seam and its red and blue sides, the
seam's sweep as the cut, call-and-answer ribbons, the gold meeting
(burst and glow), the VS badge on the seam, slanted name plates.

## 0.15.0: Liberation (2026-10-24), `tools/trailers/v0_15_0.py`

The owner: a release about bringing BN5's Liberation Missions to the run,
BN5's song 0x22, the board's theme, as the music's seed, "something
beautiful" as 0.14.0. Three eight-bar sketches from 0x22's habits, all in
the soundfont: A the operation, B the liberation's fanfare, C the
darkness's move. The owner chose A, "yet it is all over the place. with
0.14.0 you took more care." The numbers had matched the song's balance;
the writing had not (`music_rules.md`, Discipline, written from it): too
many sections in nine bars, a walking bass on new notes everywhere, every
part moving at once, a synth brass outside the approved palette.
`music_check.py --source` now sets each part's motion beside the song's
and flags the busier ones; the rewritten A read "busier than the source:
none", held up by ear, and grew into the piece (`v0_15_0_music.py`, 20
bars at 144, D minor). The owner asked that this hold for every future
piece, in the skill rather than in memory.

What was made: 35 seconds. The operation on a war room's map: a
night-blue grid with its sectors marked, each shot a monitor in a sector
of its own at 3x (four boards at 2x), every cut the camera panning to the
next, easing as BN5's board camera does (a share of what is left each
frame); a readout of PHASE against the PAR, following the board's own
banners, crimson with the map for DARKLOID PHASE; where each shot stands
in the run (ACT 2 · LAYER 5); headlines in URW Gothic settling from wide
tracking, captions in a mono face, under the monitor. Footage: the
autopilot (`--dev dark,god,onehit`) through seed 1's act 2 board, seed 7's
fork at home (its pink pad the dark way), four boards of acts 2 to 4.

What went wrong on the way: the owner, of the first cut: units standing
still, a Darkloids' phase nothing like BN5's, the exit's ring clipped (all
fixed in the game, #214, and the footage taken again: the cut showed what
the game lacked); the act card ("A layer here has gone dark") over act
2's first, ordinary layer read as if that layer were the dark one; and
the story "focuses only on liberation. well duh it's mmbn5. But what does
that mean for cyberworld endless?" The second cut tells the run: the fork
after an act, one way dark (MegaMan: "Part of it's gone dark."), TAKE IT?
at the price of the layer's dealer and Mr.Prog, never the same board
twice, three turns or it stays dark, the chip carried on. Lesson: a
release whose news comes from another game is told as what it does to
this one. Also: the dark way's swirl shows only while MegaMan stands
away from it; a line the autopilot pages at once is held on its whole
frame. The owner: "approved".

Devices now used up: the war room's map and its sector monitors, the
camera's pan as the cut, the phase-against-par readout, tracking that
settles onto the beat, the run's position on every shot.


## 0.16.0: Fight on? (2026-10-26), `tools/trailers/v0_16_0.py`

The release: BN6's Virus Battler in the run (a stake, BN6's VIRUS LIST,
the fight at fast forward, take the pot or fight on, the BtlrCard kept
across runs), the run's Crosses on Liberation boards with Team
ProtoMan's commands and a TEAM screen, a held Soul's chips in the finds,
rumble and a DualSense's triggers, a Square anyone can host.

The first cut, refused: a card table. Every shot a card dealt with a slap
onto a pile on a green felt under a lamp, suits by section, set numbers,
a foil's sweep, the pile gathered up at the end. The owner: "it is
totally not in our design style." A real-world metaphor dressed over the
footage, where every approved trailer was built from the game's own world
(0.12.0's board, 0.14.0's versus screen in the game's colours, 0.15.0's
net-blue grid). It became the owner's gate 1 (SKILL.md).

The second cut: a PET screen of its own, at the PET's scale (427 x 240,
the game's picture at 1x, scaled 3x by nearest neighbour), in
`docs/engine/ui.md`'s parts and `tools/site_art.py`'s colours, the site's
fonts (Silkscreen, Pixelify Sans): the band names the shot, a list
window holds the shots as rows with the orange cursor stepping down them
on the beat (the cut), the game's window beside it, the INFORMATION box
the caption. Sections are the list's pages, turned under the green page
arrows; the dark layer's two bars take the screen down six of the GBA's
sixteen brightness steps; the rumble flashes the game window's frame;
the screen fades up and out in sixteen steps as BN6's do; the end asks
"Fight on?" with Yes and No and the green triangle before Yes. Its first
draw had the band's name hanging below the band and Pixelify Sans' fi
ligature reading "frst": its ligatures are turned off.

The music, heard: "the music is alright. but it seems to be again rather
random rather than using an existing cool mmbn music from the rom as a
seed and orient on that style + I hear no usage of the soundfont we loved
in the last two trailers." The piece had been written straight from 0x15's
habits, with no sketch for his ear and its sampled parts few and quiet
(chords on downbeats only, as 0x15's). His measure since: music Akari
Kaida would be proud of (music_rules.md). The sketch round that followed:
three nine-bar sketches, each from one seed (A BN6's 0x15, the Battler's
rounds; B BN6's 0x22 "Showdown", diminished chords and a broken beat; C
BN5's 0x15, Kaida's), each through FluidR3 GM, GeneralUser GS v2.0.3 and
MuseScore General (`CW_SOUNDFONT_NAME`), for him to pick a sketch and a
soundfont by ear. The owner: "i like b but can't hear a difference in
soundfonts." Three banks, the same notes, no audible difference through
MP2K's cut (8 bits at 10512 Hz): FluidR3 GM stays, for its licence, and
the image holds it alone again (music_rules.md). Sketch B grew into the
piece (`v0_16_0_music.py`, 20 bars at 150, B-flat minor): its middle,
500-2000 Hz, had been well under 0x22's; the strings' chord samples
voiced an octave higher and the squares louder in the mix brought it
near (music_check.py's bands 46/32/20/2 against the seed's 49/29/21/2)
without a part more, busier than the source still none.

Before he heard that cut, the owner asked for structure: "I think we
need to define a stricter layout on how we present the changes. also its
start and end. ... I want structure, and professional polish. while still
making each trailer unique and interesting to consume", pointing at two
update trailers, Cyberpunk 2077's Edgerunners II update overview and The
Blood of Dawnwalker's Update 1.1. Their shared skeleton became SKILL.md's
The structure (every trailer), and the third cut follows it in 23 bars
(39 seconds): a cold open of two bars with no words (a Virus Battle's
last blows at fast forward, MegaMan becoming HeatCross) in one big
window, the game at 4x; the title card (the logo, FIGHT ON?, the green
RUN! button's 0.16.0 · OUT NOW); three segments on the PET screen, each a
section card (its number and name in its colour in the game's window, the
band taking its label) and two to four shots of a bar, one concrete
caption each in the INFORMATION box; the recap, three tiles in one window
(a count only where the CHANGELOG gives one: "1 in 4 finds", its "a
quarter"); the logo's stinger with 0.16.0 on the cadence; the end card
held still, never faded (the systems by name, the site, the ROM line).
One type family, the site's Pixelify Sans, inside the safe area. The
piece was rewritten to the slots: its intro under the cold open, the
hook arriving with the title, the toms lifting into each segment, the
drop for the dark layer's, the cadence on the stinger, the backing alone
again under the end card.

The owner approved that cut with one fix: "Fix the B/G font and keep
everything else." Pixelify Sans' capital B reads as a G at these sizes
("GN6'S VIRUS GATTLER"); the words are now the site's Silkscreen, whose
B, G, 8 and 6 stay apart, checked on 1:1 crops of every label and card.
Silkscreen has capitals only, so the site's address, whose case matters,
stays in Pixelify Sans with Silkscreen's B and C set in. Check a pixel
face's B, G, 8 and 6 at the cut's own size before using it.

Devices now used up: the card table, the deal as the cut, suits by
section, the foil's sweep; and from the second cut, the PET screen as the
whole frame: the list's cursor stepping as the cut, the band as the
headline, the INFORMATION box as the caption, the page arrows between
sections, the brightness steps for the drop, the frame flashing on hits.
