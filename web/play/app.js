// The page around the browser build (cyberworld.js, from `build.py web`).
// The player's ROM and the game's saves live in IndexedDB under
// /cyberworld-endless (its own name: every project page of a github.io user
// shares one origin): the ROM file is read here, checked, written there and
// never leaves the browser.
'use strict';

const ROM_SIZE = 8 * 1024 * 1024;
const ROM_SHA1 = '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6';   // BN6 Cybeast Gregar (USA)
const DATA = '/cyberworld-endless', ROM_DIR = DATA + '/rom';

const $ = (id) => document.getElementById(id);
const canvas = $('screen'), stage = $('stage'), gate = $('gate'), statusLine = $('status');
let ready = false, started = false;

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
	onAbort: (what) => say('The engine stopped: ' + what),
};

function storedRom() {
	try { return Module.FS.readdir(ROM_DIR).find((n) => /\.gba$/i.test(n)) || null; } catch (e) { return null; }
}

// Play when a ROM is stored, else ask for one.
function offer() {
	const rom = storedRom();
	$('pick').hidden = false;
	$('play').hidden = !rom;
	$('pick-label').textContent = rom ? 'Use a different ROM' : 'Choose ROM file';
	say(rom ? 'Your ROM is ready in this browser.' : 'Choose your ROM to begin.');
	if (rom) $('play').focus();
}

function start() {
	if (!ready || started || !storedRom()) return;
	started = true;
	gate.hidden = true;
	fit();
	canvas.focus();
	Module.callMain(['--rom-dir', ROM_DIR, '--data-dir', DATA, '--window', '--size', '240x160']);
}

// ---- the ROM ----

async function sha1(bytes) {
	const digest = await crypto.subtle.digest('SHA-1', bytes);
	return Array.from(new Uint8Array(digest), (b) => b.toString(16).padStart(2, '0')).join('');
}

async function takeRom(file) {
	if (!file || started) return;
	if (!ready) { say('One moment: the engine is still loading.'); return; }
	say('Checking ' + file.name + '…');
	const bytes = new Uint8Array(await file.arrayBuffer());
	if (bytes.length !== ROM_SIZE) {
		say(file.name + ' is not an 8 MB GBA ROM. You need Mega Man Battle Network 6: Cybeast Gregar (USA).');
		return;
	}
	if (crypto.subtle && (await sha1(bytes)) !== ROM_SHA1) {
		say(file.name + ' is a different version or was changed. Only the unmodified Cybeast Gregar (USA) works.');
		return;
	}
	const FS = Module.FS;
	for (const name of FS.readdir(ROM_DIR)) if (name !== '.' && name !== '..') FS.unlink(ROM_DIR + '/' + name);
	FS.writeFile(ROM_DIR + '/bn6g.gba', bytes);
	persist();
	start();
}

$('rom').addEventListener('change', (e) => takeRom(e.target.files[0]));
$('play').addEventListener('click', start);
document.addEventListener('dragover', (e) => e.preventDefault());
document.addEventListener('drop', (e) => {
	e.preventDefault();
	if (e.dataTransfer.files.length) takeRom(e.dataTransfer.files[0]);
});

$('forget').addEventListener('click', () => {
	if (!confirm('Remove your ROM and all saves from this browser?')) return;
	const req = indexedDB.deleteDatabase(DATA);
	req.onsuccess = req.onerror = req.onblocked = () => location.reload();
});

// ---- the picture: 240x160 at the largest whole scale in device pixels ----

function fit() {
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
fit();

// the last writes before the tab goes away
window.addEventListener('pagehide', () => { if (ready) persist(); });
