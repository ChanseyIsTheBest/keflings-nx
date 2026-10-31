#pragma once
#include "profiler/banjo_pc_profiler.h"
#include "banjo_pgo.h"
#include <rex/filesystem.h>
#include <rex/platform.h>
#include <rex/rex_app.h>
#include "renut_engine/overlays/fps_overlay_dialog.h"
#include "renut_engine/renut_logging.h"
#include "renut_engine/overlays/renut_logging_overlay.h"
#if !REX_PLATFORM_SWITCH
#include "renut_engine/overlays/path_setup_wizard.h"
#include "renut_engine/overlays/mnk_controls_dialog.h"
#endif
#include "renut_engine/mnk_controls.h"
#include "renut_engine/Timer.h"
#include "renut_engine/Fps.h"
#include "renut_engine/hooks.h"
#include "renut_engine/title_update.h"  // banjo-nx (2026-10-29): title update 3
#include "renut_engine/dlc_install.h"   // banjo-nx (2026-10-29): reNut upstream's DLC installer
#include "keflings/keflings_title_settings.h"  // keflings-nx (2026-10-30): title update 2's first-start settings
#include "keflings/keflings_user_picture.h"     // keflings-nx (2026-10-31): the console user's picture
#include "keflings/keflings_language.h"         // keflings-nx (2026-10-31): the game's language
#if KEFLINGS_BUILTIN_CONFIG
#include "keflings_builtin_config.h"  // keflings-nx (2026-10-31): generated from app/keflings_switch*.toml
#endif
#include "profiler/profiler.h"
#include "profiler/profiler_overlay.h"
// banjo-nx (2026-10-16): Renut's Direct3D-level renderer, the boot choice banjo_renderer = "renut"
// (app/src/renut_engine/gpu, tools/renut_gpu_import.py, docs/renut-renderer-port.md).
#if BANJO_RENUT_RENDERER
#include "renut_engine/gpu/banjo_renut_select.h"
#include "renut_engine/gpu/overlay/app_integration.h"
#include "renut_engine/gpu/banjo_renut_capture.h"
#endif
#include "cpu/banjo_enhancements.h"
#include "native/nfsmw_nativo_captura.h"
#if !REX_PLATFORM_SWITCH
#include "profiler/nfsmw_perfil_pc.h"
#endif
#include <rex/ui/window.h>
//#include <rex/discord_rpc.h>
#include <cctype>   // keflings-nx (2026-10-31): keflings_language
#include <cstdlib>
#include <utility>
#include <filesystem>
#include <functional>
#include <string>

REXCVAR_DECLARE(bool, banjo_show_fps);  // banjo-nx (2026-10-21): profiler/profiler.cpp

REXCVAR_DECLARE(bool, banjo_mesa_disk_cache_off);
REXCVAR_DECLARE(bool, masseffect_nvk_cache_per_stage);
REXCVAR_DECLARE(bool, banjo_gpu_boost);
#if BANJO_RENUT_RENDERER
REXCVAR_DECLARE(bool, banjo_renut_linked_stages);  // renut_engine/gpu/banjo_renut_tuning.cpp
#endif
REXCVAR_DECLARE(bool, banjo_gpu_zcull_greater);

