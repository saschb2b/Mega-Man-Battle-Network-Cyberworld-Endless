<h1 align="center">Mega Man Battle Network: Cyberworld Endless</h1>

<p align="center">
<b>A roguelike for Mega Man Battle Network 6.</b><br>
Jack MegaMan into a net that is generated anew every run, and see how deep he gets.
</p>

<p align="center">
<a href="https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/play/"><b>Play in the browser</b></a>
&nbsp;·&nbsp;
<a href="https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/">Project page</a>
&nbsp;·&nbsp;
<a href="https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases">Downloads</a>
</p>

<p align="center">
<a href="https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/actions/workflows/ci.yml"><img src="https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/actions/workflows/ci.yml/badge.svg" alt="CI"></a>
<img src="https://img.shields.io/badge/ROCKNIX-PortMaster-c0392b" alt="ROCKNIX handhelds through PortMaster">
<img src="https://img.shields.io/badge/Linux-AppImage%20%C2%B7%20.deb-2f6fb5" alt="Linux: AppImage and .deb">
<img src="https://img.shields.io/badge/browser-WebAssembly-6a4fb5" alt="In the browser, as WebAssembly">
<a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-3a9d23" alt="MIT license"></a>
</p>

<p align="center">
<a href="https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/clips/trailer.mp4"><img src="docs/clips/trailer-play.png" width="640" alt="The trailer, 21 seconds with sound: the Cyberworld Endless logo, a roguelike for Mega Man Battle Network 6. Watch the trailer"></a>
</p>

Everything you see and hear is BN6 itself, running from your own ROM: its
battles, chips, PET, shops and music. Cyberworld Endless builds the net
around it, one layer at a time, and keeps the run going.

- **A new net every run.** Layers of platforms and walkways in the style of
  BN6's areas, with Mystery Data, Net Dealers, Chip Traders and Mr. Progs on
  them, and the game's own viruses in its own random battles.
- **Acts and guardians.** Every third layer ends in a Navi's arena. The act
  card names the guardian, the Net Dealer stocks a chip that answers it, and
  MegaMan warns you how it fights.
- **A run that grows.** A gift to start, Guardian Data (HP, the Navi's chip
  and its Cross), BeastOut from the Graveyard, and rarer chips the deeper
  you go.
- **Deeper, harder, around again.** From the surface areas through the
  story's comps, the Undernet and the Graveyard to the Underground, then
  around again, harder.

<p align="center">
<img src="docs/clips/guardian.gif" width="480" alt="MegaMan steps into a guardian's arena in Robot Control Comp; the card reads Guardian of Robot Control Comp, BlastMan, The Living Blast">
</p>

<p align="center">
<img src="docs/screenshots/town-central.png" width="240" alt="Lan outside his house in Central Town; Dad calls: Lan, it's Dad. Have you got a minute?">
<img src="docs/screenshots/act-card.png" width="240" alt="Act 1: RoboDog Comp, circuits of a home comp, its guardian BlastMan">
<img src="docs/screenshots/net.png" width="240" alt="MegaMan on a generated layer of Robot Control Comp">
<img src="docs/screenshots/battle.png" width="240" alt="A battle against an OldStove and a Mettaur">
<img src="docs/screenshots/undernet.png" width="240" alt="A generated layer of the Undernet, a HeelNavi on its stairs">
<img src="docs/screenshots/area-clear.png" width="240" alt="Robot Control Comp: AREA CLEAR, BlastMan deleted">
</p>

It runs on ROCKNIX handhelds through PortMaster and was made for the Retroid
Nova (4:3) and the Retroid Pocket Flip 2 (16:9). The same game plays on
Android phones, tablets and handhelds, in a window on a Linux or Windows PC or
a Mac, and in a browser at
[saschb2b.github.io](https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/),
on a phone too.

## What you need

