# keflings-nx: report of 2026-10-30, 19:45

## Works

- **PC** (Vulkan, Renut): boot, menus, Choose your Giant, the first kingdom at 60 fps (frame_cap 60).
- **Emulator** (Ryujinx, `ryu-data\keflings`): NRO 4 boots to the kingdom at ~40 fps (r6), no errors; every verify
  key 0 differences in the verify runs (r2, r3).
- **Console** (the user): package a runs; all 3 cores at 99 %, ~30 fps.
- **Packages** in `D:\kingdomkeflings\packages\`:
  - `keflings-nx-2026-10-30b` (NRO 4, commit 4b9cd20e): **test this one first.** 233 localized copies, banjo-nx's
    three natives plus six Keflings natives, 246 promoted virtual calls.
  - `keflings-nx-2026-10-30c` (NRO 5, commit 882c9101): b + `elide_lr_stores` and `non_argument_as_local`. PC: 0
    differences, kingdom at 60 fps. Emulator r7: like b for 190 s, then 2-4 fps, at the moment another
    session's compile started on this PC: not confirmed either way.
  - `keflings-nx-2026-10-30a` (NRO 3): the first console test build.

## Measured

- CPU compare (PC): 487 M calls of 232 copies and the natives (package b), 349 M (package c): 0 differences.
- ARM profile (emulator r5, NRO 3): about a third of the main thread in the PPC register save/restore helpers
  (`__savegprlr_N` / `__restgprlr_N`), ~11 % in memcpy.

## Not done / open

- `non_volatile_as_local` (removes those save/restore helpers: the biggest ARM item left) hangs the game at boot on
  the PC (b15, b16). Audits ran (82 kept in ctx, 26 share_registers marks); the cause is not found. Leads in
  docs/keflings-notes.md, "Codegen options".
- The kingdom load counts ~24 M memset calls since the 14:31 regeneration (2.3 M before): not explained.
- PGO + LTO: waiting for the user's go (their decision at 14:55).
- A console profile (logs/rex/rex_profile.log from a console run) would show where the 99 % goes on the A57.

## Your decisions

- SMAA and dynamic resolution (configs/): picture changes, off by default.
- When to make the PGO + LTO build.
