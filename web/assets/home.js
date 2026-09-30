// The home page: its clips, the downloads one platform at a time (the
// visitor's first), and their links to the newest GitHub release, an
// alpha's pre-release too (without a release they keep pointing at the
// releases page).
'use strict';

// ---- the clips: they play while on screen, and not at all for visitors
// who asked for less motion (they get the controls instead) ----

(() => {
	const clips = [...document.querySelectorAll('video.clip')];
	if (window.matchMedia('(prefers-reduced-motion: reduce)').matches || !('IntersectionObserver' in window)) {
		for (const v of clips) v.controls = true;
		return;
	}
	const seen = new IntersectionObserver((entries) => {
		for (const e of entries) {
			if (e.isIntersecting) { e.target.preload = 'auto'; e.target.play().catch(() => { e.target.controls = true; }); }
			else e.target.pause();
		}
	}, { threshold: 0.25 });
	for (const v of clips) seen.observe(v);
})();

// ---- the trailer: muted while on screen, its words carry it; Sound on
// starts it over with the music. Phones, and visitors who asked for less
// motion or to save data, get its poster and the player's own controls ----

(() => {
	const v = document.querySelector('video.trailer');
	const sound = document.querySelector('.theater .sound');
	if (!v) return;
	const still = window.matchMedia('(prefers-reduced-motion: reduce)').matches || navigator.connection?.saveData
		|| !window.matchMedia('(min-width: 700px)').matches || !('IntersectionObserver' in window);
	if (still) {
		v.muted = false;
		v.loop = false;
		v.controls = true;
		return;
	}
	sound.hidden = false;
	const seen = new IntersectionObserver(([e]) => {
		if (e.isIntersecting) {
			v.preload = 'auto';
			v.play().catch(() => { sound.hidden = true; v.muted = false; v.controls = true; });
		} else v.pause();
	}, { threshold: 0.3 });
	seen.observe(v);
	sound.addEventListener('click', () => {
		const on = v.muted;
		v.muted = !on;
		if (on) { v.currentTime = 0; v.play(); }
		sound.setAttribute('aria-pressed', String(on));
		sound.querySelector('span').textContent = on ? 'Sound off' : 'Sound on';
		sound.querySelector('.icon').className = `icon ${on ? 'i-muted' : 'i-sound'}`;
	});
})();

// ---- the downloads: one platform at a time, the visitor's first (as
// Godot and OBS pick theirs), its best build on top and the others under
// it, with what to do next; without script every row shows ----

const PLATFORMS = { windows: 'Windows', macos: 'macOS', linux: 'Linux', deck: 'Steam Deck', android: 'Android', handheld: 'Handhelds', '3ds': '3DS', browser: 'Browser', all: 'All' };

// the visitor's system, as far as the browser says (a Steam Deck's desktop
// browser says Linux; an iPhone or iPad has no build, so the browser)
function detectPlatform() {
	const ua = navigator.userAgent || '';
	const p = navigator.userAgentData?.platform || navigator.platform || '';
	if (/android/i.test(ua)) return 'android';
	if (/iphone|ipad|ipod/i.test(ua) || (/mac/i.test(p) && navigator.maxTouchPoints > 1)) return 'browser';
	if (/steamos|steam deck/i.test(ua)) return 'deck';
	if (/win/i.test(p) || /windows/i.test(ua)) return 'windows';
	if (/mac/i.test(p) || /mac os x/i.test(ua)) return 'macos';
	if (/cros/i.test(ua)) return 'browser';
	if (/linux|x11/i.test(p) || /linux|x11/i.test(ua)) return 'linux';
	return 'browser';
}

(() => {
	const list = document.querySelector('.platforms');
	if (!list) return;
	const tabs = [...list.querySelectorAll('[role="tab"]')];
	const rows = [...document.querySelectorAll('#downloads > li')];
	const thens = [...document.querySelectorAll('.shop .then')];
	const picked = document.getElementById('picked');
	const mine = detectPlatform();

	let current = mine;
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
		}
		// (what to do next with the build that leads: the browser's words
		// where a release has no Android app yet)
		const next = (best?.dataset.pick || '').split(' ').find((k) => thens.some((p) => p.dataset.os === k)) || os;
		for (const p of thens) p.classList.toggle('on', p.dataset.os === next);
		picked.hidden = os !== mine || os === 'all';
		picked.textContent = `Picked for this device: ${PLATFORMS[mine]}. Another system? Choose it above.`;
	}

	list.hidden = false;
	// (again once the release says which builds it has)
	document.addEventListener('release', () => show(current, false));
	for (const t of tabs) t.addEventListener('click', () => show(t.dataset.os, false));
	list.addEventListener('keydown', (e) => {
		const i = tabs.indexOf(document.activeElement);
		if (i < 0) return;
		const step = { ArrowRight: 1, ArrowDown: 1, ArrowLeft: -1, ArrowUp: -1 }[e.key];
		const to = e.key === 'Home' ? 0 : e.key === 'End' ? tabs.length - 1 : step ? (i + step + tabs.length) % tabs.length : -1;
		if (to < 0) return;
		e.preventDefault();
		show(tabs[to].dataset.os, true);
	});
	show(mine, false);

	// the hero's button says whose download it leads to
	const hero = document.querySelector('.actions .plate[href="#play"]');
	if (hero && mine !== 'browser') hero.lastChild.textContent = `Download for ${PLATFORMS[mine]}`;
})();

// ---- the files: the newest GitHub release, an alpha's pre-release too
// (without a release the links keep pointing at the releases page) ----

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
	const link = (href, text) => { const a = document.createElement('a'); a.href = href; a.textContent = text; return a; };
	line.append(link(release.html_url, 'All files and checksums'), ' · ', link(`https://github.com/${repo}/releases`, 'Older versions'));
	for (const a of document.querySelectorAll('[data-asset]')) {
		const asset = release.assets.find((x) => x.name === a.dataset.asset);
		const info = document.createElement('span');
		info.className = 'version';
		if (!asset) {
			a.setAttribute('aria-disabled', 'true');
			a.closest('li').dataset.missing = '1';
			info.textContent = `Not in ${release.tag_name}`;
		} else {
			a.href = asset.browser_download_url;
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
