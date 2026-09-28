Cyberworld Endless for Windows
==============================

A roguelike built on Mega Man Battle Network 6: a new net every run, played
on the real game. This is the Windows build of the ROCKNIX handheld port.
https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/

Your ROM
--------
You need Mega Man Battle Network 6: Cybeast Gregar (USA) as an unmodified
.gba file (SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6). Nothing from the
game is included here. The first start looks for it in your Downloads
folder and asks for the file if it is not there; it keeps a copy in
%LOCALAPPDATA%\cyberworld-endless\rom\. Any file name works: the game checks
the contents.

Start
-----
Run cyberworld-endless.exe (the installer adds it to the Start menu). It is
not signed, so Windows may say "Windows protected your PC" the first time:
choose More info, then Run anyway.

It opens in a window at the largest whole scale that fits; F11 or Alt+Enter
switches to fullscreen. Escape twice quits (the run is saved at the start of
each layer and when you quit while MegaMan is free to move), and so does
holding Back and Start on a controller for a second, twice.

Controls
--------
A keyboard or any game controller (Xbox, PlayStation, Switch and others).

  Game Boy Advance   Keyboard                Controller
  D-Pad              W A S D or the arrows   D-Pad or left stick
  A                  J or X                  A
  B                  K or Z                  B
  L                  Q                       Left shoulder or trigger
  R                  E                       Right shoulder or trigger
  Start              Enter                   Start
  Select             R or Backspace          Back / Select

This is the layout of Capcom's PC version (the Legacy Collection). Keys are
positions: on an AZERTY keyboard you move with Z Q S D. To change them, edit
%LOCALAPPDATA%\cyberworld-endless\keys.ini, which the first start writes.

Saves
-----
Saves, the ROM's copy and runlog.txt live in %LOCALAPPDATA%\cyberworld-endless\.
Uninstalling leaves them there; delete the folder to start over.
--data-dir FOLDER keeps them elsewhere.

Steam
-----
Steam's "Add a Non-Steam Game to My Library" (the Games menu) takes
cyberworld-endless.exe.

Mega Man Battle Network is (c) Capcom. This is an unofficial fan project, not
affiliated with or endorsed by Capcom. Code under the MIT license
(LICENSE.txt); it embeds mGBA (MPL-2.0) and SDL2 (zlib), see licenses\.
