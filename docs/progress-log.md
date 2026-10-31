# keflings-nx progress log

Times are AEST (UTC+10), 2026-10-30.

- 10:18 Start. Read banjo-nx's porting-start-here, porting-guide-renderer (all), the enhancements table, the peggle2
  order of work, the XDK versions doc.
- 10:22 Extracted the game (`000D0000`, 67.5 MiB), both DLC packs and title update 2 with zumasrevenge-nx's
  stfs_extract.py. XEX header: every library XDK 2.0.7645 = Banjo N&B base's XDK.
- 10:25 Port folder made from banjo-nx's working tree (sdk, app without out/generated/assets, tools, shaders; banjo's
  docs in docs/banjo-nx). banjo-nx's renderer at its base-game commit be8476a3 kept aside for the address move.
- 10:30 Codegen g1: 2 unresolved thunk targets -> `config/renut_funcs.toml`. Rerun (g1b): 129 files, 66 MB, 11,879
  functions (Banjo base: 39,674).
- 10:35-10:55 Address move Banjo base -> Keflings (`tools/xdk_*.py`): exact hashes 5,081, then similarity, call-list
  alignment, tiny-function text, gap fill, relocation-exact (5,230; agrees with all 5,206 earlier pairs it overlaps),
  callee votes: 5,883 functions mapped. Of the renderer's 439 addresses, 60 have no Keflings counterpart (N&B game code;
  D3D functions Keflings does not link: command buffers, tessellated draws, many getters). 27 hook lines off.
  Data: format table, both D3D state tables (every getter/setter word checked), VdGlobalDevice slot, 7 more.
- 10:55 Renderer installed from banjo-nx's base commit be8476a3 (moved addresses), shared hooks made plain hooks (no
  PM4 renderer), N&B widescreen / photo hooks out. App trimmed to Renut + framework; banjo's per-game lists emptied.
- 11:00 post_codegen.py on Keflings' code (store cards, locals, inline leaves ...). First PC build started.
- 11:05 Ryujinx data folder `D:\New folder\ryu-data\keflings` from banjo's bis/system/profiles/Config.json (no sdcard).
- 11:07-11:33 PC build fixes: banjo's PM4 renderer's EDRAM shader step out; banjo HEAD's PC-build fixes taken
  (PLUME_DEKO3D blocks in draw.cpp / surfaces.cpp, the Windows `far` macro, the duplicate RepeatCensusReport) but none
  of its title-update-3 layout hunks; Keflings' XMemFree (0x821CD638, 372 aligned call sites) and GetCurrentThreadId
  (0x8245B398); 126 cvars of unbuilt banjo files generated (tools/kef_missing_cvars.py); reNut's frame limiter and FPS
  counter run from D3D's Swap (src/keflings/keflings_frame.cpp). renut.exe links at 11:33.
- 11:34 PC run 1: boots, loads Full_Init/Audio/Anims/Fonts blocks and the TU's Title_Update.blk, creates the D3D
  device, render thread starts, first shaders captured; stops at a call to guest 0 (lr 0x821EA140).
- 11:40 Cause: D3D's ring-space query (sub_821EA1A8, a function N&B does not link) on Renut's "no hardware" device
  calls a null interface callback. Hook in src/keflings/keflings_d3d_hooks.cpp (0 and 0, as for a device without
  ring or interface). Lesson: a new hook needs post_codegen.py again (direct_calls.py wires callers to __imp__).
- 11:48 PC run 4: 6 frames swapped, then a call to 0x820A94B8 (a vtable setter never registered). tools/
  kef_pointer_scan.py: 61 functions reached only by pointer, declared in config/renut_funcs.toml (fixpoint in 2
  passes; the import thunks are registered already). First commit f89cf56.
- 12:05 PC run 5: the renderer draws the game's UI correctly; the game stops at "Update Required" (avatars).
- 12:08-12:20 TU2 needs the avatar system in every variant tried (docs/keflings-notes.md "Why the base game").
- 12:20-12:34 Switched to the base game 0.0.1.2: codegen b1 (one thunk target), pointer fixpoint (65), TU2 -> base
  map and source move (661 function / 10 data addresses). PC run b1: menus and gameplay, 59 fps. Commit 0db97de.
- 12:38-12:45 Shader containers from the data blocks (21, Full_Init.blk; 38 shaders in all). Reference run on the
  SDK's emulated GPU: the giant, Keflings and buildings missing on Renut (shadows drawn).
- 12:45-13:17 Diagnosis by draw log (alpha test, cull, EQUAL depth ruled out); the models' declarations lack the
  COLOR0 their shader reads; missing inputs now read (0,0,0,1). Models render as on the reference. Commit cd71f40.
