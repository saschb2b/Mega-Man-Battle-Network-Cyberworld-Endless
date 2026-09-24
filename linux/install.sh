#!/bin/sh
# Adds Cyberworld Endless to the desktop's application menu, pointing at this
# folder. Run it again after moving the folder; to remove the entry, delete
# the .desktop file it names.
set -e
here=$(cd "$(dirname "$0")" && pwd)
apps="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
mkdir -p "$apps"
cat > "$apps/cyberworld-endless.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Cyberworld Endless
Comment=A roguelike built on Mega Man Battle Network 6 (bring your own ROM)
Exec="$here/cyberworld-endless"
Path=$here
Terminal=false
Categories=Game;ActionGame;RolePlaying;
DESKTOP
echo "Added Cyberworld Endless to the application menu ($apps/cyberworld-endless.desktop)."
