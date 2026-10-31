#!/usr/bin/env python3
"""keflings-nx (2026-10-30): exact-up-to-relocation function matching between Banjo base and Keflings.

Two links of the same library object differ only where the linker wrote addresses: branch displacements (b, bl, bc),
the high half of an address (lis rX, hi) and the low half used off that register (addi / D-form load or store with
rX as base, or ori). Everything else (registers, struct offsets, shifts and masks, compare immediates) must be equal.
Each function gets a hash of its words with those fields blanked; a hash that is unique on both sides is a match.

  python tools/xdk_reloc_match.py <base image> <base_sigs.json> <kef image> <kef_sigs.json> <in map.json>
         <out map.json>
The output map is the input map plus the new matches (an input entry that disagrees is reported and replaced).
"""
import hashlib
import json
import struct
import sys

BASE = 0x82000000


def reloc_hash(image, start, n):
    h = hashlib.sha1()
    lis_regs = set()
    for k in range(n):
        off = start - BASE + 4 * k
        if off + 4 > len(image):
            break
        w = struct.unpack_from('>I', image, off)[0]
        op = w >> 26
        rt, ra = (w >> 21) & 31, (w >> 16) & 31
        if op == 18:
            w &= 0xFC000003
        elif op == 16:
            w &= 0xFFFF0003
        elif op == 15 and ra == 0:  # lis
            w &= 0xFFFF0000
            lis_regs.add(rt)
        elif op in (14, 24) and ra in lis_regs:  # addi / ori off an address high half
            w &= 0xFFFF0000
            if rt != ra:
                lis_regs.add(rt)
        elif 32 <= op <= 55 and ra in lis_regs:  # D-form load / store off an address high half
            w &= 0xFFFF0000
            if op < 36 or op in (40, 41, 42, 43, 46, 48, 49, 50, 51):
                lis_regs.discard(rt) if rt != ra else None
        else:
            # Any other write to a tracked register ends its role as an address base.
            if op in (14, 15, 24, 25, 28, 29, 21, 20, 7, 8, 12, 13) or op == 31:
                dst = ra if op in (24, 25, 28, 29, 21, 20) or (op == 31 and ((w >> 1) & 0x3FF) in (444, 28, 60, 316, 412, 476, 124, 954, 922, 986, 26, 24, 536, 792, 824)) else rt
                lis_regs.discard(dst)
            if 32 <= op <= 39 or op in (40, 41, 42, 43, 46, 58):
                lis_regs.discard(rt)
        h.update(struct.pack('>I', w))
    return h.hexdigest()[:20]


def table(image, sigs):
    out = {}
    for a, s in sigs.items():
        out[a] = reloc_hash(image, int(a, 16), s[2])
    return out


def main():
    bimg_p, bsig_p, kimg_p, ksig_p, in_p, out_p = sys.argv[1:7]
    bimg, kimg = open(bimg_p, 'rb').read(), open(kimg_p, 'rb').read()
    bsig, ksig = json.load(open(bsig_p)), json.load(open(ksig_p))
    data = json.load(open(in_p))
    m, how = data['map'], data['how']
    bh, kh = table(bimg, bsig), table(kimg, ksig)
    bidx, kidx = {}, {}
    for a, h in bh.items():
        bidx.setdefault(h, []).append(a)
    for a, h in kh.items():
        kidx.setdefault(h, []).append(a)
    new = changed = 0
    agree = 0
    exact = {}
    for h, al in bidx.items():
        if len(al) == 1 and len(kidx.get(h, [])) == 1:
            exact[al[0]] = kidx[h][0]
    for b, k in exact.items():
        if b in m and m[b] == k:
            agree += 1
            how[b] = how.get(b, '') + ' +reloc'
        elif b in m:
            print('replaced %s: %s (%s) -> %s (reloc-exact)' % (b, m[b], how.get(b), k))
            m[b] = k
            how[b] = 'reloc'
            changed += 1
        else:
            m[b] = k
            how[b] = 'reloc'
            new += 1
    # One Keflings function per base function: drop older entries that now collide with a reloc-exact one.
    taken = {}
    for b, k in list(m.items()):
        if k in taken and taken[k] != b:
            loser = b if how.get(b, '').startswith('reloc') is False and b not in exact else taken[k]
            if loser in exact:
                loser = b if loser == taken[k] else taken[k]
            print('collision on %s: %s / %s, dropped %s' % (k, taken[k], b, loser))
            del m[loser]
            if loser == taken[k]:
                taken[k] = b
        else:
            taken[k] = b
    json.dump({'map': m, 'how': how}, open(out_p, 'w'), indent=0)
    print('reloc-exact %d (agree %d, new %d, replaced %d); map now %d' % (len(exact), agree, new, changed, len(m)))


if __name__ == '__main__':
    main()
