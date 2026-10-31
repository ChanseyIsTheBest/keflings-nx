/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 *
 * @modified    Tom Clay, 2026 - Adapted for ReXGlue runtime
 */

#include <rex/logging.h>
#include <rex/ppc/function.h>
#include <rex/runtime.h>
#include <rex/system/kernel_module.h>
#include <rex/system/kernel_state.h>
#include <rex/system/function_dispatcher.h>
#include <rex/thread/mutex.h>
#include <rex/cvar.h>

REXCVAR_DEFINE_BOOL(xam_avatar_exports, true, "Kernel",
                    "XAM's avatar exports (ordinals 1500-1521) can be looked up by ordinal; false = not there, as on a "
                    "dashboard without the avatar system");

namespace rex::system {

KernelModule::KernelModule(KernelState* kernel_state, const std::string_view path)
    : XModule(kernel_state, ModuleType::kKernelModule) {
  emulator_ = kernel_state->emulator();
  memory_ = emulator_->memory();
  export_resolver_ = kernel_state->emulator()->export_resolver();

  path_ = path;
  name_ = rex::string::utf8_find_base_name_from_guest_path(path);

  // Persist this object through reloads.
  host_object_ = true;

  OnLoad();
}

KernelModule::~KernelModule() {}

uint32_t KernelModule::GetProcAddressByOrdinal(uint16_t ordinal, uint32_t caller_address) {
  // keflings-nx (2026-10-30): xam_avatar_exports = false answers the avatar exports (xam 1500-1521) as a dashboard
  // without the avatar system would: not there.
  if (!REXCVAR_GET(xam_avatar_exports) && ordinal >= 1500 && ordinal <= 1521 && name_ == "xam") {
    REXSYS_INFO("GetProcAddressByOrdinal: xam {} not offered (xam_avatar_exports = false)", ordinal);
    return 0;
  }
  // Look up the export in the resolver
  auto export_entry = export_resolver_->GetExportByOrdinal(name_, ordinal);
  if (!export_entry) {
    REXSYS_DEBUG("GetProcAddressByOrdinal: ordinal {:04X} not found in {}", ordinal, name_);
    return 0;
  }

  if (export_entry->type == rex::runtime::Export::Type::kVariable) {
    // Variables have guest addresses we can return directly
    REXSYS_DEBUG("GetProcAddressByOrdinal: {} ({:04X}) -> variable at {:08X}", export_entry->name,
                 ordinal, export_entry->variable_ptr);
    return export_entry->variable_ptr;
  }

  auto* dispatcher = emulator_->function_dispatcher();
  uint32_t caller_module_base = dispatcher->FindCallerModuleBase(caller_address);
  ThunkKey key{caller_module_base, ordinal};

  // Check thunk cache first (already allocated for this caller's module)
  auto thunk_it = thunk_cache_.find(key);
  if (thunk_it != thunk_cache_.end()) {
    REXSYS_DEBUG("GetProcAddressByOrdinal: {} ({:04X}) in {} -> cached thunk {:08X}",
                 export_entry->name, ordinal, name_, thunk_it->second);
    return thunk_it->second;
  }

  // Look up native implementation by name from the auto-registry
  std::string imp_name = std::string("__imp__") + export_entry->name;
  REXSYS_DEBUG("GetProcAddressByOrdinal: searching registry for '{}'", imp_name);
  PPCFunc* func = rex::ppc::FindPPCFuncByName(imp_name.c_str());
  if (func) {
    uint32_t thunk_addr = dispatcher->AllocateThunk(func, caller_address);
    if (thunk_addr) {
      thunk_cache_[key] = thunk_addr;
      REXSYS_DEBUG("GetProcAddressByOrdinal: {} ({:04X}) in {} -> thunk at {:08X}",
                   export_entry->name, ordinal, name_, thunk_addr);
      return thunk_addr;
    }
  }

  // No native implementation available
  REXSYS_WARN("GetProcAddressByOrdinal: function {} ({:04X}) in {} - no native implementation",
              export_entry->name, ordinal, name_);
  return 0;
}

uint32_t KernelModule::GetProcAddressByName(const std::string_view name) {
  // TODO: Does this even work for kernel modules?
  (void)name;
  REXSYS_ERROR("KernelModule::GetProcAddressByName not implemented");
  return 0;
}

void KernelModule::InvalidateThunkCacheInRange(uint32_t lo, uint32_t hi) {
  for (auto it = thunk_cache_.begin(); it != thunk_cache_.end();) {
    if (it->second >= lo && it->second < hi) {
      it = thunk_cache_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace rex::system
