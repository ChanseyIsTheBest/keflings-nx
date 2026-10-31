// keflings-nx (2026-10-30): banjo-nx's base-game header (commit be8476a3: Banjo base's Direct3D layout, which Keflings
// shares), with the three natives whose functions Keflings links unchanged renamed to Keflings' addresses (each
// matched relocation-exact: tools/xdk_reloc_match.py): the C runtime memcpy n_82448F40 (Banjo base 82BB4510), memset
// n_82449470 (82BB4A40) and Direct3D's state flush of dirty register words n_821ED780 (8222CB90). The others keep
// Banjo's names and are not used (app/src/cpu/hot/hand.txt).
// banjo-nx - hand-written native versions of hot guest functions (CPU enhancements banjo_hand_*).
//
// The localized copies of tools/localize_native.py keep a function's own statements; these replace the statements:
// the XDK C runtime's memcpy and memset and Direct3D's two shader float-constant writers, written from what the
// routines do. They are sa2-nx's natives (app/src/cpu/hot/n_827F05C0.h, n_827EFEA0.h, n_d3d_constants.h, GPL-3.0;
// the memcpy came there from masseffect-nx's n_82AC4AF0.h): tools/crt_signatures.py of sa2-nx finds the same three C
// runtime routines in Banjo's image (unique matches), and the two constant writers are the same instructions with
// the same two offsets. Banjo's own additions are the two writers of Direct3D's state flush (namespace flush below).
// What is different here is how they are used and checked:
//
//   * Each is a template on the store policy of banjo_hot_common.h, so the guard's logged form records what it is
//     about to write like any generated copy (no hand-written list of written ranges).
//   * tools/localize_native.py puts them IN FRONT of the generated copy of the same function wherever that copy is
//     used: as a hook of its own, and inside every other function's copy that contains it (app/src/cpu/hot/hand.txt).
//     Inside a caller's copy the call is inlined: no hook, no hand-over of every register (on the Switch that
//     hand-over is what made copies with real calls slower than their originals).
//   * A native returns false to DECLINE a call it does not cover (nothing written yet): the generated copy of the
//     original then runs, in the same place.
//   * The registers the original leaves behind as scratch (hand.txt gives them per function; tools/localize_native.py
//     takes them from the original's own statements) are NOT reproduced: the natives write no register. Those
//     registers are left out of the guard's register compare for every hook whose copy contains the native. Sound
//     because they are caller-saved scratch of assembly routines and of a callee the callers reach through the
//     normal calling convention: a caller that read one after the call would get a different value, would act on
//     it, and the guard would see that in everything else it compares.
//
// ARM (the A57 is where this has to pay, and sa2-nx's first native Direct3D layer lost there for reasons no PC run
// showed: docs/api-renderer.md of sa2-nx, section 6): nothing here is per thread (no thread_local: a function call on
// the Switch), nothing copies a context, no loop keeps a count where a store could alias it, nothing is byte-swapped
// word by word (the copies are guest order to guest order), and sizes up to 128 bytes (memcpy; 64 for memset) are
// moved with overlapping 16/8/4/2-byte loads and stores without a call into the C library. In a PC run of title,
// menus, intro and gameplay 62 % of the 21.9 million memcpy calls were under 64 bytes and 21 % were 64 to 127; 46 %
// of the memsets were 4 to 7 bytes; 98 % of the 26 million constant uploads were 1 to 7 vectors.
//
// Guest function addresses may be named in this folder only (tools/direct_calls.py and tools/localize_native.py
// read every other source's addresses as hooked functions; these four are hooked, by app/src/cpu/hot/localized.txt).
#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>

#include <rex/ppc/context.h>

#if defined(__aarch64__)
#include <arm_neon.h>
#endif

#include "cpu/banjo_hot_common.h"

namespace banjo::hand {

// The natives' own keys (banjo_hot_guest.cpp), each read once: 0 not read yet, 1 off, 2 on.
enum : uint32_t {
  kMemcpy = 0,
  kMemset = 1,
  kConstants = 2,
  kFlush = 3,
  kAudioFilter = 4,
  kXAudioMix = 5,
  kGlyphWidth = 6,  // keflings-nx
  kWcscmp = 7,          // keflings-nx (2026-10-30, second round)
  kGlyphIndex = 8,      // keflings-nx
  kNthById = 9,         // keflings-nx
  kRecordSum = 10,      // keflings-nx
  kSphereFrustum = 11,  // keflings-nx
  kPairTest = 12,       // keflings-nx (2026-10-31, third round: the console profile's hottest copies)
  kSphereMerge = 13,    // keflings-nx
  kBoxMerge = 14,       // keflings-nx
  kMatrixMultiply = 15,  // keflings-nx
  kMatrixMultiplyVmx = 16,  // keflings-nx
  kTrackStep = 17,      // keflings-nx
  kBoxFrustum = 18,     // keflings-nx
  kNatives = 19
};
extern uint8_t g_state[kNatives];
uint8_t Init(uint32_t which);
[[gnu::always_inline]] inline bool On(uint32_t which) {
  const uint8_t state = g_state[which];
  if (state == 2) [[likely]] return true;
  return state == 0 && Init(which) == 2;
}

namespace detail {

// 16 bytes through a register (any alignment; one NEON load or store on the A57).
struct B16 {
  uint8_t b[16];
};
[[gnu::always_inline]] inline B16 L16(const uint8_t* p) {
  B16 v;
  std::memcpy(v.b, p, 16);
  return v;
}
[[gnu::always_inline]] inline void S16(uint8_t* p, const B16& v) { std::memcpy(p, v.b, 16); }

// 1 <= n <= 64; all loads before all stores (memmove semantics for dst <= src overlap).
[[gnu::always_inline]] inline void CopySmall(uint8_t* d, const uint8_t* s, uint32_t n) {
  if (n >= 16) {
    if (n > 32) {
      const B16 a = L16(s), b = L16(s + 16), c = L16(s + n - 32), e = L16(s + n - 16);
      S16(d, a);
      S16(d + 16, b);
      S16(d + n - 32, c);
      S16(d + n - 16, e);
    } else {
      const B16 a = L16(s), e = L16(s + n - 16);
      S16(d, a);
      S16(d + n - 16, e);
    }
  } else if (n >= 8) {
    uint64_t a, e;
    std::memcpy(&a, s, 8);
    std::memcpy(&e, s + n - 8, 8);
    std::memcpy(d, &a, 8);
    std::memcpy(d + n - 8, &e, 8);
  } else if (n >= 4) {
    uint32_t a, e;
    std::memcpy(&a, s, 4);
    std::memcpy(&e, s + n - 4, 4);
    std::memcpy(d, &a, 4);
    std::memcpy(d + n - 4, &e, 4);
  } else if (n >= 2) {
    uint16_t a, e;
    std::memcpy(&a, s, 2);
    std::memcpy(&e, s + n - 2, 2);
    std::memcpy(d, &a, 2);
    std::memcpy(d + n - 2, &e, 2);
  } else {
    *d = *s;
  }
}

// 64 < n <= 128; all loads before all stores. (Banjo's calls: a fifth are 64 to 127 bytes.)
[[gnu::always_inline]] inline void CopyMedium(uint8_t* d, const uint8_t* s, uint32_t n) {
  const B16 a = L16(s), b = L16(s + 16), c = L16(s + 32), e = L16(s + 48);
  const B16 f = L16(s + n - 64), g = L16(s + n - 48), h = L16(s + n - 32), i = L16(s + n - 16);
  S16(d, a);
  S16(d + 16, b);
  S16(d + 32, c);
  S16(d + 48, e);
  S16(d + n - 64, f);
  S16(d + n - 48, g);
  S16(d + n - 32, h);
  S16(d + n - 16, i);
}

// 1 <= n <= 64, every byte = v.
[[gnu::always_inline]] inline void FillSmall(uint8_t* d, uint8_t v, uint32_t n) {
  if (n >= 16) {
    B16 x;
    std::memset(x.b, v, 16);
    S16(d, x);
    S16(d + n - 16, x);
    if (n > 32) {
      S16(d + 16, x);
      S16(d + n - 32, x);
    }
  } else {
    const uint64_t x = uint64_t(v) * 0x0101010101010101ull;
    if (n >= 8) {
      std::memcpy(d, &x, 8);
      std::memcpy(d + n - 8, &x, 8);
    } else if (n >= 4) {
      std::memcpy(d, &x, 4);
      std::memcpy(d + n - 4, &x, 4);
    } else if (n >= 2) {
      std::memcpy(d, &x, 2);
      std::memcpy(d + n - 2, &x, 2);
    } else {
      *d = v;
    }
  }
}

// [a, a + n) is one stretch of host memory: it does not wrap around 4 GB and (on hosts with the physical-view
// offset) does not cross its boundary. n >= 1.
[[gnu::always_inline]] inline bool Straight(uint32_t a, uint32_t n) {
  if (uint64_t(a) + n > 0x100000000ull) return false;
  return ::banjo::hot::PhysOff(a) == ::banjo::hot::PhysOff(a + n - 1u);
}

}  // namespace detail

// sub_82BB4510 - XDK C runtime memcpy: r3 = dst, r4 = src, r5 = n (its 32-bit part); returns r3 as it came.
//
// The original stores r3 at -8(r1) first (a real store: it reloads r3 from there at every return), then copies
// FORWARD: a byte head up to 8-byte dst alignment, then 128-byte blocks / doublewords / words / bytes chosen by the
// alignment of src; dcbt / dcbtst are hints. Facts used:
//   * every size and address is the 32-bit part (cmplw, rlwinm, ea = u32 + u32);
//   * no loop reads past src + n or writes past dst + n;
//   * within each unit all loads precede its stores and units run upward: for dst <= src the result is memmove's,
//     for ranges that do not overlap a plain copy. For dst > src with dst - src < n the forward copy smears bytes it
//     already copied: DECLINED;
//   * n == 0 copies nothing (the save of r3 still happens).
// Also declined: ranges that wrap around 4 GB, and a destination that covers the save slot (the original would then
// return what was copied over it).
// Scratch the original leaves and this does not reproduce (hand.txt): r0, r4-r12.
namespace n_82448F40 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kMemcpy)) [[unlikely]] return false;
  const uint32_t dst = ctx.r3.u32, src = ctx.r4.u32, n = ctx.r5.u32, slot = ctx.r1.u32 - 8u;
  s.Hand(kMemcpy, n);
  if (n) {
    if (uint32_t(dst - src - 1u) < n - 1u) [[unlikely]] return false;  // dst > src, dst - src < n
    if (!detail::Straight(dst, n) || !detail::Straight(src, n)) [[unlikely]] return false;
    if (uint64_t(uint32_t(slot + 7u - dst)) < uint64_t(n) + 7u) [[unlikely]] return false;  // dst covers the slot
  }
  if (!s.Note(slot, 8)) [[unlikely]] return true;
  const uint64_t saved = __builtin_bswap64(ctx.r3.u64);
  std::memcpy(::banjo::hot::Raw(base, slot), &saved, 8);
  if (n == 0) return true;
  if (!s.Note(dst, n)) [[unlikely]] return true;
  ::banjo::hot::Card(base, dst, n);
  uint8_t* d = ::banjo::hot::Raw(base, dst);
  const uint8_t* p = ::banjo::hot::Raw(base, src);
  if (n <= 64) {
    detail::CopySmall(d, p, n);
  } else if (n <= 128) {
    detail::CopyMedium(d, p, n);
  } else if (uint32_t(src - dst) < n) {
    std::memmove(d, p, n);  // dst < src (or equal), overlapping: the forward copy is memmove
  } else {
    std::memcpy(d, p, n);
  }
  return true;
}

}  // namespace n_82448F40

