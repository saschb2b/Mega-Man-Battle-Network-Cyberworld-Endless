#!/bin/bash
# guardian_scan.sh NAVI BIOME: the first odd seed whose layer-3 guardian in
# area BIOME (its index in src/core/run.c's pools) is NAVI, by name.
cd "$(git rev-parse --show-toplevel)"
navi=$1; b=$2; n=scan$b$navi
for s in $(seq 1 2 61); do
	python3 tools/play.py stop $n >/dev/null 2>&1; rm -rf .build/play/$n
	# (the dev's state names a guardian MegaMan has never battled, which a
	# fresh profile's shows as ???)
	CYBERWORLD_STATE_POS=1 python3 tools/play.py start $n --fresh --seed $s -- --scene emu --net-biome $b --run-depth 3 --dev god,quiet >/dev/null 2>&1
	g=$(python3 tools/play.py do $n 'wait 300' 2>&1 | grep -i '^guardian' | awk '{print $2}')
	python3 tools/play.py stop $n >/dev/null 2>&1; rm -rf .build/play/$n
	if [ "$g" = "$navi" ]; then echo "$navi biome $b seed $s"; exit 0; fi
done
echo "$navi biome $b: none in seeds 1-61"
exit 1
