#!/usr/bin/env python3
"""keflings-nx (2026-10-30): move function addresses from Banjo-Kazooie: Nuts & Bolts' BASE game (XDK 2.0.7645) to
A Kingdom for Keflings (the same XDK), by content.

Both executables link the same XDK libraries (D3D9, XAPILIB, XAUD, the CRT ...), so a library function is the same
instruction text at another address. tools/func_signatures.py gives each function a 'text' hash (branch targets and
0x8xxxxxxx addresses blanked) and a 'shape' hash (immediates blanked too). A base function maps to the Keflings
function that is the only one with its text hash on both sides (an anchor); then the anchors' call lists are walked:
when a mapped pair calls the same number of functions and the unmapped callees pair up one to one by text or shape,
they map too (until nothing changes).

  python tools/xdk_match.py <base_sigs.json> <kef_sigs.json> <out_map.json>
"""
import json
import sys
from collections import defaultdict


def uniq_index(sigs, key):
    idx = defaultdict(list)
    for a, s in sigs.items():
        idx[s[key]].append(a)
    return idx


def main():
    base = json.load(open(sys.argv[1]))
    kef = json.load(open(sys.argv[2]))
    out = sys.argv[3]
    bt, kt = uniq_index(base, 1), uniq_index(kef, 1)
    bs, ks = uniq_index(base, 0), uniq_index(kef, 0)
    m = {}
    how = {}
    for h, al in bt.items():
        if len(al) == 1 and len(kt.get(h, [])) == 1 and base[al[0]][2] >= 4:
            m[al[0]] = kt[h][0]
            how[al[0]] = 'text'
    for h, al in bs.items():
        if len(al) == 1 and len(ks.get(h, [])) == 1 and al[0] not in m and base[al[0]][2] >= 8:
            k = ks[h][0]
            if k not in m.values():
                m[al[0]] = k
                how[al[0]] = 'shape'
    used = set(m.values())
    # Call-list propagation.
    changed = True
    rounds = 0
    while changed and rounds < 20:
        changed = False
        rounds += 1
        for b, k in list(m.items()):
            bc = [c for c in base[b][4] if c in base]
            kc = [c for c in kef[k][4] if c in kef]
            if len(bc) != len(kc) or not bc:
                continue
            ub = [c for c in bc if c not in m]
            uk = [c for c in kc if c not in used]
            for c in ub:
                cand = [x for x in uk if kef[x][1] == base[c][1]]
                if len(cand) != 1:
                    cand = [x for x in uk if kef[x][0] == base[c][0]]
                if len(cand) == 1 and cand[0] not in used:
                    m[c] = cand[0]
                    how[c] = 'call:' + b
                    used.add(cand[0])
                    uk.remove(cand[0])
                    changed = True
    json.dump({'map': m, 'how': how}, open(out, 'w'), indent=0)
    n_text = sum(1 for v in how.values() if v == 'text')
    n_shape = sum(1 for v in how.values() if v == 'shape')
    print('base %d, keflings %d: mapped %d (text %d, shape %d, calls %d) in %d rounds' % (
        len(base), len(kef), len(m), n_text, n_shape, len(m) - n_text - n_shape, rounds))


if __name__ == '__main__':
    main()