// sub_82BB4A40 - XDK C runtime memset: r3 = dst, r4 = value (its low byte), r5 = n; r3 is not written.
//
// The original (ori r6,r3,0; ctr = n + 1): single bytes until r6 is 4-aligned or n is used up, the low byte of r4
// replicated into a word, 16-byte blocks of four stw, up to three stw, up to three stb. Every store writes that byte,
// each byte of [dst, dst + n) exactly once, nothing outside, and nothing is read. Facts used:
//   * ctr = r5 + 1 is 64-bit: with anything in the upper half of r5 the head loop could run past n: DECLINED;
//   * ranges that wrap around 4 GB are declined.
// Scratch not reproduced (hand.txt): r0, r4, r5, r6.
namespace n_82449470 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kMemset)) [[unlikely]] return false;
  if (ctx.r5.u64 >> 32) [[unlikely]] return false;
  const uint32_t dst = ctx.r3.u32, n = ctx.r5.u32;
  s.Hand(kMemset, n);
  if (n == 0) return true;
  if (!detail::Straight(dst, n)) [[unlikely]] return false;
  if (!s.Note(dst, n)) [[unlikely]] return true;
  ::banjo::hot::Card(base, dst, n);
  uint8_t* d = ::banjo::hot::Raw(base, dst);
  if (n <= 64) {
    detail::FillSmall(d, ctx.r4.u8, n);
  } else {
    std::memset(d, int(ctx.r4.u8), n);
  }
  return true;
}

}  // namespace n_82449470

// Direct3D's two shader float-constant writers (called for every constant upload of every draw):
//   sub_8222EFA8: r3 = device, r4 = first constant, r5 = source (any alignment), r6 = count of 16-byte vectors,
//                 r7 = 64-bit dirty bits. Destination ((r3 + (r4 + 120) * 16) & ~15) + 16 k; dirty mask at r3 + 0.
//   sub_8222F0D8: the same with (r4 + 376) and the dirty mask at r3 + 8.
// The originals copy four vectors per iteration, then one per iteration: lvlx + lvrx + vor is an unaligned 16-byte
// load (lvrx at an aligned address loads nothing), stvx128 an aligned store, dcbt are hints; then ld / or r7 / std on
// the dirty mask. Facts used:
//   * every address is the 32-bit part. The counters here are 32-bit compares (addic. / cmplwi), but r9 = r6 is
//     decremented as 64 bits: a count with anything in its upper half is DECLINED;
//   * the four-at-a-time loop loads its four vectors before it stores them: ranges that overlap are DECLINED, as are
//     ranges that wrap around 4 GB;
//   * the dirty mask is read after the copy, in the same order here;
//   * the originals' first instruction saves r31 at -8(r1). The generated code keeps r31 in a local of the function
//     (zero at that point, the context's r31 is never touched) and makes the store unless the build elides dead
//     saves (REX_NV_SAVES_ELIDE, off in the shipped build): eight zero bytes there, written first, as the original
//     does. (Should a later code generator keep r31 in the context instead, the guard reports the slot's bytes.)
// Scratch not reproduced (hand.txt): r4, r5, r6, r8-r11, v0, v10-v13.
namespace constants {

template <class S>
[[gnu::always_inline]] inline bool Upload(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s,
                                          uint32_t first, uint32_t mask_offset) {
  if (!On(kConstants)) [[unlikely]] return false;
  if (ctx.r6.u64 >> 32) [[unlikely]] return false;
  const uint32_t count = ctx.r6.u32, src = ctx.r5.u32, mask = ctx.r3.u32 + mask_offset;
  const uint32_t dst = (((ctx.r4.u32 + first) << 4) + ctx.r3.u32) & ~15u;
  s.Hand(kConstants, count);
  if (mask > 0xFFFFFFF7u) [[unlikely]] return false;
  if (count > 0x0FFFFFFFu) [[unlikely]] return false;
  const uint32_t bytes = count << 4;
  if (count) {
    if (!detail::Straight(dst, bytes) || !detail::Straight(src, bytes)) [[unlikely]] return false;
    if (uint32_t(src - dst) < bytes || uint32_t(dst - src) < bytes) [[unlikely]] return false;
  }
#if !defined(REX_NV_SAVES_ELIDE)
  {
    const uint32_t slot = ctx.r1.u32 - 8u;  // std r31,-8(r1), the local r31 (zero)
    if (!s.Note(slot, 8)) [[unlikely]] return true;
    std::memset(::banjo::hot::Raw(base, slot), 0, 8);
  }
#endif
  if (count) {
    if (!s.Note(dst, bytes)) [[unlikely]] return true;
    ::banjo::hot::Card(base, dst, bytes);
    uint8_t* d = ::banjo::hot::Raw(base, dst);
    const uint8_t* p = ::banjo::hot::Raw(base, src);
    if (count <= 8) {
      const uint8_t* const end = p + bytes;  // (a pointer cursor: no store here can alias the loop's bound)
      do {
        detail::S16(d, detail::L16(p));
        d += 16;
        p += 16;
      } while (p != end);
    } else {
      std::memcpy(d, p, bytes);
    }
  }
  if (!s.Note(mask, 8)) [[unlikely]] return true;
  ::banjo::hot::Card(base, mask, 8);
  uint64_t m;
  std::memcpy(&m, ::banjo::hot::Raw(base, mask), 8);
  m |= __builtin_bswap64(ctx.r7.u64);
  std::memcpy(::banjo::hot::Raw(base, mask), &m, 8);
  return true;
}

}  // namespace constants

namespace n_8222EFA8 {
template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, S& s) {
  return constants::Upload(ctx, base, s, 120, 0);
}
}  // namespace n_8222EFA8

namespace n_8222F0D8 {
template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, S& s) {
  return constants::Upload(ctx, base, s, 376, 8);
}
}  // namespace n_8222F0D8

// Direct3D's state flush, the two functions that turn dirty bits into PM4 packets in the command buffer (called by
// the flush of every draw, once per dirty block of state):
//   sub_8222CB90: r3 = device, r4 = 64 dirty bits (bit 63 first), r5 = register number of the first bit, r6 = the
//                 shadow words (word i belongs to bit i). For every run of set bits: a type-0 header
//                 ((length - 1) << 16 | register) and the run's words, appended after the write pointer at device + 48
//                 (which names the last word written); the new pointer is stored at the end.
//   sub_8222D0E8: the same for the float constants, one bit per block of four vectors (16 registers, 64 bytes): the
//                 run is first padded with type-2 packets (0x80000000) so that its vectors land 16-aligned (the
//                 original stores them with stvx), then header ((16 * blocks - 1) << 16 | register) and the blocks.
// Facts used:
//   * both reload the buffer's limit (device + 52) for every run and call Direct3D's "more space" function when a
//     run does not fit: a call in which a run could fail is DECLINED before anything is written (Runs below);
//   * no dirty bit at all is declined (the originals would then write a run of length 0 and loop);
//   * words go from guest memory to guest memory unchanged: no byte order is touched;
//   * the constants' source must be 16-aligned and the write pointer a multiple of 4 (the vector loads and stores
//     round their addresses down): otherwise declined;
//   * the buffer between the write pointer and its limit must not overlap the shadow words or the device's two
//     pointer words (declined), and no address may be near the top of the 4 GB;
//   * the originals' prologue links a stack frame (stwu r1,-144 / -176): the old stack pointer is stored at the new
//     one unless the build elides the back chain (REX_BACK_CHAIN_ELIDE, off in the shipped build). Their saves of the
//     non-volatile registers are not stores in the generated code (those registers are locals of the function).
// Scratch not reproduced (hand.txt): r4-r12, and v0, v11-v13 for the constants. r1 and r3 end as they came.
namespace flush {

[[gnu::always_inline]] inline uint32_t Load32(uint8_t* base, uint32_t a) {
  uint32_t v;
  std::memcpy(&v, ::banjo::hot::Raw(base, a), 4);
  return __builtin_bswap32(v);
}
[[gnu::always_inline]] inline void Store32(uint8_t* p, uint32_t v) {
  v = __builtin_bswap32(v);
  std::memcpy(p, &v, 4);
}
// [a, a + n) and [b, b + m) share a byte (n, m >= 1; neither wraps).
[[gnu::always_inline]] inline bool Overlap(uint32_t a, uint32_t n, uint32_t b, uint32_t m) {
  return uint64_t(a) < uint64_t(b) + m && uint64_t(b) < uint64_t(a) + n;
}

// kUnit: bytes per dirty bit (4: register words, 64: constant blocks). kFrame: the original's frame size.
//
// One pass, no register saved for it, and only the tests exactness needs. (The first version checked every run in a
// pass of its own and compared the exact ranges: 131 A57 instructions for a call with one short run, where the
// recompiled original takes 152 on average. A second, in one pass but with every pair of ranges tested, still took
// 119. Measured with tools/hot_arm_compare.py and the Switch build's disassembly; none of it showed on the PC.)
//   * the runs are written into the window between the write pointer and the buffer's limit, so that window as a
//     whole is what must not touch the 64 shadow units, the device's two pointer words or the frame word;
//   * a run that does not fit is where the original calls for more space. Whether any run could fail is decided
//     up front from the number of dirty bits alone: n bits make at most n runs, each at most 16 bytes of padding and
//     header besides its words, so with start + 16 n + (bytes of n units) + 12 below the limit every run passes the
//     original's own test and none of them is tested again. Otherwise the call is DECLINED before anything is
//     written (about 1 call in 1000 in a PC run; the original runs, with its call for space). (Handing the rest of
//     the bits to the original at the first run that fails is also exact, but those calls reach a call the guard
//     cannot repeat, so that path would have run unchecked.)
template <class S, uint32_t kUnit, uint32_t kFrame>
[[gnu::always_inline]] inline bool Runs(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kFlush)) [[unlikely]] return false;
  uint64_t m = ctx.r4.u64;
  if (m == 0) [[unlikely]] return false;
  const uint32_t device = ctx.r3.u32, reg0 = ctx.r5.u32, src = ctx.r6.u32, frame = ctx.r1.u32 - kFrame;
  constexpr uint32_t kShadow = 64u * kUnit;  // the shadow bytes 64 dirty bits can name (256 or 4096)
  // No address near the top of the 4 GB, so no sum below wraps.
  if (device > 0xFFFFE000u || frame > 0xFFFFE000u || src > 0xFFFFE000u) [[unlikely]] return false;
  if (kUnit == 64 && (src & 15u)) [[unlikely]] return false;
  // The original's first store, the frame's back chain, comes before it reads anything. Made here before the checks
  // that need the buffer's pointers: should one of them decline, the original starts by storing the same word.
