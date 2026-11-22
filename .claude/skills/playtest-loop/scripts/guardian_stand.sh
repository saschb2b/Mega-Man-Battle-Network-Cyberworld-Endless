#!/bin/bash
# guardian_stand.sh NAVI BIOME SEED DEPTH HP [X Y DIR]: guardian NAVI on
# layer DEPTH (--guardian, so any area holds him) against a MegaMan of HP
# (--dev hp=N) who stands on his panel firing the buster (ACT overrides:
# ACT="wait 60" never shoots, so summons pile up); each hit with its
# second, and how long MegaMan lasts. A guardian's damage a second next to
# the persona's HP, before the session that meets him (lessons, s50, s52).
# X Y DIR: the way into the arena, from guardian_entry.sh (DEPTH and GUARD=1
# there too); the default is seed 3's layer 12 in ACDC HP.
cd "$(git rev-parse --show-toplevel)"
navi=$1; b=$2; seed=$3; depth=$4; hp=$5; n=stand$navi
python3 tools/play.py stop $n >/dev/null 2>&1; rm -rf .build/play/$n
python3 tools/play.py start $n --fresh --seed $seed -- --scene emu --net-biome $b --run-depth $depth --guardian $navi --dev quiet,hp=$hp >/dev/null 2>&1
python3 tools/play.py do $n "wait 500; place ${6:--189} ${7:-400} 5; wait 10; hold ${8:-DOWN+RIGHT} 60; wait 200" >/dev/null 2>&1
for i in $(seq 1 14); do python3 tools/play.py do $n 'press A; wait 40' 2>&1 | grep -q "doing battle" && break; done
s=$(python3 tools/play.py do $n 'wait 200; press A; wait 10; press START; wait 10; press A; wait 50' 2>&1)
echo "$s" | grep -q "^doing battle" || { echo "navi $navi: no battle (the way in?)"; python3 tools/play.py stop $n >/dev/null 2>&1; exit 1; }
prev=$hp; log=""; end="still standing after 80s"
for k in $(seq 1 80); do
	s=$(python3 tools/play.py do $n "${ACT:-mash B 60}" 2>&1)
	if ! echo "$s" | grep -q "^doing battle"; then end="fell after ~${k}s ($((hp / k)) HP a second)"; break; fi
	h=$(echo "$s" | grep "^hp" | awk '{print $2}' | cut -d/ -f1)
	if [ "$h" != "$prev" ]; then log="$log $((prev - h))@${k}s"; prev=$h; fi
done
echo "navi $navi depth $depth at $hp HP: $end; hits:$log"
python3 tools/play.py stop $n >/dev/null 2>&1; rm -rf .build/play/$n
