#!/usr/bin/env python3
"""keflings-nx (2026-10-30): list (or rewrite) the Banjo base addresses in a source tree with their Keflings
addresses from tools/xdk_match.py's map.

  python tools/xdk_remap_sources.py <map.json> <base_sigs.json> <src dir> [--rewrite <out dir>]

Only addresses that are function starts in Banjo base are moved (sub_X and 0xX forms). An address inside a function
(a mid-function hook or a data address) is reported as such; an unmapped function is reported with the line it is on.
--rewrite writes the tree to <out dir> with every mapped address replaced (case kept: sub_ upper, 0x as written).
"""
import json
import os
import re
import sys

ADDR = re.compile(r'(sub_|0x)(8[0-9A-Fa-f]{7})\b')


def main():
    mp = json.load(open(sys.argv[1]))['map']
    base = json.load(open(sys.argv[2]))
    src = sys.argv[3]
    rewrite = sys.argv[5] if len(sys.argv) > 5 and sys.argv[4] == '--rewrite' else None
    starts = sorted(base)
    seen = {}
    for root, _, files in os.walk(src):
        for fn in files:
            if not fn.endswith(('.cpp', '.h', '.inc', '.hlsl', '.hlsli', '.toml', '.txt')):
                continue
            path = os.path.join(root, fn)
            rel = os.path.relpath(path, src)
            text = open(path, encoding='utf-8', errors='replace').read()
            lines = text.split('\n')
            for i, line in enumerate(lines):
                for m in ADDR.finditer(line):
                    a = m.group(2).upper()
                    key = (rel, a)
                    if key in seen:
                        continue
                    if a in mp:
                        kind = 'mapped ' + mp[a]
                    elif a in base:
                        kind = 'UNMAPPED function'
                    else:
                        kind = 'not a function start'
                    seen[key] = (kind, i + 1, line.strip()[:140])
            if rewrite:
                def sub(m):
                    a = m.group(2).upper()
                    if a in mp:
                        k = mp[a]
                        return m.group(1) + (k if m.group(2).isupper() or m.group(1) == 'sub_' else k.lower())
                    return m.group(0)
                dst = os.path.join(rewrite, rel)
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                with open(dst, 'w', encoding='utf-8', newline='') as f:
                    f.write(ADDR.sub(sub, text))
    counts = {}
    for (rel, a), (kind, ln, line) in sorted(seen.items()):
        k = kind.split()[0]
        counts[k] = counts.get(k, 0) + 1
        if not kind.startswith('mapped'):
            print('%s:%d %s %s | %s' % (rel, ln, a, kind, line))
    print(counts)


if __name__ == '__main__':
    main()