// keflings-nx (2026-10-31, the user's request): the game's picture boxes (the Achievement dialog's picture, white
// before) show a console user's picture (keflings/keflings_user_picture.h).
REXCVAR_DEFINE_BOOL(keflings_user_picture, true, "Keflings",
                    "The game's picture boxes (the Achievement dialog's picture) show a console user's picture "
                    "instead of a white square. Off: white, as before")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_UINT32(keflings_user_picture_user, 1, "Keflings",
                      "Whose picture keflings_user_picture shows: the console's user number in the HOME Menu's order "
                      "(1 = the first user, 2 the second, ...); a number past the last user takes the first")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_STRING(keflings_user_picture_file, "", "Keflings",
                      "A JPEG or PNG for keflings_user_picture in place of the console user's picture (the PC has "
                      "no console users)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
// keflings-nx (2026-10-31, the user's request): the game's language by name. Keflings has eight (its table: english,
// spanish, french, german, italian, chinese, japanese, korean); this sets the SDK's user_language, which XGetLanguage
// and the XConfig language setting report to the game (the XDK's numbers). Empty: the system's language.
REXCVAR_DEFINE_STRING(keflings_language, "", "Keflings",
                      "The game's language: en, ja, de, fr, es, it, ko or zh (Chinese); empty = the console's system "
                      "language (English when the game does not have it)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DECLARE(uint32_t, user_language);  // the SDK's (xam_user.cpp)
#if REX_PLATFORM_SWITCH
extern "C" {
void RexSwitchApmRequestGpuMhz(int mhz);  // sdk/src/ui/switch_apm.cpp
void RexSwitchApmApply(void);
}
#endif

class RenutApp : public rex::ReXApp {
public:
    RenutApp(rex::ui::WindowedAppContext& ctx, std::string_view name, rex::PPCImageInfo info)
        : rex::ReXApp(ctx, name, info), app_name_(name) {}

    static std::unique_ptr<rex::ui::WindowedApp> Create(
        rex::ui::WindowedAppContext& ctx) {
        // keflings-nx: its own app name (window title, keflings-nx.cfg path store, user data), not banjo-nx's "renut".
        return std::unique_ptr<RenutApp>(new RenutApp(ctx, "keflings-nx", PPCImageConfig));
    }

    // keflings-nx (2026-10-31, the user's request "only language, input, MSAA and the player icon in the toml"): the
    // Switch build's fixed settings (app/keflings_switch_defaults.toml, compiled in) are applied before renut.toml,
    // which holds only the player's options (and may still override any of these). As sa2-nx / bejeweledblitz-nx do.
    std::string_view BuiltInConfig() const override {
#if KEFLINGS_BUILTIN_CONFIG
        return kKeflingsBuiltInConfig;
#else
        return {};
#endif
    }
    // Without a renut.toml next to the NRO, the player's default one is written there (app/keflings_switch.toml).
    std::string_view DefaultConfigFile() const override {
#if KEFLINGS_BUILTIN_CONFIG
        return kKeflingsPlayerConfig;
#else
        return {};
#endif
    }

    void OnPostInitLogging() override {
        // keflings_language (from bejeweledblitz-nx): empty = the system's language, unless user_language was set to
        // another than English.
        std::string language = REXCVAR_GET(keflings_language);
        if (language.empty() && REXCVAR_GET(user_language) == 1) {
            std::string system_name;
            language = keflings::language::FromSystem(&system_name);
            REXLOG_INFO("[language] keflings_language empty: the system language ({}) gives \"{}\"", system_name,
                        language);
        }
        if (!language.empty()) {
            static const std::pair<const char*, const char*> kLanguages[] = {
                {"en", "1"}, {"ja", "2"}, {"de", "3"}, {"fr", "4"}, {"es", "5"}, {"it", "6"}, {"ko", "7"}, {"zh", "8"}};
            const char* id = nullptr;
            for (const auto& [name, value] : kLanguages) {
                if (language.size() >= 2 && std::tolower(static_cast<unsigned char>(language[0])) == name[0] &&
                    std::tolower(static_cast<unsigned char>(language[1])) == name[1]) {
                    id = value;
                }
            }
            if (id) {
                rex::cvar::SetFlagByName("user_language", id);
                REXLOG_INFO("[language] \"{}\": user_language = {}", language, id);
            } else {
                REXLOG_WARN("[language] keflings_language = \"{}\" is not one of en, ja, de, fr, es, it, ko, zh: "
                            "unchanged", language);
            }
        }
    }


    // void OnPostSetup() override {
    //     rex::discord_rpc::Presence rpc;

    //     rpc.details_ = "";
    //     rpc.state_ = "";
    //     rpc.large_image_key_ = "e242d6b6-c34e-47a1-8c2a-5297fe33bce7";
    //     rpc.large_image_text_ = "renut";

    //     //rex::discord_rpc::Start(Application ID, Settings);
    //     rex::discord_rpc::Start("1520303728047951892", rpc);

    //     rex::cvar::LoadConfig("renut.toml");
    // }

    void OnCreateDialogs(rex::ui::ImGuiDrawer* drawer) override {
        //drawer->AddDialog(new FpsOverlayDialog(drawer));
        fps_dialog_ = std::make_unique<FpsOverlayDialog>(drawer);
        fps_dialog_->fpsManager = &fpsManager;
        fpsManager.showFPS = REXCVAR_GET(banjo_show_fps);  // banjo-nx (2026-10-21)
        drawer->AddDialog(fps_dialog_.get());
        drawer->AddDialog(new RenuLogOverlayDialog(drawer));
        // CPU/GPU profiler window ported from UnleashedRecomp (F5 / banjo_show_profiler).
        profiler_dialog_ = std::make_unique<ProfilerOverlayDialog>(drawer);
        drawer->AddDialog(profiler_dialog_.get());
#if !REX_PLATFORM_SWITCH
        path_wizard_ = new PathSetupWizard(drawer);
        drawer->AddDialog(path_wizard_);

        // Keyboard & mouse: the window exists by the time dialogs are created,
        // so this is where the host-side listener gets attached.
        mnk_dialog_ = std::make_unique<MnkControlsDialog>(drawer);
        drawer->AddDialog(mnk_dialog_.get());
        renut::mnk::AttachWindow(window());
#endif
    }

    // Native renderer: with masseffect_renderer_native = true the graphics system is the app's own
    // (src/me, from masseffect-nx) and the emulated Xenos GPU plugin is not loaded (ReXApp only loads it
    // when config.graphics is empty).
    void OnPreSetup(rex::RuntimeConfig& config) override {
        banjo::ApplyMasterSwitches();
#if REX_PLATFORM_SWITCH
        // Before the Vulkan device exists (the graphics system creates it after this). Mesa's own disk cache
        // (fossilize file on the SD card) duplicates the renderer's pipeline cache file, and on the console its
        // writer thread held the cache lock during slow SD writes while the ring thread, in the middle of a
        // vkCreateGraphicsPipelines, waited on that lock for its lookup (switch stall dumps). Same pipelines.
        if (banjo::EnhancementActive(banjo::EnhancementGroup::kNonCpu, REXCVAR_GET(banjo_mesa_disk_cache_off))) {
            setenv("MESA_SHADER_CACHE_DISABLE", "true", 1);
        }
        // banjo_gpu_zcull_greater: the driver reads NVK_ZCULL once, at its first render pass.
        if (banjo::EnhancementActive(banjo::EnhancementGroup::kNonCpu, REXCVAR_GET(banjo_gpu_zcull_greater))) {
            setenv("NVK_ZCULL", "greater", 1);
        }
        // Per-stage shader cache keys in NVK (the patched driver reads it at vkCreateDevice; an unpatched one ignores
        // it): a vertex or pixel shader already compiled for another pipeline is not compiled again. In this driver
        // the linked compile only drops vertex outputs the pixel shader does not read, so the image is the same.
        bool per_stage_cache = banjo::EnhancementActive(banjo::EnhancementGroup::kNonCpu,
                                                        REXCVAR_GET(masseffect_nvk_cache_per_stage));
#if BANJO_RENUT_RENDERER
        // banjo_renut_linked_stages (package 26): not with Renut's renderer. Per-stage keys compile each stage without
        // the other (disable_lto), which banjo-nx's translator allows for by pruning the vertex outputs itself; Renut's
        // vertex shaders write every output (zeros for the unused ones), and only the linked compile drops the ones the
        // pixel shader does not read.
        if (per_stage_cache && banjo::renut_gpu::Active() &&
            banjo::EnhancementActive(banjo::EnhancementGroup::kNonCpu, REXCVAR_GET(banjo_renut_linked_stages))) {
            per_stage_cache = false;
        }
#endif
        if (per_stage_cache) {
            setenv("NVK_SWITCH_STAGE_CACHE", "1", 1);
        }
        // banjo_gpu_boost (from sa2-nx): the official handheld GPU profile; switch_apm.cpp logs what it did on stderr
        // (rex_stderr.log) and requests nothing when docked.
        if (banjo::EnhancementActive(banjo::EnhancementGroup::kNonCpu, REXCVAR_GET(banjo_gpu_boost))) {
            RexSwitchApmRequestGpuMhz(460);
            RexSwitchApmApply();
        }
#endif
#if BANJO_RENUT_RENDERER
        if (banjo::renut_gpu::Active()) {
            // No SDK graphics system and no GPU plugin: the renderer is the app's (detached mode).
            renut::gpu::ConfigureRuntime(config);
            banjo::profiler::SetRendererInfo("Renut native (Direct3D level, plume)", "");
            return;
        }
#endif
        // keflings-nx: Renut's renderer is the only native one (banjo-nx's PM4 renderers are not built); without it the
        // SDK's emulated GPU runs (gpu_plugin).
    }

#if BANJO_RENUT_RENDERER
    // Renut's renderer draws the SDK's ImGui through its own plume drawer, and resizes its swapchain itself.
    std::unique_ptr<rex::ui::ImmediateDrawer> OnCreateImmediateDrawer() override {
        if (banjo::renut_gpu::Active()) return renut::gpu::CreateOverlayDrawer();
        return rex::ReXApp::OnCreateImmediateDrawer();
    }
    void OnWindowPixelSizeChanged(uint32_t pixel_width, uint32_t pixel_height) override {
        rex::ReXApp::OnWindowPixelSizeChanged(pixel_width, pixel_height);
        if (banjo::renut_gpu::Active()) renut::gpu::OnWindowResized();
    }
    // The window exists here (the path wizard is drawn before the game starts): Renut's renderer comes up first.
    void StartRenutRenderer() {
        if (banjo::renut_gpu::Active())
            renut::gpu::StartRenderer(app_context(), [this] { return imgui_drawer(); });
    }
#else
    void StartRenutRenderer() {}
#endif

    void OnPostSetup() override {
        // banjo-nx (2026-10-29): DLC packages in the "dlc" folder next to banjo-nx installed before the game starts
        // (reNut upstream's installer, renut_engine/dlc_install.cpp).
        renut::dlc::InstallPackages(runtime());
        keflings::SeedTitleSettings();  // keflings-nx: before the game reads its title settings
        // keflings-nx (2026-10-31): the console user's picture, read before the game asks for it.
        if (REXCVAR_GET(keflings_user_picture)) {
            REXLOG_INFO("[keflings] user picture: {}",
                        keflings::user_picture::Install(REXCVAR_GET(keflings_user_picture_user),
                                                        REXCVAR_GET(keflings_user_picture_file)));
        }
        // One log line with every enhancement and whether it is active (src/cpu/banjo_enhancements.h).
        banjo::LogEnhancements();
        // PC: function-level CPU profile (banjo_pc_profile_s = N seconds per report; 0 = off; from futari-nx).
        banjo::pc_profiler::Start();
        // Switch: per-thread CPU accounting, the optional sampler and the stall watchdog.
        banjo::profiler::Start();
        // PNG captures of the game image for testing (nfsmw_captura_cada_s = N seconds; 0 = off). Taken from
        // the presenter, so no overlay or window can cover them.
#if BANJO_RENUT_RENDERER
        // Renut's renderer has no SDK presenter: its own Present reads the frame back (banjo_renut_capture.h).
        if (banjo::renut_gpu::Active()) {
            nfsmw::captura::Arrancar(nfsmw::captura::Capturador(
                [](rex::ui::RawImage& image) { return renut::gpu::CaptureFrame(image); }));
        } else
#endif
        nfsmw::captura::Arrancar([this]() -> rex::ui::Presenter* {
            const auto* rt = runtime();
            const auto* graphics = rt ? rt->graphics_system() : nullptr;
            return graphics ? graphics->presenter() : nullptr;
        });
#if !REX_PLATFORM_SWITCH
        // Sampling CPU profiler for PC tests (nfsmw_perfil_pc_desde_s; off by default).
        nfsmw::perfil_pc::Arrancar();
#endif
        // The PGO-instrumented build (BANJO_PGO=generate) saves its counts to pgo/ every 2 minutes (nothing otherwise).
        banjo::pgo::Start();
    }

    // The PGO-instrumented build saves its counts when the game loses focus (HOME menu): it may be closed from there.
    void OnWindowFocusChanged(bool focused) override {
        if (!focused) banjo::pgo::DumpNow();
    }

    void OnShutdown() override {
#if BANJO_RENUT_RENDERER
        if (banjo::renut_gpu::Active()) renut::gpu::ShutdownRenderer();
#endif
        banjo::pc_profiler::Stop();
        nfsmw::captura::Parar();
#if !REX_PLATFORM_SWITCH
        nfsmw::perfil_pc::Parar();
#endif
        renut::mnk::DetachWindow();
        banjo::pgo::Stop();
    }

    // A game_root folder next to the executable is used when no path was given.
    // On the Switch that is sdmc:/switch/banjo-nx/game_root (the extracted disc).
    // banjo-nx (2026-10-29): then title update 3's update folder and its default.xexp check
    // (renut_engine/title_update.h), and the user data folder the DLC installer looks in.
    void OnConfigurePaths(rex::PathConfig& paths) override {
        // keflings-nx (2026-10-30): the cvars file is renut.toml next to the executable, as in banjo-nx's packages
        // (the app's name, keflings-nx, would make it keflings-nx.toml; the name keeps the user data apart from
        // banjo-nx's "renut" folder).
        paths.config_path = rex::filesystem::GetExecutableFolder() / "renut.toml";
        if (paths.game_data_root.empty()) {
            const auto folder = rex::filesystem::GetExecutableFolder();
            std::error_code ec;
            const auto extracted = folder / "game_root";
            if (!folder.empty() && std::filesystem::is_directory(extracted, ec)) {
                paths.game_data_root = extracted;
            }
        }
        banjo::title_update::ConfigurePaths(paths);
        renut::dlc::SetUserDataRoot(paths.user_data_root);
        keflings::SetUserDataRoot(paths.user_data_root);
    }

#if !REX_PLATFORM_SWITCH
    std::optional<rex::PathConfig> OnFinalizePaths(
        const rex::PathConfig& defaults,
        std::function<void(rex::PathConfig)> resume) override
    {
        StartRenutRenderer();
        path_wizard_->Init(app_name_, defaults, [resume](rex::PathConfig resolved) {
            // banjo-nx (2026-10-29): the wizard's game folder (the PC) gets the update folder and its check too.
            banjo::title_update::ConfigurePaths(resolved);
            renut::dlc::SetUserDataRoot(resolved.user_data_root);
            keflings::SetUserDataRoot(resolved.user_data_root);
            resume(resolved);
            });
        return std::nullopt;
    }
#else
    std::optional<rex::PathConfig> OnFinalizePaths(
        const rex::PathConfig& defaults,
        std::function<void(rex::PathConfig)> resume) override
    {
        StartRenutRenderer();
        return rex::ReXApp::OnFinalizePaths(defaults, std::move(resume));
    }
#endif

private:
    std::string      app_name_;
#if !REX_PLATFORM_SWITCH
    PathSetupWizard* path_wizard_ = nullptr;
    std::unique_ptr<MnkControlsDialog> mnk_dialog_;
#endif
    std::unique_ptr<FpsOverlayDialog> fps_dialog_;
    std::unique_ptr<ProfilerOverlayDialog> profiler_dialog_;
};
