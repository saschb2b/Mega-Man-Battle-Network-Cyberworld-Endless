// The downloads: every system on one page, in four groups (computer, phone
// and tablet, handheld and console, no install), each with its main file,
// its others and how to install it; above them the best pick for the
// visitor's device, its steps open, as Blender's and Telegram's pages lead
// with one, a guess the list below always answers. A system in the address
// (download/#3ds, the home page's button) is the pick. The links go to the
// newest GitHub release, an alpha's pre-release too (without a release they
// keep pointing at the releases page). Without script, the list alone shows.
'use strict';

// whose device the best pick names
const BEST_FOR = {
	windows: 'For your Windows PC', macos: 'For your Mac', linux: 'For Linux', deck: 'For your Steam Deck', android: 'For your Android device',
	ios: 'For your iPhone or iPad', handheld: 'For your handheld', '3ds': 'For your New 3DS', browser: 'No install, any device',
};

// The best pick: the system's entry copied above the list, its steps open,
// its downloads counted as the pick's (data-umami-event-slot).
function pickBest(os) {
	const best = document.getElementById('best');
	const entry = document.getElementById(os);
	if (!best || !entry) return;
	const copy = entry.cloneNode(true);
	copy.removeAttribute('id');
	copy.classList.add('lit');
	for (const d of copy.querySelectorAll('details')) d.open = true;
	for (const a of copy.querySelectorAll('[data-umami-event]')) a.dataset.umamiEventSlot = 'best';
	best.querySelector('.slot').replaceChildren(copy);
	best.dataset.os = os;
	document.getElementById('best-for').textContent = BEST_FOR[os] || 'Best pick';
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
	const os = document.getElementById(asked)?.classList.contains('entry') ? asked : detectPlatform();
	pickBest(os);
	window.addEventListener('hashchange', () => {
		const h = location.hash.slice(1);
		if (document.getElementById(h)?.classList.contains('entry')) pickBest(h);
	});
})();

// ---- Copy: the ROM's checksum, the AltStore source's address ----

document.addEventListener('click', async (e) => {
	const button = e.target.closest('button.copy');
	if (!button) return;
	try {
		await navigator.clipboard.writeText(button.dataset.copy);
		button.textContent = 'Copied';
	} catch (err) {
		// (no clipboard: the text selected, to copy by hand)
		const code = button.parentElement.querySelector('code');
		if (code) getSelection().selectAllChildren(code);
		button.textContent = 'Selected';
	}
	setTimeout(() => { button.textContent = 'Copy'; }, 2000);
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
		hint.textContent = 'No release yet';
		for (const a of document.querySelectorAll('[data-asset], [data-needs]')) a.setAttribute('aria-disabled', 'true');
		return;
	}
	const date = new Date(release.published_at).toLocaleDateString(undefined, { year: 'numeric', month: 'short', day: 'numeric' });
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
			info.textContent = asset ? `${release.tag_name} · ${(asset.size / 1048576).toFixed(1)} MB` : `Not in ${release.tag_name}`;
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
			info.textContent = `Not in ${release.tag_name}`;
			entry.querySelector('.what').append(info);
		}
	}
	// (a pick this release has no file for, an iPhone's before its app or
	// Android's without its APK: the browser, which needs none)
	const best = document.getElementById('best');
	if (!best.hidden && document.getElementById(best.dataset.os)?.dataset.missing) pickBest('browser');
	// the 3DS title's link as a QR code, for FBI's Remote Install, which
	// downloads a CIA and installs it on the HOME Menu
	const cia = release.assets.find((x) => x.name === 'cyberworld-endless.cia');
	const modules = cia && typeof qr === 'function' ? qr(cia.browser_download_url) : null;
	if (modules) for (const code of document.querySelectorAll('.qr .code')) code.replaceChildren(qrSvg(modules, `QR code of the 3DS title's link, ${release.tag_name}`));
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