- A handheld running ROCKNIX with PortMaster installed, a Steam Deck, an
  x86-64 Linux PC (glibc 2.34 or newer: Ubuntu 22.04, Debian 12, Fedora 35
  and later), a 64-bit Windows 10 or 11 PC, a Mac with macOS 11 or newer
  (Apple silicon or Intel), an Android 5 or newer phone, tablet or
  handheld, or a current browser.
- **Mega Man Battle Network 6: Cybeast Gregar (USA)** as an unmodified `.gba`
  file, dumped from your own cartridge. Its SHA-1 is
  `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`. Cybeast Falzar, other regions
  and the Legacy Collection version do not work yet.

No download and no page contains Capcom data. Without the ROM there is no
game.

## Install

1. Download `cyberworld.zip` from the [releases](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases) and unpack it (or
   build it with `python3 build.py package`, see [Building](#building)).
2. Copy `cyberworld/` and `Cyberworld Endless.sh` into the handheld's
   `ports` folder (on ROCKNIX: `/storage/roms/ports/`).
3. Copy your ROM into `ports/cyberworld/rom/`. The file name does not matter;
   the game checks the contents.
4. Refresh the game list and start **Cyberworld Endless**.

The first start records the game's boot once, which takes a few seconds.
After that the title screen appears straight away.

### On a Linux PC

Three ways from the [releases](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases), each with a menu entry and an icon you
can pin to the dock:

- **AppImage** (any distribution): download
  `cyberworld-endless-x86_64.AppImage`, make it executable (`chmod +x`, or
  the file's Properties) and start it. On the first start it offers to add
  itself to the application menu. It does not need libfuse2.
- **.deb** (Debian, Ubuntu, Mint, Pop!_OS): download
  `cyberworld-endless_amd64.deb` and open it with your software centre, or
  `sudo apt install ./cyberworld-endless_amd64.deb`. It appears in the menu
  as **Cyberworld Endless**; remove it like any other package.
- **Flatpak** (any distribution with Flatpak): download
  `cyberworld-endless.flatpak` and open it with your software centre, or
  `flatpak install --user cyberworld-endless.flatpak`.

On the first start without a ROM it looks in Downloads and in EmuDeck's and
RetroDECK's folders, then asks for the file (a file chooser when `zenity`
or `kdialog` is installed), and keeps a copy in
`~/.local/share/cyberworld-endless/rom/` (the Flatpak's in
`~/.var/app/io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless/data/`), where your saves
also live. It opens in a window at the largest whole scale that fits; F11
or Alt+Enter switches to fullscreen.

With Steam installed, the first start from the desktop offers to add the
game to Steam as well, with its library artwork; `--add-to-steam` does it
later (the AppImage: `./cyberworld-endless-x86_64.AppImage --add-to-steam`;
the Flatpak: see the Steam Deck steps below).

`cyberworld-endless-linux-x86_64.tar.gz` is the same game as a plain folder:
unpack it anywhere and run `./cyberworld-endless`; `./install.sh` adds it
to the menu. Its `README.md` has the keyboard keys.

### On Windows

From the [releases](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases), for 64-bit Windows 10 and 11:

- **Installer:** `cyberworld-endless-setup-x64.exe` installs the game for
  you alone (no administrator), adds it to the Start menu and, if you like,
  the desktop; Settings > Apps removes it again.
- **Zip:** `cyberworld-endless-windows-x64.zip` holds the same game in a
  folder: unpack it anywhere and run `cyberworld-endless.exe`.

The game is not signed, so Windows may say "Windows protected your PC" the
first time: **More info**, then **Run anyway**. The first start looks for
the ROM in Downloads and asks for the file if it is not there; it keeps a
copy in `%LOCALAPPDATA%\cyberworld-endless\rom\`, where the saves live
too (uninstalling leaves them). It opens in a window at the largest whole
scale that fits; F11 or Alt+Enter switches to fullscreen. Keyboards and
controllers (Xbox, PlayStation, Switch) work as on Linux. Steam's own **Add
a Non-Steam Game** takes `cyberworld-endless.exe`.

### On a Mac

`cyberworld-endless-macos.dmg` from the [releases](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases) holds one app for Apple
silicon and Intel Macs, macOS 11 and newer: drag **Cyberworld Endless** into
Applications. Apple has not notarized it (that takes a paid developer
account), so the first start is refused: open **System Settings > Privacy &
Security**, choose **Open Anyway** beside Cyberworld Endless and confirm (on
macOS 14 and older, Control-click the app and choose **Open**). The first
start looks for the ROM in Downloads and asks for the file if it is not
there; the ROM's copy, the saves and `keys.ini` live in
`~/Library/Application Support/cyberworld-endless/`.

### On a Steam Deck

The Deck runs the Linux build, best as a Flatpak, the way SteamOS installs
software:

1. In Desktop Mode, download `cyberworld-endless.flatpak` from the
   [releases](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases)
   and open it: Discover installs it (and the runtime it needs from
   Flathub).
2. Add it to Steam with its library artwork (the capsules, the banner, the
   logo and the icon): open **Konsole** and paste

   ```bash
   python3 "$(flatpak info --show-location io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless)/files/share/cyberworld-endless/steam/add-to-steam.py"
   ```

   It asks to close Steam for a moment (Steam reads its games only when it
   starts), adds the game and opens Steam again; run it again after
   reinstalling, add `--remove` to take the entry away. Without the
   artwork: right-click **Cyberworld Endless** in the application menu and
   choose **Add to Steam**.
3. Return to Gaming Mode: the game is in your library under **Non-Steam**,
   and Steam hands the Deck's controls to it as a gamepad.
4. The ROM: with EmuDeck or RetroDECK there is nothing to do. The game looks
   in `Emulation/roms/gba` and `retrodeck/roms/gba`, on the Deck and on its
   SD card, finds the ROM by its contents and keeps a copy of its own.
   Otherwise put the `.gba` file (unzipped) into Downloads, or choose it on
   the first start in Desktop Mode.

The AppImage works too: right-click it, **Properties**, **Permissions**,
**Is executable**, and start it once in Desktop Mode. It offers to add
itself to the application menu and to Steam, with the artwork.

In Gaming Mode it fills the screen, at 5x (1200x800) on the Deck's 1280x800.
The Deck's A, B, L1 and R1 are the GBA's A, B, L and R, the Menu button (☰)
is Start and the View button (⧉) is Select. To quit, hold View and Menu
for a second, then again; or use the Steam button's **Exit Game**. Saves
live in `~/.var/app/io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless/data/cyberworld-endless/`
(the AppImage's in `~/.local/share/cyberworld-endless/`) and survive
SteamOS updates.

### On Android

Download `cyberworld-endless.apk` from the
[releases](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases)
on the phone, tablet or handheld and open it (Android asks once to allow
installs from your browser or file manager). The first start asks for your
ROM with Android's own file picker; the app checks it and keeps a copy in its
own storage, beside your saves. A handheld's own controls, a Bluetooth or USB
controller and the touch screen all work: the game draws touch controls
round the picture until a controller's button is pressed, and Back asks
before it quits. Uninstalling the app deletes its saves.

### In a browser

Open **[the player](https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/play/)** and choose your ROM file, or drop it on
the page. The page checks it and keeps it, with your saves, in the
browser's own storage (IndexedDB); it is never uploaded. Next time **Play**
starts straight away. **Forget ROM and saves** removes both.

On a phone or tablet the game fills the screen and draws its own buttons
round the picture: a D-pad, A, B, L, R, Start and Select, under the picture
when the phone is upright and beside it when it lies on its side. A
controller or a keyboard puts them away until the screen is touched again.
Added to the home screen (the browser's *Add to Home screen* or *Install*),
it opens full screen like an app.

## Playing

On the title screen, press Start and choose **NEW GAME** or **CONTINUE**.
CONTINUE shows how deep your saved run is; the corner shows your best depth.

In the net, the buttons are the Game Boy Advance's and BN6 plays as it always
has. On a keyboard the layout is the one Capcom's PC version (the Legacy
Collection) uses: the left hand moves and opens the Custom screen, the right
hand uses chips and the buster.

| Button | Keyboard | Also |
| --- | --- | --- |
| D-Pad | W A S D | Arrow keys |
| A | J | X |
| B | K | Z |
| L / R | Q / E | |
| Start | Enter | Keypad Enter |
| Select | R | Backspace |

Keys are positions, so on an AZERTY keyboard you move with Z Q S D.
`keys.ini` in the save folder (`~/.local/share/cyberworld-endless/` on Linux,
`savedata/` on the handheld) changes them; it is written with these defaults
on the first start. F11 or Alt+Enter switches to fullscreen; Escape twice
quits. Controllers use their own buttons (A, B, shoulders, Start, Back);
holding Back and Start for a second, twice, quits. A touch screen shows the
buttons on it from its first touch (a Steam Deck's too), until a key or a
controller is used again.

| Button | In the net | In battle |
| --- | --- | --- |
| A | Talk, open Mystery Data | Use a chip |
| B | Run; in a chat, hold to fast-forward the text | Fire the buster |
| L | Ask MegaMan where you are and what's ahead | Open the Custom screen; on it, hold L and press R to try to run |
| R | Jack in (at the town's statue) | Open the Custom screen |
| Start | Open the PET | Pause |
| Select | Hold for the map of the layer so far: where you have been, the services (those MegaMan senses but you have not reached as rings, or pips on the edge), and the way to the exit or guardian | |

In the net, a direction held toward a walkway lines MegaMan up with it and
follows it round its turns, and A turns him to the navi or Mystery Data
beside him, or walks him up to one a step or two before him.

### A run

A new stretch of net has opened under the town: the Endless Net. Its
paths change every time someone jacks in, it only goes down, and
everything in it is copied from the real net, the guardians too, built
from every battle MegaMan has fought. Something at the bottom keeps
copying. Dad calls the first time; after that Lan and MegaMan talk it
over as they go, and L asks MegaMan where they are.

A run begins in town: Central Town or ACDC Town, Capcom's own, set out a
little differently each run, with shops, houses, townsfolk to talk to and
signs to read. Walk to the town's statue (the blue bird on Central Town's
plaza, the squirrel in ACDC Town's park) and press R: Lan jacks MegaMan
in, and the net begins.

Each layer is a new layout of platforms and walkways in the style of one of
the game's areas. Find the exit pad to go one layer deeper. On the way, the
game's own random battles come up, with the viruses of that area. How hard
a battle is depends on how deep you are, not on the area: the viruses grow
stronger (V2, V3, SP) act by act, and a battle never holds more than MegaMan
can be expected to handle at that point. The first battles of a run, and
the first after each guardian, are gentler. Rewards follow the Busting
Level as in BN6.

Three layers make an act. The second layer of every act always has the Net
Dealer and a Recovery Mr. Prog. The third ends in a guardian's arena, and
the room before it again has a heal and the Net Dealer. The card at the
start of each act names the guardian waiting at its end, so you can set
your folder for it: every Net Dealer keeps two of a chip that answers the
act (the guardian's weakness, or a hard hitter when it has none) and says
so, and on the guardian's layer MegaMan warns you of its way of fighting.
Step into the arena and the Navi logs in for the game's own boss battle.
Guardians remember how your earlier battles went. Their Guardian Data
ends with the way on: two areas for the next act, each named with its
guardian and his element, so you choose the fight your folder answers.

A run is **the short net**: three acts, then the Cybeast Nest on layer 10,
about an hour. When the Nest's guardian falls, the run is won, and that
opens **the endless net**: the long dive described below, repeating
harder after its own Nest.

The endless net's first four acts visit four of Central, Seaside, Sky and Green Area, the
Robot Control, Aquarium, Judge Tree, Mr. Weather and CopyBot comps, two home
computers and the Aquarium, ACDC, Green and Sky homepages. The order is
random, but the gentler areas come first (Central or a home computer) and
the hardest last (Sky, Mr. Weather,
ACDC HP, CopyBot's comp). Then come the Undernet and the Graveyard, and
layer 19 is the Underground. After that the cycle starts again, harder.

### Getting stronger

- **A gift to start.** On the first layer a Mr. Prog lets you pick one: two
  HPMemory, a ★3 chip or a NaviCust program. If your last run ended before
  its first guardian, he adds an HPMemory.
- **Guardian Data.** Every guardian leaves five HPMemory (+100 max HP), its
  own Navi chip at the version you beat (in `*` where BN6 has one and your
  folder doesn't use its letter), and its Cross where it has one.
  Taking it also restores MegaMan's HP.
- **The NaviCust.** Guardian Data also offers three NaviCust programs, one
  from each of three builds (buster, hand, guard, field, HP), or BugFrags
  if you take none. The second and fourth acts' guardians grow the board
  from 4x4 to 5x4, then 5x5. A layout that breaks BN6's rules runs bugged,
  and MegaMan says what the bug does.
- **Crosses.** Deleting HeatMan, ElecMan, SlashMan, EraseMan or ChargeMan
  gives MegaMan their Cross for the rest of the run, chosen in the Custom
  screen as in BN6.
- **BeastOut.** The Graveyard's guardian wakes the Cybeast, and BeastOut
  joins the Custom screen.
- **Chips.** Mystery Data, shops and traders draw from the whole chip
  library by rarity: Megas deeper down and, rarely, a Giga. Green Mystery
  Data holds chips, zenny and BugFrags; in deep layers a blue one may hold
  ScrtData.

### Places to find

| Place | What it does |
| --- | --- |
| Net Dealer (a green Normal Navi) | The game's shop: chips, an HP Memory and SubChips |
| NaviCust vendor (a pink navi) | NaviCust programs from the game's own shops |
| Chip Trader | Three chips in, one out |
| BugFrag Trader | The game's BugFrag trades |
| Recovery Mr. Prog | Restores HP |
| Server | A strong virus signal: an optional harder battle that pays a better chip. From the fourth act it may hold an SP Navi. Never on the first layer |
| Dark flame | Enters the Undernet: tougher viruses, and an exit one layer deeper |
| Golden gate | Three ScrtData open the Secret Area in Undernet Zero |

### What carries over

MegaMan starts every run as strong as the first time: what runs leave
behind is options. From the second run, NEW GAME opens a setup:

- **Net:** the short net, or the endless net once a short one is won.
- **Folder:** BN6's starting folder, or one opened in any run: Blade
  (swords in S for LifeSword), once any guardian falls, and Storm (Elec
  chips), once an Aqua guardian does. Chips the net gives lean to the
  codes the folder holds most.
- **Cross:** a Cross MegaMan has from the first battle, once its Navi
  (HeatMan, ElecMan, SlashMan, EraseMan, ChargeMan) has fallen as a
  guardian. It is the run's only Cross: guardians' Cross data won't fit
  beside it.
- **Threat:** rungs that each add one constraint (stronger viruses from act
  2, fewer heals, dearer dealers, EX guardians, SP Navis in Servers), each
  opened by winning on the one below.
- **Help:** two more HPMemory at the start, a heal on every layer, gentler
  battles. Helped runs count for everything.

Every chip MegaMan holds joins the Library, BN6's own, which every run's
PET shows whole: a Chip Trader's prize is new to it first, and the
summary counts what a run added. The run's summary names what it opened
and the closest goal. Milestones put BN6's own marks on the title:
Gregar's head for a won short net, Bass for the endless net's Nest, the S
for the Secret Area, the green disc for a win on the top threat rung, STD,
MEGA and GIGA COMP for a class of the Library complete.

### Saving and losing

The run is saved each time you arrive on a layer, and again when you quit
while MegaMan is free to move on a layer (not in a battle, a talk or a
guardian's scene); CONTINUE brings you back to where it was saved. The
PET's Save is switched off during a run. When MegaMan is
deleted the run is over: the title screen shows how deep you got, how many
viruses and Navis you deleted, and your best depth. When the short net's
Nest falls, the run is won.

Saves live in `ports/cyberworld/savedata/`. To give up a run without playing
it out, delete `savedata/run.sav`; your best depth is kept in `profile.sav`.

## Screen

The game's 240x160 picture is scaled by a whole number so it stays sharp: 5x
on the Nova's 1280x960 screen and 6x on the Flip 2's 1920x1080 screen, with
black borders around it. On a PC the window keeps the same rule as it is
resized.

## Troubleshooting

- **"Put your Mega Man Battle Network 6: Cybeast Gregar (USA) ROM in ..."**:
  the game found no ROM. Check that the `.gba` file is in
  `ports/cyberworld/rom/`.
- **"... is not a supported ROM"** or **"... is not an 8 MB GBA ROM"**: the
  file is a different game, version or region, or it is patched or
  compressed. Only the unmodified US Cybeast Gregar works.
- **Anything else**: the last start's output is in `ports/cyberworld/log.txt`.
  Please attach it when you report a problem.

## Building

For developers. Builds run in Docker (Debian trixie, matching ROCKNIX's glibc
and SDL2):

```bash
python3 build.py
```

```bash
python3 build.py test
```

```bash
python3 build.py package
```

```bash
python3 build.py run
```

`run` builds the Linux desktop binary and plays it on this machine in a
window, with the ROM from `~/.cache/mmbn-ref/roms` (or `CYBERWORLD_ROM_DIR`)
and saves in `.build/desktop`. `python3 build.py serve` builds the project
site and the browser version and serves them on `http://localhost:8080`;
`python3 build.py screenshots` retakes the screenshots in
`docs/screenshots` from scripted headless runs, and `python3 build.py clips`
records the site's short videos (WebM and MP4, `docs/clips`, and a GIF of
the guardian for this README) the same way. `python3 build.py release`
writes the release files to `build/release/`: `cyberworld.zip` for
PortMaster; for Linux the AppImage (with its `.zsync` for updates), the
`.deb` and the tar.gz; and `cyberworld-endless-web.zip`. Each target builds in its own Docker image
(`docker/`): the handheld on Debian trixie as ROCKNIX, the Linux desktop on
bookworm with SDL2 built to load X11, Wayland and the sound servers at run
time, the browser with Emscripten.

GitHub Actions (`.github/workflows/`) builds every target on each push and
pull request, runs the ROM-free tests under the sanitizers and lints the
scripts. A push to `main` publishes the browser build on GitHub Pages; a
tag such as `v1.0.0` publishes a release with all three archives.

`build.py shot` runs the game headlessly for scripted screenshots and soak
tests, `build.py atlas` draws every area's layers and `build.py tour` has the
game show every room of them. In the game, Select+R opens a dev menu (no
random battles, can't die, one-hit enemies, speed, skip to the next layer or
guardian). See [docs/DEVTOOLS.md](docs/DEVTOOLS.md) for the tools,
[AGENTS.md](AGENTS.md) for the development workflow,
[docs/LEVEL_DESIGN.md](docs/LEVEL_DESIGN.md) for how layers are laid out and
[docs/ROM_DATA.md](docs/ROM_DATA.md) for where the ROM data comes from.

## Credits

Mega Man Battle Network is © Capcom. This project is not affiliated with or
endorsed by Capcom. The code is MIT-licensed. The
[bn6f disassembly](https://github.com/dism-exe/bn6f) was an invaluable map of
the game's data; none of its files are included here. The game's own code
runs on an embedded [mGBA](https://github.com/mgba-emu/mgba) core (0.10.5,
MPL-2.0; its license ships in `licenses/`).
