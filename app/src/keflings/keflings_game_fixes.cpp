// keflings-nx (2026-10-30): fixes of the game's own code (mid-asm hooks of app/config/renut_hooks.toml).
//
// Each changes only a path where the original would crash; the key switches it off (the original instructions run).

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc.h>

REXCVAR_DEFINE_BOOL(keflings_fix_no_current_screen, true, "Keflings",
                    "A screen's constructor (sub_821C99A0) with no current screen in the screen manager returns "
                    "instead of calling through a null vtable (the original's unchecked second read)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

REXCVAR_DEFINE_BOOL(keflings_diag_image_capture, true, "Keflings",
                    "Log the first calls of the game's image capture (sub_8212F860: renders into a render target, "
                    "resolves it into a line texture and copies that to CPU memory; Renut does not read resolves "
                    "back, so the copy holds zeros)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

// 2026-10-30 (evening, title update 2): the avatar giant (app/config/renut_hooks.toml, 0x82158F74).
REXCVAR_DEFINE_BOOL(keflings_avatar_giant, false, "Keflings",
                    "Title update 2's avatar giant (\"You\"): off without avatar data (this runtime), so the giant "
                    "lookup never makes it and Choose your Giant offers the four giants of the base game")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

// r9 == 0: the name the giant lookup (sub_82158E80) was asked for is "avatars". true: go to its exit.
bool keflings_skip_avatar_giant(PPCRegister& r9) {
  if (r9.u32 != 0) return false;
  static const bool keep = REXCVAR_GET(keflings_avatar_giant);
  if (keep) return false;
  static bool logged = false;
  if (!logged) {
    logged = true;
    REXLOG_INFO("[keflings] avatar giant not made (no avatar data; keflings_avatar_giant)");
  }
  return true;
}

REXCVAR_DEFINE_BOOL(keflings_fix_null_object, true, "Keflings",
                    "A game object of kind 'None' (sub_8216E830 through the factory sub_8217CDC0) gives null instead "
                    "of a call through its null vtable (title update 2 on the Switch)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

// 0x8216E8E4: r27 = the object the factory made. true: go to the function's return (it returns r27, null).
bool keflings_null_object(PPCRegister& r27) {
  if (r27.u32 != 0) [[likely]] return false;
  static const bool on = REXCVAR_GET(keflings_fix_null_object);
  if (!on) return false;
  static int logged = 0;
  if (logged < 4) {
    ++logged;
    REXLOG_WARN("[keflings] object of kind 'None' not made (sub_8216E830 returns null; keflings_fix_null_object)");
  }
  return true;
}

// The image capture is reached only through its class's vtable (0x82064940): this definition replaces the function
// table's entry and runs the original after the log line.
REX_EXTERN(__imp__sub_8212F860);
extern "C" REX_FUNC(sub_8212F860) {
  static const bool on = REXCVAR_GET(keflings_diag_image_capture);
  static int logged = 0;
  if (on && logged < 8) {
    ++logged;
    auto word = [base](uint32_t a) { return __builtin_bswap32(*reinterpret_cast<const uint32_t*>(base + a)); };
    const uint32_t obj = ctx.r3.u32;
    REXLOG_WARN("[keflings] image capture (sub_8212F860) #{}: image {:08X} {}x{} (+16 {:08X}), callback {:08X}, "
                "lr {:08X}: its CPU copy will be zeros (no resolve read-back)",
                logged, obj, word(obj + 8), word(obj + 12), word(obj + 16), ctx.r4.u32, uint32_t(ctx.lr));
  }
  __imp__sub_8212F860(ctx, base);
}

// 0x821AB438, before lbz r10,10460(r3): r3 = the manager's current screen as read the second time. true: go to the
// function's return (0x821AB470).
bool keflings_no_current_screen(PPCRegister& r3) {
  if (r3.u32 != 0) [[likely]] return false;
  static const bool on = REXCVAR_GET(keflings_fix_no_current_screen);
  if (!on) return false;
  static bool logged = false;
  if (!logged) {
    logged = true;
    REXLOG_WARN("[keflings] screen constructor with no current screen: skipped its calls to the current screen "
                "(sub_821C99A0, keflings_fix_no_current_screen)");
  }
  return true;
}
