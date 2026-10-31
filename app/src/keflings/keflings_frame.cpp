// keflings-nx (2026-10-30): the frame limiter, the shader compile mode and the FPS counter of reNut / banjo-nx
// (renut_engine/frameHooks.cpp and renut_engine/FPS.cpp, whose N&B hooks this port does not build), run once a frame
// from the game's D3DDevice_Swap (keflings_frame.h). reNut's code (masterspike52, reuse permitted) as banjo-nx has it.
#include "keflings/keflings_frame.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#include <rex/cvar.h>
#include <rex/logging.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <thread>

#include "profiler/profiler.h"
#include "renut_engine/Fps.h"
#include "renut_engine/display_sync.h"
#include "renut_engine/hooks.h"

REXCVAR_DEFINE_INT32(target_refreshRate, -1, "Nuts&Bolts/Performance",
                     "banjo-nx's presentation-interval override (N&B's hook); no effect in keflings-nx")
    .range(-1, 9);

REXCVAR_DEFINE_STRING(frame_cap, "Off", "Nuts&Bolts/Performance",
                      "Frame-rate cap (low-overhead limiter that cuts GPU load). "
                      "Off = uncapped, Display = match monitor refresh.")
    .allowed({"Off", "30", "60", "120", "144", "Display"});

REXCVAR_DEFINE_BOOL(sync_shader_compile, true, "Nuts&Bolts/Graphics",
                    "Compile shaders synchronously to stop the black flash when new effects first "
                    "appear (small one-time stutter instead). Off = engine default (async).");

void renutFrameLimit();
void renutApplyShaderCompileMode();

namespace {

#if defined(_WIN32)
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif
// One process-wide timer, created on first use. Falls back to a normal waitable timer (and then to sleep_for) if the
// high-resolution flag isn't supported.
HANDLE FrameTimer() {
  static HANDLE timer = []() -> HANDLE {
    HANDLE h = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    if (!h)
      h = CreateWaitableTimerExW(nullptr, nullptr, 0, TIMER_ALL_ACCESS);
    return h;
  }();
  return timer;
}
#endif

// The cvar as a target FPS: 0 = uncapped, "Display" = monitor refresh (queried once), otherwise the number.
int FrameCapFps() {
  const std::string &v = REXCVAR_GET(frame_cap);
  if (v.empty() || v == "Off")
    return 0;
  if (v == "Display") {
#if defined(_WIN32)
    static int hz = []() -> int {
      DEVMODEW dm{};
      dm.dmSize = sizeof(dm);
      if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm) && dm.dmDisplayFrequency > 1)
        return static_cast<int>(dm.dmDisplayFrequency);
      return 60;
    }();
    return hz;
#else
    return 60;  // the Switch panel and docked output both run at 60 Hz
#endif
  }
  return std::atoi(v.c_str());
}

}  // namespace

// The frame limiter: sleeps until the next frame's deadline (or, with banjo_display_sync, a refresh plus the measured
// phase), so the host renders no more frames than the cap.
void renutFrameLimit() {
  using clock = std::chrono::steady_clock;
  static clock::time_point next = clock::now();
  banjo::display_sync::Start();

  const int fps = FrameCapFps();
  if (fps <= 0) {
    next = clock::now();
    banjo::profiler::RecordLimiterWait(0.0);
    return;
  }
  const clock::time_point waitStart = clock::now();
  struct WaitRecorder {
    clock::time_point start;
    ~WaitRecorder() {
      banjo::profiler::RecordLimiterWait(std::chrono::duration<double, std::milli>(clock::now() - start).count());
    }
  } waitRecorder{waitStart};

  const auto frameDur = std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(1.0 / fps));
  clock::time_point now = clock::now();

  static uint64_t previousRelease = 0;
  {
    const auto toNs = [](clock::time_point t) {
      return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count());
    };
    bool anchored = false;
    const uint64_t release = banjo::display_sync::NextRelease(
        toNs(now), previousRelease, uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(frameDur).count()),
        &anchored);
    if (release) {
      previousRelease = release;
      next = clock::time_point(std::chrono::duration_cast<clock::duration>(std::chrono::nanoseconds(release)));
      if (next > now) {
        constexpr auto kTail = std::chrono::microseconds(400);
        for (;;) {
          now = clock::now();
          if (now >= next)
            break;
          if (next - now > kTail)
            std::this_thread::sleep_for(next - now - kTail);
          else
            std::this_thread::yield();
        }
      }
      banjo::display_sync::NoteRelease(release, anchored);
      return;
    }
    previousRelease = 0;
  }

  next += frameDur;
  if (next <= now) {
    next = now;  // behind: resync rather than burst to catch up
    return;
  }
#if defined(_WIN32)
  HANDLE timer = FrameTimer();
#endif
  constexpr auto kSpinTail = std::chrono::microseconds(400);
  for (;;) {
    now = clock::now();
    if (now >= next)
      break;
    const auto remaining = next - now;
    if (remaining > kSpinTail) {
      const auto waitFor = remaining - kSpinTail;
#if defined(_WIN32)
      if (timer) {
        LARGE_INTEGER due;
        due.QuadPart = -static_cast<LONGLONG>(std::chrono::duration_cast<std::chrono::nanoseconds>(waitFor).count() /
                                              100);
        if (SetWaitableTimer(timer, &due, 0, nullptr, nullptr, FALSE))
          WaitForSingleObject(timer, INFINITE);
        else
          std::this_thread::sleep_for(waitFor);
      } else {
        std::this_thread::sleep_for(waitFor);
      }
#else
      std::this_thread::sleep_for(waitFor);
#endif
    } else {
      std::this_thread::yield();
    }
  }
}

// The engine's shader compile mode mirrored from sync_shader_compile (only when it changes).
void renutApplyShaderCompileMode() {
  static int applied = -1;
  const bool sync = REXCVAR_GET(sync_shader_compile);
  if (static_cast<int>(sync) != applied) {
    applied = static_cast<int>(sync);
    rex::cvar::SetFlagByName("async_shader_compilation", sync ? "false" : "true");
  }
}

void FPSCounter::Tick() {
  auto Time = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> delta = Time - lastTick;
  lastTick = Time;
  float ms = static_cast<float>(delta.count());
  frameTimes.push_back(ms);
  if (frameTimes.size() > AverageCount)
    frameTimes.erase(frameTimes.begin());
  float total = 0.0f;
  for (float f : frameTimes)
    total += f;
  averageMs = total / frameTimes.size();
  averageFps = 1000.0f / averageMs;
}

// The game's frames (one per D3DDevice_Swap): native/nfsmw_nativo_captura.cpp's captures and frame rate read it. In
// banjo-nx its PM4 renderer's Swap hook counted them (me/banjo_me_hooks.cpp).
std::atomic<uint64_t> g_banjo_game_frames{0};

namespace keflings {

void OnGameSwap() {
  g_banjo_game_frames.fetch_add(1, std::memory_order_relaxed);
  static uint32_t swaps = 0;
  if (++swaps <= 6 || (swaps % 600) == 0)
    REXLOG_INFO("game: D3DDevice_Swap {}", swaps);
  renutApplyShaderCompileMode();
  renutFrameLimit();
  banjo::profiler::FrameBoundary();
  fpsManager.GetCreateCounter("Tick")->Tick();
}

}  // namespace keflings
