/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 *
 * @modified    Tom Clay, 2026 - Adapted for ReXGlue runtime
 */

#include <rex/cvar.h>
#include <rex/kernel/xam/private.h>
#include <rex/logging.h>
#include <rex/hook.h>
#include <rex/types.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xtypes.h>

// keflings-nx (2026-10-30): A Kingdom for Keflings' title update 2 shows "Update Required" and exits to the dashboard
// when XamAvatarInitialize fails (PC runs 5-7). With xam_avatars the avatar system initialises and every query of it
// gives a defined answer: no signed-out profiles, and E_FAIL for the rest (each logged once), as a console whose
// avatar data is not there. Off: the SDK's answers (initialisation fails; the other exports are plain stubs).
REXCVAR_DEFINE_BOOL(xam_avatars, false, "Kernel",
                    "XamAvatarInitialize succeeds and the avatar queries fail cleanly (no avatar data), for games that "
                    "refuse to run without the avatar system (A Kingdom for Keflings' title update 2)");

namespace rex {
namespace kernel {
namespace xam {

u32 XamAvatarInitialize_entry(u32 unk1,                  // 1, 4, etc
                              u32 unk2,                  // 0 or 1
                              u32 processor_number,      // for thread creation?
                              mapped_u32 function_ptrs,  // 20b, 5 pointers
                              mapped_void unk5,          // ptr in data segment
                              u32 unk6                   // flags - 0x00300000, 0x30, etc
) {
  if (REXCVAR_GET(xam_avatars)) {
    REXKRNL_INFO("XamAvatarInitialize({:#x}, {:#x}, {}, ..., {:#x}): success (xam_avatars)", unk1, unk2,
                 processor_number, unk6);
    return 0;
  }
  // Negative to fail. Game should immediately call XamAvatarShutdown.
  return ~0u;
}

void XamAvatarShutdown_entry() {
  // No-op.
}

}  // namespace xam
}  // namespace kernel
}  // namespace rex

REX_EXPORT(__imp__XamAvatarInitialize, rex::kernel::xam::XamAvatarInitialize_entry)
REX_EXPORT(__imp__XamAvatarShutdown, rex::kernel::xam::XamAvatarShutdown_entry)

// keflings-nx (2026-10-30): with xam_avatars a query answers 'value' (and logs its first call); without, the plain
// stub as before (r3 left as it came).
#define KEFLINGS_AVATAR_QUERY(name, value)                                                                     \
  extern "C" REX_FUNC(name) {                                                                                  \
    (void)base;                                                                                                \
    static bool logged = false;                                                                                \
    const bool on = REXCVAR_GET(xam_avatars);                                                                  \
    if (!logged) {                                                                                             \
      logged = true;                                                                                           \
      REXKRNL_WARN("{} {}", #name, on ? "answered (xam_avatars)" : "STUB");                                    \
    }                                                                                                          \
    if (on)                                                                                                    \
      ctx.r3.u64 = (value);                                                                                    \
  }                                                                                                            \
  static rex::ppc::detail::PPCFuncRegistrar _ppc_reg_##name(#name, &name);

KEFLINGS_AVATAR_QUERY(__imp__XamAvatarBeginEnumAssets, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarEndEnumAssets, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarEnumAssets, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGenerateMipMaps, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetAssetBinary, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetAssetIcon, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetAssets, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetAssetsResultSize, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetInstalledAssetPackageDescription, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetInstrumentation, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetManifestLocalUser, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetManifestsByXuid, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetMetadataRandom, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetMetadataSignedOutProfile, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarGetMetadataSignedOutProfileCount, 0)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarLoadAnimation, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarManifestGetBodyType, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarReinstallAwardedAsset, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarSetCustomAsset, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarSetManifest, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarSetMocks, 0x80004005u)
KEFLINGS_AVATAR_QUERY(__imp__XamAvatarWearNow, 0x80004005u)
