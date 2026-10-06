# Changelog

## Unreleased

- **MegaMan arrives from any side of a layer** (issue #106). He arrived at
  the top of the screen on nearly every layer, facing straight down, so
  every layer was walked top to bottom. BN6's own maps, read from the ROM
  (`--atlas arrivals`: every jack-in and every warp onto the net areas'
  maps, 54 places), put him at the top, the bottom, the left and the right
  about as often, always facing along one of the world's axes into the
  map. Each layer now plans its side: an act's three layers arrive at three
  sides, and an act never opens at the side the last one closed at.
  MegaMan arrives on the pad nearest that side, facing along the way's
  first leg, everywhere he enters a layer: from the last layer's exit pad,
  through Lan's HP's portals, at a CONTINUE (at the start; elsewhere as he
  stood). Over the study's runs he arrives at the top 32%, the bottom 29%,
  the left 19% and the right 18% (88%, 0%, 9% and 2% before), facing into
  the layer 97%; the way still runs through the signature (96%), and walks
  from the left or right are a tenth shorter. The Aquarium's and Mr.
  Weather's acts, one layout round one signature, no longer build one
  layer three times: each is entered from another side. The atlas report
  names each layer's arrival. This build makes layers differently, so a
  CONTINUE starts the layer afresh.
- **CONTINUE no longer comes back with MegaMan stuck** (issue #23). Runs
  continued from their saves came back on the map with MegaMan unable to
  walk or open the PET, while L, the map and the dev menu still worked: the
  state had been written while BN6 itself held him (a fade's mark, its
  conveyor flag, a cutscene's walk, a chat's flag), and the CONTINUE's way
  back into the map does not undo those. Every save now waits until BN6
  lets MegaMan walk and open the PET (the layer's checkpoint, the arena
  door's, home's, the PET's Save and the quit's). A save already written so
  is freed as it continues: held for two seconds with nothing under way, or
  the pad pushed with no step taken, BN6's holds are let go and the map is
  entered again; a continue that would have stayed black, its map waiting on
  a fade, goes on too.
- **BN5's battles roll as the run does** (issue #102). Every run's first
  battle in BN5's engine drew alike: BN5's random numbers began from its
  core's boot state each session, whatever the run's seed, so seeds 1 to
  4 all opened on the same hand. Each battle there now starts BN5's two
  random number words from the run, on the frame BN5 begins it: a random
  battle's from the layer's seed and its battles so far, a guardian's
  from the layer's seed. Its first hand, its green Mystery Data and find
  and its reward differ from run to run as BN6's do, and the same seed
  fights the same battle again, after a CONTINUE too.
- **ProtoMan can be hit after his cross** (issue #55). A player found
  ProtoMan untouchable after he dashed his X-shaped cross while Silence's
  music played: every attack passed through him, with no clank of his
  shield, turn after turn. It is BN6's own slip, seen in BN6 alone: during
  each stroke of the X his hurtbox goes off whenever something touches him
  and comes back in the next column, and the cross ends without bringing
  it back. Silence's music touches him every few frames, so a touch after
  the last column was likely; MegaMan standing where a stroke ends did it
  too. He stayed that way until his next cross, which comes only with
  MegaMan on the middle row's middle or front panel. Now attacks strike
  him again as he lands from the cross, as BN6 has it after a cross cut
  short, both as a guardian's copy and in the rival's netbattle
  (docs/FIDELITY.md).
- **The Library's card on the second screen** (issue #83). In the PET's
  Library the second screen shows the chip under the cursor as the Custom
  screen's card: its picture twice as large, its element, power and
  text, the codes it comes in (BN6's Library leaves them out), and how
  many copies the run holds in the folder and the Pack. A number never
  seen stays BN6's "??" with a blank card, and the P.A. Memo shows the
  Library's counts alone. Beside it, the Library's classes go two to a
  line under BN6's own tab names (StdChip, MegaChip, GigaChip).
- **A mail's sender on the second screen** (issue #83). In E-Mail the
  second screen shows the mail under the cursor, or open, by its sender,
  as BN5 DS reads a mail with its sender's face: Dad's face from the ROM
  and his name for the lab's mails and each guardian's (the BBS, a
  board, by its name alone), the subject, and how many of the whole
  list's mails are new (the top screen shows four at a time).
- **ACCESSING while the PET's menu is open** (issue #83). As BN5 DS dims
  its field while its PET menu is open, the second screen's PET at home
  lies dimmed under a still band, ACCESSING, in the PET's own green with
  BN6's three stripes, in the town and on the net alike; as the menu
  shuts, the home or the net's map is back at once.
- **Anonymous statistics, asked for once** (issue #104). At its first
  start the game asks, in the PET's panel over the title, whether it may
  send anonymous play statistics; the cursor starts on No, B and Escape
  answer no, and nothing is sent before a yes. A yes sends, to the game's
  own site on the developer's Umami, the game's start (its system,
  version and screen size), a run's setup as it begins, each guardian's
  battle as it ends (who, won, lost or left, the layer, net and threat,
  MegaMan's HP left and the battle's seconds, the how-manyth meeting) and
  a run's end (how far, where, by whom, its minutes and setup): no names,
  IDs, ROMs, paths or saves. The answer is `settings.ini`'s `statistics`
  line; the controls screen (SELECT on the title) shows it on a Statistics
  row whose A asks again, and on the 3DS SELECT on the title asks. The
  requests go out beside the frames with each system's own way: libcurl
  where Linux and the handhelds have it (loaded, never needed to start),
  WinHTTP on Windows, NSURLSession on Macs, iPhones and iPads, Android's
  HttpURLConnection, the browser's fetch, and on the New 3DS devkitPro's
  libcurl with mbedTLS and Let's Encrypt's roots (the 3DS's own TLS stops
  at 1.1, which the server refuses). Each is tried once, three failures
  in a row end them for the session, and a system that cannot send never
  asks. The Flatpak shares the network for this alone, and Android's app
  asks for the internet permission. The run log names ProtoMan's duels
  `duel` and a won run's last line `won`.
- **Super bosses: the Cybeast and Bass** (issue #100). Each waits at a
  place of its own, never at random. The endless net's Nest is the
  Cybeast's den: no copied guardian at its end but Gregar itself, in
  BN6's own final battle with its final battle theme, and Gregar SP on
  every Net after the first (the Net copied the last battle). Once the
  Secret Area has been cleared in any run, Bass waits there in every run
  after, in BN6's dormant stone, his form following his record with
  MegaMan as BN6's chain does: Bass, Bass SP once beaten, Bass BX once he
  has fallen twice and the Cybeast once. The run foreshadows them: a
  growl from below at the Nest, a bystander's word that the floor keeps
  shaking, L's and MegaMan's words before the arena, Chaud's call after
  the Secret Area's first clear (another signal down there, no copy's), a
  BBS post about a cloaked Navi, and at the golden gate MegaMan sensing a
  dark signal (Bass by name once they have met, and his form), the
  question starting on No; neither is named on the PET before they meet.
  Their entrance is BN6's own staging: the music fades as MegaMan nears
  the bigger arena (7 x 7 where the layer has room), the floor rumbles,
  Bass's stone cracks, the screen fades to white and he stands there out
  of it, throwing his cloak open, or the Cybeast rears and roars; then a
  grander title card. Their battles are BN6's records, on the map's own
  background, with no running. They pay a GigaChip of BN6's (BassAnly,
  Bass, ColForce; BugRSwrd), five HPMemory and a full heal; Bass leaves
  in a white fade with BN6's sound, the Cybeast in a long one, the Net
  silent after it, and Dad calls. BN6's own random records of Bass BX,
  MegaMan's beast and the Navi the US version cut no longer turn up as a
  Server's challenge. Once fought, MegaMan briefs their moves.
- **Silence after a guardian's battle** (issue #100). The staging's hush
  played song 0xFF, which stops nothing in BN6: the area's theme played on
  as MegaMan stepped into an arena and, after the battle, under the
  guardian's last word. It plays BN6's own "no song" now, 0x63, and after
  the battle once the map's theme has started, so the last word comes in
  silence and the theme returns with the Guardian Data, as the staging
  meant (docs/BOSSES.md).
- **Face to face with a guardian** (issue #100). Before the talk MegaMan
  steps up beside the guardian, at his height, and turns to him; the
  guardian logs in facing MegaMan. Where an arena was entered from above,
  MegaMan used to walk straight at the guardian, who then stood half
  behind the chat box through the talk; now both stand clear of it
  whichever side the arena is entered from, Bass and the Cybeast too. The
  steps are the staging's, as BN6's cutscenes walk MegaMan; his own
  walking is untouched.
- **Layers to remember** (issue #98). After several layers the net felt
  the same: its rooms were all of a size, no landmark stood on most of
  them, and an act of Seaside, Sky, Green, the Graveyard or the Nest
  always built two of its three layers alike. Each layer is now built
  round one room it is remembered by, its signature, after its area's
  own landmarks: Central's crater or big field, framed by an avenue of
  cybertrees; Seaside's great field, or one round a pool; Sky's ring road
  round a pad, or its pods grown to a plaza; Green's grove under the
  giant cybertree, or its grass with holes in it; the Graveyard's great
  slab or a slab round one hole, below its monument; the Undernet's court
  with its statue between braziers, or a great plus; the Nest's plus or
  slabs; a plaza or a plus in the comps and on the homepages. It is the
  layer's biggest room, the way to the exit or the guardian runs through
  it, and the area's landmark stands at it: over the study's layers
  Green's giant tree stands on 82% of its layers (41% before) and the
  Undernet's statue on half (3%). An act's three layers are three
  places, never two in one layout and one signature where the area has
  others, and an act no longer opens as the last one closed. The tile
  test draws the new layers cleaner (2.4 tiles off and 75.1 seams per
  100 panels, from 2.7 and 82.4). A comb's rung meets its field square
  on, so holding the arrow's way no longer walks MegaMan past a lane
  there. BN5's Science Labs and Oran Isle, whose art draws every room as
  its small hubs, keep their layers without one. The design, what
  Warframe's tilesets taught and the numbers are in docs/LEVEL_DESIGN.md
  (Identity); `build.py atlas` names each layer's signature, the dev step
  `sig` puts MegaMan in it, and `build.py screenshots identity-central`
  (and -seaside, -sky, -green, -graveyard, -undernet), `build.py clips
  identity` and `tools/before_after.py` with the same names show it. This
  build makes layers differently, so a CONTINUE starts the layer afresh.
- **A thank-you after a download** (issue #103). On the site's download
  page, the first download of a visit (any release file, or the iPhone's
  Add to SideStore) opens a mail from Saschb2b in a PET window beside it:
  thanks, a Buy me a coffee button, and on GitHub a star for the
  repository, a follow and the issues for bugs and ideas. The download
  starts as it always did. Esc, Close or a click outside closes it and
  hands the keyboard back to the link; its links open a new tab, so the
  page's install steps stay. The site's analytics count its opening and
  its links as they count the others, never who clicked.
- **BeastOut from the start, once the endless net is beaten** (issue #99).
  When the endless net's own Nest has fallen in any run (the milestone of
  Bass's mark on the title), the JACK-IN SETUP's Help row shows a fifth
  helper, BeastOut, on a line under the other four, NEW the first time.
  On, BN6's own BeastOut is in the Custom screen from the run's first
  battle, in the short net too, which never reaches the Graveyard: the
  emblem under OK, its three turns a battle, BeastOver if pressed again
  while tired. Off, the Graveyard's guardian gives it as before. It is
  remembered with the last setup, named over the summary's title and in
  Dad's dive report as every helper is, and counts for every unlock; the
  run that opens it says "Unlocked: the BeastOut start". In such a run Dad
  calls as it begins to say he has unlocked the PET's CybeastButton
  (MegaMan tamed the beast at the Nest before), the Graveyard's guardian
  stirs the Cybeast without unlocking anything, and where the older Net's
  battles run, MegaMan says the Cybeast can't come in, as he says of a
  Cross. The setup's rows draw a pixel closer while the fifth helper
  shows, so its notes keep their three lines over JACK IN!.
- **A ROMs screen before the game, and saves that outlast a reinstall**
  (issue #97). On a PC, a Mac, Android, an iPhone or an iPad the game
  first opens on its ROMs screen, drawn in the look of the second
  screen's PET panels on the game's own screen (a phone's one screen
  too): two cartridge slots, BN6 Cybeast Gregar (needed) and BN5 Team
  Colonel (optional), each an open spot until its ROM is in. A slot
  opens the system's own chooser (Windows' file dialog, `zenity` or
  `kdialog` on Linux, the Mac's open panel; a phone's folder picker), or
  a file is dropped on the window, and one that is not right is named
  with the reason ("BN6 Cybeast Falzar, not Gregar"). A cartridge in
  shows its game's face, drawn from the player's own ROM; PLAY starts as
  soon as BN6 is in, and later starts go straight to the game. R on the
  title opens the screen again to add BN5: on Android that took a hidden
  icon shortcut before, or a folder chosen once. On a phone or tablet
  the app keeps a copy of the saves in the ROM folder
  (`cyberworld-endless.cwsave`), renewed a few seconds after each save,
  and after a reinstall, choosing that folder again offers them back (in
  Android's emulator: uninstalled, installed again, BRING BACK, and
  every save file as it was, byte for byte). Android's own backup keeps
  the saves and settings now, never the ROMs: it took the ROM too, and a
  reinstall could bring BN6 back with older saves and never show the
  page that adds BN5. The browser's player has the same two cartridges,
  their faces drawn from its ROMs, **Play** (was **Jack in**), and
  **Save a backup** and **Load a backup**, in the same file. Its layout
  takes three ideas from game launchers on Dribbble: the empty slot as a
  card with a plus and its one action, one accent colour for the cursor
  and PLAY alone, and the chosen cartridge lit round its edge with a
  status chip (docs/FIDELITY.md). Also: the title's line of keys, which
  a saved run's CONTINUE row hid, stands between the logo and the menu
  then; and the Android game reads its screen's size again as it starts,
  where a size that came just before (the navigation bar hidden) was
  lost, leaving a black band and taps a little off.
- **The town's hour** (issue #90, the epic #84). The day goes on as the run
  does: morning as it begins, then afternoon, evening and night before the
  Nest, Central Town's streets and people in the hour's light (chats keep
  their own colours, and indoors the lights stay on).
- **The second screen at home** (issue #91, the epic #84). The PET's home
  panel names the visit's hour and lists the request you hold and how far
  along it is, and the ways lit in Lan's HP: the pink pad's area and
  guardian, the links', and the older portals back to areas won.
- **The home shop** (issue #89, the epic #84). AsterLand's clerk (BN6's
  own) runs BN6's Order Service: any chip in your Library, in your
  folder's codes where it comes in them, at twice a Net Dealer's price,
  one order a visit. The SubChip seller beside him stocks the keys the act
  ahead needs (Unlockers, RushFood, a WWW-ID) and MiniEnrg, FullEnrg,
  SneakRun and Untrap. On the second screen, a shop's panel now names the
  entry under the cursor as the list shows it. A facing the counter
  reaches the clerk or the seller from anywhere in front of him, a step
  away too (a playtester faced it from four places and got the
  showcase's text), L in AsterLand says where orders and SubChips are,
  and the shopper stands at the NEW case, out of the seller's way.
- **Requests at home** (issue #88, the epic #84). At each visit three
  people post a request for the act ahead: the NetBattler at AsterLand's
  request board (a chip of an element from your Pack, or a vow: no
  Mr.Prog patch-ups until the act's guardian falls), the NetBattle club's
  member in class 6-1 (quick wins, or wins without a scratch) and the man
  from the lab in town (open every Mystery Data on a layer, or clean
  wins). Take one or none: one at a time, as BN6's Request BBS has it.
  MegaMan says when it is done, or when a patch broke the vow, and the
  one who asked pays at the next visit: zenny, BugFrags, a chip a tier up
  in your folder's codes, or two HPMemory for a kept vow. L at home says
  where a reward waits, and L in the town names, once a visit, the
  requests posted, an order AsterLand can still make, and which way its
  door is. A vow is posted
  from the second visit on (taken blind at a run's start, one cost a
  playtester his run on layer 1), and while one holds, L and the arrow
  no longer lead to the Recovery Mr.Prog: L says the vow and where the
  dealer's MiniEnrg is, and the Mr.Prog's patch asks first, on No. Its
  reminder waits for the act's card and words, which it had ended
  unseen. A run that ends holding a request hears about it from the one
  who asked at the next run's start. Home after an act, the PET's Save
  saves where Lan stands, and CONTINUE goes on from there (it said
  "Saves begin on layer 1"). A run saved before continues with no
  request.
- **AsterLand and the Cyber Academy open** (issue #96, the epic #84).
  Central Town's chip shop and Lan's school are places of home now, BN6's
  own maps as they stand. In AsterLand the clerk stands behind his
  counter, BN6's own Chip Trader trades from AsterLand's prizes in the
  folder's codes, the request board shows the visit's post, and the
  shop's checks read as BN6 has them; the Number Trader is out of order,
  as its public codes would hand every run BN6's prizes. The Academy's gate leads through the foyer and
  both hallways to Lan's class 6-1, where the NetBattle club meets on a
  day without class; the other rooms stay shut. L names the way out, and
  the map's label and the second screen the place. Also: A reads the
  real world's checks again. The probe it looks along, widened for
  talking to navis on the Net's platforms, read past every check a cell
  deep, all of those in Lan's room among them; it is BN6's own off the
  Net now.
- **A town that remembers** (issue #87, the epic #84). Central Town's
  plaza Mr.Prog calls the Net's news: at a run's start how the last run
  ended ("BLASTMAN'S COPY DELETED MEGAMAN! ON LAYER 3!"), after each act
  the guardian just deleted and where. Lan's classmate, the neighbor, the
  man from the lab and the gossip by the statue speak of the run and the
  runs before it ("Lan! You beat BlastMan's copy? No way!"), one line
  each, new at each visit. And the crowd moves: at each visit a different
  few are out, and the others stand at one another's places. The lines
  that still sent Lan to the bird statue's port are gone.
- **Going back, priced by the Net's clock** (issue #95, the epic #84).
  In Lan's HP two more links go back to the last two areas the run has
  won: one layer of the area at its act's tier, with the Net Dealer and a
  Recovery Mr.Prog, its battles and Mystery Data, a new layout each time,
  and its exit pad leads home again. Each link opens once a visit. The
  price: while MegaMan goes back, the Net keeps copying, so every trip
  moves the Net's clock a notch, and every guardian after it has a tenth
  more HP (BlastMan's 400 is 520 after three trips; two trips cost about
  an act of a guardian's HP). MegaMan says so beside the link before the
  first trip, the trip's card and the second screen's home panel show the
  clock, L names it in Lan's HP and before a guardian, and Dad's report
  counts the trips. A trip holds no HPMemory, ScrtData or RegUp: what makes
  MegaMan stronger stays where the run goes on. Also: L at home names the
  way on from each place (the PC, the stairs, Lan's front door, the pink
  pad) and the way-on arrow points there, and the second screen's home
  panel says where to go at home.
- **Lan's HP, the warp zone, and Central Town home** (issues #85, #86,
  #93 and #94, the epic #84).
  Every jack-in now arrives in BN6's own Lan's HP, MegaMan's homepage,
  and its pink pad and link squares are the run's ways on. An act's
  guardian deleted, his exit takes MegaMan there: the act's AREA CLEAR
  card, a word on the act ("Phew! We're home,Lan!"), the next act's
  portals lit as BN6 shows an open link, and the run saved ("From Lan's
  HP" on a CONTINUE). The Guardian Data no longer asks "Which way?": the
  act's own area is the pink pad, the other way and the dark way into
  the Undernet (sealed until the Secret Area has been cleared) are links.
  MegaMan can't know where a link leads before taking it, so beside one
  he says what he reads through it ("Whoa,salty data! Like the sea!")
  and the strong Navi's signal, naming it only where he has battled him.
  R asks BN6's "MegaMan, jack out?" and takes Lan back to his PC. The
  arrow leads onto the corridor up to the pink pad, whose lip a walk
  straight from the blue pad meets (a playtester pushed at it for
  sixteen calls), and shows after a push there, as on a layer; MegaMan
  says what Lan's HP is once, then a word. Home
  is Central Town, the same every run: a run begins in Lan's room (BN6's
  own, his house too), and R at his PC jacks MegaMan in ("Jack in!
  MegaMan, Execute!!"); down the stairs and out of the front door (L in
  the house names it) is the town, its townsfolk and shops as before, no longer with a jack-in of
  its own. The other towns are no longer a run's. docs/HOME.md has the
  design: next, going back through older portals priced by the Net's
  clock, the townsfolk's jobs, a shop at home, and a town that
  remembers.
- **No lessons on BN6's own mechanics.** The game is for Battle Network
  players: MegaMan no longer explains the Pack and the Folder after the
  first chip comes in, nor a battlefield's Mystery Data after the first
  battle with one, nor the exit pad, nor the NaviCust's rules (the command
  line, bugs), the way into it in the PET, or that L and R turn a program
  only with its Spin (a held Spin is still named). What the run adds (Rush's gaps,
  vows, the Net's clock, the older Net) is still said.
- **Clearer words.** The Net Dealer and MegaMan say why Rush needs a
  RushFood for each panel of a gap ("He eats just one!"), and L names a
  second violet mark on the map as another one.
- **The older Net's chips that sit out are named once a run.** After the
  first battle there, MegaMan no longer names again the chips his arrival
  words had named (a playtester heard them twice, as if new).
- **Townsfolk who walk stay drawn as they walk.** Seaside's and Green
  Town's walker vanished the moment he set off and came back when he
  stopped: his sprite's walks are empty in BN6's data. Both walk as
  other people now, and Central Town's late Academy kid, who glided on
  her way back (one still frame facing south), walks both ways.
- **A BN5 battle's BugFrag and green Mystery Data reach the run.** BN5's
  results screen gives two rewards at most: the busting level's, then
  the find of a green Mystery Data still standing on the enemies' side as
  the battle is won (1000 to 3000 zenny, 1 to 3 BugFrags, or a chip).
  Only the first reached the run: a playtester's screen said "50 z",
  then "BugFrag 1", and his BugFrags stayed at 0. Both go into the run
  now, BugFrags to your count, and the run log names the find (`reward
  200 zenny find 1 BugFrag`). A find's chip with no namesake in BN6
  (AirHoc) shows as 200 zenny, as a busting reward's does, and `--dev
  gem` sets a Mystery Data in BN5's battles too.

## 0.10.0 (2026-10-05)

- **The second screen's polish** (issue #81). The NaviCust programs'
  blocks wear BN6's own colours, measured from its preview of a program of
  each; every panel lays out at an Android display's larger sizes too
  (the AYN Thor's 413 x 360 and up), which captures check with
  `--second-size WxH`; the README shows the battle's, the folder editor's
  and the NaviCustomizer's panels beside BN6's screens, and
  docs/FIDELITY.md says what the second screen is and whose its pictures
  and words are.
- **Shops and traders on the second screen** (issue #78). At the Net
  Dealer and the NaviCust vendor the entry under the cursor is a card (a
  chip's picture, code, element, power and text; a program's shape, kind
  and what it does; an item's name and, for the run's own, what it does)
  with what the run holds of it: the copies in the folder and the Pack, a
  program's held and on the board. A Chip, Special or BugFrag Trader says
  what it takes (BN6's own terms) and that it gives a chip new to the
  Library, and how many chips the Pack holds. Outside the folder editor
  BN6 counts the folder's chips in the Pack too; the second screen leaves
  them out, as the editor and the traders do, and `--dev pack=N` adds its
  chips to those counts.
- **The PET at home and its screens on the second screen** (issues #79,
  #77). In the town the second screen is no longer dark: MegaMan's face
  from the ROM, twice as large, beside where R jacks him in and the run's
  setup; on a layer the PET's menu and its KeyItem, SubChip, E-Mail, Comm
  and Save screens show the same home with the layer's next step (its
  guardian, or the exit pad). MegaMan's status adds the run's record (the
  layer, the guardians beaten, the viruses deleted, his max HP and what
  the NaviCust's programs add to it, the Cross brought) and the programs
  on his board; the Library, its classes against what a run can hold and
  the chips this run added. On the title the PET is at rest: the
  profile's record (runs, best layer, guardians beaten, viruses deleted,
  short nets won) and whether BN5's ROM was found beside BN6's.
- **The NaviCustomizer on the second screen** (issue #76). The program
  the cursor is on, in the list, on the board or held over it, is a card:
  its shape in its colour, its name, what kind of part it is and where
  that goes, the copies left or where it stands, whether it fits the
  board's free cells as they are, whether L and R turn it (its colour's
  Spin), and what it does in MegaMan's words. Under it, what RUN would
  bring as the board stands: no bug, or its cause, the same truth a step
  before RUN tells it; and the board's rules in a line. On RUN, the
  programs on the board.
- **Battles on the second screen** (issue #74). On the Custom screen the
  chip under the cursor is a large card, as the DS games drew their
  chips: its picture twice as large, its code, element and power, and its
  text from the Library, the chips picked so far under it and, on OK,
  their list in order. In CROSSSELECT the Cross under the cursor says
  what hits MegaMan twice as hard in it and what it gives; on the Beast
  Out emblem, the turns the EmotionCounter leaves (or that MegaMan is
  tired, when a Beast Out ends in BeastOver) and the Cybeast's Attack+30.
  While MegaMan fights: the turn's chips in order with their power as
  the HUD writes it, the used ones dim and the next lit; his Cross and its
  weakness, Beast Out's turns left and Full Synchro; every enemy by name
  with its HP; and against a guardian MegaMan has met before, his element
  and what MegaMan told of him at the arena. A panel the game's state
  leaves as it was is not drawn again.
- **The folder editor shows the whole folder on the second screen**
  (issue #75), as BN5 DS's does: its thirty chips as BN6's own icons, each
  framed by its rank (grey standard, blue Mega, red Giga, purple Dark) and
  marked where it is the Regular chip or a TagChip, the cursor's chip lit
  in gold; under them the folder's makeup, its chips by code (the Custom
  screen deals by code), by element, and its Megas and Gigas against the
  folder's limits; then the pack, scrolled to the cursor's chip on its
  side. The top screen keeps BN6's own card of the chip under the
  cursor.
- **The second screen is the PET beside the game** (issue #73). On a 3DS
  and on an Android handheld with a second display, the bottom screen
  wears the PET's frame now, as BN5 DS's and Operate Shooting Star's did:
  the screen's name and the place in its header, HP, Zenny and BugFrags
  under it, and on the net the layer's map, always open. It knows which
  of BN6's screens the player is on, the first step of the second screen
  that follows the game (#72): battles, the folder editor, the
  NaviCustomizer and the PET's other screens get panels of their own
  next.
- **The code keeps its shape.** `build.py lint` now counts the code's
  smells against `tests/lint/smells.txt`, where none may join and none
  may grow: a file past 1000 lines (a header past 400), a `.c` file
  reaching into more than 20 of the game's headers, a function taking
  more than 7 parameters or indented past 5 levels, a block of 10 lines
  repeated, and lines a character says outside a words file (`*_words.c`,
  `*_lines.c`, `*_text.c`). Every header must compile on its own and come
  first in its own `.c` file. AGENTS.md's Code structure says why.
- **Every line a character says lives in a words file.** The director,
  which ran the run in one 5095-line file with its dialogue among its
  logic, is fourteen files of one domain each (`director_*.c`), what they
  say in seven words files beside them; the layer's objects, its chats,
  the guardians, the NaviCust, the towns, the Crosses, the BN5 wait and
  the run summary likewise. docs/VOICE.md's "Where the lines live" names the file for
  each speaker, so a voice pass reads every line in one place. Nothing a
  player sees or hears changed: the same scenarios gave the same pictures
  and the same words, line for line, before and after each step.
- **The longest files are split by what they do.** The layer generator
  (2059 lines) is its driver and three parts: the detours, the landmarks,
  where the set pieces stand. The tile picker (1599) is its checks, what
  it learns from the original maps and how it picks; the map maker (1288)
  its course, what it learns, what it pastes and the set pieces' cells.
  The entry point (1381) keeps the command line and the frame loop; the
  start's folder and ROMs, the 3DS's own start, the screens without a ROM,
  the headless runs' input and pictures and the dev tools have files of
  their own. Two files stay past 1000 lines, listed in
  `tests/lint/smells.txt`: the platform and the older net's core. The
  same layers, maps, pictures and logs came out before and after.

## 0.9.0 (2026-10-05)

- **Everyone talks like BN6 now.** A player wrote that our dialogue felt
  like "reading a summary of a regular sentence", and it did: our boxes
  held nine to eleven words, packed facts together with colons and rarely
  showed a feeling. BN6's whole script, read character by character
  (docs/VOICE.md), holds about six words a box and leads with a reaction.
  Every line was rewritten to it: MegaMan teases and worries and calls Lan
  by name, Lan answers and decides, Mr.Prog speaks in capitals, Chaud in
  "Hmph."s, the bystanders gossip, and each guardian has his habit of
  speech, ChargeMan's "Choo,choo!", SpoutMan's "drip", DiveMan's
  "Awooga!", EraseMan's cackle. The same facts are said, one to a box,
  and written as BN6 writes them ("Lan,look!", Mr.Prog, the Net, Buster,
  the LevBus).
- **No freeze entering BN5's areas.** BN5's first boot froze the game
  for about five seconds on a handheld (3.4 on a fast computer) as a run's
  first BN5 area was made. It now begins as the game starts and runs in
  the background: on a thread of its own on every native build (desktop,
  Android, iOS, PortMaster), in each frame's spare time in the browser,
  never in a frame's way. Layers are made and runs play the same whether
  it has finished or not, and once done it is kept, so it runs once.
- **The older net's wait.** Where a BN5 battle comes before the boot is
  done (a fast player, a slow device, a phone's browser), it waits on the
  battle switch's white behind a screen in BN6's own look: MegaMan in
  BN6's chat box saying the older net's system is still starting up, and
  that it only has to once; a gauge in the Custom gauge's place and form
  filling as it goes, its percent beside it in the HP box's; the net's
  light streaming past. It opens only where the wait needs it and closes
  as the battle opens behind it. It replaces the browser's plain "Waking
  the older net..." note.
- **The switch to a BN5 battle breaks the map into blocks as it fades to
  white,** as BN6's own battle switch does; it faded plainly.
- **The frame log names each second's longest frame** (`frame_log = on`
  in `settings.ini`), the hitch an average hides.
- **The controls screen: a controller's buttons, and the keys, set in the
  game, on every build but the 3DS (issue #37).** Select on the title
  screen opens it, the line under the menu naming the button it is on; on
  a phone or tablet with a controller, CONTROLLER in the touch controls'
  menu opens it mid-run. A and B take a preset (as labeled; B on X, left
  of A as on the GBA, which CybeastID asked for on an Xbox pad; or A and B
  swapped), or each of A, B, L, R, Start and Select takes the button or
  key pressed for it, the button it had going to whoever lost theirs.
  Defaults brings them back. Done asks for the new A within ten seconds,
  else the old controls stay, and the screen itself works on the pad's own
  A, B and D-pad, the keyboard and taps whatever is set, so no choice
  locks a player out. Nintendo's pads keep a map of their own (their A
  sits where the others' B does), in `pad.ini` beside `keys.ini`, which
  names the buttons as SDL does and sets the D-pad and sticks by hand.
  Back and Start held still quits, whatever they are set to.
- **Joy-Cons work on Android (issue #37, Pink5G's report).** A left
  Joy-Con has no A, B, X or Y, so SDL 2 made it no controller and the game
  never opened it: SDL sent its Minus on as Escape, which asked to quit,
  its L as nothing, and dropped its stick, while a right one's stick did
  the moving. Both now get a mapping of their own, each its half of one
  controller in a pair and a small one held sideways alone, with A where
  it is marked, and the left one's arrows, which Android leaves without a
  key code, reach the game as its D-pad. Seen in the Android emulator with
  virtual Joy-Cons as Linux reports them: Minus opens the controls screen,
  the stick and the arrows walk Lan through the town, L asks MegaMan the
  way.
- **A controller plugged in or paired while the game runs is picked up,
  and one taken away is let go.** A pad that left kept its place, and one
  coming after it at the same place was never opened.
- **Each start opens with the developer's boot screen and a word from
  MegaMan on where to report bugs.** On white, Saschb2b comes up in two
  pieces on the two tones of a chime, each out of a GBA-style mosaic:
  "Sasch" on the first, "b2b" beside it on the second, and the name
  glides to the middle. The letters are a bold pixel font of our own with
  a light shadow; the chime is "UI Success Chime" by SoundShelfStudio,
  from Pixabay (no Nintendo logo or sound). Then
  MegaMan asks, in the game's chat box, for bugs and ideas on the
  project's GitHub page, whose address stands above him in letters and as
  a QR code a phone's camera takes from the screen, beside what helps: a
  star, a follow, a report. Then the title. About six seconds in all, and
  any button skips each part; a scripted or headless start (`--scene`,
  play.py) still begins at the title, and `--scene intro` captures the
  start.
- **The AYN Thor's lower screen keeps the layer's map open,** as the 3DS's
  bottom screen does, and so does a second display beside any Android
  handheld's: the floor MegaMan has seen, the way on, what he has come
  near or senses. The game draws it every fifth frame at a size the
  screen shows whole (413 x 360 at 3x on the Thor's 1240 x 1080, black
  round it), and that screen never takes the focus: the controller, Back
  and the touch controls stay with the game, and a touch on the map does
  nothing. The map leaves with the game for the background, giving the
  lower screen its launcher back, and follows a display plugged in or
  out. A phone without a second display plays as before; from Android 12
  a TV on HDMI goes on mirroring the game. Tried in the emulator with a
  display the size of the Thor's lower screen, not yet on a Thor: reports
  are welcome.
- **The 3DS's bottom screen goes black when a run ends,** as the title
  keeps it; it had kept the run's last map.
- **What the project has mapped of BN6 and BN5, open for anyone to use.**
  docs/symbols holds, per ROM, a symbol file that mGBA's and no$gba's
  debuggers load, and the same symbols as JSON and CSV: every address the
  engine uses (RAM, the routines it calls and hooks, the tables it reads,
  the fields of the game's structures, event flags), each with what it is
  and how it was verified, all from the project's own headers and notes.
  `tools/bn6f_match.py` locates the functions of the bn6f disassembly,
  made from Cybeast Falzar, in Cybeast Gregar by their bytes: 13,386 of
  its 13,646 (98.1%), and 4,329 (31.7%) in BN5 Team Colonel, which has no
  public disassembly. bn6f carries no license, so its names are not
  redistributed: `build.py symbols --bn6f DIR` makes that full map from a
  local bn6f checkout, kept out of git. docs/SYMBOLS.md says how it was
  all found and checked, how to load the files and what may be reused;
  `build.py lint` keeps them in step with the engine's headers.
- **BN6's own DarkChips play in BN6's battles, on every build (issue
  #70).** BN6 keeps four of BN5's DarkChips with their code: DrkSword,
  DarkThnd, DrkRecov and DarkInvs. On the middle layer of every act whose
  battles are BN6's, a purple flame of darkness holds one the run lacks,
  on the 3DS and without BN5's ROM too. Taken, it goes to the Pack, and
  EDIT puts it in the folder: three DarkChips at most, as BN6's own rule
  has it, MegaMan saying so in his own words. Each use burns a BugFrag for
  its dark power (a 400 cut over the six panels ahead, a 200 Thunder that
  paralyzes, 1000 HP back, or eight seconds where nothing touches MegaMan
  and the darkness fights for him), and bugs him for the rest of the
  battle as a NaviCust bug does; with no BugFrag it is only its base chip,
  and the Custom screen says so: DrkSword 80 over Sword's picture. Every
  battle its dark power runs in costs 20 max HP, as BN5's battles take
  it. A bystander on the flame's layer says BN6's own words on them.
- **A DarkChip is one chip in both nets.** BN5's flame of one of those four
  says our net plays it too, and BN6's chip of it goes to the Pack. In
  BN5's battles, a DarkChip of BN6's folder comes by BN5's own rule, when
  MegaMan worries, first among the run's three, never twice.
- **A CONTINUE on a flame's layer finds the flame the layer was made
  with,** and one taken before the save stays taken: by its code a
  CONTINUE after the PET's Save there would have offered, and given, the
  next DarkChip too.
- **A Recovery Mr. Prog patches MegaMan once on his layer: half of his
  max HP, and to full before the arena** (issue #71, from ChaseThe3nd's
  proposal in discussion #5). He healed fully as often as asked, so every
  detour's HP came back at the next Prog and the Net Dealer's MiniEnrg had
  little use. Asked again, he says his patch is spent and points to the
  dealer's MiniEnrg; L and the arrow no longer lead to him. A won guardian
  still heals fully, and the Heals helper keeps full heals as often as
  asked.
- **BN5's guardians are fought at three quarters of BN6's guardians' HP.**
  They fight in BN5's engine, where the run's Cross never comes and the
  buster is the NaviCust's alone: KnightMan at 600 HP, behind armor that
  turns every blow but while he swings or leaps, outlasted a playtester's
  whole first act. He now comes at 450 in act 1, and the Net Dealer's word
  on the act's layers names his opening.
- **The DarkChip's words say when it comes, every time,** not only a
  profile's first: when MegaMan is worried, down to a quarter of his HP or
  hit again and again, his face showing it. The question to take it
  starts on Yes, as holding one costs nothing till it is used. And if he
  falls after using one, BN5's darkness may get him up and fight with his
  body a while: MegaMan says it might at the flame, and says what it was
  after a battle where it did.
- **Each run names its chips that sit out** of the older net's battles on
  its first such layer, after a profile's first time; a playtester's new
  run's CrakShot and Atk+10 went unnamed.
- **The run summary no longer shows "BN5 found" over its first lines.**
- **The browser plays its sound sooner, played by keyboard.** Its menus'
  sounds came late: the page kept the handhelds' longer sound buffer (1024
  samples, which the browser's script processor plays a buffer behind). A
  page played by keyboard now takes the desktop's half (512 samples),
  about 10 to 20 ms sooner; the GBA core's ring stays at 32 ms there, as
  two of the shorter reads can fall between two frames of the page's one
  thread. A phone's page keeps the longer buffer. Measured in Chrome on
  Linux, the browser and the system add about 75 ms of their own (64 ms
  of it the system's output), which no page can shorten.
- **BN5's own Navis guard its areas, fought in BN5's engine (issue
  #69).** Where BN5 dresses an area, its guardian was always one of BN6's.
  Now, in half the runs, BN5's own Navi waits in the arena where BN5 sets
  him roaming: KnightMan in ACDC Area, ShadowMan in Oran Area,
  TomahawkMan in SciLab, NumberMan or ToadMan in End Area, Colonel in its
  Undernet. He stands in his own BN5 sprite, speaks with his own face, and
  his battle is BN5's, from BN5's own record for him at the version the
  act's band takes, his HP capped at the band's top. His Guardian Data
  gives what BN6's guardians' do, and in place of a Navi chip BN6 lacks, a
  chip of his kind: KnightMan's JustcOne, ToadMan's BblWrap. Nebula Area
  keeps BN6's guardians. A run without BN5's ROM meets BN6's, as before.
- **A BN5 guardian beaten gives his Soul for the run's BN5 battles (issue
  #69).** His Guardian Data says what it does: from then on, in BN5's
  battles, pick a chip of his Soul's kind and UNITE on the Custom screen,
  and MegaMan fights with his Soul for a few turns, as BN5 has it: once a
  battle, and only while he is calm. His kind of DarkChip, offered when
  MegaMan is hurt and kept to a calmer turn, unites too: Chaos Unison,
  which costs no max HP, as BN5 counts it no DarkChip used. Every Soul won
  stays to the end of the run, through CONTINUE; none carries over to the
  next run, and on BN6's net they do nothing.
- **Upright on a phone, the picture fills the screen's width.** It was
  drawn at the largest whole scale: 960 pixels wide on a 1080-wide phone,
  with black bars at its sides, small over the touch controls. It now
  fills the width with sharp scaling, as on a 640x480 handheld (each
  pixel's edge a little soft, none wider than the next): 1080x720 there,
  in the Android and iPhone apps and on the browser's page. The touch
  controls under it keep their size; on its side, and where a whole scale
  falls only a few pixels short, the picture stays whole. `screen = whole`
  in `settings.ini` keeps whole pixels.
- **A turned Android phone keeps the game whole.** Turning the phone
  between upright and on its side could leave the game drawn 240 pixels
  wide in a corner, or the screen black till the app was closed (in the
  emulator from the first turn): SDL took Android's new size up on Java's
  own thread, switching the renderer's targets in the middle of the
  game's frame. The game now takes a turn up on its own thread: 54 turns
  in a row in the emulator each laid it out right.
- **BN5's battles keep to the act's band from below too.** BN5 places its
  battles by its story, so from act 2 its early areas fought far under
  the act: ACDC Area's two Mettaurs at 80 HP where BN6's battles start at
  150. A battle in BN5's areas now comes from the act's band, floor and
  cap, as BN6's do: its own viruses taken up their versions (Mettaur to
  Mettaur2, as BN5 has them) until the battle fits, and where the area's
  own still fall short, from BN5's other areas.
- **About half of BN5's chip rewards come in your folder's codes (issue
  #63),** as BN6's do: where both games' chip has one of the folder's
  codes, BN5's own results screen shows it in that code, and that is the
  chip you get.
- **The browser takes BN5 too (issue #68).** The player's page takes
  Battle Network 5: Team Colonel (USA) beside BN6's ROM, both at once or
  BN5 later (**Add BN5 Team Colonel**), checks each by its SHA-1 and keeps
  it in the browser's storage with BN6's; a wrong file is refused with its
  reason ("BN5 Team ProtoMan, not Team Colonel"). BN5's areas then dress
  runs and fight their random battles in BN5's own engine, as on the other
  builds but the 3DS. The page runs BN5's frames in BN6's place, never both
  at once; BN5's first start runs a slice of each frame while the title
  shows, and a battle that comes before it is done waits behind a short
  note, once per browser. **Forget ROM and saves** is now **Forget ROMs
  and saves**.
- **A DarkChip comes when MegaMan is hurt.** BN5 worries him by hits,
  and every battle in its areas opened him calm, so it took seven hits in
  one battle to bring a DarkChip, whatever his HP: a playtester at 10 of
  120 HP never saw his. A battle in BN5's areas now opens as worried as
  his HP says: at a quarter of his HP or less his DarkChip comes as soon
  as his hand is dealt again. MegaMan says so at the flame.
- **An act's card and L say where battles are BN5's.** In BN5's areas the
  act's card reads "Act 1 - older net battles", and L's first words name
  the area "where battles run the older net's way", so a territory is
  known before its first battle (issue #65).
- **The run log records BN5's battles, and what each gave (issue #63).**
  `runlog.txt` in the data folder had a line for every BN6 battle and
  none for a battle in BN5's engine. Each now has one, with its viruses
  by BN5's numbers and the reward as the run got it: `reward Wind *`
  (BN6's Wind, in the Pack), `reward 200 zenny` (for a chip BN6 lacks),
  `reward HP+50`.
- **MegaMan names the chips that sit out of BN5's battles (issue #62).**
  He said "4 of ours didn't exist back then, so they sat out" after the
  first BN5 battle, and a playtester asked which. Arriving where BN5's
  battles are, he now names them before the first one, beside the
  Crosses: "Our CrakShot and Atk+10 didn't exist back then either, so
  they'll sit out." (the Standard folder; the Storm folder's ElcPuls1,
  DolThdr1 and Atk+10), and names them again after it. More than three
  come as two names and how many more.
- **BN5's battles fire your buster as your NaviCust makes it (issue
  #62).** MegaMan went into BN5's battles with BN5's starting buster, 1
  a shot and 10 a charged shot, whatever Attack+1, Speed+1, Charge+1 or
  BustPack the run had installed. Its Attack, Speed and Charge now go in
  as the PET's STATUS shows them: with Attack LV 2, a shot takes 2 from
  BN5's Mettaurs. The NaviCust's other programs still sit out.
- **BN5's battles keep the touch controls' D-pad to four ways (issue
  #60).** BN6 stands on its map while BN5's battle runs, and the D-pad
  took that for the map, where it steers eight ways; in BN6's battles it
  keeps to four. Nor does the map held open with SELECT, or a shake under
  way, carry into BN5's battle.
- **`build.py pacing` lists BN5's battles too (issue #60).** For every act
  whose area BN5's dresses, it reads the records each layer's battles are
  picked from against the act's band, as the director picks them: none
  lies past it.
- **Each battle in BN5's engine has its line in the run log (issue
  #60).** It wrote none: a run ended in one showed only "run over". The
  line names the record, its viruses, their HP and MegaMan's before and
  after.
- **A battle on demand: the `battle` step (issue #61).** A capture's
  `--input "300:battle"` and play.py's `battle` start the layer's next
  random battle as soon as MegaMan is free on its map: on BN5 territory a
  battle in BN5's engine, elsewhere BN6's. play.py's state says a battle
  in BN5's engine runs from its first frame to its last.
- **`--dev god`, `onehit` and `fragile` and the dev menu work in BN5's
  battles (issue #61).** They held only BN6's battle objects; BN5's are
  laid out alike, and the menu opens over its battle too, which holds
  still under it.
- **The autopilot fights BN5's battles (issue #61).** It played them by a
  blind button pattern, which lost the first one of seed 1's ACDC Area at
  0 HP; it now reads BN5's battle as it reads BN6's (the Custom screen's
  hand and cursor, the panels and the attacks on them, the viruses), picks
  the chips that go together, and never a DarkChip. Over seeds 1-12 in
  ACDC Area, End Area and Nebula Area it won 99 of 100, and `weak`, which
  keeps BN5's viruses at 1 HP as it keeps BN6's, all 144 of its own.
- **`CYBERWORLD_EMU_DEBUG=1` follows a battle in BN5's engine (issue
  #61).** Every 30 of its frames a line gives its game mode and sub-mode,
  the battle's phase, MegaMan's HP, the Custom gauge and the battle's
  clock, as BN6's core's line does.
- **With the core on its own thread, a battle in BN5's engine holds BN6
  where it was (issue #60).** BN6's next frame had already been started
  as the battle began, so BN6 walked one frame further into the battle's
  moment, and a run with `CYBERWORLD_EMU_THREAD=1` went another way than
  the same run on one thread. Several frames played at once
  (`--dev speed=4`) stop at the battle too.
- **`tests/test_emu.c` runs two cores side by side (issues #58, #61).**
  BN6's core as `emu_init` makes it, with a hook on it, and a second as
  the guest core is made, on a ROM of the test's own bytes that plays a
  tone: BN6's hook runs in BN6's frames alone, a breakpoint written into
  the guest's code goes to its own board and never to BN6's hooks, each
  one's RAM stays its own, the guest's tone reaches the sound ring BN6's
  silence fills, and its state saves and loads. CI runs it under the
  sanitizers.
- **BN5's areas bring their battles as BN6's areas do.** BN6's chance of
  a battle rises with the walk since the last one, and only entering the
  map a battle returns to set it back: a battle in BN5's engine never
  left BN6's map, so after the first one each came at the walk's top
  chance, every one to three checks. A playtester met eight battles in
  under two layers. The walk now starts over after each.
- **The flame of darkness is named, marked and always stands.** L names
  it among what he senses and says where it burns, and the map marks it
  an Event until its DarkChip is taken. Where a layer's services and data
  left no bystander, the flame had nowhere to stand: it now takes a green
  Mystery Data's place. A playtester walked two layers of ACDC Area and
  never met one.
- **A chip BN5's results screen shows is the chip you get.** Where a
  battle's list of viruses started at an odd address in BN5's ROM, the
  reward rows were fitted for the wrong viruses: a playtester's results
  screen showed MrkCan1 S, a chip BN6 has none of, and it came back as
  200 zenny without a word. Such rewards now show as zenny on BN5's own
  screen, as the others do.
- **The split names BN5's areas as the older net's.** MegaMan says "the
  older net's Oran Area" where BN5's area takes a way's place, and "The
  older net's Oran Area it is!" as you choose: two playtesters could not
  tell which way led into BN5's net.
- **MegaMan says, before BN5's first battle, that the older net had no
  Crosses.** On the first layer a profile reaches whose battles are BN5's,
  he says so once, after his arrival words: a playtester's HeatCross was
  gone in BN5's battle without a word. A BN5 Soul may stand in later.
- **A run continued without BN5's ROM starts its layer again.** A layer
  BN5's area drew is laid out otherwise than the BN6 area's own, so a run
  saved with BN5's ROM beside BN6's and continued without it (on the 3DS
  or in the browser, or with the ROM taken away) put MegaMan's saved
  place on another layer, and the screen stayed black while the music
  played a copy of BN5's song that was no longer there. CONTINUE now
  starts such a layer from its arrival, in BN6's area, as after an update.
- **A layer's Mystery Data hold the same after a CONTINUE.** Each of a
  layer's objects now rolls from its own share of the layer's seed: the
  Chip Trader's prize, the program list and an official Chip Order read
  the Library and the programs found, which grow as you play, and their
  draws moved the rolls after them, so a layer rebuilt by CONTINUE could
  hold other chips in the same Mystery Data. Layers are made anew in this
  build: a CONTINUE starts the layer again from its arrival.
- **A win in BN5's battles counts its viruses.** The run summary said
  "Viruses deleted 0" after battles won in BN5's areas: only BN6's
  battles were counted. A BN5 battle won now adds the viruses it set.
- **L warns of BN6's viruses only where BN6's battles are.** In End
  Area, L said "We may meet StarFish here again", and the battle was
  BN5's Whirlies: BN6's viruses never fight in BN5's areas.
- **Phones take Battle Network 5 too: the first start asks for the folder
  your ROMs are in (issue #66).** Android and iOS took one file, BN6's, so
  BN5's areas never came to a phone. The app now opens only the folder's
  `.gba` files, copies in BN6 Cybeast Gregar and BN5 Team Colonel (USA),
  each checked by its SHA-1, and looks in the folder again at each start,
  so BN5 put there later comes in by itself. Every other `.gba` is named with why it was refused ("BN6
  Cybeast Falzar, not Gregar", "BN5 Team ProtoMan, not Team Colonel"), and
  a zipped one is told to be unzipped. Android keeps apps out of Download
  itself: **Choose the files instead** takes BN6 and BN5 in one picker,
  and the app icon's **ROMs** shortcut opens the page again to add BN5
  later. On iOS, Files' **On My iPhone › Cyberworld** is looked in too.
- **The game opens again on Android after a quit.** Quitting with Back,
  then opening the game while Android still kept it in memory, closed it
  at once: SDL ran the game a second time with the first session's state,
  its quit among it. The game now runs in a process of its own that ends
  with the session.
- **BN5's battles open in half a second, not two and a half.** Its
  battle opens on a plain white screen, which held long enough to read as
  a hang; those frames now run four at a time, unheard, and the field
  fades in as before.
- **The run's summary names its helpers.** A helped run's summary says
  which helpers it had, in a small line over its title ("HELP: HP+ ALL
  \*"), as a helped run was always meant to be named; until now only
  Dad's dive report listed them.
- **A fourth helper, All \*: every chip in \* (issue #18).** For players
  who like the battles but not the hunt for codes: the JACK-IN SETUP's
  Help row has All \* beside HP+, Heals and Gentle. With it every chip of
  the run comes in \*, the wildcard: the starting folder, rewards, Mystery
  Data, the Net Dealers' stock, trades, gifts, vaults and prizes, and the
  chips in BN5's battles. Any five chips go in a hand, and every Program
  Advance forms from its chips in order, the codes-in-a-row ones too
  (three Cannons make GigaCan1, where BN6 itself takes one \* in such a
  row). Off by default and chosen per run; the run's summary and Dad's
  dive report name it, and like the other helpers it counts for unlocks.
- **Once the Guardian Data is taken, the arrow and L lead straight to the
  exit pad.** The way on still went round the panel where the guardian
  had stood: three panels from the new pad, on the same floor, L said
  "the exit lies down and to the left, but the way winds" and the arrow
  pointed up. The way now goes through panels left empty: a guardian's
  once his Guardian Data is taken, and a Mystery Data's once it is
  opened.
- **Beside a walkway's mouth, the arrow shows the way that walks MegaMan
  in.** Standing off a walkway's line at its mouth, the arrow pointed
  across onto the line. Held briefly, that moved him a few steps along
  the edge with the arrow unchanged, so the walkway's own way seemed the
  next thing to try, and it stood him still; held long, it carried him
  past the line and the arrow flipped back. It now points straight left,
  right, up or down, between the two: holding it slides MegaMan along the
  edge onto the line and on into the walkway, as BN6 moves him, and no
  hold, however long, carries him past.
- **CircusMan's battle data says how fast his tent falls:** half a
  second after our panel lights, with no way out once down, so no long
  chip while he crackles on his panel.
- **ProtoMan's no-hit duel forgives the fight's first second.** A virus
  could ram MegaMan as BATTLE START left the screen, losing the rule
  before you could act.
- **A program's reminder stays said across CONTINUE.** MegaMan said all
  of a program left off the board once, but a CONTINUE forgot that he had
  (and which programs had stood on the board), so its whole reminder came
  back each session. Both are now kept with the run's save.
- **The Undernet's copy and the Undernet itself are told apart.** The
  ScrtData's words put the golden gate "in the Undernet", the split's
  sealed dark way led "into the Undernet" and opened after the Secret
  Area: which was inside which? The gate now stands in "the Undernet's
  copy a dark warp leads to", and the split's way goes "into the
  Undernet itself", opened by clearing the Secret Area past the gate.
- **MegaMan's word before ProtoMan's duel fits any HP:** "let's heal up
  first if we're hurt", where "let's be at full HP" was said at full HP.
- **BN5's areas move as they do in BN5.** Where Battle Network 5: Team
  Colonel dresses an area, its layers now stand on that area's own
  background, moving as in BN5: ACDC Area's black net of blue diamonds
  blinking as it drifts, End Area's dark red field of green and red
  diamonds, Nebula Area's flickering static; and their lights and
  emblems pulse in BN5's own colours. They stood on the BN6 area's
  background (Central Area's portholes, Seaside Area's bubbles, Green
  Area's rings), with that area's colour cycles running over BN5's
  colours.
- **Central Area and Green Area draw in their own tiles after a BN5
  run.** Where BN5's ACDC Area or Nebula Area had dressed a run, a later
  run in the same sitting drew Central Area's or Green Area's floors
  from BN5's tiles, a jumble of pieces, until the game was restarted.
- **A Server that holds a Navi says so.** "A strong virus signal! Its
  viruses outclass this layer" held ElementMan SP for a playtester on a
  guardian's layer. A Server's battle is now rolled with its layer, so
  MegaMan names a Navi's signal as one before you choose ("It's
  ElementMan SP!", or "One we've never battled"), and it holds the same
  battle after a CONTINUE.
- **BN5's battles keep to the act's difficulty.** Its areas fought their
  own maps' viruses whatever the act: End Area dressing a run's first act
  threw Whirlies at a starting MegaMan, who was deleted in his first
  battle. A BN5 battle is now held to the act's band as BN6's are, from
  the area's own battles where they fit and BN5's other areas' where they
  don't, and BN5's story battles (its roaming Navis) never come.
- **Bystanders keep off the way.** A Navi standing beside a two-wide neck
  before the walkway down to an arena walled it off, and a playtester
  never reached the guardian. Bystanders now never stand by the way from
  a layer's arrival to its exit or arena (146 did over the tests' 300
  layers, none now); CONTINUE starts a saved layer afresh.
- **MegaMan explains the old net's codes.** The first time a chip of
  yours fights in BN5's battles with another code (BN5's chip of the
  name never had yours), Lan notices ("Huh? In there our Thunder S was
  Thunder *!") and MegaMan says why: the old net reads chip codes its own
  way, a wildcard where it can, and out here they're ours again. A chip
  won there that comes back re-coded is named the same way, once.
- **Every chip of the run's folder works in BN5's battles.** A chip whose
  code BN5's chip of the same name never had came into BN5's Custom
  screen as a blank, which could be picked and did nothing (nine of the
  Storm folder's thirty). Each now goes in with a code BN5's chip has: the
  same letter where it has it, else *, so the folder's codes still pick
  together. A chip won there comes back with a code BN6's chip has.
- **An early L or R is kept in BN5's battles,** as in BN6's: pressed up to
  two and a half seconds before the Custom gauge fills, it opens the
  Custom screen as it fills.
- **BN5's orange Navi among the bystanders of ACDC Area and End Area.**
  Where Team Colonel dresses those areas, its orange Navi takes turns
  with its purple HeelNavi as the Navis standing about a layer, as both
  do in BN5's own net, each speaking with its own face.
- **Dark holes in Nebula Area.** About half of Nebula Area's layers show
  one of BN5's dark holes, the purple vortex its Nebula Area stands past
  a platform's edge, past the back of the layer's biggest room.
- **BN5's wall of dark flames in Nebula Area.** On Nebula Area's layers
  the pillar of flames is BN5's own wall of dark flames, standing across
  the walkway. HeatMan's or ChargeMan's Cross clears it as before, and it
  burns out as it does in BN5.
- **BN5's Security Cube on its ACDC Area and End Area layers.** Where
  Team Colonel dresses an area, a layer's security cube is BN5's own: the
  green cube with X eyes that bars the way to a friend's homepage in its
  ACDC Area. It asks the same P-Code or toll as before, and opens as it
  does in BN5, its eyes popping open before it flickers out.
- **Three more of Battle Network 5's net areas join runs.** With Team
  Colonel's ROM beside BN6's, Oran Area turns up in place of Green
  Area, SciLab in place of Sky Area and BN5's Undernet in place of the
  Undernet, each in about half the runs that come there, in BN5's own
  tiles, music, bystanders and battles, as ACDC Area, End Area and
  Nebula Area do. Their maps hold only small octagon hubs, so the
  layers' rooms are drawn as those hubs, framed by their rims; Oran
  Area's raised floors and BN5's Undernet's pale courts come in their
  areas' main colours. The Undernet's statue and braziers stand there
  as in BN6's, for its number doors.
- **BN5's battles stand in front of their own area's background.** A
  random battle in one of BN5's areas showed yellow rings wherever it
  was fought; now ACDC Area's are blue diamonds, Oran Area's red ones,
  the Undernet's static, each area's own as BN5 draws it.
- **The run saves at a guardian arena's door.** As MegaMan steps in, the
  run is saved; a quit during the fight goes on, at CONTINUE, from the
  arena's door with the HP he walked in with, where it went back to the
  layer's start before.
- **L starts over after a CONTINUE.** Its first words are again where
  you are, the guardian, the heal and the way on, then "Press L again to
  hear it!" for the rest. Where L had spoken on the layer before the
  save, the first L after a CONTINUE said the rest at once, a dozen
  boxes, and nothing of where you were.
- **The map marks a layer's locks, and L says where they stand.** A
  security cube, a Link Navi's obstacle or the Undernet's doors show on
  the map (SELECT, and the 3DS's bottom screen) as a violet Event mark,
  like the gates: a ring until you have seen it, gone once it opens.
  Where L senses Mystery Data behind one, he now says which way it is:
  "The cube's straight down, close by: the violet mark on the map." A
  cube you had heard of and had its P-Code for was nowhere on the map.
- **No random battle on the way into a guardian's arena.** From the room
  where MegaMan says the arena is just ahead, across the bridge into it,
  no random battle starts while the guardian waits; one began three
  panels short of the arena, right after MegaMan's "there's no running
  from a guardian!". That room, with its Net Dealer, is the last place
  to get ready.
- **ProtoMan's record on a layer stays the same through a CONTINUE.** His
  squad and his time to beat leaned on the last battles you had fought,
  which a CONTINUE brings back as you quit, so a layer's "pair of viruses
  in 0:12.00" could come back as "three viruses in 0:14.50". Now the
  layer alone decides them.
- **A chip's description ends with one stop** in an official Chip Order
  and a vault: "A piercng thunder attack!", where "!." and ".." had
  come (the spelling is BN6's own).
- **A guardian's Cross data no longer sounds like an offer.** With a
  Cross brought, MegaMan says "We can only carry one Cross down here,
  Lan, so we keep our SlashCross." and that the next dive can start
  with the new one; "It won't fit beside our SlashCross" had read as
  a swap that never came.
- **The Net Dealer says how many RushFood Rush needs.** His word on a
  Rush gap now says Rush only comes when you hold as many as its panels
  ("when you hold 3 RushFood, and he eats one"); only MegaMan had said
  so, at the bone panels, after you might have bought just one.
- **The split says "never battled" once.** Where no way's guardian has
  been battled, MegaMan says "Navis we've never battled guard both ways:
  Aquarium HP and Judge Tree Comp." He had said "a Navi we've never
  battled" for each way.
- **Every "Are you sure?" in a shop starts on Yes, as in BN6.** A visit's
  first purchase had started on No and the rest on Yes, so a LEFT, A
  learned on the first answered No to the next. An A pressed in the
  question's first fifth of a second still does nothing, so one carried
  over from the Net Dealer's words buys nothing.
- **A layer no longer turns black as MegaMan arrives.** A layer with a
  Link Navi obstacle left its talk on the game map it used; a later layer
  on the same map without one kept pointing at it after newer layers had
  written over it, and the game, unpacking it on arrival, wrote over all
  of its memory: a black screen at the exit pad that only a restart left.
  Now such a layer gives the map its own talk back.
- **DarkChips in BN5's areas, at a price.** On the middle layer of an act
  whose battles are BN5's, a purple flame of darkness holds a DarkChip
  (issue #64). MegaMan names its price before you take it: each battle
  one is used in costs him 20 max HP, one HPMemory, for the rest of the
  run. A DarkChip comes to BN5's Custom screen only as BN5 offers it,
  when MegaMan worries; BN6's battles never offer one. Nothing of them
  carries over to the next run.
- **BN5's areas fight BN5's own battles.** Where Battle Network 5: Team
  Colonel sits beside BN6's ROM and dresses an area (ACDC Area, End Area,
  Nebula Area), that area's random battles are BN5's, in BN5's own engine
  on a second core: its viruses and their AI, its battle screens. MegaMan
  goes in with the run's HP and folder, each chip as BN5's chip of the
  same name (a chip BN5 has none of sits out), and comes back with the HP
  the battle left and its reward: a chip as BN6's chip of the same name,
  or zenny. A loss there ends the run. The switch fades to white and
  flashes into BN5's battle, and after the first one MegaMan says what it
  was. Desktop, Android, iOS, PortMaster and browser builds; the 3DS keeps
  BN6's battles.
- **On walkways and bands the arrow points the way they run.** It pointed
  straight across the screen on bands two panels wide and flipped at
  their junctions, and held along a band's line, its way ran MegaMan
  into the corner where the band narrowed. Off a platform's middle it
  now shows the walkways' diagonals alone: along the band, a turn once
  MegaMan can take it, and first across onto the line of a narrower way
  ahead where he stands off it. Inside a platform it still points as the
  crow flies, and from a Mystery Data's corner it leads out first.
- **The program pick says whether each program fits.** After a guardian,
  each program of the NaviCust's draft now says "fits now" or "fits if
  we move one" beside its colour, from your board as it stands; MegaMan
  had said a program would not fit only after you took it.
- **MegaMan names a program left off the board once.** He said the same
  boxes after every battle and in every briefing, even for a program you
  had taken off yourself. Now he says it all once, then a line in L's
  words where it fits as the board stands, and nothing for a program you
  took off.
- **The split names each way's Navi and area,** "CircusMan (Sky Area)";
  the areas were only in the words before.
- **A guardian's HPMemory, chip and heal take two boxes,** where they took
  five.
- **L's first words on a layer are the guardian, the heal and the way
  on.** The rest (the area's viruses, what is here, the map's marks and
  set pieces, ProtoMan) comes at a second L. Those words also say where
  Mystery Data you sense but have not seen lies ("behind the security
  cube"), as the counters count it.
- **CONTINUE says where the run goes on from** ("From the layer's start",
  "From where you saved", "From the Guardian Data", "From where you left
  off"), and keeps a guardian a Navi on the net had named.
- **"Leaving already?" starts on Yes every time.** It started on No where
  it was a shop's first question.
- **Bystanders keep the basics for your first runs,** and the rumor of
  floor that can't be seen is told only where an area hides some.
- **"Pack" with its capital,** as the PET writes it.
- **Falzar's Navis have faces.** SpoutMan, TomahawkMan, TenguMan,
  GroundMan and DustMan, whom Gregar has no portrait of, spoke without a
  face. Each now speaks with a portrait from MegaMan's battle data: his
  own battle sprite, framed as a BN6 face on his title card's colour.
- **"BN5 found" says what it lends:** "BN5 found: its net joins ours".
- **The program vendor keeps one face.** Her shop window showed BN6's
  orange technician navi beside her own face in the chat and her sprite on
  the map; it now shows hers.
- **Every guardian in his own shape.** Seven of the seventeen stood on the
  net as a HeelNavi, "the Nest's copy didn't come out right". BlastMan and
  ElementMan now stand in Gregar's own sprites of them (turned toward the
  camera where they would face away, as Gregar draws them no other way),
  and Falzar's Navis, SpoutMan, TomahawkMan, TenguMan, GroundMan and
  DustMan, whom Gregar has no overworld sprite of, in their battle sprites:
  each warps in as in battle and faces MegaMan.
- **A guardian's battle pays zenny, not a second copy of his chip.** His
  Guardian Data gives his Navi chip, and his battle's own reward, at a
  good Busting Level, gave another, which no folder can hold (one Mega
  chip of a kind): a playtester got two SpoutMan *. Where his battle's
  reward would be his chip, it is now an Unlocker's price in zenny (600 in
  the first act, 900 in the second, and so on).
- **SlipRunr says what it does.** BN6's SlipRunr makes B slide MegaMan in
  place of running, faster, and he keeps going until something stops him,
  past turns and onto pads. The program pick said only "hold B to slide
  along the net", and a playtester slid past his junction and onto a
  layer's exit pad. It now says so.
- **Purple Mystery Data holds a rare chip or better.** Its Unlocker costs
  about a layer's zenny, yet a third of purple data held a common or
  uncommon chip: a playtester carried an Unlocker over two sessions and
  opened CrakShot G, which the Net Dealer sells for less. Purple data now
  holds a rare chip, a Mega chip or, deep in a run, a Giga chip. This
  build makes layers differently, so a CONTINUE starts the layer afresh.
- **L's directions agree with the screen.**
  - "A long way yet" now means twenty-five panels or more. It started at
    fourteen, and a homepage's whole walk to its exit is sixteen to twenty
    panels: a playtester heard "a long way yet" and the layer ended eight
    panels on. (The other areas' walks are thirty to forty-five.)
  - Something close whose walk is long is named close, "though the way
    there winds": "far off" named ProtoMan four panels away on the screen.
  - While MegaMan is hurt and the arrow leads to the Recovery Mr. Prog,
    the arrow turns green, the Heal colour of the map, and L says so. The
    heal's words add "though the way there winds" where the arrow sets off
    another way, and the exit's words after them say where it lies, not
    which way the walk from here starts.
- **ProtoMan's race is named a race.** L said "ProtoMan's waiting for our
  duel!" where Chaud's call sets a race against ProtoMan's time, and a
  playtester skipped it, unsure whether walking up started a fight. L now
  says "our race against his time", and "our netbattle" where ProtoMan
  fights MegaMan himself.
- **A NaviCust bug is named at the RUN.** BN6's RUN lists only errors and
  says "RUN complete!" even over a bug, and MegaMan named the bug only once
  the PET had closed. A red note over the NaviCust screen now names the
  cause the moment you RUN ("A bug! Attack+1 is a plus part on the command
  line..."), and MegaMan still explains it on the map.
- **Green's ladders cross in one line.** Green Area's ladder layers, four
  or five parallel one-panel planks, had their rungs at random, so the way
  turned onto a new rung at almost every plank, a lane to line up each:
  some twenty calls of stops for a playtester. Each ladder now has one line
  of rungs straight across every plank, which the way crosses in one leg,
  and a rung or two more a pair to wander by. Services and navis also keep
  off the way more often. This build makes layers differently, so a
  CONTINUE starts the layer afresh.
- **An Unlocker shows its lock.** While MegaMan holds an Unlocker, the map
  marks every purple Mystery Data on the layer, seen or not, as the Net
  Dealer who sold it said where it lies. A playtester bought one and never
  found the purple data.
- **Rumors tell the folder's Program Advances only (issue #56).** BN6's
  table also holds the BattleChip Gate's recipes, made with the Japan-only
  reader of physical chips, and a navi whispered one: Cannon, Cannon and
  TankCan1 for GigaCannon, where the folder's is Cannon A, B and C. A
  whisper now also names the recipe in the folder's own code where its
  chips come in it: LifeSword in S to a folder of Sword S and WideSwrd S,
  not in H.
- **Where a new chip went.** A chip bought, traded, found in Mystery Data
  or won in battle goes to the Pack, as in BN6, and with a full folder it
  never shows in battle: a first-time playtester's chip, bought for the
  guardian, sat there unknown, and another swapped six found chips in by
  himself before a purchase taught him. The first time a chip comes, from
  anywhere, MegaMan now says where it went and how to swap it in (PET,
  Folder, EDIT).
- **A first-act navi warns about battlefield Mystery Data:** any hit breaks
  it, even yours; win with it whole and it's yours. MegaMan said so only
  after the first battle that had one.
- **One name for the net.** Chaud and ProtoMan said "the Cyberworld" where
  Dad and the BBS say "the Endless Net"; they now say the Endless Net too.
- **The summary's next goal reads "Win a short net: the endless net".**
- **The map marks the Mystery Data you've seen.** The counters counted
  data on panels the map shows, some of it never on the screen, and a
  playtester knew a green and a blue were left with no idea where. The
  map now marks each one not yet taken with a small crystal of its colour.
  Data MegaMan only senses behind a set piece stays unmarked: L names
  those without pointing.
- **+ chips, said right.** MegaMan's word on the first RegUp said a + chip
  like Atk+10 needs an attack picked after it. As in BN6, it adds to the
  attack picked just before it.

## 0.8.0 (2026-10-03)

- **A teleport island's data is in reach.** MegaMan lands on the rim of
  the island's gem, and the island's Mystery Data sat across it: the way
  there stepped onto the gem's trigger and beamed him back, so it was
  reached only round the rim, half over the void. The island is now a
  4x4 with its data straight below where he lands. This build makes
  layers differently, so a CONTINUE starts the layer afresh.
- **Mystery Data counters.** The map (hold SELECT) now counts the
  Mystery Data MegaMan knows of by colour, green, blue and purple: taken,
  of those seen or sensed behind a set piece, dimmed once all are taken.
  L shows the same counters at the top right for a few seconds, so you
  know what's left before the one-way exit pad. They count what you've
  found, never where the rest is: a secret stays a secret until you find
  it.
- **What a given-up run opened is yours to choose.** A guardian deleted
  opens his Cross start (and the Blade folder) at once, but a first run
  given up through NEW GAME jacked straight in without the setup screen,
  so there was nothing to choose them from. The setup now opens whenever
  something is open, marked NEW.
- **The arrow's way turns less.** Across a lattice of walkways the
  arrow's shortest way turned at nearly every crossing, a stop and a new
  direction each. Of the shortest ways it now takes the one that turns
  least: along one walkway and on along the next.
- **TagChips, said right.** MegaMan and the PET's mail said a tagged pair
  had to fit in the Reg memory; BN6's rule is a pair under 60 MB, whatever
  the Reg memory, and they now say so. A first-act navi now says the Net
  Dealers set up shop on an area's first layers, and MegaMan's word on the
  first RegUp adds that a + chip like Atk+10 needs an attack after it.
- **The Endless Net BBS.** A new mail in the PET collects the netizens'
  threads about the net: tips, sightings and the odd myth, with more
  posts the deeper you've been.
- **Schoolyard talk in town.** The townsfolk now trade the old rumors: a
  Giga chip under Central Town's bus (the bus stop knows better), the
  Academy's hunt for Program Advances, Higsby's back room, Mr. Famous's
  boasts, Dr. Wily blamed for everything, and the NaviCust's secret
  buttons.
- **The net's rumors.** One navi on each layer now whispers the way a
  schoolyard did: a Program Advance from BN6's own table, picked for chips
  in your folder ("Sword H, WideSwrd H and LongSwrd H, in that order...
  LifeSrd!"), or a hint at a secret you haven't found yet, never its
  answer. Every rumor is true.
- **Fewer narrow walkways on the way, as BN6's own maps have them.** The
  way across a layer now crosses no more one-panel walkways than its
  area's original maps do: wider ways in Seaside, Green, the Secret Area,
  the Nest and Mr. Weather's comp, where layers had crossed up to three
  times as many. Central keeps its glass bridges: its layers already cross
  fewer than BN6's Central maps. This build makes layers differently, so a
  CONTINUE starts the layer afresh.
- **No long detour leads to nothing.** A branch five panels or more off
  the way that held nothing now ends in a green Mystery Data, moved from
  one lying loose by the way, and a wide band that reads as the way gets
  one of its own. Half the layers had held an empty one. This build makes
  layers differently, so a CONTINUE starts the layer afresh.
- **The map shows where sensed services stand.** On a partly explored
  layer, the shops, heals and others MegaMan senses sat as dots on the
  map's frame, read as standing at its edge. The map now zooms out to show
  them as rings where they stand, and anything still past the frame, the
  exit too, is an arrowhead pointing its way.
- **The battle controls, said right (discussion #53).** Running from a
  battle is L on the Custom screen, not L and R: a navi's tip and the
  README said otherwise. The README now also says that R on the Custom
  screen describes the chip or Cross under the cursor, and that SELECT
  hides the Custom screen to show the field; a first-act navi tells you
  so.
- **CircusMan's tent marker stays on the field (issue #52).** The yellow
  mark on the panel his tent is about to drop on was drawn over the Custom
  screen's chips when it opened. It now hides while the Custom screen
  covers the field, and shows again when you press SELECT to hide the
  window and check the field.
- **A split offers two guardians.** After a guardian, the second way on
  could name the same guardian as the act ahead (SpoutMan for both Green
  Area and Aquarium Comp, about one split in seventy). It now always offers
  another.
- **RegUps and TagChips (issue #51).** Every layer before a guardian now
  hides a RegUp, behind a lock or at the end of a detour, so your Reg
  memory grows from 4 MB to about 20 by the third act. Set a Regular Chip
  in the folder's EDIT with SELECT, and it starts every battle in your
  hand. Win your first duel against ProtoMan, and Chaud's clearance brings
  the TagChip system for good: two tagged chips come to your hand together.
- **Compression codes are kept (issue #50).** A NaviCust compression code
  you enter once is kept for every run after. Dad's new Compression mail
  lists each code you have entered, and when a program won't fit, MegaMan
  names its code if compressing it would make room. You still press the
  ten buttons yourself, and no code shows until you have entered it once.
- **Compressed programs count as compressed (issue #54).** MegaMan judged
  the NaviCust's room by a program's full shape even after its
  compression code was entered, so he called a second compressed Custom1,
  or an UnderSht beside two of them, a program that would not fit. He now
  reads each program's shape as the PET has it, and counts every copy of
  a program already on the board.
- **L tells what the set pieces are (issue #48).** On a layer's first L,
  MegaMan now senses its set pieces along with its services: purple
  Mystery Data, bone panels, teleport pads, arrow panels, obstacles,
  cubes and the Undernet's doors. The first time you meet each kind, he
  says what it is and what opens it.
- **The Undernet's doors (issue #47).** The Undernet now locks its spurs
  with BN6's own doors. A skull door lets only WWW members through: the
  act's Net Dealer sells the WWW-ID, and one opens every skull door of the
  run. A number door asks how many flames of hatred burn on the layer, so
  count the braziers that now line its rooms; answer wrong and it seals
  itself.
- **The Net Dealer's keys always make his list.** With eight places, an
  Unlocker could take the last and push off another key the act needed.
- **Invisible paths (issue #46).** From the second act on, Seaside, Sky,
  the Undernet and the Nest can hide floor drawn as void, as BN6 does:
  from a walkway's tip or a platform's edge straight out to a lonely pad
  with a good Mystery Data on it. A navi nearby tells you he saw someone
  walk out there over nothing.
- **P-Code tellers stay put.** On a layer that also had a Rush island or a
  teleport island, the navi who tells a cube's P-Code could find no place
  to stand, and the cube asked for a toll instead.
- **Arrow lanes (issue #43).** BN6's arrow panels now carry MegaMan across
  a short lane from the end of a long detour back toward the way: walk the
  long way in, ride out. Each area draws its own (Seaside's blue panels,
  Green's magenta blocks, the Undernet's chevrons, the Judge Tree's
  bricks), and as in BN6, walked from the far end they push him back.
- **Security cubes (issue #45).** Comps, homepages and Central now lock a
  pocket with BN6's security cube, its one good Mystery Data behind it.
  The cube wants a P-Code: one navi on the layer knows it, a walk away
  from the cube, so ask around. Seaside's cubes take a toll instead, 200
  zenny in act 1 and 100 more an act.
- **Obstacles and cubes can't be walked round.** The map's floor could
  widen the one-panel walkway an obstacle or a cube stands in, leaving room
  to pass it; their mouths now stay one panel wide.
- **A at a set piece reaches it.** A navi standing a step or two from an
  obstacle, a cube or Rush's bones took the A meant for them; navis now
  keep three panels away.
- **Link Navi obstacles (issue #42).** BN6's cybertrees, flames, cyclones,
  clouds and geysers of cyberwater now stand in the mouths of pockets off
  the way, each the area's own kind, each pocket holding one good Mystery
  Data. The run's Crosses clear them as Gregar's Link Navis do: carrying
  HeatMan's Cross, A at a cybertree and HeatMan burns it away. Without the
  right Cross, MegaMan says whose would clear it, a hint for the next run.
- **No more pads that look like warps and aren't.** A layer's pads wore
  BN6's gem, ring or cube in their middle as decoration, but in BN6 every
  one of them is a warp: the gem a teleport, the cube a homepage's link,
  the ring another area's. Pads now wear them only where they warp.
- **Teleports (issue #44).** In Green, Sky and Central, a layer can hold a
  pair of BN6's gem pads: step on one and MegaMan beams to the other, the
  camera scrolling along. Either a quick way back from the end of a long
  detour to the way, or the only way to a pad of its own out in the void,
  with a blue Mystery Data on it.
- **Rush's bone gaps (issue #14).** As in BN6's net, a layer can hold a
  gap of one to three panels with bones over it, a walkway aimed across
  it at a pad of its own with a blue Mystery Data on it. Hold as many
  RushFood as the gap's panels and press A at the edge: Rush comes, eats
  one and lies in the gap for good, and MegaMan walks across, BN6's own
  cutscene and all. The Net Dealer sells RushFood where the act holds a
  gap and says where it is, and MegaMan names the bones when he first
  comes near them. Sky's gaps run longest.
- **Purple Mystery Data.** As in BN6, a layer can hold one purple data,
  locked: an Unlocker opens it, and it holds a chip of the rarest kinds,
  in a code, that no dealer sells (or the run's Spin, where it lies on that
  layer). It stands at the landmark's foot or where the longest detour
  ends, on most of an act's middle layers. The Net Dealer stocks an
  Unlocker for each purple data ahead in the act, at about a layer's
  zenny, and tells you where it lies; now and then a blue data on the same
  layer holds the key (epic #49, issue #41).
- **Mystery Data say what the walk is worth.** Blue data now lie where the
  detours end: at the far end of a spur, on its pad, in the room farthest
  off the way, the best of them where the walk is longest. Green data lie
  loose. Before, a Mystery Data's worth was rolled wherever it landed, a
  quarter of the good ones right beside the way, and all were drawn green.
  About as much is found in all (epic #49, issue #40).
- **Every navi on a layer shows.** BN6 runs at most sixteen NPCs on a map
  (Mystery Data, services, gates and bystanders each take one) and leaves
  the rest of a map's list out without a word. One layer in sixteen held
  more: its last bystanders never appeared, and on one in two hundred an
  official gate or ProtoMan's duel was missing. A layer now places its
  Mystery Data and bystanders only in the places the game will run. A
  run saved by 0.7.0 continues its current layer afresh.
- **iPhone and iPad: SideStore, not AltStore PAL.** In the EU, the AltStore
  from Apple's App Marketplace (AltStore PAL) installs only apps Apple has
  notarized, and refused the source ("missing a marketplaceID"). The
  download page, the README and the FAQ now lead with SideStore, set up
  once from a Windows, macOS or Linux computer by iloader, and name
  AltStore Classic (with AltServer on a Windows PC or a Mac) beside it.

## 0.7.0 (2026-10-02)

- **The game on iPhone and iPad.** It comes through AltStore Classic or
  SideStore, which install it with your own Apple ID and keep it up to
  date: add the source the download page links, install Cyberworld
  Endless, and choose your ROM in Files when it asks (or put it in Files,
  On My iPhone › Cyberworld, beside your saves). Touch controls round the
  picture with a tick under the thumb, clear of the notch and the home
  indicator, or a controller; turning the phone moves them under the
  picture or beside it. Sent to the background, it keeps your run where
  MegaMan stands. iOS 14 and newer. It was built and started in the iOS
  Simulator, not yet on an iPhone: reports are welcome.
- **The download page shows every system at once.** Nine tabs showed one
  system at a time; on a phone they took four rows and pushed the first
  download under the browser's toolbar. The page now leads with the best
  pick for your device, its steps open, and lists every system below in
  four groups (computer, phone and tablet, handheld and console, no
  install), each with its main file, its other formats and how to install
  it. The PortMaster port is named for every firmware that runs PortMaster
  (AmberELEC, ArkOS, Knulli, muOS, ROCKNIX and others), not ROCKNIX alone:
  its file is now `cyberworld-endless-portmaster.zip`.
- **The Net Dealers name every guardian's weakness (issue #39).** They
  said SlashMan, EraseMan, TenguMan, GroundMan and DustMan had no weak
  element, but BN6 has a second wheel, as its Cross lessons teach:
  SlashMan can't stand Breaker chips, EraseMan Wind, TenguMan Sword,
  GroundMan and DustMan Cursor (a Sword's 80 takes 160 of TenguMan's
  HP). The dealer now says so and stocks his pick of that kind: a sword
  for TenguMan, who hovers right in front between his dashes, but
  otherwise never a chip that strikes only beside MegaMan (WindRack and
  MoonBld joined the swords there). The way on after a guardian names
  their elements too: TenguMan (Wind).
  Every other guardian was right, checked against the ROM and the
  Rockman EXE Zone wiki; ElementMan is now said to change his element as
  he fights. The viruses' weakness a dealer names counts each kind as
  the area's battles field it (Swordy2 is Fire). A run saved by an
  earlier build starts its current layer afresh.
- **Pressing L no longer changes the battles to come.** On an act's
  first layer in acts 3 and 5, MegaMan's status, naming the viruses the
  area may hold, drew one of the run's random numbers each time it
  opened. It now reads every version the act's battles can roll, and
  draws none.
- **The GBA core rests while the game waits for the screen (issue
  #35).** BN6 waits for each frame by reading the screen's status over
  and over, half of the core's work on a layer and more in battle. The
  core now sleeps through that wait until the frame begins, woken as the
  GBA is by the frame's own interrupt, and the game runs as before. On a
  PC a frame of the game takes about 30% less time (a layer's 0.52 ms
  became 0.36, a battle's 0.53 became 0.35), on a Retroid Nova a
  quarter less (3.98 ms became 3.09), and on a New 3DS, where it took
  nearly all of each frame, 15.9 ms became 11.0.
- **The New 3DS shows its frames evenly (issue #35).** Each frame now
  waits for the screen's refresh: with the core faster, frames paced by
  the game's own clock came early or late 14 times a second, and with
  0.6.0 twice; now about once.
- **The engine hears from the game as things happen (issues #27-#34).**
  It used to look over the game's memory every frame for what had
  changed, and to patch hand-written code into the game to call it.
  Breakpoints in the core's copy of the game now run the engine's own
  code at the moment: a battle starting, its reward chosen, MegaMan hit
  in the rival's duel, ProtoMan's HP as he appears, a map entered, a
  choice taken, a key item given, the PET's Save, and every call into the
  game, warps among them. How the game plays is unchanged: seeded runs
  give the same logs line for line, and a run saved by 0.6.0 continues.

## 0.6.0 (2026-10-02)

- **MegaMan speaks of NaviCust bugs only when the board changes (issue
  #25).** After every trade at a Chip Trader, a player heard MegaMan say
  the NaviCust ran clean, then explain its bug all over again. BN6 counts
  the bugs anew whenever a screen like the trade's opens and closes, and
  clears them with BugStop on the board; MegaMan took each change of the
  count for news. He now names a bug only when the board itself
  changed: a RUN in the NaviCust, or an ExpMemry. And he waits for the
  trader to finish: a talk of his begun as the trade screen closed took
  the trader's box, and "Try again?" could no longer reach No.
- **A layer's way across runs wide, as Capcom's maps do.** MegaMan walks
  as in BN6, so every one-wide walkway entered from a platform takes
  lining up, and a playtester spent a third of his inputs on it. BN6's
  own net maps, measured, carry their way across over wide floor,
  crossing about one one-wide walkway between big platforms, their many
  other walkways spurs off it; ours crossed two, up to four. The way from
  the arrival to the guardian or the exit now keeps to its area's own
  count (one in most areas, three in the Graveyard): past it, its
  walkways widen to two panels. The guardian's bridge stays his gate. A
  run saved by an older build starts its layer afresh.
- **The Undernet's bridges run plain between their joints.** Every other
  panel of a bridge's straight runs showed a piece of the yellow gem its
  joints wear, cut off at the tiles' edges. Its joints now keep a look of
  their own, at a bridge's ends, turns and crossings as the original's,
  and the striped planks between them run plain; the Secret Area's
  bridges too.
- **The Undernet's guardian arenas are plateaus.** The Undernet's and the
  Secret Area's arenas were paved with their bridges' gem joints and
  striped planks, a jumble a guardian stood in. They are now drawn in the
  area's own stone, as its plateaus are; the layers are laid out as
  before.
- **The game says when it has found Battle Network 5.** Nothing told a
  player whether Team Colonel beside the BN6 ROM had been read until
  one of its areas turned up, in about half the runs. As the title comes
  up, a note in its top right corner now says "BN5 found" for a few
  seconds, in the box the map's "Run saved" uses.
- **The picture fills a 640x480 screen (issue #36).** On an RG35XX Pro
  the game's picture took 480x320 of the screen's 640x480, half of it:
  scaled by whole numbers, 3x no longer fits. Where a whole number leaves
  the picture a quarter smaller or more than the screen allows, it now
  fills the screen (640x427 there), each pixel's edge a little soft.
  `screen = whole` in `settings.ini` keeps whole pixels, `screen = fill`
  fills every screen; the Nova, the Flip 2 and most screens stay whole as
  before. The run summary's "Dad's backup got MegaMan home." now fits a
  screen only the game's width wide.
- **Bystanders don't repeat themselves across an act.** Each layer's
  Navis took their lines from a random place in the pool, and a player
  heard "The exit pad only goes down" on an act's first and third
  layers. Each layer now takes the next lines on from the last.
- **MegaMan warns that SpoutMan's whirl reaches his corners.** Landed on
  MegaMan's side, SpoutMan lights only the panels at his sides as he
  whirls, but his arms sweep the panels at his corners too, and a player
  who stepped off every lit panel was deleted on one of those. MegaMan's
  briefing now says so: get two panels away from him.
- **Battle Network 5's Nebula Area looks like itself.** Its rooms came
  out with its emblem repeated across them and its paths half platform:
  their cobbles change colour stone by stone, which the tiles had read as
  two floors. It now tells paths from platforms by shape, as BN5 draws
  it: small purple platforms with one emblem each, on long cobbled paths.
  SciLab sits out of runs for now: its maps hold no platform bigger than
  a small pad, and Sky Area's rooms drawn in it still look rough.
- **Battle Network 5 stays after the first start.** A BN6 ROM chosen in
  the desktop's dialog, or found in the Downloads or a ROMs folder, is
  copied where the next start looks, but a BN5 ROM beside it was not, so
  its areas turned up in the first session only. It is now copied along
  (Linux, macOS, Windows).
- **Mr. Weather Comp's fields are whole solar panels.** Pieces of its
  fan belts turned up where a field's edge cut them: half domes at the
  ends of its bands, a fan cluster sliced off at the guardian's arena.
  Its fields now carry their solar panels and lights alone.
- **A one-time pick says what the chip does.** A vault's and an official
  Chip Order's three chips showed names and damage only, and R did
  nothing there. Picking one now shows BN6's own description of it
  ("Colonel *: Cut enmy lines in Z shape.") before "Take it? We only get
  one!", where No goes back to the three.
- **A Navi chip's EX and SP versions show their damage.** A vault
  offered "DiveManSP D 1013": the ROM keeps a Navi chip's power as 1000
  and a tenth of it, and only the first version was read so. Its SP now
  shows 130.
- **The Net Dealer no longer denies the net's rumors.** On an act's first
  layer he said there was "no word yet" on its guardian, two platforms
  after a Navi had named him. He now says nobody has come back to tell,
  and that he doesn't sell on talk.
- **A Cross's weakness says it breaks the Cross.** The setup screen and
  the lab's mail said only that its weak element did double damage; a
  hit of it also turns MegaMan back, as in BN6.
- **Fields look like the originals' fields.** Inside a platform every
  tile took its spot's most common look, so Mr. Weather Comp's solar
  panels ran as plain bands without their lights, and the Cybeast Nest's
  floors lost their cracks. A field's middle is now laid as the original
  lays it: by its repeating pattern where it has one, else a whole
  stretch of the original field repeated, lights and cracks in place.
- **No more grey cubes floating in the Cybeast Nest's void.** Its one
  piece of scenery was debris from under the original's altar, set
  alone beside the floor; the Nest now sets none.
- **The Undernet's raised rooms are plain.** Their floors showed pieces
  of green crosses cut off at the tiles' edges, from the decorated middle
  of the original's raised court. Green panels' tiles are no longer used
  for its floors.
- **Seaside's boardwalks lose their broken arrows.** Where its yellow
  boardwalks joined the blue field or a platform, pink and orange
  triangles cut off at the tiles' edges stood in the floor: pieces of the
  arrow panels on the original's path up to its big platform. Those
  panels' tiles are no longer used for anything else.
- **Sky Area looks like Sky Area.** Its catwalks wore the thick lavender
  edges of its fields, orange lights and all, and its rooms the frames of
  its round pads, pink corners and rounded caps. Its rooms now take the
  original's fields of framed squares, its guardian's arena among them,
  and its catwalks their own thin glass edges with the clips under them.
- **Sky Area's catwalks turn cleanly.** Every bend of its cyan glass
  catwalks had lavender wall pieces from its other catwalks in it, and a
  walkway onto a purple pad was a jumble of both. Its walkways are now
  drawn from the cyan glass catwalks alone.
- **The Graveyard's and Mr. Weather Comp's edges run straight.** The
  Graveyard's slabs had a sawtooth edge, every panel poking out past the
  rim with a pale post in each notch, and holes shaped like slots;
  Weather's fields hung stepped blocks and grilles under their edges.
  Their edges are now drawn with their own rim tiles, as the original
  maps draw them.
- **Guardian arenas look like their areas.** Mr. Weather Comp's, Robot
  Control's, Central Area's and Green Area's arenas were paved with the
  walkways' tiles, lamps, circuit lines, catwalk posts and planks made for
  a path a panel wide, which tiled over a big field overlapped into a
  jumble. The arena is now one of the area's own platforms: Weather's
  solar-panel floor, Robot Control's white slab, Central's green field,
  Green's grass.
- **Stairs meet their floors cleanly.** A stair's tiles came with pieces
  of the original map around it: void notches and loose stripes in the
  floor at its foot, a bridge pad's yellow gem, the original court's
  crosses on the raised room, slivers floating beside Sky's stairs. A
  stair now brings its own ramp and side faces alone. Players had seen
  the Undernet's ramps "bugged".
- **MegaMan says when a new program needs room made first.** A player's
  Guardian Data said "Let's install it", and Custom1 fitted the NaviCust
  only after he moved two programs, which nothing had said. A program
  that fits the board only once others move now gets MegaMan's word as
  it comes into the PET, and his reminder of a program left off the
  board says the same.
- **The official gates name the clearance they take.** A Chip Order said
  "Chaud's clearance opens it!" a minute after Chaud had said his full
  clearance was still to win, and a player wondered which he had. The
  gates now say "Chaud's first clearance" or "Chaud's full clearance",
  and Chaud, giving the first, says it opens the official Chip Orders.
- **A one-time pick asks once more.** An official Chip Order's and a
  vault's three chips gave the first one to an A pressed through the
  words before them, for good. Picking one now asks "Order TrplShot J?
  We only get one!", starting on No, which goes back to the three.
- **At a walkway's corner the arrow points down the next walkway.** A run
  down a walkway stops MegaMan a little past its corner's middle, and from
  there the arrow pointed back to the middle, "straight up", though he
  could already take the next walkway: a player spent 15 moves at such
  corners. It now points down the next walkway, and where MegaMan does
  stand too far off a walkway's line to enter it, straight across to it.
- **A held B no longer hurries a talk past unseen.** Running with B held
  into a talk that opens on its own, such as the words before a guardian's
  arena, let BN6 page through it before a player saw a box. The talk now
  waits until B is let go; a fresh B still hurries it.
- **An area's virus warnings say "may".** "StarFish here again" named a
  family the area's battles can bring, and a player met none in a whole
  act. MegaMan now says "We may meet StarFish here again".
- **What a guardian fight holds is said before it.** A player lost a run
  to Colonel without knowing three things: his hits went nowhere while
  he readied a slash, his cape swept the row he landed in with only his
  own panel lit, and there was no running at 1 HP. Colonel's battle data
  now names both moves, the room before every arena says there's no
  running from a guardian, and EraseCross's drain on a Navi is said to
  be slow.
- **No battle under a guardian's staging (issues #24, #38).** Running
  into a guardian's arena could roll a random battle as the staging
  began, and the battle was fought between its black bars. The staging's
  walk to the arena's middle rolls no battle now, nor does the step into
  the arena, and the bars never cover a battle.
- **Stairs lead up onto their raised rooms (issue #22).** At a stair's
  top MegaMan dropped to the ground and walked on under the raised room,
  out of bounds: a player reported it on an Undernet layer, and it held
  on every area's stairs. The row past each stair's top is now the step
  the originals have, from the ground to the room's height.
- **MegaMan says where the golden gate stands.** Three ScrtData open the
  gate to the Secret Area, MegaMan said, and a player holding three asked
  where it was: only a bystander's rumour had placed it in the Undernet.
  MegaMan now says so as he picks one up and in his briefing, and with
  three, that the next dark warp leads there.
- **Battle Network 5's areas fight to BN5's battle music.** Its net
  areas played its own net theme, then every battle there played
  BN6's. Their random battles now play BN5's virus battle theme, and
  their guardians and ProtoMan's netbattle its boss theme, copied from
  BN5's ROM as its net themes are. Behind them stand BN5's own battle
  backgrounds: ACDC Area's blue diamonds, End Area's red field, Nebula
  Area's stars.
- **The arrow leads a hurt MegaMan to the heal.** Below three quarters of
  his HP, L said which way the layer's Recovery Mr. Prog was, but the
  arrow kept to the exit, and a player at 180 HP spent a dozen moves
  finding the way onto its platform. The arrow now leads there first,
  and L says so; healed, it goes back to the way on.
- **ProtoMan's netbattle can be won.** In the third act he came in at
  1000 HP with his full attacks, and a player ran after one hand. He now
  has half the act's guardian band's top (500 in act 3), and MegaMan
  says how he fights as the netbattle is offered: his shield stops shots
  but not chips that lock on, he dashes in to slash the panels that light
  up, and a lone lit panel under MegaMan means his WideSword takes that
  whole column.
- **An act continued from a save keeps its count.** Its AREA CLEAR card
  left out the viruses deleted (the count began again with the
  CONTINUE), and its next Net Dealer greeted MegaMan as if they had never
  met. Each save now keeps them beside the run.
- **CONTINUE doesn't greet a saved ScrtData as a new find.** A player
  holding one heard "Our second ScrtData! One more, and the golden gate
  opens!" as his run came back. MegaMan's words now come for a new one
  only.
- **The way on names a never-met guardian as ???.** After a guardian,
  the next ones were offered by name where MegaMan knew them ("EraseMan
  (Wood)") and by their area where he didn't ("Aquarium Comp"), which a
  player read as a place against a Navi. A never-met one is now "???
  (Aquarium)", as the act card names him.
- **MegaMan warns that StarFish soak up shots.** Their bubbles stop
  straight shots and trap MegaMan, which BN6 never says, and a player
  lost 350 HP to one while three Cannons went into its bubbles. Once
  StarFish have been fought in any run, MegaMan says so as an act or a
  side layer where they live begins: a chip that drops from above gets
  past them.
- **MegaMan says which program bugs the NaviCust, and why.** After a RUN
  with a bug he listed every rule a bug can come from, and a player had
  to work out which one he had broken. He now names it from the board:
  "HP+100 is a plus part on the command line: plus parts go anywhere
  else", a program off the command line, a part past the board's edge,
  or two programs of one colour side by side.
- **The map hands over to the arrow.** Letting go of SELECT now shows
  the way-on arrow, as L does, where it had faded half a minute after
  MegaMan's words: a player who looked at the map on a strip with no
  arrow tried one direction after another to find its walkway.
- **The way-on arrow turns as MegaMan stops.** It turned only after a
  new way had held for two looks five frames apart, so a picture taken
  just after a step still showed the way from before it: at a walkway's
  mouth, the arrow pointed back the way MegaMan came. Standing, the way
  can't flip, and the arrow now turns within a frame or two.
- **The Guardian Data's programs name their colour.** A program comes in
  more than one colour, and a player read the draft's pink HP+100 and a
  vendor's blue one as a contradiction. MegaMan's line for each now says
  it: "HP+100 (pink): a hundred more max HP."
- **EraseCross's bug on a Navi is said.** The setup said only what it
  does to viruses, and a player saw BlastMan's HP drain with nothing in
  the game saying why. MegaMan's briefing before a guardian now says it
  while the run has EraseCross ("a plain chip that hits him while his HP
  has a 4 in it bugs him: his HP drains away!"), and so does the PET's
  report of the dive.
- **Battle Network 5's areas in a run.** With Team Colonel (USA) in the
  same folder as your BN6 ROM, three of BN5's net areas stand in for the
  BN6 areas they resemble, each in about half the runs that come there:
  ACDC Area for Central Area ("The net of Lan's old hometown"), End Area
  for Seaside Area and Nebula Area for the Graveyard,
  in their own tiles and colours, under their own names, to their own
  music (BN5's net theme; Nebula Area its Undernet's), with BN5's purple
  HeelNavis as their bystanders, face and all. Their layouts,
  guardians and battles stay the BN6 areas'. BN6 alone plays
  as before. Not on the 3DS, which has no memory for a second ROM, or in
  the browser.
- **CircusMan's tent shows where it drops.** As his tent comes, BN6
  lights the panel MegaMan stands on for a few frames, under his feet,
  where no one can see it, and a playtester lost to him five times. A
  step off that panel within half a second clears the tent; after that
  MegaMan is held in it, and CircusMan's fade, which the briefing named,
  shows only a tenth of a second before. The panel is now marked in
  warning yellow over MegaMan for that half second, and his briefing
  names it: "When the panel under us lights up, his tent is about to
  drop on it: step off at once!"

## 0.5.3 (2026-10-01)

- **A duel with ProtoMan counts once.** Quitting after a duel and
  continuing met ProtoMan again on that layer, and each win counted anew
  in the record, as a player reported (issue #20): the run continues
  from the layer's checkpoint, made before the duel. A CONTINUE now finds
  the duel fought: ProtoMan gone, and the official gate beside him open
  where the duel was won.
- **The BugFrag Trader no longer trades for nothing.** With A pressed
  through "Try again?", it went on giving chips at 0 BugFrags, as a
  player reported (issue #21): the game's own "not enough" line waits as
  its Yes does, and was taken for a Yes. The ten BugFrags are now taken
  before the chip is given, and where they aren't there the talk ends
  after that line.

## 0.5.2 (2026-10-01)

- **The 3DS's bottom-screen map no longer stutters on the net.** The
  map read MegaMan's place from the game as it was drawn, and on a New
  3DS the next frame is already running on another core then: each read
  waited for it, 14 ms at every redraw, a frame lost six times a second.
  It draws from what the frame's update saw. SELECT's map and ProtoMan's
  duel clock read the game so too, every frame they showed. The frame
  log counts any read of the game that waits while a frame is drawn.
- **The next layer comes three times sooner on a New 3DS.** Stepping
  onto the exit pad stood still for six seconds there, where a player
  felt it froze; now two. Nearly all of it went to choosing each tile of
  the new map from the thousands the area's maps show. The choice is the
  same, made with less work: the pairs of tiles seen in the same
  surroundings are weighed once and passed together when they cannot
  win, a tile's floor is looked at once for all the maps it is compared
  with, and pixels that show the same panels are compared once. On a PC
  a layer's making went from 300 ms to 60; every map comes out as
  before. Where the making takes long, "Building the next layer..."
  shows over the picture meanwhile.

## 0.5.1 (2026-09-30)

- **The 3DS no longer crashes entering the Undernet or the Nest** (the
  short net's layer 10), as a player reported. Learning an area's tiles
  took more memory than a New 3DS gives the game there: each area kept
  what it learned at a whole map's size (30 MB for the Undernet), and a
  map, its mirror image and the area's own map stood in memory at once.
  Kept at their size (3 MB an area), the own map freed before the
  others load, and each copy checked, the Undernet needs 61 MB at most,
  and the Nest 58, where they had needed 98 and 86 before the 3DS's own
  16 MB copy of the ROM. Every platform saves the memory.
- **The 3DS's bottom-screen map is lighter to draw.** It is drawn
  straight into the memory the GPU copies from, and the GPU draws that
  screen again only when the map changes. 0.5.0 drew it through the
  software renderer and read it back, 20 ms each time: a frame lost six
  times a second on the net, in a New 3DS's frame log.
- **The arrow leads to a walkway's mouth.** Where the way on enters a
  walkway a few panels ahead and MegaMan stands off its line, the arrow
  points along the floor to the panel it starts from, then down the
  walkway once he is on its line (within a third of a panel, as BN6 lets
  him in). It had pointed past the mouth, into the platform's corner, and
  a playtester read it as pointing at nothing, five times a session.
- **The official gate on the layer where ProtoMan names his netbattle
  opens.** From the third rung, his netbattle waits for the third act,
  and the gate beside him in the first two had stayed sealed as his
  duel's prize, with no duel to win. It opens to Chaud's clearance, as
  any other official gate. Chaud and ProtoMan say where he waits in the
  net's words ("past the next two guardians", where they had said "the
  third act"), and Chaud what a win opens: "my full clearance is yours:
  the official vaults open too", as the netbattle comes after two wins,
  whose clearance a playtester already held.
- MegaMan's L says how far the walk to ProtoMan is, and when it winds
  away from where his pink mark lies ("down and to the right, far off,
  though the way there winds"), as it says for a heal or a Net Dealer:
  it had measured the way as the crow flies, "close by" across a gap.
- A Guardian Data's program, once picked, comes with whether L and R
  turn it ("L and R won't turn it: that takes the pink Spin, and we
  don't have it"), where a playtester looked for it; the reminder later
  had said it alone.
- Lan answers Chaud's call about the netbattle ("We'll be ready,
  Chaud!"), as he answers the races' calls: it had read as a message
  left.
- EraseCross's line says BN6's own rule: a plain (elementless) chip
  erases a virus whose HP has a 4 in it ("A 4 in HP: plain chips
  erase"), and bugs a Navi, whose HP then drains. It had said counters
  erase viruses, two playtesters' guess, and a third saw DiveMan's HP fall
  with nothing hitting him.
- The jack-in setup says what each Cross gives, above what hits it
  twice as hard, from BN6's own Cross mails: HeatCross "Fire chips +50,
  buster +1", ElecCross "Elec chips +50", SlashCross "Sword chips +50",
  ChargeCross "One more chip each turn". Four of the five had shown
  their weakness alone. The PET's status says it too.

## 0.5.0 (2026-09-30)

- **The first beta.** The title screen names the build a beta, and the
  release is GitHub's latest, which the AppImage's updater follows: the
  alphas were pre-releases, which it skips.
- **The 3DS's bottom screen shows the layer's map,** always open (issue
  #9): the floor MegaMan has seen, the way on to the exit or the
  guardian, the services and gates he has come near, and a mark on the
  frame's edge for those he senses, larger than SELECT's map over the
  picture, which still opens.
- **ProtoMan's official gate is his duel's prize.** It opens to the
  winner of the duel beside it, where Chaud's clearance alone had opened
  it before the duel, which then paid only the record. For a Netbattler
  already cleared, Chaud's call names what the win opens, and after a
  second win he names the next rung: "Next time, no race: ProtoMan faces
  MegaMan himself." An official Chip Order offers the Library's uncommon
  and rare chips before its common ones.
- The Recovery Mr. Prog before a guardian's arena stands a short walk
  from its door on every guardian's layer: one had stood a long way back,
  and one layer in a hundred had none. A run saved by an earlier build
  continues its layer afresh.
- MegaMan's reminder of a program left off the board also says whether L
  and R turn it, as soon as it is gotten.
- **The PortMaster port no longer needs ROCKNIX's glibc.** It is built
  on Debian bullseye and asks for glibc 2.29 at most (it asked for
  2.38), older than ArkOS, AmberELEC, muOS or Knulli have. Its zip is
  laid out as PortMaster's own, with a screenshot, a gameinfo.xml
  EmulationStation can show and one license file per part, ready for
  PortMaster's catalogue. The launcher lost a Mesa setting it did not
  need.
- **A bought-out shop says so.** A Net Dealer or NaviCust vendor with
  nothing left says "Sold out!" instead of opening an empty list: after
  "More programs? Take a look!" the empty list read as a broken shop
  (issue #17). The vendor also brings his whole list, up to four
  programs: ten random draws had often brought two or three in the first
  acts, and now and then none.
- A CONTINUE keeps what was bought: the Net Dealer and the NaviCust
  vendor had restocked everything bought on the layer before the save.
- Chaud's call on a duel layer comes once: every CONTINUE there had
  played it again, after the duel too.
- CircusMan's briefing names the tell that shows: he fades from his
  panel, and his tent drops where MegaMan stands. The panel that lights
  first is under MegaMan's feet, where a playtester never saw it in four
  tents. Chips used while CircusMan is gone find nothing, and it says so.
- MegaMan's words about the NaviCust say which programs L and R turn:
  only those of a colour whose Spin you hold, BN6's own rule. He had said
  L and R turn any program. After a RUN that leaves a bug, he says the
  RUN's "OK" hides it.
- Lan answers Chaud after a duel won, as he did after every loss.
- A collector's vault and an official gate list what each of their three
  chips hits for ("Cannon A 40"), where they had names alone.
- HackJack's chips (HackJack, HackJck EX, HackJck SP) no longer turn up
  in official vaults or anywhere else. They are left over from the
  Japanese version: the US game has no HackJack, so using one made
  MegaMan vanish until he was deleted (issue #15). A run saved by an
  earlier build continues its layer afresh.
- **A guardian's reward is taken once.** B at the Guardian Data's program
  choice gives the BugFrags it promises and takes the reward: it had
  ended the talk with nothing given and left the Guardian Data there, and
  every talk after gave its HPMemory and the Navi's chip again (a player
  on Android had six ClownMan chips and 1000 HP by layer 7, issue #16).
  B at the way on after an act's guardian says the way it takes, and at a
  vault or an official gate it leaves the chips with a word.
- **The downloads have a page of their own**, `/download/`, where the
  home page's `#play` held them beside a player at `/play/`: old links to
  `#play` open it, and a system in its address opens on that system
  (`/download/#3ds`). Every page has its canonical address, a share
  picture and words for link previews, and schema.org data (the game, the
  FAQ's questions); the site has a sitemap and a not-found page. The
  analytics count what visitors do, never who they are: downloads by file
  and system, starts in the browser, ROMs taken or refused (and why),
  platforms chosen, FAQ letters opened and links out; section anchors no
  longer count as pages of their own. A browser no longer runs a kept
  older script against a newer page.
- A strong virus signal names its prize before the choice: "It pays
  DolThdr3 B. Take it on?", where it had said "a good chip" and a
  playtester weighed the risk blind.
- **The downloads say which system each is for**, and the README's
  Install section starts with a table from system to file. Players took
  the PortMaster port's plain `cyberworld.zip`, first under Install, for
  the game on Windows. Renamed: the port is
  `cyberworld-endless-rocknix-portmaster.zip`, the Windows installer
  `cyberworld-endless-windows-x64-setup.exe` and the site's files
  `cyberworld-endless-website.zip`. The project page finds either name.
- ProtoMan is easy to find on his duel layer: the official gate he
  opens stands by him (within 8 panels on 72 of the tests' 74 duel
  layers, where it had stood anywhere), the map marks him in pink with a
  white eye and names him in its key, and MegaMan's L says which way he
  waits and how far ("ProtoMan's down and to the right, far off: the pink
  mark on the map"), on every L while the duel waits. A playtester found
  the gate alone, both marks violet, and his session ran out looking for
  the rival. A run saved by an earlier build continues its layer afresh.
- An official gate says what it holds: Chaud's call names the gate on the
  duel's layer (an official Chip Order, three chips you've held, one to
  order; from the netbattle, the official vault's three Mega chips), and
  MegaMan says it at the sealed gate. A playtester, five duels lost, took
  the gates for scenery.
- **Touch controls, remade.** The D-pad and buttons are drawn at the
  screen's own resolution as glass plates in the PET's colours, sized in
  millimetres for a thumb on every phone (the D-pad 3 cm across, A and B
  1.4 cm, smaller only where the screen has no room), each reaching a
  little past its art; a pressed one glows, and ticks under the thumb on
  Android and in browsers that can. The D-pad takes diagonals where
  MegaMan walks, whose walkways run along them, and four directions in
  battles and menus, where a diagonal only got in the way; a thumb on the
  line between two directions keeps the one it holds. Their MENU button
  pauses the game: size, opacity and haptics for them all, and EDIT
  LAYOUT, where a button is chosen with a tap, dragged where the thumb
  wants it, pinched or sized by a corner, given its own opacity, or laid
  out by a preset (default, left-handed, compact, large where there is
  room); DONE keeps it, CANCEL undoes it. The phone upright and on its
  side keep an arrangement each (`touch.ini` in the save folder). A
  player asked to play on My Boy! for its adjustable buttons.
- The project page counts its visits with Umami, on its own domain only
  and without cookies. The release's web zip, to host elsewhere, carries
  no counter.
- MegaMan beside a Net Dealer's counter, at its far end, is drawn behind
  it, as BN6 draws him: the counter keeps its original map's layer
  priorities there, where he had been drawn over it, as if standing on
  it. Only where he stands before the counter's own art: beside it a
  raised pad's edge had cut him in half.
- ProtoMan's races are against viruses a quick hand can hurry: a squad
  with a Quaker, out of reach in the air until it lands, is rolled again
  among the act's battles. Three of a playtester's four duels were
  Quakers, "a Quaker lottery"; 3 of 20 Seaside duels held one, now none.
  A run saved by an earlier build continues its layer afresh.
- A town's port takes R from a step off its ring: MegaMan steps onto it
  and jacks in, where a playtester stood at the mermaid fountain's rim a
  step short and pressed R five times. Further off, MegaMan says which way
  the nearest part of the landmark's ring lies, as the crow flies; the
  walk to its front's middle had wound round the fountain's basin and
  turned from "up and to the left" to "straight down" a step apart.

## 0.4.0 (2026-09-30)

- **The 3DS build installs on the HOME Menu:** a CIA beside the `.3dsx`,
  with the game's icon and a banner that plays the trailer's opening hits.
  The project page has a 3DS tab: the newest release's CIA as a QR code
  for FBI (Remote Install, Scan QR Code), and where the ROM goes. Releases
  carry both files.
- A layer is made in about a quarter less time (the wait at a new run's
  start and at CONTINUE, 20 seconds on a 3DS): the tile pick keeps what it
  asks of each pixel of the floor and of each tile it compares, where it
  asked again a thousand times. The layers come out the same.
- **MegaMan's words come on time** (issue #13). A new area's card and
  MegaMan's words about it are one beat: MegaMan holds still from his
  arrival until the words begin, as BN6 holds him for its own scenes, and
  A ends a card early. Free under the card, a player walked to a Mystery
  Data, opened it, and only then heard where he was. A NaviCust bug is
  named as the map comes back from the PET, where a second's wait had let
  MegaMan walk a few steps first.
- The first "Are you sure?" in a shop its keeper's words opened starts on
  No: an A carried over from the Net Dealer's words chose the list's first
  row and bought it. A buy chosen after that starts on Yes, as in BN6.
- **The BugFrag Trader trades** (issue #12). After Yes its chat held for
  good: BN6's trade is made by the Undernet's trader machine, which a
  layer's trader stands without. The director now makes it as the machine
  does: the prize from the game's own BugFrag pool, ten BugFrags taken,
  the game's lines for the hand-over and the prize, then "Try again?". No,
  to either question, ends the chat, where MegaMan had walked on with it
  open and the PET shut.
- **Chaud's prize stands beside the duel.** Every act's duel layer holds a
  sealed official gate beside ProtoMan (an official Chip Order in the first
  two acts, the official vault from the third), and Chaud's call says so.
  Win the duel that earns the clearance and the gate opens at once. A
  playtester's promise of official gates had pointed at gates he never met
  (they stand on one layer in four elsewhere). Runs saved by an earlier
  build continue their layer afresh.
- **A Nintendo 3DS build** (issue #9): a `.3dsx` for the Homebrew Launcher
  on a New 3DS, which finds the ROM where 3DS players keep GBA games.
  The picture fills the top screen's height (`screen = whole` in
  `settings.ini` for 1x), drawn through the GPU; the GBA core runs on the
  third core and draws its picture on the main one, at full speed. A new
  run's first layer takes about 20 seconds to make there, which a line on
  the screen says. `3ds/README.md`.
- A ROM of another Battle Network 6 is named for what it is ("... is Cybeast
  Gregar (Europe): only Cybeast Gregar (USA) works so far"), where the
  message said only where to put one; other games' ROMs in the folder are
  passed over by their header, without reading each whole.
- Frames played unshown to catch up (a slower machine, a 50 Hz display) and
  the boot's frames draw no GBA picture: a fifth of the core's time on a
  slow machine. The frame log (`frame_log = on`) splits a frame's time: its
  update, the GBA's share with and without its picture, drawing, present.
- An L or R kept for the Custom screen, pressed as its gauge was all but
  full, no longer lets a second press open a chip's description: once the
  kept press opens the screen, L and R are held off half a second.
- Lan answers a lost duel ("Next time, Chaud!"), where Chaud had the last
  word. Before a rematch with a guardian who has beaten MegaMan more than
  once, MegaMan says the record ("CircusMan has beaten us twice. Not a
  third time!"), not "I'm ready this time!" every time. The setup and the
  PET say what EraseCross does, not only its weakness: its counters erase
  viruses, as in BN6. Bystanders keep their beginner's tips (the Custom
  Gauge, stacking codes) to the first act.
- **Windows keeps what a save writes.** A save writes its new file beside
  the old one and renames it over it, so a power cut never leaves half a
  save; Windows' C library refuses that rename where the old file is, so
  every save after a file's first stayed beside it as a `.tmp` and the
  first stayed in use: the run's checkpoints, the profile's unlocks and
  records. Saves now replace the old file there (and on the 3DS's SD card,
  which refuses it too).
- **Smooth motion**, an option for 90, 144 and 165 Hz screens: the game
  runs at the GBA's 60 frames a second, which such a screen shows for one
  refresh or two (or two or three) in turn, a slight judder; with smooth
  motion each refresh mixes the two latest frames by time, so motion is
  even, a little blurred and a frame later. `smooth_motion = on` in the
  new `settings.ini`, `--smooth-motion on`, or the browser player's
  button. On a 60 or 120 Hz screen it shows the frames as they are. On a
  Steam Deck OLED at 90 Hz: 90 pictures a second, the game at 60.
- Less input lag on Windows: the game asks for Direct3D 11 first, which
  lets the graphics card queue one frame ahead at most, where Direct3D 9,
  the default before, left the driver to queue up to three, each a frame
  between a button and the screen. Sound on a desktop comes about a frame
  and a half sooner after what makes it (shorter buffers: 21 ms, not 32,
  and 512 samples, not 1024).
- MegaMan walks as in BN6 again: the walking assist that lined him up with
  a walkway's mouth and followed lanes round their turns is gone. It could
  step him backward, for up to a third of a second: a player with a
  controller saw him go the opposite way now and then, and a walk of 540
  held keys on one layer found 23 such steps. It was made for a
  playtester who reads still pictures; along a walkway, hold its two
  directions together, as in BN6.
- The game keeps its pace where a frame misses the display's refresh, a
  50 Hz display or a slower machine: a frame behind is played unshown
  and caught up, as the browser's player does. Each missed refresh had
  cost a frame, and a phone slower than a frame ran at half speed.
- The touch D-pad steers eight ways alike (its diagonals, which BN6's
  walkways need, were narrower) and holds its direction through its
  middle, where a thumb rolling a few pixels past it had turned UP into
  DOWN; fingers are let go when the app loses the screen.
- A rival: Chaud and the real ProtoMan (the Nest's are copies). On each
  act's second layer Chaud calls: ProtoMan is on the layer, standing on a
  pad apart, and he has busted its viruses in a time he says. Take his
  duel (it starts on No) and beat his time, by BN6's own DeleteTime; from
  the second win on, without taking a hit as well. Chaud keeps the record
  across runs ("That's 3-1 between us"), ProtoMan gets faster as he
  loses, and Dad's Records mail lists it. Each duel is one squad of the
  act's, a little above its band, the same every time the layer is made.
  docs/RIVAL.md has the design. A run saved by an older build continues
  its layer afresh.
- A duel shows its clock: in the battle, under the Custom gauge, MegaMan's
  time so far against ProtoMan's (and on the second rung whether MegaMan
  has been hit), hidden while BN6's clock holds. ProtoMan's first times
  are looser (4 s and one for each 20 HP of the squad), and before the
  first win Chaud says what it earns: the net's official gates.
- A duel's squad is one of the act's own battles, not one above them, and
  before the choice ProtoMan says how many viruses it is and MegaMan the
  stake: if they delete him, the dive is over. A playtester took a duel
  at 100 of 140 HP on layer 2, nothing saying it was a real fight, and
  his run ended there; the summary now says a deletion was in ProtoMan's
  duel, and it counts as a duel lost. Chaud's calls say the record ("It's
  0-1 between us"): after that deletion his next call was word for word
  the first. Lan answers Chaud's call, and ProtoMan greets MegaMan as the
  old rival he is.
- The rivalry's third rung is a netbattle: after two duels won, ProtoMan
  stops racing and faces MegaMan himself, from the third act on (before
  it he says where he'll wait), at the act's guardian strength. It is a
  real fight: MegaMan can run from it (a loss), and if ProtoMan deletes
  him the run ends, which MegaMan says before the choice. Then the rungs
  come round again, faster.
- Chaud's clearance opens official gates, from the second act on, on one
  layer in four: after a first duel won, a gate of level one, an official
  Chip Order of three standard chips the Library holds, one to take; after
  the netbattle won, level two, three Mega chips. Sealed, a gate says for
  whom it opens and how far the rivalry is.
- Every act's second layer holds its duel: on a small layer whose rooms
  were all taken, ProtoMan was left out (one layer in 300 of the tests').
- A won run ends as one: the Nest's last Guardian Data gives only its chip
  ("It goes in our Library for good"), no HPMemory, heal or program pick,
  and no reminder to place a program follows on the way out. The net goes
  quiet, the Cybeast growls once more ("Something deeper down is still
  awake"), and Dad calls from the lab. His Dive report and Records are
  made again as a guardian's battle ends, and the last layer's Net Dealer
  no longer says it only gets tougher from here. A playtester's first win
  ended on two lines, with rewards that could no longer matter.
- The summary names three unlocks where a run opened three or more, the
  setup's new entries before the title's marks: a first win opened the
  endless net, threat 1, a mark and a Cross start, and it named two. A
  Cross that won't fit beside the run's says the next dive can start with
  it.
- A run saved after beating a guardian, on his layer, keeps him beaten
  when an update starts the layer afresh: his Guardian Data shown or
  taken and his exit open stay, as a gift taken does. A playtester's
  run saved beside ElementMan's open exit would have met him again, and
  his Guardian Data twice.
- Dad's Dive report is marked NEW on an act's first layer, a new guardian
  ahead, and his Records once a battle has changed them; the Records say
  what "code" beside a guardian means. A playtester never saw either
  marked, so never knew when to read them.
- A strong virus signal's prize is a chip the layer's Net Dealer doesn't
  sell, from the same pool: a playtester won a third MoonBld A beside the
  two he had just bought there. A run saved by an older build continues
  its layer afresh.
- A choice, BN6's or ours, takes no A in its first fifth of a second, too
  soon to be an answer to it, so A's pressed through a Net Dealer's words
  no longer land on the shop's "Are you sure? > Yes" (BN6's default) and
  buy: a playtester's did twice in a session. B still answers No at once.
  The same fifth of a second, not half a second or more, holds A off as a
  shop's list opens and after a chat closes: the longer ones read as input
  lag.
- LibCard says a vault holds rare chips ("Vaults open at 60, rare chips
  inside"), and a bystander's word on running from a battle is an
  operator's trick, not a list of buttons.

## 0.3.0 (2026-09-29)

- The project page's downloads open on your platform (Windows, macOS,
  Linux, Steam Deck, Android, the handhelds or the browser), its best
  build first and the others under it with what each is for, then what
  to do next; the hero's button says whose download it leads to. The
  Game section shows a town, the jack-in setup and Dad's records.
- The NaviCust's Spins are found in the net, one a run: a blue Mystery
  Data on one layer of 4-8 holds a Spin of a colour you don't have yet,
  kept for every run after, and MegaMan says what it does. A program
  turns with L and R only with its colour's Spin; a run had all six from
  its start. The drafts offer only programs that fit as they can be
  turned. A profile from before starts with none. A run saved by an older
  build continues its layer afresh.
- On an act's first layer the Net Dealer has no word yet on a guardian
  MegaMan has never battled ("Nobody's come back to tell. Ask me again
  deeper in!"); from its second layer he names him and his weakness. A
  playtester's first dealer named the "???" guardian two minutes into
  the act; a bystander's rumor, sought out, still tells it early.
- A HeelNavi-shaped guardian (Falzar's Navis, GroundMan, DustMan) is
  named at every rematch, not only the first meeting, and speaks without
  a face: the HeelNavi's is the bystanders', and a playtester read
  SpoutMan's lines as a bystander's. MegaMan says "I'm ready this time!"
  only after a loss to him.
- A Net Dealer neither picks nor stocks a chip the folder already holds
  as many of as BN6 lets it (by its memory: five under 20 MB down to one
  from 50): a playtester paid 1000 zenny for two ElcPuls1 S beside the
  three his folder had, which it could not take. A run saved by an older
  build continues its layer afresh.
- Dad's mails read in BN6's own mail form (its MESSAGE band, white box,
  Dad's face as a photo, then MegaMan's): they had drawn as a map chat,
  their band garbled. NaviCode names two codes, then points to Dad's
  Records mail, and LibCard says it is a collector's vault that opens at
  its number.
- The PET's E-Mail holds a mail from Dad for every guardian MegaMan has
  battled, in any run: his battle data, the guardian's warning in
  MegaMan's words, to read again any time. A guardian battled for the
  first time mails his on the next layer ("Mail from Dad, Lan!"). The
  KeyItem list shows what the profile holds beside the run's items:
  NaviCode (the codes of guardians deleted twice, which open their
  gates), DarkPass (the Secret Area cleared) and LibCard (the Library's
  size, and a vault's), and ScrtData says what three of them open.
- The PET's Save, greyed in a run, is the run's own: it closes the PET
  and saves the run where MegaMan stands. Comm keeps BN6's grey, as
  without a link cable. Dad's lab mails two reports that head E-Mail,
  made again on each layer: the Dive report (the layer, the act and
  area, the guardian ahead as MegaMan knows him, by battle data, word on
  the net or not yet, ScrtData, what the run brought) and the Records
  (MegaMan's wins and losses against every guardian met, the codes he
  holds, the best layer). They are BN6's own mail screen, its music
  playing on.
- The PET's PLACE names the area beside the layer ("JudgeTree 14",
  "ACDC HP 8"); the label on entering a map keeps "Layer 8".
- A new run's acts prefer guardians MegaMan has never met, in any run,
  where the area's own can be one: a first meeting is a discovery, and a
  playtester 34 sessions in had met eight of the seventeen. On his
  profile, every sample run now meets two new ones by its third and
  fourth acts.
- Facing a guardian MegaMan has battled before, he reminds Lan when to
  strike just before the battle, from battle data ("Remember our battle
  data, Lan: he stands still at the back while his tower and flame play
  out: strike then!"): the layer's briefing had come ten minutes before
  a playtester's fight.
- The first layer's gift gives a head start's HPMemory (and a comfort
  one) before it offers the pick, not just before the options, where
  "2 HPMemory" then "HPMemory x2" read as the same thing twice.
- ChargeMan's warning, from battle data, says where to stand when his
  train comes: his freight cars roll down the other two rows a column or
  two behind him, so once he has passed, his own row behind him is safe.
  A playtester, twice deleted by him on layer 9, ran into a car stepping
  aside.
- The NaviCust vendor's "I brought it along" names only a program from
  earlier runs that MegaMan hasn't got now (a playtester with SuperArmor
  installed was offered it), and a bystander's word on dark warps says
  they are a side trip into a copy of the Undernet and back, beside the
  way on's sealed road down into it.
- Battles set out the rocks, rock and ice cubes and metal cubes of the
  area's original battles, which were dropped: Robot Control Comp's
  metal cubes stand in a third of its battles, the Green Area's rocks in
  a sixth, none in Central Area's. And now and then (a battle in forty or
  so, more in the Sky and Seaside Areas) a green Mystery Data sits on the
  enemies' side, as in BN6: it breaks at the first hit, from either side,
  but still there when the battle is won, it gives a second reward, a
  rare find: three times in four a chip a tier above a blue Mystery
  Data's, in the folder's codes where it comes in them, else twice its
  zenny. After the first battle that held one, kept or broken, MegaMan
  says what it was.
- "Run saved" shows in the picture's corner for a moment each time the
  run is saved on its way (a layer's arrival, a guardian's Guardian
  Data), and the quit prompt names where it was saved (the layer's
  start, the Guardian Data, or where you continued) when it cannot save
  right there: a playtester playing in short sessions asked when it was
  safe to stop.
- A Net Dealer's pick for the guardian prefers a chip in the folder's
  codes or *, where one hits nearly as hard (counted a quarter harder
  than one that comes in neither): a playtester's pick was WideSht Q
  beside a folder of S and *, and paired with nothing. A run saved by an
  older build continues its layer afresh.
- The ACDC HP takes two in three of its random battles from Central
  Area, where it took one in three: from act 3 its own Catacks alone
  fill the act's band, and they came in two of three battles there
  (three of a playtester's four were the same Catack pair). Now in one
  of three, the rest Central Area's ten kinds.
- A Chip Trader's prizes are only chips that come in the folder's codes
  or in *, where a dozen of its pool do: it had rewritten a prize's code
  to the folder's only where the chip came in one, and a playtester's
  two trades beside a folder of S and * gave SumnBlk2 H and GunDelS2 E.
- A guardian's Cross that won't fit beside the one brought is "We can
  only carry one Cross down here!", where MegaMan had said "One Cross a
  run!", the game's word rather than his.
- MegaMan knows who guards an area only once he has battled that Navi,
  in any run. Before, he knows nothing of him, not even his element: the
  act's card reads "Guardian: ???", L speaks of a strong Navi's signal
  he doesn't recognize, the arena's approach of "the guardian", the way
  on offers the areas with "a Navi we've never battled", and the arena's
  card reveals him. The Navis on the net know more: the first bystander
  on an act's first layer says who guards its end and a rumor about him
  ("They say a copy of EraseMan guards the end of Judge Tree Comp. Word
  is, he deletes Navis outright."), and the Net Dealer names him and his
  weakness. Once heard, MegaMan names him for the rest of the act, as
  hearsay ("DiveMan guards the end of it, word is"). MegaMan had named
  every guardian from the act's start and briefed a first meeting with
  the rumor himself.
- MegaMan's words before a guardian's arena give each service's way as L
  and the map do, where it lies, and say when the way there winds: they
  had named the walk's first step, and a playtester's heal Prog was "up
  and to the right" there and up and to the left on L and the map.
- MegaMan knows a guardian only once he has fought it. Meeting a copy the
  first time, he has no battle data on it: L names it and says to watch
  the yellow panels, where every attack lands. From the second meeting,
  in any run, he briefs its moves and when a hit lands "from battle data".
  The area's warnings about ScarCrows and DarkMechs likewise come once
  those viruses have been battled. The briefings had recited a copy's
  moves before MegaMan could know them, and spent the first fight's
  discovery. The data is said when it comes: a first win's Guardian Data
  brings "his battle data", and a run lost to a guardian at the first
  meeting ends on Lan's "We've got his battle data now!".
- A won run ends with words on the map and BN6's staff roll theme: as the
  short net's last guardian's exit opens, MegaMan and Lan say the net has
  gone quiet and they did it, and the summary plays the credits' song,
  the title's own once it is closed. The Nest's fall had gone straight
  from the exit pad to the title tune.
- Every guardian's warning says when a hit lands, as its fight shows it:
  HeatMan stands still at the back while his tower and flame play out,
  SpoutMan, JudgeMan and ElementMan right in front of us while they
  attack, SlashMan, TomahawkMan and Colonel beside us a moment after
  their slash, GroundMan on our side after he bursts up, and so on. A
  playtester's swords missed HeatMan and SpoutMan as they warped.
  Colonel's warning names his dark-screen slash too, which had come
  unannounced.
- A run is saved as a guardian logs out and its Guardian Data appears, so
  a CONTINUE after quitting in its talk picks up the Guardian Data, not
  the guardian: a playtester who quit there had to fight HeatMan again.
- Hurt on a layer with no Recovery Mr. Prog, MegaMan's answer to L names
  the Net Dealer, who always has MiniEnrg, and which way he is: a
  playtester at 90 of 240 ran to the exit past him, then a dozen moves
  back.
- The Aquarium's and the Judge Tree's mazes keep two panels between their
  walkways, where they ran one apart: on the Judge Tree's brick walkways a
  playtester read side-by-side walkways as terraces he could not step down
  to, and lost his way to a heal and two shops. A run continued from an
  older build starts its layer afresh.
- The setup's Cross row names the Cross's weakness ("Breaker attacks do
  2x", as BN6's own Cross tutorials have it), and MegaMan says it when a
  guardian's Cross is won. A playtester was told the wrong one.
- A risky choice starts on No: a strong virus signal's fight, a Navi
  gate's and a warp into the Undernet. A playtester's A pressed through
  the signal's words took its fight on at Yes and cost him 150 HP.
- A strong virus signal's prize is one the folder can play where one of
  its twelve draws is (in the folder's code or *), the hardest hitting of
  those: a Blade folder won HeatManEX H. A run continued from an older
  build starts its layer afresh.
- A run can begin in Green Town, the fourth starting town: the Judge
  Tree, the flower shop, the lily ponds and the stumps as Capcom made
  them, with its own theme. Lan jacks in at the knight statue on the
  flower plaza; the townsfolk, the flower shop and the stumps' tables
  have words of their own.
- More of what a run finds fits its folder. Half the Mystery Data's chips
  are rolled again for one in the folder's codes, where only the code had
  leaned: a Blade folder's finds came in its codes two times in seven, now
  about two in three. A Chip Trader's prize comes in the folder's code, or
  in * where it has none of them, so three chips the folder cannot play
  trade for one it can. A run continued from an older build starts its
  layer afresh.
- An L or R pressed before the Custom gauge fills is kept through a dodge:
  a step waits its turn (the press is given once MegaMan has moved, a
  second at most), where a d-pad press had dropped it. A playtester's R
  pressed before dodging a bubble in SpoutMan's fight never opened the
  Custom screen.
- Stepping into the room before a guardian's arena, MegaMan names the Net
  Dealer and the Recovery Mr. Prog there, and which way each is, if they
  have not been used (the heal only below full HP): the arrow leads past
  them, and a playtester reached SpoutMan at 120 of 140 HP with 1150
  zenny unspent.
- A guardian's battle chip comes in its * whenever the folder holds none
  of its letters, not half the time: a playtester's Blade folder won
  SpoutMan A.
- An exit pad takes MegaMan from anywhere on its drawn ring, from every
  side: its trigger is round, 26 units out, where it had reached less at
  the screen's top. A playtester coming down onto a pad from the upper
  right stood with his feet on its rim and stayed, twice.
- A run can begin in Seaside Town: the third starting town, beside
  Central and ACDC Town, and the first as Capcom made it, whole, with the
  whale, the pier and the station walkway at their own heights. Lan jacks
  in at the mermaid fountain on the plaza; the townsfolk, the fish shop
  and the fountain have words of their own, and the town plays its own
  theme.
- L's arrow keeps its way until the way on is clearly in the next eighth,
  not the moment it crosses the edge: it wobbled between two neighbouring
  eighths as MegaMan walked, a third of its turns swinging back within a
  second, and a playtester holding the way a picture showed ran past turns.
- A run with the Blade or Storm folder starts with the pack as BN6 would
  have it after giving that folder at NEW GAME: the Standard folder's chips
  are no longer spares in it (a playtester's Blade run slotted in its
  CrakShot and Cannons, and "nothing reaches the back" with them).
- A new run's first area and guardian avoid the last two runs' where they
  can, so any three runs running open in all three opening areas: avoiding
  the last run's alone let two of them take turns, and a playtester met the
  RoboDog Comp five runs in seven and the Seaside Area never.
- The layer-1 gift's Mr. Prog logs out once the gift is taken, and an A
  pressed within half a second of a chat closing on the map is not passed
  on: a playtester's A pressed through the last box talked to the Prog
  beside him again, two sessions running.
- L's arrow no longer shows over a jack-in's or a warp's flash and tunnel.
- After a run that opened something (a folder, a Cross start, a threat
  rung, the endless net), the setup opens on that row, with NEW beside it,
  until the next jack-in: a playtester's A went through a setup that showed
  none of his new SlashCross start, the cursor on JACK IN!.
- The Blade and Storm folders' chips that BN6's Standard folder lacks
  (LongSwrd, PanlGrab, Barrier, Thunder and the rest) play: they came up
  blank in a new run's hands, with no name and no effect, since BN6 takes a
  chip it never gave for a cheat's. The engine now marks the folder's chips
  as given, at a run's start, on every fresh layer and on CONTINUE, which
  also mends a run saved before.
- No running from a guardian, as from BN6's story bosses: "Lan, this is no
  time to run away!" A playtester ran from CircusMan at 5 HP, healed beside
  the arena and came back to a fresh fight. Virus battles, Servers and the
  Navi gates' SP fights can still be run from.
- The short net's dark way: after act 2's guardian, once the Secret Area
  has been cleared in any run, the way on offers a third way, into the
  Undernet for act 3, where one of its own Navis waits. Its battles hit
  harder and come in more kinds, and its layers hold more rich Mystery
  Data. Until it opens, MegaMan says it is sealed and what opens it.
- A run begun by an older build counts its Library's news from its first
  checkpoint on this one: a playtester's run would have ended on "Library
  32 (+32)", with 26 of them from runs before.
- The Aquarium and Judge Tree Comps' mazes wind half as much: their
  corridors run straight on where they can and more walls are knocked
  through, so the walk from the arrival to the exit takes 8 legs, as in the
  other areas, not 16; a playtester spent 110 of 249 moves walking and
  reading the map there. Their guardians' layers are smaller mazes that
  leave room for the arena, where eleven in twelve had fallen back to a
  plain route.
- A battle's chip reward comes in one of the folder's codes half the time,
  where the chip comes in one, as Mystery Data's does: a playtester's Blade
  folder of S, L and * won WaveArm1 E and the like, off every code it held.
  A guardian's own chip comes in its * then where the folder does not hold
  its letter, as his Guardian Data gives it (not ChrgeMan C for S, L and *).
- Collector's vaults: from the second act a layer may hold a vault whose
  lock counts the Library (30 chips in act 2, 60 in act 3, 90 later; its
  words say how many it wants and how many the Library holds). Open, it
  holds three rare chips, in the folder's codes where they come in them,
  and MegaMan takes one; B leaves them for later. The map marks it violet,
  and L explains it the first time.
- Threat rung 10, opened by a win on rung 9: a second guardian waits below
  the short net's Nest. The first's exit leads to layer 11, with its heal
  and Net Dealer before the arena, and MegaMan feels the second from the
  Nest's first words. The top rung, and the title's green disc, is now 10.
- The way-on arrow L shows stays ten seconds after his words and while
  MegaMan walks, up to half a minute, and fades three seconds after he stops: it faded three seconds after L's
  words, and in the Aquarium Comp's mazes of short walkways a playtester
  lost a dozen moves at a time between one L and the next.
- NaviCust programs found join later runs: every program MegaMan runs with
  (on the board, or in the PET where it fits) is kept in the profile, and a
  later run's program vendor lists two of them first, when the act may
  offer them, at the price the others have, and names them in his greeting.
  A program a draft brought, which BN6's shops never sell, can so be bought
  in a later run.
- Threat rungs 6 to 9, each opened by a win on the one below: Mystery Data
  holds chips, never zenny; Chip Traders come half as often; a Guardian Data
  drafts two programs; and gives four HPMemory. The top rung, and the
  title's green disc, is now 9.
- A Net Dealer lists a chip once, whatever its code: a playtester's list
  held ElcPuls3 A at 700 zenny, the pick, and ElcPuls3 S at 2000.
- Sealed gates: from the third act a gate may stand sealed with a guardian's
  code, which says how to earn it: delete that Navi twice as a guardian, in
  any runs. An earned code opens every such gate in later runs, to a fight
  with his SP whose chip is the prize. L names the gate, and the map marks
  it violet.
- An L or R pressed before the Custom gauge fills is kept two and a half
  seconds, not one and a half: a playtester's press two seconds early, while
  the gauge looked full, was dropped.
- A new run begins in the other town than the last one (Central Town or ACDC
  Town), as it already avoids the last run's first area and guardian: a
  playtester began on the same street four or five runs running.
- Walking onto a warp pad from the side takes it: its trigger covers the
  side corners too, where a playtester stood on the rim without leaving, on
  two runs.
- A guardian's Navi chip comes in * where BN6 has one (the V1 chips) and the
  folder doesn't use its letter: a playtester's Blade folder of S, L and *
  took a BlastMan B it could not play.
- A Guardian Data draft offers only NaviCust programs that fit beside those
  on MegaMan's board, each in a colour that fits too, on the board it
  leaves. A playtester was offered SuprArmr beside the gift's Custom1 on the
  4x4 board, where the two cannot fit. A program that cannot fit is named
  once a board size instead of on every layer. A run saved before continues,
  its current layer afresh.
- A shop's list opens a moment after its keeper's last line, so an A pressed
  twice to close it no longer picks the first chip ("Are you sure? > Yes").
- A Net Dealer names the viruses' weakness only where it holds: half or more
  of the kinds of virus the area's battles can hold at that depth share the
  element (two kinds at least). A playtester told the viruses couldn't stand
  Aqua met Piranhas, bees, crows and planes; that area's mix now gets no
  word.
- Routes: after an act's guardian, the Guardian Data asks which way on: the
  planned next area or another of its tier, each named with its guardian and
  his element. Both fit the act; B keeps the first. Acts 2 and 3 of the
  short net, 2 to 4 of the endless net. A run saved before continues, its
  current layer afresh.
- Back from the PET with a program still off the NaviCust's board, MegaMan
  says so at once (once a layer), where L's briefing said it only on the
  next layer: a playtester ran the NaviCust without placing his Guardian
  Data's program.
- L's briefing warns of the area's ScarCrows (lightning heals them, and so
  do Elec chips) and DarkMechs (they warp beside MegaMan to slash) where an
  act or a side layer begins and its battles can hold them. A playtester's
  Thunder healed a ScarCrow to full, three times, with no word why.
- No random battle before the Undernet holds two DarkMechs: their teleport
  slashes hit well past their damage value, and a pair took a playtester
  from 480 HP to 20 in act 3. CopyBot's comps hold them only in pairs, so
  they now first come in the Undernet, as in BN6.
- On a short net Lan says "Down to the Nest again", where "The Endless Net
  again" read odd to a playtester who had chosen Short.
- The Library carries over: every chip MegaMan holds joins the profile's
  Library, and each run's game is given it, so the PET's Library shows the
  whole collection and a Chip Trader's prize is new across runs. The summary
  counts the chips a run added; STD, MEGA and GIGA COMP go on the title for
  a class complete.
- Cross starts: once HeatMan, ElecMan, SlashMan, EraseMan or ChargeMan has
  fallen as a guardian in any run, the setup's new Cross row lets a run
  bring his Cross from the first battle. It is the run's only Cross: a Cross
  Navi deleted later says his data won't fit beside it. The summary
  announces a new Cross start, and its goal line points at the first.
- In the town, L and the arrow give the way to the port on foot: the first
  stretch of the walk around the houses, where they pointed straight at the
  port and sent a playtester into a house front on two runs.
- Runs open in three areas: the Seaside Area joins Central and the RoboDog
  Comp for act 1 (five kinds of virus in the act's band), and a new run
  avoids the last one's act 1 area as it does its first guardian. A
  playtester began in the RoboDog Comp four runs running.
- Rewards come in the folder's codes: the codes the folder holds most (three
  chips or more each) are read as each layer is made, and a Net Dealer's
  chip, the gift's, and half of the Mystery Data chips that come in one of
  them take it (a * stays a *). The first dealer of a run says so. A
  playtester's rewards, each in its own code, had left him hands of four or
  five codes at the final fight. A run saved before continues, its current
  layer afresh.
- A ScrtData says what it is for as it is picked up (three open the golden
  gate to the Secret Area), and L's briefing counts them only when the
  count changed, where it said so a layer late and then on every layer.
- A run's summary names only what that run unlocked: a folder earned in an
  earlier run that never reached a summary is no longer announced as new.
- SlashMan's warning names the blades he leaves stuck in our side;
  EraseMan's says his ghosts drift across the rows and to heal first.
- **Touch controls.** On a touch screen the game draws a D-pad, A, B, L, R,
  Start and Select round the picture, in its own pixel art: under it on an
  upright phone, beside it on a wide screen (the picture a whole scale
  smaller where it would leave the thumbs no room). A thumb slides across
  the D-pad and rolls from B to A; a controller or keyboard puts them away.
- **An Android app** for phones, tablets and Android handhelds (Android 5 on,
  one APK for arm64, 32-bit ARM and x86-64): the first start asks for the ROM
  with Android's own file picker, checks it and keeps a copy with the saves;
  a handheld's own controls, a controller or the touch controls play it,
  upright or on its side, and Back asks before it quits.
- **The browser on a phone or tablet:** the player fills the screen (full
  screen where the browser allows it), turns with the phone, keeps the
  screen on, and installs to the home screen as an app.
- MegaMan no longer disappears behind Seaside's walkways and pads. Seaside
  Area 1, whose map a Seaside layer took over, draws everyone behind the
  map's second tile layer, where a layer puts some walkway, the pads' rims
  and their centrepieces; its layers now take over Seaside Area 2's map,
  which draws them in front, as every other area does (Mr. Weather's comp
  likewise). A run saved in a layer by an older build starts that layer
  afresh.
- **A trailer:** 21 seconds of runs, battles, a HeatCross, a BeastOut, the
  guardians and the Undernet, with an original battle theme in the manner
  of BN6's, played through a model of the GBA's sound. The project page
  opens on it: it plays muted in the hero (its words carry it), and Sound
  on starts it over with the music; the README links it.

## 0.2.0 (2026-09-28)

- A Net Dealer met again in the same act greets in a line ("Back again,
  MegaMan! My pick for SpoutMan is first on the list."), where every
  layer's said the greeting and the pick's reasons again.
- After a CONTINUE past a deleted guardian, the act card says so.
- A program MegaMan has but has not put on the NaviCust's board is named
  in L's briefing on every layer, with how to install it; the Guardian
  Data talk says how every time, not only the first: a playtester played
  two acts believing a Guardian Data's UnderSht was running.
- Navis you talk to keep apart: a heal Prog no longer stands beside a Net
  Dealer's counter (an A meant for the Prog opened the dealer, twice), and
  none stands within two panels of another where the room has space; the
  heal before a guardian moves to a room nearby when the dealer fills the
  last one. A run saved by an older build starts its current layer afresh.
- DiveMan's warning says to hold the chips until he surfaces.
- **macOS:** one app for Apple silicon and Intel Macs (macOS 11 on) in a
  .dmg, built on a Mac by CI. It is signed ad hoc, not notarized: the first
  start needs Open Anyway in System Settings. The ROM is found in Downloads
  or chosen in the Mac's own open panel; saves live in
  `~/Library/Application Support/cyberworld-endless`.
- **Windows:** a build for 64-bit Windows 10 and 11, as an installer (for
  the user, no administrator; Start menu, desktop shortcut, uninstall in
  Settings > Apps) and as a zip to unpack anywhere. One .exe with SDL2 and
  the GBA core inside; the first start finds the ROM in Downloads or asks
  for it in Windows' own file dialog, and saves live in
  `%LOCALAPPDATA%\cyberworld-endless`.
- **Steam:** the game adds itself to Steam as a non-Steam game with its
  own library artwork: a capsule, a wide capsule, a banner, a logo and the
  icon, drawn for it (nothing from the ROM). The AppImage, the `.deb` and
  the archive offer it on their first start from the desktop and do it
  with `--add-to-steam`; on a Steam Deck the Flatpak's is one command in
  Konsole (README). Steam closes for a moment and opens again; an entry
  made before by Steam's own "Add to Steam" keeps its place and play time
  and gets the artwork.
- **The meta layer, phase one** (docs/META.md): what a run leaves for the
  next is options, never power.
  - A run is the short net: three acts, then the Cybeast Nest on layer
    10, whose fall wins the run and opens the endless net.
  - From the second run, NEW GAME opens a setup: the net, the starting
    folder, a threat rung, helpers.
  - Two starting folders open in any run: Blade (swords in S for
    LifeSword) once any guardian falls, Storm (Elec) once an Aqua
    guardian does; the setup lists each folder still closed and how it
    opens.
  - Five threat rungs open one by one with wins, each one constraint.
  - Three helpers (two more HPMemory, a heal on every layer, gentle
    battles) count for everything.
  - The summary names what a run opened, and the closest goal.
  - BN6's title marks, at BN6's places, stand for milestones: Gregar's
    head for a short net won, Bass for the endless net's Nest, the S for
    the Secret Area, the green disc for a win on the top threat rung. A
    new mark blinks in after the run's summary. The version and the best
    depth moved to a line above the copyright.
  - Runs saved by an older build start afresh.
- A returning player hears less at a run's start: the gift Mr. Prog's
  greeting is one page from the third run on, and so are the first
  layer's arrival words (a restart after a long run was called a chore).
- A Server says how strong it is before it asks ("Its viruses outclass
  this layer"), and act 1's hits no harder than half again the act's own:
  a Server on layer 2 had hit for 80 of a new MegaMan's 100 HP.
- Services keep off the way from a layer's arrival to its exit or
  guardian, and the panels beside it, where their room has another place,
  and leave no one-panel gap on the way beside another navi or Mystery
  Data: a Recovery Mr. Prog beside a turn of the way stopped MegaMan for
  three calls, and another, a panel from a Server, wedged him between
  them. A run saved by an older build starts its current layer afresh.
- A d-pad press drops a kept L or R: a kept R had opened the Custom screen
  over the UP stepping MegaMan off a lit panel. ChargeMan's warning says
  his freight cars block chips and that he stops only as he pulls back in
  at the back. Random battles keep clear of the last two battles' virus
  families where another battle fits (a Server's pair came back two
  fights after it).
- A dark warp says what the Undernet holds before it asks (tougher
  viruses, richer data, a BugFrag Trader, and an exit on to the next
  layer), and so does L the first time it names one: a playtester kept
  off one, not knowing what it was for. A Server's violet mark leaves the
  map once its battle is taken.
- Services and navis keep off the line a walkway makes across its
  platform, where MegaMan runs: a Net Dealer on a homepage's stripe took
  two sidesteps each way, and more than a quarter of them stood on such a
  line (now 19 of 1158 on 300 layers, where a room has no other place).
- L's briefing is shorter: what MegaMan senses in one sentence, the map's
  violet mark explained in full until it has been heard once and named
  after that, and on an act's first layer the guardian not named again
  after the arrival's words (one layer's briefing ran to eight pages). An
  L pressed during the act card now gives the briefing after the
  arrival's words, where it had been lost.
- The Net Dealer's pick for a guardian of no element is never a TankCan,
  which fires after a wind-up (CircusMan hopped out of its row, and 200
  went off on nothing), and a Server's prize comes in its * code where the
  chip has one (an M-Cannon R fit nothing a playtester carried). A run
  saved by an older build starts its current layer afresh.
- A random battle keeps clear of the last battle fought: the next battle
  is rolled again every few seconds, and each roll had taken the last
  one's place in memory, so a Server's viruses could come straight back
  (a HnyBmbr2 and MegaCorn, then a HonyBmbr and BombCorn).
- An L or R pressed up to a second and a half before the Custom gauge fills
  opens the Custom screen as it fills, and a press the game let pass while
  MegaMan fired or was hit is pressed again until the screen opens (half a
  second had been the limit, and a hit as the gauge filled lost the press).
- The site is one page of the game's pictures and its builds, and an FAQ
  laid out as the PET's E-Mail answers what the docs answer: the ROM,
  where it plays, a run, saves, keys and how it runs BN6.
- A Server's prize comes from where the Net Dealer finds his picks, two
  layers deeper (an M-Cannon had paid for a Server beside a dealer's
  DolThdr2), and the last battle is kept with the run's save: the first
  battle after a CONTINUE had brought the last session's pair back. A run
  saved by an older build starts its current layer afresh.
- Where a bridge plugs into a Graveyard slab (and at a few other joins),
  the join's own tiles from the originals are drawn, their lighter patch
  on the slab included, in place of plain slab tiles that left a notch.
- Central's, Seaside's and Sky's framed pads keep straight upper edges and
  meet their catwalks whole: a stamp change had left tile-wide steps along
  their upper edges, and its edge was laid over a catwalk's last panel.
  Their rims now come from whichever of the area's pads draws each tile
  alone, and a pad is its walkways' floor, as on the original maps.
- A Server's prize is named and given in full: its talk holds MegaMan now,
  so the A that paged "The virus signal left a chip behind" no longer talked
  to the Server he faced, whose own words had taken the box before the chip
  was named. The prize is the hardest hitter of a dozen rolls a tier
  richer (a FireBrn1 had paid for 120 HP of a hard battle). A run saved by
  an older build starts its current layer afresh.
- A random battle brings none of the last battle's virus families where
  another battle fits, a Server's included (a third as likely had still let
  Piranha and Puffy come twice in a row, and a MegaCorn straight after the
  Server's), and the Aquarium HP's first briefing names its ice panels
  beside the conveyors (a playtester froze on them twice).
- A shop, a trader or a navi never ends up in a walkway's mouth: the floor
  around what stands on a layer stays as the layer was laid out when its
  tiles are fitted (a Chip Trader had stood where a walkway entered his
  platform, one layer in sixteen in the homepages). A run saved by an
  older build starts its current layer afresh.
- CircusMan's warning names his tent's tell: when only MegaMan's own panel
  lights, the tent drops over him about half a second later (watched frame
  by frame; the clap's column and the lion's hoop were already right).
- Cleaner joins between a net area's two floors. Walkways meet platforms
  square on in the middle of a side, never at a corner or along an edge,
  and in the comps and homepages a walkway runs on across the platform it
  enters as a stripe of its own floor, ending a panel inside the edge, as
  their maps draw them: no more green wedges cut into orange fields. Every
  tile is also picked by what it shows (which panel's top or side face, in
  which floor), which cleared most wrong-floor tiles in every area; the
  Green HP's walkways keep their own darker shade (every one had been drawn
  from pieces of the platforms), and the Undernet's bridges turn and meet
  its plateaus in their joints with a yellow gem, as its maps draw them
  (they had left dark patches on the plateaus). Seaside's framed pads no
  longer hang a scrap of yellow boardwalk off their lower edge, and its
  layers take pieces in Seaside 2's colours for its yellow fields alone.
  `build.py tiles` checks all of it.
- MegaMan's first briefing on a layer names what the map's violet mark is:
  a strong virus signal and its Server, a dark warp into the Undernet, or
  the golden gate. In the Aquarium HP it says its battlefields' conveyor
  panels carry us along their arrows.
- Bystander navis stand two panels or more from anything else on a layer:
  one beside a Mystery Data took MegaMan's A, and each A that closed his
  words opened them again.
- The run's gift waits half a second before its three choices, so an A
  pressed through its last page no longer takes the first unread; and its
  chip is one that hits (Recov150 had been offered "hitting for 150").
- A random battle that shares a virus family with the last one is a third
  as likely (six of seven act 1 battles in the RoboDog Comp held Gunners).
- Act 1's guardian is BlastMan, DiveMan or SpoutMan, alike (a playtester
  met BlastMan in seven of eleven runs, DiveMan the only other), and a run
  never meets one guardian in two acts where another fits.
- MegaMan's tip on holding SELECT for the map is left out once the map has
  been held (it came on every run's first two layers).
- Central's layers can be a winding path two or three panels wide, as
  Central Area 1's, framed pads hung off its sides, in place of the chain
  of rooms on bridges. A raised room's stair never tops onto an octagon's
  cut corner.
- Central's, Seaside's and Sky's pads are their maps' framed pads, cut
  whole: a pale frame, a cyan band and a blue recess holding the
  centrepiece in Central, the same in yellow in Seaside and in lavender in
  Sky (they were drawn in the areas' platform floor). Sky's pads no longer
  hold a black blob: its gem's tile is the pit in one of its platforms.
- The Net Dealer before a guardian stands behind a counter in one of the
  rooms nearest the arena's antechamber when the antechamber (mostly a
  pad) has no place for one: 58 of 80 such dealers in the tests, from 20.
- HeatMan's warning says his fire tower crawls at us unlit and turns into
  our row, to sidestep it late (a playtester stepped off his lit leap into
  its path and lost on HeatMan's last 29 HP); his flamethrower sweeps the
  lit row, and his leap's yellow panels are to be cleared.
- The Graveyard's slabs are one shade of stone throughout: its pale
  platforms' tiles, which the tiles mixed into its dark slabs panel by
  panel, are drawn in the dark slabs' colours.
- The map (SELECT) marks the way on in straight runs, as far as a straight
  line over the floor goes (it zig-zagged panel by panel across platforms),
  over the floor MegaMan has come near,
  up to where he has not been: a V in a comp's maze read as a dead end
  twice, the arm on to the exit nowhere on the map.
- The Aquarium Comp and the ACDC HP take one battle in three from their
  town's other area (the Aquarium HP, Central Area): seven Aquarium
  battles in a row were Piranhas and Quakers. MegaMan's first briefing
  there says an Aqua hit on its icy battlefields freezes us.
- Bystanders never stand corner to corner with a walkway's last panel,
  where one on a platform's corner looked to stand in the way in; services
  keep off it too unless their room has no other place (a quarter of them
  stood there). The layer tests take every area, the comps and homepages
  too.
- The Graveyard's slabs carry its cyan crosses, as its maps do: one on
  each small platform's middle and rows of them alongside the line of
  holes through a big slab, kept off rims and holes. The holes run in a
  line along the slab's middle, as the originals punch them, rather than
  in a grid.
- The Nest's platforms are its grey stone in a magenta lip with spikes,
  its walkways the magenta links with a yellow gem, as Underground's maps
  draw them (every floor there had taken the links' look). The Undernet's
  and the Secret Area's plateaus are their mauve stone in a red lip with
  spikes, carrying the Undernet's magenta crosses; the Undernet's had
  taken its links' look too.
- Six guardians' warnings name the moves they were missing, each checked
  against a recording of the fight: GroundMan bursting up under the lit
  panel and his drill missiles, ChargeMan's coal bombs and freight cars,
  ProtoMan's dash across a lit row, CircusMan's lion through the burning
  hoop, ElementMan's whirlwinds, logs and grass, and Colonel's slash
  across a zigzag of lit panels.
- Central's layers lay out combs as Central Area 2 does: five 1-wide
  catwalks side by side, long and short in turn, hung off a green walkway,
  gem pads on the short ones, a rung closing one loop, the middle one
  leading on to a plaza (the maze of turns there read as random floor).
  The plus-shaped platforms of the Nest, the Undernet, the comps and the
  homepages are built at last: their lattice never fit the map's window,
  so those layers had always fallen back to another layout.
- NaviCust programs turn with L and R as they are placed from the list: a
  run holds all six of BN6's rotation items from its start (BN6 hands them
  out over its story; a drafted Shield would not fit beside SuperArmor and
  nothing turned it). The first draft and MegaMan's bug line say so.
  DiveMan's warning says bombs still reach him under the water. HeatMan's
  names his flamethrower down the row and the shadow he leaps onto in a
  burst of fire (it named only his towers and lit panels).
- The pads carry the originals' centrepieces on their middle panel, walkable:
  the red gem, the link ring or the cube on its round base, in each surface
  area's colours. Green's potted bushes stand in the gaps between its
  parallel planks, every second panel, plain and in flower.
- The layers are furnished with their areas' own props, composed as the
  originals compose theirs rather than scattered: Green's giant cybertree
  in the floor at the back of a room with an avenue of cybertrees past the
  rim on both sides, the Undernet's statue between two braziers, the
  Graveyard's monument and rows of three gravestones in walled holes,
  rows of cybertrees past the rims in Central, Sky and Green, the WELCOME
  sign beside the Net Dealer's counter and a BBS in Seaside. The game's own
  map objects; a layer loads their sprites within the game's limits.
- DiveMan's warning says he hides under the water where nothing hits him,
  to strike when he surfaces, to stand in the back column when his wave
  lights the panels and that his torpedoes run in their shadows' row (it
  said only to move between rows). A new run's first guardian is another
  than the last new run's (one retry had left DiveMan twice running).
- The Net Dealer stands behind a counter, as the originals' dealers do: Sky
  Area 3's capsule in Central, Seaside and Sky, each in its own colours, and
  Green Area 2's NetCafe desk in Green, cut whole from the ROM with their
  walls, facing the camera one panel in from a platform's back edge, the
  aisle behind them walled off. MegaMan speaks to him across it. The first of
  the originals' props on generated floors (docs/LEVEL_DESIGN.md, Props);
  runs from an older build start their layer afresh.
- An area's card fades out when a chat opens under it: an A at the gift
  Prog beside the arrival drew the chat box under the card.
- Mystery Data, services and navis no longer stand one panel's gap behind
  a walkway: the floor's own wall hides such a gap, so from the walkway
  they looked a step away on a raised block, and were a walk round (15
  in a hundred stood so; under one now). Runs from an older build start
  their layer afresh.
- DustMan's warning names the broken panels he hurls back (their first
  hit stuns) and the breath that pulls MegaMan up close for a punch, and
  says to keep a Recover chip ready.
- MegaMan's word on a NaviCust bug says where bugs come from: a program
  over the board's edge or off the command line, a Plus part on it, or
  two of one color side by side.
- The layer-1 gift's program changes how a first act plays: SuperArmor,
  Custom1, Attack+1 or Charge+1 (MegFldr1, room for a Mega chip beside a
  starting folder, is gone).
- Where the act's guardian has no weak element, the Net Dealer who names
  the viruses' weakness now stocks a chip of it.
- The title: NEW GAME over a saved run asks first, in the game's own chat
  box ("Start a new run? We'd lose our Layer 5 run!", MegaMan's face, the
  text typed out, No chosen), where one press had ended the run. The
  build's version stands in the top-left corner (v0.1.0 alpha, and the
  commits since), for a report. The infinity mark is a size smaller; the
  brightest backdrops (the Sky's clouds,
  the Seaside) are dimmed further so the logo's white stays readable; and
  with no controller connected, ENTER is named under PRESS START.
- Without its ROM, a desktop build that can show no dialog (the Flatpak,
  whose sandbox has neither zenity nor kdialog, and SDL's box on Wayland is
  zenity's; the Steam Deck's big screen, where a pad cannot answer one)
  opens its own window on a "No ROM found" screen instead of quitting at
  once: where to put the ROM, a look again every three seconds and on A,
  and once it is there the game starts itself again. On a Steam Deck the
  Flatpak had shown its icon for a moment and gone. The screens before a
  ROM is found draw in a 3x5 font of the engine's own; a handheld's error
  without its ROM showed a row of bars.
- The site's downloads show the newest release, an alpha's pre-release too
  (GitHub's "latest" skips pre-releases, and the page said there was no
  release), and list the Flatpak for the Steam Deck.
- The Guardian Data's word on how the NaviCust's board works comes with a
  player's first draft ever (the profile keeps that it came), not only at
  act 1's guardian: a run carried over from an older build met its first
  draft at act 3's, without it.

## 0.1.0 (2026-09-27)

The first alpha, and the first release. Its players' notes are
[docs/releases/v0.1.0.md](docs/releases/v0.1.0.md).

- Sky HP and Green HP vary their battles: their own random battle is a
  single one (every act 2 battle on Sky HP was Gunner and FgtrPlne), so two
  in three come from the Sky's and the Green Area's, inside the act's
  limits. The pacing report shows nine kinds of virus on Sky HP in act 2.
- The NaviCust vendor stocks from the NaviCust's pool and tiers: no HP+400
  in the first cycle (it sold at 2300 zenny in act 2 beside the dealer's
  20-HP HPMemory at 1200), no SneakRun. A on the map takes the navi or
  Mystery Data most straight ahead, not the nearest in front (a bystander a
  little off MegaMan's line took A from the Mystery Data he faced). L
  names the Recovery Mr. Prog only below three quarters of MegaMan's HP,
  says that three ScrtData open the golden gate, and SpoutMan's warning
  names his whirl.
- The NaviCust is a run's second build (docs/NAVICUST.md). Every guardian's
  Guardian Data offers three programs of three builds (buster, hand, guard,
  field, HP), each with MegaMan's words for what it does, or B for
  BugFrags; the pool is 34 of BN6's programs by tier, the large ones as the
  board grows. The act 2 and act 4 guardians give an ExpMemry: the board
  grows from 4x4 to 5x4, then 5x5. When the NaviCust's bugs change,
  MegaMan names each and what it does (BN6's RUN says OK over a part left
  past the board's edge, which bugs).
- The Net Dealer says how an AquaNdl pick lands: its needles drop where the
  guardian stands a moment later, so fire when he stops (two of three
  missed a hopping BlastMan).
- Act 1 varies more. It is Central Area or the RoboDog Comp: the Robot
  Control Comp's battles that fit act 1 were all OldStove and Mettaur, and
  it waits for act 2, where Champy and Gunner join them. Act 1's guardian
  is BlastMan or DiveMan as likely (BlastMan was two in three), and a new
  run from the title takes the next seed's where its first guardian would
  be the last run's (the profile keeps it; an older profile reads as
  none). The pacing report lists each battle roll's virus families.
  `--guardian N` pins every area's guardian for a scripted capture, and
  `--net-biome` puts its area in the run's act as well.
- The way-on arrow leads around Mystery Data and navis where MegaMan
  stands right beside one (it walked him into it, from the object's own
  panel), and its look ahead keeps clear of them; the arrow test walks
  among them now, as round objects smaller than a panel.
- The way-on arrow keeps to the walk. It showed a turn's first panel when
  the walk cut a platform's corner, and a running MegaMan was two panels
  past the turn before it turned (a look every 15 frames, now 5), so on
  BlastMan's layer it led off the start platform's corner and round in
  circles. Its eight ways are the pad's: a walkway's run lies in the middle
  of a diagonal, where on the screen it lay a hair from "left" or "right",
  and the diagonals are drawn along the floor's. It leads around Mystery
  Data and navis. A ROM-free test follows it from every room of 120
  layers to the guardian or the exit pad (the old arrow lost 208 of 956
  walks). The playtest state names the arrow's way, and
  `CYBERWORLD_STATE_POS=map` draws the whole layer with its walk.
- A battle's viruses are counted from the battle the game set up: the
  pointer that names it kept the last battle's until the setup wrote it, so
  a battle after a re-roll could count the other battle's viruses (AREA
  CLEAR showed 11 of 12). The run log writes a battle once it is known.
- A Recovery Mr. Prog talked to again on a layer heals in one box ("ALL
  PATCHED UP!") instead of his whole greeting. BlastMan's warning names his
  flame dash and fire wall beside the bombs.
- The application ID is the repository's name,
  `io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless`, as
  Flathub asks. The AppImage moves a menu entry it made under the old ID,
  and `install.sh` replaces its old entry and icons.
- A Flatpak (`linux/flatpak/`, `build.py flatpak`, on every tagged
  release): the Steam Deck's way to install software, from Discover. It
  reads the ROM from Downloads and EmuDeck's and RetroDECK's folders, read
  only, and keeps its saves in its own folder.
- Steam Deck: started by Steam's Gaming Mode or Big Picture (or in
  gamescope) the game fills the screen, 5x on the Deck (a window was scaled
  to it by a fraction, and blurred), and skips the AppImage's menu offer.
  The ROM is looked for in EmuDeck's and RetroDECK's gba folders, on the
  Deck and its SD card, in ~/ROMs and in Downloads, found by its contents
  and copied in, before any dialog. On a controller, Back and Start held a
  second ask to quit, and held again quit, as Escape does on a keyboard.
- The Net Dealer is the only green armored navi on a layer: bystanders are
  the game's dark EvilNavi and the NaviCust vendor its pink GirlNavi. The
  sprites the bystanders and the vendor had were the dealer's on the map
  (their chat faces differed, which hid it). The
  vendor, talked to again, says "More programs? Take a look!" and opens
  his list, as the dealer does.
- The Net Dealer's answer is a Standard chip (he offered two DiveMan, and a
  folder takes one), the best of eight found rather than four (half of act
  1's Elec answers hit past a third of the guardian), and always of the
  element he names (a few layers had none). One over that third he brings
  alone, and says so.
- Net Dealers, vendors, Mr. Progs and bystanders stand off the walkways'
  mouths: a dealer stood where a walkway met his platform, and the way
  on went through him (the platform's box took in the walkway's end).
  Runs saved before this version continue their layer from its start.
- A battle starts MegaMan on the panel its original battle gives him:
  column 2 row 2 on most fields, beside it where that panel is a hole or
  poison (he always stood on column 2 row 2, in the hole too). The next battle, re-rolled every five seconds on a layer, is
  written to one of two records in turn, so a battle the game has rolled
  but not yet set up keeps its own field, foes and MegaMan's panel.
- The NaviCust is in the PET from the start of a run (MegaMan, then
  NaviCust): the gift's and the vendor's programs could not be installed
  before. The gift, the vendor and a bystander say where to install them.
- R near the town's statue says which way the statue is and shows the
  way-on arrow, and the townsfolk stand off the statue's approach.
- Walkways without lining up: holding one direction at a walkway's mouth
  or a spur lines MegaMan up and takes him in (never back into the one he
  came out of); a diagonal held along a walkway follows it round its
  turns, stepping to the middle of a lane first when he is off it; a
  diagonal held into a corner slides him round it, and a push into a navi
  frees him. A key held into a platform's corner leaves him there (it
  walked him along the edge and back), and pushing a while where the pad
  goes nowhere shows the way-on arrow. Starting to walk from rest off a
  panel's middle no longer steps MegaMan sideways first: the help read
  the game's two frames before he moves as being stuck.
- Walkway mouths' corners are walls of the corner's shape, as the
  originals have them; each was two edges in one cell, and the game took
  either, pushing MegaMan out sideways at a mouth's side.
- A talks to the navi or Mystery Data MegaMan stands at even when he faces
  past it: he turns to the one before him, else the nearest (walking into
  a navi slides him round it), and reaches about 54 units, which is how
  far a short Mr. Prog can be while looking a tile away. One a step or two
  short (up to two and a half panels before him) he walks up to and talks
  to; the pad or B stops the walk. With two navis side by side, A talks to
  the one he means (the game's check took the first its probe touched).
  Of those within reach, one on the side he faces comes before a nearer
  one behind him (a vendor a step behind turned him from Mystery Data).
- A navi's tip on running from a battle says how (on the Custom screen, hold
  L and press R); it said START, which only pauses.
- An act's first layer from act 2 on always has a Net Dealer, and every
  Net Dealer stocks two of a chip of the element that answers the act
  (its guardian's weakness, else its viruses'), the hardest hitting of a
  few that takes at most a third of the act's guardian, at a price a run
  has by then (400 zenny in act 1, 300 more an act),
  and says which, naming the guardian, and that it has two. A
  bystander's tip on where Net Dealers set up no longer says the middle
  layer only. Talked to again, the Net Dealer says "Back for more?" and
  opens his list. A shop's list takes no A for its first moment, so the A
  that closed its keeper's words twice over no longer asks to buy the
  first item.
- In battle, an L or R pressed just before the Custom gauge fills opens
  the Custom screen when it does (the game drops it; a playtester
  re-pressed in every fight).
- Every Mystery Data opens: some on a new run's first layer stood at the
  world's origin or said they were locked and printed stray text, their
  picks shared with the game's own Mystery Data of other maps.
- A Navi chip's version mark reads "EX" or "SP" in chat boxes: the chat
  font draws each mark as two letters stacked in one cell, which read as a
  kanji ("ProtoMn" and one came out of a Mystery Data), and now draws them
  side by side.
- Act 2's guardian is one of 600 to 700 HP (HeatMan, SpoutMan,
  CircusMan): EraseMan's 800 after BlastMan's 400 was a wall. Guardians
  are drawn from every navi whose HP suits the act, the area's own twice
  as likely, so act 1 is not always BlastMan (DiveMan too). A guardian
  of no element has the Net Dealer stock his hardest hitter, two of it,
  and say so. MegaMan's first word on a guardian's layer warns of its way
  of fighting, for every guardian (EraseMan's ghosts and his erasing
  blow, BlastMan's rolling bombs, HeatMan's fire towers and lit panels
  and more), each written from a fight watched.
- The NaviCust vendor's programs cost a quarter of the game's prices,
  which are its endgame's (2500 to 7100 zenny against a run's 100 to
  1000 a battle or Mystery Data), and 200 more an act; HP+200, which
  would triple a first act's HP, costs 2400 and 400 more an act.
- Servers, Net Dealers and other services stand off a room's exits, where
  they blocked the way on, and bystanders stand in the open, not in a
  panel-wide gap where they pinned MegaMan. None stands beside a floor
  of another height, where he looked a step away and was a stair's walk
  round. The map's key lists what the layer holds,
  Servers and dark warps as "Event", and shows the services MegaMan
  senses but has not reached: a ring where each stands, or a pip on the
  map's edge its way. An L pressed as a chat closes is heard, and a
  later L while MegaMan is hurt says where the Recovery Mr. Prog lies.
- A Server's prize is the hardest hitting of a few chips a tier better
  than Mystery Data (it could be a WhiCapsl). The prize and the Net
  Dealer's answer are chips that strike outright: an answer that needed a
  paralysed enemy (MchnSwrd) or a hole in the floor (SumnBlk) was no
  answer. The answer is no sword either: CircusMan kept to his back
  column, and two AquaSwrd never reached him.
- A new act eases in: every battle on its first layer comes from the lower
  half of its band (two BombCorns on the first layer after BlastMan took a
  playtester from 220 HP to 10).
- L says where the exit or the guardian lies when the walk there sets off
  another way ("The way winds, so follow the arrow!"), so the words hold
  still while the arrow shows the next stretch; the Recovery Mr. Prog,
  which heals every time, is named again while MegaMan is hurt. After a
  CONTINUE, L remembers it has told where they are, the map remembers what
  was seen, and the way-on arrow stays up for as many boxes as L speaks.
- Standing anywhere on the exit pad takes MegaMan on: its trigger covers
  the pad's whole panel (on its rims he stayed before).
- The map (hold Select) fills the screen: the floor seen so far with its
  panels apart, so walkways read as lines, the whole of it when it fits,
  marks for MegaMan, the exit, heals, shops and the guardian with a key
  under it, and until the goal is seen a mark on the frame the way it lies.
- A story for the run, the Endless Net: Dad's call on the first run, Lan
  and MegaMan talking on arriving somewhere new (the first layer, the
  Undernet, the Graveyard, the Nest, the side layers, the net rebuilt
  after the Nest as Net V2 and on), Chaud's call after the Secret Area,
  and the guardians as the Nest's copies of MegaMan's old battles. Every
  chat box shows its speaker's face: Lan, MegaMan, Dad, Chaud, Mr. Prog,
  the townsfolk, the bystanders, the Net Dealer's Normal Navi, the
  NaviCust vendor's technician navi (with its own shop screen) and the
  guardians. On the map L asks MegaMan where they are, what guards the
  area and how much ScrtData they carry.
- Bystander navis are other divers, with lines for the early, middle and
  deep net and for a rebuilt one; no two on a layer say the same thing,
  and every hint is true of this game. Services, choices and rewards talk
  like the game does ("MegaMan got: ..."), the Secret Area gate counts
  your ScrtData, and the Crosses and BeastOut come with their own words.
- The run summary shows Lan, where and by whom MegaMan was deleted, the
  layer reached and a new best; depths read "Layer N" everywhere. Area
  cards fit long names, and cards wait out shops, the PET and battles.
- Chat boxes that run past three lines turn the page evenly (two and two,
  never three and a lone line), at the end of a sentence where they can.
- From layer 20 on, acts begin where they should again: the second cycle's
  act cards, easy opening battles, Net Dealer layers and layouts were one
  layer off. The NaviCust vendor's and the gift's programs always come
  from the game's full program list at their own prices, however many
  layers came before.
- Aquarium Comp's glass pools keep their whole rims: where a water channel
  meets one, the channel ends at the rim, instead of its water stepping
  over the rim and the glass in 8x8 blocks. Each pool and the rest of the
  floor are drawn on their own and laid over each other on the game's two
  tile layers, as the original sets its pools on legs apart from the water.
  The guardian's arena is a pool too, no longer a patch of glass in water.
  `build.py atlas` draws the two tile layers in the game's order, the
  second over the first.
- Joins look like the originals'. The tiles are learned from the originals
  as the game shows them, the second tile layer in front of the first, and
  a tile keeps what the originals set in front of a floor on that layer only
  where the floor needs it. Pieces of it no longer come along: Seaside's
  boardwalk railings across its platforms, Central's bridge, the broken red
  ornaments on Green's and Central's pads, CopyBot's spikes over its rims.
  Green's planks end in front of the grass's side face or behind its rim,
  and CopyBot's plateaus and red pads are drawn whole with their walkways
  ending at them (no more blocks of plateau face, nor rim strips in the red
  floor), as the Aquarium's pools are. The Judge Tree no longer draws its
  courts' flat red where a catwalk meets a room, nor stump rings on its
  pads, and Sky's pads lose the pieces of its round pods. A floor's corners
  may keep the outline the originals draw a pixel past a side face, so the
  atlas counts no fallbacks in any area. Floors, walls and objects are
  unchanged.
- Where two of Central's floors meet at an inner corner, the panels there
  no longer show scraps of yellow-green floor: they were pieces of Central
  Area 2's raised yellow plateau, whose corners fitted those joins better
  than any green floor's. Tiles of an area's other maps that draw colours
  its own map never shows on its floors are left out everywhere, so the
  brown and orange scraps at the ends of the Undernet's bridges, the
  orange blocks beside Robot Control Comp's walkways, the dark posts where
  Sky's walkways meet its platforms and the green arrow pieces under
  Mr. Weather Comp's edges are gone too. Seaside keeps Seaside 2 and 3's
  yellow panels, the only fields of its second floor, which draw its
  guardian's arena. `build.py atlas` counts such tiles per layer (other
  colours). Floors, walls, objects and scenery are unchanged.
- Robot Control Comp's white platforms keep their rims: the two light
  bands that frame the original's platforms run all the way round, with
  the dotted corner caps, where the layers showed flat white fields with
  ragged, broken edges. The bands reach half a panel into the floor,
  deeper than a tile is held to the plain floor's look, so the edges now
  take the original's own tiles; and the walls around the grey cubes on
  Comp 2's platforms no longer count as holes when its tiles are learned,
  so inner corners keep the bands too. Pads are small white platforms in
  the look of Comp 1's and the Pavilion's raised ones, no longer half the
  striped conveyor from before the robot's door. The atlas counts less
  than half the near misses and seams it did in the area. Floors, walls,
  objects and scenery are unchanged.

- The run begins in the real world, in Central Town or ACDC Town, each
  cut into pieces from the game's own map and set out again per run.
  Central Town: Lan steps out of his front door among Capcom's houses,
  Aster Land, the Academy gate, the Expo gates, the bus stop and the plaza
  with its trees and blue bird statue, the plaza, the main road and the
  houses' court sometimes wider. ACDC Town: he comes up the Metroline's
  stairs among the park with its squirrel statue, Higsby's, Dex's and
  Mayl's houses and the Ayano mansion, the blocks sometimes in another
  order. Townsfolk stand where they belong and talk about the place, and
  the houses, shops, signs and statues answer A. At the statue (or ACDC
  Town's doghouse) R plays the game's own jack-in to the first layer. The
  real world's maps, their tables, objects, checks and jack-in
  destinations are documented in docs/OVERWORLD.md and docs/ROM_DATA.md;
  `build.py town` and `build.py world` show them.
- Keyboard: the layout of Capcom's PC version (the Legacy Collection). W A S D
  move, J and K are A and B, Q and E are L and R, Enter is Start, R is Select;
  the arrows with X and Z still work. Keys are physical positions (Z Q S D on
  AZERTY). `keys.ini` in the save folder remaps them. Escape asks before it
  quits, and keys held while the window loses focus are let go.
- Linux: an AppImage (runs on any x86-64 distribution with glibc 2.34+,
  without libfuse2, and offers to add itself to the application menu; its
  update information lets AppImageUpdate or Gear Lever fetch new releases)
  and a `.deb` that installs into the menu like any package, beside the
  tar.gz. All three carry an application icon and AppStream data, and the
  window's class is the app ID, so a pinned dock icon matches the running
  game. Without a ROM the first start asks for the file (a file chooser with
  zenity or kdialog) instead of stopping.

- The game itself runs from the player's BN6 Cybeast Gregar (USA) ROM on an
  embedded mGBA core: its battles, net, PET, shops, traders and music.
  Nothing from the ROM is shipped.
- Generated net layers in the game's own map formats: floors learned from one
  of the game's maps per area (Central, Seaside, Sky, Green, Graveyard,
  Undernet, Undernet Zero, Underground), with walls the game's collision
  reads. Each area builds its layers in its own layouts, after how BN6's net
  maps are laid out (docs/LEVEL_DESIGN.md): Central's routes, crater fields
  and catwalk mazes, Seaside's fields framed by comb boardwalks, Sky's
  mirrored hubs, Green's plank ladders, Graveyard's holed slabs, the
  Undernet's webs of long bridges and lattices of crosses. Walkways wear the
  area's second floor (blue catwalks, yellow boardwalks, orange planks). Pads
  on spurs and dead ends hide most of the Mystery Data. Tiles made for one place in the original (bridges, cut corners,
  decoration hanging off an edge) are not used on plain walkways. Sky and
  Undernet layers can raise a dead-end room onto a stair taken from the
  area's own map.
- Mystery Data, shops and traders draw from the whole chip library: every
  standard chip, the Megas and, deep in a run, the Gigas, by rarity.
- The story's Robot Control, Aquarium, Judge Tree, Mr. Weather and CopyBot
  comps and the ACDC, Green and Sky homepages join the first four acts, with their own battles, music,
  backgrounds and guardians. Every area now learns its floors from all of
  its maps in the same tiles and at each of their heights, so raised pools,
  fields and platforms have floor to copy; holes that faces hang over are
  told from floor by the walls around them. The originals' free-standing
  scenery (the Aquarium's coral, shells and starfish) stands beside the
  floor, and pads take the look of the original's pads (the Aquarium's
  yellow frames on legs). CopyBot's comp, whose floors no colour tells
  apart, learns them by shape: purple plateaus in stone rims with pods
  beneath, pink and white walkways with teal discs, magenta octagon pads. Runs saved before carry over.
- Three more areas for the first four acts, built from the game's computers
  and homepages: a comp in orange and green, a homepage in pink and teal and
  a comp in blue and pink, each with its own battles, background and
  guardians (CircusMan, Colonel, BlastMan, ElementMan, JudgeMan, DiveMan).
  SpoutMan and TenguMan join Seaside's and Sky's guardians, and most areas
  alternate between two battle backgrounds.
  Every guardian leaves three HPMemory. Runs saved before this version carry
  over.
- Each layer places the game's own exit pads, Mystery Data, Normal Navis,
  Mr. Progs, the Net Dealer, the program vendor, the Chip Trader and the
  BugFrag Trader, all running on the game's NPC and text scripts. Chip
  Traders speak the game's own lines and hand out chips from its own prize
  pools, stronger with depth; deeper layers can hold a Chip Trader Special
  (10 chips for 1). Their prize is a chip new to the Library: the game's
  three in four from those it has, a whole story's there, gave a run's
  near-empty Library the same chip (BlastMan B) trade after trade.
- Random encounters from the game's own roll and its own formations: each
  area fights the battles of its original maps, 28 of the 29 virus families
  (all but WindBox) where Capcom put them, on their battlefields (grass, ice,
  holes, poison), with versions that grow with depth; viruses the story
  meets late (the dragons, Nightmare) wait for the later acts. Every third
  layer ends in a guardian's arena, staged after Hades' bosses
  (docs/BOSSES.md): a safe room with a heal and the Net Dealer before it;
  the arena seals, the guardian logs in over the boss prelude with a title
  card and lines that remember earlier battles, and the battle starts
  without a question. Deleted, it says a last word and logs out; its
  Guardian Data holds the reward and taking it makes the exit appear. An
  area-clear card and the next area's title card mark each act.
- Exits are the game's own warp pads: MegaMan jacks out and into the next
  layer, built while he jacks out. The Undernet and the Secret Area are
  entered the same way.
- Beaten Navis give their Cross, and the Graveyard's guardian Beast Out,
  through the story's own flags; the game's chat box says so.
- MegaMan stays in the run: R does not jack out, the PET's Save is off, and
  any other map sends him back to the layer.
- Choices on the game's text boxes: a strong virus signal, a dark flame into
  the Undernet, and a gate that three ScrtData open into the Secret Area.
- A project site on GitHub Pages in Battle Network's own interface: the title
  screen beside its menu, act cards over each section, a ChipFolder of
  screenshots, the Net Dealer for downloads (the latest release, 0 zenny)
  and the PET's E-Mail for the docs. The player moved to /play/. Screenshots
  of the running game (`build.py screenshots`) now appear in the README and
  on the site, and short videos (`build.py clips`: the title, the net, a
  battle, a guardian logging in, the Undernet, jacking in) play on the site
  while they are on screen; extracted assets stay out of the repository.
- A browser build on GitHub Pages (https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/): the same
  game in WebAssembly. The player chooses their ROM, which the page checks
  and keeps with the saves in the browser's IndexedDB, never uploaded.
- CI on every push and pull request: every target built with -Werror, the
  unit tests under AddressSanitizer and UBSan, script and workflow linting.
  A push to main publishes the browser build; a v* tag publishes a release
  with the PortMaster zip, the Linux archive and the site.
- A key or button press shorter than a frame now counts.
- A Linux desktop build: the game in a resizable window at a whole-number
  scale, F11 or Alt+Enter for fullscreen, keyboard and controllers, saves
  and the ROM in `~/.local/share/cyberworld-endless`. It is built on Debian
  bookworm with its own SDL2 (backends loaded at run time), so it runs on
  glibc 2.34 and newer. `build.py run` plays it here; `build.py release`
  writes the PortMaster zip and the Linux archive.
- A fair difficulty curve (docs/PROGRESSION.md). Depth, not the area, sets
  how hard a battle is: every random battle is sized from the ROM's virus HP
  and damage to its act's limits, at the highest version that fits, and the
  first battles of a run and after each guardian are gentler. The first act
  visits one of the gentler areas and the hardest come fourth; the Undernet
  now comes before the Graveyard. Guardians are chosen by HP for their act
  (BlastMan, DiveMan or SpoutMan first; Colonel from act 4), and the act's
  card names its guardian. Every act's second layer has a heal and the Net
  Dealer. A guardian leaves five HPMemory, its own Navi chip and a full
  heal; a Mr. Prog on the first layer offers a gift; rich Mystery Data may
  hold an HPMemory; a won Server challenge pays a chip. `build.py pacing`
  checks every act against its limits, and `runlog.txt` records each battle.
- Colonel fought as the enemy table's unnamed navi 17 (4000 HP at V1); he is
  navi 18, and saved runs move over.
- A title screen of its own in the game's style: the Battle Network logo from
  the ROM with an infinity mark where the 6 stood and a CYBERWORLD ENDLESS
  plate, over the battle backgrounds of the net's areas in turn (Central to
  the Cybeast Nest), animated and scrolling as in battle. CONTINUE shows the
  saved run's depth, the corner the best one; choosing either jacks in with
  the net rushing past into white. After the game's GAME OVER a run summary
  shows over the area where MegaMan was deleted.
- Checkpoints on arrival at each layer; CONTINUE returns there. A profile
  keeps the best depth. Run saves from before this version are converted.
- PortMaster launcher for ROCKNIX, made for the Retroid Nova and the Retroid
  Pocket Flip 2.
- Floor edges no longer step: every tile is tested against the layer's own
  floor (Sky's raised maps measured theirs higher), and a tile map
  avoids tiles that the original maps never set side by side, or whose floor
  stops on a tile's edge. Central Area's corners, Sky HP's faces and the Sky
  arena's lower edges were the worst. Where a catwalk meets a platform some
  joins still show.
- Layers keep to shapes the original maps draw: before its tiles are
  picked, a layer's floor fills notches, trims stray cells and widens
  walkways' bends and branches into small platforms, without changing how
  anything connects. About a third fewer tiles are approximated.
- Aquarium Comp and Mr. Weather Comp look like their originals: the
  Aquarium is a maze of water channels between rimmed glass pads (its
  platforms drawn as pads, none wider than one), and Mr. Weather's comp one
  great slab with rooms reached by its conveyor belts, now its walkways.
- Robot Control Comp's platforms are its long white slabs joined by circuit
  walkways, as in the original, instead of wide octagons around a hub.
- Dev tools (docs/DEVTOOLS.md): Select+R opens a dev menu for test runs (no
  random battles, can't die, one-hit enemies, up to 8x speed, win the battle,
  heal, zenny, the next layer or guardian, a chosen area). `build.py atlas`
  draws every area's layers with a report on their tiles; `build.py tour` has
  the game show every room of each area.
