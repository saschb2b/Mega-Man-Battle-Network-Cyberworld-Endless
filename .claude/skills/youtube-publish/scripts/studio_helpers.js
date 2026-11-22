// YouTube Studio helpers for the pieced upload (the youtube-publish skill).
// Paste the whole file into one javascript_tool call on a Studio page; a
// navigation clears them, so paste again after every one.
//
//   cwPrep()                 a file input of ours (aria-label "cw parts"),
//                            collecting every file given to it in cwParts
//   await cwJoin(name, sha256, size)
//                            the .partNN files joined in name order, checked
//                            against the master's hash and size; keeps cwFile
//   cwGive()                 cwFile handed to Studio's own upload input
//   cwThumb()                the .png among cwParts handed to the thumbnail
//                            input (#file-loader) of the open wizard or page
//   cwState()                what the open wizard holds: title, description
//                            length, playlist, the checked radios, whether
//                            the custom thumbnail took
window.cwPrep = () => {
  window.cwParts = [];
  let inp = document.getElementById('cw-parts');
  if (inp) inp.remove();
  inp = document.createElement('input');
  inp.type = 'file';
  inp.multiple = true;
  inp.id = 'cw-parts';
  inp.setAttribute('aria-label', 'cw parts');
  inp.style.cssText = 'position:fixed;left:0;top:0;width:1px;height:1px;opacity:0.01';
  document.body.appendChild(inp);
  inp.addEventListener('change', () => { for (const f of inp.files) window.cwParts.push(f); });
  return 'prepped';
};

window.cwJoin = async (name, sha, size) => {
  const parts = [...window.cwParts].filter(p => p.name.includes('.part')).sort((a, b) => a.name.localeCompare(b.name));
  const file = new File(parts, name, { type: 'video/mp4' });
  const dig = await crypto.subtle.digest('SHA-256', await file.arrayBuffer());
  const hex = [...new Uint8Array(dig)].map(b => b.toString(16).padStart(2, '0')).join('');
  window.cwFile = file;
  return { n: parts.length, size: file.size, sizeOk: file.size === size, shaOk: hex === sha };
};

window.cwGive = () => {
  const yt = document.querySelector('input[name=Filedata]');
  const dt = new DataTransfer();
  dt.items.add(window.cwFile);
  yt.files = dt.files;
  yt.dispatchEvent(new Event('change', { bubbles: true }));
  return 'given ' + window.cwFile.name;
};

window.cwThumb = () => {
  const f = window.cwParts.find(p => p.name.endsWith('.png'));
  const fl = document.getElementById('file-loader');
  const dt = new DataTransfer();
  dt.items.add(new File([f], f.name, { type: 'image/png' }));
  fl.files = dt.files;
  fl.dispatchEvent(new Event('change', { bubbles: true }));
  return f.name;
};

window.cwState = () => {
  const root = document.querySelector('ytcp-uploads-dialog') || document;
  const up = [...root.querySelectorAll('*')].find(e => e.childElementCount === 0 && e.textContent.trim() === 'Datei hochladen');
  const box = sel => root.querySelector(sel);
  return {
    title: box('#title-textarea #textbox')?.innerText,
    desc: box('#description-textarea #textbox')?.innerText.length,
    playlist: box('ytcp-video-metadata-playlists')?.innerText.trim(),
    radios: [...root.querySelectorAll('tp-yt-paper-radio-button')].filter(r => r.hasAttribute('checked')).map(r => r.innerText.trim().slice(0, 40)),
    thumbTook: !(up && up.offsetParent),
    prompt: /Einmalige Überprüfung/.test(document.body.innerText),
  };
};

cwPrep();
