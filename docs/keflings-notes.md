# A Kingdom for Keflings: port notes (keflings-nx)

Game-specific facts, fixes and decisions of the keflings-nx port (PC, then the Switch), started 2026-10-30 from
banjo-nx (`D:\Banjo kazooie port\banjo-nx`, read-only; its docs are copied in `docs/banjo-nx/`). The method and the
renderer are banjo-nx's; this page is what Keflings needed on top. The running log is `docs/progress-log.md`.

Credits: reNut's renderer (masterspike52; its author permits reuse in the user's ports), sa2-nx (input, CPU items),
NetherSX2_nx (four-core layout, NaGaa95, MIT), battd_nx (the GPU region), bejeweled3-nx and the zuma ports (the
hot-function method), banjo-nx (everything this port starts from). GPL-3.0 where the sources are. Game data is never
committed, uploaded or distributed.

## The game

| | |
|---|---|
| Title | A Kingdom for Keflings (XBLA), NinjaBee |
| Title ID | 584108DB, media 5944FF33 |
| Executable | `default.xex` 0.0.1.2 (4,890,624 bytes), image 0x82000000 + 0x500000, entry 0x821B0930 |
| Title update | TU 2 (`default.xexp`, 557,056 bytes): 0.0.1.2 -> 0.0.3.2, image 0x82000000 + 0x560000 |
| XDK | 2.0.7645 for every static library (D3D9 2.0.7645.1, XAPILIB, XAUD, X3DAUD, XMP, XHV, XONLINE, ...) |
| DLC | Kingdom Pack 1 and 2 (content type 2, each a 416-byte `blockfiles/map_pack_N.blk` + `pack.text`) |

**XDK 2.0.7645 is the XDK of Banjo-Kazooie: Nuts & Bolts' base game** (banjo-nx's XDK table: base 2.0.7645, its
title update 3 moved to 2.0.7978). So Keflings' D3D, XAudio and CRT library code is Banjo base's code at other
addresses: banjo-nx's renderer hooks are taken from its last base-game commit (`be8476a3`, before title update 3) and
moved to Keflings' addresses by content (section "Addresses").

## Files (this PC)

| What | Where |
|---|---|
| Packages as they came | `D:\kingdomkeflings\A Kingdomm for Keflings\...` (game `000D0000`, DLC `00000002`, `Title Update 2`) |
| Game, extracted | `D:\kingdomkeflings\game` (`default.xex`, `default.xexp` from the TU, `fsxb2\blockfiles\*.blk[.bz2]`) |
| Title update, extracted | `D:\kingdomkeflings\tu2` (`default.xexp`, `fsxb2\blockfiles\Title_Update.blk.bz2` and three font blocks) |
| DLC, extracted | `D:\kingdomkeflings\dlc\pack1`, `pack2` |
| Port | `D:\kingdomkeflings\keflings-nx` (git, local commits only); `app\assets` is a junction to the game folder |
| Ryujinx data | `D:\New folder\ryu-data\keflings` |

Extraction: `python D:\peggle 2\zumasrevenge-nx\tools\stfs_extract.py <package> <out>` (STFS LIVE, read-only).
The "(World) (v2)" folder's TU is the same update (same file list and sizes, another signature).

## Why the base game (decision, 2026-10-30 12:30; superseded the same evening: see "Title update 2")

