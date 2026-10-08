// The downloads: every system on one page, in four groups (computer, phone
// and tablet, handheld and console, no install), each with its main file,
// its others and how to install it; above them the best pick for the
// visitor's device, its steps open, as Blender's and Telegram's pages lead
// with one, a guess the list below always answers. A system in the address
// (download/#3ds, the home page's button) is the pick. The links go to the
// newest GitHub release, an alpha's pre-release too (without a release they
// keep pointing at the releases page). Without script, the list alone shows.
'use strict';

// whose device the best pick names, and the page's other words, in its language
const BEST_FOR = LANG === 'ja' ? {
	windows: 'お使いのWindows PCに', macos: 'お使いのMacに', linux: 'Linuxに', deck: 'お使いのSteam Deckに', android: 'お使いのAndroidに',
	ios: 'お使いのiPhone・iPadに', handheld: 'お使いの携帯ゲーム機に', '3ds': 'お使いのNew 3DSに', browser: 'インストール不要、どの端末でも',
} : {
	windows: 'For your Windows PC', macos: 'For your Mac', linux: 'For Linux', deck: 'For your Steam Deck', android: 'For your Android device',
	ios: 'For your iPhone or iPad', handheld: 'For your handheld', '3ds': 'For your New 3DS', browser: 'No install, any device',
};
const WORDS = LANG === 'ja' ? {
	best: 'おすすめ', none: 'リリースはまだありません', notIn: (tag) => `${tag}にはありません`, copy: 'コピー', copied: 'コピーしました', selected: '選択しました',
	qr: (tag) => `3DS版のQRコード（${tag}）`,
} : {
	best: 'Best pick', none: 'No release yet', notIn: (tag) => `Not in ${tag}`, copy: 'Copy', copied: 'Copied', selected: 'Selected',
	qr: (tag) => `QR code of the 3DS title's link, ${tag}`,
};

// A system's entry in the list (data-os: the entries have no ids, so a
// system in the address picks the best one without the page jumping down
// to the list's own; the pick on top shows it already).
const entryFor = (os) => document.querySelector(`#systems .entry[data-os="${CSS.escape(os)}"]`);

// The best pick: the system's entry copied above the list, its steps open,
// its downloads counted as the pick's (data-umami-event-slot).
function pickBest(os) {
	const best = document.getElementById('best');
	const entry = entryFor(os);
	if (!best || !entry) return;
	const copy = entry.cloneNode(true);
	copy.classList.add('lit');
	for (const d of copy.querySelectorAll('details')) d.open = true;
	for (const a of copy.querySelectorAll('[data-umami-event]')) a.dataset.umamiEventSlot = 'best';
	best.querySelector('.slot').replaceChildren(copy);
	best.dataset.os = os;
	document.getElementById('best-for').textContent = BEST_FOR[os] || WORDS.best;
	best.hidden = false;
}

(() => {
	// (every download says which system's entry it was taken from, the
	// list's or the pick's)
	for (const entry of document.querySelectorAll('#systems .entry')) {
		for (const a of entry.querySelectorAll('[data-umami-event]')) {
			a.dataset.umamiEventPlatform = entry.dataset.os;
			a.dataset.umamiEventSlot = 'list';
		}
	}
	const asked = location.hash.slice(1);
	pickBest(asked && entryFor(asked) ? asked : detectPlatform());
	// (a link on the page to another system, the Linux entry's to the
	// Steam Deck's: the pick changes, and comes into view)
	window.addEventListener('hashchange', () => {
		const h = location.hash.slice(1);
		if (!h || !entryFor(h)) return;
		pickBest(h);
		document.getElementById('best').scrollIntoView({ behavior: 'smooth', block: 'start' });
	});
})();

// ---- Copy: the ROM's checksum, the AltStore source's address ----

document.addEventListener('click', async (e) => {
	const button = e.target.closest('button.copy');
	if (!button) return;
	try {
		await navigator.clipboard.writeText(button.dataset.copy);
		button.textContent = WORDS.copied;
	} catch (err) {
		// (no clipboard: the text selected, to copy by hand)
		const code = button.parentElement.querySelector('code');
		if (code) getSelection().selectAllChildren(code);
		button.textContent = WORDS.selected;
	}
	setTimeout(() => { button.textContent = WORDS.copy; }, 2000);
});

// ---- the files: the newest GitHub release, an alpha's pre-release too ----