#if !defined(REX_BACK_CHAIN_ELIDE)
  if (!s.Note(frame, 4)) [[unlikely]] return true;
  Store32(::banjo::hot::Raw(base, frame), ctx.r1.u32);  // stwu r1,-frame(r1)
#endif
  const uint32_t start = Load32(base, device + 48), limit = Load32(base, device + 52);
  constexpr uint32_t kWordsPerBit = kUnit / 4;
  const uint32_t bits = uint32_t(__builtin_popcountll(m));
  // Every run fits as the original judges it (above); this also says limit > start + 12.
  if (uint64_t(start) + 16u * bits + 4u * kWordsPerBit * bits + 12u >= limit) [[unlikely]] return false;
  if (limit > 0xFFFFFFE0u || (kUnit == 64 && (start & 3u))) [[unlikely]] return false;
  // Every word written lies in [start + 4, limit + 8): a run is only written when its last word is below limit.
  // That window must not touch the shadow units (they are read while it is written) nor the device's two pointer
  // words (the original reads the limit again for every run and stores the pointer at the end). The frame word
  // needs no test: it is written before everything else, here as there.
  const uint32_t window = limit - start + 8u;
  if (Overlap(start, window, src, kShadow) || Overlap(start, window, device + 48, 8)) [[unlikely]] return false;
  if (::banjo::hot::PhysOff(start) != ::banjo::hot::PhysOff(limit + 7u) ||
      ::banjo::hot::PhysOff(src) != ::banjo::hot::PhysOff(src + kShadow - 1u))
    [[unlikely]] return false;
  s.Hand(kFlush, bits);
  uint8_t* const buffer = ::banjo::hot::Raw(base, start);  // the word the write pointer names
  const uint8_t* const shadow = ::banjo::hot::Raw(base, src);
  uint32_t wp = start;
  uint32_t bit = 0;  // the dirty bit the top of m stands for
  do {
    const uint32_t zeros = uint32_t(__builtin_clzll(m));
    m <<= zeros;
    bit += zeros;
    const uint32_t length = ~m ? uint32_t(__builtin_clzll(~m)) : 64u;
    const uint32_t words = length * kWordsPerBit, bytes = words * 4u;
    // (sub_8222CB90's test, wp + 4 * length < limit, and sub_8222D0E8's, wp + 4 * (16 * blocks + 3) < limit, hold
    // for every run: decided above.)
    uint32_t pads = 0;
    if (kUnit == 64) {
      const uint32_t at = wp & 15u;  // 8: the header lands on 12 already
      pads = at == 4 ? 1u : at == 0 ? 2u : at == 12 ? 3u : 0u;
    }
    const uint32_t run_bytes = 4u * pads + 4u + bytes;
    if (!s.Note(wp + 4u, run_bytes)) [[unlikely]] return true;
    ::banjo::hot::Card(base, wp + 4u, run_bytes);
    uint8_t* d = buffer + (wp - start) + 4;
    for (uint32_t k = 0; k < pads; ++k) {
      Store32(d, 0x80000000u);
      d += 4;
    }
    Store32(d, (((words - 1u) << 16) & 0xFFFF0000u) | (reg0 + bit * kWordsPerBit));
    d += 4;
    const uint8_t* p = shadow + size_t(bit) * kUnit;
    if (bytes <= 64) {
      detail::CopySmall(d, p, bytes);
    } else {
      std::memcpy(d, p, bytes);
    }
    wp += run_bytes;
    bit += length;
    m = length == 64 ? 0 : m << length;
  } while (m);
  if (!s.Note(device + 48, 4)) [[unlikely]] return true;
  ::banjo::hot::Card(base, device + 48, 4);
  Store32(::banjo::hot::Raw(base, device + 48), wp);
  return true;
}

}  // namespace flush

namespace n_821ED780 {
template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, S& s) {
  return flush::Runs<S, 4, 144>(ctx, base, s);
}
}  // namespace n_821ED780

namespace n_8222D0E8 {
template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, S& s) {
  return flush::Runs<S, 64, 176>(ctx, base, s);
}
}  // namespace n_8222D0E8

// XAudio's per-voice state-variable filter, sub_82B0DCB8 (banjo-nx 2026-10-23, the user's request: hand-optimise the
// audio thread; the hottest guest function of that thread, a fifth of it in the console's city profile).
//   r3 = the filter, r4 = the voice's frame buffer. It calls XAudioFrameBuffer::GetProcessingData (0x826662E0), which
//   copies the buffer's words +12, +16, +20 to its stack frame (+80): byte +81 is the channel count, +88 the samples
//   (256 floats a channel, a channel every 1024 bytes). With byte filter+24 set, the six channels' state (low at
//   +32, band at +56, a float each) is zeroed first. Then for each channel and sample, with q at +28, frequency at
//   +80 and the mode at +20 (> 0 high-pass, < 0 low-pass, 0 band-pass):
//     d = x - low; high = -(fma(q, band, -d)); band += freq * high; low += freq * band; x = the mode's output
//   each a float result of a double multiply-add (as the generated code computes fnmsubs / fmadds).
// The original stores band, low and the sample at every sample and reloads q, the frequency, the mode and the buffer
// pointer every time, each a byte-swapped guest access (nothing it writes can alias them: declined otherwise). Here
// the state stays in registers and the channels run side by side, a scalar chain each (ChannelsN). Bit-identical: the
// same double operations in the same order, rounded to float where the original rounds (-0.0 included: the negation
// follows the multiply-add as there); flush to zero off as the original sets it.
// Stores made as the original's (its frame: LR save, the save slot of r31, a local of the copy so 0, the back chain,
// GetProcessingData's three words), so the guard compares everything. Declined: no buffer object, more than 6
// channels, a buffer that overlaps the filter's words or the frame, a range that is not one stretch of memory.
// Scratch not reproduced (hand.txt): r4-r12, f0, f10-f13. r1 ends as it came, r3 = 0.
namespace audio_filter {

[[gnu::always_inline]] inline uint32_t Load32(uint8_t* base, uint32_t a) {
  uint32_t v;
  std::memcpy(&v, ::banjo::hot::Raw(base, a), 4);
  return __builtin_bswap32(v);
}
[[gnu::always_inline]] inline float LoadF(const uint8_t* p) {
  uint32_t v;
  std::memcpy(&v, p, 4);
  v = __builtin_bswap32(v);
  float f;
  std::memcpy(&f, &v, 4);
  return f;
}
[[gnu::always_inline]] inline void StoreF(uint8_t* p, float f) {
  uint32_t v;
  std::memcpy(&v, &f, 4);
  v = __builtin_bswap32(v);
  std::memcpy(p, &v, 4);
}
[[gnu::always_inline]] inline void Store32(uint8_t* p, uint32_t v) {
  v = __builtin_bswap32(v);
  std::memcpy(p, &v, 4);
}
[[gnu::always_inline]] inline bool Overlap(uint32_t a, uint32_t n, uint32_t b, uint32_t m) {
  return uint64_t(a) < uint64_t(b) + m && uint64_t(b) < uint64_t(a) + n;
}

template <int kMode, uint32_t N>
[[gnu::always_inline]] inline void ChannelsN(uint8_t* samples, float q, float freq, float* low, float* band) {
  float l[N], b[N];
  for (uint32_t c = 0; c < N; ++c) {
    l[c] = low[c];
    b[c] = band[c];
  }
  // banjo-nx (2026-10-23, batch 6): high = -h with h = float(fma(q, band, -d)) (rounding is symmetric: float(-x) is
  // -float(x)), and freq * high = (-freq) * h exactly, so the band update takes h and -freq: the negation leaves the
  // chain each sample waits on (one instruction of its latency). The same values, -0.0 included; a NaN's sign is the
  // only bit that can differ, as with the reverb (real audio has none).
  const double dq = double(q), dfreq = double(freq), ndfreq = -dfreq;
  for (uint32_t i = 0; i < 256; ++i) {
    for (uint32_t c = 0; c < N; ++c) {
      uint8_t* at = samples + 1024u * c + 4u * i;
      const double d = double(LoadF(at) - l[c]);                        // fsubs
      const float h = float(std::fma(dq, double(b[c]), -d));            // fnmsubs before its negation
      b[c] = float(std::fma(ndfreq, double(h), double(b[c])));          // fmadds: freq * high + band
      l[c] = float(std::fma(dfreq, double(b[c]), double(l[c])));        // fmadds
      StoreF(at, kMode > 0 ? -h : kMode < 0 ? l[c] : b[c]);
    }
  }
  for (uint32_t c = 0; c < N; ++c) {
    low[c] = l[c];
    band[c] = b[c];
  }
}

template <int kMode>
[[gnu::always_inline]] inline void Channels(uint8_t* samples, uint32_t nch, float q, float freq, float* low,
                                            float* band) {
  switch (nch) {
  case 1: ChannelsN<kMode, 1>(samples, q, freq, low, band); break;
  case 2: ChannelsN<kMode, 2>(samples, q, freq, low, band); break;
  case 3: ChannelsN<kMode, 3>(samples, q, freq, low, band); break;
  case 4: ChannelsN<kMode, 4>(samples, q, freq, low, band); break;
  case 5: ChannelsN<kMode, 5>(samples, q, freq, low, band); break;
  default: ChannelsN<kMode, 6>(samples, q, freq, low, band); break;
  }
}

template <class S>
[[gnu::always_inline]] inline bool Run(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kAudioFilter)) [[unlikely]] return false;
  const uint32_t self = ctx.r3.u32, fb = ctx.r4.u32, sp = ctx.r1.u32, frame = sp - 112u;
  if (fb == 0 || self > 0xFFFFE000u || fb > 0xFFFFE000u || sp > 0xFFFFE000u || sp < 0x1000u) [[unlikely]]
    return false;
  // GetProcessingData's three words (+12, +16, +20 of the buffer object).
  const uint32_t w0 = Load32(base, fb + 12), w1 = Load32(base, fb + 16), samples = Load32(base, fb + 20);
  const uint32_t nch = (w0 >> 16) & 0xFFu;  // byte +81 of the frame: the word's second byte
  if (nch > 6) [[unlikely]] return false;
  if (::banjo::hot::PhysOff(self + 20) != ::banjo::hot::PhysOff(self + 83) ||
      ::banjo::hot::PhysOff(frame) != ::banjo::hot::PhysOff(sp - 1))
    [[unlikely]] return false;
  const uint32_t bytes = 1024u * nch;
  if (nch) {
    if (samples > 0xFFFFFFFFu - bytes || ::banjo::hot::PhysOff(samples) != ::banjo::hot::PhysOff(samples + bytes - 1) ||
        Overlap(samples, bytes, self + 20, 64) || Overlap(samples, bytes, frame, 112))
      [[unlikely]] return false;
  }
  s.Hand(kAudioFilter, nch);
  // The frame, as the original makes it (each store unless the build elides it).
