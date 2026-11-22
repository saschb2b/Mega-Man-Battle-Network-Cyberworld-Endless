// The pictures of an itch.io devlog post or page text, uploaded to itch
// (docs/ITCH.md, Each release). itch shows no picture from another site,
// so tools/itch_devlog.py leaves each at its raw GitHub address; this,
// run in one of itch's own editor pages (Claude in Chrome's
// javascript_tool), uploads each picture as itch's own editor does
// (prepare, the file to itch's storage, success) and swaps its address.
// itchPictures(html) returns the HTML with every picture on itch; each
// upload is remembered in localStorage.cwImg, so a picture goes up once.
async function itchPictures(html, progress) {
  const csrf = document.querySelector('input[name=csrf_token]').value;
  const cache = JSON.parse(localStorage.cwImg || '{}');
  const srcs = [...new Set([...html.matchAll(/src="(https:\/\/raw\.githubusercontent\.com\/[^"]+)"/g)].map(m => m[1]))];
  const post = (url, body) => fetch(url, {method: 'POST', body, headers: {'X-Requested-With': 'XMLHttpRequest'}});
  for (const src of srcs) {
    if (!cache[src]) {
      const blob = await (await fetch(src)).blob();
      const bmp = await createImageBitmap(blob);
      const name = src.split('/').pop();
      const prep = await (await post('/dashboard/upload-image', new URLSearchParams({
        filename: name, thumb_size: 'original', action: 'prepare', width: bmp.width, height: bmp.height, csrf_token: csrf,
      }))).json();
      const form = new FormData();
      for (const [k, v] of Object.entries(prep.post_params)) form.append(k, v);
      form.append('file', blob, name);
      await fetch(prep.action, {method: 'POST', body: form});
      const done = await (await post(prep.success_url, new URLSearchParams({csrf_token: csrf}))).json();
      if (!done.success) throw new Error('itch refused ' + src);
      cache[src] = done.upload.thumb_url;
      localStorage.cwImg = JSON.stringify(cache);
    }
    if (progress) progress(src);
    html = html.split(src).join(cache[src]);
  }
  return html;
}
