# Kingdom for Keflings: console profile breakdown (package d)

From the console run of 2026-10-30 (logs/rex/rex_profile.log, handheld with Reverse-NX docked, CPU 1020 MHz, GPU 460.8 MHz, three cores), reports 6 to 12 (70 s of play after loading), mean 42.1 fps. Each sample is one stack sample of a busy thread (samples taken in kernel waits are not counted). Symbols from the ELF of the same build (app/out/switch/renut, 2026-10-30 23:07).

Kinds: **Hand** = the nine hand-written natives (hand.txt); **Top** = machine-optimised copies of the top list (localized.txt, without the hand ones); **Bulk** = machine-optimised copies of the bulk list (localized_bulk.txt, from the PC and ARM emulator profiles); **Generated** = the recompiled code as generated, not optimised; **Save/rest** = the PPC register save/restore helpers; **Renderer** = Renut / deko3d on the host; **SDK** = kernel, threads, audio; **libc** = memmove / memset / str*; **Locks** = mutex calls. A hand native that a copy inlines counts under that copy.

## Threads (mean CPU over the reports, % of one core) and where their busy samples go

| Thread | CPU | Samples | Hand | Top | Bulk | Copy (other) | Generated | Save/rest | Renderer | SDK | libc | Locks | Other |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Main XThread (game) | 92.4 % | 2025 | 1.0 | 2.1 | 64.3 | - | 17.0 | 3.3 | 7.0 | 1.2 | 1.9 | 1.2 | 1.0 |
| XThread209F0BA0 (4580) | 22.4 % | 477 | 2.3 | - | 32.9 | - | 60.8 | 1.5 | - | - | 0.6 | 0.2 | 1.7 |
| Render thread | 18.7 % | 414 | - | - | - | - | - | - | 81.6 | 10.4 | 4.1 | 1.0 | 2.9 |
| XThread14400EE0 (4581) | 18.0 % | 364 | - | - | 32.1 | - | 64.6 | 0.3 | - | 0.5 | - | 0.5 | 1.9 |
| XThread21400A80 (4583) | 15.1 % | 342 | - | - | 31.0 | - | 67.3 | - | - | 0.3 | 0.3 | 0.3 | 0.9 |
| XThread139FF120 (4582) | 11.8 % | 243 | 0.4 | - | 32.5 | - | 62.6 | 0.4 | - | 1.6 | 0.4 | 0.4 | 1.6 |
| XThread3C800F80 (4585) | 4.6 % | 104 | - | - | 79.8 | - | 8.7 | 1.0 | - | 4.8 | - | 1.9 | 3.8 |
| XMA decoder | 3.3 % | 62 | - | - | - | - | - | - | - | 19.4 | 4.8 | - | 75.8 |
| XThread3C800EA0 (4584) | 4.7 % | 61 | 1.6 | - | - | - | 8.2 | - | - | 27.9 | - | 23.0 | 39.3 |
| Audio worker | 1.7 % | 30 | - | - | - | - | 23.3 | - | - | 13.3 | - | 20.0 | 43.3 |
| XThread20936C20 (4571) | 1.7 % | 23 | - | - | - | - | - | - | - | 30.4 | - | 17.4 | 52.2 |
| rex::thread::TimerQueue (4534) | 1.5 % | 19 | - | - | - | - | - | - | - | 36.8 | - | 21.1 | 42.1 |
| main (UI) (4466) | 0.8 % | 16 | - | - | - | - | - | - | - | 18.8 | - | 12.5 | 68.8 |
| ? (4544) | 0.5 % | 16 | - | - | - | - | - | - | 12.5 | - | - | - | 87.5 |
| ? (4586) | 1.2 % | 10 | - | - | - | - | - | - | - | 80.0 | - | - | 20.0 |
| **All threads** | | 4206 | **0.8** | **1.0** | **43.8** | - | **30.3** | **1.8** | **11.4** | **3.3** | **1.5** | **1.5** | **4.5** |

