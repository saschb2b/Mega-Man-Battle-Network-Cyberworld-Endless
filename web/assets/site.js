// What every page shares: the site's analytics events and the visitor's
// system. The analytics are Umami's, on this site's own domain only, with
// no cookies and nothing about who a visitor is: an event says what was
// done (a download, a start in the browser), never by whom. A page with no
// analytics (an ad blocker, a copy hosted elsewhere) sends nothing.
'use strict';

// Links and buttons say theirs in their markup (data-umami-event, and
// data-umami-event-* for its details), which Umami's script reads on a
// click; what happens in a page's own code is sent from here.
// (one sent while the page loads waits for Umami's script, which the
// browser runs just before the page is ready)
const queued = [];
function track(name, data) {
	if (window.umami && typeof window.umami.track === 'function') {
		try { window.umami.track(name, data); } catch (e) { /* the analytics are never the page's business */ }
	} else if (document.readyState === 'loading') queued.push([name, data]);
}
document.addEventListener('DOMContentLoaded', () => { for (const [name, data] of queued.splice(0)) track(name, data); });

const PLATFORMS = { windows: 'Windows', macos: 'Mac', linux: 'Linux', deck: 'Steam Deck', android: 'Android', ios: 'iPhone & iPad', handheld: 'PortMaster', '3ds': 'New 3DS', browser: 'Browser' };

// the visitor's system, as far as the browser says: a New 3DS's says it is
// like an iPhone, an iPad's like a Mac (but with touch); a Steam Deck's
// desktop browser says Linux, Steam's own adds "Valve Steam"
function detectPlatform() {
	const ua = navigator.userAgent || '';
	const p = navigator.userAgentData?.platform || navigator.platform || '';
	if (/nintendo 3ds/i.test(ua)) return '3ds';
	if (/android/i.test(ua)) return 'android';
	if (/iphone|ipad|ipod/i.test(ua) || (/mac/i.test(p) && navigator.maxTouchPoints > 1)) return 'ios';
	if (/steamos|steam deck/i.test(ua) || (/valve steam/i.test(ua) && /linux|x11/i.test(ua))) return 'deck';
	if (/win/i.test(p) || /windows/i.test(ua)) return 'windows';
	if (/mac/i.test(p) || /mac os x/i.test(ua)) return 'macos';
	if (/cros/i.test(ua)) return 'browser';
	if (/linux|x11/i.test(p) || /linux|x11/i.test(ua)) return 'linux';
	return 'browser';
}