- 13:20 Switch shader package: 38 of 38 shaders through the deko3d chain (GLSL package 97 KB). Switch build started.
- 13:25 PC profile (100 s of the first kingdom, sticks wandering): hottest game functions sub_820A85E8 (7.6 % of the
  main thread), sub_820B2998, sub_820A70C8, sub_820B2160, sub_821741B8; 107 game functions with samples.
- 13:30 Console tomls (renut, play, ab, verify, SMAA and dynamic-resolution configs), forwarder NPDM (program id
  05D6EA1333488000), Ryujinx SD card with game_root.
- 13:35-13:45 Emulator run r1 (Switch NRO 1, verify config, 6 min): boots, logo, menus, Choose your Giant, no crash,
  20-25 fps in Ryujinx, ~600 draws a frame, 100 % fast path. Giant preview and town buildings missing on deko3d
  only: Renut's missing-input slot 16 is not bound by plume's deko3d back end -> fixed attributes (0,0,0,1).
- 13:45-14:05 Hot functions: 91 localized copies (top 5 + 83 bulk) and banjo-nx's three hand natives of XDK code
  Keflings links unchanged (memcpy, memset, D3D state flush). PC compare mode: 88.5 million calls, 0 differences.
- 14:09 Switch NRO 2 (copies, natives, deko3d missing-input fix). Emulator run r2 waited for SASRT's Ryujinx run.
- 14:20 Keflings hand native sub_820A85E8 (font glyph width; key keflings_hand_glyph_width), in its hook and inlined in
  the text layout's copy. Emulator run r2 started 14:20 (NRO 2).
- 14:28 r2: giant preview and buildings on deko3d, first kingdom reached, no crash; CPU compare 49.2 M calls, 0
  differences; no fast-draw verify warning with async variants off.
- 14:30 The open crash traced: sub_821AB360 (a screen constructor) re-reads the manager's current screen unchecked.
  Mid-asm hook at 0x821AB438 (null -> the function's return), key keflings_fix_no_current_screen. Also found: the
  game renders into a line texture, resolves it and locks it on the CPU (sub_82119EA8, a vtable method): SASRT's
  resolve read-back case; to check what it is for. PC build b9 (codegen with the hook, glyph native) started 14:31.
- 14:38 b9 stopped: "renut_pch.h has been modified since the precompiled header was built" (the build's own codegen
  step rewrote it after ninja had made the PCH). A second build (b9b) remakes the PCH. Lesson: after a codegen
  inside the build, expect one PCH failure; just build again.
- The read-back (sub_82119EA8) belongs to an image class whose other methods turn the CPU copy back into a texture
  and send it with xlivebase message 0x00058035 (an Xbox LIVE call: not handled by the SDK, never logged in any run
  so far). To confirm with a one-shot log of its calls.
- 14:39 PC pc_verify2: glyph native exact in its hook, but the text layout's copy differed (the tool's inline form
  did not load r3 back after a native). Tool fixed; 14:46 pc_verify3: 90 copies on, 82.9 M calls, 0 differences.
  Neither the image capture's log nor the screen guard's appeared in 150 s of menus and the first kingdom.
- 14:48 Commit f9c769ae. PGO instrumented build waits for SASRT's Ryujinx run (started 14:46).
- 14:55 User: skip PGO + LTO for now; they will say when (final build, after more optimising and console tests).
  Packages come from app\out\switch until then.
