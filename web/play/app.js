// The page around the browser build (cyberworld.js, from `build.py web`).
// The player's ROMs and the game's saves live in IndexedDB under
// /cyberworld-endless (its own name: every project page of a github.io user
// shares one origin): each ROM file is read here, checked, written there and
// never leaves the browser.
'use strict';

const ROM_SIZE = 8 * 1024 * 1024;
// (the engine's build, which build.py fills in: its .wasm fetched by it, so
// a browser never pairs a kept one with a new cyberworld.js)
const ENGINE_VERSION = '';
const DATA = '/cyberworld-endless', ROM_DIR = DATA + '/rom';
// The ROMs the game takes, each told by its header's game code, checked by
// its SHA-1 and kept under its own name in ROM_DIR, where the game finds
// BN5's beside BN6's (src/core/rom.c): BN6's, which a run needs, and BN5's,
// optional, whose net areas and their battles in BN5's own engine then
// join runs (docs/MULTIROM.md). The names and words of Android's ROM page.
const ROMS = {
	bn6: { code: 'BR5E', sha1: '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', file: 'bn6g.gba', tag: 'BN6 Cybeast Gregar (USA)' },
	bn5: { code: 'BRKE', sha1: '5f472f78d8de2df01d5039e045c043cb40969a39', file: 'bn5c.gba', tag: 'BN5 Team Colonel (USA)' },
};
// Battle Network 6's and 5's other versions, by their game code: a player
// who has one is told which it is
const OTHERS = {
	BR6E: 'BN6 Cybeast Falzar, not Gregar',
	BR6P: 'BN6 Cybeast Falzar (Europe), not Gregar (USA)',
	BR6J: 'Rockman EXE 6 Falzar (Japan), not BN6 Gregar (USA)',
	BR5P: 'BN6 Cybeast Gregar (Europe), not the USA version',
	BR5J: 'Rockman EXE 6 Gregar (Japan), not the USA version',
	BRBE: 'BN5 Team ProtoMan, not Team Colonel',
	BRBP: 'BN5 Team ProtoMan (Europe), not Team Colonel (USA)',
	BRBJ: 'Rockman EXE 5 Team of Blues (Japan), not BN5 Team Colonel (USA)',
	BRKP: 'BN5 Team Colonel (Europe), not the USA version',
	BRKJ: 'Rockman EXE 5 Team of Colonel (Japan), not the USA version',
};
const OTHER_GAME = 'not BN6 Gregar or BN5 Team Colonel';

const $ = (id) => document.getElementById(id);
const canvas = $('canvas'), stage = $('stage'), gate = $('gate'), statusLine = $('status');
let ready = false, started = false;
// a phone or tablet: the game takes the whole screen and draws its own buttons
// (?touch=1 or ?touch=0 decides it for a screen that tells it wrong)
const touchParam = new URLSearchParams(location.search).get('touch');
const touchPlay = touchParam ? touchParam === '1' : window.matchMedia('(pointer: coarse)').matches;

function say(text) { statusLine.textContent = text; }

// ---- the engine ----

let syncing = false, syncAgain = false;
function persist() {
	// IndexedDB takes one sync at a time; a write during one waits for the next
	if (syncing) { syncAgain = true; return; }
	syncing = true;
	Module.FS.syncfs(false, (err) => {
		syncing = false;
		if (err) console.warn('saving to IndexedDB failed', err);
		if (syncAgain) { syncAgain = false; persist(); }
	});
}

var Module = {
	canvas,
	locateFile: (path, prefix) => prefix + path + (ENGINE_VERSION && path.endsWith('.wasm') ? `?v=${ENGINE_VERSION}` : ''),
	print: (t) => console.log(t),
	printErr: (t) => console.warn(t),
	persist,
	preRun: [() => {
		const FS = Module.FS;
		FS.mkdir(DATA);
		FS.mount(Module.IDBFS, {}, DATA);
		Module.addRunDependency('idbfs');
		FS.syncfs(true, (err) => {
			if (err) console.warn('reading IndexedDB failed', err);
			try { FS.mkdir(ROM_DIR); } catch (e) { /* already there */ }
			Module.removeRunDependency('idbfs');
		});
	}],
	onRuntimeInitialized: () => { ready = true; offer(); },
	onAbort: (what) => { say('The engine stopped: ' + what); track('engine-stopped', { reason: String(what).slice(0, 60) }); },
};

