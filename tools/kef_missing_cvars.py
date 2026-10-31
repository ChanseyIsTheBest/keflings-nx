#!/usr/bin/env python3
"""keflings-nx (2026-10-30): the cvars that the built sources use but whose definitions were in banjo-nx files this
port does not build (its PM4 renderers, N&B's game hooks, N&B's natives). Each definition is copied as banjo-nx has it
(same type, default, category and text) into app/src/keflings/keflings_banjo_cvars.cpp, so banjo's enhancement table
and the renderer link; the code those keys switched is not in this port, so they change nothing here.

  python tools/kef_missing_cvars.py <banjo-nx app/src> <this port's root>
"""
import os
import re
import sys

EXCLUDED = [
    'me/', 'native/', 'renut_engine/fixes/', 'renut_engine/gpu/guest/photo_capture.cpp',
    'renut_engine/hooks.cpp', 'renut_engine/StopNSwap.cpp', 'renut_engine/frameHooks.cpp',
    'renut_engine/cvar_menu.cpp', 'renut_engine/shader_dump.cpp', 'renut_engine/texture_tools.cpp',
    'renut_engine/frontend_debug_menu.cpp', 'renut_engine/townsfolk.cpp', 'renut_engine/thread_slots.cpp',
    'renut_engine/env_map_pool.cpp', 'renut_engine/mnk_controls.cpp', 'renut_engine/FPS.cpp',
    'cpu/banjo_pool_lock.cpp', 'cpu/banjo_reverb_native.cpp', 'cpu/banjo_xaudio_native.cpp',
    'keflings/keflings_banjo_cvars.cpp',  # this tool's own output
]
KEPT = ['me/me_pc_stubs.cpp', 'native/nfsmw_nativo_captura.cpp']
USE = re.compile(r'REXCVAR_(?:DECLARE\(\s*[\w:<>]+\s*,\s*|GET\(\s*|SET\(\s*)(\w+)')
DEFINE = re.compile(r'REXCVAR_DEFINE_\w+\(\s*(\w+)\s*,')


def files(root, built_only):
    for d, _, fs in os.walk(root):
        for f in fs:
            if not f.endswith(('.cpp', '.h', '.inc')):
                continue
            p = os.path.join(d, f)
            rel = os.path.relpath(p, root).replace('\\', '/')
            if built_only and any(rel.startswith(e) for e in EXCLUDED) and rel not in KEPT:
                continue
            yield p


def statements(text):
    """Every REXCVAR_DEFINE_... statement, to its terminating semicolon, with the name."""
    for m in DEFINE.finditer(text):
        start = m.start()
        depth, i = 0, start
        while i < len(text):
            c = text[i]
            if c == '"':
                i += 1
                while i < len(text) and text[i] != '"':
                    i += 2 if text[i] == '\\' else 1
            elif c == '(':
                depth += 1
            elif c == ')':
                depth -= 1
            elif c == ';' and depth == 0:
                break
            i += 1
        yield m.group(1), text[start:i + 1]


def main():
    banjo_src, port = sys.argv[1], sys.argv[2]
    src = os.path.join(port, 'app', 'src')
    used, defined = set(), set()
    for p in files(src, True):
        t = open(p, encoding='utf-8', errors='replace').read()
        used |= set(USE.findall(t))
        defined |= {n for n, _ in statements(t)}
    for p in files(os.path.join(port, 'sdk', 'src'), False):
        defined |= {n for n, _ in statements(open(p, encoding='utf-8', errors='replace').read())}
    missing = sorted(used - defined)
    banjo_defs = {}
    for p in files(banjo_src, False):
        for n, s in statements(open(p, encoding='utf-8', errors='replace').read()):
            banjo_defs.setdefault(n, (s, os.path.relpath(p, banjo_src).replace('\\', '/')))
    out = ['// keflings-nx (2026-10-30), written by tools/kef_missing_cvars.py: cvars the built sources name whose',
           '// banjo-nx definitions are in files this port does not build (its PM4 renderers, N&B\'s game hooks and',
           '// natives). Copied as banjo-nx defines them; the code they switched is not in this port.',
           '#include <rex/cvar.h>', '']
    unknown = []
    for n in missing:
        if n not in banjo_defs:
            unknown.append(n)
            continue
        s, rel = banjo_defs[n]
        out.append('// from banjo-nx app/src/%s' % rel)
        out.append(s)
        out.append('')
    dst = os.path.join(src, 'keflings', 'keflings_banjo_cvars.cpp')
    open(dst, 'w', encoding='utf-8', newline='\n').write('\n'.join(out))
    print('%d used, %d missing, %d written, not found in banjo-nx: %s' % (len(used), len(missing),
                                                                          len(missing) - len(unknown), unknown))


if __name__ == '__main__':
    main()
