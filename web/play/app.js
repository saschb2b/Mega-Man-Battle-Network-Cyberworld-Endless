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
const canvas = $('canvas'), stage = $('stage'), gate = $('gate'), statusLine = $('status'), app = $('app'), menu = $('menu');
let ready = false, started = false;
// a phone or tablet: the game takes the whole screen and draws its own buttons
// (?touch=1 or ?touch=0 decides it for a screen that tells it wrong)
const touchParam = new URLSearchParams(location.search).get('touch');
const touchPlay = touchParam ? touchParam === '1' : window.matchMedia('(pointer: coarse)').matches;

// the screen's line, and the menu's while it is open (refused files stand out)
function say(text, refused) {
	statusLine.textContent = text;
	statusLine.classList.toggle('refused', !!refused);
	$('menu-note').textContent = menu.hidden ? '' : text;
}

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

// When `path` was last written, as Nintendo's apps date a save: today's by
// its hour, an older one by its day; null where there is none
function writtenAt(path) {
	let t;
	try { t = Module.FS.stat(path).mtime; } catch (e) { return null; }
	return when(t instanceof Date ? t : new Date(t));
}
function when(d) {
	if (!d || isNaN(d)) return null;
	const today = new Date().toDateString() === d.toDateString();
	return today ? 'Today ' + d.toTimeString().slice(0, 5) : d.toLocaleDateString(undefined, { day: 'numeric', month: 'short' });
}

// What is kept, in words
function keptWords() {
	const add = touchPlay ? 'tap its slot' : 'click its slot or drop it here', run = writtenAt(DATA + '/savedata/run.sav');
	if (kept('bn6')) return (run ? 'Run saved ' + run + '.' : 'Ready.') + (kept('bn5') ? '' : '\nBN5 can join: ' + add + '.');
	if (kept('bn5')) return 'BN5 is in. Now BN6.';
	return touchPlay ? 'Choose your ROM files.' : 'Drop your ROM files here, or choose them.';
}

// Play when BN6 is kept, else ask for it; BN5 asked for beside it. `note`,
// where files were refused, says why in place of what is kept.
function offer(note) {
	const six = kept('bn6');
	// (once BN6 is in, the cartridges choose the files, as the game's ROMs screen's do)
	$('insert').hidden = six;
	$('pick').hidden = six;
	$('play').hidden = !six;
	for (const id of Object.keys(ROMS)) showCart(id);
	say(note || keptWords(), !!note);
	if (six) $('play').focus();
}

// ---- the cartridges: an open spot, or the cartridge with its game's face ----

const FACES = {   // the label's mugshot: sprite list 8 (src/launcher/launcher_art.c, docs/ROM_DATA.md)
	bn6: { lists: 0x031CC4, face: 0x37 },   // MegaMan
	bn5: { lists: 0x03272C, face: 0x45 },   // Colonel
};
const drawn = {};

function showCart(id) {
	const cart = $('cart-' + id), here = kept(id);
	cart.classList.toggle('in', here);
	cart.setAttribute('aria-label', ROMS[id].tag + (here ? ': ready' : id === 'bn6' ? ': needed, choose it' : ': optional, choose it'));
	if (!here || drawn[id]) return;
	drawn[id] = true;
	try {
		face(Module.FS.readFile(ROM_DIR + '/' + ROMS[id].file), FACES[id], cart.querySelector('canvas'));
	} catch (e) { /* (a label without its face) */ }
}

// GBA LZ77 (the BIOS's type 0x10), as src/core/rom.c reads it
function lz77(d, at) {
	if (d[at] !== 0x10) return null;
	const n = d[at + 1] | d[at + 2] << 8 | d[at + 3] << 16, out = new Uint8Array(n);
	let p = at + 4, o = 0;
	while (o < n) {
		let flags = d[p++];
		for (let i = 0; i < 8 && o < n; ++i, flags <<= 1) {
			if (flags & 0x80) {
				const v = d[p] << 8 | d[p + 1];
				p += 2;
				const len = (v >> 12) + 3, dist = (v & 0xFFF) + 1;
				if (dist > o) return null;
				for (let k = 0; k < len && o < n; ++k, ++o) out[o] = out[o - dist];
			} else out[o++] = d[p++];
		}
	}
	return out;
}

