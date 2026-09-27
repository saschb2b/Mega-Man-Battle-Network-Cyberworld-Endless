# Cyberworld Endless for Linux

A roguelike built on Mega Man Battle Network 6. This is the desktop build of
the ROCKNIX handheld port: the same game in a window on a Linux PC.

## Your ROM

You need **Mega Man Battle Network 6: Cybeast Gregar (USA)** as an
unmodified `.gba` file (SHA-1 `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`).
Copy it into either:

- `~/.local/share/cyberworld-endless/rom/` (created on the first start), or
- the `rom/` folder next to `cyberworld-endless`.

Any file name works; the game checks the contents. Nothing from the game is
included here.

## Start

```sh
./cyberworld-endless
```

`./install.sh` adds it to your desktop's application menu with its icon
(`./install.sh --remove` takes it away again). Without a ROM the first start
asks for the file. The first start with one records the game's boot once,
which takes a few seconds.

The releases also have this build as an AppImage and as a `.deb` package.

It runs on x86-64 distributions with glibc 2.34 or newer (Ubuntu 22.04,
Debian 12, Fedora 35 and later) and brings its own SDL2 in `lib/`, which
uses X11 or Wayland and PipeWire, PulseAudio or ALSA, whichever the system
has.

## Controls

A keyboard or any game controller SDL knows.

| Game Boy Advance | Keyboard | Controller |
| --- | --- | --- |
| D-Pad | W A S D or the arrows | D-Pad or left stick |
| A | J or X | A |
| B | K or Z | B |
| L | Q | Left shoulder or trigger |
| R | E | Right shoulder or trigger |
| Start | Enter | Start |
| Select | R or Backspace | Back / Select |

This is the layout of Capcom's PC version (the Legacy Collection). Keys are
positions: on an AZERTY keyboard you move with Z Q S D. To change them, edit
`~/.local/share/cyberworld-endless/keys.ini`, which the first start writes
with the defaults.

F11 or Alt+Enter switches between the window and fullscreen; Escape twice
quits (the run is saved at the start of each layer), and so does holding
Back and Start on a controller for a second, twice. `--fullscreen` starts
in fullscreen and `--window` in a window; started by Steam's Gaming Mode or
Big Picture it fills the screen.

## Steam Deck

Right-click the AppImage in Desktop Mode, make it executable (Properties,
Permissions) and choose **Add to Steam**; start it from Steam, which then
gives it the Deck's controls as a gamepad. A ROM in EmuDeck's
`Emulation/roms/gba` or RetroDECK's `retrodeck/roms/gba`, on the Deck or its
SD card, or in Downloads, is found by its contents and copied in.

## Saves

Saves, the boot recording and `runlog.txt` live in
`~/.local/share/cyberworld-endless/` (or `$XDG_DATA_HOME/cyberworld-endless`).
`--data-dir DIR` keeps them elsewhere; the handheld port's `savedata/` can be
copied in to carry a run over.

## Licenses

The code is MIT-licensed (`LICENSE`). The embedded GBA core is mGBA
(MPL-2.0) and the bundled SDL2 is zlib-licensed; both licenses are in
`licenses/`. Mega Man Battle Network is © Capcom; this project is not
affiliated with or endorsed by Capcom.
