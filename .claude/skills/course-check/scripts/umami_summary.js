// The course check's numbers (SKILL.md, step 2): the game's opt-in
// statistics on the owner's Umami, summed for a period. Run in a tab of
// the Umami site the owner is signed in to, as Claude in Chrome's
// javascript_tool text (top-level await works there); set START (and END)
// first. It reads only: the owner's own session token from the page's
// localStorage, the events of the site named "Cyberworld Endless" and
// each event's data. It returns a summary, never a player's events.
// (Keys avoid the word "session": the tool blocks results that hold it.)
const START = '2026-10-06', END = null;   // ISO dates; END null: now

const tok = JSON.parse(localStorage.getItem('umami.auth'));
const H = { headers: { Authorization: 'Bearer ' + tok } };
const get = (p) => fetch(p, H).then((r) => r.json());
const sites = await get('/api/websites?pageSize=100');
const site = (sites.data || sites).find((s) => s.name === 'Cyberworld Endless');
const s = Date.parse(START), e = END ? Date.parse(END) : Date.now();

// every event of the period, a page of 200 at a time
const all = [];
for (let page = 1; page <= 100; ++page) {
	const r = await get(`/api/websites/${site.id}/events?startAt=${s}&endAt=${e}&pageSize=200&page=${page}`);
	all.push(...r.data);
	if (r.data.length < 200) break;
}
// each game event's data, eight requests at a time
const ev = all.filter((x) => x.eventType === 2 && x.eventName);
const queue = [...ev];
await Promise.all(Array.from({ length: 8 }, async () => {
	while (queue.length) {
		const x = queue.shift();
		const d = await get(`/api/websites/${site.id}/event-data/${x.id}`).catch(() => []);
		x.data = {};
		for (const k of d) x.data[k.dataKey] = k.dataType === 2 ? Number(k.numberValue) : k.dataType === 3 ? k.stringValue === 'true' : k.stringValue;
	}
}));

const tally = (a, f) => { const m = {}; for (const x of a) { const k = f(x); m[k] = (m[k] || 0) + 1; } return Object.fromEntries(Object.entries(m).sort((p, q) => q[1] - p[1])); };
const q = (a, p) => { const v = a.filter((x) => x != null && !isNaN(x)).sort((x, y) => x - y); return v.length ? v[Math.min(v.length - 1, Math.floor(p * (v.length - 1)))] : null; };
const st = (a) => ({ n: a.filter((x) => x != null).length, p25: q(a, 0.25), med: q(a, 0.5), p75: q(a, 0.75) });
const by = (n) => ev.filter((x) => x.eventName === n);
const players = (a) => new Set(a.map((x) => x.sessionId)).size;
const release = (t) => /Endless \d+\.\d+\.\d+$/.test(t || '') && !/0\.0\.1/.test(t);
const S = by('run-start'), E = by('run-end'), G = by('guardian'), O = by('online-visit');
const short = E.filter((x) => x.data.net === 'short'), endless = E.filter((x) => x.data.net === 'endless');
const perPlayer = (a, f) => Object.values(a.reduce((m, x) => { m[x.sessionId] = f(m[x.sessionId], x); return m; }, {}));
const guardianAt = (layer) => {
	const g = G.filter((x) => x.data.net === 'short' && x.data.layer === layer), won = g.filter((x) => x.data.result === 'won');
	return { won: won.length + '/' + g.length, hpLeft: st(won.map((x) => x.data.hp)), seconds: st(won.map((x) => x.data.seconds)) };
};
const md = E.filter((x) => x.data.md != null && x.data.layer > 0);
// what the runs chose (run-choices, since the first check's Phase 0): of
// each count, how many runs made it at all and its median among them
const C = by('run-choices');
const choiceKeys = ['trips', 'jobs', 'kept', 'vows', 'orders', 'skips', 'undernet', 'dark', 'duels', 'duelswon', 'netbattles', 'netbattleswon', 'edits'];
const choices = C.length ? Object.fromEntries([['runs', C.length],
	...choiceKeys.map((k) => [k, { runsWithAny: C.filter((x) => x.data[k] > 0).length, med: q(C.filter((x) => x.data[k] > 0).map((x) => x.data[k]), 0.5) }]),
	['mapSeconds', st(C.map((x) => x.data.mapseconds))], ['battleSeconds', st(C.map((x) => x.data.battleseconds))]]) : 'none yet';