// A mugshot's first frame into `canvas` (40 x 48), read from the ROM as
// src/gfx/gfx.c reads a sprite: the list's pointer (compressed or not),
// its animation's first frame, its objects' tiles in their palette
function face(rom, where, canvas) {
	const u32 = (b, o) => (b[o] | b[o + 1] << 8 | b[o + 2] << 16 | b[o + 3] << 24) >>> 0;
	const list = u32(rom, where.lists + 8 * 4) - 0x08000000, ptr = u32(rom, list + where.face * 4);
	let b;
	if (ptr & 0x80000000) {
		const d = lz77(rom, (ptr & 0x7FFFFFFF) - 0x08000000);
		if (!d) return;
		b = d.subarray(8);
	} else b = rom.subarray(ptr - 0x08000000 + 4);
	const f = u32(b, 0), tiles = u32(b, f), pals = u32(b, f + 4) + 4, mini = u32(b, f + 8), objtab = u32(b, f + 12);
	const tdata = b.subarray(tiles + 4, tiles + 4 + u32(b, tiles));
	const objs = objtab + u32(b, objtab + b[mini + u32(b, mini)] * 4);
	const dims = [[[8, 8], [16, 16], [32, 32], [64, 64]], [[16, 8], [32, 8], [32, 16], [64, 32]], [[8, 16], [8, 32], [16, 32], [32, 64]]];
	const parts = [];
	for (let o = objs; !(b[o] === 0xFF && b[o + 1] === 0xFF) && parts.length < 64; o += 5) {
		if ((b[o + 4] & 3) > 2) continue;
		const [w, h] = dims[b[o + 4] & 3][b[o + 3] & 3];
		parts.push({ tile: b[o], x: (b[o + 1] << 24) >> 24, y: (b[o + 2] << 24) >> 24, w, h, hf: b[o + 3] & 0x40, vf: b[o + 3] & 0x80, bank: b[o + 4] >> 4 });
	}
	if (!parts.length) return;
	const minx = Math.min(...parts.map((p) => p.x)), miny = Math.min(...parts.map((p) => p.y));
	const ctx = canvas.getContext('2d'), img = ctx.createImageData(canvas.width, canvas.height);
	for (const p of parts)
		for (let y = 0; y < p.h; ++y)
			for (let x = 0; x < p.w; ++x) {
				const t = p.tile + (y >> 3) * (p.w >> 3) + (x >> 3), at = t * 32 + (y & 7) * 4 + ((x & 7) >> 1);
				const ci = at < tdata.length ? ((x & 1) ? tdata[at] >> 4 : tdata[at] & 15) : 0;
				if (!ci) continue;
				const c = b[pals + p.bank * 32 + ci * 2] | b[pals + p.bank * 32 + ci * 2 + 1] << 8;
				const X = p.x - minx + (p.hf ? p.w - 1 - x : x), Y = p.y - miny + (p.vf ? p.h - 1 - y : y);
				if (X < 0 || Y < 0 || X >= canvas.width || Y >= canvas.height) continue;
				const k = (Y * canvas.width + X) * 4;
				// (five bits to eight as src/gfx/gfx.h's bgr555 spreads them)
				const r = c & 31, g = c >> 5 & 31, bl = c >> 10 & 31;
				img.data[k] = r << 3 | r >> 2;
				img.data[k + 1] = g << 3 | g >> 2;
				img.data[k + 2] = bl << 3 | bl >> 2;
				img.data[k + 3] = 255;
			}
	ctx.putImageData(img, 0, 0);
}

