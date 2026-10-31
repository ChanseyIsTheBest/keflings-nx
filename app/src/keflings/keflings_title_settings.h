// keflings-nx (2026-10-30): title update 2's first-start settings (keflings_title_settings.cpp).
#pragma once

#include <filesystem>

namespace keflings {

void SetUserDataRoot(const std::filesystem::path& root);
// Before the game starts: writes the first title settings of a new profile (Grant as the giant) if there are none.
void SeedTitleSettings();

}  // namespace keflings