// whether ROM `id` (ROMS) is kept in this browser
function kept(id) {
	try { return Module.FS.stat(ROM_DIR + '/' + ROMS[id].file).size === ROM_SIZE; } catch (e) { return false; }
}

// What is kept, in words
function keptWords() {
	if (kept('bn6') && kept('bn5')) return 'BN6 and BN5 are ready in this browser: BN5\'s net joins ours.';
	if (kept('bn6')) return 'Your ROM is ready in this browser. BN5 can join it: choose it or drop it here.';
	if (kept('bn5')) return ROMS.bn5.tag + ' is kept, for when BN6 is here. Choose your BN6 ROM to begin.';
	return 'Choose your ROM to begin.';
}

// Play when BN6 is kept, else ask for it; BN5 asked for beside it. `note`,
// where files were refused, says why in place of what is kept.
function offer(note) {
	const six = kept('bn6'), five = kept('bn5');
	$('pick').hidden = false;
	$('play').hidden = !six;
	$('pick-label').textContent = !six ? 'Choose ROM files' : five ? 'Use different ROMs' : 'Add BN5 Team Colonel';
	say(note || keptWords());
	if (six) $('play').focus();
}

function start() {
	if (!ready || started || !kept('bn6')) return;
	started = true;
	track('game-start', { input: touchPlay ? 'touch' : 'keys', bn5: kept('bn5') ? 'yes' : 'no' });
	gate.hidden = true;
	if (touchPlay) {
		// (the canvas takes its size from the page: SDL follows it, rotations too)
		document.body.classList.add('touch-play');
		canvas.style.removeProperty('width');
		canvas.style.removeProperty('height');
		if (document.documentElement.requestFullscreen) document.documentElement.requestFullscreen({ navigationUI: 'hide' }).catch(() => {});
		keepAwake();
		Module.callMain(['--rom-dir', ROM_DIR, '--data-dir', DATA, '--window', '--touch', '--smooth-motion', smoothOn() ? 'on' : 'off']);
		return;
	}
	fit();
	canvas.focus();
	Module.callMain(['--rom-dir', ROM_DIR, '--data-dir', DATA, '--window', '--size', '240x160', '--smooth-motion', smoothOn() ? 'on' : 'off']);
}

// the screen stays on while the game plays (asked again when the tab returns)
async function keepAwake() {
	try { if (navigator.wakeLock) await navigator.wakeLock.request('screen'); } catch (e) { /* not granted */ }
}
document.addEventListener('visibilitychange', () => { if (started && touchPlay && document.visibilityState === 'visible') keepAwake(); });

// ---- the ROMs ----

async function sha1(bytes) {
	const digest = await crypto.subtle.digest('SHA-1', bytes);
	return Array.from(new Uint8Array(digest), (b) => b.toString(16).padStart(2, '0')).join('');
}

// One file, told by its header's game code: BN6's or BN5's kept once its
// size and SHA-1 are right ({ id }), anything else refused ({ why, reason })
async function checkRom(file) {
	if (/\.(zip|7z|rar)$/i.test(file.name)) return { zipped: true, reason: 'zipped' };
	const head = new Uint8Array(await file.slice(0, 0xB0).arrayBuffer());
	if (head.length < 0xB0) return { why: 'not a GBA ROM', reason: 'other-game' };
	const code = String.fromCharCode(...head.subarray(0xAC, 0xB0));
	const id = Object.keys(ROMS).find((k) => ROMS[k].code === code);
	if (!id) return { why: OTHERS[code] || OTHER_GAME, reason: OTHERS[code] ? 'version' : 'other-game' };
	const changed = ROMS[id].tag + ', but changed: patched, trimmed or a bad dump';
	if (file.size !== ROM_SIZE) return { why: changed, reason: 'size' };
	const bytes = new Uint8Array(await file.arrayBuffer());
	if (crypto.subtle && (await sha1(bytes)) !== ROMS[id].sha1) return { why: changed, reason: 'changed' };
	// (BN6's in place of any .gba but BN5's: the page kept one ROM before)
	const FS = Module.FS;
	for (const name of FS.readdir(ROM_DIR))
		if (name === ROMS[id].file || (id === 'bn6' && /\.gba$/i.test(name) && name !== ROMS.bn5.file)) FS.unlink(ROM_DIR + '/' + name);
	FS.writeFile(ROM_DIR + '/' + ROMS[id].file, bytes);
	return { id };
}