(async () => {
	const repo = 'saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless';
	const hint = document.getElementById('release-hint');
	let release;
	try {
		// (releases/latest skips pre-releases, and the 0.x alphas are ones:
		// the newest published release of any kind, the list's first)
		const res = await fetch(`https://api.github.com/repos/${repo}/releases?per_page=10`, { headers: { Accept: 'application/vnd.github+json' } });
		if (!res.ok) throw new Error(res.status);
		release = (await res.json()).find((r) => !r.draft);
		if (!release) throw new Error('none');
	} catch (e) {
		hint.textContent = WORDS.none;
		for (const a of document.querySelectorAll('[data-asset], [data-needs]')) a.setAttribute('aria-disabled', 'true');
		return;
	}
	const date = new Date(release.published_at).toLocaleDateString(LANG, { year: 'numeric', month: 'short', day: 'numeric' });
	hint.replaceChildren();
	const notes = document.createElement('a');
	notes.href = release.html_url;
	notes.textContent = release.tag_name;
	notes.dataset.umamiEvent = 'outbound';
	notes.dataset.umamiEventTo = 'release';
	hint.append(notes, ` · ${date}`);
	const find = (names) => names.split(' ').map((n) => release.assets.find((x) => x.name === n)).find(Boolean);
	for (const a of document.querySelectorAll('[data-asset]')) {
		// (its names, newest first: a file renamed for the system it is for
		// keeps its old name in the releases made before)
		const asset = find(a.dataset.asset);
		const entry = a.closest('.entry');
		const main = a.classList.contains('get');
		if (!asset) {
			a.setAttribute('aria-disabled', 'true');
			a.removeAttribute('data-umami-event');
			if (main && entry) entry.dataset.missing = '1';
		} else {
			a.href = asset.browser_download_url;
			a.dataset.umamiEventFile = asset.name;
			a.dataset.umamiEventVersion = release.tag_name;
			// (the SHA-256 GitHub keeps of each file, for whoever checks)
			if (asset.digest) a.title = asset.digest;
		}
		if (main && entry) {
			const info = document.createElement('span');
			info.className = 'version';
			info.textContent = asset ? `${release.tag_name} · ${(asset.size / 1048576).toFixed(1)} MB` : WORDS.notIn(release.tag_name);
			entry.querySelector('.what').append(info);
		}
	}
	// (what needs a file without being its link: the AltStore source's buttons)
	for (const a of document.querySelectorAll('[data-needs]')) {
		if (find(a.dataset.needs)) continue;
		a.setAttribute('aria-disabled', 'true');
		a.removeAttribute('data-umami-event');
		const entry = a.closest('.entry');
		if (a.classList.contains('get') && entry && !entry.querySelector('.version')) {
			entry.dataset.missing = '1';
			const info = document.createElement('span');
			info.className = 'version';
			info.textContent = WORDS.notIn(release.tag_name);
			entry.querySelector('.what').append(info);
		}
	}
	// (a pick this release has no file for, an iPhone's before its app or
	// Android's without its APK: the browser, which needs none)
	const best = document.getElementById('best');
	if (!best.hidden && entryFor(best.dataset.os)?.dataset.missing) pickBest('browser');
	// the 3DS title's link as a QR code, for FBI's Remote Install, which
	// downloads a CIA and installs it on the HOME Menu
	const cia = release.assets.find((x) => x.name === 'cyberworld-endless.cia');
	const modules = cia && typeof qr === 'function' ? qr(cia.browser_download_url) : null;
	if (modules) for (const code of document.querySelectorAll('.qr .code')) code.replaceChildren(qrSvg(modules, WORDS.qr(release.tag_name)));
})();

// ---- after a download: a thank-you, once a visit ----

// A download (a link to a release's file, which the list's are once the
// release is read, or a button adding the AltStore source, the iPhone's way
// to the app) opens a mail from Saschb2b beside it: thanks, a coffee, the
// repository and its issues. The download goes on as without it: the dialog
// opens after the click has done its work, and its own links open a new
// tab, never over the download's navigation. The 3DS's QR code is scanned
// by FBI, not clicked here: it has no thanks. Shown once a visit, as the
// tab's sessionStorage keeps it (where that is blocked, once a page).
(() => {
	const dialog = document.getElementById('thanks');
	if (!dialog || typeof dialog.showModal !== 'function') return;
	const DOWNLOAD = 'a[href*="/releases/download/"], a[href*="/releases/latest/download/"], a[data-needs]:not([aria-disabled="true"])';
	const KEY = 'cw-thanks';
	let shown = false;
	let from = null;
	let pressedOutside = false;
	try { shown = sessionStorage.getItem(KEY) === '1'; } catch (e) { /* not kept: once a page */ }

	document.addEventListener('click', (e) => {
		const link = e.target.closest(DOWNLOAD);
		if (!link || shown) return;
		shown = true;
		try { sessionStorage.setItem(KEY, '1'); } catch (err) { /* not kept: once a page */ }
		from = link;
		setTimeout(() => {
			dialog.showModal();
			track('thanks-open', { platform: link.dataset.umamiEventPlatform || 'website' });
		});
	});
	dialog.querySelector('.close').addEventListener('click', () => dialog.close());
	// (Tab goes round the window, from its last link to Close and back)
	dialog.addEventListener('keydown', (e) => {
		if (e.key !== 'Tab') return;
		const stops = dialog.querySelectorAll('a[href], button');
		const first = stops[0], last = stops[stops.length - 1];
		if (document.activeElement !== (e.shiftKey ? first : last)) return;
		e.preventDefault();
		(e.shiftKey ? last : first).focus();
	});
	// (a click on the dialog itself is outside its window; a press that began
	// in the window, text selected and let go outside, is not one)
	dialog.addEventListener('pointerdown', (e) => { pressedOutside = e.target === dialog; });
	dialog.addEventListener('click', (e) => {
		if (e.target === dialog && pressedOutside) dialog.close();
		pressedOutside = false;
	});
	// (back to the link the download came from, as the keyboard left it)
	dialog.addEventListener('close', () => {
		if (from && from.isConnected) from.focus({ preventScroll: true });
		from = null;
	});
})();

// A QR code's modules as an SVG, dark on white with its quiet zone.
function qrSvg(modules, label) {
	const ns = 'http://www.w3.org/2000/svg', n = modules.length + 8;
	const svg = document.createElementNS(ns, 'svg');
	svg.setAttribute('viewBox', `0 0 ${n} ${n}`);
	svg.setAttribute('shape-rendering', 'crispEdges');
	svg.setAttribute('role', 'img');
	svg.setAttribute('aria-label', label);
	const back = document.createElementNS(ns, 'rect');
	back.setAttribute('width', n);
	back.setAttribute('height', n);
	back.setAttribute('fill', '#fff');
	let d = '';
	modules.forEach((row, y) => row.forEach((dark, x) => { if (dark) d += `M${x + 4} ${y + 4}h1v1h-1z`; }));
	const path = document.createElementNS(ns, 'path');
	path.setAttribute('d', d);
	path.setAttribute('fill', '#000');
	svg.append(back, path);
	return svg;
}
