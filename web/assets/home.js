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
		// each chip's picture as its icon
		const ico = document.createElement('span');
		ico.className = 'ico';
		ico.style.backgroundImage = `url(shots/${b.dataset.shot}.png)`;
		b.prepend(ico);
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
		line.textContent = 'Out of stock: the first release is on its way. Until then, build any of them from the source.';
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
		a.closest('li').querySelector('div').append(info);
	}
})();
