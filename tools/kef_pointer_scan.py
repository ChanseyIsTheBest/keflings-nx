#!/usr/bin/env python3
"""keflings-nx (2026-10-30): functions reached only through code pointers in data (vtables, callback tables), which
the generator's discovery does not see (PC run 4: "Call to invalid or unregistered function at guest address
0x820A94B8", a two-instruction vtable setter). The method of peggle2-nx's / zumasrevenge-nx's pointer_fixpoint.py,
cut down to this port:

  - the known functions and their extents: out/kef_sigs.json (tools/func_signatures.py on app/generated);
  - every 4-aligned word of the image outside the code range whose value is a 4-aligned address inside the code range,
    not a known function start and not inside any known function's extent (a jump-table label is inside its
    function's extent; a missed function is in a gap);
  - accepted when the word before the target ends code (blr, bctr, an unconditional b, or zero padding) and the
    target's own first word is not zero.
Each accepted target is printed as a config/renut_funcs.toml line with the address of one pointer that names it.

  python tools/kef_pointer_scan.py <image> <kef_sigs.json> <code lo> <code hi>
"""
import bisect
import json
import struct
import sys

BASE = 0x82000000


def ends_code(w):
    if w in (0x4E800020, 0x4E800420, 0):
        return True
    return (w >> 26) == 18 and (w & 1) == 0  # b / ba without link


def main():
    img = open(sys.argv[1], 'rb').read()
    sigs = json.load(open(sys.argv[2]))
    lo, hi = int(sys.argv[3], 16), int(sys.argv[4], 16)
    starts = sorted(int(a, 16) for a in sigs)
    ends = {s: s + 4 * sigs['%08X' % s][2] for s in starts}
    known = set(starts)
    # Everything the generated function table registers (import thunks have no instruction comments, so
    # func_signatures.py does not list them).
    import os
    import re
    init = os.path.join(os.path.dirname(os.path.abspath(sys.argv[2])), '..', 'app', 'generated', 'renut_init.cpp')
    if os.path.exists(init):
        known |= {int(m.group(1), 16) for m in re.finditer(r'\{ 0x([0-9A-Fa-f]{8}), ', open(init).read())}
    found = {}
    for off in range(0, len(img) - 3, 4):
        va = BASE + off
        if lo <= va < hi:
            continue
        t = struct.unpack_from('>I', img, off)[0]
        if not (lo <= t < hi) or t & 3 or t in known or t in found:
            continue
        i = bisect.bisect_right(starts, t) - 1
        if i >= 0 and t < ends[starts[i]]:
            continue  # inside a known function (a label)
        before = struct.unpack_from('>I', img, t - 4 - BASE)[0]
        first = struct.unpack_from('>I', img, t - BASE)[0]
        if ends_code(before) and first != 0:
            found[t] = va
    for t in sorted(found):
        print('0x%08X = {}  # pointer at 0x%08X' % (t, found[t]))
    print('# %d functions reached only by pointer' % len(found), file=sys.stderr)


if __name__ == '__main__':
    main()