## Main XThread (game): every function (2025 samples, CPU 92.4 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| hand native | 1.0 % | 20 | 5 |
| machine-optimised (top list) | 2.1 % | 43 | 3 |
| machine-optimised (bulk list) | 64.3 % | 1302 | 90 |
| generated, not optimised | 17.0 % | 345 | 150 |
| save/restore helpers | 3.3 % | 66 | 15 |
| renderer (Renut / deko3d) | 7.0 % | 141 | 25 |
| SDK (kernel, threads, audio) | 1.2 % | 24 | 13 |
| C library (mem/str) | 1.9 % | 39 | 2 |
| locks / kernel calls | 1.2 % | 24 | 2 |
| other host code | 1.0 % | 21 | 14 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `sub_82149C30` | machine-optimised (bulk list) | 166 | 8.20 % | 8.2 % |
| 2 | `sub_8214A1D0` | machine-optimised (bulk list) | 130 | 6.42 % | 14.6 % |
| 3 | `sub_820DB9F8` | machine-optimised (bulk list) | 113 | 5.58 % | 20.2 % |
| 4 | `sub_820CC638` | machine-optimised (bulk list) | 69 | 3.41 % | 23.6 % |
| 5 | `renut::gpu::CaptureDelta` | renderer (Renut / deko3d) | 54 | 2.67 % | 26.3 % |
| 6 | `sub_8212AA70` | machine-optimised (bulk list) | 48 | 2.37 % | 28.6 % |
| 7 | `sub_820C3AC0` | machine-optimised (bulk list) | 45 | 2.22 % | 30.9 % |
| 8 | `sub_8213AA78` | machine-optimised (bulk list) | 37 | 1.83 % | 32.7 % |
| 9 | `sub_820C7100` | machine-optimised (bulk list) | 33 | 1.63 % | 34.3 % |
| 10 | `sub_820C5D08` | machine-optimised (bulk list) | 32 | 1.58 % | 35.9 % |
| 11 | `sub_8212B320` | machine-optimised (bulk list) | 28 | 1.38 % | 37.3 % |
| 12 | `sub_82133AA0` | machine-optimised (bulk list) | 28 | 1.38 % | 38.7 % |
| 13 | `sub_8212B020` | machine-optimised (bulk list) | 27 | 1.33 % | 40.0 % |
| 14 | `sub_820A5448` | machine-optimised (bulk list) | 26 | 1.28 % | 41.3 % |
| 15 | `sub_8212A450` | machine-optimised (bulk list) | 25 | 1.23 % | 42.5 % |
| 16 | `memmove` | C library (mem/str) | 23 | 1.14 % | 43.7 % |
| 17 | `sub_8217BC10` | machine-optimised (bulk list) | 23 | 1.14 % | 44.8 % |
| 18 | `sub_820C7180` | machine-optimised (top list) | 23 | 1.14 % | 45.9 % |
| 19 | `sub_820C3D58` | machine-optimised (bulk list) | 22 | 1.09 % | 47.0 % |
| 20 | `sub_820CE0A0` | machine-optimised (bulk list) | 22 | 1.09 % | 48.1 % |
| 21 | `sub_820DDF60` | machine-optimised (bulk list) | 22 | 1.09 % | 49.2 % |
| 22 | `sub_820C6B88` | generated, not optimised | 21 | 1.04 % | 50.2 % |
| 23 | `sub_820C3F40` | machine-optimised (bulk list) | 20 | 0.99 % | 51.2 % |
| 24 | `sub_8212A0E8` | machine-optimised (bulk list) | 19 | 0.94 % | 52.1 % |
| 25 | `sub_820C6648` | machine-optimised (bulk list) | 18 | 0.89 % | 53.0 % |
| 26 | `sub_8217B8F0` | generated, not optimised | 18 | 0.89 % | 53.9 % |
| 27 | `sub_820C67C8` | machine-optimised (top list) | 17 | 0.84 % | 54.8 % |
| 28 | `sub_820C49D0` | machine-optimised (bulk list) | 17 | 0.84 % | 55.6 % |
| 29 | `sub_8212B108` | machine-optimised (bulk list) | 16 | 0.79 % | 56.4 % |
| 30 | `memset` | C library (mem/str) | 16 | 0.79 % | 57.2 % |
| 31 | `renut::gpu::native::SetTexture` | renderer (Renut / deko3d) | 16 | 0.79 % | 58.0 % |
| 32 | `sub_8212B208` | machine-optimised (bulk list) | 15 | 0.74 % | 58.7 % |
| 33 | `sub_820C3C38` | machine-optimised (bulk list) | 14 | 0.69 % | 59.4 % |
| 34 | `mutexLock` | locks / kernel calls | 14 | 0.69 % | 60.1 % |
| 35 | `sub_820C5DD8` | machine-optimised (bulk list) | 13 | 0.64 % | 60.7 % |
| 36 | `sub_820DC0F0` | machine-optimised (bulk list) | 13 | 0.64 % | 61.4 % |
| 37 | `__restgprlr_29` | save/restore helpers | 13 | 0.64 % | 62.0 % |
| 38 | `sub_8217BB50` | machine-optimised (bulk list) | 13 | 0.64 % | 62.7 % |
| 39 | `__savegprlr_29` | save/restore helpers | 13 | 0.64 % | 63.3 % |
| 40 | `sub_820D24E0` | machine-optimised (bulk list) | 12 | 0.59 % | 63.9 % |
| 41 | `sub_8217EAA8` | machine-optimised (bulk list) | 12 | 0.59 % | 64.5 % |
| 42 | `sub_820CE900` | machine-optimised (bulk list) | 12 | 0.59 % | 65.1 % |
| 43 | `sub_820D1028` | generated, not optimised | 11 | 0.54 % | 65.6 % |
| 44 | `sub_8218DF78` | generated, not optimised | 11 | 0.54 % | 66.2 % |
| 45 | `sub_820C6898` | machine-optimised (bulk list) | 11 | 0.54 % | 66.7 % |
| 46 | `sub_8217F7C0` | generated, not optimised | 11 | 0.54 % | 67.3 % |
| 47 | `sub_8217C910` | machine-optimised (bulk list) | 11 | 0.54 % | 67.8 % |
| 48 | `sub_8214D730` | generated, not optimised | 10 | 0.49 % | 68.3 % |
| 49 | `sub_820BAFD0` | machine-optimised (bulk list) | 10 | 0.49 % | 68.8 % |
| 50 | `sub_821497B8` | machine-optimised (bulk list) | 10 | 0.49 % | 69.3 % |
| 51 | `mutexUnlock` | locks / kernel calls | 10 | 0.49 % | 69.8 % |
| 52 | `sub_82128A78` | generated, not optimised | 10 | 0.49 % | 70.3 % |
| 53 | `__restgprlr_28` | save/restore helpers | 10 | 0.49 % | 70.8 % |
| 54 | `sub_820A80F8` | generated, not optimised | 9 | 0.44 % | 71.2 % |
| 55 | `sub_82180CC0` | generated, not optimised | 9 | 0.44 % | 71.7 % |
| 56 | `sub_82180B60` | generated, not optimised | 9 | 0.44 % | 72.1 % |
| 57 | `__savegprlr_28` | save/restore helpers | 9 | 0.44 % | 72.5 % |
| 58 | `renut::gpu::FlushRecordedVertices` | renderer (Renut / deko3d) | 9 | 0.44 % | 73.0 % |
| 59 | `sub_820E0968` | machine-optimised (bulk list) | 8 | 0.40 % | 73.4 % |
| 60 | `sub_8218E440` | machine-optimised (bulk list) | 8 | 0.40 % | 73.8 % |
| 61 | `sub_820A55A8` | machine-optimised (bulk list) | 8 | 0.40 % | 74.2 % |
| 62 | `sub_820A7D60` | generated, not optimised | 8 | 0.40 % | 74.6 % |
| 63 | `sub_820B22C0` | generated, not optimised | 8 | 0.40 % | 75.0 % |
| 64 | `renut::gpu::native::SetStreamSource` | renderer (Renut / deko3d) | 8 | 0.40 % | 75.4 % |
| 65 | `sub_820DC1F8` | machine-optimised (bulk list) | 7 | 0.35 % | 75.7 % |
| 66 | `sub_8212A908` | machine-optimised (bulk list) | 7 | 0.35 % | 76.0 % |
| 67 | `sub_820D51C0` | machine-optimised (bulk list) | 7 | 0.35 % | 76.4 % |
| 68 | `sub_82120DC0` | machine-optimised (bulk list) | 7 | 0.35 % | 76.7 % |
| 69 | `renut::gpu::render_thread::Push` | renderer (Renut / deko3d) | 7 | 0.35 % | 77.1 % |
| 70 | `sub_820C6F08` | machine-optimised (bulk list) | 7 | 0.35 % | 77.4 % |
| 71 | `rex::memory::BaseHeap::AllocRange` | SDK (kernel, threads, audio) | 6 | 0.30 % | 77.7 % |
| 72 | `__savegprlr_25` | save/restore helpers | 6 | 0.30 % | 78.0 % |
| 73 | `sub_820ACFF0` | generated, not optimised | 6 | 0.30 % | 78.3 % |
| 74 | `sub_820A5708` | generated, not optimised | 6 | 0.30 % | 78.6 % |
| 75 | `renut::gpu::QueueDraw` | renderer (Renut / deko3d) | 6 | 0.30 % | 78.9 % |
| 76 | `sub_82180D78` | hand native | 6 | 0.30 % | 79.2 % |
| 77 | `rex::kernel::xboxkrnl::RtlEnterCriticalSection_raw` | SDK (kernel, threads, audio) | 6 | 0.30 % | 79.5 % |
| 78 | `sub_820CFB88` | machine-optimised (bulk list) | 5 | 0.25 % | 79.8 % |
| 79 | `sub_8214D240` | generated, not optimised | 5 | 0.25 % | 80.0 % |
| 80 | `sub_8241D930` | generated, not optimised | 5 | 0.25 % | 80.2 % |
| 81 | `sub_820D4C00` | machine-optimised (bulk list) | 5 | 0.25 % | 80.5 % |
| 82 | `sub_82448F40` | hand native | 5 | 0.25 % | 80.7 % |
| 83 | `sub_8217D148` | machine-optimised (bulk list) | 5 | 0.25 % | 81.0 % |
| 84 | `sub_820CC0E0` | hand native | 5 | 0.25 % | 81.2 % |
| 85 | `sub_820C4170` | machine-optimised (bulk list) | 5 | 0.25 % | 81.5 % |
| 86 | `sub_820BF340` | machine-optimised (bulk list) | 4 | 0.20 % | 81.7 % |
| 87 | `sub_8241EA10` | machine-optimised (bulk list) | 4 | 0.20 % | 81.9 % |
| 88 | `sub_821FE1D0` | machine-optimised (bulk list) | 4 | 0.20 % | 82.1 % |
| 89 | `renut::gpu::FlushDeferredVertices` | renderer (Renut / deko3d) | 4 | 0.20 % | 82.3 % |
| 90 | `renut::gpu::render_thread::QueuePrefetched` | renderer (Renut / deko3d) | 4 | 0.20 % | 82.5 % |
| 91 | `sub_8241E320` | machine-optimised (bulk list) | 4 | 0.20 % | 82.7 % |
| 92 | `pthread_mutex_lock` | other host code | 4 | 0.20 % | 82.9 % |
| 93 | `sub_821D7578` | generated, not optimised | 4 | 0.20 % | 83.1 % |
| 94 | `renut::gpu::RunReadyCallbacks` | renderer (Renut / deko3d) | 4 | 0.20 % | 83.3 % |
| 95 | `sub_820C3CA0` | machine-optimised (bulk list) | 4 | 0.20 % | 83.5 % |
| 96 | `sub_820C0238` | machine-optimised (bulk list) | 4 | 0.20 % | 83.7 % |
| 97 | `renut::gpu::native::RsCullMode` | renderer (Renut / deko3d) | 4 | 0.20 % | 83.9 % |
| 98 | `renut::gpu::native::RsAlphaBlendEnable` | renderer (Renut / deko3d) | 4 | 0.20 % | 84.0 % |
| 99 | `SetShaderConstantsB` | other host code | 4 | 0.20 % | 84.2 % |
| 100 | `sub_820D5078` | generated, not optimised | 4 | 0.20 % | 84.4 % |
| 101 | `sub_8212A978` | machine-optimised (bulk list) | 4 | 0.20 % | 84.6 % |
| 102 | `sub_82179B20` | machine-optimised (bulk list) | 4 | 0.20 % | 84.8 % |
| 103 | `sub_8212AFC0` | generated, not optimised | 3 | 0.15 % | 85.0 % |
| 104 | `sub_820CB188` | machine-optimised (bulk list) | 3 | 0.15 % | 85.1 % |
| 105 | `sub_82405A80` | machine-optimised (bulk list) | 3 | 0.15 % | 85.3 % |
| 106 | `sub_820C38B0` | generated, not optimised | 3 | 0.15 % | 85.4 % |
| 107 | `sub_82158560` | generated, not optimised | 3 | 0.15 % | 85.6 % |
| 108 | `sub_820C3228` | generated, not optimised | 3 | 0.15 % | 85.7 % |
| 109 | `renut::gpu::native::RsMultiSampleAntiAlias` | renderer (Renut / deko3d) | 3 | 0.15 % | 85.9 % |
| 110 | `sub_821D57F8` | generated, not optimised | 3 | 0.15 % | 86.0 % |
| 111 | `sub_82123408` | generated, not optimised | 3 | 0.15 % | 86.2 % |
| 112 | `sub_820C3228` | machine-optimised (bulk list) | 3 | 0.15 % | 86.3 % |
| 113 | `sub_820DC1F8` | generated, not optimised | 3 | 0.15 % | 86.5 % |
| 114 | `sub_82405B78` | machine-optimised (bulk list) | 3 | 0.15 % | 86.6 % |
| 115 | `sub_821E8CC0` | generated, not optimised | 3 | 0.15 % | 86.8 % |
| 116 | `__savegprlr_23` | save/restore helpers | 3 | 0.15 % | 86.9 % |
| 117 | `sub_820C67C8` | generated, not optimised | 3 | 0.15 % | 87.1 % |
| 118 | `sub_820DBF30` | machine-optimised (bulk list) | 3 | 0.15 % | 87.2 % |
| 119 | `sub_82129C48` | generated, not optimised | 3 | 0.15 % | 87.4 % |
| 120 | `sub_820DA318` | generated, not optimised | 3 | 0.15 % | 87.5 % |
| 121 | `sub_820E1950` | generated, not optimised | 3 | 0.15 % | 87.7 % |
| 122 | `sub_820D5078` | machine-optimised (bulk list) | 3 | 0.15 % | 87.8 % |
| 123 | `sub_821DCA20` | generated, not optimised | 3 | 0.15 % | 88.0 % |
| 124 | `sub_820BB278` | machine-optimised (top list) | 3 | 0.15 % | 88.1 % |
| 125 | `sub_820DFB30` | machine-optimised (bulk list) | 2 | 0.10 % | 88.2 % |
| 126 | `sub_82130BB8` | machine-optimised (bulk list) | 2 | 0.10 % | 88.3 % |
| 127 | `sub_823F5A60` | machine-optimised (bulk list) | 2 | 0.10 % | 88.4 % |
| 128 | `sub_82194B48` | generated, not optimised | 2 | 0.10 % | 88.5 % |
| 129 | `sub_820D9150` | generated, not optimised | 2 | 0.10 % | 88.6 % |
| 130 | `rex::system::XObject::GetNativeObject` | SDK (kernel, threads, audio) | 2 | 0.10 % | 88.7 % |
| 131 | `sub_821368E8` | generated, not optimised | 2 | 0.10 % | 88.8 % |
| 132 | `sub_82194950` | generated, not optimised | 2 | 0.10 % | 88.9 % |
| 133 | `sub_820DB820` | generated, not optimised | 2 | 0.10 % | 89.0 % |
| 134 | `sub_820C82B8` | machine-optimised (bulk list) | 2 | 0.10 % | 89.1 % |
| 135 | `sub_820DB9F8` | generated, not optimised | 2 | 0.10 % | 89.2 % |
| 136 | `sub_823868C8` | machine-optimised (bulk list) | 2 | 0.10 % | 89.3 % |
| 137 | `RexSwitchPerfAdd` | other host code | 2 | 0.10 % | 89.4 % |
| 138 | `renut::gpu::native::RsDestBlend` | renderer (Renut / deko3d) | 2 | 0.10 % | 89.5 % |
| 139 | `sub_82447298` | hand native | 2 | 0.10 % | 89.6 % |
| 140 | `sub_820D9A48` | hand native | 2 | 0.10 % | 89.7 % |
| 141 | `sub_82179B20` | generated, not optimised | 2 | 0.10 % | 89.8 % |
| 142 | `sub_821289F0` | generated, not optimised | 2 | 0.10 % | 89.9 % |
| 143 | `__restgprlr_25` | save/restore helpers | 2 | 0.10 % | 90.0 % |
| 144 | `renut::gpu::native::SetRenderState` | renderer (Renut / deko3d) | 2 | 0.10 % | 90.1 % |
| 145 | `sub_820DA5D0` | generated, not optimised | 2 | 0.10 % | 90.2 % |
| 146 | `sub_820C3CF0` | machine-optimised (bulk list) | 2 | 0.10 % | 90.3 % |
| 147 | `sub_8212E790` | machine-optimised (bulk list) | 2 | 0.10 % | 90.4 % |
| 148 | `sub_82138258` | machine-optimised (bulk list) | 2 | 0.10 % | 90.5 % |
| 149 | `sub_820C2FB0` | machine-optimised (bulk list) | 2 | 0.10 % | 90.6 % |
| 150 | `sub_8241D478` | machine-optimised (bulk list) | 2 | 0.10 % | 90.7 % |
| 151 | `sub_82192A68` | machine-optimised (bulk list) | 2 | 0.10 % | 90.8 % |
| 152 | `sub_820D0CC8` | generated, not optimised | 2 | 0.10 % | 90.9 % |
| 153 | `sub_821D5090` | generated, not optimised | 2 | 0.10 % | 91.0 % |
| 154 | `sub_8214EAF0` | generated, not optimised | 2 | 0.10 % | 91.1 % |
| 155 | `renut::gpu::IsRecordingCommandBuffer` | renderer (Renut / deko3d) | 2 | 0.10 % | 91.2 % |
| 156 | `sub_820CE830` | generated, not optimised | 2 | 0.10 % | 91.3 % |
| 157 | `sub_821D5D00` | generated, not optimised | 2 | 0.10 % | 91.4 % |
| 158 | `sub_820C3CF0` | generated, not optimised | 2 | 0.10 % | 91.5 % |
| 159 | `sub_820D4568` | generated, not optimised | 2 | 0.10 % | 91.6 % |
| 160 | `sub_82192588` | generated, not optimised | 2 | 0.10 % | 91.7 % |
| 161 | `sub_820DB790` | generated, not optimised | 2 | 0.10 % | 91.8 % |
| 162 | `sub_820CCAE0` | machine-optimised (bulk list) | 2 | 0.10 % | 91.9 % |
| 163 | `sub_821306F8` | generated, not optimised | 2 | 0.10 % | 92.0 % |
| 164 | `renut::gpu::native::RsZEnable` | renderer (Renut / deko3d) | 2 | 0.10 % | 92.0 % |
| 165 | `__restgprlr_20` | save/restore helpers | 2 | 0.10 % | 92.1 % |
| 166 | `sub_821D7458` | generated, not optimised | 2 | 0.10 % | 92.2 % |
| 167 | `renut::gpu::native::SetSamplerState` | renderer (Renut / deko3d) | 2 | 0.10 % | 92.3 % |
| 168 | `sub_820DB7B0` | generated, not optimised | 2 | 0.10 % | 92.4 % |
| 169 | `sub_8214A950` | generated, not optimised | 2 | 0.10 % | 92.5 % |
| 170 | `__savegprlr_20` | save/restore helpers | 2 | 0.10 % | 92.6 % |
| 171 | `sub_8212A0E8` | generated, not optimised | 1 | 0.05 % | 92.7 % |
| 172 | `sub_820F4E20` | generated, not optimised | 1 | 0.05 % | 92.7 % |
| 173 | `sub_8214AC70` | generated, not optimised | 1 | 0.05 % | 92.8 % |
| 174 | `sub_821873D0` | generated, not optimised | 1 | 0.05 % | 92.8 % |
| 175 | `sub_820BB6E8` | generated, not optimised | 1 | 0.05 % | 92.9 % |
| 176 | `sub_820E0AA0` | generated, not optimised | 1 | 0.05 % | 92.9 % |
| 177 | `rex::memory::BaseHeap::Release` | SDK (kernel, threads, audio) | 1 | 0.05 % | 93.0 % |
| 178 | `rex::kernel::xboxkrnl::KeRaiseIrqlToDpcLevel_raw` | SDK (kernel, threads, audio) | 1 | 0.05 % | 93.0 % |
| 179 | `sub_820DBD98` | generated, not optimised | 1 | 0.05 % | 93.1 % |
| 180 | `sub_821891A8` | generated, not optimised | 1 | 0.05 % | 93.1 % |
| 181 | `__restgprlr_16` | save/restore helpers | 1 | 0.05 % | 93.2 % |
| 182 | `sub_821308C8` | generated, not optimised | 1 | 0.05 % | 93.2 % |
| 183 | `sub_82130D80` | generated, not optimised | 1 | 0.05 % | 93.3 % |
| 184 | `sub_820A6BF8` | generated, not optimised | 1 | 0.05 % | 93.3 % |
| 185 | `sub_820D4C00` | generated, not optimised | 1 | 0.05 % | 93.4 % |
| 186 | `sub_820A6C20` | generated, not optimised | 1 | 0.05 % | 93.4 % |
| 187 | `renut::frame_stats::OnGameFrame` | renderer (Renut / deko3d) | 1 | 0.05 % | 93.5 % |
| 188 | `renut::gpu::native::SetViewport` | renderer (Renut / deko3d) | 1 | 0.05 % | 93.5 % |
| 189 | `sub_8212AA70` | generated, not optimised | 1 | 0.05 % | 93.6 % |
| 190 | `sub_82145CA8` | generated, not optimised | 1 | 0.05 % | 93.6 % |
| 191 | `sub_820BEF98` | machine-optimised (bulk list) | 1 | 0.05 % | 93.7 % |
| 192 | `sub_8212A908` | generated, not optimised | 1 | 0.05 % | 93.7 % |
| 193 | `sub_8218BE80` | generated, not optimised | 1 | 0.05 % | 93.8 % |
| 194 | `sub_8217C6D8` | generated, not optimised | 1 | 0.05 % | 93.8 % |
| 195 | `sub_820E25B0` | generated, not optimised | 1 | 0.05 % | 93.9 % |
| 196 | `sub_82140AA0` | generated, not optimised | 1 | 0.05 % | 93.9 % |
| 197 | `sub_8217FD58` | generated, not optimised | 1 | 0.05 % | 94.0 % |
| 198 | `sub_82448A08` | generated, not optimised | 1 | 0.05 % | 94.0 % |
| 199 | `sub_820E4F80` | generated, not optimised | 1 | 0.05 % | 94.1 % |
| 200 | `sub_823FA1B0` | generated, not optimised | 1 | 0.05 % | 94.1 % |
| 201 | `sub_821F3E30` | generated, not optimised | 1 | 0.05 % | 94.2 % |
| 202 | `sub_82124060` | generated, not optimised | 1 | 0.05 % | 94.2 % |
| 203 | `sub_82165070` | generated, not optimised | 1 | 0.05 % | 94.3 % |
| 204 | `sub_820D8F68` | generated, not optimised | 1 | 0.05 % | 94.3 % |
| 205 | `sub_82195CA8` | generated, not optimised | 1 | 0.05 % | 94.4 % |
| 206 | `sub_820E06E8` | generated, not optimised | 1 | 0.05 % | 94.4 % |
| 207 | `sub_820BE3C0` | machine-optimised (bulk list) | 1 | 0.05 % | 94.5 % |
| 208 | `sub_820C7180` | generated, not optimised | 1 | 0.05 % | 94.5 % |
| 209 | `sub_8217C910` | generated, not optimised | 1 | 0.05 % | 94.6 % |
| 210 | `sub_820DB518` | generated, not optimised | 1 | 0.05 % | 94.6 % |
| 211 | `sub_823F9138` | machine-optimised (bulk list) | 1 | 0.05 % | 94.7 % |
| 212 | `sub_820DB5F0` | generated, not optimised | 1 | 0.05 % | 94.7 % |
| 213 | `sub_820DB5A0` | generated, not optimised | 1 | 0.05 % | 94.8 % |
| 214 | `sub_8214A1D0` | machine-optimised (bulk list) | 1 | 0.05 % | 94.8 % |
| 215 | `rex::LogCaptureSink::sink_it_` | SDK (kernel, threads, audio) | 1 | 0.05 % | 94.9 % |
| 216 | `_hidGetStates` | other host code | 1 | 0.05 % | 94.9 % |
| 217 | `pthread_mutex_trylock` | other host code | 1 | 0.05 % | 95.0 % |
| 218 | `sub_820C7078` | machine-optimised (bulk list) | 1 | 0.05 % | 95.0 % |
| 219 | `sub_82124208` | generated, not optimised | 1 | 0.05 % | 95.1 % |
| 220 | `sub_820DB7D0` | generated, not optimised | 1 | 0.05 % | 95.1 % |
| 221 | `sub_8241E1E0` | generated, not optimised | 1 | 0.05 % | 95.2 % |
| 222 | `sub_8218A6A8` | generated, not optimised | 1 | 0.05 % | 95.2 % |
| 223 | `sub_820B4B40` | generated, not optimised | 1 | 0.05 % | 95.3 % |
| 224 | `renut::gpu::native::DirtyClearRects` | renderer (Renut / deko3d) | 1 | 0.05 % | 95.3 % |
| 225 | `sub_821DFFF0` | generated, not optimised | 1 | 0.05 % | 95.4 % |
| 226 | `rex::system::xam::AppManager::DispatchMessageSync` | SDK (kernel, threads, audio) | 1 | 0.05 % | 95.4 % |
| 227 | `rex::kernel::xboxkrnl::RtlLeaveCriticalSection_entry` | SDK (kernel, threads, audio) | 1 | 0.05 % | 95.5 % |
| 228 | `rex::kernel::xboxkrnl::KeGetCurrentProcessType_entry` | SDK (kernel, threads, audio) | 1 | 0.05 % | 95.5 % |
| 229 | `sub_821179B0` | generated, not optimised | 1 | 0.05 % | 95.6 % |
| 230 | `sub_820C6580` | generated, not optimised | 1 | 0.05 % | 95.6 % |
| 231 | `sub_82140AC8` | generated, not optimised | 1 | 0.05 % | 95.7 % |
| 232 | `sub_820CCDD8` | generated, not optimised | 1 | 0.05 % | 95.7 % |
| 233 | `sub_8211ADC8` | generated, not optimised | 1 | 0.05 % | 95.8 % |
| 234 | `sub_821D52D8` | generated, not optimised | 1 | 0.05 % | 95.8 % |
| 235 | `sub_8218E440` | generated, not optimised | 1 | 0.05 % | 95.9 % |
| 236 | `sub_8241F0A0` | machine-optimised (bulk list) | 1 | 0.05 % | 95.9 % |
| 237 | `sub_823F5A60` | generated, not optimised | 1 | 0.05 % | 96.0 % |
| 238 | `sub_820DEC80` | machine-optimised (bulk list) | 1 | 0.05 % | 96.0 % |
| 239 | `sub_820C3AC0` | generated, not optimised | 1 | 0.05 % | 96.0 % |
| 240 | `sub_820C5D08` | generated, not optimised | 1 | 0.05 % | 96.1 % |
| 241 | `clock_gettime` | other host code | 1 | 0.05 % | 96.1 % |
| 242 | `pthread_once` | other host code | 1 | 0.05 % | 96.2 % |
| 243 | `sub_820C4D00` | generated, not optimised | 1 | 0.05 % | 96.2 % |
| 244 | `renut::gpu::native::SsMinFilter` | renderer (Renut / deko3d) | 1 | 0.05 % | 96.3 % |
| 245 | `sub_821DA128` | generated, not optimised | 1 | 0.05 % | 96.3 % |
| 246 | `sub_821D57B8` | generated, not optimised | 1 | 0.05 % | 96.4 % |
| 247 | `sub_821D5128` | generated, not optimised | 1 | 0.05 % | 96.4 % |
| 248 | `sub_82160C30` | generated, not optimised | 1 | 0.05 % | 96.5 % |
| 249 | `rex::Runtime::instance` | SDK (kernel, threads, audio) | 1 | 0.05 % | 96.5 % |
| 250 | `__imp__NtSetEvent` | other host code | 1 | 0.05 % | 96.6 % |
| 251 | `sub_820C8020` | generated, not optimised | 1 | 0.05 % | 96.6 % |
| 252 | `__savefpr_25` | save/restore helpers | 1 | 0.05 % | 96.7 % |
| 253 | `sub_820D9ED8` | generated, not optimised | 1 | 0.05 % | 96.7 % |
| 254 | `sub_8217BB50` | generated, not optimised | 1 | 0.05 % | 96.8 % |
| 255 | `rex::input::SlotAssignment::DevicesForUser` | SDK (kernel, threads, audio) | 1 | 0.05 % | 96.8 % |
| 256 | `sub_820CC638` | generated, not optimised | 1 | 0.05 % | 96.9 % |
| 257 | `std::__get_once_mutex` | other host code | 1 | 0.05 % | 96.9 % |
| 258 | `sub_820BCBD8` | generated, not optimised | 1 | 0.05 % | 97.0 % |
| 259 | `sub_820BF2D8` | generated, not optimised | 1 | 0.05 % | 97.0 % |
| 260 | `__savefpr_14` | save/restore helpers | 1 | 0.05 % | 97.1 % |
| 261 | `sub_821D6AE0` | generated, not optimised | 1 | 0.05 % | 97.1 % |
| 262 | `sub_821E8028` | generated, not optimised | 1 | 0.05 % | 97.2 % |
| 263 | `sub_82191FE0` | generated, not optimised | 1 | 0.05 % | 97.2 % |
| 264 | `sub_8211C7E8` | generated, not optimised | 1 | 0.05 % | 97.3 % |
| 265 | `sub_82175430` | generated, not optimised | 1 | 0.05 % | 97.3 % |
| 266 | `sub_82126430` | generated, not optimised | 1 | 0.05 % | 97.4 % |
| 267 | `sub_824451E8` | generated, not optimised | 1 | 0.05 % | 97.4 % |
| 268 | `sub_820DC070` | generated, not optimised | 1 | 0.05 % | 97.5 % |
| 269 | `sub_820E0180` | generated, not optimised | 1 | 0.05 % | 97.5 % |
| 270 | `sub_8241F0A0` | generated, not optimised | 1 | 0.05 % | 97.6 % |
| 271 | `rex::input::InputSystem::RefreshDevices` | SDK (kernel, threads, audio) | 1 | 0.05 % | 97.6 % |
| 272 | `sub_820C65F8` | machine-optimised (bulk list) | 1 | 0.05 % | 97.7 % |
| 273 | `sub_820C49D0` | generated, not optimised | 1 | 0.05 % | 97.7 % |
| 274 | `sub_821210D0` | machine-optimised (bulk list) | 1 | 0.05 % | 97.8 % |
| 275 | `sub_820CB188` | generated, not optimised | 1 | 0.05 % | 97.8 % |
| 276 | `sub_820C4990` | machine-optimised (bulk list) | 1 | 0.05 % | 97.9 % |
| 277 | `sub_820DBFF8` | machine-optimised (bulk list) | 1 | 0.05 % | 97.9 % |
| 278 | `sub_821A3B90` | generated, not optimised | 1 | 0.05 % | 98.0 % |
| 279 | `__savegprlr_22` | save/restore helpers | 1 | 0.05 % | 98.0 % |
| 280 | `__wrap_svcSleepThread` | other host code | 1 | 0.05 % | 98.1 % |
| 281 | `KickOff_hook` | other host code | 1 | 0.05 % | 98.1 % |
| 282 | `BlockOnFence_hook` | other host code | 1 | 0.05 % | 98.2 % |
| 283 | `sub_821D91D0` | generated, not optimised | 1 | 0.05 % | 98.2 % |
| 284 | `sub_821D68E8` | generated, not optimised | 1 | 0.05 % | 98.3 % |
| 285 | `__savegprlr_27` | save/restore helpers | 1 | 0.05 % | 98.3 % |
| 286 | `sub_820DC430` | generated, not optimised | 1 | 0.05 % | 98.4 % |
| 287 | `sub_82175308` | generated, not optimised | 1 | 0.05 % | 98.4 % |
| 288 | `sub_820DB538` | generated, not optimised | 1 | 0.05 % | 98.5 % |
| 289 | `sub_8211BD00` | generated, not optimised | 1 | 0.05 % | 98.5 % |
| 290 | `sub_8212E9E0` | generated, not optimised | 1 | 0.05 % | 98.6 % |
| 291 | `sub_82180968` | generated, not optimised | 1 | 0.05 % | 98.6 % |
| 292 | `renut::gpu::UnleashedCensusOn` | renderer (Renut / deko3d) | 1 | 0.05 % | 98.7 % |
| 293 | `renutFrameLimit` | other host code | 1 | 0.05 % | 98.7 % |
| 294 | `sub_820C2FB0` | generated, not optimised | 1 | 0.05 % | 98.8 % |
| 295 | `sub_82130C98` | machine-optimised (bulk list) | 1 | 0.05 % | 98.8 % |
| 296 | `_free_r` | other host code | 1 | 0.05 % | 98.9 % |
| 297 | `sub_821E89D0` | generated, not optimised | 1 | 0.05 % | 98.9 % |
| 298 | `sub_820B9E70` | generated, not optimised | 1 | 0.05 % | 99.0 % |
| 299 | `renut::gpu::native::RsFillMode` | renderer (Renut / deko3d) | 1 | 0.05 % | 99.0 % |
| 300 | `renut::gpu::native::RsSlopeScaleDepthBias` | renderer (Renut / deko3d) | 1 | 0.05 % | 99.1 % |
| 301 | `renut::gpu::native::RsSrcBlend` | renderer (Renut / deko3d) | 1 | 0.05 % | 99.1 % |
| 302 | `rex::kernel::xboxkrnl::xeKeKfReleaseSpinLock` | SDK (kernel, threads, audio) | 1 | 0.05 % | 99.2 % |
| 303 | `sub_820D65D0` | generated, not optimised | 1 | 0.05 % | 99.2 % |
| 304 | `sub_82172200` | generated, not optimised | 1 | 0.05 % | 99.3 % |
| 305 | `sub_820C10C8` | generated, not optimised | 1 | 0.05 % | 99.3 % |
| 306 | `sub_82192790` | generated, not optimised | 1 | 0.05 % | 99.4 % |
| 307 | `sub_8211FEB8` | generated, not optimised | 1 | 0.05 % | 99.4 % |
| 308 | `sub_820BFEB8` | generated, not optimised | 1 | 0.05 % | 99.5 % |
| 309 | `__restgprlr_21` | save/restore helpers | 1 | 0.05 % | 99.5 % |
| 310 | `sub_82130E60` | generated, not optimised | 1 | 0.05 % | 99.6 % |
| 311 | `sub_820D4F88` | generated, not optimised | 1 | 0.05 % | 99.6 % |
| 312 | `sub_820DADA0` | generated, not optimised | 1 | 0.05 % | 99.7 % |
| 313 | `sub_820E7A20` | generated, not optimised | 1 | 0.05 % | 99.7 % |
| 314 | `sub_820D4798` | generated, not optimised | 1 | 0.05 % | 99.8 % |
| 315 | `sub_82136AF8` | machine-optimised (bulk list) | 1 | 0.05 % | 99.8 % |
| 316 | `sub_820DB5F0` | machine-optimised (bulk list) | 1 | 0.05 % | 99.9 % |
| 317 | `sub_820D0C70` | machine-optimised (bulk list) | 1 | 0.05 % | 99.9 % |
| 318 | `sub_823F5AD8` | machine-optimised (bulk list) | 1 | 0.05 % | 100.0 % |
| 319 | `sub_820DEDC8` | machine-optimised (bulk list) | 1 | 0.05 % | 100.0 % |

