// The discord-sweep skill's helpers for a Discord tab
// (.claude/skills/discord-sweep/SKILL.md). Paste this file whole into one
// javascript_tool call after each page load that lost it (a navigation
// inside Discord keeps it), then call window.cw's functions.
//
// Two things shaped it. The tool cuts a long answer short, so cw.show
// hands a value back a piece at a time. And it blocks an answer that
// looks like a query string or a cookie ("a=b=c", an attachment's
// address), so nothing here returns an address or joins with "=".
window.cw = (() => {
  const wait = ms => new Promise(r => setTimeout(r, ms));
  const guild = () => location.pathname.split('/')[2];
  const box = () => [...document.querySelectorAll('div[role=textbox][contenteditable=true]')].pop();
  const flat = s => s.replace(/\s+/g, ' ').trim();
  const key = (el, name, code) => {
    const o = { key: name, code: name, keyCode: code, which: code, bubbles: true, cancelable: true };
    el.dispatchEvent(new KeyboardEvent('keydown', o));
    el.dispatchEvent(new KeyboardEvent('keyup', o));
  };
  const closeMenus = async () => {
    for (let i = 0; i < 2; i++) {
      key(document.querySelector('[role=menu]') || document.body, 'Escape', 27);
      await wait(500);
    }
  };

  // A piece of a value, as text: cw.show(x), then cw.show(x, 1200), ...
  function show(value, from = 0, size = 1200) {
    const s = typeof value === 'string' ? value : JSON.stringify(value, null, 1);
    return `${from}-${Math.min(from + size, s.length)} of ${s.length}\n` + s.slice(from, from + size);
  }

  // The server's channels the sidebar shows: [{id, name}].
  function channels() {
    return [...document.querySelectorAll(`a[href^="/channels/${guild()}/"]`)].map(a => ({
      id: a.getAttribute('href').split('/').pop(),
      name: flat(a.getAttribute('aria-label') || a.textContent).slice(0, 60),
    }));
  }

  // The messages on screen, oldest first: [{id, author, at, text, pictures}].
  // A message that follows its author's last one has no name of its own
  // and takes the one before it. Scroll up and call again for older ones.
  function messages() {
    let author = '';
    return [...document.querySelectorAll('li[id^="chat-messages-"]')].map(li => {
      const u = li.querySelector('[id^="message-username-"]');
      if (u) author = u.innerText.split('\n')[0].trim();
      const t = li.querySelector('time');
      const c = li.querySelector('[id^="message-content-"]');
      return {
        id: li.id.split('-').pop(),
        author,
        at: t ? t.getAttribute('datetime') : '',
        text: c ? c.innerText : '',
        pictures: li.querySelectorAll('a[class*=originalLink]').length,
      };
    });
  }

  // A forum's posts, on the forum's own page: [{id, tags_and_text}]. The
  // list is drawn as it scrolls, so this scrolls it through.
  async function threads() {
    const seen = {};
    const scroller = [...document.querySelectorAll('main div')]
      .filter(d => d.scrollHeight > d.clientHeight + 50 && /scroll|auto/.test(getComputedStyle(d).overflowY))
      .sort((a, b) => b.scrollHeight - a.scrollHeight)[0];
    for (let i = 0; i < 40; i++) {
      document.querySelectorAll('[data-item-id]').forEach(e => {
        seen[e.getAttribute('data-item-id')] = flat(e.innerText.replace(/\n+/g, ' | ')).slice(0, 300);
      });
      if (!scroller) break;
      const before = scroller.scrollTop;
      scroller.scrollTop += scroller.clientHeight * 0.8;
      await wait(400);
      if (scroller.scrollTop === before) break;
    }
    return Object.entries(seen).map(([id, tags_and_text]) => ({ id, tags_and_text }));
  }

  // Every picture attached to the messages on screen, sent to
  // scripts/receive.py as PREFIX-1.png, PREFIX-2.jpg, ... in the order of
  // the page: [{name, bytes, message}] (or an error in place of bytes).
  async function savePictures(prefix, port = 8765) {
    const out = [];
    const links = [...document.querySelectorAll('li[id^="chat-messages-"] a[class*=originalLink]')];
    for (let i = 0; i < links.length; i++) {
      const message = links[i].closest('li[id^="chat-messages-"]').id.split('-').pop();
      try {
        const blob = await (await fetch(links[i].href)).blob();
        const ext = (blob.type.split('/')[1] || 'bin').replace('jpeg', 'jpg').replace(/[^a-z0-9]/g, '');
        const name = `${prefix}-${i + 1}.${ext}`;
        const r = await fetch(`http://127.0.0.1:${port}/${name}`, { method: 'POST', body: blob });
        out.push({ name, bytes: blob.size, message, saved: r.ok });
      } catch (e) {
        out.push({ name: `${prefix}-${i + 1}`, error: String(e).slice(0, 120), message });
      }
    }
    return out;
  }

  // Sends TEXT in the thread or channel open now. THREAD is a piece of
  // its title: the message box must name it, or nothing is typed. The
  // box must hold TEXT and nothing else before it is sent. Returns the
  // last message after the send, to check against.
  async function post(thread, text) {
    const el = box();
    if (!el) throw new Error('no message box on this page');
    const label = el.getAttribute('aria-label') || '';
    if (!label.includes(thread)) throw new Error('the box is for: ' + label);
    if (flat(el.innerText)) throw new Error('the box already holds: ' + flat(el.innerText).slice(0, 80));
    el.focus();
    const data = new DataTransfer();
    data.setData('text/plain', text);
    el.dispatchEvent(new ClipboardEvent('paste', { clipboardData: data, bubbles: true, cancelable: true }));
    await wait(1000);
    if (flat(el.innerText) !== flat(text)) throw new Error('the box holds: ' + flat(el.innerText).slice(0, 120));
    key(el, 'Enter', 13);
    await wait(3000);
    const last = messages().pop();
    return { box: label, left_in_box: flat(el.innerText).length, last_id: last.id, last_author: last.author, last_text: last.text.slice(0, 160) };
  }

  // Sets a forum post's tag (TAG by its name, "On GitHub"), on the
  // forum's own page where the post's card is listed. Leaves a tag that
  // is set alone. Returns the card's text after, tags first.
  async function tag(threadId, name) {
    await closeMenus();
    const card = document.querySelector(`[data-item-id="${threadId}"]`);
    if (!card) throw new Error('no card for that post on this page (scroll to it)');
    card.scrollIntoView({ block: 'center' });
    await wait(400);
    const r = card.getBoundingClientRect();
    card.dispatchEvent(new MouseEvent('contextmenu', { bubbles: true, cancelable: true, clientX: r.left + r.width / 2, clientY: r.top + 20, button: 2 }));
    await wait(900);
    const entry = document.getElementById('thread-context-edit-tags');
    if (!entry) { await closeMenus(); throw new Error('the menu has no entry for tags (no right to set them?)'); }
    const er = entry.getBoundingClientRect();
    for (const type of ['pointerover', 'mouseover', 'mouseenter', 'pointermove', 'mousemove']) {
      entry.dispatchEvent(new MouseEvent(type, { bubbles: true, clientX: er.left + 10, clientY: er.top + 8 }));
    }
    entry.focus();
    key(entry, 'ArrowRight', 39);
    await wait(1000);
    const item = [...document.querySelectorAll('[id^="thread-context-edit-tags--"]')].find(e => e.innerText.trim() === name);
    if (!item) { await closeMenus(); throw new Error('no tag of that name'); }
    const was = item.getAttribute('aria-checked');
    if (was === 'false') { item.click(); await wait(1500); }
    await closeMenus();
    const after = document.querySelector(`[data-item-id="${threadId}"]`);
    return { was_set: was === 'true', card: flat(after.innerText.replace(/\n+/g, ' | ')).slice(0, 120), menus_open: document.querySelectorAll('[role=menu]').length };
  }

  return { show, channels, messages, threads, savePictures, post, tag, wait };
})();
'cw ready';
