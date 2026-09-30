// A QR code (ISO/IEC 18004) for one short text, such as a download link:
// byte mode, versions 1 to 10, error correction L (M where it fits in the
// same size), the mask the standard's penalty rules prefer. qr(text)
// gives the modules as rows of booleans (true dark), without the quiet
// zone; null where the text does not fit.
// eslint-disable-next-line no-unused-vars
const qr = (() => {
	// GF(256) over x^8 + x^4 + x^3 + x^2 + 1, as the standard's Reed-Solomon code
	const EXP = new Uint8Array(512);
	const LOG = new Uint8Array(256);
	for (let i = 0, x = 1; i < 255; ++i) {
		EXP[i] = x;
		LOG[x] = i;
		x <<= 1;
		if (x & 0x100) x ^= 0x11d;
	}
	for (let i = 255; i < 512; ++i) EXP[i] = EXP[i - 255];
	const mul = (a, b) => (a && b ? EXP[LOG[a] + LOG[b]] : 0);

	// the generator polynomial of `degree` (its leading 1 left out)
	function divisor(degree) {
		const d = new Uint8Array(degree);
		d[degree - 1] = 1;
		for (let i = 0, root = 1; i < degree; ++i, root = mul(root, 2))
			for (let j = 0; j < degree; ++j) d[j] = mul(d[j], root) ^ (j + 1 < degree ? d[j + 1] : 0);
		return d;
	}

	function remainder(data, d) {
		const r = new Uint8Array(d.length);
		for (const b of data) {
			const f = b ^ r[0];
			r.copyWithin(0, 1);
			r[r.length - 1] = 0;
			for (let i = 0; i < d.length; ++i) r[i] ^= mul(d[i], f);
		}
		return r;
	}

	// per version 1-10: error correction codewords per block and blocks, for L and M
	const ECC = { L: [7, 10, 15, 20, 26, 18, 20, 24, 30, 18], M: [10, 16, 26, 18, 24, 16, 18, 22, 22, 26] };
	const BLOCKS = { L: [1, 1, 1, 1, 1, 2, 2, 2, 2, 4], M: [1, 1, 1, 2, 2, 4, 4, 4, 5, 5] };
	const FORMAT = { L: 1, M: 0 };

	// the modules a version leaves for data and error correction
	function rawModules(v) {
		let n = (16 * v + 128) * v + 64;
		if (v >= 2) {
			const align = Math.floor(v / 7) + 2;
			n -= (25 * align - 10) * align - 55;
			if (v >= 7) n -= 36;
		}
		return n;
	}
	const dataCodewords = (v, ecl) => (rawModules(v) >> 3) - ECC[ecl][v - 1] * BLOCKS[ecl][v - 1];

	function alignPositions(v) {
		if (v === 1) return [];
		const n = Math.floor(v / 7) + 2, size = v * 4 + 17;
		const step = Math.floor((v * 8 + n * 3 + 5) / (n * 4 - 4)) * 2;
		const at = [6];
		for (let p = size - 7; at.length < n; p -= step) at.splice(1, 0, p);
		return at;
	}

	return (text) => {
		const bytes = new TextEncoder().encode(text);
		// the smallest version it fits, at L; then M where that version holds it
		let v = 0, ecl = 'L';
		for (let t = 1; t <= 10 && !v; ++t) if (4 + (t < 10 ? 8 : 16) + bytes.length * 8 <= dataCodewords(t, 'L') * 8) v = t;
		if (!v) return null;
		if (4 + (v < 10 ? 8 : 16) + bytes.length * 8 <= dataCodewords(v, 'M') * 8) ecl = 'M';

		// the bits: byte mode, the count, the bytes, a terminator, then padding
		const bits = [];
		const put = (value, n) => { for (let i = n - 1; i >= 0; --i) bits.push((value >>> i) & 1); };
		put(4, 4);
		put(bytes.length, v < 10 ? 8 : 16);
		for (const b of bytes) put(b, 8);
		const capacity = dataCodewords(v, ecl) * 8;
		put(0, Math.min(4, capacity - bits.length));
		put(0, (8 - (bits.length % 8)) % 8);
		for (let pad = 0xec; bits.length < capacity; pad ^= 0xec ^ 0x11) put(pad, 8);
		const data = [];
		for (let i = 0; i < bits.length; i += 8) data.push(bits.slice(i, i + 8).reduce((a, b) => (a << 1) | b, 0));

		// in blocks, each with its error correction, then interleaved
		const nblocks = BLOCKS[ecl][v - 1], ecLen = ECC[ecl][v - 1], raw = rawModules(v) >> 3;
		const nshort = nblocks - (raw % nblocks), shortLen = Math.floor(raw / nblocks);
		const d = divisor(ecLen), blocks = [];
		for (let i = 0, k = 0; i < nblocks; ++i) {
			const block = data.slice(k, k + shortLen - ecLen + (i < nshort ? 0 : 1));
			k += block.length;
			const ecc = remainder(block, d);
			if (i < nshort) block.push(0);
			blocks.push(block.concat(Array.from(ecc)));
		}
		const words = [];
		for (let i = 0; i < blocks[0].length; ++i)
			for (let j = 0; j < blocks.length; ++j) if (i !== shortLen - ecLen || j >= nshort) words.push(blocks[j][i]);

		// the fixed patterns
		const size = v * 4 + 17;
		const dark = Array.from({ length: size }, () => new Array(size).fill(false));
		const fixed = Array.from({ length: size }, () => new Array(size).fill(false));
		const set = (x, y, on) => { dark[y][x] = on; fixed[y][x] = true; };
		for (let i = 0; i < size; ++i) { set(6, i, i % 2 === 0); set(i, 6, i % 2 === 0); }
		for (const [cx, cy] of [[3, 3], [size - 4, 3], [3, size - 4]])
			for (let dy = -4; dy <= 4; ++dy)
				for (let dx = -4; dx <= 4; ++dx) {
					const x = cx + dx, y = cy + dy, r = Math.max(Math.abs(dx), Math.abs(dy));
					if (x >= 0 && x < size && y >= 0 && y < size) set(x, y, r !== 2 && r !== 4);
				}
		const at = alignPositions(v);
		for (let i = 0; i < at.length; ++i)
			for (let j = 0; j < at.length; ++j) {
				if ((i === 0 && j === 0) || (i === 0 && j === at.length - 1) || (i === at.length - 1 && j === 0)) continue;
				for (let dy = -2; dy <= 2; ++dy)
					for (let dx = -2; dx <= 2; ++dx) set(at[i] + dx, at[j] + dy, Math.max(Math.abs(dx), Math.abs(dy)) !== 1);
			}
		const format = (mask) => {
			const value = (FORMAT[ecl] << 3) | mask;
			let r = value;
			for (let i = 0; i < 10; ++i) r = (r << 1) ^ ((r >>> 9) * 0x537);
			const f = ((value << 10) | r) ^ 0x5412, bit = (i) => ((f >>> i) & 1) === 1;
			for (let i = 0; i <= 5; ++i) set(8, i, bit(i));
			set(8, 7, bit(6));
			set(8, 8, bit(7));
			set(7, 8, bit(8));
			for (let i = 9; i < 15; ++i) set(14 - i, 8, bit(i));
			for (let i = 0; i < 8; ++i) set(size - 1 - i, 8, bit(i));
			for (let i = 8; i < 15; ++i) set(8, size - 15 + i, bit(i));
			set(8, size - 8, true);
		};
		format(0);
		if (v >= 7) {
			let r = v;
			for (let i = 0; i < 12; ++i) r = (r << 1) ^ ((r >>> 11) * 0x1f25);
			const f = (v << 12) | r;
			for (let i = 0; i < 18; ++i) {
				const on = ((f >>> i) & 1) === 1, a = size - 11 + (i % 3), b = Math.floor(i / 3);
				set(a, b, on);
				set(b, a, on);
			}
		}

		// the codewords, up and down in pairs of columns from the right
		for (let right = size - 1, i = 0; right >= 1; right -= 2) {
			if (right === 6) right = 5;
			for (let vert = 0; vert < size; ++vert)
				for (let j = 0; j < 2; ++j) {
					const x = right - j, y = ((right + 1) & 2) === 0 ? size - 1 - vert : vert;
					if (!fixed[y][x] && i < words.length * 8) {
						dark[y][x] = ((words[i >>> 3] >>> (7 - (i & 7))) & 1) === 1;
						++i;
					}
				}
		}

		// the mask with the lowest penalty
		const MASKS = [
			(x, y) => (x + y) % 2 === 0, (x, y) => y % 2 === 0, (x) => x % 3 === 0, (x, y) => (x + y) % 3 === 0,
			(x, y) => (Math.floor(x / 3) + Math.floor(y / 2)) % 2 === 0, (x, y) => ((x * y) % 2) + ((x * y) % 3) === 0,
			(x, y) => (((x * y) % 2) + ((x * y) % 3)) % 2 === 0, (x, y) => (((x + y) % 2) + ((x * y) % 3)) % 2 === 0,
		];
		const apply = (m) => {
			for (let y = 0; y < size; ++y) for (let x = 0; x < size; ++x) if (!fixed[y][x] && MASKS[m](x, y)) dark[y][x] = !dark[y][x];
		};
		const penalty = () => {
			let score = 0, n = 0;
			const line = (get) => {
				for (let a = 0; a < size; ++a) {
					let run = 1;
					for (let b = 1; b <= size; ++b) {
						if (b < size && get(a, b) === get(a, b - 1)) ++run;
						else { if (run >= 5) score += 3 + run - 5; run = 1; }
					}
					// (the finder's 1:1:3:1:1 with four light modules on one side)
					for (let b = 0; b + 11 <= size; ++b) {
						const s = Array.from({ length: 11 }, (_, k) => (get(a, b + k) ? 1 : 0)).join('');
						if (s === '10111010000' || s === '00001011101') score += 40;
					}
				}
			};
			line((a, b) => dark[a][b]);
			line((a, b) => dark[b][a]);
			for (let y = 0; y + 1 < size; ++y)
				for (let x = 0; x + 1 < size; ++x) {
					const c = dark[y][x];
					if (c === dark[y][x + 1] && c === dark[y + 1][x] && c === dark[y + 1][x + 1]) score += 3;
				}
			for (const row of dark) for (const c of row) n += c ? 1 : 0;
			return score + 10 * Math.floor(Math.abs(n * 20 - size * size * 10) / (size * size));
		};
		let best = 0, lowest = Infinity;
		for (let m = 0; m < 8; ++m) {
			apply(m);
			format(m);
			const p = penalty();
			if (p < lowest) { lowest = p; best = m; }
			apply(m);
		}
		apply(best);
		format(best);
		return dark;
	};
})();

// (and in node, for a check)
if (typeof module !== 'undefined') module.exports = qr;
