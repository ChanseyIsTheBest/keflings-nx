// banjo-nx: the boot-time choice between banjo-nx's native renderer and Renut's (banjo_renut_select.h).
#include "renut_engine/gpu/banjo_renut_select.h"

#include <string>

#include <rex/cvar.h>
#include <rex/logging/macros.h>

// keflings-nx (2026-10-30): renut is the default; "native" leaves the game's Direct3D to the SDK's emulated Xenos GPU
// (gpu_plugin = "xenos", the PC's reference image): banjo-nx's own native renderers are not built in this port.
REXCVAR_DEFINE_STRING(banjo_renderer, "renut", "Banjo",
                      "Renderer, chosen at boot. renut = Renut's Direct3D-level renderer "
                      "(github.com/masterspike52/reNut), the default: the game's Direct3D runs natively through plume, "
                      "with no PM4 and no EDRAM tile model. native = the game's own Direct3D on the SDK's emulated "
                      "Xenos GPU (gpu_plugin = xenos), the PC's reference image")
    .allowed({"native", "renut"})
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

namespace banjo::renut_gpu {

// banjo-nx (2026-10-19, CPU proposal 22): Active() is inline (banjo_renut_select.h), a byte load after the first call.
namespace detail {
int8_t g_active = 0;
bool LatchActive() {
#if BANJO_SWITCH_NO_VULKAN
  // banjo-nx (2026-10-20): a Switch build without the Vulkan driver has only Renut's renderer on deko3d (the native
  // renderer and the emulated GPU need Vulkan).
  static const bool active = [] {
    if (REXCVAR_GET(banjo_renderer) != "renut")
      REXLOG_WARN("banjo_renderer = \"{}\" needs Vulkan, which this build does not have: Renut's renderer on deko3d "
                  "instead", REXCVAR_GET(banjo_renderer));
    return true;
  }();
#else
  static const bool active = REXCVAR_GET(banjo_renderer) == "renut";
#endif
  g_active = active ? 1 : -1;
  return active;
}
}  // namespace detail

}  // namespace banjo::renut_gpu