function start() {
	if (!ready || started || !kept('bn6')) return;
	started = true;
	$('pet-place').textContent = $('menu-place').textContent = 'Game';
	document.body.classList.add('playing');
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
	window.scrollTo(0, 0);
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
// to be read (Play then starts it)
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
// (a cartridge's spot chooses its file, as the launcher's A does)
for (const id of Object.keys(ROMS)) $('cart-' + id).addEventListener('click', () => { if (ready && !started) $('rom').click(); });
document.addEventListener('dragover', (e) => e.preventDefault());
document.addEventListener('drop', (e) => {
	e.preventDefault();
	takeRoms(Array.from(e.dataTransfer.files));
});

// ---- the saves in one file: src/core/backup.c's .cwsave, which the phone
// apps take from the ROM folder too ----
//   "CWSAVE1\n", u64 stamp, u32 count, then count times (u16 length, its
//   name under the data folder, u32 size, its bytes), then an FNV-1a of it all

const SAVES_FILE = 'cyberworld-endless.cwsave', SETTINGS = ['settings.ini', 'keys.ini', 'pad.ini', 'touch.ini'];

function fnv(bytes, end) {
	let h = 2166136261;
	for (let i = 0; i < end; ++i) h = Math.imul(h ^ bytes[i], 16777619) >>> 0;
	return h >>> 0;
}

function isFile(path) {
	try { return Module.FS.isFile(Module.FS.stat(path).mode); } catch (e) { return false; }
}

// This browser's saves as a .cwsave's bytes, null where there are none
function packSaves() {
	const FS = Module.FS, names = [];
	try {
		for (const n of FS.readdir(DATA + '/savedata').sort())
			if (!n.startsWith('.') && !n.endsWith('.tmp') && isFile(DATA + '/savedata/' + n)) names.push('savedata/' + n);
	} catch (e) { /* no saves yet */ }
	if (!names.includes('savedata/profile.sav')) return null;
	for (const n of SETTINGS) if (isFile(DATA + '/' + n)) names.push(n);
	const enc = new TextEncoder(), files = names.map((n) => [enc.encode(n), FS.readFile(DATA + '/' + n)]);
	const size = 20 + files.reduce((t, [n, d]) => t + 6 + n.length + d.length, 0) + 4;
	const out = new Uint8Array(size), view = new DataView(out.buffer);
	out.set(enc.encode('CWSAVE1\n'), 0);
	view.setBigUint64(8, BigInt(Math.floor(Date.now() / 1000)), true);
	view.setUint32(16, files.length, true);
	let at = 20;
	for (const [n, d] of files) {
		view.setUint16(at, n.length, true);
		out.set(n, at + 2);
		view.setUint32(at + 2 + n.length, d.length, true);
		out.set(d, at + 6 + n.length);
		at += 6 + n.length + d.length;
	}
	view.setUint32(at, fnv(out, at), true);
	return out;
}

// A .cwsave's files ({ name, data }), its profile's runs and best layer;
// null for bytes that are none, or damaged
function readSaves(bytes) {
	const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength), dec = new TextDecoder();
	if (bytes.length < 24 || dec.decode(bytes.subarray(0, 8)) !== 'CWSAVE1\n' || view.getUint32(bytes.length - 4, true) !== fnv(bytes, bytes.length - 4)) return null;
	const files = [], count = view.getUint32(16, true);
	let at = 20, runs = 0, best = 0;
	for (let i = 0; i < count; ++i) {
		if (at + 6 > bytes.length - 4) return null;
		const len = view.getUint16(at, true), name = dec.decode(bytes.subarray(at + 2, at + 2 + len)), size = view.getUint32(at + 2 + len, true);
		if (!len || at + 6 + len + size > bytes.length - 4) return null;
		const data = bytes.slice(at + 6 + len, at + 6 + len + size);
		// (the profile's blob: magic "CWP2", size, checksum, then runs and the best layer)
		if (name === 'savedata/profile.sav' && size >= 20 && new DataView(data.buffer).getUint32(0, true) === 0x43575032) {
			runs = new DataView(data.buffer).getInt32(12, true);
			best = new DataView(data.buffer).getInt32(16, true);
		}
		files.push({ name, data });
		at += 6 + len + size;
	}
	return at === bytes.length - 4 ? { files, runs, best } : null;
}

// ... into this browser: its savedata/ kept aside as savedata.old/ first
function unpackSaves(saves) {
	const FS = Module.FS, dir = DATA + '/savedata', old = DATA + '/savedata.old';
	try { for (const n of FS.readdir(old)) if (isFile(old + '/' + n)) FS.unlink(old + '/' + n); FS.rmdir(old); } catch (e) { /* none */ }
	try { FS.rename(dir, old); } catch (e) { /* none yet */ }
	FS.mkdir(dir);
	for (const f of saves.files) {
		const bare = f.name.startsWith('savedata/') ? f.name.slice(9) : null;
		if (bare ? !bare || bare.includes('/') || bare.startsWith('.') : !SETTINGS.includes(f.name)) continue;
		FS.writeFile(DATA + '/' + f.name, f.data);
	}
}