#if !defined(REX_LR_MOVES_ELIDE)
  if (!s.Note(sp - 8, 4)) [[unlikely]] return true;
  Store32(::banjo::hot::Raw(base, sp - 8), uint32_t(ctx.lr));
#endif
#if !defined(REX_NV_SAVES_ELIDE)
  if (!s.Note(sp - 16, 8)) [[unlikely]] return true;
  std::memset(::banjo::hot::Raw(base, sp - 16), 0, 8);  // r31 is a local of the copy: its save writes 0
#endif
#if !defined(REX_BACK_CHAIN_ELIDE)
  if (!s.Note(frame, 4)) [[unlikely]] return true;
  Store32(::banjo::hot::Raw(base, frame), sp);
#endif
  if (!s.Note(frame + 80, 12)) [[unlikely]] return true;
  ::banjo::hot::Card(base, frame + 80, 12);
  uint8_t* const out = ::banjo::hot::Raw(base, frame + 80);
  Store32(out, w0);
  Store32(out + 4, w1);
  Store32(out + 8, samples);
  uint8_t* const f = ::banjo::hot::Raw(base, self);
  float low[6], band[6];
  if (f[24] != 0) {
    ctx.fpscr.disableFlushMode();
    if (!s.Note(self + 32, 48)) [[unlikely]] return true;
    ::banjo::hot::Card(base, self + 32, 48);
    std::memset(f + 32, 0, 48);  // the constant at 0x8213AE04: 0.0f
  }
  if (nch) {
    ctx.fpscr.disableFlushMode();
    for (uint32_t c = 0; c < nch; ++c) {
      low[c] = LoadF(f + 32 + 4 * c);
      band[c] = LoadF(f + 56 + 4 * c);
    }
    const float q = LoadF(f + 28), freq = LoadF(f + 80);
    const int32_t mode = int32_t(Load32(base, self + 20));
    if (!s.Note(samples, bytes) || !s.Note(self + 32, 4 * nch) || !s.Note(self + 56, 4 * nch)) [[unlikely]]
      return true;
    ::banjo::hot::Card(base, samples, bytes);
    ::banjo::hot::Card(base, self + 32, 4 * nch);
    ::banjo::hot::Card(base, self + 56, 4 * nch);
    uint8_t* const p = ::banjo::hot::Raw(base, samples);
    if (mode > 0)
      Channels<1>(p, nch, q, freq, low, band);
    else if (mode < 0)
      Channels<-1>(p, nch, q, freq, low, band);
    else
      Channels<0>(p, nch, q, freq, low, band);
    for (uint32_t c = 0; c < nch; ++c) {
      StoreF(f + 32 + 4 * c, low[c]);
      StoreF(f + 56 + 4 * c, band[c]);
    }
  }
  ctx.r3.u64 = 0;
  return true;
}

}  // namespace audio_filter

namespace n_82B0DCB8 {
template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, S& s) {
  return audio_filter::Run(ctx, base, s);
}
}  // namespace n_82B0DCB8

// XAudio's ramped channel mix, rex_XAudioMixFrameBufferChannel (banjo-nx 2026-10-23, batch 6, the user's request: the
// audio mixer further): the loop of XAUDIO::CRouterEffect::Process, a sixth of the audio thread in the console's city
// profile. r3 = the destination channel, r4 = the source channel (256 big-endian floats each), r5 = the gain at the
// first sample (a vector, address rounded down to 16), r6 = the gain step per sample (likewise). The original (hand
// written VMX): saves r31 at -8(r1) (a local of the copy, so 0: the save writes 0 unless the build elides it), sets the
// flush-to-zero mode, makes the four lanes' gains g0 = gain, g1 = g0 + s, g2 = g0 + 2s, g3 = g0 + (2s + s) and the step
// 4s = 2s + 2s, then for 16 blocks of 64 bytes loads the source's four vectors and the destination's four, computes
// dst_k = src_k * g_k + dst_k (vmaddfp as the generated code makes it: a multiply, then an add) and stores the four, and
// adds 4s to each gain. Here: the same operations on the same values, a block at a time in the same order (all of a
// block's loads before its stores: the same result where the channels overlap), each guest float byte-reversed once,
// the destination's store cards marked once for its 1024 bytes instead of at each vector store. Declined (the copy
// runs): a channel not 16-byte aligned, a range that is not one stretch of memory.
// Scratch not reproduced (hand.txt): r3-r10, v0-v13. r1 ends as it came.
namespace n_rex_XAudioMixFrameBufferChannel {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kXAudioMix)) [[unlikely]] return false;
  const uint32_t dst = ctx.r3.u32, src = ctx.r4.u32, gain = ctx.r5.u32 & ~15u, step = ctx.r6.u32 & ~15u;
  if (((dst | src) & 15u) || !detail::Straight(dst, 1024) || !detail::Straight(src, 1024) ||
      !detail::Straight(gain, 16) || !detail::Straight(step, 16)) [[unlikely]]
    return false;
  s.Hand(kXAudioMix, 256);
#if !defined(REX_NV_SAVES_ELIDE)
  {
    const uint32_t slot = ctx.r1.u32 - 8u;  // std r31,-8(r1): the local r31, zero
    if (!s.Note(slot, 8)) [[unlikely]] return true;
    std::memset(::banjo::hot::Raw(base, slot), 0, 8);
  }
#endif
  ctx.fpscr.enableFlushMode();
  if (!s.Note(dst, 1024)) [[unlikely]] return true;
  ::banjo::hot::Card(base, dst, 1024);
  uint8_t* d = ::banjo::hot::Raw(base, dst);
  const uint8_t* p = ::banjo::hot::Raw(base, src);
#if defined(__aarch64__)
  auto load = [](const uint8_t* at) { return vreinterpretq_f32_u8(vrev32q_u8(vld1q_u8(at))); };
  auto store = [](uint8_t* at, float32x4_t v) { vst1q_u8(at, vrev32q_u8(vreinterpretq_u8_f32(v))); };
  const float32x4_t sv = load(::banjo::hot::Raw(base, step));
  const float32x4_t s2 = vaddq_f32(sv, sv);
  float32x4_t g0 = load(::banjo::hot::Raw(base, gain));
  float32x4_t g1 = vaddq_f32(g0, sv);
  float32x4_t g2 = vaddq_f32(g0, s2);
  float32x4_t g3 = vaddq_f32(g0, vaddq_f32(s2, sv));
  const float32x4_t s4 = vaddq_f32(s2, s2);
  for (uint32_t block = 0; block < 16; ++block, d += 64, p += 64) {
    const float32x4_t a0 = load(p), a1 = load(p + 16), a2 = load(p + 32), a3 = load(p + 48);
    const float32x4_t b0 = load(d), b1 = load(d + 16), b2 = load(d + 32), b3 = load(d + 48);
    store(d, vaddq_f32(vmulq_f32(a0, g0), b0));
    store(d + 16, vaddq_f32(vmulq_f32(a1, g1), b1));
    store(d + 32, vaddq_f32(vmulq_f32(a2, g2), b2));
    store(d + 48, vaddq_f32(vmulq_f32(a3, g3), b3));
    g0 = vaddq_f32(g0, s4);
    g1 = vaddq_f32(g1, s4);
    g2 = vaddq_f32(g2, s4);
    g3 = vaddq_f32(g3, s4);
  }
#else
  // PC: lane by lane, each a float multiply then an add (the build does not contract them).
  auto load = [](const uint8_t* at, float out[4]) {
    for (uint32_t k = 0; k < 4; ++k)
      out[k] = audio_filter::LoadF(at + 4 * k);
  };
  float sv[4], g[4][4], s4[4];
  load(::banjo::hot::Raw(base, step), sv);
  load(::banjo::hot::Raw(base, gain), g[0]);
  for (uint32_t k = 0; k < 4; ++k) {
    const float s2 = sv[k] + sv[k];
    g[1][k] = g[0][k] + sv[k];
    g[2][k] = g[0][k] + s2;
    g[3][k] = g[0][k] + (s2 + sv[k]);
    s4[k] = s2 + s2;
  }
  for (uint32_t block = 0; block < 16; ++block, d += 64, p += 64) {
    float a[4][4], b[4][4];
    for (uint32_t v = 0; v < 4; ++v) {
      load(p + 16 * v, a[v]);
      load(d + 16 * v, b[v]);
    }
    for (uint32_t v = 0; v < 4; ++v) {
      for (uint32_t k = 0; k < 4; ++k) {
        const float product = a[v][k] * g[v][k];
        audio_filter::StoreF(d + 16 * v + 4 * k, product + b[v][k]);
        g[v][k] = g[v][k] + s4[k];
      }
    }
  }
#endif
  return true;
}

}  // namespace n_rex_XAudioMixFrameBufferChannel

