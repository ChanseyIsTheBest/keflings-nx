// keflings-nx (2026-10-30): what the app framework taken from banjo-nx calls into the N&B-specific sources that this
// port does not build (their hooks name N&B's code). Each is the "nothing to do" answer; a Keflings counterpart, when
// one is written, replaces its entry here.
//
// renut::mnk (keyboard and mouse at N&B's pad hand-over) comes from src/renut_engine/mnk_controls_switch.cpp, built
// on every platform: input is the SDK's (XInput on the PC, the Switch's controllers through sa2-nx's driver).

#include <rex/logging.h>

// renut_engine/cvar_menu.cpp (N&B's in-game cvar menu) wrote renut.toml back; the PC controls overlay
// (overlays/mnk_controls_dialog.h) calls it after a rebind. Not built here: the rebind holds for the session.
void RenutSaveConfig() {
  static bool logged = false;
  if (!logged) {
    logged = true;
    REXLOG_INFO("keflings-nx: settings are not written back to renut.toml (edit the file instead)");
  }
}
