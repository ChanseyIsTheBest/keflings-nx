#!/usr/bin/env python3
"""keflings-nx (2026-10-30): Banjo base -> Keflings data addresses in the renderer (app/src/renut_engine/gpu), as
found by tools/xdk_data_map.py (lis/low-half pairs in mapped functions) and tools/xdk_find_table.py (D3D's state
tables, every getter and setter word checked). docs/keflings-notes.md "Addresses" has the evidence.

  python tools/kef_data_addresses.py app/src/renut_engine/gpu
"""
import os
import re
import sys

DATA = {
    '820052F8': '820013B8',  # D3D's per-format table (9 functions agree)
    '82E506F0': '82470680',  # render state table {getter, setter, default} x 101 (202 of 202 words)
    '82E50BD0': '82470B40',  # sampler state table x 20 (40 of 40 words)
    '82FAD204': '82490E38',  # D3D's skip-export NaN-vertex flag (DoEndExportNanVertex)
    '82FAD208': '82490E3C',  # D3D's skip-export 512-pixels flag (DoEndExport512Pixels)
    '820007FC': '820005FC',  # the VdGlobalDevice import slot (5 functions agree)
    '820018DC': '82001250',  # D3D's full-viewport constant
    '82F39E34': '82490E20',  # XPhysicalAlloc's no-large-pages flag
    '82FA36C8': '82490E40',  # the HiZ-owner word
    '82003E38': '82001278',  # MaxAnisotropy's ratio table (3 functions agree)
}


def main():
    root = sys.argv[1]
    n = 0
    for d, _, files in os.walk(root):
        for fn in files:
            if not fn.endswith(('.cpp', '.h', '.hlsl')):
                continue
            p = os.path.join(d, fn)
            t = open(p, encoding='utf-8', newline='').read()
            u = re.sub(r'0x(%s)\b' % '|'.join(DATA), lambda m: '0x' + DATA[m.group(1).upper()], t, flags=re.I)
            if u != t:
                n += 1
                open(p, 'w', encoding='utf-8', newline='').write(u)
    print('%d files changed' % n)


if __name__ == '__main__':
    main()
