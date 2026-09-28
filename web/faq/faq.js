// The FAQ as the PET's E-Mail: one letter open at a time, the cursor moved
// with the arrow keys, and a letter once read shown opened (remembered in
// this browser only). Without the script every letter shows in turn.
'use strict';

(() => {
	const rows = [...document.querySelectorAll('.mails .row')];
	const mails = new Map([...document.querySelectorAll('.mail')].map((m) => [m.id, m]));
	const inbox = document.getElementById('inbox');
	const unread = document.getElementById('new');
	const narrow = window.matchMedia('(max-width: 900px)');
	const KEY = 'cw-faq-read';
	let read;
	try { read = new Set(JSON.parse(localStorage.getItem(KEY) || '[]')); } catch (e) { read = new Set(); }

	function paint() {
		let n = 0;
		for (const r of rows) {
			const seen = read.has(r.hash.slice(1));
			r.querySelector('.icon').className = `icon ${seen ? 'i-mail-open' : 'i-mail'}`;
			if (!seen) n++;
		}
		unread.textContent = n;
	}

	function open(id, scroll) {
		if (!mails.has(id)) return false;
		for (const r of rows) {
			const on = r.hash === `#${id}`;
			if (on) r.setAttribute('aria-current', 'true');
			else r.removeAttribute('aria-current');
			r.parentElement.classList.toggle('on', on);
		}
		for (const [k, m] of mails) m.hidden = k !== id;
		read.add(id);
		try { localStorage.setItem(KEY, JSON.stringify([...read])); } catch (e) { /* kept for this visit only */ }
		paint();
		if (scroll && narrow.matches) mails.get(id).scrollIntoView({ block: 'start' });
		return true;
	}

	for (const r of rows) {
		r.addEventListener('click', (e) => {
			e.preventDefault();
			history.replaceState(null, '', r.hash);
			open(r.hash.slice(1), true);
		});
	}
	// up and down move the cursor, as on the PET; Enter opens
	inbox.addEventListener('keydown', (e) => {
		const i = rows.indexOf(document.activeElement);
		if (i < 0 || (e.key !== 'ArrowDown' && e.key !== 'ArrowUp')) return;
		e.preventDefault();
		rows[(i + (e.key === 'ArrowDown' ? 1 : rows.length - 1)) % rows.length].focus();
	});
	// back to the list on a phone, to the letter that was open
	document.querySelector('.reader .back a').addEventListener('click', (e) => {
		e.preventDefault();
		inbox.scrollIntoView({ block: 'start' });
		(rows.find((r) => r.hasAttribute('aria-current')) || rows[0]).focus({ preventScroll: true });
	});
	window.addEventListener('hashchange', () => open(location.hash.slice(1), true));

	if (!open(location.hash.slice(1), false)) open(rows[0].hash.slice(1), false);
})();
