#!/usr/bin/env python3
"""keflings-nx (2026-10-30): grow the Banjo base -> Keflings function map along the call graph.

Seeds: tools/xdk_match.py's exact map plus tools/xdk_match2.py's similar matches that are clear (score >= 0.85 with a
margin of 0.05, or score >= 0.5 with a margin of 0.25). Then, for every mapped pair, the two functions' bl targets in
instruction order are aligned (difflib over the targets' mapped names); inside a replaced run of the same length the
unmapped callees pair up in order, if their token similarity is >= 0.45 and no other function holds the Keflings side.
Callers likewise: for an unmapped base function, a mapped caller's aligned call site names its counterpart.

  python tools/xdk_propagate.py <base image> <base_sigs.json> <kef image> <kef_sigs.json> <exact.json>
         <similar.json> <out map.json>
"""
import difflib
import json
import struct
import sys

sys.path.insert(0, __import__('os').path.dirname(__file__))
from xdk_match2 import functions, BASE  # noqa: E402


def calls(image, sigs):
    out = {}
    for a, s in sigs.items():
        start = int(a, 16)
        n = s[2]
        lst = []
        for k in range(n):
            off = start - BASE + 4 * k
            if off + 4 > len(image):
                break
            w = struct.unpack_from('>I', image, off)[0]
            if (w >> 26) == 18 and (w & 1):
                li = w & 0x03FFFFFC
                if li & 0x02000000:
                    li -= 0x04000000
                t = (li if (w & 2) else start + 4 * k + li) & 0xFFFFFFFF
                lst.append('%08X' % t)
        out[a] = lst
    return out


def ratio(a, b):
    if not a or not b:
        return 0.0
    if not (0.6 <= len(a) / len(b) <= 1.66):
        return 0.0
    return difflib.SequenceMatcher(None, a, b, autojunk=False).ratio()


def main():
    bimg_p, bsig_p, kimg_p, ksig_p, exact_p, sim_p, out_p = sys.argv[1:8]
    bimg, kimg = open(bimg_p, 'rb').read(), open(kimg_p, 'rb').read()
    bsig, ksig = json.load(open(bsig_p)), json.load(open(ksig_p))
    exact = json.load(open(exact_p))['map']
    sim = json.load(open(sim_p))
    m = dict(exact)
    how = {a: 'exact' for a in exact}
    for a, r in sim.items():
        if r.get('k') and r['why'] == 'similar':
            s, nx = r['score'], r.get('next', 0)
            if (s >= 0.85 and s - nx >= 0.05) or (s >= 0.5 and s - nx >= 0.25):
                if r['k'] not in set(m.values()):
                    m[a] = r['k']
                    how[a] = 'similar %.2f/%.2f' % (s, nx)
    bt = functions(bimg, bsig)
    kt = functions(kimg, ksig)
    bc, kc = calls(bimg, bsig), calls(kimg, ksig)
    kcallers = {}
    for a, lst in kc.items():
        for t in lst:
            kcallers.setdefault(t, set()).add(a)
    used = set(m.values())
    rounds = 0
    while True:
        rounds += 1
        added = 0
        for b, k in list(m.items()):
            la = [m.get(t, '?' + t) for t in bc.get(b, [])]
            lb = [t if t in used else '!' + t for t in kc.get(k, [])]
            sm = difflib.SequenceMatcher(None, la, lb, autojunk=False)
            for op, i1, i2, j1, j2 in sm.get_opcodes():
                if op != 'replace' or (i2 - i1) != (j2 - j1):
                    continue
                for x, y in zip(la[i1:i2], lb[j1:j2]):
                    if not (x.startswith('?') and y.startswith('!')):
                        continue
                    xb, yk = x[1:], y[1:]
                    if xb in m or yk in used:
                        continue
                    r = ratio(bt.get(int(xb, 16)), kt.get(int(yk, 16)))
                    if r >= 0.45:
                        m[xb] = yk
                        used.add(yk)
                        how[xb] = 'callee of %s (%.2f)' % (b, r)
                        added += 1
        if not added or rounds > 30:
            break
    # Tiny functions (getters, setters: 2-7 instructions) whose exact text is unique on both sides.
    btext, ktext = {}, {}
    for a, s in bsig.items():
        btext.setdefault(s[1], []).append(a)
    for a, s in ksig.items():
        ktext.setdefault(s[1], []).append(a)
    tiny = 0
    for h, al in btext.items():
        if len(al) == 1 and al[0] not in m and len(ktext.get(h, [])) == 1 and ktext[h][0] not in used:
            m[al[0]] = ktext[h][0]
            used.add(ktext[h][0])
            how[al[0]] = 'tiny text'
            tiny += 1
    # Gap fill: library code keeps its object order, so between two anchors that are in order on both sides and hold
    # the same number of functions between them, the functions pair up in order (each checked by similarity).
    bstarts = sorted(bsig)
    kstarts = sorted(ksig)
    bpos = {a: i for i, a in enumerate(bstarts)}
    kpos = {a: i for i, a in enumerate(kstarts)}
    gap = 0
    for _ in range(3):
        anchors = sorted((bpos[b], kpos[k]) for b, k in m.items() if b in bpos and k in kpos)
        for (b0, k0), (b1, k1) in zip(anchors, anchors[1:]):
            if b1 - b0 != k1 - k0 or b1 - b0 < 2 or b1 - b0 > 40 or k1 <= k0:
                continue
            for d in range(1, b1 - b0):
                xb, yk = bstarts[b0 + d], kstarts[k0 + d]
                if xb in m or yk in used:
                    continue
                r = ratio(bt.get(int(xb, 16)), kt.get(int(yk, 16)))
                if r >= 0.6:
                    m[xb] = yk
                    used.add(yk)
                    how[xb] = 'gap (%.2f)' % r
                    gap += 1
    json.dump({'map': m, 'how': how}, open(out_p, 'w'), indent=0)
    print('mapped %d after %d rounds (exact %d, tiny %d, gap %d)' % (len(m), rounds, len(exact), tiny, gap))


if __name__ == '__main__':
    main()
