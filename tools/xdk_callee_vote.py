#!/usr/bin/env python3
"""keflings-nx (2026-10-30): map the wanted base functions that are still unmapped by their callees.

For an unmapped base function B whose callees are (partly) mapped, every Keflings function that calls those callees'
counterparts gets a vote per shared callee. The best candidate is taken when it shares at least two mapped callees and
at least 60 % of them, is free, its size is within 0.5x-2x of B's, and the runner-up shares fewer.

  python tools/xdk_callee_vote.py <base image> <base_sigs.json> <kef image> <kef_sigs.json> <in map.json>
         <wanted.txt> <out map.json>
"""
import json
import sys

sys.path.insert(0, __import__('os').path.dirname(__file__))
from xdk_propagate import calls  # noqa: E402


def main():
    bimg_p, bsig_p, kimg_p, ksig_p, in_p, wanted_p, out_p = sys.argv[1:8]
    bimg, kimg = open(bimg_p, 'rb').read(), open(kimg_p, 'rb').read()
    bsig, ksig = json.load(open(bsig_p)), json.load(open(ksig_p))
    data = json.load(open(in_p))
    m, how = data['map'], data['how']
    bc, kc = calls(bimg, bsig), calls(kimg, ksig)
    callers = {}
    for a, lst in kc.items():
        for t in set(lst):
            callers.setdefault(t, set()).add(a)
    wanted = sorted({l.strip().upper() for l in open(wanted_p) if l.strip()})
    used = set(m.values())
    for rnd in range(3):
        added = 0
        for b in wanted:
            if b in m or b not in bsig:
                continue
            mapped = {m[c] for c in set(bc.get(b, [])) if c in m}
            if len(mapped) < 2:
                continue
            votes = {}
            for kcal in mapped:
                for f in callers.get(kcal, ()):
                    votes[f] = votes.get(f, 0) + 1
            ranked = sorted(votes.items(), key=lambda kv: -kv[1])
            if not ranked:
                continue
            best, nb = ranked[0]
            second = ranked[1][1] if len(ranked) > 1 else 0
            size = ksig[best][2] / max(1, bsig[b][2])
            if nb >= 2 and nb >= 0.6 * len(mapped) and nb > second and best not in used and 0.5 <= size <= 2.0:
                m[b] = best
                used.add(best)
                how[b] = 'callee vote %d/%d (next %d)' % (nb, len(mapped), second)
                added += 1
        if not added:
            break
    json.dump({'map': m, 'how': how}, open(out_p, 'w'), indent=0)
    left = [b for b in wanted if b in bsig and b not in m]
    print('map %d; wanted still unmapped: %d' % (len(m), len(left)))


if __name__ == '__main__':
    main()
