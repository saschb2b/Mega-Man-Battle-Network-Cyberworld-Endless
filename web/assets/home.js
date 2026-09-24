// The home page: the ChipFolder's picker, and the Net Dealer's links to the
// latest GitHub release (without a release they keep pointing at the
// releases page).
'use strict';

// ---- the ChipFolder ----

(() => {
	const chips = [...document.querySelectorAll('#chips button')];
	const shot = document.getElementById('chip-shot'), name = document.getElementById('chip-name');
	const code = document.getElementById('chip-code'), text = document.getElementById('chip-text');
	function pick(b, focus) {
		for (const c of chips) c.setAttribute('aria-pressed', c === b ? 'true' : 'false');
		shot.src = `shots/${b.dataset.shot}.png`;
		shot.alt = b.dataset.text;
		name.textContent = b.textContent.trim();
		code.textContent = b.dataset.code;
		text.textContent = b.dataset.text;
		if (focus) b.focus();
	}
	chips.forEach((b, i) => {
		b.addEventListener('click', () => pick(b));
		b.addEventListener('keydown', (e) => {
			const d = e.key === 'ArrowDown' ? 1 : e.key === 'ArrowUp' ? -1 : 0;
			if (!d) return;
			e.preventDefault();
			pick(chips[(i + d + chips.length) % chips.length], true);
		});
	});
	if (chips.length) pick(chips[0]);
})();

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

// ---- the Net Dealer ----

(async () => {
	const repo = 'saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless';
	const line = document.getElementById('release-line');
	let release;
	try {
		const res = await fetch(`https://api.github.com/repos/${repo}/releases/latest`, { headers: { Accept: 'application/vnd.github+json' } });
		if (!res.ok) throw new Error(res.status);
		release = await res.json();
	} catch (e) {
		line.textContent = 'Out of stock: no release yet. Build them from the source.';
		for (const a of document.querySelectorAll('[data-asset]')) a.setAttribute('aria-disabled', 'true');
		return;
	}
	const date = new Date(release.published_at).toLocaleDateString(undefined, { year: 'numeric', month: 'long', day: 'numeric' });
	line.textContent = `${release.name || release.tag_name} · released ${date}`;
	for (const a of document.querySelectorAll('[data-asset]')) {
		const asset = release.assets.find((x) => x.name === a.dataset.asset);
		if (!asset) { a.setAttribute('aria-disabled', 'true'); continue; }
		a.href = asset.browser_download_url;
		const info = document.createElement('span');
		info.className = 'version';
		info.textContent = `${release.tag_name} · ${(asset.size / 1048576).toFixed(1)} MB`;
		a.closest('li').querySelector('.what').append(info);
	}
})();