- 14:52-14:55 PC indirect-call census (160 s): empty. tools/indirect_calls.py had tagged 0 of 11,554 `bctrl` sites:
  it wants the `// bctrl` comment right above the call, and this tree keeps the link-register store between them
  (`elide_lr_stores = false`; banjo-nx elides it). Tool fixed (that line and r1_local's marker allowed); dry run:
  11,554 sites. PC build b12 re-runs codegen to tag them (15:00).
- 15:06-15:09 Census 2 (160 s, boot to the first kingdom): 3,511 sites, 500 M indirect calls; 3,313 sites (87.6 %
  of the calls) always go to one target. app/indirect_promotions.txt: 246 sites (15 with two or three targets, 10
  with a dominant one), whose guards catch 96.6 % of the calls. PC build b13 applies them (15:10).
- 15:18 PC pc_verify4: 175 M promoted calls checked against the dispatch table, CPU compare 125 M calls: 0
  differences. Commit 1d2dd65b. Switch build 3 waited for katamari's Ryujinx run, then 15:23-15:26 (NRO 3, 31 MB).
- 15:28-16:24 Emulator run r3 waited: free commit stayed under the 11.5 GB the script asked for (other ports' PC
  games and Ryujinx runs). User (16:25): 8 GB free commit is enough; run_emulator.ps1 now asks for 8.
- 16:30 Package keflings-nx-2026-10-30a from NRO 3 (D:\kingdomkeflings\packages\): NRO, tomls, configs, NOTES-test,
  four-core NPDM, ELF + map + CMakeCache, source snapshot of commit ccb4bb0c (no uncommitted changes).
- 16:30-16:37 Emulator run r3 (NRO 3, verify config): boot, menus, Choose your Giant, intro, then "Loading..." from
  ~258 s to the end (420 s), the screen animating. CPU compare 68.4 M calls, 0 differences; no errors. The loading
  is not a loop: the memset hook's count climbed 14.2 M -> 37.8 M of the ~38.2 M the PC's loads reach (PC loads
  of the same code: ~20 s). Open question, not chased (user: test on the console): since the 14:31 regeneration
  the PC's and the emulator's loads count ~24 M memset calls where the morning's code and r2 counted ~2.3 M; not
  the glyph native (PC run with it off: the same 38.2 M). Possibly only where the calls are counted (through
  memset's hook rather than inlined in a caller's copy), which also makes verify runs slower there.
- 16:50-18:35 Console (user): all 3 cores at 99 %, ~30 fps. User: 5 more hand natives and 200+ machine-optimised
  functions, from the emulator's (ARM) profile. Emulator run r5 (18:09, the shipped toml, 600 s) -> 142 more bulk
  functions (220 bulk copies, 233 in all); natives of sub_8241F3F8 (wcscmp), sub_820A89B0 (glyph index),
  sub_82167928, sub_820B7CD8, sub_820C4DE0 (their functions moved to localized.txt: the bulk list skips hand
  natives). PC pc_verify5: 232 copies, 487 M calls, 0 differences. NRO 4, package keflings-nx-2026-10-30b; r6:
  boots to the kingdom, ~40 fps in the emulator, no errors.
- 18:40 User: all of banjo-nx's codegen options. The ARM profile's third of the main thread in __savegprlr /
  __restgprlr is what non_volatile_as_local removes. Audits done (notes, "Codegen options"); PC build b15 queued
  behind another port's Ryujinx run.
- 16:45 User: time to test on the Switch. Queued emulator A/B run r4 cancelled (the A/B is renut_ab.toml on the
  console). NOTES-test.txt in the package: how long the load takes on the console.
- 19:03-19:16 PC b15 (all three codegen options) and b16 (non_volatile_as_local alone) hang at boot after the D3D
  device is made. non_volatile_as_local off again (marks commented out); notes list where to look next.
- 19:25 PC b17 (elide_lr_stores + non_argument_as_local): boots to the kingdom at 60 fps, CPU compare 349 M calls 0
  differences, lr mirror 0 differences. Switch build 5 + emulator check r7 (19:30).
- 19:10 The "XDK versions of our Xbox 360 ports" doc: Beautiful Katamari (2.0.5632, QFE 0, kernel imports 2.0.5766)
  and A Kingdom for Keflings (2.0.7645, QFE 1; TU 0.0.3.2 has D3D 2.0.7661) added (the user's request).
- 19:31-19:37 Switch build 5 (NRO 5, 31.6 MB) and emulator r7: kingdom by ~90 s, 39-60 fps as r6 until ~190 s, then
  2-4 fps to the end. Another session's compile started at 19:35:11 (clang++, ninja since 19:23), the moment of the
  drop; not repeated cleanly (no time). Package keflings-nx-2026-10-30c from NRO 5, marked unconfirmed; package b
  stays the one to test first.
- 20:00-20:40 DLC check (user): both Kingdom Packs install and the game opens and reads them at boot, but drops
  them: their maps (map-select screen, DLC world terrain and init data) are in title update 2's Title_Update.blk,
  not in the base game. DLC needs TU2 (avatar system). Traced with two temporary mid-asm logs (removed).
- 20:45-21:35 Title update 2 (user): a worktree of this morning's TU2 commit (D:\kingdomkeflings\keflings-tu2, its own
  game folder game_tu2) reproduces run 8's crash (lr 0x8218F864) when the new avatar giant "You" is chosen; with a
  normal giant it plays into the kingdom. Mid-asm hook in the giant lookup (0x82158F74): the avatar giant is never
  made. Run r8: no crash, Choose your Giant opens on Grant, into the kingdom.
- 21:35-21:50 Main tree moved to TU2 (tools/kef_base_to_tu2.py), all natives and copies carried over (user); TU2 keep
  list (84, HANDOVER 0); PC build b19.
- 21:45-21:56 PC b19 (TU2): 227 copies and all nine natives compared, 266 M calls, 0 differences; Downloaded Content
  lists Kingdom Pack 1 and 2; Choose your Kingdom: The Classic Kingdom, The Orchard Kingdom, The Kingdom Crossroads;
  The Kingdom Crossroads plays. TU2's 11 shader containers are all in the Switch shader set already. Switch build 6
  (NRO 6, TU2); emulator SD: default.xexp in game_root, update/, dlc/. Emulator run r8 (21:57).
- 21:57-22:24 Switch (TU2) in the emulator: r8-r11 aborted ~37 s in (a call to address 0 from sub_8216E830: an object of
  kind "None"); the trap now writes sdmc:/switch/rex_invalid_call.txt; guard keflings_fix_null_object (0x8216E8E4).
  r12: no crash in 420 s, but the Single Player menu no longer advances on A. The PC (TU2) never makes a kind-0
  object: something Switch-only; r13 with banjo_cpu_enhancements = false (queued behind another port's run).
- 22:45-23:30 The Switch stall traced to a new profile (PC reproduces it with the saved settings set aside): the
  front end's giant is the avatar giant by default. First-start settings with Grant (keflings_seed_title_settings).
  PC fresh profile: into the kingdom. Switch build 12, emulator r15 (fresh profile): kingdom by ~60 s, 7 minutes, no
  crash, 35-38 fps in the emulator. Package keflings-nx-2026-10-30d (title update 2).

## 2026-10-31 (package e)

- 00:00 The user's console run of package d (logs in SD_CARD_keflings-nx): 29-43 fps in the kingdom; main thread 92-97 %
  of a core (it moves between cores 0-2, so no core reads 100 % in the status monitor), render thread 4.4 ms a frame,
  GPU 6.3 ms. The main thread's samples are the game's own work (copies 66 %, other generated code 17 %, the renderer
  7 %, save / restore helpers 3 %): no spin wait. Full table: docs/console-profile-2026-10-30.md.
- Peggle 1's SDK exit fix (Return to Arcade crashed: the runtime is now reset before the window), Bejeweled Blitz's
  frame-limiter fix after a late frame, Blitz's gamer-picture source (sdk gamer_picture.h; the game's achievement
  tiles, type 0, are the console user's picture: keflings_user_picture_user), 1080p docked, the MSAA level key (the
  game draws with 2x MSAA itself: 2 is exact), four-core mode removed (no NPDM), 117 more bulk copies (console
  profile, --min-samples 1), seven hand natives (pair test, sphere and box merges, scalar and VMX matrix products,
  track step, box against frustum).
- e_verify1: the six natives with stack writes differed at their first call (the guard compares every byte written,
  stack saves included); they now write the original's stack bytes. e_verify2 (160 s, kingdom): 343 copies + 326 of
  327 bulk on, 258.5 M calls compared, 0 differences; each new native 0.44-9.97 M calls compared, 0 differences.
- e_trace1 / e_trace2: the 1280 x 736 scene targets are 2x MSAA with the key at 1 and at 2 (the game's own);
  e_msaa4 / e_msaa1: x4 and x1 made, 60 fps, picture right.
- 09:00-10:00 keflings-nx 1.0.0 (the user's requests): renut.toml holds only the player's options (language, input,
  MSAA, the player icon); everything else is compiled in (app/keflings_switch_defaults.toml, BuiltInConfig, from
  sa2-nx / bejeweledblitz-nx), every log off (log_to_file, lazy logs/ folder, switch_profile_report, deko3d.log
  gated). keflings_language (8 languages; XGetLanguage honours user_language), input_joycon_sideways_stick, the NRO
  icon (the user's picture, not in git), version 1.0.0, author ChanseyIsTheBest.
- PGO: instrumented build (app/out/switch-pgo-gen), Ryujinx training with logging off. The first session's toml had
  its input_script below [input_p1] (TOML put it in that table: no presses, a title-screen profile, kept in
  out/pgo_profile_title_only_20261031); the second, the script first, played into the kingdom (sum_max 2.05e9).
  PGO + LTO release build (app/out/switch-pgo-use) as the distribution build: the 17 shaders the game makes at run
  time in the NRO (out/kef_runtime_shaders), the 21 of Full_Init.blk from the installer's deko3d_glsl.bin.
- keflings-nx-installer (from nights-nx-installer; the Kingdom Packs to dlc/, the update's files to update/):
  packages test 39 files byte for byte; shader build 21 shaders, all with the native numbers, 19 of 21 GLSL the native
  bytes, 66 variants, 0 failed. Ryujinx with the release NRO and the installer's files: installed package + the NRO's
  17, uam 0 compiles, 38.5-39.4 fps in the kingdom, render thread 3.5 ms a frame (4.2 before PGO + LTO).
