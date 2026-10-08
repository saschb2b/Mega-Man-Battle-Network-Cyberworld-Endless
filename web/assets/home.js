// The home page: its clips and the trailer, and the download button named
// for the visitor's system (the downloads have their own page).
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
		if (on) track('trailer-sound');
		if (on) { v.currentTime = 0; v.play(); }
		sound.setAttribute('aria-pressed', String(on));
		sound.querySelector('span').textContent = LANG === 'ja' ? (on ? '音を消す' : '音を出す') : on ? 'Sound off' : 'Sound on';
		sound.querySelector('.icon').className = `icon ${on ? 'i-muted' : 'i-sound'}`;
	});
})();

// ---- the way in: the hero's download button says whose download it leads
// to; the download page leads with it, so the link stays the page's own
// (an address naming the system had scrolled down to its entry in the
// list, past the pick on top) ----

(() => {
	const mine = detectPlatform();
	if (mine === 'browser') return;
	const hero = document.querySelector('.hero .actions .plate[href="download/"]');
	if (hero) hero.lastChild.textContent = LANG === 'ja' ? `${PLATFORMS[mine]}版をダウンロード` : `Download for ${PLATFORMS[mine]}`;
})();