## 4580 XThread209F0BA0: every function (477 samples, CPU 22.4 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| hand native | 2.3 % | 11 | 1 |
| machine-optimised (bulk list) | 32.9 % | 157 | 8 |
| generated, not optimised | 60.8 % | 290 | 14 |
| save/restore helpers | 1.5 % | 7 | 2 |
| C library (mem/str) | 0.6 % | 3 | 2 |
| locks / kernel calls | 0.2 % | 1 | 1 |
| other host code | 1.7 % | 8 | 7 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `sub_820D1028` | generated, not optimised | 198 | 41.51 % | 41.5 % |
| 2 | `sub_820CFB88` | machine-optimised (bulk list) | 94 | 19.71 % | 61.2 % |
| 3 | `sub_8214D730` | generated, not optimised | 56 | 11.74 % | 73.0 % |
| 4 | `sub_820BF340` | machine-optimised (bulk list) | 35 | 7.34 % | 80.3 % |
| 5 | `sub_82448F40` | hand native | 11 | 2.31 % | 82.6 % |
| 6 | `sub_82386BB0` | machine-optimised (bulk list) | 10 | 2.10 % | 84.7 % |
| 7 | `sub_821368E8` | generated, not optimised | 10 | 2.10 % | 86.8 % |
| 8 | `sub_820E1950` | generated, not optimised | 9 | 1.89 % | 88.7 % |
| 9 | `sub_823868C8` | machine-optimised (bulk list) | 7 | 1.47 % | 90.1 % |
| 10 | `sub_82385740` | generated, not optimised | 5 | 1.05 % | 91.2 % |
| 11 | `__restgprlr_16` | save/restore helpers | 5 | 1.05 % | 92.2 % |
| 12 | `sub_82125F18` | machine-optimised (bulk list) | 5 | 1.05 % | 93.3 % |
| 13 | `sub_820DDF60` | machine-optimised (bulk list) | 4 | 0.84 % | 94.1 % |
| 14 | `sub_820D9B70` | generated, not optimised | 3 | 0.63 % | 94.8 % |
| 15 | `memset` | C library (mem/str) | 2 | 0.42 % | 95.2 % |
| 16 | `sub_822CFC40` | generated, not optimised | 2 | 0.42 % | 95.6 % |
| 17 | `_malloc_r` | other host code | 2 | 0.42 % | 96.0 % |
| 18 | `__savegprlr_16` | save/restore helpers | 2 | 0.42 % | 96.4 % |
| 19 | `rmutexTryLock` | locks / kernel calls | 1 | 0.21 % | 96.6 % |
| 20 | `sub_82386A10` | machine-optimised (bulk list) | 1 | 0.21 % | 96.9 % |
| 21 | `sub_82131CC0` | generated, not optimised | 1 | 0.21 % | 97.1 % |
| 22 | `memmove` | C library (mem/str) | 1 | 0.21 % | 97.3 % |
| 23 | `__getreent` | other host code | 1 | 0.21 % | 97.5 % |
| 24 | `pthread_mutex_unlock` | other host code | 1 | 0.21 % | 97.7 % |
| 25 | `abstimespec2nsec` | other host code | 1 | 0.21 % | 97.9 % |
| 26 | `sub_82448F40` | generated, not optimised | 1 | 0.21 % | 98.1 % |
| 27 | `sub_820CFFB0` | generated, not optimised | 1 | 0.21 % | 98.3 % |
| 28 | `__syscall_clock_gettime` | other host code | 1 | 0.21 % | 98.5 % |
| 29 | `sub_821234A8` | generated, not optimised | 1 | 0.21 % | 98.7 % |
| 30 | `sub_820BF340` | generated, not optimised | 1 | 0.21 % | 99.0 % |
| 31 | `sub_820CFB88` | generated, not optimised | 1 | 0.21 % | 99.2 % |
| 32 | `_free_r` | other host code | 1 | 0.21 % | 99.4 % |
| 33 | `sub_821330D0` | machine-optimised (bulk list) | 1 | 0.21 % | 99.6 % |
| 34 | `pthread_mutex_trylock` | other host code | 1 | 0.21 % | 99.8 % |
| 35 | `sub_82124148` | generated, not optimised | 1 | 0.21 % | 100.0 % |

