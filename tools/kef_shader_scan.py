#!/usr/bin/env python3
"""keflings-nx (2026-10-30): the game's Xenos shader containers from its data blocks, for Renut's shader cache.

Keflings creates its shaders at run time from containers in its block files (fsxb2/blockfiles/*.blk, some bz2'd), so
captures (renut_shader_capture_dir) only hold what a run has seen. This decompresses the .bz2 blocks and writes every
container found (the same test as XenosRecomp's: flags & 0xFFFFFF00 == 0x102A1100, sizes inside the file, the two
reserved words zero) as <out>/<vs|ps>_<XXH3-free name: block>_<offset>.bin. XenosRecomp keys shaders by the content
hash, so the names only need to be unique. Game data: write it under out/ (never committed).

  python tools/kef_shader_scan.py <game blockfiles dir> <out dir>
"""
import bz2
import os
import struct
import sys


def containers(data):
    i = 0
    n = len(data)
    while True:
        i = data.find(b'\x10\x2A\x11', i)
        if i < 0 or i + 0x24 > n:
            return
        flags, virt, phys = struct.unpack_from('>III', data, i)
        f1c, f20 = struct.unpack_from('>II', data, i + 0x1C)
        size = virt + phys
        if (flags & 0xFFFFFF00) == 0x102A1100 and 0 < size <= n - i and f1c == 0 and f20 == 0 and size < (1 << 20):
            yield i, flags, data[i:i + size]
            i += size
        else:
            i += 1


def main():
    src, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    total = 0
    seen = set()
    for fn in sorted(os.listdir(src)):
        p = os.path.join(src, fn)
        raw = open(p, 'rb').read()
        if fn.endswith('.bz2'):
            # A 16-byte header, then concatenated bzip2 streams (Full_Game: 6): each decompressed to its end.
            parts = []
            pos = raw.find(b'BZh')
            while pos >= 0:
                dec = bz2.BZ2Decompressor()
                try:
                    parts.append(dec.decompress(raw[pos:]))
                except OSError:
                    pass
                nxt = len(raw) - len(dec.unused_data) if dec.eof else pos + 3
                pos = raw.find(b'BZh', max(nxt, pos + 3))
            if not parts:
                print('%s: no bzip2 stream' % fn)
                continue
            raw = b''.join(parts)
        base = fn.split('.')[0]
        n = 0
        for off, flags, blob in containers(raw):
            if blob in seen:
                continue
            seen.add(blob)
            kind = 'ps' if (flags & 0xFF) == 0 else 'vs'
            with open(os.path.join(out, '%s_%s_%08x.bin' % (kind, base, off)), 'wb') as f:
                f.write(blob)
            n += 1
        print('%s: %d MB, %d new containers' % (fn, len(raw) >> 20, n))
        total += n
    print('%d containers' % total)


if __name__ == '__main__':
    main()
