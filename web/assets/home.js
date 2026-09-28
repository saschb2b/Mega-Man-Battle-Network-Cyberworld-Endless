// The home page: its clips, and the download links to the newest GitHub
// release, an alpha's pre-release too (without a release they keep pointing
// at the releases page).
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

// ---- the downloads ----

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
	line.textContent = `${release.name || release.tag_name} · ${date}`;
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