## Render thread: every function (414 samples, CPU 18.7 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| renderer (Renut / deko3d) | 81.6 % | 338 | 75 |
| SDK (kernel, threads, audio) | 10.4 % | 43 | 4 |
| C library (mem/str) | 4.1 % | 17 | 3 |
| locks / kernel calls | 1.0 % | 4 | 2 |
| other host code | 2.9 % | 12 | 8 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `renut::gpu::FastDrawLocked` | renderer (Renut / deko3d) | 70 | 16.91 % | 16.9 % |
| 2 | `renut::gpu::ConvertVertices` | renderer (Renut / deko3d) | 47 | 11.35 % | 28.3 % |
| 3 | `banjo::renut_gpu::PmullHash` | SDK (kernel, threads, audio) | 40 | 9.66 % | 37.9 % |
| 4 | `renut::gpu::FillTextures` | renderer (Renut / deko3d) | 26 | 6.28 % | 44.2 % |
| 5 | `renut::gpu::PushFloatConstants` | renderer (Renut / deko3d) | 20 | 4.83 % | 49.0 % |
| 6 | `memmove` | C library (mem/str) | 14 | 3.38 % | 52.4 % |
| 7 | `std::_Function_handler<void ` | renderer (Renut / deko3d) | 13 | 3.14 % | 55.6 % |
| 8 | `renut::gpu::PagesClean` | renderer (Renut / deko3d) | 12 | 2.90 % | 58.5 % |
| 9 | `renut::gpu::WholeGeometryTry` | renderer (Renut / deko3d) | 11 | 2.66 % | 61.1 % |
| 10 | `renut::gpu::CapturePool::Give` | renderer (Renut / deko3d) | 9 | 2.17 % | 63.3 % |
| 11 | `renut::gpu::BindStreams` | renderer (Renut / deko3d) | 8 | 1.93 % | 65.2 % |
| 12 | `void renut::gpu::ExpandElement<2u>` | renderer (Renut / deko3d) | 6 | 1.45 % | 66.7 % |
| 13 | `renut::gpu::BindHashed` | renderer (Renut / deko3d) | 6 | 1.45 % | 68.1 % |
| 14 | `renut::gpu::render_thread::Run` | renderer (Renut / deko3d) | 6 | 1.45 % | 69.6 % |
| 15 | `renut::gpu::BindTargets` | renderer (Renut / deko3d) | 5 | 1.21 % | 70.8 % |
| 16 | `renut::gpu::PrepareGeometry` | renderer (Renut / deko3d) | 5 | 1.21 % | 72.0 % |
| 17 | `renut::gpu::ComputeViewport` | renderer (Renut / deko3d) | 5 | 1.21 % | 73.2 % |
| 18 | `pthread_mutex_unlock` | other host code | 4 | 0.97 % | 74.2 % |
| 19 | `plume::Deko3DCommandList::bindTextures` | renderer (Renut / deko3d) | 4 | 0.97 % | 75.1 % |
| 20 | `renut::gpu::GetHostSurfaceLocked` | renderer (Renut / deko3d) | 3 | 0.72 % | 75.8 % |
| 21 | `renut::gpu::UploadRing::Allocate` | renderer (Renut / deko3d) | 3 | 0.72 % | 76.6 % |
| 22 | `renut::gpu::ExpandIndices` | renderer (Renut / deko3d) | 3 | 0.72 % | 77.3 % |
| 23 | `renut::gpu::records::Lookup` | renderer (Renut / deko3d) | 3 | 0.72 % | 78.0 % |
| 24 | `plume::Deko3DCommandList::setVertexBuffers` | renderer (Renut / deko3d) | 3 | 0.72 % | 78.7 % |
| 25 | `mutexLock` | locks / kernel calls | 3 | 0.72 % | 79.5 % |
| 26 | `renut::gpu::FramebufferEpochLocked` | renderer (Renut / deko3d) | 2 | 0.48 % | 80.0 % |
| 27 | `dkCmdBufBindVtxBuffers` | renderer (Renut / deko3d) | 2 | 0.48 % | 80.4 % |
| 28 | `renut::gpu::DrawsWithoutPixelShader` | renderer (Renut / deko3d) | 2 | 0.48 % | 80.9 % |
| 29 | `bool renut::gpu::ProveCopyRange<renut::gpu::CachedIndices>` | renderer (Renut / deko3d) | 2 | 0.48 % | 81.4 % |
| 30 | `memset` | C library (mem/str) | 2 | 0.48 % | 81.9 % |
| 31 | `renut::gpu::SetViewport` | renderer (Renut / deko3d) | 2 | 0.48 % | 82.4 % |
| 32 | `renut::gpu::SetSlotWords` | renderer (Renut / deko3d) | 2 | 0.48 % | 82.9 % |
| 33 | `renut::gpu::GatherTargets` | renderer (Renut / deko3d) | 2 | 0.48 % | 83.3 % |
| 34 | `renut::gpu::DescribeBoundFetches` | renderer (Renut / deko3d) | 2 | 0.48 % | 83.8 % |
| 35 | `dkCmdBufDrawIndexed` | renderer (Renut / deko3d) | 2 | 0.48 % | 84.3 % |
| 36 | `renut::gpu::PushSharedConstants` | renderer (Renut / deko3d) | 2 | 0.48 % | 84.8 % |
| 37 | `renut::gpu::ConvertVertices` | renderer (Renut / deko3d) | 2 | 0.48 % | 85.3 % |
| 38 | `pthread_mutex_lock` | other host code | 2 | 0.48 % | 85.7 % |
| 39 | `plume::Deko3DBindMemExport` | renderer (Renut / deko3d) | 2 | 0.48 % | 86.2 % |
| 40 | `renut::gpu::AdvanceStreamCacheFrameLocked` | renderer (Renut / deko3d) | 2 | 0.48 % | 86.7 % |
| 41 | `renut::gpu::BindWholeStreams` | renderer (Renut / deko3d) | 2 | 0.48 % | 87.2 % |
| 42 | `renut::gpu::RefreshBindingLocked` | renderer (Renut / deko3d) | 2 | 0.48 % | 87.7 % |
| 43 | `plume::Deko3DCommandList::bindGraphicsBuffers` | renderer (Renut / deko3d) | 2 | 0.48 % | 88.2 % |
| 44 | `renut::gpu::MinAnisotropy` | renderer (Renut / deko3d) | 1 | 0.24 % | 88.4 % |
| 45 | `mutexUnlock` | locks / kernel calls | 1 | 0.24 % | 88.6 % |
| 46 | `armDCacheFlush_L0` | other host code | 1 | 0.24 % | 88.9 % |
| 47 | `banjo::renut_gpu::CountPushedConstants` | SDK (kernel, threads, audio) | 1 | 0.24 % | 89.1 % |
| 48 | `renut::gpu::ReleaseSpareBuffersWhenHeapShort` | renderer (Renut / deko3d) | 1 | 0.24 % | 89.4 % |
| 49 | `renut::gpu::RebaseIndices16` | renderer (Renut / deko3d) | 1 | 0.24 % | 89.6 % |
| 50 | `renut::gpu::CheckedHash` | renderer (Renut / deko3d) | 1 | 0.24 % | 89.9 % |
| 51 | `renut::gpu::BindTextureLocked` | renderer (Renut / deko3d) | 1 | 0.24 % | 90.1 % |
| 52 | `renut::gpu::CopyPool::Allocate` | renderer (Renut / deko3d) | 1 | 0.24 % | 90.3 % |
| 53 | `dk::detail::ImageInfo::fromImageView` | renderer (Renut / deko3d) | 1 | 0.24 % | 90.6 % |
| 54 | `dkCmdBufBindRenderTargets` | renderer (Renut / deko3d) | 1 | 0.24 % | 90.8 % |
| 55 | `renut::gpu::gpu_profile::CountDraw` | renderer (Renut / deko3d) | 1 | 0.24 % | 91.1 % |
| 56 | `renut::gpu::SetIndexBuffer` | renderer (Renut / deko3d) | 1 | 0.24 % | 91.3 % |
| 57 | `renut::gpu::DropReplacedRanges` | renderer (Renut / deko3d) | 1 | 0.24 % | 91.5 % |
| 58 | `plume::Deko3DCommandList::setPipeline` | renderer (Renut / deko3d) | 1 | 0.24 % | 91.8 % |
| 59 | `plume::Deko3DCommandList::end` | renderer (Renut / deko3d) | 1 | 0.24 % | 92.0 % |
| 60 | `dkCmdBufDraw` | renderer (Renut / deko3d) | 1 | 0.24 % | 92.3 % |
| 61 | `nvIoctl2` | other host code | 1 | 0.24 % | 92.5 % |
| 62 | `renut::gpu::render_thread::Execute` | renderer (Renut / deko3d) | 1 | 0.24 % | 92.8 % |
| 63 | `renut::gpu::FillTextures` | renderer (Renut / deko3d) | 1 | 0.24 % | 93.0 % |
| 64 | `renut::gpu::FlushTextureFillsLocked` | renderer (Renut / deko3d) | 1 | 0.24 % | 93.2 % |
| 65 | `renut::gpu::RefreshLinearLocked` | renderer (Renut / deko3d) | 1 | 0.24 % | 93.5 % |
| 66 | `plume::Deko3DCommandList::setIndexBuffer` | renderer (Renut / deko3d) | 1 | 0.24 % | 93.7 % |
| 67 | `dkCmdBufBindVtxBufferState` | renderer (Renut / deko3d) | 1 | 0.24 % | 94.0 % |
| 68 | `nvGpuChannelKickoff` | other host code | 1 | 0.24 % | 94.2 % |
| 69 | `std::_Function_handler<void ` | renderer (Renut / deko3d) | 1 | 0.24 % | 94.4 % |
| 70 | `renut::gpu::Video::KickLocked` | renderer (Renut / deko3d) | 1 | 0.24 % | 94.7 % |
| 71 | `banjo::renut_gpu::Deko3DBarrierLeanOn` | renderer (Renut / deko3d) | 1 | 0.24 % | 94.9 % |
| 72 | `renut::gpu::EntryMemo<std::unordered_map<renut::gpu::IndexKey, renut::gpu::CachedIndices, ` | renderer (Renut / deko3d) | 1 | 0.24 % | 95.2 % |
| 73 | `rex::latency::NotePresented` | SDK (kernel, threads, audio) | 1 | 0.24 % | 95.4 % |
| 74 | `plume::Deko3DCommandList::drawInstanced` | renderer (Renut / deko3d) | 1 | 0.24 % | 95.7 % |
| 75 | `dkCmdBufBindRasterizerState` | renderer (Renut / deko3d) | 1 | 0.24 % | 95.9 % |
| 76 | `parcelTransact` | other host code | 1 | 0.24 % | 96.1 % |
| 77 | `_malloc_r` | other host code | 1 | 0.24 % | 96.4 % |
| 78 | `memcmp` | C library (mem/str) | 1 | 0.24 % | 96.6 % |
| 79 | `std::__detail::_Hash_node_base* std::_Hashtable<renut::gpu::StreamKey, std::pair<renut::gp` | renderer (Renut / deko3d) | 1 | 0.24 % | 96.9 % |
| 80 | `renut::gpu::AddressPtr` | renderer (Renut / deko3d) | 1 | 0.24 % | 97.1 % |
| 81 | `renut::gpu::FlushVertexFillsLocked` | renderer (Renut / deko3d) | 1 | 0.24 % | 97.3 % |
| 82 | `renut::gpu::CardsProve` | renderer (Renut / deko3d) | 1 | 0.24 % | 97.6 % |
| 83 | `__libc_cond_wait` | other host code | 1 | 0.24 % | 97.8 % |
| 84 | `plume::Deko3DCommandList::setFramebuffer` | renderer (Renut / deko3d) | 1 | 0.24 % | 98.1 % |
| 85 | `plume::Deko3DCommandList::begin` | renderer (Renut / deko3d) | 1 | 0.24 % | 98.3 % |
| 86 | `plume::Deko3DCommandList::flushDescriptorInvalidate` | renderer (Renut / deko3d) | 1 | 0.24 % | 98.6 % |
| 87 | `dkCmdBufBindTextures` | renderer (Renut / deko3d) | 1 | 0.24 % | 98.8 % |
| 88 | `dkCmdBufBindIdxBuffer` | renderer (Renut / deko3d) | 1 | 0.24 % | 99.0 % |
| 89 | `dkCmdBufPushConstants` | renderer (Renut / deko3d) | 1 | 0.24 % | 99.3 % |
| 90 | `banjo::renut_gpu::EagerKicksOn` | SDK (kernel, threads, audio) | 1 | 0.24 % | 99.5 % |
| 91 | `renut::gpu::Video::EnsureCommandListLocked` | renderer (Renut / deko3d) | 1 | 0.24 % | 99.8 % |
| 92 | `std::_Hashtable<renut::gpu::StreamKey, std::pair<renut::gpu::StreamKey const, renut::gpu::` | renderer (Renut / deko3d) | 1 | 0.24 % | 100.0 % |

