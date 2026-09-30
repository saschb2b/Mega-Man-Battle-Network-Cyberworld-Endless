// The downloads: one system at a time, the visitor's first (as Godot and OBS
// pick theirs), its best build on top and the others under it, with what to
// do next; a system in the address (download/#3ds) opens on it. Their
// links go to the newest GitHub release, an alpha's pre-release too
// (without a release they keep pointing at the releases page). Without
// script every row shows.
'use strict';

(() => {
	const list = document.querySelector('.platforms');
	if (!list) return;
	const tabs = [...list.querySelectorAll('[role="tab"]')];
	const rows = [...document.querySelectorAll('#downloads > li')];
	const thens = [...document.querySelectorAll('.shop .then')];
	const picked = document.getElementById('picked');
	const mine = detectPlatform();
	const asked = () => { const h = location.hash.slice(1); return PLATFORMS[h] ? h : null; };

	let current = asked() || mine;
	function show(os, focus) {
		current = os;
		// (the best pick, else the first build this release has)
		const here = rows.filter((r) => os === 'all' || r.dataset.os.split(' ').includes(os));
		const best = os === 'all' ? null : here.find((r) => (r.dataset.pick || '').split(' ').includes(os) && !r.dataset.missing)
			|| here.find((r) => !r.dataset.missing);
		for (const t of tabs) {
			const on = t.dataset.os === os;
			t.setAttribute('aria-selected', String(on));
			t.tabIndex = on ? 0 : -1;
			t.classList.toggle('on', on);
			if (on && focus) t.focus();
		}
		for (const r of rows) {
			const pick = r === best;
			r.hidden = !here.includes(r);
			r.classList.toggle('pick', pick);
			let tag = r.querySelector('.tag');
			if (pick && !tag) {
				tag = document.createElement('span');
				tag.className = 'tag';
				tag.textContent = 'Best pick';
				r.querySelector('.what b').append(tag);
			} else if (!pick && tag) tag.remove();
			// (a download's event names the system it was chosen for: under
			// All, the row's own first)
			const get = r.querySelector('[data-umami-event="download"]');
			if (get) get.dataset.umamiEventPlatform = os === 'all' ? r.dataset.os.split(' ')[0] : os;
		}
		// (what to do next with the build that leads: the browser's words
		// where a release has no Android app yet)
		const next = (best?.dataset.pick || '').split(' ').find((k) => thens.some((p) => p.dataset.os === k)) || os;
		for (const p of thens) p.classList.toggle('on', p.dataset.os === next);
		picked.hidden = os !== mine || os === 'all';
		picked.textContent = `Picked for this device: ${PLATFORMS[mine]}. Another system? Choose it above.`;
	}

	// a system chosen by hand: in the address, to share, and counted
	function choose(os, focus) {
		if (os === current) return;
		show(os, focus);
		history.replaceState(null, '', `#${os}`);
		track('choose-platform', { platform: os, detected: mine });
	}

	list.hidden = false;
	// (again once the release says which builds it has)
	document.addEventListener('release', () => show(current, false));
	for (const t of tabs) t.addEventListener('click', () => choose(t.dataset.os, false));
	list.addEventListener('keydown', (e) => {
		const i = tabs.indexOf(document.activeElement);
		if (i < 0) return;
		const step = { ArrowRight: 1, ArrowDown: 1, ArrowLeft: -1, ArrowUp: -1 }[e.key];
		const to = e.key === 'Home' ? 0 : e.key === 'End' ? tabs.length - 1 : step ? (i + step + tabs.length) % tabs.length : -1;
		if (to < 0) return;
		e.preventDefault();
		choose(tabs[to].dataset.os, true);
	});
	window.addEventListener('hashchange', () => { const os = asked(); if (os && os !== current) show(os, false); });
	show(current, false);
})();

// ---- the files: the newest GitHub release, an alpha's pre-release too ----

(async () => {
	const repo = 'saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless';
	const line = document.getElementById('release-line');
	let release;
	try {
		// (releases/latest skips pre-releases, and the 0.x alphas are ones:
		// the newest published release of any kind, the list's first)
		const res = await fetch(`https://api.github.com/repos/${repo}/releases?per_page=10`, { headers: { Accept: 'application/vnd.github+json' } });
		if (!res.ok) throw new Error(res.status);
		release = (await res.json()).find((r) => !r.draft);
		if (!release) throw new Error('none');
	} catch (e) {
		line.textContent = 'No release yet: build from the source.';
		for (const a of document.querySelectorAll('[data-asset]')) a.setAttribute('aria-disabled', 'true');
		return;
	}
	const date = new Date(release.published_at).toLocaleDateString(undefined, { year: 'numeric', month: 'long', day: 'numeric' });
	line.textContent = `${release.name || release.tag_name} · ${date} · `;
	const link = (href, text, to) => {
		const a = document.createElement('a');
		a.href = href;
		a.textContent = text;
		a.dataset.umamiEvent = 'outbound';
		a.dataset.umamiEventTo = to;
		return a;
	};
	line.append(link(release.html_url, 'All files and checksums', 'release'), ' · ', link(`https://github.com/${repo}/releases`, 'Older versions', 'releases'));
	for (const a of document.querySelectorAll('[data-asset]')) {
		// (its names, newest first: a file renamed for the system it is for
		// keeps its old name in the releases made before)
		const names = a.dataset.asset.split(' ');
		const asset = names.map((n) => release.assets.find((x) => x.name === n)).find(Boolean);
		const info = document.createElement('span');
		info.className = 'version';
		if (!asset) {
			a.setAttribute('aria-disabled', 'true');
			a.removeAttribute('data-umami-event');
			a.closest('li').dataset.missing = '1';
			info.textContent = `Not in ${release.tag_name}`;
		} else {
			a.href = asset.browser_download_url;
			a.dataset.umamiEventFile = names[0];
			a.dataset.umamiEventVersion = release.tag_name;
			info.textContent = `${release.tag_name} · ${(asset.size / 1048576).toFixed(1)} MB`;
		}
		a.closest('li').querySelector('.what').append(info);
	}
	// the 3DS title's link as a QR code, for FBI's Remote Install, which
	// downloads a CIA and installs it on the HOME Menu
	const cia = release.assets.find((x) => x.name === 'cyberworld-endless.cia');
	const code = document.querySelector('#qr-3ds .code');
	const modules = cia && code && typeof qr === 'function' ? qr(cia.browser_download_url) : null;
	if (modules) code.replaceChildren(qrSvg(modules, `QR code of the 3DS title's link, ${release.tag_name}`));
	document.dispatchEvent(new Event('release'));
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
