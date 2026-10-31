// banjo-nx (2026-10-29): title update 3's files (title_update.h).
// keflings-nx (2026-10-30): A Kingdom for Keflings' title update 2: default.xexp next to default.xex, and the update
// package's other files (fsxb2\blockfiles\Title_Update.blk.bz2 and three font blocks) in an "update" folder, which the
// runtime mounts as update: (docs/keflings-notes.md, "Files").
#include "renut_engine/title_update.h"

#include <cstdlib>
#include <system_error>

#include <rex/filesystem.h>
#include <rex/logging.h>
#include <rex/system.h>

namespace banjo::title_update {

namespace fs = std::filesystem;

namespace {
bool IsUpdateFolder(const fs::path &c) {
  std::error_code ec;
  return !c.empty() && fs::exists(c / "fsxb2" / "blockfiles" / "Title_Update.blk.bz2", ec);
}
}  // namespace

void ConfigurePaths(rex::PathConfig &paths) {
  std::error_code ec;
  if (paths.update_data_root.empty()) {
    const fs::path candidates[] = {
        rex::filesystem::GetExecutableFolder() / "update",
        paths.game_data_root.empty() ? fs::path() : paths.game_data_root.parent_path() / "update",
        paths.game_data_root.empty() ? fs::path() : paths.game_data_root.parent_path() / "tu2",
        paths.game_data_root.empty() ? fs::path() : paths.game_data_root / "update",
    };
    for (const fs::path &c : candidates) {
      if (IsUpdateFolder(c)) {
        paths.update_data_root = c;
        break;
      }
    }
  }
  if (paths.update_data_root.empty())
    REXLOG_WARN("[title update] no update folder (update_data_root, or \"update\" next to keflings-nx or the game "
                "folder, holding title update 2's fsxb2\\blockfiles\\Title_Update.blk.bz2): the update's data is not "
                "there");
  else
    REXLOG_INFO("[title update] update data: {}", rex::path_to_utf8(paths.update_data_root));
  if (!paths.game_data_root.empty() && !fs::exists(paths.game_data_root / "default.xexp", ec)) {
    // The recompiled code is title update 2's: the base XEX's image would not match it at all.
    REXLOG_ERROR("[title update] {} has no default.xexp. keflings-nx is title update 2 of the game: put the update's "
                 "default.xexp next to default.xex (and its fsxb2 folder in the update folder)",
                 rex::path_to_utf8(paths.game_data_root));
    rex::ShowSimpleMessageBox(rex::SimpleMessageBoxType::Error,
                              "keflings-nx needs title update 2: put the update's default.xexp next to default.xex in "
                              "the game folder, and its fsxb2 folder in an \"update\" folder next to keflings-nx.");
  }
}

}  // namespace banjo::title_update
