// keflings-nx (2026-10-30, evening): title update 2's first-start settings.
//
// A new profile has no title-specific settings yet (XPROFILE_TITLE_SPECIFIC1, 0x63E83FFF: the SDK keeps it as the file
// <user data>/584108DB/profile/User/63E83FFF). Without them title update 2 takes its default giant, the avatar giant
// "You", which this runtime cannot make (no avatar data: keflings_avatar_giant), and the front end's giant comes out as
// an object of kind "None": the New Game flow then waits for ever (PC with the settings set aside and the Switch alike,
// 2026-10-30 22:30-23:20). The game's own settings after Grant was chosen (as it wrote them in the PC run of 21:21)
// are written once when the file is not there; from then on the game reads and writes them as usual, so a choice of
// giant in the game is kept. Key keflings_seed_title_settings.
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <system_error>

#include <rex/cvar.h>
#include <rex/logging.h>

#include "keflings/keflings_title_settings.h"

REXCVAR_DEFINE_BOOL(keflings_seed_title_settings, true, "Keflings",
                    "Title update 2: a profile with no title settings yet gets the game's own settings with Grant as "
                    "the giant (its default would be the avatar giant, which cannot be made here)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

namespace keflings {

namespace {
std::filesystem::path g_user_data_root;

// The 1,000 bytes the game wrote for TITLE_SPECIFIC1 (zero where not listed).
std::array<uint8_t, 1000> DefaultSettings() {
  std::array<uint8_t, 1000> b{};
  const uint8_t head[32] = {0x0C, 0x01, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x32, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x01};
  for (size_t i = 0; i < sizeof(head); ++i) b[i] = head[i];
  for (size_t i = 0xB8; i < 0xBC; ++i) b[i] = 0xFF;
  const uint8_t tail[8] = {0x00, 0x04, 0x01, 0x04, 0x05, 0x00, 0x00, 0x00};
  for (size_t i = 0; i < sizeof(tail); ++i) b[0x3E0 + i] = tail[i];
  return b;
}
}  // namespace

void SetUserDataRoot(const std::filesystem::path& root) { g_user_data_root = root; }

void SeedTitleSettings() {
  if (g_user_data_root.empty() || !REXCVAR_GET(keflings_seed_title_settings)) return;
  std::error_code ec;
  const auto dir = g_user_data_root / "584108DB" / "profile" / "User";
  const auto file = dir / "63E83FFF";
  if (std::filesystem::exists(file, ec)) return;
  std::filesystem::create_directories(dir, ec);
  const auto blob = DefaultSettings();
  std::ofstream f(file, std::ios::binary | std::ios::trunc);
  f.write(reinterpret_cast<const char*>(blob.data()), std::streamsize(blob.size()));
  if (f) {
    REXLOG_INFO("[keflings] title settings seeded for a new profile (giant: Grant; keflings_seed_title_settings)");
  } else {
    REXLOG_WARN("[keflings] could not write the first title settings to {}", file.string());
  }
}

}  // namespace keflings