## 4581 XThread14400EE0: every function (364 samples, CPU 18.0 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| machine-optimised (bulk list) | 32.1 % | 117 | 5 |
| generated, not optimised | 64.6 % | 235 | 5 |
| save/restore helpers | 0.3 % | 1 | 1 |
| SDK (kernel, threads, audio) | 0.5 % | 2 | 2 |
| locks / kernel calls | 0.5 % | 2 | 2 |
| other host code | 1.9 % | 7 | 4 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `sub_820D1028` | generated, not optimised | 199 | 54.67 % | 54.7 % |
| 2 | `sub_820CFB88` | machine-optimised (bulk list) | 101 | 27.75 % | 82.4 % |
| 3 | `sub_820E1950` | generated, not optimised | 26 | 7.14 % | 89.6 % |
| 4 | `sub_823868C8` | machine-optimised (bulk list) | 6 | 1.65 % | 91.2 % |
| 5 | `sub_82385740` | generated, not optimised | 5 | 1.37 % | 92.6 % |
| 6 | `sub_82386BB0` | machine-optimised (bulk list) | 5 | 1.37 % | 94.0 % |
| 7 | `sub_822CD248` | machine-optimised (bulk list) | 4 | 1.10 % | 95.1 % |
| 8 | `sub_822CFC40` | generated, not optimised | 4 | 1.10 % | 96.2 % |
| 9 | `_free_r` | other host code | 3 | 0.82 % | 97.0 % |
| 10 | `_malloc_r` | other host code | 2 | 0.55 % | 97.5 % |
| 11 | `__libc_lock_release` | other host code | 1 | 0.27 % | 97.8 % |
| 12 | `rmutexLock` | locks / kernel calls | 1 | 0.27 % | 98.1 % |
| 13 | `sub_82125F18` | machine-optimised (bulk list) | 1 | 0.27 % | 98.4 % |
| 14 | `rex::thread::WaitMultiple` | SDK (kernel, threads, audio) | 1 | 0.27 % | 98.6 % |
| 15 | `rex::thread::PosixConditionBase::WaitMultiple` | SDK (kernel, threads, audio) | 1 | 0.27 % | 98.9 % |
| 16 | `mutexUnlock` | locks / kernel calls | 1 | 0.27 % | 99.2 % |
| 17 | `sub_820CFB88` | generated, not optimised | 1 | 0.27 % | 99.5 % |
| 18 | `__restgprlr_27` | save/restore helpers | 1 | 0.27 % | 99.7 % |
| 19 | `std::chrono::_V2::steady_clock::now` | other host code | 1 | 0.27 % | 100.0 % |

