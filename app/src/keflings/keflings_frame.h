// keflings-nx (2026-10-30): the once-a-frame work banjo-nx did from N&B's main draw function (renut_engine/FPS.cpp's
// appMainDraw hook), done from the game's D3DDevice_Swap instead (Renut's Swap hook calls OnGameSwap).
#pragma once

namespace keflings {

// Shader compile mode, the frame limiter (frame_cap), the profiler's frame boundary and the FPS counter.
void OnGameSwap();

}  // namespace keflings
