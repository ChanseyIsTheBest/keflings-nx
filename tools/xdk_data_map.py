#!/usr/bin/env python3
"""keflings-nx (2026-10-30): move data addresses (globals, tables, import slots) from Banjo base to Keflings.

For a base data address D, every mapped base function (tools/xdk_*.py map) that builds D with `lis rX, hi` followed
by a D-form instruction or addi off rX (at most 8 instructions later) is found; the same two instructions in the
Keflings counterpart (same index: the functions are the same code) give Keflings' address. The candidates are voted;
the result lists every vote so a disagreement shows. An address built as D - off (a struct base) is matched too with
--near N (D within N bytes after the built address): Keflings' result is then built + (D - built).

  python tools/xdk_data_map.py <base image> <base_sigs.json> <kef image> <kef_sigs.json> <map.json> [--near N] D...
"""
import json
import struct
import sys

BASE = 0x82000000


def words(img, start, n):
    return [struct.unpack_from('>I', img, start - BASE + 4 * k)[0] for k in range(n)]


def builds(ws):
    """(index of the low half, address) of every lis/low-half pair."""
    out = []
    for i, w in enumerate(ws):
        if (w >> 26) == 15 and ((w >> 16) & 31) == 0:
            rt = (w >> 21) & 31
            hi = (w & 0xFFFF) << 16
            for j in range(i + 1, min(len(ws), i + 9)):
                x = ws[j]
                op = x >> 26
                ra = (x >> 16) & 31
                if (op == 14 or 32 <= op <= 55) and ra == rt:
                    lo = x & 0xFFFF
                    if lo & 0x8000:
                        lo -= 0x10000
                    out.append((i, j, (hi + lo) & 0xFFFFFFFF))
                if ((x >> 21) & 31) == rt and op in (14, 15, 32, 33, 34, 35, 40, 41, 42, 43):
                    break
    return out


def main():
    args = sys.argv[1:]
    near = 0
    if '--near' in args:
        k = args.index('--near')
        near = int(args[k + 1], 0)
        del args[k:k + 2]
    bimg_p, bsig_p, kimg_p, ksig_p, map_p = args[:5]
    targets = [int(a, 16) for a in args[5:]]
    bimg, kimg = open(bimg_p, 'rb').read(), open(kimg_p, 'rb').read()
    bsig, ksig = json.load(open(bsig_p)), json.load(open(ksig_p))
    m = json.load(open(map_p))['map']
    votes = {t: {} for t in targets}
    for b, k in m.items():
        if b not in bsig or k not in ksig or bsig[b][2] != ksig[k][2]:
            continue
        n = bsig[b][2]
        bw = words(bimg, int(b, 16), n)
        found = [(i, j, a) for i, j, a in builds(bw) if any(0 <= t - a <= near for t in targets)]
        if not found:
            continue
        kw = words(kimg, int(k, 16), n)
        kb = {(i, j): a for i, j, a in builds(kw)}
        for i, j, a in found:
            if (i, j) not in kb:
                continue
            for t in targets:
                if 0 <= t - a <= near:
                    ka = kb[(i, j)] + (t - a)
                    votes[t].setdefault('%08X' % ka, []).append(b)
    for t in targets:
        v = sorted(votes[t].items(), key=lambda kv: -len(kv[1]))
        print('%08X -> %s' % (t, ', '.join('%s (%d: %s)' % (a, len(fs), ' '.join(fs[:3])) for a, fs in v) or 'none'))


if __name__ == '__main__':
    main()