## 4583 XThread21400A80: every function (342 samples, CPU 15.1 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| machine-optimised (bulk list) | 31.0 % | 106 | 6 |
| generated, not optimised | 67.3 % | 230 | 7 |
| SDK (kernel, threads, audio) | 0.3 % | 1 | 1 |
| C library (mem/str) | 0.3 % | 1 | 1 |
| locks / kernel calls | 0.3 % | 1 | 1 |
| other host code | 0.9 % | 3 | 3 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `sub_820D1028` | generated, not optimised | 204 | 59.65 % | 59.6 % |
| 2 | `sub_820CFB88` | machine-optimised (bulk list) | 94 | 27.49 % | 87.1 % |
| 3 | `sub_820E1950` | generated, not optimised | 21 | 6.14 % | 93.3 % |
| 4 | `sub_823868C8` | machine-optimised (bulk list) | 5 | 1.46 % | 94.7 % |
| 5 | `sub_82386BB0` | machine-optimised (bulk list) | 3 | 0.88 % | 95.6 % |
| 6 | `sub_822CD248` | machine-optimised (bulk list) | 2 | 0.58 % | 96.2 % |
| 7 | `sub_82386A10` | machine-optimised (bulk list) | 1 | 0.29 % | 96.5 % |
| 8 | `pthread_mutex_unlock` | other host code | 1 | 0.29 % | 96.8 % |
| 9 | `_free_r` | other host code | 1 | 0.29 % | 97.1 % |
| 10 | `memset` | C library (mem/str) | 1 | 0.29 % | 97.4 % |
| 11 | `__aarch64_read_tp` | other host code | 1 | 0.29 % | 97.7 % |
| 12 | `mutexLock` | locks / kernel calls | 1 | 0.29 % | 98.0 % |
| 13 | `sub_822CE568` | generated, not optimised | 1 | 0.29 % | 98.2 % |
| 14 | `sub_822CFAD0` | generated, not optimised | 1 | 0.29 % | 98.5 % |
| 15 | `sub_82386BB8` | generated, not optimised | 1 | 0.29 % | 98.8 % |
| 16 | `sub_823866B0` | machine-optimised (bulk list) | 1 | 0.29 % | 99.1 % |
| 17 | `sub_822CC9A0` | generated, not optimised | 1 | 0.29 % | 99.4 % |
| 18 | `sub_82385740` | generated, not optimised | 1 | 0.29 % | 99.7 % |
| 19 | `rex::kernel::xboxkrnl::xeNtSetEvent` | SDK (kernel, threads, audio) | 1 | 0.29 % | 100.0 % |

