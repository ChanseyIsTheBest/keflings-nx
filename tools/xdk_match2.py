#!/usr/bin/env python3
"""keflings-nx (2026-10-30): the Banjo base -> Keflings function map for the addresses the renderer names, by
similarity (the same XDK 2.0.7645 libraries, but not byte-identical code: both games' D3D was compiled into the game
with different register allocation, so tools/xdk_match.py's exact hashes find only some of them).

Method (peggle2-nx's tools/d3d_match.py): token = primary opcode, extended opcode and register fields of each
instruction, immediates and branch displacements left out; for each wanted base function a shortlist of Keflings
functions by size and opcode histogram (cosine), then difflib's ratio on the token sequences. An exact match from
xdk_match.py's map is kept as is. The output gives score and margin to the second candidate; callers decide what is
confident (>= 0.85 and a margin of 0.05, as d3d_match.py).

  python tools/xdk_match2.py <base image> <base_sigs.json> <kef image> <kef_sigs.json> <exact map.json>
         <wanted.txt> <out.json>
"""
import collections
import difflib
import json
import math
import struct
import sys

BASE = 0x82000000
MAX_INSNS = 1500


def token(x):
    op = x >> 26
    rd, ra, rb = (x >> 21) & 31, (x >> 16) & 31, (x >> 11) & 31
    if op in (16, 18):
        return (op, x & 1, rd if op == 16 else 0, ra if op == 16 else 0)
    if op in (19, 31, 63):
        return (op, (x >> 1) & 0x3FF, rd, ra, rb)
    if op == 59:
        return (op, (x >> 1) & 0x1F, rd, ra, rb)
    if op == 30:
        return (op, (x >> 2) & 7, rd, ra)
    if op == 4:
        return (op, x & 0x7FF, rd)
    return (op, rd, ra)


def functions(image, sigs):
    starts = sorted(int(a, 16) for a in sigs)
    out = {}
    for i, s in enumerate(starts):
        n = min(sigs['%08X' % s][2], MAX_INSNS)
        off = s - BASE
        if off < 0 or off + 4 * n > len(image):
            continue
        out[s] = tuple(token(struct.unpack_from('>I', image, off + 4 * k)[0]) for k in range(n))
    return out


def histogram(tokens):
    return collections.Counter(t[0] if t[0] not in (19, 31, 59, 63) else (t[0], t[1]) for t in tokens)


def cosine(a, b):
    num = sum(a[k] * b[k] for k in a if k in b)
    den = math.sqrt(sum(v * v for v in a.values())) * math.sqrt(sum(v * v for v in b.values()))
    return num / den if den else 0.0


def main():
    bimg, bsig_p, kimg, ksig_p, exact_p, wanted_p, out_p = sys.argv[1:8]
    bsig, ksig = json.load(open(bsig_p)), json.load(open(ksig_p))
    exact = json.load(open(exact_p))['map']
    ref = functions(open(bimg, 'rb').read(), bsig)
    ours = functions(open(kimg, 'rb').read(), ksig)
    ours_hist = {s: histogram(t) for s, t in ours.items()}
    wanted = sorted({l.strip().upper() for l in open(wanted_p) if l.strip()})
    res = {}
    for a in wanted:
        addr = int(a, 16)
        rt = ref.get(addr)
        if rt is None:
            res[a] = {'k': None, 'why': 'not a base function start'}
            continue
        if a in exact:
            res[a] = {'k': exact[a], 'score': 1.0, 'next': 0.0, 'why': 'exact'}
            continue
        rh = histogram(rt)
        cands = [(cosine(rh, ours_hist[s]), s) for s, t in ours.items() if 0.7 <= len(t) / len(rt) <= 1.45]
        cands.sort(reverse=True)
        scored = []
        for _, s in cands[:40]:
            sm = difflib.SequenceMatcher(None, rt, ours[s], autojunk=False)
            scored.append((sm.ratio(), s))
        scored.sort(reverse=True)
        best = scored[0] if scored else (0, 0)
        second = scored[1] if len(scored) > 1 else (0, 0)
        res[a] = {'k': '%08X' % best[1] if best[1] else None, 'score': round(best[0], 3),
                  'next': round(second[0], 3), 'next_k': '%08X' % second[1] if second[1] else None,
                  'n': len(rt), 'why': 'similar'}
    json.dump(res, open(out_p, 'w'), indent=1)
    conf = sum(1 for r in res.values() if r.get('k') and r.get('score', 0) >= 0.85 and
               r['score'] - r.get('next', 0) >= 0.05)
    print('%d wanted: %d exact, %d confident similar, %d not starts' % (
        len(wanted), sum(1 for r in res.values() if r['why'] == 'exact'),
        conf - sum(1 for r in res.values() if r['why'] == 'exact'),
        sum(1 for r in res.values() if r['why'].startswith('not'))))


if __name__ == '__main__':
    main()