// keflings-nx (2026-10-30): sub_820BC7A8 - the advance width of one character of a font, Keflings' hottest guest
// function in the PC profile (7.6 % of the main thread: the text layout asks it for every character of every string it
// measures). r3 = font, r4 = character code (its low 16 bits); returns r3 = int(scale * size * width + 0.5f), where
//   * width = the u16 at font+26 when the word at font+32 is 0 (one width for every character);
//   * otherwise 0 for codes 150..152, else the u16 at +8 of the 16-byte glyph entry i of the table at font+336, i =
//     the first index of the code in the u16 code table at font+332 (font+20 entries; 0 when it is not there);
//   * scale and size are the floats at font+356 and font+348.
// The original searches the code table with a byte-swapped load and compare per entry: this compares the table in
// guest order against the swapped code (one load and compare per entry), and writes back only r3 instead of the
// fourteen registers of the leaf. The arithmetic is the generated code's own expressions (fmuls, fcfid/frsp of the
// width, fmadds in double rounded to float, fctiwz), so the result is bit-identical; like the original it leaves the
// fctiwz doubleword at -16(r1). Declined: a code table that wraps around 4 GB or crosses the physical-view boundary.
// Scratch the original leaves and this does not reproduce (hand.txt): r7-r11, f0, f7-f13.
namespace n_820BC7A8 {

namespace detail {
[[gnu::always_inline]] inline uint32_t U16(uint8_t* base, uint32_t a) {
  uint16_t v;
  std::memcpy(&v, ::banjo::hot::Raw(base, a), 2);
  return __builtin_bswap16(v);
}
[[gnu::always_inline]] inline uint32_t U32(uint8_t* base, uint32_t a) {
  uint32_t v;
  std::memcpy(&v, ::banjo::hot::Raw(base, a), 4);
  return __builtin_bswap32(v);
}
[[gnu::always_inline]] inline double F32(uint8_t* base, uint32_t a) {
  const uint32_t v = U32(base, a);
  float f;
  std::memcpy(&f, &v, 4);
  return double(f);
}
}  // namespace detail

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kGlyphWidth)) [[unlikely]] return false;
  const uint32_t font = ctx.r3.u32;
  uint32_t width = 0;
  if (detail::U32(base, font + 32) == 0) {
    width = detail::U16(base, font + 26);
  } else if (const uint32_t code = ctx.r4.u32 & 0xFFFFu; code - 150u >= 3u) {
    const uint32_t n = detail::U32(base, font + 20);
    uint32_t i = 0;
    if (n) {
      const uint32_t codes = detail::U32(base, font + 332);
      if (n > 0x7FFFFFFFu || !::banjo::hand::detail::Straight(codes, n * 2u)) [[unlikely]] return false;
      const uint8_t* p = ::banjo::hot::Raw(base, codes);
      const uint16_t key = __builtin_bswap16(uint16_t(code));
      for (; i < n; ++i) {
        uint16_t v;
        std::memcpy(&v, p + 2u * i, 2);
        if (v == key) break;
      }
      if (i == n) i = 0;
    }
    width = detail::U16(base, detail::U32(base, font + 336) + (i << 4) + 8u);
  }
  ctx.fpscr.disableFlushMode();
  const double scale = detail::F32(base, font + 356), size = detail::F32(base, font + 348);
  const double product = double(float(scale) * float(size));                 // fmuls f10,f0,f13
  const double w = double(float(double(int64_t(int32_t(width)))));           // extsw, fcfid, frsp
  const double sum = double(float(std::fma(product, w, double(0.5f))));      // fmadds f8,f10,f9,f0
  const int64_t word = rex::ppc::fctiwz(sum);                                // fctiwz f7,f8
  const uint32_t slot = ctx.r1.u32 - 16u;
  if (!s.Note(slot, 8)) [[unlikely]] return true;
  const uint64_t saved = __builtin_bswap64(uint64_t(word));                  // stfd f7,-16(r1)
  std::memcpy(::banjo::hot::Raw(base, slot), &saved, 8);
  ctx.r3.u64 = uint32_t(uint64_t(word));                                     // lwz r3,-12(r1)
  return true;
}

}  // namespace n_820BC7A8

// keflings-nx (2026-10-30, second round): five more of the PC profile's hot game functions (the user's request), each
// a leaf whose generated form loads and byte-swaps every value and writes every register back. Each keeps the
// generated code's own expressions (the same loads, the same 32-bit address arithmetic, the same float expressions),
// writes back r3 alone (hand.txt: exact=r3) and leaves the scratch registers as they were; they are compared with
// the generated copies by the guards (banjo_cpu_verify) like every other native.
namespace kef {
[[gnu::always_inline]] inline uint32_t U8(uint8_t* base, uint32_t a) { return *::banjo::hot::Raw(base, a); }
[[gnu::always_inline]] inline uint32_t U16(uint8_t* base, uint32_t a) {
  uint16_t v;
  std::memcpy(&v, ::banjo::hot::Raw(base, a), 2);
  return __builtin_bswap16(v);
}
[[gnu::always_inline]] inline uint16_t U16Raw(uint8_t* base, uint32_t a) {  // guest byte order
  uint16_t v;
  std::memcpy(&v, ::banjo::hot::Raw(base, a), 2);
  return v;
}
[[gnu::always_inline]] inline uint32_t U32(uint8_t* base, uint32_t a) {
  uint32_t v;
  std::memcpy(&v, ::banjo::hot::Raw(base, a), 4);
  return __builtin_bswap32(v);
}
[[gnu::always_inline]] inline double F32(uint8_t* base, uint32_t a) {
  const uint32_t v = U32(base, a);
  float f;
  std::memcpy(&f, &v, 4);
  return double(f);
}
// keflings-nx (2026-10-31, third round): a single as lfs loads it, a single's bits, a store with its log entry and
// store cards (false: the run is over, the native returns true as the others do).
[[gnu::always_inline]] inline float Fl(uint8_t* base, uint32_t a) {
  const uint32_t v = U32(base, a);
  float f;
  std::memcpy(&f, &v, 4);
  return f;
}
[[gnu::always_inline]] inline uint32_t Bits(float f) {
  uint32_t v;
  std::memcpy(&v, &f, 4);
  return v;
}
// stfs of a value an lfs loaded: the two conversions quiet a signalling NaN (rex::ppc::QuietSingleBits).
[[gnu::always_inline]] inline uint32_t Quiet(uint32_t b) {
  return (b & 0x7FFFFFFFu) > 0x7F800000u ? (b | 0x00400000u) : b;
}
template <class S>
[[gnu::always_inline]] inline bool Put32(S& s, uint8_t* base, uint32_t a, uint32_t v) {
  if (!s.Note(a, 4)) [[unlikely]] return false;
  const uint32_t swapped = __builtin_bswap32(v);
  std::memcpy(::banjo::hot::Raw(base, a), &swapped, 4);
  ::banjo::hot::Card(base, a, 4);
  return true;
}
template <class S>
[[gnu::always_inline]] inline bool Put8(S& s, uint8_t* base, uint32_t a, uint8_t v) {
  if (!s.Note(a, 1)) [[unlikely]] return false;
  *::banjo::hot::Raw(base, a) = v;
  ::banjo::hot::Card(base, a, 1);
  return true;
}
// Stack stores below r1 (no store cards, as the generated code's stack stores): the original's register saves and
// scratch copies, reproduced because the guard compares every byte a function writes.
template <class S>
[[gnu::always_inline]] inline bool Stack32(S& s, uint8_t* base, uint32_t a, uint32_t v) {
  if (!s.Note(a, 4)) [[unlikely]] return false;
  const uint32_t swapped = __builtin_bswap32(v);
  std::memcpy(::banjo::hot::Raw(base, a), &swapped, 4);
  return true;
}
template <class S>
[[gnu::always_inline]] inline bool Stack64(S& s, uint8_t* base, uint32_t a, uint64_t v) {
  if (!s.Note(a, 8)) [[unlikely]] return false;
  const uint64_t swapped = __builtin_bswap64(v);
  std::memcpy(::banjo::hot::Raw(base, a), &swapped, 8);
  return true;
}
// fmadds as the generated code has it: the double fma of the operands, rounded to single once.
[[gnu::always_inline]] inline float Fmadds(float a, float b, float c) {
  return float(std::fma(double(a), double(b), double(c)));
}
}  // namespace kef

// sub_82447298 - the C runtime's wcscmp (UTF-16, guest byte order): r3, r4 = the strings; returns r3 = -1, 0 or 1
// (sign-extended: li). The original loads both characters byte-swapped, subtracts, and stops at the first
// difference or at the end of r4's string. This compares the characters as they lie in memory (equality needs no
// swap) and swaps only the pair that decides the sign. Reads exactly the characters the original reads.
// Scratch the original leaves and this does not reproduce (hand.txt): r4, r10, r11.
namespace n_82447298 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kWcscmp)) [[unlikely]] return false;
  uint32_t a = ctx.r3.u32, b = ctx.r4.u32;
  int32_t d = 0;
  for (;;) {
    const uint16_t cb = kef::U16Raw(base, b), ca = kef::U16Raw(base, a);
    if (ca != cb) {
      d = int32_t(__builtin_bswap16(ca)) - int32_t(__builtin_bswap16(cb));
      break;
    }
    if (cb == 0) break;
    a += 2u;
    b += 2u;
  }
  ctx.r3.s64 = d < 0 ? -1 : d > 0 ? 1 : 0;
  return true;
}

}  // namespace n_82447298

// sub_820BCB70 - a font's glyph index of one character (the text layout's other callee; the search of
// n_820BC7A8): r3 = font, r4 = character code. Byte +460 0: returns r4 as it came. Otherwise i = the first index of
// the code (r4's low 16 bits) in the u16 table at font+332 (font+20 entries; 0 when absent) and the result is the u16
// at font+456's table + 2i. Declined: a code table that wraps around 4 GB or crosses the physical-view boundary.
// Scratch the original leaves and this does not reproduce (hand.txt): r7-r11.
namespace n_820BCB70 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kGlyphIndex)) [[unlikely]] return false;
  const uint32_t font = ctx.r3.u32;
  if (kef::U8(base, font + 460) == 0) {
    ctx.r3.u64 = ctx.r4.u64;
    return true;
  }
  const uint32_t n = kef::U32(base, font + 20);
  uint32_t i = 0;
  if (n) {
    const uint32_t codes = kef::U32(base, font + 332);
    if (n > 0x7FFFFFFFu || !::banjo::hand::detail::Straight(codes, n * 2u)) [[unlikely]] return false;
    const uint8_t* p = ::banjo::hot::Raw(base, codes);
    const uint16_t key = __builtin_bswap16(uint16_t(ctx.r4.u32 & 0xFFFFu));
    for (; i < n; ++i) {
      uint16_t v;
      std::memcpy(&v, p + 2u * i, 2);
      if (v == key) break;
    }
    if (i == n) i = 0;
  }
  ctx.r3.u64 = kef::U16(base, kef::U32(base, font + 456) + uint32_t(i << 1));
  return true;
}

}  // namespace n_820BCB70