## 4582 XThread139FF120: every function (243 samples, CPU 11.8 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| hand native | 0.4 % | 1 | 1 |
| machine-optimised (bulk list) | 32.5 % | 79 | 4 |
| generated, not optimised | 62.6 % | 152 | 4 |
| save/restore helpers | 0.4 % | 1 | 1 |
| SDK (kernel, threads, audio) | 1.6 % | 4 | 2 |
| C library (mem/str) | 0.4 % | 1 | 1 |
| locks / kernel calls | 0.4 % | 1 | 1 |
| other host code | 1.6 % | 4 | 4 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `sub_820D1028` | generated, not optimised | 135 | 55.56 % | 55.6 % |
| 2 | `sub_820CFB88` | machine-optimised (bulk list) | 68 | 27.98 % | 83.5 % |
| 3 | `sub_820E1950` | generated, not optimised | 12 | 4.94 % | 88.5 % |
| 4 | `sub_82386BB0` | machine-optimised (bulk list) | 5 | 2.06 % | 90.5 % |
| 5 | `sub_82385740` | generated, not optimised | 4 | 1.65 % | 92.2 % |
| 6 | `sub_823868C8` | machine-optimised (bulk list) | 3 | 1.23 % | 93.4 % |
| 7 | `sub_820DDF60` | machine-optimised (bulk list) | 3 | 1.23 % | 94.7 % |
| 8 | `rex::thread::PosixConditionBase::WaitMultiple` | SDK (kernel, threads, audio) | 3 | 1.23 % | 95.9 % |
| 9 | `memset` | C library (mem/str) | 1 | 0.41 % | 96.3 % |
| 10 | `mutexUnlock` | locks / kernel calls | 1 | 0.41 % | 96.7 % |
| 11 | `sub_82448F40` | hand native | 1 | 0.41 % | 97.1 % |
| 12 | `rex::memory::BaseHeap::AllocRange` | SDK (kernel, threads, audio) | 1 | 0.41 % | 97.5 % |
| 13 | `sub_822CC750` | generated, not optimised | 1 | 0.41 % | 97.9 % |
| 14 | `__imp__NtSetEvent` | other host code | 1 | 0.41 % | 98.4 % |
| 15 | `__imp__RtlLeaveCriticalSection` | other host code | 1 | 0.41 % | 98.8 % |
| 16 | `__savefpr_26` | save/restore helpers | 1 | 0.41 % | 99.2 % |
| 17 | `RexSwitchPerfAdd` | other host code | 1 | 0.41 % | 99.6 % |
| 18 | `_free_r` | other host code | 1 | 0.41 % | 100.0 % |

## 4585 XThread3C800F80: every function (104 samples, CPU 4.6 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| machine-optimised (bulk list) | 79.8 % | 83 | 15 |
| generated, not optimised | 8.7 % | 9 | 7 |
| save/restore helpers | 1.0 % | 1 | 1 |
| SDK (kernel, threads, audio) | 4.8 % | 5 | 4 |
| locks / kernel calls | 1.9 % | 2 | 2 |
| other host code | 3.8 % | 4 | 3 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `sub_824014A8` | machine-optimised (bulk list) | 30 | 28.85 % | 28.8 % |
| 2 | `sub_82409650` | machine-optimised (bulk list) | 12 | 11.54 % | 40.4 % |
| 3 | `sub_82407530` | machine-optimised (bulk list) | 7 | 6.73 % | 47.1 % |
| 4 | `sub_824093E8` | machine-optimised (bulk list) | 7 | 6.73 % | 53.8 % |
| 5 | `sub_8240A160` | machine-optimised (bulk list) | 6 | 5.77 % | 59.6 % |
| 6 | `sub_82406498` | machine-optimised (bulk list) | 5 | 4.81 % | 64.4 % |
| 7 | `sub_823F6638` | machine-optimised (bulk list) | 4 | 3.85 % | 68.3 % |
| 8 | `sub_823FA480` | generated, not optimised | 3 | 2.88 % | 71.2 % |
| 9 | `sub_823FC450` | machine-optimised (bulk list) | 3 | 2.88 % | 74.0 % |
| 10 | `sub_824172E0` | machine-optimised (bulk list) | 2 | 1.92 % | 76.0 % |
| 11 | `sub_823F60A8` | machine-optimised (bulk list) | 2 | 1.92 % | 77.9 % |
| 12 | `rex::kernel::xboxkrnl::xeKeKfAcquireSpinLock` | SDK (kernel, threads, audio) | 2 | 1.92 % | 79.8 % |
| 13 | `RexSwitchPerfAdd` | other host code | 2 | 1.92 % | 81.7 % |
| 14 | `sub_823FBAE8` | machine-optimised (bulk list) | 1 | 0.96 % | 82.7 % |
| 15 | `sub_823FCC90` | generated, not optimised | 1 | 0.96 % | 83.7 % |
| 16 | `sub_824172E0` | generated, not optimised | 1 | 0.96 % | 84.6 % |
| 17 | `rex::thread::PosixConditionHandle<rex::thread::Event>::posix_wait_handle` | SDK (kernel, threads, audio) | 1 | 0.96 % | 85.6 % |
| 18 | `pthread_mutex_unlock` | other host code | 1 | 0.96 % | 86.5 % |
| 19 | `mutexTryLock` | locks / kernel calls | 1 | 0.96 % | 87.5 % |
| 20 | `sub_823F6F58` | machine-optimised (bulk list) | 1 | 0.96 % | 88.5 % |
| 21 | `sub_823F9048` | generated, not optimised | 1 | 0.96 % | 89.4 % |
| 22 | `sub_82417208` | generated, not optimised | 1 | 0.96 % | 90.4 % |
| 23 | `rex::system::XObject::GetNativeObject` | SDK (kernel, threads, audio) | 1 | 0.96 % | 91.3 % |
| 24 | `__imp__KfLowerIrql` | other host code | 1 | 0.96 % | 92.3 % |
| 25 | `sub_823F67F0` | machine-optimised (bulk list) | 1 | 0.96 % | 93.3 % |
| 26 | `__savegprlr_26` | save/restore helpers | 1 | 0.96 % | 94.2 % |
| 27 | `sub_82420398` | generated, not optimised | 1 | 0.96 % | 95.2 % |
| 28 | `mutexUnlock` | locks / kernel calls | 1 | 0.96 % | 96.2 % |
| 29 | `rex::kernel::xboxkrnl::xeKeSetEvent` | SDK (kernel, threads, audio) | 1 | 0.96 % | 97.1 % |
| 30 | `sub_823FC450` | generated, not optimised | 1 | 0.96 % | 98.1 % |
| 31 | `sub_82417208` | machine-optimised (bulk list) | 1 | 0.96 % | 99.0 % |
| 32 | `sub_823F68D8` | machine-optimised (bulk list) | 1 | 0.96 % | 100.0 % |