keflings-nx is recompiled from the **base game 0.0.1.2**, not from title update 2 (the first codegen, g1, was TU2's):

- TU2 adds its own avatar start-up (`sub_8243ECD8` in TU2: looks up 19 XAM avatar exports by ordinal, 1500-1521, and
  checks every one). With the SDK's avatar stubs (XamAvatarInitialize fails) it shows "Update Required: a system
  update is required to use this feature" and exits to the dashboard after OK (PC runs 5-7).
- With the exports made unresolvable (`xam_avatar_exports = false`, as on a dashboard without avatars) it does the
  same (run 9).
- With the avatar system made to initialise and every query failing cleanly (`xam_avatars = true`) it reaches the
  main menu, then calls a null avatar object when Single Player is chosen (run 8): it needs real avatar data (the
  dashboard's avatar assets), which the runtime cannot provide.
- The base game links the XDK's avatar library too, but has none of TU2's own avatar code, and runs (PC run b1:
  menus and gameplay).

What is lost: TU2's fixes and the two Kingdom Pack DLC (416-byte unlock files that TU2 reads). The three SDK keys
(`xam_system_version`, `xam_avatars`, `xam_avatar_exports`) stay, defaults keeping the SDK's behaviour, for a later
attempt at TU2. Every renderer and port address was moved TU2 -> base by content (`tools/kef_tu2_to_base.py`: 661
function and 10 data addresses; the TU2 -> base map agrees with all 8,820 relocation-exact pairs it overlaps).

## Decisions

- (Superseded at 12:30, see above: the base game.) The port was first built from title update 2 (as banjo-nx is from
  its update): the generator applies the sibling `default.xexp` itself.
- The build's internal names stay banjo-nx's (`renut`, `renut_recomp`, the `banjo_*` keys), so its CMake, tools and
  post-codegen passes work unchanged; the user-facing names (NRO, SD folder) are `keflings-nx`.
- First codegen with the safe options only: `non_volatile_as_local`, `elide_lr_stores` and `non_argument_as_local`
  need this game's audits (read_before_write.py, non_argument_audit.py) and are off until those have run.

## Codegen

- g1 (2026-10-30): 2 "target not in any function": 0x82400B78 and 0x82408280, both reached only through C++ adjustor
  thunks (`addi r3,r3,+-4; b target`); added to `config/renut_funcs.toml` with that evidence.

## Renderer census (porting-guide-renderer.md section 2, "What your game decides")

| # | Question | Keflings | How it was found |
|---|---|---|---|
| 1 | Where draws come from | **direct calls only**: the game links no D3D command-buffer code (BeginCommandBuffer, EndCommandBuffer, RunCommandBuffer, SetCommandBufferPredication and the tessellated draws have no counterpart: every candidate is another Draw* already matched). Draws go through Renut's queued path (`QueueDraw`), not the record-time path (`RecordDraw`) | function map (static); PC run to confirm |
| 2 | Where resources come from | a small set: 2 render targets (22 MB) and 123 textures (45 MB) in the first kingdom; stream cache 14 MB (8,845 streams) | PC memory report |
| 3 | How state is set | the D3D setters through the state tables (both found, every getter and setter matched) | static |
| 4 | Memexport | none seen (memexport readback 0 MB; no export draw in the frame log) | PC memory report, draw log |
| 5 | Shadow passes | **stencil shadow volumes**, not shadow maps: ~80 draws a frame with no pixel shader and no colour writes (VS B9C41E7DC0BB8500, stencil on) | draw log |
| 6 | Threads | PC: the main thread carries the game (26 % of busy samples); the render thread waits 72 % of its time. Console: pending a rex_profile.log | PC profile |
| - | Draws a frame | 300-600 in the first kingdom (PC 9-12 k draws a second at 60 fps); 100 % on Renut's fast path on deko3d (emulator) | logs |

Consequence: banjo-nx's record-time items (porting guide 4.1 setter marks into ops, 4.5-4.7 record-time objects,
the replay stream) have no work to do in this game; the items of the draw itself (fast path, pipelines by content,
pushed uniforms, whole-buffer copies with store cards) apply to queued draws too.

## Switch (deko3d)

- Shader package: `tools/switch_shaders.bat` (the PC build's XenosRecomp in deko3d mode with the PC's early-alpha HLSL,
  then renut_switch_shader_cache.py, deko3d_shaders.py, deko3d_glsl_opt.py, deko3d_shader_package.py): 38 of 38
  shaders, GLSL package 97 KB. The containers: 21 from `Full_Init.blk` (`tools/kef_shader_scan.py`; the other blocks
  hold none) and the rest from run-time captures.
- NVK not linked (`REXGLUE_SWITCH_NVK` OFF): Renut on deko3d only. NRO 30 MB.
- Ryujinx run r1 (6 min, verify keys on): boots, NinjaBee logo, menus, Choose your Giant; no crash; 20-25 fps in the
  emulator. The giant preview and the town's buildings were missing on deko3d alone: Renut's missing-input slot is 16,
  plume's deko3d back end binds only 0-15 and still pointed the attributes at buffer 16. Fix: fixed attributes,
  (0,0,0,1) (`Deko3DOptions::missingInputsW1`, from `keflings_renut_missing_input_w1`).
- Run r1's verify warnings: two `banjo_renut_fast_draw_verify: the kept or cached pipeline differs from the one made`
  for VS AB0BA7170E2F17B5, with equal keys (only the pipeline object differed): checked with async variants off.
- Ryujinx run r2 (NRO 2, 14:20, 7 min, verify config with `banjo_deko3d_async_variants = false`, A every 8 s):
  boot, menus, Choose your Giant with the giant preview and the town's buildings (the fix works), into the first
  kingdom (tutorial, Basic Workshop): giant, Keflings, buildings, trees and shadows as on PC. No crash. Verify keys:
  CPU compare 49.2 million calls of 90 copies (all 83 bulk copies reached), **0 differences** (2 differed once and
  not when repeated: inputs changed by another thread); snapshot deltas, push constants, store cards, fetch cache,
  resource objects: 0 differ; **no fast-draw verify warning** with async variants off (r1's two came from the async
  variants: the kept pipeline object is the asynchronously made variant, its key equal). Fast path 100 % (30,852 of
  30,852 draws in 10 s); ~840 draws a frame in the kingdom; 3.3-4 fps in the emulator with every self-check on
  (r1's menus 20-25 fps: verify runs every hot call twice).
- `banjo_deko3d_async_variants` stays ON in the shipped toml (banjo-nx ships it as exact; r1's two warnings had equal
  keys, i.e. the same state, a different pipeline object). Not proven here beyond that: to look at on the console
  (the user's renut_verify.toml sets it as the shipped toml does).

## Hot functions

- PC profile (`banjo_pc_profile_s = 20`, 100 s of the first kingdom, sticks wandering): 107 game functions with
  samples. Top five (main thread): sub_820A85E8 7.6 %, sub_820B2998 4.1 %, sub_820A70C8 3.5 %, sub_820B2160 2.9 %,
  sub_821741B8 2.8 % (PC-only: `fmal` 4.9 %, the x86-64-v2 build's FMA library call; one instruction on the A57).
- Machine-optimised (optimising-functions.md): `localized.txt` the top five, `localized_bulk.txt` 88 more (83 copies;
  5 refused, tiny leaves and app-named addresses left out): 91 localized copies with changed-only write-backs, small
  callees inlined, direct dispatch where the copy can call a copy.
- Hand-written natives: banjo-nx's natives of XDK code that Keflings links unchanged (relocation-exact): the C runtime
  memcpy (sub_824212D0) and memset (sub_824218A0), and D3D's state flush of dirty register words (sub_821D7C30).
- Keflings' own hand native (2026-10-30 14:20, key `keflings_hand_glyph_width`): **sub_820A85E8 is a font's advance
  width of one character** (r3 = font, r4 = character code; width = the font's fixed width at +26 when +32 is 0, else
  0 for codes 150-152, else the glyph entry's width found by a linear search of the u16 code table at +332; result
  `int(scale * size * width + 0.5f)`). The text layout (sub_820A70C8, number 3 of the profile) calls it for every
  character it measures. The native compares the code table in guest byte order (one load and compare per entry
  instead of load, swap, compare), keeps the generated float expressions (bit-identical) and writes back r3 alone
  instead of 14 registers; it stands in front of the copy in its own hook and inside sub_820A70C8's copy.
- The other four of the top five get no hand native, each for a reason:
  * sub_820B2998 and sub_820B2160 are virtual-call dispatchers (a loop over 8-byte entries calling vtable slot 540,
    and a jump through slot 452 of an object picked by a 292 index): what they cost is the indirect call itself, which
    a native cannot make (a native calls nothing; the guard's run stops at a call it cannot repeat). The lever there is
    an indirect-call promotion once a console profile names the targets.
  * sub_820A70C8 (text layout, 278 lines in its copy) already gets the glyph native inlined in its copy.
  * sub_821741B8 (253 lines, 12 real calls) is mostly calls; its localized copy is what can be done without the
    console profile that would show where its own time goes.
- PC compare mode (`banjo_cpu_verify = true`, 130 s): every copy reached, 88.5 million calls compared with the
  generated originals (memcpy 25.8 M, memset 14.3 M), **0 differences**.
- With the glyph native (PC run pc_verify2, 14:39): the native's own hook 5.9 M calls, 0 differences, but the text
  layout's copy (sub_820A70C8) and its caller's bulk copy (sub_820A6E20) differed on their first call (f1 = a huge
  width sum instead of 128.0) and were switched off by their guards. Cause: tools/localize_native.py's inline form
  of a hand native never loaded the `exact=` registers back into the caller's locals (banjo-nx's natives leave theirs
  as they came, so it never mattered); this native returns r3. Fixed in the tool (the exact registers the caller
  holds are loaded back after the native; for banjo-nx's natives that is the identity). PC run pc_verify3 (14:46,
  150 s, A every 8 s, sticks): all 90 copies on, 82.9 M calls compared, **0 differences** (glyph native 3.1 M,
  text layout 310 k). The guards did their job: this is why every native is compared before it ships.

## Indirect-call promotions (banjo-nx's codegen enhancement, sa2-nx's method)

- PC census (`banjo_diag_indirect_census`, logs/indirect_census_2.txt): 160 s of boot, menus, Choose your Giant and
  the first kingdom: 3,511 `bctrl` sites ran, 500 M indirect calls; 3,313 sites (87.6 % of the calls) always went to
  one target. The two busiest sites (sub_8229BA98 site 0 and sub_8229C518 site 1, 6.5 % of all calls each) and the
  top-5 dispatcher sub_820B2998's virtual call (2.2 %) are among them.
- `app/indirect_promotions.txt` (tools/indirect_promotions.py, default thresholds): 246 sites, 15 with two or three
  targets, 10 with one dominant target; the guards catch 96.6 % of the census's calls. Each guard compares the
  target, so a target the census did not see still takes the indirect call.
- PC check (pc_verify4, 150 s): 175 M promoted calls checked against the dispatch table, 0 differences; CPU compare
  125 M calls, 0 differences. On the Switch a promoted call is a compare and a direct `bl` (GCC can inline it)
  instead of the table load, null check and `blr`.
- sub_820B2160 (top-5 number 4) is a tail call (`bctr`), which the tool does not promote.

## Codegen options (banjo-nx's full set, 2026-10-30 18:45, the user's request)

- Why: the ARM profile of emulator run r5 (NRO 3, shipped toml) put about a third of the main thread in the PPC
  ABI's register save/restore helpers (`__savegprlr_N` / `__restgprlr_N`: each register one byte-swapped 8-byte
  store or load to the guest stack, on nearly every non-leaf function's entry and exit). `non_volatile_as_local`
  keeps r14-r31 / f14-f31 / v14+ as C++ locals instead; `elide_lr_stores` drops the link-register stores;
  `non_argument_as_local` keeps r0, r2, r11, r12, f0, v32-v63 local across calls. These were the only codegen
  options of banjo-nx not on here (all others already matched).
- Audits (banjo-nx's procedure), on raw generator output (tools/codegen.bat, no post-codegen):
  * tools/non_argument_audit.py without the option: ENTRY 262 (the save/restore helpers, kept by name, and the
    functions reading r12 / r11 on entry), RET 35 (callers reading r11 after sub_822A9B10). Every sub_ of both
    lists (82) in `non_argument_as_local_keep`. With the option (`--with-option`): **HANDOVER 0**.
  * tools/read_before_write.py with non_volatile_as_local: 32 functions read a non-volatile before writing it on
    some path. 26 marked `share_registers` (config/renut_funcs.toml; most in 0x822C0000-0x8236FFFF, a module
    passing values in r14-r31, several of them also r11-after-sub_822A9B10 readers). Left unmarked: 4 whose only
    such reads are cr fields (mfcr saves; cr_as_local was on in every build that ran) and sub_82434F00 /
    sub_82435290 (`xor r30,r30,r30`, zeroing). After marking, only those 6 remain.
- What checks it: the PC compare mode does NOT (copies and originals come from the same codegen). The PC build's
  own self-checks (the lr and r1 mirrors, "0 link-register write-backs ... 0 differences") and plain play: boot,
  menus, the kingdom, on PC and in the emulator, are the test.
- **Result (19:03-19:16): `non_volatile_as_local` hangs the game at boot.** PC b15 (all three on) and b16
  (`non_volatile_as_local` alone, the 26 marks in) both stop right after Renut's "automatic front buffer" line
  (the D3D device is made): no frame, no error, the log just ends; one memcpy call. So it is that option, or the
  marks with it. Not found tonight; candidates for the next session: (1) a marked function that is not really a
  split piece (banjo-nx's note: marking a normal function share_registers clobbers its callers' registers through
  the copy-back), so try the option with the marks one by one; (2) a function the audit cannot see into
  (hooks, mid-asm hooks, the renderer's guest callbacks) that reads a non-volatile its caller set; (3) the four
  cr-only functions left unmarked (sub_82422BC0 looks like a setjmp: mfcr into r4). A hang (not a crash) points at
  a wait whose condition lives in a register: a thread loop or a spin on a flag.
- `non_volatile_as_local` is OFF again (the 26 marks commented out in config/renut_funcs.toml, kept for that work).
- **`elide_lr_stores` and `non_argument_as_local` are ON** (b17, 19:25): PC boot to the first kingdom at 60 fps; CPU
  compare 349 M calls, 0 differences; the link-register mirror check after 784 M calls, 0 differences. Switch
  build 5 / package c carry them.

## Title update 2 (2026-10-30 evening, the user's request: "get update 2 working")

keflings-nx is title update 2 (0.0.3.2) again, with everything made for the base game carried over:

- **The avatar wall, solved.** With `xam_avatars = true` (XamAvatarInitialize succeeds, every avatar query fails
  cleanly) the update reaches the menus (this morning's run 8). Its one crash came from the new giant "avatars"
  ("You": the player's avatar as the giant, `STR_YOU` / `STR_CUSTOM_DESC`), made in code by the giant lookup
  `sub_82158E80`: with no avatar data it has no model, and choosing it calls a null object (lr 0x8218F864). A mid-asm
  hook at 0x82158F74 (just after the lookup's comparison with "avatars") goes to the lookup's exit, so the avatar
  giant is never made or registered: Choose your Giant offers the base game's four giants, and the game plays into
  the first kingdom (TU2 checkout run r8: no crash). Key `keflings_avatar_giant` (off).
- **Addresses moved base -> TU2** by `tools/kef_base_to_tu2.py` (the inverted out/tu2_to_base5.json): 1,586 function
  and 10 data addresses in 30 files (renderer hooks, Keflings' hooks, natives, hot lists, promotions), plus the six
  `__imp__sub_` forms the first pass missed. Hand natives: all nine functions are exact matches (same code), so the
  natives carry over as written; the top list keeps 12 of 13 (sub_821741B8 has no counterpart) and the bulk list 225
  of 230. The screen-constructor guard moved to sub_821C99A0 (+0xD8 / +0x110, the same instructions).
- **Lists for TU2**: indirect promotions 166 of 246 kept (exact functions and mapped targets only); the
  non_argument_as_local keep list made again from TU2's code (84; `--with-option`: HANDOVER 0); config/renut_funcs.toml
  is TU2's (this morning's g1 and pointer scan).
- **On the Switch build only** (emulator runs r8-r11, ~37 s after boot): an abort in `InvalidFunctionTrap` from
  sub_8216E830, a call to address 0 with r3 = 0. sub_8216E830 makes a game object through the factory sub_8217CDC0
  (kinds 0-6; kind 0 "None" logs "Create an object of type 'None'???" and returns null) and calls its vtable
  unchecked; its caller sub_82199540 (the kind is a byte of a record) already handles a null result. Mid-asm hook at
  0x8216E8E4: a null object goes to the function's return (it returns null), key `keflings_fix_null_object`. The PC
  runs never hit this path. Finding the address needed the trap to write it to a file first
  (`sdmc:/switch/rex_invalid_call.txt`; on the Switch the abort came before the log line was written).
- **The stall after the guard: a new profile's default giant.** r12 (no crash) stopped at the Single Player menu:
  A no longer led anywhere. Not Switch-specific: the PC does the same with a new profile (its runs had used
  settings saved by the run that chose Grant). Logging sub_8216E830's objects on both showed the first difference
  at object #40: the front end's giant, kind 1 / type 10 with Grant saved, kind 0 / type 0 on a new profile. A new
  profile has no title-specific settings (XPROFILE_TITLE_SPECIFIC1, 0x63E83FFF, the SDK's file
  <user data>/584108DB/profile/User/63E83FFF), and title update 2's default giant is the avatar giant, which is never
  made here. Fix: `src/keflings/keflings_title_settings.cpp` writes the game's own 1,000-byte settings with Grant
  (giant 10, as the game wrote them at 21:21) when the file is not there; the game then reads and writes them as
  usual. Key `keflings_seed_title_settings`. PC with a new profile: into the kingdom with Grant.
- Build note: only the PC build re-runs the code generator when a hook toml changes; the Switch build then compiles
  the regenerated text. After a mid-asm hook change, build PC first.
- Game data: `D:\kingdomkeflings\game_tu2` (default.xex, default.xexp, fsxb2 -> game\fsxb2) and the update folder
  `D:\kingdomkeflings\tu2`; `title_update.cpp` is the TU2 version again (needs default.xexp; mounts the update
  folder: `update` next to the NRO / exe or the game folder, or `tu2` next to the game folder).

## DLC (Kingdom Pack 1 and 2), checked 2026-10-30 20:00-20:40: needs title update 2

- Install: works. The two LIVE packages (`584108DB\00000002\9A7C...`, `B2BF...`, 61,440 bytes each) go in the
  `dlc` folder next to the exe / NRO (renut_dlc_folder); reNut's installer extracts them into the user data folder
  (`<user data>\0000000000000000\584108DB\00000002\<package>\pack.text`, `blockfiles\map_pack_N.blk`).
- The game's "Proccess of Extra Content" (sub_82103A88) enumerates both at boot, opens each (XamContentCreate,
  result 0), reads `content:\pack.text` (two lines: the pack's block file and its script) and opens
  `content:\blockfiles\map_pack_N.blk` (traced with two mid-asm logs, removed again). sub_821033C8 then registers
  the block and asks the resource manager (vtable +16) for `map_pack_N\map_pack_N.slb`; when that fails it
  unregisters the block and drops the pack. Help & Options -> Downloaded Content stays empty.
- Why: each pack's `.slb` is a 128-byte stub ("Map Pack N" and a few numbers). The worlds it refers to come with
  **title update 2**: its `Title_Update.blk` holds `gui/front_end/scr_map_select.slb` (the map-select screen),
  `terrain/terrain_dlc_world_1..4.slb`, `world/map_dlc_world_2.slb`, `general/dlc_world_N_init.text` and the map
  terrain/collision files. The base game 0.0.1.2 (this port) has none of them (no "map_pack" anywhere in its blocks).
- So the DLC works only with title update 2, which needs the avatar system (section "Why the base game").
- "Download Content" on the main menu is XamShowMarketplaceUI (a stub here): the Xbox Marketplace, not a list.

## Tool fixes (banjo-nx's post-codegen tools, as used here)

- `tools/localize_native.py`: a hand native's `exact=` registers are loaded back into the caller's copy after the
  native runs (before: never; harmless for banjo-nx's natives, wrong for one that returns a value).
- `tools/indirect_calls.py`: finds a `bctrl` with the link-register store between the comment and the call (this
  codegen keeps lr stores) and keeps r1_local.py's marker after the call. Before: 0 sites tagged, so the PC census
  of indirect calls was empty and no promotion was possible.

## Shared fixes from other ports

- sasrt-nx (2026-10-30, message from the SASRT session): plume's deko3d defaults (the Zcull invalidate braces, MSAA
  sample positions, alpha-to-coverage dither off, primitive restart for indexed strips). Already in this tree: banjo-nx
  took them the same day and this port's `app/thirdparty/plume` is that copy (11 `sasrt-nx (2026-10-30` markers).
- sasrt-nx `sasrt_renut_resolve_readback`: Renut never writes a resolve's result back to guest memory, so game code
  that locks a resolved texture on the CPU (auto-exposure, glare visibility) reads 0. Whether Keflings does this is
  checked on the PC (CPU locks of resolve destinations); see "Renderer census".

## PC fixes (each with its evidence)

| Symptom | Cause | Fix |
|---|---|---|
| Call to guest 0 at lr 0x821EA140 (TU2; base 0x821BFAC8's caller) | D3D's ring-space query (a function N&B does not link) on Renut's "no hardware" device calls a null interface callback | hook `sub_821BFAC8` answers 0 and 0 there (`src/keflings/keflings_d3d_hooks.cpp`); other cases run the original |
| Call to an unregistered 0x820A94B8 (TU2) | vtable-only functions never discovered | `tools/kef_pointer_scan.py`: 65 functions declared (base), fixpoint in 2 passes |
| "Update Required" then exit (TU2) | avatar system | base game (above) |
| Giant, Keflings and buildings missing, their stencil shadows drawn | the models' vertex shader (36254C6133320A67) reads COLOR0's alpha; their declarations have no COLOR0; Renut fed missing inputs (0,0,0,0) so the blended models were transparent | missing inputs read (0,0,0,1), D3D's rule for missing components (`keflings_renut_missing_input_w1`, on) |
| The game does not pace itself (1,900 fps uncapped) | on the 360 Present waited for vsync | `frame_cap = "60"` in the tomls |

How the model bug was found (no RenderDoc here): `keflings_diag_draw_log_frame = N` logs every draw of frame N
(shaders, primitive, count, depth/colour/blend/alpha state, target, and each new vertex layout). The model draws were
there (138 a frame); alpha test off and cull/front-face/EQUAL-depth changes did not bring them back; the layout line
showed the missing COLOR0. The diagnostic keys (`keflings_diag_*`) stay, off.

Open (seen twice, not fixed): a scripted A press during a screen change can call a null "current screen"
(`*(0x82483AD8) + 124`, read unchecked by `sub_821AB360` after a checked read), lr 0x821AB45C. Not seen with presses
only in the intro (to 38 s). Needs a look at which thread clears that pointer.

## Addresses

(Banjo base -> Keflings mapping of the renderer's D3D hooks: below as it is done.)

## Package e (2026-10-31): the console run of package d and what came of it

- **Where the frame goes** (console, handheld, Reverse-NX docked, 1020 MHz, three cores): the main thread is the
  limit at 92-97 % of a core, doing the game's own work (the collision broad phase `sub_82149C30` with its pair test
  `sub_8214A1D0`, the draw matrices `sub_820DB9F8`, the model update `sub_820CC638`, ...). The render thread needs
  4.4 ms a frame, the GPU 6.3 ms. docs/console-profile-2026-10-30.md has every function by kind.
- **Third round of hand natives** (banjo_hand_natives.h, keys keflings_hand_*): the hot "functions" were partly their
  inlined leaves, so the natives are on the leaves: `sub_8214A1D0` (pair test: flag, group-id and sphere exits),
  `sub_82149560` (sphere merge without its f24-f31 save), `sub_82149B08` (box merge), `sub_8213AA78` (scalar 4x4
  product), `sub_821FCD50` (VMX 4x4 product: rows read as they lie, the generated code's own simde transposes and
  `simde_mm_dp_ps`), `sub_820C96C0` (track step), `sub_820BF340` (box against frustum without its corner table).
  `sub_8212AA70` and `sub_820C3AC0` are virtual-call dispatch: no native gains there.
- **The guard compares stack bytes too**: a native must write what the original writes below r1 (register saves,
  back chain, scratch tables), or its first compared call differs and it is switched off (e_verify1).
- **MSAA**: Keflings renders its 3D image with 2x MSAA (1280 x 736 colour and depth). keflings_renut_msaa_samples
  sets the level of the game's MSAA targets (1, 2 = the game's own, 4); single-sampled targets stay as they are.
- **Pictures**: XamReadTileToTexture type 0 with tiles 1, 2, 3, ... (64 x 64) at the first kingdom: the achievement
  pictures, which the Achievement dialog shows. With the SDK's picture source (gamer_picture.h, from Bejeweled Blitz)
  they are the console user's picture (keflings/keflings_user_picture.cpp; the PC takes keflings_user_picture_file).

## 2 MB blocks: game memory and the NRO's code (version 1.0.1, 2026-11-05, from kameo-nx and the Torchlight port's findings)

The user asked for these two fixes, a 0.0.1 version bump and a release, through the Kameo session. The reasoning is in
kameo-nx's `docs/kameo-notes.md`, section "2 MB blocks"; this is what applies here.

- **Game memory.** `RexGmInit` had the same mistake: the second argument of `virtmemFindAslr` is a guard, not an
  alignment, so the window was never on a 2 MB boundary and all game memory was on 4 KB pages, in every build including
  1.0.0. It now asks for 2 MB more and rounds the base up. `banjo_window_blocks = false` goes back.
- **The NRO's code.** `sdk/src/ui/switch_code_blocks.cpp` is Kameo's file with `renut.toml` for `kameo.toml`. In 1.0.0's
  ELF the text is 26.0 MB and the read-only data 6.7 MB, so the move costs about 33 MB of memory. No shaders in the
  read-only data here: the installer's files hold them. `banjo_code_blocks = false` in `renut.toml` goes back.
- **Checked by reading, against Kameo's traps.** Exits: `windowed_app_main_switch.cpp` and `rex_app.cpp` end with
  `__nx_applet_exit_mode = 1` and `__libnx_exit`; `exit()` (HalReturnToFirmware, the disc read error) ends in the atexit
  hook's `diagAbortWithResult`. So nothing returns to hbloader. Threads: the first of ours is `switch_perf.cpp`'s profiler,
  at constructor priority 102, after the move at 101. `runtime_switch.cpp` and `windowed_app_main_switch.cpp` are the same
  as Kameo's. New functions are at the ends of their files, so the others keep their profile.
- **`sdk/cmake/rex_version.cmake`** (Kameo's fix): the game's `v*` tags are no longer read as SDK versions. They stop the
  configure in any clone that has fetched them, which includes a clean clone of the public repository.
- **Not copied:** `banjo_tlb_test_s`, the latency test. It is diagnostics, and the first version of it was wrong.
- **Not measured in this game.** The release note's "approximately 5%" is the user's wording, from Kameo's result (30.0
  against 28.8 fps, about 4.3 %, standing still in one GPU-heavy spot at stock clocks).
- **What an emulator can't show.** Ryujinx has no loader, so the code move doesn't run there: only a self-test of the
  swap does. On a console the log should say `[code blocks] text ... now on a copy`; `left where it is` means it skipped
  itself. The path that puts the old copy back if the kernel refuses the new one has never run, on Kameo either.