({
	period: [START, END || 'now'],
	events: tally(ev, (x) => x.eventName),
	players: { started: players(S), oneRunOnly: perPlayer(S, (n) => (n || 0) + 1).filter((n) => n === 1).length },
	releaseRuns: S.filter((x) => release(x.pageTitle)).length,
	systems: tally(S.filter((x) => release(x.pageTitle)), (x) => x.urlPath),
	bn5: tally(S, (x) => x.data.bn5),
	setup: { folder: tally(S, (x) => x.data.folder), cross: tally(S, (x) => x.data.cross), helpers: tally(S, (x) => x.data.helpers) },
	maxThreatPerPlayer: tally(perPlayer(S, (m, x) => Math.max(m ?? 0, x.data.threat)), (v) => 't' + v),
	shortNet: { ended: short.length, won: short.filter((x) => x.data.won).length, wonAtThreat0: short.filter((x) => x.data.won && x.data.threat === 0).length + '/' + short.filter((x) => x.data.threat === 0).length,
		minutesWon: st(short.filter((x) => x.data.won).map((x) => x.data.minutes)),
		lostBy: tally(short.filter((x) => !x.data.won), (x) => x.data.by), lostAtLayer: tally(short.filter((x) => !x.data.won), (x) => x.data.layer + ':' + (x.data.by === 'viruses' ? 'v' : x.data.by === 'ProtoMan' ? 'P' : 'G')) },
	endlessNet: { ended: endless.length, by: tally(endless, (x) => x.data.by), layer: tally(endless, (x) => (x.data.layer >= 20 ? '20+' : x.data.layer >= 11 ? '11-19' : x.data.layer)), minutes: st(endless.map((x) => x.data.minutes)) },
	guardians: { won: G.filter((x) => x.data.result === 'won').length + '/' + G.length, layer3: guardianAt(3), layer6: guardianAt(6), layer9: guardianAt(9), nest: guardianAt(10),
		perGuardian: Object.fromEntries(Object.entries(G.reduce((m, x) => { (m[x.data.guardian] = m[x.data.guardian] || []).push(x); return m; }, {})).filter(([, a]) => a.length >= 8).map(([k, a]) => [k, a.filter((x) => x.data.result === 'won').length + '/' + a.length])) },
	protoManDeaths: { runs: E.filter((x) => x.data.by === 'ProtoMan').length, players: players(E.filter((x) => x.data.by === 'ProtoMan')), layer: tally(E.filter((x) => x.data.by === 'ProtoMan'), (x) => x.data.layer) },
	economy: { mysteryDataPerLayer: st(md.map((x) => x.data.md / x.data.layer)), act1MysteryData: st(E.filter((x) => x.data.net === 'short' && x.data.layer >= 4 && x.data.md1 > 0).map((x) => x.data.md1)),
		zennyWonShort: st(short.filter((x) => x.data.won && x.data.zenny != null).map((x) => x.data.zenny)), zennyAct1: st(E.filter((x) => x.data.layer >= 4 && x.data.zenny1 > 0).map((x) => x.data.zenny1)) },
	choices,
	online: { squarePlayers: players(O), visits: O.length, minutes: st(O.map((x) => x.data.minutes)), ended: tally(O, (x) => x.data.ended),
		netbattles: by('netbattle').length, netbattlePlayers: players(by('netbattle')), watched: by('online-watch').length, deepnetPosts: by('deepnet-post').length },
})
