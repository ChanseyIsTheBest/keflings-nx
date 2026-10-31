// keflings-nx (2026-10-30): Renut-renderer hooks on D3D functions that Keflings links and Banjo-Kazooie: N&B does not
// (so banjo-nx's renderer has no hook for them). Each says what the original does and why the hook is needed.
#include <rex/hook.h>
#include <rex/logging.h>
#include <rex/ppc.h>
#include <rex/types.h>

#include "renut_engine/gpu/banjo_renut_select.h"
#include "renut_engine/gpu/guest/guest_memory.h"

namespace {

constexpr uint32_t kDeviceOffsetFlags = 10940;  // device-private flags byte (renderer: kDeviceOffsetFlags)
constexpr uint8_t kDeviceFlagNoHardware = 0x80;
constexpr uint32_t kDeviceOffsetRing = 13232;  // non-zero when the device has a hardware ring to measure

// Through the renderer's translation: the device lives in a physical heap, which has a host offset on the PC.
uint32_t LoadU32(uint32_t va) {
  const auto *p = renut::gpu::GuestPtr<const be_u32>(va);
  return p ? uint32_t(*p) : 0;
}
void StoreU32(uint32_t va, uint32_t v) {
  if (auto *p = renut::gpu::GuestPtr<be_u32>(va))
    *p = v;
}

}  // namespace

// void sub_821EA1A8(CDevice *device, DWORD *a, DWORD *b): D3D's ring-buffer space query, called once a frame by the
// game's sub_820B5280 (which stores both figures / 1024 in two globals). The original: with a hardware ring
// (device + 13232 set) the figures come from the ring pointers; else, on a device flagged "no hardware" (byte 10940,
// 0x80), from an interface object at device + 13516 through its callback at +180; else 0 and 0.
// Renut's devices are all flagged "no hardware" (gpu/d3d/hooks/device.cpp: no GPU hardware setup, immediate fences)
// and have no ring and no such interface: the original then called a null callback (PC run 1: "Call to invalid or
// unregistered function at guest address 0x00000000", lr 0x821EA140). The hook gives that case the answer of a device
// with neither ring nor interface, 0 and 0, and leaves every other case to the original.
REX_EXTERN(__imp__sub_821EA1A8);
BANJO_RENUT_HOOK_RAW(sub_821EA1A8) {
  const uint32_t device = ctx.r3.u32;
  const auto *flags = renut::gpu::GuestPtr<const uint8_t>(device ? device + kDeviceOffsetFlags : 0);
  if (flags && (*flags & kDeviceFlagNoHardware) && LoadU32(device + kDeviceOffsetRing) == 0) {
    StoreU32(ctx.r4.u32, 0);
    StoreU32(ctx.r5.u32, 0);
    return;
  }
  __imp__sub_821EA1A8(ctx, base);
}
