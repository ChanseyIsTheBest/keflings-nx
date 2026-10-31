// keflings-nx (2026-10-31): the console user's picture in the game's picture boxes (the user's request: the
// Achievement dialog's picture was a white square). From bejeweledblitz-nx's gamer pictures (2026-10-30).
//
// The game reads its pictures through XAM's tile reads (XamReadTileToTexture), which this runtime filled with white:
// there are no gamer or achievement pictures here. With the source set below, those tiles are the picture of the
// console's user chosen by keflings_user_picture_user. Only the system headers and the SDK's standard-only
// gamer_picture.h here: switch.h's typedefs collide with rex's (windowed_app_main_switch.cpp).

#include "keflings/keflings_user_picture.h"

#include <rex/kernel/xam/gamer_picture.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

#if defined(__SWITCH__)
#include <switch.h>
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
// The SDK's stb_image (sdk/thirdparty/stb, which rex::ui builds for PNG icons): its own static copy, JPEG and PNG.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_FAILURE_STRINGS
#include <stb_image.h>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace keflings::user_picture {

namespace {

// Read once in Install, before the game starts; only read afterwards.
int g_width = 0;
int g_height = 0;
std::vector<uint8_t> g_rgba;

bool Decode(const std::vector<uint8_t>& bytes) {
  int width = 0, height = 0, channels = 0;
  stbi_uc* pixels = stbi_load_from_memory(bytes.data(), int(bytes.size()), &width, &height, &channels, 4);
  if (!pixels) return false;
  g_width = width;
  g_height = height;
  g_rgba.assign(pixels, pixels + size_t(width) * size_t(height) * 4);
  stbi_image_free(pixels);
  return width > 0 && height > 0;
}

bool ReadFile(const std::string& path, std::vector<uint8_t>* bytes) {
  FILE* file = std::fopen(path.c_str(), "rb");
  if (!file) return false;
  std::fseek(file, 0, SEEK_END);
  const long size = std::ftell(file);
  std::fseek(file, 0, SEEK_SET);
  bytes->resize(size > 0 ? size_t(size) : 0);
  const bool read = size > 0 && std::fread(bytes->data(), 1, bytes->size(), file) == bytes->size();
  std::fclose(file);
  return read;
}

#if defined(__SWITCH__)
// The picture (JPEG, 256 x 256) of the console's user number `user` (1-based, the HOME Menu's order), or of the first
// user when there are fewer. Returns the number of users (-1: the account service could not be read) and in *taken the
// user whose picture it is (0: none).
int ReadConsoleUser(uint32_t user, std::vector<uint8_t>* jpeg, uint32_t* taken) {
  *taken = 0;
  // acc:u0 as an application (a game override), acc:u1 when started as an applet.
  if (R_FAILED(accountInitialize(AccountServiceType_Application)) &&
      R_FAILED(accountInitialize(AccountServiceType_System))) {
    return -1;
  }
  AccountUid users[ACC_USER_LIST_SIZE] = {};
  s32 count = 0;
  if (R_FAILED(accountListAllUsers(users, ACC_USER_LIST_SIZE, &count))) count = -1;
  if (count > 0) {
    const uint32_t index = user >= 1 && user <= uint32_t(count) ? user - 1 : 0;
    AccountProfile profile;
    if (R_SUCCEEDED(accountGetProfile(&profile, users[index]))) {
      u32 size = 0, read = 0;
      if (R_SUCCEEDED(accountProfileGetImageSize(&profile, &size)) && size) {
        jpeg->resize(size);
        if (R_FAILED(accountProfileLoadImage(&profile, jpeg->data(), size, &read))) read = 0;
        jpeg->resize(read);
        if (read) *taken = index + 1;
      }
      accountProfileClose(&profile);
    }
  }
  accountExit();
  return count;
}
#endif

// XAM's source (rex/kernel/xam/gamer_picture.h): the picture's centre square, each tile pixel the average of the
// picture's pixels it covers (256 x 256 to 64 x 64: 4 x 4 each). Every player gets the one picture.
bool Fill(uint32_t player, uint32_t size, uint8_t* pixels, uint32_t pitch) {
  (void)player;
  if (g_rgba.empty()) return false;
  const int side = std::min(g_width, g_height);
  const int left = (g_width - side) / 2;
  const int top = (g_height - side) / 2;
  for (uint32_t y = 0; y < size; ++y) {
    const int y0 = int(uint64_t(y) * side / size);
    const int y1 = std::max(y0 + 1, int(uint64_t(y + 1) * side / size));
    uint8_t* out = pixels + size_t(y) * pitch;
    for (uint32_t x = 0; x < size; ++x) {
      const int x0 = int(uint64_t(x) * side / size);
      const int x1 = std::max(x0 + 1, int(uint64_t(x + 1) * side / size));
      uint32_t sum[4] = {};
      for (int sy = y0; sy < y1; ++sy) {
        const uint8_t* in = &g_rgba[(size_t(top + sy) * g_width + left + x0) * 4];
        for (int sx = x0; sx < x1; ++sx, in += 4) {
          sum[0] += in[0];
          sum[1] += in[1];
          sum[2] += in[2];
          sum[3] += in[3];
        }
      }
      const uint32_t count = uint32_t((y1 - y0) * (x1 - x0));
      out[4 * x + 0] = uint8_t((sum[3] + count / 2) / count);  // A, R, G, B
      out[4 * x + 1] = uint8_t((sum[0] + count / 2) / count);
      out[4 * x + 2] = uint8_t((sum[1] + count / 2) / count);
      out[4 * x + 3] = uint8_t((sum[2] + count / 2) / count);
    }
  }
  return true;
}

}  // namespace

std::string Install(uint32_t user, const std::string& file) {
  std::vector<uint8_t> bytes;
  std::string source;
  if (!file.empty()) {
    source = ReadFile(file, &bytes) ? file : file + " (unreadable)";
  }
#if defined(__SWITCH__)
  if (bytes.empty()) {
    uint32_t taken = 0;
    const int count = ReadConsoleUser(user, &bytes, &taken);
    source = count < 0  ? "the console's users unreadable"
             : !taken   ? std::to_string(count) + " console user(s), no picture read"
                        : "console user " + std::to_string(taken) + " of " + std::to_string(count) +
                            (taken != user ? " (user " + std::to_string(user) + " asked for)" : "");
  }
#else
  if (bytes.empty() && source.empty()) source = "no console users on this platform";
#endif
  if (bytes.empty() || !Decode(bytes)) return source + ": no picture, the picture boxes stay white";
  rex::kernel::xam::SetGamerPictureSource(&Fill);
  return source + " (" + std::to_string(g_width) + "x" + std::to_string(g_height) + ")";
}

}  // namespace keflings::user_picture
