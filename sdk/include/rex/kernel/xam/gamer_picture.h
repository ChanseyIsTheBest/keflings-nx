#pragma once
// Pictures from the host for XAM's tile reads (from bejeweledblitz-nx's gamer pictures, 2026-10-30; keflings-nx
// 2026-10-31). XamReadTileToTexture fills the picture with white when no source is set, as before. With a source, the
// tiles it would fill white (a gamer picture, an achievement picture: this runtime has none of them) are filled from
// the source instead. Only standard headers here: the Switch's switch.h collides with rex's typedefs.

#include <cstdint>

namespace rex::kernel::xam {

// Fills the picture of a player (0 = the profile): size x size pixels in the console's D3DFMT_LIN_A8R8G8B8 (each pixel
// the bytes A, R, G, B), rows `pitch` bytes apart. False when there is no picture; the tile is then white.
using GamerPictureSource = bool (*)(uint32_t player, uint32_t size, uint8_t* pixels, uint32_t pitch);

void SetGamerPictureSource(GamerPictureSource source);

}  // namespace rex::kernel::xam
