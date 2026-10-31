#!/usr/bin/env python3
"""keflings-nx (2026-10-30): last pass of the Banjo base -> Keflings map, for wanted functions only.

An unmapped wanted base function B between its nearest mapped neighbours P < B < S (in Banjo's address order) whose
counterparts are in order (KP < KS) and at most 12 functions apart: the free Keflings functions between KP and KS are
scored by token similarity (tools/xdk_match2.py's tokens); the best is taken at >= 0.5 with a margin of 0.15.
Hand entries (--manual base=kef,...) are applied first, each with its evidence in docs/keflings-notes.md.

  python tools/xdk_bracket.py <base image> <base_sigs.json> <kef image> <kef_sigs.json> <in map.json> <wanted.txt>
         <out map.json> [--manual B=K,B=K]
"""
import bisect
import json
import sys

sys.path.insert(0, __import__('os').path.dirname(__file__))
from xdk_match2 import functions  # noqa: E402
from xdk_propagate import ratio  # noqa: E402


def main():
    bimg_p, bsig_p, kimg_p, ksig_p, in_p, wanted_p, out_p = sys.argv[1:8]
    manual = {}
    if len(sys.argv) > 9 and sys.argv[8] == '--manual':
        for pair in sys.argv[9].split(','):
            b, k = pair.split('=')
            manual[b.upper()] = k.upper()
    bsig, ksig = json.load(open(bsig_p)), json.load(open(ksig_p))
    data = json.load(open(in_p))
    m, how = data['map'], data['how']
    for b, k in manual.items():
        m[b] = k
        how[b] = 'manual'
    bt = functions(open(bimg_p, 'rb').read(), bsig)
    kt = functions(open(kimg_p, 'rb').read(), ksig)
    bs, ks = sorted(bsig), sorted(ksig)
    wanted = sorted({l.strip().upper() for l in open(wanted_p) if l.strip()})
    used = set(m.values())
    added = 0
    for b in wanted:
        if b in m or b not in bsig:
            continue
        i = bs.index(b)
        p = next((bs[j] for j in range(i - 1, max(-1, i - 30), -1) if bs[j] in m), None)
        s = next((bs[j] for j in range(i + 1, min(len(bs), i + 30)) if bs[j] in m), None)
        if not p or not s or m[p] >= m[s]:
            continue
        lo, hi = bisect.bisect_right(ks, m[p]), bisect.bisect_left(ks, m[s])
        if hi - lo > 12 or hi <= lo:
            continue
        cands = sorted(((ratio(bt.get(int(b, 16)), kt.get(int(k, 16))), k) for k in ks[lo:hi] if k not in used),
                       reverse=True)
        if not cands:
            continue
        best = cands[0]
        nxt = cands[1][0] if len(cands) > 1 else 0.0
        if best[0] >= 0.5 and best[0] - nxt >= 0.15:
            m[b] = best[1]
            used.add(best[1])
            how[b] = 'bracket %s..%s (%.2f/%.2f)' % (p, s, best[0], nxt)
            added += 1
    json.dump({'map': m, 'how': how}, open(out_p, 'w'), indent=0)
    left = [b for b in wanted if b in bsig and b not in m]
    print('bracket added %d; map %d; wanted still unmapped %d' % (added, len(m), len(left)))
    for b in left:
        print('  unmapped', b, bsig[b][2])


if __name__ == '__main__':
    main()
