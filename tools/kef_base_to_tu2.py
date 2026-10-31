#!/usr/bin/env python3
"""keflings-nx (2026-10-30, evening): move the port's Keflings addresses from the base game 0.0.1.2 to title update 2,
in place: the reverse of tools/kef_tu2_to_base.py (the user's request: title update 2, with every hand-written native
and localized copy carried over).

Functions: the TU2 -> base map of out/tu2_to_base5.json, inverted (1:1); an address is moved only when it is a base
function start (out/kef_sigs.json) in the map. Data: kef_tu2_to_base.DATA, inverted. Forms moved:
  sub_XXXXXXXX, 0xXXXXXXXX, n_XXXXXXXX (hand natives' namespaces) in sources and tomls;
  a bare XXXXXXXX token (8 hex digits) in the .txt lists (localized, bulk, hand, indirect promotions).
Instruction addresses inside a function (mid-asm hooks) are not function starts: they are listed (--report) and
moved by hand (function start + the same offset where the function is an exact match).

  python tools/kef_base_to_tu2.py <tu2_to_base map.json> <base sigs.json> <file or dir>... [--dry]
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from kef_tu2_to_base import DATA  # TU2 -> base

PREFIXED = re.compile(r'\b(sub_|n_|f_|o_|0x)(8[0-9A-Fa-f]{7})\b')
BARE = re.compile(r'(?<![0-9A-Za-z_])(8[0-9A-Fa-f]{7})(?![0-9A-Za-z_])')
EXTS = ('.cpp', '.h', '.inc', '.hlsl', '.hlsli', '.toml', '.txt')


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    dry = '--dry' in sys.argv
    m = json.load(open(args[0]))
    t2b, how = m['map'], m['how']
    base_sigs = json.load(open(args[1]))
    b2t = {b: t for t, b in t2b.items()}
    data_b2t = {b: t for t, b in DATA.items()}
    paths = []
    for root in args[2:]:
        if os.path.isfile(root):
            paths.append(root)
            continue
        for d, _, fs in os.walk(root):
            paths += [os.path.join(d, f) for f in fs if f.endswith(EXTS)]
    total = {'functions': 0, 'data': 0, 'files': 0, 'unmapped': set(), 'inexact': set()}
    for p in paths:
        raw = open(p, 'rb').read()
        had_bom = raw[:3] == b'\xef\xbb\xbf'
        t = raw.decode('utf-8-sig')

        def move(a, keep_case_of):
            a = a.upper()
            if a in base_sigs and a in b2t:
                total['functions'] += 1
                if not how.get(b2t[a], '').startswith('exact'):
                    total['inexact'].add(a)
                k = b2t[a]
                return k if keep_case_of.isupper() or keep_case_of.isdigit() else k.lower()
            if a in data_b2t:
                total['data'] += 1
                return data_b2t[a]
            if a in base_sigs:
                total['unmapped'].add(a)
            return None

        def psub(mm):
            k = move(mm.group(2), mm.group(2))
            return mm.group(1) + k if k else mm.group(0)

        u = PREFIXED.sub(psub, t)
        if p.endswith('.txt'):
            def bsub(mm):
                k = move(mm.group(1), mm.group(1))
                return k if k else mm.group(0)
            u = BARE.sub(bsub, u)
        if u != t:
            total['files'] += 1
            if not dry:
                with open(p, 'w', encoding='utf-8-sig' if had_bom else 'utf-8', newline='') as f:
                    f.write(u)
    print('%s%d files changed: %d function addresses, %d data addresses moved' %
          ('(dry run) ' if dry else '', total['files'], total['functions'], total['data']))
    if total['inexact']:
        print('moved by a non-exact match (check these):', ' '.join(sorted(total['inexact'])))
    if total['unmapped']:
        print('base functions with no TU2 counterpart (left as they are):', ' '.join(sorted(total['unmapped'])))


if __name__ == '__main__':
    main()