$('export').addEventListener('click', () => {
	if (!ready) { say('One moment: the engine is still loading.'); return; }
	const bytes = packSaves();
	if (!bytes) { say('No saves in this browser yet.'); return; }
	const a = document.createElement('a');
	a.href = URL.createObjectURL(new Blob([bytes], { type: 'application/octet-stream' }));
	a.download = SAVES_FILE;
	document.body.appendChild(a);
	a.click();
	a.remove();
	setTimeout(() => URL.revokeObjectURL(a.href), 1000);
	try { localStorage.setItem('cw-backup-at', String(Date.now())); } catch (e) { /* not kept */ }
	if (!menu.hidden) showKept();
	say('Backup saved: ' + SAVES_FILE + '.');
	track('saves-export');
});

$('saves-file').addEventListener('change', async (e) => {
	const file = e.target.files[0];
	e.target.value = '';
	if (!file) return;
	if (!ready) { say('One moment: the engine is still loading.'); return; }
	const saves = readSaves(new Uint8Array(await file.arrayBuffer()));
	if (!saves) { say(file.name + ' is no saves backup, or it is damaged.'); return; }
	const here = packSaves() ? readSaves(packSaves()) : null;
	const what = saves.runs + ' runs, best Layer ' + saves.best;
	if (here && !confirm('Replace this browser\'s saves (' + here.runs + ' runs, best Layer ' + here.best + ') with the backup\'s (' + what + ')? This browser\'s are kept aside.')) return;
	unpackSaves(saves);
	track('saves-import');
	// (the game reads its saves as it starts: a running one starts again, once they are kept)
	if (started) { Module.FS.syncfs(false, () => location.reload()); return; }
	persist();
	say('Saves brought back: ' + what + '.');
});

$('forget').addEventListener('click', () => {
	if (!confirm('Remove your ROMs and all saves from this browser?')) return;
	track('forget-rom');
	const req = indexedDB.deleteDatabase(DATA);
	req.onsuccess = req.onerror = req.onblocked = () => location.reload();
});

// ---- the screen: 240x160 at the largest whole scale in device pixels that
// the window leaves it (the site's bar away while the game plays), the
// ROMs on it at the same size; a phone's narrow page its whole width ----

function fit() {
	if (touchPlay && started) return;
	const dpr = window.devicePixelRatio || 1, root = document.documentElement;
	if (document.fullscreenElement === app) {
		const k = Math.max(1, Math.floor(Math.min((screen.width * dpr) / 240, (screen.height * dpr) / 160)));
		app.style.setProperty('--fw', (240 * k) / dpr + 'px');
		return;
	}
	const bar = document.querySelector('.pet-bar'), band = app.querySelector('.pet-band'), hints = $('hints');
	const room = window.innerHeight - (started ? 0 : bar.offsetHeight) - band.offsetHeight - (hints.offsetHeight + 10) - 52;
	const width = app.parentElement.clientWidth - 30;
	// (a phone's or tablet's: the page's width, as the game takes the whole
	// screen once it starts; elsewhere the largest whole scale)
	const k = Math.max(1, Math.floor(Math.min((width * dpr) / 240, (room * dpr) / 160)));
	root.style.setProperty('--sw', touchPlay ? '100%' : (240 * k) / dpr + 'px');
	// (one of the game's pixels on the page: the ROMs screen's unit)
	root.style.setProperty('--px', (touchPlay ? Math.max(1.25, width / 240) : k / dpr) + 'px');
}

window.addEventListener('resize', fit);
document.addEventListener('fullscreenchange', fit);
$('fullscreen').addEventListener('click', () => {
	closeMenu();
	if (document.fullscreenElement) document.exitFullscreen();
	else app.requestFullscreen().catch(() => {});
});

// ---- the PET menu: over the game, which waits under it (cw_set_paused) ----

function plates() { return Array.from(menu.querySelectorAll('.menu-body:not([hidden]) .plate')); }

// the cursor: on the plate the keys or the pointer last chose, one only
function cursorTo(plate) {
	for (const li of menu.querySelectorAll('.menu li.cursor')) li.classList.remove('cursor');
	if (plate) plate.parentElement.classList.add('cursor');
}
menu.addEventListener('focusin', (e) => { if (e.target.classList.contains('plate')) cursorTo(e.target); });
menu.addEventListener('mouseover', (e) => {
	const plate = e.target.closest('.plate');
	if (plate && document.activeElement !== plate) plate.focus({ preventScroll: true });
});

