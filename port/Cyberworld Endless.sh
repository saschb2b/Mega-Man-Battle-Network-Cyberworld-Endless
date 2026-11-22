#!/bin/bash

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}
if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source $controlfolder/control.txt
[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
get_controls
# (the update's messages, where an older PortMaster lacks them)
type pm_message > /dev/null 2>&1 || pm_message() { :; }
type pm_message_end > /dev/null 2>&1 || pm_message_end() { :; }
type pm_show_error > /dev/null 2>&1 || pm_show_error() { echo "$1"; }

GAMEDIR=/$directory/ports/cyberworld
PORT_LAUNCHER="$(readlink -f "${BASH_SOURCE[0]}")"
# (the game's own update: downloaded there by the game, put in place by
# update.py once it ends; docs/engine/updates.md)
UPDATEDIR="${GAMEDIR%/*}/.${GAMEDIR##*/}-update"
if [ -f "$UPDATEDIR/transaction.json" ] && [ -f "$UPDATEDIR/recovery.py" ]; then
  python3 "$UPDATEDIR/recovery.py" recover --game-dir "$GAMEDIR" --launcher "$PORT_LAUNCHER" || exit 1
fi

cd $GAMEDIR
> "$GAMEDIR/log.txt" && exec > >(tee "$GAMEDIR/log.txt") 2>&1

$ESUDO chmod +x "$GAMEDIR/cyberworld.aarch64"
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"
if command -v python3 > /dev/null && [ -f "$GAMEDIR/update.py" ]; then
  mkdir -p "$UPDATEDIR" && export CYBERWORLD_UPDATE_DIR="$UPDATEDIR"
fi

$GPTOKEYB "cyberworld.aarch64" &
pm_platform_helper "$GAMEDIR/cyberworld.aarch64"
./cyberworld.aarch64 --rom-dir "$GAMEDIR/rom" --data-dir "$GAMEDIR"

if [ -f "$UPDATEDIR/ready.txt" ]; then
  pm_message "Installing the update. Keep the device on."
  python3 "$GAMEDIR/update.py" apply --game-dir "$GAMEDIR" --launcher "$PORT_LAUNCHER" >> "$GAMEDIR/update.log" 2>&1
  update_result=$?
  pm_message_end
  if [ "$update_result" -eq 1 ]; then
    pm_show_error "The update failed. The previous version is kept: see update.log."
  fi
  if [ "$update_result" -eq 10 ] || [ "$update_result" -eq 1 ]; then
    pm_finish
    exec "$PORT_LAUNCHER"
  fi
fi
pm_finish