// The files chosen or dropped: each kept or refused, and why; the game
// starts once BN6 is kept, unless a file was refused, whose reason stays
// to be read (Jack in then starts it)
let checking = false;
async function takeRoms(files) {
	if (!files.length || started || checking) return;
	if (!ready) { say('One moment: the engine is still loading.'); return; }
	checking = true;
	say('Checking ' + (files.length === 1 ? files[0].name : files.length + ' files') + '…');
	const refused = [];
	let took = 0;
	for (const file of files) {
		const r = await checkRom(file).catch(() => ({ why: 'could not be read', reason: 'unreadable' }));
		if (r.id) {
			++took;
			track('rom-accepted', { rom: r.id });
			continue;
		}
		refused.push(r.zipped ? file.name + ' is zipped: unzip it first.' : file.name + ': ' + r.why + '.');
		track('rom-rejected', { reason: r.reason });
	}
	checking = false;
	if (took) persist();
	if (!refused.length && kept('bn6')) { start(); return; }
	const lines = refused.slice(0, 4);
	if (refused.length > 4) lines.push('and ' + (refused.length - 4) + ' more.');
	offer(lines.concat(keptWords()).join('\n'));
}

$('rom').addEventListener('change', (e) => { takeRoms(Array.from(e.target.files)); e.target.value = ''; });
$('play').addEventListener('click', start);
document.addEventListener('dragover', (e) => e.preventDefault());
document.addEventListener('drop', (e) => {
	e.preventDefault();
	takeRoms(Array.from(e.dataTransfer.files));
});

$('forget').addEventListener('click', () => {
	if (!confirm('Remove your ROMs and all saves from this browser?')) return;
	track('forget-rom');
	const req = indexedDB.deleteDatabase(DATA);
	req.onsuccess = req.onerror = req.onblocked = () => location.reload();
});

// ---- the picture: 240x160 at the largest whole scale in device pixels ----

function fit() {
	if (touchPlay && started) return;
	const dpr = window.devicePixelRatio || 1;
	const full = document.fullscreenElement === stage;
	const w = full ? screen.width : Math.min(stage.parentElement.clientWidth, 240 * 8);
	const h = full ? screen.height : Math.max(160, window.innerHeight - 180);
	const k = Math.max(1, Math.floor(Math.min((w * dpr) / 240, (h * dpr) / 160)));
	canvas.style.setProperty('width', (240 * k) / dpr + 'px', 'important');
	canvas.style.setProperty('height', (160 * k) / dpr + 'px', 'important');
}

window.addEventListener('resize', fit);
document.addEventListener('fullscreenchange', fit);
$('fullscreen').addEventListener('click', () => {
	if (document.fullscreenElement) document.exitFullscreen();
	else stage.requestFullscreen().catch(() => {});
	canvas.focus();
});

// Smooth motion: the game's 60 frames mixed at each refresh of a 90, 144 or
// 165 Hz screen, which shows them unevenly otherwise (remembered here, and
// switched while the game runs)
function smoothOn() { try { return localStorage.getItem('cw-smooth') === 'on'; } catch (e) { return false; } }
function showSmooth() {
	const on = smoothOn();
	$('smooth').textContent = 'Smooth motion: ' + (on ? 'on' : 'off');
	$('smooth').setAttribute('aria-pressed', on ? 'true' : 'false');
}
$('smooth').addEventListener('click', () => {
	const on = !smoothOn();
	try { localStorage.setItem('cw-smooth', on ? 'on' : 'off'); } catch (e) { /* not kept */ }
	showSmooth();
	track('smooth-motion', { state: on ? 'on' : 'off' });
	if (started) Module.ccall('cw_set_smooth', null, ['number'], [on ? 1 : 0]);
	canvas.focus();
});
showSmooth();
fit();

// the last writes before the tab goes away
window.addEventListener('pagehide', () => { if (ready) persist(); });
