#!/usr/bin/env python3
"""keflings-nx (2026-10-30): the package's other tomls, each renut.toml with the keys it lists changed (so two logs
compare directly), in the manner of banjo-nx's tools/make_enhancement_configs.py.

  python tools/kef_make_configs.py dist/switch/keflings-nx
"""
import os
import re
import sys

# The A/B targets that apply to Keflings' draws (all immediate, none recorded: docs/keflings-notes.md "Renderer
# census"), and target 1 (banjo_renut_precise_barriers, Vulkan only) as the control that does nothing on the Switch.
AB_TARGETS = '20,22,25,28,33,47,62,1'

CONFIGS = {
    'renut_play.toml': ('Play: every diagnostic and profiler off, the log at warnings.', {
        'log_level': '"warning"',
        'switch_profile_stacks': 'false',
        'banjo_stall_watch_seconds': '0',
        'renut_gpu_profile': '0.0',
        'banjo_renut_diag_gpu_shaders': 'false',
    }),
    'renut_ab.toml': ('In-run A/B of the exact items that apply to this game, 30 s halves; target 1 does nothing on '
                      'the Switch (the control). Stand still at one spot for 10+ minutes (several rounds).', {
        'banjo_renut_ab_targets': '"%s"' % AB_TARGETS,
        'banjo_renut_ab_s': '30',
    }),
    'renut_verify.toml': ('Emulator check: every self-check on (slow); each difference is logged.', {
        'banjo_cpu_verify': 'true',
        'banjo_deko3d_push_constants_verify': 'true',
        'banjo_deko3d_txq_verify': 'true',
        'banjo_renut_chain_reuse_verify': 'true',
        'banjo_renut_fast_draw_verify': 'true',
        'banjo_renut_fetch_cache_verify': 'true',
        'banjo_renut_persistent_shared_verify': 'true',
        'banjo_renut_resource_objects_verify': 'true',
        'banjo_renut_snapshot_deltas_verify': 'true',
        'banjo_renut_store_cards_verify': 'true',
        'banjo_renut_tags_check': 'true',
        'banjo_renut_whole_buffers_verify': 'true',
    }),
    'configs/keflings_smaa.toml': ('Picture change, your decision: SMAA 1x before the HUD (in place of MSAA: SMAA '
                                   'runs on single-sampled targets only).', {
        'banjo_renut_smaa': 'true',
        'keflings_renut_msaa_samples': '1',
    }),
    'configs/keflings_game_picture.toml': ('The game\'s own picture (pixel-identical): 1280x720 docked too, its own '
                                           '2x MSAA.', {
        'banjo_renut_render_height_docked': '720',
        'banjo_renut_display_height_docked': '720',
        'keflings_renut_msaa_samples': '2',
    }),
    'configs/keflings_dynamic_res.toml': ('Picture change, your decision: dynamic resolution, 540-720 handheld and '
                                          '720-1080 docked by the GPU time (banjo-nx\'s steps).', {
        'banjo_renut_dynamic_resolution': 'true',
        'banjo_renut_dynamic_handheld_low': '540',
        'banjo_renut_dynamic_handheld_high': '720',
        'banjo_renut_dynamic_docked_low': '720',
        'banjo_renut_dynamic_docked_high': '1080',
        'banjo_renut_dynamic_steps': '4',
        'banjo_renut_dynamic_gpu_ms': '15.5',
        'banjo_renut_dynamic_up_s': '1.0',
    }),
    'configs/keflings_all_off.toml': ('Both enhancement groups off: the original paths everywhere (for comparison).', {
        'banjo_cpu_enhancements': 'false',
        'banjo_noncpu_enhancements': 'false',
    }),
}


def main():
    d = sys.argv[1]
    base = open(os.path.join(d, 'renut.toml'), encoding='utf-8').read()
    for name, (what, keys) in CONFIGS.items():
        text = base
        added = []
        for k, v in keys.items():
            pat = re.compile(r'^%s\s*=.*$' % re.escape(k), re.M)
            if pat.search(text):
                text = pat.sub('%s = %s' % (k, v), text)
            else:
                added.append('%s = %s' % (k, v))
        head = '# %s: renut.toml with: %s\n# %s\n' % (os.path.basename(name), ', '.join(keys), what)
        if added:
            text = text.rstrip('\n') + '\n\n# Added by this config:\n' + '\n'.join(added) + '\n'
        p = os.path.join(d, name)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        open(p, 'w', encoding='utf-8', newline='\n').write(head + text)
        print('wrote', p)


if __name__ == '__main__':
    main()