// sub_82180D78 - the index of the r5-th (0-based) element whose word +104 equals r4 in a list: r3 = owner, the list
// at *(r3+4): count at +4, element size at +8, element pointers' array at +16 (element i's pointer at
// array + size * i, 32-bit arithmetic). Returns r3 = that index, or -1 (sign-extended) when there are fewer.
// Scratch the original leaves and this does not reproduce (hand.txt): r6-r11.
namespace n_82180D78 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kNthById)) [[unlikely]] return false;
  const uint32_t list = kef::U32(base, ctx.r3.u32 + 4u);
  const int32_t count = int32_t(kef::U32(base, list + 4u));
  const int32_t id = ctx.r4.s32, want = ctx.r5.s32;
  int32_t seen = 0;
  for (int32_t i = 0; i < count; ++i) {
    // i >= 0 here, as the original's own test finds every time (its other side loads from element 0).
    const uint32_t size = kef::U32(base, list + 8u), array = kef::U32(base, list + 16u);
    const uint32_t element = kef::U32(base, uint32_t(int64_t(int32_t(size)) * int64_t(i)) + array);
    if (int32_t(kef::U32(base, element + 104u)) == id) {
      if (want == seen) {
        ctx.r3.s64 = i;
        return true;
      }
      ++seen;
    }
  }
  ctx.r3.s64 = -1;
  return true;
}

}  // namespace n_82180D78

// sub_820CC0E0 - the sum of the words at +52 of a table of 88-byte records: r3 = owner; the count is the word +16 of
// *(r3+224), the records start at *(r3+264). Returns r3 = the sum as the original keeps it (64-bit: the words are
// added zero-extended into the doubleword), 0 when the count is not above 0.
// Scratch the original leaves and this does not reproduce (hand.txt): r9-r11 (and XER's carry).
namespace n_820CC0E0 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kRecordSum)) [[unlikely]] return false;
  const uint32_t owner = ctx.r3.u32;
  const int32_t count = int32_t(kef::U32(base, kef::U32(base, owner + 224u) + 16u));
  uint64_t sum = 0;
  if (count > 0) {
    uint32_t p = kef::U32(base, owner + 264u) + 52u;
    for (int32_t k = 0; k < count; ++k, p += 88u) sum += kef::U32(base, p);
  }
  ctx.r3.u64 = sum;
  return true;
}

}  // namespace n_820CC0E0

// sub_820D9A48 - a sphere against the six planes of a frustum: r3 = frustum (byte +116: planes valid; plane i at
// +20 + 16i: nx, ny, nz, d), r4 = centre (x, y, z), f1 = radius, r6 = where to store the last plane's distance + the
// radius (or 0). Returns r3 = 0 (outside a plane), 1 (crosses a plane), 2 (inside all, or no planes). Each distance
// is the generated code's expression: fmuls nz*z, fmadds nx*x, fmadds y*ny (double fma rounded to float), fadds d;
// the compares are against -f1 and f1 in double; a NaN distance neither leaves nor crosses (as bge/blt branch).
// Scratch the original leaves and this does not reproduce (hand.txt): f0, f4-f13, r9-r11.
namespace n_820D9A48 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kSphereFrustum)) [[unlikely]] return false;
  const uint32_t frustum = ctx.r3.u32;
  if (kef::U8(base, frustum + 116u) == 0) {
    ctx.r3.u64 = 2;
    return true;
  }
  ctx.fpscr.disableFlushMode();
  const uint32_t centre = ctx.r4.u32;
  const double z = kef::F32(base, centre + 8u), y = kef::F32(base, centre + 4u), x = kef::F32(base, centre + 0u);
  const double r = ctx.f1.f64, neg_r = -r;
  bool crosses = false;
  double dist = 0.0;
  for (uint32_t i = 0; i < 6; ++i) {
    const uint32_t plane = frustum + 20u + 16u * i;
    const double nz = kef::F32(base, plane + 8u);
    const double t9 = double(float(nz) * float(z));                     // fmuls f9,f0,f13
    const double nx = kef::F32(base, plane + 0u), ny = kef::F32(base, plane + 4u), d = kef::F32(base, plane + 12u);
    const double t5 = double(float(std::fma(nx, x, t9)));              // fmadds f5,f8,f11,f9
    const double t4 = double(float(std::fma(y, ny, t5)));              // fmadds f4,f12,f7,f5
    dist = double(float(t4) + float(d));                               // fadds f0,f4,f6
    if (dist < neg_r) {
      ctx.r3.u64 = 0;
      return true;
    }
    if (dist < r) crosses = true;
  }
  if (crosses) {
    ctx.r3.u64 = 1;
    return true;
  }
  const uint32_t out = ctx.r6.u32;
  if (out != 0) {
    const float v = float(dist + r);  // double(float(f0 + f1)): the double sum rounded once
    uint32_t bits;
    std::memcpy(&bits, &v, 4);
    if (!s.Note(out, 4)) [[unlikely]] return true;
    const uint32_t swapped = __builtin_bswap32(bits);
    std::memcpy(::banjo::hot::Raw(base, out), &swapped, 4);
    ::banjo::hot::Card(base, out, 4);
  }
  ctx.r3.u64 = 2;
  return true;
}

}  // namespace n_820D9A48

// keflings-nx (2026-10-31, third round): the console profile's hottest copies (docs/console-profile-2026-10-30.md).
// The collision broad phase sub_82149C30 (the main thread's 1st) spends its time in the pair test sub_8214A1D0 (2nd)
// and in its two bounds leaves sub_82149560 / sub_82149B08 (inlined into its copy); the draw's matrices sub_820DB9F8
// (3rd) in the VMX matrix product sub_821FCD50 (inlined); the model update sub_820CC638 (4th) in the track step
// sub_820C96C0 (inlined); sub_8213AA78 (7th) is the scalar matrix product; sub_820BF340 is the box-against-frustum
// test of the worker threads. Each keeps its function's generated float expressions, bit-identical.

// sub_8214A1D0 - the pair test of two collision objects (r4, r5; r3 the tree, r6 / r7 indices): returns r3 = 0, or 1
// when they touch. Every pair of a node's objects comes here, and nearly every pair leaves at once: a flag test, the
// object's list of group ids, or the two bounding spheres apart (distance squared not below the radii's sum squared
// and not 0). Those exits read and return here, without the original's frame and its save / restore of r24-r31.
// Spheres that touch (recursion into the sub-objects, the narrow test sub_821492A0, the contact marks): the copy.
// Scratch the original leaves and this does not reproduce (hand.txt): r8-r11, f0-f13 of the sphere test.
namespace n_8214A1D0 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kPairTest)) [[unlikely]] return false;
  const uint32_t a = ctx.r4.u32, b = ctx.r5.u32;
  // The original's frame as it writes it: __savegprlr_24 (r24-r31 at r1 - 72 .. - 16, the link register at r1 - 8),
  // then stwu r1,-160(r1) (the back chain). A touching pair's copy writes them again, the same.
  {
    const uint32_t sp = ctx.r1.u32;
    const uint64_t saved[8] = {ctx.r24.u64, ctx.r25.u64, ctx.r26.u64, ctx.r27.u64,
                               ctx.r28.u64, ctx.r29.u64, ctx.r30.u64, ctx.r31.u64};
    for (uint32_t k = 0; k < 8; ++k) {
      if (!kef::Stack64(s, base, sp - 72u + 8u * k, saved[k])) [[unlikely]] return true;
    }
    if (!kef::Stack32(s, base, sp - 8u, uint32_t(ctx.lr))) [[unlikely]] return true;
    if (!kef::Stack32(s, base, sp - 160u, sp)) [[unlikely]] return true;
  }
  const uint32_t gb = kef::U32(base, b + 48u);
  if (kef::U32(base, a + 48u) == gb && kef::U32(base, a + 40u) == 0 && kef::U32(base, b + 40u) == 0) {
    ctx.r3.u64 = 0;
    return true;
  }
  const uint32_t fb = kef::U16(base, b + 28u);
  if (!(fb & 1u) || !((fb >> 2) & 1u) || (kef::U16(base, a + 30u) & kef::U16(base, b + 30u))) {
    ctx.r3.u64 = 0;
    return true;
  }
  const uint32_t fa = kef::U16(base, a + 28u);
  if (!((fa >> 5) & 1u) && !((fb >> 5) & 1u)) {
    // The group ids a + 60 on (count a + 56): 1 when b's group is among them.
    const uint32_t count = kef::U32(base, a + 56u);
    for (uint32_t i = 0; i < count; ++i) {
      if (kef::U32(base, a + 60u + 4u * i) == gb) {
        ctx.r3.u64 = 1;
        return true;
      }
    }
    ctx.r3.u64 = 0;
    return true;
  }
  ctx.fpscr.disableFlushMode();
  const float dy = kef::Fl(base, b + 20u) - kef::Fl(base, a + 20u);   // fsubs f12,f0,f13
  const float dx = kef::Fl(base, b + 16u) - kef::Fl(base, a + 16u);   // fsubs f9,f11,f10
  const float radii = kef::Fl(base, a + 0u) + kef::Fl(base, b + 0u);  // fadds f5,f7,f8
  const float dz = kef::Fl(base, b + 24u) - kef::Fl(base, a + 24u);   // fsubs f3,f6,f4
  const float yy = dy * dy;                                           // fmuls f2,f12,f12
  const float rr = radii * radii;                                     // fmuls f1,f5,f5
  const float d2 = kef::Fmadds(dz, dz, kef::Fmadds(dx, dx, yy));     // fmadds f0 / f13
  if (d2 < rr || d2 == 0.0f) return false;  // they touch: the copy
  ctx.r3.u64 = 0;
  return true;
}

}  // namespace n_8214A1D0