## XMA decoder: every function (62 samples, CPU 3.3 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| SDK (kernel, threads, audio) | 19.4 % | 12 | 4 |
| C library (mem/str) | 4.8 % | 3 | 2 |
| other host code | 75.8 % | 47 | 14 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `decode_frame` | other host code | 29 | 46.77 % | 46.8 % |
| 2 | `rex::audio::XmaContext::Work` | SDK (kernel, threads, audio) | 7 | 11.29 % | 58.1 % |
| 3 | `ff_imdct_half_neon` | other host code | 3 | 4.84 % | 62.9 % |
| 4 | `decode_receive_frame_internal` | other host code | 3 | 4.84 % | 67.7 % |
| 5 | `rex::audio::XmaContext::ConvertFrame` | SDK (kernel, threads, audio) | 3 | 4.84 % | 72.6 % |
| 6 | `memset` | C library (mem/str) | 2 | 3.23 % | 75.8 % |
| 7 | `_free_r` | other host code | 2 | 3.23 % | 79.0 % |
| 8 | `memmove` | C library (mem/str) | 1 | 1.61 % | 80.6 % |
| 9 | `rex::audio::XmaDecoder::WorkerThreadMain` | SDK (kernel, threads, audio) | 1 | 1.61 % | 82.3 % |
| 10 | `av_mallocz` | other host code | 1 | 1.61 % | 83.9 % |
| 11 | `ff_vector_fmul_scalar_neon` | other host code | 1 | 1.61 % | 85.5 % |
| 12 | `pthread_mutex_lock` | other host code | 1 | 1.61 % | 87.1 % |
| 13 | `std::_Hashtable<unsigned long, std::pair<unsigned long const, std::vector<unsigned int, st` | other host code | 1 | 1.61 % | 88.7 % |
| 14 | `_malloc_r` | other host code | 1 | 1.61 % | 90.3 % |
| 15 | `ff_vector_fmul_window_neon` | other host code | 1 | 1.61 % | 91.9 % |
| 16 | `av_packet_unref` | other host code | 1 | 1.61 % | 93.5 % |
| 17 | `rex::stream::BitStream::Copy` | SDK (kernel, threads, audio) | 1 | 1.61 % | 95.2 % |
| 18 | `fft16_neon` | other host code | 1 | 1.61 % | 96.8 % |
| 19 | `fft_pass_neon` | other host code | 1 | 1.61 % | 98.4 % |
| 20 | `ff_fft_calc_neon` | other host code | 1 | 1.61 % | 100.0 % |

## 4584 XThread3C800EA0: every function (61 samples, CPU 4.7 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| hand native | 1.6 % | 1 | 1 |
| generated, not optimised | 8.2 % | 5 | 1 |
| SDK (kernel, threads, audio) | 27.9 % | 17 | 6 |
| locks / kernel calls | 23.0 % | 14 | 5 |
| other host code | 39.3 % | 24 | 12 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `mutexUnlock` | locks / kernel calls | 8 | 13.11 % | 13.1 % |
| 2 | `rex::thread::PosixConditionBase::WaitMultiple` | SDK (kernel, threads, audio) | 6 | 9.84 % | 23.0 % |
| 3 | `_free_r` | other host code | 5 | 8.20 % | 31.1 % |
| 4 | `rex::system::util::ObjectTable::LookupObject` | SDK (kernel, threads, audio) | 5 | 8.20 % | 39.3 % |
| 5 | `sub_82147018` | generated, not optimised | 5 | 8.20 % | 47.5 % |
| 6 | `RexSwitchPerfAdd` | other host code | 3 | 4.92 % | 52.5 % |
| 7 | `pthread_mutex_unlock` | other host code | 3 | 4.92 % | 57.4 % |
| 8 | `_memalign_r` | other host code | 3 | 4.92 % | 62.3 % |
| 9 | `rmutexLock` | locks / kernel calls | 2 | 3.28 % | 65.6 % |
| 10 | `mutexTryLock` | locks / kernel calls | 2 | 3.28 % | 68.9 % |
| 11 | `rex::kernel::xboxkrnl::RtlLeaveCriticalSection_entry` | SDK (kernel, threads, audio) | 2 | 3.28 % | 72.1 % |
| 12 | `rex::system::XObject::WaitMultiple` | SDK (kernel, threads, audio) | 2 | 3.28 % | 75.4 % |
| 13 | `_malloc_r` | other host code | 2 | 3.28 % | 78.7 % |
| 14 | `pthread_mutex_lock` | other host code | 2 | 3.28 % | 82.0 % |
| 15 | `std::chrono::_V2::steady_clock::now` | other host code | 1 | 1.64 % | 83.6 % |
| 16 | `mutexIsLockedByCurrentThread` | locks / kernel calls | 1 | 1.64 % | 85.2 % |
| 17 | `rex::kernel::xboxkrnl::RtlEnterCriticalSection_raw` | SDK (kernel, threads, audio) | 1 | 1.64 % | 86.9 % |
| 18 | `rex::system::XThread::CheckTitleTermination` | SDK (kernel, threads, audio) | 1 | 1.64 % | 88.5 % |
| 19 | `rmutexUnlock` | locks / kernel calls | 1 | 1.64 % | 90.2 % |
| 20 | `__imp__NtSetEvent` | other host code | 1 | 1.64 % | 91.8 % |
| 21 | `__syscall_lock_acquire` | other host code | 1 | 1.64 % | 93.4 % |
| 22 | `__libc_lock_release_recursive` | other host code | 1 | 1.64 % | 95.1 % |
| 23 | `sub_82448F40` | hand native | 1 | 1.64 % | 96.7 % |
| 24 | `pthread_mutex_trylock` | other host code | 1 | 1.64 % | 98.4 % |
| 25 | `__syscall_clock_gettime` | other host code | 1 | 1.64 % | 100.0 % |

## Audio worker: every function (30 samples, CPU 1.7 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| generated, not optimised | 23.3 % | 7 | 5 |
| SDK (kernel, threads, audio) | 13.3 % | 4 | 4 |
| locks / kernel calls | 20.0 % | 6 | 3 |
| other host code | 43.3 % | 13 | 9 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `_free_r` | other host code | 4 | 13.33 % | 13.3 % |
| 2 | `mutexUnlock` | locks / kernel calls | 4 | 13.33 % | 26.7 % |
| 3 | `sub_823FAA08` | generated, not optimised | 2 | 6.67 % | 33.3 % |
| 4 | `pthread_mutex_unlock` | other host code | 2 | 6.67 % | 40.0 % |
| 5 | `sub_823F70D0` | generated, not optimised | 2 | 6.67 % | 46.7 % |
| 6 | `sub_8245B410` | generated, not optimised | 1 | 3.33 % | 50.0 % |
| 7 | `rex::thread::PosixCondition<rex::thread::Semaphore>::post_execution` | SDK (kernel, threads, audio) | 1 | 3.33 % | 53.3 % |
| 8 | `__imp__RtlEnterCriticalSection` | other host code | 1 | 3.33 % | 56.7 % |
| 9 | `rex::thread::ComputeAlertableWaitTimeout` | SDK (kernel, threads, audio) | 1 | 3.33 % | 60.0 % |
| 10 | `sub_823FABF0` | generated, not optimised | 1 | 3.33 % | 63.3 % |
| 11 | `pthread_self` | other host code | 1 | 3.33 % | 66.7 % |
| 12 | `sub_823FBEA8` | generated, not optimised | 1 | 3.33 % | 70.0 % |
| 13 | `rmutexLock` | locks / kernel calls | 1 | 3.33 % | 73.3 % |
| 14 | `pthread_mutex_trylock` | other host code | 1 | 3.33 % | 76.7 % |
| 15 | `rex::thread::PosixConditionBase::WaitMultiple` | SDK (kernel, threads, audio) | 1 | 3.33 % | 80.0 % |
| 16 | `rex::system::util::ObjectTable::LookupObject` | SDK (kernel, threads, audio) | 1 | 3.33 % | 83.3 % |
| 17 | `mutexTryLock` | locks / kernel calls | 1 | 3.33 % | 86.7 % |
| 18 | `_malloc_r` | other host code | 1 | 3.33 % | 90.0 % |
| 19 | `pthread_mutex_lock` | other host code | 1 | 3.33 % | 93.3 % |
| 20 | `__syscall_lock_try_acquire` | other host code | 1 | 3.33 % | 96.7 % |
| 21 | `__syscall_clock_gettime` | other host code | 1 | 3.33 % | 100.0 % |

## 4571 XThread20936C20: every function (23 samples, CPU 1.7 %)

| Kind | Share | Samples | Functions |
|---|---:|---:|---:|
| SDK (kernel, threads, audio) | 30.4 % | 7 | 3 |
| locks / kernel calls | 17.4 % | 4 | 3 |
| other host code | 52.2 % | 12 | 6 |

| # | Function | Kind | Samples | Share | Cumulative |
|---:|---|---|---:|---:|---:|
| 1 | `rex::kernel::xboxkrnl::xeNtWaitForMultipleObjectsEx` | SDK (kernel, threads, audio) | 3 | 13.04 % | 13.0 % |
| 2 | `pthread_mutex_trylock` | other host code | 3 | 13.04 % | 26.1 % |
| 3 | `rex::thread::PosixConditionBase::WaitMultiple` | SDK (kernel, threads, audio) | 3 | 13.04 % | 39.1 % |
| 4 | `_free_r` | other host code | 3 | 13.04 % | 52.2 % |
| 5 | `mutexLock` | locks / kernel calls | 2 | 8.70 % | 60.9 % |
| 6 | `__syscall_clock_gettime` | other host code | 2 | 8.70 % | 69.6 % |
| 7 | `_malloc_r` | other host code | 2 | 8.70 % | 78.3 % |
| 8 | `rex::kernel::xboxkrnl::RtlEnterCriticalSection_raw` | SDK (kernel, threads, audio) | 1 | 4.35 % | 82.6 % |
| 9 | `mutexUnlock` | locks / kernel calls | 1 | 4.35 % | 87.0 % |
| 10 | `mutexIsLockedByCurrentThread` | locks / kernel calls | 1 | 4.35 % | 91.3 % |
| 11 | `sched_yield` | other host code | 1 | 4.35 % | 95.7 % |
| 12 | `FLAGS_clock_no_scaling_storage_` | other host code | 1 | 4.35 % | 100.0 % |

