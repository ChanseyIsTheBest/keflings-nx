#!/usr/bin/env python3
"""keflings-nx (2026-10-30): move the renderer's and the port's own Keflings addresses from title update 2 to the base
game 0.0.1.2 (docs/keflings-notes.md, "Why the base game"), in place.

Functions: the TU2 -> base map (tools/xdk_match.py, xdk_reloc_match.py, xdk_match2.py, xdk_propagate.py,
xdk_callee_vote.py, xdk_bracket.py with TU2 as the reference); an address is moved only when it is a TU2 function start
in the map. Data (moved afterwards, so a moved function address is never taken for data): tools/xdk_data_map.py and
tools/xdk_find_table.py with the same map.

  python tools/kef_tu2_to_base.py <tu2_to_base map.json> <tu2 sigs.json> <dir>...
"""
import json
import os
import re
import sys

DATA = {  # TU2 -> base, with the evidence of tools/xdk_data_map.py / xdk_find_table.py
    '82470680': '82440650',  # render state table: 202 of 202 getter/setter words
    '82470B40': '82440B10',  # sampler state table: 40 of 40
    '82490E38': '824606D8',  # skip-export NaN-vertex flag
    '82490E3C': '824606DC',  # skip-export 512-pixels flag
    '820005FC': '820005F8',  # VdGlobalDevice import slot (19 functions agree)
    '82490E20': '824606C0',  # XPhysicalAlloc's no-large-pages flag
    '82490E40': '824606E0',  # HiZ-owner word (4 functions agree)
    '82470CA0': '82440C70',  # the global qword every device copies (sub_821D33F0: lis r11,0x8244; ld r11,3184(r11))
    # unchanged: 820013B8 (format table), 82001250 (full viewport), 82001278 (anisotropy table)
}
ADDR = re.compile(r'(sub_|0x)(8[0-9A-Fa-f]{7})\b')


def main():
    m = json.load(open(sys.argv[1]))['map']
    tu2 = json.load(open(sys.argv[2]))
    changed_files = moved = data = 0
    for root in sys.argv[3:]:
        for d, _, fs in os.walk(root):
            for fn in fs:
                if not fn.endswith(('.cpp', '.h', '.inc', '.hlsl', '.hlsli')):
                    continue
                p = os.path.join(d, fn)
                t = open(p, encoding='utf-8-sig', newline='').read()
                had_bom = open(p, 'rb').read(3) == b'\xef\xbb\xbf'

                def fsub(mm):
                    nonlocal moved
                    a = mm.group(2).upper()
                    if a in tu2 and a in m:
                        moved += 1
                        k = m[a]
                        return mm.group(1) + (k if mm.group(1) == 'sub_' or mm.group(2).isupper() else k.lower())
                    return mm.group(0)

                u = ADDR.sub(fsub, t)

                def dsub(mm):
                    nonlocal data
                    a = mm.group(2).upper()
                    if mm.group(1) == '0x' and a in DATA:
                        data += 1
                        return '0x' + DATA[a]
                    return mm.group(0)

                u = ADDR.sub(dsub, u)
                if u != t:
                    changed_files += 1
                    with open(p, 'w', encoding='utf-8-sig' if had_bom else 'utf-8', newline='') as f:
                        f.write(u)
    print('%d files changed: %d function addresses, %d data addresses moved' % (changed_files, moved, data))


if __name__ == '__main__':
    main()