// sub_82149560 - a node's bounding sphere (centre r3, radius at r4) made to hold another sphere (centre r5, radius
// f1). The original saves and restores f24-f31 at every call. Here: radius 0 (nothing), the other sphere already
// inside (nothing), the node's sphere inside the other (the other's centre and radius copied, word by word as the
// original's lwz / stw); a real merge: the copy. Scratch not reproduced (hand.txt): f0, f2-f13.
namespace n_82149560 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kSphereMerge)) [[unlikely]] return false;
  // The original's saves: stw r12 (the link register) at r1 - 8, then __savefpr_24 (f24-f31 at r1 - 72 .. - 16).
  {
    const uint32_t sp = ctx.r1.u32;
    if (!kef::Stack32(s, base, sp - 8u, uint32_t(ctx.lr))) [[unlikely]] return true;
    const uint64_t saved[8] = {ctx.f24.u64, ctx.f25.u64, ctx.f26.u64, ctx.f27.u64,
                               ctx.f28.u64, ctx.f29.u64, ctx.f30.u64, ctx.f31.u64};
    for (uint32_t k = 0; k < 8; ++k) {
      if (!kef::Stack64(s, base, sp - 72u + 8u * k, saved[k])) [[unlikely]] return true;
    }
  }
  ctx.fpscr.disableFlushMode();
  const double radius = ctx.f1.f64;
  if (radius == 0.0) return true;  // fcmpu f1, 0.0: beq
  const uint32_t c = ctx.r3.u32, pr = ctx.r4.u32, o = ctx.r5.u32;
  const float f5 = kef::Fl(base, c + 4u) - kef::Fl(base, o + 4u);  // fsubs f5,f8,f12
  const float f4 = kef::Fl(base, c + 8u) - kef::Fl(base, o + 8u);  // fsubs f4,f7,f11
  const float f6 = kef::Fl(base, c + 0u) - kef::Fl(base, o + 0u);  // fsubs f6,f10,f9
  const float node = kef::Fl(base, pr);                            // f13
  const float f2 = kef::Fmadds(f6, f6, kef::Fmadds(f4, f4, f5 * f5));
  const float dist = __builtin_sqrtf(f2);                          // fsqrts f3,f2
  if (double(dist + float(radius)) <= double(node)) return true;   // fadds f0,f3,f1; ble: inside already
  if (double(node + dist) > radius) return false;                  // fadds f0,f13,f3; bgt: a real merge
  for (uint32_t k = 0; k < 12u; k += 4u) {
    if (!kef::Put32(s, base, c + k, kef::U32(base, o + k))) [[unlikely]] return true;
  }
  if (!kef::Put32(s, base, pr, kef::Bits(float(radius)))) [[unlikely]] return true;  // stfs f1
  return true;
}

}  // namespace n_82149560

// sub_82149B08 - a node's box (r3 + 8 .. + 28: min x y z, max x y z) made to hold an object's (r4): its own box
// (r4 + 40) or, without one, its centre (r4 + 16 .. + 24) as a point; nothing when the object's flag bit 3 is clear.
// The same compares and stores in the same order (the box pointer read again after each store, as the original
// does); a stored value is the loaded single, a signalling NaN quieted as lfs / stfs do. The original's stack copies
// of the centre (-16 .. -8 below r1) are not made (nothing reads them). Scratch not reproduced: r9-r11, f0, f11-f13.
namespace n_82149B08 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kBoxMerge)) [[unlikely]] return false;
  const uint32_t node = ctx.r3.u32, obj = ctx.r4.u32;
  if (!((kef::U16(base, obj + 28u) >> 3) & 1u)) return true;
  ctx.fpscr.disableFlushMode();
  // Lower bounds take a smaller value (bge skips), upper bounds a larger one (ble skips); a NaN is stored.
  auto lower = [&](uint32_t at, uint32_t bits) {
    float v, cur = kef::Fl(base, node + at);
    std::memcpy(&v, &bits, 4);
    return (v >= cur) || kef::Put32(s, base, node + at, kef::Quiet(bits));
  };
  auto upper = [&](uint32_t at, uint32_t bits) {
    float v, cur = kef::Fl(base, node + at);
    std::memcpy(&v, &bits, 4);
    return (v <= cur) || kef::Put32(s, base, node + at, kef::Quiet(bits));
  };
  if (kef::U32(base, obj + 40u) != 0) {
    if (!lower(8u, kef::U32(base, kef::U32(base, obj + 40u) + 8u))) return true;
    if (!lower(12u, kef::U32(base, kef::U32(base, obj + 40u) + 12u))) return true;
    if (!lower(16u, kef::U32(base, kef::U32(base, obj + 40u) + 16u))) return true;
    if (!upper(20u, kef::U32(base, kef::U32(base, obj + 40u) + 20u))) return true;
    if (!upper(24u, kef::U32(base, kef::U32(base, obj + 40u) + 24u))) return true;
    (void)upper(28u, kef::U32(base, kef::U32(base, obj + 40u) + 28u));
    return true;
  }
  const uint32_t x = kef::U32(base, obj + 16u), y = kef::U32(base, obj + 20u), z = kef::U32(base, obj + 24u);
  // The original's copy of the centre at r1 - 16 .. - 8 (stw, read back with lfs).
  const uint32_t sp = ctx.r1.u32;
  if (!kef::Stack32(s, base, sp - 16u, x) || !kef::Stack32(s, base, sp - 12u, y) ||
      !kef::Stack32(s, base, sp - 8u, z)) [[unlikely]] return true;
  if (!lower(8u, x) || !lower(12u, y) || !lower(16u, z) || !upper(20u, x) || !upper(24u, y)) return true;
  (void)upper(28u, z);
  return true;
}

}  // namespace n_82149B08

// sub_8213AA78 - a 4 x 4 single matrix product, out (r5)[4i + j] = sum over k of a (r3)[4k + j] * b (r4)[4i + k],
// as the original's fmuls and three fmadds in its order (k = 1, 2, 3, 0). The 32 inputs are read once; an output
// that overlaps an input (the original's stores fall between its loads): the copy. r3 is not written.
// Scratch not reproduced: r6-r11, f0-f13 (r30 / r31 are saved and restored by the original).
namespace n_8213AA78 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kMatrixMultiply)) [[unlikely]] return false;
  const uint32_t pa = ctx.r3.u32, pb = ctx.r4.u32, po = ctx.r5.u32;
  if (uint32_t(po - pa + 63u) < 127u || uint32_t(po - pb + 63u) < 127u) [[unlikely]] return false;
  if (pa > 0xFFFFFFC0u || pb > 0xFFFFFFC0u || po > 0xFFFFFFC0u) [[unlikely]] return false;
  // The original's saves: std r30 at r1 - 16, std r31 at r1 - 8.
  if (!kef::Stack64(s, base, ctx.r1.u32 - 16u, ctx.r30.u64) || !kef::Stack64(s, base, ctx.r1.u32 - 8u, ctx.r31.u64))
      [[unlikely]] return true;
  ctx.fpscr.disableFlushMode();
  float a[16], b[16], out[16];
  for (uint32_t k = 0; k < 16; ++k) {
    a[k] = kef::Fl(base, pa + 4u * k);
    b[k] = kef::Fl(base, pb + 4u * k);
  }
  for (uint32_t i = 0; i < 4; ++i) {
    for (uint32_t j = 0; j < 4; ++j) {
      const float f9 = b[4 * i + 1] * a[4 + j];                     // fmuls f9,f10,f0
      const float f5 = kef::Fmadds(b[4 * i + 2], a[8 + j], f9);     // fmadds f5,f8,f13,f9
      const float f4 = kef::Fmadds(b[4 * i + 3], a[12 + j], f5);    // fmadds f4,f7,f12,f5
      out[4 * i + j] = kef::Fmadds(a[j], b[4 * i + 0], f4);         // fmadds f3,f11,f6,f4
    }
  }
  if (!s.Note(po, 64)) [[unlikely]] return true;
  uint8_t* d = ::banjo::hot::Raw(base, po);
  for (uint32_t k = 0; k < 16; ++k) {
    const uint32_t w = __builtin_bswap32(kef::Bits(out[k]));
    std::memcpy(d + 4 * k, &w, 4);
  }
  ::banjo::hot::Card(base, po, 64);
  return true;
}

}  // namespace n_8213AA78

