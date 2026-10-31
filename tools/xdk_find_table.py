#!/usr/bin/env python3
"""keflings-nx (2026-10-30): find a table of function pointers in Keflings' image by its Banjo base counterpart.

The table at <base VA> holds <count> entries of <stride> bytes, with function addresses at the given word fields
(e.g. D3D's state tables: {getter, setter, default}, stride 12, fields 0,1). Each base entry's function words are moved
with the function map; Keflings' image is searched for the first entry's words, and the hit is checked entry by entry.

  python tools/xdk_find_table.py <base image> <kef image> <map.json> <base VA> <count> <stride> <fields>
"""
import json
import struct
import sys

BASE = 0x82000000


def main():
    bimg_p, kimg_p, map_p, va, count, stride, fields = sys.argv[1:8]
    bimg, kimg = open(bimg_p, 'rb').read(), open(kimg_p, 'rb').read()
    m = json.load(open(map_p))['map']
    va, count, stride = int(va, 16), int(count), int(stride)
    fields = [int(f) for f in fields.split(',')]

    def entry(img, a, i):
        return [struct.unpack_from('>I', img, a - BASE + i * stride + 4 * f)[0] for f in fields]

    want = []
    for i in range(count):
        ws = entry(bimg, va, i)
        want.append([int(m['%08X' % w], 16) if '%08X' % w in m else None for w in ws])
    first = next(i for i, w in enumerate(want) if all(x is not None for x in w))
    pat = b''.join(struct.pack('>I', x) for x in want[first])
    hits = []
    pos = kimg.find(pat)
    while pos >= 0:
        hits.append(pos)
        pos = kimg.find(pat, pos + 1)
    for h in hits:
        if len(fields) > 1 and fields[1] - fields[0] != 1:
            continue
        kva = BASE + h - 4 * fields[0] - first * stride
        good = bad = unknown = 0
        for i in range(count):
            got = entry(kimg, kva, i)
            for g, w in zip(got, want[i]):
                if w is None:
                    unknown += 1
                elif g == w:
                    good += 1
                else:
                    bad += 1
        print('Keflings table at %08X: %d words agree, %d differ, %d unmapped' % (kva, good, bad, unknown))
    if not hits:
        print('no hit for entry %d' % first)


if __name__ == '__main__':
    main()
