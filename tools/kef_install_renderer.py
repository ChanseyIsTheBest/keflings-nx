#!/usr/bin/env python3
"""keflings-nx (2026-10-30): install banjo-nx's base-game Renut renderer, its addresses moved to Keflings
(tools/xdk_remap_sources.py --rewrite), into app/src/renut_engine/gpu.

Every one-line hook or import (BANJO_RENUT_HOOK*, REX_EXTERN, REX_IMPORT) whose address is not a function start in
Keflings' code (out/kef_sigs.json) is commented out with a note: Banjo has the function, Keflings does not (or the map
did not find it; docs/keflings-notes.md "Addresses" lists them). Multi-line hooks (BANJO_RENUT_HOOK_RAW) at such
addresses are N&B game code (guest/widescreen.cpp) and are handled in that file by hand.

  python tools/kef_install_renderer.py <rewritten gpu dir> <kef_sigs.json> <dest gpu dir>
"""
import json
import os
import re
import shutil
import sys

ONE_LINE = re.compile(r'^(\s*)((?:BANJO_RENUT_HOOK|BANJO_RENUT_HOOK_SHARED|REX_EXTERN|REX_IMPORT)\s*\(\s*(?:__imp__)?'
                      r'sub_(8[0-9A-F]{7})\b.*)$')


def main():
    src, sigs_p, dst = sys.argv[1:4]
    kef = json.load(open(sigs_p))
    if os.path.isdir(dst):
        shutil.rmtree(dst)
    shutil.copytree(src, dst)
    off = []
    for root, _, files in os.walk(dst):
        for fn in files:
            if not fn.endswith(('.cpp', '.h')):
                continue
            p = os.path.join(root, fn)
            lines = open(p, encoding='utf-8', newline='').read().split('\n')
            changed = False
            for i, l in enumerate(lines):
                m = ONE_LINE.match(l)
                if not m or m.group(3) in kef:
                    continue
                nxt = lines[i + 1] if i + 1 < len(lines) else ''
                if 'BANJO_RENUT_HOOK_RAW' in nxt and m.group(3) in nxt:
                    continue  # the RAW hook's extern: the file is handled by hand
                lines[i] = m.group(1) + '// keflings-nx: no Keflings counterpart (Banjo base 0x%s): ' % m.group(3) + \
                    m.group(2)
                off.append('%s:%d %s' % (os.path.relpath(p, dst), i + 1, m.group(2)[:100]))
                changed = True
            if changed:
                open(p, 'w', encoding='utf-8', newline='').write('\n'.join(lines))
    for o in off:
        print('off', o)
    print('%d hook lines off' % len(off))


if __name__ == '__main__':
    main()
