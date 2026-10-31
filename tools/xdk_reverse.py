#!/usr/bin/env python3
"""keflings-nx (2026-10-30): the Banjo base functions most like given Keflings functions (the reverse of
tools/xdk_match2.py), to name a Keflings function the forward map did not reach.

  python tools/xdk_reverse.py <base image> <base_sigs.json> <kef image> <kef_sigs.json> <map.json> KEF_ADDR...
"""
import difflib
import json
import sys

sys.path.insert(0, __import__('os').path.dirname(__file__))
from xdk_match2 import functions, histogram, cosine  # noqa: E402


def main():
    bimg, bsig_p, kimg, ksig_p, map_p = sys.argv[1:6]
    bsig, ksig = json.load(open(bsig_p)), json.load(open(ksig_p))
    m = json.load(open(map_p))['map']
    inv = {v: k for k, v in m.items()}
    ref = functions(open(bimg, 'rb').read(), bsig)
    ours = functions(open(kimg, 'rb').read(), ksig)
    ref_hist = {s: histogram(t) for s, t in ref.items()}
    for a in sys.argv[6:]:
        k = int(a, 16)
        kt = ours[k]
        kh = histogram(kt)
        cands = sorted(((cosine(kh, ref_hist[s]), s) for s, t in ref.items() if 0.7 <= len(t) / len(kt) <= 1.45),
                       reverse=True)[:60]
        scored = sorted(((difflib.SequenceMatcher(None, kt, ref[s], autojunk=False).ratio(), s) for _, s in cands),
                        reverse=True)[:4]
        print('%s (%d ins, mapped from %s):' % (a, len(kt), inv.get(a.upper())),
              ', '.join('%08X %.2f%s' % (s, r, ' (mapped to %s)' % m['%08X' % s] if '%08X' % s in m else '')
                        for r, s in scored))


if __name__ == '__main__':
    main()
