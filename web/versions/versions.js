// The versions page: one release open at a time in the reader, the newest
// unless the address names another (#v0-12-0, or #v0.12.0 as the tag
// writes it); the list's cursor on it, the arrow keys moving it as on the
// PET. Without the script every release shows in turn, newest first.
'use strict';

(() => {
	const rows = [...document.querySelectorAll('.releases .row')];
	const releases = new Map([...document.querySelectorAll('.release')].map((r) => [r.id, r]));
	const list = document.getElementById('versions');
	if (!rows.length) return;

	// (an address's version as the page's id: v0.12.0, 0.12.0 or v0-12-0)
	function id(hash) {
		const h = decodeURIComponent(hash.replace(/^#/, ''));
		const m = /^v?(\d+)[.-](\d+)[.-](\d+)$/.exec(h);
		return m ? `v${m[1]}-${m[2]}-${m[3]}` : h;
	}

	// (from: 'list' for the list's row, 'walk' for a release's next or
	// earlier, 'link' for an address that named it; the newest opened by
	// itself is not counted)
	function open(name, scroll, from) {
		const release = releases.get(name);
		if (!release) return false;
		if (from) track('version-open', { version: name.slice(1).replace(/-/g, '.'), from });
		for (const r of rows) {
			const on = r.hash === `#${name}`;
			if (on) r.setAttribute('aria-current', 'true');
			else r.removeAttribute('aria-current');
			r.parentElement.classList.toggle('on', on);
		}
		for (const [k, r] of releases) {
			r.hidden = k !== name;
			if (k !== name) for (const v of r.querySelectorAll('video')) v.pause();
		}
		if (scroll) release.scrollIntoView({ block: 'start' });
		return true;
	}

	function go(e, hash, from) {
		e.preventDefault();
		history.replaceState(null, '', hash);
		open(id(hash), true, from);
	}
	for (const r of rows) r.addEventListener('click', (e) => go(e, r.hash, 'list'));
	for (const a of document.querySelectorAll('.walk a')) a.addEventListener('click', (e) => go(e, a.hash, 'walk'));

	// up and down move the cursor, as on the PET; Enter opens
	list.addEventListener('keydown', (e) => {
		const i = rows.indexOf(document.activeElement);
		if (i < 0 || (e.key !== 'ArrowDown' && e.key !== 'ArrowUp')) return;
		e.preventDefault();
		rows[(i + (e.key === 'ArrowDown' ? 1 : rows.length - 1)) % rows.length].focus();
	});
	// (on a phone the list is under the reader: its pills lead there, to the open row)
	for (const a of document.querySelectorAll('.reader .jump a, .reader .back a')) {
		a.addEventListener('click', (e) => {
			e.preventDefault();
			list.scrollIntoView({ block: 'start' });
			(rows.find((r) => r.hasAttribute('aria-current')) || rows[0]).focus({ preventScroll: true });
		});
	}
	window.addEventListener('hashchange', () => named(true));

	// a trailer started counts, once a page
	for (const v of document.querySelectorAll('video.trailer')) {
		v.addEventListener('play', () => track('trailer-play', { version: v.dataset.version, from: 'versions' }), { once: true });
	}

	// the notes' clips play while on screen, and not at all for visitors who
	// asked for less motion (they get the controls instead), as on the home page
	const clips = [...document.querySelectorAll('.notes video.clip')];
	if (window.matchMedia('(prefers-reduced-motion: reduce)').matches || !('IntersectionObserver' in window)) {
		for (const v of clips) v.controls = true;
	} else {
		const seen = new IntersectionObserver((entries) => {
			for (const e of entries) {
				if (e.isIntersecting) { e.target.preload = 'auto'; e.target.play().catch(() => { e.target.controls = true; }); }
				else e.target.pause();
			}
		}, { threshold: 0.25 });
		for (const v of clips) seen.observe(v);
	}

	// (the release an address names, its address then written as the page's id)
	function named(scroll) {
		if (!location.hash || !open(id(location.hash), scroll, 'link')) return false;
		if (location.hash !== `#${id(location.hash)}`) history.replaceState(null, '', `#${id(location.hash)}`);
		return true;
	}
	if (!named(true)) open(rows[0].hash.slice(1), false);
})();
