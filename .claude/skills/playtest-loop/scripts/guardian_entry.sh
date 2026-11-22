#!/bin/bash
# guardian_entry.sh NAVI BIOME SEED: walks the autopilot to the guardian's
# arena and prints MegaMan's last positions before its staging begins (the
# way in, for guardian_watch.sh: start two positions back, hold the way
# they change: -Y UP+LEFT, +Y DOWN+RIGHT, -X DOWN+LEFT, +X UP+RIGHT).
# DEPTH (default 3) and GUARD=1 (NAVI guarding every area, for a guardian
# no area's layer-3 pool holds) the same for both scripts.
cd "$(git rev-parse --show-toplevel)"
n=en$1; b=$2; s=$3
python3 tools/play.py stop $n >/dev/null 2>&1; rm -rf .build/play/$n
CYBERWORLD_STATE_POS=1 CYBERWORLD_AUTOPILOT=1 python3 tools/play.py start $n --fresh --seed $s -- --scene emu --net-biome $b --run-depth ${DEPTH:-3} ${GUARD:+--guardian $1} --dev ${DEV:-god,quiet} >/dev/null 2>&1
python3 tools/play.py do $n 'wait 600' >/dev/null 2>&1
last=""
for i in $(seq 1 300); do
	out=$(python3 tools/play.py do $n 'wait 10' 2>&1)
	p=$(echo "$out" | grep '^pos' | awk '{print $2, $3}')
	c=$(echo "$out" | grep '^pos' | sed 's/.*cinema //')
	# (MegaMan's words in the room before the arena open a chat too: read
	# them and walk on)
	if echo "$out" | grep -q "^chat open" && ! echo "$out" | grep -q "^guardian .* staging"; then
		python3 tools/play.py do $n 'press A; wait 30; press A; wait 30' >/dev/null 2>&1
		if python3 tools/play.py do $n 'wait 1' 2>&1 | grep -q '^guardian .* waiting'; then continue; fi
	fi
	if [ "$c" != "0" ] || echo "$out" | grep -q "doing battle"; then echo "$1 b$b s$s: $last -> $p"; break; fi
	last=$(echo "$last | $p" | awk -F' [|] ' '{s=""; for (i = (NF > 4 ? NF - 3 : 1); i <= NF; ++i) s = s (s ? " | " : "") $i; print s}')
done
python3 tools/play.py stop $n >/dev/null 2>&1; rm -rf .build/play/$n
