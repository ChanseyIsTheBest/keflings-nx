#!/usr/bin/env python3
"""keflings-nx (2026-10-30): the interpolator tables of Xenos shader containers (register, usage, usage index), as
XenosRecomp's shader.h lays them out, to compare what a vertex shader exports with what its pixel shader reads.

  python tools/kef_interpolators.py <dump dir> [hash ...]     (hash: the 16-digit XXH3 of the container)
"""
import os
import struct
import sys

try:
    import xxhash  # noqa: F401
except ImportError:
    xxhash = None

USAGE = ['Position', 'BlendWeight', 'BlendIndices', 'Normal', 'PointSize', 'TexCoord', 'Tangent', 'Binormal',
         'TessFactor', 'PositionT', 'Color', 'Fog', 'Depth', 'Sample']


def table(blob):
    flags = struct.unpack_from('>I', blob, 0)[0]
    shader_off = struct.unpack_from('>I', blob, 0x18)[0]
    s = shader_off
    interp_info = struct.unpack_from('>I', blob, s + 0x14)[0]
    count = (interp_info >> 5) & 0x1F
    pixel = (flags & 0xFF) == 0
    out = []
    for i in range(count):
        if pixel:
            v = struct.unpack_from('>I', blob, s + 0x20 + 4 * i)[0]
        else:
            f18, nelem = struct.unpack_from('>II', blob, s + 0x18)
            v = struct.unpack_from('>I', blob, s + 0x24 + 4 * (f18 + nelem + i))[0]
        idx, usage, reg = v & 0xF, (v >> 4) & 0xF, (v >> 8) & 0xF
        out.append((reg, USAGE[usage] if usage < len(USAGE) else str(usage), idx))
    return ('ps' if pixel else 'vs'), sorted(out)


def main():
    d = sys.argv[1]
    want = {h.upper() for h in sys.argv[2:]}
    for fn in sorted(os.listdir(d)):
        if not fn.endswith('.bin'):
            continue
        name = fn[3:-4].upper()
        if want and name not in want:
            continue
        blob = open(os.path.join(d, fn), 'rb').read()
        try:
            kind, t = table(blob)
        except struct.error:
            continue
        print('%s %s: %s' % (kind, name, ', '.join('r%d=%s%d' % x for x in t)))


if __name__ == '__main__':
    main()