function showKept() {
	$('kept-bn6').textContent = kept('bn6') ? 'In' : 'Not yet';
	$('kept-bn5').textContent = kept('bn5') ? 'In' : 'Not in (optional)';
	$('kept-run').textContent = writtenAt(DATA + '/savedata/run.sav') || 'None yet';
	let at = null;
	try { at = localStorage.getItem('cw-backup-at'); } catch (e) { /* not kept */ }
	$('kept-backup').textContent = at ? when(new Date(Number(at))) : 'Never';
}

function openMenu() {
	if (!menu.hidden) return;
	if (started) Module.ccall('cw_set_paused', null, ['number'], [1]);
	if (ready) showKept();
	$('menu-note').textContent = '';
	$('menu-main').hidden = false;
	$('menu-controls').hidden = true;
	menu.hidden = false;
	$('continue').focus();
	track('menu-open');
}

function closeMenu() {
	if (menu.hidden) return;
	menu.hidden = true;
	if (started) { Module.ccall('cw_set_paused', null, ['number'], [0]); canvas.focus(); }
}

$('menu-open').addEventListener('click', openMenu);
$('continue').addEventListener('click', closeMenu);
menu.addEventListener('click', (e) => { if (e.target === menu) closeMenu(); });
$('show-controls').addEventListener('click', () => { $('menu-main').hidden = true; $('menu-controls').hidden = false; $('hide-controls').focus(); });
$('hide-controls').addEventListener('click', () => { $('menu-controls').hidden = true; $('menu-main').hidden = false; $('show-controls').focus(); });
// (the label's own file picker, by the keys too)
$('import').addEventListener('keydown', (e) => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); $('saves-file').click(); } });
// F1 opens and closes it, Esc closes it, the arrows walk its plates
document.addEventListener('keydown', (e) => {
	if (e.key === 'F1') { e.preventDefault(); e.stopPropagation(); if (menu.hidden) openMenu(); else closeMenu(); return; }
	if (menu.hidden) return;
	if (e.key === 'Escape') { e.preventDefault(); closeMenu(); return; }
	if (e.key === 'ArrowDown' || e.key === 'ArrowUp') {
		e.preventDefault();
		const list = plates(), i = list.indexOf(document.activeElement);
		list[(i + (e.key === 'ArrowDown' ? 1 : list.length - 1)) % list.length].focus();
	}
	// (none of it reaches the game, which waits)
	e.stopPropagation();
}, true);

// ---- the keys under the screen: the keyboard's, or a controller's once one is used ----

function showHints() {
	const pad = navigator.getGamepads && Array.from(navigator.getGamepads()).some(Boolean);
	const keys = pad
		? [['A', 'A'], ['B', 'B'], ['L R', 'L R'], ['Start', 'Start'], ['Back', 'Select'], ['F1', 'Menu']]
		: [['WASD', 'Move'], ['J', 'A'], ['K', 'B'], ['Q E', 'L R'], ['Enter', 'Start'], ['Bksp', 'Select'], ['F1', 'Menu']];
	$('hints').innerHTML = keys.map(([k, what]) => '<span>' + k.split(' ').map((c) => '<kbd>' + c + '</kbd>').join('') + what + '</span>').join('');
	fit();
}
window.addEventListener('gamepadconnected', showHints);
window.addEventListener('gamepaddisconnected', showHints);
showHints();

// Smooth motion: the game's 60 frames mixed at each refresh of a 90, 144 or
// 165 Hz screen, which shows them unevenly otherwise (remembered here, and
// switched while the game runs)
function smoothOn() { try { return localStorage.getItem('cw-smooth') === 'on'; } catch (e) { return false; } }
function showSmooth() {
	const on = smoothOn();
	$('smooth-label').textContent = 'Smooth motion: ' + (on ? 'on' : 'off');
	$('smooth').setAttribute('aria-pressed', on ? 'true' : 'false');
}
$('smooth').addEventListener('click', () => {
	const on = !smoothOn();
	try { localStorage.setItem('cw-smooth', on ? 'on' : 'off'); } catch (e) { /* not kept */ }
	showSmooth();
	track('smooth-motion', { state: on ? 'on' : 'off' });
	if (started) Module.ccall('cw_set_smooth', null, ['number'], [on ? 1 : 0]);
});
showSmooth();

// the last writes before the tab goes away
window.addEventListener('pagehide', () => { if (ready) persist(); });