// sub_821FCD50 - the 4 x 4 matrix product of VMX128 (out r3 = a r4 x b r5 by rows, vmsum4fp128 dot products of a's
// rows with b's columns). The original reads both matrices with lvx / lvsl / vperm (the unaligned rows), transposes b
// and stores the 16 results one word at a time (stvewx128). Here each row is read as it lies, and the very same
// vector operations of the generated code (the transposes, simde_mm_dp_ps with the flush mode on) give the same 16
// floats, stored at once. Every load comes before every store, as in the original, so an output over an input is
// fine. An output not word-aligned (stvewx128 would round its addresses down): the copy.
// Scratch not reproduced: r4-r11, v0, v7, v46-v63 (r31 is saved and restored by the original).
namespace n_821FCD50 {

[[gnu::always_inline]] inline simde__m128i Row(uint8_t* base, uint32_t a) {
  // The generated layout: lane 3 = the row's first word (PPC element 0), each word in host order.
  return simde_mm_set_epi32(int32_t(kef::U32(base, a)), int32_t(kef::U32(base, a + 4u)),
                            int32_t(kef::U32(base, a + 8u)), int32_t(kef::U32(base, a + 12u)));
}

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kMatrixMultiplyVmx)) [[unlikely]] return false;
  const uint32_t po = ctx.r3.u32, pa = ctx.r4.u32, pb = ctx.r5.u32;
  if (po & 3u) [[unlikely]] return false;
  if (pa > 0xFFFFFFC0u || pb > 0xFFFFFFC0u || po > 0xFFFFFFC0u) [[unlikely]] return false;
  // The original's save: std r31 at r1 - 8 (before its loads, so an output over the slot is the copy's case).
  if (uint32_t(po - (ctx.r1.u32 - 8u) + 63u) < 71u) [[unlikely]] return false;
  if (!kef::Stack64(s, base, ctx.r1.u32 - 8u, ctx.r31.u64)) [[unlikely]] return true;
  // b's rows (v59 v58 v57 v61) and a's (v60 v56 v62 v63), as the original's vperm results.
  const simde__m128i b0 = Row(base, pb), b1 = Row(base, pb + 16u), b2 = Row(base, pb + 32u), b3 = Row(base, pb + 48u);
  const simde__m128i a0 = Row(base, pa), a1 = Row(base, pa + 16u), a2 = Row(base, pa + 32u), a3 = Row(base, pa + 48u);
  // b's columns: vmrghw / vmrglw as the generated code has them (unpack of the second operand first).
  const simde__m128i v54 = simde_mm_unpackhi_epi32(b2, b0);   // vmrghw128 v54,v59,v57
  const simde__m128i v53 = simde_mm_unpackhi_epi32(b3, b1);   // vmrghw128 v53,v58,v61
  const simde__m128i v61 = simde_mm_unpacklo_epi32(b3, b1);   // vmrglw128 v61,v58,v61
  const simde__m128i v59 = simde_mm_unpacklo_epi32(b2, b0);   // vmrglw128 v59,v59,v57
  const simde__m128 c58 = simde_mm_castsi128_ps(simde_mm_unpackhi_epi32(v53, v54));  // vmrghw128 v58,v54,v53
  const simde__m128 c55 = simde_mm_castsi128_ps(simde_mm_unpackhi_epi32(v61, v59));  // vmrghw128 v55,v59,v61
  const simde__m128 c57 = simde_mm_castsi128_ps(simde_mm_unpacklo_epi32(v53, v54));  // vmrglw128 v57,v54,v53
  const simde__m128 c61 = simde_mm_castsi128_ps(simde_mm_unpacklo_epi32(v61, v59));  // vmrglw128 v61,v59,v61
  ctx.fpscr.enableFlushMode();
  const simde__m128 r0 = simde_mm_castsi128_ps(a0), r1 = simde_mm_castsi128_ps(a1);
  const simde__m128 r2 = simde_mm_castsi128_ps(a2), r3 = simde_mm_castsi128_ps(a3);
  auto dp = [](simde__m128 x, simde__m128 y) { return simde_mm_castps_si128(simde_mm_dp_ps(x, y, 0xFF)); };
  const simde__m128i d59 = dp(r0, c58), d53 = dp(r0, c55), d54 = dp(r0, c57), d60 = dp(r0, c61);
  const simde__m128i d52 = dp(r1, c61), d51 = dp(r1, c57), d50 = dp(r1, c55), d56 = dp(r1, c58);
  const simde__m128i d49 = dp(r2, c61), d46 = dp(r2, c58), d48 = dp(r2, c57), d47 = dp(r2, c55);
  const simde__m128i d61 = dp(r3, c61), d57 = dp(r3, c57), d55 = dp(r3, c55), d58 = dp(r3, c58);
  // The rows of the result (v63 v62 v61 v60 before their stores).
  const simde__m128i row0 = simde_mm_unpackhi_epi32(simde_mm_unpackhi_epi32(d60, d54), simde_mm_unpackhi_epi32(d53, d59));
  const simde__m128i row1 = simde_mm_unpackhi_epi32(simde_mm_unpackhi_epi32(d52, d51), simde_mm_unpackhi_epi32(d50, d56));
  const simde__m128i row2 = simde_mm_unpackhi_epi32(simde_mm_unpackhi_epi32(d49, d48), simde_mm_unpackhi_epi32(d47, d46));
  const simde__m128i row3 = simde_mm_unpackhi_epi32(simde_mm_unpackhi_epi32(d61, d57), simde_mm_unpackhi_epi32(d55, d58));
  if (!s.Note(po, 64)) [[unlikely]] return true;
  uint8_t* d = ::banjo::hot::Raw(base, po);
  const simde__m128i rows[4] = {row0, row1, row2, row3};
  for (uint32_t r = 0; r < 4; ++r) {
    alignas(16) uint32_t lanes[4];
    simde_mm_store_si128(reinterpret_cast<simde__m128i*>(lanes), rows[r]);
    for (uint32_t e = 0; e < 4; ++e) {  // PPC element e is lane 3 - e
      const uint32_t w = __builtin_bswap32(lanes[3 - e]);
      std::memcpy(d + 16 * r + 4 * e, &w, 4);
    }
  }
  ::banjo::hot::Card(base, po, 64);
  return true;
}

}  // namespace n_821FCD50

// sub_820C96C0 - one step of an animation track's time (track r3, time at r4, direction byte at r5): time plus or
// minus the frame step (the global at 0x8248E2FC), then at the end the track loops, turns round or stops (its flags
// +16), below 0 it turns round, and the result is kept within [0, length - the constant at 0x82088400]; r3 = 1 when
// it reached an end, else 0. The same compares, stores and float expressions in the same order. Scratch not
// reproduced: r9-r11, f0, f10-f13.
namespace n_820C96C0 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kTrackStep)) [[unlikely]] return false;
  const uint32_t t = ctx.r3.u32, pt = ctx.r4.u32, pd = ctx.r5.u32;
  ctx.fpscr.disableFlushMode();
  ctx.r3.u64 = 0;
  if (kef::Fl(base, t + 20u) == 0.0f) return true;  // beqlr: not playing
  const uint32_t dir = kef::U8(base, pd);
  const float now = kef::Fl(base, pt), step = kef::Fl(base, 0x8248E2FCu);
  const float f11 = dir ? now - step : now + step;  // fsubs / fadds f11,f13,f0
  if (!kef::Put32(s, base, pt, kef::Bits(f11))) [[unlikely]] return true;
  float f0 = f11;
  bool to_end_test = false;  // true: straight to the length test (loc_820C9790)
  if (!(f0 < kef::Fl(base, t + 4u))) {  // at or past the end
    const uint32_t flags = kef::U32(base, t + 16u);
    ctx.r3.u64 = 1;
    if (flags & 1u) {  // turn round: time = length - (time - length)
      if (!kef::Put8(s, base, pd, 1)) [[unlikely]] return true;
      const float len = kef::Fl(base, t + 4u);
      const float over = kef::Fl(base, pt) - len;  // fsubs f11,f13,f0
      if (!kef::Put32(s, base, pt, kef::Bits(len - over))) [[unlikely]] return true;  // fsubs f10,f0,f11
    } else if (flags & 2u) {  // loop: time - length
      if (!kef::Put32(s, base, pt, kef::Bits(f0 - kef::Fl(base, t + 4u)))) [[unlikely]] return true;
    }
  } else if (f0 >= 0.0f) {
    to_end_test = true;
  } else {  // below 0: turn round, time = -time
    ctx.r3.u64 = 1;
    if (!kef::Put8(s, base, pd, 0)) [[unlikely]] return true;
    const uint32_t bits = kef::Quiet(kef::U32(base, pt)) ^ 0x80000000u;  // lfs, fneg, stfs
    if (!kef::Put32(s, base, pt, bits)) [[unlikely]] return true;
  }
  if (!to_end_test) {  // loc_820C977C
    f0 = kef::Fl(base, pt);
    if (!(f0 >= 0.0f)) {
      (void)kef::Put32(s, base, pt, 0u);  // stfs f12 (0.0)
      return true;
    }
  }
  const float len = kef::Fl(base, t + 4u);  // loc_820C9790
  if (f0 < len) return true;
  (void)kef::Put32(s, base, pt, kef::Bits(len - kef::Fl(base, 0x82088400u)));  // fsubs f0,f13,f0
  return true;
}

}  // namespace n_820C96C0

// sub_820BF340 - a box (min r4, max r5) against six planes (r3, 16 bytes each: normal x y z, distance): r3 = 0
// outside a plane, 1 crossing one, 2 inside all. The original writes the box's eight corners into a table below r1
// and picks the nearest and farthest corner of each plane by its normal's signs; here the corners are picked from the
// six values directly (the table is read by nothing else). The same float expressions in the same order.
// Scratch not reproduced: r4-r11, f0-f13.
namespace n_820BF340 {

template <class S>
[[gnu::always_inline]] inline bool Native(PPCContext& __restrict ctx, uint8_t* base, [[maybe_unused]] S& s) {
  if (!On(kBoxFrustum)) [[unlikely]] return false;
  const uint32_t planes = ctx.r3.u32, lo = ctx.r4.u32, hi = ctx.r5.u32;
  ctx.fpscr.disableFlushMode();
  const float xs[2] = {kef::Fl(base, lo + 0u), kef::Fl(base, hi + 0u)};
  const float ys[2] = {kef::Fl(base, lo + 4u), kef::Fl(base, hi + 4u)};
  const float zs[2] = {kef::Fl(base, lo + 8u), kef::Fl(base, hi + 8u)};
  // The original's table of the eight corners at r1 - 96 (corner c = x of bit 2, y of bit 1, z of bit 0, 12 bytes
  // each; stfs of lfs-loaded values, a signalling NaN quieted), written whole before the planes are read.
  {
    const uint32_t sp = ctx.r1.u32;
    if (!s.Note(sp - 96u, 96)) [[unlikely]] return true;
    uint8_t* t = ::banjo::hot::Raw(base, sp - 96u);
    for (uint32_t c = 0; c < 8; ++c) {
      const uint32_t w[3] = {__builtin_bswap32(kef::Quiet(kef::Bits(xs[(c >> 2) & 1u]))),
                             __builtin_bswap32(kef::Quiet(kef::Bits(ys[(c >> 1) & 1u]))),
                             __builtin_bswap32(kef::Quiet(kef::Bits(zs[c & 1u])))};
      std::memcpy(t + 12u * c, w, 12);
    }
  }
  bool crosses = false;
  for (uint32_t k = 0; k < 6; ++k) {
    const uint32_t p = planes + 16u * k;
    const float nx = kef::Fl(base, p + 0u), ny = kef::Fl(base, p + 4u), nz = kef::Fl(base, p + 8u);
    const float d = kef::Fl(base, p + 12u);
    const uint32_t sx = nx > 0.0f, sy = ny > 0.0f, sz = nz > 0.0f;
    // The far corner (each coordinate the max where the normal is positive).
    float f8 = ys[sy] * ny;                          // fmuls f8,f9,f13
    float f5 = kef::Fmadds(zs[sz], nz, f8);         // fmadds f5,f7,f12,f8
    float f4 = kef::Fmadds(xs[sx], nx, f5);         // fmadds f4,f6,f11,f5
    if (f4 + d < 0.0f) {                             // fadds f3,f4,f10; blt
      ctx.r3.u64 = 0;
      return true;
    }
    // The near corner (the other one of each).
    f8 = ys[sy ^ 1u] * ny;
    f5 = kef::Fmadds(zs[sz ^ 1u], nz, f8);
    f4 = kef::Fmadds(nx, xs[sx ^ 1u], f5);         // fmadds f4,f11,f6,f5
    if (!(f4 + d >= 0.0f)) crosses = true;
  }
  ctx.r3.u64 = crosses ? 1 : 2;
  return true;
}

}  // namespace n_820BF340

}  // namespace banjo::hand
