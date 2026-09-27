#!/bin/bash
# guardian_watch.sh NAVI BIOME SEED X Y DIR: MegaMan placed at X Y, DIR held
# into the arena, the intro read, one chip sent, then four sheets of the
# fight (every 8 frames, in god mode: he stands and takes every move). The
# sheets' paths are printed; scripts/montage.py NAVI packs them.
cd "$(git rev-parse --show-toplevel)"
n=w$1
python3 tools/play.py stop $n >/dev/null 2>&1; rm -rf .build/play/$n
python3 tools/play.py start $n --fresh --seed $3 -- --scene emu --net-biome $2 --run-depth 3 --dev god,quiet >/dev/null 2>&1
python3 tools/play.py do $n "wait 500; place $4 $5 5; wait 10; hold $6 60; wait 200" >/dev/null 2>&1
for i in $(seq 1 14); do
	python3 tools/play.py do $n 'press A; wait 40' 2>&1 | grep -q "doing battle" && break
done
# (the Custom screen slides in for a while: START and A before it are lost)
python3 tools/play.py do $n 'wait 200; press A; wait 10; press START; wait 10; press A; wait 150' >/dev/null 2>&1
for k in 1 2 3 4; do python3 tools/play.py do $n 'wait 190' --every 8 2>&1 | grep picture | sed "s/^/$1 $k /"; done
python3 tools/play.py stop $n >/dev/null 2>&1
